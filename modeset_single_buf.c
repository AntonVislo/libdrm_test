#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include "cbmp.h"

#define CRTC_ID_DP 68
#define CONN_ID_DP 205
#define CRTC_ID_HDMI 112
#define CONN_ID_HDMI 207
#define MAX_CONN 10

struct bufferObject{
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t handle;
    uint32_t size;
    uint8_t *vaddr;
    uint32_t fb_id;
};

typedef struct connectorObject{
    uint32_t conn_id;
    uint32_t encod_id;
    uint32_t crtc_id;
    const char *name;
    struct bufferObject buf;
    drmModeModeInfo *crtc_mode;
}connectorObject;

int imgToFb(char *img, struct bufferObject *bo)
{
    BMP* bmp = bopen(img);
    unsigned int x, y, width, height;
    unsigned char r, g, b;
    unsigned char *offset = bo->vaddr;
    width = get_width(bmp);
    height = get_height(bmp);
    for(y = 0; y < bo->height; y++)
    {
       for(x = 0; x < bo->width; x++)
       {
	    if(x < width && y < height)
		{
            get_pixel_rgb(bmp,x , (height - y - 1), &r, &g, &b);
            *(offset + 0) = r;
            *(offset + 1) = g;
            *(offset + 2) = b;
		}
	    offset +=4;		
        }
    }
    bclose(bmp);
    return 0;
}

static int modesetCreatefb(int fd,struct  bufferObject *bo)
{
    struct drm_mode_create_dumb create = {};
     struct drm_mode_map_dumb map = {};

    /* create a dumb-buffer, the pixel format is XRGB888 */
    create.width = bo->width;
    create.height = bo->height;
    create.bpp = 32;
    drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &create);

    /* bind the dumb-buffer to an FB object */
    bo->pitch = create.pitch;
    bo->size = create.size;
    bo->handle = create.handle;
    drmModeAddFB(fd, bo->width, bo->height, 24, 32, bo->pitch,
               bo->handle, &bo->fb_id);

    /* map the dumb-buffer to userspace */
    map.handle = create.handle;
    drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &map);

    bo->vaddr = mmap(0, create.size, PROT_READ | PROT_WRITE,
            MAP_SHARED, fd, map.offset);

    /* initialize the dumb-buffer with white-color */
    memset(bo->vaddr, 0xff, bo->size);

    return 0;
}

static void modesetDestroyfb(int fd,struct  bufferObject *bo)
{
    struct drm_mode_destroy_dumb destroy = {};

    drmModeRmFB(fd, bo->fb_id);

    munmap(bo->vaddr, bo->size);

    destroy.handle = bo->handle;
    drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy);
}

int createfb(int fd, struct bufferObject *bo, uint32_t conn_id, uint32_t crtc_id, drmModeModeInfo *mod)
{
    drmModeConnector *conn;
    drmModeCrtc *crtc;
    conn = drmModeGetConnector(fd, conn_id);
    crtc = drmModeGetCrtc(fd, crtc_id);
    bo->width = mod->hdisplay;
    bo->height = mod->vdisplay;
    printf("\tcreate buf, width: %d, height: %d\n",bo->width, bo->height);
    printf("\tset crtc mode %s\n",mod->name);
    modesetCreatefb(fd, bo);
    drmModeSetCrtc(fd, crtc_id, bo->fb_id,
            0, 0, &conn_id, 1, mod);
    drmModeFreeConnector(conn);
    drmModeFreeCrtc(crtc);
    return 0;   
}
int main(int argc, char **argv)
{
    int fd;
    uint8_t countConnectedConn = 0;
    connectorObject connectors[MAX_CONN];
    drmModeRes *res;
    if (argc < 2)
	{
		printf("not enough arguments");
		return -1;
	}
    if(access(argv[1], F_OK)!= 0)
	{
		printf("file %s does not exist\n", argv[1]);
                return -1;
	}

    fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
    //get all drm resources
    res = drmModeGetResources(fd);
    printf("\tfind %d connectors in system\n", res->count_connectors);
    // find connected connectors from all avaliable
    for(int i = 0; i < res->count_connectors; i++)
    {
        drmModeConnectorPtr conn = drmModeGetConnector(fd, res->connectors[i]);
        const char *name = drmModeGetConnectorTypeName(conn->connector_type);
        printf("\t****CONN# %d*****\n", i);
        printf("\tconnector type name: %s\n", name);
        if (conn->connection==DRM_MODE_CONNECTED)
				{
                    // for all connected connectors
					printf("\tconnector status: connected\n");
					printf("\tencoder id: %d\n", conn->encoder_id);
                    //get encoder struct to find crtc id
					drmModeEncoderPtr enc = drmModeGetEncoder(fd, conn->encoder_id);
					printf("\tcrtc id: %d\n", enc->crtc_id);
                    if(countConnectedConn < MAX_CONN){
                        connectors[countConnectedConn].conn_id = conn->connector_id;
                        connectors[countConnectedConn].name = name;
                        connectors[countConnectedConn].encod_id = conn->encoder_id;
                        connectors[countConnectedConn].crtc_id = enc->crtc_id;
                        connectors[countConnectedConn].crtc_mode = &conn->modes[0];
                        //create frame buffer and attach it to crtc
                        createfb(fd, &connectors[countConnectedConn].buf, 
                                connectors[countConnectedConn].conn_id,
                                connectors[countConnectedConn].crtc_id,
                                connectors[countConnectedConn].crtc_mode);
                        countConnectedConn++;
                        printf("\tsave connector object, num: %d\n", countConnectedConn);
                    }
                    else{
                        printf("\tcouldn't create connector object, limit reached\n");
                    }
                    drmModeFreeEncoder(enc);
				}
				else if (conn->connection==DRM_MODE_DISCONNECTED)
					printf("\tconnector status: disconnected\n");
				else
					printf("\tconnector status: unknown connection\n");
        drmModeFreeConnector(conn);
    }
    drmModeFreeResources(res);
    int col = 1;
    printf("\tstart load image %s to fb\n", argv[1]);
    for (int i = 0; i < countConnectedConn; i++)
    {
        imgToFb(argv[1], &connectors[i].buf);
    }
    printf("\tEnter num to exit\n");
    scanf("%d",&col);
    for (int i = 0; i < countConnectedConn; i++)
    {
        modesetDestroyfb(fd, &connectors[i].buf);
    }
    close(fd);
    return 0;
}


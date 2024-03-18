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

struct buffer_object{
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t handle;
    uint32_t size;
    uint8_t *vaddr;
    uint32_t fb_id;
};



struct buffer_object buf;
struct buffer_object buf_hdmi;

int imgToFb(const char *img, struct buffer_object *bo)
{
    BMP* bmp = bopen(img);
    unsigned int x, y, width, height;
    unsigned char r, g, b;
    unsigned char *offset = bo->vaddr;
    width = get_width(bmp);
    height = get_height(bmp);
    printf("height: %d, width: %d\n", width, height);
    for(y = 0; y < buf.height; y++)
    {
       for(x = 0; x < buf.width; x++)
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
    bwrite(bmp, "./picture1.bmp");
    bclose(bmp);
    return 0;
}

static int modeset_create_fb(int fd,struct  buffer_object *bo)
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

static void modeset_destroy_fb(int fd,struct  buffer_object *bo)
{
    struct drm_mode_destroy_dumb destroy = {};

    drmModeRmFB(fd, bo->fb_id);

    munmap(bo->vaddr, bo->size);

    destroy.handle = bo->handle;
    drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy);
}

int main(int argc, char **argv)
{
    int fd;
    drmModeConnector *conn;
    drmModeCrtc *crtc;
    drmModeRes *res;
    uint32_t conn_id;
    uint32_t crtc_id;
    if (argc < 3)
	{
		printf("not enough arguments");
		return -1;
	}
    if(access(argv[1], F_OK)!= 0)
	{
		printf("file %s does not exist\n", argv[1]);
                return -1;
	}
   if(access(argv[2], F_OK)!= 0)
	{
		printf("file %s does not exist\n", argv[2]);
                return -1;
	} 
    fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
    res = drmModeGetResources(fd);
    /************INIT DP FB*************/
    crtc_id = CRTC_ID_DP;
    conn_id = CONN_ID_DP;
    conn = drmModeGetConnector(fd, conn_id);
    crtc = drmModeGetCrtc(fd, crtc_id);
    buf.width = crtc->mode.hdisplay;
    buf.height = crtc->mode.vdisplay;
    printf("buf.width: %d, buf.height: %d\n",buf.width, buf.height);
    modeset_create_fb(fd, &buf);
    drmModeSetCrtc(fd, crtc_id, buf.fb_id,
            0, 0, &conn_id, 1, &crtc->mode);
    drmModeFreeConnector(conn);
    drmModeFreeCrtc(crtc);   
    /************INIT HDMI FB************/
    crtc_id = CRTC_ID_HDMI;
    conn_id = CONN_ID_HDMI;
    conn = drmModeGetConnector(fd, conn_id);
    crtc = drmModeGetCrtc(fd, crtc_id);
    buf_hdmi.width = crtc->mode.hdisplay;
    buf_hdmi.height = crtc->mode.vdisplay;
    printf("buf-hdmi.width: %d, buf_hdmi.height: %d\n",buf_hdmi.width, buf_hdmi.height);
    modeset_create_fb(fd, &buf_hdmi);
    drmModeSetCrtc(fd, crtc_id, buf_hdmi.fb_id,
            0, 0, &conn_id, 1, &crtc->mode);


 int col = 1;
    while(col >= 0 ){
    	printf("Enter colour for fb, print -1 to exit\n");
    	scanf("%d",&col);
	printf("you enter: %d\n", col);
    	for (int i = 0 ; i < buf.width*buf.height; i++)
		{
			*(buf.vaddr + i*4 + 1) = *(buf_hdmi.vaddr + i*4 + 1) =  0;
			*(buf.vaddr + i*4 + 2) = *(buf_hdmi.vaddr + i*4 + 2) = (char)col&0xff;
			*(buf.vaddr + i*4 + 3) = *(buf_hdmi.vaddr + i*4 + 3) = 0; 
		}
    }
    imgToFb(argv[1], &buf);
    imgToFb(argv[2], &buf_hdmi);
    printf("Enter colour for fb, print -1 to exit\n");
    scanf("%d",&col);
    modeset_destroy_fb(fd, &buf);
    modeset_destroy_fb(fd, &buf_hdmi);
    drmModeFreeCrtc(crtc);
    drmModeFreeConnector(conn);
    drmModeFreeResources(res);

    close(fd);

    return 0;
}


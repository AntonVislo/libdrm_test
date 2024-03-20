#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <libdrm/drm.h>
#include <xf86drmMode.h>
#include <xf86drm.h>
#include <libdrm_test.h>
int enum_drm(void)
{
	char *files[] = {"/dev/dri/card0", "/dev/dri/card1"};
	for(int cnt_files = 0; cnt_files < 2; cnt_files ++)
	{ 
		printf("file: %s", files[cnt_files]);
		int fd = open(files[cnt_files], O_RDWR | O_CLOEXEC);
		if(fd < 0){
			printf(" can`t open card0\n");
			return -1;
		}
		printf("card 0 open\n");
		
		if(drmIsKMS(fd))
			printf("DRM support KMS\n");
		else
		{	
			printf("Error, DRM not support KMS\n");
			close(fd);
			return 0;
		}
		uint64_t has_dump;
		if(drmGetCap(fd, DRM_CAP_DUMB_BUFFER, &has_dump) < 0 || !has_dump)
		{
			printf("error, card 0 does not support bump buffer\n");
			close(fd);
			return -1;
		}

		drmModeResPtr res = drmModeGetResources(fd);
		if(!res)
		{
			printf("error get resources\n");
			close(fd);
			return -1;
		}
		//crtc
		printf("***************PRINT CRTC*****************\n");
		for(int i = 0; i < res->count_crtcs; i++)
		{
			drmModeCrtcPtr crtc = drmModeGetCrtc(fd, res->crtcs[i]);
			if(!crtc)
			{
				printf("error get crtc\n");
			}
			else
			{
				printf("\tcrtc id: %d, mode name: %s\n", 
				crtc->crtc_id, crtc->mode.name);
			}
		}
		
		printf("*************PRINT CONNECTORS**************\n");
		for (int i = 0; i < res->count_connectors; i++)
		{

			drmModeConnector *conn = drmModeGetConnector(fd, res->connectors[i]);
	        	if (!conn) {
	            	printf("cannot retrieve DRM connector\n");
	        	}
			else{
				printf("#%d\n\tconnector id: %d\n",i ,conn->connector_id);
				const char *name = drmModeGetConnectorTypeName(conn->connector_type);
				printf("\tconnector type: %s\n", name);
				if (conn->connection==DRM_MODE_CONNECTED)
				{
					printf("\tconnector status: connected\n");
					printf("\tencoder id: %d\n", conn->encoder_id);
					drmModeEncoderPtr enc = drmModeGetEncoder(fd, conn->encoder_id);
					printf("\tcrtc id: %d\n", enc->crtc_id);
				}
				else if (conn->connection==DRM_MODE_DISCONNECTED)
					printf("\tconnector status: disconnected\n");
				else
					printf("\tconnector status: unknown connection\n");
				for(int i = 0; i < conn->count_modes; i++){
					printf("\tmode: %s\n", conn->modes[i].name);
				}
			}
		}

		drmModePlaneResPtr planes_res = drmModeGetPlaneResources(fd);
		printf("planes count: %d\n", planes_res->count_planes);

		for (int i = 0; i < planes_res->count_planes; i++)
		{
			printf("plane id:%d\n", planes_res->planes[i]);
			drmModePlanePtr plane = drmModeGetPlane(fd,
						 planes_res->planes[i]);
			printf("\tplane info: count formats: %d\n"
			       "\tcrtc_id: %d, fb_id: %d\n",	
			        plane->count_formats, plane->crtc_id, plane->fb_id);
		}
	close(fd);
	}
	return 0;
}

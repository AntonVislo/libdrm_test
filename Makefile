all: main_drm.c
	gcc -o drm_test main_drm.c -I /usr/include/libdrm/ -ldrm -Wall -O0 -g
mode: modeset_single_buf.c cbmp.c
	gcc -o mode modeset_single_buf.c cbmp.c -I ./ -I /usr/include/libdrm/ -ldrm  -Wall -O0 -g
mode_static: modeset_single_buf.c cbmp.c
	gcc -o mode_static modeset_single_buf.c cbmp.c -I ./ -I /usr/include/libdrm/ -ldrm  -Wall -O0 -g --static



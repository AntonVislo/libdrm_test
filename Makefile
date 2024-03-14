all:
	gcc -o drm_test main_drm.c `pkg-config --cflags --libs libdrm` -Wall -O0 -g
mode:
	gcc -o mode modeset_single_buf.c `pkg-config --cflags --libs libdrm` -Wall -O0 -g

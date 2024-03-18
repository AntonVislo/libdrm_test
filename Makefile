all:
	gcc -o drm_test main_drm.c -I /usr/include/libdrm/ -ldrm -Wall -O0 -g
mode:
	rm -f mode
	gcc -o mode modeset_single_buf.c cbmp.c -I ./ -I /usr/include/libdrm/ -ldrm  -Wall -O0 -g
mode_hdmi:
	rm -f mode_hdmi
	gcc -o mode_hdmi modeset_single_buf_hdmi.c cbmp.c -I ./ -I /usr/include/libdrm/ -ldrm  -Wall -O0 -g


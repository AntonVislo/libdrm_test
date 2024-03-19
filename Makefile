all: modeset_single_buf.c cbmp.c
	gcc -o mode modeset_single_buf.c cbmp.c -I ./ -I /usr/include/libdrm/ -ldrm  -Wall -O0 -g
all_static: modeset_single_buf.c cbmp.c
	gcc -o mode_static modeset_single_buf.c cbmp.c -I ./ -I /usr/include/libdrm/ -ldrm  -Wall -O0 -g --static
enum: enum_drm.c
	gcc -o enum_drm enum_drm.c -I /usr/include/libdrm/ -ldrm -Wall -O0 -g



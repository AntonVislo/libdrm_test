SOURCES=modeset_single_buf.c cbmp.c enum_drm.c

all: $(SOURCES)
	gcc -o mode $(SOURCES) -I ./ -I /usr/include/libdrm/ -ldrm  -Wall -O0 -g
all_static: $(SOURCES)
	gcc -o mode_static $(SOURCES) -I ./ -I /usr/include/libdrm/ -ldrm  -Wall -O0 -g --static
atomic: modeset_atomic_crtc.c
	gcc -o mode_atomic modeset_atomic_crtc.c cbmp.c -I ./ -I /usr/include/libdrm/ -ldrm  -Wall -O0 -g



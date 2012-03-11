CFLAGS = -O2 -W -Wall -g `pkg-config audacious --cflags`
CC = gcc

all:	boost.so

boost.so:	boost.c gui.c
	$(CC) -fPIC -shared $(CFLAGS) -o boost.so boost.c gui.c

install:	boost.so
	install -m 755 boost.so `pkg-config audacious --variable=effect_plugin_dir`

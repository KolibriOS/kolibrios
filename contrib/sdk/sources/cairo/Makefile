
LIBRARY = cairo2

CC = gcc
CFLAGS = -c -O2 -ffast-math -Wall -Winline -Wno-attributes -fomit-frame-pointer

LD = ld
LDFLAGS = -shared -s -nostdlib -T ../newlib/dll.lds --entry _DllStartup --image-base=0 --version-script cairo.ver --output-def $(LIBRARY).orig.def --out-implib $(LIBRARY).dll.a

STRIP = $(PREFIX)strip

DEFINES = -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -U_MSC_VER -DHAVE_CONFIG_H=1 -DCAIRO_NO_MUTEX=1  

INCLUDES = -I. -I../newlib/include -I../pixman -I../zlib -I../libpng -I../freetype/include


LIBPATH:= -L../../lib

LIBS:=  -ldll -lgcc -lfreetype.dll -lz.dll -lpixman-1.dll -lpng16.dll -lc.dll

CAIRO_SOURCES =					\
	cairo.c 				\
	cairo-analysis-surface.c		\
	cairo-arc.c 				\
	cairo-array.c 				\
	cairo-atomic.c 				\
	cairo-base64-stream.c 			\
	cairo-base85-stream.c 			\
	cairo-bentley-ottmann.c 		\
	cairo-bentley-ottmann-rectangular.c 	\
	cairo-bentley-ottmann-rectilinear.c 	\
	cairo-botor-scan-converter.c 		\
	cairo-boxes.c 				\
	cairo-boxes-intersect.c			\
	cairo-cache.c 				\
	cairo-cff-subset.c			\
	cairo-clip.c 				\
	cairo-clip-boxes.c			\
	cairo-clip-polygon.c			\
	cairo-clip-region.c			\
	cairo-clip-surface.c			\
	cairo-clip-tor-scan-converter.c		\
	cairo-color.c 				\
	cairo-composite-rectangles.c 		\
	cairo-compositor.c			\
	cairo-contour.c				\
	cairo-damage.c				\
	cairo-debug.c				\
	cairo-default-context.c			\
	cairo-deflate-stream.c			\
	cairo-device.c 				\
	cairo-error.c				\
	cairo-fallback-compositor.c		\
	cairo-fixed.c 				\
	cairo-font-face.c 			\
	cairo-font-face-twin.c 			\
	cairo-font-face-twin-data.c 		\
	cairo-font-options.c 			\
	cairo-freed-pool.c 			\
	cairo-freelist.c 			\
	cairo-ft-font.c				\
	cairo-gstate.c 				\
	cairo-hash.c 				\
	cairo-hull.c 				\
	cairo-image-compositor.c		\
	cairo-image-info.c 			\
	cairo-image-source.c			\
	cairo-image-surface.c 			\
	cairo-lzw.c 				\
	cairo-mask-compositor.c			\
	cairo-matrix.c 				\
	cairo-mempool.c				\
	cairo-mesh-pattern-rasterizer.c		\
	cairo-misc.c 				\
	cairo-mono-scan-converter.c		\
	cairo-mutex.c 				\
	cairo-no-compositor.c			\
	cairo-observer.c 			\
	cairo-output-stream.c 			\
	cairo-paginated-surface.c 		\
	cairo-path.c 				\
	cairo-path-bounds.c 			\
	cairo-path-fill.c 			\
	cairo-path-fixed.c 			\
	cairo-path-in-fill.c 			\
	cairo-path-stroke.c 			\
	cairo-path-stroke-boxes.c		\
	cairo-path-stroke-polygon.c		\
	cairo-path-stroke-traps.c		\
	cairo-path-stroke-tristrip.c		\
	cairo-pattern.c				\
	cairo-pdf-operators.c			\
	cairo-pdf-shading.c			\
	cairo-pen.c 				\
	cairo-png.c				\
	cairo-polygon.c 			\
	cairo-polygon-intersect.c		\
	cairo-polygon-reduce.c			\
	cairo-raster-source-pattern.c		\
	cairo-recording-surface.c		\
	cairo-rectangle.c			\
	cairo-rectangular-scan-converter.c	\
	cairo-region.c				\
	cairo-rtree.c				\
	cairo-scaled-font.c			\
	cairo-scaled-font-subsets.c		\
	cairo-script-surface.c			\
	cairo-shape-mask-compositor.c		\
	cairo-slope.c				\
	cairo-spans.c				\
	cairo-spans-compositor.c		\
	cairo-spline.c				\
	cairo-stroke-dash.c			\
	cairo-stroke-style.c			\
	cairo-surface.c				\
	cairo-surface-clipper.c			\
	cairo-surface-fallback.c		\
	cairo-surface-offset.c			\
	cairo-surface-snapshot.c		\
	cairo-surface-subsurface.c		\
	cairo-surface-wrapper.c			\
	cairo-svg-surface.c			\
	cairo-tor22-scan-converter.c		\
	cairo-tor-scan-converter.c		\
	cairo-toy-font-face.c			\
	cairo-traps.c				\
	cairo-traps-compositor.c		\
	cairo-tristrip.c			\
	cairo-truetype-subset.c			\
	cairo-type1-fallback.c			\
	cairo-type1-glyph-names.c		\
	cairo-type1-subset.c			\
	cairo-type3-glyph-surface.c		\
	cairo-unicode.c				\
	cairo-user-font.c			\
	cairo-version.c				\
	cairo-wideint.c				\
	$(NULL)


SOURCES= $(CAIRO_SOURCES)

OBJECTS =  $(patsubst %.c, src/%.o, $(SOURCES))

ifeq ($(findstring ebox,$(MAKECMDGOALS)),ebox)
CFLAGS+=-march=pentium-mmx
endif

# targets 


all: $(LIBRARY).a $(LIBRARY).dll

ebox: $(LIBRARY).a $(LIBRARY).dll

$(LIBRARY).a: $(OBJECTS) Makefile
	ar cvrs $(LIBRARY).a $(OBJECTS)
	mv -f $(LIBRARY).a ../../lib


$(LIBRARY).dll: $(OBJECTS) Makefile
	$(LD) $(LDFLAGS) $(LIBPATH) -o $@ $(OBJECTS) $(LIBS)
	$(STRIP) $@
	sed -e "s/ @[^ ]*//" $(LIBRARY).orig.def > $(LIBRARY).def
	sed -f ../newlib/cmd1.sed $(LIBRARY).def > mem
	sed -f ../newlib/cmd2.sed mem >$(LIBRARY).inc
	mv -f $@ ../../bin
	mv -f $(LIBRARY).dll.a ../../lib   

%.o : %.c 
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

clean: 
	-rm -f */*.o 








 
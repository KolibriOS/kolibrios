
LIBRARY = cairo

CC = gcc

CFLAGS = -c -O2 -fomit-frame-pointer

LDIMPORT:= -nostdlib --out-implib libcairoimp.a --exclude-libs libamz.a

LDFLAGS:=  -shared -s -T ../newlib/dll.lds -Map map --image-base 0

DEFINES = -DHAVE_CONFIG_H -DCAIRO_NO_MUTEX -U_WIN32 -U_MSC_VER -U__WIN32__ 

INCLUDES = -I../newlib/include -I../pixman -I../zlib -I../libpng

LIBPATH:= -L../newlib -L../pixman -L../libpng

LIBS:=  -lamz -lgcc -lcimp -lpiximp -lpngimp

CAIRO_SOURCES =					\
	cairo-analysis-surface.c 		\
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
	cairo.c 				\
	cairo-cache.c 				\
	cairo-clip.c 				\
	cairo-color.c 				\
	cairo-composite-rectangles.c 		\
	cairo-debug.c				\
	cairo-device.c 				\
	cairo-fixed.c 				\
	cairo-font-face.c 			\
	cairo-font-face-twin.c 			\
	cairo-font-face-twin-data.c 		\
	cairo-font-options.c 			\
	cairo-freelist.c 			\
	cairo-freed-pool.c 			\
	cairo-gstate.c 				\
	cairo-hash.c 				\
	cairo-hull.c 				\
	cairo-image-info.c 			\
	cairo-image-surface.c 			\
	cairo-lzw.c 				\
	cairo-matrix.c 				\
	cairo-recording-surface.c 		\
	cairo-misc.c 				\
	cairo-mutex.c 				\
	cairo-observer.c 			\
	cairo-output-stream.c 			\
	cairo-paginated-surface.c 		\
	cairo-path-bounds.c 			\
	cairo-path.c 				\
	cairo-path-fill.c 			\
	cairo-path-fixed.c 			\
	cairo-path-in-fill.c 			\
	cairo-path-stroke.c 			\
	cairo-pattern.c				\
	cairo-pdf-operators.c			\
	cairo-pen.c 				\
	cairo-png.c				\
	cairo-polygon.c 			\
	cairo-rectangle.c			\
	cairo-rectangular-scan-converter.c	\
	cairo-region.c				\
	cairo-rtree.c				\
	cairo-scaled-font.c			\
	cairo-slope.c				\
	cairo-spans.c				\
	cairo-spline.c				\
	cairo-stroke-style.c			\
	cairo-surface.c				\
	cairo-surface-fallback.c		\
	cairo-surface-clipper.c			\
	cairo-surface-offset.c			\
	cairo-surface-snapshot.c		\
	cairo-surface-subsurface.c		\
	cairo-surface-wrapper.c			\
	cairo-svg-surface.c			\
	cairo-tor-scan-converter.c		\
	cairo-toy-font-face.c			\
	cairo-traps.c				\
	cairo-unicode.c				\
	cairo-user-font.c			\
	cairo-version.c				\
	cairo-wideint.c				\
	$(NULL)

 
CAIRO_FONT = 					\
	cairo-cff-subset.c 			\
	cairo-scaled-font-subsets.c 		\
	cairo-truetype-subset.c		 	\
	cairo-type1-fallback.c 			\
	cairo-type1-subset.c 			\
        cairo-type3-glyph-surface.c		\
	$(NULL)
 

SOURCES= $(CAIRO_SOURCES) $(CAIRO_FONT)

OBJECTS =  $(patsubst %.c, src/%.o, $(SOURCES))


# targets 


all:$(LIBRARY).a $(LIBRARY).dll


$(LIBRARY).a: $(OBJECTS) Makefile
	ar cvrs $(LIBRARY).a $(OBJECTS)


$(LIBRARY).dll: $(OBJECTS) Makefile
	ld $(LDFLAGS) $(LDIMPORT) $(LIBPATH) -o $@ $(OBJECTS) $(LIBS) 

%.o : %.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<









 
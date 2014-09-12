include ../config.mk

OBJS= clip.o vertex.o api.o list.o init.o matrix.o texture.o \
      misc.o clear.o light.o select.o get.o error.o \
      zbuffer.o zline.o zdither.o ztriangle.o \
      zmath.o image_util.o msghandling.o \
      arrays.o specbuf.o glu.o kosgl.o
	        

INCLUDES = -I$(TINYGL)/include -I$(MENUETDEV)/include
LIB = libTinyGL.a

all: $(LIB)

$(LIB): $(OBJS)
	rm -f $(LIB)
	ar rcs $(LIB) $(OBJS)
	copy /y $@ $(TINYGL)\lib
	del $(LIB)

clean:
	rm -f *~ *.o *.a $(TINYGL)/$(LIB)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $*.c

clip.o: zgl.h zfeatures.h
vertex.o: zgl.h zfeatures.h
light.o: zgl.h zfeatures.h
matrix.o: zgl.h zfeatures.h
list.o: zgl.h opinfo.h zfeatures.h
arrays.c: zgl.h zfeatures.h
specbuf.o: zgl.h zfeatures.h
glx.o: zgl.h zfeatures.h
nglx.o: zgl.h zfeatures.h
zline.o: zgl.h zfeatures.h zline.h
ztriangle.o: ztriangle.c ztriangle.h zgl.h zfeatures.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $*.c

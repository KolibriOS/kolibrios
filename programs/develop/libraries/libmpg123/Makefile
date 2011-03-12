
CC=gcc
CPP=gcc -E

CFLAGS= -c -combine -O2 -fomit-frame-pointer -ffast-math -mmmx -finline-functions 

LDIMPORT:= -nostdlib --out-implib libmpg123imp.a --exclude-libs libamz.a

LDFLAGS:=  -shared -s -T../newlib/dll.lds --image-base 0

INCLUDES= -I../newlib/include

LIBPATH:= -L../newlib

LIBS:=  -lamz -lgcc -lcimp
 

NAME:=	  libmpg123

DEFINES:= -DHAVE_CONFIG_H -DOPT_MMX -DOPT_MULTI -DREAL_IS_FLOAT -DNOXFERMEM


SRCS=				  		\
			libmpg123.c		\
			compat.c		\
			dct64.c			\
			dct64_i386.c		\
			equalizer.c		\
			optimize.c		\
			icy.c			\
			id3.c			\
			index.c			\
			layer1.c		\
			layer2.c		\
			layer3.c		\
			synth.c			\
			tabinit.c		\
			parse.c			\
			readers.c		\
			frame.c			\
			format.c

ASM=						\
			getcpuflags.S		\
			tabinit_mmx.S		\
			dct64_mmx.S		\
			synth_mmx.S

ASM_OBJS =  $(patsubst %.S, %.o, $(ASM))
SRCS_OBJ = $(patsubst %.c, %.o, $(SRCS))


all: $(NAME).dll

$(NAME).dll: $(SRCS_OBJ) $(ASM_OBJS) Makefile
	ld $(LDIMPORT) $(LIBPATH) $(LDFLAGS) -o $@ $(SRCS_OBJ) $(ASM_OBJS) $(LIBS) 


%.o : %.S $(ASM) Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

%.o: %.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

clean: 
	-rm -f *.o 

 
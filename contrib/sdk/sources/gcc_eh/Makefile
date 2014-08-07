
LIBRARY= libgcc_eh

CC = kos32-gcc
AR = kos32-ar
LD = kos32-ld
CPP= kos32-g++
STRIP = kos32-strip

CFLAGS = -U_Win32 -U_WIN32 -U__MINGW32__ -c -O2 -fno-ident -fomit-frame-pointer

ARFLAG = crs

INCLUDES= -I. -I../newlib/libc/include

LIBS:=  -ldll -lc.dll


DEFINES= -DIN_GCC -DUSE_EMUTLS=1


SOURCES = 	unwind-c.c			\
		unwind-dw2.c			\
		unwind-dw2-fde.c
 
OBJECTS =  $(patsubst %.cc, %.o, $(patsubst %.c, %.o, $(SOURCES)))


# targets 

all:$(LIBRARY).a

$(LIBRARY).a: $(OBJECTS) Makefile
	$(AR) cvrs $(LIBRARY).a $(OBJECTS)		
	mv -f $(LIBRARY).a ../../lib

%.o : %.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

%.o : %.cc Makefile
	$(CPP) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<


clean: 
	-rm -f *.o 





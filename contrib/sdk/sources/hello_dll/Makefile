LIBRARY= libhello

CC = kos32-gcc
AR = kos32-ar
LD = kos32-ld
STRIP = kos32-strip

INSTALLDIR ?= /home/autobuild/tools/win32/lib

CFLAGS = -UWIN32 -UWindows -U_WINDOWS -U_WIN32 -U__WIN32__ -c -O2 -fno-ident -fomit-frame-pointer

LDFLAGS=$(LDFLAGS_CMD)
LDFLAGS+= -shared -s -T dll.lds --entry _DllStartup --image-base=0
LDFLAGS+=  --out-implib $(LIBRARY).dll.a
ARFLAGS:= crs
INCLUDES= -I. -I../newlib/libc/include
LIBS:=  -ldll -lc.dll

SOURCES = libhello.c 
OBJECTS =  $(patsubst %.c, %.o, $(SOURCES))

all:$(LIBRARY).a $(LIBRARY).dll

$(LIBRARY).a: $(OBJECTS) Makefile
	$(AR) $(ARFLAGS) $(LIBRARY).a $(OBJECTS)
	mv -f $(LIBRARY).a $(INSTALLDIR)

$(LIBRARY).dll: libhello.def $(OBJECTS) Makefile
	$(LD) $(LDFLAGS) -o $@  libhello.def $(OBJECTS) $(LIBS)
	$(STRIP) $@
	#mv -f $(LIBRARY).dll.a $(INSTALLDIR)


%.o : %.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

clean: 
	-rm -f *.o 

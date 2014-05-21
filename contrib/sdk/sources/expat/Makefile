LIBRARY= libexpat

CC = kos32-gcc
AR = kos32-ar
LD = kos32-ld

CFLAGS = -U_Win32 -U_WIN32 -U__MINGW32__ -c -O2 -fomit-frame-pointer

ARFLAGS = crs

INCLUDES= -I. -I../newlib/libc/include

DEFS = -DHAVE_EXPAT_CONFIG_H

DEFINES= $(DEFS)

SRCS = 	lib/xmlparse.c		\
	lib/xmlrole.c		\
	lib/xmltok.c		\
	lib/xmltok_impl.c	\
	lib/xmltok_ns.c		\
	$(NULL) 

OBJS =  $(patsubst %.c, %.o, $(SRCS))

# targets 

all:$(LIBRARY).a 

$(LIBRARY).a: $(OBJS) Makefile
	$(AR) $(ARFLAGS) $(LIBRARY).a $(OBJS)
	mv -f $(LIBRARY).a ../../lib

%.o : %.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

clean: 
	-rm -f lib/*.o 





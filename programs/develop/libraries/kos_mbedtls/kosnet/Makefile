NEWLIB_INCLUDES=D:\KOSSDK\newlib\libc\include

CC = kos32-gcc
AR = kos32-ar

CFLAGS	?= -O2
WARNING_CFLAGS ?=  -Wall -W -Wdeclaration-after-statement
LDFLAGS ?=

LOCAL_CFLAGS = $(WARNING_CFLAGS) -I $(NEWLIB_INCLUDES) -I include -D_FILE_OFFSET_BITS=64
LOCAL_LDFLAGS =

AR_DASH ?= -
ARFLAGS = $(AR_DASH)src

OBJS= socket.o network.o dlfcn.o

.PHONY: all static clean

all: static

static: libkosnet.a

libkosnet.a: $(OBJS)
	echo "  AR    $@"
	$(AR) $(ARFLAGS) $@ $(OBJS)

.c.o:
	echo "  CC    $<"
	$(CC) $(LOCAL_CFLAGS) $(CFLAGS) -c $<

clean:
ifndef WINDOWS
	rm -f *.o libkosnet.a
else
	del /Q /F *.o libkosnet.a
endif

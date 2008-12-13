
CC = gcc
FASM = e:/fasm/fasm.exe
CFLAGS = -c -O2 -fomit-frame-pointer -fno-builtin-printf 
LDRHD  = -shared -T ld.x -s --file-alignment 32

INCLUDES = -I ../include

HFILES:=     ../include/types.h		\
             ../include/syscall.h	\
             ../include/link.h        	\
             pci.h        		\
             usb.h

SRC_DEP:=    pci.inc		\
	     detect.inc		\
	     hcd.inc		\
	     hid.inc

USB_SRC:=    usb.c     

USB_OBJ:=    usb.obj


USB =        usb.dll

all: $(USB)

$(USB): $(USB_OBJ) $(SRC_DEP) $(HFILES) Makefile
	wlink name usb.dll SYS nt_dll lib libdrv op offset=0 op nod op maxe=25 op el op STUB=stub.exe op START=_drvEntry @usb.lk1
	kpack.exe usb.dll usb.drv

usb.obj : usb.c $(SRC_DEP) $(HFILES) Makefile
	$(CC) $(INCLUDES) $(CFLAGS) -o usb.obj usb.c

%.obj : %.c $(HFILES)
	$(CC) $(CFLAGS) -o $@ $<

%.obj: %.asm
	as -o $@ $<




CC = gcc
CFLAGS = -c -O2 -fomit-frame-pointer -fno-builtin-printf 
LDFLAGS = -nostdlib -shared -s -Map usb.map --image-base 0\
	  --file-alignment 512 --section-alignment 4096

DEFINES	 = -D__KERNEL__ -DCONFIG_X86_32 

DRV_TOPDIR   = $(CURDIR)/../..

DRV_INCLUDES = $(DRV_TOPDIR)/include

INCLUDES = 	-I$(DRV_INCLUDES) \
		-I$(DRV_INCLUDES)/linux

LIBPATH = $(DRV_TOPDIR)/ddk



SRC_DEP:=    pci.inc		\
	     detect.inc		\
	     hcd.inc		\
	     hid.inc

USB_SRC:=    usb.c     

USB_OBJ:=    usb.obj

LIBS:=    -lddk -lcore 

USB =        usb.dll

all: $(USB)

$(USB): $(USB_OBJ) $(SRC_DEP) $(HFILES) Makefile
	ld $(LDFLAGS) -L$(LIBPATH) -T usb.lds -o $@ $(USB_OBJ) $(LIBS)  
	kpack.exe usb.dll usb.drv

usb.obj : usb.c $(SRC_DEP) $(HFILES) Makefile
	$(CC) $(DEFINES) $(INCLUDES) $(CFLAGS) -o usb.obj usb.c

%.obj : %.c $(HFILES)
	$(CC) $(CFLAGS) -o $@ $<

%.obj: %.asm
	as -o $@ $<



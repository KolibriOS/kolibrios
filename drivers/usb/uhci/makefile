
CC = gcc

DRV_DIR   = $(CURDIR)/../..

DRV_INCLUDES = $(DRV_DIR)/include

INCLUDES = -I$(DRV_INCLUDES) -I$(DRV_INCLUDES)/linux

DEFINES	 = -D__KERNEL__ -DCONFIG_X86_32

CFLAGS = -c -O2 -fomit-frame-pointer -fno-builtin-printf 

LDFLAGS = -nostdlib -shared -s -Map acpi.map --image-base 0\
	  --file-alignment 512 --section-alignment 4096

LIBPATH:= -L$(DRV_DIR)/ddk 

LIBS:=    -lgcc -lddk -lcore


NAME= usb

NAME_SRCS= 	usb.c		\
		buffer.c

SRC_DEP = 	hcd.inc		\
		detect.inc	\
		hid.inc		\
		pci.inc		\


all: $(NAME).dll

NAME_OBJS =  $(patsubst %.S, %.o, $(patsubst %.asm, %.o,\
            $(patsubst %.c, %.o, $(NAME_SRCS))))


$(NAME).dll: $(NAME_OBJS) usb.lds Makefile
	ld $(LIBPATH) $(LDFLAGS) -T usb.lds -o $@ $(NAME_OBJS) $(LIBS)
	kpack $(NAME).dll $(NAME).drv  

%.o: %.c $(HFILES) $(SRC_DEP) Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

%.o: %.S $(HFILES) Makefile
	as -o $@ $<





CC = gcc
FASM = e:/fasm/fasm.exe

CFLAGS = -c -O2 -fomit-frame-pointer -fno-builtin-printf 

DRV_DIR   = $(CURDIR)/..

DRV_INCLUDES = $(DRV_DIR)/include

ACPI_INCLUDES = ./acpica/include

INCLUDES = -I$(DRV_INCLUDES) -I$(ACPI_INCLUDES) -I$(DRV_DIR)/include/linux

DEFINES	 = -D__KERNEL__ -DCONFIG_X86_32

LDFLAGS = -nostdlib -shared -s -Map acpi.map --image-base 0\
	  --file-alignment 512 --section-alignment 4096

LIBPATH:= -L$(DRV_DIR)/ddk -L./acpica

LIBS:=    -lacpica -lgcc -lddk -lcore


NAME= acpi

NAME_SRCS= 	acpi.c		\
		scan.c		\
		pci_irq.c	\
		pci_root.c	\
		pci/access.c	\
		pci/pci.c	\
		pci/probe.c	\
		pci_bind.c


all: $(NAME).dll

NAME_OBJS =  $(patsubst %.S, %.o, $(patsubst %.asm, %.o,\
            $(patsubst %.c, %.o, $(NAME_SRCS))))


$(NAME).dll: $(NAME_OBJS) acpi.lds Makefile
	ld $(LIBPATH) $(LDFLAGS) -T acpi.lds -o $@ $(NAME_OBJS) $(LIBS)
	kpack $(NAME).dll  

%.o : %.c $(HFILES) Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

%.o : %.S $(HFILES) Makefile
	as -o $@ $<




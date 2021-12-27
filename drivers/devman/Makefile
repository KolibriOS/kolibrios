CC = kos32-gcc
LD = kos32-ld
KPACK = kpack

CFLAGS = -c -w -O2 -fomit-frame-pointer -fno-builtin-printf 

DRV_DIR = $(CURDIR)/..
INCLUDES = -I$(DRV_DIR)/devman/acpica/include -I$(DRV_DIR)/devman -I$(DRV_DIR)/include/linux -I$(DRV_DIR)/include -I$(DRV_DIR)/include/uapi

DEFINES = -D__KERNEL__ -DCONFIG_X86_32 -DCONFIG_X86_L1_CACHE_SHIFT=6 -DCONFIG_ARCH_HAS_CACHE_LINE_SIZE -DHAVE_ACPICA -DCONFIG_ACPI -DLINUX_MOD_DEVICETABLE_H -DCONFIG_PCI

LDFLAGS = -nostdlib -shared -s -Map acpi.map --image-base 0 --file-alignment 512 --section-alignment 4096

LIBPATH := -L$(DRV_DIR)/ddk -L../devman/acpica  -L../../contrib/sdk/lib
LIBS := -lacpica -lgcc -lddk -lcore

NAME = acpi.sys

NAME_SRCS = acpi.c \
            scan.c \
            pci_root.c \
            pci_bind.c \
            pci_irq.c \
            pci/probe.c \
            pci/pci.c \
            pci/access.c

all: $(NAME)

NAME_OBJS = $(patsubst %.S, %.o, $(patsubst %.asm, %.o, \
            $(patsubst %.c, %.o, $(NAME_SRCS))))


$(NAME): $(NAME_OBJS) ../devman/acpi.lds Makefile
	$(LD) $(LIBPATH) $(LDFLAGS) -T ../devman/acpi.lds -o $@ $(NAME_OBJS) $(LIBS)
	$(KPACK) $(NAME)

%.o : %.c $(HFILES) Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

clean:
	rm -rf *.o

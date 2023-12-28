CC = kos32-gcc
LD = kos32-ld
KPACK = kpack

DDK_TOPDIR      = ../../ddk
DRV_INCLUDES    = ../../include

INCLUDES=   -I$(DRV_INCLUDES)		\
            -I$(DRV_INCLUDES)/asm 	\
            -I$(DRV_INCLUDES)/uapi	\
            -I$(DRV_INCLUDES)/drm

NAME=k10temp

DEFINES = -DKOLIBRI -D__KERNEL__ -DCONFIG_X86_32 -DCONFIG_DMI -DCONFIG_TINY_RCU
DEFINES+= -DCONFIG_X86_L1_CACHE_SHIFT=6 -DCONFIG_ARCH_HAS_CACHE_LINE_SIZE
DEFINES+= -DCONFIG_PRINTK -DCONFIG_PCI -DCONFIG_PCI  -DCONFIG_AMD_NB -DKBUILD_MODNAME=\"k10temp\"

CFLAGS= -c -Os -march=i686 -fno-ident -msse2 -fomit-frame-pointer -fno-builtin-printf
CFLAGS+= -mno-stack-arg-probe -mpreferred-stack-boundary=2 -mincoming-stack-boundary=2 -mno-ms-bitfields 

LIBPATH = -L $(DDK_TOPDIR)
LIBPATH+= -L ../../../contrib/sdk/lib -L

LIBS:= -lddk -lcore -lgcc

LDFLAGS = -nostdlib -shared -s --major-os-version 0 --minor-os-version 7 \
          --major-subsystem-version 0 --minor-subsystem-version 5 --subsystem native \
          --image-base 0 --file-alignment 512 --section-alignment 4096

OBJS = k10temp.o ../pci.o ../amd_nb.o ../cpu_detect.o 

all: $(OBJS) $(NAME).sys 

$(NAME).sys: $(OBJS) 
	$(LD) $(LIBPATH) $(LDFLAGS) -T ../drv.lds $(OBJS) -o $@ $(NAME_OBJS) $(LIBS)
	$(KPACK) $(NAME).sys

%.o : %.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<  

clean:
	rm -f $(OBJS) $(NAME).sys

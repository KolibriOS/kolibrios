CC = kos32-gcc
LD = kos32-ld
KPACK = kpack


DDK_TOPDIR      = ../../ddk
DRV_INCLUDES    = ../../include

INCLUDES =	-I$(DRV_INCLUDES)	\
			-I$(DRV_INCLUDES)/asm 	\
			-I$(DRV_INCLUDES)/uapi	\
			-I$(DRV_INCLUDES)/drm
NAME=coretemp

DEFINES = -DKOLIBRI -D__KERNEL__ -DCONFIG_X86_32 -DCONFIG_DMI -DCONFIG_TINY_RCU
DEFINES+= -DCONFIG_X86_L1_CACHE_SHIFT=6 -DCONFIG_ARCH_HAS_CACHE_LINE_SIZE
DEFINES+= -DCONFIG_PRINTK

CFLAGS= -c -O2 $(DEFINES) $(INCLUDES) -march=i686 -fno-ident -msse2 -fomit-frame-pointer -fno-builtin-printf
CFLAGS+= -mno-stack-arg-probe -mpreferred-stack-boundary=2 -mincoming-stack-boundary=2 -mno-ms-bitfields

LIBPATH = -L $(DDK_TOPDIR)
LIBPATH+= -L ../../../contrib/sdk/lib

LIBS:=    -lddk -lcore -lgcc

PE_FLAGS = --major-os-version 0 --minor-os-version 7 --major-subsystem-version 0 \
          --minor-subsystem-version 5 --subsystem native

LDFLAGS = -nostdlib -shared -s $(PE_FLAGS) --image-base 0\
	  --file-alignment 512 --section-alignment 4096

all: $(NAME).dll

$(NAME).dll: 
	$(CC) $(CFLAGS) coretemp.c
	$(LD) $(LIBPATH) $(LDFLAGS) -T ../drv.lds coretemp.o -o $@ $(NAME_OBJS) $(LIBS)
	$(KPACK) $(NAME).dll

clean:
	rm -rf *.o *.dll


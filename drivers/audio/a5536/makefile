
CC = gcc
FASM = e:/fasm/fasm.exe
CFLAGS = -c -O2 -fomit-frame-pointer -fno-builtin-printf 
LDFLAGS = -nostdlib -shared -s -Map geode.map --image-base 0\
	  --file-alignment 512 --section-alignment 4096

DEFINES	 = -D__KERNEL__ -DCONFIG_X86_32 

DRV_TOPDIR   = $(CURDIR)/../..

DRV_INCLUDES = $(DRV_TOPDIR)/include

INCLUDES = 	-I$(DRV_INCLUDES) \
		-I$(DRV_INCLUDES)/linux

LIBPATH = $(DRV_TOPDIR)/ddk

LIBS:=    -lddk -lcore 

HFILES:=     geode.h			

SRC_DEP:=    
GEODE_SRC:=    amd_geode.h     

NAME:=   geode
GEODE:=  geode.dll

all: $(GEODE)

$(GEODE): geode.obj $(SRC_DEP) $(HFILES) Makefile
	ld $(LDFLAGS) -L$(LIBPATH) -T driver.lds -o $@ geode.obj $(LIBS)  
	kpack.exe geode.dll geode.drv

geode.obj : geode.c $(SRC_DEP) $(HFILES) Makefile
	$(CC) $(DEFINES) $(INCLUDES) $(CFLAGS) -o geode.obj geode.c




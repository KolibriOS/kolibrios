
CC = gcc
FASM = e:/fasm/fasm.exe
CFLAGS = -c -O2 -fomit-frame-pointer -fno-builtin-printf 
LDRHD  = -shared -T ld.x -s --file-alignment 32

INCLUDES = -I ../include

HFILES:=     ../include/types.h		\
             ../include/syscall.h	\
             geode.h			\
             pci.h

SRC_DEP:=    
GEODE_SRC:=    amd_geode.h     

NAME:=   geode
GEODE:=  geode.dll

all: $(GEODE)

$(GEODE): geode.obj $(SRC_DEP) $(HFILES) Makefile
	wlink name $(GEODE) SYS nt_dll lib libdrv op offset=0 op nod op maxe=25 op el op STUB=stub.exe op START=_drvEntry @$(NAME).lk
	kpack.exe geode.dll geode.drv

geode.obj : geode.c $(SRC_DEP) $(HFILES) Makefile
	$(CC) $(INCLUDES) $(CFLAGS) -o geode.obj geode.c




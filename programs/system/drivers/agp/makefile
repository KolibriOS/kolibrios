
CC = gcc
FASM = e:/fasm/fasm.exe
CFLAGS = -c -O2 -fomit-frame-pointer -fno-builtin-printf 
LDRHD  = -shared -T ld.x -s --file-alignment 32

HFILES:=     syscall.h    	\
             pci.h        	\

SRC_DEP:=    detect.inc

AGP_SRC:=    agp.c     


AGP =  agp.dll

all: $(AGP)

$(AGP): agp.obj $(SRC_DEP) $(HFILES) Makefile
	wlink name agp.dll SYS nt_dll lib libdrv op offset=0 op nod op maxe=25 op el op STUB=stub.exe op START=_drvEntry @agp.lk
	kpack.exe agp.dll agp.drv

agp.obj : agp.c $(SRC_DEP) $(HFILES) Makefile
	$(CC) $(CFLAGS) -o agp.obj agp.c




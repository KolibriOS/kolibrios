
CC = gcc
FASM = e:/fasm/fasm.exe
CFLAGS = -c -O2 -fomit-frame-pointer -fno-builtin-printf 
LDRHD  = -shared -T ld.x -s --file-alignment 32

HFILES:=     ati2d.h      	\
             syscall.h    	\
             pci.h        	\
             accel_2d.h   	\
             r5xx_regs.h 	\
             microcode.h

SRC_DEP:=    init.c    		\
             pci.c              \
             ati_mem.c       	\
             r500.inc           \
             pixmap.inc		\
             accel_2d.inc  	\
             accel_3d.inc

ATI_SRC:=    ati2d.c     

ATI_OBJ:=    ati2d.obj


ATI_OBJ  =  $(patsubst %.s, %.obj, $(patsubst %.asm, %.obj,\
            $(patsubst %.c, %.obj, $(ATI_SRC))))


ATI2D =  ati2d.dll

all: $(ATI2D)

$(ATI2D): $(ATI_OBJ) $(SRC_DEP) $(HFILES) Makefile
	wlink name ati2d.dll SYS nt_dll lib libdrv op offset=0 op nod op maxe=25 op el op STUB=stub.exe op START=_drvEntry @rhd.lk1
	kpack.exe ati2d.dll ati2d.drv

ati2d.obj : ati2d.c $(SRC_DEP) $(HFILES)
	$(CC) $(CFLAGS) -o ati2d.obj ati2d.c

curhelp.obj : curhelp.asm
	$(FASM) curhelp.asm
 
%.obj : %.c $(HFILES)
	$(CC) $(CFLAGS) -o $@ $<

%.obj: %.asm
	as -o $@ $<



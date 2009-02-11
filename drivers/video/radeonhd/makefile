
CC = gcc
FASM = e:/fasm/fasm.exe
CFLAGS = -c -Os -fomit-frame-pointer -fno-builtin-printf 
LDCORE = -shared -s --file-alignment 32 --output-def core.def --out-implib core.lib
LDRHD  = -shared -T ld.x -s --file-alignment 32

CORE_SRC:= core.asm

HFILES:=	common.h		\
		rhd.h			\
        	rhd_card.h		\
	        rhd_connector.h		\
        	rhd_output.h		\
           	rhd_mc.h		\
		rhd_hdmi.h		\
           	pci.h

RHD_SRC:=	rhd.c			\
		rhd_id.c		\
           	rhd_mem.c		\
           	rhd_vga.c		\
           	rhd_mc.c		\
           	rhd_crtc.c		\
           	rhd_dac.c		\
           	rhd_pll.c		\
           	rhd_lut.c		\
           	rhd_atombios.c   	\
		rhd_atomout.c		\
		rhd_biosscratch.c	\
		rhd_atomcrtc.c		\
		rhd_atompll.c		\
           	rhd_i2c.c		\
         	rhd_edid.c       	\
	        rhd_connector.c  	\
	   	rhd_ddia.c       	\
           	rhd_dig.c        	\
           	rhd_monitor.c    	\
           	rhd_modes.c      	\
           	rhd_output.c     	\
           	rhd_lvtma.c      	\
           	rhd_tmds.c       	\
		rhd_hdmi.c		\
           	xf86i2c.c        	\
           	string.c         	\
           	malloc.c         	\
           	vsprintf.c       	\
           	memset.asm       	\
           	s_ceilf.asm      	\
           	dbg.c            	\
           	pci.c			

DBG_DEFINES = -DDBG_CALL

ATOM_BIOS_PARSER_SRCS = rhd_atomwrapper.c         \
                        AtomBios/CD_Operations.c      \
                        AtomBios/Decoder.c            \
                        AtomBios/hwserv_drv.c

ATOM_BIOS_PARSER_OBJS = rhd_atomwrapper.obj       \
                        AtomBios/CD_Operations.obj    \
                        AtomBios/Decoder.obj          \
                        AtomBios/hwserv_drv.obj 

ATOM_BIOS_INCLUDES = -I ./AtomBios/includes
ATOM_BIOS_DEFINES = -DATOM_BIOS=1
ATOM_BIOS_PARSER_INCLUDES = -I./AtomBios
ATOM_BIOS_PARSER_DEFINES = -DDRIVER_PARSER \
                           -DDISABLE_EASF  \
                           -DENABLE_ALL_SERVICE_FUNCTIONS \
                           -DATOM_BIOS_PARSER=1

DEFINES  = $(INCLUDES) $(ATOM_BIOS_INCLUDES) \
           $(ATOM_BIOS_PARSER_INCLUDES) $(ATOM_BIOS_DEFINES)\
           $(ATOM_BIOS_PARSER_DEFINES) $(DBG_DEFINES)

CORE_OBJS = $(patsubst %.s, %.obj, $(patsubst %.asm, %.obj,\
            $(patsubst %.c, %.obj, $(CORE_SRC))))

RHD_OBJS =  $(patsubst %.s, %.obj, $(patsubst %.asm, %.obj,\
            $(patsubst %.c, %.obj, $(RHD_SRC))))


RHD =  rhd.exe
CORE = core.dll

all: $(CORE) $(RHD)


$(RHD): $(RHD_OBJS) $(ATOM_BIOS_PARSER_OBJS) Makefile
	wmake -f rhd.mk

$(CORE): $(CORE_OBJS) Makefile
	ld $(LDCORE) -o $@ $(CORE_OBJS)

rhd_crtc.obj : rhd_crtc.c
	$(CC) $(CFLAGS) $(DEFINES) -o $@ -c $<

malloc.obj : malloc.c
	$(CC) $(CFLAGS) $(DEFINES) -o $@ -c $<

rhd_monitor.obj : rhd_monitor.c
	$(CC) $(CFLAGS) $(DEFINES) -o $@ -c $<

xf86i2c.obj : xf86i2c.c
	$(CC) $(CFLAGS) $(DEFINES) -o $@ -c $<

string.obj : string.c
	$(CC) $(CFLAGS) $(DEFINES) -o $@ -c $<

%.obj : %.c $(HFILES)
	$(CC) $(CFLAGS) -masm=intel $(DEFINES) -o $@ -c $<

%.obj: %.asm
	as -o $@ $<


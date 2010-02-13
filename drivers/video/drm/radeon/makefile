
CC = gcc
FASM = e:/fasm/fasm.exe

DEFINES	 = -D__KERNEL__ -DCONFIG_X86_32 

DRV_TOPDIR   = $(CURDIR)/../../..
DRM_TOPDIR   = $(CURDIR)/..

DRV_INCLUDES = $(DRV_TOPDIR)/include

INCLUDES = 	-I$(DRV_INCLUDES) -I$(DRV_INCLUDES)/drm	\
		-I$(DRV_INCLUDES)/linux

CFLAGS   = -c -O2 $(INCLUDES) $(DEFINES) -march=i686 -fomit-frame-pointer -fno-builtin-printf

LIBPATH:= $(DRV_TOPDIR)/ddk

LIBS:=    -ldrv -lcore 

LDFLAGS = -nostdlib -shared -s -Map atikms.map --image-base 0\
	  --file-alignment 512 --section-alignment 4096


NAME:=	  atikms

HFILES:=     		$(DRV_INCLUDES)/linux/types.h		\
			$(DRV_INCLUDES)/linux/list.h		\
			$(DRV_INCLUDES)/linux/pci.h		\
			$(DRV_INCLUDES)/drm/drm.h		\
			$(DRV_INCLUDES)/drm/drmP.h		\
			$(DRV_INCLUDES)/drm/drm_edid.h		\
			$(DRV_INCLUDES)/drm/drm_crtc.h		\
			$(DRV_INCLUDES)/drm/drm_mode.h		\
			$(DRV_INCLUDES)/drm/drm_mm.h		\
			atom.h					\
			radeon.h				\
			radeon_asic.h

NAME_SRC=							\
			pci.c					\
			$(DRM_TOPDIR)/drm_mm.c			\
			$(DRM_TOPDIR)/drm_edid.c		\
			$(DRM_TOPDIR)/drm_modes.c		\
			$(DRM_TOPDIR)/drm_crtc.c		\
			$(DRM_TOPDIR)/drm_crtc_helper.c		\
			$(DRM_TOPDIR)/drm_fb_helper.c		\
			$(DRM_TOPDIR)/drm_dp_i2c_helper.c	\
			$(DRM_TOPDIR)/i2c/i2c-core.c		\
			$(DRM_TOPDIR)/i2c/i2c-algo-bit.c	\
			radeon_device.c				\
			radeon_clocks.c				\
			radeon_i2c.c				\
			atom.c					\
			radeon_atombios.c			\
			radeon_agp.c				\
			atombios_crtc.c				\
			atombios_dp.c				\
			radeon_encoders.c			\
			radeon_connectors.c			\
			radeon_bios.c				\
			radeon_combios.c			\
			radeon_legacy_crtc.c			\
			radeon_legacy_encoders.c		\
			radeon_legacy_tv.c			\
			radeon_display.c			\
			radeon_gart.c				\
			radeon_ring.c				\
			radeon_object_kos.c			\
			radeon_gem.c				\
			r100.c					\
			r200.c					\
			r300.c					\
			r420.c					\
			rv515.c					\
			r520.c					\
			r600.c					\
			r600_audio.c				\
			r600_hdmi.c				\
			rs400.c					\
			rs600.c					\
			rs690.c					\
			rv770.c					\
			radeon_fb.c				\
			rdisplay.c				\
			rdisplay_kms.c				\
			radeon_pm.c				\
			cmdline.c				\
			cursor.S				\
			fwblob.asm

FW_SRCS=							\
			firmware/R100_cp.bin.ihex		\
			firmware/R200_cp.bin.ihex		\
			firmware/R300_cp.bin.ihex		\
			firmware/R420_cp.bin.ihex		\
			firmware/RS690_cp.bin.ihex		\
			firmware/RS600_cp.bin.ihex		\
			firmware/R520_cp.bin.ihex		\


SRC_DEP:=    


NAME_OBJS =  $(patsubst %.S, %.o, $(patsubst %.asm, %.o,\
            $(patsubst %.c, %.o, $(NAME_SRC))))


FW_BINS = $(patsubst %.bin.ihex, %.bin, $(FW_SRCS))



all: $(NAME).dll

$(NAME).dll: $(NAME_OBJS) $(SRC_DEP) $(HFILES) atikms.lds Makefile
	ld -L$(LIBPATH) $(LDFLAGS) -T atikms.lds -o $@ $(NAME_OBJS) $(LIBS)  


%.o : %.c $(HFILES) Makefile
	$(CC) $(CFLAGS) $(DEFINES) -o $@ $<

%.o : %.S $(HFILES) Makefile
	as -o $@ $<

fwblob.o:  fwblob.asm $(FW_BINS) Makefile
	$(FASM) $< $@


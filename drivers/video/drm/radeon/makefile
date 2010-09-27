

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

LIBS:=    -lddk -lcore 

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
			evergreen.c				\
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

FW_BINS=							\
			firmware/R100_cp.bin			\
			firmware/R200_cp.bin			\
			firmware/R300_cp.bin			\
			firmware/R420_cp.bin			\
			firmware/R520_cp.bin			\
								\
			firmware/RS690_cp.bin			\
			firmware/RS600_cp.bin			\
			firmware/RS780_me.bin			\
			firmware/RS780_pfp.bin			\
								\
			firmware/RV610_me.bin			\
			firmware/RV620_me.bin			\
			firmware/RV630_me.bin			\
			firmware/RV635_me.bin			\
			firmware/RV670_me.bin			\
			firmware/RV710_me.bin			\
			firmware/RV730_me.bin			\
			firmware/RV770_me.bin			\
								\
			firmware/RV610_pfp.bin			\
			firmware/RV620_pfp.bin			\
			firmware/RV630_pfp.bin			\
			firmware/RV635_pfp.bin			\
			firmware/RV670_pfp.bin			\
			firmware/RV710_pfp.bin			\
			firmware/RV730_pfp.bin			\
			firmware/RV770_pfp.bin			\
								\
			firmware/R600_rlc.bin			\
			firmware/R700_rlc.bin


SRC_DEP:=    


NAME_OBJS =  $(patsubst %.S, %.o, $(patsubst %.asm, %.o,\
            $(patsubst %.c, %.o, $(NAME_SRC))))



all: $(NAME).dll

$(NAME).dll: $(NAME_OBJS) $(FW_BINS) $(SRC_DEP) $(HFILES) atikms.lds Makefile
	ld -L$(LIBPATH) $(LDFLAGS) -T atikms.lds -o $@ $(NAME_OBJS) $(LIBS)  


%.o : %.c $(HFILES) Makefile
	$(CC) $(CFLAGS) $(DEFINES) -o $@ $<

%.o : %.S $(HFILES) Makefile
	as -o $@ $<

fwblob.o:  fwblob.asm $(FW_BINS) Makefile
	$(FASM) $< $@


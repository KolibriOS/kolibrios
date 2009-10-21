
CC = gcc
FASM = e:/fasm/fasm.exe
CFLAGS = -c -O2 -fomit-frame-pointer -fno-builtin-printf 
LDFLAGS = -nostdlib -shared -s -Map atikms.map --image-base 0 --file-alignment 512 --section-alignment 4096

DRM_TOPDIR   = $(CURDIR)/..
DRM_INCLUDES = $(DRM_TOPDIR)/includes

LIBPATH:= .

LIBS:=    -ldrv -lcore 

NAME:=	  atikms

INCLUDES = -I$(DRM_INCLUDES) -I$(DRM_INCLUDES)/linux -I$(DRM_INCLUDES)/drm

HFILES:=     		$(DRM_INCLUDES)/linux/types.h		\
			$(DRM_INCLUDES)/linux/list.h		\
			$(DRM_INCLUDES)/pci.h		\
			$(DRM_INCLUDES)/drm/drm.h			\
			$(DRM_INCLUDES)/drm/drmP.h		\
			$(DRM_INCLUDES)/drm/drm_edid.h		\
			$(DRM_INCLUDES)/drm/drm_crtc.h		\
			$(DRM_INCLUDES)/drm/drm_mode.h		\
			$(DRM_INCLUDES)/drm/drm_mm.h		\
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
			$(DRM_TOPDIR)/i2c/i2c-core.c		\
			$(DRM_TOPDIR)/i2c/i2c-algo-bit.c	\
			$(DRM_TOPDIR)/idr.c			\
			radeon_gem.c				\
			radeon_device.c				\
			radeon_clocks.c				\
			radeon_i2c.c				\
			atom.c					\
			radeon_atombios.c			\
			radeon_agp.c				\
			atombios_crtc.c				\
			radeon_encoders.c			\
			radeon_connectors.c			\
			radeon_bios.c				\
			radeon_combios.c			\
			radeon_legacy_crtc.c			\
			radeon_legacy_encoders.c		\
			radeon_legacy_tv.c			\
			radeon_display.c			\
			radeon_object.c				\
			radeon_gart.c				\
			radeon_ring.c				\
			r100.c					\
			r200.c					\
			r300.c					\
			r420.c					\
			rv515.c					\
			r520.c					\
			rs400.c					\
			rs600.c					\
			rs690.c					\
			radeon_fb.c				\
			rdisplay.c


SRC_DEP:=    


NAME_OBJS =  $(patsubst %.s, %.o, $(patsubst %.asm, %.o,\
            $(patsubst %.c, %.o, $(NAME_SRC))))



all: $(NAME).dll

$(NAME).dll: $(NAME_OBJS) $(SRC_DEP) $(HFILES) atikms.lds Makefile
	ld -L$(LIBPATH) $(LDFLAGS) -T atikms.lds -o $@ $(NAME_OBJS) vsprintf.obj icompute.obj $(LIBS)  


%.o : %.c $(HFILES) Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ -c $<

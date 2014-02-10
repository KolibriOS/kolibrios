
CC = gcc
LD = ld
AS = as
FASM = fasm.exe

DEFINES	 = -D__KERNEL__ -DCONFIG_X86_32 

DRV_TOPDIR   = $(CURDIR)/../../..
DRV_INCLUDES = $(DRV_TOPDIR)/include
DRM_TOPDIR   = $(CURDIR)/..

INCLUDES = 	-I$(DRV_INCLUDES) -I$(DRV_INCLUDES)/drm	\
		-I$(DRV_INCLUDES)/linux -I$(DRV_INCLUDES)/linux/uapi

CFLAGS   = -c -O2 $(INCLUDES) -march=i686 -fomit-frame-pointer -fno-builtin-printf
CFLAGS+= -mno-ms-bitfields

LIBPATH:= $(DRV_TOPDIR)/ddk

LIBS:=    -lddk -lcore -lgcc

LDFLAGS = -nostdlib -shared -s --image-base 0\
	  --file-alignment 512 --section-alignment 4096


NAME:=	  vmwgfx

HFILES:=     		$(DRV_INCLUDES)/linux/types.h		\
			$(DRV_INCLUDES)/linux/list.h		\
			$(DRV_INCLUDES)/linux/pci.h		\
			$(DRV_INCLUDES)/drm/drm.h		\
			$(DRV_INCLUDES)/drm/drmP.h		\
			$(DRV_INCLUDES)/drm/drm_edid.h		\
			$(DRV_INCLUDES)/drm/drm_crtc.h		\
			$(DRV_INCLUDES)/drm/drm_mode.h		\
			$(DRV_INCLUDES)/drm/drm_mm.h

NAME_SRC=							\
			main.c					\
			pci.c					\
			vmwgfx_buffer.c				\
			vmwgfx_context.c			\
			vmwgfx_dmabuf.c				\
			vmwgfx_drv.c				\
			vmwgfx_execbuf.c			\
			vmwgfx_fence.c				\
			vmwgfx_fifo.c				\
			vmwgfx_gmr.c				\
			vmwgfx_gmrid_manager.c			\
			vmwgfx_irq.c				\
			vmwgfx_kms.c				\
			vmwgfx_marker.c				\
			vmwgfx_mob.c				\
			vmwgfx_resource.c			\
			vmwgfx_scrn.c				\
			vmwgfx_shader.c				\
			vmwgfx_surface.c			\
			vmwgfx_ttm_glue.c			\
			../hdmi.c				\
			../i2c/i2c-core.c			\
			../ttm/ttm_bo.c				\
			../ttm/ttm_bo_manager.c			\
			../ttm/ttm_execbuf_util.c		\
			../ttm/ttm_lock.c			\
			../ttm/ttm_memory.c			\
			../ttm/ttm_object.c			\
			../ttm/ttm_page_alloc.c			\
			../ttm/ttm_tt.c				\
			../drm_crtc.c				\
			../drm_crtc_helper.c			\
			../drm_drv.c				\
			../drm_edid.c				\
			../drm_gem.c				\
			../drm_global.c				\
			../drm_hashtab.c			\
			../drm_irq.c				\
			../drm_mm.c				\
			../drm_modes.c				\
			../drm_pci.c				\
			../drm_stub.c				\
			../drm_vma_manager.c




NAME_OBJS =  $(patsubst %.S, %.o, $(patsubst %.asm, %.o,\
            $(patsubst %.c, %.o, $(NAME_SRC))))



all: $(NAME).dll

$(NAME).dll: $(NAME_OBJS) $(LIBPATH)/libcore.a $(LIBPATH)/libddk.a vmw.lds Makefile
	$(LD) -L$(LIBPATH) $(LDFLAGS) -T vmw.lds -o $@ $(NAME_OBJS) $(LIBS)
	kpack $@  


%.o : %.c $(HFILES) Makefile
	$(CC) $(CFLAGS) $(DEFINES) -o $@ $<

%.o : %.S $(HFILES) Makefile
	$(AS) -o $@ $<

clean: 
	-rm -f */*.o


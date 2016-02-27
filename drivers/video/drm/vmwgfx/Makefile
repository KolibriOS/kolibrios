CC = kos32-gcc
FASM = fasm.exe

DEFINES	= -DDRM_DEBUG_CODE=0 -D__KERNEL__ -DCONFIG_X86_32 -DCONFIG_PCI -DCONFIG_X86_CMPXCHG64
DEFINES += -DCONFIG_TINY_RCU -DCONFIG_X86_L1_CACHE_SHIFT=6
DEFINES+= -DKBUILD_MODNAME=\"vmwgfx.dll\"

DDK_TOPDIR      = /d/kos/kolibri/drivers/ddk
DRV_INCLUDES    = /d/kos/kolibri/drivers/include
DRM_TOPDIR   = $(CURDIR)/..

INCLUDES =	-I$(DRV_INCLUDES)	\
		-I$(DRV_INCLUDES)/asm 	\
		-I$(DRV_INCLUDES)/uapi	\
		-I$(DRV_INCLUDES)/drm -I./ -I$(DRV_INCLUDES)

CFLAGS= -c -O2 $(INCLUDES) $(DEFINES) -march=i686 -fno-ident -msse2 -fomit-frame-pointer -fno-builtin-printf
CFLAGS+= -mno-stack-arg-probe -mno-ms-bitfields

LIBPATH:= $(DDK_TOPDIR)

LIBS:=    -lddk -lcore -lgcc

PE_FLAGS = --major-os-version 0 --minor-os-version 7 --major-subsystem-version 0 \
          --minor-subsystem-version 5 --subsystem native

LDFLAGS = -nostdlib -shared -s $(PE_FLAGS) --image-base 0\
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
			vmwgfx_binding.c			\
			vmwgfx_buffer.c				\
			vmwgfx_cmdbuf.c				\
			vmwgfx_cmdbuf_res.c			\
			vmwgfx_context.c			\
			vmwgfx_cotable.c			\
			vmwgfx_dmabuf.c				\
			vmwgfx_drv.c				\
			vmwgfx_execbuf.c			\
			vmwgfx_fb.c				\
			vmwgfx_fence.c				\
			vmwgfx_fifo.c				\
			vmwgfx_gmr.c				\
			vmwgfx_gmrid_manager.c			\
			vmwgfx_irq.c				\
			vmwgfx_kms.c				\
			vmwgfx_ldu.c				\
			vmwgfx_marker.c				\
			vmwgfx_mob.c				\
			vmwgfx_overlay.c			\
			vmwgfx_prime.c				\
			vmwgfx_resource.c			\
			vmwgfx_scrn.c				\
			vmwgfx_shader.c				\
			vmwgfx_so.c				\
			vmwgfx_stdu.c				\
			vmwgfx_surface.c			\
			vmwgfx_ttm_glue.c			\
			main.c					\
			pci.c					\
			../hdmi.c				\
			../i2c/i2c-core.c			\
			../ttm/ttm_bo.c				\
			../ttm/ttm_bo_manager.c			\
			../ttm/ttm_bo_util.c			\
			../ttm/ttm_execbuf_util.c		\
			../ttm/ttm_lock.c			\
			../ttm/ttm_memory.c			\
			../ttm/ttm_object.c			\
			../ttm/ttm_page_alloc.c			\
			../ttm/ttm_tt.c				\
			../drm_atomic.c				\
			../drm_atomic_helper.c			\
			../drm_bridge.c				\
			../drm_cache.c				\
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
			../drm_modeset_lock.c			\
			../drm_pci.c				\
			../drm_plane_helper.c			\
			../drm_rect.c				\
			../drm_stub.c				\
			../drm_vma_manager.c




NAME_OBJS =  $(patsubst %.S, %.o, $(patsubst %.asm, %.o,\
            $(patsubst %.c, %.o, $(NAME_SRC))))



all: $(NAME).dll

$(NAME).dll: $(NAME_OBJS) vmw.lds Makefile
	$(LD) -L$(LIBPATH) $(LDFLAGS) -T vmw.lds -o $@ $(NAME_OBJS) $(LIBS)


%.o : %.c $(HFILES) Makefile
	$(CC) $(CFLAGS) $(DEFINES) -o $@ $<

%.o : %.S $(HFILES) Makefile
	$(AS) -o $@ $<

clean: 
	-rm -f */*.o


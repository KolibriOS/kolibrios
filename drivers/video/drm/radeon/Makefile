CC = kos32-gcc
LD = ld
AS = as
FASM = fasm.exe

DEFINES	 = -DDRM_DEBUG_CODE=0 -D__KERNEL__ -DCONFIG_X86 -DCONFIG_X86_32 -DCONFIG_PCI
DEFINES += -DCONFIG_X86_CMPXCHG64 -DCONFIG_TINY_RCU -DCONFIG_X86_L1_CACHE_SHIFT=6
DEFINES += -DCONFIG_DRM_FBDEV_EMULATION -DCONFIG_DMI
DEFINES += -DKBUILD_MODNAME=\"ati.dll\"

DDK_TOPDIR   = $(CURDIR)/../../..
DDK_INCLUDES = $(DRV_TOPDIR)/include
DRM_TOPDIR   = $(CURDIR)/..

INCLUDES =	-I$(DDK_INCLUDES)	\
		-I$(DDK_INCLUDES)/asm 	\
		-I$(DDK_INCLUDES)/uapi	\
		-I$(DDK_INCLUDES)/drm -I./

CFLAGS= -c -O2 $(INCLUDES) $(DEFINES) -march=i686 -fno-ident -fomit-frame-pointer -fno-builtin-printf
CFLAGS+= -mno-stack-arg-probe -mpreferred-stack-boundary=2 -mincoming-stack-boundary=2 -mno-ms-bitfields

LIBPATH:= $(DDK_TOPDIR)

LIBS:=    -lddk -lcore -lgcc

PE_FLAGS = --major-os-version 0 --minor-os-version 7 --major-subsystem-version 0 \
          --minor-subsystem-version 5 --subsystem native

LDFLAGS = -nostdlib -shared -s $(PE_FLAGS) --image-base 0\
	  --file-alignment 512 --section-alignment 4096


NAME:=	  atikms

NAME_SRC=							\
			main.c					\
			pci.c					\
			../ttm/ttm_bo.c				\
			../ttm/ttm_bo_manager.c			\
			../ttm/ttm_bo_util.c			\
			../ttm/ttm_execbuf_util.c		\
			../ttm/ttm_memory.c			\
			../ttm/ttm_page_alloc.c			\
			../ttm/ttm_tt.c				\
			$(DRM_TOPDIR)/drm_atomic.c		\
			$(DRM_TOPDIR)/drm_atomic_helper.c	\
			$(DRM_TOPDIR)/drm_bridge.c		\
			$(DRM_TOPDIR)/drm_cache.c		\
			$(DRM_TOPDIR)/drm_crtc.c		\
			$(DRM_TOPDIR)/drm_crtc_helper.c		\
			$(DRM_TOPDIR)/drm_dp_helper.c		\
			$(DRM_TOPDIR)/drm_dp_mst_topology.c	\
			$(DRM_TOPDIR)/drm_drv.c			\
			$(DRM_TOPDIR)/drm_edid.c		\
			$(DRM_TOPDIR)/drm_fb_helper.c		\
			$(DRM_TOPDIR)/drm_gem.c			\
			$(DRM_TOPDIR)/drm_global.c		\
			$(DRM_TOPDIR)/drm_irq.c			\
			$(DRM_TOPDIR)/drm_mm.c			\
			$(DRM_TOPDIR)/drm_modes.c		\
			$(DRM_TOPDIR)/drm_modeset_lock.c	\
			$(DRM_TOPDIR)/drm_pci.c			\
			$(DRM_TOPDIR)/drm_plane_helper.c	\
			$(DRM_TOPDIR)/drm_probe_helper.c	\
			$(DRM_TOPDIR)/drm_rect.c		\
			$(DRM_TOPDIR)/drm_stub.c		\
			$(DRM_TOPDIR)/drm_vma_manager.c		\
			$(DRM_TOPDIR)/i2c/i2c-core.c		\
			$(DRM_TOPDIR)/i2c/i2c-algo-bit.c	\
			hmm.c					\
			radeon_device.c				\
			atom.c					\
			atombios_crtc.c				\
			atombios_dp.c				\
			atombios_encoders.c			\
			atombios_i2c.c				\
			btc_dpm.c				\
			cayman_blit_shaders.c			\
			ci_dpm.c				\
			ci_smc.c				\
			cik.c					\
			cik_blit_shaders.c			\
			cik_sdma.c				\
			cypress_dpm.c				\
			dce3_1_afmt.c				\
			dce6_afmt.c				\
			evergreen.c				\
			evergreen_blit_shaders.c		\
			evergreen_cs.c				\
			evergreen_dma.c				\
			evergreen_hdmi.c			\
			kv_dpm.c				\
			kv_smc.c				\
			ni.c					\
			ni_dma.c				\
			ni_dpm.c				\
			radeon_agp.c				\
			radeon_asic.c				\
			radeon_atombios.c			\
			radeon_audio.c				\
			radeon_benchmark.c			\
			radeon_bios.c				\
			radeon_combios.c			\
			radeon_connectors.c			\
			radeon_cs.c				\
			radeon_clocks.c				\
			radeon_display.c			\
			radeon_dp_auxch.c			\
			radeon_dp_mst.c				\
			radeon_encoders.c			\
			radeon_fence.c				\
			radeon_fb.c				\
			radeon_gart.c				\
			radeon_gem.c				\
			radeon_i2c.c				\
			radeon_ib.c				\
			radeon_irq_kms.c			\
			radeon_kms.c				\
			radeon_legacy_crtc.c			\
			radeon_legacy_encoders.c		\
			radeon_legacy_tv.c			\
			radeon_object.c				\
			radeon_pm.c				\
			radeon_ring.c				\
			radeon_sa.c				\
			radeon_semaphore.c			\
			radeon_sync.c				\
			radeon_test.c				\
			radeon_ttm.c				\
			radeon_ucode.c				\
			radeon_uvd.c				\
			radeon_vce.c				\
			radeon_vm.c				\
			rdisplay_kms.c				\
			r100.c					\
			r200.c					\
			r300.c					\
			r420.c					\
			rv515.c					\
			rv730_dpm.c				\
			rv740_dpm.c				\
			r520.c					\
			r600.c					\
			r600_blit_shaders.c			\
			r600_cs.c				\
			r600_dma.c				\
			r600_dpm.c				\
			r600_hdmi.c				\
			rs400.c					\
			rs600.c					\
			rs690.c					\
			rv6xx_dpm.c				\
			rs780_dpm.c				\
			rv770.c					\
			rv770_dma.c				\
			rv770_dpm.c				\
			rv770_smc.c				\
			rdisplay.c				\
			cmdline.c				\
			si.c					\
			si_blit_shaders.c			\
			si_dma.c				\
			si_dpm.c				\
			si_smc.c				\
			sumo_dpm.c				\
			sumo_smc.c				\
			trinity_dpm.c				\
			trinity_smc.c				\
			utils.c					\
			uvd_v1_0.c				\
			uvd_v2_2.c				\
			uvd_v3_1.c				\
			uvd_v4_2.c				\
			vce_v1_0.c				\
			vce_v2_0.c				\
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

$(NAME).dll: $(NAME_OBJS) $(FW_BINS) $(SRC_DEP) $(LIBPATH)/libcore.a $(LIBPATH)/libddk.a atikms.lds Makefile
	$(LD) -L$(LIBPATH) $(LDFLAGS) -T atikms.lds -o $@ $(NAME_OBJS) $(LIBS)  


%.o : %.c $(HFILES) Makefile
	$(CC) $(CFLAGS) -o $@ $<

%.o : %.S $(HFILES) Makefile
	$(AS) -o $@ $<

fwblob.o:  fwblob.asm $(FW_BINS) Makefile
	$(FASM) $< $@


clean: 
	-rm -f */*.o


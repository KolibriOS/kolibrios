
SRC_DIR:=$(SDK_DIR)/sources
MESA_SRC:= $(SRC_DIR)/Mesa/mesa-9.2.5/src

CFLAGS_OPT = -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32 -U_MSC_VER -Os
CFLAGS_OPT+= -fomit-frame-pointer -fno-ident -msse2 -mno-ms-bitfields
CFLAGS   = -c $(CFLAGS_OPT)

LDFLAGS = -shared -s --image-base 0 -T dll.lds -e _DllStartup

INC_MESA:= -I$(SRC_DIR)/newlib/libc/include -I$(SRC_DIR)/Mesa/mesa-9.2.5/include -I$(MESA_SRC)/mesa
INC_MESA+= -I$(MESA_SRC)/gallium/include -I$(MESA_SRC)/gallium/auxiliary
INC_MESA+= -I$(MESA_SRC)/gallium/winsys -I$(MESA_SRC)/gallium/drivers
INC_MESA+= -I$(MESA_SRC)/gallium/state_trackers/gbm -I$(MESA_SRC)/gbm/main
INC_MESA+= -I$(SRC_DIR)/libdrm
INC_I965:= -I$(SRC_DIR)/libdrm/intel -I$(SRC_DIR)/libdrm/include/drm

LIBS:=  -ldll -lgallium -lgcc -lc.dll

MESA_DEFS= -DMESA_DLL -DBUILD_GL32 -DMAPI_MODE_UTIL -DMAPI_TABLE_NUM_STATIC=87 -DUSE_X86_ASM -DUSE_MMX_ASM -DUSE_SSE_ASM -DMAPI_TABLE_NUM_DYNAMIC=256
MESA_DEFS+= -DPACKAGE_NAME=\"Mesa\" -DPACKAGE_VERSION=\"9.2.5\" -DPACKAGE_BUGREPORT=\"https://bugs.freedesktop.org/enter_bug.cgi\?product=Mesa\"
MESA_DEFS+= -DPACKAGE_URL=\"\" -DPACKAGE=\"mesa\" -DVERSION=\"9.2.5\" -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1
MESA_DEFS+= -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -D_GLAPI_DLL_EXPORTS 


GALLIUM_SRC:= 						\
		auxiliary/cso_cache/cso_cache.c		\
		auxiliary/cso_cache/cso_context.c	\
		auxiliary/cso_cache/cso_hash.c		\
		auxiliary/draw/draw_context.c		\
		auxiliary/draw/draw_fs.c		\
		auxiliary/draw/draw_gs.c		\
		auxiliary/draw/draw_pipe.c		\
		auxiliary/draw/draw_pipe_aaline.c	\
		auxiliary/draw/draw_pipe_aapoint.c	\
		auxiliary/draw/draw_pipe_clip.c		\
		auxiliary/draw/draw_pipe_cull.c		\
		auxiliary/draw/draw_pipe_flatshade.c	\
		auxiliary/draw/draw_pipe_offset.c	\
		auxiliary/draw/draw_pipe_pstipple.c 	\
		auxiliary/draw/draw_pipe_stipple.c 	\
		auxiliary/draw/draw_pipe_twoside.c 	\
		auxiliary/draw/draw_pipe_unfilled.c 	\
		auxiliary/draw/draw_pipe_util.c 	\
		auxiliary/draw/draw_pipe_validate.c 	\
		auxiliary/draw/draw_pipe_vbuf.c 	\
		auxiliary/draw/draw_pipe_wide_line.c 	\
		auxiliary/draw/draw_pipe_wide_point.c 	\
		auxiliary/draw/draw_prim_assembler.c 	\
		auxiliary/draw/draw_pt.c 		\
		auxiliary/draw/draw_pt_emit.c 		\
		auxiliary/draw/draw_pt_fetch.c 		\
		auxiliary/draw/draw_pt_fetch_emit.c 	\
		auxiliary/draw/draw_pt_fetch_shade_emit.c \
		auxiliary/draw/draw_pt_fetch_shade_pipeline.c \
		auxiliary/draw/draw_pt_post_vs.c 	\
		auxiliary/draw/draw_pt_so_emit.c 	\
		auxiliary/draw/draw_pt_util.c 		\
		auxiliary/draw/draw_pt_vsplit.c 	\
		auxiliary/draw/draw_vertex.c 		\
		auxiliary/draw/draw_vs.c 		\
		auxiliary/draw/draw_vs_exec.c 		\
		auxiliary/draw/draw_vs_variant.c 	\
		auxiliary/os/os_misc.c 			\
		auxiliary/os/os_time.c 			\
		auxiliary/rtasm/rtasm_cpu.c 		\
		auxiliary/rtasm/rtasm_execmem.c 	\
		auxiliary/rtasm/rtasm_x86sse.c 		\
		auxiliary/tgsi/tgsi_build.c 		\
		auxiliary/tgsi/tgsi_dump.c 		\
		auxiliary/tgsi/tgsi_exec.c 		\
		auxiliary/tgsi/tgsi_info.c 		\
		auxiliary/tgsi/tgsi_iterate.c 		\
		auxiliary/tgsi/tgsi_parse.c 		\
		auxiliary/tgsi/tgsi_sanity.c 		\
		auxiliary/tgsi/tgsi_scan.c 		\
		auxiliary/tgsi/tgsi_strings.c 		\
		auxiliary/tgsi/tgsi_text.c 		\
		auxiliary/tgsi/tgsi_transform.c 	\
		auxiliary/tgsi/tgsi_ureg.c 		\
		auxiliary/tgsi/tgsi_util.c 		\
		auxiliary/translate/translate.c 	\
		auxiliary/translate/translate_cache.c 	\
		auxiliary/translate/translate_generic.c \
		auxiliary/translate/translate_sse.c 	\
		auxiliary/util/u_debug.c 		\
		auxiliary/util/u_debug_describe.c 	\
		auxiliary/util/u_debug_memory.c 	\
		auxiliary/util/u_debug_refcnt.c 	\
		auxiliary/util/u_debug_stack.c 		\
		auxiliary/util/u_debug_symbol.c 	\
		auxiliary/util/u_dump_defines.c 	\
		auxiliary/util/u_dump_state.c 		\
		auxiliary/util/u_bitmask.c 		\
		auxiliary/util/u_blit.c			\
		auxiliary/util/u_blitter.c		\
		auxiliary/util/u_cache.c 		\
		auxiliary/util/u_caps.c 		\
		auxiliary/util/u_cpu_detect.c 		\
		auxiliary/util/u_dl.c			\
		auxiliary/util/u_draw.c			\
		auxiliary/util/u_draw_quad.c		\
		auxiliary/util/u_format.c		\
		auxiliary/util/u_format_other.c		\
		auxiliary/util/u_format_latc.c 		\
		auxiliary/util/u_format_srgb.c		\
		auxiliary/util/u_format_table.c		\
		auxiliary/util/u_format_s3tc.c 		\
		auxiliary/util/u_format_rgtc.c 		\
		auxiliary/util/u_format_etc.c 		\
		auxiliary/util/u_format_tests.c 	\
		auxiliary/util/u_format_yuv.c 		\
		auxiliary/util/u_format_zs.c 		\
		auxiliary/util/u_framebuffer.c 		\
		auxiliary/util/u_gen_mipmap.c 		\
		auxiliary/util/u_handle_table.c 	\
		auxiliary/util/u_hash.c 		\
		auxiliary/util/u_hash_table.c 		\
		auxiliary/util/u_helpers.c 		\
		auxiliary/util/u_index_modify.c 	\
		auxiliary/util/u_keymap.c 		\
		auxiliary/util/u_linear.c 		\
		auxiliary/util/u_linkage.c		\
		auxiliary/util/u_network.c		\
		auxiliary/util/u_math.c 		\
		auxiliary/util/u_mm.c 			\
		auxiliary/util/u_pstipple.c 		\
		auxiliary/util/u_ringbuffer.c 		\
		auxiliary/util/u_sampler.c 		\
		auxiliary/util/u_simple_shaders.c 	\
		auxiliary/util/u_slab.c 		\
		auxiliary/util/u_snprintf.c 		\
		auxiliary/util/u_staging.c 		\
		auxiliary/util/u_suballoc.c 		\
		auxiliary/util/u_surface.c 		\
		auxiliary/util/u_surfaces.c 		\
		auxiliary/util/u_texture.c 		\
		auxiliary/util/u_tile.c 		\
		auxiliary/util/u_transfer.c 		\
		auxiliary/util/u_resource.c 		\
		auxiliary/util/u_upload_mgr.c 		\
		auxiliary/util/u_vbuf.c 		\
		auxiliary/vl/vl_csc.c 			\
		auxiliary/vl/vl_compositor.c 		\
		auxiliary/vl/vl_matrix_filter.c 	\
		auxiliary/vl/vl_median_filter.c 	\
		auxiliary/vl/vl_decoder.c 		\
		auxiliary/vl/vl_mpeg12_decoder.c 	\
		auxiliary/vl/vl_mpeg12_bitstream.c 	\
		auxiliary/vl/vl_zscan.c 		\
		auxiliary/vl/vl_idct.c 			\
		auxiliary/vl/vl_mc.c 			\
		auxiliary/vl/vl_vertex_buffers.c 	\
		auxiliary/vl/vl_video_buffer.c

GBM_SRC = 						\
		auxiliary/pipe-loader/pipe_loader.c	\
		auxiliary/pipe-loader/pipe_loader_drm.c	\
		auxiliary/pipe-loader/pipe_loader_sw.c	\
		state_trackers/gbm/gbm_drm.c		\
		targets/gbm/gbm.c			\
		winsys/sw/null/null_sw_winsys.c

I965_SRC = 					\
	$(NULL)

GALLIUM_OBJS =  $(patsubst %.c, %.o, $(patsubst %.S, %.o, $(patsubst %.cpp, %.o, $(GALLIUM_SRC))))
GBM_OBJS =  $(patsubst %.c, %.o, $(GBM_SRC))

OSMESA_OBJS =  $(patsubst %.c, %.o, $(patsubst %.cpp, %.o, $(OSMESA_SRC)))
I965_OBJS =  $(patsubst %.c, %.o, $(patsubst %.cpp, %.o, $(I965_SRC)))

# targets

all: libgallium.a gbm_gallium_drm.dll

libgallium.a: $(GALLIUM_OBJS) Makefile
	$(AR) crs $@ $(GALLIUM_OBJS)
	mv -f $@ $(INSTALLDIR)

gbm_gallium_drm.dll: libgallium.a $(GBM_OBJS) Makefile
	$(LD) $(LDFLAGS) -o $@ $(GBM_OBJS) gbm.def $(LIBS)
	mv -f $@ $(SDK_DIR)/bin
  
i965_dri.drv: $(I965_OBJS) $(MESA_OBJS) dri.def Makefile
	$(LD) $(LDFLAGS) -o $@ $(I965_OBJS) $(MESA_OBJS) dri.def $(LIBS)
	mv -f $@ $(SDK_DIR)/bin

osmesa.dll: $(MESA_OBJS) $(OSMESA_OBJS) osmesa.def Makefile
	$(LD) $(LDFLAGS) --out-implib libosmesa.dll.a -o $@ $(MESA_OBJS) $(OSMESA_OBJS) osmesa.def $(LIBS)
	mv -f libosmesa.dll.a $(INSTALLDIR)
	mv -f $@ $(SDK_DIR)/bin

%.o : %.c Makefile
	$(CC) $(CFLAGS) -std=c99 $(MESA_DEFS) $(INC_MESA) $(INC_I965) -o $@ $<


%.o : %.cpp Makefile
	$(CC) $(CFLAGS) $(MESA_DEFS) $(INC_MESA) $(INC_I965) -o $@ $<



SRC_DIR:=$(SDK_DIR)/sources
MESA_SRC:= $(SRC_DIR)/Mesa/src

CFLAGS_OPT = -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32 -U_MSC_VER -Os
CFLAGS_OPT+= -fomit-frame-pointer -mno-ms-bitfields
CFLAGS   = -c $(CFLAGS_OPT)

LDFLAGS = -nostdlib -shared -s --image-base 0 -T $(SRC_DIR)/newlib/dll.lds -e _DllStartup

INC_MESA:= -I$(SRC_DIR)/newlib/libc/include -I$(SRC_DIR)/Mesa/include 
INC_MESA+= -I./ -I$(MESA_SRC)/glsl -I$(MESA_SRC)/mesa -I$(MESA_SRC)/mapi
INC_MESA+= -I$(SRC_DIR)/libdrm -I$(MESA_SRC) -I$(SRC_DIR)/expat/lib
INC_I965:= -I$(SRC_DIR)/libdrm/intel -I$(SRC_DIR)/libdrm/include/drm -I$(MESA_SRC)/mesa/drivers/dri/common 

LIBPATH:= -L$(SDK_DIR)/lib -L/home/autobuild/tools/win32/mingw32/lib

LIBS:=  -ldll -lglsl -lGL.dll -lsupc++ -lgcc_eh -ldrm.dll -lexpat -lc.dll -lgcc

MESA_DEFS= -DMESA_DLL -DBUILD_GL32 -DMAPI_MODE_UTIL -DMAPI_TABLE_NUM_STATIC=87 -DUSE_X86_ASM -DUSE_MMX_ASM -DUSE_SSE_ASM -DMAPI_TABLE_NUM_DYNAMIC=256
MESA_DEFS+= -DPACKAGE_NAME=\"Mesa\" -DPACKAGE_VERSION=\"9.2.5\" -DPACKAGE_BUGREPORT=\"https://bugs.freedesktop.org/enter_bug.cgi\?product=Mesa\"
MESA_DEFS+= -DPACKAGE_URL=\"\" -DPACKAGE=\"mesa\" -DVERSION=\"9.2.5\" -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1
MESA_DEFS+= -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -D_GLAPI_DLL_EXPORTS


MAIN_SRC = 				\
	drivers/common/driverfuncs.c	\
	drivers/common/meta.c		\
	drivers/dri/common/dri_util.c	\
	drivers/dri/common/utils.c	\
	drivers/dri/common/xmlconfig.c	\
	main/accum.c			\
	main/api_arrayelt.c		\
	main/api_exec.c			\
	main/api_loopback.c		\
	main/api_validate.c		\
	main/arbprogram.c		\
	main/arrayobj.c			\
	main/atifragshader.c		\
	main/attrib.c			\
	main/blend.c			\
	main/bufferobj.c		\
	main/buffers.c			\
	main/clear.c			\
	main/clip.c			\
	main/colortab.c			\
	main/condrender.c		\
	main/context.c			\
	main/convolve.c			\
	main/cpuinfo.c			\
	main/debug.c			\
	main/depth.c			\
	main/dlist.c			\
	main/drawpix.c			\
	main/drawtex.c			\
	main/enable.c			\
	main/enums.c			\
	main/errors.c			\
	main/es1_conversion.c		\
	main/eval.c			\
	main/execmem.c			\
	main/extensions.c		\
	main/ff_fragment_shader.cpp	\
	main/fbobject.c			\
	main/feedback.c			\
	main/ffvertex_prog.c		\
	main/fog.c			\
	main/format_pack.c		\
	main/format_unpack.c		\
	main/formatquery.c		\
	main/formats.c			\
	main/framebuffer.c		\
	main/get.c			\
	main/getstring.c		\
	main/glformats.c		\
	main/hash.c			\
	main/hash_table.c		\
	main/hint.c			\
	main/histogram.c		\
	main/image.c			\
	main/imports.c			\
	main/light.c			\
	main/lines.c			\
	main/matrix.c			\
	main/mipmap.c			\
	main/mm.c			\
	main/multisample.c		\
	main/pack.c			\
	main/pbo.c			\
	main/pixel.c			\
	main/pixelstore.c		\
	main/pixeltransfer.c		\
	main/points.c			\
	main/polygon.c			\
	main/querymatrix.c		\
	main/queryobj.c			\
	main/rastpos.c			\
	main/readpix.c			\
	main/remap.c			\
	main/renderbuffer.c		\
	main/samplerobj.c		\
	main/scissor.c			\
	main/set.c			\
	main/shader_query.c		\
	main/shaderapi.c		\
	main/shaderobj.c		\
	main/shared.c			\
	main/state.c			\
	main/stencil.c			\
	main/syncobj.c			\
	main/texcompress.c		\
	main/texcompress_cpal.c		\
	main/texcompress_etc.c		\
	main/texcompress_fxt1.c		\
	main/texcompress_rgtc.c		\
	main/texcompress_s3tc.c		\
	main/texenv.c			\
	main/texformat.c		\
	main/texgen.c			\
	main/texgetimage.c		\
	main/teximage.c			\
	main/texobj.c			\
	main/texparam.c			\
	main/texstate.c			\
	main/texstorage.c		\
	main/texstore.c			\
	main/texturebarrier.c		\
	main/transformfeedback.c	\
	main/uniform_query.cpp		\
	main/uniforms.c			\
	main/varray.c			\
	main/version.c			\
	main/viewport.c			\
	main/vtxfmt.c			\
	math/m_debug_clip.c		\
	math/m_debug_norm.c		\
	math/m_debug_xform.c		\
	math/m_eval.c			\
	math/m_matrix.c			\
	math/m_translate.c		\
	math/m_vector.c			\
	math/m_xform.c			\
	program/arbprogparse.c		\
	program/ir_to_mesa.cpp		\
	program/lex.yy.c		\
	program/prog_cache.c		\
	program/prog_execute.c		\
	program/prog_hash_table.c	\
	program/prog_instruction.c	\
	program/prog_noise.c		\
	program/prog_opt_constant_fold.c\
	program/prog_optimize.c		\
	program/prog_parameter.c	\
	program/prog_parameter_layout.c	\
	program/prog_print.c		\
	program/prog_statevars.c	\
	program/program.c		\
	program/program_parse.tab.c	\
	program/program_parse_extra.c	\
	program/programopt.c		\
	program/register_allocate.c	\
	program/sampler.cpp		\
	program/string_to_uint_map.cpp	\
	program/symbol_table.c		\
	swrast/s_aaline.c		\
	swrast/s_aatriangle.c		\
	swrast/s_alpha.c		\
	swrast/s_atifragshader.c	\
	swrast/s_bitmap.c		\
	swrast/s_blend.c		\
	swrast/s_blit.c			\
	swrast/s_clear.c		\
	swrast/s_context.c		\
	swrast/s_copypix.c		\
	swrast/s_depth.c		\
	swrast/s_drawpix.c		\
	swrast/s_feedback.c		\
	swrast/s_fog.c			\
	swrast/s_fragprog.c		\
	swrast/s_lines.c		\
	swrast/s_logic.c		\
	swrast/s_masking.c		\
	swrast/s_points.c		\
	swrast/s_renderbuffer.c		\
	swrast/s_span.c			\
	swrast/s_stencil.c		\
	swrast/s_texcombine.c		\
	swrast/s_texfetch.c		\
	swrast/s_texfilter.c		\
	swrast/s_texrender.c		\
	swrast/s_texture.c		\
	swrast/s_triangle.c		\
	swrast/s_zoom.c			\
	swrast_setup/ss_context.c	\
	swrast_setup/ss_triangle.c	\
	tnl/t_context.c			\
	tnl/t_draw.c			\
	tnl/t_pipeline.c		\
	tnl/t_rasterpos.c		\
	tnl/t_vb_fog.c			\
	tnl/t_vb_light.c		\
	tnl/t_vb_normals.c		\
	tnl/t_vb_points.c		\
	tnl/t_vb_program.c		\
	tnl/t_vb_render.c		\
	tnl/t_vb_texgen.c		\
	tnl/t_vb_texmat.c		\
	tnl/t_vb_vertex.c		\
	tnl/t_vertex.c			\
	tnl/t_vertex_generic.c		\
	tnl/t_vertex_sse.c		\
	tnl/t_vp_build.c		\
	vbo/vbo_context.c		\
	vbo/vbo_exec.c			\
	vbo/vbo_exec_api.c		\
	vbo/vbo_exec_array.c		\
	vbo/vbo_exec_draw.c		\
	vbo/vbo_exec_eval.c		\
	vbo/vbo_noop.c			\
	vbo/vbo_primitive_restart.c	\
	vbo/vbo_rebase.c		\
	vbo/vbo_save.c			\
	vbo/vbo_save_api.c		\
	vbo/vbo_save_draw.c		\
	vbo/vbo_save_loopback.c		\
	vbo/vbo_split.c			\
	vbo/vbo_split_copy.c		\
	vbo/vbo_split_inplace.c		\
	x86/common_x86.c		\
	x86/common_x86_asm.S		\
	x86/mmx_blend.S			\
	x86/read_rgba_span_x86.S	\
	x86/rtasm/x86sse.c		\
	x86/sse.c			\
	x86/sse_normal.S		\
	x86/sse_xform1.S		\
	x86/sse_xform2.S		\
	x86/sse_xform3.S		\
	x86/sse_xform4.S		\
	x86/x86_cliptest.S		\
	x86/x86_xform.c			\
	x86/x86_xform2.S		\
	x86/x86_xform3.S		\
	x86/x86_xform4.S		\
	$(NULL)

I965_SRC = 					\
	drivers/dri/i965/intel_batchbuffer.c 	\
	drivers/dri/i965/intel_blit.c 		\
	drivers/dri/i965/intel_buffer_objects.c \
	drivers/dri/i965/intel_buffers.c 	\
	drivers/dri/i965/intel_context.c 	\
	drivers/dri/i965/intel_extensions.c 	\
	drivers/dri/i965/intel_fbo.c 		\
	drivers/dri/i965/intel_mipmap_tree.c 	\
	drivers/dri/i965/intel_regions.c 	\
	drivers/dri/i965/intel_resolve_map.c 	\
	drivers/dri/i965/intel_screen.c 	\
	drivers/dri/i965/intel_pixel.c 		\
	drivers/dri/i965/intel_pixel_bitmap.c 	\
	drivers/dri/i965/intel_pixel_copy.c 	\
	drivers/dri/i965/intel_pixel_draw.c 	\
	drivers/dri/i965/intel_pixel_read.c 	\
	drivers/dri/i965/intel_state.c 		\
	drivers/dri/i965/intel_syncobj.c 	\
	drivers/dri/i965/intel_tex.c 		\
	drivers/dri/i965/intel_tex_copy.c 	\
	drivers/dri/i965/intel_tex_image.c 	\
	drivers/dri/i965/intel_tex_subimage.c 	\
	drivers/dri/i965/intel_tex_validate.c 	\
	drivers/dri/i965/brw_blorp.cpp 		\
	drivers/dri/i965/brw_blorp_blit.cpp 	\
	drivers/dri/i965/brw_blorp_clear.cpp 	\
	drivers/dri/i965/brw_cc.c 		\
	drivers/dri/i965/brw_cfg.cpp 		\
	drivers/dri/i965/brw_clear.c 		\
	drivers/dri/i965/brw_clip.c 		\
	drivers/dri/i965/brw_clip_line.c 	\
	drivers/dri/i965/brw_clip_point.c 	\
	drivers/dri/i965/brw_clip_state.c 	\
	drivers/dri/i965/brw_clip_tri.c 	\
	drivers/dri/i965/brw_clip_unfilled.c 	\
	drivers/dri/i965/brw_clip_util.c 	\
	drivers/dri/i965/brw_context.c 		\
	drivers/dri/i965/brw_cubemap_normalize.cpp 	\
	drivers/dri/i965/brw_curbe.c 		\
	drivers/dri/i965/brw_disasm.c 		\
	drivers/dri/i965/brw_draw.c 		\
	drivers/dri/i965/brw_draw_upload.c 	\
	drivers/dri/i965/brw_eu.c 		\
	drivers/dri/i965/brw_eu_compact.c 	\
	drivers/dri/i965/brw_eu_emit.c 		\
	drivers/dri/i965/brw_eu_util.c 		\
	drivers/dri/i965/brw_fs.cpp 			\
	drivers/dri/i965/brw_fs_channel_expressions.cpp \
	drivers/dri/i965/brw_fs_copy_propagation.cpp \
	drivers/dri/i965/brw_fs_cse.cpp 	\
	drivers/dri/i965/brw_fs_emit.cpp 	\
	drivers/dri/i965/brw_fs_fp.cpp 		\
	drivers/dri/i965/brw_fs_live_variables.cpp 	\
	drivers/dri/i965/brw_fs_reg_allocate.cpp 	\
	drivers/dri/i965/brw_fs_vector_splitting.cpp \
	drivers/dri/i965/brw_fs_visitor.cpp 	\
	drivers/dri/i965/brw_gs.c 		\
	drivers/dri/i965/brw_gs_emit.c 		\
	drivers/dri/i965/brw_gs_state.c 	\
	drivers/dri/i965/brw_lower_texture_gradients.cpp \
	drivers/dri/i965/brw_misc_state.c 	\
	drivers/dri/i965/brw_program.c 		\
	drivers/dri/i965/brw_primitive_restart.c\
	drivers/dri/i965/brw_queryobj.c 	\
	drivers/dri/i965/brw_schedule_instructions.cpp \
	drivers/dri/i965/brw_sf.c 		\
	drivers/dri/i965/brw_sf_emit.c 		\
	drivers/dri/i965/brw_sf_state.c 	\
	drivers/dri/i965/brw_shader.cpp 	\
	drivers/dri/i965/brw_state_batch.c 	\
	drivers/dri/i965/brw_state_cache.c 	\
	drivers/dri/i965/brw_state_dump.c 	\
	drivers/dri/i965/brw_state_upload.c 	\
	drivers/dri/i965/brw_surface_formats.c 	\
	drivers/dri/i965/brw_tex.c 		\
	drivers/dri/i965/brw_tex_layout.c 	\
	drivers/dri/i965/brw_urb.c 		\
	drivers/dri/i965/brw_util.c 		\
	drivers/dri/i965/brw_vec4.cpp 		\
	drivers/dri/i965/brw_vec4_copy_propagation.cpp \
	drivers/dri/i965/brw_vec4_emit.cpp 	\
	drivers/dri/i965/brw_vec4_live_variables.cpp \
	drivers/dri/i965/brw_vec4_reg_allocate.cpp 	\
	drivers/dri/i965/brw_vec4_visitor.cpp 	\
	drivers/dri/i965/brw_vec4_vp.cpp 	\
	drivers/dri/i965/brw_vs.c 		\
	drivers/dri/i965/brw_vs_state.c 	\
	drivers/dri/i965/brw_vs_surface_state.c \
	drivers/dri/i965/brw_vtbl.c 		\
	drivers/dri/i965/brw_wm.c 		\
	drivers/dri/i965/brw_wm_iz.cpp 		\
	drivers/dri/i965/brw_wm_sampler_state.c \
	drivers/dri/i965/brw_wm_state.c 	\
	drivers/dri/i965/brw_wm_surface_state.c \
	drivers/dri/i965/gen6_blorp.cpp 	\
	drivers/dri/i965/gen6_cc.c 		\
	drivers/dri/i965/gen6_clip_state.c 	\
	drivers/dri/i965/gen6_depthstencil.c 	\
	drivers/dri/i965/gen6_gs_state.c 	\
	drivers/dri/i965/gen6_multisample_state.c 	\
	drivers/dri/i965/gen6_queryobj.c 	\
	drivers/dri/i965/gen6_sampler_state.c 	\
	drivers/dri/i965/gen6_scissor_state.c 	\
	drivers/dri/i965/gen6_sf_state.c 	\
        drivers/dri/i965/gen6_sol.c 		\
	drivers/dri/i965/gen6_urb.c 		\
	drivers/dri/i965/gen6_viewport_state.c 	\
	drivers/dri/i965/gen6_vs_state.c 	\
	drivers/dri/i965/gen6_wm_state.c 	\
	drivers/dri/i965/gen7_blorp.cpp 	\
	drivers/dri/i965/gen7_clip_state.c 	\
	drivers/dri/i965/gen7_disable.c 	\
	drivers/dri/i965/gen7_misc_state.c 	\
	drivers/dri/i965/gen7_sampler_state.c 	\
	drivers/dri/i965/gen7_sf_state.c 	\
	drivers/dri/i965/gen7_sol_state.c 	\
	drivers/dri/i965/gen7_urb.c 		\
	drivers/dri/i965/gen7_viewport_state.c 	\
	drivers/dri/i965/gen7_vs_state.c 	\
	drivers/dri/i965/gen7_wm_state.c 	\
	drivers/dri/i965/gen7_wm_surface_state.c\
	$(NULL)

MESA_OBJS =  $(patsubst %.c, %.o, $(patsubst %.S, %.o, $(patsubst %.cpp, %.o, $(MAIN_SRC))))
I965_OBJS =  $(patsubst %.c, %.o, $(patsubst %.cpp, %.o, $(I965_SRC)))

# targets

all: i965_dri.drv

i965_dri.drv: $(I965_OBJS) $(MESA_OBJS) dri.def Makefile
	$(LD) $(LDFLAGS) $(LIBPATH) -o $@ $(I965_OBJS) $(MESA_OBJS) dri.def $(LIBS)
#	$(STRIP) $@
	mv -f $@ $(SDK_DIR)/bin

%.o : %.c Makefile
	$(CC) $(CFLAGS) -std=c99 $(MESA_DEFS) $(INC_MESA) $(INC_I965) -o $@ $<

%.o : %.S Makefile
	$(CC) -c -U_WIN32 -DUSE_X86_ASM -DUSE_MMX_ASM -DUSE_SSE_ASM -I $(MESA_SRC)/mesa -o $@ $<

%.o : %.cpp Makefile
	$(CC) $(CFLAGS) $(MESA_DEFS) $(INC_MESA) $(INC_I965) -o $@ $<


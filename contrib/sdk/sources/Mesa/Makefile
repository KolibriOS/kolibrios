
EGL = egl

CC = gcc

CFLAGS_OPT = -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32 -U_MSC_VER -Os 
CFLAGS_OPT+= -fomit-frame-pointer -mno-ms-bitfields
CFLAGS   = -c $(CFLAGS_OPT)

LD = ld
LDFLAGS = -nostdlib -shared -s --image-base 0 -T ../newlib/dll.lds --out-implib lib$(EGL).dll.a -e _DllStartup

STRIP = $(PREFIX)strip

INC_MESA=  -I../newlib/include -I./include -I./src -I./src/glsl -I./src/mesa -I./src/mapi
INC_EGL=   -I../newlib/include -I../../includes -I./include -I../libdrm -I../libdrm/include/drm -I./src/egl/main -I./src/gbm/backends/dri -I./src/gbm/main

LIBPATH:= -L../../lib

LIBS:=  -ldll -ldrm.dll -lc.dll -lgcc

MESA_DEFS= -DMESA_DLL -DBUILD_GL32 -DMAPI_MODE_UTIL -DMAPI_TABLE_NUM_STATIC=87 -DUSE_X86_ASM -DUSE_MMX_ASM -DUSE_SSE_ASM -DMAPI_TABLE_NUM_DYNAMIC=256
MESA_DEFS+= -DPACKAGE_NAME=\"Mesa\" -DPACKAGE_VERSION=\"9.2.5\" -DPACKAGE_BUGREPORT=\"https://bugs.freedesktop.org/enter_bug.cgi\?product=Mesa\"
MESA_DEFS+= -DPACKAGE_URL=\"\" -DPACKAGE=\"mesa\" -DVERSION=\"9.2.5\" -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1
MESA_DEFS+= -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1

EGL_DEFS = -DHAVE_DRM_PLATFORM -D__unix__ -DMESA_EGL_NO_X11_HEADERS -D_EGL_BUILT_IN_DRIVER_DRI2

EGL_SRC = 					\
	src/egl/drivers/dri2/egl_dri2.c		\
	src/egl/drivers/dri2/platform_drm.c	\
	src/egl/main/eglapi.c			\
	src/egl/main/eglarray.c			\
	src/egl/main/eglconfig.c		\
	src/egl/main/eglcontext.c		\
	src/egl/main/eglcurrent.c		\
	src/egl/main/egldisplay.c		\
	src/egl/main/egldriver.c		\
	src/egl/main/eglfallbacks.c		\
	src/egl/main/eglglobals.c		\
	src/egl/main/eglimage.c			\
	src/egl/main/egllog.c			\
	src/egl/main/eglmisc.c			\
	src/egl/main/eglmode.c			\
	src/egl/main/eglscreen.c		\
	src/egl/main/eglstring.c		\
	src/egl/main/eglsurface.c		\
	src/egl/main/eglsync.c			\
	src/gbm/backends/dri/driver_name.c	\
	src/gbm/backends/dri/gbm_dri.c		\
	src/gbm/main/backend.c			\
	src/gbm/main/gbm.c			\
	$(NULL)

GLSL_SRC = \
	src/glsl/glcpp/glcpp-lex.c		\
	src/glsl/glcpp/glcpp-parse.c		\
	src/glsl/glcpp/pp.c			\
	src/glsl/ast_array_index.cpp		\
	src/glsl/ast_expr.cpp			\
	src/glsl/ast_function.cpp		\
	src/glsl/ast_to_hir.cpp			\
	src/glsl/ast_type.cpp			\
	src/glsl/builtin_function.cpp		\
	src/glsl/builtin_types.cpp		\
	src/glsl/builtin_variables.cpp		\
	src/glsl/glsl_lexer.cpp			\
	src/glsl/glsl_parser.cpp		\
	src/glsl/glsl_parser_extras.cpp		\
	src/glsl/glsl_symbol_table.cpp		\
	src/glsl/glsl_types.cpp			\
	src/glsl/hir_field_selection.cpp	\
	src/glsl/ir.cpp				\
	src/glsl/ir_basic_block.cpp		\
	src/glsl/ir_builder.cpp			\
	src/glsl/ir_clone.cpp			\
	src/glsl/ir_constant_expression.cpp	\
	src/glsl/ir_expression_flattening.cpp	\
	src/glsl/ir_function.cpp		\
	src/glsl/ir_function_can_inline.cpp	\
	src/glsl/ir_function_detect_recursion.cpp	\
	src/glsl/ir_hierarchical_visitor.cpp	\
	src/glsl/ir_hv_accept.cpp		\
	src/glsl/ir_import_prototypes.cpp	\
	src/glsl/ir_print_visitor.cpp		\
	src/glsl/ir_reader.cpp			\
	src/glsl/ir_rvalue_visitor.cpp		\
	src/glsl/ir_set_program_inouts.cpp	\
	src/glsl/ir_validate.cpp		\
	src/glsl/ir_variable_refcount.cpp	\
	src/glsl/link_functions.cpp	\
	src/glsl/link_interface_blocks.cpp	\
	src/glsl/link_uniform_block_active_visitor.cpp	\
	src/glsl/link_uniform_blocks.cpp	\
	src/glsl/link_uniform_initializers.cpp	\
	src/glsl/link_uniforms.cpp		\
	src/glsl/link_varyings.cpp		\
	src/glsl/linker.cpp			\
	src/glsl/loop_analysis.cpp		\
	src/glsl/loop_controls.cpp		\
	src/glsl/loop_unroll.cpp		\
	src/glsl/lower_clip_distance.cpp	\
	src/glsl/lower_discard.cpp		\
	src/glsl/lower_discard_flow.cpp		\
	src/glsl/lower_if_to_cond_assign.cpp	\
	src/glsl/lower_instructions.cpp		\
	src/glsl/lower_jumps.cpp		\
	src/glsl/lower_mat_op_to_vec.cpp	\
	src/glsl/lower_named_interface_blocks.cpp	\
	src/glsl/lower_noise.cpp		\
	src/glsl/lower_output_reads.cpp		\
	src/glsl/lower_packed_varyings.cpp	\
	src/glsl/lower_packing_builtins.cpp	\
	src/glsl/lower_texture_projection.cpp	\
	src/glsl/lower_ubo_reference.cpp	\
	src/glsl/lower_variable_index_to_cond_assign.cpp	\
	src/glsl/lower_vec_index_to_cond_assign.cpp	\
	src/glsl/lower_vec_index_to_swizzle.cpp	\
	src/glsl/lower_vector.cpp		\
	src/glsl/lower_vector_insert.cpp	\
	src/glsl/opt_algebraic.cpp		\
	src/glsl/opt_array_splitting.cpp	\
	src/glsl/opt_constant_folding.cpp	\
	src/glsl/opt_constant_propagation.cpp	\
	src/glsl/opt_constant_variable.cpp	\
	src/glsl/opt_copy_propagation.cpp	\
	src/glsl/opt_copy_propagation_elements.cpp	\
	src/glsl/opt_dead_builtin_varyings.cpp	\
	src/glsl/opt_dead_code.cpp		\
	src/glsl/opt_dead_code_local.cpp	\
	src/glsl/opt_dead_functions.cpp		\
	src/glsl/opt_flatten_nested_if_blocks.cpp	\
	src/glsl/opt_flip_matrices.cpp		\
	src/glsl/opt_function_inlining.cpp	\
	src/glsl/opt_if_simplification.cpp	\
	src/glsl/opt_noop_swizzle.cpp		\
	src/glsl/opt_redundant_jumps.cpp	\
	src/glsl/opt_structure_splitting.cpp	\
	src/glsl/opt_swizzle_swizzle.cpp	\
	src/glsl/opt_tree_grafting.cpp		\
	src/glsl/ralloc.c			\
	src/glsl/s_expression.cpp		\
	src/glsl/strtod.c			\
	$(NULL)

LIBGL_SRC = \
	src/mapi/glapi/glapi.c			\
	src/mapi/glapi/glapi_dispatch.c		\
	src/mapi/glapi/glapi_entrypoint.c	\
	src/mapi/glapi/glapi_getproc.c		\
	src/mapi/glapi/glapi_nop.c		\
	src/mapi/glapi/glapi_x86.S		\
	src/mapi/u_current.c			\
	src/mapi/u_execmem.c			\
	$(NULL)
 
EGL_OBJS   =  $(patsubst %.c, %.o, $(EGL_SRC))
GLSL_OBJS  =  $(patsubst %.cpp, %.o, $(patsubst %.c, %.o, $(GLSL_SRC)))
LIBGL_OBJS =  $(patsubst %.c, %.o, $(patsubst %.S, %.o, $(LIBGL_SRC)))

# targets 

all: libGL.dll $(EGL).dll libglsl.a 

$(EGL).dll: $(EGL_OBJS) Makefile
	$(LD) $(LDFLAGS) $(LIBPATH) -o $@ egl.def $(EGL_OBJS) $(LIBS)
	$(STRIP) $@
#	sed -e "s/ @[^ ]*//" egl1.def > egl.def
	mv -f $@ ../../bin
	mv -f lib$(EGL).dll.a ../../lib   

libglsl.a : $(GLSL_OBJS) Makefile
	ar cvrs libglsl.a $(GLSL_OBJS) 
	mv -f libglsl.a ../../lib

libGL.dll: $(LIBGL_OBJS) Makefile
	$(LD) $(LDFLAGS) $(LIBPATH) -o $@ $(LIBGL_OBJS) $(LIBS)
	$(STRIP) $@
#	sed -e "s/ @[^ ]*//" egl1.def > egl.def
	mv -f $@ ../../bin
	mv -f libGL.dll.a ../../lib   

src/glsl/glcpp/%.o : src/glsl/glcpp/%.c Makefile
	$(CC) $(CFLAGS) -std=c99 $(MESA_DEFS) $(INC_MESA) -o $@ $<

src/glsl/%.o : src/glsl/%.c Makefile
	$(CC) $(CFLAGS) -std=c99 $(MESA_DEFS) $(INC_MESA) -o $@ $<

src/mapi/glapi/%.o : src/mapi/glapi/%.c Makefile
	$(CC) $(CFLAGS) -std=c99 $(MESA_DEFS) $(INC_MESA) -o $@ $<
	
%.o : %.c Makefile
	$(CC) $(CFLAGS) $(MESA_DEFS) $(EGL_DEFS) $(INC_EGL) -o $@ $<


%.o : %.cpp Makefile
	$(CC) $(CFLAGS) $(MESA_DEFS) $(INC_MESA) -o $@ $<
 
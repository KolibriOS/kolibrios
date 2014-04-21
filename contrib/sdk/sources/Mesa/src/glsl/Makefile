
SRC_DIR:=$(SDK_DIR)/sources
MESA_SRC:= $(SRC_DIR)/Mesa/src

CFLAGS_OPT = -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32 -U_MSC_VER -Os
CFLAGS_OPT+= -fomit-frame-pointer -mno-ms-bitfields
CFLAGS   = -c $(CFLAGS_OPT)

INC_MESA:= -I$(SRC_DIR)/newlib/include -I$(SRC_DIR)/Mesa/include
INC_MESA+= -I./src -I$(MESA_SRC)/glsl -I$(MESA_SRC)/mesa -I$(MESA_SRC)/mapi

MESA_DEFS= -DMESA_DLL -DBUILD_GL32 -DMAPI_MODE_UTIL -DMAPI_TABLE_NUM_STATIC=87 -DUSE_X86_ASM -DUSE_MMX_ASM -DUSE_SSE_ASM -DMAPI_TABLE_NUM_DYNAMIC=256
MESA_DEFS+= -DPACKAGE_NAME=\"Mesa\" -DPACKAGE_VERSION=\"9.2.5\" -DPACKAGE_BUGREPORT=\"https://bugs.freedesktop.org/enter_bug.cgi\?product=Mesa\"
MESA_DEFS+= -DPACKAGE_URL=\"\" -DPACKAGE=\"mesa\" -DVERSION=\"9.2.5\" -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1
MESA_DEFS+= -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1

GLSL_SRC = \
	glcpp/glcpp-lex.c		\
	glcpp/glcpp-parse.c		\
	glcpp/pp.c			\
	ast_array_index.cpp		\
	ast_expr.cpp			\
	ast_function.cpp		\
	ast_to_hir.cpp			\
	ast_type.cpp			\
	builtin_function.cpp		\
	builtin_types.cpp		\
	builtin_variables.cpp		\
	glsl_lexer.cpp			\
	glsl_parser.cpp			\
	glsl_parser_extras.cpp		\
	glsl_symbol_table.cpp		\
	glsl_types.cpp			\
	hir_field_selection.cpp		\
        ir.cpp                          \
        ir_basic_block.cpp              \
        ir_builder.cpp                  \
        ir_clone.cpp                    \
        ir_constant_expression.cpp      \
        ir_expression_flattening.cpp    \
        ir_function.cpp         	\
        ir_function_can_inline.cpp      \
        ir_function_detect_recursion.cpp\
        ir_hierarchical_visitor.cpp     \
        ir_hv_accept.cpp                \
        ir_import_prototypes.cpp        \
        ir_print_visitor.cpp            \
        ir_reader.cpp                   \
        ir_rvalue_visitor.cpp           \
        ir_set_program_inouts.cpp       \
        ir_validate.cpp         	\
        ir_variable_refcount.cpp        \
        link_functions.cpp      	\
        link_interface_blocks.cpp       \
        link_uniform_block_active_visitor.cpp   \
        link_uniform_blocks.cpp 	\
        link_uniform_initializers.cpp   \
        link_uniforms.cpp               \
        link_varyings.cpp               \
        linker.cpp                      \
        loop_analysis.cpp               \
        loop_controls.cpp               \
        loop_unroll.cpp         	\
        lower_clip_distance.cpp 	\
        lower_discard.cpp               \
        lower_discard_flow.cpp          \
        lower_if_to_cond_assign.cpp     \
        lower_instructions.cpp          \
        lower_jumps.cpp         	\
        lower_mat_op_to_vec.cpp 	\
        lower_named_interface_blocks.cpp\
        lower_noise.cpp         	\
        lower_output_reads.cpp          \
        lower_packed_varyings.cpp       \
        lower_packing_builtins.cpp      \
        lower_texture_projection.cpp    \
        lower_ubo_reference.cpp 	\
        lower_variable_index_to_cond_assign.cpp \
        lower_vec_index_to_cond_assign.cpp      \
        lower_vec_index_to_swizzle.cpp  \
        lower_vector.cpp                \
        lower_vector_insert.cpp 	\
        opt_algebraic.cpp               \
        opt_array_splitting.cpp 	\
        opt_constant_folding.cpp        \
        opt_constant_propagation.cpp    \
        opt_constant_variable.cpp       \
        opt_copy_propagation.cpp        \
        opt_copy_propagation_elements.cpp       \
        opt_dead_builtin_varyings.cpp   \
        opt_dead_code.cpp               \
        opt_dead_code_local.cpp 	\
        opt_dead_functions.cpp          \
        opt_flatten_nested_if_blocks.cpp\
        opt_flip_matrices.cpp           \
        opt_function_inlining.cpp       \
        opt_if_simplification.cpp       \
        opt_noop_swizzle.cpp            \
        opt_redundant_jumps.cpp 	\
        opt_structure_splitting.cpp     \
        opt_swizzle_swizzle.cpp 	\
        opt_tree_grafting.cpp           \
        ralloc.c                        \
        s_expression.cpp                \
        strtod.c                        \
	$(NULL)


GLSL_OBJS  =  $(patsubst %.cpp, %.o, $(patsubst %.c, %.o, $(GLSL_SRC)))

# targets

all: libglsl.a

libglsl.a : $(GLSL_OBJS) Makefile
	$(AR) crs libglsl.a $(GLSL_OBJS)
	mv -f libglsl.a $(SDK_DIR)/lib

%.o : %.c Makefile
	$(CC) $(CFLAGS) -std=c99 $(MESA_DEFS) $(INC_MESA) -o $@ $<

%.o : %.cpp Makefile
	$(CC) $(CFLAGS) $(MESA_DEFS) $(INC_MESA) -o $@ $<


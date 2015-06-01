
#include "nir.h"
#include "nir_search.h"

#ifndef NIR_OPT_ALGEBRAIC_STRUCT_DEFS
#define NIR_OPT_ALGEBRAIC_STRUCT_DEFS

struct transform {
   const nir_search_expression *search;
   const nir_search_value *replace;
   unsigned condition_offset;
};

struct opt_state {
   void *mem_ctx;
   bool progress;
   const bool *condition_flags;
};

#endif

   
static const nir_search_variable search66_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_bool,
};

static const nir_search_constant search66_1 = {
   { nir_search_value_constant },
   { 0x3f800000 /* 1.0 */ },
};
static const nir_search_expression search66 = {
   { nir_search_value_expression },
   nir_op_iand,
   { &search66_0.value, &search66_1.value },
};
   
static const nir_search_variable replace66_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace66 = {
   { nir_search_value_expression },
   nir_op_b2f,
   { &replace66_0.value },
};
   
static const nir_search_variable search76_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search76_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search76 = {
   { nir_search_value_expression },
   nir_op_iand,
   { &search76_0.value, &search76_1.value },
};
   
static const nir_search_variable replace76 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
   
static const nir_search_variable search77_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_constant search77_1 = {
   { nir_search_value_constant },
   { 0x0 /* 0 */ },
};
static const nir_search_expression search77 = {
   { nir_search_value_expression },
   nir_op_iand,
   { &search77_0.value, &search77_1.value },
};
   
static const nir_search_constant replace77 = {
   { nir_search_value_constant },
   { 0x0 /* 0 */ },
};
   
static const nir_search_variable search83_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search83_0 = {
   { nir_search_value_expression },
   nir_op_inot,
   { &search83_0_0.value },
};

static const nir_search_variable search83_1_0 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search83_1 = {
   { nir_search_value_expression },
   nir_op_inot,
   { &search83_1_0.value },
};
static const nir_search_expression search83 = {
   { nir_search_value_expression },
   nir_op_iand,
   { &search83_0.value, &search83_1.value },
};
   
static const nir_search_variable replace83_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace83_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace83_0 = {
   { nir_search_value_expression },
   nir_op_ior,
   { &replace83_0_0.value, &replace83_0_1.value },
};
static const nir_search_expression replace83 = {
   { nir_search_value_expression },
   nir_op_inot,
   { &replace83_0.value },
};

static const struct transform nir_opt_algebraic_iand_xforms[] = {
   { &search66, &replace66.value, 0 },
   { &search76, &replace76.value, 0 },
   { &search77, &replace77.value, 0 },
   { &search83, &replace83.value, 0 },
};
   
static const nir_search_variable search81_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search81_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search81 = {
   { nir_search_value_expression },
   nir_op_ixor,
   { &search81_0.value, &search81_1.value },
};
   
static const nir_search_constant replace81 = {
   { nir_search_value_constant },
   { 0x0 /* 0 */ },
};

static const struct transform nir_opt_algebraic_ixor_xforms[] = {
   { &search81, &replace81.value, 0 },
};
   
static const nir_search_variable search61_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search61_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search61 = {
   { nir_search_value_expression },
   nir_op_seq,
   { &search61_0.value, &search61_1.value },
};
   
static const nir_search_variable replace61_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace61_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace61_0 = {
   { nir_search_value_expression },
   nir_op_feq,
   { &replace61_0_0.value, &replace61_0_1.value },
};
static const nir_search_expression replace61 = {
   { nir_search_value_expression },
   nir_op_b2f,
   { &replace61_0.value },
};

static const struct transform nir_opt_algebraic_seq_xforms[] = {
   { &search61, &replace61.value, 7 },
};
   
static const nir_search_variable search142_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search142_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   true,
   nir_type_invalid,
};

static const nir_search_variable search142_0_2 = {
   { nir_search_value_variable },
   2, /* c */
   true,
   nir_type_invalid,
};
static const nir_search_expression search142_0 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &search142_0_0.value, &search142_0_1.value, &search142_0_2.value },
};

static const nir_search_variable search142_1 = {
   { nir_search_value_variable },
   3, /* d */
   true,
   nir_type_invalid,
};
static const nir_search_expression search142 = {
   { nir_search_value_expression },
   nir_op_fne,
   { &search142_0.value, &search142_1.value },
};
   
static const nir_search_variable replace142_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace142_1_0 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace142_1_1 = {
   { nir_search_value_variable },
   3, /* d */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace142_1 = {
   { nir_search_value_expression },
   nir_op_fne,
   { &replace142_1_0.value, &replace142_1_1.value },
};

static const nir_search_variable replace142_2_0 = {
   { nir_search_value_variable },
   2, /* c */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace142_2_1 = {
   { nir_search_value_variable },
   3, /* d */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace142_2 = {
   { nir_search_value_expression },
   nir_op_fne,
   { &replace142_2_0.value, &replace142_2_1.value },
};
static const nir_search_expression replace142 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &replace142_0.value, &replace142_1.value, &replace142_2.value },
};
   
static const nir_search_variable search143_0 = {
   { nir_search_value_variable },
   0, /* d */
   true,
   nir_type_invalid,
};

static const nir_search_variable search143_1_0 = {
   { nir_search_value_variable },
   1, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search143_1_1 = {
   { nir_search_value_variable },
   2, /* b */
   true,
   nir_type_invalid,
};

static const nir_search_variable search143_1_2 = {
   { nir_search_value_variable },
   3, /* c */
   true,
   nir_type_invalid,
};
static const nir_search_expression search143_1 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &search143_1_0.value, &search143_1_1.value, &search143_1_2.value },
};
static const nir_search_expression search143 = {
   { nir_search_value_expression },
   nir_op_fne,
   { &search143_0.value, &search143_1.value },
};
   
static const nir_search_variable replace143_0 = {
   { nir_search_value_variable },
   1, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace143_1_0 = {
   { nir_search_value_variable },
   0, /* d */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace143_1_1 = {
   { nir_search_value_variable },
   2, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace143_1 = {
   { nir_search_value_expression },
   nir_op_fne,
   { &replace143_1_0.value, &replace143_1_1.value },
};

static const nir_search_variable replace143_2_0 = {
   { nir_search_value_variable },
   0, /* d */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace143_2_1 = {
   { nir_search_value_variable },
   3, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace143_2 = {
   { nir_search_value_expression },
   nir_op_fne,
   { &replace143_2_0.value, &replace143_2_1.value },
};
static const nir_search_expression replace143 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &replace143_0.value, &replace143_1.value, &replace143_2.value },
};

static const struct transform nir_opt_algebraic_fne_xforms[] = {
   { &search142, &replace142.value, 0 },
   { &search143, &replace143.value, 0 },
};
   
static const nir_search_variable search13_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_constant search13_1 = {
   { nir_search_value_constant },
   { 0x0 /* 0 */ },
};
static const nir_search_expression search13 = {
   { nir_search_value_expression },
   nir_op_imul,
   { &search13_0.value, &search13_1.value },
};
   
static const nir_search_constant replace13 = {
   { nir_search_value_constant },
   { 0x0 /* 0 */ },
};
   
static const nir_search_variable search15_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_constant search15_1 = {
   { nir_search_value_constant },
   { 0x1 /* 1 */ },
};
static const nir_search_expression search15 = {
   { nir_search_value_expression },
   nir_op_imul,
   { &search15_0.value, &search15_1.value },
};
   
static const nir_search_variable replace15 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
   
static const nir_search_variable search17_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_constant search17_1 = {
   { nir_search_value_constant },
   { 0xffffffff /* -1 */ },
};
static const nir_search_expression search17 = {
   { nir_search_value_expression },
   nir_op_imul,
   { &search17_0.value, &search17_1.value },
};
   
static const nir_search_variable replace17_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace17 = {
   { nir_search_value_expression },
   nir_op_ineg,
   { &replace17_0.value },
};
   
static const nir_search_variable search63_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search63_0 = {
   { nir_search_value_expression },
   nir_op_b2i,
   { &search63_0_0.value },
};

static const nir_search_variable search63_1_0 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search63_1 = {
   { nir_search_value_expression },
   nir_op_b2i,
   { &search63_1_0.value },
};
static const nir_search_expression search63 = {
   { nir_search_value_expression },
   nir_op_imul,
   { &search63_0.value, &search63_1.value },
};
   
static const nir_search_variable replace63_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace63_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace63_0 = {
   { nir_search_value_expression },
   nir_op_iand,
   { &replace63_0_0.value, &replace63_0_1.value },
};
static const nir_search_expression replace63 = {
   { nir_search_value_expression },
   nir_op_b2i,
   { &replace63_0.value },
};

static const struct transform nir_opt_algebraic_imul_xforms[] = {
   { &search13, &replace13.value, 0 },
   { &search15, &replace15.value, 0 },
   { &search17, &replace17.value, 0 },
   { &search63, &replace63.value, 0 },
};
   
static const nir_search_variable search74_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search74_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search74 = {
   { nir_search_value_expression },
   nir_op_uge,
   { &search74_0.value, &search74_1.value },
};
   
static const nir_search_constant replace74 = {
   { nir_search_value_constant },
   { NIR_TRUE /* True */ },
};
   
static const nir_search_variable search154_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search154_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   true,
   nir_type_invalid,
};

static const nir_search_variable search154_0_2 = {
   { nir_search_value_variable },
   2, /* c */
   true,
   nir_type_invalid,
};
static const nir_search_expression search154_0 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &search154_0_0.value, &search154_0_1.value, &search154_0_2.value },
};

static const nir_search_variable search154_1 = {
   { nir_search_value_variable },
   3, /* d */
   true,
   nir_type_invalid,
};
static const nir_search_expression search154 = {
   { nir_search_value_expression },
   nir_op_uge,
   { &search154_0.value, &search154_1.value },
};
   
static const nir_search_variable replace154_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace154_1_0 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace154_1_1 = {
   { nir_search_value_variable },
   3, /* d */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace154_1 = {
   { nir_search_value_expression },
   nir_op_uge,
   { &replace154_1_0.value, &replace154_1_1.value },
};

static const nir_search_variable replace154_2_0 = {
   { nir_search_value_variable },
   2, /* c */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace154_2_1 = {
   { nir_search_value_variable },
   3, /* d */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace154_2 = {
   { nir_search_value_expression },
   nir_op_uge,
   { &replace154_2_0.value, &replace154_2_1.value },
};
static const nir_search_expression replace154 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &replace154_0.value, &replace154_1.value, &replace154_2.value },
};
   
static const nir_search_variable search155_0 = {
   { nir_search_value_variable },
   0, /* d */
   true,
   nir_type_invalid,
};

static const nir_search_variable search155_1_0 = {
   { nir_search_value_variable },
   1, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search155_1_1 = {
   { nir_search_value_variable },
   2, /* b */
   true,
   nir_type_invalid,
};

static const nir_search_variable search155_1_2 = {
   { nir_search_value_variable },
   3, /* c */
   true,
   nir_type_invalid,
};
static const nir_search_expression search155_1 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &search155_1_0.value, &search155_1_1.value, &search155_1_2.value },
};
static const nir_search_expression search155 = {
   { nir_search_value_expression },
   nir_op_uge,
   { &search155_0.value, &search155_1.value },
};
   
static const nir_search_variable replace155_0 = {
   { nir_search_value_variable },
   1, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace155_1_0 = {
   { nir_search_value_variable },
   0, /* d */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace155_1_1 = {
   { nir_search_value_variable },
   2, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace155_1 = {
   { nir_search_value_expression },
   nir_op_uge,
   { &replace155_1_0.value, &replace155_1_1.value },
};

static const nir_search_variable replace155_2_0 = {
   { nir_search_value_variable },
   0, /* d */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace155_2_1 = {
   { nir_search_value_variable },
   3, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace155_2 = {
   { nir_search_value_expression },
   nir_op_uge,
   { &replace155_2_0.value, &replace155_2_1.value },
};
static const nir_search_expression replace155 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &replace155_0.value, &replace155_1.value, &replace155_2.value },
};

static const struct transform nir_opt_algebraic_uge_xforms[] = {
   { &search74, &replace74.value, 0 },
   { &search154, &replace154.value, 0 },
   { &search155, &replace155.value, 0 },
};
   
static const nir_search_variable search12_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_constant search12_1 = {
   { nir_search_value_constant },
   { 0x0 /* 0.0 */ },
};
static const nir_search_expression search12 = {
   { nir_search_value_expression },
   nir_op_fmul,
   { &search12_0.value, &search12_1.value },
};
   
static const nir_search_constant replace12 = {
   { nir_search_value_constant },
   { 0x0 /* 0.0 */ },
};
   
static const nir_search_variable search14_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_constant search14_1 = {
   { nir_search_value_constant },
   { 0x3f800000 /* 1.0 */ },
};
static const nir_search_expression search14 = {
   { nir_search_value_expression },
   nir_op_fmul,
   { &search14_0.value, &search14_1.value },
};
   
static const nir_search_variable replace14 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
   
static const nir_search_variable search16_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_constant search16_1 = {
   { nir_search_value_constant },
   { 0xbf800000 /* -1.0 */ },
};
static const nir_search_expression search16 = {
   { nir_search_value_expression },
   nir_op_fmul,
   { &search16_0.value, &search16_1.value },
};
   
static const nir_search_variable replace16_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace16 = {
   { nir_search_value_expression },
   nir_op_fneg,
   { &replace16_0.value },
};
   
static const nir_search_variable search64_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search64_0 = {
   { nir_search_value_expression },
   nir_op_b2f,
   { &search64_0_0.value },
};

static const nir_search_variable search64_1_0 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search64_1 = {
   { nir_search_value_expression },
   nir_op_b2f,
   { &search64_1_0.value },
};
static const nir_search_expression search64 = {
   { nir_search_value_expression },
   nir_op_fmul,
   { &search64_0.value, &search64_1.value },
};
   
static const nir_search_variable replace64_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace64_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace64_0 = {
   { nir_search_value_expression },
   nir_op_iand,
   { &replace64_0_0.value, &replace64_0_1.value },
};
static const nir_search_expression replace64 = {
   { nir_search_value_expression },
   nir_op_b2f,
   { &replace64_0.value },
};
   
static const nir_search_variable search108_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search108_0 = {
   { nir_search_value_expression },
   nir_op_fexp2,
   { &search108_0_0.value },
};

static const nir_search_variable search108_1_0 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search108_1 = {
   { nir_search_value_expression },
   nir_op_fexp2,
   { &search108_1_0.value },
};
static const nir_search_expression search108 = {
   { nir_search_value_expression },
   nir_op_fmul,
   { &search108_0.value, &search108_1.value },
};
   
static const nir_search_variable replace108_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace108_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace108_0 = {
   { nir_search_value_expression },
   nir_op_fadd,
   { &replace108_0_0.value, &replace108_0_1.value },
};
static const nir_search_expression replace108 = {
   { nir_search_value_expression },
   nir_op_fexp2,
   { &replace108_0.value },
};

static const struct transform nir_opt_algebraic_fmul_xforms[] = {
   { &search12, &replace12.value, 0 },
   { &search14, &replace14.value, 0 },
   { &search16, &replace16.value, 0 },
   { &search64, &replace64.value, 0 },
   { &search108, &replace108.value, 0 },
};
   
static const nir_search_constant search18_0 = {
   { nir_search_value_constant },
   { 0x0 /* 0.0 */ },
};

static const nir_search_variable search18_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search18_2 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search18 = {
   { nir_search_value_expression },
   nir_op_ffma,
   { &search18_0.value, &search18_1.value, &search18_2.value },
};
   
static const nir_search_variable replace18 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
   
static const nir_search_variable search19_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_constant search19_1 = {
   { nir_search_value_constant },
   { 0x0 /* 0.0 */ },
};

static const nir_search_variable search19_2 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search19 = {
   { nir_search_value_expression },
   nir_op_ffma,
   { &search19_0.value, &search19_1.value, &search19_2.value },
};
   
static const nir_search_variable replace19 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
   
static const nir_search_variable search20_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search20_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_constant search20_2 = {
   { nir_search_value_constant },
   { 0x0 /* 0.0 */ },
};
static const nir_search_expression search20 = {
   { nir_search_value_expression },
   nir_op_ffma,
   { &search20_0.value, &search20_1.value, &search20_2.value },
};
   
static const nir_search_variable replace20_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace20_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace20 = {
   { nir_search_value_expression },
   nir_op_fmul,
   { &replace20_0.value, &replace20_1.value },
};
   
static const nir_search_variable search21_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_constant search21_1 = {
   { nir_search_value_constant },
   { 0x3f800000 /* 1.0 */ },
};

static const nir_search_variable search21_2 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search21 = {
   { nir_search_value_expression },
   nir_op_ffma,
   { &search21_0.value, &search21_1.value, &search21_2.value },
};
   
static const nir_search_variable replace21_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace21_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace21 = {
   { nir_search_value_expression },
   nir_op_fadd,
   { &replace21_0.value, &replace21_1.value },
};
   
static const nir_search_constant search22_0 = {
   { nir_search_value_constant },
   { 0x3f800000 /* 1.0 */ },
};

static const nir_search_variable search22_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search22_2 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search22 = {
   { nir_search_value_expression },
   nir_op_ffma,
   { &search22_0.value, &search22_1.value, &search22_2.value },
};
   
static const nir_search_variable replace22_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace22_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace22 = {
   { nir_search_value_expression },
   nir_op_fadd,
   { &replace22_0.value, &replace22_1.value },
};
   
static const nir_search_variable search30_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search30_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_variable search30_2 = {
   { nir_search_value_variable },
   2, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression search30 = {
   { nir_search_value_expression },
   nir_op_ffma,
   { &search30_0.value, &search30_1.value, &search30_2.value },
};
   
static const nir_search_variable replace30_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace30_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace30_0 = {
   { nir_search_value_expression },
   nir_op_fmul,
   { &replace30_0_0.value, &replace30_0_1.value },
};

static const nir_search_variable replace30_1 = {
   { nir_search_value_variable },
   2, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace30 = {
   { nir_search_value_expression },
   nir_op_fadd,
   { &replace30_0.value, &replace30_1.value },
};

static const struct transform nir_opt_algebraic_ffma_xforms[] = {
   { &search18, &replace18.value, 0 },
   { &search19, &replace19.value, 0 },
   { &search20, &replace20.value, 0 },
   { &search21, &replace21.value, 0 },
   { &search22, &replace22.value, 0 },
   { &search30, &replace30.value, 3 },
};
   
static const nir_search_variable search49_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search49_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search49 = {
   { nir_search_value_expression },
   nir_op_umin,
   { &search49_0.value, &search49_1.value },
};
   
static const nir_search_variable replace49 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const struct transform nir_opt_algebraic_umin_xforms[] = {
   { &search49, &replace49.value, 0 },
};
   
static const nir_search_variable search50_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search50_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search50 = {
   { nir_search_value_expression },
   nir_op_umax,
   { &search50_0.value, &search50_1.value },
};
   
static const nir_search_variable replace50 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const struct transform nir_opt_algebraic_umax_xforms[] = {
   { &search50, &replace50.value, 0 },
};
   
static const nir_search_variable search41_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search41_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search41_0 = {
   { nir_search_value_expression },
   nir_op_flt,
   { &search41_0_0.value, &search41_0_1.value },
};

static const nir_search_variable search41_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search41_2 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search41 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &search41_0.value, &search41_1.value, &search41_2.value },
};
   
static const nir_search_variable replace41_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace41_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace41 = {
   { nir_search_value_expression },
   nir_op_fmin,
   { &replace41_0.value, &replace41_1.value },
};
   
static const nir_search_variable search42_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search42_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search42_0 = {
   { nir_search_value_expression },
   nir_op_flt,
   { &search42_0_0.value, &search42_0_1.value },
};

static const nir_search_variable search42_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_variable search42_2 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search42 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &search42_0.value, &search42_1.value, &search42_2.value },
};
   
static const nir_search_variable replace42_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace42_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace42 = {
   { nir_search_value_expression },
   nir_op_fmax,
   { &replace42_0.value, &replace42_1.value },
};
   
static const nir_search_variable search43_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_bool,
};
static const nir_search_expression search43_0 = {
   { nir_search_value_expression },
   nir_op_inot,
   { &search43_0_0.value },
};

static const nir_search_variable search43_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_variable search43_2 = {
   { nir_search_value_variable },
   2, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression search43 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &search43_0.value, &search43_1.value, &search43_2.value },
};
   
static const nir_search_variable replace43_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace43_1 = {
   { nir_search_value_variable },
   2, /* c */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace43_2 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace43 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &replace43_0.value, &replace43_1.value, &replace43_2.value },
};
   
static const nir_search_variable search44_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search44_1_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search44_1_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_variable search44_1_2 = {
   { nir_search_value_variable },
   2, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression search44_1 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &search44_1_0.value, &search44_1_1.value, &search44_1_2.value },
};

static const nir_search_variable search44_2 = {
   { nir_search_value_variable },
   3, /* d */
   false,
   nir_type_invalid,
};
static const nir_search_expression search44 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &search44_0.value, &search44_1.value, &search44_2.value },
};
   
static const nir_search_variable replace44_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace44_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace44_2 = {
   { nir_search_value_variable },
   3, /* d */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace44 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &replace44_0.value, &replace44_1.value, &replace44_2.value },
};
   
static const nir_search_variable search116_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_constant search116_1 = {
   { nir_search_value_constant },
   { NIR_TRUE /* True */ },
};

static const nir_search_constant search116_2 = {
   { nir_search_value_constant },
   { NIR_FALSE /* False */ },
};
static const nir_search_expression search116 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &search116_0.value, &search116_1.value, &search116_2.value },
};
   
static const nir_search_variable replace116_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_constant replace116_1 = {
   { nir_search_value_constant },
   { 0x0 /* 0 */ },
};
static const nir_search_expression replace116 = {
   { nir_search_value_expression },
   nir_op_ine,
   { &replace116_0.value, &replace116_1.value },
};
   
static const nir_search_variable search117_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_constant search117_1 = {
   { nir_search_value_constant },
   { NIR_FALSE /* False */ },
};

static const nir_search_constant search117_2 = {
   { nir_search_value_constant },
   { NIR_TRUE /* True */ },
};
static const nir_search_expression search117 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &search117_0.value, &search117_1.value, &search117_2.value },
};
   
static const nir_search_variable replace117_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_constant replace117_1 = {
   { nir_search_value_constant },
   { 0x0 /* 0 */ },
};
static const nir_search_expression replace117 = {
   { nir_search_value_expression },
   nir_op_ieq,
   { &replace117_0.value, &replace117_1.value },
};
   
static const nir_search_constant search118_0 = {
   { nir_search_value_constant },
   { NIR_TRUE /* True */ },
};

static const nir_search_variable search118_1 = {
   { nir_search_value_variable },
   0, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_variable search118_2 = {
   { nir_search_value_variable },
   1, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression search118 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &search118_0.value, &search118_1.value, &search118_2.value },
};
   
static const nir_search_variable replace118 = {
   { nir_search_value_variable },
   0, /* b */
   false,
   nir_type_invalid,
};
   
static const nir_search_constant search119_0 = {
   { nir_search_value_constant },
   { NIR_FALSE /* False */ },
};

static const nir_search_variable search119_1 = {
   { nir_search_value_variable },
   0, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_variable search119_2 = {
   { nir_search_value_variable },
   1, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression search119 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &search119_0.value, &search119_1.value, &search119_2.value },
};
   
static const nir_search_variable replace119 = {
   { nir_search_value_variable },
   1, /* c */
   false,
   nir_type_invalid,
};
   
static const nir_search_variable search120_0 = {
   { nir_search_value_variable },
   0, /* a */
   true,
   nir_type_invalid,
};

static const nir_search_variable search120_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_variable search120_2 = {
   { nir_search_value_variable },
   2, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression search120 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &search120_0.value, &search120_1.value, &search120_2.value },
};
   
static const nir_search_variable replace120_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_constant replace120_0_1 = {
   { nir_search_value_constant },
   { 0x0 /* 0 */ },
};
static const nir_search_expression replace120_0 = {
   { nir_search_value_expression },
   nir_op_ine,
   { &replace120_0_0.value, &replace120_0_1.value },
};

static const nir_search_variable replace120_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace120_2 = {
   { nir_search_value_variable },
   2, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace120 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &replace120_0.value, &replace120_1.value, &replace120_2.value },
};
   
static const nir_search_variable search121_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search121_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_variable search121_2 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search121 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &search121_0.value, &search121_1.value, &search121_2.value },
};
   
static const nir_search_variable replace121 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};

static const struct transform nir_opt_algebraic_bcsel_xforms[] = {
   { &search41, &replace41.value, 0 },
   { &search42, &replace42.value, 0 },
   { &search43, &replace43.value, 0 },
   { &search44, &replace44.value, 0 },
   { &search116, &replace116.value, 0 },
   { &search117, &replace117.value, 0 },
   { &search118, &replace118.value, 0 },
   { &search119, &replace119.value, 0 },
   { &search120, &replace120.value, 0 },
   { &search121, &replace121.value, 0 },
};
   
static const nir_search_variable search60_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search60_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search60 = {
   { nir_search_value_expression },
   nir_op_sge,
   { &search60_0.value, &search60_1.value },
};
   
static const nir_search_variable replace60_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace60_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace60_0 = {
   { nir_search_value_expression },
   nir_op_fge,
   { &replace60_0_0.value, &replace60_0_1.value },
};
static const nir_search_expression replace60 = {
   { nir_search_value_expression },
   nir_op_b2f,
   { &replace60_0.value },
};

static const struct transform nir_opt_algebraic_sge_xforms[] = {
   { &search60, &replace60.value, 7 },
};
   
static const nir_search_variable search99_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search99_0 = {
   { nir_search_value_expression },
   nir_op_fexp2,
   { &search99_0_0.value },
};
static const nir_search_expression search99 = {
   { nir_search_value_expression },
   nir_op_fsqrt,
   { &search99_0.value },
};
   
static const nir_search_constant replace99_0_0 = {
   { nir_search_value_constant },
   { 0x3f000000 /* 0.5 */ },
};

static const nir_search_variable replace99_0_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace99_0 = {
   { nir_search_value_expression },
   nir_op_fmul,
   { &replace99_0_0.value, &replace99_0_1.value },
};
static const nir_search_expression replace99 = {
   { nir_search_value_expression },
   nir_op_fexp2,
   { &replace99_0.value },
};
   
static const nir_search_variable search112_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search112 = {
   { nir_search_value_expression },
   nir_op_fsqrt,
   { &search112_0.value },
};
   
static const nir_search_variable replace112_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace112_0 = {
   { nir_search_value_expression },
   nir_op_frsq,
   { &replace112_0_0.value },
};
static const nir_search_expression replace112 = {
   { nir_search_value_expression },
   nir_op_frcp,
   { &replace112_0.value },
};

static const struct transform nir_opt_algebraic_fsqrt_xforms[] = {
   { &search99, &replace99.value, 0 },
   { &search112, &replace112.value, 10 },
};
   
static const nir_search_variable search7_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_constant search7_1 = {
   { nir_search_value_constant },
   { 0x0 /* 0 */ },
};
static const nir_search_expression search7 = {
   { nir_search_value_expression },
   nir_op_iadd,
   { &search7_0.value, &search7_1.value },
};
   
static const nir_search_variable replace7 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
   
static const nir_search_variable search9_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search9_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search9_0 = {
   { nir_search_value_expression },
   nir_op_imul,
   { &search9_0_0.value, &search9_0_1.value },
};

static const nir_search_variable search9_1_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search9_1_1 = {
   { nir_search_value_variable },
   2, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression search9_1 = {
   { nir_search_value_expression },
   nir_op_imul,
   { &search9_1_0.value, &search9_1_1.value },
};
static const nir_search_expression search9 = {
   { nir_search_value_expression },
   nir_op_iadd,
   { &search9_0.value, &search9_1.value },
};
   
static const nir_search_variable replace9_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace9_1_0 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace9_1_1 = {
   { nir_search_value_variable },
   2, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace9_1 = {
   { nir_search_value_expression },
   nir_op_iadd,
   { &replace9_1_0.value, &replace9_1_1.value },
};
static const nir_search_expression replace9 = {
   { nir_search_value_expression },
   nir_op_imul,
   { &replace9_0.value, &replace9_1.value },
};
   
static const nir_search_variable search11_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search11_0 = {
   { nir_search_value_expression },
   nir_op_ineg,
   { &search11_0_0.value },
};

static const nir_search_variable search11_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search11 = {
   { nir_search_value_expression },
   nir_op_iadd,
   { &search11_0.value, &search11_1.value },
};
   
static const nir_search_constant replace11 = {
   { nir_search_value_constant },
   { 0x0 /* 0 */ },
};
   
static const nir_search_variable search133_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_constant search133_1_0 = {
   { nir_search_value_constant },
   { 0x0 /* 0 */ },
};

static const nir_search_variable search133_1_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search133_1 = {
   { nir_search_value_expression },
   nir_op_isub,
   { &search133_1_0.value, &search133_1_1.value },
};
static const nir_search_expression search133 = {
   { nir_search_value_expression },
   nir_op_iadd,
   { &search133_0.value, &search133_1.value },
};
   
static const nir_search_variable replace133_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace133_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace133 = {
   { nir_search_value_expression },
   nir_op_isub,
   { &replace133_0.value, &replace133_1.value },
};

static const struct transform nir_opt_algebraic_iadd_xforms[] = {
   { &search7, &replace7.value, 0 },
   { &search9, &replace9.value, 0 },
   { &search11, &replace11.value, 0 },
   { &search133, &replace133.value, 0 },
};
   
static const nir_search_variable search75_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_constant search75_1 = {
   { nir_search_value_constant },
   { 0x0 /* 0.0 */ },
};
static const nir_search_expression search75 = {
   { nir_search_value_expression },
   nir_op_fand,
   { &search75_0.value, &search75_1.value },
};
   
static const nir_search_constant replace75 = {
   { nir_search_value_constant },
   { 0x0 /* 0.0 */ },
};

static const struct transform nir_opt_algebraic_fand_xforms[] = {
   { &search75, &replace75.value, 0 },
};
   
static const nir_search_variable search2_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search2_0 = {
   { nir_search_value_expression },
   nir_op_fabs,
   { &search2_0_0.value },
};
static const nir_search_expression search2 = {
   { nir_search_value_expression },
   nir_op_fabs,
   { &search2_0.value },
};
   
static const nir_search_variable replace2_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace2 = {
   { nir_search_value_expression },
   nir_op_fabs,
   { &replace2_0.value },
};
   
static const nir_search_variable search3_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search3_0 = {
   { nir_search_value_expression },
   nir_op_fneg,
   { &search3_0_0.value },
};
static const nir_search_expression search3 = {
   { nir_search_value_expression },
   nir_op_fabs,
   { &search3_0.value },
};
   
static const nir_search_variable replace3_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace3 = {
   { nir_search_value_expression },
   nir_op_fabs,
   { &replace3_0.value },
};
   
static const nir_search_constant search134_0_0 = {
   { nir_search_value_constant },
   { 0x0 /* 0.0 */ },
};

static const nir_search_variable search134_0_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search134_0 = {
   { nir_search_value_expression },
   nir_op_fsub,
   { &search134_0_0.value, &search134_0_1.value },
};
static const nir_search_expression search134 = {
   { nir_search_value_expression },
   nir_op_fabs,
   { &search134_0.value },
};
   
static const nir_search_variable replace134_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace134 = {
   { nir_search_value_expression },
   nir_op_fabs,
   { &replace134_0.value },
};

static const struct transform nir_opt_algebraic_fabs_xforms[] = {
   { &search2, &replace2.value, 0 },
   { &search3, &replace3.value, 0 },
   { &search134, &replace134.value, 0 },
};
   
static const nir_search_variable search71_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search71_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search71 = {
   { nir_search_value_expression },
   nir_op_ieq,
   { &search71_0.value, &search71_1.value },
};
   
static const nir_search_constant replace71 = {
   { nir_search_value_constant },
   { NIR_TRUE /* True */ },
};
   
static const nir_search_variable search115_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_bool,
};

static const nir_search_constant search115_1 = {
   { nir_search_value_constant },
   { 0x0 /* 0 */ },
};
static const nir_search_expression search115 = {
   { nir_search_value_expression },
   nir_op_ieq,
   { &search115_0.value, &search115_1.value },
};
   
static const nir_search_variable replace115_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace115 = {
   { nir_search_value_expression },
   nir_op_inot,
   { &replace115_0.value },
};
   
static const nir_search_variable search148_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search148_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   true,
   nir_type_invalid,
};

static const nir_search_variable search148_0_2 = {
   { nir_search_value_variable },
   2, /* c */
   true,
   nir_type_invalid,
};
static const nir_search_expression search148_0 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &search148_0_0.value, &search148_0_1.value, &search148_0_2.value },
};

static const nir_search_variable search148_1 = {
   { nir_search_value_variable },
   3, /* d */
   true,
   nir_type_invalid,
};
static const nir_search_expression search148 = {
   { nir_search_value_expression },
   nir_op_ieq,
   { &search148_0.value, &search148_1.value },
};
   
static const nir_search_variable replace148_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace148_1_0 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace148_1_1 = {
   { nir_search_value_variable },
   3, /* d */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace148_1 = {
   { nir_search_value_expression },
   nir_op_ieq,
   { &replace148_1_0.value, &replace148_1_1.value },
};

static const nir_search_variable replace148_2_0 = {
   { nir_search_value_variable },
   2, /* c */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace148_2_1 = {
   { nir_search_value_variable },
   3, /* d */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace148_2 = {
   { nir_search_value_expression },
   nir_op_ieq,
   { &replace148_2_0.value, &replace148_2_1.value },
};
static const nir_search_expression replace148 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &replace148_0.value, &replace148_1.value, &replace148_2.value },
};
   
static const nir_search_variable search149_0 = {
   { nir_search_value_variable },
   0, /* d */
   true,
   nir_type_invalid,
};

static const nir_search_variable search149_1_0 = {
   { nir_search_value_variable },
   1, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search149_1_1 = {
   { nir_search_value_variable },
   2, /* b */
   true,
   nir_type_invalid,
};

static const nir_search_variable search149_1_2 = {
   { nir_search_value_variable },
   3, /* c */
   true,
   nir_type_invalid,
};
static const nir_search_expression search149_1 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &search149_1_0.value, &search149_1_1.value, &search149_1_2.value },
};
static const nir_search_expression search149 = {
   { nir_search_value_expression },
   nir_op_ieq,
   { &search149_0.value, &search149_1.value },
};
   
static const nir_search_variable replace149_0 = {
   { nir_search_value_variable },
   1, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace149_1_0 = {
   { nir_search_value_variable },
   0, /* d */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace149_1_1 = {
   { nir_search_value_variable },
   2, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace149_1 = {
   { nir_search_value_expression },
   nir_op_ieq,
   { &replace149_1_0.value, &replace149_1_1.value },
};

static const nir_search_variable replace149_2_0 = {
   { nir_search_value_variable },
   0, /* d */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace149_2_1 = {
   { nir_search_value_variable },
   3, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace149_2 = {
   { nir_search_value_expression },
   nir_op_ieq,
   { &replace149_2_0.value, &replace149_2_1.value },
};
static const nir_search_expression replace149 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &replace149_0.value, &replace149_1.value, &replace149_2.value },
};

static const struct transform nir_opt_algebraic_ieq_xforms[] = {
   { &search71, &replace71.value, 0 },
   { &search115, &replace115.value, 0 },
   { &search148, &replace148.value, 0 },
   { &search149, &replace149.value, 0 },
};
   
static const nir_search_variable search47_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search47_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search47 = {
   { nir_search_value_expression },
   nir_op_imin,
   { &search47_0.value, &search47_1.value },
};
   
static const nir_search_variable replace47 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const struct transform nir_opt_algebraic_imin_xforms[] = {
   { &search47, &replace47.value, 0 },
};
   
static const nir_search_variable search101_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search101_0 = {
   { nir_search_value_expression },
   nir_op_fexp2,
   { &search101_0_0.value },
};
static const nir_search_expression search101 = {
   { nir_search_value_expression },
   nir_op_frsq,
   { &search101_0.value },
};
   
static const nir_search_constant replace101_0_0 = {
   { nir_search_value_constant },
   { 0xbf000000 /* -0.5 */ },
};

static const nir_search_variable replace101_0_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace101_0 = {
   { nir_search_value_expression },
   nir_op_fmul,
   { &replace101_0_0.value, &replace101_0_1.value },
};
static const nir_search_expression replace101 = {
   { nir_search_value_expression },
   nir_op_fexp2,
   { &replace101_0.value },
};

static const struct transform nir_opt_algebraic_frsq_xforms[] = {
   { &search101, &replace101.value, 0 },
};
   
static const nir_search_variable search1_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search1_0 = {
   { nir_search_value_expression },
   nir_op_ineg,
   { &search1_0_0.value },
};
static const nir_search_expression search1 = {
   { nir_search_value_expression },
   nir_op_ineg,
   { &search1_0.value },
};
   
static const nir_search_variable replace1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
   
static const nir_search_variable search131_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search131 = {
   { nir_search_value_expression },
   nir_op_ineg,
   { &search131_0.value },
};
   
static const nir_search_constant replace131_0 = {
   { nir_search_value_constant },
   { 0x0 /* 0 */ },
};

static const nir_search_variable replace131_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace131 = {
   { nir_search_value_expression },
   nir_op_isub,
   { &replace131_0.value, &replace131_1.value },
};

static const struct transform nir_opt_algebraic_ineg_xforms[] = {
   { &search1, &replace1.value, 0 },
   { &search131, &replace131.value, 13 },
};
   
static const nir_search_variable search93_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search93_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search93 = {
   { nir_search_value_expression },
   nir_op_fpow,
   { &search93_0.value, &search93_1.value },
};
   
static const nir_search_variable replace93_0_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace93_0_0 = {
   { nir_search_value_expression },
   nir_op_flog2,
   { &replace93_0_0_0.value },
};

static const nir_search_variable replace93_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace93_0 = {
   { nir_search_value_expression },
   nir_op_fmul,
   { &replace93_0_0.value, &replace93_0_1.value },
};
static const nir_search_expression replace93 = {
   { nir_search_value_expression },
   nir_op_fexp2,
   { &replace93_0.value },
};
   
static const nir_search_variable search95_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_constant search95_1 = {
   { nir_search_value_constant },
   { 0x3f800000 /* 1.0 */ },
};
static const nir_search_expression search95 = {
   { nir_search_value_expression },
   nir_op_fpow,
   { &search95_0.value, &search95_1.value },
};
   
static const nir_search_variable replace95 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
   
static const nir_search_variable search96_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_constant search96_1 = {
   { nir_search_value_constant },
   { 0x40000000 /* 2.0 */ },
};
static const nir_search_expression search96 = {
   { nir_search_value_expression },
   nir_op_fpow,
   { &search96_0.value, &search96_1.value },
};
   
static const nir_search_variable replace96_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace96_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace96 = {
   { nir_search_value_expression },
   nir_op_fmul,
   { &replace96_0.value, &replace96_1.value },
};
   
static const nir_search_variable search97_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_constant search97_1 = {
   { nir_search_value_constant },
   { 0x40800000 /* 4.0 */ },
};
static const nir_search_expression search97 = {
   { nir_search_value_expression },
   nir_op_fpow,
   { &search97_0.value, &search97_1.value },
};
   
static const nir_search_variable replace97_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace97_0_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace97_0 = {
   { nir_search_value_expression },
   nir_op_fmul,
   { &replace97_0_0.value, &replace97_0_1.value },
};

static const nir_search_variable replace97_1_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace97_1_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace97_1 = {
   { nir_search_value_expression },
   nir_op_fmul,
   { &replace97_1_0.value, &replace97_1_1.value },
};
static const nir_search_expression replace97 = {
   { nir_search_value_expression },
   nir_op_fmul,
   { &replace97_0.value, &replace97_1.value },
};
   
static const nir_search_constant search98_0 = {
   { nir_search_value_constant },
   { 0x40000000 /* 2.0 */ },
};

static const nir_search_variable search98_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search98 = {
   { nir_search_value_expression },
   nir_op_fpow,
   { &search98_0.value, &search98_1.value },
};
   
static const nir_search_variable replace98_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace98 = {
   { nir_search_value_expression },
   nir_op_fexp2,
   { &replace98_0.value },
};

static const struct transform nir_opt_algebraic_fpow_xforms[] = {
   { &search93, &replace93.value, 8 },
   { &search95, &replace95.value, 0 },
   { &search96, &replace96.value, 0 },
   { &search97, &replace97.value, 0 },
   { &search98, &replace98.value, 0 },
};
   
static const nir_search_variable search70_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search70_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search70 = {
   { nir_search_value_expression },
   nir_op_ige,
   { &search70_0.value, &search70_1.value },
};
   
static const nir_search_constant replace70 = {
   { nir_search_value_constant },
   { NIR_TRUE /* True */ },
};
   
static const nir_search_variable search146_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search146_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   true,
   nir_type_invalid,
};

static const nir_search_variable search146_0_2 = {
   { nir_search_value_variable },
   2, /* c */
   true,
   nir_type_invalid,
};
static const nir_search_expression search146_0 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &search146_0_0.value, &search146_0_1.value, &search146_0_2.value },
};

static const nir_search_variable search146_1 = {
   { nir_search_value_variable },
   3, /* d */
   true,
   nir_type_invalid,
};
static const nir_search_expression search146 = {
   { nir_search_value_expression },
   nir_op_ige,
   { &search146_0.value, &search146_1.value },
};
   
static const nir_search_variable replace146_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace146_1_0 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace146_1_1 = {
   { nir_search_value_variable },
   3, /* d */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace146_1 = {
   { nir_search_value_expression },
   nir_op_ige,
   { &replace146_1_0.value, &replace146_1_1.value },
};

static const nir_search_variable replace146_2_0 = {
   { nir_search_value_variable },
   2, /* c */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace146_2_1 = {
   { nir_search_value_variable },
   3, /* d */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace146_2 = {
   { nir_search_value_expression },
   nir_op_ige,
   { &replace146_2_0.value, &replace146_2_1.value },
};
static const nir_search_expression replace146 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &replace146_0.value, &replace146_1.value, &replace146_2.value },
};
   
static const nir_search_variable search147_0 = {
   { nir_search_value_variable },
   0, /* d */
   true,
   nir_type_invalid,
};

static const nir_search_variable search147_1_0 = {
   { nir_search_value_variable },
   1, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search147_1_1 = {
   { nir_search_value_variable },
   2, /* b */
   true,
   nir_type_invalid,
};

static const nir_search_variable search147_1_2 = {
   { nir_search_value_variable },
   3, /* c */
   true,
   nir_type_invalid,
};
static const nir_search_expression search147_1 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &search147_1_0.value, &search147_1_1.value, &search147_1_2.value },
};
static const nir_search_expression search147 = {
   { nir_search_value_expression },
   nir_op_ige,
   { &search147_0.value, &search147_1.value },
};
   
static const nir_search_variable replace147_0 = {
   { nir_search_value_variable },
   1, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace147_1_0 = {
   { nir_search_value_variable },
   0, /* d */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace147_1_1 = {
   { nir_search_value_variable },
   2, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace147_1 = {
   { nir_search_value_expression },
   nir_op_ige,
   { &replace147_1_0.value, &replace147_1_1.value },
};

static const nir_search_variable replace147_2_0 = {
   { nir_search_value_variable },
   0, /* d */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace147_2_1 = {
   { nir_search_value_variable },
   3, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace147_2 = {
   { nir_search_value_expression },
   nir_op_ige,
   { &replace147_2_0.value, &replace147_2_1.value },
};
static const nir_search_expression replace147 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &replace147_0.value, &replace147_1.value, &replace147_2.value },
};

static const struct transform nir_opt_algebraic_ige_xforms[] = {
   { &search70, &replace70.value, 0 },
   { &search146, &replace146.value, 0 },
   { &search147, &replace147.value, 0 },
};
   
static const nir_search_constant search109_0 = {
   { nir_search_value_constant },
   { 0x3f800000 /* 1.0 */ },
};

static const nir_search_variable search109_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search109 = {
   { nir_search_value_expression },
   nir_op_fdiv,
   { &search109_0.value, &search109_1.value },
};
   
static const nir_search_variable replace109_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace109 = {
   { nir_search_value_expression },
   nir_op_frcp,
   { &replace109_0.value },
};

static const struct transform nir_opt_algebraic_fdiv_xforms[] = {
   { &search109, &replace109.value, 0 },
};
   
static const nir_search_variable search6_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_constant search6_1 = {
   { nir_search_value_constant },
   { 0x0 /* 0.0 */ },
};
static const nir_search_expression search6 = {
   { nir_search_value_expression },
   nir_op_fadd,
   { &search6_0.value, &search6_1.value },
};
   
static const nir_search_variable replace6 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
   
static const nir_search_variable search8_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search8_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search8_0 = {
   { nir_search_value_expression },
   nir_op_fmul,
   { &search8_0_0.value, &search8_0_1.value },
};

static const nir_search_variable search8_1_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search8_1_1 = {
   { nir_search_value_variable },
   2, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression search8_1 = {
   { nir_search_value_expression },
   nir_op_fmul,
   { &search8_1_0.value, &search8_1_1.value },
};
static const nir_search_expression search8 = {
   { nir_search_value_expression },
   nir_op_fadd,
   { &search8_0.value, &search8_1.value },
};
   
static const nir_search_variable replace8_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace8_1_0 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace8_1_1 = {
   { nir_search_value_variable },
   2, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace8_1 = {
   { nir_search_value_expression },
   nir_op_fadd,
   { &replace8_1_0.value, &replace8_1_1.value },
};
static const nir_search_expression replace8 = {
   { nir_search_value_expression },
   nir_op_fmul,
   { &replace8_0.value, &replace8_1.value },
};
   
static const nir_search_variable search10_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search10_0 = {
   { nir_search_value_expression },
   nir_op_fneg,
   { &search10_0_0.value },
};

static const nir_search_variable search10_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search10 = {
   { nir_search_value_expression },
   nir_op_fadd,
   { &search10_0.value, &search10_1.value },
};
   
static const nir_search_constant replace10 = {
   { nir_search_value_constant },
   { 0x0 /* 0.0 */ },
};
   
static const nir_search_variable search28_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_constant search28_0_1_0 = {
   { nir_search_value_constant },
   { 0x3f800000 /* 1.0 */ },
};

static const nir_search_variable search28_0_1_1_0 = {
   { nir_search_value_variable },
   1, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression search28_0_1_1 = {
   { nir_search_value_expression },
   nir_op_fneg,
   { &search28_0_1_1_0.value },
};
static const nir_search_expression search28_0_1 = {
   { nir_search_value_expression },
   nir_op_fadd,
   { &search28_0_1_0.value, &search28_0_1_1.value },
};
static const nir_search_expression search28_0 = {
   { nir_search_value_expression },
   nir_op_fmul,
   { &search28_0_0.value, &search28_0_1.value },
};

static const nir_search_variable search28_1_0 = {
   { nir_search_value_variable },
   2, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_variable search28_1_1 = {
   { nir_search_value_variable },
   1, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression search28_1 = {
   { nir_search_value_expression },
   nir_op_fmul,
   { &search28_1_0.value, &search28_1_1.value },
};
static const nir_search_expression search28 = {
   { nir_search_value_expression },
   nir_op_fadd,
   { &search28_0.value, &search28_1.value },
};
   
static const nir_search_variable replace28_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace28_1 = {
   { nir_search_value_variable },
   2, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace28_2 = {
   { nir_search_value_variable },
   1, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace28 = {
   { nir_search_value_expression },
   nir_op_flrp,
   { &replace28_0.value, &replace28_1.value, &replace28_2.value },
};
   
static const nir_search_variable search29_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search29_1_0 = {
   { nir_search_value_variable },
   1, /* c */
   false,
   nir_type_invalid,
};

static const nir_search_variable search29_1_1_0 = {
   { nir_search_value_variable },
   2, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_variable search29_1_1_1_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search29_1_1_1 = {
   { nir_search_value_expression },
   nir_op_fneg,
   { &search29_1_1_1_0.value },
};
static const nir_search_expression search29_1_1 = {
   { nir_search_value_expression },
   nir_op_fadd,
   { &search29_1_1_0.value, &search29_1_1_1.value },
};
static const nir_search_expression search29_1 = {
   { nir_search_value_expression },
   nir_op_fmul,
   { &search29_1_0.value, &search29_1_1.value },
};
static const nir_search_expression search29 = {
   { nir_search_value_expression },
   nir_op_fadd,
   { &search29_0.value, &search29_1.value },
};
   
static const nir_search_variable replace29_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace29_1 = {
   { nir_search_value_variable },
   2, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace29_2 = {
   { nir_search_value_variable },
   1, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace29 = {
   { nir_search_value_expression },
   nir_op_flrp,
   { &replace29_0.value, &replace29_1.value, &replace29_2.value },
};
   
static const nir_search_variable search31_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search31_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search31_0 = {
   { nir_search_value_expression },
   nir_op_fmul,
   { &search31_0_0.value, &search31_0_1.value },
};

static const nir_search_variable search31_1 = {
   { nir_search_value_variable },
   2, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression search31 = {
   { nir_search_value_expression },
   nir_op_fadd,
   { &search31_0.value, &search31_1.value },
};
   
static const nir_search_variable replace31_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace31_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace31_2 = {
   { nir_search_value_variable },
   2, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace31 = {
   { nir_search_value_expression },
   nir_op_ffma,
   { &replace31_0.value, &replace31_1.value, &replace31_2.value },
};
   
static const nir_search_variable search106_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search106_0 = {
   { nir_search_value_expression },
   nir_op_flog2,
   { &search106_0_0.value },
};

static const nir_search_variable search106_1_0 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search106_1 = {
   { nir_search_value_expression },
   nir_op_flog2,
   { &search106_1_0.value },
};
static const nir_search_expression search106 = {
   { nir_search_value_expression },
   nir_op_fadd,
   { &search106_0.value, &search106_1.value },
};
   
static const nir_search_variable replace106_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace106_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace106_0 = {
   { nir_search_value_expression },
   nir_op_fmul,
   { &replace106_0_0.value, &replace106_0_1.value },
};
static const nir_search_expression replace106 = {
   { nir_search_value_expression },
   nir_op_flog2,
   { &replace106_0.value },
};
   
static const nir_search_variable search107_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search107_0 = {
   { nir_search_value_expression },
   nir_op_flog2,
   { &search107_0_0.value },
};

static const nir_search_variable search107_1_0_0 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search107_1_0 = {
   { nir_search_value_expression },
   nir_op_flog2,
   { &search107_1_0_0.value },
};
static const nir_search_expression search107_1 = {
   { nir_search_value_expression },
   nir_op_fneg,
   { &search107_1_0.value },
};
static const nir_search_expression search107 = {
   { nir_search_value_expression },
   nir_op_fadd,
   { &search107_0.value, &search107_1.value },
};
   
static const nir_search_variable replace107_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace107_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace107_0 = {
   { nir_search_value_expression },
   nir_op_fdiv,
   { &replace107_0_0.value, &replace107_0_1.value },
};
static const nir_search_expression replace107 = {
   { nir_search_value_expression },
   nir_op_flog2,
   { &replace107_0.value },
};
   
static const nir_search_variable search132_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_constant search132_1_0 = {
   { nir_search_value_constant },
   { 0x0 /* 0.0 */ },
};

static const nir_search_variable search132_1_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search132_1 = {
   { nir_search_value_expression },
   nir_op_fsub,
   { &search132_1_0.value, &search132_1_1.value },
};
static const nir_search_expression search132 = {
   { nir_search_value_expression },
   nir_op_fadd,
   { &search132_0.value, &search132_1.value },
};
   
static const nir_search_variable replace132_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace132_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace132 = {
   { nir_search_value_expression },
   nir_op_fsub,
   { &replace132_0.value, &replace132_1.value },
};

static const struct transform nir_opt_algebraic_fadd_xforms[] = {
   { &search6, &replace6.value, 0 },
   { &search8, &replace8.value, 0 },
   { &search10, &replace10.value, 0 },
   { &search28, &replace28.value, 2 },
   { &search29, &replace29.value, 2 },
   { &search31, &replace31.value, 4 },
   { &search106, &replace106.value, 0 },
   { &search107, &replace107.value, 0 },
   { &search132, &replace132.value, 0 },
};
   
static const nir_search_constant search85_0 = {
   { nir_search_value_constant },
   { 0x0 /* 0 */ },
};

static const nir_search_variable search85_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search85 = {
   { nir_search_value_expression },
   nir_op_ishl,
   { &search85_0.value, &search85_1.value },
};
   
static const nir_search_constant replace85 = {
   { nir_search_value_constant },
   { 0x0 /* 0 */ },
};
   
static const nir_search_variable search86_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_constant search86_1 = {
   { nir_search_value_constant },
   { 0x0 /* 0 */ },
};
static const nir_search_expression search86 = {
   { nir_search_value_expression },
   nir_op_ishl,
   { &search86_0.value, &search86_1.value },
};
   
static const nir_search_variable replace86 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const struct transform nir_opt_algebraic_ishl_xforms[] = {
   { &search85, &replace85.value, 0 },
   { &search86, &replace86.value, 0 },
};
   
static const nir_search_variable search92_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search92_0 = {
   { nir_search_value_expression },
   nir_op_fexp2,
   { &search92_0_0.value },
};
static const nir_search_expression search92 = {
   { nir_search_value_expression },
   nir_op_flog2,
   { &search92_0.value },
};
   
static const nir_search_variable replace92 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
   
static const nir_search_variable search102_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search102_0 = {
   { nir_search_value_expression },
   nir_op_fsqrt,
   { &search102_0_0.value },
};
static const nir_search_expression search102 = {
   { nir_search_value_expression },
   nir_op_flog2,
   { &search102_0.value },
};
   
static const nir_search_constant replace102_0 = {
   { nir_search_value_constant },
   { 0x3f000000 /* 0.5 */ },
};

static const nir_search_variable replace102_1_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace102_1 = {
   { nir_search_value_expression },
   nir_op_flog2,
   { &replace102_1_0.value },
};
static const nir_search_expression replace102 = {
   { nir_search_value_expression },
   nir_op_fmul,
   { &replace102_0.value, &replace102_1.value },
};
   
static const nir_search_variable search103_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search103_0 = {
   { nir_search_value_expression },
   nir_op_frcp,
   { &search103_0_0.value },
};
static const nir_search_expression search103 = {
   { nir_search_value_expression },
   nir_op_flog2,
   { &search103_0.value },
};
   
static const nir_search_variable replace103_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace103_0 = {
   { nir_search_value_expression },
   nir_op_flog2,
   { &replace103_0_0.value },
};
static const nir_search_expression replace103 = {
   { nir_search_value_expression },
   nir_op_fneg,
   { &replace103_0.value },
};
   
static const nir_search_variable search104_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search104_0 = {
   { nir_search_value_expression },
   nir_op_frsq,
   { &search104_0_0.value },
};
static const nir_search_expression search104 = {
   { nir_search_value_expression },
   nir_op_flog2,
   { &search104_0.value },
};
   
static const nir_search_constant replace104_0 = {
   { nir_search_value_constant },
   { 0xbf000000 /* -0.5 */ },
};

static const nir_search_variable replace104_1_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace104_1 = {
   { nir_search_value_expression },
   nir_op_flog2,
   { &replace104_1_0.value },
};
static const nir_search_expression replace104 = {
   { nir_search_value_expression },
   nir_op_fmul,
   { &replace104_0.value, &replace104_1.value },
};
   
static const nir_search_variable search105_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search105_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search105_0 = {
   { nir_search_value_expression },
   nir_op_fpow,
   { &search105_0_0.value, &search105_0_1.value },
};
static const nir_search_expression search105 = {
   { nir_search_value_expression },
   nir_op_flog2,
   { &search105_0.value },
};
   
static const nir_search_variable replace105_0 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace105_1_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace105_1 = {
   { nir_search_value_expression },
   nir_op_flog2,
   { &replace105_1_0.value },
};
static const nir_search_expression replace105 = {
   { nir_search_value_expression },
   nir_op_fmul,
   { &replace105_0.value, &replace105_1.value },
};

static const struct transform nir_opt_algebraic_flog2_xforms[] = {
   { &search92, &replace92.value, 0 },
   { &search102, &replace102.value, 0 },
   { &search103, &replace103.value, 0 },
   { &search104, &replace104.value, 0 },
   { &search105, &replace105.value, 0 },
};
   
static const nir_search_variable search32_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search32_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search32_0 = {
   { nir_search_value_expression },
   nir_op_flt,
   { &search32_0_0.value, &search32_0_1.value },
};
static const nir_search_expression search32 = {
   { nir_search_value_expression },
   nir_op_inot,
   { &search32_0.value },
};
   
static const nir_search_variable replace32_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace32_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace32 = {
   { nir_search_value_expression },
   nir_op_fge,
   { &replace32_0.value, &replace32_1.value },
};
   
static const nir_search_variable search33_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search33_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search33_0 = {
   { nir_search_value_expression },
   nir_op_fge,
   { &search33_0_0.value, &search33_0_1.value },
};
static const nir_search_expression search33 = {
   { nir_search_value_expression },
   nir_op_inot,
   { &search33_0.value },
};
   
static const nir_search_variable replace33_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace33_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace33 = {
   { nir_search_value_expression },
   nir_op_flt,
   { &replace33_0.value, &replace33_1.value },
};
   
static const nir_search_variable search34_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search34_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search34_0 = {
   { nir_search_value_expression },
   nir_op_feq,
   { &search34_0_0.value, &search34_0_1.value },
};
static const nir_search_expression search34 = {
   { nir_search_value_expression },
   nir_op_inot,
   { &search34_0.value },
};
   
static const nir_search_variable replace34_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace34_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace34 = {
   { nir_search_value_expression },
   nir_op_fne,
   { &replace34_0.value, &replace34_1.value },
};
   
static const nir_search_variable search35_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search35_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search35_0 = {
   { nir_search_value_expression },
   nir_op_fne,
   { &search35_0_0.value, &search35_0_1.value },
};
static const nir_search_expression search35 = {
   { nir_search_value_expression },
   nir_op_inot,
   { &search35_0.value },
};
   
static const nir_search_variable replace35_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace35_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace35 = {
   { nir_search_value_expression },
   nir_op_feq,
   { &replace35_0.value, &replace35_1.value },
};
   
static const nir_search_variable search36_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search36_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search36_0 = {
   { nir_search_value_expression },
   nir_op_ilt,
   { &search36_0_0.value, &search36_0_1.value },
};
static const nir_search_expression search36 = {
   { nir_search_value_expression },
   nir_op_inot,
   { &search36_0.value },
};
   
static const nir_search_variable replace36_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace36_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace36 = {
   { nir_search_value_expression },
   nir_op_ige,
   { &replace36_0.value, &replace36_1.value },
};
   
static const nir_search_variable search37_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search37_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search37_0 = {
   { nir_search_value_expression },
   nir_op_ige,
   { &search37_0_0.value, &search37_0_1.value },
};
static const nir_search_expression search37 = {
   { nir_search_value_expression },
   nir_op_inot,
   { &search37_0.value },
};
   
static const nir_search_variable replace37_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace37_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace37 = {
   { nir_search_value_expression },
   nir_op_ilt,
   { &replace37_0.value, &replace37_1.value },
};
   
static const nir_search_variable search38_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search38_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search38_0 = {
   { nir_search_value_expression },
   nir_op_ieq,
   { &search38_0_0.value, &search38_0_1.value },
};
static const nir_search_expression search38 = {
   { nir_search_value_expression },
   nir_op_inot,
   { &search38_0.value },
};
   
static const nir_search_variable replace38_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace38_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace38 = {
   { nir_search_value_expression },
   nir_op_ine,
   { &replace38_0.value, &replace38_1.value },
};
   
static const nir_search_variable search39_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search39_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search39_0 = {
   { nir_search_value_expression },
   nir_op_ine,
   { &search39_0_0.value, &search39_0_1.value },
};
static const nir_search_expression search39 = {
   { nir_search_value_expression },
   nir_op_inot,
   { &search39_0.value },
};
   
static const nir_search_variable replace39_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace39_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace39 = {
   { nir_search_value_expression },
   nir_op_ieq,
   { &replace39_0.value, &replace39_1.value },
};
   
static const nir_search_variable search82_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search82_0 = {
   { nir_search_value_expression },
   nir_op_inot,
   { &search82_0_0.value },
};
static const nir_search_expression search82 = {
   { nir_search_value_expression },
   nir_op_inot,
   { &search82_0.value },
};
   
static const nir_search_variable replace82 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const struct transform nir_opt_algebraic_inot_xforms[] = {
   { &search32, &replace32.value, 0 },
   { &search33, &replace33.value, 0 },
   { &search34, &replace34.value, 0 },
   { &search35, &replace35.value, 0 },
   { &search36, &replace36.value, 0 },
   { &search37, &replace37.value, 0 },
   { &search38, &replace38.value, 0 },
   { &search39, &replace39.value, 0 },
   { &search82, &replace82.value, 0 },
};
   
static const nir_search_variable search62_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search62_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search62 = {
   { nir_search_value_expression },
   nir_op_sne,
   { &search62_0.value, &search62_1.value },
};
   
static const nir_search_variable replace62_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace62_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace62_0 = {
   { nir_search_value_expression },
   nir_op_fne,
   { &replace62_0_0.value, &replace62_0_1.value },
};
static const nir_search_expression replace62 = {
   { nir_search_value_expression },
   nir_op_b2f,
   { &replace62_0.value },
};

static const struct transform nir_opt_algebraic_sne_xforms[] = {
   { &search62, &replace62.value, 7 },
};
   
static const nir_search_variable search125_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search125_0 = {
   { nir_search_value_expression },
   nir_op_ftrunc,
   { &search125_0_0.value },
};
static const nir_search_expression search125 = {
   { nir_search_value_expression },
   nir_op_f2u,
   { &search125_0.value },
};
   
static const nir_search_variable replace125_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace125 = {
   { nir_search_value_expression },
   nir_op_f2u,
   { &replace125_0.value },
};

static const struct transform nir_opt_algebraic_f2u_xforms[] = {
   { &search125, &replace125.value, 0 },
};
   
static const nir_search_variable search122_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search122_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_variable search122_2 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search122 = {
   { nir_search_value_expression },
   nir_op_fcsel,
   { &search122_0.value, &search122_1.value, &search122_2.value },
};
   
static const nir_search_variable replace122 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};

static const struct transform nir_opt_algebraic_fcsel_xforms[] = {
   { &search122, &replace122.value, 0 },
};
   
static const nir_search_variable search127_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_constant search127_1_0 = {
   { nir_search_value_constant },
   { 0x0 /* 0 */ },
};

static const nir_search_variable search127_1_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search127_1 = {
   { nir_search_value_expression },
   nir_op_isub,
   { &search127_1_0.value, &search127_1_1.value },
};
static const nir_search_expression search127 = {
   { nir_search_value_expression },
   nir_op_isub,
   { &search127_0.value, &search127_1.value },
};
   
static const nir_search_variable replace127_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace127_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace127 = {
   { nir_search_value_expression },
   nir_op_iadd,
   { &replace127_0.value, &replace127_1.value },
};
   
static const nir_search_variable search129_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search129_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search129 = {
   { nir_search_value_expression },
   nir_op_isub,
   { &search129_0.value, &search129_1.value },
};
   
static const nir_search_variable replace129_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace129_1_0 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace129_1 = {
   { nir_search_value_expression },
   nir_op_ineg,
   { &replace129_1_0.value },
};
static const nir_search_expression replace129 = {
   { nir_search_value_expression },
   nir_op_iadd,
   { &replace129_0.value, &replace129_1.value },
};

static const struct transform nir_opt_algebraic_isub_xforms[] = {
   { &search127, &replace127.value, 0 },
   { &search129, &replace129.value, 12 },
};
   
static const nir_search_variable search46_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search46_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search46 = {
   { nir_search_value_expression },
   nir_op_fmax,
   { &search46_0.value, &search46_1.value },
};
   
static const nir_search_variable replace46 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const struct transform nir_opt_algebraic_fmax_xforms[] = {
   { &search46, &replace46.value, 0 },
};
   
static const nir_search_variable search140_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search140_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   true,
   nir_type_invalid,
};

static const nir_search_variable search140_0_2 = {
   { nir_search_value_variable },
   2, /* c */
   true,
   nir_type_invalid,
};
static const nir_search_expression search140_0 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &search140_0_0.value, &search140_0_1.value, &search140_0_2.value },
};

static const nir_search_variable search140_1 = {
   { nir_search_value_variable },
   3, /* d */
   true,
   nir_type_invalid,
};
static const nir_search_expression search140 = {
   { nir_search_value_expression },
   nir_op_feq,
   { &search140_0.value, &search140_1.value },
};
   
static const nir_search_variable replace140_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace140_1_0 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace140_1_1 = {
   { nir_search_value_variable },
   3, /* d */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace140_1 = {
   { nir_search_value_expression },
   nir_op_feq,
   { &replace140_1_0.value, &replace140_1_1.value },
};

static const nir_search_variable replace140_2_0 = {
   { nir_search_value_variable },
   2, /* c */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace140_2_1 = {
   { nir_search_value_variable },
   3, /* d */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace140_2 = {
   { nir_search_value_expression },
   nir_op_feq,
   { &replace140_2_0.value, &replace140_2_1.value },
};
static const nir_search_expression replace140 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &replace140_0.value, &replace140_1.value, &replace140_2.value },
};
   
static const nir_search_variable search141_0 = {
   { nir_search_value_variable },
   0, /* d */
   true,
   nir_type_invalid,
};

static const nir_search_variable search141_1_0 = {
   { nir_search_value_variable },
   1, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search141_1_1 = {
   { nir_search_value_variable },
   2, /* b */
   true,
   nir_type_invalid,
};

static const nir_search_variable search141_1_2 = {
   { nir_search_value_variable },
   3, /* c */
   true,
   nir_type_invalid,
};
static const nir_search_expression search141_1 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &search141_1_0.value, &search141_1_1.value, &search141_1_2.value },
};
static const nir_search_expression search141 = {
   { nir_search_value_expression },
   nir_op_feq,
   { &search141_0.value, &search141_1.value },
};
   
static const nir_search_variable replace141_0 = {
   { nir_search_value_variable },
   1, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace141_1_0 = {
   { nir_search_value_variable },
   0, /* d */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace141_1_1 = {
   { nir_search_value_variable },
   2, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace141_1 = {
   { nir_search_value_expression },
   nir_op_feq,
   { &replace141_1_0.value, &replace141_1_1.value },
};

static const nir_search_variable replace141_2_0 = {
   { nir_search_value_variable },
   0, /* d */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace141_2_1 = {
   { nir_search_value_variable },
   3, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace141_2 = {
   { nir_search_value_expression },
   nir_op_feq,
   { &replace141_2_0.value, &replace141_2_1.value },
};
static const nir_search_expression replace141 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &replace141_0.value, &replace141_1.value, &replace141_2.value },
};

static const struct transform nir_opt_algebraic_feq_xforms[] = {
   { &search140, &replace140.value, 0 },
   { &search141, &replace141.value, 0 },
};
   
static const nir_search_variable search23_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search23_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_constant search23_2 = {
   { nir_search_value_constant },
   { 0x0 /* 0.0 */ },
};
static const nir_search_expression search23 = {
   { nir_search_value_expression },
   nir_op_flrp,
   { &search23_0.value, &search23_1.value, &search23_2.value },
};
   
static const nir_search_variable replace23 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
   
static const nir_search_variable search24_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search24_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_constant search24_2 = {
   { nir_search_value_constant },
   { 0x3f800000 /* 1.0 */ },
};
static const nir_search_expression search24 = {
   { nir_search_value_expression },
   nir_op_flrp,
   { &search24_0.value, &search24_1.value, &search24_2.value },
};
   
static const nir_search_variable replace24 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
   
static const nir_search_variable search25_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search25_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search25_2 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search25 = {
   { nir_search_value_expression },
   nir_op_flrp,
   { &search25_0.value, &search25_1.value, &search25_2.value },
};
   
static const nir_search_variable replace25 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
   
static const nir_search_constant search26_0 = {
   { nir_search_value_constant },
   { 0x0 /* 0.0 */ },
};

static const nir_search_variable search26_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search26_2 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search26 = {
   { nir_search_value_expression },
   nir_op_flrp,
   { &search26_0.value, &search26_1.value, &search26_2.value },
};
   
static const nir_search_variable replace26_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace26_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace26 = {
   { nir_search_value_expression },
   nir_op_fmul,
   { &replace26_0.value, &replace26_1.value },
};
   
static const nir_search_variable search27_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search27_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_variable search27_2 = {
   { nir_search_value_variable },
   2, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression search27 = {
   { nir_search_value_expression },
   nir_op_flrp,
   { &search27_0.value, &search27_1.value, &search27_2.value },
};
   
static const nir_search_variable replace27_0_0 = {
   { nir_search_value_variable },
   2, /* c */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace27_0_1_0 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace27_0_1_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace27_0_1 = {
   { nir_search_value_expression },
   nir_op_fsub,
   { &replace27_0_1_0.value, &replace27_0_1_1.value },
};
static const nir_search_expression replace27_0 = {
   { nir_search_value_expression },
   nir_op_fmul,
   { &replace27_0_0.value, &replace27_0_1.value },
};

static const nir_search_variable replace27_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace27 = {
   { nir_search_value_expression },
   nir_op_fadd,
   { &replace27_0.value, &replace27_1.value },
};

static const struct transform nir_opt_algebraic_flrp_xforms[] = {
   { &search23, &replace23.value, 0 },
   { &search24, &replace24.value, 0 },
   { &search25, &replace25.value, 0 },
   { &search26, &replace26.value, 0 },
   { &search27, &replace27.value, 1 },
};
   
static const nir_search_variable search55_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search55_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search55_0 = {
   { nir_search_value_expression },
   nir_op_flt,
   { &search55_0_0.value, &search55_0_1.value },
};

static const nir_search_variable search55_1_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search55_1_1 = {
   { nir_search_value_variable },
   2, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression search55_1 = {
   { nir_search_value_expression },
   nir_op_flt,
   { &search55_1_0.value, &search55_1_1.value },
};
static const nir_search_expression search55 = {
   { nir_search_value_expression },
   nir_op_ior,
   { &search55_0.value, &search55_1.value },
};
   
static const nir_search_variable replace55_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace55_1_0 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace55_1_1 = {
   { nir_search_value_variable },
   2, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace55_1 = {
   { nir_search_value_expression },
   nir_op_fmax,
   { &replace55_1_0.value, &replace55_1_1.value },
};
static const nir_search_expression replace55 = {
   { nir_search_value_expression },
   nir_op_flt,
   { &replace55_0.value, &replace55_1.value },
};
   
static const nir_search_variable search56_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search56_0_1 = {
   { nir_search_value_variable },
   1, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression search56_0 = {
   { nir_search_value_expression },
   nir_op_flt,
   { &search56_0_0.value, &search56_0_1.value },
};

static const nir_search_variable search56_1_0 = {
   { nir_search_value_variable },
   2, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_variable search56_1_1 = {
   { nir_search_value_variable },
   1, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression search56_1 = {
   { nir_search_value_expression },
   nir_op_flt,
   { &search56_1_0.value, &search56_1_1.value },
};
static const nir_search_expression search56 = {
   { nir_search_value_expression },
   nir_op_ior,
   { &search56_0.value, &search56_1.value },
};
   
static const nir_search_variable replace56_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace56_0_1 = {
   { nir_search_value_variable },
   2, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace56_0 = {
   { nir_search_value_expression },
   nir_op_fmin,
   { &replace56_0_0.value, &replace56_0_1.value },
};

static const nir_search_variable replace56_1 = {
   { nir_search_value_variable },
   1, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace56 = {
   { nir_search_value_expression },
   nir_op_flt,
   { &replace56_0.value, &replace56_1.value },
};
   
static const nir_search_variable search57_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search57_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search57_0 = {
   { nir_search_value_expression },
   nir_op_fge,
   { &search57_0_0.value, &search57_0_1.value },
};

static const nir_search_variable search57_1_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search57_1_1 = {
   { nir_search_value_variable },
   2, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression search57_1 = {
   { nir_search_value_expression },
   nir_op_fge,
   { &search57_1_0.value, &search57_1_1.value },
};
static const nir_search_expression search57 = {
   { nir_search_value_expression },
   nir_op_ior,
   { &search57_0.value, &search57_1.value },
};
   
static const nir_search_variable replace57_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace57_1_0 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace57_1_1 = {
   { nir_search_value_variable },
   2, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace57_1 = {
   { nir_search_value_expression },
   nir_op_fmin,
   { &replace57_1_0.value, &replace57_1_1.value },
};
static const nir_search_expression replace57 = {
   { nir_search_value_expression },
   nir_op_fge,
   { &replace57_0.value, &replace57_1.value },
};
   
static const nir_search_variable search58_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search58_0_1 = {
   { nir_search_value_variable },
   1, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression search58_0 = {
   { nir_search_value_expression },
   nir_op_fge,
   { &search58_0_0.value, &search58_0_1.value },
};

static const nir_search_variable search58_1_0 = {
   { nir_search_value_variable },
   2, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_variable search58_1_1 = {
   { nir_search_value_variable },
   1, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression search58_1 = {
   { nir_search_value_expression },
   nir_op_fge,
   { &search58_1_0.value, &search58_1_1.value },
};
static const nir_search_expression search58 = {
   { nir_search_value_expression },
   nir_op_ior,
   { &search58_0.value, &search58_1.value },
};
   
static const nir_search_variable replace58_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace58_0_1 = {
   { nir_search_value_variable },
   2, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace58_0 = {
   { nir_search_value_expression },
   nir_op_fmax,
   { &replace58_0_0.value, &replace58_0_1.value },
};

static const nir_search_variable replace58_1 = {
   { nir_search_value_variable },
   1, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace58 = {
   { nir_search_value_expression },
   nir_op_fge,
   { &replace58_0.value, &replace58_1.value },
};
   
static const nir_search_variable search78_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search78_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search78 = {
   { nir_search_value_expression },
   nir_op_ior,
   { &search78_0.value, &search78_1.value },
};
   
static const nir_search_variable replace78 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
   
static const nir_search_variable search79_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_constant search79_1 = {
   { nir_search_value_constant },
   { 0x0 /* 0 */ },
};
static const nir_search_expression search79 = {
   { nir_search_value_expression },
   nir_op_ior,
   { &search79_0.value, &search79_1.value },
};
   
static const nir_search_variable replace79 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
   
static const nir_search_variable search84_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search84_0 = {
   { nir_search_value_expression },
   nir_op_inot,
   { &search84_0_0.value },
};

static const nir_search_variable search84_1_0 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search84_1 = {
   { nir_search_value_expression },
   nir_op_inot,
   { &search84_1_0.value },
};
static const nir_search_expression search84 = {
   { nir_search_value_expression },
   nir_op_ior,
   { &search84_0.value, &search84_1.value },
};
   
static const nir_search_variable replace84_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace84_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace84_0 = {
   { nir_search_value_expression },
   nir_op_iand,
   { &replace84_0_0.value, &replace84_0_1.value },
};
static const nir_search_expression replace84 = {
   { nir_search_value_expression },
   nir_op_inot,
   { &replace84_0.value },
};

static const struct transform nir_opt_algebraic_ior_xforms[] = {
   { &search55, &replace55.value, 0 },
   { &search56, &replace56.value, 0 },
   { &search57, &replace57.value, 0 },
   { &search58, &replace58.value, 0 },
   { &search78, &replace78.value, 0 },
   { &search79, &replace79.value, 0 },
   { &search84, &replace84.value, 0 },
};
   
static const nir_search_variable search48_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search48_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search48 = {
   { nir_search_value_expression },
   nir_op_imax,
   { &search48_0.value, &search48_1.value },
};
   
static const nir_search_variable replace48 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const struct transform nir_opt_algebraic_imax_xforms[] = {
   { &search48, &replace48.value, 0 },
};
   
static const nir_search_variable search52_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search52 = {
   { nir_search_value_expression },
   nir_op_fsat,
   { &search52_0.value },
};
   
static const nir_search_variable replace52_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_constant replace52_0_1 = {
   { nir_search_value_constant },
   { 0x0 /* 0.0 */ },
};
static const nir_search_expression replace52_0 = {
   { nir_search_value_expression },
   nir_op_fmax,
   { &replace52_0_0.value, &replace52_0_1.value },
};

static const nir_search_constant replace52_1 = {
   { nir_search_value_constant },
   { 0x3f800000 /* 1.0 */ },
};
static const nir_search_expression replace52 = {
   { nir_search_value_expression },
   nir_op_fmin,
   { &replace52_0.value, &replace52_1.value },
};
   
static const nir_search_variable search53_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search53_0 = {
   { nir_search_value_expression },
   nir_op_fsat,
   { &search53_0_0.value },
};
static const nir_search_expression search53 = {
   { nir_search_value_expression },
   nir_op_fsat,
   { &search53_0.value },
};
   
static const nir_search_variable replace53_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace53 = {
   { nir_search_value_expression },
   nir_op_fsat,
   { &replace53_0.value },
};
   
static const nir_search_variable search65_0_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search65_0_0 = {
   { nir_search_value_expression },
   nir_op_b2f,
   { &search65_0_0_0.value },
};

static const nir_search_variable search65_0_1_0 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search65_0_1 = {
   { nir_search_value_expression },
   nir_op_b2f,
   { &search65_0_1_0.value },
};
static const nir_search_expression search65_0 = {
   { nir_search_value_expression },
   nir_op_fadd,
   { &search65_0_0.value, &search65_0_1.value },
};
static const nir_search_expression search65 = {
   { nir_search_value_expression },
   nir_op_fsat,
   { &search65_0.value },
};
   
static const nir_search_variable replace65_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace65_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace65_0 = {
   { nir_search_value_expression },
   nir_op_ior,
   { &replace65_0_0.value, &replace65_0_1.value },
};
static const nir_search_expression replace65 = {
   { nir_search_value_expression },
   nir_op_b2f,
   { &replace65_0.value },
};

static const struct transform nir_opt_algebraic_fsat_xforms[] = {
   { &search52, &replace52.value, 6 },
   { &search53, &replace53.value, 0 },
   { &search65, &replace65.value, 0 },
};
   
static const nir_search_variable search40_0_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search40_0_0 = {
   { nir_search_value_expression },
   nir_op_fabs,
   { &search40_0_0_0.value },
};
static const nir_search_expression search40_0 = {
   { nir_search_value_expression },
   nir_op_fneg,
   { &search40_0_0.value },
};

static const nir_search_constant search40_1 = {
   { nir_search_value_constant },
   { 0x0 /* 0.0 */ },
};
static const nir_search_expression search40 = {
   { nir_search_value_expression },
   nir_op_fge,
   { &search40_0.value, &search40_1.value },
};
   
static const nir_search_variable replace40_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_constant replace40_1 = {
   { nir_search_value_constant },
   { 0x0 /* 0.0 */ },
};
static const nir_search_expression replace40 = {
   { nir_search_value_expression },
   nir_op_feq,
   { &replace40_0.value, &replace40_1.value },
};
   
static const nir_search_variable search138_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search138_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   true,
   nir_type_invalid,
};

static const nir_search_variable search138_0_2 = {
   { nir_search_value_variable },
   2, /* c */
   true,
   nir_type_invalid,
};
static const nir_search_expression search138_0 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &search138_0_0.value, &search138_0_1.value, &search138_0_2.value },
};

static const nir_search_variable search138_1 = {
   { nir_search_value_variable },
   3, /* d */
   true,
   nir_type_invalid,
};
static const nir_search_expression search138 = {
   { nir_search_value_expression },
   nir_op_fge,
   { &search138_0.value, &search138_1.value },
};
   
static const nir_search_variable replace138_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace138_1_0 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace138_1_1 = {
   { nir_search_value_variable },
   3, /* d */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace138_1 = {
   { nir_search_value_expression },
   nir_op_fge,
   { &replace138_1_0.value, &replace138_1_1.value },
};

static const nir_search_variable replace138_2_0 = {
   { nir_search_value_variable },
   2, /* c */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace138_2_1 = {
   { nir_search_value_variable },
   3, /* d */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace138_2 = {
   { nir_search_value_expression },
   nir_op_fge,
   { &replace138_2_0.value, &replace138_2_1.value },
};
static const nir_search_expression replace138 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &replace138_0.value, &replace138_1.value, &replace138_2.value },
};
   
static const nir_search_variable search139_0 = {
   { nir_search_value_variable },
   0, /* d */
   true,
   nir_type_invalid,
};

static const nir_search_variable search139_1_0 = {
   { nir_search_value_variable },
   1, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search139_1_1 = {
   { nir_search_value_variable },
   2, /* b */
   true,
   nir_type_invalid,
};

static const nir_search_variable search139_1_2 = {
   { nir_search_value_variable },
   3, /* c */
   true,
   nir_type_invalid,
};
static const nir_search_expression search139_1 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &search139_1_0.value, &search139_1_1.value, &search139_1_2.value },
};
static const nir_search_expression search139 = {
   { nir_search_value_expression },
   nir_op_fge,
   { &search139_0.value, &search139_1.value },
};
   
static const nir_search_variable replace139_0 = {
   { nir_search_value_variable },
   1, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace139_1_0 = {
   { nir_search_value_variable },
   0, /* d */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace139_1_1 = {
   { nir_search_value_variable },
   2, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace139_1 = {
   { nir_search_value_expression },
   nir_op_fge,
   { &replace139_1_0.value, &replace139_1_1.value },
};

static const nir_search_variable replace139_2_0 = {
   { nir_search_value_variable },
   0, /* d */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace139_2_1 = {
   { nir_search_value_variable },
   3, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace139_2 = {
   { nir_search_value_expression },
   nir_op_fge,
   { &replace139_2_0.value, &replace139_2_1.value },
};
static const nir_search_expression replace139 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &replace139_0.value, &replace139_1.value, &replace139_2.value },
};

static const struct transform nir_opt_algebraic_fge_xforms[] = {
   { &search40, &replace40.value, 0 },
   { &search138, &replace138.value, 0 },
   { &search139, &replace139.value, 0 },
};
   
static const nir_search_variable search100_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search100_0 = {
   { nir_search_value_expression },
   nir_op_fexp2,
   { &search100_0_0.value },
};
static const nir_search_expression search100 = {
   { nir_search_value_expression },
   nir_op_frcp,
   { &search100_0.value },
};
   
static const nir_search_variable replace100_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace100_0 = {
   { nir_search_value_expression },
   nir_op_fneg,
   { &replace100_0_0.value },
};
static const nir_search_expression replace100 = {
   { nir_search_value_expression },
   nir_op_fexp2,
   { &replace100_0.value },
};
   
static const nir_search_variable search110_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search110_0 = {
   { nir_search_value_expression },
   nir_op_frcp,
   { &search110_0_0.value },
};
static const nir_search_expression search110 = {
   { nir_search_value_expression },
   nir_op_frcp,
   { &search110_0.value },
};
   
static const nir_search_variable replace110 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
   
static const nir_search_variable search111_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search111_0 = {
   { nir_search_value_expression },
   nir_op_fsqrt,
   { &search111_0_0.value },
};
static const nir_search_expression search111 = {
   { nir_search_value_expression },
   nir_op_frcp,
   { &search111_0.value },
};
   
static const nir_search_variable replace111_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace111 = {
   { nir_search_value_expression },
   nir_op_frsq,
   { &replace111_0.value },
};
   
static const nir_search_variable search113_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search113_0 = {
   { nir_search_value_expression },
   nir_op_frsq,
   { &search113_0_0.value },
};
static const nir_search_expression search113 = {
   { nir_search_value_expression },
   nir_op_frcp,
   { &search113_0.value },
};
   
static const nir_search_variable replace113_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace113 = {
   { nir_search_value_expression },
   nir_op_fsqrt,
   { &replace113_0.value },
};

static const struct transform nir_opt_algebraic_frcp_xforms[] = {
   { &search100, &replace100.value, 0 },
   { &search110, &replace110.value, 0 },
   { &search111, &replace111.value, 0 },
   { &search113, &replace113.value, 11 },
};
   
static const nir_search_variable search80_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search80_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search80 = {
   { nir_search_value_expression },
   nir_op_fxor,
   { &search80_0.value, &search80_1.value },
};
   
static const nir_search_constant replace80 = {
   { nir_search_value_constant },
   { 0x0 /* 0.0 */ },
};

static const struct transform nir_opt_algebraic_fxor_xforms[] = {
   { &search80, &replace80.value, 0 },
};
   
static const nir_search_constant search89_0 = {
   { nir_search_value_constant },
   { 0x0 /* 0 */ },
};

static const nir_search_variable search89_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search89 = {
   { nir_search_value_expression },
   nir_op_ushr,
   { &search89_0.value, &search89_1.value },
};
   
static const nir_search_constant replace89 = {
   { nir_search_value_constant },
   { 0x0 /* 0 */ },
};
   
static const nir_search_variable search90_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_constant search90_1 = {
   { nir_search_value_constant },
   { 0x0 /* 0 */ },
};
static const nir_search_expression search90 = {
   { nir_search_value_expression },
   nir_op_ushr,
   { &search90_0.value, &search90_1.value },
};
   
static const nir_search_variable replace90 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const struct transform nir_opt_algebraic_ushr_xforms[] = {
   { &search89, &replace89.value, 0 },
   { &search90, &replace90.value, 0 },
};
   
static const nir_search_variable search91_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search91_0 = {
   { nir_search_value_expression },
   nir_op_flog2,
   { &search91_0_0.value },
};
static const nir_search_expression search91 = {
   { nir_search_value_expression },
   nir_op_fexp2,
   { &search91_0.value },
};
   
static const nir_search_variable replace91 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
   
static const nir_search_variable search94_0_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search94_0_0 = {
   { nir_search_value_expression },
   nir_op_flog2,
   { &search94_0_0_0.value },
};

static const nir_search_variable search94_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search94_0 = {
   { nir_search_value_expression },
   nir_op_fmul,
   { &search94_0_0.value, &search94_0_1.value },
};
static const nir_search_expression search94 = {
   { nir_search_value_expression },
   nir_op_fexp2,
   { &search94_0.value },
};
   
static const nir_search_variable replace94_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace94_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace94 = {
   { nir_search_value_expression },
   nir_op_fpow,
   { &replace94_0.value, &replace94_1.value },
};

static const struct transform nir_opt_algebraic_fexp2_xforms[] = {
   { &search91, &replace91.value, 0 },
   { &search94, &replace94.value, 9 },
};
   
static const nir_search_constant search87_0 = {
   { nir_search_value_constant },
   { 0x0 /* 0 */ },
};

static const nir_search_variable search87_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search87 = {
   { nir_search_value_expression },
   nir_op_ishr,
   { &search87_0.value, &search87_1.value },
};
   
static const nir_search_constant replace87 = {
   { nir_search_value_constant },
   { 0x0 /* 0 */ },
};
   
static const nir_search_variable search88_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_constant search88_1 = {
   { nir_search_value_constant },
   { 0x0 /* 0 */ },
};
static const nir_search_expression search88 = {
   { nir_search_value_expression },
   nir_op_ishr,
   { &search88_0.value, &search88_1.value },
};
   
static const nir_search_variable replace88 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const struct transform nir_opt_algebraic_ishr_xforms[] = {
   { &search87, &replace87.value, 0 },
   { &search88, &replace88.value, 0 },
};
   
static const nir_search_variable search59_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search59_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search59 = {
   { nir_search_value_expression },
   nir_op_slt,
   { &search59_0.value, &search59_1.value },
};
   
static const nir_search_variable replace59_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace59_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace59_0 = {
   { nir_search_value_expression },
   nir_op_flt,
   { &replace59_0_0.value, &replace59_0_1.value },
};
static const nir_search_expression replace59 = {
   { nir_search_value_expression },
   nir_op_b2f,
   { &replace59_0.value },
};

static const struct transform nir_opt_algebraic_slt_xforms[] = {
   { &search59, &replace59.value, 7 },
};
   
static const nir_search_variable search124_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search124_0 = {
   { nir_search_value_expression },
   nir_op_ftrunc,
   { &search124_0_0.value },
};
static const nir_search_expression search124 = {
   { nir_search_value_expression },
   nir_op_f2i,
   { &search124_0.value },
};
   
static const nir_search_variable replace124_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace124 = {
   { nir_search_value_expression },
   nir_op_f2i,
   { &replace124_0.value },
};

static const struct transform nir_opt_algebraic_f2i_xforms[] = {
   { &search124, &replace124.value, 0 },
};
   
static const nir_search_variable search67_0_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search67_0_0 = {
   { nir_search_value_expression },
   nir_op_b2f,
   { &search67_0_0_0.value },
};
static const nir_search_expression search67_0 = {
   { nir_search_value_expression },
   nir_op_fneg,
   { &search67_0_0.value },
};

static const nir_search_constant search67_1 = {
   { nir_search_value_constant },
   { 0x0 /* 0 */ },
};
static const nir_search_expression search67 = {
   { nir_search_value_expression },
   nir_op_flt,
   { &search67_0.value, &search67_1.value },
};
   
static const nir_search_variable replace67 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
   
static const nir_search_constant search68_0_0 = {
   { nir_search_value_constant },
   { 0x0 /* 0.0 */ },
};

static const nir_search_variable search68_0_1_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search68_0_1 = {
   { nir_search_value_expression },
   nir_op_b2f,
   { &search68_0_1_0.value },
};
static const nir_search_expression search68_0 = {
   { nir_search_value_expression },
   nir_op_fsub,
   { &search68_0_0.value, &search68_0_1.value },
};

static const nir_search_constant search68_1 = {
   { nir_search_value_constant },
   { 0x0 /* 0 */ },
};
static const nir_search_expression search68 = {
   { nir_search_value_expression },
   nir_op_flt,
   { &search68_0.value, &search68_1.value },
};
   
static const nir_search_variable replace68 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
   
static const nir_search_variable search136_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search136_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   true,
   nir_type_invalid,
};

static const nir_search_variable search136_0_2 = {
   { nir_search_value_variable },
   2, /* c */
   true,
   nir_type_invalid,
};
static const nir_search_expression search136_0 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &search136_0_0.value, &search136_0_1.value, &search136_0_2.value },
};

static const nir_search_variable search136_1 = {
   { nir_search_value_variable },
   3, /* d */
   true,
   nir_type_invalid,
};
static const nir_search_expression search136 = {
   { nir_search_value_expression },
   nir_op_flt,
   { &search136_0.value, &search136_1.value },
};
   
static const nir_search_variable replace136_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace136_1_0 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace136_1_1 = {
   { nir_search_value_variable },
   3, /* d */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace136_1 = {
   { nir_search_value_expression },
   nir_op_flt,
   { &replace136_1_0.value, &replace136_1_1.value },
};

static const nir_search_variable replace136_2_0 = {
   { nir_search_value_variable },
   2, /* c */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace136_2_1 = {
   { nir_search_value_variable },
   3, /* d */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace136_2 = {
   { nir_search_value_expression },
   nir_op_flt,
   { &replace136_2_0.value, &replace136_2_1.value },
};
static const nir_search_expression replace136 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &replace136_0.value, &replace136_1.value, &replace136_2.value },
};
   
static const nir_search_variable search137_0 = {
   { nir_search_value_variable },
   0, /* d */
   true,
   nir_type_invalid,
};

static const nir_search_variable search137_1_0 = {
   { nir_search_value_variable },
   1, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search137_1_1 = {
   { nir_search_value_variable },
   2, /* b */
   true,
   nir_type_invalid,
};

static const nir_search_variable search137_1_2 = {
   { nir_search_value_variable },
   3, /* c */
   true,
   nir_type_invalid,
};
static const nir_search_expression search137_1 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &search137_1_0.value, &search137_1_1.value, &search137_1_2.value },
};
static const nir_search_expression search137 = {
   { nir_search_value_expression },
   nir_op_flt,
   { &search137_0.value, &search137_1.value },
};
   
static const nir_search_variable replace137_0 = {
   { nir_search_value_variable },
   1, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace137_1_0 = {
   { nir_search_value_variable },
   0, /* d */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace137_1_1 = {
   { nir_search_value_variable },
   2, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace137_1 = {
   { nir_search_value_expression },
   nir_op_flt,
   { &replace137_1_0.value, &replace137_1_1.value },
};

static const nir_search_variable replace137_2_0 = {
   { nir_search_value_variable },
   0, /* d */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace137_2_1 = {
   { nir_search_value_variable },
   3, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace137_2 = {
   { nir_search_value_expression },
   nir_op_flt,
   { &replace137_2_0.value, &replace137_2_1.value },
};
static const nir_search_expression replace137 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &replace137_0.value, &replace137_1.value, &replace137_2.value },
};

static const struct transform nir_opt_algebraic_flt_xforms[] = {
   { &search67, &replace67.value, 0 },
   { &search68, &replace68.value, 0 },
   { &search136, &replace136.value, 0 },
   { &search137, &replace137.value, 0 },
};
   
static const nir_search_variable search73_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search73_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search73 = {
   { nir_search_value_expression },
   nir_op_ult,
   { &search73_0.value, &search73_1.value },
};
   
static const nir_search_constant replace73 = {
   { nir_search_value_constant },
   { NIR_FALSE /* False */ },
};
   
static const nir_search_variable search152_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search152_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   true,
   nir_type_invalid,
};

static const nir_search_variable search152_0_2 = {
   { nir_search_value_variable },
   2, /* c */
   true,
   nir_type_invalid,
};
static const nir_search_expression search152_0 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &search152_0_0.value, &search152_0_1.value, &search152_0_2.value },
};

static const nir_search_variable search152_1 = {
   { nir_search_value_variable },
   3, /* d */
   true,
   nir_type_invalid,
};
static const nir_search_expression search152 = {
   { nir_search_value_expression },
   nir_op_ult,
   { &search152_0.value, &search152_1.value },
};
   
static const nir_search_variable replace152_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace152_1_0 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace152_1_1 = {
   { nir_search_value_variable },
   3, /* d */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace152_1 = {
   { nir_search_value_expression },
   nir_op_ult,
   { &replace152_1_0.value, &replace152_1_1.value },
};

static const nir_search_variable replace152_2_0 = {
   { nir_search_value_variable },
   2, /* c */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace152_2_1 = {
   { nir_search_value_variable },
   3, /* d */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace152_2 = {
   { nir_search_value_expression },
   nir_op_ult,
   { &replace152_2_0.value, &replace152_2_1.value },
};
static const nir_search_expression replace152 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &replace152_0.value, &replace152_1.value, &replace152_2.value },
};
   
static const nir_search_variable search153_0 = {
   { nir_search_value_variable },
   0, /* d */
   true,
   nir_type_invalid,
};

static const nir_search_variable search153_1_0 = {
   { nir_search_value_variable },
   1, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search153_1_1 = {
   { nir_search_value_variable },
   2, /* b */
   true,
   nir_type_invalid,
};

static const nir_search_variable search153_1_2 = {
   { nir_search_value_variable },
   3, /* c */
   true,
   nir_type_invalid,
};
static const nir_search_expression search153_1 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &search153_1_0.value, &search153_1_1.value, &search153_1_2.value },
};
static const nir_search_expression search153 = {
   { nir_search_value_expression },
   nir_op_ult,
   { &search153_0.value, &search153_1.value },
};
   
static const nir_search_variable replace153_0 = {
   { nir_search_value_variable },
   1, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace153_1_0 = {
   { nir_search_value_variable },
   0, /* d */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace153_1_1 = {
   { nir_search_value_variable },
   2, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace153_1 = {
   { nir_search_value_expression },
   nir_op_ult,
   { &replace153_1_0.value, &replace153_1_1.value },
};

static const nir_search_variable replace153_2_0 = {
   { nir_search_value_variable },
   0, /* d */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace153_2_1 = {
   { nir_search_value_variable },
   3, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace153_2 = {
   { nir_search_value_expression },
   nir_op_ult,
   { &replace153_2_0.value, &replace153_2_1.value },
};
static const nir_search_expression replace153 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &replace153_0.value, &replace153_1.value, &replace153_2.value },
};

static const struct transform nir_opt_algebraic_ult_xforms[] = {
   { &search73, &replace73.value, 0 },
   { &search152, &replace152.value, 0 },
   { &search153, &replace153.value, 0 },
};
   
static const nir_search_variable search126_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_constant search126_1_0 = {
   { nir_search_value_constant },
   { 0x0 /* 0.0 */ },
};

static const nir_search_variable search126_1_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search126_1 = {
   { nir_search_value_expression },
   nir_op_fsub,
   { &search126_1_0.value, &search126_1_1.value },
};
static const nir_search_expression search126 = {
   { nir_search_value_expression },
   nir_op_fsub,
   { &search126_0.value, &search126_1.value },
};
   
static const nir_search_variable replace126_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace126_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace126 = {
   { nir_search_value_expression },
   nir_op_fadd,
   { &replace126_0.value, &replace126_1.value },
};
   
static const nir_search_variable search128_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search128_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search128 = {
   { nir_search_value_expression },
   nir_op_fsub,
   { &search128_0.value, &search128_1.value },
};
   
static const nir_search_variable replace128_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace128_1_0 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace128_1 = {
   { nir_search_value_expression },
   nir_op_fneg,
   { &replace128_1_0.value },
};
static const nir_search_expression replace128 = {
   { nir_search_value_expression },
   nir_op_fadd,
   { &replace128_0.value, &replace128_1.value },
};

static const struct transform nir_opt_algebraic_fsub_xforms[] = {
   { &search126, &replace126.value, 0 },
   { &search128, &replace128.value, 12 },
};
   
static const nir_search_variable search0_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search0_0 = {
   { nir_search_value_expression },
   nir_op_fneg,
   { &search0_0_0.value },
};
static const nir_search_expression search0 = {
   { nir_search_value_expression },
   nir_op_fneg,
   { &search0_0.value },
};
   
static const nir_search_variable replace0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
   
static const nir_search_variable search130_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search130 = {
   { nir_search_value_expression },
   nir_op_fneg,
   { &search130_0.value },
};
   
static const nir_search_constant replace130_0 = {
   { nir_search_value_constant },
   { 0x0 /* 0.0 */ },
};

static const nir_search_variable replace130_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace130 = {
   { nir_search_value_expression },
   nir_op_fsub,
   { &replace130_0.value, &replace130_1.value },
};

static const struct transform nir_opt_algebraic_fneg_xforms[] = {
   { &search0, &replace0.value, 0 },
   { &search130, &replace130.value, 13 },
};
   
static const nir_search_variable search69_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search69_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search69 = {
   { nir_search_value_expression },
   nir_op_ilt,
   { &search69_0.value, &search69_1.value },
};
   
static const nir_search_constant replace69 = {
   { nir_search_value_constant },
   { NIR_FALSE /* False */ },
};
   
static const nir_search_variable search144_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search144_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   true,
   nir_type_invalid,
};

static const nir_search_variable search144_0_2 = {
   { nir_search_value_variable },
   2, /* c */
   true,
   nir_type_invalid,
};
static const nir_search_expression search144_0 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &search144_0_0.value, &search144_0_1.value, &search144_0_2.value },
};

static const nir_search_variable search144_1 = {
   { nir_search_value_variable },
   3, /* d */
   true,
   nir_type_invalid,
};
static const nir_search_expression search144 = {
   { nir_search_value_expression },
   nir_op_ilt,
   { &search144_0.value, &search144_1.value },
};
   
static const nir_search_variable replace144_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace144_1_0 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace144_1_1 = {
   { nir_search_value_variable },
   3, /* d */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace144_1 = {
   { nir_search_value_expression },
   nir_op_ilt,
   { &replace144_1_0.value, &replace144_1_1.value },
};

static const nir_search_variable replace144_2_0 = {
   { nir_search_value_variable },
   2, /* c */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace144_2_1 = {
   { nir_search_value_variable },
   3, /* d */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace144_2 = {
   { nir_search_value_expression },
   nir_op_ilt,
   { &replace144_2_0.value, &replace144_2_1.value },
};
static const nir_search_expression replace144 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &replace144_0.value, &replace144_1.value, &replace144_2.value },
};
   
static const nir_search_variable search145_0 = {
   { nir_search_value_variable },
   0, /* d */
   true,
   nir_type_invalid,
};

static const nir_search_variable search145_1_0 = {
   { nir_search_value_variable },
   1, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search145_1_1 = {
   { nir_search_value_variable },
   2, /* b */
   true,
   nir_type_invalid,
};

static const nir_search_variable search145_1_2 = {
   { nir_search_value_variable },
   3, /* c */
   true,
   nir_type_invalid,
};
static const nir_search_expression search145_1 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &search145_1_0.value, &search145_1_1.value, &search145_1_2.value },
};
static const nir_search_expression search145 = {
   { nir_search_value_expression },
   nir_op_ilt,
   { &search145_0.value, &search145_1.value },
};
   
static const nir_search_variable replace145_0 = {
   { nir_search_value_variable },
   1, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace145_1_0 = {
   { nir_search_value_variable },
   0, /* d */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace145_1_1 = {
   { nir_search_value_variable },
   2, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace145_1 = {
   { nir_search_value_expression },
   nir_op_ilt,
   { &replace145_1_0.value, &replace145_1_1.value },
};

static const nir_search_variable replace145_2_0 = {
   { nir_search_value_variable },
   0, /* d */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace145_2_1 = {
   { nir_search_value_variable },
   3, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace145_2 = {
   { nir_search_value_expression },
   nir_op_ilt,
   { &replace145_2_0.value, &replace145_2_1.value },
};
static const nir_search_expression replace145 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &replace145_0.value, &replace145_1.value, &replace145_2.value },
};

static const struct transform nir_opt_algebraic_ilt_xforms[] = {
   { &search69, &replace69.value, 0 },
   { &search144, &replace144.value, 0 },
   { &search145, &replace145.value, 0 },
};
   
static const nir_search_variable search4_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search4_0 = {
   { nir_search_value_expression },
   nir_op_iabs,
   { &search4_0_0.value },
};
static const nir_search_expression search4 = {
   { nir_search_value_expression },
   nir_op_iabs,
   { &search4_0.value },
};
   
static const nir_search_variable replace4_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace4 = {
   { nir_search_value_expression },
   nir_op_iabs,
   { &replace4_0.value },
};
   
static const nir_search_variable search5_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search5_0 = {
   { nir_search_value_expression },
   nir_op_ineg,
   { &search5_0_0.value },
};
static const nir_search_expression search5 = {
   { nir_search_value_expression },
   nir_op_iabs,
   { &search5_0.value },
};
   
static const nir_search_variable replace5_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace5 = {
   { nir_search_value_expression },
   nir_op_iabs,
   { &replace5_0.value },
};
   
static const nir_search_constant search135_0_0 = {
   { nir_search_value_constant },
   { 0x0 /* 0 */ },
};

static const nir_search_variable search135_0_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search135_0 = {
   { nir_search_value_expression },
   nir_op_isub,
   { &search135_0_0.value, &search135_0_1.value },
};
static const nir_search_expression search135 = {
   { nir_search_value_expression },
   nir_op_iabs,
   { &search135_0.value },
};
   
static const nir_search_variable replace135_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace135 = {
   { nir_search_value_expression },
   nir_op_iabs,
   { &replace135_0.value },
};

static const struct transform nir_opt_algebraic_iabs_xforms[] = {
   { &search4, &replace4.value, 0 },
   { &search5, &replace5.value, 0 },
   { &search135, &replace135.value, 0 },
};
   
static const nir_search_variable search45_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search45_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search45 = {
   { nir_search_value_expression },
   nir_op_fmin,
   { &search45_0.value, &search45_1.value },
};
   
static const nir_search_variable replace45 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
   
static const nir_search_variable search51_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_constant search51_0_1 = {
   { nir_search_value_constant },
   { 0x0 /* 0.0 */ },
};
static const nir_search_expression search51_0 = {
   { nir_search_value_expression },
   nir_op_fmax,
   { &search51_0_0.value, &search51_0_1.value },
};

static const nir_search_constant search51_1 = {
   { nir_search_value_constant },
   { 0x3f800000 /* 1.0 */ },
};
static const nir_search_expression search51 = {
   { nir_search_value_expression },
   nir_op_fmin,
   { &search51_0.value, &search51_1.value },
};
   
static const nir_search_variable replace51_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace51 = {
   { nir_search_value_expression },
   nir_op_fsat,
   { &replace51_0.value },
};
   
static const nir_search_variable search54_0_0_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_constant search54_0_0_0_1 = {
   { nir_search_value_constant },
   { 0x0 /* 0.0 */ },
};
static const nir_search_expression search54_0_0_0 = {
   { nir_search_value_expression },
   nir_op_fmax,
   { &search54_0_0_0_0.value, &search54_0_0_0_1.value },
};

static const nir_search_constant search54_0_0_1 = {
   { nir_search_value_constant },
   { 0x3f800000 /* 1.0 */ },
};
static const nir_search_expression search54_0_0 = {
   { nir_search_value_expression },
   nir_op_fmin,
   { &search54_0_0_0.value, &search54_0_0_1.value },
};

static const nir_search_constant search54_0_1 = {
   { nir_search_value_constant },
   { 0x0 /* 0.0 */ },
};
static const nir_search_expression search54_0 = {
   { nir_search_value_expression },
   nir_op_fmax,
   { &search54_0_0.value, &search54_0_1.value },
};

static const nir_search_constant search54_1 = {
   { nir_search_value_constant },
   { 0x3f800000 /* 1.0 */ },
};
static const nir_search_expression search54 = {
   { nir_search_value_expression },
   nir_op_fmin,
   { &search54_0.value, &search54_1.value },
};
   
static const nir_search_variable replace54_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_constant replace54_0_1 = {
   { nir_search_value_constant },
   { 0x0 /* 0.0 */ },
};
static const nir_search_expression replace54_0 = {
   { nir_search_value_expression },
   nir_op_fmax,
   { &replace54_0_0.value, &replace54_0_1.value },
};

static const nir_search_constant replace54_1 = {
   { nir_search_value_constant },
   { 0x3f800000 /* 1.0 */ },
};
static const nir_search_expression replace54 = {
   { nir_search_value_expression },
   nir_op_fmin,
   { &replace54_0.value, &replace54_1.value },
};

static const struct transform nir_opt_algebraic_fmin_xforms[] = {
   { &search45, &replace45.value, 0 },
   { &search51, &replace51.value, 5 },
   { &search54, &replace54.value, 0 },
};
   
static const nir_search_variable search72_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search72_1 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search72 = {
   { nir_search_value_expression },
   nir_op_ine,
   { &search72_0.value, &search72_1.value },
};
   
static const nir_search_constant replace72 = {
   { nir_search_value_constant },
   { NIR_FALSE /* False */ },
};
   
static const nir_search_variable search114_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_bool,
};

static const nir_search_constant search114_1 = {
   { nir_search_value_constant },
   { 0x0 /* 0 */ },
};
static const nir_search_expression search114 = {
   { nir_search_value_expression },
   nir_op_ine,
   { &search114_0.value, &search114_1.value },
};
   
static const nir_search_variable replace114 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
   
static const nir_search_variable search150_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search150_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   true,
   nir_type_invalid,
};

static const nir_search_variable search150_0_2 = {
   { nir_search_value_variable },
   2, /* c */
   true,
   nir_type_invalid,
};
static const nir_search_expression search150_0 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &search150_0_0.value, &search150_0_1.value, &search150_0_2.value },
};

static const nir_search_variable search150_1 = {
   { nir_search_value_variable },
   3, /* d */
   true,
   nir_type_invalid,
};
static const nir_search_expression search150 = {
   { nir_search_value_expression },
   nir_op_ine,
   { &search150_0.value, &search150_1.value },
};
   
static const nir_search_variable replace150_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace150_1_0 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace150_1_1 = {
   { nir_search_value_variable },
   3, /* d */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace150_1 = {
   { nir_search_value_expression },
   nir_op_ine,
   { &replace150_1_0.value, &replace150_1_1.value },
};

static const nir_search_variable replace150_2_0 = {
   { nir_search_value_variable },
   2, /* c */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace150_2_1 = {
   { nir_search_value_variable },
   3, /* d */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace150_2 = {
   { nir_search_value_expression },
   nir_op_ine,
   { &replace150_2_0.value, &replace150_2_1.value },
};
static const nir_search_expression replace150 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &replace150_0.value, &replace150_1.value, &replace150_2.value },
};
   
static const nir_search_variable search151_0 = {
   { nir_search_value_variable },
   0, /* d */
   true,
   nir_type_invalid,
};

static const nir_search_variable search151_1_0 = {
   { nir_search_value_variable },
   1, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search151_1_1 = {
   { nir_search_value_variable },
   2, /* b */
   true,
   nir_type_invalid,
};

static const nir_search_variable search151_1_2 = {
   { nir_search_value_variable },
   3, /* c */
   true,
   nir_type_invalid,
};
static const nir_search_expression search151_1 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &search151_1_0.value, &search151_1_1.value, &search151_1_2.value },
};
static const nir_search_expression search151 = {
   { nir_search_value_expression },
   nir_op_ine,
   { &search151_0.value, &search151_1.value },
};
   
static const nir_search_variable replace151_0 = {
   { nir_search_value_variable },
   1, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace151_1_0 = {
   { nir_search_value_variable },
   0, /* d */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace151_1_1 = {
   { nir_search_value_variable },
   2, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace151_1 = {
   { nir_search_value_expression },
   nir_op_ine,
   { &replace151_1_0.value, &replace151_1_1.value },
};

static const nir_search_variable replace151_2_0 = {
   { nir_search_value_variable },
   0, /* d */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace151_2_1 = {
   { nir_search_value_variable },
   3, /* c */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace151_2 = {
   { nir_search_value_expression },
   nir_op_ine,
   { &replace151_2_0.value, &replace151_2_1.value },
};
static const nir_search_expression replace151 = {
   { nir_search_value_expression },
   nir_op_bcsel,
   { &replace151_0.value, &replace151_1.value, &replace151_2.value },
};

static const struct transform nir_opt_algebraic_ine_xforms[] = {
   { &search72, &replace72.value, 0 },
   { &search114, &replace114.value, 0 },
   { &search150, &replace150.value, 0 },
   { &search151, &replace151.value, 0 },
};
   
static const nir_search_variable search123_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};
static const nir_search_expression search123_0 = {
   { nir_search_value_expression },
   nir_op_b2i,
   { &search123_0_0.value },
};
static const nir_search_expression search123 = {
   { nir_search_value_expression },
   nir_op_i2b,
   { &search123_0.value },
};
   
static const nir_search_variable replace123 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const struct transform nir_opt_algebraic_i2b_xforms[] = {
   { &search123, &replace123.value, 0 },
};

static bool
nir_opt_algebraic_block(nir_block *block, void *void_state)
{
   struct opt_state *state = void_state;

   nir_foreach_instr_safe(block, instr) {
      if (instr->type != nir_instr_type_alu)
         continue;

      nir_alu_instr *alu = nir_instr_as_alu(instr);
      if (!alu->dest.dest.is_ssa)
         continue;

      switch (alu->op) {
      case nir_op_iand:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_iand_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_iand_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_ixor:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_ixor_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_ixor_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_seq:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_seq_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_seq_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_fne:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_fne_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_fne_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_imul:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_imul_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_imul_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_uge:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_uge_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_uge_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_fmul:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_fmul_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_fmul_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_ffma:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_ffma_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_ffma_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_umin:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_umin_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_umin_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_umax:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_umax_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_umax_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_bcsel:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_bcsel_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_bcsel_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_sge:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_sge_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_sge_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_fsqrt:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_fsqrt_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_fsqrt_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_iadd:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_iadd_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_iadd_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_fand:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_fand_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_fand_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_fabs:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_fabs_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_fabs_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_ieq:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_ieq_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_ieq_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_imin:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_imin_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_imin_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_frsq:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_frsq_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_frsq_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_ineg:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_ineg_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_ineg_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_fpow:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_fpow_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_fpow_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_ige:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_ige_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_ige_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_fdiv:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_fdiv_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_fdiv_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_fadd:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_fadd_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_fadd_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_ishl:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_ishl_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_ishl_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_flog2:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_flog2_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_flog2_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_inot:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_inot_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_inot_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_sne:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_sne_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_sne_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_f2u:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_f2u_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_f2u_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_fcsel:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_fcsel_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_fcsel_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_isub:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_isub_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_isub_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_fmax:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_fmax_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_fmax_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_feq:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_feq_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_feq_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_flrp:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_flrp_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_flrp_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_ior:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_ior_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_ior_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_imax:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_imax_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_imax_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_fsat:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_fsat_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_fsat_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_fge:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_fge_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_fge_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_frcp:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_frcp_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_frcp_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_fxor:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_fxor_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_fxor_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_ushr:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_ushr_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_ushr_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_fexp2:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_fexp2_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_fexp2_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_ishr:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_ishr_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_ishr_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_slt:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_slt_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_slt_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_f2i:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_f2i_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_f2i_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_flt:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_flt_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_flt_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_ult:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_ult_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_ult_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_fsub:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_fsub_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_fsub_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_fneg:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_fneg_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_fneg_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_ilt:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_ilt_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_ilt_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_iabs:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_iabs_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_iabs_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_fmin:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_fmin_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_fmin_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_ine:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_ine_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_ine_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_i2b:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_i2b_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_i2b_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      default:
         break;
      }
   }

   return true;
}

static bool
nir_opt_algebraic_impl(nir_function_impl *impl, const bool *condition_flags)
{
   struct opt_state state;

   state.mem_ctx = ralloc_parent(impl);
   state.progress = false;
   state.condition_flags = condition_flags;

   nir_foreach_block(impl, nir_opt_algebraic_block, &state);

   if (state.progress)
      nir_metadata_preserve(impl, nir_metadata_block_index |
                                  nir_metadata_dominance);

   return state.progress;
}


bool
nir_opt_algebraic(nir_shader *shader)
{
   bool progress = false;
   bool condition_flags[14];
   const nir_shader_compiler_options *options = shader->options;

   condition_flags[0] = true;
   condition_flags[1] = options->lower_flrp;
   condition_flags[2] = !options->lower_flrp;
   condition_flags[3] = options->lower_ffma;
   condition_flags[4] = !options->lower_ffma;
   condition_flags[5] = !options->lower_fsat;
   condition_flags[6] = options->lower_fsat;
   condition_flags[7] = options->lower_scmp;
   condition_flags[8] = options->lower_fpow;
   condition_flags[9] = !options->lower_fpow;
   condition_flags[10] = options->lower_fsqrt;
   condition_flags[11] = !options->lower_fsqrt;
   condition_flags[12] = options->lower_sub;
   condition_flags[13] = options->lower_negate;

   nir_foreach_overload(shader, overload) {
      if (overload->impl)
         progress |= nir_opt_algebraic_impl(overload->impl, condition_flags);
   }

   return progress;
}


#include "nir.h"
#include "nir_search.h"

#ifndef NIR_OPT_ALGEBRAIC_STRUCT_DEFS
#define NIR_OPT_ALGEBRAIC_STRUCT_DEFS

struct transform {
   const nir_search_expression *search;
   const nir_search_value *replace;
   unsigned condition_offset;
};

struct opt_state {
   void *mem_ctx;
   bool progress;
   const bool *condition_flags;
};

#endif

   
static const nir_search_variable search157_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search157_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search157_0 = {
   { nir_search_value_expression },
   nir_op_fadd,
   { &search157_0_0.value, &search157_0_1.value },
};

static const nir_search_constant search157_1 = {
   { nir_search_value_constant },
   { 0x0 /* 0.0 */ },
};
static const nir_search_expression search157 = {
   { nir_search_value_expression },
   nir_op_fge,
   { &search157_0.value, &search157_1.value },
};
   
static const nir_search_variable replace157_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace157_1_0 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace157_1 = {
   { nir_search_value_expression },
   nir_op_fneg,
   { &replace157_1_0.value },
};
static const nir_search_expression replace157 = {
   { nir_search_value_expression },
   nir_op_fge,
   { &replace157_0.value, &replace157_1.value },
};

static const struct transform nir_opt_algebraic_late_fge_xforms[] = {
   { &search157, &replace157.value, 0 },
};
   
static const nir_search_variable search159_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search159_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search159_0 = {
   { nir_search_value_expression },
   nir_op_fadd,
   { &search159_0_0.value, &search159_0_1.value },
};

static const nir_search_constant search159_1 = {
   { nir_search_value_constant },
   { 0x0 /* 0.0 */ },
};
static const nir_search_expression search159 = {
   { nir_search_value_expression },
   nir_op_fne,
   { &search159_0.value, &search159_1.value },
};
   
static const nir_search_variable replace159_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace159_1_0 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace159_1 = {
   { nir_search_value_expression },
   nir_op_fneg,
   { &replace159_1_0.value },
};
static const nir_search_expression replace159 = {
   { nir_search_value_expression },
   nir_op_fne,
   { &replace159_0.value, &replace159_1.value },
};

static const struct transform nir_opt_algebraic_late_fne_xforms[] = {
   { &search159, &replace159.value, 0 },
};
   
static const nir_search_variable search156_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search156_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search156_0 = {
   { nir_search_value_expression },
   nir_op_fadd,
   { &search156_0_0.value, &search156_0_1.value },
};

static const nir_search_constant search156_1 = {
   { nir_search_value_constant },
   { 0x0 /* 0.0 */ },
};
static const nir_search_expression search156 = {
   { nir_search_value_expression },
   nir_op_flt,
   { &search156_0.value, &search156_1.value },
};
   
static const nir_search_variable replace156_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace156_1_0 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace156_1 = {
   { nir_search_value_expression },
   nir_op_fneg,
   { &replace156_1_0.value },
};
static const nir_search_expression replace156 = {
   { nir_search_value_expression },
   nir_op_flt,
   { &replace156_0.value, &replace156_1.value },
};

static const struct transform nir_opt_algebraic_late_flt_xforms[] = {
   { &search156, &replace156.value, 0 },
};
   
static const nir_search_variable search158_0_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable search158_0_1 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression search158_0 = {
   { nir_search_value_expression },
   nir_op_fadd,
   { &search158_0_0.value, &search158_0_1.value },
};

static const nir_search_constant search158_1 = {
   { nir_search_value_constant },
   { 0x0 /* 0.0 */ },
};
static const nir_search_expression search158 = {
   { nir_search_value_expression },
   nir_op_feq,
   { &search158_0.value, &search158_1.value },
};
   
static const nir_search_variable replace158_0 = {
   { nir_search_value_variable },
   0, /* a */
   false,
   nir_type_invalid,
};

static const nir_search_variable replace158_1_0 = {
   { nir_search_value_variable },
   1, /* b */
   false,
   nir_type_invalid,
};
static const nir_search_expression replace158_1 = {
   { nir_search_value_expression },
   nir_op_fneg,
   { &replace158_1_0.value },
};
static const nir_search_expression replace158 = {
   { nir_search_value_expression },
   nir_op_feq,
   { &replace158_0.value, &replace158_1.value },
};

static const struct transform nir_opt_algebraic_late_feq_xforms[] = {
   { &search158, &replace158.value, 0 },
};

static bool
nir_opt_algebraic_late_block(nir_block *block, void *void_state)
{
   struct opt_state *state = void_state;

   nir_foreach_instr_safe(block, instr) {
      if (instr->type != nir_instr_type_alu)
         continue;

      nir_alu_instr *alu = nir_instr_as_alu(instr);
      if (!alu->dest.dest.is_ssa)
         continue;

      switch (alu->op) {
      case nir_op_fge:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_late_fge_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_late_fge_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_fne:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_late_fne_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_late_fne_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_flt:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_late_flt_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_late_flt_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      case nir_op_feq:
         for (unsigned i = 0; i < ARRAY_SIZE(nir_opt_algebraic_late_feq_xforms); i++) {
            const struct transform *xform = &nir_opt_algebraic_late_feq_xforms[i];
            if (state->condition_flags[xform->condition_offset] &&
                nir_replace_instr(alu, xform->search, xform->replace,
                                  state->mem_ctx)) {
               state->progress = true;
               break;
            }
         }
         break;
      default:
         break;
      }
   }

   return true;
}

static bool
nir_opt_algebraic_late_impl(nir_function_impl *impl, const bool *condition_flags)
{
   struct opt_state state;

   state.mem_ctx = ralloc_parent(impl);
   state.progress = false;
   state.condition_flags = condition_flags;

   nir_foreach_block(impl, nir_opt_algebraic_late_block, &state);

   if (state.progress)
      nir_metadata_preserve(impl, nir_metadata_block_index |
                                  nir_metadata_dominance);

   return state.progress;
}


bool
nir_opt_algebraic_late(nir_shader *shader)
{
   bool progress = false;
   bool condition_flags[14];
   const nir_shader_compiler_options *options = shader->options;

   condition_flags[0] = true;
   condition_flags[1] = options->lower_flrp;
   condition_flags[2] = !options->lower_flrp;
   condition_flags[3] = options->lower_ffma;
   condition_flags[4] = !options->lower_ffma;
   condition_flags[5] = !options->lower_fsat;
   condition_flags[6] = options->lower_fsat;
   condition_flags[7] = options->lower_scmp;
   condition_flags[8] = options->lower_fpow;
   condition_flags[9] = !options->lower_fpow;
   condition_flags[10] = options->lower_fsqrt;
   condition_flags[11] = !options->lower_fsqrt;
   condition_flags[12] = options->lower_sub;
   condition_flags[13] = options->lower_negate;

   nir_foreach_overload(shader, overload) {
      if (overload->impl)
         progress |= nir_opt_algebraic_late_impl(overload->impl, condition_flags);
   }

   return progress;
}


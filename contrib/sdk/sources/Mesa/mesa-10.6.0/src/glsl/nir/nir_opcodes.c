
#include "nir.h"

const nir_op_info nir_op_infos[nir_num_opcodes] = {
{
   .name = "b2f",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_bool
   },
   .algebraic_properties =
      0
},
{
   .name = "b2i",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_int,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_bool
   },
   .algebraic_properties =
      0
},
{
   .name = "ball2",
   .num_inputs = 1,
   .output_size = 1,
   .output_type = nir_type_bool,
   .input_sizes = {
      2
   },
   .input_types = {
      nir_type_bool
   },
   .algebraic_properties =
      0
},
{
   .name = "ball3",
   .num_inputs = 1,
   .output_size = 1,
   .output_type = nir_type_bool,
   .input_sizes = {
      3
   },
   .input_types = {
      nir_type_bool
   },
   .algebraic_properties =
      0
},
{
   .name = "ball4",
   .num_inputs = 1,
   .output_size = 1,
   .output_type = nir_type_bool,
   .input_sizes = {
      4
   },
   .input_types = {
      nir_type_bool
   },
   .algebraic_properties =
      0
},
{
   .name = "ball_fequal2",
   .num_inputs = 2,
   .output_size = 1,
   .output_type = nir_type_bool,
   .input_sizes = {
      2, 2
   },
   .input_types = {
      nir_type_float, nir_type_float
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE
},
{
   .name = "ball_fequal3",
   .num_inputs = 2,
   .output_size = 1,
   .output_type = nir_type_bool,
   .input_sizes = {
      3, 3
   },
   .input_types = {
      nir_type_float, nir_type_float
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE
},
{
   .name = "ball_fequal4",
   .num_inputs = 2,
   .output_size = 1,
   .output_type = nir_type_bool,
   .input_sizes = {
      4, 4
   },
   .input_types = {
      nir_type_float, nir_type_float
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE
},
{
   .name = "ball_iequal2",
   .num_inputs = 2,
   .output_size = 1,
   .output_type = nir_type_bool,
   .input_sizes = {
      2, 2
   },
   .input_types = {
      nir_type_int, nir_type_int
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE
},
{
   .name = "ball_iequal3",
   .num_inputs = 2,
   .output_size = 1,
   .output_type = nir_type_bool,
   .input_sizes = {
      3, 3
   },
   .input_types = {
      nir_type_int, nir_type_int
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE
},
{
   .name = "ball_iequal4",
   .num_inputs = 2,
   .output_size = 1,
   .output_type = nir_type_bool,
   .input_sizes = {
      4, 4
   },
   .input_types = {
      nir_type_int, nir_type_int
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE
},
{
   .name = "bany2",
   .num_inputs = 1,
   .output_size = 1,
   .output_type = nir_type_bool,
   .input_sizes = {
      2
   },
   .input_types = {
      nir_type_bool
   },
   .algebraic_properties =
      0
},
{
   .name = "bany3",
   .num_inputs = 1,
   .output_size = 1,
   .output_type = nir_type_bool,
   .input_sizes = {
      3
   },
   .input_types = {
      nir_type_bool
   },
   .algebraic_properties =
      0
},
{
   .name = "bany4",
   .num_inputs = 1,
   .output_size = 1,
   .output_type = nir_type_bool,
   .input_sizes = {
      4
   },
   .input_types = {
      nir_type_bool
   },
   .algebraic_properties =
      0
},
{
   .name = "bany_fnequal2",
   .num_inputs = 2,
   .output_size = 1,
   .output_type = nir_type_bool,
   .input_sizes = {
      2, 2
   },
   .input_types = {
      nir_type_float, nir_type_float
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE
},
{
   .name = "bany_fnequal3",
   .num_inputs = 2,
   .output_size = 1,
   .output_type = nir_type_bool,
   .input_sizes = {
      3, 3
   },
   .input_types = {
      nir_type_float, nir_type_float
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE
},
{
   .name = "bany_fnequal4",
   .num_inputs = 2,
   .output_size = 1,
   .output_type = nir_type_bool,
   .input_sizes = {
      4, 4
   },
   .input_types = {
      nir_type_float, nir_type_float
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE
},
{
   .name = "bany_inequal2",
   .num_inputs = 2,
   .output_size = 1,
   .output_type = nir_type_bool,
   .input_sizes = {
      2, 2
   },
   .input_types = {
      nir_type_int, nir_type_int
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE
},
{
   .name = "bany_inequal3",
   .num_inputs = 2,
   .output_size = 1,
   .output_type = nir_type_bool,
   .input_sizes = {
      3, 3
   },
   .input_types = {
      nir_type_int, nir_type_int
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE
},
{
   .name = "bany_inequal4",
   .num_inputs = 2,
   .output_size = 1,
   .output_type = nir_type_bool,
   .input_sizes = {
      4, 4
   },
   .input_types = {
      nir_type_int, nir_type_int
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE
},
{
   .name = "bcsel",
   .num_inputs = 3,
   .output_size = 0,
   .output_type = nir_type_unsigned,
   .input_sizes = {
      0, 0, 0
   },
   .input_types = {
      nir_type_bool, nir_type_unsigned, nir_type_unsigned
   },
   .algebraic_properties =
      0
},
{
   .name = "bfi",
   .num_inputs = 3,
   .output_size = 0,
   .output_type = nir_type_unsigned,
   .input_sizes = {
      0, 0, 0
   },
   .input_types = {
      nir_type_unsigned, nir_type_unsigned, nir_type_unsigned
   },
   .algebraic_properties =
      0
},
{
   .name = "bfm",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_unsigned,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_int, nir_type_int
   },
   .algebraic_properties =
      0
},
{
   .name = "bit_count",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_unsigned,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_unsigned
   },
   .algebraic_properties =
      0
},
{
   .name = "bitfield_insert",
   .num_inputs = 4,
   .output_size = 0,
   .output_type = nir_type_unsigned,
   .input_sizes = {
      0, 0, 1, 1
   },
   .input_types = {
      nir_type_unsigned, nir_type_unsigned, nir_type_int, nir_type_int
   },
   .algebraic_properties =
      0
},
{
   .name = "bitfield_reverse",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_unsigned,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_unsigned
   },
   .algebraic_properties =
      0
},
{
   .name = "f2b",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_bool,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "f2i",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_int,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "f2u",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_unsigned,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fabs",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fadd",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_float, nir_type_float
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE | NIR_OP_IS_ASSOCIATIVE
},
{
   .name = "fall2",
   .num_inputs = 1,
   .output_size = 1,
   .output_type = nir_type_float,
   .input_sizes = {
      2
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fall3",
   .num_inputs = 1,
   .output_size = 1,
   .output_type = nir_type_float,
   .input_sizes = {
      3
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fall4",
   .num_inputs = 1,
   .output_size = 1,
   .output_type = nir_type_float,
   .input_sizes = {
      4
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fall_equal2",
   .num_inputs = 2,
   .output_size = 1,
   .output_type = nir_type_float,
   .input_sizes = {
      2, 2
   },
   .input_types = {
      nir_type_float, nir_type_float
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE
},
{
   .name = "fall_equal3",
   .num_inputs = 2,
   .output_size = 1,
   .output_type = nir_type_float,
   .input_sizes = {
      3, 3
   },
   .input_types = {
      nir_type_float, nir_type_float
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE
},
{
   .name = "fall_equal4",
   .num_inputs = 2,
   .output_size = 1,
   .output_type = nir_type_float,
   .input_sizes = {
      4, 4
   },
   .input_types = {
      nir_type_float, nir_type_float
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE
},
{
   .name = "fand",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_float, nir_type_float
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE
},
{
   .name = "fany2",
   .num_inputs = 1,
   .output_size = 1,
   .output_type = nir_type_float,
   .input_sizes = {
      2
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fany3",
   .num_inputs = 1,
   .output_size = 1,
   .output_type = nir_type_float,
   .input_sizes = {
      3
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fany4",
   .num_inputs = 1,
   .output_size = 1,
   .output_type = nir_type_float,
   .input_sizes = {
      4
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fany_nequal2",
   .num_inputs = 2,
   .output_size = 1,
   .output_type = nir_type_float,
   .input_sizes = {
      2, 2
   },
   .input_types = {
      nir_type_float, nir_type_float
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE
},
{
   .name = "fany_nequal3",
   .num_inputs = 2,
   .output_size = 1,
   .output_type = nir_type_float,
   .input_sizes = {
      3, 3
   },
   .input_types = {
      nir_type_float, nir_type_float
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE
},
{
   .name = "fany_nequal4",
   .num_inputs = 2,
   .output_size = 1,
   .output_type = nir_type_float,
   .input_sizes = {
      4, 4
   },
   .input_types = {
      nir_type_float, nir_type_float
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE
},
{
   .name = "fceil",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fcos",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fcsel",
   .num_inputs = 3,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0, 0, 0
   },
   .input_types = {
      nir_type_float, nir_type_float, nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fddx",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fddx_coarse",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fddx_fine",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fddy",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fddy_coarse",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fddy_fine",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fdiv",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_float, nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fdot2",
   .num_inputs = 2,
   .output_size = 1,
   .output_type = nir_type_float,
   .input_sizes = {
      2, 2
   },
   .input_types = {
      nir_type_float, nir_type_float
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE
},
{
   .name = "fdot3",
   .num_inputs = 2,
   .output_size = 1,
   .output_type = nir_type_float,
   .input_sizes = {
      3, 3
   },
   .input_types = {
      nir_type_float, nir_type_float
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE
},
{
   .name = "fdot4",
   .num_inputs = 2,
   .output_size = 1,
   .output_type = nir_type_float,
   .input_sizes = {
      4, 4
   },
   .input_types = {
      nir_type_float, nir_type_float
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE
},
{
   .name = "feq",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_bool,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_float, nir_type_float
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE
},
{
   .name = "fexp2",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "ffloor",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "ffma",
   .num_inputs = 3,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0, 0, 0
   },
   .input_types = {
      nir_type_float, nir_type_float, nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "ffract",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fge",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_bool,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_float, nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "find_lsb",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_int,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_int
   },
   .algebraic_properties =
      0
},
{
   .name = "flog2",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "flrp",
   .num_inputs = 3,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0, 0, 0
   },
   .input_types = {
      nir_type_float, nir_type_float, nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "flt",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_bool,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_float, nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fmax",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_float, nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fmin",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_float, nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fmod",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_float, nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fmov",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fmul",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_float, nir_type_float
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE | NIR_OP_IS_ASSOCIATIVE
},
{
   .name = "fne",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_bool,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_float, nir_type_float
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE
},
{
   .name = "fneg",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fnoise1_1",
   .num_inputs = 1,
   .output_size = 1,
   .output_type = nir_type_float,
   .input_sizes = {
      1
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fnoise1_2",
   .num_inputs = 1,
   .output_size = 1,
   .output_type = nir_type_float,
   .input_sizes = {
      2
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fnoise1_3",
   .num_inputs = 1,
   .output_size = 1,
   .output_type = nir_type_float,
   .input_sizes = {
      3
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fnoise1_4",
   .num_inputs = 1,
   .output_size = 1,
   .output_type = nir_type_float,
   .input_sizes = {
      4
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fnoise2_1",
   .num_inputs = 1,
   .output_size = 2,
   .output_type = nir_type_float,
   .input_sizes = {
      1
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fnoise2_2",
   .num_inputs = 1,
   .output_size = 2,
   .output_type = nir_type_float,
   .input_sizes = {
      2
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fnoise2_3",
   .num_inputs = 1,
   .output_size = 2,
   .output_type = nir_type_float,
   .input_sizes = {
      3
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fnoise2_4",
   .num_inputs = 1,
   .output_size = 2,
   .output_type = nir_type_float,
   .input_sizes = {
      4
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fnoise3_1",
   .num_inputs = 1,
   .output_size = 3,
   .output_type = nir_type_float,
   .input_sizes = {
      1
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fnoise3_2",
   .num_inputs = 1,
   .output_size = 3,
   .output_type = nir_type_float,
   .input_sizes = {
      2
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fnoise3_3",
   .num_inputs = 1,
   .output_size = 3,
   .output_type = nir_type_float,
   .input_sizes = {
      3
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fnoise3_4",
   .num_inputs = 1,
   .output_size = 3,
   .output_type = nir_type_float,
   .input_sizes = {
      4
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fnoise4_1",
   .num_inputs = 1,
   .output_size = 4,
   .output_type = nir_type_float,
   .input_sizes = {
      1
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fnoise4_2",
   .num_inputs = 1,
   .output_size = 4,
   .output_type = nir_type_float,
   .input_sizes = {
      2
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fnoise4_3",
   .num_inputs = 1,
   .output_size = 4,
   .output_type = nir_type_float,
   .input_sizes = {
      3
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fnoise4_4",
   .num_inputs = 1,
   .output_size = 4,
   .output_type = nir_type_float,
   .input_sizes = {
      4
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fnot",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "for",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_float, nir_type_float
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE
},
{
   .name = "fpow",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_float, nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "frcp",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fround_even",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "frsq",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fsat",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fsign",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fsin",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fsqrt",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fsub",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_float, nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "ftrunc",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "fxor",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_float, nir_type_float
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE
},
{
   .name = "i2b",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_bool,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_int
   },
   .algebraic_properties =
      0
},
{
   .name = "i2f",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_int
   },
   .algebraic_properties =
      0
},
{
   .name = "iabs",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_int,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_int
   },
   .algebraic_properties =
      0
},
{
   .name = "iadd",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_int,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_int, nir_type_int
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE | NIR_OP_IS_ASSOCIATIVE
},
{
   .name = "iand",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_unsigned,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_unsigned, nir_type_unsigned
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE | NIR_OP_IS_ASSOCIATIVE
},
{
   .name = "ibitfield_extract",
   .num_inputs = 3,
   .output_size = 0,
   .output_type = nir_type_int,
   .input_sizes = {
      0, 1, 1
   },
   .input_types = {
      nir_type_int, nir_type_int, nir_type_int
   },
   .algebraic_properties =
      0
},
{
   .name = "idiv",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_int,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_int, nir_type_int
   },
   .algebraic_properties =
      0
},
{
   .name = "ieq",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_bool,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_int, nir_type_int
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE
},
{
   .name = "ifind_msb",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_int,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_int
   },
   .algebraic_properties =
      0
},
{
   .name = "ige",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_bool,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_int, nir_type_int
   },
   .algebraic_properties =
      0
},
{
   .name = "ilt",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_bool,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_int, nir_type_int
   },
   .algebraic_properties =
      0
},
{
   .name = "imax",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_int,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_int, nir_type_int
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE | NIR_OP_IS_ASSOCIATIVE
},
{
   .name = "imin",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_int,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_int, nir_type_int
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE | NIR_OP_IS_ASSOCIATIVE
},
{
   .name = "imov",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_int,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_int
   },
   .algebraic_properties =
      0
},
{
   .name = "imul",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_int,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_int, nir_type_int
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE | NIR_OP_IS_ASSOCIATIVE
},
{
   .name = "imul_high",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_int,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_int, nir_type_int
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE
},
{
   .name = "ine",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_bool,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_int, nir_type_int
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE
},
{
   .name = "ineg",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_int,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_int
   },
   .algebraic_properties =
      0
},
{
   .name = "inot",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_int,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_int
   },
   .algebraic_properties =
      0
},
{
   .name = "ior",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_unsigned,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_unsigned, nir_type_unsigned
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE | NIR_OP_IS_ASSOCIATIVE
},
{
   .name = "ishl",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_int,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_int, nir_type_int
   },
   .algebraic_properties =
      0
},
{
   .name = "ishr",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_int,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_int, nir_type_int
   },
   .algebraic_properties =
      0
},
{
   .name = "isign",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_int,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_int
   },
   .algebraic_properties =
      0
},
{
   .name = "isub",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_int,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_int, nir_type_int
   },
   .algebraic_properties =
      0
},
{
   .name = "ixor",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_unsigned,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_unsigned, nir_type_unsigned
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE | NIR_OP_IS_ASSOCIATIVE
},
{
   .name = "ldexp",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_float, nir_type_int
   },
   .algebraic_properties =
      0
},
{
   .name = "pack_half_2x16",
   .num_inputs = 1,
   .output_size = 1,
   .output_type = nir_type_unsigned,
   .input_sizes = {
      2
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "pack_half_2x16_split",
   .num_inputs = 2,
   .output_size = 1,
   .output_type = nir_type_unsigned,
   .input_sizes = {
      1, 1
   },
   .input_types = {
      nir_type_float, nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "pack_snorm_2x16",
   .num_inputs = 1,
   .output_size = 1,
   .output_type = nir_type_unsigned,
   .input_sizes = {
      2
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "pack_snorm_4x8",
   .num_inputs = 1,
   .output_size = 1,
   .output_type = nir_type_unsigned,
   .input_sizes = {
      4
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "pack_unorm_2x16",
   .num_inputs = 1,
   .output_size = 1,
   .output_type = nir_type_unsigned,
   .input_sizes = {
      2
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "pack_unorm_4x8",
   .num_inputs = 1,
   .output_size = 1,
   .output_type = nir_type_unsigned,
   .input_sizes = {
      4
   },
   .input_types = {
      nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "seq",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_float, nir_type_float
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE
},
{
   .name = "sge",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_float, nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "slt",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_float, nir_type_float
   },
   .algebraic_properties =
      0
},
{
   .name = "sne",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_float, nir_type_float
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE
},
{
   .name = "u2f",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_float,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_unsigned
   },
   .algebraic_properties =
      0
},
{
   .name = "uadd_carry",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_bool,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_unsigned, nir_type_unsigned
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE
},
{
   .name = "ubitfield_extract",
   .num_inputs = 3,
   .output_size = 0,
   .output_type = nir_type_unsigned,
   .input_sizes = {
      0, 1, 1
   },
   .input_types = {
      nir_type_unsigned, nir_type_int, nir_type_int
   },
   .algebraic_properties =
      0
},
{
   .name = "udiv",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_unsigned,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_unsigned, nir_type_unsigned
   },
   .algebraic_properties =
      0
},
{
   .name = "ufind_msb",
   .num_inputs = 1,
   .output_size = 0,
   .output_type = nir_type_int,
   .input_sizes = {
      0
   },
   .input_types = {
      nir_type_unsigned
   },
   .algebraic_properties =
      0
},
{
   .name = "uge",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_bool,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_unsigned, nir_type_unsigned
   },
   .algebraic_properties =
      0
},
{
   .name = "ult",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_bool,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_unsigned, nir_type_unsigned
   },
   .algebraic_properties =
      0
},
{
   .name = "umax",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_unsigned,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_unsigned, nir_type_unsigned
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE | NIR_OP_IS_ASSOCIATIVE
},
{
   .name = "umin",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_unsigned,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_unsigned, nir_type_unsigned
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE | NIR_OP_IS_ASSOCIATIVE
},
{
   .name = "umod",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_unsigned,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_unsigned, nir_type_unsigned
   },
   .algebraic_properties =
      0
},
{
   .name = "umul_high",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_unsigned,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_unsigned, nir_type_unsigned
   },
   .algebraic_properties =
      NIR_OP_IS_COMMUTATIVE
},
{
   .name = "unpack_half_2x16",
   .num_inputs = 1,
   .output_size = 2,
   .output_type = nir_type_float,
   .input_sizes = {
      1
   },
   .input_types = {
      nir_type_unsigned
   },
   .algebraic_properties =
      0
},
{
   .name = "unpack_half_2x16_split_x",
   .num_inputs = 1,
   .output_size = 1,
   .output_type = nir_type_float,
   .input_sizes = {
      1
   },
   .input_types = {
      nir_type_unsigned
   },
   .algebraic_properties =
      0
},
{
   .name = "unpack_half_2x16_split_y",
   .num_inputs = 1,
   .output_size = 1,
   .output_type = nir_type_float,
   .input_sizes = {
      1
   },
   .input_types = {
      nir_type_unsigned
   },
   .algebraic_properties =
      0
},
{
   .name = "unpack_snorm_2x16",
   .num_inputs = 1,
   .output_size = 2,
   .output_type = nir_type_float,
   .input_sizes = {
      1
   },
   .input_types = {
      nir_type_unsigned
   },
   .algebraic_properties =
      0
},
{
   .name = "unpack_snorm_4x8",
   .num_inputs = 1,
   .output_size = 4,
   .output_type = nir_type_float,
   .input_sizes = {
      1
   },
   .input_types = {
      nir_type_unsigned
   },
   .algebraic_properties =
      0
},
{
   .name = "unpack_unorm_2x16",
   .num_inputs = 1,
   .output_size = 2,
   .output_type = nir_type_float,
   .input_sizes = {
      1
   },
   .input_types = {
      nir_type_unsigned
   },
   .algebraic_properties =
      0
},
{
   .name = "unpack_unorm_4x8",
   .num_inputs = 1,
   .output_size = 4,
   .output_type = nir_type_float,
   .input_sizes = {
      1
   },
   .input_types = {
      nir_type_unsigned
   },
   .algebraic_properties =
      0
},
{
   .name = "ushr",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_unsigned,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_unsigned, nir_type_unsigned
   },
   .algebraic_properties =
      0
},
{
   .name = "usub_borrow",
   .num_inputs = 2,
   .output_size = 0,
   .output_type = nir_type_bool,
   .input_sizes = {
      0, 0
   },
   .input_types = {
      nir_type_unsigned, nir_type_unsigned
   },
   .algebraic_properties =
      0
},
{
   .name = "vec2",
   .num_inputs = 2,
   .output_size = 2,
   .output_type = nir_type_unsigned,
   .input_sizes = {
      1, 1
   },
   .input_types = {
      nir_type_unsigned, nir_type_unsigned
   },
   .algebraic_properties =
      0
},
{
   .name = "vec3",
   .num_inputs = 3,
   .output_size = 3,
   .output_type = nir_type_unsigned,
   .input_sizes = {
      1, 1, 1
   },
   .input_types = {
      nir_type_unsigned, nir_type_unsigned, nir_type_unsigned
   },
   .algebraic_properties =
      0
},
{
   .name = "vec4",
   .num_inputs = 4,
   .output_size = 4,
   .output_type = nir_type_unsigned,
   .input_sizes = {
      1, 1, 1, 1
   },
   .input_types = {
      nir_type_unsigned, nir_type_unsigned, nir_type_unsigned, nir_type_unsigned
   },
   .algebraic_properties =
      0
},
};


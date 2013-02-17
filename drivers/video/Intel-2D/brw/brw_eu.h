/*
   Copyright (C) Intel Corp.  2006.  All Rights Reserved.
   Intel funded Tungsten Graphics (http://www.tungstengraphics.com) to
   develop this 3D driver.

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   "Software"), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice (including the
   next paragraph) shall be included in all copies or substantial
   portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL THE COPYRIGHT OWNER(S) AND/OR ITS SUPPLIERS BE
   LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
   OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
   WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 **********************************************************************/
/*
 * Authors:
 *   Keith Whitwell <keith@tungstengraphics.com>
 */


#ifndef BRW_EU_H
#define BRW_EU_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#define BRW_SWIZZLE4(a,b,c,d) (((a)<<0) | ((b)<<2) | ((c)<<4) | ((d)<<6))
#define BRW_GET_SWZ(swz, idx) (((swz) >> ((idx)*2)) & 0x3)

#define BRW_SWIZZLE_NOOP      BRW_SWIZZLE4(0,1,2,3)
#define BRW_SWIZZLE_XYZW      BRW_SWIZZLE4(0,1,2,3)
#define BRW_SWIZZLE_XXXX      BRW_SWIZZLE4(0,0,0,0)
#define BRW_SWIZZLE_YYYY      BRW_SWIZZLE4(1,1,1,1)
#define BRW_SWIZZLE_ZZZZ      BRW_SWIZZLE4(2,2,2,2)
#define BRW_SWIZZLE_WWWW      BRW_SWIZZLE4(3,3,3,3)
#define BRW_SWIZZLE_XYXY      BRW_SWIZZLE4(0,1,0,1)

#define WRITEMASK_X 0x1
#define WRITEMASK_Y 0x2
#define WRITEMASK_Z 0x4
#define WRITEMASK_W 0x8

#define WRITEMASK_XY (WRITEMASK_X | WRITEMASK_Y)
#define WRITEMASK_XYZ (WRITEMASK_X | WRITEMASK_Y | WRITEMASK_Z)
#define WRITEMASK_XYZW (WRITEMASK_X | WRITEMASK_Y | WRITEMASK_Z | WRITEMASK_W)

/** Number of general purpose registers (VS, WM, etc) */
#define BRW_MAX_GRF 128

/** Number of message register file registers */
#define BRW_MAX_MRF 16


#define BRW_ALIGN_1   0
#define BRW_ALIGN_16  1

#define BRW_ADDRESS_DIRECT                        0
#define BRW_ADDRESS_REGISTER_INDIRECT_REGISTER    1

#define BRW_CHANNEL_X     0
#define BRW_CHANNEL_Y     1
#define BRW_CHANNEL_Z     2
#define BRW_CHANNEL_W     3

enum brw_compression {
	BRW_COMPRESSION_NONE,
	BRW_COMPRESSION_2NDHALF,
	BRW_COMPRESSION_COMPRESSED,
};

#define GEN6_COMPRESSION_1Q		0
#define GEN6_COMPRESSION_2Q		1
#define GEN6_COMPRESSION_3Q		2
#define GEN6_COMPRESSION_4Q		3
#define GEN6_COMPRESSION_1H		0
#define GEN6_COMPRESSION_2H		2

#define BRW_CONDITIONAL_NONE  0
#define BRW_CONDITIONAL_Z     1
#define BRW_CONDITIONAL_NZ    2
#define BRW_CONDITIONAL_EQ    1	/* Z */
#define BRW_CONDITIONAL_NEQ   2	/* NZ */
#define BRW_CONDITIONAL_G     3
#define BRW_CONDITIONAL_GE    4
#define BRW_CONDITIONAL_L     5
#define BRW_CONDITIONAL_LE    6
#define BRW_CONDITIONAL_R     7
#define BRW_CONDITIONAL_O     8
#define BRW_CONDITIONAL_U     9

#define BRW_DEBUG_NONE        0
#define BRW_DEBUG_BREAKPOINT  1

#define BRW_DEPENDENCY_NORMAL         0
#define BRW_DEPENDENCY_NOTCLEARED     1
#define BRW_DEPENDENCY_NOTCHECKED     2
#define BRW_DEPENDENCY_DISABLE        3

#define BRW_EXECUTE_1     0
#define BRW_EXECUTE_2     1
#define BRW_EXECUTE_4     2
#define BRW_EXECUTE_8     3
#define BRW_EXECUTE_16    4
#define BRW_EXECUTE_32    5

#define BRW_HORIZONTAL_STRIDE_0   0
#define BRW_HORIZONTAL_STRIDE_1   1
#define BRW_HORIZONTAL_STRIDE_2   2
#define BRW_HORIZONTAL_STRIDE_4   3

#define BRW_INSTRUCTION_NORMAL    0
#define BRW_INSTRUCTION_SATURATE  1

#define BRW_MASK_ENABLE   0
#define BRW_MASK_DISABLE  1

/** @{
 *
 * Gen6 has replaced "mask enable/disable" with WECtrl, which is
 * effectively the same but much simpler to think about.  Now, there
 * are two contributors ANDed together to whether channels are
 * executed: The predication on the instruction, and the channel write
 * enable.
 */
/**
 * This is the default value.  It means that a channel's write enable is set
 * if the per-channel IP is pointing at this instruction.
 */
#define BRW_WE_NORMAL		0
/**
 * This is used like BRW_MASK_DISABLE, and causes all channels to have
 * their write enable set.  Note that predication still contributes to
 * whether the channel actually gets written.
 */
#define BRW_WE_ALL		1
/** @} */

enum opcode {
	/* These are the actual hardware opcodes. */
	BRW_OPCODE_MOV =	1,
	BRW_OPCODE_SEL =	2,
	BRW_OPCODE_NOT =	4,
	BRW_OPCODE_AND =	5,
	BRW_OPCODE_OR =	6,
	BRW_OPCODE_XOR =	7,
	BRW_OPCODE_SHR =	8,
	BRW_OPCODE_SHL =	9,
	BRW_OPCODE_RSR =	10,
	BRW_OPCODE_RSL =	11,
	BRW_OPCODE_ASR =	12,
	BRW_OPCODE_CMP =	16,
	BRW_OPCODE_CMPN =	17,
	BRW_OPCODE_JMPI =	32,
	BRW_OPCODE_IF =	34,
	BRW_OPCODE_IFF =	35,
	BRW_OPCODE_ELSE =	36,
	BRW_OPCODE_ENDIF =	37,
	BRW_OPCODE_DO =	38,
	BRW_OPCODE_WHILE =	39,
	BRW_OPCODE_BREAK =	40,
	BRW_OPCODE_CONTINUE = 41,
	BRW_OPCODE_HALT =	42,
	BRW_OPCODE_MSAVE =	44,
	BRW_OPCODE_MRESTORE = 45,
	BRW_OPCODE_PUSH =	46,
	BRW_OPCODE_POP =	47,
	BRW_OPCODE_WAIT =	48,
	BRW_OPCODE_SEND =	49,
	BRW_OPCODE_SENDC =	50,
	BRW_OPCODE_MATH =	56,
	BRW_OPCODE_ADD =	64,
	BRW_OPCODE_MUL =	65,
	BRW_OPCODE_AVG =	66,
	BRW_OPCODE_FRC =	67,
	BRW_OPCODE_RNDU =	68,
	BRW_OPCODE_RNDD =	69,
	BRW_OPCODE_RNDE =	70,
	BRW_OPCODE_RNDZ =	71,
	BRW_OPCODE_MAC =	72,
	BRW_OPCODE_MACH =	73,
	BRW_OPCODE_LZD =	74,
	BRW_OPCODE_SAD2 =	80,
	BRW_OPCODE_SADA2 =	81,
	BRW_OPCODE_DP4 =	84,
	BRW_OPCODE_DPH =	85,
	BRW_OPCODE_DP3 =	86,
	BRW_OPCODE_DP2 =	87,
	BRW_OPCODE_DPA2 =	88,
	BRW_OPCODE_LINE =	89,
	BRW_OPCODE_PLN =	90,
	BRW_OPCODE_NOP =	126,

	/* These are compiler backend opcodes that get translated into other
	 * instructions.
	 */
	FS_OPCODE_FB_WRITE = 128,
	SHADER_OPCODE_RCP,
	SHADER_OPCODE_RSQ,
	SHADER_OPCODE_SQRT,
	SHADER_OPCODE_EXP2,
	SHADER_OPCODE_LOG2,
	SHADER_OPCODE_POW,
	SHADER_OPCODE_SIN,
	SHADER_OPCODE_COS,
	FS_OPCODE_DDX,
	FS_OPCODE_DDY,
	FS_OPCODE_PIXEL_X,
	FS_OPCODE_PIXEL_Y,
	FS_OPCODE_CINTERP,
	FS_OPCODE_LINTERP,
	FS_OPCODE_TEX,
	FS_OPCODE_TXB,
	FS_OPCODE_TXD,
	FS_OPCODE_TXF,
	FS_OPCODE_TXL,
	FS_OPCODE_TXS,
	FS_OPCODE_DISCARD,
	FS_OPCODE_SPILL,
	FS_OPCODE_UNSPILL,
	FS_OPCODE_PULL_CONSTANT_LOAD,

	VS_OPCODE_URB_WRITE,
	VS_OPCODE_SCRATCH_READ,
	VS_OPCODE_SCRATCH_WRITE,
	VS_OPCODE_PULL_CONSTANT_LOAD,
};

#define BRW_PREDICATE_NONE             0
#define BRW_PREDICATE_NORMAL           1
#define BRW_PREDICATE_ALIGN1_ANYV             2
#define BRW_PREDICATE_ALIGN1_ALLV             3
#define BRW_PREDICATE_ALIGN1_ANY2H            4
#define BRW_PREDICATE_ALIGN1_ALL2H            5
#define BRW_PREDICATE_ALIGN1_ANY4H            6
#define BRW_PREDICATE_ALIGN1_ALL4H            7
#define BRW_PREDICATE_ALIGN1_ANY8H            8
#define BRW_PREDICATE_ALIGN1_ALL8H            9
#define BRW_PREDICATE_ALIGN1_ANY16H           10
#define BRW_PREDICATE_ALIGN1_ALL16H           11
#define BRW_PREDICATE_ALIGN16_REPLICATE_X     2
#define BRW_PREDICATE_ALIGN16_REPLICATE_Y     3
#define BRW_PREDICATE_ALIGN16_REPLICATE_Z     4
#define BRW_PREDICATE_ALIGN16_REPLICATE_W     5
#define BRW_PREDICATE_ALIGN16_ANY4H           6
#define BRW_PREDICATE_ALIGN16_ALL4H           7

#define BRW_ARCHITECTURE_REGISTER_FILE    0
#define BRW_GENERAL_REGISTER_FILE         1
#define BRW_MESSAGE_REGISTER_FILE         2
#define BRW_IMMEDIATE_VALUE               3

#define BRW_REGISTER_TYPE_UD  0
#define BRW_REGISTER_TYPE_D   1
#define BRW_REGISTER_TYPE_UW  2
#define BRW_REGISTER_TYPE_W   3
#define BRW_REGISTER_TYPE_UB  4
#define BRW_REGISTER_TYPE_B   5
#define BRW_REGISTER_TYPE_VF  5	/* packed float vector, immediates only? */
#define BRW_REGISTER_TYPE_HF  6
#define BRW_REGISTER_TYPE_V   6	/* packed int vector, immediates only, uword dest only */
#define BRW_REGISTER_TYPE_F   7

#define BRW_ARF_NULL                  0x00
#define BRW_ARF_ADDRESS               0x10
#define BRW_ARF_ACCUMULATOR           0x20
#define BRW_ARF_FLAG                  0x30
#define BRW_ARF_MASK                  0x40
#define BRW_ARF_MASK_STACK            0x50
#define BRW_ARF_MASK_STACK_DEPTH      0x60
#define BRW_ARF_STATE                 0x70
#define BRW_ARF_CONTROL               0x80
#define BRW_ARF_NOTIFICATION_COUNT    0x90
#define BRW_ARF_IP                    0xA0

#define BRW_MRF_COMPR4			(1 << 7)

#define BRW_AMASK   0
#define BRW_IMASK   1
#define BRW_LMASK   2
#define BRW_CMASK   3

#define BRW_THREAD_NORMAL     0
#define BRW_THREAD_ATOMIC     1
#define BRW_THREAD_SWITCH     2

#define BRW_VERTICAL_STRIDE_0                 0
#define BRW_VERTICAL_STRIDE_1                 1
#define BRW_VERTICAL_STRIDE_2                 2
#define BRW_VERTICAL_STRIDE_4                 3
#define BRW_VERTICAL_STRIDE_8                 4
#define BRW_VERTICAL_STRIDE_16                5
#define BRW_VERTICAL_STRIDE_32                6
#define BRW_VERTICAL_STRIDE_64                7
#define BRW_VERTICAL_STRIDE_128               8
#define BRW_VERTICAL_STRIDE_256               9
#define BRW_VERTICAL_STRIDE_ONE_DIMENSIONAL   0xF

#define BRW_WIDTH_1       0
#define BRW_WIDTH_2       1
#define BRW_WIDTH_4       2
#define BRW_WIDTH_8       3
#define BRW_WIDTH_16      4

#define BRW_STATELESS_BUFFER_BOUNDARY_1K      0
#define BRW_STATELESS_BUFFER_BOUNDARY_2K      1
#define BRW_STATELESS_BUFFER_BOUNDARY_4K      2
#define BRW_STATELESS_BUFFER_BOUNDARY_8K      3
#define BRW_STATELESS_BUFFER_BOUNDARY_16K     4
#define BRW_STATELESS_BUFFER_BOUNDARY_32K     5
#define BRW_STATELESS_BUFFER_BOUNDARY_64K     6
#define BRW_STATELESS_BUFFER_BOUNDARY_128K    7
#define BRW_STATELESS_BUFFER_BOUNDARY_256K    8
#define BRW_STATELESS_BUFFER_BOUNDARY_512K    9
#define BRW_STATELESS_BUFFER_BOUNDARY_1M      10
#define BRW_STATELESS_BUFFER_BOUNDARY_2M      11

#define BRW_POLYGON_FACING_FRONT      0
#define BRW_POLYGON_FACING_BACK       1

#define BRW_MESSAGE_TARGET_NULL               0
#define BRW_MESSAGE_TARGET_MATH               1 /* reserved on GEN6 */
#define BRW_MESSAGE_TARGET_SAMPLER            2
#define BRW_MESSAGE_TARGET_GATEWAY            3
#define BRW_MESSAGE_TARGET_DATAPORT_READ      4
#define BRW_MESSAGE_TARGET_DATAPORT_WRITE     5
#define BRW_MESSAGE_TARGET_URB                6
#define BRW_MESSAGE_TARGET_THREAD_SPAWNER     7

#define GEN6_MESSAGE_TARGET_DP_SAMPLER_CACHE  4
#define GEN6_MESSAGE_TARGET_DP_RENDER_CACHE   5
#define GEN6_MESSAGE_TARGET_DP_CONST_CACHE    9

#define BRW_SAMPLER_RETURN_FORMAT_FLOAT32     0
#define BRW_SAMPLER_RETURN_FORMAT_UINT32      2
#define BRW_SAMPLER_RETURN_FORMAT_SINT32      3

#define BRW_SAMPLER_MESSAGE_SAMPLE	              0
#define BRW_SAMPLER_MESSAGE_SIMD8_SAMPLE              0
#define BRW_SAMPLER_MESSAGE_SIMD16_SAMPLE             0
#define BRW_SAMPLER_MESSAGE_SIMD16_SAMPLE_BIAS        0
#define BRW_SAMPLER_MESSAGE_SIMD8_KILLPIX             1
#define BRW_SAMPLER_MESSAGE_SIMD4X2_SAMPLE_LOD        1
#define BRW_SAMPLER_MESSAGE_SIMD16_SAMPLE_LOD         1
#define BRW_SAMPLER_MESSAGE_SIMD4X2_SAMPLE_GRADIENTS  2
#define BRW_SAMPLER_MESSAGE_SIMD8_SAMPLE_GRADIENTS    2
#define BRW_SAMPLER_MESSAGE_SIMD4X2_SAMPLE_COMPARE    0
#define BRW_SAMPLER_MESSAGE_SIMD16_SAMPLE_COMPARE     2
#define BRW_SAMPLER_MESSAGE_SIMD8_SAMPLE_BIAS_COMPARE 0
#define BRW_SAMPLER_MESSAGE_SIMD8_SAMPLE_LOD_COMPARE  1
#define BRW_SAMPLER_MESSAGE_SIMD4X2_RESINFO           2
#define BRW_SAMPLER_MESSAGE_SIMD16_RESINFO            2
#define BRW_SAMPLER_MESSAGE_SIMD4X2_LD                3
#define BRW_SAMPLER_MESSAGE_SIMD8_LD                  3
#define BRW_SAMPLER_MESSAGE_SIMD16_LD                 3

#define GEN5_SAMPLER_MESSAGE_SAMPLE              0
#define GEN5_SAMPLER_MESSAGE_SAMPLE_BIAS         1
#define GEN5_SAMPLER_MESSAGE_SAMPLE_LOD          2
#define GEN5_SAMPLER_MESSAGE_SAMPLE_COMPARE      3
#define GEN5_SAMPLER_MESSAGE_SAMPLE_DERIVS       4
#define GEN5_SAMPLER_MESSAGE_SAMPLE_BIAS_COMPARE 5
#define GEN5_SAMPLER_MESSAGE_SAMPLE_LOD_COMPARE  6
#define GEN5_SAMPLER_MESSAGE_SAMPLE_LD           7
#define GEN5_SAMPLER_MESSAGE_SAMPLE_RESINFO      10

/* for GEN5 only */
#define BRW_SAMPLER_SIMD_MODE_SIMD4X2                   0
#define BRW_SAMPLER_SIMD_MODE_SIMD8                     1
#define BRW_SAMPLER_SIMD_MODE_SIMD16                    2
#define BRW_SAMPLER_SIMD_MODE_SIMD32_64                 3

#define BRW_DATAPORT_OWORD_BLOCK_1_OWORDLOW   0
#define BRW_DATAPORT_OWORD_BLOCK_1_OWORDHIGH  1
#define BRW_DATAPORT_OWORD_BLOCK_2_OWORDS     2
#define BRW_DATAPORT_OWORD_BLOCK_4_OWORDS     3
#define BRW_DATAPORT_OWORD_BLOCK_8_OWORDS     4

#define BRW_DATAPORT_OWORD_DUAL_BLOCK_1OWORD     0
#define BRW_DATAPORT_OWORD_DUAL_BLOCK_4OWORDS    2

#define BRW_DATAPORT_DWORD_SCATTERED_BLOCK_8DWORDS   2
#define BRW_DATAPORT_DWORD_SCATTERED_BLOCK_16DWORDS  3

/* This one stays the same across generations. */
#define BRW_DATAPORT_READ_MESSAGE_OWORD_BLOCK_READ          0
/* GEN4 */
#define BRW_DATAPORT_READ_MESSAGE_OWORD_DUAL_BLOCK_READ     1
#define BRW_DATAPORT_READ_MESSAGE_MEDIA_BLOCK_READ          2
#define BRW_DATAPORT_READ_MESSAGE_DWORD_SCATTERED_READ      3
/* G45, GEN5 */
#define G45_DATAPORT_READ_MESSAGE_RENDER_UNORM_READ	    1
#define G45_DATAPORT_READ_MESSAGE_OWORD_DUAL_BLOCK_READ     2
#define G45_DATAPORT_READ_MESSAGE_AVC_LOOP_FILTER_READ	    3
#define G45_DATAPORT_READ_MESSAGE_MEDIA_BLOCK_READ          4
#define G45_DATAPORT_READ_MESSAGE_DWORD_SCATTERED_READ      6
/* GEN6 */
#define GEN6_DATAPORT_READ_MESSAGE_RENDER_UNORM_READ	    1
#define GEN6_DATAPORT_READ_MESSAGE_OWORD_DUAL_BLOCK_READ     2
#define GEN6_DATAPORT_READ_MESSAGE_MEDIA_BLOCK_READ          4
#define GEN6_DATAPORT_READ_MESSAGE_OWORD_UNALIGN_BLOCK_READ  5
#define GEN6_DATAPORT_READ_MESSAGE_DWORD_SCATTERED_READ      6

#define BRW_DATAPORT_READ_TARGET_DATA_CACHE      0
#define BRW_DATAPORT_READ_TARGET_RENDER_CACHE    1
#define BRW_DATAPORT_READ_TARGET_SAMPLER_CACHE   2

#define BRW_DATAPORT_RENDER_TARGET_WRITE_SIMD16_SINGLE_SOURCE                0
#define BRW_DATAPORT_RENDER_TARGET_WRITE_SIMD16_SINGLE_SOURCE_REPLICATED     1
#define BRW_DATAPORT_RENDER_TARGET_WRITE_SIMD8_DUAL_SOURCE_SUBSPAN01         2
#define BRW_DATAPORT_RENDER_TARGET_WRITE_SIMD8_DUAL_SOURCE_SUBSPAN23         3
#define BRW_DATAPORT_RENDER_TARGET_WRITE_SIMD8_SINGLE_SOURCE_SUBSPAN01       4

/**
 * Message target: Shared Function ID for where to SEND a message.
 *
 * These are enumerated in the ISA reference under "send - Send Message".
 * In particular, see the following tables:
 * - G45 PRM, Volume 4, Table 14-15 "Message Descriptor Definition"
 * - Sandybridge PRM, Volume 4 Part 2, Table 8-16 "Extended Message Descriptor"
 * - BSpec, Volume 1a (GPU Overview) / Graphics Processing Engine (GPE) /
 *   Overview / GPE Function IDs
 */
enum brw_message_target {
   BRW_SFID_NULL                     = 0,
   BRW_SFID_MATH                     = 1, /* Only valid on Gen4-5 */
   BRW_SFID_SAMPLER                  = 2,
   BRW_SFID_MESSAGE_GATEWAY          = 3,
   BRW_SFID_DATAPORT_READ            = 4,
   BRW_SFID_DATAPORT_WRITE           = 5,
   BRW_SFID_URB                      = 6,
   BRW_SFID_THREAD_SPAWNER           = 7,

   GEN6_SFID_DATAPORT_SAMPLER_CACHE  = 4,
   GEN6_SFID_DATAPORT_RENDER_CACHE   = 5,
   GEN6_SFID_DATAPORT_CONSTANT_CACHE = 9,

   GEN7_SFID_DATAPORT_DATA_CACHE     = 10,
};

#define GEN7_MESSAGE_TARGET_DP_DATA_CACHE     10

#define BRW_DATAPORT_WRITE_MESSAGE_OWORD_BLOCK_WRITE                0
#define BRW_DATAPORT_WRITE_MESSAGE_OWORD_DUAL_BLOCK_WRITE           1
#define BRW_DATAPORT_WRITE_MESSAGE_MEDIA_BLOCK_WRITE                2
#define BRW_DATAPORT_WRITE_MESSAGE_DWORD_SCATTERED_WRITE            3
#define BRW_DATAPORT_WRITE_MESSAGE_RENDER_TARGET_WRITE              4
#define BRW_DATAPORT_WRITE_MESSAGE_STREAMED_VERTEX_BUFFER_WRITE     5
#define BRW_DATAPORT_WRITE_MESSAGE_FLUSH_RENDER_CACHE               7

/* GEN6 */
#define GEN6_DATAPORT_WRITE_MESSAGE_DWORD_ATOMIC_WRITE              7
#define GEN6_DATAPORT_WRITE_MESSAGE_OWORD_BLOCK_WRITE               8
#define GEN6_DATAPORT_WRITE_MESSAGE_OWORD_DUAL_BLOCK_WRITE          9
#define GEN6_DATAPORT_WRITE_MESSAGE_MEDIA_BLOCK_WRITE               10
#define GEN6_DATAPORT_WRITE_MESSAGE_DWORD_SCATTERED_WRITE           11
#define GEN6_DATAPORT_WRITE_MESSAGE_RENDER_TARGET_WRITE             12
#define GEN6_DATAPORT_WRITE_MESSAGE_STREAMED_VB_WRITE               13
#define GEN6_DATAPORT_WRITE_MESSAGE_RENDER_TARGET_UNORM_WRITE       14

#define BRW_MATH_FUNCTION_INV                              1
#define BRW_MATH_FUNCTION_LOG                              2
#define BRW_MATH_FUNCTION_EXP                              3
#define BRW_MATH_FUNCTION_SQRT                             4
#define BRW_MATH_FUNCTION_RSQ                              5
#define BRW_MATH_FUNCTION_SIN                              6 /* was 7 */
#define BRW_MATH_FUNCTION_COS                              7 /* was 8 */
#define BRW_MATH_FUNCTION_SINCOS                           8 /* was 6 */
#define BRW_MATH_FUNCTION_TAN                              9 /* gen4 */
#define BRW_MATH_FUNCTION_FDIV                             9 /* gen6+ */
#define BRW_MATH_FUNCTION_POW                              10
#define BRW_MATH_FUNCTION_INT_DIV_QUOTIENT_AND_REMAINDER   11
#define BRW_MATH_FUNCTION_INT_DIV_QUOTIENT                 12
#define BRW_MATH_FUNCTION_INT_DIV_REMAINDER                13

#define BRW_MATH_INTEGER_UNSIGNED     0
#define BRW_MATH_INTEGER_SIGNED       1

#define BRW_MATH_PRECISION_FULL        0
#define BRW_MATH_PRECISION_PARTIAL     1

#define BRW_MATH_SATURATE_NONE         0
#define BRW_MATH_SATURATE_SATURATE     1

#define BRW_MATH_DATA_VECTOR  0
#define BRW_MATH_DATA_SCALAR  1

#define BRW_URB_OPCODE_WRITE  0

#define BRW_URB_SWIZZLE_NONE          0
#define BRW_URB_SWIZZLE_INTERLEAVE    1
#define BRW_URB_SWIZZLE_TRANSPOSE     2

#define BRW_SCRATCH_SPACE_SIZE_1K     0
#define BRW_SCRATCH_SPACE_SIZE_2K     1
#define BRW_SCRATCH_SPACE_SIZE_4K     2
#define BRW_SCRATCH_SPACE_SIZE_8K     3
#define BRW_SCRATCH_SPACE_SIZE_16K    4
#define BRW_SCRATCH_SPACE_SIZE_32K    5
#define BRW_SCRATCH_SPACE_SIZE_64K    6
#define BRW_SCRATCH_SPACE_SIZE_128K   7
#define BRW_SCRATCH_SPACE_SIZE_256K   8
#define BRW_SCRATCH_SPACE_SIZE_512K   9
#define BRW_SCRATCH_SPACE_SIZE_1M     10
#define BRW_SCRATCH_SPACE_SIZE_2M     11

#define REG_SIZE (8*4)

struct brw_instruction {
	struct {
		unsigned opcode:7;
		unsigned pad:1;
		unsigned access_mode:1;
		unsigned mask_control:1;
		unsigned dependency_control:2;
		unsigned compression_control:2; /* gen6: quater control */
		unsigned thread_control:2;
		unsigned predicate_control:4;
		unsigned predicate_inverse:1;
		unsigned execution_size:3;
		/**
		 * Conditional Modifier for most instructions.  On Gen6+, this is also
		 * used for the SEND instruction's Message Target/SFID.
		 */
		unsigned destreg__conditionalmod:4;
		unsigned acc_wr_control:1;
		unsigned cmpt_control:1;
		unsigned debug_control:1;
		unsigned saturate:1;
	} header;

	union {
		struct {
			unsigned dest_reg_file:2;
			unsigned dest_reg_type:3;
			unsigned src0_reg_file:2;
			unsigned src0_reg_type:3;
			unsigned src1_reg_file:2;
			unsigned src1_reg_type:3;
			unsigned pad:1;
			unsigned dest_subreg_nr:5;
			unsigned dest_reg_nr:8;
			unsigned dest_horiz_stride:2;
			unsigned dest_address_mode:1;
		} da1;

		struct {
			unsigned dest_reg_file:2;
			unsigned dest_reg_type:3;
			unsigned src0_reg_file:2;
			unsigned src0_reg_type:3;
			unsigned src1_reg_file:2;        /* 0x00000c00 */
			unsigned src1_reg_type:3;        /* 0x00007000 */
			unsigned pad:1;
			int dest_indirect_offset:10;	/* offset against the deref'd address reg */
			unsigned dest_subreg_nr:3; /* subnr for the address reg a0.x */
			unsigned dest_horiz_stride:2;
			unsigned dest_address_mode:1;
		} ia1;

		struct {
			unsigned dest_reg_file:2;
			unsigned dest_reg_type:3;
			unsigned src0_reg_file:2;
			unsigned src0_reg_type:3;
			unsigned src1_reg_file:2;
			unsigned src1_reg_type:3;
			unsigned pad:1;
			unsigned dest_writemask:4;
			unsigned dest_subreg_nr:1;
			unsigned dest_reg_nr:8;
			unsigned dest_horiz_stride:2;
			unsigned dest_address_mode:1;
		} da16;

		struct {
			unsigned dest_reg_file:2;
			unsigned dest_reg_type:3;
			unsigned src0_reg_file:2;
			unsigned src0_reg_type:3;
			unsigned pad0:6;
			unsigned dest_writemask:4;
			int dest_indirect_offset:6;
			unsigned dest_subreg_nr:3;
			unsigned dest_horiz_stride:2;
			unsigned dest_address_mode:1;
		} ia16;

		struct {
			unsigned dest_reg_file:2;
			unsigned dest_reg_type:3;
			unsigned src0_reg_file:2;
			unsigned src0_reg_type:3;
			unsigned src1_reg_file:2;
			unsigned src1_reg_type:3;
			unsigned pad:1;

			int jump_count:16;
		} branch_gen6;

		struct {
			unsigned dest_reg_file:1;
			unsigned flag_subreg_num:1;
			unsigned pad0:2;
			unsigned src0_abs:1;
			unsigned src0_negate:1;
			unsigned src1_abs:1;
			unsigned src1_negate:1;
			unsigned src2_abs:1;
			unsigned src2_negate:1;
			unsigned pad1:7;
			unsigned dest_writemask:4;
			unsigned dest_subreg_nr:3;
			unsigned dest_reg_nr:8;
		} da3src;
	} bits1;


	union {
		struct {
			unsigned src0_subreg_nr:5;
			unsigned src0_reg_nr:8;
			unsigned src0_abs:1;
			unsigned src0_negate:1;
			unsigned src0_address_mode:1;
			unsigned src0_horiz_stride:2;
			unsigned src0_width:3;
			unsigned src0_vert_stride:4;
			unsigned flag_subreg_nr:1;
			unsigned flag_reg_nr:1;
			unsigned pad:5;
		} da1;

		struct {
			int src0_indirect_offset:10;
			unsigned src0_subreg_nr:3;
			unsigned src0_abs:1;
			unsigned src0_negate:1;
			unsigned src0_address_mode:1;
			unsigned src0_horiz_stride:2;
			unsigned src0_width:3;
			unsigned src0_vert_stride:4;
			unsigned flag_subreg_nr:1;
			unsigned flag_reg_nr:1;
			unsigned pad:5;
		} ia1;

		struct {
			unsigned src0_swz_x:2;
			unsigned src0_swz_y:2;
			unsigned src0_subreg_nr:1;
			unsigned src0_reg_nr:8;
			unsigned src0_abs:1;
			unsigned src0_negate:1;
			unsigned src0_address_mode:1;
			unsigned src0_swz_z:2;
			unsigned src0_swz_w:2;
			unsigned pad0:1;
			unsigned src0_vert_stride:4;
			unsigned flag_subreg_nr:1;
			unsigned flag_reg_nr:1;
			unsigned pad1:5;
		} da16;

		struct {
			unsigned src0_swz_x:2;
			unsigned src0_swz_y:2;
			int src0_indirect_offset:6;
			unsigned src0_subreg_nr:3;
			unsigned src0_abs:1;
			unsigned src0_negate:1;
			unsigned src0_address_mode:1;
			unsigned src0_swz_z:2;
			unsigned src0_swz_w:2;
			unsigned pad0:1;
			unsigned src0_vert_stride:4;
			unsigned flag_subreg_nr:1;
			unsigned flag_reg_nr:1;
			unsigned pad1:5;
		} ia16;

		/* Extended Message Descriptor for Ironlake (Gen5) SEND instruction.
		 *
		 * Does not apply to Gen6+.  The SFID/message target moved to bits
		 * 27:24 of the header (destreg__conditionalmod); EOT is in bits3.
		 */
		struct {
			unsigned pad:26;
			unsigned end_of_thread:1;
			unsigned pad1:1;
			unsigned sfid:4;
		} send_gen5;  /* for Ironlake only */

		struct {
			unsigned src0_rep_ctrl:1;
			unsigned src0_swizzle:8;
			unsigned src0_subreg_nr:3;
			unsigned src0_reg_nr:8;
			unsigned pad0:1;
			unsigned src1_rep_ctrl:1;
			unsigned src1_swizzle:8;
			unsigned src1_subreg_nr_low:2;
		} da3src;
	} bits2;

	union {
		struct {
			unsigned src1_subreg_nr:5;
			unsigned src1_reg_nr:8;
			unsigned src1_abs:1;
			unsigned src1_negate:1;
			unsigned src1_address_mode:1;
			unsigned src1_horiz_stride:2;
			unsigned src1_width:3;
			unsigned src1_vert_stride:4;
			unsigned pad0:7;
		} da1;

		struct {
			unsigned src1_swz_x:2;
			unsigned src1_swz_y:2;
			unsigned src1_subreg_nr:1;
			unsigned src1_reg_nr:8;
			unsigned src1_abs:1;
			unsigned src1_negate:1;
			unsigned src1_address_mode:1;
			unsigned src1_swz_z:2;
			unsigned src1_swz_w:2;
			unsigned pad1:1;
			unsigned src1_vert_stride:4;
			unsigned pad2:7;
		} da16;

		struct {
			int src1_indirect_offset:10;
			unsigned src1_subreg_nr:3;
			unsigned src1_abs:1;
			unsigned src1_negate:1;
			unsigned src1_address_mode:1;
			unsigned src1_horiz_stride:2;
			unsigned src1_width:3;
			unsigned src1_vert_stride:4;
			unsigned flag_subreg_nr:1;
			unsigned flag_reg_nr:1;
			unsigned pad1:5;
		} ia1;

		struct {
			unsigned src1_swz_x:2;
			unsigned src1_swz_y:2;
			int  src1_indirect_offset:6;
			unsigned src1_subreg_nr:3;
			unsigned src1_abs:1;
			unsigned src1_negate:1;
			unsigned pad0:1;
			unsigned src1_swz_z:2;
			unsigned src1_swz_w:2;
			unsigned pad1:1;
			unsigned src1_vert_stride:4;
			unsigned flag_subreg_nr:1;
			unsigned flag_reg_nr:1;
			unsigned pad2:5;
		} ia16;

		struct {
			int jump_count:16;	/* note: signed */
			unsigned pop_count:4;
			unsigned pad0:12;
		} if_else;

		/* This is also used for gen7 IF/ELSE instructions */
		struct {
			/* Signed jump distance to the ip to jump to if all channels
			 * are disabled after the break or continue.  It should point
			 * to the end of the innermost control flow block, as that's
			 * where some channel could get re-enabled.
			 */
			int jip:16;

			/* Signed jump distance to the location to resume execution
			 * of this channel if it's enabled for the break or continue.
			 */
			int uip:16;
		} break_cont;

		/**
		 * \defgroup SEND instructions / Message Descriptors
		 *
		 * @{
		 */

		/**
		 * Generic Message Descriptor for Gen4 SEND instructions.  The structs
		 * below expand function_control to something specific for their
		 * message.  Due to struct packing issues, they duplicate these bits.
		 *
		 * See the G45 PRM, Volume 4, Table 14-15.
		 */
		struct {
			unsigned function_control:16;
			unsigned response_length:4;
			unsigned msg_length:4;
			unsigned msg_target:4;
			unsigned pad1:3;
			unsigned end_of_thread:1;
		} generic;

		/**
		 * Generic Message Descriptor for Gen5-7 SEND instructions.
		 *
		 * See the Sandybridge PRM, Volume 2 Part 2, Table 8-15.  (Sadly, most
		 * of the information on the SEND instruction is missing from the public
		 * Ironlake PRM.)
		 *
		 * The table claims that bit 31 is reserved/MBZ on Gen6+, but it lies.
		 * According to the SEND instruction description:
		 * "The MSb of the message description, the EOT field, always comes from
		 *  bit 127 of the instruction word"...which is bit 31 of this field.
		 */
		struct {
			unsigned function_control:19;
			unsigned header_present:1;
			unsigned response_length:5;
			unsigned msg_length:4;
			unsigned pad1:2;
			unsigned end_of_thread:1;
		} generic_gen5;

		/** G45 PRM, Volume 4, Section 6.1.1.1 */
		struct {
			unsigned function:4;
			unsigned int_type:1;
			unsigned precision:1;
			unsigned saturate:1;
			unsigned data_type:1;
			unsigned pad0:8;
			unsigned response_length:4;
			unsigned msg_length:4;
			unsigned msg_target:4;
			unsigned pad1:3;
			unsigned end_of_thread:1;
		} math;

		/** Ironlake PRM, Volume 4 Part 1, Section 6.1.1.1 */
		struct {
			unsigned function:4;
			unsigned int_type:1;
			unsigned precision:1;
			unsigned saturate:1;
			unsigned data_type:1;
			unsigned snapshot:1;
			unsigned pad0:10;
			unsigned header_present:1;
			unsigned response_length:5;
			unsigned msg_length:4;
			unsigned pad1:2;
			unsigned end_of_thread:1;
		} math_gen5;

		/** G45 PRM, Volume 4, Section 4.8.1.1.1 [DevBW] and [DevCL] */
		struct {
			unsigned binding_table_index:8;
			unsigned sampler:4;
			unsigned return_format:2;
			unsigned msg_type:2;
			unsigned response_length:4;
			unsigned msg_length:4;
			unsigned msg_target:4;
			unsigned pad1:3;
			unsigned end_of_thread:1;
		} sampler;

		/** G45 PRM, Volume 4, Section 4.8.1.1.2 [DevCTG] */
		struct {
			unsigned binding_table_index:8;
			unsigned sampler:4;
			unsigned msg_type:4;
			unsigned response_length:4;
			unsigned msg_length:4;
			unsigned msg_target:4;
			unsigned pad1:3;
			unsigned end_of_thread:1;
		} sampler_g4x;

		/** Ironlake PRM, Volume 4 Part 1, Section 4.11.1.1.3 */
		struct {
			unsigned binding_table_index:8;
			unsigned sampler:4;
			unsigned msg_type:4;
			unsigned simd_mode:2;
			unsigned pad0:1;
			unsigned header_present:1;
			unsigned response_length:5;
			unsigned msg_length:4;
			unsigned pad1:2;
			unsigned end_of_thread:1;
		} sampler_gen5;

		struct {
			unsigned binding_table_index:8;
			unsigned sampler:4;
			unsigned msg_type:5;
			unsigned simd_mode:2;
			unsigned header_present:1;
			unsigned response_length:5;
			unsigned msg_length:4;
			unsigned pad1:2;
			unsigned end_of_thread:1;
		} sampler_gen7;

		struct brw_urb_immediate {
			unsigned opcode:4;
			unsigned offset:6;
			unsigned swizzle_control:2;
			unsigned pad:1;
			unsigned allocate:1;
			unsigned used:1;
			unsigned complete:1;
			unsigned response_length:4;
			unsigned msg_length:4;
			unsigned msg_target:4;
			unsigned pad1:3;
			unsigned end_of_thread:1;
		} urb;

		struct {
			unsigned opcode:4;
			unsigned offset:6;
			unsigned swizzle_control:2;
			unsigned pad:1;
			unsigned allocate:1;
			unsigned used:1;
			unsigned complete:1;
			unsigned pad0:3;
			unsigned header_present:1;
			unsigned response_length:5;
			unsigned msg_length:4;
			unsigned pad1:2;
			unsigned end_of_thread:1;
		} urb_gen5;

		struct {
			unsigned opcode:3;
			unsigned offset:11;
			unsigned swizzle_control:1;
			unsigned complete:1;
			unsigned per_slot_offset:1;
			unsigned pad0:2;
			unsigned header_present:1;
			unsigned response_length:5;
			unsigned msg_length:4;
			unsigned pad1:2;
			unsigned end_of_thread:1;
		} urb_gen7;

		/** 965 PRM, Volume 4, Section 5.10.1.1: Message Descriptor */
		struct {
			unsigned binding_table_index:8;
			unsigned msg_control:4;
			unsigned msg_type:2;
			unsigned target_cache:2;
			unsigned response_length:4;
			unsigned msg_length:4;
			unsigned msg_target:4;
			unsigned pad1:3;
			unsigned end_of_thread:1;
		} dp_read;

		/** G45 PRM, Volume 4, Section 5.10.1.1.2 */
		struct {
			unsigned binding_table_index:8;
			unsigned msg_control:3;
			unsigned msg_type:3;
			unsigned target_cache:2;
			unsigned response_length:4;
			unsigned msg_length:4;
			unsigned msg_target:4;
			unsigned pad1:3;
			unsigned end_of_thread:1;
		} dp_read_g4x;

		/** Ironlake PRM, Volume 4 Part 1, Section 5.10.2.1.2. */
		struct {
			unsigned binding_table_index:8;
			unsigned msg_control:3;
			unsigned msg_type:3;
			unsigned target_cache:2;
			unsigned pad0:3;
			unsigned header_present:1;
			unsigned response_length:5;
			unsigned msg_length:4;
			unsigned pad1:2;
			unsigned end_of_thread:1;
		} dp_read_gen5;

		/** G45 PRM, Volume 4, Section 5.10.1.1.2.  For both Gen4 and G45. */
		struct {
			unsigned binding_table_index:8;
			unsigned msg_control:3;
			unsigned last_render_target:1;
			unsigned msg_type:3;
			unsigned send_commit_msg:1;
			unsigned response_length:4;
			unsigned msg_length:4;
			unsigned msg_target:4;
			unsigned pad1:3;
			unsigned end_of_thread:1;
		} dp_write;

		/** Ironlake PRM, Volume 4 Part 1, Section 5.10.2.1.2. */
		struct {
			unsigned binding_table_index:8;
			unsigned msg_control:3;
			unsigned last_render_target:1;
			unsigned msg_type:3;
			unsigned send_commit_msg:1;
			unsigned pad0:3;
			unsigned header_present:1;
			unsigned response_length:5;
			unsigned msg_length:4;
			unsigned pad1:2;
			unsigned end_of_thread:1;
		} dp_write_gen5;

		/**
		 * Message for the Sandybridge Sampler Cache or Constant Cache Data Port.
		 *
		 * See the Sandybridge PRM, Volume 4 Part 1, Section 3.9.2.1.1.
		 **/
		struct {
			unsigned binding_table_index:8;
			unsigned msg_control:5;
			unsigned msg_type:3;
			unsigned pad0:3;
			unsigned header_present:1;
			unsigned response_length:5;
			unsigned msg_length:4;
			unsigned pad1:2;
			unsigned end_of_thread:1;
		} gen6_dp_sampler_const_cache;

		/**
		 * Message for the Sandybridge Render Cache Data Port.
		 *
		 * Most fields are defined in the Sandybridge PRM, Volume 4 Part 1,
		 * Section 3.9.2.1.1: Message Descriptor.
		 *
		 * "Slot Group Select" and "Last Render Target" are part of the
		 * 5-bit message control for Render Target Write messages.  See
		 * Section 3.9.9.2.1 of the same volume.
		 */
		struct {
			unsigned binding_table_index:8;
			unsigned msg_control:3;
			unsigned slot_group_select:1;
			unsigned last_render_target:1;
			unsigned msg_type:4;
			unsigned send_commit_msg:1;
			unsigned pad0:1;
			unsigned header_present:1;
			unsigned response_length:5;
			unsigned msg_length:4;
			unsigned pad1:2;
			unsigned end_of_thread:1;
		} gen6_dp;

		/**
		 * Message for any of the Gen7 Data Port caches.
		 *
		 * Most fields are defined in BSpec volume 5c.2 Data Port / Messages /
		 * Data Port Messages / Message Descriptor.  Once again, "Slot Group
		 * Select" and "Last Render Target" are part of the 6-bit message
		 * control for Render Target Writes.
		 */
		struct {
			unsigned binding_table_index:8;
			unsigned msg_control:3;
			unsigned slot_group_select:1;
			unsigned last_render_target:1;
			unsigned msg_control_pad:1;
			unsigned msg_type:4;
			unsigned pad1:1;
			unsigned header_present:1;
			unsigned response_length:5;
			unsigned msg_length:4;
			unsigned pad2:2;
			unsigned end_of_thread:1;
		} gen7_dp;
		/** @} */

		struct {
			unsigned src1_subreg_nr_high:1;
			unsigned src1_reg_nr:8;
			unsigned pad0:1;
			unsigned src2_rep_ctrl:1;
			unsigned src2_swizzle:8;
			unsigned src2_subreg_nr:3;
			unsigned src2_reg_nr:8;
			unsigned pad1:2;
		} da3src;

		int d;
		unsigned ud;
		float f;
	} bits3;
};


/* These aren't hardware structs, just something useful for us to pass around:
 *
 * Align1 operation has a lot of control over input ranges.  Used in
 * WM programs to implement shaders decomposed into "channel serial"
 * or "structure of array" form:
 */
struct brw_reg {
	unsigned type:4;
	unsigned file:2;
	unsigned nr:8;
	unsigned subnr:5;		/* :1 in align16 */
	unsigned negate:1;		/* source only */
	unsigned abs:1;		/* source only */
	unsigned vstride:4;		/* source only */
	unsigned width:3;		/* src only, align1 only */
	unsigned hstride:2;   		/* align1 only */
	unsigned address_mode:1;	/* relative addressing, hopefully! */
	unsigned pad0:1;

	union {
		struct {
			unsigned swizzle:8;		/* src only, align16 only */
			unsigned writemask:4;		/* dest only, align16 only */
			int  indirect_offset:10;	/* relative addressing offset */
			unsigned pad1:10;		/* two dwords total */
		} bits;

		float f;
		int   d;
		unsigned ud;
	} dw1;
};

struct brw_indirect {
	unsigned addr_subnr:4;
	int addr_offset:10;
	unsigned pad:18;
};

#define BRW_EU_MAX_INSN_STACK 5
#define BRW_EU_MAX_INSN 10000

struct brw_compile {
	struct brw_instruction *store;
	unsigned nr_insn;

	int gen;

	/* Allow clients to push/pop instruction state:
	*/
	struct brw_instruction stack[BRW_EU_MAX_INSN_STACK];
	bool compressed_stack[BRW_EU_MAX_INSN_STACK];
	struct brw_instruction *current;

	unsigned flag_value;
	bool single_program_flow;
	bool compressed;

	/* Control flow stacks:
	 * - if_stack contains IF and ELSE instructions which must be patched
	 *   (and popped) once the matching ENDIF instruction is encountered.
	 */
	struct brw_instruction **if_stack;
	int if_stack_depth;
	int if_stack_array_size;
};

static inline int type_sz(unsigned type)
{
	switch (type) {
	case BRW_REGISTER_TYPE_UD:
	case BRW_REGISTER_TYPE_D:
	case BRW_REGISTER_TYPE_F:
		return 4;
	case BRW_REGISTER_TYPE_HF:
	case BRW_REGISTER_TYPE_UW:
	case BRW_REGISTER_TYPE_W:
		return 2;
	case BRW_REGISTER_TYPE_UB:
	case BRW_REGISTER_TYPE_B:
		return 1;
	default:
		return 0;
	}
}

/**
 * Construct a brw_reg.
 * \param file  one of the BRW_x_REGISTER_FILE values
 * \param nr  register number/index
 * \param subnr  register sub number
 * \param type  one of BRW_REGISTER_TYPE_x
 * \param vstride  one of BRW_VERTICAL_STRIDE_x
 * \param width  one of BRW_WIDTH_x
 * \param hstride  one of BRW_HORIZONTAL_STRIDE_x
 * \param swizzle  one of BRW_SWIZZLE_x
 * \param writemask  WRITEMASK_X/Y/Z/W bitfield
 */
static inline struct brw_reg brw_reg(unsigned file,
				     unsigned nr,
				     unsigned subnr,
				     unsigned type,
				     unsigned vstride,
				     unsigned width,
				     unsigned hstride,
				     unsigned swizzle,
				     unsigned writemask)
{
	struct brw_reg reg;
	if (file == BRW_GENERAL_REGISTER_FILE)
		assert(nr < BRW_MAX_GRF);
	else if (file == BRW_MESSAGE_REGISTER_FILE)
		assert((nr & ~(1 << 7)) < BRW_MAX_MRF);
	else if (file == BRW_ARCHITECTURE_REGISTER_FILE)
		assert(nr <= BRW_ARF_IP);

	reg.type = type;
	reg.file = file;
	reg.nr = nr;
	reg.subnr = subnr * type_sz(type);
	reg.negate = 0;
	reg.abs = 0;
	reg.vstride = vstride;
	reg.width = width;
	reg.hstride = hstride;
	reg.address_mode = BRW_ADDRESS_DIRECT;
	reg.pad0 = 0;

	/* Could do better: If the reg is r5.3<0;1,0>, we probably want to
	 * set swizzle and writemask to W, as the lower bits of subnr will
	 * be lost when converted to align16.  This is probably too much to
	 * keep track of as you'd want it adjusted by suboffset(), etc.
	 * Perhaps fix up when converting to align16?
	 */
	reg.dw1.bits.swizzle = swizzle;
	reg.dw1.bits.writemask = writemask;
	reg.dw1.bits.indirect_offset = 0;
	reg.dw1.bits.pad1 = 0;
	return reg;
}

/** Construct float[16] register */
static inline struct brw_reg brw_vec16_reg(unsigned file,
					   unsigned nr,
					   unsigned subnr)
{
	return brw_reg(file,
		       nr,
		       subnr,
		       BRW_REGISTER_TYPE_F,
		       BRW_VERTICAL_STRIDE_16,
		       BRW_WIDTH_16,
		       BRW_HORIZONTAL_STRIDE_1,
		       BRW_SWIZZLE_XYZW,
		       WRITEMASK_XYZW);
}

/** Construct float[8] register */
static inline struct brw_reg brw_vec8_reg(unsigned file,
					  unsigned nr,
					  unsigned subnr)
{
	return brw_reg(file,
		       nr,
		       subnr,
		       BRW_REGISTER_TYPE_F,
		       BRW_VERTICAL_STRIDE_8,
		       BRW_WIDTH_8,
		       BRW_HORIZONTAL_STRIDE_1,
		       BRW_SWIZZLE_XYZW,
		       WRITEMASK_XYZW);
}

/** Construct float[4] register */
static inline struct brw_reg brw_vec4_reg(unsigned file,
					  unsigned nr,
					  unsigned subnr)
{
	return brw_reg(file,
		       nr,
		       subnr,
		       BRW_REGISTER_TYPE_F,
		       BRW_VERTICAL_STRIDE_4,
		       BRW_WIDTH_4,
		       BRW_HORIZONTAL_STRIDE_1,
		       BRW_SWIZZLE_XYZW,
		       WRITEMASK_XYZW);
}

/** Construct float[2] register */
static inline struct brw_reg brw_vec2_reg(unsigned file,
					  unsigned nr,
					  unsigned subnr)
{
	return brw_reg(file,
		       nr,
		       subnr,
		       BRW_REGISTER_TYPE_F,
		       BRW_VERTICAL_STRIDE_2,
		       BRW_WIDTH_2,
		       BRW_HORIZONTAL_STRIDE_1,
		       BRW_SWIZZLE_XYXY,
		       WRITEMASK_XY);
}

/** Construct float[1] register */
static inline struct brw_reg brw_vec1_reg(unsigned file,
					  unsigned nr,
					  unsigned subnr)
{
	return brw_reg(file,
		       nr,
		       subnr,
		       BRW_REGISTER_TYPE_F,
		       BRW_VERTICAL_STRIDE_0,
		       BRW_WIDTH_1,
		       BRW_HORIZONTAL_STRIDE_0,
		       BRW_SWIZZLE_XXXX,
		       WRITEMASK_X);
}


static inline struct brw_reg __retype(struct brw_reg reg,
				      unsigned type)
{
	reg.type = type;
	return reg;
}

static inline struct brw_reg __retype_d(struct brw_reg reg)
{
	return __retype(reg, BRW_REGISTER_TYPE_D);
}

static inline struct brw_reg __retype_ud(struct brw_reg reg)
{
	return __retype(reg, BRW_REGISTER_TYPE_UD);
}

static inline struct brw_reg __retype_uw(struct brw_reg reg)
{
	return __retype(reg, BRW_REGISTER_TYPE_UW);
}

static inline struct brw_reg __sechalf(struct brw_reg reg)
{
	if (reg.vstride)
		reg.nr++;
	return reg;
}

static inline struct brw_reg __suboffset(struct brw_reg reg,
					 unsigned delta)
{
	reg.subnr += delta * type_sz(reg.type);
	return reg;
}

static inline struct brw_reg __offset(struct brw_reg reg,
				      unsigned delta)
{
	reg.nr += delta;
	return reg;
}

static inline struct brw_reg byte_offset(struct brw_reg reg,
					 unsigned bytes)
{
	unsigned newoffset = reg.nr * REG_SIZE + reg.subnr + bytes;
	reg.nr = newoffset / REG_SIZE;
	reg.subnr = newoffset % REG_SIZE;
	return reg;
}


/** Construct unsigned word[16] register */
static inline struct brw_reg brw_uw16_reg(unsigned file,
					  unsigned nr,
					  unsigned subnr)
{
	return __suboffset(__retype(brw_vec16_reg(file, nr, 0), BRW_REGISTER_TYPE_UW), subnr);
}

/** Construct unsigned word[8] register */
static inline struct brw_reg brw_uw8_reg(unsigned file,
					 unsigned nr,
					 unsigned subnr)
{
	return __suboffset(__retype(brw_vec8_reg(file, nr, 0), BRW_REGISTER_TYPE_UW), subnr);
}

/** Construct unsigned word[1] register */
static inline struct brw_reg brw_uw1_reg(unsigned file,
					 unsigned nr,
					 unsigned subnr)
{
	return __suboffset(__retype(brw_vec1_reg(file, nr, 0), BRW_REGISTER_TYPE_UW), subnr);
}

static inline struct brw_reg brw_imm_reg(unsigned type)
{
	return brw_reg( BRW_IMMEDIATE_VALUE,
			0,
			0,
			type,
			BRW_VERTICAL_STRIDE_0,
			BRW_WIDTH_1,
			BRW_HORIZONTAL_STRIDE_0,
			0,
			0);
}

/** Construct float immediate register */
static inline struct brw_reg brw_imm_f(float f)
{
	struct brw_reg imm = brw_imm_reg(BRW_REGISTER_TYPE_F);
	imm.dw1.f = f;
	return imm;
}

/** Construct integer immediate register */
static inline struct brw_reg brw_imm_d(int d)
{
	struct brw_reg imm = brw_imm_reg(BRW_REGISTER_TYPE_D);
	imm.dw1.d = d;
	return imm;
}

/** Construct uint immediate register */
static inline struct brw_reg brw_imm_ud(unsigned ud)
{
	struct brw_reg imm = brw_imm_reg(BRW_REGISTER_TYPE_UD);
	imm.dw1.ud = ud;
	return imm;
}

/** Construct ushort immediate register */
static inline struct brw_reg brw_imm_uw(uint16_t uw)
{
	struct brw_reg imm = brw_imm_reg(BRW_REGISTER_TYPE_UW);
	imm.dw1.ud = uw | (uw << 16);
	return imm;
}

/** Construct short immediate register */
static inline struct brw_reg brw_imm_w(int16_t w)
{
	struct brw_reg imm = brw_imm_reg(BRW_REGISTER_TYPE_W);
	imm.dw1.d = w | (w << 16);
	return imm;
}

/* brw_imm_b and brw_imm_ub aren't supported by hardware - the type
 * numbers alias with _V and _VF below:
 */

/** Construct vector of eight signed half-byte values */
static inline struct brw_reg brw_imm_v(unsigned v)
{
	struct brw_reg imm = brw_imm_reg(BRW_REGISTER_TYPE_V);
	imm.vstride = BRW_VERTICAL_STRIDE_0;
	imm.width = BRW_WIDTH_8;
	imm.hstride = BRW_HORIZONTAL_STRIDE_1;
	imm.dw1.ud = v;
	return imm;
}

/** Construct vector of four 8-bit float values */
static inline struct brw_reg brw_imm_vf(unsigned v)
{
	struct brw_reg imm = brw_imm_reg(BRW_REGISTER_TYPE_VF);
	imm.vstride = BRW_VERTICAL_STRIDE_0;
	imm.width = BRW_WIDTH_4;
	imm.hstride = BRW_HORIZONTAL_STRIDE_1;
	imm.dw1.ud = v;
	return imm;
}

#define VF_ZERO 0x0
#define VF_ONE  0x30
#define VF_NEG  (1<<7)

static inline struct brw_reg brw_imm_vf4(unsigned v0,
					 unsigned v1,
					 unsigned v2,
					 unsigned v3)
{
	struct brw_reg imm = brw_imm_reg(BRW_REGISTER_TYPE_VF);
	imm.vstride = BRW_VERTICAL_STRIDE_0;
	imm.width = BRW_WIDTH_4;
	imm.hstride = BRW_HORIZONTAL_STRIDE_1;
	imm.dw1.ud = ((v0 << 0) |
		      (v1 << 8) |
		      (v2 << 16) |
		      (v3 << 24));
	return imm;
}

static inline struct brw_reg brw_address(struct brw_reg reg)
{
	return brw_imm_uw(reg.nr * REG_SIZE + reg.subnr);
}

/** Construct float[1] general-purpose register */
static inline struct brw_reg brw_vec1_grf(unsigned nr, unsigned subnr)
{
	return brw_vec1_reg(BRW_GENERAL_REGISTER_FILE, nr, subnr);
}

/** Construct float[2] general-purpose register */
static inline struct brw_reg brw_vec2_grf(unsigned nr, unsigned subnr)
{
	return brw_vec2_reg(BRW_GENERAL_REGISTER_FILE, nr, subnr);
}

/** Construct float[4] general-purpose register */
static inline struct brw_reg brw_vec4_grf(unsigned nr, unsigned subnr)
{
	return brw_vec4_reg(BRW_GENERAL_REGISTER_FILE, nr, subnr);
}

/** Construct float[8] general-purpose register */
static inline struct brw_reg brw_vec8_grf(unsigned nr, unsigned subnr)
{
	return brw_vec8_reg(BRW_GENERAL_REGISTER_FILE, nr, subnr);
}

static inline struct brw_reg brw_uw8_grf(unsigned nr, unsigned subnr)
{
	return brw_uw8_reg(BRW_GENERAL_REGISTER_FILE, nr, subnr);
}

static inline struct brw_reg brw_uw16_grf(unsigned nr, unsigned subnr)
{
	return brw_uw16_reg(BRW_GENERAL_REGISTER_FILE, nr, subnr);
}

/** Construct null register (usually used for setting condition codes) */
static inline struct brw_reg brw_null_reg(void)
{
	return brw_vec8_reg(BRW_ARCHITECTURE_REGISTER_FILE,
			    BRW_ARF_NULL,
			    0);
}

static inline struct brw_reg brw_address_reg(unsigned subnr)
{
	return brw_uw1_reg(BRW_ARCHITECTURE_REGISTER_FILE,
			   BRW_ARF_ADDRESS,
			   subnr);
}

/* If/else instructions break in align16 mode if writemask & swizzle
 * aren't xyzw.  This goes against the convention for other scalar
 * regs:
 */
static inline struct brw_reg brw_ip_reg(void)
{
	return brw_reg(BRW_ARCHITECTURE_REGISTER_FILE,
		       BRW_ARF_IP,
		       0,
		       BRW_REGISTER_TYPE_UD,
		       BRW_VERTICAL_STRIDE_4, /* ? */
		       BRW_WIDTH_1,
		       BRW_HORIZONTAL_STRIDE_0,
		       BRW_SWIZZLE_XYZW, /* NOTE! */
		       WRITEMASK_XYZW); /* NOTE! */
}

static inline struct brw_reg brw_acc_reg(void)
{
	return brw_vec8_reg(BRW_ARCHITECTURE_REGISTER_FILE,
			    BRW_ARF_ACCUMULATOR,
			    0);
}

static inline struct brw_reg brw_notification_1_reg(void)
{
	return brw_reg(BRW_ARCHITECTURE_REGISTER_FILE,
		       BRW_ARF_NOTIFICATION_COUNT,
		       1,
		       BRW_REGISTER_TYPE_UD,
		       BRW_VERTICAL_STRIDE_0,
		       BRW_WIDTH_1,
		       BRW_HORIZONTAL_STRIDE_0,
		       BRW_SWIZZLE_XXXX,
		       WRITEMASK_X);
}

static inline struct brw_reg brw_flag_reg(void)
{
	return brw_uw1_reg(BRW_ARCHITECTURE_REGISTER_FILE,
			   BRW_ARF_FLAG,
			   0);
}

static inline struct brw_reg brw_mask_reg(unsigned subnr)
{
	return brw_uw1_reg(BRW_ARCHITECTURE_REGISTER_FILE,
			   BRW_ARF_MASK,
			   subnr);
}

static inline struct brw_reg brw_message_reg(unsigned nr)
{
	assert((nr & ~(1 << 7)) < BRW_MAX_MRF);
	return brw_vec8_reg(BRW_MESSAGE_REGISTER_FILE, nr, 0);
}

static inline struct brw_reg brw_message4_reg(unsigned nr,
					      int subnr)
{
	assert((nr & ~(1 << 7)) < BRW_MAX_MRF);
	return brw_vec4_reg(BRW_MESSAGE_REGISTER_FILE, nr, subnr);
}

/* This is almost always called with a numeric constant argument, so
 * make things easy to evaluate at compile time:
 */
static inline unsigned cvt(unsigned val)
{
	switch (val) {
	case 0: return 0;
	case 1: return 1;
	case 2: return 2;
	case 4: return 3;
	case 8: return 4;
	case 16: return 5;
	case 32: return 6;
	}
	return 0;
}

static inline struct brw_reg __stride(struct brw_reg reg,
				    unsigned vstride,
				    unsigned width,
				    unsigned hstride)
{
	reg.vstride = cvt(vstride);
	reg.width = cvt(width) - 1;
	reg.hstride = cvt(hstride);
	return reg;
}

static inline struct brw_reg vec16(struct brw_reg reg)
{
	return __stride(reg, 16,16,1);
}

static inline struct brw_reg vec8(struct brw_reg reg)
{
	return __stride(reg, 8,8,1);
}

static inline struct brw_reg vec4(struct brw_reg reg)
{
	return __stride(reg, 4,4,1);
}

static inline struct brw_reg vec2(struct brw_reg reg)
{
	return __stride(reg, 2,2,1);
}

static inline struct brw_reg vec1(struct brw_reg reg)
{
	return __stride(reg, 0,1,0);
}

static inline struct brw_reg get_element(struct brw_reg reg, unsigned elt)
{
	return vec1(__suboffset(reg, elt));
}

static inline struct brw_reg get_element_ud(struct brw_reg reg, unsigned elt)
{
	return vec1(__suboffset(__retype(reg, BRW_REGISTER_TYPE_UD), elt));
}

static inline struct brw_reg brw_swizzle(struct brw_reg reg,
					 unsigned x,
					 unsigned y,
					 unsigned z,
					 unsigned w)
{
	assert(reg.file != BRW_IMMEDIATE_VALUE);

	reg.dw1.bits.swizzle = BRW_SWIZZLE4(BRW_GET_SWZ(reg.dw1.bits.swizzle, x),
					    BRW_GET_SWZ(reg.dw1.bits.swizzle, y),
					    BRW_GET_SWZ(reg.dw1.bits.swizzle, z),
					    BRW_GET_SWZ(reg.dw1.bits.swizzle, w));
	return reg;
}

static inline struct brw_reg brw_swizzle1(struct brw_reg reg,
					  unsigned x)
{
	return brw_swizzle(reg, x, x, x, x);
}

static inline struct brw_reg brw_writemask(struct brw_reg reg,
					   unsigned mask)
{
	assert(reg.file != BRW_IMMEDIATE_VALUE);
	reg.dw1.bits.writemask &= mask;
	return reg;
}

static inline struct brw_reg brw_set_writemask(struct brw_reg reg,
					       unsigned mask)
{
	assert(reg.file != BRW_IMMEDIATE_VALUE);
	reg.dw1.bits.writemask = mask;
	return reg;
}

static inline struct brw_reg brw_negate(struct brw_reg reg)
{
	reg.negate ^= 1;
	return reg;
}

static inline struct brw_reg brw_abs(struct brw_reg reg)
{
	reg.abs = 1;
	return reg;
}

/***********************************************************************
*/
static inline struct brw_reg brw_vec4_indirect(unsigned subnr,
					       int offset)
{
	struct brw_reg reg =  brw_vec4_grf(0, 0);
	reg.subnr = subnr;
	reg.address_mode = BRW_ADDRESS_REGISTER_INDIRECT_REGISTER;
	reg.dw1.bits.indirect_offset = offset;
	return reg;
}

static inline struct brw_reg brw_vec1_indirect(unsigned subnr,
					       int offset)
{
	struct brw_reg reg =  brw_vec1_grf(0, 0);
	reg.subnr = subnr;
	reg.address_mode = BRW_ADDRESS_REGISTER_INDIRECT_REGISTER;
	reg.dw1.bits.indirect_offset = offset;
	return reg;
}

static inline struct brw_reg deref_4f(struct brw_indirect ptr, int offset)
{
	return brw_vec4_indirect(ptr.addr_subnr, ptr.addr_offset + offset);
}

static inline struct brw_reg deref_1f(struct brw_indirect ptr, int offset)
{
	return brw_vec1_indirect(ptr.addr_subnr, ptr.addr_offset + offset);
}

static inline struct brw_reg deref_4b(struct brw_indirect ptr, int offset)
{
	return __retype(deref_4f(ptr, offset), BRW_REGISTER_TYPE_B);
}

static inline struct brw_reg deref_1uw(struct brw_indirect ptr, int offset)
{
	return __retype(deref_1f(ptr, offset), BRW_REGISTER_TYPE_UW);
}

static inline struct brw_reg deref_1d(struct brw_indirect ptr, int offset)
{
	return __retype(deref_1f(ptr, offset), BRW_REGISTER_TYPE_D);
}

static inline struct brw_reg deref_1ud(struct brw_indirect ptr, int offset)
{
	return __retype(deref_1f(ptr, offset), BRW_REGISTER_TYPE_UD);
}

static inline struct brw_reg get_addr_reg(struct brw_indirect ptr)
{
	return brw_address_reg(ptr.addr_subnr);
}

static inline struct brw_indirect brw_indirect_offset(struct brw_indirect ptr, int offset)
{
	ptr.addr_offset += offset;
	return ptr;
}

static inline struct brw_indirect brw_indirect(unsigned addr_subnr, int offset)
{
	struct brw_indirect ptr;
	ptr.addr_subnr = addr_subnr;
	ptr.addr_offset = offset;
	ptr.pad = 0;
	return ptr;
}

/** Do two brw_regs refer to the same register? */
static inline bool brw_same_reg(struct brw_reg r1, struct brw_reg r2)
{
	return r1.file == r2.file && r1.nr == r2.nr;
}

static inline struct brw_instruction *current_insn( struct brw_compile *p)
{
	return &p->store[p->nr_insn];
}

static inline void brw_set_predicate_control( struct brw_compile *p, unsigned pc )
{
	p->current->header.predicate_control = pc;
}

static inline void brw_set_predicate_inverse(struct brw_compile *p, bool predicate_inverse)
{
	p->current->header.predicate_inverse = predicate_inverse;
}

static inline void brw_set_conditionalmod( struct brw_compile *p, unsigned conditional )
{
	p->current->header.destreg__conditionalmod = conditional;
}

static inline void brw_set_access_mode(struct brw_compile *p, unsigned access_mode)
{
	p->current->header.access_mode = access_mode;
}

static inline void brw_set_mask_control(struct brw_compile *p, unsigned value)
{
	p->current->header.mask_control = value;
}

static inline void brw_set_saturate(struct brw_compile *p, unsigned value)
{
	p->current->header.saturate = value;
}

static inline void brw_set_acc_write_control(struct brw_compile *p, unsigned value)
{
	if (p->gen >= 060)
		p->current->header.acc_wr_control = value;
}

void brw_pop_insn_state(struct brw_compile *p);
void brw_push_insn_state(struct brw_compile *p);
void brw_set_compression_control(struct brw_compile *p, enum brw_compression control);
void brw_set_predicate_control_flag_value( struct brw_compile *p, unsigned value );

void brw_compile_init(struct brw_compile *p, int gen, void *store);

void brw_set_dest(struct brw_compile *p, struct brw_instruction *insn,
		  struct brw_reg dest);
void brw_set_src0(struct brw_compile *p, struct brw_instruction *insn,
		  struct brw_reg reg);
void brw_set_src1(struct brw_compile *p,
		  struct brw_instruction *insn,
		  struct brw_reg reg);

void gen6_resolve_implied_move(struct brw_compile *p,
			       struct brw_reg *src,
			       unsigned msg_reg_nr);

static inline struct brw_instruction *
brw_next_insn(struct brw_compile *p, unsigned opcode)
{
	struct brw_instruction *insn;

	assert(p->nr_insn + 1 < BRW_EU_MAX_INSN);

	insn = &p->store[p->nr_insn++];
	*insn = *p->current;

	if (p->current->header.destreg__conditionalmod) {
		p->current->header.destreg__conditionalmod = 0;
		p->current->header.predicate_control = BRW_PREDICATE_NORMAL;
	}

	insn->header.opcode = opcode;
	return insn;
}

/* Helpers for regular instructions: */
#define ALU1(OP)							\
static inline struct brw_instruction *brw_##OP(struct brw_compile *p,	\
					       struct brw_reg dest,	\
					       struct brw_reg src0)	\
{									\
   return brw_alu1(p, BRW_OPCODE_##OP, dest, src0);			\
}

#define ALU2(OP)							\
static inline struct brw_instruction *brw_##OP(struct brw_compile *p,	\
					       struct brw_reg dest,	\
					       struct brw_reg src0,	\
						struct brw_reg src1)	\
{									\
   return brw_alu2(p, BRW_OPCODE_##OP, dest, src0, src1);		\
}

/* Rounding operations (other than RNDD) require two instructions - the first
 * stores a rounded value (possibly the wrong way) in the dest register, but
 * also sets a per-channel "increment bit" in the flag register.  A predicated
 * add of 1.0 fixes dest to contain the desired result.
 *
 * Sandybridge and later appear to round correctly without an ADD.
 */
#define ROUND(OP)							\
static inline void brw_##OP(struct brw_compile *p,			\
			    struct brw_reg dest,			\
			    struct brw_reg src)				\
{									\
	struct brw_instruction *rnd, *add;				\
	rnd = brw_next_insn(p, BRW_OPCODE_##OP);			\
	brw_set_dest(p, rnd, dest);					\
	brw_set_src0(p, rnd, src);					\
	if (p->gen < 060) {						\
		/* turn on round-increments */				\
		rnd->header.destreg__conditionalmod = BRW_CONDITIONAL_R; \
		add = brw_ADD(p, dest, dest, brw_imm_f(1.0f));		\
		add->header.predicate_control = BRW_PREDICATE_NORMAL;	\
	}								\
}

static inline struct brw_instruction *brw_alu1(struct brw_compile *p,
					       unsigned opcode,
					       struct brw_reg dest,
					       struct brw_reg src)
{
	struct brw_instruction *insn = brw_next_insn(p, opcode);
	brw_set_dest(p, insn, dest);
	brw_set_src0(p, insn, src);
	return insn;
}

static inline struct brw_instruction *brw_alu2(struct brw_compile *p,
					       unsigned opcode,
					       struct brw_reg dest,
					       struct brw_reg src0,
					       struct brw_reg src1 )
{
	struct brw_instruction *insn = brw_next_insn(p, opcode);
	brw_set_dest(p, insn, dest);
	brw_set_src0(p, insn, src0);
	brw_set_src1(p, insn, src1);
	return insn;
}

static inline struct brw_instruction *brw_ADD(struct brw_compile *p,
					      struct brw_reg dest,
					      struct brw_reg src0,
					      struct brw_reg src1)
{
	/* 6.2.2: add */
	if (src0.type == BRW_REGISTER_TYPE_F ||
	    (src0.file == BRW_IMMEDIATE_VALUE &&
	     src0.type == BRW_REGISTER_TYPE_VF)) {
		assert(src1.type != BRW_REGISTER_TYPE_UD);
		assert(src1.type != BRW_REGISTER_TYPE_D);
	}

	if (src1.type == BRW_REGISTER_TYPE_F ||
	    (src1.file == BRW_IMMEDIATE_VALUE &&
	     src1.type == BRW_REGISTER_TYPE_VF)) {
		assert(src0.type != BRW_REGISTER_TYPE_UD);
		assert(src0.type != BRW_REGISTER_TYPE_D);
	}

	return brw_alu2(p, BRW_OPCODE_ADD, dest, src0, src1);
}

static inline struct brw_instruction *brw_MUL(struct brw_compile *p,
					      struct brw_reg dest,
					      struct brw_reg src0,
					      struct brw_reg src1)
{
	/* 6.32.38: mul */
	if (src0.type == BRW_REGISTER_TYPE_D ||
	    src0.type == BRW_REGISTER_TYPE_UD ||
	    src1.type == BRW_REGISTER_TYPE_D ||
	    src1.type == BRW_REGISTER_TYPE_UD) {
		assert(dest.type != BRW_REGISTER_TYPE_F);
	}

	if (src0.type == BRW_REGISTER_TYPE_F ||
	    (src0.file == BRW_IMMEDIATE_VALUE &&
	     src0.type == BRW_REGISTER_TYPE_VF)) {
		assert(src1.type != BRW_REGISTER_TYPE_UD);
		assert(src1.type != BRW_REGISTER_TYPE_D);
	}

	if (src1.type == BRW_REGISTER_TYPE_F ||
	    (src1.file == BRW_IMMEDIATE_VALUE &&
	     src1.type == BRW_REGISTER_TYPE_VF)) {
		assert(src0.type != BRW_REGISTER_TYPE_UD);
		assert(src0.type != BRW_REGISTER_TYPE_D);
	}

	assert(src0.file != BRW_ARCHITECTURE_REGISTER_FILE ||
	       src0.nr != BRW_ARF_ACCUMULATOR);
	assert(src1.file != BRW_ARCHITECTURE_REGISTER_FILE ||
	       src1.nr != BRW_ARF_ACCUMULATOR);

	return brw_alu2(p, BRW_OPCODE_MUL, dest, src0, src1);
}

static inline struct brw_instruction *brw_JMPI(struct brw_compile *p,
					       struct brw_reg dest,
					       struct brw_reg src0,
					       struct brw_reg src1)
{
	struct brw_instruction *insn = brw_alu2(p, BRW_OPCODE_JMPI, dest, src0, src1);

	insn->header.execution_size = 1;
	insn->header.compression_control = BRW_COMPRESSION_NONE;
	insn->header.mask_control = BRW_MASK_DISABLE;

	p->current->header.predicate_control = BRW_PREDICATE_NONE;

	return insn;
}


ALU1(MOV);
ALU2(SEL);
ALU1(NOT);
ALU2(AND);
ALU2(OR);
ALU2(XOR);
ALU2(SHR);
ALU2(SHL);
ALU2(RSR);
ALU2(RSL);
ALU2(ASR);
ALU1(FRC);
ALU1(RNDD);
ALU2(MAC);
ALU2(MACH);
ALU1(LZD);
ALU2(DP4);
ALU2(DPH);
ALU2(DP3);
ALU2(DP2);
ALU2(LINE);
ALU2(PLN);

ROUND(RNDZ);
ROUND(RNDE);

#undef ALU1
#undef ALU2
#undef ROUND

/* Helpers for SEND instruction */
void brw_set_dp_read_message(struct brw_compile *p,
			     struct brw_instruction *insn,
			     unsigned binding_table_index,
			     unsigned msg_control,
			     unsigned msg_type,
			     unsigned target_cache,
			     unsigned msg_length,
			     unsigned response_length);

void brw_set_dp_write_message(struct brw_compile *p,
			      struct brw_instruction *insn,
			      unsigned binding_table_index,
			      unsigned msg_control,
			      unsigned msg_type,
			      unsigned msg_length,
			      bool header_present,
			      bool last_render_target,
			      unsigned response_length,
			      bool end_of_thread,
			      bool send_commit_msg);

void brw_urb_WRITE(struct brw_compile *p,
		   struct brw_reg dest,
		   unsigned msg_reg_nr,
		   struct brw_reg src0,
		   bool allocate,
		   bool used,
		   unsigned msg_length,
		   unsigned response_length,
		   bool eot,
		   bool writes_complete,
		   unsigned offset,
		   unsigned swizzle);

void brw_ff_sync(struct brw_compile *p,
		 struct brw_reg dest,
		 unsigned msg_reg_nr,
		 struct brw_reg src0,
		 bool allocate,
		 unsigned response_length,
		 bool eot);

void brw_fb_WRITE(struct brw_compile *p,
		  int dispatch_width,
                  unsigned msg_reg_nr,
                  struct brw_reg src0,
                  unsigned msg_control,
                  unsigned binding_table_index,
                  unsigned msg_length,
                  unsigned response_length,
                  bool eot,
                  bool header_present);

void brw_SAMPLE(struct brw_compile *p,
		struct brw_reg dest,
		unsigned msg_reg_nr,
		struct brw_reg src0,
		unsigned binding_table_index,
		unsigned sampler,
		unsigned writemask,
		unsigned msg_type,
		unsigned response_length,
		unsigned msg_length,
		bool header_present,
		unsigned simd_mode);

void brw_math_16(struct brw_compile *p,
		 struct brw_reg dest,
		 unsigned function,
		 unsigned saturate,
		 unsigned msg_reg_nr,
		 struct brw_reg src,
		 unsigned precision);

void brw_math(struct brw_compile *p,
	      struct brw_reg dest,
	      unsigned function,
	      unsigned saturate,
	      unsigned msg_reg_nr,
	      struct brw_reg src,
	      unsigned data_type,
	      unsigned precision);

void brw_math2(struct brw_compile *p,
	       struct brw_reg dest,
	       unsigned function,
	       struct brw_reg src0,
	       struct brw_reg src1);

void brw_oword_block_read(struct brw_compile *p,
			  struct brw_reg dest,
			  struct brw_reg mrf,
			  uint32_t offset,
			  uint32_t bind_table_index);

void brw_oword_block_read_scratch(struct brw_compile *p,
				  struct brw_reg dest,
				  struct brw_reg mrf,
				  int num_regs,
				  unsigned offset);

void brw_oword_block_write_scratch(struct brw_compile *p,
				   struct brw_reg mrf,
				   int num_regs,
				   unsigned offset);

void brw_dword_scattered_read(struct brw_compile *p,
			      struct brw_reg dest,
			      struct brw_reg mrf,
			      uint32_t bind_table_index);

void brw_dp_READ_4_vs(struct brw_compile *p,
		      struct brw_reg dest,
		      unsigned location,
		      unsigned bind_table_index);

void brw_dp_READ_4_vs_relative(struct brw_compile *p,
			       struct brw_reg dest,
			       struct brw_reg addrReg,
			       unsigned offset,
			       unsigned bind_table_index);

/* If/else/endif.  Works by manipulating the execution flags on each
 * channel.
 */
struct brw_instruction *brw_IF(struct brw_compile *p,
			       unsigned execute_size);
struct brw_instruction *gen6_IF(struct brw_compile *p, uint32_t conditional,
				struct brw_reg src0, struct brw_reg src1);

void brw_ELSE(struct brw_compile *p);
void brw_ENDIF(struct brw_compile *p);

/* DO/WHILE loops:
*/
struct brw_instruction *brw_DO(struct brw_compile *p,
			       unsigned execute_size);

struct brw_instruction *brw_WHILE(struct brw_compile *p,
				  struct brw_instruction *patch_insn);

struct brw_instruction *brw_BREAK(struct brw_compile *p, int pop_count);
struct brw_instruction *brw_CONT(struct brw_compile *p, int pop_count);
struct brw_instruction *gen6_CONT(struct brw_compile *p,
				  struct brw_instruction *do_insn);
/* Forward jumps:
*/
void brw_land_fwd_jump(struct brw_compile *p,
		       struct brw_instruction *jmp_insn);

void brw_NOP(struct brw_compile *p);

void brw_WAIT(struct brw_compile *p);

/* Special case: there is never a destination, execution size will be
 * taken from src0:
 */
void brw_CMP(struct brw_compile *p,
	     struct brw_reg dest,
	     unsigned conditional,
	     struct brw_reg src0,
	     struct brw_reg src1);

void brw_print_reg(struct brw_reg reg);

static inline void brw_math_invert(struct brw_compile *p,
				   struct brw_reg dst,
				   struct brw_reg src)
{
	brw_math(p,
		 dst,
		 BRW_MATH_FUNCTION_INV,
		 BRW_MATH_SATURATE_NONE,
		 0,
		 src,
		 BRW_MATH_PRECISION_FULL,
		 BRW_MATH_DATA_VECTOR);
}

void brw_set_uip_jip(struct brw_compile *p);

uint32_t brw_swap_cmod(uint32_t cmod);

void brw_disasm(FILE *file,
		const struct brw_instruction *inst,
		int gen);

#endif


//typedef unsigned int uint32_t;

#include "drmP.h"
#include "drm.h"
#include "radeon_drm.h"
#include "radeon.h"

#include "r600_shader.h"
#include "r600_reg.h"
#include "r600_reg_auto_r6xx.h"
#include "r600_reg_r6xx.h"
#include "r600_reg_r7xx.h"



/*
vertex format

struct vertex
{
    float x, y;
    float s, t;
};

vertex shader

VFETCH: ADDR(4) CNT(1) VALID_PIX
  FETCH R0
EXP_DONE POS0, R0.XY01
EXT_DONE PARAM0, R0.ZW01
*/

uint32_t R600_video_vs[] =
{

    /* 0 */
    CF_DWORD0(ADDR(4)),
    CF_DWORD1(POP_COUNT(0),
      CF_CONST(0),
      COND(SQ_CF_COND_ACTIVE),
     /* I_COUNT(1),*/ 0,
      CALL_COUNT(0),
      END_OF_PROGRAM(0),
      VALID_PIXEL_MODE(0),
      CF_INST(SQ_CF_INST_VTX),
      WHOLE_QUAD_MODE(0),
      BARRIER(1)),
    /* 1 */
    CF_ALLOC_IMP_EXP_DWORD0(
      ARRAY_BASE(CF_POS0),
      TYPE(SQ_EXPORT_POS),
      RW_GPR(0),
      RW_REL(ABSOLUTE),
      INDEX_GPR(0),
      ELEM_SIZE(0)),
    CF_ALLOC_IMP_EXP_DWORD1_SWIZ(
      SRC_SEL_X(SQ_SEL_X),
      SRC_SEL_Y(SQ_SEL_Y),
      SRC_SEL_Z(SQ_SEL_0),
      SRC_SEL_W(SQ_SEL_1),
      R6xx_ELEM_LOOP(0),
      BURST_COUNT(0),
      END_OF_PROGRAM(0),
      VALID_PIXEL_MODE(0),
      CF_INST(SQ_CF_INST_EXPORT_DONE),
      WHOLE_QUAD_MODE(0),
      BARRIER(1)),
    /* 2 */
    CF_ALLOC_IMP_EXP_DWORD0(
      ARRAY_BASE(0),
      TYPE(SQ_EXPORT_PARAM),
      RW_GPR(0),
      RW_REL(ABSOLUTE),
      INDEX_GPR(0),
      ELEM_SIZE(0)),
    CF_ALLOC_IMP_EXP_DWORD1_SWIZ(
      SRC_SEL_X(SQ_SEL_Z),
      SRC_SEL_Y(SQ_SEL_W),
      SRC_SEL_Z(SQ_SEL_0),
      SRC_SEL_W(SQ_SEL_1),
      R6xx_ELEM_LOOP(0),
      BURST_COUNT(0),
      END_OF_PROGRAM(1),
      VALID_PIXEL_MODE(0),
      CF_INST(SQ_CF_INST_EXPORT_DONE),
      WHOLE_QUAD_MODE(0),
      BARRIER(0)),
    /* 3 */
    0x00000000,
    0x00000000,
    /* 4/5 */
    VTX_DWORD0(VTX_INST(SQ_VTX_INST_FETCH),
      FETCH_TYPE(SQ_VTX_FETCH_VERTEX_DATA),
      FETCH_WHOLE_QUAD(0),
      BUFFER_ID(0),
      SRC_GPR(0),
      SRC_REL(ABSOLUTE),
      SRC_SEL_X(SQ_SEL_X),
      MEGA_FETCH_COUNT(16)),
    VTX_DWORD1_GPR(DST_GPR(0),
      DST_REL(0),
      DST_SEL_X(SQ_SEL_X),
      DST_SEL_Y(SQ_SEL_Y),
      DST_SEL_Z(SQ_SEL_Z),
      DST_SEL_W(SQ_SEL_W),
      USE_CONST_FIELDS(0),
      DATA_FORMAT(FMT_32_32_32_32_FLOAT),
      NUM_FORMAT_ALL(SQ_NUM_FORMAT_SCALED),
      FORMAT_COMP_ALL(SQ_FORMAT_COMP_SIGNED),
      SRF_MODE_ALL(SRF_MODE_ZERO_CLAMP_MINUS_ONE)),
    VTX_DWORD2(OFFSET(0),
      ENDIAN_SWAP(SQ_ENDIAN_NONE),
      CONST_BUF_NO_STRIDE(0),
      MEGA_FETCH(1)),
    VTX_DWORD_PAD
};


/*
pixel shader

00 TEX: ADDR(2) CNT(1) VALID_PIX
      0  SAMPLE R0, v0.xy01, t0, s0
      1  SAMPLE R1, v0.xy01, t0, s1

01 EXP_DONE: PIX0, R0

02 ALU: ADDR() CNT(1)
      2  KILLNE ____, R1.x___, c0.x___

END_OF_PROGRAM



 */
uint32_t R600_video_ps[]=
{
    /* CF INST 0 */
    CF_DWORD0(ADDR(2)),
    CF_DWORD1(POP_COUNT(0),
        CF_CONST(0),
        COND(SQ_CF_COND_ACTIVE),
     /*   I_COUNT(1), */ 0,
        CALL_COUNT(0),
        END_OF_PROGRAM(0),
        VALID_PIXEL_MODE(0),
        CF_INST(SQ_CF_INST_TEX),
        WHOLE_QUAD_MODE(0),
        BARRIER(1)),

#if 0
    /* CF INST 1 */
    CF_ALU_DWORD0(ADDR( ),
        KCACHE_BANK0(0),
        KCACHE_BANK1(0),
        KCACHE_MODE0(SQ_CF_KCACHE_NOP));
    CF_ALU_DWORD1(KCACHE_MODE1(SQ_CF_KCACHE_NOP),
        KCACHE_ADDR0(0),
        KCACHE_ADDR1(0),
        I_COUNT(1),
        USES_WATERFALL(0),
        CF_INST(SQ_CF_INST_ALU),
        WHOLE_QUAD_MODE(0),
        BARRIER(1));
#endif

    /* CF INST 1 */
    CF_ALLOC_IMP_EXP_DWORD0(ARRAY_BASE(CF_PIXEL_MRT0),
        TYPE(SQ_EXPORT_PIXEL),
        RW_GPR(0),
        RW_REL(ABSOLUTE),
        INDEX_GPR(0),
        ELEM_SIZE(1)),
    CF_ALLOC_IMP_EXP_DWORD1_SWIZ(SRC_SEL_X(SQ_SEL_X),
        SRC_SEL_Y(SQ_SEL_Y),
        SRC_SEL_Z(SQ_SEL_Z),
        SRC_SEL_W(SQ_SEL_W),
        R6xx_ELEM_LOOP(0),
        BURST_COUNT(1),
        END_OF_PROGRAM(1),
        VALID_PIXEL_MODE(0),
        CF_INST(SQ_CF_INST_EXPORT_DONE),
        WHOLE_QUAD_MODE(0),
        BARRIER(1)),

#if 0
    /*  KILLNE  c0.x, r1.x   */
    ALU_DWORD0(SRC0_SEL(ALU_SRC_CFILE_BASE + 0),
        SRC0_REL(ABSOLUTE),
        SRC0_ELEM(ELEM_X),
        SRC0_NEG(0),
        SRC1_SEL(ALU_SRC_GPR_BASE + 1),
        SRC1_REL(ABSOLUTE),
        SRC1_ELEM(ELEM_X),
        SRC1_NEG(0),
        INDEX_MODE(SQ_INDEX_LOOP),
        PRED_SEL(SQ_PRED_SEL_OFF),
        LAST(1)),
    R7xx_ALU_DWORD1_OP2(SRC0_ABS(0),
        SRC1_ABS(0),
        UPDATE_EXECUTE_MASK(0),
        UPDATE_PRED(0),
        WRITE_MASK(0),
        FOG_MERGE(0),
        OMOD(SQ_ALU_OMOD_OFF),
        ALU_INST(SQ_OP2_INST_KILLNE),
        BANK_SWIZZLE(SQ_ALU_VEC_012),
        DST_GPR(0),
        DST_REL(ABSOLUTE),
        DST_ELEM(ELEM_X),
        CLAMP(0)),

#endif

    /* TEX INST 0 */
    TEX_DWORD0(TEX_INST(SQ_TEX_INST_SAMPLE),
        BC_FRAC_MODE(0),
        FETCH_WHOLE_QUAD(0),
        RESOURCE_ID(0),
        SRC_GPR(0),
        SRC_REL(ABSOLUTE),
        R7xx_ALT_CONST(0)),
    TEX_DWORD1(DST_GPR(0),
        DST_REL(ABSOLUTE),
        DST_SEL_X(SQ_SEL_X), /* R */
        DST_SEL_Y(SQ_SEL_Y), /* G */
        DST_SEL_Z(SQ_SEL_Z), /* B */
        DST_SEL_W(SQ_SEL_W), /* A */
        LOD_BIAS(0),
        COORD_TYPE_X(TEX_UNNORMALIZED),
        COORD_TYPE_Y(TEX_UNNORMALIZED),
        COORD_TYPE_Z(TEX_UNNORMALIZED),
        COORD_TYPE_W(TEX_UNNORMALIZED)),
    TEX_DWORD2(OFFSET_X(0),
        OFFSET_Y(0),
        OFFSET_Z(0),
        SAMPLER_ID(0),
        SRC_SEL_X(SQ_SEL_X),
        SRC_SEL_Y(SQ_SEL_Y),
        SRC_SEL_Z(SQ_SEL_0),
        SRC_SEL_W(SQ_SEL_1)),
    TEX_DWORD_PAD

#if 0
    TEX_DWORD0(TEX_INST(SQ_TEX_INST_SAMPLE),
        BC_FRAC_MODE(0),
        FETCH_WHOLE_QUAD(0),
        RESOURCE_ID(1),
        SRC_GPR(0),
        SRC_REL(ABSOLUTE),
        R7xx_ALT_CONST(0)),
    TEX_DWORD1(DST_GPR(1),
        DST_REL(ABSOLUTE),
        DST_SEL_X(SQ_SEL_X), /* R */
        DST_SEL_Y(SQ_SEL_MASK), /* G */
        DST_SEL_Z(SQ_SEL_MASK), /* B */
        DST_SEL_W(SQ_SEL_MASK), /* A */
        LOD_BIAS(0),
        COORD_TYPE_X(TEX_UNNORMALIZED),
        COORD_TYPE_Y(TEX_UNNORMALIZED),
        COORD_TYPE_Z(TEX_UNNORMALIZED),
        COORD_TYPE_W(TEX_UNNORMALIZED)),
    TEX_DWORD2(OFFSET_X(0),
        OFFSET_Y(0),
        OFFSET_Z(0),
        SAMPLER_ID(0),
        SRC_SEL_X(SQ_SEL_X),
        SRC_SEL_Y(SQ_SEL_Y),
        SRC_SEL_Z(SQ_SEL_0),
        SRC_SEL_W(SQ_SEL_1)),
    TEX_DWORD_PAD

#endif
};

const u32 r600_video_ps_size = ARRAY_SIZE(R600_video_ps);
const u32 r600_video_vs_size = ARRAY_SIZE(R600_video_vs);



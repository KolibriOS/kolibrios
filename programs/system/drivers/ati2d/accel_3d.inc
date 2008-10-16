
#include "radeon_reg.h"

#define BEGIN_ACCEL(n)          BEGIN_RING(2*(n))
//#define FINISH_ACCEL()          ADVANCE_RING()
#define FINISH_ACCEL()          COMMIT_RING()

#define OUT_ACCEL_REG(reg, val) OUT_RING_REG(reg, val)

#define RADEON_SWITCH_TO_3D()                                           \
do {									\
        u32_t wait_until = 0;                        \
	BEGIN_ACCEL(1);							\
        wait_until |= RADEON_WAIT_HOST_IDLECLEAN | RADEON_WAIT_3D_IDLECLEAN;        \
	OUT_ACCEL_REG(RADEON_WAIT_UNTIL, wait_until);			\
	FINISH_ACCEL();							\
} while (0);


struct blendinfo {
    Bool dst_alpha;
    Bool src_alpha;
    u32_t blend_cntl;
};

static struct blendinfo RadeonBlendOp[] = {
    /* 0 - Clear */
    {0, 0, RADEON_SRC_BLEND_GL_ZERO	      | RADEON_DST_BLEND_GL_ZERO},
    /* 1 - Src */
    {0, 0, RADEON_SRC_BLEND_GL_ONE	      | RADEON_DST_BLEND_GL_ZERO},
    /* 2 - Dst */
    {0, 0, RADEON_SRC_BLEND_GL_ZERO	      | RADEON_DST_BLEND_GL_ONE},
    /* 3 - Over */
    {0, 1, RADEON_SRC_BLEND_GL_ONE	      | RADEON_DST_BLEND_GL_ONE_MINUS_SRC_ALPHA},
    /* 4 - OverReverse */
    {1, 0, RADEON_SRC_BLEND_GL_ONE_MINUS_DST_ALPHA | RADEON_DST_BLEND_GL_ONE},
    /* 5 - In */
    {1, 0, RADEON_SRC_BLEND_GL_DST_ALPHA     | RADEON_DST_BLEND_GL_ZERO},
    /* 6 - InReverse */
    {0, 1, RADEON_SRC_BLEND_GL_ZERO	      | RADEON_DST_BLEND_GL_SRC_ALPHA},
    /* 7 - Out */
    {1, 0, RADEON_SRC_BLEND_GL_ONE_MINUS_DST_ALPHA | RADEON_DST_BLEND_GL_ZERO},
    /* 8 - OutReverse */
    {0, 1, RADEON_SRC_BLEND_GL_ZERO	      | RADEON_DST_BLEND_GL_ONE_MINUS_SRC_ALPHA},
    /* 9 - Atop */
    {1, 1, RADEON_SRC_BLEND_GL_DST_ALPHA     | RADEON_DST_BLEND_GL_ONE_MINUS_SRC_ALPHA},
    /* 10- AtopReverse */
    {1, 1, RADEON_SRC_BLEND_GL_ONE_MINUS_DST_ALPHA | RADEON_DST_BLEND_GL_SRC_ALPHA},
    /* 11 - Xor */
    {1, 1, RADEON_SRC_BLEND_GL_ONE_MINUS_DST_ALPHA | RADEON_DST_BLEND_GL_ONE_MINUS_SRC_ALPHA},
    /* 12 - Add */
    {0, 0, RADEON_SRC_BLEND_GL_ONE	      | RADEON_DST_BLEND_GL_ONE},
};


static void Init3DEngine(RHDPtr rhdPtr)
{
   // RADEONInfoPtr  info       = RADEONPTR(pScrn);
    u32_t gb_tile_config, su_reg_dest, vap_cntl;
   // ACCEL_PREAMBLE();

    u32_t *ring, write;

   // info->texW[0] = info->texH[0] = info->texW[1] = info->texH[1] = 1;

    if (IS_R300_3D || IS_R500_3D)
    {

        BEGIN_ACCEL(3);
	OUT_ACCEL_REG(R300_RB3D_DSTCACHE_CTLSTAT, R300_DC_FLUSH_3D | R300_DC_FREE_3D);
	OUT_ACCEL_REG(R300_RB3D_ZCACHE_CTLSTAT, R300_ZC_FLUSH | R300_ZC_FREE);
	OUT_ACCEL_REG(RADEON_WAIT_UNTIL, RADEON_WAIT_2D_IDLECLEAN | RADEON_WAIT_3D_IDLECLEAN);
	FINISH_ACCEL();

        if ( IS_R500_3D)
        {
           u32_t gb_pipe_sel = INREG(R400_GB_PIPE_SELECT);

           rhdPtr->num_gb_pipes = ((gb_pipe_sel >> 12) & 0x3) + 1;
           RADEONOUTPLL(R500_DYN_SCLK_PWMEM_PIPE, (1 | ((gb_pipe_sel >> 8) & 0xf) << 4));
	}

        dbgprintf("Pipes count %d\n", rhdPtr->num_gb_pipes );

	gb_tile_config = (R300_ENABLE_TILING | R300_TILE_SIZE_16 | R300_SUBPIXEL_1_16);

        switch(rhdPtr->num_gb_pipes) {
	case 2: gb_tile_config |= R300_PIPE_COUNT_R300; break;
	case 3: gb_tile_config |= R300_PIPE_COUNT_R420_3P; break;
	case 4: gb_tile_config |= R300_PIPE_COUNT_R420; break;
	default:
	case 1: gb_tile_config |= R300_PIPE_COUNT_RV350; break;
	}

	BEGIN_ACCEL(5);
	OUT_ACCEL_REG(R300_GB_TILE_CONFIG, gb_tile_config);
	OUT_ACCEL_REG(RADEON_WAIT_UNTIL, RADEON_WAIT_2D_IDLECLEAN | RADEON_WAIT_3D_IDLECLEAN);
	OUT_ACCEL_REG(R300_DST_PIPE_CONFIG, R300_PIPE_AUTO_CONFIG);
	OUT_ACCEL_REG(R300_GB_SELECT, 0);
	OUT_ACCEL_REG(R300_GB_ENABLE, 0);
	FINISH_ACCEL();

	if (IS_R500_3D) {
            su_reg_dest = ((1 << rhdPtr->num_gb_pipes) - 1);
	    BEGIN_ACCEL(2);
	    OUT_ACCEL_REG(R500_SU_REG_DEST, su_reg_dest);
	    OUT_ACCEL_REG(R500_VAP_INDEX_OFFSET, 0);
	    FINISH_ACCEL();
	}

	BEGIN_ACCEL(3);
	OUT_ACCEL_REG(R300_RB3D_DSTCACHE_CTLSTAT, R300_DC_FLUSH_3D | R300_DC_FREE_3D);
	OUT_ACCEL_REG(R300_RB3D_ZCACHE_CTLSTAT, R300_ZC_FLUSH | R300_ZC_FREE);
	OUT_ACCEL_REG(RADEON_WAIT_UNTIL, RADEON_WAIT_2D_IDLECLEAN | RADEON_WAIT_3D_IDLECLEAN);
	FINISH_ACCEL();

	BEGIN_ACCEL(5);
	OUT_ACCEL_REG(R300_GB_AA_CONFIG, 0);
	OUT_ACCEL_REG(R300_RB3D_DSTCACHE_CTLSTAT, R300_DC_FLUSH_3D | R300_DC_FREE_3D);
	OUT_ACCEL_REG(R300_RB3D_ZCACHE_CTLSTAT, R300_ZC_FLUSH | R300_ZC_FREE);
	OUT_ACCEL_REG(R300_GB_MSPOS0, ((8 << R300_MS_X0_SHIFT) |
				       (8 << R300_MS_Y0_SHIFT) |
				       (8 << R300_MS_X1_SHIFT) |
				       (8 << R300_MS_Y1_SHIFT) |
				       (8 << R300_MS_X2_SHIFT) |
				       (8 << R300_MS_Y2_SHIFT) |
				       (8 << R300_MSBD0_Y_SHIFT) |
				       (7 << R300_MSBD0_X_SHIFT)));
	OUT_ACCEL_REG(R300_GB_MSPOS1, ((8 << R300_MS_X3_SHIFT) |
				       (8 << R300_MS_Y3_SHIFT) |
				       (8 << R300_MS_X4_SHIFT) |
				       (8 << R300_MS_Y4_SHIFT) |
				       (8 << R300_MS_X5_SHIFT) |
				       (8 << R300_MS_Y5_SHIFT) |
				       (8 << R300_MSBD1_SHIFT)));
	FINISH_ACCEL();

	BEGIN_ACCEL(5);
	OUT_ACCEL_REG(R300_GA_ENHANCE, R300_GA_DEADLOCK_CNTL | R300_GA_FASTSYNC_CNTL);
	OUT_ACCEL_REG(R300_GA_POLY_MODE, R300_FRONT_PTYPE_TRIANGE | R300_BACK_PTYPE_TRIANGE);
	OUT_ACCEL_REG(R300_GA_ROUND_MODE, (R300_GEOMETRY_ROUND_NEAREST |
					   R300_COLOR_ROUND_NEAREST));
	OUT_ACCEL_REG(R300_GA_COLOR_CONTROL, (R300_RGB0_SHADING_GOURAUD |
					      R300_ALPHA0_SHADING_GOURAUD |
					      R300_RGB1_SHADING_GOURAUD |
					      R300_ALPHA1_SHADING_GOURAUD |
					      R300_RGB2_SHADING_GOURAUD |
					      R300_ALPHA2_SHADING_GOURAUD |
					      R300_RGB3_SHADING_GOURAUD |
					      R300_ALPHA3_SHADING_GOURAUD));
	OUT_ACCEL_REG(R300_GA_OFFSET, 0);
	FINISH_ACCEL();

	BEGIN_ACCEL(5);
	OUT_ACCEL_REG(R300_SU_TEX_WRAP, 0);
	OUT_ACCEL_REG(R300_SU_POLY_OFFSET_ENABLE, 0);
	OUT_ACCEL_REG(R300_SU_CULL_MODE, R300_FACE_NEG);
	OUT_ACCEL_REG(R300_SU_DEPTH_SCALE, 0x4b7fffff);
	OUT_ACCEL_REG(R300_SU_DEPTH_OFFSET, 0);
	FINISH_ACCEL();

	/* setup the VAP */
        if (rhdPtr->has_tcl)
	    vap_cntl = ((5 << R300_PVS_NUM_SLOTS_SHIFT) |
			(5 << R300_PVS_NUM_CNTLRS_SHIFT) |
			(9 << R300_VF_MAX_VTX_NUM_SHIFT));
	else
	    vap_cntl = ((10 << R300_PVS_NUM_SLOTS_SHIFT) |
			(5 << R300_PVS_NUM_CNTLRS_SHIFT) |
			(5 << R300_VF_MAX_VTX_NUM_SHIFT));

        if (rhdPtr->ChipSet == RHD_FAMILY_RV515)
	    vap_cntl |= (2 << R300_PVS_NUM_FPUS_SHIFT);
        else if ((rhdPtr->ChipSet == RHD_FAMILY_RV530) ||
                 (rhdPtr->ChipSet == RHD_FAMILY_RV560))
	    vap_cntl |= (5 << R300_PVS_NUM_FPUS_SHIFT);
        else if (rhdPtr->ChipSet == RHD_FAMILY_R420)
	    vap_cntl |= (6 << R300_PVS_NUM_FPUS_SHIFT);
        else if ((rhdPtr->ChipSet == RHD_FAMILY_R520) ||
                 (rhdPtr->ChipSet == RHD_FAMILY_R580) ||
                 (rhdPtr->ChipSet == RHD_FAMILY_RV570))
	    vap_cntl |= (8 << R300_PVS_NUM_FPUS_SHIFT);
	else
	    vap_cntl |= (4 << R300_PVS_NUM_FPUS_SHIFT);

        if (rhdPtr->has_tcl)
	    BEGIN_ACCEL(15);
	else
	    BEGIN_ACCEL(9);
	OUT_ACCEL_REG(R300_VAP_VTX_STATE_CNTL, 0);
	OUT_ACCEL_REG(R300_VAP_PVS_STATE_FLUSH_REG, 0);

        if (rhdPtr->has_tcl)
	    OUT_ACCEL_REG(R300_VAP_CNTL_STATUS, 0);
	else
	    OUT_ACCEL_REG(R300_VAP_CNTL_STATUS, R300_PVS_BYPASS);
	OUT_ACCEL_REG(R300_VAP_CNTL, vap_cntl);
	OUT_ACCEL_REG(R300_VAP_PVS_STATE_FLUSH_REG, 0);
	OUT_ACCEL_REG(R300_VAP_VTE_CNTL, R300_VTX_XY_FMT | R300_VTX_Z_FMT);
	OUT_ACCEL_REG(R300_VAP_PSC_SGN_NORM_CNTL, 0);

	OUT_ACCEL_REG(R300_VAP_PROG_STREAM_CNTL_EXT_0,
		      ((R300_SWIZZLE_SELECT_X << R300_SWIZZLE_SELECT_X_0_SHIFT) |
		       (R300_SWIZZLE_SELECT_Y << R300_SWIZZLE_SELECT_Y_0_SHIFT) |
		       (R300_SWIZZLE_SELECT_Z << R300_SWIZZLE_SELECT_Z_0_SHIFT) |
		       (R300_SWIZZLE_SELECT_W << R300_SWIZZLE_SELECT_W_0_SHIFT) |
		       ((R300_WRITE_ENA_X | R300_WRITE_ENA_Y | R300_WRITE_ENA_Z | R300_WRITE_ENA_W)
			<< R300_WRITE_ENA_0_SHIFT) |
		       (R300_SWIZZLE_SELECT_X << R300_SWIZZLE_SELECT_X_1_SHIFT) |
		       (R300_SWIZZLE_SELECT_Y << R300_SWIZZLE_SELECT_Y_1_SHIFT) |
		       (R300_SWIZZLE_SELECT_Z << R300_SWIZZLE_SELECT_Z_1_SHIFT) |
		       (R300_SWIZZLE_SELECT_W << R300_SWIZZLE_SELECT_W_1_SHIFT) |
		       ((R300_WRITE_ENA_X | R300_WRITE_ENA_Y | R300_WRITE_ENA_Z | R300_WRITE_ENA_W)
			<< R300_WRITE_ENA_1_SHIFT)));
	OUT_ACCEL_REG(R300_VAP_PROG_STREAM_CNTL_EXT_1,
		      ((R300_SWIZZLE_SELECT_X << R300_SWIZZLE_SELECT_X_2_SHIFT) |
		       (R300_SWIZZLE_SELECT_Y << R300_SWIZZLE_SELECT_Y_2_SHIFT) |
		       (R300_SWIZZLE_SELECT_Z << R300_SWIZZLE_SELECT_Z_2_SHIFT) |
		       (R300_SWIZZLE_SELECT_W << R300_SWIZZLE_SELECT_W_2_SHIFT) |
		       ((R300_WRITE_ENA_X | R300_WRITE_ENA_Y | R300_WRITE_ENA_Z | R300_WRITE_ENA_W)
			<< R300_WRITE_ENA_2_SHIFT)));

        if (rhdPtr->has_tcl) {
	    OUT_ACCEL_REG(R300_VAP_PVS_FLOW_CNTL_OPC, 0);
	    OUT_ACCEL_REG(R300_VAP_GB_VERT_CLIP_ADJ, 0x3f800000);
	    OUT_ACCEL_REG(R300_VAP_GB_VERT_DISC_ADJ, 0x3f800000);
	    OUT_ACCEL_REG(R300_VAP_GB_HORZ_CLIP_ADJ, 0x3f800000);
	    OUT_ACCEL_REG(R300_VAP_GB_HORZ_DISC_ADJ, 0x3f800000);
	    OUT_ACCEL_REG(R300_VAP_CLIP_CNTL, R300_CLIP_DISABLE);
	}
	FINISH_ACCEL();

	/* pre-load the vertex shaders */
        if (rhdPtr->has_tcl) {
	    /* exa mask shader program */
	    BEGIN_ACCEL(13);
	    OUT_ACCEL_REG(R300_VAP_PVS_VECTOR_INDX_REG, 0);
	    /* PVS inst 0 */
	    OUT_ACCEL_REG(R300_VAP_PVS_VECTOR_DATA_REG,
			  (R300_PVS_DST_OPCODE(R300_VE_ADD) |
			   R300_PVS_DST_REG_TYPE(R300_PVS_DST_REG_OUT) |
			   R300_PVS_DST_OFFSET(0) |
			   R300_PVS_DST_WE_X | R300_PVS_DST_WE_Y |
			   R300_PVS_DST_WE_Z | R300_PVS_DST_WE_W));
	    OUT_ACCEL_REG(R300_VAP_PVS_VECTOR_DATA_REG,
			  (R300_PVS_SRC_REG_TYPE(R300_PVS_SRC_REG_INPUT) |
			   R300_PVS_SRC_OFFSET(0) |
			   R300_PVS_SRC_SWIZZLE_X(R300_PVS_SRC_SELECT_X) |
			   R300_PVS_SRC_SWIZZLE_Y(R300_PVS_SRC_SELECT_Y) |
			   R300_PVS_SRC_SWIZZLE_Z(R300_PVS_SRC_SELECT_Z) |
			   R300_PVS_SRC_SWIZZLE_W(R300_PVS_SRC_SELECT_W)));
	    OUT_ACCEL_REG(R300_VAP_PVS_VECTOR_DATA_REG,
			  (R300_PVS_SRC_REG_TYPE(R300_PVS_SRC_REG_INPUT) |
			   R300_PVS_SRC_OFFSET(0) |
			   R300_PVS_SRC_SWIZZLE_X(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_Y(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_Z(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_W(R300_PVS_SRC_SELECT_FORCE_0)));
	    OUT_ACCEL_REG(R300_VAP_PVS_VECTOR_DATA_REG,
			  (R300_PVS_SRC_REG_TYPE(R300_PVS_SRC_REG_INPUT) |
			   R300_PVS_SRC_OFFSET(0) |
			   R300_PVS_SRC_SWIZZLE_X(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_Y(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_Z(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_W(R300_PVS_SRC_SELECT_FORCE_0)));

	    /* PVS inst 1 */
	    OUT_ACCEL_REG(R300_VAP_PVS_VECTOR_DATA_REG,
			  (R300_PVS_DST_OPCODE(R300_VE_ADD) |
			   R300_PVS_DST_REG_TYPE(R300_PVS_DST_REG_OUT) |
			   R300_PVS_DST_OFFSET(1) |
			   R300_PVS_DST_WE_X | R300_PVS_DST_WE_Y |
			   R300_PVS_DST_WE_Z | R300_PVS_DST_WE_W));
	    OUT_ACCEL_REG(R300_VAP_PVS_VECTOR_DATA_REG,
			  (R300_PVS_SRC_REG_TYPE(R300_PVS_SRC_REG_INPUT) |
			   R300_PVS_SRC_OFFSET(6) |
			   R300_PVS_SRC_SWIZZLE_X(R300_PVS_SRC_SELECT_X) |
			   R300_PVS_SRC_SWIZZLE_Y(R300_PVS_SRC_SELECT_Y) |
			   R300_PVS_SRC_SWIZZLE_Z(R300_PVS_SRC_SELECT_Z) |
			   R300_PVS_SRC_SWIZZLE_W(R300_PVS_SRC_SELECT_W)));
	    OUT_ACCEL_REG(R300_VAP_PVS_VECTOR_DATA_REG,
			  (R300_PVS_SRC_REG_TYPE(R300_PVS_SRC_REG_INPUT) |
			   R300_PVS_SRC_OFFSET(6) |
			   R300_PVS_SRC_SWIZZLE_X(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_Y(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_Z(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_W(R300_PVS_SRC_SELECT_FORCE_0)));
	    OUT_ACCEL_REG(R300_VAP_PVS_VECTOR_DATA_REG,
			  (R300_PVS_SRC_REG_TYPE(R300_PVS_SRC_REG_INPUT) |
			   R300_PVS_SRC_OFFSET(6) |
			   R300_PVS_SRC_SWIZZLE_X(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_Y(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_Z(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_W(R300_PVS_SRC_SELECT_FORCE_0)));

	    /* PVS inst 2 */
	    OUT_ACCEL_REG(R300_VAP_PVS_VECTOR_DATA_REG,
			  (R300_PVS_DST_OPCODE(R300_VE_ADD) |
			   R300_PVS_DST_REG_TYPE(R300_PVS_DST_REG_OUT) |
			   R300_PVS_DST_OFFSET(2) |
			   R300_PVS_DST_WE_X | R300_PVS_DST_WE_Y |
			   R300_PVS_DST_WE_Z | R300_PVS_DST_WE_W));
	    OUT_ACCEL_REG(R300_VAP_PVS_VECTOR_DATA_REG,
			  (R300_PVS_SRC_REG_TYPE(R300_PVS_SRC_REG_INPUT) |
			   R300_PVS_SRC_OFFSET(7) |
			   R300_PVS_SRC_SWIZZLE_X(R300_PVS_SRC_SELECT_X) |
			   R300_PVS_SRC_SWIZZLE_Y(R300_PVS_SRC_SELECT_Y) |
			   R300_PVS_SRC_SWIZZLE_Z(R300_PVS_SRC_SELECT_Z) |
			   R300_PVS_SRC_SWIZZLE_W(R300_PVS_SRC_SELECT_W)));
	    OUT_ACCEL_REG(R300_VAP_PVS_VECTOR_DATA_REG,
			  (R300_PVS_SRC_REG_TYPE(R300_PVS_SRC_REG_INPUT) |
			   R300_PVS_SRC_OFFSET(7) |
			   R300_PVS_SRC_SWIZZLE_X(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_Y(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_Z(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_W(R300_PVS_SRC_SELECT_FORCE_0)));
	    OUT_ACCEL_REG(R300_VAP_PVS_VECTOR_DATA_REG,
			  (R300_PVS_SRC_REG_TYPE(R300_PVS_SRC_REG_INPUT) |
			   R300_PVS_SRC_OFFSET(7) |
			   R300_PVS_SRC_SWIZZLE_X(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_Y(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_Z(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_W(R300_PVS_SRC_SELECT_FORCE_0)));
	    FINISH_ACCEL();

	    BEGIN_ACCEL(9);
	    /* exa no mask instruction */
	    OUT_ACCEL_REG(R300_VAP_PVS_VECTOR_INDX_REG, 3);
	    /* PVS inst 0 */
	    OUT_ACCEL_REG(R300_VAP_PVS_VECTOR_DATA_REG,
			  (R300_PVS_DST_OPCODE(R300_VE_ADD) |
			   R300_PVS_DST_REG_TYPE(R300_PVS_DST_REG_OUT) |
			   R300_PVS_DST_OFFSET(0) |
			   R300_PVS_DST_WE_X | R300_PVS_DST_WE_Y |
			   R300_PVS_DST_WE_Z | R300_PVS_DST_WE_W));
	    OUT_ACCEL_REG(R300_VAP_PVS_VECTOR_DATA_REG,
			  (R300_PVS_SRC_REG_TYPE(R300_PVS_SRC_REG_INPUT) |
			   R300_PVS_SRC_OFFSET(0) |
			   R300_PVS_SRC_SWIZZLE_X(R300_PVS_SRC_SELECT_X) |
			   R300_PVS_SRC_SWIZZLE_Y(R300_PVS_SRC_SELECT_Y) |
			   R300_PVS_SRC_SWIZZLE_Z(R300_PVS_SRC_SELECT_Z) |
			   R300_PVS_SRC_SWIZZLE_W(R300_PVS_SRC_SELECT_W)));
	    OUT_ACCEL_REG(R300_VAP_PVS_VECTOR_DATA_REG,
			  (R300_PVS_SRC_REG_TYPE(R300_PVS_SRC_REG_INPUT) |
			   R300_PVS_SRC_OFFSET(0) |
			   R300_PVS_SRC_SWIZZLE_X(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_Y(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_Z(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_W(R300_PVS_SRC_SELECT_FORCE_0)));
	    OUT_ACCEL_REG(R300_VAP_PVS_VECTOR_DATA_REG,
			  (R300_PVS_SRC_REG_TYPE(R300_PVS_SRC_REG_INPUT) |
			   R300_PVS_SRC_OFFSET(0) |
			   R300_PVS_SRC_SWIZZLE_X(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_Y(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_Z(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_W(R300_PVS_SRC_SELECT_FORCE_0)));

	    /* PVS inst 1 */
	    OUT_ACCEL_REG(R300_VAP_PVS_VECTOR_DATA_REG,
			  (R300_PVS_DST_OPCODE(R300_VE_ADD) |
			   R300_PVS_DST_REG_TYPE(R300_PVS_DST_REG_OUT) |
			   R300_PVS_DST_OFFSET(1) |
			   R300_PVS_DST_WE_X | R300_PVS_DST_WE_Y |
			   R300_PVS_DST_WE_Z | R300_PVS_DST_WE_W));
	    OUT_ACCEL_REG(R300_VAP_PVS_VECTOR_DATA_REG,
			  (R300_PVS_SRC_REG_TYPE(R300_PVS_SRC_REG_INPUT) |
			   R300_PVS_SRC_OFFSET(6) |
			   R300_PVS_SRC_SWIZZLE_X(R300_PVS_SRC_SELECT_X) |
			   R300_PVS_SRC_SWIZZLE_Y(R300_PVS_SRC_SELECT_Y) |
			   R300_PVS_SRC_SWIZZLE_Z(R300_PVS_SRC_SELECT_Z) |
			   R300_PVS_SRC_SWIZZLE_W(R300_PVS_SRC_SELECT_W)));
	    OUT_ACCEL_REG(R300_VAP_PVS_VECTOR_DATA_REG,
			  (R300_PVS_SRC_REG_TYPE(R300_PVS_SRC_REG_INPUT) |
			   R300_PVS_SRC_OFFSET(6) |
			   R300_PVS_SRC_SWIZZLE_X(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_Y(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_Z(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_W(R300_PVS_SRC_SELECT_FORCE_0)));
	    OUT_ACCEL_REG(R300_VAP_PVS_VECTOR_DATA_REG,
			  (R300_PVS_SRC_REG_TYPE(R300_PVS_SRC_REG_INPUT) |
			   R300_PVS_SRC_OFFSET(6) |
			   R300_PVS_SRC_SWIZZLE_X(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_Y(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_Z(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_W(R300_PVS_SRC_SELECT_FORCE_0)));
	    FINISH_ACCEL();

	    /* Xv shader program */
	    BEGIN_ACCEL(9);
	    OUT_ACCEL_REG(R300_VAP_PVS_VECTOR_INDX_REG, 5);

	    OUT_ACCEL_REG(R300_VAP_PVS_VECTOR_DATA_REG,
			  (R300_PVS_DST_OPCODE(R300_VE_ADD) |
			   R300_PVS_DST_REG_TYPE(R300_PVS_DST_REG_OUT) |
			   R300_PVS_DST_OFFSET(0) |
			   R300_PVS_DST_WE_X | R300_PVS_DST_WE_Y |
			   R300_PVS_DST_WE_Z | R300_PVS_DST_WE_W));
	    OUT_ACCEL_REG(R300_VAP_PVS_VECTOR_DATA_REG,
			  (R300_PVS_SRC_REG_TYPE(R300_PVS_SRC_REG_INPUT) |
			   R300_PVS_SRC_OFFSET(0) |
			   R300_PVS_SRC_SWIZZLE_X(R300_PVS_SRC_SELECT_X) |
			   R300_PVS_SRC_SWIZZLE_Y(R300_PVS_SRC_SELECT_Y) |
			   R300_PVS_SRC_SWIZZLE_Z(R300_PVS_SRC_SELECT_Z) |
			   R300_PVS_SRC_SWIZZLE_W(R300_PVS_SRC_SELECT_W)));
	    OUT_ACCEL_REG(R300_VAP_PVS_VECTOR_DATA_REG,
			  (R300_PVS_SRC_REG_TYPE(R300_PVS_SRC_REG_INPUT) |
			   R300_PVS_SRC_OFFSET(0) |
			   R300_PVS_SRC_SWIZZLE_X(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_Y(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_Z(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_W(R300_PVS_SRC_SELECT_FORCE_0)));
	    OUT_ACCEL_REG(R300_VAP_PVS_VECTOR_DATA_REG,
			  (R300_PVS_SRC_REG_TYPE(R300_PVS_SRC_REG_INPUT) |
			   R300_PVS_SRC_OFFSET(0) |
			   R300_PVS_SRC_SWIZZLE_X(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_Y(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_Z(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_W(R300_PVS_SRC_SELECT_FORCE_0)));

	    OUT_ACCEL_REG(R300_VAP_PVS_VECTOR_DATA_REG,
			  (R300_PVS_DST_OPCODE(R300_VE_ADD) |
			   R300_PVS_DST_REG_TYPE(R300_PVS_DST_REG_OUT) |
			   R300_PVS_DST_OFFSET(1) |
			   R300_PVS_DST_WE_X | R300_PVS_DST_WE_Y |
			   R300_PVS_DST_WE_Z | R300_PVS_DST_WE_W));
	    OUT_ACCEL_REG(R300_VAP_PVS_VECTOR_DATA_REG,
			  (R300_PVS_SRC_REG_TYPE(R300_PVS_SRC_REG_INPUT) |
			   R300_PVS_SRC_OFFSET(6) |
			   R300_PVS_SRC_SWIZZLE_X(R300_PVS_SRC_SELECT_X) |
			   R300_PVS_SRC_SWIZZLE_Y(R300_PVS_SRC_SELECT_Y) |
			   R300_PVS_SRC_SWIZZLE_Z(R300_PVS_SRC_SELECT_Z) |
			   R300_PVS_SRC_SWIZZLE_W(R300_PVS_SRC_SELECT_W)));
	    OUT_ACCEL_REG(R300_VAP_PVS_VECTOR_DATA_REG,
			  (R300_PVS_SRC_REG_TYPE(R300_PVS_SRC_REG_INPUT) |
			   R300_PVS_SRC_OFFSET(6) |
			   R300_PVS_SRC_SWIZZLE_X(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_Y(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_Z(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_W(R300_PVS_SRC_SELECT_FORCE_0)));
	    OUT_ACCEL_REG(R300_VAP_PVS_VECTOR_DATA_REG,
			  (R300_PVS_SRC_REG_TYPE(R300_PVS_SRC_REG_INPUT) |
			   R300_PVS_SRC_OFFSET(6) |
			   R300_PVS_SRC_SWIZZLE_X(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_Y(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_Z(R300_PVS_SRC_SELECT_FORCE_0) |
			   R300_PVS_SRC_SWIZZLE_W(R300_PVS_SRC_SELECT_FORCE_0)));
	    FINISH_ACCEL();
	}

	/* pre-load the RS instructions */
	BEGIN_ACCEL(4);
	if (IS_R300_3D) {
	    /* rasterizer source table
	     * R300_RS_TEX_PTR is the offset into the input RS stream
	     * 0,1 are tex0
	     * 2,3 are tex1
	     */
	    OUT_ACCEL_REG(R300_RS_IP_0,
			  (R300_RS_TEX_PTR(0) |
			   R300_RS_SEL_S(R300_RS_SEL_C0) |
			   R300_RS_SEL_T(R300_RS_SEL_C1) |
			   R300_RS_SEL_R(R300_RS_SEL_K0) |
			   R300_RS_SEL_Q(R300_RS_SEL_K1)));
	    OUT_ACCEL_REG(R300_RS_IP_1,
			  (R300_RS_TEX_PTR(2) |
			   R300_RS_SEL_S(R300_RS_SEL_C0) |
			   R300_RS_SEL_T(R300_RS_SEL_C1) |
			   R300_RS_SEL_R(R300_RS_SEL_K0) |
			   R300_RS_SEL_Q(R300_RS_SEL_K1)));
	    /* src tex */
	    /* R300_INST_TEX_ID - select the RS source table entry
	     * R300_INST_TEX_ADDR - the FS temp register for the texture data
	     */
	    OUT_ACCEL_REG(R300_RS_INST_0, (R300_INST_TEX_ID(0) |
					   R300_RS_INST_TEX_CN_WRITE |
					   R300_INST_TEX_ADDR(0)));
	    /* mask tex */
	    OUT_ACCEL_REG(R300_RS_INST_1, (R300_INST_TEX_ID(1) |
					   R300_RS_INST_TEX_CN_WRITE |
					   R300_INST_TEX_ADDR(1)));

	} else {
	    /* rasterizer source table
	     * R300_RS_TEX_PTR is the offset into the input RS stream
	     * 0,1 are tex0
	     * 2,3 are tex1
	     */
	    OUT_ACCEL_REG(R500_RS_IP_0, ((0 << R500_RS_IP_TEX_PTR_S_SHIFT) |
					 (1 << R500_RS_IP_TEX_PTR_T_SHIFT) |
					 (R500_RS_IP_PTR_K0 << R500_RS_IP_TEX_PTR_R_SHIFT) |
					 (R500_RS_IP_PTR_K1 << R500_RS_IP_TEX_PTR_Q_SHIFT)));

	    OUT_ACCEL_REG(R500_RS_IP_1, ((2 << R500_RS_IP_TEX_PTR_S_SHIFT) |
					 (3 << R500_RS_IP_TEX_PTR_T_SHIFT) |
					 (R500_RS_IP_PTR_K0 << R500_RS_IP_TEX_PTR_R_SHIFT) |
					 (R500_RS_IP_PTR_K1 << R500_RS_IP_TEX_PTR_Q_SHIFT)));
	    /* src tex */
	    /* R500_RS_INST_TEX_ID_SHIFT - select the RS source table entry
	     * R500_RS_INST_TEX_ADDR_SHIFT - the FS temp register for the texture data
	     */
	    OUT_ACCEL_REG(R500_RS_INST_0, ((0 << R500_RS_INST_TEX_ID_SHIFT) |
					   R500_RS_INST_TEX_CN_WRITE |
					   (0 << R500_RS_INST_TEX_ADDR_SHIFT)));
	    /* mask tex */
	    OUT_ACCEL_REG(R500_RS_INST_1, ((1 << R500_RS_INST_TEX_ID_SHIFT) |
					   R500_RS_INST_TEX_CN_WRITE |
					   (1 << R500_RS_INST_TEX_ADDR_SHIFT)));
	}
	FINISH_ACCEL();

	/* pre-load FS tex instructions */
	if (IS_R300_3D) {
	    BEGIN_ACCEL(2);
	    /* tex inst for src texture */
	    OUT_ACCEL_REG(R300_US_TEX_INST_0,
			  (R300_TEX_SRC_ADDR(0) |
			   R300_TEX_DST_ADDR(0) |
			   R300_TEX_ID(0) |
			   R300_TEX_INST(R300_TEX_INST_LD)));

	    /* tex inst for mask texture */
	    OUT_ACCEL_REG(R300_US_TEX_INST_1,
			  (R300_TEX_SRC_ADDR(1) |
			   R300_TEX_DST_ADDR(1) |
			   R300_TEX_ID(1) |
			   R300_TEX_INST(R300_TEX_INST_LD)));
	    FINISH_ACCEL();
	}

	if (IS_R300_3D) {
	    BEGIN_ACCEL(9);
	    OUT_ACCEL_REG(R300_US_CONFIG, (0 << R300_NLEVEL_SHIFT) | R300_FIRST_TEX);
	    OUT_ACCEL_REG(R300_US_PIXSIZE, 1); /* highest temp used */
	    OUT_ACCEL_REG(R300_US_CODE_ADDR_0,
			  (R300_ALU_START(0) |
			   R300_ALU_SIZE(0) |
			   R300_TEX_START(0) |
			   R300_TEX_SIZE(0)));
	    OUT_ACCEL_REG(R300_US_CODE_ADDR_1,
			  (R300_ALU_START(0) |
			   R300_ALU_SIZE(0) |
			   R300_TEX_START(0) |
			   R300_TEX_SIZE(0)));
	    OUT_ACCEL_REG(R300_US_CODE_ADDR_2,
			  (R300_ALU_START(0) |
			   R300_ALU_SIZE(0) |
			   R300_TEX_START(0) |
			   R300_TEX_SIZE(0)));
	} else {
	    BEGIN_ACCEL(7);
	    OUT_ACCEL_REG(R300_US_CONFIG, R500_ZERO_TIMES_ANYTHING_EQUALS_ZERO);
	    OUT_ACCEL_REG(R300_US_PIXSIZE, 1); /* highest temp used */
	    OUT_ACCEL_REG(R500_US_FC_CTRL, 0);
	}
	OUT_ACCEL_REG(R300_US_W_FMT, 0);
	OUT_ACCEL_REG(R300_US_OUT_FMT_1, (R300_OUT_FMT_UNUSED |
					  R300_OUT_FMT_C0_SEL_BLUE |
					  R300_OUT_FMT_C1_SEL_GREEN |
					  R300_OUT_FMT_C2_SEL_RED |
					  R300_OUT_FMT_C3_SEL_ALPHA));
	OUT_ACCEL_REG(R300_US_OUT_FMT_2, (R300_OUT_FMT_UNUSED |
					  R300_OUT_FMT_C0_SEL_BLUE |
					  R300_OUT_FMT_C1_SEL_GREEN |
					  R300_OUT_FMT_C2_SEL_RED |
					  R300_OUT_FMT_C3_SEL_ALPHA));
	OUT_ACCEL_REG(R300_US_OUT_FMT_3, (R300_OUT_FMT_UNUSED |
					  R300_OUT_FMT_C0_SEL_BLUE |
					  R300_OUT_FMT_C1_SEL_GREEN |
					  R300_OUT_FMT_C2_SEL_RED |
					  R300_OUT_FMT_C3_SEL_ALPHA));
	FINISH_ACCEL();


	BEGIN_ACCEL(3);
	OUT_ACCEL_REG(R300_FG_DEPTH_SRC, 0);
	OUT_ACCEL_REG(R300_FG_FOG_BLEND, 0);
	OUT_ACCEL_REG(R300_FG_ALPHA_FUNC, 0);
	FINISH_ACCEL();

	BEGIN_ACCEL(13);
	OUT_ACCEL_REG(R300_RB3D_ABLENDCNTL, 0);
	OUT_ACCEL_REG(R300_RB3D_ZSTENCILCNTL, 0);
	OUT_ACCEL_REG(R300_RB3D_ZCACHE_CTLSTAT, R300_ZC_FLUSH | R300_ZC_FREE);
	OUT_ACCEL_REG(R300_RB3D_BW_CNTL, 0);
	OUT_ACCEL_REG(R300_RB3D_ZCNTL, 0);
	OUT_ACCEL_REG(R300_RB3D_ZTOP, 0);
	OUT_ACCEL_REG(R300_RB3D_ROPCNTL, 0);

	OUT_ACCEL_REG(R300_RB3D_AARESOLVE_CTL, 0);
	OUT_ACCEL_REG(R300_RB3D_COLOR_CHANNEL_MASK, (R300_BLUE_MASK_EN |
						     R300_GREEN_MASK_EN |
						     R300_RED_MASK_EN |
						     R300_ALPHA_MASK_EN));
	OUT_ACCEL_REG(R300_RB3D_DSTCACHE_CTLSTAT, R300_DC_FLUSH_3D | R300_DC_FREE_3D);
	OUT_ACCEL_REG(R300_RB3D_CCTL, 0);
	OUT_ACCEL_REG(R300_RB3D_DITHER_CTL, 0);
	OUT_ACCEL_REG(R300_RB3D_DSTCACHE_CTLSTAT, R300_DC_FLUSH_3D | R300_DC_FREE_3D);
	FINISH_ACCEL();

	BEGIN_ACCEL(7);
	OUT_ACCEL_REG(R300_SC_EDGERULE, 0xA5294A5);
	OUT_ACCEL_REG(R300_SC_SCISSOR0, ((0 << R300_SCISSOR_X_SHIFT) |
					 (0 << R300_SCISSOR_Y_SHIFT)));
	OUT_ACCEL_REG(R300_SC_SCISSOR1, ((8191 << R300_SCISSOR_X_SHIFT) |
					 (8191 << R300_SCISSOR_Y_SHIFT)));

	if (IS_R300_3D) {
	    /* clip has offset 1440 */
	    OUT_ACCEL_REG(R300_SC_CLIP_0_A, ((1088 << R300_CLIP_X_SHIFT) |
					     (1088 << R300_CLIP_Y_SHIFT)));
	    OUT_ACCEL_REG(R300_SC_CLIP_0_B, (((1080 + 2920) << R300_CLIP_X_SHIFT) |
					     ((1080 + 2920) << R300_CLIP_Y_SHIFT)));
	} else {
	    OUT_ACCEL_REG(R300_SC_CLIP_0_A, ((0 << R300_CLIP_X_SHIFT) |
					     (0 << R300_CLIP_Y_SHIFT)));
	    OUT_ACCEL_REG(R300_SC_CLIP_0_B, ((4080 << R300_CLIP_X_SHIFT) |
					     (4080 << R300_CLIP_Y_SHIFT)));
	}
	OUT_ACCEL_REG(R300_SC_CLIP_RULE, 0xAAAA);
	OUT_ACCEL_REG(R300_SC_SCREENDOOR, 0xffffff);
	FINISH_ACCEL();
    } else if ((rhdPtr->ChipSet == RHD_FAMILY_RV250) ||
               (rhdPtr->ChipSet == RHD_FAMILY_RV280) ||
               (rhdPtr->ChipSet == RHD_FAMILY_RS300) ||
               (rhdPtr->ChipSet == RHD_FAMILY_R200)) {

	BEGIN_ACCEL(7);
        if (rhdPtr->ChipSet == RHD_FAMILY_RS300) {
	    OUT_ACCEL_REG(R200_SE_VAP_CNTL_STATUS, RADEON_TCL_BYPASS);
	} else {
	    OUT_ACCEL_REG(R200_SE_VAP_CNTL_STATUS, 0);
	}
	OUT_ACCEL_REG(R200_PP_CNTL_X, 0);
	OUT_ACCEL_REG(R200_PP_TXMULTI_CTL_0, 0);
	OUT_ACCEL_REG(R200_SE_VTX_STATE_CNTL, 0);
	OUT_ACCEL_REG(R200_RE_CNTL, 0x0);
	OUT_ACCEL_REG(R200_SE_VTE_CNTL, 0);
	OUT_ACCEL_REG(R200_SE_VAP_CNTL, R200_VAP_FORCE_W_TO_ONE |
	    R200_VAP_VF_MAX_VTX_NUM);
	FINISH_ACCEL();

	BEGIN_ACCEL(5);
	OUT_ACCEL_REG(RADEON_RE_TOP_LEFT, 0);
	OUT_ACCEL_REG(RADEON_RE_WIDTH_HEIGHT, 0x07ff07ff);
	OUT_ACCEL_REG(RADEON_AUX_SC_CNTL, 0);
	OUT_ACCEL_REG(RADEON_RB3D_PLANEMASK, 0xffffffff);
	OUT_ACCEL_REG(RADEON_SE_CNTL, (RADEON_DIFFUSE_SHADE_GOURAUD |
				       RADEON_BFACE_SOLID |
				       RADEON_FFACE_SOLID |
				       RADEON_VTX_PIX_CENTER_OGL |
				       RADEON_ROUND_MODE_ROUND |
				       RADEON_ROUND_PREC_4TH_PIX));
	FINISH_ACCEL();
    } else {
	BEGIN_ACCEL(2);
        if ((rhdPtr->ChipSet == RHD_FAMILY_RADEON) ||
            (rhdPtr->ChipSet == RHD_FAMILY_RV200))
	    OUT_ACCEL_REG(RADEON_SE_CNTL_STATUS, 0);
	else
	    OUT_ACCEL_REG(RADEON_SE_CNTL_STATUS, RADEON_TCL_BYPASS);
	OUT_ACCEL_REG(RADEON_SE_COORD_FMT,
	    RADEON_VTX_XY_PRE_MULT_1_OVER_W0 |
	    RADEON_VTX_ST0_NONPARAMETRIC |
	    RADEON_VTX_ST1_NONPARAMETRIC |
	    RADEON_TEX1_W_ROUTING_USE_W0);
	FINISH_ACCEL();

	BEGIN_ACCEL(5);
	OUT_ACCEL_REG(RADEON_RE_TOP_LEFT, 0);
	OUT_ACCEL_REG(RADEON_RE_WIDTH_HEIGHT, 0x07ff07ff);
	OUT_ACCEL_REG(RADEON_AUX_SC_CNTL, 0);
	OUT_ACCEL_REG(RADEON_RB3D_PLANEMASK, 0xffffffff);
	OUT_ACCEL_REG(RADEON_SE_CNTL, (RADEON_DIFFUSE_SHADE_GOURAUD |
				       RADEON_BFACE_SOLID |
				       RADEON_FFACE_SOLID |
				       RADEON_VTX_PIX_CENTER_OGL |
				       RADEON_ROUND_MODE_ROUND |
				       RADEON_ROUND_PREC_4TH_PIX));
	FINISH_ACCEL();
    }

}

static Bool R300TextureSetup(int w, int h, int unit)
{
    //RINFO_FROM_SCREEN(pPix->drawable.pScreen);
    u32_t txfilter, txformat0, txformat1, txoffset, txpitch;
 //   int w = pPict->pDrawable->width;
 //   int h = pPict->pDrawable->height;
    int i, pixel_shift;

    //ACCEL_PREAMBLE();

    //TRACE;

    //txpitch = exaGetPixmapPitch(pPix);
    txpitch = rhd.displayWidth * 4;

   // txoffset = exaGetPixmapOffset(pPix) + info->fbLocation + pScrn->fbOffset;
    txoffset = rhd.FbIntAddress + rhd.FbScanoutStart;

    if ((txoffset & 0x1f) != 0)
        dbgprintf("Bad texture offset 0x%x\n", (int)txoffset);
    if ((txpitch & 0x1f) != 0)
        dbgprintf("Bad texture pitch 0x%x\n", (int)txpitch);

    /* TXPITCH = pixels (texels) per line - 1 */
    pixel_shift = 32 >> 4;
    txpitch >>= pixel_shift;
    txpitch -= 1;

  //  if (RADEONPixmapIsColortiled(pPix))
  //      txoffset |= R300_MACRO_TILE;

 //   for (i = 0; i < sizeof(R300TexFormats) / sizeof(R300TexFormats[0]); i++)
 //   {
 //       if (R300TexFormats[i].fmt == pPict->format)
 //           break;
 //   }

    //txformat1 = R300TexFormats[i].card_fmt;
    txformat1 = R300_EASY_TX_FORMAT(X, Y, Z, W, W8Z8Y8X8);

    txformat0 = ((((w - 1) & 0x7ff) << R300_TXWIDTH_SHIFT) |
		 (((h - 1) & 0x7ff) << R300_TXHEIGHT_SHIFT));

    if (IS_R500_3D && ((w - 1) & 0x800))
	txpitch |= R500_TXWIDTH_11;

    if (IS_R500_3D && ((h - 1) & 0x800))
	txpitch |= R500_TXHEIGHT_11;

    /* Use TXPITCH instead of TXWIDTH for address computations: we could
     * omit this if there is no padding, but there is no apparent advantage
     * in doing so.
     */
    txformat0 |= R300_TXPITCH_EN;

  //  info->texW[unit] = w;
  //  info->texH[unit] = h;

  //  if (pPict->repeat && !(unit == 0 && need_src_tile_x))
  //    txfilter = R300_TX_CLAMP_S(R300_TX_CLAMP_WRAP);
  //  else
      txfilter = R300_TX_CLAMP_S(R300_TX_CLAMP_CLAMP_GL);

  //  if (pPict->repeat && !(unit == 0 && need_src_tile_y))
  //    txfilter |= R300_TX_CLAMP_T(R300_TX_CLAMP_WRAP);
  //  else
      txfilter |= R300_TX_CLAMP_T(R300_TX_CLAMP_CLAMP_GL);

    txfilter |= (unit << R300_TX_ID_SHIFT);

//    switch (pPict->filter) {
//    case PictFilterNearest:
	txfilter |= (R300_TX_MAG_FILTER_NEAREST | R300_TX_MIN_FILTER_NEAREST);
//        break;
//    case PictFilterBilinear:
//        txfilter |= (R300_TX_MAG_FILTER_LINEAR | R300_TX_MIN_FILTER_LINEAR);
//        break;
//    default:
//        RADEON_FALLBACK(("Bad filter 0x%x\n", pPict->filter));
//    }

    {
      u32_t *ring, write;

      BEGIN_ACCEL(7);
      OUT_ACCEL_REG(R300_TX_FILTER0_0 + (unit * 4), txfilter);
      OUT_ACCEL_REG(R300_TX_FILTER1_0 + (unit * 4), 0);
      OUT_ACCEL_REG(R300_TX_FORMAT0_0 + (unit * 4), txformat0);
      OUT_ACCEL_REG(R300_TX_FORMAT1_0 + (unit * 4), txformat1);
      OUT_ACCEL_REG(R300_TX_FORMAT2_0 + (unit * 4), txpitch);
      OUT_ACCEL_REG(R300_TX_OFFSET_0 + (unit * 4), txoffset);
//    if (!pPict->repeat)
      OUT_ACCEL_REG(R300_TX_BORDER_COLOR_0 + (unit * 4), 0);
      FINISH_ACCEL();
    }
//    if (pPict->transform != 0) {
//        is_transform[unit] = TRUE;
//        transform[unit] = pPict->transform;
//    } else {
//        is_transform[unit] = FALSE;
//    }

    return TRUE;
}

static u32_t RADEONGetBlendCntl(int op, u32_t dst_format)
{
    u32_t sblend, dblend;

    sblend = RadeonBlendOp[op].blend_cntl & RADEON_SRC_BLEND_MASK;
    dblend = RadeonBlendOp[op].blend_cntl & RADEON_DST_BLEND_MASK;

    /* If there's no dst alpha channel, adjust the blend op so that we'll treat
     * it as always 1.
     */
    if (PICT_FORMAT_A(dst_format) == 0 && RadeonBlendOp[op].dst_alpha) {
	if (sblend == RADEON_SRC_BLEND_GL_DST_ALPHA)
	    sblend = RADEON_SRC_BLEND_GL_ONE;
	else if (sblend == RADEON_SRC_BLEND_GL_ONE_MINUS_DST_ALPHA)
	    sblend = RADEON_SRC_BLEND_GL_ZERO;
    }

    return sblend | dblend;
}

static Bool R300PrepareComposite(int op, int srcX, int srcY,
                                int dstX, int dstY,
                                int w, int h)
{
  //  RINFO_FROM_SCREEN(pDst->drawable.pScreen);
    u32_t dst_format, dst_offset, dst_pitch;
    u32_t txenable, colorpitch;
    u32_t blendcntl;
    int pixel_shift;
    u32_t *ring, write;

    //ACCEL_PREAMBLE();

    //TRACE;

    //if (!info->XInited3D)
    //    RADEONInit3DEngine(pScrn);

    //if (!R300GetDestFormat(pDstPicture, &dst_format))
    //    return FALSE;
    dst_format = R300_COLORFORMAT_ARGB8888;

    pixel_shift = 32 >> 4;

    //dst_offset = exaGetPixmapOffset(pDst) + info->fbLocation + pScrn->fbOffset;
    dst_offset = rhd.FbIntAddress + rhd.FbScanoutStart;

    //dst_pitch = exaGetPixmapPitch(pDst);
    dst_pitch = rhd.displayWidth * 4;
    colorpitch = dst_pitch >> pixel_shift;

  //  if (RADEONPixmapIsColortiled(pDst))
  //      colorpitch |= R300_COLORTILE;

    colorpitch |= dst_format;

    if ((dst_offset & 0x0f) != 0)
        dbgprintf("Bad destination offset 0x%x\n", (int)dst_offset);
    if (((dst_pitch >> pixel_shift) & 0x7) != 0)
        dbgprintf("Bad destination pitch 0x%x\n", (int)dst_pitch);

  //  if (!RADEONSetupSourceTile(pSrcPicture, pSrc, TRUE, FALSE))
  //      return FALSE;

    if (!R300TextureSetup(w, h, 0))
	return FALSE;

    txenable = R300_TEX_0_ENABLE;

  //  RADEON_SWITCH_TO_3D();

    /* setup the VAP */
    BEGIN_ACCEL(7);

    /* These registers define the number, type, and location of data submitted
     * to the PVS unit of GA input (when PVS is disabled)
     * DST_VEC_LOC is the slot in the PVS input vector memory when PVS/TCL is
     * enabled.  This memory provides the imputs to the vertex shader program
     * and ordering is not important.  When PVS/TCL is disabled, this field maps
     * directly to the GA input memory and the order is signifigant.  In
     * PVS_BYPASS mode the order is as follows:
     * Position
     * Point Size
     * Color 0-3
     * Textures 0-7
     * Fog
     */

     OUT_ACCEL_REG(R300_VAP_PROG_STREAM_CNTL_0,
                   ((R300_DATA_TYPE_FLOAT_2 << R300_DATA_TYPE_0_SHIFT) |
                    (0 << R300_SKIP_DWORDS_0_SHIFT) |
                    (0 << R300_DST_VEC_LOC_0_SHIFT) |
                     R300_SIGNED_0 |
                    (R300_DATA_TYPE_FLOAT_2 << R300_DATA_TYPE_1_SHIFT) |
                    (0 << R300_SKIP_DWORDS_1_SHIFT) |
                    (6 << R300_DST_VEC_LOC_1_SHIFT) |
                     R300_LAST_VEC_1 |
                     R300_SIGNED_1));

    /* load the vertex shader
     * We pre-load vertex programs in RADEONInit3DEngine():
     * - exa no mask
     * - exa mask
     * - Xv
     * Here we select the offset of the vertex program we want to use
     */
	    OUT_ACCEL_REG(R300_VAP_PVS_CODE_CNTL_0,
			  ((3 << R300_PVS_FIRST_INST_SHIFT) |
			   (4 << R300_PVS_XYZW_VALID_INST_SHIFT) |
			   (4 << R300_PVS_LAST_INST_SHIFT)));
	    OUT_ACCEL_REG(R300_VAP_PVS_CODE_CNTL_1,
			  (4 << R300_PVS_LAST_VTX_SRC_INST_SHIFT));

    /* Position and one or two sets of 2 texture coordinates */
    OUT_ACCEL_REG(R300_VAP_OUT_VTX_FMT_0, R300_VTX_POS_PRESENT);
    OUT_ACCEL_REG(R300_VAP_OUT_VTX_FMT_1, (2 << R300_TEX_0_COMP_CNT_SHIFT));

    OUT_ACCEL_REG(R300_TX_INVALTAGS, 0x0);
    OUT_ACCEL_REG(R300_TX_ENABLE, txenable);
    FINISH_ACCEL();

    /* setup pixel shader */
    if (IS_R300_3D) {
    } else {
        u32_t output_fmt;
        u32_t src_color, src_alpha;
        u32_t mask_color, mask_alpha;

        if (PICT_FORMAT_RGB(PICT_a8r8g8b8) == 0)
	    src_color = (R500_ALU_RGB_R_SWIZ_A_0 |
			 R500_ALU_RGB_G_SWIZ_A_0 |
			 R500_ALU_RGB_B_SWIZ_A_0);
	else
	    src_color = (R500_ALU_RGB_R_SWIZ_A_R |
			 R500_ALU_RGB_G_SWIZ_A_G |
			 R500_ALU_RGB_B_SWIZ_A_B);

        if (PICT_FORMAT_A(PICT_a8r8g8b8) == 0)
	    src_alpha = R500_ALPHA_SWIZ_A_1;
	else
	    src_alpha = R500_ALPHA_SWIZ_A_A;

        mask_color = (R500_ALU_RGB_R_SWIZ_B_1 |
                      R500_ALU_RGB_G_SWIZ_B_1 |
                      R500_ALU_RGB_B_SWIZ_B_1);
        mask_alpha = R500_ALPHA_SWIZ_B_1;

	/* shader output swizzling */
        output_fmt = (R300_OUT_FMT_C4_8         |
                      R300_OUT_FMT_C0_SEL_BLUE  |
                      R300_OUT_FMT_C1_SEL_GREEN |
                      R300_OUT_FMT_C2_SEL_RED   |
                      R300_OUT_FMT_C3_SEL_ALPHA);

	BEGIN_ACCEL(6);
        OUT_ACCEL_REG(R300_RS_COUNT,
                     ((2 << R300_RS_COUNT_IT_COUNT_SHIFT) |
                       R300_RS_COUNT_HIRES_EN));

        OUT_ACCEL_REG(R300_RS_INST_COUNT, R300_INST_COUNT_RS(0) | R300_TX_OFFSET_RS(6));

        OUT_ACCEL_REG(R500_US_CODE_ADDR, (R500_US_CODE_START_ADDR(0) |
                                          R500_US_CODE_END_ADDR(1)));
        OUT_ACCEL_REG(R500_US_CODE_RANGE, (R500_US_CODE_RANGE_ADDR(0) |
                                           R500_US_CODE_RANGE_SIZE(1)));
        OUT_ACCEL_REG(R500_US_CODE_OFFSET, 0);

	OUT_ACCEL_REG(R300_US_OUT_FMT_0, output_fmt);
	FINISH_ACCEL();

        BEGIN_ACCEL(13);
        OUT_ACCEL_REG(R500_GA_US_VECTOR_INDEX, 0);
	    /* tex inst for src texture */
        OUT_ACCEL_REG(R500_GA_US_VECTOR_DATA, (R500_INST_TYPE_TEX     |
                                               R500_INST_TEX_SEM_WAIT |
                                               R500_INST_RGB_WMASK_R  |
                                               R500_INST_RGB_WMASK_G  |
                                               R500_INST_RGB_WMASK_B  |
                                               R500_INST_ALPHA_WMASK  |
                                               R500_INST_RGB_CLAMP |
                                               R500_INST_ALPHA_CLAMP));

       OUT_ACCEL_REG(R500_GA_US_VECTOR_DATA, (R500_TEX_ID(0) |
						   R500_TEX_INST_LD |
						   R500_TEX_SEM_ACQUIRE |
						   R500_TEX_IGNORE_UNCOVERED));

	    OUT_ACCEL_REG(R500_GA_US_VECTOR_DATA, (R500_TEX_SRC_ADDR(0) |
						   R500_TEX_SRC_S_SWIZ_R |
						   R500_TEX_SRC_T_SWIZ_G |
						   R500_TEX_DST_ADDR(0) |
						   R500_TEX_DST_R_SWIZ_R |
						   R500_TEX_DST_G_SWIZ_G |
						   R500_TEX_DST_B_SWIZ_B |
						   R500_TEX_DST_A_SWIZ_A));
	    OUT_ACCEL_REG(R500_GA_US_VECTOR_DATA, (R500_DX_ADDR(0) |
						   R500_DX_S_SWIZ_R |
						   R500_DX_T_SWIZ_R |
						   R500_DX_R_SWIZ_R |
						   R500_DX_Q_SWIZ_R |
						   R500_DY_ADDR(0) |
						   R500_DY_S_SWIZ_R |
						   R500_DY_T_SWIZ_R |
						   R500_DY_R_SWIZ_R |
						   R500_DY_Q_SWIZ_R));
	    OUT_ACCEL_REG(R500_GA_US_VECTOR_DATA, 0x00000000);
	    OUT_ACCEL_REG(R500_GA_US_VECTOR_DATA, 0x00000000);

	/* ALU inst */
	/* *_OMASK* - output component write mask */
	OUT_ACCEL_REG(R500_GA_US_VECTOR_DATA, (R500_INST_TYPE_OUT |
					       R500_INST_TEX_SEM_WAIT |
					       R500_INST_LAST |
					       R500_INST_RGB_OMASK_R |
					       R500_INST_RGB_OMASK_G |
					       R500_INST_RGB_OMASK_B |
					       R500_INST_ALPHA_OMASK |
					       R500_INST_RGB_CLAMP |
					       R500_INST_ALPHA_CLAMP));
	/* ALU inst
	 * temp addresses for texture inputs
	 * RGB_ADDR0 is src tex (temp 0)
	 * RGB_ADDR1 is mask tex (temp 1)
	 */
	OUT_ACCEL_REG(R500_GA_US_VECTOR_DATA, (R500_RGB_ADDR0(0) |
					       R500_RGB_ADDR1(1) |
					       R500_RGB_ADDR2(0)));
	/* ALU inst
	 * temp addresses for texture inputs
	 * ALPHA_ADDR0 is src tex (temp 0)
	 * ALPHA_ADDR1 is mask tex (temp 1)
	 */
	OUT_ACCEL_REG(R500_GA_US_VECTOR_DATA, (R500_ALPHA_ADDR0(0) |
					       R500_ALPHA_ADDR1(1) |
					       R500_ALPHA_ADDR2(0)));

	/* R500_ALU_RGB_TARGET - RGB render target */
	OUT_ACCEL_REG(R500_GA_US_VECTOR_DATA, (R500_ALU_RGB_SEL_A_SRC0 |
					       src_color |
					       R500_ALU_RGB_SEL_B_SRC1 |
					       mask_color |
					       R500_ALU_RGB_TARGET(0)));

	/* R500_ALPHA_RGB_TARGET - alpha render target */
	OUT_ACCEL_REG(R500_GA_US_VECTOR_DATA, (R500_ALPHA_OP_MAD |
					       R500_ALPHA_ADDRD(0) |
					       R500_ALPHA_SEL_A_SRC0 |
					       src_alpha |
					       R500_ALPHA_SEL_B_SRC1 |
					       mask_alpha |
					       R500_ALPHA_TARGET(0)));

	OUT_ACCEL_REG(R500_GA_US_VECTOR_DATA, (R500_ALU_RGBA_OP_MAD |
					       R500_ALU_RGBA_ADDRD(0) |
					       R500_ALU_RGBA_R_SWIZ_0 |
					       R500_ALU_RGBA_G_SWIZ_0 |
					       R500_ALU_RGBA_B_SWIZ_0 |
					       R500_ALU_RGBA_A_SWIZ_0));
	FINISH_ACCEL();
    }

    BEGIN_ACCEL(3);

    OUT_ACCEL_REG(R300_RB3D_COLOROFFSET0, dst_offset);
    OUT_ACCEL_REG(R300_RB3D_COLORPITCH0, colorpitch);

    blendcntl = RADEONGetBlendCntl(op, PICT_a8r8g8b8);
    OUT_ACCEL_REG(R300_RB3D_BLENDCNTL, blendcntl | R300_ALPHA_BLEND_ENABLE | R300_READ_ENABLE);

    FINISH_ACCEL();

    return TRUE;
}






#define VTX_COUNT 4

static __inline__ u32_t F_TO_DW(float val)
{
    union {
	float f;
        u32_t l;
    } tmp;
    tmp.f = val;
    return tmp.l;
}

#define OUT_RING_F(x) OUT_RING(F_TO_DW(x))

#define VTX_OUT(_dstX, _dstY, _srcX, _srcY)     \
do {								\
    OUT_RING_F(_dstX);						\
    OUT_RING_F(_dstY);						\
    OUT_RING_F(_srcX);						\
    OUT_RING_F(_srcY);						\
} while (0)



static void RadeonCompositeTile(int srcX, int srcY,
                                int dstX, int dstY,
                                int w, int h)
{
  //  RINFO_FROM_SCREEN(pDst->drawable.pScreen);
    int vtx_count;
    xPointFixed srcTopLeft, srcTopRight, srcBottomLeft, srcBottomRight;
    xPointFixed maskTopLeft, maskTopRight, maskBottomLeft, maskBottomRight;

    u32_t *ring, write;


 //   ACCEL_PREAMBLE();

 //   ENTER_DRAW(0);

    /* ErrorF("RadeonComposite (%d,%d) (%d,%d) (%d,%d) (%d,%d)\n",
       srcX, srcY, maskX, maskY,dstX, dstY, w, h); */

    srcTopLeft.x     = IntToxFixed(srcX);
    srcTopLeft.y     = IntToxFixed(srcY);
    srcTopRight.x    = IntToxFixed(srcX + w);
    srcTopRight.y    = IntToxFixed(srcY);
    srcBottomLeft.x  = IntToxFixed(srcX);
    srcBottomLeft.y  = IntToxFixed(srcY + h);
    srcBottomRight.x = IntToxFixed(srcX + w);
    srcBottomRight.y = IntToxFixed(srcY + h);

/*
    if (is_transform[0]) {
	transformPoint(transform[0], &srcTopLeft);
	transformPoint(transform[0], &srcTopRight);
	transformPoint(transform[0], &srcBottomLeft);
	transformPoint(transform[0], &srcBottomRight);
    }
    if (is_transform[1]) {
	transformPoint(transform[1], &maskTopLeft);
	transformPoint(transform[1], &maskTopRight);
	transformPoint(transform[1], &maskBottomLeft);
	transformPoint(transform[1], &maskBottomRight);
    }
*/
    vtx_count = VTX_COUNT;

    BEGIN_ACCEL(1);
    OUT_ACCEL_REG(R300_VAP_VTX_SIZE, vtx_count);
    FINISH_ACCEL();

    BEGIN_RING(4 * vtx_count + 4);

      OUT_RING(CP_PACKET3(R200_CP_PACKET3_3D_DRAW_IMMD_2,
                        4 * vtx_count));
      OUT_RING(RADEON_CP_VC_CNTL_PRIM_TYPE_TRI_FAN |
              RADEON_CP_VC_CNTL_PRIM_WALK_RING |
              (4 << RADEON_CP_VC_CNTL_NUM_SHIFT));


      VTX_OUT((float)dstX, (float)dstY,
              xFixedToFloat(srcTopLeft.x) / w,      // info->texW[0],
              xFixedToFloat(srcTopLeft.y) / h);     // info->texH[0]);

      VTX_OUT((float)dstX, (float)(dstY + h),
              xFixedToFloat(srcBottomLeft.x) / w,   // info->texW[0],
              xFixedToFloat(srcBottomLeft.y) / h);  // info->texH[0]);

      VTX_OUT((float)(dstX + w), (float)(dstY + h),
              xFixedToFloat(srcBottomRight.x) / w,  // info->texW[0],
              xFixedToFloat(srcBottomRight.y) / h); // info->texH[0]);

      VTX_OUT((float)(dstX + w), (float)dstY,
              xFixedToFloat(srcTopRight.x) / w,     // info->texW[0],
              xFixedToFloat(srcTopRight.y) / h);    // info->texH[0]);

          /* flushing is pipelined, free/finish is not */
      OUT_ACCEL_REG(R300_RB3D_DSTCACHE_CTLSTAT, R300_DC_FLUSH_3D);

 //     OUT_ACCEL_REG(RADEON_WAIT_UNTIL, RADEON_WAIT_3D_IDLECLEAN);

    COMMIT_RING();

 //   LEAVE_DRAW(0);
}
#undef VTX_OUT
#undef VTX_OUT_MASK

#if 0

int RadeonComposite( blit_t *blit)
{
    int tileSrcY, tileMaskY, tileDstY;
    int remainingHeight;

    R300PrepareComposite(3, blit->src_x, blit->src_y,
                             blit->dst_x, blit->dst_y,
                             blit->w, blit->h);

//    if (!need_src_tile_x && !need_src_tile_y) {
        RadeonCompositeTile( blit->src_x, blit->src_y,
                             blit->dst_x, blit->dst_y,
                             blit->w, blit->h);
        return 0;
  //  }

    /* Tiling logic borrowed from exaFillRegionTiled */

#if 0
    modulus(srcY, src_tile_height, tileSrcY);
    tileMaskY = maskY;
    tileDstY = dstY;

    remainingHeight = height;
    while (remainingHeight > 0) {
	int remainingWidth = width;
	int tileSrcX, tileMaskX, tileDstX;
	int h = src_tile_height - tileSrcY;

	if (h > remainingHeight)
	    h = remainingHeight;
	remainingHeight -= h;

	modulus(srcX, src_tile_width, tileSrcX);
	tileMaskX = maskX;
	tileDstX = dstX;

	while (remainingWidth > 0) {
	    int w = src_tile_width - tileSrcX;
	    if (w > remainingWidth)
		w = remainingWidth;
	    remainingWidth -= w;

	    FUNC_NAME(RadeonCompositeTile)(pDst,
					   tileSrcX, tileSrcY,
					   tileMaskX, tileMaskY,
					   tileDstX, tileDstY,
					   w, h);

	    tileSrcX = 0;
	    tileMaskX += w;
	    tileDstX += w;
	}
	tileSrcY = 0;
	tileMaskY += h;
	tileDstY += h;
    }
#endif
}

#endif

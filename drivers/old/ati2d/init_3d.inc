/*
 * Copyright 2000 ATI Technologies Inc., Markham, Ontario, and
 *                VA Linux Systems Inc., Fremont, California.
 *
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation on the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL ATI, VA LINUX SYSTEMS AND/OR
 * THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */


int Init3DEngine(RHDPtr info)
{
    u32_t gb_tile_config, su_reg_dest, vap_cntl;

    u32_t *ring;
    u32_t ifl;

   // info->texW[0] = info->texH[0] = info->texW[1] = info->texH[1] = 1;

    ifl = safe_cli();

    FIFOWait(64);
    delay(2);

    if (IS_R300_3D || IS_R500_3D) {

	BEGIN_ACCEL(3);
        OUT_ACCEL_REG(R300_RB3D_DSTCACHE_CTLSTAT, R300_DC_FLUSH_3D | R300_DC_FREE_3D);
        OUT_ACCEL_REG(R300_RB3D_ZCACHE_CTLSTAT, R300_ZC_FLUSH | R300_ZC_FREE);
        OUT_ACCEL_REG(RADEON_WAIT_UNTIL, RADEON_WAIT_2D_IDLECLEAN | RADEON_WAIT_3D_IDLECLEAN);
	FINISH_ACCEL();

        gb_tile_config = (R300_ENABLE_TILING | R300_TILE_SIZE_16 | R300_SUBPIXEL_1_16);

        switch(info->num_gb_pipes)
        {
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
        su_reg_dest = ((1 << info->num_gb_pipes) - 1);
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
        if (info->has_tcl)
            vap_cntl = ((5 << R300_PVS_NUM_SLOTS_SHIFT) |
                        (5 << R300_PVS_NUM_CNTLRS_SHIFT) |
                        (9 << R300_VF_MAX_VTX_NUM_SHIFT));
        else
            vap_cntl = ((10 << R300_PVS_NUM_SLOTS_SHIFT) |
                        (5 << R300_PVS_NUM_CNTLRS_SHIFT) |
                        (5 << R300_VF_MAX_VTX_NUM_SHIFT));

        if (info->ChipFamily == CHIP_FAMILY_RV515)
            vap_cntl |= (2 << R300_PVS_NUM_FPUS_SHIFT);
        else if ((info->ChipFamily == CHIP_FAMILY_RV530) ||
		 (info->ChipFamily == CHIP_FAMILY_RV560) ||
		 (info->ChipFamily == CHIP_FAMILY_RV570))
            vap_cntl |= (5 << R300_PVS_NUM_FPUS_SHIFT);
	else if ((info->ChipFamily == CHIP_FAMILY_RV410) ||
		 (info->ChipFamily == CHIP_FAMILY_R420))
            vap_cntl |= (6 << R300_PVS_NUM_FPUS_SHIFT);
        else if ((info->ChipFamily == CHIP_FAMILY_R520) ||
		 (info->ChipFamily == CHIP_FAMILY_R580))
            vap_cntl |= (8 << R300_PVS_NUM_FPUS_SHIFT);
        else
            vap_cntl |= (4 << R300_PVS_NUM_FPUS_SHIFT);

    if (info->has_tcl)
        BEGIN_ACCEL(15);
	else
	    BEGIN_ACCEL(9);
        OUT_ACCEL_REG(R300_VAP_VTX_STATE_CNTL, 0);
        OUT_ACCEL_REG(R300_VAP_PVS_STATE_FLUSH_REG, 0);

        if (info->has_tcl)
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

    if (info->has_tcl) {
            OUT_ACCEL_REG(R300_VAP_PVS_FLOW_CNTL_OPC, 0);
            OUT_ACCEL_REG(R300_VAP_GB_VERT_CLIP_ADJ, 0x3f800000);
            OUT_ACCEL_REG(R300_VAP_GB_VERT_DISC_ADJ, 0x3f800000);
            OUT_ACCEL_REG(R300_VAP_GB_HORZ_CLIP_ADJ, 0x3f800000);
            OUT_ACCEL_REG(R300_VAP_GB_HORZ_DISC_ADJ, 0x3f800000);
            OUT_ACCEL_REG(R300_VAP_CLIP_CNTL, R300_CLIP_DISABLE);
        }
	FINISH_ACCEL();

	/* pre-load the vertex shaders */
    if (info->has_tcl) {
        /* exa mask/Xv bicubic shader program

           dcl_position   v0
           dcl_texcoord   v1
           dcl_texcoord1  v2

           mov oPos,  v0
           mov oT0,   v1
           mov oT1,   v2      */


	    BEGIN_ACCEL(13);
            OUT_ACCEL_REG(R300_VAP_PVS_VECTOR_INDX_REG, 0);
	    /* PVS inst 0 */
            OUT_ACCEL_REG (R300_VAP_PVS_VECTOR_DATA_REG,
                          (R300_PVS_DST_OPCODE(R300_VE_ADD) |
                           R300_PVS_DST_REG_TYPE(R300_PVS_DST_REG_OUT) |
                           R300_PVS_DST_OFFSET(0) |
                           R300_PVS_DST_WE_X | R300_PVS_DST_WE_Y |
                           R300_PVS_DST_WE_Z | R300_PVS_DST_WE_W));
            OUT_ACCEL_REG (R300_VAP_PVS_VECTOR_DATA_REG,
                      (R300_PVS_SRC_REG_TYPE(R300_PVS_SRC_REG_INPUT) |
                       R300_PVS_SRC_OFFSET(0) |
                       R300_PVS_SRC_SWIZZLE_X(R300_PVS_SRC_SELECT_X) |
                       R300_PVS_SRC_SWIZZLE_Y(R300_PVS_SRC_SELECT_Y) |
                       R300_PVS_SRC_SWIZZLE_Z(R300_PVS_SRC_SELECT_Z) |
                       R300_PVS_SRC_SWIZZLE_W(R300_PVS_SRC_SELECT_W)));
            OUT_ACCEL_REG (R300_VAP_PVS_VECTOR_DATA_REG,
                      (R300_PVS_SRC_REG_TYPE(R300_PVS_SRC_REG_INPUT) |
                       R300_PVS_SRC_OFFSET(0) |
                       R300_PVS_SRC_SWIZZLE_X(R300_PVS_SRC_SELECT_FORCE_0) |
                       R300_PVS_SRC_SWIZZLE_Y(R300_PVS_SRC_SELECT_FORCE_0) |
                       R300_PVS_SRC_SWIZZLE_Z(R300_PVS_SRC_SELECT_FORCE_0) |
                       R300_PVS_SRC_SWIZZLE_W(R300_PVS_SRC_SELECT_FORCE_0)));
            OUT_ACCEL_REG (R300_VAP_PVS_VECTOR_DATA_REG,
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

        /* exa no mask instruction

             dcl_position   v0
             dcl_texcoord   v1

             mov oPos,  v0
             mov oT0,   v1      */


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

	if (IS_R300_3D)
	    BEGIN_ACCEL(4);
	else {
	    BEGIN_ACCEL(6);
            OUT_ACCEL_REG(R300_US_CONFIG, R500_ZERO_TIMES_ANYTHING_EQUALS_ZERO);
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

	BEGIN_ACCEL(5);
        OUT_ACCEL_REG(R300_SC_EDGERULE, 0xA5294A5);
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
    } else if ((info->ChipFamily == CHIP_FAMILY_RV250) ||
             (info->ChipFamily == CHIP_FAMILY_RV280) ||
             (info->ChipFamily == CHIP_FAMILY_RS300) ||
	       (info->ChipFamily == CHIP_FAMILY_R200)) {

	BEGIN_ACCEL(6);
        if (info->ChipFamily == CHIP_FAMILY_RS300) {
            OUT_ACCEL_REG(R200_SE_VAP_CNTL_STATUS, RADEON_TCL_BYPASS);
        } else {
            OUT_ACCEL_REG(R200_SE_VAP_CNTL_STATUS, 0);
        }
        OUT_ACCEL_REG(R200_PP_CNTL_X, 0);
        OUT_ACCEL_REG(R200_PP_TXMULTI_CTL_0, 0);
        OUT_ACCEL_REG(R200_SE_VTX_STATE_CNTL, 0);
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
        if ((info->ChipFamily == CHIP_FAMILY_RADEON) ||
            (info->ChipFamily == CHIP_FAMILY_RV200))
            OUT_ACCEL_REG(RADEON_SE_CNTL_STATUS, 0);
        else
            OUT_ACCEL_REG(RADEON_SE_CNTL_STATUS, RADEON_TCL_BYPASS);
	OUT_ACCEL_REG(RADEON_SE_COORD_FMT,
	    RADEON_VTX_XY_PRE_MULT_1_OVER_W0 |
                                       RADEON_VTX_ST0_NONPARAMETRIC     |
                                       RADEON_VTX_ST1_NONPARAMETRIC     |
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
    safe_sti(ifl);
    FIFOWait(64);
    delay(2);

}


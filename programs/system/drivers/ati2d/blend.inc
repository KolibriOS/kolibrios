
struct blendinfo {
    Bool dst_alpha;
    Bool src_alpha;
    u32_t blend_cntl;
};

static struct blendinfo RadeonBlendOp[] = {
    /* 0 - Clear */
    {0, 0, RADEON_SRC_BLEND_GL_ZERO                | RADEON_DST_BLEND_GL_ZERO},
    /* 1 - Src */
    {0, 0, RADEON_SRC_BLEND_GL_ONE                 | RADEON_DST_BLEND_GL_ZERO},
    /* 2 - Dst */
    {0, 0, RADEON_SRC_BLEND_GL_ZERO                | RADEON_DST_BLEND_GL_ONE},
    /* 3 - Over */
    {0, 1, RADEON_SRC_BLEND_GL_ONE                 | RADEON_DST_BLEND_GL_ONE_MINUS_SRC_ALPHA},
    /* 4 - OverReverse */
    {1, 0, RADEON_SRC_BLEND_GL_ONE_MINUS_DST_ALPHA | RADEON_DST_BLEND_GL_ONE},
    /* 5 - In */
    {1, 0, RADEON_SRC_BLEND_GL_DST_ALPHA           | RADEON_DST_BLEND_GL_ZERO},
    /* 6 - InReverse */
    {0, 1, RADEON_SRC_BLEND_GL_ZERO                | RADEON_DST_BLEND_GL_SRC_ALPHA},
    /* 7 - Out */
    {1, 0, RADEON_SRC_BLEND_GL_ONE_MINUS_DST_ALPHA | RADEON_DST_BLEND_GL_ZERO},
    /* 8 - OutReverse */
    {0, 1, RADEON_SRC_BLEND_GL_ZERO                | RADEON_DST_BLEND_GL_ONE_MINUS_SRC_ALPHA},
    /* 9 - Atop */
    {1, 1, RADEON_SRC_BLEND_GL_DST_ALPHA           | RADEON_DST_BLEND_GL_ONE_MINUS_SRC_ALPHA},
    /* 10- AtopReverse */
    {1, 1, RADEON_SRC_BLEND_GL_ONE_MINUS_DST_ALPHA | RADEON_DST_BLEND_GL_SRC_ALPHA},
    /* 11 - Xor */
    {1, 1, RADEON_SRC_BLEND_GL_ONE_MINUS_DST_ALPHA | RADEON_DST_BLEND_GL_ONE_MINUS_SRC_ALPHA},
    /* 12 - Add */
    {0, 0, RADEON_SRC_BLEND_GL_ONE                 | RADEON_DST_BLEND_GL_ONE},
};

static Bool R200PrepareComposite(local_pixmap_t *dstpix, int dstX, int dstY,
                                 local_pixmap_t *srcpix, int srcX, int srcY,
                                 int w, int h, int alpha)
{
    u32_t tex_size = 0, txformat, blend_cntl;
    int dst_pitch;

    if ((w > 2048) || (h > 2048))
        return FALSE;

    txformat = RADEON_TXFORMAT_ARGB8888      |
               RADEON_TXFORMAT_ALPHA_IN_MAP  |
               RADEON_TXFORMAT_NON_POWER2;

    blend_cntl = RADEON_SRC_BLEND_GL_SRC_ALPHA |
                 RADEON_DST_BLEND_GL_ONE_MINUS_SRC_ALPHA;

    tex_size = ((h - 1) << 16) | (w - 1);
    {
        u32_t *ring;
        u32_t ifl  = safe_cli();

        BEGIN_ACCEL(168);
        OUT_ACCEL_REG(R200_PP_TXFORMAT_0, txformat);
        OUT_ACCEL_REG(R200_PP_TXFORMAT_X_0, 0);
        OUT_ACCEL_REG(R200_PP_TXSIZE_0, tex_size);
        OUT_ACCEL_REG(R200_PP_TXPITCH_0, srcpix->pitch - 32);
        OUT_ACCEL_REG(R200_PP_TXOFFSET_0, (u32_t)srcpix->local);
        OUT_ACCEL_REG(R200_PP_TXFILTER_0, R200_MAG_FILTER_NEAREST |
				      R200_MIN_FILTER_NEAREST |
				      R200_CLAMP_S_WRAP |
				      R200_CLAMP_T_WRAP);

        OUT_ACCEL_REG(RADEON_RB3D_CNTL, RADEON_COLOR_FORMAT_ARGB8888 |
                                        RADEON_ALPHA_BLEND_ENABLE);
        OUT_ACCEL_REG(RADEON_PP_CNTL, RADEON_TEX_0_ENABLE |
                                      RADEON_TEX_BLEND_0_ENABLE);
        OUT_ACCEL_REG(R200_PP_TFACTOR_0, alpha << 24);
        OUT_ACCEL_REG(R200_PP_TXCBLEND_0, R200_TXC_ARG_A_TFACTOR_COLOR |
                                          R200_TXC_ARG_B_R0_ALPHA);
        OUT_ACCEL_REG(R200_PP_TXCBLEND2_0,R200_TXC_OUTPUT_REG_R0);
        OUT_ACCEL_REG(R200_PP_TXABLEND_0, R200_TXA_ARG_A_TFACTOR_ALPHA |
                                          R200_TXA_ARG_B_R0_ALPHA);
        OUT_ACCEL_REG(R200_PP_TXABLEND2_0, R200_TXA_OUTPUT_REG_R0);
        OUT_ACCEL_REG(R200_SE_VTX_FMT_0, 0);
        OUT_ACCEL_REG(R200_SE_VTX_FMT_1, (2 << R200_VTX_TEX0_COMP_CNT_SHIFT));
        OUT_ACCEL_REG(RADEON_RB3D_BLENDCNTL, blend_cntl);

        OUT_ACCEL_REG(RADEON_RB3D_COLORPITCH, dstpix->pitch >> 2);
        OUT_ACCEL_REG(RADEON_RB3D_COLOROFFSET, (u32_t)dstpix->local);

        COMMIT_RING();
        safe_sti(ifl);
    };

    return TRUE;
}

static Bool R300TextureSetup(RHDPtr info,local_pixmap_t *srcpix, int w, int h, int unit)
{
    u32_t txfilter, txformat0, txformat1, txoffset, txpitch;
 //   int w = pPict->pDrawable->width;
 //   int h = pPict->pDrawable->height;
    int i, pixel_shift;


    txpitch = srcpix->pitch;
    txoffset = (u32_t)srcpix->local;

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

    txformat1 = R300_TX_FORMAT_A8R8G8B8;

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
        u32_t *ring;
        u32_t ifl  = safe_cli();

        BEGIN_ACCEL(7);

        OUT_ACCEL_REG(R300_TX_FILTER0_0 + (unit * 4), txfilter);
        OUT_ACCEL_REG(R300_TX_FILTER1_0 + (unit * 4), 0);
        OUT_ACCEL_REG(R300_TX_FORMAT0_0 + (unit * 4), txformat0);
        OUT_ACCEL_REG(R300_TX_FORMAT1_0 + (unit * 4), txformat1);
        OUT_ACCEL_REG(R300_TX_FORMAT2_0 + (unit * 4), txpitch);
        OUT_ACCEL_REG(R300_TX_OFFSET_0 + (unit * 4), txoffset);
//    if (!pPict->repeat)
        OUT_ACCEL_REG(R300_TX_BORDER_COLOR_0 + (unit * 4), 0);

        COMMIT_RING();
        safe_sti(ifl);
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

   // sblend = RadeonBlendOp[op].blend_cntl & RADEON_SRC_BLEND_MASK;
   // dblend = RadeonBlendOp[op].blend_cntl & RADEON_DST_BLEND_MASK;

    /* If there's no dst alpha channel, adjust the blend op so that we'll treat
     * it as always 1.
     */
  //  if ( RadeonBlendOp[op].dst_alpha) {
  //      if (sblend == RADEON_SRC_BLEND_GL_DST_ALPHA)
  //          sblend = RADEON_SRC_BLEND_GL_ONE;
  //      else if (sblend == RADEON_SRC_BLEND_GL_ONE_MINUS_DST_ALPHA)
  //          sblend = RADEON_SRC_BLEND_GL_ZERO;
  //  }

    //return sblend | dblend;

    return RADEON_SRC_BLEND_GL_SRC_ALPHA | RADEON_DST_BLEND_GL_ONE_MINUS_SRC_ALPHA;
}


Bool R300PrepareComposite(local_pixmap_t *dstpix, int dstX, int dstY,
                          local_pixmap_t *srcpix, int srcX, int srcY,
                          int w, int h, int op)
{
  //  RINFO_FROM_SCREEN(pDst->drawable.pScreen);
    u32_t dst_format, dst_offset, dst_pitch;
    u32_t txenable, colorpitch;
    u32_t blendcntl;
    int pixel_shift;
    u32_t *ring;

    u32_t ifl;

    RHDPtr info = &rhd;

    dst_format = R300_COLORFORMAT_ARGB8888;

    dst_offset = (u32_t)dstpix->local;

    dst_pitch = dstpix->pitch;

    pixel_shift = 32 >> 4;

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

    if (!R300TextureSetup(&rhd, srcpix, w, h, 0))
	return FALSE;

    txenable = R300_TEX_0_ENABLE;

  //  RADEON_SWITCH_TO_3D();

    /* setup the VAP */

    ifl = safe_cli();

    BEGIN_RING(32*2);

    OUT_ACCEL_REG(RADEON_WAIT_UNTIL, RADEON_WAIT_HOST_IDLECLEAN |
                                     RADEON_WAIT_3D_IDLECLEAN);

    /* These registers define the number, type, and location of data submitted
     * to the PVS unit of GA input (when PVS is disabled)
     * DST_VEC_LOC is the slot in the PVS input vector memory when PVS/TCL is
     * enabled.  This memory provides the imputs to the vertex shader program
     * and ordering is not important.  When PVS/TCL is disabled, this field maps
     * directly to the GA input memory and the order is signifigant.  In
     * PVS_BYPASS mode the order is as follows:
     * 0 Position
     * 1 Point Size
     * 2 Color 0
     * 3 Color 1
     * 4 Color 2
     * 5 Color 3
     * 6 Textures 0
     * 7 Textures 1
     * 8 Textures 2
     * 9 Textures 3 - 7
     * 14 Fog
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
    if (info->has_tcl) {
	    OUT_ACCEL_REG(R300_VAP_PVS_CODE_CNTL_0,
                     ((3 << R300_PVS_FIRST_INST_SHIFT) |
                      (4 << R300_PVS_XYZW_VALID_INST_SHIFT) |
                      (4 << R300_PVS_LAST_INST_SHIFT)));
	    OUT_ACCEL_REG(R300_VAP_PVS_CODE_CNTL_1,
                      (4 << R300_PVS_LAST_VTX_SRC_INST_SHIFT));
    }

    /* Position and one or two sets of 2 texture coordinates */
    OUT_ACCEL_REG(R300_VAP_OUT_VTX_FMT_0, R300_VTX_POS_PRESENT);   //VTX_COLOR_0_PRESENT
    OUT_ACCEL_REG(R300_VAP_OUT_VTX_FMT_1, (2 << R300_TEX_0_COMP_CNT_SHIFT));

    OUT_ACCEL_REG(R300_TX_INVALTAGS, 0x0);
    OUT_ACCEL_REG(R300_TX_ENABLE, txenable);
    FINISH_ACCEL();

    /* setup pixel shader */


    /* setup pixel shader */
    if (IS_R300_3D)
    {
        u32_t output_fmt;
        int src_color, src_alpha;
        int mask_color, mask_alpha;

	    src_color = R300_ALU_RGB_SRC0_RGB;

	    src_alpha = R300_ALU_ALPHA_SRC0_A;

	    mask_color = R300_ALU_RGB_1_0;
	    mask_alpha = R300_ALU_ALPHA_1_0;

	/* shader output swizzling */
        output_fmt = (R300_OUT_FMT_C4_8 |
                      R300_OUT_FMT_C0_SEL_BLUE |
                      R300_OUT_FMT_C1_SEL_GREEN |
                      R300_OUT_FMT_C2_SEL_RED |
                      R300_OUT_FMT_C3_SEL_ALPHA);


	/* setup the rasterizer, load FS */
	BEGIN_ACCEL(10);
	    /* 2 components: 2 for tex0 */
	    OUT_ACCEL_REG(R300_RS_COUNT,
                      ((2 << R300_RS_COUNT_IT_COUNT_SHIFT) |
                        R300_RS_COUNT_HIRES_EN));

	    OUT_ACCEL_REG(R300_RS_INST_COUNT, R300_INST_COUNT_RS(0) | R300_TX_OFFSET_RS(6));

	    OUT_ACCEL_REG(R300_US_CODE_OFFSET, (R300_ALU_CODE_OFFSET(0) |
                        R300_ALU_CODE_SIZE(0)   |
						R300_TEX_CODE_OFFSET(0) |
						R300_TEX_CODE_SIZE(0)));

	    OUT_ACCEL_REG(R300_US_CODE_ADDR_3,
			  (R300_ALU_START(0) |
               R300_ALU_SIZE(0)  |
			   R300_TEX_START(0) |
               R300_TEX_SIZE(0)  |
			   R300_RGBA_OUT));

        OUT_ACCEL_REG(R300_US_PIXSIZE, 1); /* highest temp used */
	/* shader output swizzling */
        OUT_ACCEL_REG(R300_US_OUT_FMT_0, output_fmt);

	/* tex inst for src texture is pre-loaded in RADEONInit3DEngine() */
	/* tex inst for mask texture is pre-loaded in RADEONInit3DEngine() */

	/* RGB inst
	 * temp addresses for texture inputs
	 * ALU_RGB_ADDR0 is src tex (temp 0)
	 * ALU_RGB_ADDR1 is mask tex (temp 1)
	 * R300_ALU_RGB_OMASK - output components to write
	 * R300_ALU_RGB_TARGET_A - render target
	 */
        OUT_ACCEL_REG(R300_US_ALU_RGB_ADDR(0),
                     (R300_ALU_RGB_ADDR0(0) |
                      R300_ALU_RGB_ADDR1(1) |
                      R300_ALU_RGB_ADDR2(0) |
                      R300_ALU_RGB_ADDRD(0) |
                      R300_ALU_RGB_OMASK((R300_ALU_RGB_MASK_R |
                                          R300_ALU_RGB_MASK_G |
                                          R300_ALU_RGB_MASK_B)) |
                      R300_ALU_RGB_TARGET_A));
	/* RGB inst
	 * ALU operation
	 */
	OUT_ACCEL_REG(R300_US_ALU_RGB_INST(0),
                 (R300_ALU_RGB_SEL_A(src_color) |
                  R300_ALU_RGB_MOD_A(R300_ALU_RGB_MOD_NOP) |
                  R300_ALU_RGB_SEL_B(mask_color) |
                  R300_ALU_RGB_MOD_B(R300_ALU_RGB_MOD_NOP) |
                  R300_ALU_RGB_SEL_C(R300_ALU_RGB_0_0) |
                  R300_ALU_RGB_MOD_C(R300_ALU_RGB_MOD_NOP) |
                  R300_ALU_RGB_OP(R300_ALU_RGB_OP_MAD) |
                  R300_ALU_RGB_OMOD(R300_ALU_RGB_OMOD_NONE) |
                  R300_ALU_RGB_CLAMP));
	/* Alpha inst
	 * temp addresses for texture inputs
	 * ALU_ALPHA_ADDR0 is src tex (0)
	 * ALU_ALPHA_ADDR1 is mask tex (1)
	 * R300_ALU_ALPHA_OMASK - output components to write
	 * R300_ALU_ALPHA_TARGET_A - render target
	 */
	OUT_ACCEL_REG(R300_US_ALU_ALPHA_ADDR(0),
                 (R300_ALU_ALPHA_ADDR0(0) |
                  R300_ALU_ALPHA_ADDR1(1) |
                  R300_ALU_ALPHA_ADDR2(0) |
                  R300_ALU_ALPHA_ADDRD(0) |
                  R300_ALU_ALPHA_OMASK(R300_ALU_ALPHA_MASK_A) |
                  R300_ALU_ALPHA_TARGET_A |
                  R300_ALU_ALPHA_OMASK_W(R300_ALU_ALPHA_MASK_NONE)));
	/* Alpha inst
	 * ALU operation
	 */
	OUT_ACCEL_REG(R300_US_ALU_ALPHA_INST(0),
                 (R300_ALU_ALPHA_SEL_A(src_alpha) |
                  R300_ALU_ALPHA_MOD_A(R300_ALU_ALPHA_MOD_NOP) |
                  R300_ALU_ALPHA_SEL_B(mask_alpha) |
                  R300_ALU_ALPHA_MOD_B(R300_ALU_ALPHA_MOD_NOP) |
                  R300_ALU_ALPHA_SEL_C(R300_ALU_ALPHA_0_0) |
                  R300_ALU_ALPHA_MOD_C(R300_ALU_ALPHA_MOD_NOP) |
                  R300_ALU_ALPHA_OP(R300_ALU_ALPHA_OP_MAD) |
                  R300_ALU_ALPHA_OMOD(R300_ALU_ALPHA_OMOD_NONE) |
                  R300_ALU_ALPHA_CLAMP));
    FINISH_ACCEL();
    }
    else
    {
        u32_t output_fmt;
        u32_t src_color, src_alpha;
        u32_t mask_color, mask_alpha;

//        if (PICT_FORMAT_RGB(PICT_a8r8g8b8) == 0)
//            src_color = (R500_ALU_RGB_R_SWIZ_A_0 |
//                         R500_ALU_RGB_G_SWIZ_A_0 |
//                         R500_ALU_RGB_B_SWIZ_A_0);
//        else
	    src_color = (R500_ALU_RGB_R_SWIZ_A_R |
                     R500_ALU_RGB_G_SWIZ_A_G |
                     R500_ALU_RGB_B_SWIZ_A_B);

//        if (PICT_FORMAT_A(PICT_a8r8g8b8) == 0)
//            src_alpha = R500_ALPHA_SWIZ_A_1;
//        else
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
        COMMIT_RING();

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
    OUT_ACCEL_REG(R300_RB3D_BLENDCNTL, blendcntl | R300_ALPHA_BLEND_ENABLE |
                                       R300_READ_ENABLE);

    FINISH_ACCEL();

    safe_sti(ifl);

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
do {                                            \
    OUT_RING_F(_dstX);                          \
    OUT_RING_F(_dstY);                          \
    OUT_RING_F(_srcX);                          \
    OUT_RING_F(_srcY);                          \
} while (0)



static int R300CompositeTile(int srcX, int srcY,
                                int dstX, int dstY,
                                int w, int h)
{
  //  RINFO_FROM_SCREEN(pDst->drawable.pScreen);
    int vtx_count;
    xPointFixed srcTopLeft, srcTopRight, srcBottomLeft, srcBottomRight;
    xPointFixed maskTopLeft, maskTopRight, maskBottomLeft, maskBottomRight;

    u32_t *ring;

    u32_t ifl;

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


    ifl = safe_cli();

    BEGIN_RING(7 + 4 * vtx_count);

      OUT_ACCEL_REG(R300_VAP_VTX_SIZE, vtx_count);

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

    safe_sti(ifl);

 //   LEAVE_DRAW(0);
}


static int R200CompositeTile(int srcX, int srcY,
                             int dstX, int dstY,
                             int w, int h)
{
    xPointFixed srcTopLeft, srcTopRight, srcBottomLeft, srcBottomRight;
    xPointFixed maskTopLeft, maskTopRight, maskBottomLeft, maskBottomRight;

    srcTopLeft.x     = IntToxFixed(srcX);
    srcTopLeft.y     = IntToxFixed(srcY);
    srcTopRight.x    = IntToxFixed(srcX + w);
    srcTopRight.y    = IntToxFixed(srcY);
    srcBottomLeft.x  = IntToxFixed(srcX);
    srcBottomLeft.y  = IntToxFixed(srcY + h);
    srcBottomRight.x = IntToxFixed(srcX + w);
    srcBottomRight.y = IntToxFixed(srcY + h);

    {
        u32_t *ring;
        u32_t  ifl = safe_cli();

        BEGIN_RING(4 + 4 * VTX_COUNT);

        OUT_RING(CP_PACKET3(R200_CP_PACKET3_3D_DRAW_IMMD_2, 4 * VTX_COUNT));
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

        OUT_ACCEL_REG(RADEON_WAIT_UNTIL, RADEON_WAIT_3D_IDLECLEAN);

        COMMIT_RING();
        safe_sti(ifl);
    };
};

#undef VTX_OUT
#undef VTX_OUT_MASK


int RadeonComposite( io_blit_t *blit)
{
    int tileSrcY, tileMaskY, tileDstY;
    int remainingHeight;

    local_pixmap_t *srcpixmap;
    local_pixmap_t *dstpixmap;

    dbgprintf("Blit Alpha src: %x dst: %x\n",blit->srcpix, blit->dstpix);

    dstpixmap = (blit->dstpix == (void*)-1) ? &scr_pixmap : blit->dstpix ;
    srcpixmap = (blit->srcpix == (void*)-1) ? &scr_pixmap : blit->srcpix ;

    {
        u32_t *ring;
        u32_t ifl = safe_cli();

        BEGIN_RING(2 + 6);

        CP_REG(R5XX_DP_WRITE_MASK, 0xFF000000);

        OUT_RING(CP_PACKET3(RADEON_CNTL_PAINT_MULTI, 4));

        OUT_RING(RADEON_GMC_DST_PITCH_OFFSET_CNTL  |
                 RADEON_GMC_BRUSH_SOLID_COLOR      |
                 RADEON_GMC_DST_32BPP              |
                 RADEON_GMC_SRC_DATATYPE_COLOR     |
                 R5XX_GMC_CLR_CMP_CNTL_DIS         |
                 R5XX_ROP3_P
                );

        OUT_RING(srcpixmap->pitch_offset);
        OUT_RING(blit->alpha<<24);
        OUT_RING( 0 );
        OUT_RING((srcpixmap->width<<16)|srcpixmap->height);

        COMMIT_RING();

        RHDPtr info = &rhd;

        if( IS_R300_3D || IS_R500_3D )
        {
            R300PrepareComposite(dstpixmap, blit->dst_x, blit->dst_y,
                                 srcpixmap, blit->src_x, blit->src_y,
                                 blit->w, blit->h, 3);

//    if (!need_src_tile_x && !need_src_tile_y) {
            R300CompositeTile( blit->src_x, blit->src_y,
                               blit->dst_x, blit->dst_y,
                               blit->w, blit->h);
        }
        else if ((info->ChipFamily == CHIP_FAMILY_RV250) ||
                 (info->ChipFamily == CHIP_FAMILY_RV280) ||
                 (info->ChipFamily == CHIP_FAMILY_RS300) ||
                 (info->ChipFamily == CHIP_FAMILY_R200))
        {
            R200PrepareComposite(dstpixmap, blit->dst_x, blit->dst_y,
                                 srcpixmap, blit->src_x, blit->src_y,
                                 blit->w, blit->h, blit->alpha);
            R200CompositeTile( blit->src_x, blit->src_y,
                               blit->dst_x, blit->dst_y,
                               blit->w, blit->h);
        };


        safe_sti(ifl);
    };



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

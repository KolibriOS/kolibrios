

int ClearPixmap(io_clear_t *io)
{
     u32_t *ring;

     local_pixmap_t *dstpixmap;

     dstpixmap = (io->dstpix == (void*)-1) ? &scr_pixmap : io->dstpix ;

     lock_device();

#if R300_PIO

     FIFOWait(6);

     OUTREG(R5XX_DP_GUI_MASTER_CNTL,
            RADEON_GMC_DST_PITCH_OFFSET_CNTL  |
            RADEON_GMC_BRUSH_SOLID_COLOR      |
            RADEON_GMC_DST_32BPP              |
            RADEON_GMC_SRC_DATATYPE_COLOR     |
            R5XX_GMC_CLR_CMP_CNTL_DIS         |
            R5XX_GMC_WR_MSK_DIS               |
            R5XX_ROP3_P
           );

     OUTREG(R5XX_DP_BRUSH_FRGD_CLR, io->color);
     OUTREG(R5XX_DP_CNTL, R5XX_DST_X_LEFT_TO_RIGHT | R5XX_DST_Y_TOP_TO_BOTTOM);
     OUTREG(R5XX_DST_PITCH_OFFSET, dstpixmap->pitch_offset);
     OUTREG(R5XX_DST_Y_X, 0);
     OUTREG(R5XX_DST_WIDTH_HEIGHT,(dstpixmap->width<<16)|dstpixmap->height);

#else
      BEGIN_RING(6);

        OUT_RING(CP_PACKET3(RADEON_CNTL_PAINT_MULTI, 4));

        OUT_RING(RADEON_GMC_DST_PITCH_OFFSET_CNTL  |
                 RADEON_GMC_BRUSH_SOLID_COLOR      |
                 RADEON_GMC_DST_32BPP              |
                 RADEON_GMC_SRC_DATATYPE_COLOR     |
                 R5XX_GMC_CLR_CMP_CNTL_DIS         |
                 R5XX_GMC_WR_MSK_DIS               |
                 R5XX_ROP3_P
                );

        OUT_RING(dstpixmap->pitch_offset);
        OUT_RING(io->color);
        OUT_RING( 0 );
        OUT_RING((dstpixmap->width<<16)|dstpixmap->height);
      COMMIT_RING();

#endif

     unlock_device();

     return ERR_OK;
}


int Line(io_draw_t *draw)
{
     local_pixmap_t *dstpixmap;
     clip_t clip;
     int x0, y0, x1, y1;

     dstpixmap = (draw->dstpix == (void*)-1) ? &scr_pixmap : draw->dstpix ;

     x0 = draw->x0;
     y0 = draw->y0;

     x1 = draw->x1;
     y1 = draw->y1;

     clip.xmin = 0;
     clip.ymin = 0;
     clip.xmax = dstpixmap->width-1;
     clip.ymax = dstpixmap->height-1;

    if ( !LineClip(&clip, &x0, &y0, &x1, &y1 ))
    {
        u32_t *ring, write;

        lock_device();

#if R300_PIO

        FIFOWait(6);

        OUTREG(R5XX_DP_GUI_MASTER_CNTL,
               RADEON_GMC_DST_PITCH_OFFSET_CNTL  |
               RADEON_GMC_BRUSH_SOLID_COLOR      |
               RADEON_GMC_DST_32BPP              |
               RADEON_GMC_SRC_DATATYPE_COLOR     |
               R5XX_GMC_CLR_CMP_CNTL_DIS         |
               R5XX_GMC_WR_MSK_DIS               |
               R5XX_ROP3_P
              );

        OUTREG(R5XX_DST_LINE_PATCOUNT, 0x55 << R5XX_BRES_CNTL_SHIFT);

        OUTREG(R5XX_DP_BRUSH_FRGD_CLR, draw->color);
        OUTREG(R5XX_DST_PITCH_OFFSET, dstpixmap->pitch_offset);

        OUTREG(R5XX_DST_LINE_START,(y0<<16)|x0);
        OUTREG(R5XX_DST_LINE_END,(y1<<16)|x1);
#else
       BEGIN_RING(6);

         OUT_RING(CP_PACKET3(RADEON_CNTL_PAINT_POLYLINE, 4));
         OUT_RING(RADEON_GMC_DST_PITCH_OFFSET_CNTL  |
                  RADEON_GMC_BRUSH_SOLID_COLOR      |
                  RADEON_GMC_DST_32BPP              |
                  RADEON_GMC_SRC_DATATYPE_COLOR     |
                  R5XX_GMC_CLR_CMP_CNTL_DIS         |
                  R5XX_GMC_WR_MSK_DIS               |
                  R5XX_ROP3_P);

         OUT_RING(dstpixmap->pitch_offset);
         OUT_RING(draw->color);
         OUT_RING((y0<<16)|x0);
         OUT_RING((y1<<16)|x1);
       COMMIT_RING();
#endif

        unlock_device();
    };
    return ERR_OK;
}

int DrawRect(io_draw_t* draw)
{
    int x0, y0, x1, y1, xend, yend;

    local_pixmap_t *dstpixmap;
    clip_t dst_clip;

    dstpixmap = (draw->dstpix == (void*)-1) ? &scr_pixmap : draw->dstpix ;

    x0 = draw->x0;
    y0 = draw->y0;

    x1 = xend = x0 + draw->w - 1;
    y1 = yend = y0 + draw->h - 1;

    dst_clip.xmin = 0;
    dst_clip.ymin = 0;
    dst_clip.xmax = dstpixmap->width-1;
    dst_clip.ymax = dstpixmap->height-1;


//  dbgprintf("draw rect x0:%d, y0:%d, x1:%d, y1:%d, color: %x\n",
//             x0, y0, x1, y1, draw->color);

    if( ! BlockClip( &dst_clip, &x0, &y0, &x1, &y1))
    {
        u32_t *ring;
        int w, h;

        w = x1 - x0 + 1;
        h = y1 - y0 + 1;

        lock_device();

#if R300_PIO

        FIFOWait(7);

        OUTREG(R5XX_DP_GUI_MASTER_CNTL,
               RADEON_GMC_DST_PITCH_OFFSET_CNTL  |
               RADEON_GMC_BRUSH_SOLID_COLOR      |
               RADEON_GMC_DST_32BPP              |
               RADEON_GMC_SRC_DATATYPE_COLOR     |
               R5XX_GMC_CLR_CMP_CNTL_DIS         |
               R5XX_GMC_WR_MSK_DIS               |
               R5XX_ROP3_P
              );

        OUTREG(R5XX_DP_BRUSH_FRGD_CLR, draw->color);
        OUTREG(R5XX_DP_CNTL, R5XX_DST_X_LEFT_TO_RIGHT | R5XX_DST_Y_TOP_TO_BOTTOM);
        OUTREG(R5XX_DST_PITCH_OFFSET, dstpixmap->pitch_offset);
        OUTREG(R5XX_DST_Y_X,(y0<<16)|x0);
        OUTREG(R5XX_DST_WIDTH_HEIGHT,(w<<16)|h);

        if( draw->color != draw->border)
        {
            OUTREG(R5XX_DP_BRUSH_FRGD_CLR, draw->border);

            if( y0 == draw->y0)
            {
                FIFOWait(2);

                OUTREG(R5XX_DST_Y_X,(y0<<16)|x0);
                OUTREG(R5XX_DST_WIDTH_HEIGHT,(w<<16)|1);
                y0++;
                h--;
            }
            if( y1 == yend )
            {
                FIFOWait(2);

                OUTREG(R5XX_DST_Y_X,(y1<<16)|x0);
                OUTREG(R5XX_DST_WIDTH_HEIGHT,(w<<16)|1);
                h--;
            }
            if( (h > 0) && (x0 == draw->x0))
            {
                FIFOWait(2);

                OUTREG(R5XX_DST_Y_X,(y0<<16)|x0);
                OUTREG(R5XX_DST_WIDTH_HEIGHT,(1<<16)|h);
            }
            if( (h > 0) && (x1 == xend))
            {
                FIFOWait(2);

                OUTREG(R5XX_DST_Y_X,(y0<<16)|x1);
                OUTREG(R5XX_DST_WIDTH_HEIGHT,(1<<16)|h);
            }
        };
#else

      BEGIN_RING(64);

        OUT_RING(CP_PACKET3(RADEON_CNTL_PAINT_MULTI, 4));

        OUT_RING(RADEON_GMC_DST_PITCH_OFFSET_CNTL  |
                 RADEON_GMC_BRUSH_SOLID_COLOR      |
                 RADEON_GMC_DST_32BPP              |
                 RADEON_GMC_SRC_DATATYPE_COLOR     |
                 R5XX_GMC_CLR_CMP_CNTL_DIS         |
                 R5XX_GMC_WR_MSK_DIS               |
                 R5XX_ROP3_P
                );

        OUT_RING(dstpixmap->pitch_offset);
        OUT_RING(draw->color);
        OUT_RING((x0<<16)|y0);
        OUT_RING((w<<16)|h);
        OUT_RING(CP_PACKET2());
        OUT_RING(CP_PACKET2());

        if( draw->color != draw->border)
        {
            if( y0 == draw->y0) {
                OUT_RING(CP_PACKET3(RADEON_CNTL_PAINT_MULTI, 4));
                OUT_RING(RADEON_GMC_DST_PITCH_OFFSET_CNTL  |
                         RADEON_GMC_BRUSH_SOLID_COLOR      |
                         RADEON_GMC_DST_32BPP              |
                         RADEON_GMC_SRC_DATATYPE_COLOR     |
                         R5XX_GMC_CLR_CMP_CNTL_DIS         |
                         R5XX_GMC_WR_MSK_DIS               |
                         R5XX_ROP3_P
                        );

                OUT_RING(dstpixmap->pitch_offset);
                OUT_RING(draw->border);
                OUT_RING((x0<<16)|y0);
                OUT_RING((w<<16)|1);
                OUT_RING(CP_PACKET2());
                OUT_RING(CP_PACKET2());

           //   y0++;
           //   h--;
            }
            if( y1 == yend ) {
                OUT_RING(CP_PACKET3(RADEON_CNTL_PAINT_MULTI, 4));
                OUT_RING(RADEON_GMC_DST_PITCH_OFFSET_CNTL  |
                         RADEON_GMC_BRUSH_SOLID_COLOR      |
                         RADEON_GMC_DST_32BPP              |
                         RADEON_GMC_SRC_DATATYPE_COLOR     |
                         R5XX_GMC_CLR_CMP_CNTL_DIS         |
                         R5XX_GMC_WR_MSK_DIS               |
                         R5XX_ROP3_P
                        );

                OUT_RING(dstpixmap->pitch_offset);
                OUT_RING(draw->border);
                OUT_RING((x0<<16)|y1);
                OUT_RING((w<<16)|1);
                OUT_RING(CP_PACKET2());
                OUT_RING(CP_PACKET2());
            //  h--;
            }
            if( (h > 0) && (x0 == draw->x0)) {
                OUT_RING(CP_PACKET3(RADEON_CNTL_PAINT_MULTI, 4));
                OUT_RING(RADEON_GMC_DST_PITCH_OFFSET_CNTL  |
                         RADEON_GMC_BRUSH_SOLID_COLOR      |
                         RADEON_GMC_DST_32BPP              |
                         RADEON_GMC_SRC_DATATYPE_COLOR     |
                         R5XX_GMC_CLR_CMP_CNTL_DIS         |
                         R5XX_GMC_WR_MSK_DIS               |
                         R5XX_ROP3_P
                      );

                OUT_RING(dstpixmap->pitch_offset);
                OUT_RING(draw->border);
                OUT_RING((x0<<16)|y0);
                OUT_RING((1<<16)|h);
                OUT_RING(CP_PACKET2());
                OUT_RING(CP_PACKET2());
            }
            if( (h > 0) && (x1 == xend)) {
                OUT_RING(CP_PACKET3(RADEON_CNTL_PAINT_MULTI, 4));
                OUT_RING(RADEON_GMC_DST_PITCH_OFFSET_CNTL  |
                       RADEON_GMC_BRUSH_SOLID_COLOR      |
                       RADEON_GMC_DST_32BPP              |
                       RADEON_GMC_SRC_DATATYPE_COLOR     |
                       R5XX_GMC_CLR_CMP_CNTL_DIS         |
                       R5XX_GMC_WR_MSK_DIS               |
                       R5XX_ROP3_P
                      );

                OUT_RING(dstpixmap->pitch_offset);
                OUT_RING(draw->border);
                OUT_RING((x1<<16)|y0);
                OUT_RING((1<<16)|h);
                OUT_RING(CP_PACKET2());
                OUT_RING(CP_PACKET2());
            }
        };

/*

        CP_REG(R5XX_DP_GUI_MASTER_CNTL,
                 RADEON_GMC_DST_PITCH_OFFSET_CNTL  |
                 RADEON_GMC_BRUSH_SOLID_COLOR      |
                 RADEON_GMC_DST_32BPP              |
                 RADEON_GMC_SRC_DATATYPE_COLOR     |
                 R5XX_GMC_CLR_CMP_CNTL_DIS         |
                 R5XX_GMC_WR_MSK_DIS               |
                 R5XX_ROP3_P
                );
        CP_REG(R5XX_DP_BRUSH_FRGD_CLR, draw->color);
        CP_REG(R5XX_DP_CNTL, R5XX_DST_X_LEFT_TO_RIGHT | R5XX_DST_Y_TOP_TO_BOTTOM);

        CP_REG(R5XX_DST_PITCH_OFFSET, dstpixmap->pitch_offset);
        CP_REG(R5XX_DST_Y_X,(y0<<16)|x0);
        CP_REG(R5XX_DST_WIDTH_HEIGHT,(w<<16)|h);
        if( draw->color != draw->border)
        {
        CP_REG(R5XX_DP_GUI_MASTER_CNTL,
                 RADEON_GMC_DST_PITCH_OFFSET_CNTL  |
                 RADEON_GMC_BRUSH_SOLID_COLOR      |
                 RADEON_GMC_DST_32BPP              |
                 RADEON_GMC_SRC_DATATYPE_COLOR     |
                 R5XX_GMC_CLR_CMP_CNTL_DIS         |
                 R5XX_GMC_WR_MSK_DIS               |
                 R5XX_ROP3_P
                );
           CP_REG(R5XX_DP_BRUSH_FRGD_CLR, draw->border);
        CP_REG(R5XX_DP_CNTL, R5XX_DST_X_LEFT_TO_RIGHT | R5XX_DST_Y_TOP_TO_BOTTOM);

        CP_REG(R5XX_DST_PITCH_OFFSET, dstpixmap->pitch_offset);


           if( y0 == draw->y0) {
              CP_REG(R5XX_DST_Y_X,(y0<<16)|x0);
              CP_REG(R5XX_DST_WIDTH_HEIGHT,(w<<16)|1);
              y0++;
              h--;
           }
           if( y1 == yend ) {
              CP_REG(R5XX_DST_Y_X,(y1<<16)|x0);
              CP_REG(R5XX_DST_WIDTH_HEIGHT,(w<<16)|1);
              h--;
           }
           if( (h > 0) && (x0 == draw->x0)) {
              CP_REG(R5XX_DST_Y_X,(y0<<16)|x0);
              CP_REG(R5XX_DST_WIDTH_HEIGHT,(1<<16)|h);
           }
           if( (h > 0) && (x1 == xend)) {
              CP_REG(R5XX_DST_Y_X,(y0<<16)|x1);
              CP_REG(R5XX_DST_WIDTH_HEIGHT,(1<<16)|h);
           }
        };
*/

      COMMIT_RING();
#endif
        unlock_device();
    };
    return ERR_OK;
}

int FillRect(io_fill_t *fill)
{
    local_pixmap_t *dstpixmap;
    clip_t dst_clip;
    int x0, y0, x1, y1, xend, yend;

    dstpixmap = (fill->dstpix == (void*)-1) ? &scr_pixmap : fill->dstpix ;

    x0 = fill->x;
    y0 = fill->y;

    xend = x1 = x0 + fill->w - 1;
    yend = y1 = y0 + fill->h - 1;

    dst_clip.xmin = 0;
    dst_clip.ymin = 0;
    dst_clip.xmax = dstpixmap->width-1;
    dst_clip.ymax = dstpixmap->height-1;

//  dbgprintf("fill rect x0:%d, y0:%d, x1:%d, y1:%d\n",
//             x0, y0, x1, y1);

    if( ! BlockClip(&dst_clip, &x0, &y0, &x1, &y1))
    {
        u32_t *ring, write;

        int w = x1 - x0 + 1;
        int h = y1 - y0 + 1;

        lock_device();

#if R300_PIO

        FIFOWait(9);

        OUTREG(R5XX_DP_GUI_MASTER_CNTL,
               RADEON_GMC_DST_PITCH_OFFSET_CNTL |
               R5XX_GMC_BRUSH_8X8_MONO_FG_BG    |
               RADEON_GMC_DST_32BPP             |
               R5XX_GMC_SRC_DATATYPE_COLOR      |
               R5XX_GMC_CLR_CMP_CNTL_DIS        |
               R5XX_GMC_WR_MSK_DIS              |
               R5XX_ROP3_P
               );

        OUTREG(R5XX_DP_BRUSH_BKGD_CLR, fill->bkcolor);
        OUTREG(R5XX_DP_BRUSH_FRGD_CLR, fill->fcolor);

        OUTREG(R5XX_BRUSH_DATA0, fill->bmp0);
        OUTREG(R5XX_BRUSH_DATA1, fill->bmp1);

        OUTREG(R5XX_DP_CNTL, R5XX_DST_X_LEFT_TO_RIGHT | R5XX_DST_Y_TOP_TO_BOTTOM);
        OUTREG(R5XX_DST_PITCH_OFFSET, dstpixmap->pitch_offset);

        OUTREG(R5XX_DST_Y_X,(y0<<16)|x0);
        OUTREG(R5XX_DST_HEIGHT_WIDTH,(h<<16)|w);

        if( (fill->border & 0xFF000000) != 0)
        {
            FIFOWait(2);

            OUTREG(R5XX_DP_GUI_MASTER_CNTL,
                   RADEON_GMC_DST_PITCH_OFFSET_CNTL  |
                   RADEON_GMC_BRUSH_SOLID_COLOR      |
                   RADEON_GMC_DST_32BPP              |
                   RADEON_GMC_SRC_DATATYPE_COLOR     |
                   R5XX_GMC_CLR_CMP_CNTL_DIS         |
                   R5XX_GMC_WR_MSK_DIS               |
                   R5XX_ROP3_P
                  );

            OUTREG(R5XX_DP_BRUSH_FRGD_CLR, fill->border);

            if( y0 == fill->y)
            {
                FIFOWait(2);

                OUTREG(R5XX_DST_Y_X,(y0<<16)|x0);
                OUTREG(R5XX_DST_WIDTH_HEIGHT,(w<<16)|1);
                y0++;
                h--;
            }
            if( y1 == yend )
            {
                FIFOWait(2);

                OUTREG(R5XX_DST_Y_X,(y1<<16)|x0);
                OUTREG(R5XX_DST_WIDTH_HEIGHT,(w<<16)|1);
                h--;
            }
            if( (h > 0) && (x0 == fill->x))
            {
                FIFOWait(2);

                OUTREG(R5XX_DST_Y_X,(y0<<16)|x0);
                OUTREG(R5XX_DST_WIDTH_HEIGHT,(1<<16)|h);
            }
            if( (h > 0) && (x1 == xend))
            {
                FIFOWait(2);

                OUTREG(R5XX_DST_Y_X,(y0<<16)|x1);
                OUTREG(R5XX_DST_WIDTH_HEIGHT,(1<<16)|h);
            }
        };


#else
      BEGIN_RING(9+10*2);

        OUT_RING(CP_PACKET3(RADEON_CNTL_PAINT, 7));
        OUT_RING(RADEON_GMC_DST_PITCH_OFFSET_CNTL    |
                 R5XX_GMC_BRUSH_8X8_MONO_FG_BG       |
                 RADEON_GMC_DST_32BPP                |
                 RADEON_GMC_SRC_DATATYPE_COLOR       |
                 R5XX_GMC_CLR_CMP_CNTL_DIS           |
                 R5XX_GMC_WR_MSK_DIS                 |
                 R5XX_ROP3_P
                );

        OUT_RING(dstpixmap->pitch_offset);
        OUT_RING(fill->bkcolor);
        OUT_RING(fill->fcolor);

        OUT_RING(fill->bmp0);
        OUT_RING(fill->bmp1);

        OUT_RING((y0<<16)|x0);
        OUT_RING((y1<<16)|x1);

        if( (fill->border & 0xFF000000) != 0)
        {
            CP_REG(R5XX_DP_GUI_MASTER_CNTL,
                   RADEON_GMC_DST_PITCH_OFFSET_CNTL  |
                   RADEON_GMC_BRUSH_SOLID_COLOR      |
                   RADEON_GMC_DST_32BPP              |
                   RADEON_GMC_SRC_DATATYPE_COLOR     |
                   R5XX_GMC_CLR_CMP_CNTL_DIS         |
                   R5XX_GMC_WR_MSK_DIS               |
                   R5XX_ROP3_P
                  );

            CP_REG(R5XX_DP_BRUSH_FRGD_CLR, fill->border);

            if( y0 == fill->y)
            {
                CP_REG(R5XX_DST_Y_X,(y0<<16)|x0);
                CP_REG(R5XX_DST_WIDTH_HEIGHT,(w<<16)|1);
                y0++;
                h--;
            }
            if( y1 == yend )
            {
                CP_REG(R5XX_DST_Y_X,(y1<<16)|x0);
                CP_REG(R5XX_DST_WIDTH_HEIGHT,(w<<16)|1);
                h--;
            }
            if( (h > 0) && (x0 == fill->x))
            {
                CP_REG(R5XX_DST_Y_X,(y0<<16)|x0);
                CP_REG(R5XX_DST_WIDTH_HEIGHT,(1<<16)|h);
            }
            if( (h > 0) && (x1 == xend))
            {
                CP_REG(R5XX_DST_Y_X,(y0<<16)|x1);
                CP_REG(R5XX_DST_WIDTH_HEIGHT,(1<<16)|h);
            }
        };

      COMMIT_RING();

#endif
        unlock_device();
     };
     return ERR_OK;
};


#define ADDRREG(addr)  ((volatile u32_t *)(rhd.MMIOBase + (addr)))


static int blit_host(u32_t dstpitch, int dstx, int dsty,
                     u32_t  src, int srcx, int srcy,
                     int w, int h, int srcpitch, Bool trans, color_t key)
{
    u32_t     dp_cntl;
    color_t  *src_addr;

    lock_device();

#if R300_PIO

    dp_cntl =  RADEON_GMC_DST_PITCH_OFFSET_CNTL  |
               RADEON_GMC_BRUSH_NONE             |
               RADEON_GMC_DST_32BPP              |
               RADEON_GMC_SRC_DATATYPE_COLOR     |
               RADEON_DP_SRC_SOURCE_HOST_DATA    |
               R5XX_GMC_WR_MSK_DIS               |
               R5XX_ROP3_S;

    if( trans == FALSE )
    {
        dp_cntl|= R5XX_GMC_CLR_CMP_CNTL_DIS;
        FIFOWait(5);
    }
    else
        FIFOWait(8);

    OUTREG(R5XX_DP_GUI_MASTER_CNTL, dp_cntl);

    OUTREG(R5XX_DP_CNTL, R5XX_DST_X_LEFT_TO_RIGHT |
                         R5XX_DST_Y_TOP_TO_BOTTOM);

    OUTREG(R5XX_DST_PITCH_OFFSET, dstpitch);

    if( trans )
    {
        OUTREG(R5XX_CLR_CMP_CLR_SRC, key);
        OUTREG(R5XX_CLR_CMP_MASK, R5XX_CLR_CMP_MSK);
        OUTREG(R5XX_CLR_CMP_CNTL, R5XX_SRC_CMP_EQ_COLOR |
                                  R5XX_CLR_CMP_SRC_SOURCE);
    };

    OUTREG(RADEON_DST_Y_X, (dsty << 16) | (dstx & 0xffff));
    OUTREG(RADEON_DST_HEIGHT_WIDTH, (h << 16) | w);

    src_addr = &((color_t*)src)[srcpitch*srcy/4+srcx];

    while ( h-- )
    {
        color_t *tmp_src = src_addr;
        src_addr += srcpitch/4;

        int left = w;

        while( left )
        {
            volatile u32_t *d;

            if( left > 8 )
            {
                int i;

                R5xxFIFOWait(8);
                d = ADDRREG(RADEON_HOST_DATA0);

				/* Unrolling doesn't improve performance */
                for ( i = 0; i < 8; i++)
                    *d++ = *tmp_src++;
                left -= 8;
            }
            else
            {
                R5xxFIFOWait(left);

                if( h )
                    d = ADDRREG(RADEON_HOST_DATA7) - (left - 1);
                else
                    d = ADDRREG(RADEON_HOST_DATA_LAST) - (left - 1);

                for ( ; left; --left)
                    *d++ = *tmp_src++;
                left = 0;
            };
        };
    };

#endif

    unlock_device();

    return ERR_OK;
}


int Blit(io_blit_t *blit)
{
    clip_t src_clip, dst_clip;

    local_pixmap_t *srcpixmap;
    local_pixmap_t *dstpixmap;

    u32_t  srcpitchoffset;
    Bool   need_sync = FALSE;

    dstpixmap = (blit->dstpix == (void*)-1) ? &scr_pixmap : blit->dstpix ;
    srcpixmap = (blit->srcpix == (void*)-1) ? &scr_pixmap : blit->srcpix ;

    src_clip.xmin = 0;
    src_clip.ymin = 0;
    src_clip.xmax = srcpixmap->width-1;
    src_clip.ymax = srcpixmap->height-1;

    dst_clip.xmin = 0;
    dst_clip.ymin = 0;
    dst_clip.xmax = dstpixmap->width-1;
    dst_clip.ymax = dstpixmap->height-1;

    if( !blit_clip(&dst_clip, &blit->dst_x, &blit->dst_y,
                   &src_clip, &blit->src_x, &blit->src_y,
                   &blit->w, &blit->h) )
    {
        u32_t *ring, write;
/*
        if( (srcpixmap->flags & PX_MEM_MASK)==PX_MEM_SYSTEM)
            return blit_host(dstpixmap->pitch_offset,
                             blit->dst_x, blit->dst_y,
                             srcpixmap->mapped,
                             blit->src_x, blit->src_y,
                             blit->w, blit->h,
                             srcpixmap->pitch,
                             FALSE, 0 );
*/

//        if( (srcpixmap->flags & PX_MEM_MASK)==PX_MEM_SYSTEM)
//        {
//            srcpitchoffset =  bind_pixmap(srcpixmap);
//            need_sync = TRUE;
//        }
//        else
            srcpitchoffset = srcpixmap->pitch_offset;

        lock_device();

#if R300_PIO

        FIFOWait(7);

        OUTREG(R5XX_DP_GUI_MASTER_CNTL,
               RADEON_GMC_SRC_PITCH_OFFSET_CNTL  |
               RADEON_GMC_DST_PITCH_OFFSET_CNTL  |
               RADEON_GMC_BRUSH_NONE             |
               RADEON_GMC_DST_32BPP              |
               RADEON_GMC_SRC_DATATYPE_COLOR     |
               RADEON_DP_SRC_SOURCE_MEMORY       |
               R5XX_GMC_CLR_CMP_CNTL_DIS         |
               R5XX_GMC_WR_MSK_DIS               |
               R5XX_ROP3_S
              );

        OUTREG(R5XX_DP_CNTL, R5XX_DST_X_LEFT_TO_RIGHT | R5XX_DST_Y_TOP_TO_BOTTOM);

        OUTREG(R5XX_DST_PITCH_OFFSET, dstpixmap->pitch_offset);
        OUTREG(R5XX_SRC_PITCH_OFFSET, srcpitchoffset);

        OUTREG(R5XX_SRC_Y_X,(blit->src_y<<16)|blit->src_x);
        OUTREG(R5XX_DST_Y_X,(blit->dst_y<<16)|blit->dst_x);
        OUTREG(R5XX_DST_HEIGHT_WIDTH,(blit->h<<16)|blit->w);

#else
    BEGIN_RING(7);

        OUT_RING(CP_PACKET3(RADEON_CNTL_BITBLT, 5));

        OUT_RING(RADEON_GMC_SRC_PITCH_OFFSET_CNTL  |
                 RADEON_GMC_DST_PITCH_OFFSET_CNTL  |
                 RADEON_GMC_BRUSH_NONE             |
                 RADEON_GMC_DST_32BPP              |
                 RADEON_GMC_SRC_DATATYPE_COLOR     |
                 RADEON_DP_SRC_SOURCE_MEMORY       |
                 R5XX_GMC_CLR_CMP_CNTL_DIS         |
                 R5XX_GMC_WR_MSK_DIS               |
                 R5XX_ROP3_S
                );

        OUT_RING(srcpitchoffset);
        OUT_RING(dstpixmap->pitch_offset);

        OUT_RING((blit->src_x<<16)|blit->src_y);
        OUT_RING((blit->dst_x<<16)|blit->dst_y);
        OUT_RING((blit->w<<16)|blit->h);
    COMMIT_RING();

#endif

       if( need_sync == TRUE )
            R5xx2DIdleLocal();

        unlock_device();

     };
     return ERR_OK;
};


int BlitTransparent(io_blit_t *blit)
{
     clip_t src_clip, dst_clip;

     local_pixmap_t *srcpixmap;
     local_pixmap_t *dstpixmap;

     u32_t  srcpitchoffset;
     Bool   need_sync = FALSE;

    // dbgprintf("Transblit src: %x dst: %x\n",blit->srcpix, blit->dstpix);

     dstpixmap = (blit->dstpix == (void*)-1) ? &scr_pixmap : blit->dstpix ;
     srcpixmap = (blit->srcpix == (void*)-1) ? &scr_pixmap : blit->srcpix ;

     //dbgprintf("srcpixmap: %x dstpixmap: %x\n",srcpixmap, dstpixmap);

     //dbgprintf("dst.width: %d dst.height: %d\n", dstpixmap->width,dstpixmap->height);
     //dbgprintf("src.width: %d src.height: %d\n", srcpixmap->width,srcpixmap->height);
     //dbgprintf("srcpitch: %x dstpitch: %x\n",
     //           srcpixmap->pitch_offset,dstpixmap->pitch_offset);
     src_clip.xmin = 0;
     src_clip.ymin = 0;
     src_clip.xmax = srcpixmap->width-1;
     src_clip.ymax = srcpixmap->height-1;

     dst_clip.xmin = 0;
     dst_clip.ymin = 0;
     dst_clip.xmax = dstpixmap->width-1;
     dst_clip.ymax = dstpixmap->height-1;

     if( !blit_clip(&dst_clip, &blit->dst_x, &blit->dst_y,
                    &src_clip, &blit->src_x, &blit->src_y,
                    &blit->w, &blit->h) )
     {
        u32_t *ring, write;


//        if( (srcpixmap->flags & PX_MEM_MASK)==PX_MEM_SYSTEM)
//        {
//            srcpitchoffset =  bind_pixmap(srcpixmap);
//            need_sync = TRUE;
//        }
//        else
            srcpitchoffset = srcpixmap->pitch_offset;

        lock_device();

#if R300_PIO

        FIFOWait(10);

        OUTREG(R5XX_DP_GUI_MASTER_CNTL,
               RADEON_GMC_SRC_PITCH_OFFSET_CNTL  |
               RADEON_GMC_DST_PITCH_OFFSET_CNTL  |
               RADEON_GMC_BRUSH_NONE             |
               RADEON_GMC_DST_32BPP              |
               RADEON_GMC_SRC_DATATYPE_COLOR     |
               RADEON_DP_SRC_SOURCE_MEMORY       |
               R5XX_GMC_WR_MSK_DIS               |
               R5XX_ROP3_S
              );

        OUTREG(R5XX_DP_CNTL, R5XX_DST_X_LEFT_TO_RIGHT | R5XX_DST_Y_TOP_TO_BOTTOM);

        OUTREG(R5XX_CLR_CMP_CLR_SRC, blit->key);
        OUTREG(R5XX_CLR_CMP_MASK, R5XX_CLR_CMP_MSK);
        OUTREG(R5XX_CLR_CMP_CNTL, R5XX_SRC_CMP_EQ_COLOR | R5XX_CLR_CMP_SRC_SOURCE);

        OUTREG(R5XX_DST_PITCH_OFFSET, dstpixmap->pitch_offset);
        OUTREG(R5XX_SRC_PITCH_OFFSET, srcpitchoffset);

        OUTREG(R5XX_SRC_Y_X,(blit->src_y<<16)|blit->src_x);
        OUTREG(R5XX_DST_Y_X,(blit->dst_y<<16)|blit->dst_x);
        OUTREG(R5XX_DST_HEIGHT_WIDTH,(blit->h<<16)|blit->w);

#else

        BEGIN_RING(10);

          OUT_RING(CP_PACKET3(RADEON_CNTL_TRANBLT, 8));

          OUT_RING(RADEON_GMC_SRC_PITCH_OFFSET_CNTL  |
                   RADEON_GMC_DST_PITCH_OFFSET_CNTL  |
                   RADEON_GMC_BRUSH_NONE             |
                   RADEON_GMC_DST_32BPP              |
                   RADEON_GMC_SRC_DATATYPE_COLOR     |
                   RADEON_DP_SRC_SOURCE_MEMORY       |
                   R5XX_GMC_WR_MSK_DIS               |
                   R5XX_ROP3_S
                  );

          OUT_RING(srcpitchoffset);
          OUT_RING(dstpixmap->pitch_offset);

          OUT_RING(R5XX_CLR_CMP_SRC_SOURCE | R5XX_SRC_CMP_EQ_COLOR);
          OUT_RING(blit->key);
          OUT_RING(0xFFFFFFFF);

          OUT_RING((blit->src_x<<16)|blit->src_y);
          OUT_RING((blit->dst_x<<16)|blit->dst_y);
          OUT_RING((blit->w<<16)|blit->h);

        COMMIT_RING();

#endif

       if( need_sync == TRUE )
            R5xx2DIdleLocal();

       unlock_device();


     };
     return ERR_OK;
}




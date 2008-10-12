

int DrawRect(draw_t* draw)
{
     int x0, y0, x1, y1;

     local_pixmap_t *dstpixmap;
     clip_t dst_clip;

     dstpixmap = (draw->dstpix == (void*)-1) ? &scr_pixmap : draw->dstpix ;

     x0 = draw->x0;
     y0 = draw->y0;

     x1 = x0+draw->w-1;
     y1 = y0+draw->h-1;

     dst_clip.xmin = 0;
     dst_clip.ymin = 0;
     dst_clip.xmax = dstpixmap->width-1;
     dst_clip.ymax = dstpixmap->height-1;


//  dbgprintf("draw rect x0:%d, y0:%d, x1:%d, y1:%d, color: %x\n",
//             x0, y0, x1, y1, draw->color);

     if( ! BlockClip( &dst_clip, &x0, &y0, &x1, &y1))
     {
        u32_t *ring, write;
        u32_t ifl;
        int w, h;

        w = x1-x0+1;
        h = y1-y0+1;

        ifl = safe_cli();

#if R300_PIO

        R5xxFIFOWait(6);

        OUTREG(R5XX_DP_GUI_MASTER_CNTL,
               rhd.gui_control              |
               R5XX_ROP3_P                  |
               R5XX_GMC_BRUSH_SOLID_COLOR   |
               R5XX_GMC_SRC_DATATYPE_COLOR  |
               R5XX_GMC_CLR_CMP_CNTL_DIS    |
               R5XX_GMC_WR_MSK_DIS
              );

        OUTREG(R5XX_DP_BRUSH_FRGD_CLR, draw->color);
        OUTREG(R5XX_DP_CNTL, R5XX_DST_X_LEFT_TO_RIGHT | R5XX_DST_Y_TOP_TO_BOTTOM);
        OUTREG(R5XX_DST_PITCH_OFFSET, dstpixmap->pitch_offset);
        OUTREG(R5XX_DST_Y_X,(draw->y0<<16)|draw->x0);
        OUTREG(R5XX_DST_WIDTH_HEIGHT,(w<<16)|h);
#else
      BEGIN_RING();
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
        OUT_RING((draw->x0<<16)|y0);
        OUT_RING((w<<16)|h);
      COMMIT_RING();
#endif
        safe_sti(ifl);
     };
     return ERR_OK;
}

int FillRect(fill_t *fill)
{
     local_pixmap_t *dstpixmap;
     clip_t dst_clip;
     int x0, y0, x1, y1;

     dstpixmap = (fill->dstpix == (void*)-1) ? &scr_pixmap : fill->dstpix ;

     x0 = fill->x;
     y0 = fill->y;

     x1 = x0+fill->w-1;
     y1 = y0+fill->h-1;

     dst_clip.xmin = 0;
     dst_clip.ymin = 0;
     dst_clip.xmax = dstpixmap->width-1;
     dst_clip.ymax = dstpixmap->height-1;

//  dbgprintf("fill rect x0:%d, y0:%d, x1:%d, y1:%d\n",
//             x0, y0, x1, y1);

     if( ! BlockClip(&dst_clip, &x0, &y0, &x1, &y1))
     {
        u32_t *ring, write;
        u32_t ifl = safe_cli();

#if R300_PIO

        int w = x1-x0+1;
        int h = y1-y0+1;

        R5xxFIFOWait(9);

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
#else
      BEGIN_RING();
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
      COMMIT_RING();

#endif
        safe_sti(ifl);
     };
     return ERR_OK;
};

#if 0
int Blit(blit_t *blit)
{
  int x0, y0, x1, y1;

  x0 = blit->src_x;
  y0 = blit->src_y;

  x1 = x0+blit->w-1;
  y1 = y0+blit->h-1;


  if( ! BlockClip(&clip, &x0, &y0, &x1, &y1))
  {
     u32 *ring, write;
     int w, h;
     u32 ifl;

     w = x1-x0+1;
     h = y1-y0+1;

     ifl = safe_cli();

#if R300_PIO


#else

     BEGIN_RING();
       OUT_RING(CP_PACKET3(RADEON_CNTL_BITBLT, 5));

       OUT_RING(RADEON_GMC_SRC_PITCH_OFFSET_CNTL  |
                RADEON_GMC_DST_PITCH_OFFSET_CNTL  |
                RADEON_GMC_BRUSH_NONE             |
                RADEON_GMC_DST_32BPP              |
                RADEON_GMC_SRC_DATATYPE_COLOR     |
                RADEON_DP_SRC_SOURCE_MEMORY       |
                (1 << 28)+(1 << 30) | R5XX_ROP3_S);

       OUT_RING(rhd.dst_pitch_offset);
       OUT_RING(rhd.dst_pitch_offset);

       OUT_RING((x0<<16)|y0);
       OUT_RING((blit->dst_x<<16)|blit->dst_y);
       OUT_RING((w<<16)|h);
     COMMIT_RING();

#endif

     safe_sti(ifl);
  } ;
  return ERR_OK;
}

int Line2P(draw_t *draw)
{
  local_pixmap_t *dstpixmap;
  int x0, y0, x1, y1;

  dstpixmap = (draw->dstpix == (void*)-1) ? &scr_pixmap : draw->dstpix ;

  x0 = draw->x0;
  y0 = draw->y0;

  x1 = draw->x1;
  y1 = draw->y1;

  if ( !LineClip(&clip, &x0, &y0, &x1, &y1 ))
  {
    u32 efl;
    u32 *ring, write;

    efl = safe_cli();

#if R300_PIO

     R5xxFIFOWait(6);

     OUTREG(R5XX_DP_GUI_MASTER_CNTL,
            rhd.gui_control             |
            R5XX_GMC_BRUSH_SOLID_COLOR  |
            R5XX_GMC_SRC_DATATYPE_COLOR |
            R5XX_GMC_CLR_CMP_CNTL_DIS   |
            R5XX_GMC_WR_MSK_DIS         |
            R5XX_ROP3_P
            );

     OUTREG(R5XX_DST_LINE_PATCOUNT, 0x55 << R5XX_BRES_CNTL_SHIFT);

     OUTREG(R5XX_DP_BRUSH_FRGD_CLR, draw->color);
     OUTREG(R5XX_DST_PITCH_OFFSET, rhd.dst_pitch_offset);

     OUTREG(R5XX_DST_LINE_START,(y0<<16)|x0);
     OUTREG(R5XX_DST_LINE_END,(y1<<16)|x1);

#else

     BEGIN_RING();
         OUT_RING(CP_PACKET3(RADEON_CNTL_PAINT_POLYLINE, 4));
         OUT_RING(RADEON_GMC_DST_PITCH_OFFSET_CNTL  |
                  RADEON_GMC_BRUSH_SOLID_COLOR      |
                  RADEON_GMC_DST_32BPP              |
                  RADEON_GMC_SRC_DATATYPE_COLOR     |
                  R5XX_GMC_CLR_CMP_CNTL_DIS         |
                  R5XX_GMC_WR_MSK_DIS               |
                  R5XX_ROP3_P);

         OUT_RING(rhd.dst_pitch_offset);
         OUT_RING(draw->color);
         OUT_RING((y0<<16)|x0);
         OUT_RING((y1<<16)|x1);
     COMMIT_RING();

#endif
     safe_sti(efl);

  };
  return ERR_OK;
}
#endif

int CreatePixmap(pixmap_t *io)
{
     local_pixmap_t *pixmap;

     unsigned pitch;
     size_t size;

     void *local;

     if( (io->width == 0) || (io->width > 2048)||
         (io->height == 0)|| (io->height > 2048))
     {
        dbgprintf("Invalid pixmap size w:%d h:%d\n", io->width,io->height);
        return ERR_PARAM;
     };

     pitch = ((io->width+15)&~15)*4;
     size = pitch*io->height;

     dbgprintf("pitch = %d\n", pitch);

     local = rhd_mem_alloc(&rhd,RHD_MEM_FB,size) ;
     if ( !local)
     {
        dbgprintf("Not enough memory for pixmap\n");
        return ERR_PARAM;
     };

     pixmap = malloc(sizeof(local_pixmap_t));
     if(!pixmap)
     {
        rhd_mem_free(&rhd, RHD_MEM_FB,local);
        return ERR_PARAM;
     }
     else
     {
        void *mapped;

        size = (size+4095) & ~ 4095;

        if (mapped = UserAlloc(size))
        {
           CommitPages(mapped, ((u32_t)local+rhd.PhisBase)|7|(1<<9), size);

           io->mapped = mapped;
           io->pitch   = pitch;
           io->handle  = (u32_t)pixmap;

           pixmap->width   = io->width;
           pixmap->height  = io->height;
           pixmap->format  = PICT_a8r8g8b8;
           pixmap->flags   = io->flags;
           pixmap->pitch   = pitch;
           pixmap->mapped  = mapped;
           pixmap->pitch_offset =  ((pitch/64)<<22)| (((u32_t)local+rhd.FbIntAddress)>>10);
           pixmap->local   = local;

           dbgprintf("pixmap.pitch_offset: %x\n", pixmap->pitch_offset);
           dbgprintf("width: %d height: %d\n",pixmap->width,pixmap->height );
           dbgprintf("map at %x\n", pixmap->mapped);

           return ERR_OK;
        };
        rhd_mem_free(&rhd, RHD_MEM_FB,local);
        free(pixmap);
     };
     return ERR_PARAM;
};


int DestroyPixmap( pixmap_t *io )
{
     local_pixmap_t *pixmap;
     size_t size;

     dbgprintf("Destroy pixmap %x\n", io->handle);

     if(io->handle == -1)
        return ERR_PARAM;
     else
        pixmap = (local_pixmap_t*)io->handle;

     size = (pixmap->pitch*pixmap->height+4095) & ~ 4095;

     UnmapPages(pixmap->mapped, size);
     UserFree(pixmap->mapped);

     rhd_mem_free(&rhd,RHD_MEM_FB,pixmap->local);
     free(pixmap);

     io->format  = 0;
     io->pitch   = 0;
     io->mapped  = NULL;
     io->handle  = 0;

     return ERR_OK;
};

#if 0

int LockPixmap(userpixmap_t *io)
{
   pixmap_t *pixmap;
   size_t    size;
   void     *usermap;

   dbgprintf("Lock pixmap %x\n", io->pixmap);

   if(io->pixmap == (pixmap_t*)-1)
     return ERR_PARAM;
   else
     pixmap = io->pixmap;

   if( (pixmap->flags & 1) == PX_LOCK )
     return ERR_PARAM;

   size = (pixmap->pitch*pixmap->width+4095) & ~ 4095;
   if (usermap = UserAlloc(size))
   {
     CommitPages(usermap, ((u32_t)pixmap->raw+rhd.PhisBase)|7|(1<<9), size);
     pixmap->flags |= PX_LOCK;
     pixmap->usermap = usermap;
     io->usermap = usermap;
     io->pitch   = pixmap->pitch;
     dbgprintf("map at %x\n", io->usermap);

     return ERR_OK;
   }
   else
     return ERR_PARAM;
};

int UnlockPixmap(userpixmap_t *io)
{
  pixmap_t *pixmap;
  size_t    size;

  dbgprintf("Unlock pixmap %x\n", io->pixmap);

  if(io->pixmap == (pixmap_t*)-1)
    return ERR_PARAM;
  else
    pixmap = io->pixmap;

  if( (pixmap->flags & 1) != PX_LOCK )
    return ERR_PARAM;

/*   Sanity checks  */

  if( (pixmap->usermap == 0)||
      ((u32_t)pixmap->usermap >= 0x80000000) ||
      ((u32_t)pixmap->usermap & 4095)
    )
    return ERR_PARAM;

  size = (pixmap->pitch*pixmap->width+4095) & ~ 4095;

  UnmapPages(pixmap->usermap, size);
  UserFree(pixmap->usermap);
  pixmap->usermap =  NULL;
  pixmap->flags  &= ~PX_LOCK;
  io->usermap     =  NULL;
  io->pitch       =  0;

  return ERR_OK;
};

#endif

int PixBlit(pixblit_t *blit)
{
     clip_t src_clip, dst_clip;

     local_pixmap_t *srcpixmap;
     local_pixmap_t *dstpixmap;

     //dbgprintf("Pixblit src: %x dst: %x\n",blit->srcpix, blit->dstpix);

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

        u32_t ifl = safe_cli();

#if R300_PIO

        R5xxFIFOWait(7);

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
        OUTREG(R5XX_SRC_PITCH_OFFSET, srcpixmap->pitch_offset);

        OUTREG(R5XX_SRC_Y_X,(blit->src_y<<16)|blit->src_x);
        OUTREG(R5XX_DST_Y_X,(blit->dst_y<<16)|blit->dst_x);
        OUTREG(R5XX_DST_HEIGHT_WIDTH,(blit->h<<16)|blit->w);

#else

        BEGIN_RING();
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

          OUT_RING(srcpixmap->pitch_offset);
          OUT_RING(dstpixmap->pitch_offset);

          OUT_RING((blit->src_x<<16)|blit->src_y);
          OUT_RING((blit->dst_x<<16)|blit->dst_y);
          OUT_RING((blit->w<<16)|blit->h);
        COMMIT_RING();

#endif
       safe_sti(ifl);
     };
     return ERR_OK;
};


int TransBlit(pixblit_t *blit)
{
     clip_t src_clip, dst_clip;

     local_pixmap_t *srcpixmap;
     local_pixmap_t *dstpixmap;

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

        u32_t ifl = safe_cli();

#if R300_PIO

        R5xxFIFOWait(10);

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

        OUTREG(R5XX_CLR_CMP_CLR_SRC, 0xFF000000);
        OUTREG(R5XX_CLR_CMP_MASK, R5XX_CLR_CMP_MSK);
        OUTREG(R5XX_CLR_CMP_CNTL, R5XX_SRC_CMP_EQ_COLOR | R5XX_CLR_CMP_SRC_SOURCE);

        OUTREG(R5XX_DST_PITCH_OFFSET, dstpixmap->pitch_offset);
        OUTREG(R5XX_SRC_PITCH_OFFSET, srcpixmap->pitch_offset);

        OUTREG(R5XX_SRC_Y_X,(blit->src_y<<16)|blit->src_x);
        OUTREG(R5XX_DST_Y_X,(blit->dst_y<<16)|blit->dst_x);
        OUTREG(R5XX_DST_HEIGHT_WIDTH,(blit->h<<16)|blit->w);

#else

        BEGIN_RING();
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

          OUT_RING(srcpixmap->pitch_offset);
          OUT_RING(dstpixmap->pitch_offset);

          OUT_RING(R5XX_CLR_CMP_SRC_SOURCE | R5XX_SRC_CMP_EQ_COLOR);
          OUT_RING(0xFF000000);
          OUT_RING(0xFF000000);

          OUT_RING((blit->src_x<<16)|blit->src_y);
          OUT_RING((blit->dst_x<<16)|blit->dst_y);
          OUT_RING((blit->w<<16)|blit->h);

        COMMIT_RING();

#endif

       safe_sti(ifl);
     };
     return ERR_OK;
}


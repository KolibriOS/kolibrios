
#define BRUSH_MONO   (0<<4)


int DrawRect(draw_t* draw)
{
  int x0, y0, x1, y1;

  x0 = draw->x;
  y0 = draw->y;

  x1 = x0+draw->w-1;
  y1 = y0+draw->h-1;

//  dbgprintf("draw rect x0:%d, y0:%d, x1:%d, y1:%d, color: %x\n",
//             x0, y0, x1, y1, draw->color);

  if( ! BlockClip( &x0, &y0, &x1, &y1))
  {
     u32 *ring, write;
     int w, h;
     u32 ifl;

     w = x1-x0+1;
     h = y1-y0+1;

     ifl = safe_cli();


#if R300_PIO

     R5xxFIFOWait(7);

     OUTREG(R5XX_DP_GUI_MASTER_CNTL, rhd.gui_control | R5XX_ROP3_P |
                                     R5XX_GMC_BRUSH_SOLID_COLOR |
                                     R5XX_GMC_SRC_DATATYPE_COLOR);

     OUTREG(R5XX_DP_BRUSH_FRGD_CLR, draw->color);
     OUTREG(R5XX_DP_WRITE_MASK, 0xFFFFFFFF);
     OUTREG(R5XX_DP_CNTL, R5XX_DST_X_LEFT_TO_RIGHT | R5XX_DST_Y_TOP_TO_BOTTOM);
     OUTREG(R5XX_DST_PITCH_OFFSET, rhd.dst_pitch_offset);
     OUTREG(R5XX_DST_Y_X,(y0<<16)|x0);
     OUTREG( R5XX_DST_WIDTH_HEIGHT,(w<<16)|h);

#else

     BEGIN_RING();
       OUT_RING(CP_PACKET3(RADEON_CNTL_PAINT_MULTI, 4));


       OUT_RING(RADEON_GMC_DST_PITCH_OFFSET_CNTL  |
                RADEON_GMC_BRUSH_SOLID_COLOR      |
                RADEON_GMC_DST_32BPP              |
                RADEON_GMC_SRC_DATATYPE_COLOR     |
                (1 << 28)+(1 << 30) | R5XX_ROP3_P);

       OUT_RING(rhd.dst_pitch_offset);
       OUT_RING(draw->color);
       OUT_RING((y0<<16)|x0);
       OUT_RING((w<<16)|h);
     COMMIT_RING();

#endif

     safe_sti(ifl);
  } ;
  return 0;
}

int FillRect(fill_t *fill)
{
  int x0, y0, x1, y1;

  x0 = fill->x;
  y0 = fill->y;

  x1 = x0+fill->w-1;
  y1 = y0+fill->h-1;

//  dbgprintf("fill rect x0:%d, y0:%d, x1:%d, y1:%d\n",
//             x0, y0, x1, y1);

  if( ! BlockClip( &x0, &y0, &x1, &y1))
  {
     u32 *ring, write;
     u32 ifl;

     ifl = safe_cli();

#if R300_PIO

#else

     BEGIN_RING();
       OUT_RING(CP_PACKET3(RADEON_CNTL_PAINT, 7));
       OUT_RING(RADEON_GMC_DST_PITCH_OFFSET_CNTL    |
                BRUSH_MONO                          |
                RADEON_GMC_DST_32BPP                |
                RADEON_GMC_SRC_DATATYPE_COLOR       |
                (1 << 28)+(1 << 30) | R5XX_ROP3_P);

         OUT_RING(rhd.dst_pitch_offset);
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
  return 0;
}

int Blit(blit_t *blit)
{
  int x0, y0, x1, y1;

  x0 = blit->src_x;
  y0 = blit->src_y;

  x1 = x0+blit->w-1;
  y1 = y0+blit->h-1;


  if( ! BlockClip( &x0, &y0, &x1, &y1))
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
  return 0;
}

int Line2P(line2p_t *draw)
{

  int x0, y0, x1, y1;

  x0 = draw->x0;
  y0 = draw->y0;

  x1 = draw->x1;
  y1 = draw->y1;

  if ( !LineClip( &x0, &y0, &x1, &y1 ))
  {
    u32 ifl;
    u32 *ring, write;

    ifl = safe_cli();

#if R300_PIO

     R5xxFIFOWait(7);

     OUTREG(R5XX_DP_GUI_MASTER_CNTL, rhd.gui_control | R5XX_ROP3_P |
                                     R5XX_GMC_BRUSH_SOLID_COLOR |
                                     R5XX_GMC_SRC_DATATYPE_COLOR);

     OUTREG(R5XX_DST_LINE_PATCOUNT, 0x55 << R5XX_BRES_CNTL_SHIFT);

     OUTREG(R5XX_DP_BRUSH_FRGD_CLR, draw->color);
     OUTREG(R5XX_DP_WRITE_MASK, 0xFFFFFFFF);
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
                  (1 << 28)+(1 << 30) | R5XX_ROP3_P);

         OUT_RING(rhd.dst_pitch_offset);
         OUT_RING(draw->color);
         OUT_RING((y0<<16)|x0);
         OUT_RING((y1<<16)|x1);
     COMMIT_RING();

#endif
     safe_sti(ifl);

  };
  return 0;
}


int CreatePixmap(new_pixmap_t *io)
{

  pixmap_t *pixmap;

  u32_t pitch;
  void *raw;

  if( (io->width == 0) || (io->width > 2048)||
      (io->height == 0)|| (io->height > 2048))
  {
     dbgprintf("Invalid pixmap size w:%d h:%d\n", io->width,io->height);
     return 0;

  };

  pitch = ((io->width+15)&~15)*4;
  dbgprintf("pitch = %d\n", pitch);

  raw = rhd_mem_alloc(&rhd,RHD_MEM_FB,pitch*io->height) ;
  if (! raw)
  {
    dbgprintf("Not enough memory for pixmap\n");
    return 0;
  };

  pixmap = malloc(sizeof(pixmap_t));
  if(!pixmap)
  {
    rhd_mem_free(&rhd, RHD_MEM_FB,raw);
    return 0;
  }
  else
  {
    io->pixmap = (u32_t)pixmap;
    io->format = PICT_a8r8g8b8;
    io->pitch = pitch;

    pixmap->width  = io->width;
    pixmap->height = io->height;
    pixmap->format = PICT_a8r8g8b8;
    pixmap->pitch = pitch;
    pixmap->offset = (u32_t)raw-rhd.FbBase+rhd.FbIntAddress;
    pixmap->pitch_offset =  ((pitch/64)<<22)| (pixmap->offset>>10);
    pixmap->raw = raw;

    dbgprintf("pixmap.pitch_offset: %x\n", pixmap->pitch_offset);
    dbgprintf("width: %d height: %d\n",pixmap->width,pixmap->height );
    dbgprintf("pixmap.offset: %x\n", pixmap->offset);

  }
  return 1;
}



int PixBlit(pixblit_t *blit)
{

     u32 *ring, write;
     int w, h;
     u32 ifl;
     int x0, y0;

     pixmap_t *srcpixmap;
     pixmap_t *dstpixmap;

     dbgprintf("Pixblit src: %x dst: %x\n",blit->srcpix, blit->dstpix);

     dstpixmap = (blit->dstpix == (void*)-1) ? &scr_pixmap : blit->dstpix ;
     srcpixmap = (blit->srcpix == (void*)-1) ? &scr_pixmap : blit->srcpix ;

     dbgprintf("srcpixmap: %x dstpixmap: %x\n",srcpixmap, dstpixmap);

     dbgprintf("dst.width: %d dst.height: %d\n", dstpixmap->width,dstpixmap->height);
     dbgprintf("src.width: %d src.height: %d\n", srcpixmap->width,srcpixmap->height);
     dbgprintf("srcpitch: %x dstpitch: %x\n",
                srcpixmap->pitch_offset,dstpixmap->pitch_offset);

     ifl = safe_cli();


     BEGIN_RING();
       OUT_RING(CP_PACKET3(RADEON_CNTL_BITBLT, 5));

       OUT_RING(RADEON_GMC_SRC_PITCH_OFFSET_CNTL  |
                RADEON_GMC_DST_PITCH_OFFSET_CNTL  |
                RADEON_GMC_BRUSH_NONE             |
                RADEON_GMC_DST_32BPP              |
                RADEON_GMC_SRC_DATATYPE_COLOR     |
                RADEON_DP_SRC_SOURCE_MEMORY       |
                (1 << 28)+(1 << 30) | R5XX_ROP3_S);

       OUT_RING(srcpixmap->pitch_offset);
       OUT_RING(dstpixmap->pitch_offset);

  //   x0 = blit->src_x;
  //   y0 = blit->src_y;
  //   w =  blit->w;
  //   h =  blit->h;

       OUT_RING((blit->src_x<<16)|blit->src_y);
       OUT_RING((blit->dst_x<<16)|blit->dst_y);
       OUT_RING((blit->w<<16)|blit->h);
     COMMIT_RING();

     safe_sti(ifl);
  return 0;
}

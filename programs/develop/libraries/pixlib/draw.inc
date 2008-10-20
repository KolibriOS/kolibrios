
#include <mmintrin.h>


static void HLine(char *addr,int width, color_t color)
{
     __m64 dst_color;

     dst_color = _mm_cvtsi32_si64(color);
     dst_color = _mm_unpacklo_pi32(dst_color, dst_color);

     while (width >= 8)
     {
        __asm__ __volatile__ (
        "movq %[clr],   (%0)\n\t"
        "movq %[clr],  8(%0)\n\t"
        "movq %[clr], 16(%0)\n\t"
        "movq %[clr], 24(%0)\n\t"
        :: "r" (addr), [clr] "y" (dst_color));
        addr += 32;
        width -= 8;
     }
     if (width >= 4)
     {
        __asm__ __volatile__ (
        "movq %[clr],   (%0)\n\t"
        "movq %[clr],  8(%0)\n\t"
        :: "r" (addr), [clr] "y" (dst_color));
        addr += 16;
        width -= 4;
     }
     if (width >= 2)
     {
        __asm__ __volatile__ (
        "movq %[clr],  (%0)\n\t"
        :: "r" (addr), [clr] "y" (dst_color));
        addr += 8;
        width -= 2;
     }
     if ( width )
        __asm__ __volatile__ (
        "movd %[clr],  (%0)\n\t"
        :: "r" (addr), [clr] "y" (dst_color));

     _mm_empty();
}

static void pxDraw(char *dst_addr, int pitch, int w, int h, color_t dst_color)
{
     __m64 color;

     color = _mm_cvtsi32_si64(dst_color);
     color = _mm_unpacklo_pi32(color, color);

     while(h--)
     {
        char *tmp_dst =dst_addr;
        int width = w;
        dst_addr += pitch;

        while(width >= 8)
        {
           __asm__ __volatile__
           ("movq %[clr],   (%0)\n\t"
            "movq %[clr],  8(%0)\n\t"
            "movq %[clr], 16(%0)\n\t"
            "movq %[clr], 24(%0)\n\t"
            :: "r" (tmp_dst), [clr] "y" (color));
           tmp_dst += 32;
           width -= 8;
        };
        if(width >= 4)
        {
           __asm__ __volatile__
           ("movq %[clr],  (%0)\n\t"
            "movq %[clr], 8(%0)\n\t"
            :: "r" (tmp_dst), [clr] "y" (color));
           tmp_dst += 16;
           width -= 4;
        };

        if (width >= 2)
        {
           __asm__ __volatile__
           ("movq %[clr],  (%0)\n\t"
            :: "r" (tmp_dst), [clr] "y" (color));
           tmp_dst += 8;
           width -= 2;
        };
        if(width)
           __asm__ __volatile__
           ("movd %[clr], (%0)\n\t"
            :: "r" (tmp_dst), [clr] "y" (color));
     };
     _mm_empty();
};

int ClearPixmap(pixmap_t *pixmap, color_t color)
{
     if( (srv_hw2d != 0) &&
         ( ((int)pixmap == -1) ||
           ( (pixmap->flags & PX_MEM_MASK)==PX_MEM_LOCAL) ) )
     {
       ioctl_t io;

       if((int)pixmap != -1)
          pixmap = (pixmap_t*)pixmap->handle;

       io.handle   = srv_hw2d;
       io.io_code  = PX_CLEAR;
       io.input    = &pixmap;
       io.inp_size = 2;
       io.output   = NULL;
       io.out_size = 0;

       return call_service(&io);
     }

     pixmap = (pixmap == (void*)-1) ? &scrn_pixmap : pixmap ;

     pxDraw(pixmap->mapped, pixmap->pitch,
            pixmap->width, pixmap->height, color);
     return ERR_OK;
};

int Line(pixmap_t *pixmap, int x0, int y0, int x1, int y1, color_t color)
{

     clip_t clip;

     if( (srv_hw2d != 0) &&
         ( (pixmap == (void*)-1) ||
           ( (pixmap->flags & PX_MEM_MASK)==PX_MEM_LOCAL) ) )
     {
       ioctl_t io;

       if((int)pixmap != -1)
          pixmap = (pixmap_t*)pixmap->handle;

       io.handle   = srv_hw2d;
       io.io_code  = PX_LINE;
       io.input    = &pixmap;
       io.inp_size = 6;
       io.output   = NULL;
       io.out_size = 0;

       return call_service(&io);
     }

     pixmap = (pixmap == (void*)-1) ? &scrn_pixmap : pixmap ;

     clip.xmin = 0;
     clip.ymin = 0;
     clip.xmax = pixmap->width-1;
     clip.ymax = pixmap->height-1;

     if ( !LineClip( &clip, &x0, &y0, &x1, &y1 ))
     {
        int dx, dy;
        int sx, sy;
        int e, e1, e2, e3;

        char *addr;

        dx = x1 - x0;
        dy = y1 - y0;

        if( dx || dy)
        {
           if( dy == 0 )
           {
              if (dx < 0)
              {
                 dx = -dx;
                 x0 =  x1;
              };

              addr = &((char*)(pixmap->mapped))[pixmap->pitch*y0 + x0*4];
              HLine(addr, dx, color);

              return ERR_OK;
           };
           if( dx == 0 )
           {
              if (dy < 0)
              {
                 dy = -dy;
                 y0 =  y1;
              };
              addr = &((char*)(pixmap->mapped))[pixmap->pitch*y0 + x0*4];

              while ( dy-- )
              {
                 *(color_t*)addr = color;
                 addr += pixmap->pitch;
              }
              return ERR_OK;
           }

           sx = 4;
           if ( dx < 0 )
           {
              dx = -dx;
              sx = -sx;
           }
           sy = pixmap->pitch;
           if ( dy < 0 )
           {
              dy = -dy;
              sy = -sy;
           };

           addr = &((char*)(pixmap->mapped))[pixmap->pitch*y0 + x0*4];

           if (dx > dy)
           {
              e1 = dy << 1;
              e2 = e1 - (dx << 1);
              e  = e1 - dx;

              e3 = e2 - e1;
              e  = e - e1;


              while (dx--)
              {
                 *(color_t*)addr = color;
                 addr += sx;
                 e += e1;
                 if (e >= 0)
                 {
                    addr += sy;
                    e += e3;
                 }
              }
           }
           else
           {
              e1 = dx << 1;
              e2 = e1 - (dy << 1);
              e  = e1 - dy;
              e3 = e2 - e1;
              e  = e - e1;

              while (dy--)
              {
                 *(color_t*)addr = color;
                 addr += sy;
                 e += e1;
                 if (e >= 0)
                 {
                    addr += sx;
                    e += e3;
                 }
              }
           }
        }    /*  ( dx || dy ) */
     }
     return ERR_OK;
}

int DrawRect(pixmap_t *pixmap, int xorg, int yorg,
               int width, int height,
               color_t dst_color, color_t border)
{

     if( ( width <= 0 ) || ( height<=0 ) )
        return ERR_PARAM;

/*   if "hardware acceleration present" and
     "destinastion is primary screen or local videomemory"
*/

     if( (srv_hw2d != 0) &&
         ( (pixmap == (void*)-1) ||
           ( (pixmap->flags & PX_MEM_MASK)==PX_MEM_LOCAL) ) )
     {
       ioctl_t io;

       if((int)pixmap != -1)
          pixmap = (pixmap_t*)pixmap->handle;

       io.handle   = srv_hw2d;
       io.io_code  = PX_DRAW_RECT;
       io.input    = &pixmap;
       io.inp_size = 7;
       io.output   = NULL;
       io.out_size = 0;

       return call_service(&io);
     }
     else
/*     no acceleration or destination in system memory  */
     {
        clip_t clip;
        int x0, y0, x1, y1, xend, yend;

        pixmap = (pixmap == (void*)-1) ? &scrn_pixmap : pixmap ;

        x0 = xorg;
        y0 = yorg;

        xend = x1 = x0 + width - 1;
        yend = y1 = y0 + height - 1;

        clip.xmin = 0;
        clip.ymin = 0;
        clip.xmax = pixmap->width-1;
        clip.ymax = pixmap->height-1;

        if( ! BlockClip( &clip, &x0, &y0, &x1, &y1))
        {
           int w, h;

           char *dst_addr;

           w = x1 - x0 + 1;
           h = y1 - y0 + 1;

           if( (dst_color == border) ||
               ((border & 0xFF000000)==0))
           {
              dst_addr  = &((char*)(pixmap->mapped))[pixmap->pitch*y0 + x0*4];
              pxDraw(dst_addr, pixmap->pitch, w, h, dst_color);
           }
           else
           {
              if( y0 == yorg)
              {
                 dst_addr  = &((char*)(pixmap->mapped))[pixmap->pitch*y0 + x0*4];
                 HLine(dst_addr, w, border);
                 y0++;
                 h--;
              }
              if( y1 == yend )
              {
                 dst_addr  = &((char*)(pixmap->mapped))[pixmap->pitch*y1 + x0*4];
                 HLine(dst_addr, w, border);
                 h--;
              }
              if( (h > 0) && (x0 == xorg))
              {
                 int dy = h;

                 dst_addr  = &((char*)(pixmap->mapped))[pixmap->pitch*y0 + x0*4];

                 while ( dy-- )
                 {
                    *(color_t*)dst_addr = border;
                    dst_addr += pixmap->pitch;
                 }
                 x0++;
                 w--;
              }
              if( (h > 0) && (x1 == xend))
              {
                 int dy = h;

                 dst_addr  = &((char*)(pixmap->mapped))[pixmap->pitch*y0 + x1*4];

                 while ( dy-- )
                 {
                    *(color_t*)dst_addr = border;
                    dst_addr += pixmap->pitch;
                 }
                 w--;
              }
              if( (w > 0) && (h > 0) )
              {
                 dst_addr  = &((char*)(pixmap->mapped))[pixmap->pitch*y0 + x0*4];
                 pxDraw(dst_addr, pixmap->pitch, w, h, dst_color);
              }
           }
        };
     };
     return ERR_OK;
};

int FillRect(pixmap_t *pixmap, int xorg, int yorg,
               int width, int height,
               brush_t *dst_brush, color_t border)
{
     if( ( width <= 0 ) || ( height<=0 ) )
        return ERR_PARAM;

/*   if "hardware acceleration present" and
     "destinastion is primary screen or local videomemory"
*/

     if( (srv_hw2d != 0) &&
         ( (pixmap == (void*)-1) ||
           ( (pixmap->flags & PX_MEM_MASK)==PX_MEM_LOCAL) ) )
     {
        fill_t fill;
        ioctl_t io;

        fill.dstpix = ((int)pixmap == -1) ? (pixmap_t*)-1 : (pixmap_t*)pixmap->handle;
        fill.x = xorg;
        fill.y = yorg;
        fill.w = width;
        fill.h = height;
        fill.bkcolor = dst_brush->bkcolor;
        fill.fcolor  = dst_brush->fcolor;
        fill.bmp0 = dst_brush->bmp[0];
        fill.bmp1 = dst_brush->bmp[1];
        fill.border = border;

        io.handle   = srv_hw2d;
        io.io_code  = PX_FILL_RECT;
        io.input    = &fill;
        io.inp_size = 10;
        io.output   = NULL;
        io.out_size = 0;

        return call_service(&io);
     }
     else
/*      no acceleration or destination in system memory  */
     {
        clip_t clip;
        int x0, y0, x1, y1, xend, yend;

        pixmap = (pixmap == (void*)-1) ? &scrn_pixmap : pixmap ;

        x0 = xorg;
        y0 = yorg;
        x1 = xend = x0 + width - 1;
        y1 = yend = y0 + height - 1;

        clip.xmin = 0;
        clip.ymin = 0;
        clip.xmax = pixmap->width-1;
        clip.ymax = pixmap->height-1;

        if( ! BlockClip( &clip, &x0, &y0, &x1, &y1))
        {
           int w, h, bh, bm_y;

           __m64 clr_bb, clr_bf, clr_fb, clr_ff;
           char *dst_addr;

           clr_bb = _mm_cvtsi32_si64(dst_brush->bkcolor);
           clr_ff = _mm_cvtsi32_si64(dst_brush->fcolor);

           clr_bb = _mm_unpacklo_pi32(clr_bb, clr_bb);
           clr_ff = _mm_unpacklo_pi32(clr_ff, clr_ff);

           clr_bf = _mm_unpacklo_pi32(clr_ff, clr_bb);
           clr_fb = _mm_unpacklo_pi32(clr_bb, clr_ff);

           w = x1 -x0 + 1;
           bh = h = y1 -y0 + 1;

           dst_addr  = & ((char*)(pixmap->mapped))[pixmap->pitch*y0+x0*4];

           bm_y = y0 & 7;

           while(h--)
           {
              u8_t mask = dst_brush->bits[bm_y];

              char *tmp_dst = dst_addr;
              int width = w;
              dst_addr += pixmap->pitch;
              bm_y = (bm_y+1) & 7;

              while(width>=2)
              {
                 __asm__ __volatile__
                 ("rolb $2, %0 \n\t"
                  :"+g" (mask):"g"(mask)
                  :"cc");

                 switch( mask & 3 )
                 {
                    case 0:
                       __asm__ __volatile__
                       ("movq %[clr], (%0)\n\t" :: "r" (tmp_dst), [clr] "y" (clr_bb));
                       break;

                    case 1:
                       __asm__ __volatile__
                       ("movq %[clr], (%0)\n\t" :: "r" (tmp_dst), [clr] "y" (clr_fb));
                       break;

                    case 2:
                       __asm__ __volatile__
                       ("movq %[clr], (%0)\n\t" :: "r" (tmp_dst), [clr] "y" (clr_bf));
                       break;

                    case 3:
                       __asm__ __volatile__
                       ("movq %[clr], (%0)\n\t" :: "r" (tmp_dst), [clr] "y" (clr_ff));
                       break;
                 }
                 width   -=2;
                 tmp_dst +=8;
              }
              if( width )
                 if( mask & 1 )
                    __asm__ __volatile__
                    ("movd %[clr], (%0)\n\t" :: "r" (tmp_dst), [clr] "y" (clr_ff));
                 else
                    __asm__ __volatile__
                    ("movd %[clr], (%0)\n\t" :: "r" (tmp_dst), [clr] "y" (clr_bb));
           };

           if( (border & 0xFF000000) != 0)
           {
              if( y0 == yorg)
              {
                 dst_addr  = &((char*)(pixmap->mapped))[pixmap->pitch*y0 + x0*4];
                 HLine(dst_addr, w, border);
                 y0++;
                 bh--;
              }
              if( y1 == yend )
              {
                 dst_addr  = &((char*)(pixmap->mapped))[pixmap->pitch*y1 + x0*4];
                 HLine(dst_addr, w, border);
                 bh--;
              }
              if( (bh > 0) && (x0 == xorg))
              {
                 int dy = bh;

                 dst_addr  = &((char*)(pixmap->mapped))[pixmap->pitch*y0 + x0*4];

                 while ( dy-- )
                 {
                    *(color_t*)dst_addr = border;
                    dst_addr += pixmap->pitch;
                 }
              }
              if( (bh > 0) && (x1 == xend))
              {
                 int dy = bh;

                 dst_addr  = &((char*)(pixmap->mapped))[pixmap->pitch*y0 + x1*4];

                 while ( dy-- )
                 {
                    *(color_t*)dst_addr = border;
                    dst_addr += pixmap->pitch;
                 }
              }
           };

           _mm_empty();
        };
     };
     return ERR_OK;
};


int Blit(pixmap_t *dst_pixmap, int dst_x, int dst_y,
         pixmap_t *src_pixmap, int src_x, int src_y,
         int width, int height)
{

     clip_t src_clip, dst_clip;

     if( ( width <= 0 ) || ( height<=0 ) )
        return ERR_PARAM;

/*   if "hardware acceleration present" and
     "destinastion is primary screen or local videomemory"  and
     "source is primary screen or local videomemory"
*/
     if( (srv_hw2d != 0) &&
         ( (dst_pixmap == (void*)-1) ||
           ( (dst_pixmap->flags & PX_MEM_MASK)==PX_MEM_LOCAL) )  &&
         ( (src_pixmap == (void*)-1) ||
           ( (src_pixmap->flags & PX_MEM_MASK)==PX_MEM_LOCAL) ) )
     {
        ioctl_t io;
        pxblit_t *blit = (pxblit_t*)&dst_pixmap;

        if((int)dst_pixmap != -1)
        blit->dst_pixmap = (pixmap_t*)dst_pixmap->handle;

        if((int)src_pixmap != -1)
           blit->src_pixmap = (pixmap_t*)src_pixmap->handle;

        io.handle   = srv_hw2d;
        io.io_code  = PX_BLIT;
        io.input    = blit;
        io.inp_size = 8;
        io.output   = NULL;
        io.out_size = 0;

        return call_service(&io);
     }

     dst_pixmap = (dst_pixmap == (void*)-1) ? &scrn_pixmap : dst_pixmap ;
     src_pixmap = (src_pixmap == (void*)-1) ? &scrn_pixmap : src_pixmap ;

     src_clip.xmin = 0;
     src_clip.ymin = 0;
     src_clip.xmax = src_pixmap->width-1;
     src_clip.ymax = src_pixmap->height-1;

     dst_clip.xmin = 0;
     dst_clip.ymin = 0;
     dst_clip.xmax = dst_pixmap->width-1;
     dst_clip.ymax = dst_pixmap->height-1;

     if( !blit_clip(&dst_clip, &dst_x, &dst_y,
                    &src_clip, &src_x, &src_y,
                    &width, &height) )
     {
        color_t *src_addr = &((color_t*)(src_pixmap->mapped))[src_pixmap->pitch*src_y/4+src_x];
        color_t *dst_addr = &((color_t*)(dst_pixmap->mapped))[dst_pixmap->pitch*dst_y/4+dst_x];

        while( height-- )
        {
           int w = width;
           color_t *tmp_src = src_addr;
           color_t *tmp_dst = dst_addr;

           src_addr += src_pixmap->pitch/4;
           dst_addr += dst_pixmap->pitch/4;

           while( w >= 8)
           {
              __asm__ __volatile__ (
              "movq     (%0),   %%mm0\n"
              "movq    8(%0),   %%mm1\n"
              "movq   16(%0),   %%mm2\n"
              "movq   24(%0),   %%mm3\n"
              "movq   %%mm0,    (%1)\n"
              "movq   %%mm1,   8(%1)\n"
              "movq   %%mm2,  16(%1)\n"
              "movq   %%mm3,  24(%1)\n"
               :: "r" (tmp_src), "r" (tmp_dst)
               : "memory", "%mm0", "%mm1", "%mm2", "%mm3");
              w -= 8;
              tmp_src += 8;
              tmp_dst += 8;
           };
           if( w >= 4 )
           {
              __asm__ __volatile__ (
              "movq     (%0),   %%mm0\n"
              "movq    8(%0),   %%mm1\n"
              "movq   %%mm0,    (%1)\n"
              "movq   %%mm1,   8(%1)\n"
              :: "r" (tmp_src), "r" (tmp_dst)
              : "memory", "%mm0", "%mm1");
              w -= 4;
              tmp_src += 4;
              tmp_dst += 4;
           };
           if( w >= 2 )
           {
              __asm__ __volatile__ (
              "movq     (%0),   %%mm0\n"
              "movq   %%mm0,    (%1)\n"
              :: "r" (tmp_src), "r" (tmp_dst)
              : "memory", "%mm0");
              w -= 2;
              tmp_src += 2;
              tmp_dst += 2;
           };
           if( w )
              *tmp_dst = *tmp_src;
        };
     };
     return ERR_OK;
};

int TransparentBlit(pixmap_t *dst_pixmap, int dst_x, int dst_y,
         pixmap_t *src_pixmap, int src_x, int src_y,
         int width, int height, color_t key)
{
     clip_t src_clip, dst_clip;

/*   if "hardware acceleration present" and
     "destinastion is primary screen or local videomemory"  and
     "source is primary screen or local videomemory"
*/

     if( (srv_hw2d != 0) &&
         ( (dst_pixmap == (void*)-1) ||
           ( (dst_pixmap->flags & PX_MEM_MASK)==PX_MEM_LOCAL) )  &&
         ( (src_pixmap == (void*)-1) ||
           ( (src_pixmap->flags & PX_MEM_MASK)==PX_MEM_LOCAL) ) )
     {
        ioctl_t io;
        pxblit_t *blit = (pxblit_t*)&dst_pixmap;

        if((int)dst_pixmap != -1)
           blit->dst_pixmap = (pixmap_t*)dst_pixmap->handle;

        if((int)src_pixmap != -1)
           blit->src_pixmap = (pixmap_t*)src_pixmap->handle;

        io.handle   = srv_hw2d;
        io.io_code  = PX_BLIT_TRANSPARENT;
        io.input    = blit;
        io.inp_size = 9;
        io.output   = NULL;
        io.out_size = 0;

        return call_service(&io);
     };

     src_clip.xmin = 0;
     src_clip.ymin = 0;
     src_clip.xmax = src_pixmap->width-1;
     src_clip.ymax = src_pixmap->height-1;

     dst_clip.xmin = 0;
     dst_clip.ymin = 0;
     dst_clip.xmax = dst_pixmap->width-1;
     dst_clip.ymax = dst_pixmap->height-1;


     if( !blit_clip(&dst_clip, &dst_x, &dst_y,
                    &src_clip, &src_x, &src_y,
                    &width, &height) )
     {
        __m64 clr_key;

        dst_pixmap = (dst_pixmap == (void*)-1) ? &scrn_pixmap : dst_pixmap ;
        src_pixmap = (src_pixmap == (void*)-1) ? &scrn_pixmap : src_pixmap ;

        color_t *src_addr = &((color_t*)(src_pixmap->mapped))[src_pixmap->pitch*src_y/4+src_x];
        color_t *dst_addr = &((color_t*)(dst_pixmap->mapped))[dst_pixmap->pitch*dst_y/4+dst_x];

        clr_key = _mm_cvtsi32_si64(key);
        clr_key = _mm_unpacklo_pi32(clr_key, clr_key);

        while( height-- )
        {
           int w = width;
           color_t *tmp_src = src_addr;
           color_t *tmp_dst = dst_addr;

           src_addr += src_pixmap->pitch/4;
           dst_addr += dst_pixmap->pitch/4;

           while( w >= 2)
           {
              __asm__ __volatile__ (
              "movq    %[clr_key], %%mm0      \n\t"
              "pcmpeqd %[src_clr], %%mm0      \n\t"
              "pand    %%mm0,      %[dst_clr] \n\t"
              "pandn   %[src_clr], %%mm0      \n\t"
              "por     %%mm0,      %[dst_clr] \n\t"
              "movq    %[dst_clr], (%0)"
              :: "r" (tmp_dst),
              [src_clr] "y" (*(__m64*)tmp_src),
              [dst_clr] "y" (*(__m64*)tmp_dst),
              [clr_key] "y" (clr_key)
              :"memory","mm0");

              w -= 2;
              tmp_src += 2;
              tmp_dst += 2;
           };
           if( w && (*tmp_src != key) )
              *tmp_dst = *tmp_src;
        };
     };
     return ERR_OK;
}



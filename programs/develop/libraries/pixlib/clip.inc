
#define CLIP_TOP        1
#define CLIP_BOTTOM     2
#define CLIP_RIGHT      4
#define CLIP_LEFT       8


static int _L1OutCode( clip_t *clip, int x, int y )
/*=================================

    Verify that a point is inside or outside the active viewport.   */
{
     int   flag;

     flag = 0;
     if( x < clip->xmin ) {
        flag |= CLIP_LEFT;
     } else if( x > clip->xmax ) {
        flag |= CLIP_RIGHT;
     }
     if( y < clip->ymin ) {
        flag |= CLIP_TOP;
     } else if( y > clip->ymax ) {
        flag |= CLIP_BOTTOM;
     }
     return( flag );
}


static void line_inter( int * x1, int* y1, int x2, int y2, int x )
/*===========================================================================

    Find the intersection of a line with a boundary of the viewport.
    (x1, y1) is outside and ( x2, y2 ) is inside the viewport.
    NOTE : the signs of denom and ( x - *x1 ) cancel out during division
           so make both of them positive before rounding.   */
{
     int numer;
     int denom;

     denom = abs( x2 - *x1 );
     numer = 2L * (long)( y2 - *y1 ) * abs( x - *x1 );
     if( numer > 0 ) {
        numer += denom;                     /* round to closest pixel   */
     } else {
        numer -= denom;
     }
     *y1 += numer / ( denom << 1 );
     *x1 = x;
}


int LineClip( clip_t *clip, int *x1, int *y1, int *x2, int *y2 )
/*=============================================================

    Clips the line with end points (x1,y1) and (x2,y2) to the active
    viewport using the Cohen-Sutherland clipping algorithm. Return the
    clipped coordinates and a decision drawing flag.    */
{
     int flag1;
     int flag2;

     flag1 = _L1OutCode( clip, *x1, *y1 );
     flag2 = _L1OutCode( clip, *x2, *y2 );
     for( ;; ) {
        if( flag1 & flag2 ) break;                  /* trivially outside    */
        if( flag1 == flag2 ) break;                 /* completely inside    */
        if( flag1 == 0 ) {                          /* first point inside   */
           if( flag2 & CLIP_TOP ) {
              line_inter( y2, x2, *y1, *x1, clip->ymin );
           } else if( flag2 & CLIP_BOTTOM ) {
              line_inter( y2, x2, *y1, *x1, clip->ymax );
           } else if( flag2 & CLIP_RIGHT ) {
              line_inter( x2, y2, *x1, *y1, clip->xmax );
           } else if( flag2 & CLIP_LEFT ) {
              line_inter( x2, y2, *x1, *y1, clip->xmin );
           }
           flag2 = _L1OutCode( clip, *x2, *y2 );
        } else {                                    /* second point inside  */
           if( flag1 & CLIP_TOP ) {
              line_inter( y1, x1, *y2, *x2, clip->ymin );
           } else if( flag1 & CLIP_BOTTOM ) {
              line_inter( y1, x1, *y2, *x2, clip->ymax );
           } else if( flag1 & CLIP_RIGHT ) {
              line_inter( x1, y1, *x2, *y2, clip->xmax );
           } else if( flag1 & CLIP_LEFT ) {
              line_inter( x1, y1, *x2, *y2, clip->xmin );
           }
           flag1 = _L1OutCode( clip, *x1, *y1 );
        }
     }
     return( flag1 & flag2 );
}


static void block_inter( clip_t *clip, int *x, int *y, int flag )
/*======================================================

    Find the intersection of a block with a boundary of the viewport.   */
{
     if( flag & CLIP_TOP ) {
        *y = clip->ymin;
     } else if( flag & CLIP_BOTTOM ) {
        *y = clip->ymax;
     } else if( flag & CLIP_RIGHT ) {
        *x = clip->xmax;
     } else if( flag & CLIP_LEFT ) {
        *x = clip->xmin;
     }
}


int BlockClip(clip_t *clip, int *x1, int *y1, int *x2, int* y2 )
/*==============================================================

    Clip a block with opposite corners (x1,y1) and (x2,y2) to the
    active viewport based on the Cohen-Sutherland algorithm for line
    clipping. Return the clipped coordinates and a decision drawing
    flag ( 0 draw : 1 don't draw ). */
{
     int  flag1;
     int  flag2;

     flag1 = _L1OutCode( clip, *x1, *y1 );
     flag2 = _L1OutCode( clip, *x2, *y2 );
     for( ;; ) {
        if( flag1 & flag2 ) break;                  /* trivially outside    */
        if( flag1 == flag2 ) break;                 /* completely inside    */
        if( flag1 == 0 ) {
           block_inter( clip, x2, y2, flag2 );
           flag2 = _L1OutCode( clip,  *x2, *y2 );
        } else {
           block_inter( clip, x1, y1, flag1 );
           flag1 = _L1OutCode( clip, *x1, *y1 );
        }
     }
     return( flag1 & flag2 );
}


int blit_clip(clip_t *dst_clip,int *dst_x,int *dst_y,
              clip_t *src_clip,int *src_x, int *src_y,
              int *w, int *h)
{
     int sx0, sy0, sx1, sy1;

     sx0 = *src_x;
     sy0 = *src_y;

     sx1 = sx0 + *w - 1;
     sy1 = sy0 + *h - 1;


     if( ! BlockClip( src_clip, &sx0, &sy0, &sx1, &sy1))
     {
        int dx0, dy0, dx1, dy1;

        dx0 = *dst_x + sx0 - *src_x;
        dy0 = *dst_y + sy0 - *src_y;

        dx1 = dx0 + sx1 - sx0;
        dy1 = dy0 + sy1 - sy0;

        if( ! BlockClip( dst_clip, &dx0, &dy0, &dx1, &dy1))
        {
           *w = dx1 - dx0 + 1;
           *h = dy1 - dy0 + 1;

           *src_x += dx0 - *dst_x;
           *src_y += dy0 - *dst_y;

           *dst_x = dx0;
           *dst_y = dx0;

           return 0;
        };
     };
     return 1;
};


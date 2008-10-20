
brush_t* CreateHatch(int hatch, color_t bkcolor, color_t fcolor)
{
  if (hatch < HATCH_MAX)
  {
    if( br_slab.available )
    {
      brush_t *brush;

      br_slab.available--;
      brush = (brush_t*)br_slab.nextavail;
      br_slab.nextavail = *(void**)brush;

      brush->bkcolor = bkcolor;
      brush->fcolor  = fcolor;

      brush->bmp[0] = hatches[hatch*2];
      brush->bmp[1] = hatches[hatch*2+1];
      return brush;
    }
    return NULL;
  }
  return NULL;
};

brush_t* CreateMonoBrush(color_t bkcolor, color_t fcolor,u32_t bmp0,u32_t bmp1)
{
     if( br_slab.available )
     {
       brush_t *brush;

       br_slab.available--;
       brush = (brush_t*)br_slab.nextavail;
       br_slab.nextavail = *(void**)brush;

       brush->bkcolor = bkcolor;
       brush->fcolor  = fcolor;

       brush->bmp[0] = bmp0;
       brush->bmp[1] = bmp1;
       return brush;
     }
     return NULL;
};

void DestroyBrush(brush_t *brush)
{
  *(void**)brush = br_slab.nextavail;
  br_slab.nextavail = brush;
  br_slab.available++;
};

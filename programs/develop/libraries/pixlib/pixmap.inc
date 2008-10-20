
static pixmap_t* alloc_pixmap()
{
     if( px_slab.available )
     {
        pixmap_t *pixmap;

        px_slab.available--;
        pixmap = (pixmap_t*)px_slab.nextavail;
        px_slab.nextavail = *(void**)pixmap;

        return pixmap;
     }
     return NULL;
};

static void  free_pixmap(pixmap_t *pixmap)
{
     *(void**)pixmap = px_slab.nextavail;
     px_slab.nextavail = pixmap;
     px_slab.available++;
};

pixmap_t* CreatePixmap(unsigned width, unsigned height, u32_t format, u32_t flags)
{
     pixmap_t *pixmap;

     if( (width  == 0) || ( width > 2048)||
         (height == 0) || (height > 2048)||
         (format != PICT_a8r8g8b8))
        return NULL;

     pixmap = alloc_pixmap();

     if( pixmap )
     {
        void *raw;
        int  pitch;

        pixmap->width   = width;
        pixmap->height  = height;
        pixmap->format  = format;
        pixmap->flags   = flags;

        if( (srv_hw2d != 0) &&
            ( (flags & PX_MEM_MASK)==PX_MEM_LOCAL) )
        {
           ioctl_t io;
           io.handle   = srv_hw2d;
           io.io_code  = PX_CREATE;
           io.input    = pixmap;
           io.inp_size = 7;
           io.output   = NULL;
           io.out_size = 0;

           if (call_service(&io)==ERR_OK)
              return pixmap;
           else
           {
              free_pixmap(pixmap) ;
              return NULL;
           }
        }
        else
     {
/*
   Only system memory
*/
        pixmap->flags &= ~PX_MEM_MASK;

        pitch = ((width+8)&~8)*4;
        raw = UserAlloc(pitch * height);

        if (! raw)
        {
           free_pixmap(pixmap);
           return NULL;
        };
        pixmap->pitch   = pitch;
        pixmap->mapped  = raw;
     };
     return pixmap;
  };
  return NULL;
};

int DestroyPixmap( pixmap_t *pixmap)
{
     int retval = ERR_OK;

     if(pixmap == (pixmap_t*)-1)
        return ERR_PARAM;

     if( (srv_hw2d != 0) &&
         ( (pixmap->flags & PX_MEM_MASK)==PX_MEM_LOCAL) )
     {
        int retval;

        ioctl_t io;
        io.handle   = srv_hw2d;
        io.io_code  = PX_DESTROY;
        io.input    = pixmap;
        io.inp_size = 7;
        io.output   = NULL;
        io.out_size = 0;

        retval = call_service(&io);
     }
     else
     {
        UserFree(pixmap->mapped);

        pixmap->pitch  = 0;
        pixmap->mapped = 0;
        pixmap->handle = 0;
     };
     free_pixmap(pixmap);

     return retval;
};

void* LockPixmap(pixmap_t *pixmap)
{
     if(pixmap == (pixmap_t*)-1)
        return scrn_pixmap.mapped;
     else
        return pixmap->mapped;
}

int UnlockPixmap(pixmap_t *pixmap)
{
     return ERR_OK;
};

int GetPixmapPitch(pixmap_t *pixmap)
{
     if(pixmap == (pixmap_t*)-1)
        return scrn_pixmap.pitch;
     else
        return pixmap->pitch;
};

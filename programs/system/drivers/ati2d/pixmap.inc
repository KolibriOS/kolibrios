
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
           CommitPages(mapped, ((u32_t)local+rhd.LinearAddr)|7|(1<<9), size);

           io->mapped = mapped;
           io->pitch   = pitch;
           io->handle  = (u32_t)pixmap;

           pixmap->width   = io->width;
           pixmap->height  = io->height;
           pixmap->format  = PICT_a8r8g8b8;
           pixmap->flags   = io->flags;
           pixmap->pitch   = pitch;
           pixmap->mapped  = mapped;
           pixmap->pitch_offset =  ((pitch/64)<<22)| (((u32_t)local+rhd.fbLocation)>>10);
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

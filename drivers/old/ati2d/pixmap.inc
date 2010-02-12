
int CreatePixmap(pixmap_t *io)
{
    local_pixmap_t *pixmap;

    unsigned pitch;
    size_t size;

    addr_t  mem_local = 0;
    addr_t  mem_dma   = 0;
    void   *mapped;

    if( (io->width == 0) || (io->width > 2048)||
        (io->height == 0)|| (io->height > 2048))
    {
        dbgprintf("Invalid pixmap size w:%d h:%d\n", io->width,io->height);
        return ERR_PARAM;
    };

    pixmap = malloc(sizeof(local_pixmap_t));

    if(!pixmap)
        return ERR_PARAM;

    pitch = ((io->width+15)&~15)*4;
    size = pitch*io->height;

    dbgprintf("pitch = %d\n", pitch);

    if( (io->flags & PX_MEM_MASK) == PX_MEM_LOCAL ) {
        mem_local = rhd_mem_alloc(&rhd,RHD_MEM_FB,size);
        mem_dma = mem_local + rhd.fbLocation;
    }
    else
        mem_local = mem_dma = AllocPages( size >> 12 );

    if ( !mem_local) {
        dbgprintf("Not enough memory for pixmap\n");
        free(pixmap);
        return ERR_PARAM;
    };

    pixmap->pitch_offset = ((pitch/64)<<22)| (mem_dma>>10);
    pixmap->local   = mem_dma;

    size = (size+4095) & ~ 4095;

    if (mapped = UserAlloc(size))
    {
        CommitPages(mapped, mem_dma|7|(1<<9), size);

        io->mapped = mapped;
        io->pitch   = pitch;
        io->handle  = (u32_t)pixmap;

        pixmap->width   = io->width;
        pixmap->height  = io->height;
        pixmap->format  = PICT_a8r8g8b8;
        pixmap->flags   = io->flags;
        pixmap->pitch   = pitch;
        pixmap->mapped  = mapped;

        dbgprintf("pixmap.pitch_offset: %x\n", pixmap->pitch_offset);
        dbgprintf("width: %d height: %d\n",pixmap->width,pixmap->height );
        dbgprintf("map at %x\n", pixmap->mapped);

        return ERR_OK;
    };
    rhd_mem_free(&rhd, RHD_MEM_FB, mem_local);
    free(pixmap);

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

    if( (io->flags & PX_MEM_MASK) == PX_MEM_LOCAL )
    {
        rhd_mem_free(&rhd,RHD_MEM_FB,pixmap->local-rhd.fbLocation);
    }
    else
    {
        count_t pages = size >> 12;
        addr_t  base  = pixmap->local;

        while( pages--)
        {
            addr_t tmp;
 //           __asm__ __volatile__(
//            "call *__imp__PageFree"
//            :"=eax" (tmp):"a" (base) );
//            base+= 4096;
        };
    }

    free(pixmap);

    io->format  = 0;
    io->pitch   = 0;
    io->mapped  = NULL;
    io->handle  = 0;

    return ERR_OK;
};


# define ATI_PCIGART_PAGE_SIZE      4096    /**< PCI GART page size */
# define ATI_PCIGART_PAGE_MASK		(~(ATI_PCIGART_PAGE_SIZE-1))

#define ATI_PCIE_WRITE  0x4
#define ATI_PCIE_READ   0x8

#define upper_32_bits(n) ((u32_t)(((n) >> 16) >> 16))


static void bind_pcie(u32_t *gart, addr_t base, count_t pages)
{
    addr_t page_base;

    while(pages--)
    {
        page_base = base & ATI_PCIGART_PAGE_MASK;

        page_base >>= 8;
        page_base |= (upper_32_bits(base) & 0xff) << 24;
        page_base |= ATI_PCIE_READ | ATI_PCIE_WRITE;

        *gart = page_base;
        base+= 4096;
        gart++;
    }
    __asm__ __volatile("sfence":::"memory");

    RADEON_WRITE_PCIE(RADEON_PCIE_TX_GART_CNTL,
                      RADEON_PCIE_TX_GART_EN
                     | RADEON_PCIE_TX_GART_INVALIDATE_TLB);
}

static void bind_pci(u32_t *gart, addr_t base, count_t pages)
{
    u32_t tmp;

    tmp = INREG(RADEON_AIC_CNTL);
    OUTREG(RADEON_AIC_CNTL, tmp & ~RADEON_PCIGART_TRANSLATE_EN);

    while(pages--)
    {
        *gart = base & ATI_PCIGART_PAGE_MASK;
         base+= 4096;
         gart++;
    }
    __asm__ __volatile("sfence":::"memory");

    OUTREG(RADEON_AIC_CNTL, tmp | RADEON_PCIGART_TRANSLATE_EN);
    OUTREG(RADEON_AIC_PT_BASE, rhd.gart_table_dma);
}

static addr_t bind_pixmap(local_pixmap_t *pixmap)
{
    u32_t   *gart = rhd.gart_table;
    count_t  pages = ((pixmap->height * pixmap->pitch+4095)&~4095)>>12;
    addr_t   base = pixmap->local;

    if( rhd.gart_type == RADEON_IS_PCIE)
        bind_pcie(gart, base, pages);
    else
        bind_pci(gart, base, pages);

    return ((pixmap->pitch / 64) << 22) | (rhd.gart_vm_start >> 10);
}

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


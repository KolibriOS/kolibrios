

#include <types.h>
#include <core.h>

#include "multiboot.h"

extern u32_t pg_balloc;

extern u32_t mem_amount;
extern u32_t rd_base;
extern u32_t rd_fat      ;
extern u32_t rd_fat_end  ;
extern u32_t rd_root     ;
extern u32_t rd_root_end ;

extern multiboot_info_t *boot_mbi;

/* Check if the bit BIT in FLAGS is set.  */
#define CHECK_FLAG(flags,bit)	((flags) & (1 << (bit)))

void parse_mbi()
{
   u32_t   last_page = 0;

   if (CHECK_FLAG (boot_mbi->flags, 1))
     DBG ("boot_device = 0x%x\n", (unsigned) boot_mbi->boot_device);

  /* Is the command line passed?  */
   if (CHECK_FLAG (boot_mbi->flags, 2))
     DBG ("cmdline = %s\n", (char *) boot_mbi->cmdline);

  /* Are mods_* valid?  */
   if (CHECK_FLAG (boot_mbi->flags, 3))
   {
     module_t *mod;
     int i;

     DBG ("mods_count = %d, mods_addr = 0x%x\n",
         (u32_t) boot_mbi->mods_count, (u32_t) boot_mbi->mods_addr);
     for (i = 0, mod = (module_t *) boot_mbi->mods_addr;
          i < boot_mbi->mods_count;i++, mod++)
     {
        pg_balloc = mod->mod_end;
        DBG (" mod_start = 0x%x, mod_end = 0x%x, string = %s\n",
            (u32_t) mod->mod_start,(u32_t) mod->mod_end, (char *) mod->string);
     };
     mod--;
     rd_base     = mod->mod_start+OS_BASE;
     rd_fat      = rd_base + 512;
     rd_fat_end  = rd_base + 512 + 4278;
     rd_root     = rd_base + 512*19;
     rd_root_end = rd_base + 512*33;
   //  printf(" rd_base = %x\n", rd_base);
   }

   if (CHECK_FLAG (boot_mbi->flags, 6))
   {
      memory_map_t *mmap;
      u32_t page;

      DBG("mmap_addr = 0x%x, mmap_length = 0x%x\n",
         (unsigned) boot_mbi->mmap_addr, (unsigned) boot_mbi->mmap_length);

      for (mmap = (memory_map_t *) boot_mbi->mmap_addr;
          (u32_t) mmap < boot_mbi->mmap_addr + boot_mbi->mmap_length;
           mmap = (memory_map_t *) ((u32_t) mmap
				    + mmap->size + sizeof (mmap->size)))
      {
         u32_t page;

         DBG (" size = 0x%x, base_addr = 0x%x%x,"
              " length = 0x%x%x, type = 0x%x\n",
             (unsigned) mmap->size,
             (unsigned) mmap->base_addr_high,
             (unsigned) mmap->base_addr_low,
             (unsigned) mmap->length_high,
             (unsigned) mmap->length_low,
             (unsigned) mmap->type);

         if( mmap->type != 1)
           continue;
         page = (mmap->base_addr_low+mmap->length_low)&(~4095);
         if(page > last_page)
         last_page = page;
      }
   }

   if(last_page > 256*1024*1024)
     last_page = 256*1024*1024;

   mem_amount = last_page;

 };


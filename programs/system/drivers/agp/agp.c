
#include "types.h"

#include <stdio.h>
#include <malloc.h>
#include <memory.h>

#include "pci.h"
#include "agp.h"

#include "syscall.h"


agp_t agp_dev;


int __stdcall srv_agp(ioctl_t *io);


u32_t __stdcall drvEntry(int action)
{
    u32_t retval;

    int i;

    if(action != 1)
        return 0;

    if(!dbg_open("/rd/1/drivers/agp.log"))
    {
        printf("Can't open /rd/1/drivers/agp.log\nExit\n");
        return 0;
    }

    if( FindPciDevice() == 0)
    {
        dbgprintf("Device not found\n");
        return 0;
    };

    return 0;

//    retval = RegService("AGP", srv_2d);
//    dbgprintf("reg service %s as: %x\n", "HDRAW", retval);

//    return retval;
};


static void intel_8xx_tlbflush(void *mem)
{
    u32_t temp;

    temp = pciReadLong(agp_dev.PciTag, INTEL_AGPCTRL);
    pciWriteLong(agp_dev.PciTag, INTEL_AGPCTRL, temp & ~(1 << 7));
    temp = pciReadLong(agp_dev.PciTag, INTEL_AGPCTRL);
    pciWriteLong(agp_dev.PciTag, INTEL_AGPCTRL, temp | (1 << 7));
}


static aper_size_t intel_8xx_sizes[7] =
{
    { 256, 65536, 64,  0 },
    { 128, 32768, 32, 32 },
    {  64, 16384, 16, 48 },
    {  32,  8192,  8, 56 },
    {  16,  4096,  4, 60 },
    {   8,  2048,  2, 62 },
    {   4,  1024,  1, 63 }
};

#if 0
static int agp_backend_initialize(struct agp_bridge_data *bridge)
{
	int size_value, rc, got_gatt=0, got_keylist=0;

	bridge->max_memory_agp = agp_find_max();
	bridge->version = &agp_current_version;

	if (bridge->driver->needs_scratch_page) {
		void *addr = bridge->driver->agp_alloc_page(bridge);

		if (!addr) {
			printk(KERN_ERR PFX "unable to get memory for scratch page.\n");
			return -ENOMEM;
		}
		flush_agp_mappings();

		bridge->scratch_page_real = virt_to_gart(addr);
		bridge->scratch_page =
		    bridge->driver->mask_memory(bridge, bridge->scratch_page_real, 0);
	}

	size_value = bridge->driver->fetch_size();
	if (size_value == 0) {
		printk(KERN_ERR PFX "unable to determine aperture size.\n");
		rc = -EINVAL;
		goto err_out;
	}
	if (bridge->driver->create_gatt_table(bridge)) {
		printk(KERN_ERR PFX
		    "unable to get memory for graphics translation table.\n");
		rc = -ENOMEM;
		goto err_out;
	}
	got_gatt = 1;

	bridge->key_list = vmalloc(PAGE_SIZE * 4);
	if (bridge->key_list == NULL) {
		printk(KERN_ERR PFX "error allocating memory for key lists.\n");
		rc = -ENOMEM;
		goto err_out;
	}
	got_keylist = 1;

	/* FIXME vmalloc'd memory not guaranteed contiguous */
	memset(bridge->key_list, 0, PAGE_SIZE * 4);

	if (bridge->driver->configure()) {
		printk(KERN_ERR PFX "error configuring host chipset.\n");
		rc = -EINVAL;
		goto err_out;
	}

	return 0;

err_out:
	if (bridge->driver->needs_scratch_page) {
		bridge->driver->agp_destroy_page(
				gart_to_virt(bridge->scratch_page_real));
		flush_agp_mappings();
	}
	if (got_gatt)
		bridge->driver->free_gatt_table(bridge);
	if (got_keylist) {
		vfree(bridge->key_list);
		bridge->key_list = NULL;
	}
	return rc;
}


#endif


static int intel_845_configure(void *bridge)
{
    u32_t temp;
    u8_t  temp2;
    aper_size_t *current_size;

    agp_t *agp = (agp_t*)bridge;

    current_size = agp->current_size;

	/* aperture size */
    pciWriteByte(agp->PciTag, INTEL_APSIZE, current_size->size_value);

    dbgprintf("INTEL_APSIZE %d\n", current_size->size_value );

    if (agp->apbase_config != 0)
    {
        pciWriteLong(agp->PciTag, AGP_APBASE, agp->apbase_config);
    }
    else
    {
		/* address to map to */
        temp = pciReadLong(agp->PciTag, AGP_APBASE);
        agp->gart_addr = (temp & PCI_MAP_MEMORY_ADDRESS_MASK);
        agp->apbase_config = temp;
	}

    dbgprintf("AGP_APBASE %x\n", temp );

	/* attbase - aperture base */
    pciWriteLong(agp->PciTag, INTEL_ATTBASE, agp->gatt_dma);

	/* agpctrl */
    pciWriteLong(agp->PciTag, INTEL_AGPCTRL, 0x0000);

	/* agpm */
    temp2 = pciReadByte(agp->PciTag, INTEL_I845_AGPM);
    pciWriteByte(agp->PciTag, INTEL_I845_AGPM, temp2 | (1 << 1));
	/* clear any possible error conditions */
    pciWriteWord(agp->PciTag, INTEL_I845_ERRSTS, 0x001c);
	return 0;
}


int agp_generic_create_gatt_table(agp_t *bridge)
{
    count_t pages;

    pages = bridge->current_size->pages_count;

    bridge->gatt_dma = AllocPages(pages);

    bridge->gatt_table = (u32_t*)MapIoMem((void*)bridge->gatt_dma,
                                     pages<<12, PG_SW+PG_NOCACHE);

    dbgprintf("gatt map %x at %x %d pages\n",bridge->gatt_dma ,
               bridge->gatt_table, pages);

    if (bridge->gatt_table == NULL)
        return -30;//ENOMEM;

	/* AK: bogus, should encode addresses > 4GB */
//    for (i = 0; i < num_entries; i++) {
//        writel(bridge->scratch_page, bridge->gatt_table+i);
//        readl(bridge->gatt_table+i);    /* PCI Posting. */
//    }

	return 0;
}



static int __intel_8xx_fetch_size(u8_t temp)
{
	int i;
    aper_size_t *values;

   // values = A_SIZE_8(agp_bridge->driver->aperture_sizes);

    values = intel_8xx_sizes;

    for (i = 0; i < 7; i++)
    {
        if (temp == values[i].size_value)
        {
            agp_dev.previous_size =
                agp_dev.current_size = (void *) (values + i);
            agp_dev.aperture_size_idx = i;
			return values[i].size;
		}
	}
	return 0;
}

static int intel_8xx_fetch_size(void)
{
    u8_t temp;

    temp = pciReadByte(agp_dev.PciTag, INTEL_APSIZE);
	return __intel_8xx_fetch_size(temp);
}


int agp_bind_memory(addr_t agp_addr, addr_t dma_addr, size_t size)
{
	int ret_val;
    count_t count;

//    if (curr == NULL)
//        return -EINVAL;

//    if (curr->is_bound == TRUE) {
//        printk(KERN_INFO PFX "memory %p is already bound!\n", curr);
//        return -EINVAL;
//    }
//    if (curr->is_flushed == FALSE) {
//        curr->bridge->driver->cache_flush();
//        curr->is_flushed = TRUE;
//    }
//    ret_val = curr->bridge->driver->insert_memory(curr, pg_start, curr->type);

    u32_t volatile *table = &agp_dev.gatt_table[agp_addr>>12];

    count = size >> 12;

    dma_addr |= 0x00000017;

    while(count--)
    {
        addr_t tmp;

        *table = dma_addr;
        tmp = *table;                    /* PCI Posting. */
        table++;
        dma_addr+=4096;
    }

    agp_dev.tlb_flush(NULL);

//    if (ret_val != 0)
//        return ret_val;

//    curr->is_bound = TRUE;
//    curr->pg_start = pg_start;
	return 0;
}


static agp_t intel_845_driver =
{
//    .aperture_sizes     = intel_8xx_sizes,
//    .size_type          = U8_APER_SIZE,
//    .num_aperture_sizes = 7,
    .configure          = intel_845_configure,
    .fetch_size         = intel_8xx_fetch_size,
//    .cleanup            = intel_8xx_cleanup,
    .tlb_flush          = intel_8xx_tlbflush,
//    .mask_memory        = agp_generic_mask_memory,
//    .masks              = intel_generic_masks,
//    .agp_enable         = agp_generic_enable,
//    .cache_flush        = global_cache_flush,
//    .create_gatt_table  = agp_generic_create_gatt_table,
//    .free_gatt_table    = agp_generic_free_gatt_table,
//    .insert_memory      = agp_generic_insert_memory,
//    .remove_memory      = agp_generic_remove_memory,
//    .alloc_by_type      = agp_generic_alloc_by_type,
//    .free_by_type       = agp_generic_free_by_type,
//    .agp_alloc_page     = agp_generic_alloc_page,
//    .agp_destroy_page   = agp_generic_destroy_page,
};


#include "detect.inc"

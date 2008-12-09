
#include "types.h"
#include "link.h"

#include <stdio.h>
#include <malloc.h>
#include <memory.h>

#include "pci.h"
#include "agp.h"

#include "syscall.h"


agp_t *bridge;


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


#include "pci.inc"

static void intel_8xx_tlbflush(void *mem)
{
    u32_t temp;

    temp = pciReadLong(bridge->PciTag, INTEL_AGPCTRL);
    pciWriteLong(bridge->PciTag, INTEL_AGPCTRL, temp & ~(1 << 7));
    temp = pciReadLong(bridge->PciTag, INTEL_AGPCTRL);
    pciWriteLong(bridge->PciTag, INTEL_AGPCTRL, temp | (1 << 7));
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



static int intel_845_configure()
{
    u32_t temp;
    u8_t  temp2;
    aper_size_t *current_size;

    current_size = bridge->current_size;

	/* aperture size */
    pciWriteByte(bridge->PciTag, INTEL_APSIZE, current_size->size_value);

    dbgprintf("INTEL_APSIZE %d\n", current_size->size_value );

    if (bridge->apbase_config != 0)
    {
        pciWriteLong(bridge->PciTag, AGP_APBASE, bridge->apbase_config);
    }
    else
    {
		/* address to map to */
        temp = pciReadLong(bridge->PciTag, AGP_APBASE);
        bridge->gart_addr = (temp & PCI_MAP_MEMORY_ADDRESS_MASK);
        bridge->apbase_config = temp;
	}

    dbgprintf("AGP_APBASE %x\n", temp );

	/* attbase - aperture base */
    pciWriteLong(bridge->PciTag, INTEL_ATTBASE, bridge->gatt_dma);

	/* agpctrl */
    pciWriteLong(bridge->PciTag, INTEL_AGPCTRL, 0x0000);

	/* agpm */
    temp2 = pciReadByte(bridge->PciTag, INTEL_I845_AGPM);
    pciWriteByte(bridge->PciTag, INTEL_I845_AGPM, temp2 | (1 << 1));
	/* clear any possible error conditions */
    pciWriteWord(bridge->PciTag, INTEL_I845_ERRSTS, 0x001c);
	return 0;
}


int agp_generic_create_gatt_table()
{
    count_t pages;

    pages = bridge->current_size->pages_count;

    if( bridge->gatt_dma = AllocPages(pages))
    {
        if(bridge->gatt_table =
           (u32_t*)MapIoMem((void*)bridge->gatt_dma,
                            pages<<12, PG_SW+PG_NOCACHE))
        {
            dbgprintf("gatt map %x at %x %d pages\n",bridge->gatt_dma ,
                       bridge->gatt_table, pages);

	/* AK: bogus, should encode addresses > 4GB */

            u32_t volatile *table = bridge->gatt_table;

            count_t count = bridge->current_size->num_entries;

            while(count--) {            /* FIXME memset */
                addr_t tmp;

                *table = 0;
                table++;
            }
            return 1;
        };
    };
    dbgprintf("unable to get memory for "
              "graphics translation table.\n");
	return 0;
}


static int __intel_8xx_fetch_size(u8_t temp)
{
	int i;
    aper_size_t *values;

    values = bridge->aperture_sizes;

    values = intel_8xx_sizes;

    for (i = 0; i < 7; i++)
    {
        if (temp == values[i].size_value)
        {
            bridge->previous_size =
                bridge->current_size = (void *) (values + i);
            bridge->aperture_size_idx = i;
			return values[i].size;
		}
	}
	return 0;
}

static int intel_8xx_fetch_size(void)
{
    u8_t temp;

    temp = pciReadByte(bridge->PciTag, INTEL_APSIZE);
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

    u32_t volatile *table = &bridge->gatt_table[agp_addr>>12];

    count = size >> 12;

    dma_addr |= 0x00000017;

    while(count--)
    {
        *table = dma_addr;
        table++;
        dma_addr+=4096;
    }

    bridge->tlb_flush(NULL);

//    if (ret_val != 0)
//        return ret_val;

//    curr->is_bound = TRUE;
//    curr->pg_start = pg_start;
	return 0;
}

void get_agp_version(agp_t *bridge)
{
    u32_t ncapid;

	/* Exit early if already set by errata workarounds. */
	if (bridge->major_version != 0)
		return;

    ncapid = pciReadLong(bridge->PciTag, bridge->capndx);
	bridge->major_version = (ncapid >> AGP_MAJOR_VERSION_SHIFT) & 0xf;
	bridge->minor_version = (ncapid >> AGP_MINOR_VERSION_SHIFT) & 0xf;
}

static void agp_v2_parse_one(u32_t *requested_mode, u32_t *bridge_agpstat, u32_t *vga_agpstat)
{
    u32_t tmp;

	if (*requested_mode & AGP2_RESERVED_MASK) {
        dbgprintf("reserved bits set (%x) in mode 0x%x. Fixed.\n",
			*requested_mode & AGP2_RESERVED_MASK, *requested_mode);
		*requested_mode &= ~AGP2_RESERVED_MASK;
	}

	/* Check the speed bits make sense. Only one should be set. */
	tmp = *requested_mode & 7;
	switch (tmp) {
		case 0:
            dbgprintf("Setting to x1 mode.\n");
			*requested_mode |= AGPSTAT2_1X;
			break;
		case 1:
		case 2:
			break;
		case 3:
			*requested_mode &= ~(AGPSTAT2_1X);	/* rate=2 */
			break;
		case 4:
			break;
		case 5:
		case 6:
		case 7:
			*requested_mode &= ~(AGPSTAT2_1X|AGPSTAT2_2X); /* rate=4*/
			break;
	}

	/* disable SBA if it's not supported */
	if (!((*bridge_agpstat & AGPSTAT_SBA) && (*vga_agpstat & AGPSTAT_SBA) && (*requested_mode & AGPSTAT_SBA)))
		*bridge_agpstat &= ~AGPSTAT_SBA;

	/* Set rate */
	if (!((*bridge_agpstat & AGPSTAT2_4X) && (*vga_agpstat & AGPSTAT2_4X) && (*requested_mode & AGPSTAT2_4X)))
		*bridge_agpstat &= ~AGPSTAT2_4X;

	if (!((*bridge_agpstat & AGPSTAT2_2X) && (*vga_agpstat & AGPSTAT2_2X) && (*requested_mode & AGPSTAT2_2X)))
		*bridge_agpstat &= ~AGPSTAT2_2X;

	if (!((*bridge_agpstat & AGPSTAT2_1X) && (*vga_agpstat & AGPSTAT2_1X) && (*requested_mode & AGPSTAT2_1X)))
		*bridge_agpstat &= ~AGPSTAT2_1X;

	/* Now we know what mode it should be, clear out the unwanted bits. */
	if (*bridge_agpstat & AGPSTAT2_4X)
		*bridge_agpstat &= ~(AGPSTAT2_1X | AGPSTAT2_2X);	/* 4X */

	if (*bridge_agpstat & AGPSTAT2_2X)
		*bridge_agpstat &= ~(AGPSTAT2_1X | AGPSTAT2_4X);	/* 2X */

	if (*bridge_agpstat & AGPSTAT2_1X)
		*bridge_agpstat &= ~(AGPSTAT2_2X | AGPSTAT2_4X);	/* 1X */

	/* Apply any errata. */
    if (bridge->flags & AGP_ERRATA_FASTWRITES)
		*bridge_agpstat &= ~AGPSTAT_FW;

    if (bridge->flags & AGP_ERRATA_SBA)
		*bridge_agpstat &= ~AGPSTAT_SBA;

    if (bridge->flags & AGP_ERRATA_1X) {
		*bridge_agpstat &= ~(AGPSTAT2_2X | AGPSTAT2_4X);
		*bridge_agpstat |= AGPSTAT2_1X;
	}

	/* If we've dropped down to 1X, disable fast writes. */
	if (*bridge_agpstat & AGPSTAT2_1X)
		*bridge_agpstat &= ~AGPSTAT_FW;
}


static void agp_v3_parse_one(u32_t *requested_mode,
                             u32_t *bridge_agpstat,
                             u32_t *vga_agpstat)
{
    u32_t origbridge = *bridge_agpstat, origvga = *vga_agpstat;
    u32_t tmp;

    if (*requested_mode & AGP3_RESERVED_MASK)
    {
        dbgprintf("reserved bits set (%x) in mode 0x%x. Fixed.\n",
			*requested_mode & AGP3_RESERVED_MASK, *requested_mode);
		*requested_mode &= ~AGP3_RESERVED_MASK;
	}

	/* Check the speed bits make sense. */
	tmp = *requested_mode & 7;
	if (tmp == 0) {
        dbgprintf("Setting to AGP3 x4 mode.\n");
		*requested_mode |= AGPSTAT3_4X;
	}
	if (tmp >= 3) {
        dbgprintf("Setting to AGP3 x8 mode.\n");
		*requested_mode = (*requested_mode & ~7) | AGPSTAT3_8X;
	}

	/* ARQSZ - Set the value to the maximum one.
	 * Don't allow the mode register to override values. */
	*bridge_agpstat = ((*bridge_agpstat & ~AGPSTAT_ARQSZ) |
        max_t(u32_t,(*bridge_agpstat & AGPSTAT_ARQSZ),(*vga_agpstat & AGPSTAT_ARQSZ)));

	/* Calibration cycle.
	 * Don't allow the mode register to override values. */
	*bridge_agpstat = ((*bridge_agpstat & ~AGPSTAT_CAL_MASK) |
        min_t(u32_t,(*bridge_agpstat & AGPSTAT_CAL_MASK),(*vga_agpstat & AGPSTAT_CAL_MASK)));

	/* SBA *must* be supported for AGP v3 */
	*bridge_agpstat |= AGPSTAT_SBA;

	/*
	 * Set speed.
	 * Check for invalid speeds. This can happen when applications
	 * written before the AGP 3.0 standard pass AGP2.x modes to AGP3 hardware
	 */
	if (*requested_mode & AGPSTAT_MODE_3_0) {
		/*
		 * Caller hasn't a clue what it is doing. Bridge is in 3.0 mode,
		 * have been passed a 3.0 mode, but with 2.x speed bits set.
		 * AGP2.x 4x -> AGP3.0 4x.
		 */
		if (*requested_mode & AGPSTAT2_4X) {
            dbgprintf("broken AGP3 flags (%x). Fixed.\n", *requested_mode);
			*requested_mode &= ~AGPSTAT2_4X;
			*requested_mode |= AGPSTAT3_4X;
		}
	} else {
		/*
		 * The caller doesn't know what they are doing. We are in 3.0 mode,
		 * but have been passed an AGP 2.x mode.
		 * Convert AGP 1x,2x,4x -> AGP 3.0 4x.
		 */
        dbgprintf("broken AGP2 flags (%x) in AGP3 mode. Fixed.\n",*requested_mode);
		*requested_mode &= ~(AGPSTAT2_4X | AGPSTAT2_2X | AGPSTAT2_1X);
		*requested_mode |= AGPSTAT3_4X;
	}

	if (*requested_mode & AGPSTAT3_8X) {
		if (!(*bridge_agpstat & AGPSTAT3_8X)) {
			*bridge_agpstat &= ~(AGPSTAT3_8X | AGPSTAT3_RSVD);
			*bridge_agpstat |= AGPSTAT3_4X;
            dbgprintf("requested AGPx8 but bridge not capable.\n");
			return;
		}
		if (!(*vga_agpstat & AGPSTAT3_8X)) {
			*bridge_agpstat &= ~(AGPSTAT3_8X | AGPSTAT3_RSVD);
			*bridge_agpstat |= AGPSTAT3_4X;
            dbgprintf("requested AGPx8 but graphic card not capable.\n");
			return;
		}
		/* All set, bridge & device can do AGP x8*/
		*bridge_agpstat &= ~(AGPSTAT3_4X | AGPSTAT3_RSVD);
		goto done;

	} else {

		/*
		 * If we didn't specify AGPx8, we can only do x4.
		 * If the hardware can't do x4, we're up shit creek, and never
		 *  should have got this far.
		 */
		*bridge_agpstat &= ~(AGPSTAT3_8X | AGPSTAT3_RSVD);
		if ((*bridge_agpstat & AGPSTAT3_4X) && (*vga_agpstat & AGPSTAT3_4X))
			*bridge_agpstat |= AGPSTAT3_4X;
		else {
            dbgprintf("Badness. Don't know which AGP mode to set. "
							"[bridge_agpstat:%x vga_agpstat:%x fell back to:- bridge_agpstat:%x vga_agpstat:%x]\n",
							origbridge, origvga, *bridge_agpstat, *vga_agpstat);
			if (!(*bridge_agpstat & AGPSTAT3_4X))
                dbgprintf("Bridge couldn't do AGP x4.\n");
			if (!(*vga_agpstat & AGPSTAT3_4X))
                dbgprintf("Graphic card couldn't do AGP x4.\n");
			return;
		}
	}

done:
	/* Apply any errata. */
    if (bridge->flags & AGP_ERRATA_FASTWRITES)
		*bridge_agpstat &= ~AGPSTAT_FW;

    if (bridge->flags & AGP_ERRATA_SBA)
		*bridge_agpstat &= ~AGPSTAT_SBA;

    if (bridge->flags & AGP_ERRATA_1X) {
		*bridge_agpstat &= ~(AGPSTAT2_2X | AGPSTAT2_4X);
		*bridge_agpstat |= AGPSTAT2_1X;
	}
}


u32_t agp_collect_device_status(agp_t *bridge, u32_t requested_mode,
                                u32_t bridge_agpstat)
{
    PCITAG  vgaTag;
    u32_t   vga_agpstat;
    int     cap_ptr;

    for (;;)
    {
        vgaTag = pci_find_class(PCI_CLASS_DISPLAY_VGA);
        if (vgaTag == -1)
        {
            dbgprintf("Couldn't find an AGP VGA controller.\n");
			return 0;
		}
        cap_ptr = pci_find_capability(vgaTag, PCI_CAP_ID_AGP);
		if (cap_ptr)
			break;
	}

	/*
	 * Ok, here we have a AGP device. Disable impossible
	 * settings, and adjust the readqueue to the minimum.
	 */
    vga_agpstat = pciReadLong(vgaTag, cap_ptr+PCI_AGP_STATUS);

	/* adjust RQ depth */
	bridge_agpstat = ((bridge_agpstat & ~AGPSTAT_RQ_DEPTH) |
         min_t(u32_t, (requested_mode & AGPSTAT_RQ_DEPTH),
         min_t(u32_t, (bridge_agpstat & AGPSTAT_RQ_DEPTH), (vga_agpstat & AGPSTAT_RQ_DEPTH))));

	/* disable FW if it's not supported */
	if (!((bridge_agpstat & AGPSTAT_FW) &&
          (vga_agpstat & AGPSTAT_FW) &&
          (requested_mode & AGPSTAT_FW)))
		bridge_agpstat &= ~AGPSTAT_FW;

	/* Check to see if we are operating in 3.0 mode */
    if (bridge->mode & AGPSTAT_MODE_3_0)
		agp_v3_parse_one(&requested_mode, &bridge_agpstat, &vga_agpstat);
	else
		agp_v2_parse_one(&requested_mode, &bridge_agpstat, &vga_agpstat);

	return bridge_agpstat;
}


void agp_device_command(u32_t bridge_agpstat, int agp_v3)
{
    PCITAG device = 0;
	int mode;

	mode = bridge_agpstat & 0x7;
	if (agp_v3)
		mode *= 4;

    for_each_pci_dev(device)
    {
        int agp = pci_find_capability(device, PCI_CAP_ID_AGP);
		if (!agp)
			continue;

        dbgprintf("Putting AGP V%d device at into %dx mode\n",
                agp_v3 ? 3 : 2, mode);
        pciWriteLong(device, agp + PCI_AGP_COMMAND, bridge_agpstat);
	}
}


struct agp_3_5_dev
{
    link_t link;
    int    capndx;
    u32_t  maxbw;
    PCITAG tag;
};


/*
 * Fully configure and enable an AGP 3.0 host bridge and all the devices
 * lying behind it.
 */
int agp_3_5_enable(agp_t *bridge)
{
    u8_t   mcapndx;
    u32_t  isoch, arqsz;
    u32_t  tstatus, mstatus, ncapid;
    u32_t  mmajor;
    u16_t  mpstat;

    link_t dev_list;

    struct agp_3_5_dev *cur, *pos;

	unsigned int ndevs = 0;
    PCITAG dev = 0;
	int ret = 0;

	/* Extract some power-on defaults from the target */
    tstatus = pciReadLong(bridge->PciTag, bridge->capndx+AGPSTAT);
	isoch     = (tstatus >> 17) & 0x1;
	if (isoch == 0)	/* isoch xfers not available, bail out. */
        return -1;

	arqsz     = (tstatus >> 13) & 0x7;

    list_initialize(&dev_list);

	/* Find all AGP devices, and add them to dev_list. */
    for_each_pci_dev(dev)
    {
        u16_t devclass;

		mcapndx = pci_find_capability(dev, PCI_CAP_ID_AGP);
		if (mcapndx == 0)
			continue;

        devclass = pciReadWord(dev, 0x0A);

        switch (devclass & 0xff00)
        {
			case 0x0600:    /* Bridge */
				/* Skip bridges. We should call this function for each one. */
				continue;

			case 0x0001:    /* Unclassified device */
				/* Don't know what this is, but log it for investigation. */
				if (mcapndx != 0) {
                    dbgprintf("Wacky, found unclassified AGP device.\n");
				}
				continue;

			case 0x0300:    /* Display controller */
			case 0x0400:    /* Multimedia controller */
                if((cur = malloc(sizeof(*cur))) == NULL)
                {
                    ret = -1;
					goto free_and_exit;
				}
                cur->tag = dev;
                list_prepend(&cur->link, &dev_list);
				ndevs++;
				continue;

			default:
				continue;
		}
	}

	/*
	 * Take an initial pass through the devices lying behind our host
	 * bridge.  Make sure each one is actually an AGP 3.0 device, otherwise
	 * exit with an error message.  Along the way store the AGP 3.0
	 * cap_ptr for each device
	 */

    cur = (struct agp_3_5_dev*)dev_list.next;

    while(&cur->link != &dev_list)
    {
        dev = cur->tag;

        mpstat = pciReadWord(dev, PCI_STATUS);
		if ((mpstat & PCI_STATUS_CAP_LIST) == 0)
			continue;

        mcapndx = pciReadByte(dev, PCI_CAPABILITY_LIST);
		if (mcapndx != 0) {
			do {
                ncapid = pciReadLong(dev, mcapndx);
				if ((ncapid & 0xff) != 2)
					mcapndx = (ncapid >> 8) & 0xff;
			}
			while (((ncapid & 0xff) != 2) && (mcapndx != 0));
		}

		if (mcapndx == 0) {
            dbgprintf("woah!  Non-AGP device "
				"found on the secondary bus of an AGP 3.5 bridge!\n");
            ret = -1;
			goto free_and_exit;
		}

		mmajor = (ncapid >> AGP_MAJOR_VERSION_SHIFT) & 0xf;
		if (mmajor < 3) {
            dbgprintf("woah!  AGP 2.0 device "
				"found on the secondary bus of an AGP 3.5 "
				"bridge operating with AGP 3.0 electricals!\n");
            ret = -1;
			goto free_and_exit;
		}

		cur->capndx = mcapndx;

        mstatus = pciReadLong(dev, cur->capndx+AGPSTAT);

		if (((mstatus >> 3) & 0x1) == 0) {
            dbgprintf("woah!  AGP 3.x device "
				"not operating in AGP 3.x mode found on the "
				"secondary bus of an AGP 3.5 bridge operating "
				"with AGP 3.0 electricals!\n");
            ret = -1;
			goto free_and_exit;
		}
        cur = (struct agp_3_5_dev*)cur->link.next;
	}

	/*
	 * Call functions to divide target resources amongst the AGP 3.0
	 * masters.  This process is dramatically different depending on
	 * whether isochronous transfers are supported.
	 */
	if (isoch) {
        ret = agp_3_5_isochronous_node_enable(bridge, &dev_list, ndevs);
		if (ret) {
            dbgprintf("Something bad happened setting "
                      "up isochronous xfers.  Falling back to "
                      "non-isochronous xfer mode.\n");
		} else {
			goto free_and_exit;
		}
	}
	agp_3_5_nonisochronous_node_enable(bridge, dev_list, ndevs);

free_and_exit:
	/* Be sure to free the dev_list */
    for (pos = (struct agp_3_5_dev*)dev_list.next; &pos->link != &dev_list; )
    {
        cur = pos;

        pos = (struct agp_3_5_dev*)pos->link.next;
        free(cur);
	}

get_out:
	return ret;
}


void agp_generic_enable(u32_t requested_mode)
{
    u32_t bridge_agpstat, temp;

    get_agp_version(bridge);

    dbgprintf("Found an AGP %d.%d compliant device.\n",
           bridge->major_version, bridge->minor_version);

    bridge_agpstat = pciReadLong(bridge->PciTag,
                     bridge->capndx + PCI_AGP_STATUS);

    bridge_agpstat = agp_collect_device_status(bridge, requested_mode, bridge_agpstat);
	if (bridge_agpstat == 0)
		/* Something bad happened. FIXME: Return error code? */
		return;

	bridge_agpstat |= AGPSTAT_AGP_ENABLE;

	/* Do AGP version specific frobbing. */
    if (bridge->major_version >= 3)
    {
        if (bridge->mode & AGPSTAT_MODE_3_0)
        {
			/* If we have 3.5, we can do the isoch stuff. */
            if (bridge->minor_version >= 5)
                agp_3_5_enable(bridge);
            agp_device_command(bridge_agpstat, TRUE);
			return;
        }
        else
        {
		    /* Disable calibration cycle in RX91<1> when not in AGP3.0 mode of operation.*/
		    bridge_agpstat &= ~(7<<10) ;
            temp = pciReadLong(bridge->PciTag, bridge->capndx+AGPCTRL);
		    temp |= (1<<9);
            pciWriteLong(bridge->PciTag, bridge->capndx+AGPCTRL, temp);

            dbgprintf("Device is in legacy mode,"
                      " falling back to 2.x\n");
		}
	}

	/* AGP v<3 */
    agp_device_command(bridge_agpstat, FALSE);
}


static agp_t intel_845_driver =
{
    .aperture_sizes     = intel_8xx_sizes,
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
    .create_gatt_table  = agp_generic_create_gatt_table,
//    .free_gatt_table    = agp_generic_free_gatt_table,
//    .insert_memory      = agp_generic_insert_memory,
//    .remove_memory      = agp_generic_remove_memory,
//    .alloc_by_type      = agp_generic_alloc_by_type,
//    .free_by_type       = agp_generic_free_by_type,
//    .agp_alloc_page     = agp_generic_alloc_page,
//    .agp_destroy_page   = agp_generic_destroy_page,
};

int init_bridge(PCITAG pciTag)
{
    size_t size_value;

    bridge = &intel_845_driver;

    bridge->PciTag = pciTag;

    bridge->capndx = pci_find_capability(pciTag, PCI_CAP_ID_AGP);

    size_value = bridge->fetch_size();

    if (size_value == 0) {
        dbgprintf("unable to determine aperture size.\n");
        return 0;
    };

    dbgprintf("fetch size = %x\n", size_value);

    if( bridge->create_gatt_table() )
    {
        bridge->configure();
        return 1;
    }
    return 0;
}


#include "detect.inc"

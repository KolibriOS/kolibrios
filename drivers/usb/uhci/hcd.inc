
#define UHCI_USBLEGSUP          0x00c0  /* legacy support */
#define UHCI_USBLEGSUP_DEFAULT  0x2000  /* only PIRQ enable set */
#define UHCI_USBLEGSUP_RWC      0x8f00  /* the R/WC bits */
#define UHCI_USBLEGSUP_RO       0x5040  /* R/O and reserved bits */


#define UHCI_USBCMD                  0  /* command register */
#define UHCI_USBINTR                 4  /* interrupt register */
#define UHCI_USBCMD_RUN         0x0001  /* RUN/STOP bit */
#define UHCI_USBCMD_HCRESET     0x0002  /* Host Controller reset */
#define UHCI_USBCMD_EGSM        0x0008  /* Global Suspend Mode */
#define UHCI_USBCMD_CONFIGURE   0x0040  /* Config Flag */
#define UHCI_USBINTR_RESUME     0x0002  /* Resume interrupt enable */


#define USBCMD                       0
#define  USBCMD_RS              0x0001  /* Run/Stop */
#define  USBCMD_HCRESET         0x0002  /* Host reset */
#define  USBCMD_GRESET          0x0004  /* Global reset */
#define  USBCMD_EGSM            0x0008  /* Global Suspend Mode */
#define  USBCMD_FGR             0x0010  /* Force Global Resume */
#define  USBCMD_SWDBG           0x0020  /* SW Debug mode */
#define  USBCMD_CF              0x0040  /* Config Flag (sw only) */
#define  USBCMD_MAXP            0x0080  /* Max Packet (0 = 32, 1 = 64) */

#define  USBSTS                      2
#define   USBSTS_USBINT         0x0001  /* Interrupt due to IOC */
#define   USBSTS_ERROR          0x0002  /* Interrupt due to error */
#define   USBSTS_RD             0x0004  /* Resume Detect */
#define   USBSTS_HSE            0x0008  /* Host System Error: PCI problems */
#define   USBSTS_HCPE           0x0010  /* Host Controller Process Error:
                                         * the schedule is buggy */
#define   USBSTS_HCH            0x0020  /* HC Halted */


#define  USBFRNUM                    6
#define  USBFLBASEADD                8
#define  USBSOF                     12
#define  USBSOF_DEFAULT             64  /* Frame length is exactly 1 ms */

#define  USBPORTSC1                 16
#define  USBPORTSC2                 18

#define  UHCI_RH_MAXCHILD            7


/*
 * Make sure the controller is completely inactive, unable to
 * generate interrupts or do DMA.
 */
void uhci_reset_hc(hc_t *hc)
{
	/* Turn off PIRQ enable and SMI enable.  (This also turns off the
	 * BIOS's USB Legacy Support.)  Turn off all the R/WC bits too.
	 */
    pciWriteWord(hc->PciTag, UHCI_USBLEGSUP, UHCI_USBLEGSUP_RWC);

	/* Reset the HC - this will force us to get a
	 * new notification of any already connected
	 * ports due to the virtual disconnect that it
	 * implies.
	 */
    out16(hc->iobase + UHCI_USBCMD, UHCI_USBCMD_HCRESET);
    __asm__ __volatile__ ("":::"memory");

    delay(20/10);

    if (in16(hc->iobase + UHCI_USBCMD) & UHCI_USBCMD_HCRESET)
        dbgprintf("HCRESET not completed yet!\n");

	/* Just to be safe, disable interrupt requests and
	 * make sure the controller is stopped.
	 */
    out16(hc->iobase + UHCI_USBINTR, 0);
    out16(hc->iobase + UHCI_USBCMD, 0);
};

int uhci_check_and_reset_hc(hc_t *hc)
{
    u16_t legsup;
	unsigned int cmd, intr;

	/*
	 * When restarting a suspended controller, we expect all the
	 * settings to be the same as we left them:
	 *
	 *	PIRQ and SMI disabled, no R/W bits set in USBLEGSUP;
	 *	Controller is stopped and configured with EGSM set;
	 *	No interrupts enabled except possibly Resume Detect.
	 *
	 * If any of these conditions are violated we do a complete reset.
	 */
    legsup = pciReadWord(hc->PciTag, UHCI_USBLEGSUP);
	if (legsup & ~(UHCI_USBLEGSUP_RO | UHCI_USBLEGSUP_RWC)) {
        dbgprintf("%s: legsup = 0x%04x\n",__FUNCTION__, legsup);
		goto reset_needed;
	}

    cmd = in16(hc->iobase + UHCI_USBCMD);
    if ( (cmd & UHCI_USBCMD_RUN) ||
        !(cmd & UHCI_USBCMD_CONFIGURE) ||
        !(cmd & UHCI_USBCMD_EGSM))
    {
        dbgprintf("%s: cmd = 0x%04x\n", __FUNCTION__, cmd);
		goto reset_needed;
	}

    intr = in16(hc->iobase + UHCI_USBINTR);
    if (intr & (~UHCI_USBINTR_RESUME))
    {
        dbgprintf("%s: intr = 0x%04x\n", __FUNCTION__, intr);
		goto reset_needed;
	}
	return 0;

reset_needed:
    dbgprintf("Performing full reset\n");
    uhci_reset_hc(hc);
	return 1;
}

void hc_interrupt()
{
    hc_t   *hc;

//    printf("USB interrupt\n");

    hc = (hc_t*)hc_list.next;

    while( &hc->list != &hc_list)
    {
        hc_t       *htmp;
        request_t  *rq;
        u16_t  status;

        htmp = hc;

        hc = (hc_t*)hc->list.next;

        status = in16(htmp->iobase + USBSTS);
        if (!(status & ~USBSTS_HCH))            /* shared interrupt, not mine */
            continue;
        out16(htmp->iobase + USBSTS, status);     /* Clear it */

        rq = (request_t*)htmp->rq_list.next;

        while( &rq->list != &htmp->rq_list)
        {
            request_t *rtmp;
            td_t      *td;

            rtmp = rq;
            rq = (request_t*)rq->list.next;

            td  = rtmp->td_tail;

            if( td->status & TD_CTRL_ACTIVE)
                continue;

            list_del(&rtmp->list);

            RaiseEvent(rtmp->evh, 0, &rtmp->event);
        };
    }
};



bool init_hc(hc_t *hc)
{
    int    port;
    u32_t  ifl;
    u16_t  dev_status;
    td_t   *td;
    int i;

    dbgprintf("\n\ninit uhci %x\n\n", hc->pciId);

    for(i=0;i<6;i++)
    {
        if(hc->ioBase[i]){
           hc->iobase = hc->ioBase[i];
       //    dbgprintf("Io base_%d 0x%x\n", i,hc->ioBase[i]);
           break;
        };
    };

	/* The UHCI spec says devices must have 2 ports, and goes on to say
	 * they may have more but gives no way to determine how many there
	 * are.  However according to the UHCI spec, Bit 7 of the port
	 * status and control register is always set to 1.  So we try to
	 * use this to our advantage.  Another common failure mode when
	 * a nonexistent register is addressed is to return all ones, so
	 * we test for that also.
	 */
    for (port = 0; port < 2; port++)
    {
        u32_t status;

        status = in16(hc->iobase + USBPORTSC1 + (port * 2));
        dbgprintf("port%d status %x\n", port, status);
        if (!(status & 0x0080) || status == 0xffff)
			break;
	}
    dbgprintf("detected %d ports\n\n", port);

    hc->numports = port;

	/* Kick BIOS off this hardware and reset if the controller
	 * isn't already safely quiescent.
	 */
    uhci_check_and_reset_hc(hc);

    hc->frame_base   = (u32_t*)KernelAlloc(4096);
    hc->frame_dma    = GetPgAddr(hc->frame_base);
    hc->frame_number = 0;

    hc->td_pool = dma_pool_create("uhci_td", NULL,
                                  sizeof(td_t), 16, 0);
    if (!hc->td_pool)
    {
        dbgprintf("unable to create td dma_pool\n");
        goto err_create_td_pool;
    }

    for (i = 0; i < UHCI_NUM_SKELQH; i++)
    {
    qh_t *qh = alloc_qh();

    qh->qlink = 1;
    qh->qelem = 1;

        hc->qh[i] = qh;
    }
    for (i = SKEL_ISO + 1; i < SKEL_ASYNC; ++i)
        hc->qh[i]->qlink = hc->qh[SKEL_ASYNC]->dma | 2;

    for (i = 0; i < 1024; i++)
    {
        int qnum;

        qnum = 8 - (int) __bsf( i | 1024);

        if (qnum <= 1)
            qnum = 9;

        hc->frame_base[i] = hc->qh[qnum]->dma | 2;
    }

    mb();

    /* Set the frame length to the default: 1 ms exactly */
    out8(hc->iobase + USBSOF, USBSOF_DEFAULT);

	/* Store the frame list base address */
    out32(hc->iobase + USBFLBASEADD, hc->frame_dma);

	/* Set the current frame number */
    out16(hc->iobase + USBFRNUM, 0);

    out16(hc->iobase + USBSTS, 0x3F);

    out16(hc->iobase + UHCI_USBINTR, 4);

    AttachIntHandler(hc->irq_line, hc_interrupt, 0);

    pciWriteWord(hc->PciTag, UHCI_USBLEGSUP, UHCI_USBLEGSUP_DEFAULT);

    out16(hc->iobase + USBCMD, USBCMD_RS | USBCMD_CF |
                               USBCMD_MAXP);

    for (port = 0; port < hc->numports; ++port)
        out16(hc->iobase + USBPORTSC1 + (port * 2), 0x200);

    for (port = 0; port < 2; ++port)
    {
        time_t timeout;

        delay(100/10);

        u32_t status = in16(hc->iobase + USBPORTSC1 + (port * 2));
        dbgprintf("port%d status %x\n", port, status);

        out16(hc->iobase + USBPORTSC1 + (port * 2), 0);

        timeout = 100/10;
        while(timeout--)
        {
            delay(10/10);
            status = in16(hc->iobase + USBPORTSC1 + (port * 2));
            if(status & 1)
            {
                udev_t *dev = kmalloc(sizeof(udev_t),0);

                out16(hc->iobase + USBPORTSC1 + (port * 2), 0x0E);

                delay(20/10);

                dbgprintf("enable port\n");
                status = in16(hc->iobase + USBPORTSC1 + (port * 2));
                dbgprintf("port%d status %x\n", port, status);

                INIT_LIST_HEAD(&dev->list);

                dev->host     = hc;
                dev->port     = port;
                dev->ep0_size = 8;
                dev->status   = status;

                dbgprintf("port%d connected", port);
                if(status & 4)
                    dbgprintf(" enabled");
                else
                    dbgprintf(" disabled");
                if(status & 0x100){
                    dev->speed = 0x4000000;
                    dbgprintf(" low speed\n");
                } else {
                    dev->speed = 0;
                    dbgprintf(" full speed\n");
                };

                if(set_address(dev)) {
                    list_add_tail(&dev->list, &newdev_list);
                    hc->port_map |= 1<<port;
                }
                else {
                    free(dev);
                    out16(hc->iobase + USBPORTSC1 + (port * 2), 0);
                }
                break;
            };
        };
    };
    return true;

err_create_td_pool:

    KernelFree(hc->frame_base);

    return false;
};

u16_t __attribute__((aligned(16)))
      req_descr[4]  = {0x0680,0x0100,0x0000,8};

/*
IN(69) OUT(E1) SETUP(2D)
SETUP(0) IN(1)
SETUP(0) OUT(1) OUT(0) OUT(1)...IN(1)
SETUP(0) IN(1) IN(0) IN(1)...OUT(0)
*/


bool set_address(udev_t *dev)
{
    static  udev_id   = 0;
    static  udev_addr = 0;
    static  u16_t __attribute__((aligned(16)))
            req_addr[4] = {0x0500,0x0001,0x0000,0x0000};

    static  u16_t __attribute__((aligned(16)))
            req_descr[4]  = {0x0680,0x0100,0x0000,8};

    static  u32_t   data[2] __attribute__((aligned(16)));

    qh_t  *qh;
    td_t  *td0, *td1, *td2;
    u32_t   dev_status;
    count_t timeout;
    int address;

    address   = ++udev_addr;

    req_addr[1] = address;

    if( !ctrl_request(dev, &req_addr, DOUT, NULL, 0))
        return false;

    dev->addr = address;
    dev->id   = (++udev_id << 8) | address;

    dbgprintf("set address %d\n", address);

    data[0] = 0;
    data[1] = 0;

    if( !ctrl_request(dev, &req_descr, DIN, data, 8))
        return false;

    dev_descr_t *descr = (dev_descr_t*)&data;
    dev->ep0_size = descr->bMaxPacketSize0;

    return true;
}

#define ALIGN16(x) (((x)+15)&~15)

#define MakePtr( cast, ptr, addValue ) (cast)((addr_t)(ptr)+(addr_t)(addValue))

request_t *alloc_rq_buffer(udev_t *dev, endp_t *enp, u32_t dir,
                           size_t data_size)
{
    size_t  packet_size = dev->ep0_size;
    int     dsize = data_size;
    size_t  buf_size;

    addr_t  buf_dma;
    addr_t  td_dma;
    addr_t  data_dma;

    request_t *rq;

    td_t  *td, *td_prev;
    int td_count = 0;

    while(dsize > 0)
    {
        td_count++;
        dsize-= packet_size;
    };

    buf_size = ALIGN16(sizeof(request_t)) + ALIGN16(data_size) +
               td_count*sizeof(td_t);

    rq = (request_t*)hcd_buffer_alloc(buf_size, &buf_dma);
    memset(rq, 0, buf_size);

    data_dma = buf_dma + ALIGN16(sizeof(request_t));
    td_dma = data_dma + ALIGN16(data_size);

    INIT_LIST_HEAD(&rq->list);

    rq->data = MakePtr(addr_t, rq, ALIGN16(sizeof(request_t)));
    td = MakePtr(td_t*, rq->data, ALIGN16(data_size));
    rq->td_head = td;
    rq->size = data_size;
    rq->dev  = dev;

    td_prev = NULL;

    dsize = data_size;

    while(dsize != 0)
    {
        if ( dsize < packet_size)
        {
            packet_size = dsize;
        };

        td->dma = td_dma;

        td->link = 1;

        if( td_prev )
            td_prev->link   = td->dma | 4;
        td->status = TD_CTRL_ACTIVE | dev->speed;
        td->token  = TOKEN(packet_size,enp->toggle,enp->address,
                           dev->addr,dir);
        td->buffer = data_dma;
        td->bk     = td_prev;

        td_prev = td;

        td++;
        td_dma+= sizeof(td_t);
        data_dma+= packet_size;

        dsize-= packet_size;
        enp->toggle ^= DATA1;
    };

    td_prev->status |= TD_CTRL_IOC;
    rq->td_tail = td_prev;

    rq->evh = CreateEvent(NULL, MANUAL_DESTROY);

    if(rq->evh.handle == 0)
        printf("%s: epic fail\n", __FUNCTION__);

    rq->event.code    = 0xFF000001;
    rq->event.data[0] = (addr_t)rq;

    return rq;
}

bool ctrl_request(udev_t *dev, void *req, u32_t pid,
                  void *data, size_t req_size)
{
    size_t  packet_size = dev->ep0_size;
    size_t  size = req_size;
    u32_t   toggle = DATA1;

    td_t   *td0, *td, *td_prev;
    qh_t   *qh;
    addr_t  data_dma = 0;
    hc_t   *hc = dev->host;

    addr_t  td_dma = 0;

    bool    retval;


    request_t *rq = (request_t*)kmalloc(sizeof(request_t),0);

    INIT_LIST_HEAD(&rq->list);

    rq->data = (addr_t)data;
    rq->size = req_size;
    rq->dev  = dev;

    td0 = dma_pool_alloc(hc->td_pool, 0, &td_dma);
    td0->dma = td_dma;

//    dbgprintf("alloc td0 %x dma %x\n", td0, td_dma);

    td0->status = 0x00800000 | dev->speed;
    td0->token  = TOKEN( 8, DATA0, 0, dev->addr, 0x2D);
    td0->buffer = DMA(req);
    td0->bk     = NULL;

    if(data)
        data_dma = DMA(data);

    td_prev = td0;

    while(size > 0)
    {
        if ( size < packet_size)
        {
            packet_size = size;
        };

        td = dma_pool_alloc(hc->td_pool, 0, &td_dma);
        td->dma = td_dma;

//        dbgprintf("alloc td %x dma %x\n", td, td->dma);

        td_prev->link   = td->dma | 4;
        td->status = TD_CTRL_ACTIVE | dev->speed;
        td->token  = TOKEN(packet_size, toggle, 0,dev->addr, pid);
        td->buffer = data_dma;
        td->bk     = td_prev;

        td_prev = td;

        data_dma+= packet_size;
        size-= packet_size;
        toggle ^= DATA1;
    }

    td = dma_pool_alloc(hc->td_pool, 0, &td_dma);
    td->dma = td_dma;

//    dbgprintf("alloc td %x dma %x\n", td, td->dma);

    td_prev->link   = td->dma | 4;

    pid = (pid == DIN) ? DOUT : DIN;

    td->link = 1;
    td->status = TD_CTRL_ACTIVE | TD_CTRL_IOC | dev->speed ;
    td->token  = (0x7FF<<21)|DATA1|(dev->addr<<8)|pid;
    td->buffer = 0;
    td->bk     = td_prev;

    rq->td_head = td0;
    rq->td_tail = td;

    rq->evh = CreateEvent(NULL, MANUAL_DESTROY);

    if(rq->evh.handle == 0)
        printf("%s: epic fail\n", __FUNCTION__);

    rq->event.code    = 0xFF000001;
    rq->event.data[0] = (addr_t)rq;

    u32_t efl = safe_cli();

    list_add_tail(&rq->list, &dev->host->rq_list);

    qh = dev->host->qh[SKEL_ASYNC];

    qh->qelem = td0->dma;

    mb();

    safe_sti(efl);

    WaitEvent(rq->evh.handle, rq->evh.euid);

    dbgprintf("td0 status 0x%0x\n", td0->status);
    dbgprintf("td  status 0x%0x\n", td->status);

    if( (td0->status & TD_ANY_ERROR) ||
        (td_prev->status & TD_ANY_ERROR) ||
        (td->status & TD_ANY_ERROR))
    {
        u32_t dev_status = in16(dev->host->iobase + USBSTS);

        dbgprintf("\nframe %x, cmd %x status %x\n",
                   in16(dev->host->iobase + USBFRNUM),
                   in16(dev->host->iobase + USBCMD),
                    dev_status);
        dbgprintf("td0 status %x\n",td0->status);
        dbgprintf("td_prev status %x\n",td_prev->status);
        dbgprintf("td status %x\n",td->status);
        dbgprintf("qh %x \n", qh->qelem);

        retval = false;
    } else retval = true;

    qh->qelem = 1;

    mb();

    do
    {
        td_prev = td->bk;
        dma_pool_free(hc->td_pool, td, td->dma);
        td = td_prev;
    }while( td != NULL);

/*
    delete event;
*/
    kfree(rq);

    return retval;
};


bool init_device(udev_t *dev)
{
    static  u16_t __attribute__((aligned(16)))
            req_descr[4]  = {0x0680,0x0100,0x0000,18};

    static  u16_t __attribute__((aligned(16)))
            req_conf[4]  = {0x0680,0x0200,0x0000,9};

    static dev_descr_t __attribute__((aligned(16))) descr;

    interface_descr_t *interface;

    u32_t  data[8];

    u8_t *dptr;
    conf_descr_t      *conf;

    dbgprintf("\ninit device %x, host %x, port %d\n\n",
               dev->id, dev->host->pciId, dev->port);

    if( !ctrl_request(dev, req_descr, DIN, &descr, 18))
    {
        dbgprintf("%s epic fail\n",__FUNCTION__);
        return;
    };

    dev->dev_descr = descr;

    dbgprintf("device descriptor:\n\n"
              "bLength             %d\n"
              "bDescriptorType     %d\n"
              "bcdUSB              %x\n"
              "bDeviceClass        %x\n"
              "bDeviceSubClass     %x\n"
              "bDeviceProtocol     %x\n"
              "bMaxPacketSize0     %d\n"
              "idVendor            %x\n"
              "idProduct           %x\n"
              "bcdDevice           %x\n"
              "iManufacturer       %x\n"
              "iProduct            %x\n"
              "iSerialNumber       %x\n"
              "bNumConfigurations  %d\n\n",
              descr.bLength, descr.bDescriptorType,
              descr.bcdUSB,  descr.bDeviceClass,
              descr.bDeviceSubClass, descr.bDeviceProtocol,
              descr.bMaxPacketSize0, descr.idVendor,
              descr.idProduct, descr.bcdDevice,
              descr.iManufacturer, descr.iProduct,
              descr.iSerialNumber, descr.bNumConfigurations);

    req_conf[3] = 8;
    if( !ctrl_request(dev, req_conf, DIN, &data, 8))
        return;

    conf = (conf_descr_t*)&data;

    size_t conf_size = conf->wTotalLength;

    req_conf[3] = conf_size;
    conf = malloc(conf_size);

    if( !ctrl_request(dev, req_conf, DIN, conf, conf_size))
        return;

    dptr = (u8_t*)conf;
    dptr+= conf->bLength;

    dbgprintf("configuration descriptor\n\n"
              "bLength             %d\n"
              "bDescriptorType     %d\n"
              "wTotalLength        %d\n"
              "bNumInterfaces      %d\n"
              "bConfigurationValue %x\n"
              "iConfiguration      %d\n"
              "bmAttributes        %x\n"
              "bMaxPower           %dmA\n\n",
              conf->bLength,
              conf->bDescriptorType,
              conf->wTotalLength,
              conf->bNumInterfaces,
              conf->bConfigurationValue,
              conf->iConfiguration,
              conf->bmAttributes,
              conf->bMaxPower*2);

        interface = (interface_descr_t*)dptr;

        switch(interface->bInterfaceClass)
        {
            case USB_CLASS_AUDIO:
                dbgprintf( "audio device\n");
                break;
            case USB_CLASS_HID:
                dev->conf = conf;
                list_del(&dev->list);
                return init_hid(dev);

            case USB_CLASS_PRINTER:
                dbgprintf("printer\n");
                break;
            case USB_CLASS_MASS_STORAGE:
                dbgprintf("mass storage device\n");
                break;
            case USB_CLASS_HUB:
                dbgprintf("hub device\n");
                break;
            default:
                dbgprintf("unknown device\n");
        };
};

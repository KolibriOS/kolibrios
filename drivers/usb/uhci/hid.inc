
struct hid_class_descriptor {
    u8_t   bDescriptorType;
    u16_t  wDescriptorLength;
} __attribute__ ((packed));

struct hid_descriptor {
    u8_t   bLength;
    u8_t   bDescriptorType;
    u16_t  bcdHID;
    u8_t   bCountryCode;
    u8_t   bNumDescriptors;

	struct hid_class_descriptor desc[1];
} __attribute__ ((packed));

void create_hid_mouse(udev_t *dev, endpoint_descr_t *en_d);
void create_hid_keyboard(udev_t *dev, endpoint_descr_t *en_d);

bool init_hid(udev_t *dev)
{
    interface_descr_t              *interface;
    struct hid_descriptor          *hds;
    struct hid_class_descriptor    *hidclass;
    u8_t *dptr = (u8_t*)dev->conf;

    int i=0, j=0;

    dbgprintf( "init hid device\n");

    dptr+= dev->conf->bLength;

  //  for(i = 0; i < dev->conf->bNumInterfaces; i++)
  //  {
        interface = (interface_descr_t*)dptr;
        dptr+= interface->bLength;

        dbgprintf("interface           %d\n\n"
                  "bLength             %d\n"
                  "bDescriptorType     %d\n"
                  "bInterfaceNumber    %d\n"
                  "bAlternateSetting   %d\n"
                  "bNumEndpoints       %d\n"
                  "bInterfaceClass     %d\n"
                  "bInterfaceSubClass  %d\n"
                  "bInterfaceProtocol  %d\n"
                  "iInterface          %d\n\n",
                  i+1,
                  interface->bLength,
                  interface->bDescriptorType,
                  interface->bInterfaceNumber,
                  interface->bAlternateSetting,
                  interface->bNumEndpoints,
                  interface->bInterfaceClass,
                  interface->bInterfaceSubClass,
                  interface->bInterfaceProtocol,
                  interface->iInterface);

        hds = (struct hid_descriptor*) dptr;

        dbgprintf("hid descriptor\n\n"
                  "bLength             %d\n"
                  "bDescriptorType     %d\n"
                  "bcdHID              %x\n"
                  "bCountryCode        %d\n"
                  "bNumDescriptors     %d\n",
                  hds->bLength,
                  hds->bDescriptorType,
                  hds->bcdHID,
                  hds->bCountryCode,
                  hds->bNumDescriptors);

        for(j=0; j < hds->bNumDescriptors; j++)
        {
            dbgprintf("bDescriptorType    %d\n"
                      "wDescriptorLength  %d\n",
                      hds->desc[j].bDescriptorType,
                      hds->desc[j].wDescriptorLength);
        };
        dptr+= hds->bLength;

        endpoint_descr_t *ep;
        ep = (endpoint_descr_t*)dptr;

        dbgprintf("\nendpoint\n\n"
                  "bLength             %d\n"
                  "bDescriptorType     %d\n"
                  "bEndpointAddress    %d\n"
                  "bmAttributes        %d\n"
                  "wMaxPacketSize      %d\n"
                  "bInterval           %d\n",
                  ep->bLength, ep->bDescriptorType,
                  ep->bEndpointAddress, ep->bmAttributes,
                  ep->wMaxPacketSize, ep->bInterval);
        dptr+= ep->bLength;

        switch(interface->bInterfaceProtocol)
        {
            case 1:
                create_hid_keyboard(dev, ep);
                break;

            case 2:
                create_hid_mouse(dev, ep);
                break;
        }
  //  }
    return true;
};


bool mouse_handler(udev_t *dev, struct tag_request *rq)
{
    td_t   *td;
    qh_t   *qh;

    td = rq->td_head;

    if( (td->status &0x7FF)==rq->size-1)
    {
        struct boot_packet *pkt;
        pkt = (struct boot_packet *)rq->data;
        SetMouseData(pkt->buttons, pkt->x, -pkt->y, -pkt->z, 0);
        memset(pkt,0, sizeof(*pkt));
    };
    td->status = TD_CTRL_ACTIVE | TD_CTRL_IOC | dev->speed;
    td->token ^= DATA1;

    u32_t efl = safe_cli();
      list_add_tail(&rq->list, &dev->host->rq_list);
      qh = dev->host->qh[rq->qnum];
      qh->qelem = rq->td_head->dma;
      mb();
    safe_sti(efl);

    return true;
};

typedef union
{
    uint16_t value;
    struct
    {
        uint8_t code;
        uint8_t ecode;
    };
}scancode_t;

static const uint16_t usb_keycode[256] =
{
  /*   0       1       2       3       4       5       6       7    */
  /*   8       9       A       B       C       D       E       F    */

    0x0000, 0x0000, 0x0000, 0x0000, 0x001E, 0x0030, 0x002E, 0x0020,   /*  0x00  */
    0x0012, 0x0021, 0x0022, 0x0023, 0x0017, 0x0024, 0x0025, 0x0026,

    0x0032, 0x0031, 0x0018, 0x0019, 0x0010, 0x0013, 0x001F, 0x0014,   /*  0x10  */
    0x0016, 0x002F, 0x0011, 0x002D, 0x0015, 0x002C, 0x0002, 0x0003,

    0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000A, 0x000B,   /*  0x20  */
    0x001C, 0x0001, 0x000E, 0x000F, 0x0039, 0x000C, 0x000D, 0x001A,

    0x001B, 0x002B, 0x002B, 0x0027, 0x0028, 0x0029, 0x0033, 0x0034,   /*  0x30  */
    0x0035, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F, 0x0040,

    0x0041, 0x0042, 0x0043, 0x0044, 0x0057, 0x0058, 0xE037, 0x0046,   /*  0x40  */
    0x0000, 0xE052, 0xE047, 0xE049, 0xE053, 0xE04F, 0xE051, 0xE04D,

    0xE04B, 0xE050, 0xE048, 0x0045, 0xE035, 0x0037, 0x004A, 0x004E,   /*  0x50  */
    0xE01C, 0x004F, 0x0050, 0x0051, 0x004B, 0x004C, 0x004D, 0x0047,

    0x0048, 0x0049, 0x0052, 0x0053, 0x0056, 0xE05D, 0xE05E, 0x0059,   /*  0x60  */
    0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B,

    0x006C, 0x006D, 0x006E, 0x0076, 0x0000, 0x0000, 0x0000, 0x0000,   /*  0x70  */
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,

    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x007E, 0x0000, 0x0073,   /*  0x80  */
    0x0070, 0x007D, 0x0079, 0x007B, 0x005C, 0x0000, 0x0000, 0x0000,

    0x00F2, 0x00F1, 0x0078, 0x0077, 0x0076, 0x0000, 0x0000, 0x0000,   /*  0x90  */
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,

    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,   /*  0xA0  */
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,

    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,   /*  0xB0  */
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,

    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,   /*  0xC0  */
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,

    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,   /*  0xD0  */
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,

    0x001D, 0x002A, 0x0038, 0xE05B, 0xE01D, 0x0036, 0xE038, 0xE05C,   /*  0xE0  */
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,

    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,   /*  0xFE0  */
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,

};

static uint8_t  kbd_old[8];

bool keyboard_handler(udev_t *dev, struct tag_request *rq)
{
    td_t   *td;
    qh_t   *qh;

    uint8_t *data;
    int    i;

    td = rq->td_head;

    if( (td->status &0x7FF)==rq->size-1)
    {
        uint8_t action;
        scancode_t scan;

        data = (uint8_t*)rq->data;

        action = data[0] ^ kbd_old[0];

        for (i = 0; i < 8; i++)
        {
            if(action & 1)
            {
                scan.value = usb_keycode[i+224];

                if(data[0]>>i & 1)
                {
                    if(scan.ecode)
                        SetKeyboardData(scan.ecode);
                    SetKeyboardData(scan.code);
                }
                else
                {
                    if(scan.ecode)
                        SetKeyboardData(scan.ecode);
                    SetKeyboardData(scan.code|0x80);
                };
            }
            action >>= 1;
        }


        for (i = 2; i < 8; i++)
        {
            if ((kbd_old[i] > 3) && (memscan(data+2, kbd_old[i], 6) == data+8))
            {
                scan.value = usb_keycode[kbd_old[i]];

                if (scan.value)
                {
                    if(scan.ecode)
                        SetKeyboardData(scan.ecode);
                    SetKeyboardData(scan.code|0x80);
                  //  input_report_key(kbd->dev, usb_kbd_keycode[kbd->old[i]], 0);
                }
                else
                    dbgprintf("Unknown key (scancode %#x) released.\n", kbd_old[i]);
            }

            if ((data[i] > 3) && (memscan(kbd_old+2, data[i], 6) == kbd_old+8))
            {
                scan.value = usb_keycode[data[i]];

                if (scan.value)
                {
                    if(scan.ecode)
                        SetKeyboardData(scan.ecode);
                    SetKeyboardData(scan.code);
                   //input_report_key(kbd->dev, usb_kbd_keycode[kbd->new[i]], 1);
                }
                else
                    dbgprintf("Unknown key (scancode %#x) released.\n", data[i]);
            }
        }
        memcpy(kbd_old, data, 8);
    };

    td->status = TD_CTRL_ACTIVE | TD_CTRL_IOC | dev->speed;
    td->token ^= DATA1;

    u32_t efl = safe_cli();
      list_add_tail(&rq->list, &dev->host->rq_list);
      qh = dev->host->qh[rq->qnum];
      qh->qelem = rq->td_head->dma;
      mb();
    safe_sti(efl);

    return true;
};

void create_hid_mouse(udev_t *dev, endpoint_descr_t *en_d)
{
    request_t *rq;
    endp_t    enp;

    addr_t  address;
    addr_t  size;
    u32_t   toggle;

    qh_t   *qh;

    static  u16_t __attribute__((aligned(16)))
            req_set_conf[4]  = {0x0900,0x0001,0x0000,0x0000};

    if( !ctrl_request(dev, req_set_conf, DOUT, 0, 0))
        return;

    enp.address = en_d->bEndpointAddress;
    enp.size    = en_d->wMaxPacketSize;
    enp.toggle  = DATA0;

    rq = alloc_rq_buffer(dev, &enp, DIN, enp.size);

    rq->qnum = 6;
    rq->handler = &mouse_handler;

    u32_t efl = safe_cli();
      list_add_tail(&rq->list, &dev->host->rq_list);
      qh = dev->host->qh[rq->qnum];
      qh->qelem = rq->td_head->dma;
      mb();
    safe_sti(efl);

    dbgprintf("%s\n",__FUNCTION__);
}

void create_hid_keyboard(udev_t *dev, endpoint_descr_t *en_d)
{
    request_t *rq;
    endp_t    enp;

    addr_t  address;
    addr_t  size;
    u32_t   toggle;

    qh_t   *qh;

    static  u16_t __attribute__((aligned(16)))
            req_set_conf[4]  = {0x0900,0x0001,0x0000,0x0000};

    if( !ctrl_request(dev, req_set_conf, DOUT, 0, 0))
        return;

    enp.address = en_d->bEndpointAddress;
    enp.size    = en_d->wMaxPacketSize;
    enp.toggle  = DATA0;

    rq = alloc_rq_buffer(dev, &enp, DIN, enp.size);

    rq->qnum = 4;
    rq->handler = &keyboard_handler;

    u32_t efl = safe_cli();
      list_add_tail(&rq->list, &dev->host->rq_list);
      qh = dev->host->qh[rq->qnum];
      qh->qelem = rq->td_head->dma;
      mb();
    safe_sti(efl);

    dbgprintf("%s\n",__FUNCTION__);
}

void *memscan(void *addr, int c, size_t size)
{
    if (!size)
        return addr;
    __asm__ __volatile__(
        "repnz; scasb       \n\t"
        "jnz 1f             \n\t"
        "dec %%edi          \n\t"
    "1:"
        : "=D" (addr), "=c" (size)
        : "D" (addr), "1" (size), "a" (c)
        : "memory");
    return addr;
}


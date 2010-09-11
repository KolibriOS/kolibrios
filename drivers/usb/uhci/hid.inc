
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

        if( interface->bInterfaceProtocol == 2)
            create_hid_mouse(dev, ep);
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
    };
    td->status = TD_CTRL_ACTIVE | TD_CTRL_IOC | dev->speed;
    td->token ^= DATA1;

    u32_t efl = safe_cli();
      list_add_tail(&rq->list, &dev->host->rq_list);
      qh = dev->host->qh[6];
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

    void *packet;

    td_t   *td;
    qh_t   *qh;

    static  u16_t __attribute__((aligned(16)))
            req_set_conf[4]  = {0x0900,0x0001,0x0000,0x0000};

    if( !ctrl_request(dev, req_set_conf, DOUT, 0, 0))
        return;

    enp.address = en_d->bEndpointAddress;
    enp.size    = en_d->wMaxPacketSize;
    enp.toggle  = DATA0;

    packet = kzalloc(enp.size, 0);

    rq = create_request(dev, &enp, DIN, packet, enp.size);
    rq->qnum = 6;
    rq->handler = &mouse_handler;

    u32_t efl = safe_cli();
      list_add_tail(&rq->list, &dev->host->rq_list);
      qh = dev->host->qh[6];
      qh->qelem = rq->td_head->dma;
      mb();
    safe_sti(efl);

    dbgprintf("create_hid_mouse\n");
}

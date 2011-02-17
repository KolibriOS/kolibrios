

#include <ddk.h>
#include <mutex.h>
#include <pci.h>
#include <linux/dmapool.h>
#include <linux/string.h>
#include <syscall.h>
#include "usb.h"

int __stdcall srv_usb(ioctl_t *io);

bool init_hc(hc_t *hc);

static slab_t   qh_slab;

LIST_HEAD( hc_list );
LIST_HEAD( newdev_list );
LIST_HEAD( rq_list );

u32_t drvEntry(int action, char *cmdline)
{
    u32_t   retval;
    hc_t   *hc;
    udev_t *dev;

    int     i;

    if(action != 1)
        return 0;

    if(!dbg_open("/rd/1/drivers/usb.log"))
    {
        printf("Can't open /rd/1/drivers/usb.log\nExit\n");
        return 0;
    }

    if( !FindUSBControllers() ) {
        dbgprintf("no uhci devices found\n");
        return 0;
    };

    hcd_buffer_create();

    qh_slab.available = 256;
    qh_slab.start     = KernelAlloc(4096);
    qh_slab.nextavail = (addr_t)qh_slab.start;
    qh_slab.dma       = GetPgAddr(qh_slab.start);

    qh_t    *p;
    addr_t  dma;

    for (i = 0, p = (qh_t*)qh_slab.start, dma = qh_slab.dma;
         i < 256; i++, p++, dma+= sizeof(qh_t))
    {
       p->qlink  = (addr_t)(p+1);
       p->qelem  = 1;
       p->dma    = dma;
       p->r1     = 0;
    };

    hc = (hc_t*)hc_list.next;

    while( &hc->list != &hc_list)
    {
        hc_t *tmp = hc;
        hc = (hc_t*)hc->list.next;

        if( !init_hc(tmp))
            list_del(&tmp->list);
    };

    dbgprintf("\n");

    dev = (udev_t*)newdev_list.next;
    while( &dev->list != &newdev_list)
    {
        udev_t *tmp = dev;
        dev = (udev_t*)dev->list.next;

        if(tmp->id != 0)
            init_device(tmp);
    }

    while(1)
    {
        udev_t    *dev;
        request_t *rq;
        kevent_t    event;
        u32_t       handle;

        event.code    = 0;
        event.data[0] = 0;

        handle = GetEvent(&event);

//        dbgprintf("event handle 0x%0x code 0x%0x\n",
//                   handle, event.code);

        if(event.code != 0xFF000001)
            continue;

        rq = (request_t*)event.data[0];

//        dbgprintf("rq = 0x%0x\n", rq);

        rq->handler(rq->dev, rq);
    };

    retval = RegService("USB", srv_usb);
    dbgprintf("reg service USB as: %x\n", retval);

    return retval;
};


#define API_VERSION     0x01000100

#define SRV_GETVERSION  0


int __stdcall srv_usb(ioctl_t *io)
{
  u32_t *inp;
  u32_t *outp;

  inp = io->input;
  outp = io->output;

  switch(io->io_code)
  {
    case SRV_GETVERSION:
      if(io->out_size==4)
      {
        *outp = API_VERSION;
        return 0;
      }
      break;


    default:
      return ERR_PARAM;
  };
  return ERR_PARAM;
}


static qh_t* alloc_qh()
{
    if( qh_slab.available )
    {
        qh_t *qh;

        qh_slab.available--;
        qh = (qh_t*)qh_slab.nextavail;
        qh_slab.nextavail = qh->qlink;
        return qh;
     }
     return NULL;
};

static void  free_qh(qh_t *qh)
{
     qh->qlink = qh_slab.nextavail;
     qh_slab.nextavail = (addr_t)qh;
     qh_slab.available++;
};


#include "pci.inc"
#include "detect.inc"
#include "hcd.inc"
#include "hid.inc"

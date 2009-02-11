

#include "types.h"
#include "link.h"

#include <stdio.h>
#include <malloc.h>
#include <memory.h>

#include "pci.h"

#include "syscall.h"
#include "usb.h"

static Bool FindPciDevice();

int __stdcall srv_usb(ioctl_t *io);

Bool init_hc(hc_t *hc);

static slab_t   qh_slab;
static slab_t   td_slab;

static link_t  hc_list;
static link_t  newdev_list;
static link_t  rq_list;

u32_t __stdcall drvEntry(int action)
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

    list_initialize(&hc_list);
    list_initialize(&newdev_list);
    list_initialize(&rq_list);

    if( !FindPciDevice() ) {
        dbgprintf("no uhci devices found\n");
        return 0;
    };

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

     td_slab.available = 128;
     td_slab.start     = KernelAlloc(4096);
     td_slab.nextavail = (addr_t)td_slab.start;
     td_slab.dma       = GetPgAddr(td_slab.start);

     td_t *td;
     for (i = 0, td = (td_t*)td_slab.start, dma = td_slab.dma;
          i < 128; i++, td++, dma+= sizeof(td_t))
     {
        td->link   = (addr_t)(td+1);
        td->status = 0;
        td->token  = 0;
        td->buffer = 0;
        td->dma    = dma;
     };


    hc = (hc_t*)hc_list.next;

    while( &hc->link != &hc_list)
    {
        init_hc(hc);
        hc = (hc_t*)hc->link.next;
    }

    dbgprintf("\n");

    dev = (udev_t*)newdev_list.next;
    while( &dev->link != &newdev_list)
    {
        udev_t *tmp = dev;
        dev = (udev_t*)dev->link.next;

        if(tmp->id != 0)
            init_device(tmp);
    }

    while(1)
    {
        udev_t    *dev;
        request_t *rq;

        rq = (request_t*)rq_list.next;
        while( &rq->link != &rq_list)
        {
            qh_t      *qh;
            td_t      *td;

            td  = rq->td_head;
            dev = rq->dev;
            qh  = dev->host->qh1;

            qh->qelem = td->dma;

            __asm__ __volatile__ ("":::"memory");
            rq = (request_t*)rq->link.next;
        };

        delay(10/10);

        rq = (request_t*)rq_list.next;
        while( &rq->link != &rq_list)
        {
            request_t *tmp;
            td_t      *td;

            tmp = rq;
            rq = (request_t*)rq->link.next;

            td  = tmp->td_head;

            if( td->status & TD_CTRL_ACTIVE)
                continue;

            tmp->handler(tmp->dev, tmp);
        };
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

static td_t* alloc_td()
{
    if( td_slab.available )
    {
        td_t *td;

        td_slab.available--;
        td = (td_t*)td_slab.nextavail;
        td_slab.nextavail = td->link;
        return td;
     }
     return NULL;
};

static void  free_td(td_t *td)
{
     td->link = td_slab.nextavail;
     td_slab.nextavail = (addr_t)td;
     td_slab.available++;
};

#include "pci.inc"
#include "detect.inc"
#include "hcd.inc"
#include "hid.inc"

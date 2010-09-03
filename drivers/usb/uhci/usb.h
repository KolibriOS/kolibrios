
typedef struct list_head  list_t;


typedef struct {
  int     available;          /**< Count of available items in this slab. */
  void   *start;              /**< Start address of first item. */
  addr_t  nextavail;          /**< The index of next available item. */
  addr_t  dma;
} slab_t;


#define USB_CLASS_AUDIO                    1
#define USB_CLASS_COMM                     2
#define USB_CLASS_HID                      3
#define USB_CLASS_PHYSICAL                 5
#define USB_CLASS_STILL_IMAGE              6
#define USB_CLASS_PRINTER                  7
#define USB_CLASS_MASS_STORAGE             8
#define USB_CLASS_HUB                      9
#define USB_CLASS_CDC_DATA              0x0a
#define USB_CLASS_CSCID                 0x0b    /* chip+ smart card */
#define USB_CLASS_CONTENT_SEC           0x0d    /* content security */
#define USB_CLASS_VIDEO                 0x0e
#define USB_CLASS_WIRELESS_CONTROLLER	0xe0
#define USB_CLASS_MISC                  0xef
#define USB_CLASS_APP_SPEC              0xfe
#define USB_CLASS_VENDOR_SPEC           0xff


typedef struct
{
    addr_t  qlink;
    addr_t  qelem;

    addr_t  dma;
    u32_t   r1;

}qh_t __attribute__((aligned(16)));

typedef struct
{
    list_t  list;

    addr_t  iobase;

    u32_t   *frame_base;
    count_t frame_number;
    addr_t  frame_dma;

    qh_t   *qh1;

    u32_t  *data;
    addr_t  data_dma;

    u32_t   port_map;

    int     numports;

    u32_t   pciId;
    PCITAG  PciTag;
    addr_t  ioBase[6];
    addr_t  memBase[6];
    size_t  memSize[6];
    u32_t   memType[6];
}hc_t;

typedef struct tag_td
{
	/* Hardware fields */
    addr_t   link;
    u32_t    status;
    u32_t    token;
    addr_t   buffer;

	/* Software fields */
    addr_t   dma;

    struct   tag_td *bk;

  //  struct list_head list;

  //  int frame;          /* for iso: what frame? */
  //  struct list_head fl_list;

    u32_t    reserved[2];
} td_t __attribute__((aligned(16)));

#define TD_CTRL_SPD         (1 << 29)   /* Short Packet Detect */
#define TD_CTRL_C_ERR_MASK	(3 << 27)	/* Error Counter bits */
#define TD_CTRL_C_ERR_SHIFT	27
#define TD_CTRL_LS          (1 << 26)   /* Low Speed Device */
#define TD_CTRL_IOS         (1 << 25)   /* Isochronous Select */
#define TD_CTRL_IOC         (1 << 24)   /* Interrupt on Complete */
#define TD_CTRL_ACTIVE		(1 << 23)	/* TD Active */
#define TD_CTRL_STALLED		(1 << 22)	/* TD Stalled */
#define TD_CTRL_DBUFERR		(1 << 21)	/* Data Buffer Error */
#define TD_CTRL_BABBLE		(1 << 20)	/* Babble Detected */
#define TD_CTRL_NAK         (1 << 19)   /* NAK Received */
#define TD_CTRL_CRCTIMEO	(1 << 18)	/* CRC/Time Out Error */
#define TD_CTRL_BITSTUFF    (1 << 17)   /* Bit Stuff Error */

#define TD_ANY_ERROR   (TD_CTRL_STALLED | TD_CTRL_DBUFERR  | \
                        TD_CTRL_BABBLE  | TD_CTRL_CRCTIMEO | \
                        TD_CTRL_BITSTUFF)

typedef struct __attribute__ ((packed))
{
    u8_t    bLength;
    u8_t    bDescriptorType;
    u16_t   bcdUSB;

    u8_t    bDeviceClass;
    u8_t    bDeviceSubClass;
    u8_t    bDeviceProtocol;
    u8_t    bMaxPacketSize0;

    u16_t   idVendor;
    u16_t   idProduct;
    u16_t   bcdDevice;

    u8_t    iManufacturer;
    u8_t    iProduct;
    u8_t    iSerialNumber;
    u8_t    bNumConfigurations;
}dev_descr_t;

typedef struct  __attribute__ ((packed))
{
    u8_t    bLength;
    u8_t    bDescriptorType;
    u16_t   wTotalLength;
    u8_t    bNumInterfaces;
    u8_t    bConfigurationValue;
    u8_t    iConfiguration;
    u8_t    bmAttributes;
    u8_t    bMaxPower;
}conf_descr_t;

typedef struct __attribute__ ((packed))
{
    u8_t    bLength;
    u8_t    bDescriptorType;
    u8_t    bInterfaceNumber;
    u8_t    bAlternateSetting;
    u8_t    bNumEndpoints;
    u8_t    bInterfaceClass;
    u8_t    bInterfaceSubClass;
    u8_t    bInterfaceProtocol;
    u8_t    iInterface;
}interface_descr_t ;

typedef struct __attribute__ ((packed))
{
    u8_t    bLength;
    u8_t    bDescriptorType;

    u8_t    bEndpointAddress;
    u8_t    bmAttributes;
    u16_t   wMaxPacketSize;
    u8_t    bInterval;

	/* NOTE:  these two are _only_ in audio endpoints. */
	/* use USB_DT_ENDPOINT*_SIZE in bLength, not sizeof. */
    u8_t    bRefresh;
    u8_t    bSynchAddress;
}endpoint_descr_t;

typedef struct
{
    addr_t  address;
    addr_t  size;
    u32_t   toggle;

}endp_t;

typedef struct __attribute__ ((packed))
{
    u8_t    bRequestType;
    u8_t    bRequest;
    u16_t   wValue;
    u16_t   wIndex;
    u16_t   wLength;
}ctrl_request_t;


typedef struct
{
    list_t  list;
    u32_t   id;

    hc_t   *host;

    u32_t   speed;
    addr_t  addr;

    addr_t  ep0_size;

    endp_t  enp;

    u32_t   status;
    int     port;

    dev_descr_t   dev_descr;
    conf_descr_t  *conf;
}udev_t;

typedef struct   tag_request
{
    list_t        list;
    td_t         *td_head;
    td_t         *td_tail;
    addr_t        data;
    size_t        size;
    udev_t       *dev;
    u32_t         type;
    bool        (*handler)(udev_t *dev, struct   tag_request *rq);
}request_t;


#define  DMA(val) GetPgAddr(val)|(((addr_t)(val))&0xFFF)

#define  TOKEN( size, toggle, ep, addr, pid) \
         ( (((size)-1)<<21)|(toggle)|(((ep)&0xF)<<15)|((addr)<<8)|(pid))

bool FindUSBControllers();

bool ctrl_request(udev_t *dev, void *req, u32_t dir,
                  void *data, size_t req_size);

bool set_address(udev_t *dev);

bool init_device(udev_t *dev);

bool init_hid(udev_t *dev);

struct boot_packet
{
    u8_t  buttons;
    char  x;
    char  y;
    char  z;
}__attribute__ ((packed));

#define DOUT   0xE1
#define DIN    0x69

#define DATA0  (0<<19)
#define DATA1  (1<<19)


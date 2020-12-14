#include<menuet/os.h>

#define PCI_FN		62

void get_pci_version(__u8 * major,__u8 * minor)
{
 int r;
 __asm__ __volatile__("int $0x40":"=a"(r):"0"(PCI_FN),"b"(0));
 *minor=r&0xFF;
 *major=(r>>8)&0xFF;
}

void pci_get_last_bus(__u8 * last_bus)
{
 __asm__ __volatile__("int $0x40":"=a"(*last_bus):"0"(PCI_FN),"b"(1));
}

void get_pci_access_mechanism(__u8 * mechanism)
{
 __asm__ __volatile__("int $0x40":"=a"(*mechanism):"0"(PCI_FN),"b"(2));
}

#define read_config(x,c,bits) \
    __u##bits pci_read_config_##x (__u8 bus,__u8 dev,__u8 fn,__u8 reg) \
    { \
      __u##bits __ret; \
      __u16 cx; \
      __u16 bx; \
      cx=(((fn&7)|(((dev)&~7)<<3))<<8)|reg; \
      bx=(bus<<8)|(c); \
      __asm__ __volatile__("int $0x40":"=a"(__ret):"0"(PCI_FN),"b"(bx),"c"(cx)); \
      return __ret; \
     }

#define write_config(x,c,bits) \
    void pci_write_config_##x (__u8 bus,__u8 dev,__u8 fn,__u8 reg,__u##bits val) \
    { \
     __u16 cx,bx; \
     cx=(((fn&7)|(((dev)&~7)<<3))<<8)|reg; \
     bx=(bus<<8)|(c); \
     __asm__ __volatile__("int $0x40"::"a"(PCI_FN),"b"(bx),"c"(cx),"d"(val)); \
    }

#define rw_config(x,c,bits) \
    read_config(x,4+c,bits) \
    write_config(x,7+c,bits)
        
rw_config(byte,0,8)
rw_config(word,1,16)
rw_config(dword,2,32)

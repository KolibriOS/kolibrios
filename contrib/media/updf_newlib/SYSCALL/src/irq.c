#include<menuet/os.h>

__u32 __menuet__get_irq_owner(__u32 irq)
{
 __u32 __ret;
 __asm__ __volatile__("int $0x40":"=a"(__ret):"0"(41),"b"(irq));
 return __ret;
}

int __menuet__get_data_read_by_irq(__u32 irq,__u32 * num_bytes_in_buf,__u8 * data)
{
 __u32 a,b,c;
 __asm__ __volatile__("int $0x40":"=a"(a),"=b"(b),"=c"(c):"0"(42),"1"(irq));
 if(num_bytes_in_buf) *num_bytes_in_buf=a;
 if(data) *data=b & 0xFF;
 return c;
}

int __menuet__send_data_to_device(__u16 port,__u8 val)
{
 int __ret;
 __asm__ __volatile__("int $0x40":"=a"(__ret):"0"(43),"b"(port),"c"(val));
 return __ret;
}

void __menuet__program_irq(void * intr_table,__u32 irq_num)
{
 __asm__ __volatile__("int $0x40"::"a"(44),"b"(intr_table),"c"(irq_num));
}

int __menuet__reserve_irq(int irqno)
{
 int __ret;
 __asm__ __volatile__("int $0x40":"=a"(__ret):"0"(45),"b"(0),"c"(irqno));
 return __ret;
}

int __menuet__free_irq(int irqno)
{
 int __ret;
 __asm__ __volatile__("int $0x40":"=a"(__ret):"0"(45),"b"(1),"c"(irqno));
 return __ret;
}

int __menuet__reserve_port_area(__u32 start,__u32 end)
{
 int __ret;
 __asm__("int $0x40":"=a"(__ret):"a"(45),"b"(0),"c"(start),"d"(end));
 return __ret;
}

int __menuet__free_port_area(__u32 start,__u32 end)
{
 int __ret;
 __asm__("int $0x40":"=a"(__ret):"a"(45),"b"(1),"c"(start),"d"(end));
 return __ret;
}

void __menuet__define_app_internal_intrs(void * intr_table)
{
 __asm__("int $0x40"::"a"(49),"b"(0),"c"(intr_table));
}


struct process_table_entry_
{
 __u32 cpu_usage __attribute__((packed));
 __u16 pos_in_windowing_stack __attribute__((packed));
 __u16 win_stack_val_at_ecx __attribute__((packed));
 __u16 rez1 __attribute__((packed));
 char name[11] __attribute__((packed));
 __u8 rez2 __attribute__((packed));
 __u32 memstart __attribute__((packed));
 __u32 memused __attribute__((packed));
 __u32 pid __attribute__((packed));
 __u32 winx_start,winy_start __attribute__((packed));
 __u32 winx_size,winy_size __attribute__((packed));
 __u8  slot __attribute__((packed));
 __u8 rez3 __attribute__((packed));
 __u32 clarx_start,clary_start __attribute__((packed));
 __u32 clarx_size,clary_size __attribute__((packed));
 __u8 win_condition __attribute__((packed));
 __u8 buf[955] __attribute__((packed));
}  __attribute__((packed));

#define TYPEWIN(D,C,B,A,Y,RR,GG,BB) (D<<31)|(C<<30)|(B<<29)|(A<<28)|(Y<<24)|\
(RR<<16)|(GG<<8)|BB

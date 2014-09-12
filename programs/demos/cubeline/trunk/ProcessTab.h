
#pragma pack(push, 1)
struct process_table_entry_
{
 __u32 cpu_usage;
 __u16 pos_in_windowing_stack;
 __u16 win_stack_val_at_ecx;
 __u16 rez1;
 char name[11];
 __u8 rez2;
 __u32 memstart;
 __u32 memused;
 __u32 pid;
 __u32 winx_start,winy_start;
 __u32 winx_size,winy_size;
 __u8  slot;
 __u8 rez3;
 __u32 clarx_start,clary_start;
 __u32 clarx_size,clary_size;
 __u8 win_condition;
 __u8 buf[955];
};
#pragma pack(pop)

#define TYPEWIN(D,C,B,A,Y,RR,GG,BB) (D<<31)|(C<<30)|(B<<29)|(A<<28)|(Y<<24)|\
(RR<<16)|(GG<<8)|BB

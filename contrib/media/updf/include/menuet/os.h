#ifndef __GLIBC__MENUET_OS_H
#define __GLIBC__MENUET_OS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char       __u8;
typedef unsigned short      __u16;
typedef unsigned long       __u32;

#pragma pack(push,1)

void __menuet__define_window(__u16 x1,__u16 y1,__u16 xsize,__u16 ysize,
     __u32 body_color,__u32 grab_color,__u32 frame_color);
void __menuet__window_redraw(int status);
void __menuet__putpixel(__u32 x,__u32 y,__u32 color);
int __menuet__getkey(void);
__u32 __menuet__getsystemclock(void);
void __menuet__write_text(__u16 x,__u16 y,__u32 color,char * text,int len);
void __menuet__delay100(int m);
__u32 __menuet__open(char * name,char * data);
void __menuet__save(char * name,char * data,__u32 count);
void __menuet__putimage(__u16 x1,__u16 y1,__u16 xsize,__u16 ysize,char * image);
void __menuet__make_button(__u16 x1,__u16 y1,__u16 xsize,__u16 ysize, int id,__u32 color);
int __menuet__get_button_id(void);
int __menuet__wait_for_event(void);
int __menuet__check_for_event(void);
void __menuet__bar(__u16 x1,__u16 y1,__u16 xsize,__u16 ysize,__u32 color);
void __menuet__sys_exit(void);
void * __menuet__exec_thread(void (* func_ptr)(void),__u32 stack_size,int * retp);
void __menuet__idle(void);

void __menuet__exec_ramdisk(char * filename,char * args,...);
void __menuet__exec_hd(char * filename,char * args,...);

struct process_table_entry
{
 __u32 cpu_usage;
 __u16 pos_in_windowing_stack;
 __u16 win_stack_val_at_ecx;
 __u16 reserved1;
 char name[12];
 __u32 memstart;
 __u32 memused;
 __u32 pid;
 __u32 winx_start,winy_start;
 __u32 winx_size,winy_size;
 __u16 thread_state;
 __u16 reserved2;
 __u32 client_left,client_top,client_width,client_height;
 __u8 window_state;
 __u8 reserved3[1024-71];
};

#define PID_WHOAMI		(-1)

int __menuet__get_process_table(struct process_table_entry * proctab,int pid);
void __menuet__get_screen_max(__u16 * x,__u16 * y);

#define BTYPE_TILE                  1
#define BTYPE_STRETCH               2

void __menuet__set_background_size(__u32 xsz,__u32 ysz);
void __menuet__write_background_mem(__u32 pos,__u32 color);
void __menuet__draw_background(void);
void __menuet__set_background_draw_type(int type);
void __menuet__background_blockmove(char * src_ptr,__u32 bgr_dst,__u32 count);

void __menuet__reset_mpu401(void);
void __menuet__write_mpu401(__u8 d);

__u32 __menuet__get_date(void);

void __menuet__line(__u16 x1,__u16 y1,__u16 x2,__u16 y2,__u32 color);

void __menuet__set_bitfield_for_wanted_events(__u16 ev);

#define EVENT_REDRAW              0x00000001
#define EVENT_KEY                 0x00000002
#define EVENT_BUTTON              0x00000004
#define EVENT_END_REQUEST         0x00000008
#define EVENT_DESKTOP_BACK_DRAW   0x00000010
#define EVENT_MOUSE_CHANGE        0x00000020
#define EVENT_IPC		  0x00000040
#define EVENT_GET_IRQS_MASK       0xFFFF0000
#define EVENT_GET_IRQ(e)          (((e)>>16)&0xFFFF)

__u32 __menuet__get_irq_owner(__u32 irq);
int __menuet__get_data_read_by_irq(__u32 irq,__u32 * num_bytes_in_buf,__u8 * data);
int __menuet__send_data_to_device(__u16 port,__u8 val);
void __menuet__program_irq(void * intr_table,__u32 irq_num);
int __menuet__reserve_irq(int irqno);
int __menuet__free_irq(int irqno);
int __menuet__reserve_port_area(__u32 start,__u32 end);
int __menuet__free_port_area(__u32 start,__u32 end);


#define NAME_LEN                   512

#define STC_READ                  0
#define STC_WRITE                 1
#define STC_APPEND                2

struct systree_info
{
	__u32 command;
	__u32 file_offset_low;
	__u32 file_offset_high;
	__u32 size;
	__u32 data_pointer;
	char _zero;
	const char* nameptr;
};

struct systree_info2
{
	__u32 command;
	__u32 file_offset_low;
	__u32 file_offset_high;
	__u32 size;
	__u32 data_pointer;
	char name[NAME_LEN];
};

struct bdfe_time
{
	__u8 seconds;
	__u8 minutes;
	__u8 hours;
	__u8 reserved;
};
struct bdfe_date
{
	__u8 day;
	__u8 month;
	__u16 year;
};
struct bdfe_item
{
	__u32 attr;
	__u8 nametype;
	__u8 reserved[3];
	struct bdfe_time ctime;
	struct bdfe_date cdate;
	struct bdfe_time atime;
	struct bdfe_date adate;
	struct bdfe_time mtime;
	struct bdfe_date mdate;
	__u32 filesize_low;
	__u32 filesize_high;
};

int __kolibri__system_tree_access(struct systree_info * info);
int __kolibri__system_tree_access2(struct systree_info2 * info);

int __fslayer_open(char * name,int flags);
int __fslayer_close(int fd);
int __fslayer_lseek(int fd,int pos,int seek_type);
int __fslayer_tell(int fd);
int __fslayer_read(int fd,void * buffer,__u32 count);
int __fslayer_write(int fd,void * buffer,__u32 count);

typedef struct
{
 __u8 lock;
 __u8 resvd[3];
 __u32 ptr_to_fmsg_pos;
 /* Below is for message */
 __u32 sender_pid;
 __u32 msg_length;
} msgrcva_t /*__attribute__((packed))*/;

void send_message(int pid,void * msg_ptr,int message_size);
void define_receive_area(msgrcva_t * rcva_ptr,int size);

void __menuet__sound_load_block(char * blockptr);
void __menuet__sound_play_block(void);

void __menuet__dga_get_caps(int * xres,int * yres,int * bpp,int * bpscan);

void get_pci_version(__u8 * major,__u8 * minor);
void pci_get_last_bus(__u8 * last_bus);
void get_pci_access_mechanism(__u8 * mechanism);

void pci_write_config_byte(__u8 bus,__u8 dev,__u8 fn,__u8 reg,__u8 val);
void pci_write_config_word(__u8 bus,__u8 dev,__u8 fn,__u8 reg,__u16 val);
void pci_write_config_dword(__u8 bus,__u8 dev,__u8 fn,__u8 reg,__u32 val);
__u8 pci_read_config_byte(__u8 bus,__u8 dev,__u8 fn,__u8 reg);
__u16 pci_read_config_word(__u8 bus,__u8 dev,__u8 fn,__u8 reg);
__u32 pci_read_config_dword(__u8 bus,__u8 dev,__u8 fn,__u8 reg);

typedef struct{
	char* name;
	void* pointer;
} IMP_ENTRY;
typedef const IMP_ENTRY* IMP_TABLE;
IMP_TABLE __kolibri__cofflib_load(const char*);
__u32 __kolibri__cofflib_getproc(IMP_TABLE, const char*);


static __inline__ void __menuet__debug_out_byte(const char ch){
    __asm__ __volatile__ ("int $0x40"::"a"(63L),"b"(1L),"c"((__u8)ch));
}

static __inline__ __u32 __menuet__wtf(void){
 __u32 __ret;
 __asm__ __volatile__("int $0x40":"=a"(__ret):"0"(26),"b"(9));
 return __ret;
   }

void __menuet__debug_out(const char* str);

#define TIME_GETH(x)	((x)&0x000000FF)
#define TIME_GETM(x)	((((x)&0x00FF00)>>8)&0xFF)
#define TIME_GETS(x)	((((x)&0xFF0000)>>16)&0xFF)

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif

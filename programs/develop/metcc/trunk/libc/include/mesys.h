#ifndef mesys_h
#define mesys_h
#ifdef GNUC
#define stdcall __stdcall
#define cdecl __cdecl
#else
#define stdcall __attribute__ ((__stdcall))
#define cdecl  __attribute__ ((__cdecl))
#endif

typedef unsigned long dword;
typedef unsigned char byte;
typedef unsigned short word;

extern void stdcall _msys_draw_window(int xcoord,int ycoord, int xsize,
			       int ysize,int workcolor,int type,
			       int captioncolor,int windowtype,int bordercolor);
extern int  stdcall _msys_read_file(char* filename,int fileoffset,int size,void* data,
                     int* filesize);
extern int  stdcall _msys_write_file(char* filename,int fileoffset, int size, void* data);
extern int  stdcall _msys_create_file(char* filename);
extern void stdcall _msys_run_program(char* filename,char* parameters);
extern void stdcall _msys_debug_out(int c);
extern void debug_out_str(char* str);
extern void stdcall _msys_set_background_size(int xsize,int ysize);
extern void stdcall _msys_write_background_mem(int pos,int color);
extern void stdcall _msys_draw_background(void);
extern void stdcall _msys_set_background_draw_type(int type);
extern void stdcall _msys_background_blockmove(void* src,int bgr_pos, int count);
extern void stdcall _msys_draw_bar(int x, int y, int xsize, int ysize, int color);
extern void stdcall _msys_make_button(int x, int y, int xsize, int ysize, int id, int color);
extern int  stdcall _msys_get_button_id(void);
extern int  stdcall _msys_get_system_clock(void);
extern int  stdcall _msys_get_date(void);
extern void stdcall _msys_delay(int m);
extern void stdcall _msys_dga_get_resolution(int* xres, int* yres, int* bpp, int* bpscan);
extern int  stdcall _msys_wait_for_event_infinite(void);
extern int  stdcall _msys_check_for_event(void);
extern int  stdcall _msys_wait_for_event(int time);
extern void stdcall _msys_set_wanted_events(int ev);
extern void stdcall _msys_exit(void);
extern void stdcall _msys_putimage(int x, int y, int xsize, int ysize, void* image);
extern void stdcall _msys_send_message(int pid, void* msg, int size);
extern void stdcall _msys_define_receive_area(void* area, int size);
extern int  stdcall _msys_get_irq_owner(int irq);
extern int  stdcall _msys_get_data_read_by_irq(int irq, int* size, void* data);
extern int  stdcall _msys_send_data_to_device(int port, unsigned char val);
extern int  stdcall _msys_receive_data_from_device(int port,unsigned char* data);
extern void stdcall _msys_program_irq(void* intrtable, int irq);
extern void stdcall _msys_reserve_irq(int irq);
extern void stdcall _msys_free_irq(int irq);
extern int  stdcall _msys_reserve_port_area(int start,int end);
extern int  stdcall _msys_free_port_area(int start,int end);
extern int  stdcall _msys_get_key(void);
extern void stdcall _msys_set_keyboard_mode(int mode);
extern void stdcall _msys_line(int x1,int y1,int x2,int y2,int color);
extern void stdcall _msys_midi_reset(void);
extern void stdcall _msys_midi_send(int data);
extern int  stdcall _msys_get_pci_version(void);
extern int  stdcall _msys_get_last_pci_bus(void);
extern int  stdcall _msys_get_pci_access_mechanism(void);
extern int  stdcall _msys_pci_read_config_byte(int bus,int dev,int fn,int reg);
extern int  stdcall _msys_pci_read_config_word(int bus,int dev,int fn,int reg);
extern int  stdcall _msys_pci_read_config_dword(int bus,int dev,int fn,int reg);
extern int  stdcall _msys_pci_write_config_byte(int bus,int dev,int fn,int reg,int value);
extern int  stdcall _msys_pci_write_config_word(int bus,int dev,int fn,int reg,int value);
extern int  stdcall _msys_pci_write_config_value(int bus,int dev,int fn,int reg,int value);
extern int  stdcall _msys_putpixel(int x,int y,int color);
#pragma pack(push,1)
typedef struct {
  int cpu_usage;             //+0
  int window_pos_info;       //+4
  short int reserved1;       //+8
  char name[12];             //+10
  int memstart;              //+22
  int memused;               //+26
  int pid;                   //+30
  int winx_start;            //+34
  int winy_start;            //+38
  int winx_size;             //+42
  int winy_size;             //+46
  short int slot_info;       //+50
  short int reserved2;       //+52
  int clientx;               //+54
  int clienty;               //+58
  int clientwidth;           //+62
  int clientheight;          //+66
  unsigned char window_state;//+70
  char reserved3[1024-71];   //+71
} process_table_entry;
#pragma pack(pop)
extern int  stdcall _msys_get_process_table(process_table_entry* proctab,int pid);
extern int  stdcall _msys_get_screen_size(int* x,int* y);
extern void stdcall _msys_sound_load_block(void* blockptr);
extern void stdcall _msys_sound_play_block(void);
extern void stdcall _msys_sound_set_channels(int channels);
extern void stdcall _msys_sound_set_data_size(int size);
extern void stdcall _msys_sound_set_frequency(int frequency);
extern void stdcall _msys_sound_speaker_play(void* data);
extern void stdcall _msys_write_text(int x,int y,int color,char* text,int len);
extern void* stdcall  _msys_start_thread(void (* func_ptr)(void),int stack_size,int* pid);
extern void stdcall _msys_window_redraw(int status);
extern void* malloc(int);
extern void  free(void*);
extern void* realloc(void*,int);

extern dword* stdcall _msys_cofflib_load(char* name);
extern char* stdcall _msys_cofflib_getproc(void* exp,char* sz_name);

#endif

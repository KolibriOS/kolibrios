#ifndef kolibrisys_h
#define kolibrisys_h
/*
#ifdef GNUC
#define stdcall __stdcall
#define cdecl __cdecl
#else
#define stdcall  ((__stdcall))
#define cdecl    ((__cdecl))
#endif
*/
//#ifdef GNUC
//#define stdcall __stdcall
//#else
#define cdecl   __attribute__ ((cdecl))
#define stdcall __attribute__ ((stdcall))
//#endif

typedef unsigned int dword;
typedef unsigned char byte;
typedef unsigned short word;

typedef unsigned int fpos_t;
typedef unsigned int size_t;

typedef struct process_table_entry{
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
}__attribute__((packed));

//-----------------------------------------------------------------------------------
//------------------------KolibriOS system acces to files----------------------------
//-----------------------------------------------------------------------------------
extern dword stdcall _ksys_get_filesize(char *filename);
extern dword stdcall _ksys_readfile(char *filename,dword pos,dword blocksize,void *data);
extern dword stdcall _ksys_rewritefile(char *filename,dword blocksize,void *data);
extern dword stdcall _ksys_appendtofile(char *filename,dword pos,dword blocksize,void *data);
//-----------------------------------------------------------------------------------

//----------------------Run program---------------------------------------------------
extern void stdcall _ksys_run_program(char* filename,char* parameters);
//------------------------------------------------------------------------------------

//--------------------Debug output---------------------------------------------------
extern void stdcall _ksys_debug_out(int c);
extern void stdcall debug_out_str(char* str);
//-----------------------------------------------------------------------------------

//--------------------------Mouse state----------------------------------------------
extern int   stdcall _ksys_GetMouseXY(void);
extern int   stdcall _ksys_GetMouseButtonsState(void);
//-----------------------------------------------------------------------------------

//--------------------------get skin height------------------------------------------
extern int   stdcall _ksys_get_skin_height(void);
//-----------------------------------------------------------------------------------

//----------------------------background---------------------------------------------
extern void stdcall _ksys_set_background_size(int xsize,int ysize);
extern void stdcall _ksys_write_background_mem(int pos,int color);
extern void stdcall _ksys_draw_background(void);
extern void stdcall _ksys_set_background_draw_type(int type);
extern void stdcall _ksys_background_blockmove(void* src,int bgr_pos, int count);
//-----------------------------------------------------------------------------------

//----------------------------functionf for draw window,lines.bar,etc.---------------
extern void stdcall _ksys_draw_window(int xcoord,int ycoord, int xsize,
                               int ysize,int workcolor,int type,
                               int captioncolor,int windowtype,int bordercolor);
extern void stdcall _ksys_window_redraw(int status);
extern int  stdcall _ksys_putpixel(int x,int y,int color);
extern void stdcall _ksys_draw_bar(int x, int y, int xsize, int ysize, int color);
extern void stdcall _ksys_line(int x1,int y1,int x2,int y2,int color);
extern void stdcall _ksys_putimage(int x, int y, int xsize, int ysize, void* image);
//-----------------------------------------------------------------------------------

//--------------------------write text(system fonts 6x9)-----------------------------
extern void stdcall _ksys_write_text(int x,int y,int color,char* text,int len);
//-----------------------------------------------------------------------------------

//------------------  get screen size  and bytes per pixel---------------------------
extern int  stdcall _ksys_get_screen_size(int* x,int* y);
extern void stdcall _ksys_dga_get_resolution(int* xres, int* yres, int* bpp, int* bpscan);
//-----------------------------------------------------------------------------------

//-------------------------------craete thread---------------------------------------
extern void* stdcall  _ksys_start_thread(void (* func_ptr)(void),int stack_size,int* pid);
//-----------------------------------------------------------------------------------

//------------------system button(Old function. Better use libGUI functions.)--------
extern void stdcall _ksys_make_button(int x, int y, int xsize, int ysize, int id, int color);
extern int  stdcall _ksys_get_button_id(void); //get state of system button
//------------------------------------------------------------------------------------

//----------------------system clock(in 1/100 sec.) and date--------------------------
extern int  stdcall _ksys_get_system_clock(void);
extern int  stdcall _ksys_get_date(void);
//------------------------------------------------------------------------------------

//-------------------------system delay(in 1/100 sec.)-------------------------------
extern void stdcall _ksys_delay(int m);
//-----------------------------------------------------------------------------------

//------------------------system events----------------------------------------------
extern int  stdcall _ksys_wait_for_event_infinite(void);
extern int  stdcall _ksys_check_for_event(void);
extern int  stdcall _ksys_wait_for_event(int time);
extern void stdcall _ksys_set_wanted_events(int ev);
//-----------------------------------------------------------------------------------

//----------------------------system exit program------------------------------------
extern void stdcall _ksys_exit(void);
//-----------------------------------------------------------------------------------

//-----------------------------system IPC send message-------------------------------
extern void stdcall _ksys_send_message(int pid, void* msg, int size);
//-----------------------------------------------------------------------------------

//---------------------------system work with IRQ from user mode---------------------
extern void stdcall _ksys_define_receive_area(void* area, int size);
extern int  stdcall _ksys_get_irq_owner(int irq);
extern int  stdcall _ksys_get_data_read_by_irq(int irq, int* size, void* data);
extern int  stdcall _ksys_send_data_to_device(int port, unsigned char val);
extern int  stdcall _ksys_receive_data_from_device(int port,unsigned char* data);
extern void stdcall _ksys_program_irq(void* intrtable, int irq);
extern void stdcall _ksys_reserve_irq(int irq);
extern void stdcall _ksys_free_irq(int irq);
//----------------------------------------------------------------------------------

//----------------------------system reserve diapason of ports----------------------
extern int  stdcall _ksys_reserve_port_area(int start,int end);
extern int  stdcall _ksys_free_port_area(int start,int end);
//----------------------------------------------------------------------------------

//-------------functions get key and set keyboard mode------------------------------
extern int  stdcall _ksys_get_key(void);
extern void stdcall _ksys_set_keyboard_mode(int mode);
//----------------------------------------------------------------------------------

//--------------simple work with MPU401 sound device---------------------------------
extern void stdcall _ksys_midi_reset(void);
extern void stdcall _ksys_midi_send(int data);
//-----------------------------------------------------------------------------------

//--------------------------acces to PCI BUS from user mode---------------------------
extern int  stdcall _ksys_get_pci_version(void);
extern int  stdcall _ksys_get_last_pci_bus(void);
extern int  stdcall _ksys_get_pci_access_mechanism(void);
extern int  stdcall _ksys_pci_read_config_byte(int bus,int dev,int fn,int reg);
extern int  stdcall _ksys_pci_read_config_word(int bus,int dev,int fn,int reg);
extern int  stdcall _ksys_pci_read_config_dword(int bus,int dev,int fn,int reg);
extern int  stdcall _ksys_pci_write_config_byte(int bus,int dev,int fn,int reg,int value);
extern int  stdcall _ksys_pci_write_config_word(int bus,int dev,int fn,int reg,int value);
extern int  stdcall _ksys_pci_write_config_value(int bus,int dev,int fn,int reg,int value);
//--------------------------------------------------------------------------------------

//------------------------Process information--------------------------------------
extern int  stdcall _ksys_get_process_table(struct process_table_entry *proctab,int pid); //if pid=-1 than get info about him.
//---------------------------------------------------------------------------------

//-----------------Old functions for work with sound(Sound Blaster only).---------
extern void stdcall _ksys_sound_load_block(void* blockptr);
extern void stdcall _ksys_sound_play_block(void);
extern void stdcall _ksys_sound_set_channels(int channels);
extern void stdcall _ksys_sound_set_data_size(int size);
extern void stdcall _ksys_sound_set_frequency(int frequency);
//--------------------------------------------------------------------------------

//------------------------------system speaker(integrated speaker)----------------
extern void stdcall _ksys_sound_speaker_play(void* data);
//--------------------------------------------------------------------------------

//------------------function for work with Dinamic Link Librarys(DLL)--------------
extern dword* stdcall _ksys_cofflib_load(char* name);
extern char* stdcall _ksys_cofflib_getproc(void* exp,char* sz_name);
//---------------------------------------------------------------------------------

#endif

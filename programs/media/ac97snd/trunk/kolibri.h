
#ifdef __cplusplus
extern "C"
{
#endif

#define FONT0          0x00000000
#define FONT1          0x10000000

#define BT_NORMAL      0x00000000
#define BT_NOFRAME     0x20000000
#define BT_HIDE        0x40000000
#define BT_DEL         0x80000000

#define EV_REDRAW      1
#define EV_KEY         2
#define EV_BUTTON      3

#define REL_SCREEN     0
#define REL_WINDOW     1

#define FILE_NOT_FOUND 5
#define FILE_EOF       6

typedef unsigned int DWORD;
typedef unsigned short int WORD;
typedef unsigned int size_t;

typedef struct
{  DWORD pci_cmd;
   DWORD irq;
   DWORD glob_cntrl;
   DWORD glob_sta;
   DWORD codec_io_base;
   DWORD ctrl_io_base;
   DWORD codec_mem_base;
   DWORD ctrl_mem_base;
   DWORD codec_id;
} CTRL_INFO;

typedef struct
{   DWORD       cmd;
    DWORD       offset;
    DWORD       r1;
    DWORD       count;
    DWORD       buff;
    char        r2;
    char       *name;
} FILEIO;

typedef struct
{   DWORD    attr;
    DWORD    flags;
    DWORD    cr_time;
    DWORD    cr_date;
    DWORD    acc_time;
    DWORD    acc_date;
    DWORD    mod_time;
    DWORD    mod_date;
    DWORD    size;
    DWORD    size_high; 
} FILEINFO;

void  _stdcall InitHeap(size_t heap_size);
void* _stdcall UserAlloc(size_t size);

void _stdcall CreateThread(void *fn, char *p_stack);
DWORD _stdcall GetMousePos(DWORD rel_type);

void _stdcall debug_out_hex(DWORD val);
void debug_out_str(char* str);

int _stdcall get_fileinfo(char *name,FILEINFO* pinfo);
int _stdcall read_file (char *name,char*buff,int offset,int count,int *reads);

void exit();
int _stdcall get_key(int *key);
int _cdecl get_button_id();
void delay(int val);
int wait_for_event(int time);
int wait_for_event_infinite();
void BeginDraw(void);
void EndDraw(void);
void _stdcall DrawWindow(int x,int y, int sx, int sy,int workcolor,int style,
                               int captioncolor,int windowtype,int bordercolor);
void _stdcall debug_out(int ch);
void _stdcall make_button(int x, int y, int xsize, int ysize, int id, int color);
void _stdcall draw_bar(int x, int y, int xsize, int ysize, int color);
void _stdcall write_text(int x,int y,int color,char* text,int len);

#ifdef __cplusplus
extern "C"
}
#endif

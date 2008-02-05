
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

#if 0

typedef unsigned int DWORD;
typedef unsigned short int WORD;

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

void  _stdcall GetNotify(void *event);

void _stdcall CreateThread(void *fn, char *p_stack);

DWORD _stdcall GetMousePos(DWORD rel_type);

void _stdcall debug_out_hex(DWORD val);
void debug_out_str(char* str);

int _stdcall get_fileinfo(const char *name,FILEINFO* pinfo);
int _stdcall read_file (const char *name,char*buff,DWORD offset, DWORD count,DWORD *reads);
int _stdcall write_file(const char *name,char*buff,int offset,int count,int *writes);

//void exit();
int _stdcall get_key(int *key);
int _stdcall remap_key(int key);

int _cdecl get_button_id();

//void delay(int val);

int wait_for_event(int time);
int wait_for_event_infinite();
void BeginDraw(void);
void EndDraw(void);

void _stdcall GetScreenSize(int *x, int*y);
void _stdcall DrawWindow(int x,int y, int sx, int sy,int workcolor,int style,
                               int captioncolor,int windowtype,int bordercolor);
void _stdcall debug_out(int ch);
void _stdcall make_button(int x, int y, int xsize, int ysize, int id, int color);
void _stdcall draw_bar(int x, int y, int xsize, int ysize, int color);
void _stdcall write_text(int x,int y,int color,char* text,int len);

#endif

typedef struct
{   unsigned    attr;
    unsigned    flags;
    unsigned    cr_time;
    unsigned    cr_date;
    unsigned    acc_time;
    unsigned    acc_date;
    unsigned    mod_time;
    unsigned    mod_date;
    unsigned    size;
    unsigned    size_high; 
} FILEINFO;


unsigned init_heap(void);
void *user_alloc(unsigned size);
unsigned user_free(void *);

int create_file(const char *name);
int get_fileinfo(const char *name,FILEINFO* pinfo);
int read_file (const char *name,void *buff, unsigned offset, unsigned count,unsigned *reads);
int write_file(const char *name,const void *buff,unsigned offset,unsigned count,unsigned *writes);

#pragma aux init_heap =   \
            "mov EAX, 68" \
            "mov EBX, 11" \
            "int 0x40"    \
            value [EAX] \
            modify [ EBX ];

#pragma aux user_alloc = \
            "mov EAX, 68" \
            "mov EBX, 12" \
            "int 0x40"    \
            parm [ ECX ] value [EAX] \
            modify [ EBX ];

#pragma aux user_free =   \
            "mov EAX, 68" \
            "mov EBX, 13" \
            "int 0x40"    \
            parm [ ECX ] value [EAX] \
            modify [ EBX ];

#pragma aux create_file  = \
            "push 0"           \
            "push 0"           \ 
            "mov [esp+1], eax" \
            "push 0"           \
            "push 0"           \
            "push 0"           \
            "push 0"           \
            "push 2"           \
            "mov ebx, esp"     \  
            "mov eax, 70"      \
            "int 0x40"         \
            "add esp, 28"      \      
            parm [EAX] value [EAX] \
            modify [ EBX ];

#pragma aux get_fileinfo  = \
            "push 0"           \
            "push 0"           \ 
            "mov [esp+1], eax" \
            "push ebx"         \
            "push 0"           \
            "push 0"           \
            "push 0"           \
            "push 5"           \
            "mov ebx, esp"     \  
            "mov eax, 70"      \
            "int 0x40"         \
            "add esp, 28"      \      
            parm [EAX] [ebx] value [EAX] \
            modify [ EBX ];

#pragma aux read_file  = \
            "push 0"           \
            "push 0"           \ 
            "mov [esp+1], eax" \
            "push ebx"         \
            "push edx"         \
            "push 0"           \
            "push ecx"         \
            "push 0"           \
            "mov ebx, esp"     \  
            "mov eax, 70"      \
            "int 0x40"         \
            "test esi, esi"    \
            "jz skip"          \
            "mov [esi], ebx"   \
     "skip:"                   \       
            "add esp, 28"      \      
            parm [EAX] [EBX] [ECX] [EDX] [ESI] value [EAX] \
            modify [ EBX ];

#pragma aux write_file  = \
            "push 0"           \
            "push 0"           \ 
            "mov [esp+1], eax" \
            "push ebx"         \
            "push edx"         \
            "push 0"           \
            "push ecx"         \
            "push 3"           \
            "mov ebx, esp"     \  
            "mov eax, 70"      \
            "int 0x40"         \
            "test esi, esi"    \
            "jz skip"          \
            "mov [esi], ebx"   \
     "skip:"                   \       
            "add esp, 28"      \      
            parm [EAX] [EBX] [ECX] [EDX] [ESI] value [EAX] \
            modify [ EBX ];

#ifdef __cplusplus
extern "C"
}
#endif

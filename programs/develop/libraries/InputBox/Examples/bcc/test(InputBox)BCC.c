/* Event Constants */
#define REDRAW_EVENT 1
#define KEY_EVENT    2
#define BUTTON_EVENT 3

/* Window Style Constants */
#define WS_SKINNED_FIXED 0x4000000
#define WS_COORD_CLIENT  0x20000000
#define WS_CAPTION       0x10000000

/* Caption Style Constants */
#define CS_MOVABLE 0

/* Charset specifiers for DrawText */
#define DT_CP866_8X16 0x10000000

/* Fill styles for DrawText */
#define DT_FILL_OPAQUE 0x40000000

/* Draw zero terminated string for DrawText */
#define DT_ZSTRING 0x80000000

// Button identifiers
#define BUTTON_CLOSE 1
#define SET_LEFT_BUTTON 1111
#define SET_TOP_BUTTON 2222
#define SET_CAPTION_BUTTON 3333

// Flags = [mouse|screen|parent][number|string]
#define IBF_STRING 0      // в буфер будет записана строка
#define IBF_NUMBER 1      // в буфер будет записано число
#define IBF_MOUSE_REL 0   // относительно положения указателя мыши
#define IBF_SCREEN_REL 8  // относительно экрана
#define IBF_PARENT_REL 16 // относительно родительского окна

// Errors
#define IBE_NO_ERROR 0        // успешно, нет ошибки
#define IBE_NUMBER_OVERFLOW 1 // переполнение при вводе числа
#define IBE_RESULT_TOO_LONG 2 // результат не умещается в буфер

#pragma pack(1)

struct TButtonInput{
  union{
    struct{
      unsigned char MouseButton;
      unsigned short ID;
      unsigned char HiID;
    };
    unsigned long Value;
  };
};

struct TKeyboardInput{
  union{
    struct{
      unsigned char Flag;
      char Code;
      union{
        char Scan;
        unsigned short Control;
      };
    };
    unsigned long Value;
  };
};

struct TSize{
  union{
    struct{
      unsigned short Height;
      unsigned short Width;
    };
    unsigned long Value;
  };
};

struct TBox{
  long Left;
  long Top;
  unsigned long SizeX;
  unsigned long SizeY;
};

extern struct TButtonInput   __stdcall GetButton(void);
extern struct TKeyboardInput __stdcall GetKey(void);
extern struct TSize          __stdcall GetScreenSize(void);
extern unsigned long         __stdcall WaitEvent(void);
extern void                  __stdcall BeginDraw(void);
extern void                  __stdcall EndDraw(void);
extern void                  __stdcall ThreadTerminate(void);

extern void                  __stdcall DrawWindow(long          Left,
                                                  long          Top,
                                                  unsigned long Width,
                                                  unsigned long Height,
                                                  char*         Caption,
                                                  unsigned long BackColor,
                                                  unsigned long Style,
                                                  unsigned long CapStyle);
                                                  
extern void                  __stdcall DrawText(long X,
                                                long Y,
                                                char* Text,
                                                unsigned long ForeColor,
                                                unsigned long BackColor,
                                                unsigned long Flags,
                                                unsigned long Count);

extern void                  __stdcall DrawButton(long Left,
                                                  long Top,
                                                  unsigned long Width,
                                                  unsigned long Height,
                                                  unsigned long BackColor,
                                                  unsigned long Style,
                                                  unsigned long ID);
                                                  
extern void                  __stdcall SetWindowPos(long Left, long Top, unsigned long Width, unsigned long Height);
extern void*                 __stdcall LoadLibrary(char* Path);
extern void*                 __stdcall GetProcAddress(void* hLib, char* ProcName);
extern void                  __stdcall SetWindowCaption(char* Caption);
extern long                  __stdcall RunFile(char* Path, char* CmdLine);

struct TSize Screen;
struct TBox Window;

unsigned long err;

char Buf[100];
long NewPos;

void* InputBoxLib;

unsigned long __stdcall (* InputBox)(void* Buffer, char* Caption, char* Prompt, char* Default,
                                     unsigned long Flags, unsigned long BufferSize, void* RedrawProc);
                                     
void OnRedraw(void){
  BeginDraw();
  DrawWindow(Window.Left, Window.Top, Window.SizeX, Window.SizeY, "Test InputBox", 0xFFFFFF, WS_SKINNED_FIXED + WS_COORD_CLIENT + WS_CAPTION, CS_MOVABLE);

  DrawButton(8, 60, 81, 33, 0x0FF0000, 0, SET_LEFT_BUTTON);
  DrawButton(104, 60, 81, 33, 0x000FF00, 0, SET_TOP_BUTTON);
  DrawButton(200, 60, 93, 33, 0x0FFFF00, 0, SET_CAPTION_BUTTON);

  DrawText(28, 72, "Set Left", 0, 0x0FFFFFF, DT_ZSTRING, 0);
  DrawText(122, 72, "Set Top", 0, 0x0FFFFFF, DT_ZSTRING, 0);
  DrawText(214, 72, "Set Caption", 0, 0x0FFFFFF, DT_ZSTRING, 0);

  EndDraw();
}
/* -------------------------------------------------------- */
void Main(){
  InputBoxLib = LoadLibrary("/sys/lib/InputBox.obj");
  InputBox = GetProcAddress(InputBoxLib, "InputBox");

  Window.Left  = 100;
  Window.Top   = 70;
  Window.SizeX = 315;
  Window.SizeY = 200;

  while(1){
    switch (WaitEvent()){
    case BUTTON_EVENT:
      switch (GetButton().ID){
      case BUTTON_CLOSE:
        ThreadTerminate();
        break;
      case SET_LEFT_BUTTON:
        err = InputBox(&NewPos, "Input", "Enter new left", "100", IBF_NUMBER + IBF_PARENT_REL, sizeof(NewPos), &OnRedraw);
        if (err) RunFile("/sys/@notify", "Error");
        SetWindowPos(NewPos, -1, -1, -1);
        break;
      case SET_TOP_BUTTON:
        err = InputBox(&NewPos, "Input", "Enter new top", "70", IBF_NUMBER + IBF_SCREEN_REL, sizeof(NewPos), &OnRedraw);
        if (err) RunFile("/sys/@notify", "Error");
        SetWindowPos(-1, NewPos, -1, -1);
        break;
      case SET_CAPTION_BUTTON:
        InputBox(&Buf, "Input", "Enter new caption", "Test InputBox", IBF_STRING + IBF_MOUSE_REL, sizeof(Buf), &OnRedraw);
        SetWindowCaption(Buf);
        break;
      }
      break;
    case KEY_EVENT:
      GetKey();
      break;
    case REDRAW_EVENT:
      OnRedraw();
      break;
    }
  }
}
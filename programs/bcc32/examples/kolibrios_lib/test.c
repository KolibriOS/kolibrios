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

/* Button identifiesrs */
#define BUTTON_CLOSE 1

/* Key code values */
#define KEY_CODE_ENTER 13

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

struct TRect{
  long Left;
  long Top;
  long Right;
  long Bottom;
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
                                                  long          Right,
                                                  long          Bottom,
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

struct TSize Screen;
struct TRect Window;

void Main(){
  Screen = GetScreenSize();
  Window.Right  = Screen.Width / 3;
  Window.Bottom = Screen.Height / 3;
  Window.Left   = (Screen.Width  - Window.Right)  / 2;
  Window.Top    = (Screen.Height - Window.Bottom) / 2;
  while(1){
    switch (WaitEvent()){
    case BUTTON_EVENT:
      if (GetButton().ID == BUTTON_CLOSE) ThreadTerminate();
      break;
    case KEY_EVENT:
      if (GetKey().Code == KEY_CODE_ENTER) ThreadTerminate();
      break;
    case REDRAW_EVENT:
      BeginDraw();
      DrawWindow(Window.Right, Window.Bottom, Window.Left, Window.Top, "C with KolibriOS.lib", 0x00C0DDEE, WS_SKINNED_FIXED | WS_COORD_CLIENT | WS_CAPTION, CS_MOVABLE);
      DrawText( 8, 25, "Press Enter to quit", 0, 0x00FFFFFF, DT_CP866_8X16 | DT_FILL_OPAQUE | DT_ZSTRING, 0);
      EndDraw();
      break;
    }
  }
}

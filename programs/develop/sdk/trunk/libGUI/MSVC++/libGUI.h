/*
       service  structures of libGUI
*/

typedef unsigned int                            DWORD;
typedef unsigned char                           BYTE;
typedef unsigned short int                      WORD;
typedef unsigned int                            size_t;

#define stdcall	__stdcall

/////////////////////////////////////////////////////////////////////////
//                      libGUI sysyem messages types
/////////////////////////////////////////////////////////////////////////
#define         MESSAGE_FULL_REDRAW_ALL                         1
#define         MESSAGE_KEYS_EVENT                                      2
#define         MESSAGE_SPECIALIZED                                     3
#define         MESSAGE_SET_FOCUSE                                      4
#define         MESSAGE_CHANGE_FOCUSE                           5
#define         MESSAGE_MOUSE_EVENT                                     6
#define         MESSAGE_CHANGE_POSITION_EVENT                   7
#define         MESSAGE_CHANGESIZE_EVENT                                8
#define         MESSAGE_CALL_TIMER_EVENT                                9
#define         MESSAGE_FULL_REDRAW_ALL_WITH_FINITION           10
#define         MESSAGE_SET_MAIN_PARENT                         11
#define         MESSAGE_DESTROY_CONTROL                         -1

/////////////////////////////////////////////////////////////////////////
//                      system keys states
/////////////////////////////////////////////////////////////////////////
#define KEY_DOWN                                                16
#define KEY_UP                                                  17
#define KEY_HOTKEY                                              18
/////////////////////////////////////////////////////////////////////////
//                      system mouse buttons states
/////////////////////////////////////////////////////////////////////////
#define MOUSE_LEFT_BUTTON_DOWN                          19
#define MOUSE_LEFT_BUTTON_UP                                    20
#define MOUSE_RIGHT_BUTTON_DOWN                         21
#define MOUSE_RIGHT_BUTTON_UP                           22
#define MOUSE_MIDDLE_BUTTON_DOWN                                23
#define MOUSE_MIDDLE_BUTTON_UP                          24
#define MOUSE_4_BUTTON_DOWN                                     25
#define MOUSE_4_BUTTON_UP                                       26
#define MOUSE_5_BUTTON_DOWN                                     27
#define MOUSE_5_BUTTON_UP                                       28


//-----------------------------------------------------------------------
//              CONNECT EVENTS FOR CALLBACKs
//-----------------------------------------------------------------------

////////////////////////////////////////////////////////////////
//              connect events for button
////////////////////////////////////////////////////////////////
#define BUTTON_ENTER_EVENT                                      29
#define BUTTON_LEAVE_EVENT                                      30
#define BUTTON_PRESSED_EVENT                                    31
#define BUTTON_RELEASED_EVENT                           32

////////////////////////////////////////////////////////////////
//              connect events for scroll bar
////////////////////////////////////////////////////////////////
#define SCROLLBAR_CHANGED_EVENT                         33

////////////////////////////////////////////////////////////////
//              connect events for main parent window
////////////////////////////////////////////////////////////////
#define DELETE_EVENT                                            36

////////////////////////////////////////////////////////////////
//                      font type structure
////////////////////////////////////////////////////////////////
#pragma pack(push,1)
struct  FONT
{
        DWORD           *fnt_draw;
        DWORD           *fnt_unpacker;
        DWORD           *fnt_fd;
        DWORD           *fnt_bk;
        int             sizex;
        int             sizey;
        int             size;
        int             encoding_type;
        char            *font;
        char            *fnt_name;
        DWORD           type;
        DWORD           flags;
};
#pragma pack(pop)

typedef struct  FONT    font_t;

////////////////////////////////////////////////////////////////
//      header of parent of control
////////////////////////////////////////////////////////////////

#pragma pack(push,1)
struct HEADERPARENT
{
        DWORD   *ctrl_proc;
        DWORD   *ctrl_fd;
        DWORD   *ctrl_bk;
        DWORD   *child_fd;
        DWORD   *child_bk;
        DWORD   *parent;
        DWORD   *main_parent;
        DWORD   ctrl_x;
        DWORD   ctrl_y;
        DWORD   ctrl_sizex;
        DWORD   ctrl_sizey;
        DWORD   ctrl_ID;
        DWORD   *active_control_for_keys;
        DWORD   *active_control_for_mouse;
        DWORD   *callback;
        DWORD   *finition;
        DWORD   *timer;
        DWORD   flags;

        DWORD   **control_for_callback_function;
        DWORD   **callback_for_control_callback;
        DWORD   number_callbacks;
        DWORD   *global_active_control_for_keys;
        DWORD   *message;
        DWORD   *timer_bk;
        DWORD   *timer_fd;
        DWORD   number_timers_for_controls;
        DWORD   *calev_bk;
        DWORD   *calev_fd;
        DWORD   *IDL_func;
        DWORD   *IDL_func_data;
};
#pragma pack(pop)

typedef struct HEADERPARENT parent_t;

////////////////////////////////////////////////////////////////
//      header of control
////////////////////////////////////////////////////////////////
#pragma pack(push,1)
struct HEADER
{
        DWORD   *ctrl_proc;
        DWORD   *ctrl_fd;
        DWORD   *ctrl_bk;
        DWORD   *child_fd;
        DWORD   *child_bk;
        DWORD   *parent;
        DWORD   *main_parent;
        DWORD   ctrl_x;
        DWORD   ctrl_y;
        DWORD   ctrl_sizex;
        DWORD   ctrl_sizey;
        DWORD   ctrl_ID;
        DWORD   *active_control_for_keys;
        DWORD   *active_control_for_mouse;
        DWORD   *callback;
        DWORD   *finition;
        DWORD   *timer;
        DWORD   flags;
};
#pragma pack(pop)

typedef struct HEADER header_t;
////////////////////////////////////////////////////////////////
//      callback structure for callback function of control
////////////////////////////////////////////////////////////////
#pragma pack(push,1)
struct CALLBACK
{
        DWORD   *clb_bk;
        DWORD   *clb_fd;
        DWORD   *clb_control;
        DWORD   *func;
        DWORD   *func_data;
        DWORD   connect_event;
        DWORD   flags;
};
#pragma pack(pop)

typedef struct CALLBACK gui_callback_t;
////////////////////////////////////////////////////////////////
//                      timer
////////////////////////////////////////////////////////////////
#pragma pack(push,1)
struct TIMER
{
        DWORD   *tmr_bk;
        DWORD   *tmr_fd;
        DWORD   *tmr_parent;
        DWORD   *func;
        DWORD   *func_data;
        DWORD   last_time;
        DWORD   time_tick;
        DWORD   flags;
};
#pragma pack(pop)

typedef struct TIMER gui_timer_t;
////////////////////////////////////////////////////////////////
//              structure for callback events
////////////////////////////////////////////////////////////////
#pragma pack(push,1)
struct CALLBACKEVENT
{
        DWORD   *calev_bk;
        DWORD   *calev_fd;
        DWORD   *calev_parent;
        DWORD   *func;
        DWORD   *func_data;
        DWORD   event_type;
};
#pragma pack(pop)

typedef struct CALLBACKEVENT gui_callbackevent_t;

////////////////////////////////////////////////////////////////
//              type of data - structure message
////////////////////////////////////////////////////////////////
#pragma pack(push,1)
struct MESSAGE
{
        DWORD   type;
        DWORD   arg1;
        DWORD   arg2;
        DWORD   arg3;
        DWORD   arg4;
};
#pragma pack(pop)

typedef struct MESSAGE gui_message_t;

////////////////////////////////////////////////////////////////
//                              button
////////////////////////////////////////////////////////////////
#pragma pack(push,1)
struct ControlButton
{
        DWORD   *ctrl_proc;
        DWORD   *ctrl_fd;
        DWORD   *ctrl_bk;
        DWORD   *child_fd;
        DWORD   *child_bk;
        DWORD   *parent;
        DWORD   *main_parent;
        DWORD   ctrl_x;
        DWORD   ctrl_y;
        DWORD   ctrl_sizex;
        DWORD   ctrl_sizey;
        DWORD   ctrl_ID;
        DWORD   *active_control_for_keys;
        DWORD   *active_control_for_mouse;
        DWORD   *callback;
        DWORD   *finition;
        DWORD   *timer;
        DWORD   flags;

        //button's data
        BYTE    btn_flags;
};
#pragma pack(pop)

typedef struct ControlButton gui_button_t;

// information for creating control Button
#pragma pack(push,1)
struct ButtonData
{
        int     x;
        int     y;
        int     width;
        int     height;
};
#pragma pack(pop)

typedef struct ButtonData gui_button_data_t;

////////////////////////////////////////////////////////////////
//      scroller
////////////////////////////////////////////////////////////////
#pragma pack(push,1)
struct ControlScrollBar
{
        DWORD   *ctrl_proc;
        DWORD   *ctrl_fd;
        DWORD   *ctrl_bk;
        DWORD   *child_fd;
        DWORD   *child_bk;
        DWORD   *parent;
        DWORD   *main_parent;
        DWORD   ctrl_x;
        DWORD   ctrl_y;
        DWORD   ctrl_sizex;
        DWORD   ctrl_sizey;
        DWORD   ctrl_ID;
        DWORD   *active_control_for_keys;
        DWORD   *active_control_for_mouse;
        DWORD   *callback;
        DWORD   *finition;
        DWORD   *timer;
        DWORD   flags;

        //scroll bar's data
        float   ruller_size;
        float   ruller_pos;
        float   ruller_step;
        BYTE    scb_flags;
};
#pragma pack(pop)

typedef struct ControlScrollBar gui_scroll_bar_t;

#pragma pack(push,1)
struct ScrollBarData
{
        int     x;
        int     y;
        int     width;
        int     height;
        float   ruller_size;
        float   ruller_pos;
        float   ruller_step;
};
#pragma pack(pop)

typedef struct ScrollBarData gui_scroll_bar_data_t;
////////////////////////////////////////////////////////////////
//      progressbar
////////////////////////////////////////////////////////////////
#pragma pack(push,1)
struct ControlProgressBar
{
        DWORD   *ctrl_proc;
        DWORD   *ctrl_fd;
        DWORD   *ctrl_bk;
        DWORD   *child_fd;
        DWORD   *child_bk;
        DWORD   *parent;
        DWORD   *main_parent;
        DWORD   ctrl_x;
        DWORD   ctrl_y;
        DWORD   ctrl_sizex;
        DWORD   ctrl_sizey;
        DWORD   ctrl_ID;
        DWORD   *active_control_for_keys;
        DWORD   *active_control_for_mouse;
        DWORD   *callback;
        DWORD   *finition;
        DWORD   *timer;
        DWORD   flags;

        //progress bar's data
        float   progress;
        BYTE    prb_flags;
};
#pragma pack(pop)

typedef struct ControlProgressBar gui_progress_bar_t;

#pragma pack(push,1)
struct ProgressBarData
{
        int     x;
        int     y;
        int     width;
        int     height;
        float   progress;
};
#pragma pack(pop)

typedef struct ProgressBarData gui_progress_bar_data_t;
////////////////////////////////////////////////////////////////
//      scrolled window
////////////////////////////////////////////////////////////////
#pragma pack(push,1)
struct ControlScrolledWindow
{
        DWORD   *ctrl_proc;
        DWORD   *ctrl_fd;
        DWORD   *ctrl_bk;
        DWORD   *child_fd;
        DWORD   *child_bk;
        DWORD   *parent;
        DWORD   *main_parent;
        DWORD   ctrl_x;
        DWORD   ctrl_y;
        DWORD   ctrl_sizex;
        DWORD   ctrl_sizey;
        DWORD   ctrl_ID;
        DWORD   *active_control_for_keys;
        DWORD   *active_control_for_mouse;
        DWORD   *callback;
        DWORD   *finition;
        DWORD   *timer;
        DWORD   flags;

        //scrolled windows's data
        DWORD   virtual_x;
        DWORD   virtual_y;
        DWORD   virtual_sizex;
        DWORD   virtual_sizey;
        DWORD   *virtual_controls_x;
        DWORD   *virtual_controls_y;
        DWORD   number_virtual_controls;
        DWORD   scroll_arrea_sizex;
        DWORD   scroll_arrea_sizey;
        DWORD   *horizontal_scroll;
        DWORD   *vertical_scroll;
        BYTE    scw_flags;
};
#pragma pack(pop)

typedef struct ControlScrolledWindow gui_scrolled_window_t;

#pragma pack(push,1)
struct ScrolledWindowData
{
        int     x;
        int     y;
        int     width;
        int     height;
};
#pragma pack(pop)

typedef struct ScrolledWindowData gui_scrolled_window_data_t;

////////////////////////////////////////////////////////////////
//      image
////////////////////////////////////////////////////////////////
#pragma pack(push,1)
struct ControlImage
{
        DWORD   *ctrl_proc;
        DWORD   *ctrl_fd;
        DWORD   *ctrl_bk;
        DWORD   *child_fd;
        DWORD   *child_bk;
        DWORD   *parent;
        DWORD   *main_parent;
        DWORD   ctrl_x;
        DWORD   ctrl_y;
        DWORD   ctrl_sizex;
        DWORD   ctrl_sizey;
        DWORD   ctrl_ID;
        DWORD   *active_control_for_keys;
        DWORD   *active_control_for_mouse;
        DWORD   *callback;
        DWORD   *finition;
        DWORD   *timer;
        DWORD   flags;

        char    bits_per_pixel;
        char    bytes_per_pixel;
        char    *img;
};
#pragma pack(pop)

typedef struct ControlImage gui_image_t;

#pragma pack(push,1)
struct ImageData
{
        int     x;
        int     y;
        int     width;
        int     height;
        char    bits_per_pixel;
};
#pragma pack(pop)

typedef struct ImageData gui_image_data_t;

////////////////////////////////////////////////////////////////
//      text
////////////////////////////////////////////////////////////////
#pragma pack(push,1)
struct ControlText
{
        DWORD   *ctrl_proc;
        DWORD   *ctrl_fd;
        DWORD   *ctrl_bk;
        DWORD   *child_fd;
        DWORD   *child_bk;
        DWORD   *parent;
        DWORD   *main_parent;
        DWORD   ctrl_x;
        DWORD   ctrl_y;
        DWORD   ctrl_sizex;
        DWORD   ctrl_sizey;
        DWORD   ctrl_ID;
        DWORD   *active_control_for_keys;
        DWORD   *active_control_for_mouse;
        DWORD   *callback;
        DWORD   *finition;
        DWORD   *timer;
        DWORD   flags;

        DWORD   *font;  
        DWORD   color;
        DWORD   background_color;
        char    *text;
        BYTE    txt_flags;
};
#pragma pack(pop)

typedef struct ControlText gui_text_t;

#pragma pack(push,1)
struct TextData
{
        int     x;
        int     y;
        DWORD   *font;  
        DWORD   color;
        DWORD   background_color;
        char    background;
        char    *text;  
};
#pragma pack(pop)

typedef struct TextData gui_text_data_t;
/////////////////////////////////////////////////////////////////
//           load libGUI library and link functions
/////////////////////////////////////////////////////////////////
void    LoadLibGUI(char *lib_path);

//**********************************************************************
//                   prototypes of libGUI service functions
//**********************************************************************

extern DWORD (stdcall *LibGUIversion)(void);
extern char  (stdcall *InitLibGUI)(void);
extern void  (stdcall *LibGUImain)(parent_t *WindowParent);
extern void  (stdcall *QuitLibGUI)(parent_t *window);

extern parent_t* (stdcall *CreateWindow)(void);
extern void  (stdcall *SetWindowSizeRequest)(parent_t *WindowParent,int size_x,int size_y);

extern void  (stdcall *PackControls)(void *Parent,void *control);
extern void  (stdcall *DestroyControl)(void *control);
extern void  (stdcall *SetControlSizeRequest)(void *Control,int new_size_x,int new_size_y);
extern int   (stdcall *GetControlSizeX)(void *Control);
extern int   (stdcall *GetControlSizeY)(void *Control);
extern void  (stdcall *SetControlNewPosition)(void *Control,int new_x,int new_y);
extern int   (stdcall *GetControlPositionX)(void *Control);
extern int   (stdcall *GetControlPositionY)(void *Control);
extern void* (stdcall *SetFocuse)(void *Control);
extern void  (stdcall *RedrawControl)(void *Control);
extern void  (stdcall *SpecialRedrawControl)(void *Control);

extern gui_callback_t* (stdcall *SetCallbackFunction)(void *Control,
                                        int event_name,void *callback_func,
                                        void *callback_func_data);
extern void  (stdcall *BlockCallbackFunction)(void *Control,gui_callback_t *callback_ID);
extern void  (stdcall *UnblockCallbackFunction)(void *Control,gui_callback_t *callback_ID);

extern void  (stdcall *SetIDL_Function)(parent_t *Parent,void *function,void *function_data);
extern void  (stdcall *DestroyIDL_Function)(parent_t *Parent);

extern gui_timer_t* (stdcall *SetTimerCallbackForFunction)(parent_t *parent_window,
                                        int time_tick,void *func,void *func_data);
extern void (stdcall *DestroyTimerCallbackForFunction)(gui_timer_t *timer);

extern gui_callbackevent_t* (stdcall *SetCallbackFunctionForEvent)(parent_t *parent_window,
                                        int event_type,void *func,void *func_data);
extern void (stdcall *DestroyCallbackFunctionForEvent)(gui_callbackevent_t *callback_event);

extern gui_button_t* (stdcall *CreateButton)(gui_button_data_t *info_for_control);
extern gui_button_t* (stdcall *CreateButtonWithText)(gui_button_data_t *info,char *txt);

extern gui_progress_bar_t* (stdcall *CreateProgressBar)(gui_progress_bar_data_t *info_for_control);
extern void (stdcall *SetProgressBarPulse)(gui_progress_bar_t *ProgressBar,int time_update);
extern void (stdcall *ProgressBarSetText)(gui_progress_bar_t *pbar,char *txt);
extern char* (stdcall *ProgressBarGetText)(gui_progress_bar_t *pbar);

extern gui_scroll_bar_t* (stdcall *CreateHorizontalScrollBar)(gui_scroll_bar_data_t *info_for_control);
extern gui_scroll_bar_t* (stdcall *CreateVerticalScrollBar)(gui_scroll_bar_data_t *info_for_control);

extern gui_scrolled_window_t* (stdcall *CreateScrolledWindow)(gui_scrolled_window_data_t *info_for_control);
extern void (stdcall *ScrolledWindowPackControls)(gui_scrolled_window_t *parent,void *Control);

extern gui_image_t* (stdcall *CreateImage)(gui_image_data_t *info_for_control);

extern gui_text_t* (stdcall *CreateText)(gui_text_data_t *info_for_control);
extern void (stdcall *TextBackgroundOn)(gui_text_t *Text);
extern void (stdcall *TextBackgroundOff)(gui_text_t *Text);

extern font_t* (stdcall *LoadFont)(char *fullfontname);
extern void (stdcall *FreeFont)(font_t *font);



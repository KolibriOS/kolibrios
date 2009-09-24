/*
       service  structures of libGUI
*/
#define	NULL				(void*)0

typedef unsigned int				DWORD;
typedef unsigned char			BYTE;
typedef unsigned short int			WORD;
typedef unsigned int				size_t;

#define	stdcall	__attribute__ ((stdcall))
#define	cdecl		__attribute__ ((cdecl))
#define	pack		__attribute__	((packed))

/////////////////////////////////////////////////////////////////////////
//			libGUI sysyem messages types
/////////////////////////////////////////////////////////////////////////
#define 	MESSAGE_FULL_REDRAW_ALL				1
#define 	MESSAGE_KEYS_EVENT					2
#define 	MESSAGE_SPECIALIZED					3
#define 	MESSAGE_SET_FOCUSE					4
#define 	MESSAGE_CHANGE_FOCUSE				5
#define 	MESSAGE_MOUSE_EVENT					6
#define 	MESSAGE_CHANGE_POSITION_EVENT			7
#define 	MESSAGE_CHANGESIZE_EVENT				8
#define 	MESSAGE_CALL_TIMER_EVENT				9
#define 	MESSAGE_FULL_REDRAW_ALL_WITH_FINITION		10
#define 	MESSAGE_SET_MAIN_PARENT				11
#define 	MESSAGE_DESTROY_CONTROL				-1

/////////////////////////////////////////////////////////////////////////
//			system keys states
/////////////////////////////////////////////////////////////////////////
#define	KEY_DOWN						16
#define	KEY_UP							17
#define	KEY_HOTKEY						18
/////////////////////////////////////////////////////////////////////////
//			system mouse buttons states
/////////////////////////////////////////////////////////////////////////
#define	MOUSE_LEFT_BUTTON_DOWN				19
#define	MOUSE_LEFT_BUTTON_UP					20
#define	MOUSE_RIGHT_BUTTON_DOWN				21
#define	MOUSE_RIGHT_BUTTON_UP				22
#define	MOUSE_MIDDLE_BUTTON_DOWN				23
#define	MOUSE_MIDDLE_BUTTON_UP				24
#define	MOUSE_4_BUTTON_DOWN					25
#define	MOUSE_4_BUTTON_UP					26
#define	MOUSE_5_BUTTON_DOWN					27
#define	MOUSE_5_BUTTON_UP					28


//-----------------------------------------------------------------------
//		CONNECT EVENTS FOR CALLBACKs
//-----------------------------------------------------------------------

////////////////////////////////////////////////////////////////
//		connect events for button
////////////////////////////////////////////////////////////////
#define	BUTTON_ENTER_EVENT					29
#define	BUTTON_LEAVE_EVENT					30
#define	BUTTON_PRESSED_EVENT					31
#define	BUTTON_RELEASED_EVENT				32

////////////////////////////////////////////////////////////////
//		connect events for scroll bar
////////////////////////////////////////////////////////////////
#define	SCROLLBAR_CHANGED_EVENT				33

////////////////////////////////////////////////////////////////
//		connect events for main parent window
////////////////////////////////////////////////////////////////
#define	DELETE_EVENT						36

////////////////////////////////////////////////////////////////
//			font type structure
////////////////////////////////////////////////////////////////
struct  FONT
{
	DWORD		*fnt_draw;
	DWORD		*fnt_unpacker;
	DWORD		*fnt_fd;
 	DWORD		*fnt_bk;
	int		sizex;
	int		sizey;
	int		size;
	int		encoding_type;
	char		*font;
	char		*fnt_name;
	DWORD		type;
	DWORD		flags;
}pack;

typedef struct	FONT	font_t;

////////////////////////////////////////////////////////////////
//      header of parent of control
////////////////////////////////////////////////////////////////

struct HEADERPARENT
{
	DWORD	*ctrl_proc;
	DWORD	*ctrl_fd;
	DWORD	*ctrl_bk;
	DWORD	*child_fd;
	DWORD	*child_bk;
	DWORD	*parent;
	DWORD	*main_parent;
	DWORD	ctrl_x;
	DWORD	ctrl_y;
	DWORD	ctrl_sizex;
	DWORD	ctrl_sizey;
	DWORD	ctrl_ID;
	DWORD	*active_control_for_keys;
	DWORD	*active_control_for_mouse;
	DWORD	*callback;
	DWORD	*finition;
	DWORD	*timer;
	DWORD	flags;

	DWORD	**control_for_callback_function;
	DWORD	**callback_for_control_callback;
	DWORD	number_callbacks;
	DWORD	*global_active_control_for_keys;
	DWORD	*message;
	DWORD	*timer_bk;
	DWORD	*timer_fd;
	DWORD	number_timers_for_controls;
	DWORD	*calev_bk;
	DWORD	*calev_fd;
	DWORD	*IDL_func;
	DWORD	*IDL_func_data;
}pack;

typedef struct HEADERPARENT parent_t;

////////////////////////////////////////////////////////////////
//      header of control
////////////////////////////////////////////////////////////////

struct HEADER
{
	DWORD	*ctrl_proc;
	DWORD	*ctrl_fd;
	DWORD	*ctrl_bk;
	DWORD	*child_fd;
	DWORD	*child_bk;
	DWORD	*parent;
	DWORD	*main_parent;
	DWORD	ctrl_x;
	DWORD	ctrl_y;
	DWORD	ctrl_sizex;
	DWORD	ctrl_sizey;
	DWORD	ctrl_ID;
	DWORD	*active_control_for_keys;
	DWORD	*active_control_for_mouse;
	DWORD	*callback;
	DWORD	*finition;
	DWORD	*timer;
	DWORD	flags;
}pack;


typedef struct HEADER header_t;
////////////////////////////////////////////////////////////////
//      callback structure for callback function of control
////////////////////////////////////////////////////////////////

struct CALLBACK
{
	DWORD	*clb_bk;
	DWORD	*clb_fd;
	DWORD	*clb_control;
	DWORD	*func;
	DWORD	*func_data;
	DWORD	connect_event;
	DWORD	flags;
}pack;


typedef struct CALLBACK gui_callback_t;
////////////////////////////////////////////////////////////////
//      		timer
////////////////////////////////////////////////////////////////

struct TIMER
{
	DWORD	*tmr_bk;
	DWORD	*tmr_fd;
	DWORD	*tmr_parent;
	DWORD	*func;
	DWORD	*func_data;
	DWORD	last_time;
	DWORD	time_tick;
	DWORD	flags;
}pack;


typedef struct TIMER gui_timer_t;
////////////////////////////////////////////////////////////////
//      	structure for callback events
////////////////////////////////////////////////////////////////

struct CALLBACKEVENT
{
	DWORD	*calev_bk;
	DWORD	*calev_fd;
	DWORD	*calev_parent;
	DWORD	*func;
	DWORD	*func_data;
	DWORD	event_type;
}pack;


typedef struct CALLBACKEVENT gui_callbackevent_t;

////////////////////////////////////////////////////////////////
//		type of data - structure message
////////////////////////////////////////////////////////////////

struct MESSAGE
{
	DWORD	type;
	DWORD	arg1;
	DWORD	arg2;
	DWORD	arg3;
	DWORD	arg4;
}pack;


typedef struct MESSAGE gui_message_t;

////////////////////////////////////////////////////////////////
//      			button
////////////////////////////////////////////////////////////////

struct ControlButton
{
	DWORD	*ctrl_proc;
	DWORD	*ctrl_fd;
	DWORD	*ctrl_bk;
	DWORD	*child_fd;
	DWORD	*child_bk;
	DWORD	*parent;
	DWORD	*main_parent;
	DWORD	ctrl_x;
	DWORD	ctrl_y;
	DWORD	ctrl_sizex;
	DWORD	ctrl_sizey;
	DWORD	ctrl_ID;
	DWORD	*active_control_for_keys;
	DWORD	*active_control_for_mouse;
	DWORD	*callback;
	DWORD	*finition;
	DWORD	*timer;
	DWORD	flags;

	//button's data
	BYTE	btn_flags;
}pack;


typedef struct ControlButton gui_button_t;

// information for creating control Button

struct ButtonData
{
	int	x;
	int	y;
	int	width;
	int	height;
}pack;


typedef struct ButtonData gui_button_data_t;

////////////////////////////////////////////////////////////////
//      scroller
////////////////////////////////////////////////////////////////

struct ControlScrollBar
{
	DWORD	*ctrl_proc;
	DWORD	*ctrl_fd;
	DWORD	*ctrl_bk;
	DWORD	*child_fd;
	DWORD	*child_bk;
	DWORD	*parent;
	DWORD	*main_parent;
	DWORD	ctrl_x;
	DWORD	ctrl_y;
	DWORD	ctrl_sizex;
	DWORD	ctrl_sizey;
	DWORD	ctrl_ID;
	DWORD	*active_control_for_keys;
	DWORD	*active_control_for_mouse;
	DWORD	*callback;
	DWORD	*finition;
	DWORD	*timer;
	DWORD	flags;

	//scroll bar's data
	float	ruller_size;
	float	ruller_pos;
	float	ruller_step;
	BYTE	scb_flags;
}pack;


typedef struct ControlScrollBar gui_scroll_bar_t;


struct ScrollBarData
{
	int	x;
	int	y;
	int	width;
	int	height;
	float	ruller_size;
	float	ruller_pos;
	float	ruller_step;
}pack;


typedef struct ScrollBarData gui_scroll_bar_data_t;
////////////////////////////////////////////////////////////////
//      progressbar
////////////////////////////////////////////////////////////////

struct ControlProgressBar
{
	DWORD	*ctrl_proc;
	DWORD	*ctrl_fd;
	DWORD	*ctrl_bk;
	DWORD	*child_fd;
	DWORD	*child_bk;
	DWORD	*parent;
	DWORD	*main_parent;
	DWORD	ctrl_x;
	DWORD	ctrl_y;
	DWORD	ctrl_sizex;
	DWORD	ctrl_sizey;
	DWORD	ctrl_ID;
	DWORD	*active_control_for_keys;
	DWORD	*active_control_for_mouse;
	DWORD	*callback;
	DWORD	*finition;
	DWORD	*timer;
	DWORD	flags;

	//progress bar's data
	float	progress;
	BYTE	prb_flags;
}pack;


typedef struct ControlProgressBar gui_progress_bar_t;


struct ProgressBarData
{
	int	x;
	int	y;
	int	width;
	int	height;
	float	progress;
}pack;


typedef struct ProgressBarData gui_progress_bar_data_t;
////////////////////////////////////////////////////////////////
//      scrolled window
////////////////////////////////////////////////////////////////

struct ControlScrolledWindow
{
	DWORD	*ctrl_proc;
	DWORD	*ctrl_fd;
	DWORD	*ctrl_bk;
	DWORD	*child_fd;
	DWORD	*child_bk;
	DWORD	*parent;
	DWORD	*main_parent;
	DWORD	ctrl_x;
	DWORD	ctrl_y;
	DWORD	ctrl_sizex;
	DWORD	ctrl_sizey;
	DWORD	ctrl_ID;
	DWORD	*active_control_for_keys;
	DWORD	*active_control_for_mouse;
	DWORD	*callback;
	DWORD	*finition;
	DWORD	*timer;
	DWORD	flags;

	//scrolled windows's data
	DWORD	virtual_x;
	DWORD	virtual_y;
	DWORD	virtual_sizex;
	DWORD	virtual_sizey;
	DWORD	*virtual_controls_x;
	DWORD	*virtual_controls_y;
	DWORD	number_virtual_controls;
	DWORD	scroll_arrea_sizex;
	DWORD	scroll_arrea_sizey;
	DWORD	*horizontal_scroll;
	DWORD	*vertical_scroll;
	BYTE	scw_flags;
}pack;


typedef struct ControlScrolledWindow gui_scrolled_window_t;


struct ScrolledWindowData
{
	int	x;
	int	y;
	int	width;
	int	height;
}pack;


typedef struct ScrolledWindowData gui_scrolled_window_data_t;

////////////////////////////////////////////////////////////////
//      image
////////////////////////////////////////////////////////////////

struct ControlImage
{
	DWORD	*ctrl_proc;
	DWORD	*ctrl_fd;
	DWORD	*ctrl_bk;
	DWORD	*child_fd;
	DWORD	*child_bk;
	DWORD	*parent;
	DWORD	*main_parent;
	DWORD	ctrl_x;
	DWORD	ctrl_y;
	DWORD	ctrl_sizex;
	DWORD	ctrl_sizey;
	DWORD	ctrl_ID;
	DWORD	*active_control_for_keys;
	DWORD	*active_control_for_mouse;
	DWORD	*callback;
	DWORD	*finition;
	DWORD	*timer;
	DWORD	flags;

	char	bits_per_pixel;
	char	bytes_per_pixel;
	char	*img;
}pack;


typedef struct ControlImage gui_image_t;


struct ImageData
{
	int	x;
	int	y;
	int	width;
	int	height;
	char	bits_per_pixel;
}pack;


typedef struct ImageData gui_image_data_t;

////////////////////////////////////////////////////////////////
//      text
////////////////////////////////////////////////////////////////

struct ControlText
{
	DWORD	*ctrl_proc;
	DWORD	*ctrl_fd;
	DWORD	*ctrl_bk;
	DWORD	*child_fd;
	DWORD	*child_bk;
	DWORD	*parent;
	DWORD	*main_parent;
	DWORD	ctrl_x;
	DWORD	ctrl_y;
	DWORD	ctrl_sizex;
	DWORD	ctrl_sizey;
	DWORD	ctrl_ID;
	DWORD	*active_control_for_keys;
	DWORD	*active_control_for_mouse;
	DWORD	*callback;
	DWORD	*finition;
	DWORD	*timer;
	DWORD	flags;

	DWORD	*font;	
	DWORD	color;
	DWORD	background_color;
	char	*text;
	BYTE	txt_flags;
}pack;


typedef struct ControlText gui_text_t;


struct TextData
{
	int	x;
	int	y;
	DWORD	*font;	
	DWORD	color;
	DWORD	background_color;
	char	background;
	char	*text;	
}pack;


typedef struct TextData gui_text_data_t;
/////////////////////////////////////////////////////////////////
//           load libGUI library and link functions
/////////////////////////////////////////////////////////////////
void	LoadLibGUI(char *lib_path);

//**********************************************************************
//           		libGUI service functions
//**********************************************************************

DWORD stdcall (*LibGUIversion)(void);
char  stdcall (*InitLibGUI)(void);
void  stdcall (*LibGUImain)(parent_t *WindowParent);
void  stdcall (*QuitLibGUI)(parent_t *window);

void* stdcall (*CreateWindow)(void);
void  stdcall (*SetWindowSizeRequest)(parent_t *WindowParent,int size_x,int size_y);

void  stdcall (*PackControls)(void *Parent,void *control);
void  stdcall (*DestroyControl)(void *control);
void  stdcall (*SetControlSizeRequest)(void *Control,int new_size_x,int new_size_y);
int   stdcall (*GetControlSizeX)(void *Control);
int   stdcall (*GetControlSizeY)(void *Control);
void  stdcall (*SetControlNewPosition)(void *Control,int new_x,int new_y);
int   stdcall (*GetControlPositionX)(void *Control);
int   stdcall (*GetControlPositionY)(void *Control);
void* stdcall (*SetFocuse)(void *Control);
void  stdcall (*RedrawControl)(void *Control);
void  stdcall (*SpecialRedrawControl)(void *Control);

gui_callback_t* stdcall (*SetCallbackFunction)(void *Control,
					int event_name,void *callback_func,
					void *callback_func_data);
void  stdcall (*BlockCallbackFunction)(void *Control,gui_callback_t *callback_ID);
void  stdcall (*UnblockCallbackFunction)(void *Control,gui_callback_t *callback_ID);

void  stdcall (*SetIDL_Function)(parent_t *Parent,void *function,void *function_data);
void  stdcall (*DestroyIDL_Function)(parent_t *Parent);

gui_timer_t* stdcall (*SetTimerCallbackForFunction)(parent_t *parent_window,
					int time_tick,void *func,void *func_data);
void stdcall (*DestroyTimerCallbackForFunction)(gui_timer_t *timer);

gui_callbackevent_t* stdcall (*SetCallbackFunctionForEvent)(parent_t *parent_window,
					int event_type,void *func,void *func_data);
void stdcall (*DestroyCallbackFunctionForEvent)(gui_callbackevent_t *callback_event);

gui_button_t* stdcall (*CreateButton)(gui_button_data_t *info_for_control);
gui_button_t*	stdcall (*CreateButtonWithText)(gui_button_data_t *info,char *txt);

gui_progress_bar_t* stdcall (*CreateProgressBar)(gui_progress_bar_data_t *info_for_control);
void stdcall (*SetProgressBarPulse)(gui_progress_bar_t *ProgressBar,int time_update);
void stdcall (*ProgressBarSetText)(gui_progress_bar_t *pbar,char *txt);
char* stdcall (*ProgressBarGetText)(gui_progress_bar_t *pbar);

gui_scroll_bar_t* stdcall (*CreateHorizontalScrollBar)(gui_scroll_bar_data_t *info_for_control);
gui_scroll_bar_t* stdcall (*CreateVerticalScrollBar)(gui_scroll_bar_data_t *info_for_control);

gui_scrolled_window_t* stdcall (*CreateScrolledWindow)(gui_scrolled_window_data_t *info_for_control);
void stdcall (*ScrolledWindowPackControls)(gui_scrolled_window_t *parent,void *Control);

gui_image_t* stdcall (*CreateImage)(gui_image_data_t *info_for_control);

gui_text_t* stdcall (*CreateText)(gui_text_data_t *info_for_control);
void stdcall (*TextBackgroundOn)(gui_text_t *Text);
void stdcall (*TextBackgroundOff)(gui_text_t *Text);

font_t* stdcall (*LoadFont)(char *fullfontname);
void stdcall (*FreeFont)(font_t *font);


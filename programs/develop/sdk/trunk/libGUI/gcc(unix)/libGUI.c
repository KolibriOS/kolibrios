/*
	libGUI dinamic library SDK
	load library and link function
 */

#include "libGUI.h"


struct IMPORT
{
	char *name;
	void *function;
}__attribute__((packed));

typedef struct IMPORT import_t;

static char *sys_libGUI_path="/sys/lib/libGUI.obj";

static char* funcnames[] = {"LibGUIversion","InitLibGUI","LibGUImain","QuitLibGUI",

				"CreateWindow","SetWindowSizeRequest",

				"PackControls","DestroyControl","SetControlSizeRequest","GetControlSizeX",
				"GetControlSizeY","SetControlNewPosition","GetControlPositionX",
				"GetControlPositionY","SetFocuse","RedrawControl","SpecialRedrawControl",

				"SetCallbackFunction","BlockCallbackFunction","UnblockCallbackFunction",

				"SetIDL_Function","DestroyIDL_Function",

				"SetTimerCallbackForFunction","DestroyTimerCallbackForFunction",

				"SetCallbackFunctionForEvent","DestroyCallbackFunctionForEvent",

				"CreateButton","CreateButtonWithText",

				"CreateProgressBar","SetProgressBarPulse","ProgressBarSetText","ProgressBarGetText",

				"CreateHorizontalScrollBar","CreateVerticalScrollBar",

				"CreateScrolledWindow","ScrolledWindowPackControls",

				"CreateImage",

				"CreateText","TextBackgroundOn","TextBackgroundOff",

				"LoadFont","FreeFont"};


static inline void* gui_ksys_load_dll(char *path)
{
	void	*dll_export;

     __asm__ __volatile__(
	"int $0x40"
	:"=a"(dll_export)
	:"a"(68),"b"(19),"c"(path));

	return(dll_export);
}

static inline void gui_ksys_debug_out(int c)
{
     __asm__ __volatile__(
	"int $0x40"
	:
	:"a"(63),"b"(1),"c"(c));
}

static void gui_debug_out_str(char *s)
{

	while(*s)
	{
		if (*s=='\n') gui_ksys_debug_out(13);

		gui_ksys_debug_out(*s);
		s++;
	}
}

static int gui_strcmp(const char* string1, const char* string2)
{
	while (1)
	{
		if (*string1<*string2)
			return -1;
		if (*string1>*string2)
			return 1;
		if (*string1=='\0')
			return 0;
		string1++;
		string2++;
	}
}

static void* gui_cofflib_getproc(import_t *lib, char *name)
{
	int i;

	for(i = 0; lib[i].name && gui_strcmp(name, lib[i].name); i++);

	if(lib[i].name)	return lib[i].function;
			else	return NULL;
}

static inline void gui_ksys_exit(int value)
{
     __asm__ __volatile__(
	"int $0x40"
	:
	:"a"(-1),"b"(value));
}

void link_libGUI(import_t *exp,char **imports)
{
	LibGUIversion=(DWORD stdcall (*)(void))
			gui_cofflib_getproc(exp,imports[0]);
	InitLibGUI=(char stdcall (*)(void))
			gui_cofflib_getproc(exp,imports[1]);
	LibGUImain=(void stdcall (*)(parent_t *WindowParent))
			gui_cofflib_getproc(exp,imports[2]);
	QuitLibGUI=(void stdcall (*)(parent_t *window))
			gui_cofflib_getproc(exp,imports[3]);
	CreateWindow=(void* stdcall (*)(void))
			gui_cofflib_getproc(exp,imports[4]);
	SetWindowSizeRequest=(void stdcall (*)(parent_t *WindowParent,int size_x,int size_y))
			gui_cofflib_getproc(exp,imports[5]);
	PackControls=(void stdcall (*)(void *Parent,void *control))
			gui_cofflib_getproc(exp,imports[6]);
	DestroyControl=(void stdcall (*)(void *control))
			gui_cofflib_getproc(exp,imports[7]);
	SetControlSizeRequest=(void stdcall (*)(void *Control,int new_size_x,int new_size_y))
			gui_cofflib_getproc(exp,imports[8]);
	GetControlSizeX=(int stdcall (*)(void *Control))
			gui_cofflib_getproc(exp,imports[9]);
	GetControlSizeY=(int stdcall (*)(void *Control))
			gui_cofflib_getproc(exp,imports[10]);
	SetControlNewPosition=(void stdcall (*)(void *Control,int new_x,int new_y))
			gui_cofflib_getproc(exp,imports[11]);
	GetControlPositionX=(int stdcall (*)(void *Control))
			gui_cofflib_getproc(exp,imports[12]);
	GetControlPositionY=(int stdcall (*)(void *Control))
			gui_cofflib_getproc(exp,imports[13]);
	SetFocuse=(void* stdcall (*)(void *Control))
			gui_cofflib_getproc(exp,imports[14]);
	RedrawControl=(void stdcall (*)(void *Control))
			gui_cofflib_getproc(exp,imports[15]);
	SpecialRedrawControl=(void stdcall (*)(void *Control))
			gui_cofflib_getproc(exp,imports[16]);
	SetCallbackFunction=(gui_callback_t* stdcall (*)(void *Control,
					int event_name,void *callback_func,void *callback_func_data))
			gui_cofflib_getproc(exp,imports[17]);
	BlockCallbackFunction=(void stdcall (*)(void *Control,gui_callback_t *callback_ID))
			gui_cofflib_getproc(exp,imports[18]);
	UnblockCallbackFunction=(void stdcall (*)(void *Control,gui_callback_t *callback_ID))
			gui_cofflib_getproc(exp,imports[19]);
	SetIDL_Function=(void stdcall (*)(parent_t *Parent,void *function,void *function_data))
			gui_cofflib_getproc(exp,imports[20]);
	DestroyIDL_Function=(void stdcall (*)(parent_t *Parent))
			gui_cofflib_getproc(exp,imports[21]);
	SetTimerCallbackForFunction=(gui_timer_t* stdcall (*)(parent_t *parent_window,
					int time_tick,void *func,void *func_data))
			gui_cofflib_getproc(exp,imports[22]);
	DestroyTimerCallbackForFunction=(void stdcall (*)(gui_timer_t *timer))
			gui_cofflib_getproc(exp,imports[23]);
	SetCallbackFunctionForEvent=(gui_callbackevent_t* stdcall (*)(parent_t *parent_window,
					int event_type,void *func,void *func_data))
			gui_cofflib_getproc(exp,imports[24]);
	DestroyCallbackFunctionForEvent=(void stdcall (*)(gui_callbackevent_t *callback_event))
			gui_cofflib_getproc(exp,imports[25]);
	CreateButton=(gui_button_t* stdcall (*)(gui_button_data_t *info_for_control))
			gui_cofflib_getproc(exp,imports[26]);
	CreateButtonWithText=(gui_button_t* stdcall (*)(gui_button_data_t *info,char *txt))
			gui_cofflib_getproc(exp,imports[27]);
	CreateProgressBar=(gui_progress_bar_t* stdcall (*)(gui_progress_bar_data_t *info_for_control))
			gui_cofflib_getproc(exp,imports[28]);
	SetProgressBarPulse=(void stdcall (*)(gui_progress_bar_t *ProgressBar,int time_update))
			gui_cofflib_getproc(exp,imports[29]);
	ProgressBarSetText=(void stdcall (*)(gui_progress_bar_t *pbar,char *txt))
			gui_cofflib_getproc(exp,imports[30]);
	ProgressBarGetText=(char* stdcall (*)(gui_progress_bar_t *pbar))
			gui_cofflib_getproc(exp,imports[31]);
	CreateHorizontalScrollBar=(gui_scroll_bar_t* stdcall (*)(gui_scroll_bar_data_t *info_for_control))
			gui_cofflib_getproc(exp,imports[32]);
	CreateVerticalScrollBar=(gui_scroll_bar_t* stdcall (*)(gui_scroll_bar_data_t *info_for_control))
			gui_cofflib_getproc(exp,imports[33]);
	CreateScrolledWindow=(gui_scrolled_window_t* stdcall (*)(gui_scrolled_window_data_t *info_for_control))
			gui_cofflib_getproc(exp,imports[34]);
	ScrolledWindowPackControls=(void stdcall (*)(gui_scrolled_window_t *parent,void *Control))
			gui_cofflib_getproc(exp,imports[35]);
	CreateImage=(gui_image_t* stdcall (*)(gui_image_data_t *info_for_control))
			gui_cofflib_getproc(exp,imports[36]);
	CreateText=(gui_text_t* stdcall (*)(gui_text_data_t *info_for_control))
			gui_cofflib_getproc(exp,imports[37]);
	TextBackgroundOn=(void stdcall (*)(gui_text_t *Text))
			gui_cofflib_getproc(exp,imports[38]);
	TextBackgroundOff=(void stdcall (*)(gui_text_t *Text))
			gui_cofflib_getproc(exp,imports[39]);
	LoadFont=(font_t* stdcall (*)(char *fullfontname))
			gui_cofflib_getproc(exp,imports[40]);
	FreeFont=(void stdcall (*)(font_t *font))
			gui_cofflib_getproc(exp,imports[41]);
}

void	LoadLibGUI(char *lib_path)
{
	import_t	*export;
	char		*path;

	if (lib_path==NULL)
	{
		path=sys_libGUI_path;
		export=(import_t*)gui_ksys_load_dll(path);
	}
	else
	{
		path=lib_path;
		export=(import_t*)gui_ksys_load_dll(path);
	}

	if (export==NULL)
	{
		gui_debug_out_str("\ncan't load lib=");
		gui_debug_out_str(path);
		gui_ksys_exit(0);
	}
	else
	{
		link_libGUI(export,funcnames);
		if (InitLibGUI())
		{
			gui_debug_out_str("\ncan't initialize libGUI");
			gui_ksys_exit(0);
		}
	}
}





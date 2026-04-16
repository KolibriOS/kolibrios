;-----------------------------------------------------------------------
;                    assembler SDK for libGUI
;-----------------------------------------------------------------------

HEADER_SIZE				      =  18*4
NULL					      =  0

;///////////////////////////////////////////////////////////////////////
;//               libGUI system messages types
;///////////////////////////////////////////////////////////////////////
MESSAGE_FULL_REDRAW_ALL 		      =  1
MESSAGE_KEYS_EVENT			      =  2
MESSAGE_SPECIALIZED			      =  3
MESSAGE_SET_FOCUSE			      =  4
MESSAGE_CHANGE_FOCUSE			      =  5
MESSAGE_MOUSE_EVENT			      =  6
MESSAGE_CHANGE_POSITION_EVENT		      =  7
MESSAGE_CHANGESIZE_EVENT		      =  8
MESSAGE_CALL_TIMER_EVENT		      =  9
MESSAGE_FULL_REDRAW_ALL_WITH_FINITION	      =  10
MESSAGE_SET_MAIN_PARENT 		      =  11
MESSAGE_DESTROY_CONTROL 		      =  -1

;///////////////////////////////////////////////////////////////////////
;//                         system keys states
;///////////////////////////////////////////////////////////////////////
KEY_DOWN				      =  16
KEY_UP					      =  17
KEY_HOTKEY				      =  18
;///////////////////////////////////////////////////////////////////////
;//                       system mouse buttons states
;///////////////////////////////////////////////////////////////////////
MOUSE_LEFT_BUTTON_DOWN			      =  19
MOUSE_LEFT_BUTTON_UP			      =  20
MOUSE_RIGHT_BUTTON_DOWN 		      =  21
MOUSE_RIGHT_BUTTON_UP			      =  22
MOUSE_MIDDLE_BUTTON_DOWN		      =  23
MOUSE_MIDDLE_BUTTON_UP			      =  24
MOUSE_4_BUTTON_DOWN			      =  25
MOUSE_4_BUTTON_UP			      =  26
MOUSE_5_BUTTON_DOWN			      =  27
MOUSE_5_BUTTON_UP			      =  28

;---------------------------------------------------------------------
;                 CONNECT EVENTS FOR CALLBACKs
;---------------------------------------------------------------------

;////////////////////////////////////////////////////////////////
;//                 connect events for button
;////////////////////////////////////////////////////////////////
BUTTON_ENTER_EVENT			      =  29
BUTTON_LEAVE_EVENT			      =  30
BUTTON_PRESSED_EVENT			      =  31
BUTTON_RELEASED_EVENT			      =  32

;////////////////////////////////////////////////////////////////
;//                connect events for scroll bar
;////////////////////////////////////////////////////////////////
SCROLLBAR_CHANGED_EVENT 		      =  33

;////////////////////////////////////////////////////////////////
;//            connect events for main parent window
;////////////////////////////////////////////////////////////////
DELETE_EVENT				      =  36

;-----------------------------------------------------------------------
;------------------------libGUI functions-------------------------------
;-----------------------------------------------------------------------

;DWORD LibGUIversion(void);
macro LibGUIversion
{
	call [imp_LibGUIversion]
}
;char InitLibGUI(void);
macro InitLibGUI
{
	call [imp_InitLibGUI]
}

;void LibGUImain(parent_t *WindowParent);
macro LibGUImain WindowParent
{
	pushd WindowParent
	call [imp_LibGUImain]
	add esp,1*4
}

;void QuitLibGUI(parent_t *window);
macro QuitLibGUI window
{
	pushd window
	call [imp_QuitLibGUI]
	add esp,1*4
}
;void* CreateWindow(void)
macro CreateWindow
{
	call [imp_CreateWindow]
}
;void SetWindowSizeRequest(parent_t *WindowParent,int size_x,int size_y)
macro SetWindowSizeRequest WindowParent,sizex,sizey
{
	pushd sizey
	pushd sizex
	pushd WindowParent
	call [imp_SetWindowSizeRequest]
	add esp,3*4
}
;void PackControls(void *Parent,void *control);
macro PackControls Parent,control
{
	pushd control
	pushd Parent
	call [imp_PackControls]
	add esp,2*4
}
;void DestroyControl(void *control);
macro DestroyControl control
{
	pushd control
	call [imp_DestroyControl]
	add esp,1*4
}
;void SetControlSizeRequest(void *Control,int new_size_x,int new_size_y);
macro SetControlSizeRequest Control,new_size_x,new_size_y
{
	pushd new_size_y
	pushd new_size_x
	pushd Control
	call [imp_SetControlSizeRequest]
	add esp 3*4
}
;int GetControlSizeX(void *Control);
macro GetControlSizeX Control
{
	pushd Control
	call [imp_GetControlSizeX]
	add esp 1*4
}
;int GetControlSizeY(void *Control);
macro GetControlSizeY Control
{
	pushd Control
	call [imp_GetControlSizeY]
	add esp 1*4
}
;void SetControlNewPosition(void *Control,int new_x,int new_y)
macro SetControlNewPosition Control,new_x,new_y
{
	pushd new_y
	pushd new_x
	pushd Control
	call [imp_SetControlNewPosition]
	add esp,3*4
}
;int GetControlPositionX(void *Control);
macro GetControlPositionX Control
{
	pushd Control
	call [imp_GetControlPositionX]
	add esp,1*4
}
;int GetControlPositionY(void *Control);
macro GetControlPositionY Control
{
	pushd Control
	call [imp_GetControlPositionY]
	add esp,1*4
}
;void* SetFocuse(void *Control);
macro SetFocuse Control
{
	pushd Control
	call [imp_SetFocuse]
	add esp,1*4
}
;void RedrawControl(void *Control)
macro RedrawControl Control
{
	pushd Control
	call [imp_RedrawControl]
	add esp,1*4
}
;void SpecialRedrawControl(void *Control)
macro SpecialRedrawControl Control
{
	pushd Control
	call [imp_SpecialRedrawControl]
	add esp,1*4
}
;SetCallbackFunction(void *Control,int event_name,void *callback_func,void *callback_func_data);
macro SetCallbackFunction Control,event_name,callback_func,callback_func_data
{
	pushd callback_func_data
	pushd callback_func
	pushd event_name
	pushd Control
	call [imp_SetCallbackFunction]
	add esp,4*4
}
;BlockCallbackFunction(void *Control,gui_callback_t *callback_ID)
macro BlockCallbackFunction Control,callback_ID
{
	pushd callback_ID
	pushd Control
	call [imp_BlockCallbackFunction]
	add esp,2*4
}
;void UnblockCallbackFunction(void *Control,gui_callback_t *callback_ID)
macro UnblockCallbackFunction Control,callback_ID
{
	pushd callback_ID
	pushd Control
	call [imp_UnblockCallbackFunction]
	add esp,2*4
}
;void SetIDL_Function(parent_t *Parent,void *function,void *function_data)
macro SetIDL_Function Parent,function,function_data
{
	pushd function_data
	pushd function
	pushd Parent
	call [imp_SetIDL_Function]
	add esp,3*4
}
;void DestroyIDL_Function(parent_t *Parent)
macro DestroyIDL_Function Parent
{
	pushd Parent
	call [imp_DestroyIDL_Function]
	add eso,1*4
}
;SetTimerCallbackForFunction(parent_t *parent_window,int time_tick,void *func,void *func_data)
macro SetTimerCallbackForFunction parent_window,time_tick,func,func_data
{
	pushd func_data
	pushd func
	pushd time_tick
	pushd parent_window
	call [imp_SetTimerCallbackForFunction]
	add esp,4*4
}
;DestroyTimerCallbackForFunction(gui_timer_t *timer)
macro DestroyTimerCallbackForFunction timer
{
	pushd timer
	call [imp_DestroyTimerCallbackForFunction]
	add esp,1*4
}
;SetCallbackFunctionForEvent)(parent_t *parent_window,int event_type,void *func,void *func_data)
macro SetCallbackFunctionForEvent parent_window,event_type,func,func_data
{
	pushd func_data
	pushd func
	pushd event_type
	pushd parent_window
	call [imp_SetCallbackFunctionForEvent]
	add esp,4*4
}
;DestroyCallbackFunctionForEvent(gui_callbackevent_t *callback_event)
macro DestroyCallbackFunctionForEvent callback_event
{
	pushd callback_event
	call [imp_DestroyCallbackFunctionForEvent]
	add esp,1*4
}
;CreateButton(gui_button_data_t *info_for_control)
macro CreateButton info_for_control
{
	pushd info_for_control
	call [imp_CreateButton]
	add esp,1*4
}
;CreateButtonWithText(gui_button_data_t *info,char *txt)
macro CreateButtonWithText info,txt
{
	pushd txt
	pushd info
	call [imp_CreateButtonWithText]
	add esp,2*4
}
;CreateProgressBar(gui_progress_bar_data_t *info_for_control)
macro CreateProgressBar info_for_control
{
	pushd info_for_control
	call [imp_CreateProgressBar]
	add esp,1*4
}
;SetProgressBarPulse)(gui_progress_bar_t *ProgressBar,int time_update)
macro SetProgressBarPulse ProgressBar,time_update
{
	pushd time_update
	pushd ProgressBar
	call [imp_SetProgressBarPulse]
	add esp,2*4
}
;ProgressBarSetText(gui_progress_bar_t *pbar,char *txt)
macro ProgressBarSetText pbar,txt
{
	pushd txt
	pushd pbar
	call [imp_ProgressBarSetText]
	add esp,2*4
}
;ProgressBarGetText(gui_progress_bar_t *pbar)
macro ProgressBarGetText pbar
{
	pushd pbar
	call [imp_ProgressBarGetText]
	add esp,1*4
}
;CreateHorizontalScrollBar(gui_scroll_bar_data_t *info_for_control)
macro CreateHorizontalScrollBar info_for_control
{
	pushd info_for_control
	call [imp_CreateHorizontalScrollBar]
	add esp,1*4
}
;CreateVerticalScrollBar(gui_scroll_bar_data_t *info_for_control)
macro CreateVerticalScrollBar info_for_control
{
	pushd info_for_control
	call [imp_CreateVerticalScrollBar]
	add esp,1*4
}
;CreateScrolledWindow)(gui_scrolled_window_data_t *info_for_control)
macro CreateScrolledWindow info_for_control
{
	pushd info_for_control
	call [imp_CreateScrolledWindow]
	add esp,1*4
}
;ScrolledWindowPackControls(gui_scrolled_window_t *parent,void *Control)
macro ScrolledWindowPackControls parent,Control
{
	pushd Control
	pushd parent
	call [imp_ScrolledWindowPackControls]
	add esp,2*4
}
;CreateImage(gui_image_data_t *info_for_control)
macro CreateImage info_for_control
{
	pushd info_for_control
	call [imp_CreateImage]
	add esp,1*4
}
;CreateText(gui_text_data_t *info_for_control)
macro CreateText info_for_control
{
	pushd info_for_control
	call [imp_CreateText]
	add esp,1*4
}
;TextBackgroundOn(gui_text_t *Text)
macro TextBackgroundOn Txt
{
	pushd Txt
	call [imp_TextBackgroundOn]
	add esp,1*4
}
;TextBackgroundOff(gui_text_t *Text)
macro TextBackgroundOff Txt
{
	pushd Txt
	call [imp_TextBackgroundOff]
	add esp,1*4
}
;LoadFont(char *fullfontname)
macro LoadFont fullfontname
{
	pushd fullfontname
	call [imp_LoadFont]
	add esp,1*4
}
;FreeFont(font_t *font)
macro FreeFont font
{
	pushd font
	call [imp_FreeFont]
	add esp,1*4
}

;----------------------------------------------------------------
;----------------------debug output------------------------------
;----------------------------------------------------------------
;IN
;esi=stirng
align 4
gui_ksys_debug_out_str:

       pushad
       mov eax,63
       mov ebx,1

       next_simbol_print:

	      xor edx,edx
	      mov dl,[esi]
	      test dl,dl
	      jz exit_print_str

	      cmp dl,10
	      jne no_new_line
		     mov ecx,13
		     int 0x40
	      no_new_line:

	      xor ecx,ecx
	      mov cl, dl
	      int 0x40

	      add esi,1

       jmp next_simbol_print

       exit_print_str:
       popad

       ret

;-------------------------------------------------------------------
;-------------------get proc from export table----------------------
;-------------------------------------------------------------------
;INPUT
;ebx=pointer to export table
;esi=pointer to function name
;OUTPUT
;eax=pointer to function
align 4
gui_ksys_cofflib_getproc:

	next_name_check:

	mov ecx,[ebx]

	test ecx,ecx
	jz end_export

		push esi
		;cmp export string with name
		next_simbol_check:

		xor eax,eax
		mov al,[ecx];load byte of func name from export
		xor edx,edx
		mov dl,[esi]

		cmp al,dl
		jne exit_check_simbol
		test al,al
		jne no_end_string
			mov eax,[ebx+4]
			pop esi
			jmp end_export
		 no_end_string:

		add ecx,1
		add esi,1
		jmp next_simbol_check

		exit_check_simbol:
		pop esi

	add ebx,8
	jmp next_name_check

	end_export:

	ret

;-------------------------------------------------------------------
;-----------------------Link libGUI---------------------------------
;-------------------------------------------------------------------

link_libGUI:

	mov edi,eax;save export table pointer
	mov esi,libGUI_imp

	next_libGUI_func_link:

		push esi
		mov eax,esi
		mov esi,[eax]

		mov ebx,edi;pointer to export table
		call gui_ksys_cofflib_getproc
		pop esi
		mov [esi],eax ;func proc

	add esi,4
	cmp [esi],dword 0
	jne next_libGUI_func_link

	ret

;////////////////////////////////////////////////////////////////
;//                     Load libGUI
;////////////////////////////////////////////////////////////////
;IN
;pointer to path or
LoadLibGUI:

	push eax
	push ebx
	push ecx
	push edx
	push esi
	push edi

	mov ecx,[esp+28];path

	test ecx,ecx
	jnz have_other_path
		mov ecx,sys_libGUI_path
	have_other_path:

	;try to load libGUI library
	mov eax,68
	mov ebx,19
	int 0x40

	test eax,eax
	jnz libGUI_loaded_successfully
		mov esi,message_cant_load_lib
		call gui_ksys_debug_out_str

		xor eax,eax
		dec eax
		int 0x40;system exit program
	libGUI_loaded_successfully:

	call link_libGUI

	mov eax,68
	mov ebx,11
	int 0x40

	InitLibGUI

	test eax,eax
	jz libGUI_initialized_successfully
		mov esi,message_cant_initialize_lib
		call gui_ksys_debug_out_str

		xor eax,eax
		dec eax
		int 0x40;system exit program
	libGUI_initialized_successfully:

	pop edi
	pop esi
	pop edx
	pop ecx
	pop ebx
	pop eax

	ret 4

;////////////////////////////////////////////////////////////////
;//                     font type structure
;////////////////////////////////////////////////////////////////
struc font_t
{
 .fnt_draw			rd 1;pointer
 .fnt_unpacker			rd 1;pointer
 .fnt_fd			rd 1;pointer
 .fnt_bk			rd 1;pointer
 .sizex 			rd 1;
 .sizey 			rd 1;
 .size				rd 1;
 .encoding_type 		rd 1;
 .font				rd 1;pointer
 .fnt_name			rd 1;pointer
 .type				rd 1;
 .flags 			rd 1;
}

;////////////////////////////////////////////////////////////////
;//                    header of parent of control
;////////////////////////////////////////////////////////////////
struc parent_t
{
 .ctrl_proc			 rd 1;pointer
 .ctrl_fd			 rd 1;pointer
 .ctrl_bk			 rd 1;pointer
 .child_fd			 rd 1;pointer
 .child_bk			 rd 1;pointer
 .parent			 rd 1;pointer
 .main_parent			 rd 1;pointer
 .ctrl_x			 rd 1;
 .ctrl_y			 rd 1;
 .ctrl_sizex			 rd 1;
 .ctrl_sizey			 rd 1;
 .ctrl_ID			 rd 1;
 .active_control_for_keys	 rd 1;pointer
 .active_control_for_mouse	 rd 1;pointer
 .callback			 rd 1;pointer
 .finition			 rd 1;pointer
 .timer 			 rd 1;pointer
 .flags 			 rd 1;

 .control_for_callback_function  rd 1;array of pointers
 .callback_for_control_callback  rd 1;array of pointers
 .number_callbacks		 rd 1;
 .global_active_control_for_keys rd 1;pointer
 .message			 rd 1;pointer
 .timer_bk			 rd 1;pointer
 .timer_fd			 rd 1;pointer
 .number_timers_for_controls	 rd 1;
 .calev_bk			 rd 1;pointer
 .calev_fd			 rd 1;pointer
 .IDL_func			 rd 1;pointer
 .IDL_func_data 		 rd 1;pointer
}

;////////////////////////////////////////////////////////////////
;//                     header of control
;////////////////////////////////////////////////////////////////
struc header_t
{
 .ctrl_proc			rd 1;pointer
 .ctrl_fd			rd 1;pointer
 .ctrl_bk			rd 1;pointer
 .child_fd			rd 1;pointer
 .child_bk			rd 1;pointer
 .parent			rd 1;pointer
 .main_parent			rd 1;pointer
 .ctrl_x			rd 1;
 .ctrl_y			rd 1;
 .ctrl_sizex			rd 1;
 .ctrl_sizey			rd 1;
 .ctrl_ID			rd 1;
 .active_control_for_keys	rd 1;pointer
 .active_control_for_mouse	rd 1;pointer
 .callback			rd 1;pointer
 .finition			rd 1;pointer
 .timer 			rd 1;pointer
 .flags 			rd 1;
}

;////////////////////////////////////////////////////////////////
;//      callback structure for callback function of control
;////////////////////////////////////////////////////////////////
struc gui_callback_t
{
 .clb_bk			rd 1;pointer
 .clb_fd			rd 1;pointer
 .clb_control			rd 1;pointer
 .func				rd 1;pointer
 .func_data			rd 1;pointer
 .connect_event 		rd 1;
 .flags 			rd 1;
}

;////////////////////////////////////////////////////////////////
;//                         timer
;////////////////////////////////////////////////////////////////
struc gui_timer_t
{
 .tmr_bk			rd 1;pointer
 .tmr_fd			rd 1;pointer
 .tmr_parent			rd 1;pointer
 .func				rd 1;pointer
 .func_data			rd 1;pointer
 .last_time			rd 1;
 .time_tick			rd 1;
 .flags 			rd 1;
}

;////////////////////////////////////////////////////////////////
;//               structure for callback events
;////////////////////////////////////////////////////////////////
struc gui_callbackevent_t
{
 .calev_bk			rd 1;pointer
 .calev_fd			rd 1;pointer
 .calev_parent			rd 1;pointer
 .func				rd 1;pointer
 .func_data			rd 1;pointer
 .event_type			rd 1;
}

;////////////////////////////////////////////////////////////////
;//               type of data - structure message
;////////////////////////////////////////////////////////////////
struc gui_message_t
{
 .type				rd 1;
 .arg1				rd 1;
 .arg2				rd 1;
 .arg3				rd 1;
 .arg4				rd 1;
}

;////////////////////////////////////////////////////////////////
;//                         button
;////////////////////////////////////////////////////////////////

struc gui_button_t
{
 .ctrl_proc			rd 1;pointer
 .ctrl_fd			rd 1;pointer
 .ctrl_bk			rd 1;pointer
 .child_fd			rd 1;pointer
 .child_bk			rd 1;pointer
 .parent			rd 1;pointer
 .main_parent			rd 1;pointer
 .ctrl_x			rd 1;
 .ctrl_y			rd 1;
 .ctrl_sizex			rd 1;
 .ctrl_sizey			rd 1;
 .ctrl_ID			rd 1;
 .active_control_for_keys	rd 1;pointer
 .active_control_for_mouse	rd 1;pointer
 .callback			rd 1;pointer
 .finition			rd 1;pointer
 .timer 			rd 1;pointer
 .flags 			rd 1;

 .btn_flags			rb 1;
}

;information for creating control Button

struc gui_button_data_t
{
 .x				rd 1;
 .y				rd 1;
 .wight 			rd 1;
 .height			rd 1;
}

;////////////////////////////////////////////////////////////////
;//                      scroll bar
;////////////////////////////////////////////////////////////////

struc gui_scroll_bar_t
{
 .ctrl_proc			rd 1;pointer
 .ctrl_fd			rd 1;pointer
 .ctrl_bk			rd 1;pointer
 .child_fd			rd 1;pointer
 .child_bk			rd 1;pointer
 .parent			rd 1;pointer
 .main_parent			rd 1;pointer
 .ctrl_x			rd 1;
 .ctrl_y			rd 1;
 .ctrl_sizex			rd 1;
 .ctrl_sizey			rd 1;
 .ctrl_ID			rd 1;
 .active_control_for_keys	rd 1;pointer
 .active_control_for_mouse	rd 1;pointer
 .callback			rd 1;pointer
 .finition			rd 1;pointer
 .timer 			rd 1;pointer
 .flags 			rd 1;

 .ruller_size			rd 1;float
 .ruller_pos			rd 1;float
 .ruller_step			rd 1;float
 .scb_flags			rb 1;
}

struc gui_scroll_bar_data_t
{
 .x				rd 1;
 .y				rd 1;
 .wight 			rd 1;
 .height			rd 1;
 .ruller_size			rd 1;float
 .ruller_pos			rd 1;float
 .ruller_step			rd 1;float
}

;////////////////////////////////////////////////////////////////
;//                     progressbar
;////////////////////////////////////////////////////////////////

struc gui_progress_bar_t
{
 .ctrl_proc			rd 1;pointer
 .ctrl_fd			rd 1;pointer
 .ctrl_bk			rd 1;pointer
 .child_fd			rd 1;pointer
 .child_bk			rd 1;pointer
 .parent			rd 1;pointer
 .main_parent			rd 1;pointer
 .ctrl_x			rd 1;
 .ctrl_y			rd 1;
 .ctrl_sizex			rd 1;
 .ctrl_sizey			rd 1;
 .ctrl_ID			rd 1;
 .active_control_for_keys	rd 1;pointer
 .active_control_for_mouse	rd 1;pointer
 .callback			rd 1;pointer
 .finition			rd 1;pointer
 .timer 			rd 1;pointer
 .flags 			rd 1;

 .progress			rd 1;float
 .prb_flags			rb 1;
}

struc gui_progress_bar_data_t
{
 .x				rd 1;
 .y				rd 1;
 .wight 			rd 1;
 .height			rd 1;
 .progress			rd 1;float
}

;////////////////////////////////////////////////////////////////
;//                  scrolled window
;////////////////////////////////////////////////////////////////

struc gui_scrolled_window_t
{
 .ctrl_proc			rd 1;pointer
 .ctrl_fd			rd 1;pointer
 .ctrl_bk			rd 1;pointer
 .child_fd			rd 1;pointer
 .child_bk			rd 1;pointer
 .parent			rd 1;pointer
 .main_parent			rd 1;pointer
 .ctrl_x			rd 1;
 .ctrl_y			rd 1;
 .ctrl_sizex			rd 1;
 .ctrl_sizey			rd 1;
 .ctrl_ID			rd 1;
 .active_control_for_keys	rd 1;pointer
 .active_control_for_mouse	rd 1;pointer
 .callback			rd 1;pointer
 .finition			rd 1;pointer
 .timer 			rd 1;pointer
 .flags 			rd 1;

 .virtual_x			rd 1;
 .virtual_y			rd 1;
 .virtual_sizex 		rd 1;
 .virtual_sizey 		rd 1;
 .virtual_controls_x		rd 1;pointer
 .virtual_controls_y		rd 1;pointer
 .number_virtual_controls	rd 1;
 .scroll_arrea_sizex		rd 1;
 .scroll_arrea_sizey		rd 1;
 .horizontal_scroll		rd 1;pointer
 .vertical_scroll		rd 1;pointer
 .scw_flags			rb 1;
}

struc gui_scrolled_window_data_t
{
 .x				rd 1;
 .y				rd 1;
 .wight 			rd 1;
 .height			rd 1;
}

;////////////////////////////////////////////////////////////////
;//                           image
;////////////////////////////////////////////////////////////////

struc gui_image_t
{
 .ctrl_proc			rd 1;pointer
 .ctrl_fd			rd 1;pointer
 .ctrl_bk			rd 1;pointer
 .child_fd			rd 1;pointer
 .child_bk			rd 1;pointer
 .parent			rd 1;pointer
 .main_parent			rd 1;pointer
 .ctrl_x			rd 1;
 .ctrl_y			rd 1;
 .ctrl_sizex			rd 1;
 .ctrl_sizey			rd 1;
 .ctrl_ID			rd 1;
 .active_control_for_keys	rd 1;pointer
 .active_control_for_mouse	rd 1;pointer
 .callback			rd 1;pointer
 .finition			rd 1;pointer
 .timer 			rd 1;pointer
 .flags 			rd 1;

 .bits_per_pixel		rb 1;
 .bytes_per_pixel		rb 1;
 .img				rd 1;pointer
}


struc gui_image_data_t
{
 .x				rd 1;
 .y				rd 1;
 .wight 			rd 1;
 .height			rd 1;
 .bits_per_pixel		rb 1;
}

;////////////////////////////////////////////////////////////////
;//                         text
;////////////////////////////////////////////////////////////////

struc gui_text_t
{
 .ctrl_proc			rd 1;pointer
 .ctrl_fd			rd 1;pointer
 .ctrl_bk			rd 1;pointer
 .child_fd			rd 1;pointer
 .child_bk			rd 1;pointer
 .parent			rd 1;pointer
 .main_parent			rd 1;pointer
 .ctrl_x			rd 1;
 .ctrl_y			rd 1;
 .ctrl_sizex			rd 1;
 .ctrl_sizey			rd 1;
 .ctrl_ID			rd 1;
 .active_control_for_keys	rd 1;pointer
 .active_control_for_mouse	rd 1;pointer
 .callback			rd 1;pointer
 .finition			rd 1;pointer
 .timer 			rd 1;pointer
 .flags 			rd 1;

 .font				rd 1;pointer
 .color 			rd 1;
 .background_color		rd 1;
 .text				rd 1;pointer
 .txt_flags			rb 1;
}

struc gui_text_data_t
{
 .x				rd 1;
 .y				rd 1;
 .font				rd 1;pointer
 .color 			rd 1;
 .background_color		rd 1;
 .background			rb 1;
 .text				rd 1;pointer
}

;//**********************************************************************
;//                   libGUI service functions
;//**********************************************************************

;default system path to libGUI
sys_libGUI_path 			db '/sys/lib/libGUI.obj',0
message_cant_load_lib			db 'cant load lib',13,10,0
message_cant_initialize_lib		db 'cant initialize libGUI',13,10,0

szLibGUIversion 			db 'LibGUIversion',0
szInitLibGUI				db 'InitLibGUI',0
szLibGUImain				db 'LibGUImain',0
szQuitLibGUI				db 'QuitLibGUI',0

szCreateWindow				db 'CreateWindow',0
szSetWindowSizeRequest			db 'SetWindowSizeRequest',0

szPackControls				db 'PackControls',0
szDestroyControl			db 'DestroyControl',0
szSetControlSizeRequest 		db 'SetControlSizeRequest',0
szGetControlSizeX			db 'GetControlSizeX',0
szGetControlSizeY			db 'GetControlSizeY',0
szSetControlNewPosition 		db 'SetControlNewPosition',0
szGetControlPositionX			db 'GetControlPositionX',0
szGetControlPositionY			db 'GetControlPositionY',0
szSetFocuse				db 'SetFocuse',0
szRedrawControl 			db 'RedrawControl',0
szSpecialRedrawControl			db 'SpecialRedrawControl',0

szSetCallbackFunction			db 'SetCallbackFunction',0
szBlockCallbackFunction 		db 'BlockCallbackFunction',0
szUnblockCallbackFunction		db 'UnblockCallbackFunction',0

szSetIDL_Function			db 'SetIDL_Function',0
szDestroyIDL_Function			db 'DestroyIDL_Function',0

szSetTimerCallbackForFunction		db 'SetTimerCallbackForFunction',0
szDestroyTimerCallbackForFunction	db 'DestroyTimerCallbackForFunction',0

szSetCallbackFunctionForEvent		db 'SetCallbackFunctionForEvent',0
szDestroyCallbackFunctionForEvent	db 'DestroyCallbackFunctionForEvent',0

szCreateButton				db 'CreateButton',0
szCreateButtonWithText			db 'CreateButtonWithText',0

szCreateProgressBar			db 'CreateProgressBar',0
szSetProgressBarPulse			db 'SetProgressBarPulse',0
szProgressBarSetText			db 'ProgressBarSetText',0
szProgressBarGetText			db 'ProgressBarGetText',0

szCreateHorizontalScrollBar		db 'CreateHorizontalScrollBar',0
szCreateVerticalScrollBar		db 'CreateVerticalScrollBar',0

szCreateScrolledWindow			db 'CreateScrolledWindow',0
szScrolledWindowPackControls		db 'ScrolledWindowPackControls',0

szCreateImage				db 'CreateImage',0

szCreateText				db 'CreateText',0
szTextBackgroundOn			db 'TextBackgroundOn',0
szTextBackgroundOff			db 'TextBackgroundOff',0

szLoadFont				db 'LoadFont',0
szFreeFont				db 'FreeFont',0

libGUI_imp:

imp_LibGUIversion			    dd szLibGUIversion
imp_InitLibGUI				    dd szInitLibGUI
imp_LibGUImain				    dd szLibGUImain
imp_QuitLibGUI				    dd szQuitLibGUI

imp_CreateWindow			    dd szCreateWindow
imp_SetWindowSizeRequest		    dd szSetWindowSizeRequest

imp_PackControls			    dd szPackControls
imp_DestroyControl			    dd szDestroyControl
imp_SetControlSizeRequest		    dd szSetControlSizeRequest
imp_GetControlSizeX			    dd szGetControlSizeX
imp_GetControlSizeY			    dd szGetControlSizeY
imp_SetControlNewPosition		    dd szSetControlNewPosition
imp_GetControlPositionX 		    dd szGetControlPositionX
imp_GetControlPositionY 		    dd szGetControlPositionY
imp_SetFocuse				    dd szSetFocuse
imp_RedrawControl			    dd szRedrawControl
imp_SpecialRedrawControl		    dd szSpecialRedrawControl

imp_SetCallbackFunction 		    dd szSetCallbackFunction
imp_BlockCallbackFunction		    dd szBlockCallbackFunction
imp_UnblockCallbackFunction		    dd szUnblockCallbackFunction

imp_SetIDL_Function			    dd szSetIDL_Function
imp_DestroyIDL_Function 		    dd szDestroyIDL_Function

imp_SetTimerCallbackForFunction 	    dd szSetTimerCallbackForFunction
imp_DestroyTimerCallbackForFunction	    dd szDestroyTimerCallbackForFunction

imp_SetCallbackFunctionForEvent 	    dd szSetCallbackFunctionForEvent
imp_DestroyCallbackFunctionForEvent	    dd szDestroyCallbackFunctionForEvent

imp_CreateButton			    dd szCreateButton
imp_CreateButtonWithText		    dd szCreateButtonWithText

imp_CreateProgressBar			    dd szCreateProgressBar
imp_SetProgressBarPulse 		    dd szSetProgressBarPulse
imp_ProgressBarSetText			    dd szProgressBarSetText
imp_ProgressBarGetText			    dd szProgressBarGetText

imp_CreateHorizontalScrollBar		    dd szCreateHorizontalScrollBar
imp_CreateVerticalScrollBar		    dd szCreateVerticalScrollBar

imp_CreateScrolledWindow		    dd szCreateScrolledWindow
imp_ScrolledWindowPackControls		    dd szScrolledWindowPackControls

imp_CreateImage 			    dd szCreateImage

imp_CreateText				    dd szCreateText
imp_TextBackgroundOn			    dd szTextBackgroundOn
imp_TextBackgroundOff			    dd szTextBackgroundOff

imp_LoadFont				    dd szLoadFont
imp_FreeFont				    dd szFreeFont

					    dd 0


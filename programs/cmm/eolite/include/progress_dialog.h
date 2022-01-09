
#define COPYFORM_W 380
#define COPYFORM_H 100
#define PR_LEFT 14
#define PR_TOP  32
#define PR_W  COPYFORM_W-PR_LEFT-PR_LEFT
#define PR_H  18

proc_info Dialog_Form;
progress_bar copy_bar = {0,PR_LEFT,PR_TOP,PR_W,PR_H,0,0,1,0xFFFFFF,0x00FF00,0x555555};
checkbox remember_choice = { T_COPY_REMEMBER_CHOICE, false };

enum {
	REDRAW_FLAG,
	COPY_FLAG, 
	MOVE_FLAG, 
	DELETE_FLAG, 
};

enum {
	BTN_ABORT=2,
	BTN_REPLACE,
	BTN_SKIP
};

void DisplayOperationForm(int operation_flag)
{
	dword title;
	dword event_mode;
	if (operation_flag==COPY_FLAG) {
		title = T_COPY_WINDOW_TITLE;
		copy_bar.progress_color = 0x00FF00;
		copy_bar.value = 0; 
		copy_bar.max = 0;
	}
	else if (operation_flag==MOVE_FLAG) {
		title = T_MOVE_WINDOW_TITLE;
		copy_bar.progress_color = 0x00FF00;
		copy_bar.value = 0; 
		copy_bar.max = 0;
	}
	else if (operation_flag==DELETE_FLAG) {
		title = T_DELETE_WINDOW_TITLE;
		copy_bar.progress_color = 0xF17A65;
		copy_bar.value = 0; 
		copy_bar.max = 0;
	}
	copy_bar.frame_color = sc.line;
	
	if (copy_state == FILE_EXISTS) {
		event_mode = #WaitEvent;
	} else {
		event_mode = #CheckEvent;
	}
	event_mode();
	switch(EAX)
	{
		 case evButton:
			switch(GetButtonID())
			{
				case 1:
				case BTN_ABORT:
					DialogExit();
					break;
				case BTN_REPLACE:
					if (is_remember == true) {
						saved_state = FILE_REPLACE;
					}
					copy_state = FILE_REPLACE;
					break;
				case BTN_SKIP:
					if (is_remember == true) {
						saved_state = FILE_SKIP;
					}
					copy_state = FILE_SKIP;
					break;
				default:
					if (remember_choice.click(EAX+1)) {
						is_remember = remember_choice.checked;
					}
			}
			break;
		case evReDraw:
			DefineAndDrawWindow(Form.left+Form.width-200, Form.top+90, COPYFORM_W+9,
				skin_h+COPYFORM_H, 0x34, sc.work, title, 0);
			GetProcessInfo(#Dialog_Form, SelfInfo);
			DrawCaptButton(COPYFORM_W-PR_LEFT-101, PR_TOP+PR_H+6, 100,26, BTN_ABORT, sc.button, sc.button_text, T_COPY_ABORT);

			if (copy_state == FILE_EXISTS)
			{
				#define REPLACEY PR_TOP+PR_H+48
				draw_icon_16w(PR_LEFT, REPLACEY-3, 20);
				WriteText(PR_LEFT+25, REPLACEY, 0x90, sc.work_text, T_OVERWRITE_ALERT);
				DrawCaptButton(PR_LEFT, REPLACEY+30, 100,26, BTN_REPLACE, sc.button, sc.button_text, T_COPY_REPLACE);
				DrawCaptButton(PR_LEFT+110, REPLACEY+30, 100,26, BTN_SKIP, sc.button, sc.button_text, T_COPY_SKIP);
				remember_choice.draw(PR_LEFT+225, REPLACEY+37);
			}

			DrawRectangle3D(PR_LEFT-1, PR_TOP-1, PR_W+1, PR_H+1, sc.dark, sc.light);
	}
}

void DialogExit() {
	ActivateWindow(GetProcessSlot(Form.ID));
	ExitProcess();
}

void Operation_Draw_Progress(dword filename) {
	static int old_state;
	if (Dialog_Form.cwidth==0)
	{
		copy_bar.value++;
		return;
	}

	if (old_state != copy_state) {
		old_state = copy_state;
		if (copy_state == FILE_EXISTS) MoveSize(OLD,OLD,OLD,skin_h+COPYFORM_H+70);
		if (copy_state == FILE_DEFAULT) MoveSize(OLD,OLD,OLD,skin_h+COPYFORM_H);
	}

	DisplayOperationForm(REDRAW_FLAG);

	progressbar_draw stdcall (#copy_bar);
	if (copy_state == FILE_DEFAULT)
	{
		DrawBar(PR_LEFT, PR_TOP-20, COPYFORM_W-PR_LEFT, 15, sc.work);
		progressbar_progress stdcall (#copy_bar);
	}
	WriteTextWithBg(PR_LEFT, PR_TOP-20, 0xD0, sc.work_text, filename, sc.work);

	WriteTextWithBg(PR_LEFT, PR_TOP+PR_H+5, 0xD0, sc.work_text, 
		sprintf(#param, "%i/%i", copy_bar.value, copy_bar.max), sc.work);
}
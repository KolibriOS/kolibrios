
#define WIN_DIALOG_W 380
#define WIN_DIALOG_H 100
#define PR_LEFT 14
#define PR_TOP  32
#define PR_W  WIN_DIALOG_W-PR_LEFT-PR_LEFT
#define PR_H  18

proc_info Dialog_Form;
progress_bar copy_bar = {0,PR_LEFT,PR_TOP,PR_W,PR_H,0,0,1,0xFFFFFF,0x00FF00,0x555555};

enum {
	REDRAW_FLAG,
	COPY_FLAG, 
	MOVE_FLAG, 
	DELETE_FLAG, 
};

void DisplayOperationForm(int operation_flag)
{
	dword title;
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
	copy_bar.frame_color = sc.work_graph;
	switch(CheckEvent())
	{
		 case evButton:
			DialogExit();
			break;
			
		case evReDraw:
			DefineAndDrawWindow(Form.left+Form.width-200, Form.top+90, WIN_DIALOG_W+9,
				skin_height+WIN_DIALOG_H, 0x34, sc.work, title, 0);
			GetProcessInfo(#Dialog_Form, SelfInfo);
			DrawCaptButton(WIN_DIALOG_W-PR_LEFT-101, PR_TOP+PR_H+6, 100,26, 2, 
				sc.button, sc.button_text, T_ABORT_WINDOW_BUTTON);

			DrawRectangle3D(PR_LEFT-1, PR_TOP-1, PR_W+1, PR_H+1, sc.work_dark, sc.work_light);
	}
}

void DialogExit() {
	ActivateWindow(GetProcessSlot(Form.ID));
	ExitProcess();
}

void Operation_Draw_Progress(dword filename) {
	if (Dialog_Form.cwidth==0)
	{
		copy_bar.value++;
		return;
	}
	DisplayOperationForm(REDRAW_FLAG);
	DrawBar(PR_LEFT, PR_TOP-20, WIN_DIALOG_W-PR_LEFT, 15, sc.work);
	WriteText(PR_LEFT, PR_TOP-20, 0x90, sc.work_text, filename);

	progressbar_draw stdcall (#copy_bar);
	progressbar_progress stdcall (#copy_bar);

	WriteTextWithBg(PR_LEFT, PR_TOP+PR_H+5, 0xD0, sc.work_text, 
		sprintf(#param, "%i/%i", copy_bar.value, copy_bar.max), sc.work);
}
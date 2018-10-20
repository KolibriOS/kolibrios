
#define WIN_DIALOG_W 380
#define WIN_DIALOG_H 85
#define PR_LEFT 14
#define PR_TOP  28
#define PR_W  WIN_DIALOG_W-PR_LEFT-PR_LEFT
#define PR_H  14

proc_info Dialog_Form;
progress_bar copy_bar = {0,PR_LEFT,PR_TOP,PR_W,PR_H,0,0,1,0xFFFFFF,0x00FF00,0x555555};
//sensor copying = {PR_LEFT,PR_TOP,WIN_DIALOG_W-PR_LEFT-PR_LEFT,19};

int operation_flag;
enum {
	COPY_FLAG, 
	MOVE_FLAG, 
	DELETE_FLAG, 
	OPERATION_END
};

void DisplayOperationForm()
{
	dword title;
	if (operation_flag==COPY_FLAG) {
		title = T_COPY_WINDOW_TITLE;
		copy_bar.progress_color = 0x00FF00;
	}
	else if (operation_flag==MOVE_FLAG) {
		title = T_MOVE_WINDOW_TITLE;
		copy_bar.progress_color = 0x00FF00;
	}
	else if (operation_flag==DELETE_FLAG) {
		title = T_DELETE_WINDOW_TITLE;
		copy_bar.progress_color = 0xF17A65;
	}
	copy_bar.frame_color = system.color.work_graph;
	switch(CheckEvent())
	{
		 case evButton:
			notify(T_CANCEL_PASTE);
			DialogExit();
			break;
			
		case evReDraw:
			DefineAndDrawWindow(Form.left+Form.width-200,Form.top+90,WIN_DIALOG_W+9,skin_height+WIN_DIALOG_H,0x34,system.color.work,title,0);
			GetProcessInfo(#Dialog_Form, SelfInfo);
			DrawCaptButtonSmallText(WIN_DIALOG_W-PR_LEFT-80, PR_TOP+PR_H+6, 80,22, 2, 
				system.color.work_button, system.color.work_button_text, T_ABORT_WINDOW_BUTTON);

			DrawRectangle3D(PR_LEFT-1, PR_TOP-1, PR_W+1, PR_H+1, system.color.work_dark, system.color.work_light);
			//copying.draw_wrapper();
	}
}

void DialogExit() {
	action_buf = OPERATION_END;
	ActivateWindow(GetProcessSlot(Form.ID));
	ExitProcess();
}

void Operation_Draw_Progress(dword filename) {
	if (Dialog_Form.cwidth==0)
	{
		copy_bar.value++;
		return;
	}
	DisplayOperationForm();
	DrawBar(PR_LEFT, PR_TOP-14, WIN_DIALOG_W-PR_LEFT, 10, system.color.work);
	WriteText(PR_LEFT, PR_TOP-14, 0x80, system.color.work_text, filename);

	progressbar_draw stdcall (#copy_bar);
	progressbar_progress stdcall (#copy_bar);
	//copy_bar.value++;
	//pause(1);
	//copying.draw_progress(copy_bar.value*copying.w/copy_bar.max, copy_bar.value, copy_bar.max-copy_bar.value, "");

	DrawBar(PR_LEFT, PR_TOP+PR_H+6, 100, 15, system.color.work);
	WriteText(PR_LEFT, PR_TOP+PR_H+6, 0x80, system.color.work_text, sprintf(#param, "%i/%i", copy_bar.value, copy_bar.max));
}
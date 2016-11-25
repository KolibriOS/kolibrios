#define WIN_DIALOG_W 420
#define WIN_DIALOG_H 100
proc_info Dialog_Form;

progress_bar copy_bar = {0,16,60,50,23,0,0,1,0xFFFFFF,0x00FF00,0x555555};

int operation_flag;
enum {
	COPY_FLAG, 
	MOVE_FLAG, 
	DELETE_FLAG, 
	OPERATION_END
};

void DisplayOperationForm()
{
	dword title, message;
	switch(CheckEvent())
	{
		 case evButton:
			notify(T_CANCEL_PASTE);
			DialogExit();
			break;
			
		case evReDraw:
			if (operation_flag==COPY_FLAG) {
				title = T_COPY_WINDOW_TITLE;
				message = T_COPY_WINDOW_TEXT;
			}
			else if (operation_flag==MOVE_FLAG) {
				title = T_MOVE_WINDOW_TITLE;
				message = T_MOVE_WINDOW_TEXT;
			}
			else if (operation_flag==DELETE_FLAG) {
				title = T_DELETE_WINDOW_TITLE;
				message = T_DELETE_WINDOW_TEXT;
			}
			DefineAndDrawWindow(Form.left+Form.width-200,Form.top+90,WIN_DIALOG_W,skin_height+WIN_DIALOG_H,0x34,system.color.work,title,0);
			GetProcessInfo(#Dialog_Form, SelfInfo);
			WriteText(45, 11, 0x90, system.color.work_text, message);
			DrawFlatButton(Dialog_Form.cwidth - 105, copy_bar.top-2 , T_CANCEL_PASTE, T_ABORT_WINDOW_BUTTON);
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
	copy_bar.width = Dialog_Form.cwidth-32 - 100;
	DisplayOperationForm();
	DrawIconByExtension(filename, filename+strrchr(filename,'.'), 16, 19, system.color.work);
	DrawBar(45, 32, Dialog_Form.cwidth-45, 15, system.color.work);
	WriteText(45, 32, 0x90, 0x000000, filename);
	progressbar_draw stdcall (#copy_bar);
	progressbar_progress stdcall (#copy_bar);
	//copy_bar.value++;
	//pause(20);
}
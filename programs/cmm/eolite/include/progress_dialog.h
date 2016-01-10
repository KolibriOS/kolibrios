#define WIN_DIALOG_W 345
#define WIN_DIALOG_H 110
proc_info Dialog_Form;

progress_bar copy_bar = {0,16,49,50,20,0,0,1,0xFFFFFF,0x00FF00,0x000000};

enum {COPY_FLAG, MOVE_FLAG, DELETE_FLAG, OPERATION_END};
int operation_flag;

void DisplayOperationForm()
{
	  switch(CheckEvent())
	  {
		 case evButton:
			notify(T_CANCEL_PASTE);
			DialogExit();
			break;
		 
		case evReDraw:
			if (operation_flag==COPY_FLAG) DefineAndDrawWindow(Form.left+Form.width-200,Form.top+90,WIN_DIALOG_W,GetSkinHeight()+WIN_DIALOG_H,0x34,0xFFFFFF,T_COPY_WINDOW_TITLE);
			else if (operation_flag==MOVE_FLAG) DefineAndDrawWindow(Form.left+Form.width-200,Form.top+90,WIN_DIALOG_W,GetSkinHeight()+WIN_DIALOG_H,0x34,0xFFFFFF,T_MOVE_WINDOW_TITLE);
			else DefineAndDrawWindow(Form.left+Form.width-200,Form.top+90,WIN_DIALOG_W,GetSkinHeight()+WIN_DIALOG_H,0x34,0xFFFFFF,T_DELETE_WINDOW_TITLE);
			
			GetProcessInfo(#Dialog_Form, SelfInfo);
			
			if (operation_flag==COPY_FLAG) WriteText(45, 11, 0x80, system.color.work_text, T_COPY_WINDOW_TEXT);
			else if (operation_flag==MOVE_FLAG) WriteText(45, 11, 0x80, system.color.work_text, T_MOVE_WINDOW_TEXT);
			else WriteText(45, 11, 0x80, system.color.work_text, T_DELETE_WINDOW_TEXT);
			
			DrawFlatButton(Dialog_Form.cwidth - 96, Dialog_Form.cheight - 32, 80, 22, 10, T_ABORT_WINDOW_BUTTON);
			DrawBar(8, 10, 32, 32, 0xFFFfff);
			break;
	  }
}

void DialogExit() {
	action_buf = OPERATION_END;
	ActivateWindow(GetProcessSlot(Form.ID));
	ExitProcess();
}


void Operation_Draw_Progress(dword copying_filename) {
	if (Dialog_Form.cwidth==0)
	{
		copy_bar.value++;
		return;
	}
	copy_bar.width = Dialog_Form.cwidth-32;
	DisplayOperationForm();
	Put_icon(copying_filename+strrchr(copying_filename,'.'), 16, 19, 0xFFFfff, 0);
	DrawBar(45, 29, Dialog_Form.cwidth-45, 10, 0xFFFFFF);
	WriteText(45, 29, 0x80, 0x000000, copying_filename);
	progressbar_draw stdcall (#copy_bar);
	progressbar_progress stdcall (#copy_bar);
	//copy_bar.value++;
	//pause(50);
}
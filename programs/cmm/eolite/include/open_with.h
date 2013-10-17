//Leency 2013

void OpenWith()
{
	?define OPEN_LIST_W 300
	?define OPEN_LIST_H 300
	mouse mm;
	word key, slot, index, start_y;
	llist app_list;
	proc_info MenuForm;
	int texty, newi;

	app_list.ClearList();
	app_list.SetSizes(files.w-OPEN_LIST_W/2+files.x+Form.left+4,files.h-OPEN_LIST_H/2+files.y+Form.top+GetSkinHeight(),OPEN_LIST_W,OPEN_LIST_H,150,18);
	SetEventMask(100111b);
	goto _APP_LIST_DRAW;

	loop() switch(WaitEvent())
	{
		case evMouse:
				slot = GetProcessSlot(MenuForm.ID);
				if (slot != GetActiveProcess()) ExitProcess();
				mm.get();
				if (mm.lkm) ExitProcess();
				break;
				
		case evKey:
				key = GetKey();
				if (key==27) ExitProcess();
				break;
				
		case evReDraw: _APP_LIST_DRAW:
				DefineAndDrawWindow(app_list.x, app_list.y,app_list.w+3,app_list.h+6,0x01, 0, 0, 0x01fffFFF);
				GetProcessInfo(#MenuForm, SelfInfo);
				DrawPopup(0,0,app_list.w,app_list.h,0, col_work,col_border);
				WriteText(10,10, 0x80, 0, "Select application to open file"w);
				WriteText(10,23, 0x80, 0, #file_name);
				WriteTextB(app_list.w/2-25,app_list.h/2, 0x90, 0, "S O O N");

				_APP_LIST_ITEMS_DRAW:
				break;
	}
}
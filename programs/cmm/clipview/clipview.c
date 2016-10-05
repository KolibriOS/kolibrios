#ifndef AUTOBUILD
	?include "lang.h--"
#endif

#define MEMSIZE 0xDFE800
#include "..\lib\mem.h"
#include "..\lib\strings.h"
#include "..\lib\list_box.h"
#include "..\lib\clipboard.h"
#include "..\lib\gui.h"
#include "..\lib\obj\box_lib.h"


//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

?define WINDOW_HEADER "Clipboard viewer v1.0"
?define T_DELETE_LAST_SLOT "Delete last slot"
?define T_DELETE_ALL_SLOTS "Delete all slots"
?define T_RESET_BUFFER_LOCK "Reset the lock buffer"
?define T_COLUMNS_TITLE "# | Data size | Data type | Contents"
?define T_COLUMN_VIEW "View"
?define T_VIEW_OPTIONS "TEXT  HEX"
?define T_CLIPBOARD_IS_EMPTY "Clipboard is empty"
?define DEFAULT_SAVE_PATH "/tmp0/1/clipview.tmp"
char *data_type[] = { "Text", "Image", "RAW", "Unknown" };

enum {
	BT_DELETE_LAST_SLOT = 10,
	BT_DELETE_ALL_SLOTS,
	BT_UNLOCK
};


#define PANEL_TOP_H 20
#define PANEL_BOTTOM_H 30
#define LIST_PADDING 12

llist list;

proc_info Form;

Clipboard clipboard;

scroll_bar scroll1 = { 18,200,398, 44,18,0,115,15,0,0xeeeeee,0xD2CED0,0x555555,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1};


//===================================================//
//                                                   //
//                       CODE                        //
//                                                   //
//===================================================//

void main()
{   
	int id;

	list.SetFont(8, 14, 0x90);
	list.no_selection = true;
	SetEventMask(0x27);
	load_dll(boxlib, #box_lib_init,0);
	loop() 
	{
	  WaitEventTimeout(10);
	  switch(EAX & 0xFF)
	  {
	  	case evMouse:
			if (!CheckActiveProcess(Form.ID)) break;
			mouse.get();
			scrollbar_v_mouse (#scroll1);
			if (list.first != scroll1.position)
			{
				list.first = scroll1.position;
				Draw_List();
				break;
			}
	  		if (mouse.vert) && (list.MouseScroll(mouse.vert)) Draw_List();
	  		break;


		case evButton:
			id=GetButtonID();
			if (id==1) ExitProcess();
			if (id==BT_DELETE_LAST_SLOT) EventDeleteLastSlot();
			if (id==BT_DELETE_ALL_SLOTS) EventDeleteAllSlots();
			if (id==BT_UNLOCK) EventResetBufferLock();
			if (id>=100) && (id<300) EventOpenAsText(id-100);
			if (id>=300) EventOpenAsHex(id-300);
			break;
	  
		case evKey:
			GetKeys(); 
			if (list.ProcessKey(key_scancode)) Draw_List();
			break;
		 
		case evReDraw:
			system.color.get();			
			DefineAndDrawWindow(screen.width-700/2,80,700,454+skin_height,0x73,0xE4DFE1,WINDOW_HEADER,0);
			GetProcessInfo(#Form, SelfInfo);
			IF (Form.status_window>=2) break;
			if (Form.height < 200) { MoveSize(OLD,OLD,OLD,200); break; }
			if (Form.width  < 570) { MoveSize(OLD,OLD,570,OLD); break; }
			list.SetSizes(LIST_PADDING, LIST_PADDING+PANEL_TOP_H, Form.cwidth-LIST_PADDING-LIST_PADDING-scroll1.size_x, 
				Form.cheight-PANEL_BOTTOM_H-PANEL_TOP_H-LIST_PADDING-LIST_PADDING, 20);
		 	DrawWindowContent();
		 	Draw_List();
		 	break;

		default:
			if (clipboard.GetSlotCount() > list.count) Draw_List();
			break;
	  }
   }
}

void DrawWindowContent()
{
	int button_x = list.x;
	DrawBar(0,0, Form.cwidth, PANEL_TOP_H, system.color.work);
	DrawBar(0,Form.cheight-PANEL_BOTTOM_H, Form.cwidth, PANEL_BOTTOM_H, system.color.work);
	DrawRectangle3D(list.x-2, list.y-2, list.w+3+scroll1.size_x, list.h+3, system.color.work_dark, system.color.work_light);
	DrawWideRectangle(list.x-LIST_PADDING, list.y-LIST_PADDING, LIST_PADDING*2+list.w+scroll1.size_x, LIST_PADDING*2+list.h, LIST_PADDING-2, system.color.work);
	button_x += DrawStandartCaptButton(button_x, list.y + list.h + 8, BT_DELETE_LAST_SLOT, system.color.work_button, system.color.work_button_text, T_DELETE_LAST_SLOT);
	button_x += DrawStandartCaptButton(button_x, list.y + list.h + 8, BT_DELETE_ALL_SLOTS, system.color.work_button, system.color.work_button_text, T_DELETE_ALL_SLOTS);
	button_x += DrawStandartCaptButton(button_x, list.y + list.h + 8, BT_UNLOCK, system.color.work_button, system.color.work_button_text, T_RESET_BUFFER_LOCK);
	DrawRectangle(list.x-1, list.y-1, list.w+1+scroll1.size_x, list.h+1, system.color.work_graph);
	WriteText(list.x+12, list.y - 23, list.font_type, system.color.work_text, T_COLUMNS_TITLE);
	WriteText(list.x+list.w-68, list.y - 23, list.font_type, system.color.work_text, T_COLUMN_VIEW);
}

void DrawScroller()
{
	scroll1.bckg_col = MixColors(system.color.work, 0xBBBbbb, 80);
	scroll1.frnt_col = MixColors(system.color.work,0xFFFfff,120);
	scroll1.line_col = system.color.work_graph;

	scroll1.max_area = list.count;
	scroll1.cur_area = list.visible;
	scroll1.position = list.first;

	scroll1.all_redraw=1;
	scroll1.start_x = list.x + list.w;
	scroll1.start_y = list.y-1;
	scroll1.size_y = list.h+2;

	scrollbar_v_draw(#scroll1);
}


void Draw_List()
{
	int i, yyy, list_last, slot_data_type_number;
	dword text_color = 0x000000;
	char line_text[512];
	dword size_kb;

	list.count = clipboard.GetSlotCount();
	list.CheckDoesValuesOkey();


	if (list.count > list.visible) list_last = list.visible; else list_last = list.count;

	for (i=0; i<list.visible; i++;) DeleteButton(list.first + i + 100);
	for (i=0; i<list.visible; i++;) DeleteButton(list.first + i + 300);

	for (i=0; i<list_last; i++;)
	{
		clipboard.GetSlotData(list.first + i);
		yyy = i*list.item_h+list.y;		
		DrawBar(list.x,yyy,list.w, list.item_h, 0xFFFfff);
		WriteText(list.x+12, yyy+list.text_y, list.font_type, text_color, itoa(list.first + i));
		//WriteText(list.x+44, yyy+list.text_y, list.font_type, text_color, itoa(clipboard.slot_data.size));
		size_kb = ConvertSizeToKb(clipboard.slot_data.size);
		WriteText(list.x+44, yyy+list.text_y, list.font_type, text_color, size_kb);
		slot_data_type_number = clipboard.slot_data.type;
		WriteText(list.x+140, yyy+list.text_y, list.font_type, text_color, data_type[slot_data_type_number]);
		WriteText(list.x+list.w - 88, yyy+list.text_y, list.font_type, 0x006597, T_VIEW_OPTIONS);
		DefineButton(list.x+list.w - 95, yyy, 50, list.item_h, 100+i+BT_HIDE, NULL);
		DefineButton(list.x+list.w - 95 + 51, yyy, 40, list.item_h, 300+i+BT_HIDE, NULL);

		strlcpy(#line_text, clipboard.slot_data.content, list.w-236 - 95/list.font_w-3);
		WriteText(list.x+236, yyy+list.text_y, list.font_type, text_color, #line_text);
	}
	DrawBar(list.x,i*list.item_h+list.y, list.w, -i*list.item_h+ list.h, 0xFFFfff);
	if (!list.count) WriteText(-strlen(T_CLIPBOARD_IS_EMPTY)*list.font_w + list.w / 2 + list.x + 1, list.h / 2 - 8 + list.y, list.font_type, 0x999999, T_CLIPBOARD_IS_EMPTY);
	DrawScroller();
}

int SaveSlotContents(int slot_id) {
	clipboard.GetSlotData(slot_id);
	EAX = WriteFile(clipboard.slot_data.size, clipboard.slot_data.content, DEFAULT_SAVE_PATH);
	if (!EAX)
	{
		return true;
	}
	else {
		notify("'Can not create /tmp0/1/clipview.tmp\nPreview function is not available.' -E");
	 	return false;
	}
}

//===================================================//
//                                                   //
//                     EVENTS                        //
//                                                   //
//===================================================//

void EventDeleteLastSlot()
{
	clipboard.DelLastSlot();
	Draw_List();
}

void EventDeleteAllSlots()
{
	while (clipboard.GetSlotCount()) clipboard.DelLastSlot();
	Draw_List();
}

void EventResetBufferLock() {
	clipboard.ResetBlockingBuffer();
	Draw_List();
}

void EventOpenAsText(int slot_id) {
	if (SaveSlotContents(slot_id)) RunProgram("/sys/tinypad", DEFAULT_SAVE_PATH);
}

void EventOpenAsHex(int slot_id) {
	if (SaveSlotContents(slot_id)) RunProgram("/sys/develop/heed", DEFAULT_SAVE_PATH);
}

stop:

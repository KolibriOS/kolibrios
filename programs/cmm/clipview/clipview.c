#ifndef AUTOBUILD
	?include "lang.h--"
#endif

#define MEMSIZE 4096*20
#include "..\lib\mem.h"
#include "..\lib\strings.h"
#include "..\lib\list_box.h"
#include "..\lib\clipboard.h"
#include "..\lib\gui.h"
#include "..\lib\obj\box_lib.h"

#include "..\lib\patterns\select_list.h"


//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

?define WINDOW_HEADER "Clipboard Viewer v1.03"
?define T_DELETE_LAST_SLOT "Delete last slot"
?define T_DELETE_ALL_SLOTS "Delete all slots"
?define T_RESET_BUFFER_LOCK "Reset the lock buffer"
?define T_COLUMNS_TITLE "# | Data size | Data type | Contents"
?define T_COLUMN_VIEW "| View"
?define T_VIEW_OPTIONS "TEXT  HEX"
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

proc_info Form;

//===================================================//
//                                                   //
//                       CODE                        //
//                                                   //
//===================================================//

void main()
{   
	int id;
	SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE + EVM_MOUSE_FILTER);
	load_dll(boxlib, #box_lib_init,0);
	loop() 
	{
	  WaitEventTimeout(10);
	  switch(EAX & 0xFF)
	  {
	  	case evMouse:
			SelectList_ProcessMouse();
	  		break;

		case evButton:
			id = GetButtonID();
			if (id==1) ExitProcess();
			if (id==BT_DELETE_LAST_SLOT) EventDeleteLastSlot();
			if (id==BT_DELETE_ALL_SLOTS) EventDeleteAllSlots();
			if (id==BT_UNLOCK) EventResetBufferLock();
			if (id>=100) && (id<300) EventOpenAsText(id-100);
			if (id>=300) EventOpenAsHex(id-300);
			break;
	  
		case evKey:
			GetKeys(); 
			if (select_list.ProcessKey(key_scancode)) ClipViewSelectListDraw();
			break;
		 
		case evReDraw:
			system.color.get();			
			DefineAndDrawWindow(screen.width-700/2,80,700,454+skin_height,0x73,0xE4DFE1,WINDOW_HEADER,0);
			GetProcessInfo(#Form, SelfInfo);
			IF (Form.status_window>=2) break;
			if (Form.height < 200) { MoveSize(OLD,OLD,OLD,200); break; }
			if (Form.width  < 570) { MoveSize(OLD,OLD,570,OLD); break; }
			SelectList_Init(
				LIST_PADDING, 
				LIST_PADDING+PANEL_TOP_H, 
				Form.cwidth-LIST_PADDING-LIST_PADDING-scroll1.size_x, 
				Form.cheight-PANEL_BOTTOM_H-PANEL_TOP_H-LIST_PADDING-LIST_PADDING,
				true
				);
		 	DrawWindowContent();
		 	break;

		default:
			if (Clipboard__GetSlotCount() > select_list.count) ClipViewSelectListDraw();
			break;
	  }
   }
}

void DrawWindowContent()
{
	int button_x = select_list.x;
	DrawBar(0,0, Form.cwidth, PANEL_TOP_H, system.color.work);
	DrawBar(0,Form.cheight-PANEL_BOTTOM_H, Form.cwidth, PANEL_BOTTOM_H, system.color.work);
	DrawWideRectangle(select_list.x-LIST_PADDING, select_list.y-LIST_PADDING, LIST_PADDING*2+select_list.w+scroll1.size_x, LIST_PADDING*2+select_list.h, LIST_PADDING-2, system.color.work);
	button_x += DrawStandartCaptButton(button_x, select_list.y + select_list.h + 8, BT_DELETE_LAST_SLOT, T_DELETE_LAST_SLOT);
	button_x += DrawStandartCaptButton(button_x, select_list.y + select_list.h + 8, BT_DELETE_ALL_SLOTS, T_DELETE_ALL_SLOTS);
	button_x += DrawStandartCaptButton(button_x, select_list.y + select_list.h + 8, BT_UNLOCK, T_RESET_BUFFER_LOCK);
	WriteText(select_list.x+12, select_list.y - 23, select_list.font_type, system.color.work_text, T_COLUMNS_TITLE);
	WriteText(select_list.x+select_list.w - 88-14, select_list.y - 23, select_list.font_type, system.color.work_text, T_COLUMN_VIEW);
 	ClipViewSelectListDraw();
 	SelectList_DrawBorder();
}

dword slot_data;
struct clipboard_data
{
	dword	size;
	dword	type;
	dword	encoding;
	dword	content;
	dword	content_offset;
} cdata;

void SelectList_DrawLine(dword i)
{
	int yyy, length, slot_data_type_number;
	dword line_text[2048];
	dword size_kb;
	dword text_color = 0;
	dword bgcol = 0xFFFfff;
	if (i%2) bgcol = 0xF1F1F1;

	slot_data = Clipboard__GetSlotData(select_list.first + i);
	cdata.size = ESDWORD[slot_data];
	cdata.type = ESDWORD[slot_data+4];
	if (cdata.type==SLOT_DATA_TYPE_TEXT) || (cdata.type==SLOT_DATA_TYPE_TEXT_BLOCK)
		cdata.content_offset = 12;
	else 
		cdata.content_offset = 8;
	cdata.content = slot_data + cdata.content_offset; 

	yyy = i*select_list.item_h+select_list.y;
	DrawBar(select_list.x+1, yyy, select_list.w-1, select_list.item_h, bgcol);
	WriteText(select_list.x+12, yyy+select_list.text_y, select_list.font_type, text_color, itoa(select_list.first + i));
	size_kb = ConvertSizeToKb(cdata.size);
	WriteText(select_list.x+44, yyy+select_list.text_y, select_list.font_type, text_color, size_kb);
	slot_data_type_number = cdata.type;
	WriteText(select_list.x+140, yyy+select_list.text_y, select_list.font_type, text_color, data_type[slot_data_type_number]);
	WriteText(select_list.x+select_list.w - 88, yyy+select_list.text_y, select_list.font_type, 0x006597, T_VIEW_OPTIONS);
	DefineButton(select_list.x+select_list.w - 95, yyy, 50, select_list.item_h, 100+i+BT_HIDE, NULL);
	DefineButton(select_list.x+select_list.w - 95 + 51, yyy, 40, select_list.item_h, 300+i+BT_HIDE, NULL);

	length = select_list.w-236 - 95 / select_list.font_w - 2;
	if (cdata.size - cdata.content_offset < length) length = cdata.size - cdata.content_offset;
	memmov(#line_text, cdata.content, length);
	replace_char(#line_text, 0, 31, length); // 31 is a dot
	WriteText(select_list.x+236, yyy+select_list.text_y, select_list.font_type, text_color, #line_text);
}

int SaveSlotContents(dword size, off) {
	EAX = CreateFile(size, off, DEFAULT_SAVE_PATH);
	if (!EAX)
	{
		return true;
	}
	else {
		notify("'Can not create /tmp0/1/clipview.tmp\nPreview function is not available.' -E");
	 	return false;
	}
}

void ClipViewSelectListDraw() {
	select_list.count = Clipboard__GetSlotCount();
	SelectList_Draw();
}

void SelectList_LineChanged() {
	return;
}

//===================================================//
//                                                   //
//                     EVENTS                        //
//                                                   //
//===================================================//

void EventDeleteLastSlot()
{
	int i;
	for (i=0; i<select_list.visible; i++;) DeleteButton(select_list.first + i + 100);
	for (i=0; i<select_list.visible; i++;) DeleteButton(select_list.first + i + 300);
	Clipboard__DeleteLastSlot();
	ClipViewSelectListDraw();
}

void EventDeleteAllSlots()
{
	while (Clipboard__GetSlotCount()) Clipboard__DeleteLastSlot();
	ClipViewSelectListDraw();
}

void EventResetBufferLock() {
	Clipboard__ResetBlockingBuffer();
	ClipViewSelectListDraw();
}

void EventOpenAsText(int slot_id) {
	slot_data = Clipboard__GetSlotData(slot_id);
	cdata.size = ESDWORD[slot_data]-12;
	cdata.content = slot_data+12;
	if (SaveSlotContents(cdata.size, cdata.content)) RunProgram("/sys/tinypad", DEFAULT_SAVE_PATH);
}

void EventOpenAsHex(int slot_id) {
	slot_data = Clipboard__GetSlotData(slot_id);
	cdata.size = ESDWORD[slot_data];
	cdata.content = slot_data;
	if (SaveSlotContents(cdata.size, cdata.content)) RunProgram("/sys/develop/heed", DEFAULT_SAVE_PATH);
}

stop:

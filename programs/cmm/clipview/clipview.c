#define ENTRY_POINT #main

#define MEMSIZE 4096*20

#include "..\lib\strings.h"
#include "..\lib\clipboard.h"
#include "..\lib\gui.h"
#include "..\lib\fs.h"

//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

?define T_COLUMNS_TITLE "#    Size      Type        Contents"
?define DEFAULT_SAVE_PATH "/tmp0/1/clipview.tmp"
char *data_type[] = { "Text", "Image", "RAW", "Unknown" };

enum {
	BT_DELETE_LAST = 10,
	BT_DELETE_ALL,
	BT_UNLOCK,
	BT_LIST_LEFT,
	BT_LIST_RIGHT
};

#define PANEL_BOTTOM_H 35
#define GAP 5
#define LIST_Y 32
#define PANEL_TOP_H LIST_Y-2
#define LINE_H 20
#define TEXT_Y LINE_H - 14 / 2

proc_info Form;

struct LIST {
	int w,h;
	int count;
	int first;
	int visible;
} list = 0;

//===================================================//
//                                                   //
//                       CODE                        //
//                                                   //
//===================================================//

void main()
{   
	@mem_init();
	@SetEventMask(EVM_REDRAW + EVM_BUTTON);
	loop() switch(@WaitEventTimeout(10))
	{
		case evButton:
			@GetButtonID();
			switch(EAX) {
				case 1: 
					ExitProcess(); 
				case BT_DELETE_LAST:
					EventDeleteLastSlot();
					break;
				case BT_DELETE_ALL:
					EventDeleteAllSlots();
					break;
				case BT_UNLOCK:
					EventResetBufferLock();
					break;
				case BT_LIST_LEFT:
					list.first -= list.visible;
					if (list.first <= 0) list.first = 0;
					ClipViewSelectListDraw();
					break;
				case BT_LIST_RIGHT:
					if (list.first + list.visible < list.count) list.first += list.visible;
					ClipViewSelectListDraw();
					break;
				default:
					if (EAX>=300) EventOpenAsHex(EAX-300);
					else EventOpenAsText(EAX-100);
			}
			break;
		 
		case evReDraw:
			sc.get();			
			DefineAndDrawWindow(GetScreenWidth()-600/2,80,600,400,0x73,NULL,"Clipboard Viewer",NULL);
			GetProcessInfo(#Form, SelfInfo);
			IF (Form.status_window&ROLLED_UP) break;
			IF (Form.height < 200) { MoveSize(OLD,OLD,OLD,200); break; }
			IF (Form.width  < 570) { MoveSize(OLD,OLD,570,OLD); break; }
		 	DrawWindowContent();
		 	break;

		default:
			if (Clipboard__GetSlotCount() > list.count) ClipViewSelectListDraw();
   }
}

void DrawWindowContent()
{
	list.w = Form.cwidth-GAP-GAP; 
	list.h = Form.cheight-PANEL_BOTTOM_H-LIST_Y-GAP;
	list.visible = list.h / LINE_H;

	DrawBar(0,0, Form.cwidth, PANEL_TOP_H, sc.work);
	DrawBar(0,Form.cheight-PANEL_BOTTOM_H, Form.cwidth, PANEL_BOTTOM_H, sc.work);
	DrawWideRectangle(GAP-GAP, LIST_Y-GAP, GAP*2+list.w, GAP*2+list.h, GAP-2, sc.work);

	DefineButton(GAP, list.h + LIST_Y + 8, 110, 25, BT_DELETE_LAST, sc.button);
	$inc edx
	$add ebx, 130 << 16    //BT_DELETE_ALL
	$int 64
	$inc edx
	$add ebx, 130 << 16    //BT_UNLOCK
	$int 64

	WriteText(GAP+11, LIST_Y + list.h + 14, 0x90, sc.button_text, "Delete last      Delete all      Reset lock");

	WriteText(GAP+18, LIST_Y - 23, 0x90, sc.work_text, T_COLUMNS_TITLE);
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
	int yyy, slot_data_type_number;

	yyy = i*LINE_H+LIST_Y;
	DrawBar(GAP, yyy, list.w, LINE_H, -i%2 * 0x0E0E0E + 0xF1F1f1);

	if (list.first + i >= list.count) {
		return;
	}

	slot_data = Clipboard__GetSlotData(list.first + i);
	cdata.size = ESDWORD[slot_data];
	cdata.type = ESDWORD[slot_data+4];
	if (cdata.type==SLOT_DATA_TYPE_TEXT) || (cdata.type==SLOT_DATA_TYPE_TEXT_BLOCK)
		cdata.content_offset = 12;
	else 
		cdata.content_offset = 8;
	cdata.content = slot_data + cdata.content_offset; 

	WriteText(list.first+i/10^1*8+GAP+12, yyy+TEXT_Y, 0x90, 0x000000, itoa(list.first + i));
	EDX = ConvertSizeToKb(cdata.size);
	WriteText(GAP+44+16, yyy+TEXT_Y, 0x90, 0x000000, EDX);
	slot_data_type_number = cdata.type;
	if (slot_data_type_number >= 4) slot_data_type_number = 3;
	WriteText(GAP+140, yyy+TEXT_Y, 0x90, 0x000000, data_type[slot_data_type_number]);
	WriteTextB(GAP+list.w - 88, yyy+TEXT_Y, 0x90, 0x006597, "TEXT  HEX");
	DefineButton(GAP+list.w - 98, yyy, 50, LINE_H, 100+i+BT_HIDE, NULL);
	$add edx, 200
	$add ebx, 52 << 16 - 10 //BT_HEX
	$int 64

	ESI = list.w - 345 / 8;
	if (cdata.size - cdata.content_offset < ESI) ESI = cdata.size - cdata.content_offset;
	WriteText(GAP+236, yyy+TEXT_Y, 0x30, 0x000000, cdata.content);
}

int SaveSlotContents() {
	if (! CreateFile(cdata.size, cdata.content, DEFAULT_SAVE_PATH))	{
		return true;
	} else {
		notify("'Can not create /tmp0/1/clipview.tmp' -E");
	 	return false;
	}
}

void ClipViewSelectListDraw()
{
	int i, list_last;

	list.count = Clipboard__GetSlotCount();
	if (list.first >= list.count) list.first = list.count - list.visible;
	if (list.first < 0) list.first = 0;

	if (list.count > list.visible) list_last = list.visible; else list_last = list.count;

	for (i=0; i<list_last; i++;) SelectList_DrawLine(i); 

	DrawBar(GAP,i*LINE_H+LIST_Y, list.w, -i*LINE_H+ list.h, 0xFFFfff);
	if (!list.count) WriteText(list.w / 2 + GAP - 60, 
		list.h / 2 - 8 + LIST_Y, 0x90, 0x999999, "No data to show");

	//Show "<" and ">" buttons and a page number
	//in case when there are items more than visible at once
	if (list.count > list.visible) {
		param[0] = list.first / list.visible + '0';
		param[1] = '\0';
		DefineButton(Form.cwidth-84-GAP, list.h + LIST_Y + 8, 25, 25, BT_LIST_LEFT, sc.button); //BT_LEFT
		$inc edx
		$add ebx, 57 << 16 //BT_RIGHT
		$int 64
		WriteText(Form.cwidth-84-GAP+10, list.h + LIST_Y + 14, 0x90, sc.button_text, "<      >");
		$add ebx, 28 << 16
		$mov edx, #param;
		$mov edi, sc.work
		$mov ecx, 11010000b << 24
		$add ecx, sc.work_text //page number
		$int 64
	}
}

void SelectList_DrawBorder() {
	DrawRectangle3D(GAP-2, LIST_Y-2,
		list.w+3, list.h+3, 
		sc.dark, sc.light);
	DrawRectangle3D(GAP-1, LIST_Y-1, 
		list.w+1, list.h+1, 
		sc.line, sc.line);
}

//===================================================//
//                                                   //
//                     EVENTS                        //
//                                                   //
//===================================================//

void EventDeleteLastSlot()
{
	int i;
	for (i=0; i<list.visible; i++;) {
		DeleteButton(list.first + i + 100);
		$add edx, 200
		$int 64
	}
	Clipboard__DeleteLastSlot();
	DrawWindowContent();
}

void EventDeleteAllSlots()
{
	while (Clipboard__GetSlotCount()) Clipboard__DeleteLastSlot();
	DrawWindowContent();
}

void EventResetBufferLock() {
	Clipboard__ResetBlockingBuffer();
	DrawWindowContent();
}

void EventOpenAsText(int slot_id) {
	slot_data = Clipboard__GetSlotData(slot_id);
	cdata.size = ESDWORD[slot_data]-12;
	cdata.content = slot_data+12;
	if (SaveSlotContents()) RunProgram("/sys/develop/cedit", DEFAULT_SAVE_PATH);
}

void EventOpenAsHex(int slot_id) {
	slot_data = Clipboard__GetSlotData(slot_id);
	cdata.size = ESDWORD[slot_data];
	cdata.content = slot_data;
	if (SaveSlotContents()) RunProgram("/sys/develop/heed", DEFAULT_SAVE_PATH);
}

stop:

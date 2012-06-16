//Leency - 2012

#define ITEM_HEIGHT 18
#define ITEM_WIDTH  138

char *ITEMS_LIST[]={
"View in Tinypad   F3",
"View in TextEdit  F4",
"--------------------", //сделать определение таких линий и рисовать их
"KOI-8         Ctrl+K",
"UTF           Ctrl+U",
0}; 

proc_info MenuForm;

	
void menu_rmb()
{

	mouse mm;
	int items_num, items_cur;
	byte id, key;
	
	SetEventMask(100111b); 
	
	loop() switch(CheckEvent())
	{
		case evMouse:
				mm.get();
				id=mm.y/ITEM_HEIGHT;
				if (id<0) || (id+1>items_num) break;
				if (items_cur<>id)
				{
					items_cur=id;
					goto _ITEMS_DRAW;
				}
				
				break;
				
		case evButton: 
				id=GetButtonID();
				if (id==1) ExitProcess();
				_BUTTON_MARK:
				if (id==10) WB1.Scan(52); //View html code
				if (id==11) WB1.Scan(53); //View html code
				if (id==12) break;
				if (id==13) WB1.Scan(11); //KOI
				if (id==14) WB1.Scan(21); //UTF
				
				ExitProcess();
				break;
				
		case evKey:
				key = GetKey();
				if (key==27) ExitProcess();
				if (key==178) && (items_cur)
				{
					items_cur--;
					goto _ITEMS_DRAW;
				}
				if (key==177) && (items_cur+1<items_num)
				{
					items_cur++;
					goto _ITEMS_DRAW;
				}
				if (key==13)
				{
					id=items_cur+10;
					//WriteDebug(IntToStr(id));
					goto _BUTTON_MARK;
				}
				break;
				
		case evReDraw:
				for (items_num=0; ITEMS_LIST[items_num]<>0; items_num++;) {};
				DefineAndDrawWindow(Form.left+m.x,Form.top+m.y+GetSkinWidth()+3,ITEM_WIDTH,items_num*ITEM_HEIGHT+1,0x01,0x10FFFFFF,0,0,0);

				DrawRegion(0,0,ITEM_WIDTH,items_num*ITEM_HEIGHT+1,0x777777); //ободок
				_ITEMS_DRAW:
				for (i=0; i<items_num; i++;)
				{
					DefineButton(0, i*ITEM_HEIGHT, ITEM_WIDTH, ITEM_HEIGHT, i+10+BT_HIDE, 0xFFFFFF);
					if (i<>items_cur) EDX=0xFFFFFF; else EDX=0x94AECE;
					DrawBar(1, i*ITEM_HEIGHT+1, ITEM_WIDTH-1, ITEM_HEIGHT, EDX);
					WriteText(8,i*ITEM_HEIGHT+6,0x80,0x000000,ITEMS_LIST[i],0);
				}
		default:
				GetProcessInfo(#MenuForm, SelfInfo);
				id=GetSlot(MenuForm.ID);
				if (id<>ActiveProcess()) ExitProcess();			
	}
}
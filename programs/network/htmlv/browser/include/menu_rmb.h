//Leency - 2012

#define ITEM_HEIGHT 18
#define ITEM_WIDTH  138

char *ITEMS_LIST[]={
"View in Tinypad   F3",52,
"View in TextEdit  F4",53,
"-",0,
"KOI-8         Ctrl+K",11,
"UTF           Ctrl+U",21,
0}; 


proc_info MenuForm;

	
void menu_rmb()
{

	mouse mm;
	int items_num, items_cur;
	int id1, key, i;
	
	SetEventMask(100111b); 
	
	loop() switch(WaitEvent())
	{
	case evMouse:
				mm.get();

				GetProcessInfo(#MenuForm, SelfInfo);
				id1=GetProcessSlot(MenuForm.ID);
				if (id1<>GetActiveProcess()) ExitProcess();			
				
				id1=mm.y/ITEM_HEIGHT;
				if (id1<0) || (id1+1>items_num) || (mm.x<0) || (mm.x>ITEM_WIDTH) break;
				if (items_cur<>id1)
				{
					items_cur=id1;
					goto _ITEMS_DRAW;
				}
				
				break;
				
		case evButton: 
				id1=GetButtonID()-10;
				WB1.Scan(ITEMS_LIST[id1*2+1]);
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
					WB1.Scan(ITEMS_LIST[items_cur*2+1]);
					ExitProcess();
				}
				break;
				
		case evReDraw:
				while (ITEMS_LIST[items_num*2]) items_num++;
				DefineAndDrawWindow(Form.left+m.x,Form.top+m.y+GetSkinWidth()+3,ITEM_WIDTH,items_num*ITEM_HEIGHT+1,0x01,0x10FFFFFF,0,0,0);

				DrawRegion(0,0,ITEM_WIDTH,items_num*ITEM_HEIGHT+1,0x777777); //ободок
				_ITEMS_DRAW:
				for (i=0; i<items_num; i++;)
				{
					if (!strcmp(ITEMS_LIST[i*2],"-"))
					{
						DrawBar(1, i*ITEM_HEIGHT+1, ITEM_WIDTH-1, ITEM_HEIGHT, 0xFFFFFF);
						DrawBar(1, i*ITEM_HEIGHT+1+9, ITEM_WIDTH-1, 1, 0x999999);
						continue;
					}
					DefineButton(0, i*ITEM_HEIGHT, ITEM_WIDTH, ITEM_HEIGHT, i+10+BT_HIDE, 0xFFFFFF);
					if (i<>items_cur) EDX=0xFFFFFF; else EDX=0x94AECE;
					DrawBar(1, i*ITEM_HEIGHT+1, ITEM_WIDTH-1, ITEM_HEIGHT, EDX);
					WriteText(8,i*ITEM_HEIGHT+6,0x80,0x000000,ITEMS_LIST[i*2],0);
				}
	}
}






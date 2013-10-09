//Leency - 2012-2013

#define ITEM_HEIGHT 19
#define ITEM_WIDTH  165
dword col_work    = 0xE4DFE1;
dword col_border  = 0x9098B0;

char *ITEMS_LIST[]={
"WIN                  F5",54,
"DOS              Ctrl+D",04,
"KOI              Ctrl+K",11,
"UTF              Ctrl+U",21,
#ifdef LANG_RUS
"€бе®¤­ЁЄ бва ­Ёжл    F3",52,
"ЋзЁбвЁвм Єни Є авЁ­®Є"  ,02,
//"TrueType fonts"       ,05,
#else
"View source          F3",52,
"Free image cache"       ,02,
#endif
//"TrueType fonts"      ,05,
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
				if (mm.lkm) || (mm.pkm)
				{
					ActivateWindow(GetProcessSlot(Form.ID));
					SendWindowMessage(evButton, ITEMS_LIST[items_cur*2+1]);
					SwitchToAnotherThread();
					ExitProcess();
				}
				if (items_cur<>id1)
				{
					items_cur=id1;
					goto _ITEMS_DRAW;
				}
				break;
				
		case evButton: 
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
				DefineAndDrawWindow(Form.left+m.x,Form.top+m.y+GetSkinHeight()+3,ITEM_WIDTH+2,items_num*ITEM_HEIGHT+4,0x01, 0, 0, 0x01fffFFF);
				DrawPopup(0,0,ITEM_WIDTH,items_num*ITEM_HEIGHT+2,0, -1,col_border);

				_ITEMS_DRAW:
				for (i=0; i<items_num; i++;)
				{
					if (i<>items_cur) EDX=col_work; else EDX=0x94AECE;
					DrawBar(2, i*ITEM_HEIGHT+2, ITEM_WIDTH-2, ITEM_HEIGHT, EDX);
					if (i<>items_cur) WriteText(19,i*ITEM_HEIGHT+9,0x80,0xf2f2f2,ITEMS_LIST[i*2]);
					WriteText(18,i*ITEM_HEIGHT+8,0x80,0x000000,ITEMS_LIST[i*2]);
					//if (ITEMS_LIST[i*2+1]==5) && (use_truetype==1) DrawBar(ITEM_WIDTH-18, i*ITEM_HEIGHT+9, 4, 4, 0x444444);
				}
				DrawBar(7, cur_encoding*ITEM_HEIGHT+9, 4, 4, 0x444444); //показывает выбраную кодировку
	}
}






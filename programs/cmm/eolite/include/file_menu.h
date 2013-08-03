//Leency 2008-2013

#ifdef LANG_RUS
char *captions[] = {
	"Открыть",               "Enter",
	"Открыть с помощью...",  "CrlEnt",
	"Открыть как текст",     "F3",
	"Открыть в HEX",         "F4",
	//
	"Копировать",            "Crl+C",
	"Вырезать",              "Crl+X",
	"Вставить",              "Crl+V",
	"Переименовать",         "F2",
	"Удалить",               "Del",
	"Обновить",              "F5",
	0, 0};

#else
char *captions[] = {
	"Open",          "Enter",
	"Open with...",  "CrlEnt",
	"View as text",  "F3",
	"View as HEX",   "F4",
	//
	"Copy",          "Crl+C",
	"Cut",           "Crl+X",
	"Paste",         "Crl+V",
	"Rename",        "F2",
	"Delete",        "Del",
	"Refresh",       "F5",
	0, 0};
#endif

proc_info MenuForm;

void FileMenu()
{
	mouse mm;
	word id, key, slot, index;
	int ccount=0, cur, newi, linew=10, lineh=18, texty;
	for (index=0; captions[index]!=0; index+=2)
	{
		ccount++;
		if (strlen(captions[index])>linew) linew = strlen(captions[index]);
	}
	linew = linew + 3 * 6 + 50;
	texty = lineh/2-4;
	SetEventMask(100111b);

	goto _MENU_DRAW;
	loop() switch(WaitEvent())
	{
		case evMouse:
				slot = GetProcessSlot(MenuForm.ID);
				if (slot != GetActiveProcess()) ExitProcess();
				mm.get();
				newi = mm.y - 1 / lineh;
				if (mm.y<=0) || (mm.y>ccount*lineh+5) || (mm.x<0) || (mm.x>linew) newi=-1;
				if (cur<>newi)
				{
					cur=newi;
					goto _ITEMS_DRAW;
				}
				break;

		case evButton: 
				id=GetButtonID();
				if (id==100) Open();
				if (id==101) notify("Not compleated yet");
				if (id==102) FnProcess(3); //F3
				if (id==103) FnProcess(4); //F4
				if (id==104) Copy(#file_path, NOCUT);
				if (id==105) Copy(#file_path, CUT);
				if (id==106) CreateThread(#Paste,#copy_stak);
				if (id==107) FnProcess(2);
				if (id==108) Del_Form();
				if (id==109) FnProcess(5);
				ExitProcess();
				break;
				
		case evKey:
				IF (GetKey()==27) ExitProcess();
				break;
				
		case evReDraw: _MENU_DRAW:
				DefineAndDrawWindow(m.x+Form.left+5,m.y+Form.top+GetSkinHeight(),linew+3,ccount*lineh+6,0x01, 0, 0, 0x01fffFFF);
				GetProcessInfo(#MenuForm, SelfInfo);
				DrawRectangle(0,0,linew+1,ccount*lineh+2,col_border);
				DrawBar(1,1,linew,1,0xFFFfff);
				PutShadow(linew+2,1,1,ccount*lineh+2,0,2);
				PutShadow(linew+3,2,1,ccount*lineh+2,0,1);
				PutShadow(1,ccount*lineh+3,linew+2,1,0,2);
				PutShadow(2,ccount*lineh+4,linew+1,1,0,1);

				_ITEMS_DRAW:
				for (index=0; captions[index*2]!=0; index++)
				{
					DefineButton(1,index*lineh+1,linew,lineh-1,index+100+BT_HIDE+BT_NOFRAME,0xFFFFFF);
					DrawBar(1,index*lineh+2,1,lineh,0xFFFfff);
					if (index==cur)
					{
						DrawBar(2,index*lineh+2,linew-1,lineh,0xFFFfff);
					}
					else
					{
						DrawBar(2,index*lineh+2,linew-1,lineh,col_work);
						WriteText(8,index*lineh+texty+3,0x80,0xf2f2f2,captions[index*2]);
					}
					WriteText(7,index*lineh+texty+2,0x80,0x000000,captions[index*2]);
					WriteText(-strlen(captions[index*2+1])*6-6+linew,index*lineh+texty+2,0x80,0x888888,captions[index*2+1]);
				}
	}
}
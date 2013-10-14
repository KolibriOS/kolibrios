//Leency 2008-2013

//pay attension: >200 this is only file actions, not supported by folders
#ifdef LANG_RUS
char *file_captions[] = {
	"Открыть",               "Enter",100,
	"Открыть с помощью...",  "CrlEnt",201,
	"Открыть как текст",     "F3",202,
	"Открыть как HEX",       "F4",203,
	"Копировать",            "Crl+C",104,
	"Вырезать",              "Crl+X",105,
	"Вставить",              "Crl+V",106,
	"Переименовать",         "F2",207,
	"Удалить",               "Del",108,
	"Обновить папку",        "F5",109,
	0, 0, 0};
#elif LANG_EST
char *file_captions[] = {
	"Ava",            "Enter",100,
	"Ava ...",        "CrlEnt",201,
	"Vaata tekstina", "F3",202,
	"Vaata HEX",      "F4",203,
	"Kopeeri",        "Crl+C",104,
	"Lїika",          "Crl+X",105,
	"Aseta",          "Crl+V",106,
	"Nimeta №mber",   "F2",207,
	"Kustuta",        "Del",108,
	"Vфrskenda",      "F5",109,
	0, 0, 0};
#else
char *file_captions[] = {
	"Open",          "Enter",100,
	"Open with...",  "CrlEnt",201,
	"View as text",  "F3",202,
	"View as HEX",   "F4",203,
	"Copy",          "Crl+C",104,
	"Cut",           "Crl+X",105,
	"Paste",         "Crl+V",106,
	"Rename",        "F2",207,
	"Delete",        "Del",108,
	"Refresh",       "F5",109,
	0, 0, 0};
#endif


void FileMenu()
{
	mouse mm;
	word id, key, slot, index, start_y;
	proc_info MenuForm;
	int ccount=0, cur=0, newi, linew=10, lineh=18, texty;
	for (index=0; file_captions[index]!=0; index+=3)
	{
		if (itdir) && (file_captions[index+2]>=200) continue;
		if (strlen(file_captions[index])>linew) linew = strlen(file_captions[index]);
		ccount++;
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
				action_buf = GetButtonID();
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
				DrawPopupShadow(1,1,linew,ccount*lineh,0);

				_ITEMS_DRAW:
				for (index=0, start_y=0; file_captions[index*3]!=0; index++)
				{
					DefineButton(1,start_y+1,linew,lineh-1,file_captions[index*3+2]+BT_HIDE+BT_NOFRAME,0xFFFFFF);
					if ((itdir) && (file_captions[index*3+2]>=200)) continue;
					DrawBar(1,start_y+2,1,lineh,0xFFFfff);
					if (start_y/lineh==cur)
					{
						DrawBar(2,start_y+2,linew-1,lineh,0xFFFfff);
					}
					else
					{
						DrawBar(2,start_y+2,linew-1,lineh,col_work);
						WriteText(8,start_y+texty+3,0x80,0xf2f2f2,file_captions[index*3]);
					}
					WriteText(7,start_y+texty+2,0x80,0x000000,file_captions[index*3]);
					WriteText(-strlen(file_captions[index*3+1])*6-6+linew,start_y+texty+2,0x80,0x888888,file_captions[index*3+1]);
					start_y+=lineh;
				}
	}
}
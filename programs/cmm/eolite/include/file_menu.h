//Leency 2008-2013

char *captions[] = {
	"Open",          "Enter",
	"Open with...",  "CrlEnt",
	"View as text",  "F3",
	"View as HEX",   "F4",
	"Rename",        "F2",
	"Delete",        "Del",
	//"Refresh",       "F5",
	0};

void FileMenu()
{
	proc_info MenuForm;
	mouse mm;
	word id, key, slot;
	int ccount=0, cur, newi, linew=10, lineh=18, texty;
	for (i=0; captions[i]!=0; i+=2)
	{
		ccount++;
		if (strlen(captions[i])>linew) linew = strlen(captions[i]);
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
				if (id==102) FnProcess(3);
				if (id==103) FnProcess(4);
				if (id==104) FnProcess(2);
				if (id==105) Del_Form();
				if (id==106) FnProcess(5);
				ExitProcess();
				break;
				
		case evKey:
				IF (GetKey()==27) ExitProcess();
				break;
				
		case evReDraw: _MENU_DRAW:
				DefineAndDrawWindow(m.x+Form.left+5,m.y+Form.top+GetSkinHeight(),linew+3,ccount*lineh+6,0x01, 0, 0, 0x01fffFFF);
				GetProcessInfo(#MenuForm, SelfInfo);
				/* _PutImage(1,23, 16,44, #factions); //иконки	*/
				DrawRectangle(0,0,linew+1,ccount*lineh+2,col_border);
				DrawBar(1,1,linew,1,0xFFFfff);
				PutShadow(linew+2,1,1,ccount*lineh+2,0,2);
				PutShadow(linew+3,2,1,ccount*lineh+2,0,1);
				PutShadow(1,ccount*lineh+3,linew+2,1,0,2);
				PutShadow(2,ccount*lineh+4,linew+1,1,0,1);

				_ITEMS_DRAW:
				for (i=0; captions[i*2]!=0; i++)
				{
					DefineButton(1,i*lineh+1,linew,lineh-1,i+100+BT_HIDE+BT_NOFRAME,0xFFFFFF);
					DrawBar(1,i*lineh+2,1,lineh,0xFFFfff);
					if (i==cur) DrawBar(2,i*lineh+2,linew-1,lineh,0xFFFfff); else DrawBar(2,i*lineh+2,linew-1,lineh,col_work);
					WriteText(7,i*lineh+texty+2,0x80,0x000000,captions[i*2]);
					WriteText(-strlen(captions[i*2+1])*6-6+linew,i*lineh+texty+2,0x80,0x999999,captions[i*2+1]);
				}
	}
}
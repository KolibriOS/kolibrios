//Leency 2008-2013

char *captions[] = {
	"Open",          "Enter",
	"Open with...",  "Ctrl+Ent",
	"View as text",  "F3",
	"View as HEX",   "F4",
	"Rename",        "F2",
	"Delete",        "Del",
	0};

void FileMenu()
{
	proc_info MenuForm;
	word id, key, slot;
	int ccount=0, linew=10, lineh=18, texty;
	for (i=0; captions[i]!=0; i+=2)
	{
		ccount++;
		if (strlen(captions[i])>linew) linew = strlen(captions[i]);
	}
	linew = linew + 3 * 6 + 70;
	texty = lineh/2-4;
	SetEventMask(100111b);

	goto _MENU_DRAW;
	loop() switch(WaitEvent())
	{
		case evMouse:
				slot = GetProcessSlot(MenuForm.ID);
				if (slot != GetActiveProcess()) ExitProcess();
				break;

		case evButton: 
				id=GetButtonID();
				if (id==100) Open();
				if (id==101) notify("Not compleated yet");
				if (id==102) ActionsProcess(3);
				if (id==103) ActionsProcess(4);
				if (id==104) ActionsProcess(2);
				if (id==105) Del_Form();
				ExitProcess();
				break;
				
		case evKey:
				IF (GetKey()==27) ExitProcess();
				break;
				
		case evReDraw: _MENU_DRAW:
				DefineAndDrawWindow(m.x+Form.left+5,m.y+Form.top+GetSkinHeight(),linew+2,ccount*lineh+2,0x01,0xEEEeee,0x01fffFFF);
				GetProcessInfo(#MenuForm, SelfInfo);
				/* _PutImage(1,23, 16,44, #factions); //иконки	*/
				DrawRectangle(0,0,linew+1,ccount*lineh+1,col_border);
				PutShadow(linew+2,1,1,ccount*lineh+1,0,1);
				PutShadow(1,ccount*lineh+2,linew+1,1,0,1);

				for (i=0; captions[i*2]!=0; i++)
				{
					DefineButton(1,i*lineh,linew,lineh-1,i+100+BT_HIDE,0xFFFFFF);
					DrawBar(1,i*lineh+1,1,lineh,0xFFFfff);
					DrawBar(2,i*lineh+1,linew-1,lineh,col_work);
					WriteText(6,i*lineh+texty+1,0x80,0x000000,captions[i*2]);
					WriteText(-strlen(captions[i*2+1])*6-7+linew,i*lineh+texty+1,0x80,0x999999,captions[i*2+1]);
				}
	}
}
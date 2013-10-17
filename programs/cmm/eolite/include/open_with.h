//Leency 2013

llist app_list;
struct app_list_string { char item[1024]; char icon; };
app_list_string app_paths[100];

int GetListOfPrograms()
{
	byte section[32], parametr[32], option[256], InfType=0;
	char bukva[2];
	int tj, ti;
	static dword buff, fsize;

	debug("GetListOfPrograms()");

	free(buff);
	if (!GetFile(#buff, #fsize, abspath("Eolite.ini")))
	{
		notify("Eolite.ini not found. Don't know any programm.");
		return -1;
	}

	for (tj=0; tj<fsize; tj++;) 
	{   
		bukva = ESBYTE[buff+tj];
		switch (bukva)
		{
			case ';': InfType=COMMENT; break;				
			case '[': InfType=SECTION; section=NULL; break;
			case ']': InfType=PARAM; break;
			case '=': InfType=OPTION; break;
			case 0x0a:
			case 0x0d:
				InfType=PARAM;				
				if (!strcmp(#section,"Associations")) && (option)
				{
					for (ti=0; ti<app_list.count; ti++) //do not add duplications
					{
						if (strcmp(#app_paths[ti].item, #option)==0) GOTO _OUT;
					}
					// for (i=0; ext[i]!=0; i+=2;) if (!strcmp(extension, ext[i])) { icon_n = ext[i+1]; break;	}
					strcpy(#app_paths[app_list.count].item, #option);
					app_list.count++;
				}
				_OUT:
				parametr=option=NULL;
				break;
			default:
				IF (InfType==SECTION) chrcat(#section, bukva);
				IF (InfType==PARAM) chrcat(#parametr, bukva);
				IF (InfType==OPTION) chrcat(#option, bukva);
		}
	}
}



void OpenWith()
{
	#define WIN_W 290
	#define OPEN_LIST_VISIBLE_N 12
	#define OPEN_LIST_LINE_H 20
	#define PANEL_H 40
	#define PADDING 8;
	int WIN_H;
	mouse mm;
	word key, slot;
	proc_info MenuForm;

	app_list.ClearList();
	app_list.SetSizes(PADDING,PANEL_H+1,WIN_W-PADDING-PADDING,OPEN_LIST_VISIBLE_N*OPEN_LIST_LINE_H,150,OPEN_LIST_LINE_H);
	if (!app_list.count) if (GetListOfPrograms()==-1) return;
	SetEventMask(100111b);
	goto _APP_LIST_DRAW;

	loop() switch(WaitEvent())
	{
		case evMouse:
				slot = GetProcessSlot(MenuForm.ID);
				if (slot != GetActiveProcess()) ExitProcess();
				mm.get();
				//if (mm.lkm) ExitProcess();
				if (mm.vert)
				{
					app_list.MouseScroll(mm.vert);
					DrawAppList();
				}
				if (mm.x>app_list.x) && (mm.x<app_list.x+app_list.w) && (mm.y>app_list.y) && (mm.y<app_list.y+app_list.h)
				{
					app_list.current_temp = mm.y - app_list.y / app_list.line_h + app_list.first;
					if (app_list.current_temp != app_list.current)
					{
						app_list.current = app_list.current_temp;
						DrawAppList();
					}
				}

				break;
				
		case evKey:
				key = GetKey();
				if (key==27) ExitProcess();
				break;

		case evButton:
				RunProgram(#app_paths[GetButtonID()-10].item, #file_path);
				ExitProcess();
				break;
				
		case evReDraw: _APP_LIST_DRAW:
				WIN_H = app_list.h+PANEL_H+PADDING+2;
				DefineAndDrawWindow(files.w-WIN_W/2+files.x+Form.left+6, 
				files.h-WIN_H/2+files.y+Form.top+GetSkinHeight(),WIN_W+1,WIN_H,0x01, 0, 0, 0x01fffFFF);
				GetProcessInfo(#MenuForm, SelfInfo);
				DrawPopup(0,0,MenuForm.width-2,MenuForm.height-2,0, col_work, col_border);
				DrawRectangle(app_list.x-1, app_list.y-2, app_list.w+1, app_list.h+2, col_border);

				Put_icon(#file_name+_strrchr(#file_name,'.'), 10, 13, col_work);
				WriteText(35,10, 0x80, 0, T_SELECT_APP_TO_OPEN_WITH);
				WriteText(35,23, 0x80, 0, #file_name);

				DrawAppList();
	}
}

void DrawAppList()
{
	dword index, col_bg;
	for (index = 0; (index<app_list.visible) && (index+app_list.first<app_list.count); index++)
	{
		DefineButton(app_list.x, index*app_list.line_h+app_list.y, app_list.w, app_list.line_h-1, index+app_list.first+10+BT_HIDE+BT_NOFRAME, 0);
		if (index+app_list.first==app_list.current) col_bg = col_selec; else col_bg = 0xFFFfff;
		DrawBar(app_list.x, index*app_list.line_h+app_list.y, app_list.w, app_list.line_h, col_bg);
		Put_icon("kex", app_list.x+4, index*app_list.line_h+app_list.y+2, col_bg);
		WriteText(app_list.x+23, index*app_list.line_h+app_list.y+7, 0x80, 0, #app_paths[index+app_list.first].item);
	}
}
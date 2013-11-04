//Leency & SoUrcerer, LGPL

//LoginPanel
#define PANEL_W 220
#define PANEL_H 140
int panel_x, panel_y;

//edit_boxes at LoginPanel 
int	mouse_dd;
unsigned char email_text[128];
unsigned char pass_text[32];
edit_box login_box= {PANEL_W-6,207,16,0xffffff,0x94AECE,0xffffff,0xffffff,0,sizeof(email_text)+2,#email_text,#mouse_dd,0b10};
edit_box pass_box= {PANEL_W-6,207,16,0xffffff,0x94AECE,0xffffff,0xffffff,0,sizeof(pass_text)+2,#pass_text,#mouse_dd,0b1};

void StopConnect(dword message)
{
	if (message) notify(message);
	aim = STOP;
}


void LoginBoxLoop()
{
	int key, id;
	char socket_char;

	SetLoginStatus(NULL);

	goto _LB_DRAW;
	loop()
	{
		WaitEventTimeout(2);
		switch(EAX & 0xFF)
		{
			case evMouse:
				if (!CheckActiveProcess(Form.ID)) break;
				edit_box_mouse stdcall (#login_box);
				edit_box_mouse stdcall (#pass_box);
				break;
				
			case evButton:
				id = GetButtonID(); 
				if (id==1) SaveAndExit();
				if (id==11)	SettingsDialog();
				if (id==12)
				{
					if (!aim) aim=RESOLVE; else aim=NULL;
					GetSettings();
					DrawLoginScreen();
				}
				break;
				
			case evKey:
				key = GetKey();
				if (key==9)
				{
					if (login_box.flags & 0b10)
						{ pass_box.flags = 0b11; login_box.flags = 0; }
					else
						{ pass_box.flags = 0b1; login_box.flags = 0b10; }
					edit_box_draw stdcall(#login_box);
					edit_box_draw stdcall(#pass_box);
					break;				
				}
				if (key==13) && (!aim)
				{
					aim=RESOLVE;
					GetSettings();
					DrawLoginScreen();
				}
				EAX=key<<8;
				edit_box_key stdcall(#login_box);
				edit_box_key stdcall(#pass_box);
				break;
				
			case evReDraw: _LB_DRAW:
				if !(DefineWindow(LOGIN_HEADER)) break;
				DrawLoginScreen();
				break;

			default:
				if (!aim) break;
				if (!email_text) StopConnect("Enter email!");
				if (!pass_text) StopConnect("Enter password!");
				if ((!login) || (!POP_server_path)) StopConnect("Email should be such as username@somesite.com");
				
				if (aim == RESOLVE)
				{
					sockaddr.sin_family = AF_INET4;
					AX = POP_server_port;
					$xchg	al, ah
					sockaddr.sin_port = AX;
					sockaddr.sin_addr = GetIPfromAdress(#POP_server_path);					
					if (!sockaddr.sin_addr) { StopConnect("Can't obtain server IP."); break;}
					aim = OPEN_CONNECTION;
					break;
				}
				
				if (aim == OPEN_CONNECTION)
				{
					socketnum = Socket(AF_INET4, SOCK_STREAM, 0);
					if (socketnum == 0xffffffff) { StopConnect("Cannot open socket."); break;}
					Connect(socketnum, #sockaddr, 16);
					aim = GET_ANSWER_CONNECT;
					break;
				}

		
				if (aim == GET_ANSWER_CONNECT)
				{
					ticks = Receive(socketnum, #immbuffer, BUFFERSIZE, 0);
					if ((ticks == 0xffffff) || (ticks < 2)) { StopConnect("Connection failed"); break;}
					immbuffer[ticks]='\0';					
					
					if (immbuffer[ticks-2]=='\n')
					{
						if (strstr(#immbuffer,"+OK"))
							aim = SEND_USER;
						else
							StopConnect("Failed to connect to server. Retry...");
					}
				}

				if (aim == SEND_USER)
				{
					request_len = GetRequest("USER", #login);
					if (Send(socketnum, #request, request_len, 0) == 0xffffffff)
					{
						SetLoginStatus("Failed to send USER. Retry...");
						break;
					}
					aim = GET_ANSWER_USER;
				}

				if (aim == GET_ANSWER_USER)
				{
					ticks = Receive(socketnum, #immbuffer, BUFFERSIZE, 0);
					if ((ticks == 0xffffffff) || (ticks < 2)) { SetLoginStatus("Connection failed"); break;}
					immbuffer[ticks]='\0';						
					
					if (immbuffer[ticks-2]=='\n') 
					{
						if (strstr(#immbuffer,"+OK"))
							aim = SEND_PASS;
						else
						{
							debug(#immbuffer);
							StopConnect("Wrong username");
						}
					}
					else
					{
						StopConnect("Connection failed"); 
					}
				}

				if (aim == SEND_PASS)
				{		
					request_len = GetRequest("PASS", #pass_text);
					if (Send(socketnum, #request, request_len, 0) == 0xffffffff)
					{
						SetLoginStatus("Failed to send PASS. Retry...");
						break;
					}
					aim = GET_ANSWER_PASS;
				}

				if (aim == GET_ANSWER_PASS)
				{	
					ticks = Receive(socketnum, #immbuffer, BUFFERSIZE, 0);
					if ((ticks == 0xffffff) || (ticks < 2)) { SetLoginStatus("Server disconnected"); break;}
					immbuffer[ticks]='\0';						
					
					if (immbuffer[ticks-2]=='\n')
					{
						if (strstr(#immbuffer,"+OK")) MailBoxLoop();
						else
						{
							StopConnect("Wrong password or POP3 disabled");
							debug(#immbuffer);
						}
					}
					else
					{
						StopConnect("Connection failed"); 
					}
				
				}
				
		}
	}
}



void DrawLoginScreen()
{				
	panel_x = Form.cwidth - PANEL_W /2;
	panel_y = Form.cheight - PANEL_H /2 - 5;

	DrawBar(0,0, Form.cwidth, Form.cheight, sc.work);
	
	WriteText(panel_x,panel_y,0x80,sc.work_text,"Your Email:");
	DrawRectangle(panel_x, panel_y+12, PANEL_W,20, sc.work_graph); //border
	DrawRectangle3D(panel_x+1, panel_y+13, PANEL_W-2,18, 0xDDDddd, 0xFFFfff); //shadow
	DrawRectangle(panel_x+2, panel_y+14, PANEL_W-4,16, 0xFFFfff);
	login_box.left = panel_x+3;
	login_box.top = panel_y+15;
	edit_box_draw stdcall(#login_box);
	
	WriteText(panel_x,panel_y+40,0x80,sc.work_text,"Password:");
	DrawRectangle(panel_x, panel_y+52, PANEL_W,20, sc.work_graph); //border
	DrawRectangle3D(panel_x+1, panel_y+53, PANEL_W-2,18, 0xDDDddd, 0xFFFfff); //shadow
	DrawRectangle(panel_x+2, panel_y+54, PANEL_W-4,16, 0xFFFfff);
	pass_box.left = panel_x+3;
	pass_box.top = panel_y+55;
	edit_box_draw stdcall(#pass_box);
	
	if (!aim)
	{
		DrawCaptButton(panel_x,panel_y+90,100,20,11,sc.work_button, sc.work_button_text,"Settings");
		DrawCaptButton(panel_x+120,panel_y+90,100,20,12,sc.work_button, sc.work_button_text,"Enter >");
	} 
	else DrawCaptButton(panel_x+120,panel_y+90,100,20,12,sc.work_button, sc.work_button_text,"Stop");
	
	SetLoginStatus(cur_st_text);
}


int GetRequest(dword command, text)
{
	strcpy(#request, command);
	if (text)
	{
		chrcat(#request, ' ');
		strcat(#request, text);
	}
	strcat(#request, "\n");
	return strlen(#request);
}

void GetServerPathAndLogin()
{
	int i=strchr(#email_text,'@');
	
	POP_server_path=login=NULL;

	if (i)
	{
		strcpy(#POP_server_path, "pop.");
		strcat(#POP_server_path, #email_text+i);
	}
	strcpy(#login, #email_text);
	login[i-1]=NULL;
}


void GetSettings()
{
	GetServerPathAndLogin();
	if (checked[CUSTOM])
	{
		POP_server_port = DEFAULT_POP_PORT;
	}
	if (checked[MANUAL])
	{
		strcpy(#POP_server_path, #POP_server1);
		POP_server_port = atoi(#POP_server_port1);
	}
	if (!strcmp(#POP_server_path, "pop.gmail.com"))	StopConnect("Can't connect GMAIL servers. Not support SSL/TLS encryption");
}

void SetLoginStatus(dword text1)
{
	if (text1) WriteText(10, Form.cheight-22, 0x80, sc.work_text, text1);
	cur_st_text = text1;
}
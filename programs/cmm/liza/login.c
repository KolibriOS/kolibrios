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


void LoginBoxLoop()
{
	int key, id;
	char socket_char;

	SetLoginStatus(NULL,NULL);

	goto _LB_DRAW;
	loop()
	{
		WaitEventTimeout(2);
		switch(EAX & 0xFF)
		{
			case evMouse:
				IF (GetProcessSlot(Form.ID)-GetActiveProcess()!=0) break;
				edit_box_mouse stdcall (#login_box);
				edit_box_mouse stdcall (#pass_box);
				break;
				
			case evButton:
				id = GetButtonID(); 
				if (id==1) SaveAndExit();
				if (id==11)	OptionsLoop();
				if (id==12)
				{
					if (!aim) aim=GET_PORT; else aim=NULL;
					GetSettings();
					SetLoginStatus(NULL, NULL);
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
				if (key==13)
				{
					if (aim) break;
					aim=GET_PORT;
					GetSettings();
					SetLoginStatus(NULL, NULL);
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
				if (!aim) { SetLoginStatus(NULL, NULL); break; }
				if (!email_text) { notify("Enter email!"); aim=NULL; }
				if (!pass_text) { notify("Enter password!"); aim=NULL; }
				if ((!login) || (!POP_server_path)) { notify("Email should be such as username@somesite.com"); aim=NULL; }
				
				if (aim == GET_PORT)
				{
					SetLoginStatus(5, "Search for free local port...");
					local_port = GetFreePort(1000);
					if (!local_port) { notify("Error: There is no free local ports"); aim=NULL; break;}
					SetLoginStatus(12, "Obtain server IP...");
					aim = GET_SERVER_IP;
				}

				if (aim == GET_SERVER_IP)
				{
					POP_server_IP = GetIPfromAdress(#POP_server_path);
					if (!POP_server_IP) { SetLoginStatus(12, "Can't obtain server IP. Retry..."); break; }
					SetLoginStatus(25, "Obtain to open socket..."w);
					aim = GET_SOCKET;
				}

				if (aim == GET_SOCKET)
				{
					socket = OpenSocket(local_port, POP_server_port, POP_server_IP, SOCKET_ACTIVE);
					if (socket == 0xffffffff) { SetLoginStatus(25, "Error obtaining socket. Retry..."); break;}
					SetLoginStatus(40, "Establish a connection...");
					aim = CONNECT;
				}

				if (aim == CONNECT)
				{
					connection_status=StatusSocket(socket);
					if (connection_status==0) {notify("Connection to server isn't possible"); aim=NULL; break; };
					if (connection_status==7) {SetLoginStatus(40, "Server disconnected. Retry..."); break; };
					if (connection_status!=4) break; //0-connection isn't possible, 2-connecting, 4-connected, 7-server disconnected
					SetLoginStatus(55, "Connection established. Reading answer...");
					immfree();
					aim = GET_ANSWER_CONNECT;
				}
				
				if (aim == GET_ANSWER_CONNECT)
				{
					if (!PollSocket(socket)) break;
					socket_char=ReadSocket(socket);
					immputc(socket_char);
					
					if (socket_char=='\n')
					{
						debug(#immbuffer);
						if (strstr(#immbuffer,"+OK"))
						{
							SetLoginStatus(60, "Verifying username...");
							aim = SEND_USER;
							immfree();
						}
						else
						{
							immfree(); 
							//aim=NULL; //may don't need retry?
							SetLoginStatus(55, "Failed to connect to server. Retry...");
						}
					}
				}

				if (aim == SEND_USER)
				{
					request_len = GetRequest("USER", #login);
					WriteSocket(socket,request_len,#request);
					if (EAX == 0xffffffff) { SetLoginStatus(60, "Failed to send USER. Retry..."); break;}
					SetLoginStatus(70, "Login verifying...");
					debug("Send USER, awaiting answer...");
					aim = GET_ANSWER_USER;
				}

				if (aim == GET_ANSWER_USER)
				{
					if (!PollSocket(socket)) break;
					socket_char=ReadSocket(socket);
					immputc(socket_char);
					
					if (socket_char=='\n') 
					{
						debug("GOT::");
						debug(#immbuffer);
						if (strstr(#immbuffer,"+OK"))
							{ aim = SEND_PASS; SetLoginStatus(80, "Verifying password..."); immfree(); }
						else
							{ notify("Wrong username"); immfree(); aim=NULL;}
					}
				}

				if (aim == SEND_PASS)
				{		
					debug("\n Send PASS, awaiting answer...");			
					request_len = GetRequest("PASS", #pass_text);
					WriteSocket(socket,request_len,#request);
					if (EAX == 0xffffffff) { SetLoginStatus(80, "Failed to send PASS. Retry..."); break;}
					aim = GET_ANSWER_PASS;
				}

				if (aim == GET_ANSWER_PASS)
				{	
					if (!PollSocket(socket)) break;
					socket_char=ReadSocket(socket);
					immputc(socket_char);
					
					if (socket_char=='\n')
					{
						debug("GOT::");
						debug(#immbuffer);
						if (strstr(#immbuffer,"+OK"))
						{
							SetLoginStatus(100, "Entering mailbox...");
							aim=SEND_NSTAT;
							immfree();
							MailBoxLoop();
						}
						else
						{
							notify("Wrong password");
							aim=NULL;
							immfree();
						}
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
	
	SetLoginStatus(cur_st_percent, cur_st_text);
}


int GetRequest(dword command, text)
{
	strcpy(#request, command);
	if (text)
	{
		strcat(#request, " ");
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
	GetServerPathAndLogin(); //get
	if (checked[CUSTOM])
	{
		if (!strcmp(#POP_server_path, "pop.gmail.com")) POP_server_port = 995;
		else POP_server_port = DEFAULT_POP_PORT;
	}
	if (checked[MANUAL])
	{
		strcpy(#POP_server_path, #POP_server1);
		POP_server_port = atoi(#POP_server_port1);
	}
	debug(#POP_server_path);
	debug(itoa(POP_server_port));
}

void SetLoginStatus(dword percent1, text1)
{
	DrawProgressBar(10, Form.cheight-22, PANEL_W, 12, sc.work, sc.work_graph, 0x54B1D6, sc.work_text, percent1, text1);
	cur_st_percent = percent1;
	cur_st_text = text1;
}
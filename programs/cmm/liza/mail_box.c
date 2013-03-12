//Leency & SoUrcerer, LGPL

#define LIST_INFO_H 59
int status_bar_h = 15;

scroll_bar scroll1 = { 17,200,210, LIST_INFO_H-3,18,0,115,15,0,0xCCCccc,0xD2CED0,0x555555,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1};
scroll_bar scroll2 = { 17,200,210, LIST_INFO_H,18,0,115,15,0,0xCCCccc,0xD2CED0,0x555555,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1};


llist mail_list;
llist letter_view;

char from[256];
char to[256];
char date[256];
char subj[256];
dword mdata;

char *listbuffer;
char *listpointer;

char *mailbuffer;
char *mailpointer;


enum {
	GET_MAIL = 20,
	SEND_MAIL,
	DELETE_LETTER,
	SAVE_LETTER,
	STOP_LOADING,
	EXIT_MAIL,
	CHANGE_CHARSET,
	CLOSE_CHANGE_CHARSET
};


void MailBoxLoop()
{
	int key, id;
	mouse m;
	int panels_drag = 0;
	char socket_char;
	int letter_size;
	dword line_col, text_col;

	mail_list.h = Form.cheight/4;
	mail_list.ClearList();
	SetMailBoxStatus( NULL , NULL);
	cur_charset = 0;
	
	goto _MB_DRAW;

	loop()	
	{
		WaitEventTimeout(2);
		switch(EAX & 0xFF)
		{
			case evMouse:
				IF (GetProcessSlot(Form.ID)-GetActiveProcess()!=0) break;
				m.get();
				
				if (!m.lkm) panels_drag=0;
				if (m.lkm) && (m.y>mail_list.y+mail_list.h-1) && (m.y<mail_list.y+mail_list.h+6)
				&& (!scroll1.delta2) && (!scroll2.delta2) panels_drag = 1;
				if (panels_drag)
				{
					if (m.y<mail_list.y+mail_list.min_h) || (m.y>Form.cheight-letter_view.min_h-status_bar_h-LIST_INFO_H) break;
					mail_list.h = m.y - mail_list.y-2;
					DrawMailBox();
					break;
				}

				if (!mail_list.count) break;
				if (!panels_drag) { scrollbar_v_mouse (#scroll1); scrollbar_v_mouse (#scroll2); }
				
				if (mail_list.first <> scroll1.position)
				{
					mail_list.first = scroll1.position;
					DrawMailList();
					break;
				};
				if (letter_view.first <> scroll2.position)
				{
					letter_view.first = scroll2.position;
					DrawLetter();
					break;
				};
				
				if (mail_list.y+mail_list.h + 10 > m.y)
				{
					if (mail_list.MouseScroll(m.vert)) DrawMailList();
				}
				else
				{
					if (letter_view.MouseScroll(m.vert)) DrawLetter();
				}

				break;				
			case evButton:
				id = GetButtonID(); 
				if (id==1) SaveAndExit();
				if (id==GET_MAIL) aim = SEND_NSTAT;
				if (id==SAVE_LETTER)
				{
					if (!mailbuffer) break;
					WriteFile(strlen(mailbuffer), mailbuffer, "mail.txt");
					pause(10);
					RunProgram("tinypad", "mail.txt");
				}
				if (id==STOP_LOADING) 
				{
					StopLoading();
					DrawStatusBar();
					DrawMailList();
				}
				if (id==EXIT_MAIL) 
				{
					StopLoading();
					LoginBoxLoop();
				}
				if (id==CHANGE_CHARSET) 
				{
					DefineButton(0,0,Form.cwidth,Form.cheight, CLOSE_CHANGE_CHARSET+BT_HIDE+BT_NOFRAME);
					DrawRectangle(Form.cwidth-100, Form.cheight-status_bar_h- 70, 70, 82, sc.work_graph);
					DrawRectangle3D(Form.cwidth-99, Form.cheight-status_bar_h- 69, 68, 80, 0xFFFfff, sc.work);
					for (id=0; id<5; id++)
					{
						if (cur_charset==id+1) { line_col=sc.work_button; text_col=sc.work_button_text; }
						else { line_col=sc.work; text_col=sc.work_text; }
						DrawBar(Form.cwidth-98, id*16+Form.cheight-status_bar_h- 68, 67, 16, line_col);
						DrawCaptButton(Form.cwidth-100, id*16+Form.cheight-status_bar_h- 68, 70,16, 10+id+BT_HIDE,
						0, text_col, charsets[id+1]);
					}
				}
				if (id==CLOSE_CHANGE_CHARSET) goto _MB_DRAW;

				if (id>=30)
				{
					if (aim) break;
					mail_list.current = mail_list.first + id - 30;
					DrawMailList();
					aim = SEND_RETR;
				}

				break;				
			case evKey:
				key = GetKey();

				if (key == 177){ //down
					if (aim) break;
					if (mail_list.current >= mail_list.count-1) break;
					mail_list.current++ ;
					if (mail_list.current >= mail_list.first + mail_list.visible) mail_list.first++ ;
					DrawMailList();
					aim = SEND_RETR;
				}
				if (key == 178){ //up
					if (aim) break;
					if (mail_list.current<=0) break;
					mail_list.current-- ;
					if (mail_list.current < mail_list.first) mail_list.first-- ;
					DrawMailList();
					aim = SEND_RETR;
				}
				if (key == 180){ //home
					if (aim) break;
					mail_list.first = mail_list.current = 0;
					DrawMailList();
					aim = SEND_RETR;
				}
				if (key == 181){ //end
					if (aim) break;
					mail_list.first = mail_list.count - mail_list.visible;
					mail_list.current = mail_list.count - 1; 
					DrawMailList();
					aim = SEND_RETR;
				}

				break;
			case evReDraw: _MB_DRAW:
				if !(DefineWindow(MAILBOX_HEADER)) break;
				scroll1.bckg_col = scroll2.bckg_col = 0xBBBbbb;
				scroll1.frnt_col = scroll2.frnt_col = sc.work;
				scroll1.line_col = scroll2.line_col = sc.work_graph;
				DrawToolbar();
				DrawMailBox();

				break;
			default:
				if (aim == SEND_NSTAT)
				{
					debug("Counting mail, awaiting answer...");			
					request_len = GetRequest("STAT", NULL);
					WriteSocket(socket,request_len,#request);
					if (EAX == 0xffffffff) { debug("Error sending STAT. Retry..."w); break;}
					aim = GET_ANSWER_NSTAT;
				}
				
				if (aim == GET_ANSWER_NSTAT)
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
							mail_list.count = GetMailCount();
							debug("Letters:");
							debug(itoa(mail_list.count));
							listbuffer = free(listbuffer);
							listbuffer = mem_Alloc(30*mail_list.count); //24* original
							listpointer = listbuffer;	
							aim = SEND_NLIST;
							debug("Recieving mail list...");
							immfree();
						}
						else
						{
							notify("Sorry, can't recieve your mail");
							immfree();
							aim=NULL;    //aim = SEND_NLIST;
						}
					}
				}

				if (aim == SEND_NLIST)
				{		
					WriteText(5, Form.cheight-11, 0x80, sc.work_text, "Send LIST, awaiting answer...");
					request_len = GetRequest("LIST", NULL);
					WriteSocket(socket,request_len,#request);
					if (EAX == 0xffffffff) {debug("Error while sending LIST. Retry..."); break;}
					aim = GET_ANSWER_NLIST;
				}

				if (aim == GET_ANSWER_NLIST)
				{	
					ticks = PollSocket(socket);
					if (!ticks) break;
					for (;ticks>0;ticks--)
					{
						socket_char=ReadSocket(socket);
						listputc(socket_char);
						
						if (socket_char=='.') //this way of checking end of message IS BAD
						{
							aim = SEND_RETR;
							debug("Got mail list");
							DrawMailBox();

							CreateMailsArray();
							SetMailsSizes();
						}
					} 
				}
				if (aim == SEND_RETR)
				{
					from = to = date = subj = cur_charset = NULL;
					letter_view.ClearList();
					DrawMailBox();
					debug("Send RETR, awaiting answer...");
					request_len = GetRequest("RETR", itoa(mail_list.current+1));
					WriteSocket(socket,request_len,#request);
					if (EAX == 0xffffffff) { notify("Error while trying to get letter from server"); aim=NULL; break;}

					mailbuffer = free(mailbuffer);
					letter_size = GetLetterSize(mail_list.current+1) + 1024;
					mailbuffer = malloc(letter_size);
					mailpointer = mailbuffer;
					aim = GET_ANSWER_RETR;
				}
				if (aim == GET_ANSWER_RETR)
				{
					ticks=PollSocket(socket);
					if (!ticks) break;

					for (;ticks>0;ticks--)
					{
						socket_char=ReadSocket(socket);
						//debugch(socket_char);
						*mailpointer=socket_char;
						mailpointer++;
						*mailpointer='\0';
						if (!aim) continue;

						if (letter_size + mailbuffer - mailpointer - 2 < 0)
						{
							debug("Buffer overflow!!1 Realloc..."w);
							letter_size += 4096;
							mailbuffer = realloc(mailbuffer, letter_size);
							if (!mailbuffer) {debug("Relloc error!"); aim=NULL; break;}
						}

						if (letter_size>5000)
						{
							id = mailpointer - mailbuffer * 100 ;
							id /= letter_size - 1024;
							if (id!=cur_st_percent) SetMailBoxStatus( id , NULL);
						}

						ParceMail();
					} 
				}

		}
	}
}


void DrawMailBox()
{
	DrawMailList();
	DrawLetterInfo();
	DrawLetter();
	DrawStatusBar();
}


void DrawToolbar()
{
	#define BUT_Y 7
	#define BUT_H 22
	#define BUT_W 74
	#define BUT_SPACE 11
	int toolbar_w = BUT_Y + BUT_H + BUT_Y + 3;
	mail_list.SetSizes(0, toolbar_w, Form.cwidth - scroll1.size_x - 1, mail_list.h, 60,18);

	DrawBar(0,0, Form.cwidth,toolbar_w-3, sc.work);
	DrawCaptButton(10                    , BUT_Y, BUT_W, BUT_H, GET_MAIL,          sc.work_button, sc.work_button_text,"Get mail");
	DrawCaptButton(BUT_W+BUT_SPACE   + 10, BUT_Y, BUT_W, BUT_H, SEND_MAIL,         sc.work_button, sc.work_button_text,"Send Email");
	DrawCaptButton(BUT_W+BUT_SPACE*2 + 10, BUT_Y, BUT_W, BUT_H, DELETE_LETTER,     sc.work_button, sc.work_button_text,"Delete");
	DrawCaptButton(BUT_W+BUT_SPACE*3 + 10, BUT_Y, BUT_W, BUT_H, SAVE_LETTER,       sc.work_button, sc.work_button_text,"Save");
	DrawCaptButton(Form.cwidth-BUT_W - 10, BUT_Y, BUT_W, BUT_H, EXIT_MAIL,         sc.work_button, sc.work_button_text,"< Exit");

	DrawBar(0, mail_list.y-3, mail_list.w,1, sc.work_graph);
	DrawBar(0, mail_list.y-2, mail_list.w,1, 0xdfdfdf);
	DrawBar(0, mail_list.y-1, mail_list.w,1, 0xf0f0f0);
}


void DrawMailList()
{
	int i, on_y;
	mail_list.visible = mail_list.h / mail_list.line_h;

	for (i=30; i<150; i++) DeleteButton(i); 
	for (i=0; (i<mail_list.visible) && (i+mail_list.first<mail_list.count); i++)
	{
		on_y = i*mail_list.line_h + mail_list.y;
		if (mail_list.current==mail_list.first+i)
		{
			DrawBar(0, on_y, mail_list.w, mail_list.line_h-1, 0xEEEeee);
			WriteText(strlen(itoa(i+mail_list.first+1))*6 + 42, on_y+5, 0x80, 0, #subj);
			PutMailDirectionImage(strlen(itoa(i+mail_list.first+1))*6 + 18, mail_list.line_h-12/2+ on_y, #to, #from);
		}
		else
		{
			DrawBar(0, on_y, mail_list.w, mail_list.line_h-1, 0xFFFfff);
			PutMailDirectionImage(strlen(itoa(i+mail_list.first+1))*6 + 18, mail_list.line_h-12/2+ on_y, NULL, NULL);
		}
		DefineButton(0, on_y, mail_list.w-1, mail_list.line_h, 30+i+BT_HIDE+BT_NOFRAME);
		DrawBar(0, on_y + mail_list.line_h-1, mail_list.w, 1, 0xCCCccc);
		WriteText(10, on_y+5, 0x80, 0, itoa(i+mail_list.first+1));
		//WriteText(mail_list.w - 40, on_y+5, 0x80, 0, itoa(GetLetterSize(i+mail_list.first+1)));
		WriteText(mail_list.w - 40, on_y+5, 0x80, 0, ConvertSize(GetLetterSize(i+mail_list.first+1)));
	}
	DrawBar(0, i*mail_list.line_h + mail_list.y, mail_list.w, -i*mail_list.line_h+mail_list.h, 0xFFFfff);
	DrawScroller1();
}


void PutMailDirectionImage(int x,y, to_str, from_str)
{
	char mail_direction=0;
	if (strstri(to_str, #email_text)) mail_direction = 1;
	if (strstri(from_str, #email_text)) mail_direction = 2;
	_PutImage(x, y, 18,12, sizeof(in_out_mail)/3*mail_direction + #in_out_mail);
}


dword ConvertSize(unsigned int bytes)
{
	unsigned char size_prefix[8], size_nm[4];
	if (bytes>=1073741824) strcpy(#size_nm, " Gb");
	else if (bytes>=1048576) strcpy(#size_nm, " Mb");
	else if (bytes>=1024) strcpy(#size_nm, " Kb");
	else strcpy(#size_nm, " b ");
	while (bytes>1023) bytes/=1024;
	strcpy(#size_prefix, itoa(bytes));
	strcat(#size_prefix, #size_nm);
	return #size_prefix;
}


void DrawLetterInfo()
{
	int lt_y = mail_list.y+mail_list.h;

	DrawBar(0, lt_y, mail_list.w, 1, sc.work_graph);
	DrawBar(0, lt_y+1, Form.cwidth, 1, LBUMP);
	DrawBar(0, lt_y+2, Form.cwidth, LIST_INFO_H-4, sc.work);
	WriteText(mail_list.w-30/2, lt_y, 0x80, 0x888888, "= = =");
	WriteText(mail_list.w-30/2, lt_y+1, 0x80, 0xEeeeee, "= = =");
	DrawBar(0, lt_y+LIST_INFO_H-2, mail_list.w, 1, sc.work_graph); //bottom
	DrawBar(0, lt_y+LIST_INFO_H-1, mail_list.w, 1, 0xdfdfdf);
	DrawBar(0, lt_y+LIST_INFO_H  , mail_list.w, 1, 0xf0f0f0);

	WriteTextB(10, lt_y+8 , 0x80, sc.work_text, "From:");
	WriteText(45, lt_y+8 , 0x80, sc.work_text, #from);

	WriteTextB(10, lt_y+20 , 0x80, sc.work_text, "To:");
	WriteText(45, lt_y+20 , 0x80, sc.work_text, #to);
	
	WriteTextB(10, lt_y+32, 0x80, sc.work_text, "Date:");
	WriteText(45, lt_y+32, 0x80, sc.work_text, #date);

	WriteTextB(10, lt_y+44, 0x80, sc.work_text, "Subject:");
	WriteText(66, lt_y+44, 0x80, sc.work_text, #subj);
}

void DrawLetter()
{
	int i=0;
	dword cur_line, next_line, line_text;
	cur_line = mdata;

	letter_view.SetSizes(0, mail_list.y+mail_list.h+LIST_INFO_H+1, Form.cwidth - scroll2.size_x - 1, 
		Form.cheight - mail_list.y - mail_list.h - LIST_INFO_H - 1 - status_bar_h, 60, 12);

	if (mailbuffer) && (!aim)
	{
		for ( ; i < letter_view.first; i++) cur_line = GetNextLine(cur_line);

		for (i=0; i < letter_view.h / letter_view.line_h; i++)
		{
			next_line = GetNextLine(cur_line);
			line_text = CopyBetweenOffsets(cur_line, next_line);
			cur_line = next_line;
			if (cur_line >= mailpointer) || (cur_line==1) break;
			DrawBar(letter_view.x, i*letter_view.line_h + letter_view.y, letter_view.w, letter_view.line_h, 0xFFFfff);
			if (line_text) { WriteText(letter_view.x+5, i*letter_view.line_h+letter_view.y+3, 0x80, 0, line_text); free(line_text);}
		}
	}
	DrawBar(letter_view.x, i*letter_view.line_h + letter_view.y, letter_view.w, -i*letter_view.line_h + letter_view.h-1, 0xFFFfff);
	DrawBar(letter_view.x, letter_view.y + letter_view.h-1, letter_view.w, 1, sc.work_graph);
	DrawScroller2();
}

void DrawScroller1()
{
	scroll1.max_area = mail_list.count;
	scroll1.cur_area = mail_list.visible;
	scroll1.position = mail_list.first;

	scroll1.all_redraw=1;
	scroll1.start_x = mail_list.x + mail_list.w;
	scroll1.start_y = mail_list.y - 3;
	scroll1.size_y = mail_list.h + 4;
	scrollbar_v_draw(#scroll1);
}

void DrawScroller2()
{
	scroll2.max_area = letter_view.count;
	scroll2.cur_area = letter_view.visible;
	scroll2.position = letter_view.first;

	scroll2.all_redraw=1;
	scroll2.start_x = letter_view.x + letter_view.w;
	scroll2.start_y = letter_view.y - 3;
	scroll2.size_y = letter_view.h + 3;
	scrollbar_v_draw(#scroll2);
}



void DrawStatusBar()
{
	int st_y = Form.cheight -status_bar_h;
	DrawBar(0, st_y, Form.cwidth, status_bar_h, sc.work);
	if (aim) {
		SetMailBoxStatus(cur_st_percent, cur_st_text);
		DrawCaptButton(240, st_y+1, 36, status_bar_h-3, STOP_LOADING, sc.work_button, sc.work_button_text,"Stop");
	}
	DrawCaptButton(Form.cwidth - 100, st_y+1, 70, status_bar_h-2, CHANGE_CHARSET+BT_HIDE, sc.work, sc.work_text,charsets[cur_charset]);
}



void SetMailBoxStatus(dword percent1, text1)
{
	DrawProgressBar(3, Form.cheight -status_bar_h + 1, 220, 12, sc.work, 0xC3C3C3, 0x54B1D6, sc.work_text, percent1, text1);
	cur_st_percent = percent1;
	cur_st_text = text1;
}


void StopLoading()
{
	aim = NULL;
	mailbuffer = free(mailbuffer);
	to = from = date = subj = cur_charset = NULL;
	while (PollSocket(socket)) ReadSocket(socket);	
}

int GetMailCount(){
	char tmpbuf4[512];
	strcpyb(#immbuffer, #tmpbuf4, "+OK ", " ");
	return atoi(#tmpbuf4);
}

















void listputc(char agot_char){		
	*listpointer=agot_char;
	listpointer++;
	*listpointer='\0';
}

int GetLetterSize_(int number){
	char serch_num[24];
	char letter_size1[24];
	
	strcpy(#serch_num, "\n");
	strcat(#serch_num, itoa(number));
	strcat(#serch_num, " ");
	strcpyb(listbuffer, #letter_size1, #serch_num, "\n");
	return atoi(#letter_size1); 
}


struct line_element
{
   byte m_type;
   char adress[64];
   char header[256];
   int size;
};
dword mails_db;

void CreateMailsArray()
{
	mails_db = free(mails_db);
	mails_db = malloc( mail_list.count * sizeof(line_element) );
}

dword GetCurrentElement(int el_N)
{
	return sizeof(line_element)*el_N + #mails_db;
}

void SetMailsSizes()
{
	int i, temp;
	for (i=0; i < mail_list.count; i++)
	{
		temp = GetLetterSize_(i);
		EBX = GetCurrentElement(i); //в регистр EBX суём адрес блока памяти со смещением к элементу N с котрым мы хотим работать
		EBX.line_element.size = temp;//работаем с m_type N-ного элемента структуры отраженной на блок памяти
	}
}

int GetLetterSize(int el_N)
{
	EBX = GetCurrentElement(el_N);
	return EBX.line_element.size;
}
//Leency & SoUrcerer, LGPL

void ParseMail()
{
	dword line_off, new_buf;
	char tline[256];

	if ( mailend-mailstart > 9) if (strncmp(mailend-5,"\n.\n",5)==0) // note that c-- assembles "\n.\n" to 0x0d, 0x0a, 0x2e, 0x0d, 0x0a
	{
		aim = STOP;
		mailend -= 5;
		DSBYTE[mailend] = '\0';
		if (strstr(mailstart, "+OK")!=mailstart) 
		{
			aim = GET_ANSWER_RETR;
			mailend = mailstart;
			debug("GET_ANSWER_RETR != +OK, retry GET_ANSWER_RETR");
			return;
		}
		mailsize = mailend - mailstart;
		debug("Getting QP");
		if (strstri(mailstart, "quoted-printable")!=0)
		{
			new_buf = malloc(mailsize);
			qp_decode stdcall (mailstart, new_buf, mailsize);
			if (EAX==-1) debug("Too small buffer to convert QUOTED-PRINTABLE");
			else
			{
				free(mailstart);
				mailstart = new_buf;
				mailsize = strlen(mailstart);
				mailend = mailsize + mailstart;
			}
		}
		debug("ProcessBase64");
		ProcessBase64();
		debug("GetHeaders: From, To, Date, Subject");
		GetHeader(#from, "\nFrom:");
		GetHeader(#to,   "\nTo:");
		GetHeader(#date, "\nDate:");
		GetHeader(#subj, "\nSubject:");
		debug("Get mdata");
		mdata = strstr(mailstart, "\x0a\x0d") + 3;
		debug("ConvertToDOS");
		ConvertToDOS(mdata, mailstart);
		debug("SetAtrFromCurr");
		atr.SetAtrFromCurr(mail_list.cur_y+1);
		DrawMailBox();
	}
}

void ConvertToDOS(dword inbuf, searchin)
{
	dword dos_buf=0;
	cur_charset = CH_CP866;
	if (strstri(searchin, "windows-1251")!=0) || (strstri(searchin, "windows1251")!=0) 
		{ dos_buf = ChangeCharset("CP1251", "CP866", inbuf);    cur_charset = CH_CP1251;}
	else if (strstri(searchin, "koi8-")!=0)
		{ dos_buf = ChangeCharset("KOI8-RU", "CP866", inbuf);   cur_charset = CH_KOI8;}
	else if (strstri(searchin, "utf-8")!=0) || (strstri(searchin, "utf8")!=0)
		{ dos_buf = ChangeCharset("UTF-8", "CP866", inbuf);     cur_charset = CH_UTF8;}
	else if (strstri(searchin, "iso8859-5")!=0) || (strstri(searchin, "iso-8859-5")!=0)
		{ dos_buf = ChangeCharset("ISO8859-5", "CP866", inbuf); cur_charset = CH_ISO8859_5;}
	else if (strstri(searchin, "windows-1252")!=0) || (strstri(searchin, "windows1252")!=0) 
		{ dos_buf = ChangeCharset("CP1252", "CP866", inbuf);    cur_charset = CH_CP1252;}
}


dword CopyBetweenOffsets(dword start, end) //do not forget to free(line) after use
{
	dword line, new_line;
	if (end <= start) return 0;
	line = new_line = malloc(end - start + 3);
	while (end > start) 
	{
		DSBYTE[new_line] = DSBYTE[start];
		start++;
		new_line++;
	}
	DSBYTE[new_line] = '\0';
	return line;
}


void GetHeader(dword workstr, searchstr)
{
	char tmpbuf[512];
	dword Qoff;
	int q_start, b_start;

	strcpyb(mailstart, workstr, searchstr, "\n");
	/*
	debug(searchstr);
	debug(workstr);
	if (strlen(workstr)<8) return;
	q_start = strstri(workstr, "?Q?");
	b_start = strstri(workstr, "?B?");
	if (q_start)
	{
		qp_decode stdcall (workstr, #tmpbuf, strlen(workstr));
		ConvertToDOS(#tmpbuf, workstr);	
		strcpy(workstr, #tmpbuf);
		Qoff = strstri(workstr, "?Q?");
		strcpy(workstr, Qoff);
	}

	//any text that goes after "?=" deletes now
	if (b_start)
	{
		base64_decode stdcall (b_start, #tmpbuf, strlen(b_start)-5);
		ConvertToDOS(#tmpbuf, workstr);	
		strcpy(workstr, #tmpbuf);
	}
	if (strlen(workstr)+10*6-Form.cwidth>0) { workstr=Form.cwidth/6+workstr-12; DSBYTE[workstr]='\0';}
	*/
}



void ProcessBase64()
{
	int b_start, b_end, b_size, b_buf;
	int clean_mailstart;

	b_start = strstr(mailstart, "?B?");
	debug("b_size");
	debugi(b_start);
	if (b_start)
	{
		b_end = strstr(b_start, "?=");
		debug("b_end");
		debugi(b_end);
		b_size = b_end - b_start;
		debug("b_size");
		debugi(b_size);
		b_buf = malloc(b_size);
		strcpyb(mailstart, b_buf, "?B?", "?=");
		debug("b_buf");
		debug(b_buf);

		base64_decode stdcall (b_buf, b_buf, b_size-3);
		ConvertToDOS(b_buf, mailstart);	
		if (b_size<strlen(b_buf)) notify("base64 overflow");

		clean_mailstart = malloc(strlen(mailstart));
		strlcpy(clean_mailstart, mailstart, b_start-mailstart);
		strcat(clean_mailstart, b_buf);
		strcat(clean_mailstart, b_end+2);
		free(b_buf);
		free(mailstart);
		mailsize = strlen(clean_mailstart);
		mailstart = clean_mailstart;
		mailend = mailstart + mailsize;
		ProcessBase64();
	}
}


//
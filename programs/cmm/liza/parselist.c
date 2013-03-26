//Leency & SoUrcerer, LGPL

void ParceMail()
{
	dword line_off, new_buf;
	char tline[256];

	if ( mailpointer-mailbuffer>9 ) if ( (strncmp(mailpointer-5,"\r\n.\r\n",5)==0) || (strncmp(mailpointer-3,"\n.\n",3)==0) )
	{
		if (strstr(mailbuffer, "+OK")!=mailbuffer) 
		{
			aim = GET_ANSWER_RETR;
			mailpointer = mailbuffer;
			debug("GET_ANSWER_RETR != +OK, retry GET_ANSWER_RETR");
			return;
		}
		aim=NULL;
		DSBYTE[mailpointer+1] = '\0';
		debug("Real letter size:");
		debugi(mailpointer - mailbuffer);

		if (strstri(mailbuffer, "quoted-printable")!=0)
		{
			debug ("getting qp");
			new_buf = malloc(mailpointer-mailbuffer);
			qp_decode stdcall (mailbuffer, new_buf, mailpointer-mailbuffer);
			if (EAX==-1) debug("Too small buffer to convert QUOTED-PRINTABLE");
			else
			{
				mailbuffer = free(mailbuffer);
				mailbuffer = new_buf;
				mailpointer = strlen(mailbuffer) + mailbuffer;
			}
		}
		debug ("getting list info");
		GetHeader(#from, "\nFrom:");
		GetHeader(#to,   "\nTo:");
		GetHeader(#date, "\nDate:");
		GetHeader(#subj, "\nSubject:");
		mdata = strstr(mailbuffer, "\n\r") + 3;
		debug ("converting to dos");
		ConvertToDOS(mdata, mailbuffer);
		FromHTMLtoTXT();
		letter_view.first = letter_view.count = 0;
		
		line_off = mdata;
		while (line_off>1)
		{
			line_off = GetNextLine(line_off);
			letter_view.count++;	
		}
		atr.SetAtrFromCurr(mail_list.current+1);
		DrawMailBox();
	}
}

void ConvertToDOS(dword inbuf, searchin)
{
	dword dos_buf=0;
	if (use_iconv==1)
	{
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
	else
	{
		if (strstri(searchin, "windows-1251")!=0) wintodos( inbuf); else
		if (strstri(searchin, "koi8-")!=0)        koitodos( inbuf); else
		if (strstri(searchin, "utf-8")!=0)        utf8rutodos( inbuf);
	}
}


void FromHTMLtoTXT()
{
	dword cur_chr, txt_buf_srt, txt_buf_end, is_tag=0;
	int i;
	if (strstri(mdata, "<html>")==0) && (strstri(mailbuffer, "text/html")==0) {debug("no html tags found"); return;}
	debug ("converting: html -> txt");
	cur_chr = mdata;
	txt_buf_srt = malloc(mailpointer - mailbuffer);
	txt_buf_end = txt_buf_srt;

	while (cur_chr < mailpointer)
	{
		if (DSBYTE[cur_chr]=='<') is_tag = 1;
		if (!is_tag)
		{
		 	DSBYTE[txt_buf_end] = DSBYTE[cur_chr];
		 	txt_buf_end++;
		 	_END:
		}
		if (DSBYTE[cur_chr]=='>') is_tag = NULL;
		cur_chr++;
	}
	DSBYTE[txt_buf_end] = '\0';
	strcpy(mdata, txt_buf_srt);
	mailpointer = strlen(mailbuffer) + mailbuffer; //тупо везде это ставить
	free(txt_buf_srt);
}


dword GetNextLine(dword start_offset)
{
	dword off_n = strstr(start_offset, "\n") + 1,  //разрыв строки
	      off_w = letter_view.w / 6 - 2 + start_offset, //max длинна скроки
	      off_m;
	off_m = off_w;
	if (off_n < off_w) return off_n;
	while (off_m > start_offset) //перенос по словам
	{
		if (DSBYTE[off_m]==' ') || (DSBYTE[off_m]=='\9') || (DSBYTE[off_m]=='-') return off_m;
		off_m--;
	}
	return off_w;
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

	strcpyb(mailbuffer, workstr, searchstr, "\n");
	if (strstri(workstr, "?Q?"))
	{
		qp_decode stdcall (workstr, #tmpbuf, strlen(workstr));
		ConvertToDOS(#tmpbuf, workstr);	
		strcpy(workstr, #tmpbuf);
		Qoff = strstri(workstr, "?Q?");
		strcpy(workstr, Qoff);
	}
	if (strstr(workstr, "?B?"))
	{
		base64_decode stdcall (strstri(workstr, "?B?"), #tmpbuf, strlen(workstr));
		ConvertToDOS(#tmpbuf, workstr);	
		strcpy(workstr, #tmpbuf);
	}
	if (strlen(workstr)+10*6-Form.cwidth>0) { workstr=Form.cwidth/6+workstr-12; DSBYTE[workstr]='\0';}
}
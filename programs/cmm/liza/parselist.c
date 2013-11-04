//Leency & SoUrcerer, LGPL

void ParseMail()
{
	dword line_off, new_buf;
	char tline[256];

	if ( mailend-mailstart > 9) if (strncmp(mailend-5,"\n.\n",5)==0) // note that c-- assembles "\n.\n" to 0x0d, 0x0a, 0x2e, 0x0d, 0x0a
	{
		mailend -= 5;
		if (strstr(mailstart, "+OK")!=mailstart) 
		{
			aim = GET_ANSWER_RETR;
			mailend = mailstart;
			debug("GET_ANSWER_RETR != +OK, retry GET_ANSWER_RETR");
			return;
		}
		aim=NULL;
		DSBYTE[mailend] = '\0';
		mailsize = mailend - mailstart;

		if (strstri(mailstart, "quoted-printable")!=0)
		{
			new_buf = malloc(mailend-mailstart);
			qp_decode stdcall (mailstart, new_buf, mailend-mailstart);
			if (EAX==-1) debug("Too small buffer to convert QUOTED-PRINTABLE");
			else
			{
				free(mailstart);
				mailstart = new_buf;
				mailsize = strlen(mailstart);
				mailend = mailsize + mailstart;
			}
		}
		GetHeader(#from, "\nFrom:");
		GetHeader(#to,   "\nTo:");
		GetHeader(#date, "\nDate:");
		GetHeader(#subj, "\nSubject:");
		mdata = strstr(mailstart, "\n\r") + 3;		// 0x0d 0x0a, 0x0a
		ConvertToDOS(mdata, mailstart);
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
	if (strstri(mdata, "<html>")==0) && (strstri(mailstart, "text/html")==0) {debug("no html tags found"); return;}
	debug ("converting: html -> txt");
	cur_chr = mdata;
	txt_buf_srt = malloc(mailend - mailstart);
	txt_buf_end = txt_buf_srt;

	while (cur_chr < mailend)
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
	mailend = strlen(mailstart) + mailstart;
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

	strcpyb(mailstart, workstr, searchstr, "\n");
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
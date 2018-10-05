enum { TAG, OPTION_VALUE, TEXT, COMMENT, INLINE_COMMENT, CODE };

//you are butifull, you are butifull
dword ShowSource()
{
	dword new_buf, new_buf_start, i;
	int mode;

	if (souce_mode) return;
	souce_mode = true;
	new_buf = malloc(bufsize*5);
	new_buf_start = new_buf;
	header[strrchr(#header, '-')-2]=0;
	sprintf(new_buf,"<html><head><title>Source: %s</title><body><pre>",#header);
	new_buf += strlen(new_buf);
	for (i=bufpointer; i<bufpointer+bufsize; i++) 
	{
		switch (ESBYTE[i])
		{
			case '<':
				if (mode == COMMENT)
				{
					strcpy(new_buf, "&lt;");
					new_buf+=4;
					break;
				}
				if (ESBYTE[i+1]=='!') && (ESBYTE[i+2]=='-') && (ESBYTE[i+3]=='-')
				{
					strcpy(new_buf, "<font color=#bbb>&lt;");
					new_buf+=21;
					mode = COMMENT;
					break;
				}
				if (mode != COMMENT)
				{
					strcpy(new_buf, "<font color=#00f>&lt;");
					new_buf+=21;
					mode = TAG;
					break;
				}
				break;
			case '>':
				if (mode == OPTION_VALUE) //fix non-closed quote in TAG
				{
					strcpy(new_buf, "&quot;</font>");
					new_buf+=13;					
					mode = TAG;
					break;
				}
				if (mode == COMMENT) && (ESBYTE[i-1]=='-') && (ESBYTE[i-2]=='-')
				{
					strcpy(new_buf, "&gt;</font>");
					new_buf+=11;
					mode = TEXT;
					break;
				}
				if (mode == COMMENT) 
				{
					strcpy(new_buf, "&gt;");
					new_buf+=4;
					break;					
				}
				if (mode == TAG)
				{
					strcpy(new_buf, "&gt;</font>");
					new_buf+=11;
					mode = TEXT;
					break;
				}
				break;
			case '\"':
			case '\'':
				if (mode == TAG)
				{
					strcpy(new_buf, "<font color=#f0f>&#39;");
					new_buf+=22;
					mode = OPTION_VALUE;
					break;
				}
				if (mode == OPTION_VALUE)
				{
					strcpy(new_buf, "&#39;</font>");
					new_buf+=12;
					mode = TAG;
					break;
				}
			default:
				ESBYTE[new_buf] = ESBYTE[i];
				new_buf++;
		}
	}
	ESBYTE[new_buf] = 0;
	bufsize = new_buf - new_buf_start;
	free(bufpointer);
	bufpointer = new_buf_start;
}

/*
char* C_HL_keywords[] = {
    "switch", "if", "while", "for", "break", "continue", "return", "else",
    "union", "typedef", "static", "class", "case", "#include",
    "volatile", "register", "sizeof", "typedef", "union", "goto", "const", "auto",
    "#define", "#endif", "#error", "#ifdef", "#ifndef", "#undef", "#if", "#else", 
    "inline", 

    "int ", "dword ", "long ", "double ", "float ", "char ", "unsigned ", "signed ",
    "void ", "bool ", "enum ", "byte ", "word ", "struct ", "NULL", "loop", "stdcall ",
    ":void ", ":int ", ":bool ", ":dword ", NULL
};

dword ShowCodeSource()
{
	dword new_buf, new_buf_start, i;
	int mode = CODE;

	char spstr[64];
	dword keylen;
	dword keyn;
	dword keycolor;

	new_buf = malloc(bufsize*10);
	new_buf_start = new_buf;
	sprintf(new_buf,"<html><head><title>C/C++/C-- source: %s</title><body><pre>",#URL);
	new_buf += strlen(new_buf);
	for (i=bufpointer; i<bufpointer+bufsize; i++)
	{
		if ('<' == ESBYTE[i]) {
			strcpy(new_buf, "&lt;");
			new_buf+=4;
			continue;
		}
		if ('>' == ESBYTE[i]) {
			strcpy(new_buf, "&gt;");
			new_buf+=4;
			continue;
		}
		if (ESBYTE[i] >= '0') && (ESBYTE[i] <= '9') && (CODE == mode) {
			strcpy(new_buf, "<font color=#CF00FF>?</font>");
			ESBYTE[new_buf+20] = ESBYTE[i];
			new_buf+=28;
			if (ESBYTE[i+1] == 'x') {
				strcpy(new_buf, "<font color=#CF00FF>x</font>");
				new_buf+=28;
				i++;
			}
			continue;
		}

		if (CODE == mode) && ('\"' == ESBYTE[i]) {
			mode = TEXT;
			strcpy(new_buf, "<font color=#080>\"");
			new_buf+=18;
			continue;
		}
		if (TEXT == mode) && ('\"' == ESBYTE[i]) {
			mode = CODE;
			strcpy(new_buf, "\"</font>");
			new_buf+=8;
			continue;		
		}

		if (! strncmp(i, "//", 2) ) && (mode == CODE) {
			mode = INLINE_COMMENT;
			strcpy(new_buf, "<font color=#777>//");
			new_buf+=19;
			i++;
			continue;
		}

		if (INLINE_COMMENT == mode) {
			if (13 == ESBYTE[i]) {
				mode = CODE;
				strcpy(new_buf, "\13</font>");
				new_buf+=8;
				continue;				
			}
		}

		if (! strncmp(i, "/*", 2) ) {
			mode = COMMENT;
			strcpy(new_buf, "<font color=#665>/*");
			new_buf+=19;
			i++;
			continue;
		}
		if (! strncmp(i, "*/", 2) ) {
			mode = CODE;
			strcpy(new_buf, "*/</font>");
			new_buf+=9;
			i++;
			continue;
		}

		if (CODE == mode) for (keyn=0; C_HL_keywords[keyn]!=NULL; keyn++) 
		{
			keylen = strlen(C_HL_keywords[keyn]);
			if (! strncmp(i, C_HL_keywords[keyn], keylen) ) {

				if (keyn<31) keycolor="#f00"; else keycolor="#00f";
				sprintf(#spstr, "<font color=%s>%s</font>", keycolor, C_HL_keywords[keyn]);
				strcpy(new_buf, #spstr);

				new_buf += keylen + 24;
				i += keylen-1;
				goto _CONTINUE;
			}
		}
		ESBYTE[new_buf] = ESBYTE[i];
		new_buf++;
		_CONTINUE:		
	}
	ESBYTE[new_buf] = 0;
	bufsize = new_buf - new_buf_start;
	free(bufpointer);
	bufpointer = new_buf_start;
}
*/
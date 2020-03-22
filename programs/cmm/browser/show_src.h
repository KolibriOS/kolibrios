enum { TAG, OPTION_VALUE, TEXT, COMMENT, SCRIPT};

dword ShowSource()
{
	dword new_buf, new_buf_start, i, j;
	int opened_font=0;
	int mode = TEXT;

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
				if (mode == TEXT) && (!strncmp(i+1,"script", 6)) {
					mode = SCRIPT;
					strcpy(new_buf, "<font color=#00f>&lt;script</font><font color=#994500>"); opened_font++;
					new_buf+=54;
					i+=6;
					break;
				}
				if (mode == COMMENT) || (mode == SCRIPT) && (!strncmp(i+1,"/script>", 8)) {
					mode = TEXT;
					while (opened_font) {
						strcpy(new_buf, "</font>"); opened_font--;
						new_buf+=7;						
					}
					strcpy(new_buf, "<font color=#00f>&lt;/script&gt;</font>"); 
					new_buf+=39;
					i+=8;
					break;
				}
				if (ESBYTE[i+1]=='!') && (ESBYTE[i+2]=='-') && (ESBYTE[i+3]=='-') {
					mode = COMMENT;
					strcpy(new_buf, "<font color=#bbb>&lt;"); opened_font++;
					new_buf+=21;
					break;
				}
				if (mode == TEXT) {
					mode = TAG;
					strcpy(new_buf, "<font color=#00f>&lt;"); opened_font++;
					new_buf+=21;
					break;
				}
				if (mode == COMMENT) || (mode == SCRIPT) {
					strcpy(new_buf, "&lt;");
					new_buf+=4;
					break;
				}
				break;
			case '>':
				if (mode == OPTION_VALUE) { //fix non-closed quote in TAG
					mode = TEXT;
					while (opened_font) {
						strcpy(new_buf, "&quot;</font>"); opened_font--;
						new_buf+=13;						
					}
					break;
				}
				if (mode == COMMENT) && (ESBYTE[i-1]=='-') && (ESBYTE[i-2]=='-') {
					mode = TEXT;
					strcpy(new_buf, "&gt;</font>"); opened_font--;
					new_buf+=11;
					break;
				}
				if (mode == COMMENT) || (mode == SCRIPT) {
					strcpy(new_buf, "&gt;");
					new_buf+=4;
					break;					
				}
				if (mode == TAG) {
					mode = TEXT;
					strcpy(new_buf, "&gt;</font>"); opened_font--;
					new_buf+=11;
					break;
				}
				break;
			case '\"':
			case '\'':
				if (mode == TAG) {
					mode = OPTION_VALUE;
					strcpy(new_buf, "<font color=#F301F2>&#39;"); opened_font++;
					new_buf+=25;
					break;
				}
				if (mode == OPTION_VALUE) {
					mode = TAG;
					strcpy(new_buf, "&#39;</font>"); opened_font--;
					new_buf+=12;
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

enum { TAG, OPTION_VALUE, TEXT, COMMENT, INLINE_COMMENT, CODE };

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
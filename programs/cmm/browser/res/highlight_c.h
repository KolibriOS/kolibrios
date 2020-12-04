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

dword ShowCodeSource(dword _bufpointer, _bufsize)
{
	dword new_buf, new_buf_start, i;
	int mode = CODE;

	char spstr[64];
	dword keylen;
	dword keyn;
	dword keycolor;

	new_buf = malloc(_bufsize*10);
	new_buf_start = new_buf;
	sprintf(new_buf,"<html><head><title>%s</title><body><pre>",#current_path);
	new_buf += strlen(new_buf);
	for (i=_bufpointer; i<_bufpointer+_bufsize; i++)
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
			strcpy(new_buf, "<font color=#999>//");
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
			strcpy(new_buf, "<font color=#999>/*");
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
	LoadInternalPage(new_buf_start, new_buf - new_buf_start);
	free(new_buf_start);
}
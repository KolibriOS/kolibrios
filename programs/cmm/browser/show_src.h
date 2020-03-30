enum { TAG=1, VALUE, TEXT, COMMENT, SCRIPT};

int opened_font_counter=0;
int mode;

dword source_buf_end;

void SourceBufAdd(dword _mode, src)
{
	dword font_found_pointer, src_orig = src;

	if (_mode) mode = _mode;

	strcpy(source_buf_end, src);
	source_buf_end += strlen(source_buf_end);

	if (font_found_pointer = strstr(src, "</font>")) {
		opened_font_counter--;
		src = font_found_pointer+2;
	}

	src = src_orig;
	if (font_found_pointer = strstr(src, "<font ")) {
		opened_font_counter++;
		src = font_found_pointer+2;
	}
}

void CloseAllOpenedFonts(dword _mode)
{
	while (opened_font_counter) SourceBufAdd(_mode, "</font>");
}

dword ShowSource(dword _bufdata, _in_bufsize)
{
	dword i, j;
	bool activate_script_mode = false;
	dword source_buf_start;

	opened_font_counter=0;
	source_buf_start = malloc(_in_bufsize*5);
	source_buf_end = source_buf_start;

	SourceBufAdd(TEXT, "<html><head><title>View Source</title><body><pre>");

	for (i=_bufdata; i<_bufdata+_in_bufsize; i++) switch (ESBYTE[i])
	{
		case '<':
			if (!strncmp(i+1,"!--", 3)) SourceBufAdd(COMMENT, "<font color=#bbb>&lt;");
			else if (SCRIPT == mode) {
				if (!strncmp(i+1,"/script>", 8)) {
					CloseAllOpenedFonts(NULL);
					SourceBufAdd(TAG, "<font color=#00f>&lt;");
				}
				else SourceBufAdd(NULL, "&lt;");
			}
			else if (COMMENT == mode) {
				SourceBufAdd(NULL, "&lt;");
			}
			else if (TEXT == mode) {
				if (!strncmp(i+1,"script", 6)) activate_script_mode = true;
				SourceBufAdd(TAG, "<font color=#00f>&lt;");
			}
			break;
		case '>':
			if (TAG == mode) && (activate_script_mode) {
				activate_script_mode = false;
				SourceBufAdd(SCRIPT, "&gt;</font><font color=#994500>");
			}
			else if (VALUE == mode) CloseAllOpenedFonts(TEXT);
			else if (COMMENT == mode) && (!strncmp(i-2,"--", 2)) {
				SourceBufAdd(TEXT, "&gt;");
				CloseAllOpenedFonts(TEXT);
			}
			else if (COMMENT == mode) || (SCRIPT == mode) SourceBufAdd(NULL, "&gt;");
			else if (TAG == mode) SourceBufAdd(TEXT, "&gt;</font>");
			break;
		case '&':
			SourceBufAdd(NULL, "&amp;");
			break;
		case '\"':
		case '\'':
			if (TAG == mode) SourceBufAdd(VALUE, "<font color=#F301F2>&quot;");
			else if (VALUE == mode) SourceBufAdd(TAG, "&quot;</font>");
			else SourceBufAdd(NULL, "&quot;");
			break;
		default:
			ESBYTE[source_buf_end] = ESBYTE[i];
			source_buf_end++;
	}
	ESBYTE[source_buf_end] = 0;
	LoadInternalPage(source_buf_start, source_buf_end-source_buf_start);
	free(source_buf_start);
}

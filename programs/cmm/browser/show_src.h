enum { TAG, OPTION_VALUE, TEXT, COMMENT };

//you are butifull, you are butifull
dword ShowSource()
{
	dword new_buf, new_buf_start, i;
	byte ww, mode;

	if (souce_mode) return;
	souce_mode = true;
	new_buf_start = new_buf = malloc(bufsize*5);
	strcat(new_buf, "<pre>");
	for (i=bufpointer; i<bufpointer+bufsize; i++) 
	{
		ww = ESBYTE[i];
		new_buf++;
		switch (ww)
		{
			case '<':
				if (ESBYTE[i+1]=='!') && (ESBYTE[i+2]=='-') && (ESBYTE[i+3]=='-')
				{
					strcat(new_buf, "<font color=#ccc>&lt;");
					new_buf+=20;
					mode = COMMENT;
				}
				else
				{
					strcat(new_buf, "<font color=#00f>&lt;");
					new_buf+=20;
					mode = TAG;
				}
				break;
			case '>':
				if (mode == OPTION_VALUE) //fix non-closed quote in TAG
				{
					strcat(new_buf, "&quot;</font>");
					new_buf+=12;					
					mode = TAG;
				}
				if (mode == COMMENT) && (ESBYTE[i-1]=='-') && (ESBYTE[i-2]=='-')
				{
					strcat(new_buf, "&gt;</font>");
					new_buf+=10;
					mode = TEXT;
				}
				if (mode == TAG)
				{
					strcat(new_buf, "&gt;</font>");
					new_buf+=10;
					mode = TEXT;
				}
				break;
			case '\"':
			case '\'':
				if (mode == TAG)
				{
					strcat(new_buf, "<font color=#f0f>&quot;");
					new_buf+=22;
					mode = OPTION_VALUE;
					break;
				}
				if (mode == OPTION_VALUE)
				{
					strcat(new_buf, "&quot;</font>");
					new_buf+=12;
					mode = TAG;
					break;
				}
			default:
				chrcat(new_buf, ww);
		}
	}
	bufsize = new_buf;
	free(bufpointer);
	bufpointer = new_buf_start;
}
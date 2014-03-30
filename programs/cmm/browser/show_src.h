//you are butifull, you are butifull
dword ShowSource()
{
	dword new_buf, new_buf_start, i;
	byte ww, param, comment;

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
				}
				else
				{
					strcat(new_buf, "<font color=#00f>&lt;");
					new_buf+=20;					
				}
				break;
			case '>':
				if (!param) //fix non-closed quote
				{
					param = 1;
					strcat(new_buf, "&quot;</font>");
					new_buf+=12;					
				}
				if (ESBYTE[i-1]=='-') && (ESBYTE[i-2]=='-')
				{
					strcat(new_buf, "&gt;</font>");
					new_buf+=10;
				}
				else
				{
					strcat(new_buf, "&gt;</font>");
					new_buf+=10;					
				}
				break;
			case '\"':
			case '\'':
				if (param)
				{
					param = 0;
					strcat(new_buf, "<font color=#f0f>&quot;");
					new_buf+=22;
				}
				else
				{
					param = 1;
					strcat(new_buf, "&quot;</font>");
					new_buf+=12;
				}
				break;
			default:
				chrcat(new_buf, ww);
		}
	}
	bufsize = new_buf;
	free(bufpointer);
	bufpointer = new_buf_start;
}
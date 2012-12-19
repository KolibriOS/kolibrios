//convert text characters

dword iconv_lib = #a_iconv_lib;
char a_iconv_lib[19]="/sys/lib/iconv.obj\0";

dword iconv_open     = #aIconv_open;
dword iconv          = #aIconv;

dword  am5__ = 0x0;
dword  bm5__ = 0x0;

char aIconv_open[11] = "iconv_open\0";
char aIconv[6]       = "iconv\0";


dword ChangeCharset(dword from_chs, to_chs, conv_buf)
{
	dword cd, in_len, out_len, new_buf;	

	iconv_open stdcall (from_chs, to_chs); //CP866, CP1251, CP1252, KOI8-RU, UTF-8, ISO8859-5
	if (EAX==-1)
	{
		debug (from_chs);
		debug (to_chs);
		debug("iconv: wrong charset,\nuse only CP866, CP1251, CP1252, KOI8-RU, UTF-8, ISO8859-5");
		return 0; 
	}
	cd = EAX;

	in_len = out_len = strlen(conv_buf)+1;
	new_buf = mem_Alloc(in_len);
	iconv stdcall (cd, #conv_buf, #in_len, #new_buf, #out_len);
	cd = EAX;
	if (cd!=0)
	{
		debug("iconv: something is wrong with stdcall iconv()");
		debug(itoa(cd));
		debug("in_len");
		debug(itoa(in_len));
		debug("out_len");
		debug(itoa(out_len));
		new_buf = 0;
		return 0;
	}
	strcpy(conv_buf, new_buf);
	free(new_buf);
	return conv_buf;
}
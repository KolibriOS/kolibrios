//convert text characters
#ifndef INCLUDE_LIBICONV_H
#define INCLUDE_LIBICONV_H

#ifndef INCLUDE_KOLIBRI_H
#include "../lib/kolibri.h"
#endif

dword iconv_lib = #a_iconv_lib;
char a_iconv_lib[]="/sys/lib/iconv.obj";

dword iconv_open     = #aIconv_open;
dword iconv          = #aIconv;
$DD 2 dup 0

char aIconv_open[] = "iconv_open";
char aIconv[]       = "iconv";

char charsets[] = "UTF-8\0    KOI8-RU\0  CP1251\0   CP1252\0   ISO8859-5\0CP866";
enum { CH_UTF8, CH_KOI8, CH_CP1251, CH_CP1252, CH_ISO8859_5, CH_CP866, CH_NULL };

dword ChangeCharset(dword from_chs, to_chs, conv_buf)
{
	dword cd, in_len, out_len, new_buf;	

	from_chs = from_chs*10+#charsets;

	debug("iconv: from_chs = "); debugln(from_chs);
	debug("iconv: to_chs = "); debugln(to_chs);

	iconv_open stdcall (from_chs, to_chs); //CP866, CP1251, CP1252, KOI8-RU, UTF-8, ISO8859-5
	if (EAX==-1) {
		debugln("iconv: unsupported charset");
		return 0; 
	}
	cd = EAX;

	in_len = out_len = strlen(conv_buf)+1;
	new_buf = mem_Alloc(in_len);
	iconv stdcall (cd, #conv_buf, #in_len, #new_buf, #out_len);
	cd = EAX;
	if (cd!=0)
	{
		debugval("iconv: something is wrong with stdcall iconv()", cd);
		debugval("in_len", in_len);
		debugval("out_len", out_len);
		new_buf = free(new_buf);
		return conv_buf;
	}
	strcpy(conv_buf, new_buf);
	free(new_buf);
	return conv_buf;
}

#endif
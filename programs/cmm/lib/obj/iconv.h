//convert text characters
#ifndef INCLUDE_LIBICONV_H
#define INCLUDE_LIBICONV_H

#ifndef INCLUDE_KOLIBRI_H
#include "../lib/kolibri.h"
#endif

#ifndef INCLUDE_ENCODING_H
#include "../lib/encoding.h"
#endif

dword iconv_lib = #a_iconv_lib;
char a_iconv_lib[]="/sys/lib/iconv.obj";

dword iconv_open     = #aIconv_open;
dword iconv          = #aIconv;
$DD 2 dup 0

char aIconv_open[] = "iconv_open";
char aIconv[]       = "iconv";

char charsets[] = "UTF-8\0    KOI8-RU\0  CP1251\0   CP1252\0   ISO8859-5\0CP866\0    AUTO";
enum { CH_UTF8, CH_KOI8, CH_CP1251, CH_CP1252, CH_ISO8859_5, CH_CP866, CH_AUTO };

dword ChangeCharset(dword from_chs, to_chs, conv_buf)
{
	dword cd, in_len, out_len, new_buf;	

	iconv_open stdcall (from_chs*10+#charsets, to_chs*10+#charsets);
	if (EAX==-1) {
		debugln("iconv: unsupported charset");
		return 0; 
	}
	cd = EAX;

	in_len = strlen(conv_buf)+1;
	out_len = in_len * 2;
	new_buf = mem_Alloc(out_len);
	iconv stdcall (cd, #conv_buf, #in_len, #new_buf, #out_len);
	if (EAX!=0)
	{
		cd = EAX;
		debugval("iconv failed", cd);
		if (from_chs == CH_UTF8) && (to_chs == CH_CP866) {
			utf8rutodos(conv_buf);
		}
		if (from_chs == CH_CP1251) && (to_chs == CH_CP866) {
			wintodos(conv_buf);
		}
	} else {
		strcpy(conv_buf, new_buf);
	}
	free(new_buf);		
	return conv_buf;
}

#endif
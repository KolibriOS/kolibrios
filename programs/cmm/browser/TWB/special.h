char *unicode_symbols[]={
"quot","\"",
"amp", "&",
"lt",  "<",
"gt",  ">",
"#183","\31",   "middot", "\31", 
"#149","-",
"#151","-",
"#160"," ",     "nbsp", "\t",   "emsp", " ",
"#169","(c)",   "copy", "(c)",
"#171","<<",    "laquo","<<",
"#174","(r)",   "reg",  "(r)",
"#187",">>",    "raquo",">>",
"hellip", "...",

"trade", "[TM]",
"bdquo", ",,",

"minus", "-",
"ndash", "-",
"mdash", "-", //--

"rsquo", "'", "apos", "'",
"sect", "#",

"ensp",    " ",
"emsp13",  " ",
"emsp14",  " ",
"numsp",   " ",
"puncsp",  " ",
"thinsp",  " ",

"#1028", "\242",
"#1030", "I",
"#1031", "\244",

"#8211", "-",
"#8212", "-",
"#8217", "'",
"#8220", "\"",
"#8222", "\"", "ldquo", "\"",
"#8221", "\"", "rdquo", "\"",
"#8470", "N",
"#8722", "-",
"#9642", "-", //square in the middle of the line
"#9658", ">",
"#9660", "v",
"#10094", "<",
"#10095", ">",
"#65122", "+",

"#8594", "->",

"uarr",  "^",
"darr",  "v",
"rarr",  "->",
"larr",  "<-", 

"bull",  "\31",
"percnt","%",

"#xfeff", "",

"times", "x",
"lowast","*",

0}; 


unsigned char unicode_chars[] = "€‚ƒ„…†‡ˆ‰Š‹ŒŽ‘’“”•–—˜™š›œžŸ ¡¢£¤¥¦§¨©ª«¬­®¯àáâãäåæçèéêëìíîïðñh£\243i\105\244\0";

bool GetUnicodeSymbol(dword _line, line_size, bufpos, buf_end)
{
	int i;
	int code;
	char special_code[10];
	bool white_end = false;
	dword bufstart = bufpos;

	for (i=0; i<9; i++, bufpos++)
	{
		if (__isWhite(ESBYTE[bufpos])) {bufpos--; break;}
		if (ESBYTE[bufpos] == ';') || (bufpos >= buf_end) break;
		if (!strchr("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789#", ESBYTE[bufpos])) {bufpos--; break;}
		special_code[i] = ESBYTE[bufpos];
	}
	special_code[i] = '\0';
	
	for (i=0; unicode_symbols[i]!=0; i+=2;) 
	{
		if (!strcmp(#special_code, unicode_symbols[i]))
		{
			strncat(_line, unicode_symbols[i+1], line_size);
			return bufpos;
		}
	}

	if (special_code[0]=='#') 
	{
		code = atoi(#special_code + 1);
		if (code>=0) && (code<=255)	{
			chrncat(_line, code, line_size); //NOT ALL ASCII CODES IN KOLIBRI ARE COMPATABLE WITH STANDARDS
			return bufpos;
		}
		if (code>=1040) && (code<=1040+72) {
			chrncat(_line, unicode_chars[code-1040], line_size);
			return bufpos;
		}
	}

	chrncat(_line, '&', line_size);
	return bufstart;
}

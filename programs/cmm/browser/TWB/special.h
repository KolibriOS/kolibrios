char *unicode_symbols[]={
"quot","\"",
"amp", "&",
"lt",  "<",
"gt",  ">",
"#183","\31",   "middot", "\31", 
"#149","-",
"#151","-",
"#160"," ",     "nbsp", " ",
"#169","(c)",   "copy", "(c)",
"#171","<<",    "laquo","<<",
"#174","(r)",   "reg",  "(r)",
"#187",">>",    "raquo",">>",

"trade", "[TM]",
"bdquo", ",,",

"minus", "-",
"ndash", "-",
"mdash", "-", //--

"rsquo", "'",
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
"#65122", "+",

"#8594", "->",

"uarr",  "\24",
"darr",  "\25",
"rarr",  "\26",
"larr",  "\27", 

"bull",  "\31",
"percnt","%",

"#xfeff", "",

"times", "x",
"lowast","*",

0}; 


unsigned char unicode_chars[] = "€‚ƒ„…†‡ˆ‰Š‹ŒŽ‘’“”•–—˜™š›œžŸ ¡¢£¤¥¦§¨©ª«¬­®¯àáâãäåæçèéêëìíîïðñh£\243i\105\244\0";

bool GetUnicodeSymbol(dword _line, in_tag, size)
{
	int j;
	int code;
	
	for (j=0; unicode_symbols[j]!=0; j+=2;) 
	{
		if (!strcmp(in_tag, unicode_symbols[j]))
		{
			strncat(_line, unicode_symbols[j+1], size);
			return true;
		}
	}

	if (ESBYTE[in_tag]=='#') 
	{
		code = atoi(in_tag + 1);
		if (code>=0) && (code<=255)	{
			chrncat(_line, code, size); //NOT ALL ASCII CODES IN KOLIBRI ARE COMPATABLE WITH STANDARDS
			return true;
		}
		if (code>=1040) && (code<=1040+72) {
			chrncat(_line, unicode_chars[code-1040], size);
			return true;
		}
	}

	return false;
}

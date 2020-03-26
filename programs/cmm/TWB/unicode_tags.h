char *unicode_symbols[]={
"#32", " ",      
"#34", "\"",     "quot","\"",
"#38", "&",      "amp", "&",
"#39", "'",
"#039","'",
"#60", "<",      "lt",  "<",
"#62", ">",      "gt",  ">",
"#91", "[",
"#93", "]",
"#96", "'",
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

"#1028", "\242",
"#1030", "I",
"#1031", "\244",

"#8211", "-",
"#8217", "'",
"#8222", "\"", "ldquo", "\"",
"#8221", "\"", "rdquo", "\"",
"#8470", "N",
"#8722", "-",
"#9642", "-", //square in the middle of the line

"uarr",  "\24",
"darr",  "\25",
"rarr",  "\26",
"larr",  "\27", 

"bull",  "\31",
"percnt","%",

"#xfeff", "",

0}; 


unsigned char unicode_chars[] = "€‚ƒ„…†‡ˆ‰Š‹ŒŽ‘’“”•–—˜™š›œžŸ ¡¢£¤¥¦§¨©ª«¬­®¯àáâãäåæçèéêëìíîïðñh£\243i\105\244\0";

bool GetUnicodeSymbol(dword in_tag)
{
	int j, specia1040;
	
	for (j=0; unicode_symbols[j]!=0; j+=2;) 
	{
		if (!strcmp(in_tag, unicode_symbols[j]))
		{
			strcat(#line, unicode_symbols[j+1]);
			return true;
		}
	}

	specia1040 = atoi(in_tag + 1) - 1040;
	
	if (ESBYTE[in_tag+1] == '1') && (specia1040>=0) 
	&& (specia1040<=72) && (strlen(in_tag) == 5)
	{
		if (strlen(#line)<sizeof(line)-2) {
			/*
			j = strlen(#line);
			line[j] = unicode_chars[specia1040];
			line[j+1] = EOS;
			*/
			chrcat(#line, unicode_chars[specia1040]);
		}
		return true;
	}

	return false;
}

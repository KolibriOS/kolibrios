char *unicode_tags[]={
"nbsp",  " ",
"#38",   " ",
"#160",  " ",

"copy",  "(c)",
"#169",  "(c)",

"trade", "[TM]",

"reg",   "(r)",
"#174",  "(r)",

"bdquo", ",,",

"amp",   "&",
"#38",   "&",

"lt",    "<",
"#60",   "<",

"gt",    ">",
"#62",   ">",

"minus", "-",
"ndash", "-",
"mdash", "-", //--
"#8722", "-",
"#8211", "-",
"#151",  "-",
"#149",  "-",
"#9642", "-", //square in the middle of the line

"rsquo", "'",
"#39",   "'",
"#039",  "'",
"#96",   "'",
"#8217", "'",

"quot",  "\"",
"#34",   "\"",
"ldquo", "\"",
"rdquo", "\"",
"#8222", "\"",
"#8221", "\"",

"laquo", "<<",
"#171",  "<<",
"raquo", ">>",
"#187",  ">>",

"uarr",  "\24",
"darr",  "\25",
"rarr",  "\26",
"larr",  "\27", 

"#1028", "\242",
"#1030", "I",
"#1031", "\244",

"#8470", "N",
"bull",  "\31", //âîîáùå çäåñü òî÷êà
"percnt","%",

0}; 


unsigned char unicode_chars[] = "€‚ƒ„…†‡ˆ‰Š‹ŒŽ‘’“”•–—˜™š›œžŸ ¡¢£¤¥¦§¨©ª«¬­®¯àáâãäåæçèéêëìíîïðñh£\243i\105\244\0";

bool GetUnicodeSymbol(dword in_tag)
{
	int j, specia1040;
	
	for (j=0; unicode_tags[j]!=0; j+=2;) 
	{
		if (!strcmp(in_tag, unicode_tags[j]))
		{
			strcat(#line, unicode_tags[j+1]);
			return true;
		}
	}

	specia1040 = atoi(in_tag + 1) - 1040;
	
	if (tag[1] == '1') && (specia1040>=0) 
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

// unicode conversion for special characters
char *unicode_tags[]={
"nbsp",  " ",
"#38",   " ",
"#160",  " ",

"ntilde", "n", // spanish n special ñ
"#224",   "à", // spanish a with grove accent 'à'
"#241",  "n", // spanish symbol

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

"rsquo", "'",
"#39",   "'",
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
"bull",  "-", //âîîáùå çäåñü òî÷êà
"percnt","%",

0};

// function to be called to fix the special symbols
void fixSpecial(dword text) {
	byte ch;
	int i , j, z;
	dword aux;
	dword text2 = text;
	loop () {
		ch = ESBYTE[text];
		if (!ch) return;
		if (ch=='&') {
			i = 0;
			j = 0;
			text++;
			while ( i < 7)  {
				ch = ESBYTE[text];
				if (ch == ';') {
					z = get_symbol(#str);
					if (z == -1) { // not found
						ch = ' ';
					}
					else { // tag found
						aux = unicode_tags[z];
						strtrim(aux);
						ch = ESBYTE[aux];
						// copy the special symbol found
						while (ch) {
							ESBYTE[text2] = ch;
							aux++;
							text2++;
							ch = ESBYTE[aux];
						}
						ch = ESBYTE[text2];
					}
					// clean the old symbol
					while (ch != ';') {
						ESBYTE[text2] = ' ';// should be (char) 0;
						text2++;
						ch = ESBYTE[text2];
					}
					ESBYTE[text2] = ' '; // should be '' or char 0
					break;
				}
				str[i] = ch;
				if (!ch) return;
				text++;
				i++;
			}
			for (i=0; i < 7; i++) str[i] = 0; // clean str
		}
		text++;
		text2++;
	}
}



// function to look for the conversion of special characters
// if not found--> return -1
int get_symbol(char *str2) {
	int i,j;
	//debugln(#str2);
	for (i=0; unicode_tags[i]!=0; i+=2) {
		if (strcmp(str2, unicode_tags[i]) == 0) {
			return (i+1);
		}
	}	
	return -1;
}




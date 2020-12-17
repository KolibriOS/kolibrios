
struct _tag
{
	char name[32];
	char prior[32];
	char params[6000];
	bool opened;
	collection attributes;
	collection values;
	dword value;
	bool is();
	bool parse_tag();
	void debug_tag();
	bool get_next_param();
	dword get_value_of();
} tag=0;

bool _tag::is(dword _text) 
{ 
	if ( !strcmp(#name, _text) ) {
		return true;
	} else {
		return false; 
	}
}

bool _tag::parse_tag(dword _bufpos, bufend)
{
	bool retok = true;
	dword bufpos = ESDWORD[_bufpos];

	dword closepos;
	dword whitepos;

	if (name) strcpy(#prior, #name); else prior = '\0';
	name = '\0';
	params = '\0';
	attributes.drop();
	values.drop();		

	if (!strncmp(bufpos,"!--",3))
	{
		bufpos+=3;
		//STRSTR
		while (strncmp(bufpos,"-->",3)!=0) && (bufpos < bufend)
		{
			bufpos++;
		}
		bufpos+=2;
		goto _RET;
	}

	if (ESBYTE[bufpos] == '/') {
		opened = false;
		bufpos++;
	} else {
		opened = true;
	}

	closepos = strchr(bufpos, '>');
	whitepos = strchrw(bufpos, bufend-bufpos);
	if (whitepos > closepos) {
		//no param
		strncpy(#name, bufpos, math.min(closepos - bufpos, sizeof(tag.name)));
		debug_tag();
		params = '\0';
		bufpos = closepos;
	} else {
		//we have param
		strncpy(#name, bufpos, math.min(whitepos - bufpos, sizeof(tag.name)));
		strncpy(#params, whitepos, math.min(closepos - whitepos, sizeof(tag.params)));
		debug_tag();
		bufpos = closepos;
		while (get_next_param());
	}

	if (!name) {
		retok = false;
		goto _RET;
	}

	strlwr(#name);

	// ignore text inside the next tags
	if (is("script")) || (is("style")) || (is("binary")) || (is("select")) { 
		strcpy(#prior, #name);
		sprintf(#name, "</%s>", #prior);
		if (strstri(bufpos, #name)) bufpos = EAX-1;
		retok = false;
		goto _RET;
	}

	if (name[strlen(#name)-1]=='/') name[strlen(#name)-1]=NULL; //for <br/>

_RET:
	ESDWORD[_bufpos] = bufpos;
	return retok;
}

void _tag::debug_tag()
{
	if (debug_mode) { 
		debugch('<'); 
		if (!opened) debugch('/');
		debug(#name);
		debugln(">");
		if (params) {
			debug("params: "); 
			debugln(#params+1);
		}
	}
}

bool _tag::get_next_param()
{
	byte  quotes = NULL;
	int   i;
	unsigned char  val[6000];
	unsigned char attr[6000];

	if (!params) return false;
	
	i = strlen(#params) - 1;
	if (params[i] == '/') i--;
	while (i>0) && (__isWhite(params[i])) i--;

	if (params[i] == '"') || (params[i] == '\'')
	{
		//remove quotes
		quotes = params[i];
		params[i] = '\0';
		i--;

		//find VAL start and copy
		i = strrchr(#params, quotes);
		strlcpy(#val, #params + i, sizeof(val)-1);
		params[i] = '\0'; 
		i--;

		//find ATTR end
		while (i > 0) && (params[i] != '=') i--;
		params[i+1] = '\0';
	}
	else
	{
		//find VAL start and copy
		while (i > 0) && (params[i] != '=') i--;
		i++;
		strlcpy(#val, #params + i, sizeof(val)-1);

		//already have ATTR end
	}

	//find ATTR start and copy
	while (i>0) && (!__isWhite(params[i])) i--;
	strlcpy(#attr, #params + i + 1, sizeof(attr)-1);
	params[i] = '\0';
 
	//fix case: src=./images/KolibriOS_logo2.jpg?sid=e8ece8b38b
	i = strchr(#attr,'=');
	if (!quotes) && (i) {
		strlcpy(#val, i+1, sizeof(val)-1);
		ESBYTE[i+1] = '\0';
	}
	strlwr(#attr);
	strrtrim(#val);

	attributes.add(#attr);
	values.add(#val);

	if (debug_mode) {
		debug("atr: "); debugln(#attr);
		debug("val: "); debugln(#val);
		debugch('\n');
	}

	return true;
}

dword _tag::get_value_of(dword _attr_name)
{
	int pos = attributes.get_pos_by_name(_attr_name);
	if (pos == -1) {
		value = 0;
	} else {
		value = values.get(pos);
	}
	return value;
}


struct _tag
{
	char name[32];
	char prior[32];
	bool opened;
	collection attributes;
	collection values;
	dword value;
	bool is();
	bool parse();
	void debug_tag();
	dword get_next_param();
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

bool _tag::parse(dword _bufpos, bufend)
{
	bool retok = true;
	dword bufpos = ESDWORD[_bufpos];
	dword params, paramsend;

	dword closepos;
	dword whitepos;

	if (name) strcpy(#prior, #name); else prior = '\0';
	name = '\0';
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
		retok = false;
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

	if (debug_mode) {
		if (!closepos) debugln("null closepos");
		if (!whitepos) debugln("null whitepos");
	}

	if (!whitepos) || (whitepos > closepos) {
		//no param
		strncpy(#name, bufpos, math.min(closepos - bufpos, sizeof(tag.name)));
		debug_tag();
		bufpos = closepos;
	} else {
		//we have param
		strncpy(#name, bufpos, math.min(whitepos - bufpos, sizeof(tag.name)));
		debug_tag();
		bufpos = closepos;

		params = malloc(closepos - whitepos + 1);
		strncpy(params, whitepos, closepos - whitepos);
		if (debug_mode) { debug("params: "); debugln(params+1); }
		paramsend = params + closepos - whitepos;
		while (paramsend = get_next_param(params, paramsend-1));
		free(params);
	}

	if (name) {
		strlwr(#name);
		// ignore text inside the next tags
		if (is("script")) || (is("style")) || (is("binary")) || (is("select")) { 
			strcpy(#prior, #name);
			sprintf(#name, "</%s>", #prior);
			if (strstri(bufpos, #name)) bufpos = EAX-1;
			retok = false;
		} else {
			if (name[strlen(#name)-1]=='/') name[strlen(#name)-1]=NULL; //for <br/>
		}
	} else {
		retok = false;
	}

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
	}
}

dword _tag::get_next_param(dword ps, pe)
{
	// "ps" - param start
	// "pe" - param end
	// "q"  - quote char
	char q = NULL;
	dword fixeq;
	unsigned char  val[6000];
	unsigned char attr[6000];
	
	if (ESBYTE[pe] == '/') pe--;
	while (pe>ps) && (__isWhite(ESBYTE[pe])) pe--;

	if (ESBYTE[pe] == '"') || (ESBYTE[pe] == '\'')
	{
		//remove quote
		q = ESBYTE[pe];
		ESBYTE[pe] = '\0';
		pe--;

		//find VAL start and copy
		pe = strrchr(ps, q) + ps;
		strlcpy(#val, pe, sizeof(val)-1);
		ESBYTE[pe] = '\0'; 
		pe--;

		//find ATTR end
		while (pe > ps) && (ESBYTE[pe] != '=') pe--;
		ESBYTE[pe+1] = '\0';
	}
	else
	{
		//find VAL start and copy
		while (pe > ps) && (ESBYTE[pe] != '=') pe--;
		pe++;
		strlcpy(#val, pe, sizeof(val)-1);
		//already have ATTR end
	}

	//find ATTR start and copy
	while (pe>ps) && (!__isWhite(ESBYTE[pe])) pe--;
	strlcpy(#attr, pe + 1, sizeof(attr)-1);
	ESBYTE[pe] = '\0';
 
	//fix case: src=./images/KolibriOS_logo2.jpg?sid=e8ece8b38b
	fixeq = strchr(#attr,'=');
	if (!q) && (fixeq) {
		strlcpy(#val, fixeq+1, sizeof(val)-1);
		ESBYTE[fixeq+1] = '\0';
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

	if (pe==ps) return NULL;
	return pe;
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

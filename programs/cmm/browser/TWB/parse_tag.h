/*
struct _DOM {
	collection tag;
	collection tagparam;
	collection_int parent;
} dom;
*/

struct _tag
{
	char name[32];
	char prior[32];
	bool opened;
	collection attributes;
	collection values;
	dword value;
	dword number;
	bool is();
	bool parse();
	dword get_next_param();
	dword get_value_of();
	signed get_number_of();
} tag=0;

bool _tag::is(dword _text) 
{ 
	return streq(#name, _text);
}

bool _tag::parse(dword _bufpos, bufend)
{
	bool retok = true;
	dword bufpos = ESDWORD[_bufpos];
	dword params, paramsend;

	dword closepos;
	dword whitepos;
	dword openpos;

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

	if (!whitepos) || (whitepos > closepos) {
		//no param
		strncpy(#name, bufpos, math.min(closepos - bufpos, sizeof(tag.name)));
		bufpos = closepos;
	} else {
		//we have param
		while (chrlnum(whitepos, '\"', closepos - whitepos)%2) { //alt="Next>>"
			if (!openpos = strchr(closepos+1, '<')) break;
			if (openpos < strchr(closepos+1, '>')) break;
			if (!closepos = EAX) {closepos = bufend;break;}
		}
		strncpy(#name, bufpos, math.min(whitepos - bufpos, sizeof(tag.name)));
		bufpos = closepos;

		params = malloc(closepos - whitepos + 1);
		strncpy(params, whitepos, closepos - whitepos);
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

dword _tag::get_next_param(dword ps, pe)
{
	// "ps" - param start
	// "pe" - param end
	// "q"  - quote char
	char q = NULL;
	dword fixeq;
	dword val;
	dword attr;
	
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
		val = pe;
		if (pe>ps) pe--;
		ESBYTE[pe] = '\0'; 

		//find ATTR end
		while (pe > ps) && (ESBYTE[pe] != '=') pe--;
		ESBYTE[pe] = '\0';
	}
	else
	{
		//find VAL start and copy
		while (pe > ps) && (ESBYTE[pe] != '=') pe--;
		val = pe+1;
		ESBYTE[pe] = '\0';
		//already have ATTR end
	}

	//find ATTR start and copy
	while (pe>ps) && (!__isWhite(ESBYTE[pe])) pe--;
	attr = pe + 1;
	ESBYTE[pe] = '\0';

	// Fix case: src=./images/logo?sid=e8ece8b38b
	// Exchange '=' and '\0' position.
	// attr: src=./images/logo?sid   =>   src
	// val:  e8ece8b38b              =>   ./images/logo?sid=e8ece8b38b
	fixeq = strchr(attr,'=');
	if (!q) && (fixeq) {
		ESBYTE[val-1] >< ESBYTE[fixeq];
		val = fixeq+1;
	}
	strlwr(attr);
	strrtrim(val);

	attributes.add(attr);
	values.add(val);

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

signed _tag::get_number_of(dword _attr_name)
{
	if (get_value_of(_attr_name)) {
		number = atoi(value);
	} else {
		number = 0;
	}
	return number;
}

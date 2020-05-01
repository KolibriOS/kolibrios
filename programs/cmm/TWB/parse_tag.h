
struct _tag
{
	char name[32];
	char prior[32];
	char params[5000];
	bool opened;
	collection attributes;
	collection values;
	bool is();
	bool reset();
	bool parse_params();
	bool get_next_param();
	dword get_value_of();
} tag;

bool _tag::is(dword _text) 
{ 
	if ( !strcmp(#tag.name, _text) ) {
		return true;
	} else {
		return false; 
	}
}

bool _tag::reset()
{
	if (!name) return false;
	strcpy(#prior, #name);
	name = NULL;
	opened = true;
	attributes.drop();
	values.drop();
	return true;
}

bool _tag::parse_params()
{
	bool result = false;
	if (!name) return false;
	if (debug_mode) {
		debug("\n\ntag: "); debugln(#name);
		debug("params: "); debugln(#params);
		debugln(" ");
	}
	while (get_next_param()) {
		result = true;
		if (debug_mode) {
			debug("attribute: "); debugln(attributes.get(attributes.count-1));
			debug("value: "); debugln(values.get(values.count-1));
			debugln(" ");
		}
	};
	return result;
}

bool _tag::get_next_param()
{
	byte  quotes = NULL;
	int   i;
	unsigned char  val[4000];
	unsigned char attr[4000];

	if (!params) return false;
	
	i = strlen(#params) - 1;
	if (params[i] == '/') i--;
	while (i>0) && (__isWhite(params[i])) i--;

	if (params[i] == '"') || (params[i] == '\'')
	{
		//remove quotes
		quotes = params[i];
		params[i] = EOS;
		i--;

		//find VAL start and copy
		i = strrchr(#params, quotes);
		strlcpy(#val, #params + i, sizeof(val)-1);
		params[i] = EOS; 
		i--;

		//find ATTR end
		while (i > 0) && (params[i] != '=') i--;
		params[i+1] = EOS;
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
	strlwr(#attr);
	params[i] = '\0';
 
	//fix case: src=./images/KolibriOS_logo2.jpg?sid=e8ece8b38b
	i = strchr(#attr,'=');
	if (!quotes) && (i) {
		strlcpy(#val, i+1, sizeof(val)-1);
		ESBYTE[i+1] = '\0';
	}

	attributes.add(#attr);
	values.add(#val);

	return true;
}

dword _tag::get_value_of(dword _attr_name)
{
	int pos = attributes.get_pos_by_name(_attr_name);
	if (pos == -1) {
		return 0;
	} else {
		return values.get(pos);
	}
}

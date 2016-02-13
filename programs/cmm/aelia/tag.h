struct _tag {
	dword start;
	dword name;
	dword param[10];
	dword value[10];
	void parce();
	int nameis();
	void clear();
};

void _tag::parce()
{
	dword o = name = start;
	while (ESBYTE[o]!=' ') && (ESBYTE[o]) o++; //searching for a space after tag name
	ESBYTE[o] = '\0';
	strlwr(name);
}

int _tag::nameis(dword _in_tag_name)
{
	if (name) && (strcmp(_in_tag_name, name)==0) return true;
	return false;
}

void _tag::clear() 
{
	start=name=0;
}


/*
unsigned int GetNextParam()
{
	byte	kavichki=0;
	int		i = strlen(#tagparam) - 1;
	
	if (!tagparam) return 0;
	
	WHILE((i > 0) && ((tagparam[i] == '"') || (tagparam[i] == ' ') || (tagparam[i] == '\'') || (tagparam[i] == '/')))
	{
		IF (tagparam[i] == '"') || (tagparam[i] == '\'') kavichki=tagparam[i];
		tagparam[i] = 0x00;
		i--;
	}

	if (kavichki)
	{
		i=strrchr(#tagparam, kavichki);
		strlcpy(#val, #tagparam + i, sizeof(val));
	}
	else
	{
		WHILE((i > 0) && (tagparam[i] <>'=')) i--; //i=strrchr(#tagparam, '=')+1;
		i++;
		strlcpy(#val, #tagparam + i, sizeof(val));

		WHILE (val[0] == ' ') strcpy(#val, #val+1);
	}
	tagparam[i] = 0x00;

	FOR ( ; ((tagparam[i] <>' ') && (i > 0); i--)
	{
		IF (tagparam[i] == '=') //äåðçêàÿ çàãëóøêà
			tagparam[i + 1] = 0x00;
	}
	strlcpy(#attr, #tagparam + i + 1, sizeof(attr));
	tagparam[i] = 0x00;
	strlwr(#attr);
	return 1;
}
*/

//
//   STYLE
//

struct _style {
	bool b, u, i, s;
	bool h1, h2, h3, h4, h5, h6;
	bool a;
	bool pre;
	bool ignore;
	dword color;
	void clear();
} style;

void _style::clear()
{
	b=u=i=s=0;
	h1=h2=h3=h4=h5=h6=0;
	a=0;
	pre=0;
	ignore=0;
	color=0;
}


//
//   TEXT
//

struct _text {
	dword start;
	int x, y;
};
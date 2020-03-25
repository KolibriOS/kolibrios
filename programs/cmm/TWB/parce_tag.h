bool GetNextParam()
{
	byte  quotes = NULL;
	int   i;
	
	if (!tagparam) return false;

	if (debug_mode) {
		debug("tag: "); debugln(#tag);
		debug("tagparam: "); debugln(#tagparam);
	}
	
	i = strlen(#tagparam) - 1;

	if (tagparam[i] == '/') i--;

	while (i>0) && (__isWhite(tagparam[i])) i--;

	if (tagparam[i] == '"') || (tagparam[i] == '\'')
	{
		//find VAL end
		quotes = tagparam[i];
		tagparam[i] = '\0'; i--;

		//find VAL start and copy
		i = strrchr(#tagparam, quotes);
		strlcpy(#val, #tagparam + i, sizeof(val)-1);
		tagparam[i] = '\0'; i--;

		//find ATTR end
		while (i > 0) && (tagparam[i] != '=') i--;
		tagparam[i+1] = '\0';
	}
	else
	{
		//find VAL end
		//already have

		//find VAL start and copy
		while (i > 0) && (tagparam[i] != '=') i--;
		i++;
		strlcpy(#val, #tagparam + i, sizeof(val)-1);
		// tagparam[i] = '\0';

		//find ATTR end
		//already have
	}

	//find ATTR start and copy
	while (i>0) && (!__isWhite(tagparam[i])) i--;
	strlcpy(#attr, #tagparam + i + 1, sizeof(attr)-1);
	tagparam[i] = '\0';
 
	//fix case: src=./images/KolibriOS_logo2.jpg?sid=e8ece8b38b
	i = strchr(#attr,'=');
	if (!quotes) && (i) {
		strlcpy(#val, i+1, sizeof(val)-1);
		ESBYTE[i+1] = '\0';
	}

	strlwr(#attr);

	if (debug_mode) {
		debug("val: "); debugln(#val);
		debug("attr: "); debugln(#attr);
		debugln(" ");
	}

	return true;
}



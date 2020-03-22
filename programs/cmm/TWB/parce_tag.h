bool GetNextParam()
{
	byte  quotes = NULL;
	int   i;
	
	if (!tagparam) return false;

	if (debug_mode) {
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
		strlcpy(#val, #tagparam + i, sizeof(val));
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
		strlcpy(#val, #tagparam + i, sizeof(val));
		tagparam[i] = '\0';

		//find ATTR end
		//already have
	}

	//find ATTR start and copy
	while (i>0) && (!__isWhite(tagparam[i])) i--;
	strlcpy(#attr, #tagparam + i + 1, sizeof(attr));
	tagparam[i] = '\0';

	strlwr(#attr);

	if (debug_mode) {
		if (quotes) {
			debug("quote: "); debugch(quotes); debugln(" ");
		}
		else {
			debugln("unquoted text");
		}
		sprintf(#param, "val: %s\nattr: %s\n\n", #val, #attr);
		debug(#param);		
	}

	return true;
}



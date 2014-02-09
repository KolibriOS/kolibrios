unsigned int GetNextParam()
{
	byte	kavichki = false;
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
		strlcpy(#options, #tagparam + i, sizeof(options));
	}
	else
	{
		WHILE((i > 0) && (tagparam[i] <>'=')) i--; //i=strrchr(#tagparam, '=')+1;
		i++;
		strlcpy(#options, #tagparam + i, sizeof(options));

		WHILE (options[0] == ' ') strcpy(#options, #options+1);
	}
	tagparam[i] = 0x00;

	FOR ( ; ((tagparam[i] <>' ') && (i > 0); i--)
	{
		IF (tagparam[i] == '=') //дерзкая заглушка
			tagparam[i + 1] = 0x00;
	}
	strlcpy(#parametr, #tagparam + i + 1, sizeof(parametr));
	tagparam[i] = 0x00;
	strlwr(#parametr);
	return 1;
}
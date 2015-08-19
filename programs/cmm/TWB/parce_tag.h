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
		IF (tagparam[i] == '=') //дерзкая заглушка
			tagparam[i + 1] = 0x00;
	}
	strlcpy(#attr, #tagparam + i + 1, sizeof(attr));
	tagparam[i] = 0x00;
	strlwr(#attr);
	return 1;
}
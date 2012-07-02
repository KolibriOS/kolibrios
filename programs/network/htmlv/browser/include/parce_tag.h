void GetNextParam()
{
	byte	kavichki = false;
	int		i = strlen(#tagparam) - 1;
	
	WHILE((i > 0) && ((tagparam[i] == '"') || (tagparam[i] == ' ') || (tagparam[i] == '\'') || (tagparam[i] == '/')))
	{
		IF (tagparam[i] == '"') || (tagparam[i] == '\'') kavichki=tagparam[i];
		tagparam[i] = 0x00;
		i--;
	}

	IF (kavichki)
	{
		i=strrchr(#tagparam, kavichki);
		strcpy(#options, #tagparam + i);
	}
	ELSE
	{
		WHILE((i > 0) && (tagparam[i] <>'=')) i--; //i=strrchr(#tagparam, '=')+1;
		i++;
		
		strcpy(#options, #tagparam + i); //копируем опцию
		WHILE (options[0] == ' ') strcpy(#options, #options+1);
	}
	tagparam[i] = 0x00;

	FOR ( ; ((tagparam[i] <>' ') && (i > 0); i--)
	{
		IF (tagparam[i] == '=') //дерзкая заглушка
			tagparam[i + 1] = 0x00;
	}

	strcpy(#parametr, #tagparam + i + 1); //копируем параметр
	tagparam[i] = 0x00;
}

:dword arrayInit(dword len)
{
	dword alloc = 0;
	dword position = 0;
	alloc = malloc(len*4+MEMBUF);
	position = alloc;
	DSDWORD[position] = 0;
	position+=4;
	DSDWORD[position] = len;
	return alloc;
}
:dword ReallocArray(dword array,nMem)
{
	dword NEW = 0;
	dword OLD = 0;
	dword FLEN = 0;
	dword LEN = 0;
	
	//test2("relloc",0);
	
	OLD = array;
	FLEN = nMem*4+8;
	LEN = DSDWORD[array];
	
	NEW = arrayInit(nMem);
	
	array+=8;
	while(LEN)
	{
		A = DSDWORD[array];
		IF(A)
		{
			arraySet(#NEW,DSDWORD[A],DSDWORD[A+4]);
			free(A);
		}
		LEN--;
		array+=4;
	}
	free(OLD);

	return NEW;
}

:byte arraySet(dword array,key,value)
{
	dword NEW = 0;
	dword LEN = 0;
	dword FLEN = 0;
	dword POSITION = 0;
	dword VKEY = 0;
	dword FULL = 0;
	dword OST = 0;
	dword BEGIN = 0;
	dword gArray = 0;
	
	gArray = DSDWORD[array];
	
	IF(gArray)
	{
		LEN = DSDWORD[gArray];
		OST = DSDWORD[gArray+4];
	}
	ELSE
	{
		LEN = 0;
		OST = 8;
	}

	IF(LEN) 
	{
		IF(!(LEN%OST)) 
		{
			gArray = ReallocArray(gArray,OST<<1);
			DSDWORD[array] = gArray;
		}
	}
	ELSE 
	{
		gArray = arrayInit(OST);
		DSDWORD[array] = gArray;
	}
	
	BEGIN = gArray;
	
	gArray+=4;
	FLEN = DSDWORD[gArray];
	
	gArray+=4;
	POSITION = key%FLEN*4+gArray;
	
	FULL = FLEN*4+gArray;

	LOOP_COL2:
	VKEY = DSDWORD[POSITION];
	
	IF(!VKEY)
	{
		
		NEW = malloc(8);
		DSDWORD[NEW] = key;
		DSDWORD[NEW+4] = value;
		DSDWORD[POSITION] = NEW;
		DSDWORD[BEGIN] = DSDWORD[BEGIN]+1;
		return 1;
	}
	
	IF(DSDWORD[VKEY] == key)
	{
		DSDWORD[VKEY+4] = value;
		return 2;
	}
	// collision
	POSITION+=4;
	IF(POSITION>FULL) POSITION = gArray+8;
	goto LOOP_COL2;
	return 3;
}

:dword arrayGet(dword array,key)
{
	
	dword NEW = 0;
	dword LEN = 0;
	dword MEMLEN = 0;
	dword POSITION = 0;
	dword VKEY = 0;
	dword FULL = 0;
	
	
	LEN = DSDWORD[array];
	array+=4;
	MEMLEN = DSDWORD[array];
	array+=4;
	POSITION = key%MEMLEN*4+array;
	FULL = LEN*4+array;
	
	Z = 2;
	
	LOOP_COL1:
	VKEY = DSDWORD[POSITION];

	IF(!VKEY) return 0;
	IF(DSDWORD[VKEY] == key) return DSDWORD[VKEY+4];
	// collision
	POSITION+=4;
	IF(POSITION>FULL)
	{
		POSITION = array+8;
		Z--;
		IF(!Z) return 0;
	}
	goto LOOP_COL1;
}

:dword arrayDelete(dword array,key)
{
	
	dword NEW = 0;
	dword LEN = 0;
	dword MEMLEN = 0;
	dword POSITION = 0;
	dword VKEY = 0;
	dword FULL = 0;
	dword TMP = 0;
	
	
	LEN = DSDWORD[array];
	array+=4;
	MEMLEN = DSDWORD[array];
	array+=4;
	POSITION = key%MEMLEN*4+array;
	FULL = LEN*4+array;
	
	Z = 2;
	
	LOOP_COL1:
	VKEY = DSDWORD[POSITION];

	IF(!VKEY) return 0;
	IF(DSDWORD[VKEY] == key) 
	{
		TMP = DSDWORD[VKEY+4];
		free(VKEY);
		DSDWORD[POSITION] = 0;
		DSDWORD[array] = DSDWORD[array]-1;
		return TMP;
	}
	// collision
	POSITION+=4;
	IF(POSITION>FULL)
	{
		POSITION = array+8;
		Z--;
		IF(!Z) return 0;
	}
	goto LOOP_COL1;
}

// dict

:dword hash(dword str)
{
	byte S = 0;
	dword s1 = 0;
	dword s2 = 0;
	S = DSBYTE[str];
	WHILE(S)
	{
		s1 += S;
		s2 += s1;
		S = DSBYTE[str];
		str++;
	}
	s1<<=16;
	RETURN s1|s2;
}

:byte dictSet(dword array,key,data)
{
	RETURN arraySet(array,hash(key),data);
}

:byte dictDel(dword array,key)
{
	RETURN arrayDelete(array,hash(key));
}

:dword dictGet(dword array,key)
{
	RETURN arrayGet(array,hash(key));
}


// list
:void listAppend(dword position,data) // TODO Fixed!!
{
	dword len = 0;
	dword flen = 0;
	dword news = 0;
	dword array = 0;
	dword i = 0;
	array = DSDWORD[position];
	len = DSDWORD[array];
	flen = DSDWORD[array+4];
	IF(len>=flen)
	{
		news = malloc(flen<<3+8);
		DSDWORD[position] = news;
		i = flen<<1+2;
		while(i)
		{
			DSDWORD[news] = DSDWORD[array];
			news+=4;
			array+=4;
			i--;
		}
		array = news;
	}
	
	DSDWORD[len*4+array+8] = data;
	DSDWORD[array] = DSDWORD[array]+1;
}
:dword listGet(dword array,key)
{
	return DSDWORD[key*4+array+8];
}
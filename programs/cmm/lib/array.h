// Author: Pavel Iakovlev by. pavelyakov


// Array memory: [dword key][byte flags][dword left][dword right][dword value] -> 17 bytes = 1 position
// If key don't exists then value == 0
:struct Array
{
	dword memory;
	dword offsetMemory;
	dword recursiveIndex(dword i, address);
	byte set(dword key, data);
	dword get(dword key);
	//dword del(dword key);
	byte init(dword size);
};

:dword Array::recursiveIndex(dword key, address)
{
	dword flags = 0;
	flags = DSBYTE[address + 4];
	IF (DSDWORD[address] == key) RETURN address;
	//IF (flags & 100b) RETURN address; // if delete
	IF (flags & 010b) && (DSDWORD[address] < key) RETURN recursiveIndex(key, DSDWORD[address + 5]); // left tree
	IF (flags & 001b) && (DSDWORD[address] > key) RETURN recursiveIndex(key, DSDWORD[address + 9]); // right tree
	RETURN address;
}
:byte Array::init(dword size)
{
	IF(!size) RETURN 0;
	IF(!memory)
	{
		memory = malloc(size * 17);
		EBX = memory;
		DSDWORD[EBX] = 0;
		DSBYTE[EBX + 4] = 0;
		DSDWORD[EBX + 5] = 0;
		DSDWORD[EBX + 9] = 0;
		DSDWORD[EBX + 13] = 0;
		offsetMemory = 17;
		RETURN 0xFF;
	}
	memory = realloc(size * 17);
	RETURN 0xFF;
}
:byte Array::set(dword key, data)
{
	dword address = 0;
	dword newOffset = 0;
	address = recursiveIndex(key, memory);
	/*IF(DSBYTE[address + 4] & 100b)
	{
		IF(DSDWORD[address] < key)
		{
			DSBYTE[address + 4] |= 10b;
			DSDWORD[address + 5] = newOffset;
		}
		ELSE IF(DSDWORD[address] > key)
		{
			DSBYTE[address + 4] |= 01b;
			DSDWORD[address + 9] = newOffset;
		}
		ELSE
		{
			DSDWORD[address + 13] = data;
			RETURN 0xFF;
		}
	}*/
	newOffset = memory + offsetMemory;
	IF(DSDWORD[address] < key)
	{
		DSBYTE[address + 4] |= 10b;
		DSDWORD[address + 5] = newOffset;
	}
	ELSE IF(DSDWORD[address] > key)
	{
		DSBYTE[address + 4] |= 01b;
		DSDWORD[address + 9] = newOffset;
	}
	ELSE
	{
		DSDWORD[address + 13] = data;
		RETURN 0xFF;
	}
	DSDWORD[newOffset] = key;
	DSBYTE[newOffset+4] = 0;
	DSDWORD[newOffset+5] = 0;
	DSDWORD[newOffset+9] = 0;
	DSDWORD[newOffset+13] = data;
	offsetMemory += 17;
	RETURN 0xFF;
}
:dword Array::get(dword key)
{
	EBX = recursiveIndex(key, memory);
	IF(DSDWORD[EBX] != key) RETURN 0;
	IF(DSBYTE[EBX + 4] & 100b) RETURN 0;
	RETURN DSDWORD[EBX + 13];
}
/*:dword Array::del(dword key)
{
	dword address = 0;
	address = recursiveIndex(key, memory);
	IF(DSDWORD[address] != key) RETURN 0;
	DSBYTE[address + 4] |= 100b;
	RETURN 0xFF;
}*/

:struct Dictionary
{
	Array array;
	dword hash(dword text);
	byte set(dword key, value);
	dword get(dword key);
	byte init(dword size);
};

:dword Dictionary::hash(dword text)
{
	dword s1 = 1;
	dword s2 = 0;
	
	WHILE(DSBYTE[text])
	{
		s1 += DSBYTE[text];
		s2 += s1;
		text++;
	}
	IF(s1>0x3FFF) RETURN 0;
	IF(s2>0x3FFFF) RETURN 0;
	RETURN s2<<14|s1;
}

:byte Dictionary::set(dword key, value)
{
	RETURN array.set(hash(key),value);
}

:dword Dictionary::get(dword key)
{
	RETURN array.get(hash(key));
}

:byte Dictionary::init(dword size)
{
	RETURN array.init(size);
}
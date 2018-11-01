// Author: Pavel Iakovlev by. pavelyakov

#ifndef INCLUDE_ARRAY_H
#define INCLUDE_ARRAY_H

// Array memory: [dword key][byte flags][dword left][dword right][dword value] -> 17 bytes = 1 position
// If key don't exists then value == 0
:struct Array
{
	dword memory;
	dword offsetMemory;
	dword lenInitSize;
	dword recursiveIndex(dword i, address);
	byte set(dword key, data);
	dword get(dword key);
	void reallocMemory(dword newSize);
	//dword del(dword key);
	byte init(dword size);
};

:void Array::reallocMemory(dword newSize)
{
	memory = realloc(memory, newSize);
	lenInitSize = newSize;
}

:dword Array::recursiveIndex(dword key, address)
{
	dword flags = 0;
	IF (DSDWORD[address] == key) RETURN address;
	flags = DSBYTE[address + 4];
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
		lenInitSize = size * 17;
		memory = malloc(lenInitSize);
		EBX = memory;
		DSDWORD[EBX] = 0;
		DSBYTE[EBX + 4] = 0;
		DSDWORD[EBX + 5] = 0;
		DSDWORD[EBX + 9] = 0;
		DSDWORD[EBX + 13] = 0;
		offsetMemory = 17;
		RETURN 0xFF;
	}
	IF(size > lenInitSize)
	{
		reallocMemory(size * 17);
		RETURN 0xFF;
	}
	RETURN 0;
}
:byte Array::set(dword key, data)
{
	dword address = 0;
	dword newOffset = 0;
	IF(offsetMemory > lenInitSize) reallocMemory(offsetMemory << 1);
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
		DSBYTE[address + 4] |= 010b; // set flag left address
		DSDWORD[address + 5] = newOffset;
	}
	ELSE IF(DSDWORD[address] > key)
	{
		DSBYTE[address + 4] |= 001b; // set flag right address
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

:dword Dictionary::hash(dword text) // max 255 bytes as strings => 4 byte or duble word hash
{
	dword checkSum1 = 1;
	dword checkSum2 = 0;
	dword beginAddress = 0;
	
	beginAddress = text;
	WHILE(DSBYTE[text])
	{
		checkSum1 += DSBYTE[text];
		checkSum2 += checkSum1;
		text++;
	}
	//IF(h1 > 0x03FFF) RETURN h1 << 8 ^ h2;
	//IF(h2 > 0x3FFFF) RETURN h1 << 8 ^ h2;
	EAX = text - beginAddress;
	EAX <<= 23;
	RETURN EAX | checkSum2;
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

#endif
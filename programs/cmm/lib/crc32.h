/* CRC32 function; Author PaulCodeman */
:dword crc32Table = 0;
:dword makeCRCTable(void)
{
	dword i = 0;
	dword c = 0;
	dword ii = 0;
	dword crcTable = 0;
	IF (crc32Table) RETURN 0;
	crc32Table = malloc(4*256);
	crcTable = crc32Table;
	WHILE (i < 256)
	{
		c = i;
		ii = 0;
		WHILE (ii < 8)
		{
			IF (c&1)
			{
				c >>= 1;
				c ^= 0xEDB88320;
			}
			ELSE c >>= 1;
			ii++;
		}
		DSDWORD[crcTable] = c;
		crcTable += 4;
		i++;
	}
}
:dword crc32(dword bytes, length)
{
	dword crc = 0;
	dword i = 0;
	byte b = 0;
	IF (!crc32Table) makeCRCTable();
	crc = 0xFFFFFFFF;
	WHILE (i < length)
	{
		b = DSBYTE[bytes];
		bytes++;
		EDX = crc^b;
		EDX &= 0xFF;
		EDX <<= 2;
		EDX += crc32Table;
		crc >>= 8;
		crc ^= DSDWORD[EDX];
		i++;
	}
	RETURN crc ^ 0xFFFFFFFF;
}

/* EXAMPLE
dword str = 0;
str = "Kevin van Zonneveld";
crc32(str, strlen(str));
IF (EAX == 1249991249) ExitProcess();
*/
/* Author: PaulCodeman
	Example:
	char bufferb64[0x1000] = {0};
	base64_encode("Kevin van Zonneveld",#bufferb64, 19);
	#bufferb64 == "S2V2aW4gdmFuIFpvbm5ldmVsZA=="
*/

:dword b64_symbols = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
inline void base64_encode(dword data, buffer, length)
{
	byte o1 = 0;
	byte o2 = 0;
	byte o3 = 0;
	dword bits = 0;
	dword h1 = 0;
	dword h2 = 0;
	dword h3 = 0;
	dword h4 = 0;
	
	dword end = 0;
	end = data+length;
	
	do
	{
		o1 = DSBYTE[data];data++;
		if (data < end) {o2 = DSBYTE[data];data++;} else o2 = 0;
		if (data < end) {o3 = DSBYTE[data];data++;} else o3 = 0;
		
		bits = o1<<16;
		bits |= o2<<8;
		bits |= o3;
		
		h1 = bits>>18 & 0x3f;
		h2 = bits>>12 & 0x3f;
		h3 = bits>>6 & 0x3f;
		h4 = bits & 0x3f;
		
		DSBYTE[buffer] = DSBYTE[#b64_symbols+h1];buffer++;
		DSBYTE[buffer] = DSBYTE[#b64_symbols+h2];buffer++;
		DSBYTE[buffer] = DSBYTE[#b64_symbols+h3];buffer++;
		DSBYTE[buffer] = DSBYTE[#b64_symbols+h4];buffer++;
	} while(end > data);
	switch(length % 3)
	{
		case 1:
			buffer--;
			DSBYTE[buffer] = '=';
			buffer--;
			DSBYTE[buffer] = '=';
			buffer+=2;
		break;
		case 2:
			buffer--;
			DSBYTE[buffer] = '=';
			buffer++;
		break;
	}
	DSBYTE[buffer] = 0;
}

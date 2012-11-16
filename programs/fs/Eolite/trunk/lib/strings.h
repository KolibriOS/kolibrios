//CODED by Veliant, Leency 2008-2012. GNU GPL licence.


inline fastcall dword strlen( EDI)
{
	asm {
	  xor ecx, ecx
	  xor eax, eax
	  dec ecx
	  repne scasb
	  sub eax, 2
	  sub eax, ecx
	}
}


inline fastcall copystr( ESI,EDI)
{
	$cld
l1:
	$lodsb
	$stosb
	$test al,al
	$jnz l1
}

char buffer[11];
inline fastcall dword IntToStr( ESI)
{
     $mov     edi, #buffer
     $mov     ecx, 10
     $test     esi, esi
     $jns     f1
     $mov     al, '-'
     $stosb
     $neg     esi
f1:
     $mov     eax, esi
     $push     -'0'
f2:
     $xor     edx, edx
     $div     ecx
     $push     edx
     $test     eax, eax
     $jnz     f2
f3:
     $pop     eax
     $add     al, '0'
     $stosb
     $jnz     f3
     $mov     eax, #buffer
     $ret
}

inline fastcall dword StrToInt()
{
	ESI=EDI=EAX;
	IF(DSBYTE[ESI]=='-')ESI++;
	EAX=0;
	BH=AL;
	do{
		BL=DSBYTE[ESI]-'0';
		EAX=EAX*10+EBX;
		ESI++;
	}while(DSBYTE[ESI]>0);
	IF(DSBYTE[EDI]=='-') -EAX;
}

dword StrToCol(char* htmlcolor)
{
  dword j, color=0;
  char ch=0x00;
  
  FOR (j=0; j<6; j++)
  {
    ch=ESBYTE[htmlcolor+j];
    IF ((ch>='0') && (ch<='9')) ch -= '0';
    IF ((ch>='A') && (ch<='F')) ch -= 'A'-10;
    IF ((ch>='a') && (ch<='f')) ch -= 'a'-10;
    color = color*0x10 + ch;
  }
  
  return color;
}

inline fastcall signed char strcmp(ESI, EDI)
{
	loop()
	{
		IF (DSBYTE[ESI]<DSBYTE[EDI]) RETURN -1;
		IF (DSBYTE[ESI]>DSBYTE[EDI]) RETURN 1;
		IF (DSBYTE[ESI]=='\0') RETURN 0;
		ESI++;
		EDI++;
	}
}



inline fastcall unsigned int strchr(ESI,BL)
{
	int jj=0, last=-1;
	do{
		jj++;
		$lodsb
		IF(AL==BL) last=jj;
	} while(AL!=0);
	return last;
}


inline fastcall TitleCase( EDX)
{
	AL=DSBYTE[EDX];
	IF(AL>='a')&&(AL<='z')DSBYTE[EDX]=AL&0x5f;
	IF (AL>=160) && (AL<=175) DSBYTE[EDX] = AL - 32;	//à-ï
	IF (AL>=224) && (AL<=239) DSBYTE[EDX] = AL - 80;	//à-ï
	do{
		EDX++;
		AL=DSBYTE[EDX];
		IF(AL>='A')&&(AL<='Z'){DSBYTE[EDX]=AL|0x20; CONTINUE;}
		IF(AL>='€')&&(AL<='')DSBYTE[EDX]=AL|0x20; // -¯
		IF (AL>=144) && (AL<=159) DSBYTE[EDX] = AL + 80;	//à-ï
	}while(AL!=0);
}



inline fastcall strcpy( EDI, ESI)
{
	$cld
l2:
	$lodsb
	$stosb
	$test al,al
	$jnz l2
}


inline fastcall strcat( EDI, ESI)
{
  asm {
    mov ebx, edi
    xor ecx, ecx
    xor eax, eax
    dec ecx
    repne scasb
    dec edi
    mov edx, edi
    mov edi, esi
    xor ecx, ecx
    xor eax, eax
    dec ecx
    repne scasb
    xor ecx, 0ffffffffh
    mov edi, edx
    mov edx, ecx
    mov eax, edi
    shr ecx, 2
    rep movsd
    mov ecx, edx
    and ecx, 3
    rep movsb
    mov eax, ebx
	}
}
//------------------------------------------------------------------------------
// strcmp( ESI, EDI)
// strlen( EDI)
// strcpy( EDI, ESI) --- 0 if ==
// strcat( EDI, ESI)
// strchr( ESI,BL) --- find first BL
// strrchr( ESI,BL) --- find last BL
// strstr( EBX, EDX)
// itoa( ESI)
// atoi( EAX)
// strupr( ESI)
// strlwr( ESI) --- kyrillic symbols may not work
// strttl( EDX)
// strtok( ESI)
// strtrim( ESI) --- removes "blank" characters (\r, \n and space)
// chrnum(dword searchin, char symbol)
// strcpyb(dword searchin, copyin, startstr, endstr) --- copy string between strings
// strnumb(dword searchin, startstr, endstr) --- get number between strings
//------------------------------------------------------------------------------

inline fastcall signed int strcmp( ESI, EDI)
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


inline fastcall signed int strncmp( ESI, EDI, ECX)
{
  asm {
    MOV EBX, EDI
    XOR EAX, EAX
    MOV EDX, ECX
    OR ECX, ECX
    JE L1
    REPNE SCASB
    SUB EDX, ECX
    MOV ECX, EDX
    MOV EDI, EBX
    XOR EBX, EBX
    REPE CMPSB
    MOV AL, DSBYTE[ ESI-1]
    MOV BL, DSBYTE[ EDI-1]
    SUB EAX, EBX
L1:
  }
}


inline fastcall unsigned int strlen( EDI)
{
	$xor eax, eax
	$mov ecx, -1
	$REPNE $SCASB
	EAX-=2+ECX;
}


inline fastcall void strcpy( EDI, ESI)
{
	$cld
L2:
	$lodsb
	$stosb
	$test al,al
	$jnz L2
}


inline fastcall int strlcpy(dword ESI, EDI, EBX)
{
	EDX=0;
	do {
		DSBYTE[ESI]=DSBYTE[EDI];
		ESI++;
		EDI++;
		EDX++;
		if (EDX==EBX) { DSBYTE[ESI]='\0'; return -1;}
	} while(DSBYTE[EDI-1]!='\0');
	return 0;
}

inline fastcall strtrim( ESI)
{
	EDI = ESI;
	do{
		AL=DSBYTE[EDI];
		if (AL != '\32') && (AL != '\13') && (AL != '\10')
		{
			DSBYTE[ESI]=AL;
			ESI++;
		}
 		EDI++;
	}while(AL!=0);
	DSBYTE[ESI] = '\0';
}


inline fastcall void strcat( EDI, ESI)
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

inline fastcall void chrcat(ESI, BL)
{
	EDI = strlen(ESI);
	ESBYTE[ESI+EDI] = BL;
	ESBYTE[ESI+EDI+1] = 0;
}


inline fastcall signed int strchr( ESI,BL)
{
	int jj=0;
	do{
		jj++;
		$lodsb
		IF(AL==BL) return jj;
	} while(AL!=0);
	return 0;
}


inline fastcall signed int strrchr( ESI,BL)
{
	int jj=0, last=0;
	do{
		jj++;
		$lodsb
		IF(AL==BL) last=jj;
	} while(AL!=0);
	return last;
}


int chrnum(dword searchin, char symbol)
{
	int num = 0;
	while(DSBYTE[searchin])
	{ 
		if (DSBYTE[searchin] == symbol)	num++;
		searchin++;
	}
	return num;
}


inline fastcall signed int strstr( EBX, EDX)
{
  asm {
    MOV EDI, EDX
    XOR ECX, ECX
    XOR EAX, EAX
    DEC ECX
    REPNE SCASB
    NOT ECX
    DEC ECX
    JE LS2
    MOV ESI, ECX
    XOR ECX, ECX
    MOV EDI, EBX
    DEC ECX
    REPNE SCASB
    NOT ECX
    SUB ECX, ESI
    JBE LS2
    MOV EDI, EBX
    LEA EBX, DSDWORD[ ESI-1]
LS1: MOV ESI, EDX
    LODSB
    REPNE SCASB
    JNE LS2
    MOV EAX, ECX
    PUSH EDI
    MOV ECX, EBX
    REPE CMPSB
    POP EDI
    MOV ECX, EAX
    JNE LS1
    LEA EAX, DSDWORD[ EDI-1]
    JMP SHORT LS3
LS2: XOR EAX, EAX
LS3:
  }
}


dword strstri(dword searchin, usestr_s)
{
	dword usestr_e = usestr_s;
	char si, ue;

	while(DSBYTE[searchin])
	{ 
		si = DSBYTE[searchin];
		ue = DSBYTE[usestr_e];
		if (si>='A') && (si<='Z') si +=32;
		if (ue>='A') && (ue<='Z') ue +=32;
		if (si == ue) usestr_e++; else usestr_e = usestr_s;
		searchin++;
		if (DSBYTE[usestr_e]=='\0') return searchin;
	}
	return 0;
}


unsigned int strcpyb(dword search_in, copyin, startstr, endstr)
{
	dword startp, endp;
	dword copyin_start_off = copyin;
	if (startstr==0) startp = search_in; else startp = strstr(search_in, startstr) + strlen(startstr);
	endp = strstri(startp, endstr);
	if (endp==0) endp = startp+strlen(search_in);
	//if (startp==endp) return 0;
	do
	{ 
		DSBYTE[copyin] = DSBYTE[startp];
		copyin++;
		startp++;
	}
	while (startp<endp);
	DSBYTE[copyin] = '\0';
	return copyin_start_off;
}


/*void strcat(char *to, char *from) //тоже работает
{
	while(*to) to++;
	while(*from)
	{
		*to = *from;
		to++;
		from++;
	}
	*to = '\0';
}*/


inline fastcall dword atoi( EDI)
{
	$push ebx
	$push esi
	ESI=EDI;
	while (DSBYTE[ESI]==' ') ESI++;
	if (DSBYTE[ESI]=='-') ESI++;
	EAX=0;
	while (DSBYTE[ESI]>='0') && (DSBYTE[ESI]<='9')
	{
		$xor ebx, ebx
		EBX = DSBYTE[ESI]-'0';
		EAX *= 10;
		EAX += EBX;
		ESI++;
	} 
	IF (DSBYTE[EDI]=='-') -EAX;
	$pop esi
	$pop ebx
}



inline fastcall strupr( ESI)
{
	do{
		AL=DSBYTE[ESI];
		IF(AL>='a')IF(AL<='z')DSBYTE[ESI]=AL&0x5f;
		IF (AL>=160) && (AL<=175) DSBYTE[ESI] = AL - 32;	//а-п
		IF (AL>=224) && (AL<=239) DSBYTE[ESI] = AL - 80;	//а-п
 		ESI++;
	}while(AL!=0);
}

inline fastcall strlwr( ESI)
{
	do{
		$LODSB
		IF(AL>='A')&&(AL<='Z'){
			AL+=0x20;
			DSBYTE[ESI-1]=AL;
			CONTINUE;
		}
	}while(AL!=0);
}

inline fastcall strttl( EDX)
{
	AL=DSBYTE[EDX];
	IF(AL>='a')&&(AL<='z')DSBYTE[EDX]=AL&0x5f;
	IF (AL>=160) && (AL<=175) DSBYTE[EDX] = AL - 32;	//а-п
	IF (AL>=224) && (AL<=239) DSBYTE[EDX] = AL - 80;	//а-п
	do{
		EDX++;
		AL=DSBYTE[EDX];
		IF(AL>='A')&&(AL<='Z'){DSBYTE[EDX]=AL|0x20; CONTINUE;}
		IF(AL>='А')&&(AL<='П')DSBYTE[EDX]=AL|0x20; //†-ѓ
		IF (AL>=144) && (AL<=159) DSBYTE[EDX] = AL + 80;	//а-п
	}while(AL!=0);
}

unsigned char buffer[11];
dword itoa( ESI)
{
	$pusha
	EDI = #buffer;
	ECX = 10;
	if (ESI < 0)
	{
		 $mov     al, '-'
		 $stosb
		 $neg     esi
	}

	$mov     eax, esi
	$push    -'0'
F2:
	$xor     edx, edx
	$div     ecx
	$push    edx
	$test    eax, eax
	$jnz     F2
F3:
	$pop     eax
	$add     al, '0'
	$stosb
	$jnz     F3
	
	$mov     al, '\0'
	$stosb

	$popa
    return #buffer;
}

inline fastcall itoa_(signed int EDI, ESI)
{
	$pusha
	EBX = EDI;
	ECX = 10;
	if (ESI > 90073741824)
	{
		 $mov     al, '-'
		 $stosb
		 $neg     esi
	}

	$mov     eax, esi
	$push    -'0'
F2:
	$xor     edx, edx
	$div     ecx
	$push    edx
	$test    eax, eax
	$jnz     F2
F3:
	$pop     eax
	$add     al, '0'
	$stosb
	$jnz     F3
	
	$mov     al, '\0'
	$stosb

	$popa
    return EBX;
}  

void debugi(dword d_int)
{
	char tmpch[12];
	itoa_(#tmpch, d_int);
	debug(#tmpch);
}



#define strncpy strcpyn
#define strnmov strmovn
#define stricmp strcmpi
#define strcmpn strncmp


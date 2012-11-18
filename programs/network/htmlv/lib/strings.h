//------------------------------------------------------------------------------
// strcmp( ESI, EDI)
// strlen( EDI)
// strcpy( EDI, ESI)
// strcat( EDI, ESI)
// strchr( ESI,BL)
// strrchr( ESI,BL)
// strstr( EBX, EDX)
// itoa( ESI)
// atoi( EAX)
// strupr( ESI)
// strlwr( ESI) ----- возможно не поддерживает кириллицу
// strttl( EDX)
// strtok( ESI)
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



/*

inline fastcall signed int strcmpi( ESI,EDI)
uses EBX
{
	do{
		$lodsb
		IF(AL>='a')&&(AL<='z')AL-=0x20;
		BL=DSBYTE[(E)DI];
		IF(BL>='a')&&(BL<='z')BL-=0x20;
		AL-=BL;
		IF(!ZEROFLAG)BREAK;
		(E)DI++;
	}while(BL!=0);
}

inline char STRNCMPI((E)SI,(E)DI,(E)CX)
{
	(E)AX=0;
	LOOPNZ((E)CX){
		$lodsb
		IF(AL>='a')&&(AL<='z')AL-=0x20;
		AH=DSBYTE[EDI];
		IF(AH>='a')&&(AH<='z')AH-=0x20;
		EDI++;
		IF(AL==0)||(AH==0)||(AL!=AH)BREAK;
	}
	AL=AL-AH;
}*/



inline fastcall unsigned int strlen( EDI)
{
	$xor eax, eax
	$mov ecx, -1
	$REPNE $SCASB
	EAX-=2+ECX;
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

dword itoa( ESI)
{
	unsigned char buffer[11];
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
f2:
	$xor     edx, edx
	$div     ecx
	$push    edx
	$test    eax, eax
	$jnz     f2
f3:
	$pop     eax
	$add     al, '0'
	$stosb
	$jnz     f3
	
	$mov     al, '\0'
	$stosb
	 
     return #buffer;
} 



inline fastcall dword atoi( EDI)
{
	ESI=EDI;
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



inline fastcall dword strstr( EBX, EDX)
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




/* strtok( LPSTR dest, src, divs);
src - указатель на исходную строку или результат предыдущего вызова
dest - указатель на буфер, куда будет скопировано слово
divs - указатель на строку, содержащую символы-разделители
¬озвращает: 0, если слов больше нет
         не 0, если слово скопировано в dest (передайте это значение
               в качестве src дл€ последующего поиска) */

inline fastcall dword strtok( EDX, ESI, EBX)
{
  asm {
    XOR ECX, ECX
    MOV EDI, EBX
    XOR EAX, EAX
    DEC ECX
    REPNE SCASB
    XOR ECX, 0FFFFFFFFH
    DEC ECX
    PUSH ECX
L1: LODSB
    OR AL, AL
    JZ L4
    MOV EDI, EBX
    MOV ECX, SSDWORD[ ESP]
    REPNE SCASB
    JZ L1
    DEC ESI
L2: LODSB
    MOV EDI, EBX
    MOV ECX, SSDWORD[ ESP]
    REPNE SCASB
    JZ L3
    MOV DSBYTE[ EDX], AL
    INC EDX
    JMP SHORT L2
L3: MOV EAX, ESI
L4: POP ECX
  } DSBYTE[ EDX] = 0;
}

#define strncpy strcpyn
#define strnmov strmovn
#define stricmp strcmpi
#define strcmpn strncmp
#define strncmpi strcmpni
#define stricmpn strcmpni
#define strnicmp strcmpni
#define strincmp strcmpni
#define strcmpin strcmpni
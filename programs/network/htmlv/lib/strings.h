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
// strlwr( ESI)
// strtok( ESI)
//------------------------------------------------------------------------------


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

char buffer[11];
inline fastcall dword itoa( ESI)
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


inline fastcall dword atoi( EDI)
{
	//ESI=EDI=EAX;
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

inline fastcall unsigned int strchr( ESI,BL)
{
	int jj=0;
	do{
		jj++;
		$lodsb
		IF(AL==BL) return jj;
	} while(AL!=0);
}


inline fastcall unsigned int strrchr( ESI,BL)
{
	int jj=0, last=-1;
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


inline fastcall unsigned int strstr( EBX, EDX)
{
  asm {
    mov edi, edx
    xor ecx, ecx
    xor eax, eax
    dec ecx
    repne scasb
    not ecx
    dec ecx
    je ls2
    mov esi, ecx
    xor ecx, ecx
    mov edi, ebx
    dec ecx
    repne scasb
    not ecx
    sub ecx, esi
    jbe ls2
    mov edi, ebx
    lea ebx, DSDWORD[ esi-1]
ls1: mov esi, edx
    lodsb
    repne scasb
    jne ls2
    mov eax, ecx
    push edi
    mov ecx, ebx
    repe cmpsb
    pop edi
    mov ecx, eax
    jne ls1
    lea eax, DSDWORD[ edi-1]
    jmp short ls3
ls2: xor eax, eax
ls3:
  }
}

/* strtok( LPSTR dest, src, divs);
src - указатель на исходную строку или результат предыдущего вызова
dest - указатель на буфер, куда будет скопировано слово
divs - указатель на строку, содержащую символы-разделители
¬озвращает: 0, если слов больше нет
         не 0, если слово скопировано в dest (передайте это значение
               в качестве src дл€ последующего поиска) */

dword fastcall strtok( EDX, ESI, EBX)
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

//------------------------------------------------------------------------------
// strspn(dword text1,text2) --- example: strspn("12 year","1234567890") -> return 2
// strpbrk(dword text1,text2) --- example: strpbrk("this test", " ckfi") -> return "is test"
// strcmp( ESI, EDI)
// strlen( EDI)
// strcpy( EDI, ESI) --- 0 if ==
// strncpy(dword text1,text2,signed length)
// strcat( EDI, ESI)
// strncat(dword text1,text2,signed length) --- pasting the text of a certain length
// strchr( ESI,BL) --- find first BL
// strrchr( ESI,BL) --- find last BL
// strstr( EBX, EDX)
// itoa(signed long number) --- convert the number as a string
// atoi(dword text) --- convert a string as a number
// strupr( ESI)
// strlwr( ESI) --- kyrillic symbols may not work
// strttl( EDX)
// strtok( ESI)
// strltrim(dword text) --- removes "blank" characters on the left (\r, \n and space)
// strrtrim(dword text) --- removes "blank" characters on the right (\r, \n and space)
// strtrim(dword text) --- delete "empty" characters (\ r \ n and space) on both sides
// chrnum(dword searchin, char symbol)
// strcpyb(dword searchin, copyin, startstr, endstr) --- copy string between strings
// strnumb(dword searchin, startstr, endstr) --- get number between strings
// strdup(dword text) --- allocation under the text
//------------------------------------------------------------------------------

/*
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
*/

int strspn(dword text1,text2)
{
	dword beg;
	char s1,s2;
	int ret;
	ret = 0;
	beg = text2;
	do {
		s1 = ESBYTE[text1];
		text2 = beg;
		do {
			s2 = ESBYTE[text2];
			if(s1==s2)
			{
				if(!s2)break;
				$inc ret
				break;
			}
			else $inc text2
		} while(s2);
		$inc text1
	} while(s1);
	return ret;
}

dword strpbrk(dword text1,text2)
{
	char s,ss;
	dword beg;
	beg = text2;
	do {
		s = ESBYTE[text1];
		text2 = beg;
		do {
			ss = ESBYTE[text2];
			if(ss==s) return text1;
			$inc text2
		} while(ss);
		$inc text1
	} while(s);
	return text1;
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


signed int strcmp(dword text1, text2)
{
	char s1,s2;
	dword p1,p2;
	p1 = text1;
	p2 = text2;
	loop()
	{
		s1 = DSBYTE[text1];
		s2 = DSBYTE[text2];
		if(s1==s2)
		{
			if(s1==0) return 0;
		}
		else {
			return s1-s2;
		}
		$inc text1 
		$inc text2
	}
	return 0;
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

void strncpy(dword text1, text2, signed len)
	signed o1,o2;
{
	o1 = len/4;
	o2 = len-4*o1;
	while(o1){
		ESDWORD[text1] = ESDWORD[text2];
		text1 += 4;
		text2 += 4;
		$dec o1
	}
	while(o2){
		ESBYTE[text1] = ESBYTE[text2];
		$inc text1 
		$inc text2 
		$dec o2
	}
}

inline fastcall int strlcpy(dword ESI, EDI, EBX)
{
    if (EBX<0) return -1;
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

/*
inline fastcall void strtrim( ESI)
{
    EDI = ESI;
    do{
        AL=DSBYTE[EDI];
        if (AL != '\32') && (AL != '\13') && (AL != '\10')
        {
            DSBYTE[ESI]=AL;
            $inc ESI
        }
         $inc EDI
    }while(AL!=0);
    DSBYTE[ESI] = '\0';
}
*/

byte __isWhite(int s){ if (s==13)||(s==32)||(s==10)||(s==9) return true; return false; }
void strltrim(dword text){
	int s;
	dword back_text;
	back_text = text;
	s = ESBYTE[text];
	while(__isWhite(s))
	{
		$inc text
		s = ESBYTE[text];
	}
	loop()
	{
		ESBYTE[back_text] = s;
		$inc back_text
		if(!s) break;
		$inc text
		s = ESBYTE[text];
	};
}

void strrtrim(dword text)
{
	int s;
	dword p;
	do {
		s = ESBYTE[text];
		if(__isWhite(s))
		{
			p = text;
			while(__isWhite(s))
			{
				$inc text;
				s = ESBYTE[text];
			}
		}
		else $inc text
	} while(s);
	$dec text
	s = ESBYTE[text];
	if(__isWhite(s)) ESBYTE[p] = 0;
}

void strtrim(dword text){
	int s;
	dword p,back_text;
	back_text = text;
	s = ESBYTE[text];
	while(__isWhite(s))
	{
		$inc text
		s = ESBYTE[text];
	} 
	do {
		s = ESBYTE[text];
		if(__isWhite(s))
		{
			p = back_text;
			while(__isWhite(s))
			{
				ESBYTE[back_text] = s;
				$inc back_text
				$inc text;
				s = ESBYTE[text];
			}
		}
		else {
			ESBYTE[back_text] = s;
			$inc back_text
			$inc text
		}
	} while(s);
	$dec text
	s = ESBYTE[text];
	if(__isWhite(s)) ESBYTE[p] = 0;
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

void strncat(dword text1, text2, signed len)
	signed o1,o2;
	char s;
{
	s = ESBYTE[text1];
	while(s){
		$inc text1
		s = ESBYTE[text1];
	}
	o1 = len/4;
	o2 = len-4*o1;
	while(o1){
		ESDWORD[text1] = ESDWORD[text2];
		text1 += 4;
		text2 += 4;
		$dec o1
	}
	while(o2){
		ESBYTE[text1] = ESBYTE[text2];
		$inc text1 
		$inc text2 
		$dec o2
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
        if (DSBYTE[searchin] == symbol)    num++;
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

dword strcmpi(dword cmp1, cmp2)
{
    char si, ue;

    loop()
    { 
        si = DSBYTE[cmp1];
        ue = DSBYTE[cmp2];
        if (si>='A') && (si<='Z') si +=32;
        if (ue>='A') && (ue<='Z') ue +=32;
        if (si != ue) return -1;
        cmp1++;
        cmp2++;
        if ((DSBYTE[cmp1]=='\0') && (DSBYTE[cmp2]=='\0')) return 0;
        if (DSBYTE[cmp1]=='\0') return -1;
        if (DSBYTE[cmp2]=='\0') return 1;
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


/*void strcat(char *to, char *from) 
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
        IF (AL>=160) && (AL<=175) DSBYTE[ESI] = AL - 32;    //à-ï
        IF (AL>=224) && (AL<=239) DSBYTE[ESI] = AL - 80;    //à-ï
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
    IF (AL>=160) && (AL<=175) DSBYTE[EDX] = AL - 32;    //à-ï
    IF (AL>=224) && (AL<=239) DSBYTE[EDX] = AL - 80;    //à-ï
    do{
        EDX++;
        AL=DSBYTE[EDX];
        IF(AL>='A')&&(AL<='Z'){DSBYTE[EDX]=AL|0x20; CONTINUE;}
        IF(AL>='€')&&(AL<='')DSBYTE[EDX]=AL|0x20; // -¯
        IF (AL>=144) && (AL<=159) DSBYTE[EDX] = AL + 80;    //à-ï
    }while(AL!=0);
}

/*
dword itoa( ESI)
{
    unsigned char buffer[11];
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
*/
	
dword itoa(signed long number)
{
	unsigned char buf[11];
	dword ret;
	byte cmd;
	long mask,tmp;
	mask = 1000000000;
	cmd = true;
	if(!number){
		ESBYTE[buf] = '0';
		ESBYTE[buf+1] = 0;
		return buf;
	}
	ret = buf;
	if(number<0)
	{
		$neg number
		ESBYTE[buf] = '-';
		$inc buf
	}
	while(mask)
	{
		tmp = number / mask;
		tmp = tmp%10;
		
		if(cmd){
			if(tmp){
				ESBYTE[buf] = tmp + '0';
				$inc buf
				cmd = false;
			}
		}
		else {
			ESBYTE[buf] = tmp + '0';
			$inc buf
		}
		mask /= 10;
	}
	ESBYTE[buf] = 0;
	return ret;
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

dword strdup(dword text)
{
    dword l = strlen(text);
    dword ret = malloc(l+1);
    strncpy(ret,text,l);
    return ret;
}

void debugi(dword d_int)
{
    char tmpch[12];
    itoa_(#tmpch, d_int);
    debugln(#tmpch);
}


//#define strncpy strcpyn
#define strnmov strmovn
#define stricmp strcmpi
#define strcmpn strncmp


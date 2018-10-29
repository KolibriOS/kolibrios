#ifndef INCLUDE_STRING_H
#define INCLUDE_STRING_H

#ifndef INCLUDE_MEM_H
#include "../lib/mem.h"
#endif

//------------------------------------------------------------------------------
// strspn(dword text1,text2) --- example: strspn("12 year","1234567890") -> return 2
// strpbrk(dword text1,text2) --- example: strpbrk("this test", " ckfi") -> return "is test"
// strcmp( ESI, EDI)
// strlen( EDI)
// utf8_strlen( ESI)
// strcpy( EDI, ESI) --- 0 if ==
// strlcpy(dword text1,text2,signed length)
// strcat( EDI, ESI)
// strncat(dword text1,text2,signed length) --- pasting the text of a certain length
// strchr( ESI,BL) --- find first BL
// strrchr( ESI,BL) --- find last BL
// strstr( EBX, EDX)
// itoa(signed long number) --- convert the number as a string
// atoi(dword text) --- convert a string as a number
// strupr( ESI)
// strlwr( ESI) --- Cyrillic symbols may not work
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



inline int strspn(dword text1,text2)
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

inline dword strpbrk(dword text1,text2)
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

/*
inline signed int strncmp(dword text1,text2,len)
{
	
	loop()
	{
		if(DSBYTE[text1]!=DSBYTE[text2])return text1-text2;
		$dec len 
		if(!len)return 0;
	}
}
*/
inline fastcall unsigned int strlen( EDI)
{
	$xor eax, eax
	$mov ecx, -1
	$REPNE $SCASB
	EAX-=2+ECX;
}

inline strnlen(dword str, dword maxlen)
{
	dword cp;
	for (cp = str; (maxlen != 0) && (DSBYTE[cp] != '\0'); cp++, maxlen--);
	return cp - str;
}

inline fastcall unsigned int utf8_strlen( ESI)
{
 $xor  ecx, ecx
  _loop: 
 $lodsb
 $test  al, al
 $jz  _done
 $and al, 0xc0
 $cmp al, 0x80
 $jz  _loop 
 $inc ecx
 $jmp _loop
 
  _done:
 return ECX;
}

inline signed int strcmp(dword text1, text2)
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

/*
TODO: rewrite streq() using pure assembliy

inline fastcall void strcpy( EDI, ESI)
{
    $cld
L2:
    $lodsb
    $stosb
    $test al,al
    $jnz L2
}
*/

inline fastcall streq(ESI, EDI)
{
	loop()
	{
		if(DSBYTE[ESI]==DSBYTE[EDI])
		{
			if(DSBYTE[ESI]==0) return true;
		}
		else {
			return false;
		}
		ESI++;
		EDI++;
	}
	return true;
}

/*
signed int strncmp(dword s1, s2, signed n)
unsigned char _s1,_s2;
{
	if (n == 0)
		return 0;
	do {
		_s1 = DSBYTE[s1];
		_s2 = DSBYTE[s2];
		if (_s1 != _s2)
		{
			$dec s2
			return _s1 - _s2;
		}
		$inc s2
		if (_s1 == 0)
			break;
		$inc s1
		$dec n
	} while (n);
	return 0;
}
*/


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

:void strncpy(dword dst, src, len)
{
	while (len)
	{
		ESBYTE[dst] = ESBYTE[src];
		dst++;
		src++;
		len--;
	}
	ESBYTE[dst]=0;
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

inline byte __isWhite(int s){ if (s==13)||(s==32)||(s==10)||(s==9) return true; return false; }
inline void strltrim(dword text){
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

inline void strrtrim(dword text)
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

inline void strtrim(dword text){
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

:void strncat(dword text1, text2, signed len)
signed o1,o2;
char s;
{
	s = DSBYTE[text1];
	while(s){
		$inc text1
		s = DSBYTE[text1];
	}
	o1 = len/4;
	o2 = len-4*o1;
	while(o1){
		DSDWORD[text1] = DSDWORD[text2];
		text1 += 4;
		text2 += 4;
		$dec o1
	}
	while(o2){
		DSBYTE[text1] = DSBYTE[text2];
		$inc text1 
		$inc text2 
		$dec o2
	}
	DSBYTE[text1] = 0;
}

inline fastcall void chrcat(ESI, BL)
{
    EDI = strlen(ESI);
    ESBYTE[ESI+EDI] = BL;
    ESBYTE[ESI+EDI+1] = 0;
}

inline dword strchr(dword shb;char s)
{
	char ss;
	loop()
	{
		ss = DSBYTE[shb];
		if(!ss)return 0;
		if(ss==s)return shb;
		shb++;
	}
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


inline fastcall unsigned int chrnum( ESI, BL)
{
    int num = 0;
    while(DSBYTE[ESI])
    { 
        if (DSBYTE[ESI] == BL) num++;
        ESI++;
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

inline dword strcmpi(dword cmp1, cmp2)
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

inline dword strstri(dword searchin, usestr_s)
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
    return -1;
}


inline unsigned int strcpyb(dword search_in, copyin, startstr, endstr)
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
:unsigned char BUF_ITOA[11];
inline dword itoa(signed long number)
{
	dword ret,p;
	byte cmd;
	long mask,tmp;
	mask = 1000000000;
	cmd = true;
	p = #BUF_ITOA;
	if(!number){
		ESBYTE[p] = '0';
		ESBYTE[p+1] = 0;
		return p;
	}
	ret = p;
	if(number<0)
	{
		$neg number
		ESBYTE[p] = '-';
		$inc p
	}
	while(mask)
	{
		tmp = number / mask;
		tmp = tmp%10;
		
		if(cmd){
			if(tmp){
				ESBYTE[p] = tmp + '0';
				$inc p
				cmd = false;
			}
		}
		else {
			ESBYTE[p] = tmp + '0';
			$inc p
		}
		mask /= 10;
	}
	ESBYTE[p] = 0;
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

inline dword memchr(dword s,int c,signed len)
{
	if(!len) return NULL;
	do {
		if(DSBYTE[s] == c) return s;
		$inc s
		$dec len
	} while(len);
	return NULL;
}

inline dword strdup(dword text)
{
    dword l = strlen(text);
    dword ret = malloc(l+1);
	if(!ret) return NULL;
    strlcpy(ret,text,l);
    return ret;
}

inline dword strndup(dword str, signed maxlen)
{
	dword copy,len;

	len = strnlen(str, maxlen);
	copy = malloc(len + 1);
	if (copy != NULL)
	{
		strlcpy(copy, str, len);
		DSBYTE[len+copy] = '\0';
	}
	return copy;
}

inline dword hexdec(dword text)
{
	char s;
	dword ret,l;
	ret = 0;
	s = DSBYTE[text];
	while(s)
	{	
		ret <<= 4;
		if(s>='A')&&(s<='F')ret |= s-'A'+10;
		else if(s>='a')&&(s<='f')ret |= s-'a'+10;
		else if(s>='0')&&(s<='9')ret |= s-'0';
		text++;
		s = DSBYTE[text];
	}
	return ret;
}

inline signed csshexdec(dword text)
{
	char s;
	dword ret,l;
	byte tmp;
	l = strlen(text);
	ret = 0;
	s = DSBYTE[text];
	tmp = 0;
	if(l==6) while(s)
	{	
		ret <<= 4;
		if(s>='A')&&(s<='F')ret |= s-'A'+10;
		else if(s>='a')&&(s<='f')ret |= s-'a'+10;
		else if(s>='0')&&(s<='9')ret |= s-'0';
		text++;
		s = DSBYTE[text];
	}
	else if(l==3) while(s)
	{	
		ret |= tmp;
		ret <<= 4;
		ret |= tmp;
		ret <<= 4;
		if(s>='A')&&(s<='F')tmp = s-'A'+10;
		else if(s>='a')&&(s<='f')tmp = s-'a'+10;
		else if(s>='0')&&(s<='9')tmp = s-'0';
		text++;
		s = DSBYTE[text];
	}
	return ret;
}

inline cdecl int sprintf(dword buf, format,...)
{
	#define END_ARGS 0xFF00FF //ARGS FUNCTION
	byte s;
	char X[10];
	dword ret, tmp, l;
	dword arg = #format;
	ret = buf;
	s = DSBYTE[format];
	while(s){
		if(s=='%'){
			arg+=4;
			tmp = DSDWORD[arg];
			if(tmp==END_ARGS)goto END_FUNC_SPRINTF;
			$inc format
			s = DSBYTE[format];
			if(!s)goto END_FUNC_SPRINTF;
			switch(s)
			{
				case 's':
					l = tmp;
					s = DSBYTE[tmp];
					while(s)
					{
						DSBYTE[buf] = s;
						$inc tmp
						$inc buf
						s = DSBYTE[tmp];
					}
				break;
				case 'c':
					DSBYTE[buf] = tmp;
					$inc buf
				break;
				case 'u': //if(tmp<0)return ret;
				case 'd':
				case 'i':
					tmp = itoa(tmp);
					if(!DSBYTE[tmp])goto END_FUNC_SPRINTF;
					l = strlen(tmp);
					strlcpy(buf,tmp,l);
					buf += l;
				break;
				case 'a':
				case 'A':
					strlcpy(buf,"0x00000000",10);
					buf+=10;
					l=buf;
					while(tmp)
					{
						$dec buf
						s=tmp&0xF;
						if(s>9)DSBYTE[buf]='A'+s-10;
						else DSBYTE[buf]='0'+s;
						tmp>>=4;
					}
					buf=l;
				break;
				case 'p':
					tmp = itoa(#tmp);
					if(!DSBYTE[tmp])goto END_FUNC_SPRINTF;
					l = strlen(tmp);
					strlcpy(buf,tmp,l);
					buf += l;
				break;
				case '%':
					DSBYTE[buf] = '%';
					$inc buf
				break;
				default:
				goto END_FUNC_SPRINTF;
			}
		}
		else {
			DSBYTE[buf] = s;
			$inc buf
		}
		$inc format
		s = DSBYTE[format];
	}
	END_FUNC_SPRINTF:
	DSBYTE[buf] = 0;
	return ret;
}

inline signed strcoll(dword text1,text2)
{
	char s,ss;
	loop()
	{
		s = DSBYTE[text2];
		ss=strchr(text1,s);
		if(ss)return ss;
		text2++;
	}
	return 0;
}

:replace_char(dword in_str, char from_char, to_char, int length) {
	int i;
	for (i=0; i<length; i++) {
		if (ESBYTE[in_str+i] == from_char) ESBYTE[in_str+i] = to_char;
	}
	ESBYTE[in_str+length]=0;
}


#define strnmov strmovn
#define stricmp strcmpi
#define strcmpn strncmp

#endif
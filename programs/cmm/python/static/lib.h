:dword strcmp(dword str1,str2)
{
	LOOPCMP:
	IF(DSBYTE[str1]!=DSBYTE[str2]) RETURN DSBYTE[str1]-DSBYTE[str2];
	IF(!DSBYTE[str1]) RETURN DSBYTE[str1]-DSBYTE[str2];
	IF(!DSBYTE[str2]) RETURN DSBYTE[str1]-DSBYTE[str2];
	str1++;
	str2++;
	GOTO LOOPCMP;
}
/*
void dll_Load() {
asm {
        push    ebp
        mov     ebp, esp
        mov     esi, SSDWORD[EBP+8]
                @next_lib:    
        mov     edx, DSDWORD[ESI]
        or      edx, edx
        jz      exit
        push    esi
        mov     esi, DSDWORD[ESI+4]
        mov     edi, #libPath
		add edi,9
 
@loc01:
        lodsb
        stosb
        or      al, al
        jnz     loc01
 
        mov     eax, 68
        mov     ebx, 19
        mov     ecx, #libPath
        int     0x40
        or      eax, eax
        jz      fail
 
        push    edx
        push    eax
        call    dll_Link
 
        push    eax
        mov     eax, DSDWORD[eax]
        cmp     DSDWORD[EAX], '_bil'    // somehow this needs to be reversed..
        pop     eax
        jnz     loc02
 
        push    DSDWORD[EAX+4]
        call    dll_Init
@loc02:
 
        pop     esi
        add     esi, 8
        jmp     next_lib
@exit:
        xor     eax, eax
        leave
        ret     4
       
@fail:        
        add     esp, 4
        xor     eax, eax
        inc     eax
        leave
        ret     4
    }
}
void dll_GetProcAddress(){
asm {
        push    ebp
        mov     ebp, esp
        mov     edx, CSDWORD[EBP+8]
        xor     eax, eax
 
@next:        
        or      edx, edx
        jz      end
        cmp     CSDWORD[edx], 0
        jz      end
 
        push    CSDWORD[EBP+12]
        push    CSDWORD[EDX]
        call    dll_StrCmp
        test    eax, eax
        jz      ok
        add     edx, 8
        jmp     next
@ok:
        mov     eax, DSDWORD[EDX+4]
@end:
        leave
        ret     8
    }
}
void dll_StrCmp() {
asm {
        push    ebp
        mov     ebp, esp
        push    esi
        push    edi
        mov     esi, CSDWORD[EBP+8]
        mov     edi, CSDWORD[EBP+12]
        xor     eax, eax
@label1:
        lodsb
        scasb
        jne     fail
        or      al, al
        jnz     label1
        jmp     label_ok
@fail:
        or      eax, -1
@label_ok:
        pop     edi
        pop     esi
        leave
        ret     8
    }
}
void dll_Link() {
asm {
        push    ebp
        mov     ebp, esp
        push    eax
        mov     esi, SSDWORD[EBP+12]
        test    esi, esi
        jz      done
@next:        
        lodsd
        test    eax, eax
        jz      done
        push    eax
        push    SSDWORD[EBP+8]
        call    dll_GetProcAddress
        or      eax, eax
        jz      loc03
        mov     DSDWORD[esi-4], eax
        jmp     next
@loc03:
        mov     SSDWORD[esp], 0
@done:
        pop     eax
        leave
        ret     8
    }
}
void dll_Init() {
asm {
        push    ebp
        mov     ebp, esp
        pushad
        mov     eax, #malloc
        mov     ebx, #free;
        mov     ecx, #realloc;
        mov     edx, #dll_Load;
        call    SSDWORD[EBP+8]
        popad
        leave
        ret     4
    }
}
*/
:dword importLibrary(dword name)
{
	dword l = 0;
	dword i = 0;
	dword list = 0;
	dword key = 0;
	dword data = 0;
	
	dword ret = 0;
	
	i = malloc(1000);
	l = strcpy(TEMP,#libPath);
	l += strcpy(TEMP+l,name);
	memcpy(TEMP+l,".obj",5);
	
	EAX = 68;
	EBX = 19;
	ECX = TEMP;
	$int 0x40;
	
	list = EAX;
	
	while(DSDWORD[list])
	{
		test2(DSDWORD[list],0);
		IF(!strcmp(DSDWORD[list],"con_init"))
		{
			//dll_Init(DSDWORD[list+4]);
		}
		ELSE dictSet(ret,DSDWORD[list],DSDWORD[list+4]);
		list+=8;
	}
	EAX = DSDWORD[i+92];
	i = malloc(10);
	strcpy(i,"test");
	i = realloc(i,19);
	test1(i,0);
}
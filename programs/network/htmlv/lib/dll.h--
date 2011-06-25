//Asper
char a_libdir[43]  = "/sys/lib/\0";


//proc dll.Load, import_table:dword
int dll_Load(dword import_table)
{
                $mov     esi, import_table                                
  @next_lib:    $mov     edx, DSDWORD[esi]
                $or      edx,edx
                $jz      exit_
                $push    esi                
                $mov     esi,DSDWORD[esi+4] 
                $mov     edi,#a_libdir
                
                $push    edi
                $push    esi
                EAX=strlen(#a_libdir);
                $pop     esi
                $pop     edi
                $add     edi, eax //9
                
            @loc01: $lodsb
                $stosb
                $or      al,al
                $jnz     loc01

                                //IntToStr(EBX);
                                //$push edx    
                                //WriteDebug(#a_libdir);
                                //$pop edx
                //mcall   68,19,a_libdir
                $mov     eax, 68
                $mov     ebx, 19
                $mov     ecx,#a_libdir
                $int     0x40
                
                $or      eax,eax
                $jz      fail
                //stdcall dll.Link,eax,edx
                dll_Link(EAX, EDX);
                $push    eax
                $mov     eax, DSDWORD [eax]
                $cmp     DSDWORD [eax], 'lib_'
                $pop     eax
                //$jnz     loc02
                                //IntToStr(EBX);
                                //$push    eax
                                //WriteDebug(DSDWORD[EAX]);
                                //$pop     eax
                //stdcall dll.Init,[eax+4]
                //dll_Init(DSDWORD[EAX]); //dll_Init(DSDWORD[EAX+4]);
            @loc02:
                $pop     esi
                $add     esi,8
                $jmp     next_lib
  @exit_:        $xor     eax,eax
                return 0;
  @fail:        $add     esp,4
                $xor     eax,eax
                $inc     eax
                return -1;
}

//proc dll.Link, exp:dword,imp:dword
void dll_Link(dword exp, imp)
{
                $push    eax
                $mov     esi, imp
                $test    esi, esi
                $jz      done
  @next:        $lodsd
                $test    eax,eax
                $jz      done

                //stdcall dll.GetProcAddress,[exp],eax
                dll_GetProcAddress(exp,EAX);
                $or      eax,eax
                $jz      loc03

                $mov     DSDWORD[esi-4],eax
                $jmp     next
  @loc03: 
                $mov     DSDWORD[esp],0
  @done:        $pop     eax
}

//proc dll.Init, dllentry:dword
void dll_Init(dword dllentry)
{
                $pushad
                EAX=#mem_Alloc;
                EBX=#mem_Free;
                ECX=#mem_ReAlloc;
                EDX=#dll_Load;
                DSDWORD [dllentry+4] stdcall ();                
                $popad
}

//proc dll.GetProcAddress, exp:dword,sz_name:dword
dword dll_GetProcAddress(dword exp, sz_name)
{
                $push esi
                $mov     edx, exp
                $xor     eax,eax
  @next:        $or      edx,edx
                $jz      end_
                $cmp     edx,0
                $jz      end_
                strcmp(DSDWORD[EDX],sz_name);
                $test    eax,eax
                $jz      ok
                $add     edx,8
                $jmp     next
  @ok:
                $mov     eax, DSDWORD[edx+4]
  @end_:         
                $pop  esi
    return EAX;
}


int load_dll2(dword dllname, import_table, byte need_init)
{
   //dword dllentry=0;
// load DLL
        $mov     eax, 68
        $mov     ebx, 19
        ECX=dllname;
        $int     0x40
        $test    eax, eax
        $jz      exit01

// initialize import
        $mov     edx,eax
        ESI=import_table;

@import_loop01:
        $lodsd
        $test    eax,eax
        $jz      import_done01
        $push    edx
@import_find01:
        $mov     ebx,DSDWORD[EDX]
        $test    ebx, ebx
        $jz      exit01
        $push    eax
@nex101:
        $mov     cl,DSBYTE[EAX];
        $cmp     cl,DSBYTE[EBX];
        $jnz     import_find_next01
        $test    cl,cl
        $jz      import_found01
        $inc     eax
        $inc     ebx
        $jmp     nex101
@import_find_next01:
        $pop     eax
        $add     edx, 8
        $jmp     import_find01
@import_found01:
        $pop     eax
        $mov     eax,DSDWORD[edx+4]
        $mov     DSDWORD[esi-4],eax
        $pop     edx
       
        $jmp     import_loop01
@import_done01:
        IF (need_init) dll_Init(EDX);
        return 0;
@exit01:
        return -1;
}

format ELF
include 'proc32.inc'
section '.text' executable

public _ksys_cofflib_load
public _ksys_cofflib_getproc

proc  _ksys_cofflib_load stdcall, name:dword

        mov eax, 68
        mov ebx, 19
        mov ecx, [name]
        int 0x40
        ret
endp

proc  _ksys_cofflib_getproc stdcall, export:dword,name:dword

        mov ebx,[export]

        next_name_check:

        mov ecx,[ebx]
        test ecx,ecx
        jz end_export

        ;cmp export string with name
        mov esi,[name]
        xor edi,edi
        next_simbol_check:

        xor eax,eax
        mov al,[ecx]
;	siemargl moved to post-check
;        test al,al
;        jz exit_check_simbol

                xor edx,edx
                mov dl,[esi]
                cmp al,dl
                je simbols_equvalent
                        add edi,1
                        jmp exit_check_simbol
                simbols_equvalent:
        test al,al
        jz exit_check_simbol

		  ;pushad

		  ;mov	cl,al
		  ;mov	ebx,1
		  ;mov	eax,63
		  ;int	0x40

		  ;popad

        add ecx,1
        add esi,1
        jmp next_simbol_check
        exit_check_simbol:

        test edi,edi
        jnz function_not_finded
                mov eax,[ebx+4]
                jmp end_export
        function_not_finded:

        add ebx,8

        jmp next_name_check

        end_export:

        ret 
endp


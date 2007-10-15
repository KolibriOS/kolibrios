
_ksys_cofflib_getproc:

        mov ebx,[esp+4]

        next_name_check:

        mov ecx,[ebx]
        test ecx,ecx
        jz end_export

        ;cmp export string with name
        mov esi,[esp+8]
        xor edi,edi
        next_simbol_check:

        xor eax,eax
        mov al,[ecx]
        test al,al
        jz exit_check_simbol

                xor edx,edx
                mov dl,[esi]
                cmp al,dl
                je simbols_equvalent
                        add edi,1
                        jmp exit_check_simbol
                simbols_equvalent:

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

        ret 8

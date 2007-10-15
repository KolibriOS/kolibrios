format ELF
section '.text' executable

public console_init
public console_printf
public console_exit

align 4
console_init:

        pushad

        mov eax,[console_init_status]
        test eax,eax
        jnz console_initializated

        mov [console_init_status],1

        mov eax,68
        mov ebx,19
        mov ecx,console_path
        int 0x40

        test eax,eax
        jz console_not_loaded

                mov ebx,[eax+4]
                mov [con_start],ebx

                mov ebx,[eax+4+16]
                mov [con_init],ebx

                mov ebx,[eax+4+32]
                mov [con_printf],ebx

                push 1
                call [con_start]

                push caption
                push -1
                push -1
                push -1
                push -1
                call [con_init]

        console_not_loaded:

        console_initializated:

        popad

        ret

align 4
console_printf:

        pop [return_addres]

        call [con_printf]
        ;add esp,8

        push [return_addres]

        ret

align 4
console_exit:

        push 0
        call [con_exit]

        ret


;-----------------------------
console_path db '/sys/dll/console.obj',0
caption      db 'Console',0

align 4
con_start                 rd 1
con_init                  rd 1
con_printf                rd 1
con_exit                  rd 1
console_init_status       rd 1
return_addres             rd 1

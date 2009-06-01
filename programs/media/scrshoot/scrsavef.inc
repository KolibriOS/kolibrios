save_file:
pusha
        bt      dword [flags],0
        jnc     .not_shooted

        cmp     byte [ed_buffer.1],0
        je      .no_file_name

        mov     edi,sign_n_input
        call    zstr_to_int
        mov     [sign_n],al

        ; генерируем имя файла
        xor     eax,eax
        mov     ecx,1056
        mov     edi,file_name
        rep     stosb   ; заполняем нулями

        mov     esi,ed_buffer.1
        mov     edi,file_name
.next:
        mov     ah,[esi]
        test    ah,ah
        jz      .str_end
        cmp     ah,'*'
        jne     .no_insert_number
        jmp     .insert_number
.no_insert_number:
        mov     [edi],ah
        inc     edi
        inc     esi
        jmp     .next
.insert_number:
        bt      word [ch6.flags],1
        jnc     @f
        mov     eax,[cur_number]
        ;mov     ebx,4
        movsx   ebx,byte [sign_n]
        push    ebx
        call    int_to_str
        pop     ebx
        add     edi,ebx
@@:
        inc     esi
        jmp     .next
.str_end:

;;;;clean memory 

        mov     ecx,dword [sf_buf.bmp_header]
        mov     ebx,13
        mov     eax,68
        mcall

; Вычисляем, сколько байт нужно прибавлять к каждой строке,
; чтобы следующая была выравнена на 4 байта.
; Результат помещаем в edi.

        xor     edx,edx
        movzx   eax,word [scr_buf.width]  ; eax = ширина картинки
        lea     eax,[eax*3]               ; eax = байт на строку картинки
        mov     ebx,4                     ; ebx = 4
        mov     edi,ebx                   ; edi = 4
        div     ebx                       ; eax = байт на строку картинки div 4  edx = байт... mod 4
        sub     edi,edx                   ; edi = количество байт
        test    edx,edx
        jnz     @f
        xor     edi,edi
@@:

        ; выделяем память
;        mov     ecx,[scr_buf.end_ptr]
;        mov     [sf_buf.bmp_header],ecx

;        add     ecx,0x36
;        mov     [sf_buf.bmp_area],ecx
        movsx   ebx,word [scr_buf.width]
        movsx   edx,word [scr_buf.height]
        ;imul    ebx,edx
        lea     ebx,[ebx*3]
        add     ebx,edi
        imul    ebx,edx
        mov     ecx,ebx

;        mov     [sf_buf.end],ecx
        add     ecx,0x36
        mov     ebx,12
        mov     eax,68
        mcall

        mov     [sf_buf.bmp_header],eax 
        add     eax,0x36
        mov     [sf_buf.bmp_area],eax
        add     eax,ecx
        sub     eax,0x36
        mov     [sf_buf.end],eax

;        push    edi
        ; заполняем структуру для сохранения файла
        mov     ebp,fs_struc
        mov     [ebp],dword 2

        mov     eax,[sf_buf.bmp_header]
        mov     ebx,[sf_buf.end]
        sub     ebx,eax
        mov     [ebp+12],ebx
        mov     [ebp+16],eax
        mov     [ebp+21],dword file_name

        ; заполняем заголовок bmp
        mov     ebp,[sf_buf.bmp_header]
        mov     [ebp],word 'BM'
        mov     ebx,[sf_buf.end]
        sub     ebx,[sf_buf.bmp_area]
        mov     [ebp+34],ebx    ; размер картинки
        mov     [ebp+10],dword 0x36     ; размер заголовка
        add     ebx,0x36
        mov     [ebp+2],ebx ; размер файла
        mov     [ebp+14],dword 0x28
        movzx   eax,word [scr_buf.width]
        mov     [ebp+18],eax ; горизонтальный размер
        movzx   eax,word [scr_buf.height]
        mov     [ebp+22],eax ; вертикльный размер
        mov     [ebp+26],word 1
        mov     [ebp+28],word 24

        ; сохраняем
        mov     esi,[scr_buf.ptr]
        mov     edi,[sf_buf.end]
        movzx   edx,word [scr_buf.height]
        movzx   ebx,word [scr_buf.width]
        lea     ebx,[ebx*3]
        
        test    ebx,11b
        jz      @f
        shr     ebx,2
        inc     ebx
        shl     ebx,2
@@:
        cld
align 4
.next_str:
        sub  edi,ebx
        mov ecx,ebx
        shr ecx,2
        rep movsd
        sub edi,ebx
        dec edx
        jnz .next_str 

       draw_status saving
        mov     eax,70
        mov     ebx,fs_struc
        mcall

        test    eax,eax
        jne     save_error

        inc     dword [cur_number]
        call    draw_number

        draw_status saved_ok

popa
ret

.no_file_name:
        draw_status no_file_name
popa
ret

.not_shooted:
        draw_status not_shooted
popa
ret

save_error:
        cmp     al,5
        jne     @f
        mov     [status.text],dword bad_file_name
@@:
        cmp     al,8
        jne     @f
        mov     [status.text],dword disk_filled
@@:

        cmp     al,9
        jne     @f
        mov     [status.text],dword bad_fat_table
@@:

        cmp     al,10
        jne     @f
        mov     [status.text],dword ac_den
@@:

        cmp     al,11
        jne     @f
        mov     [status.text],dword device_er
@@:
        call    send_draw_status
popa
ret
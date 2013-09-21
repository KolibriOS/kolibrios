include 'libimg/libimg.asm'


;возвращает указатель на память либо 0
LoadIcon:
path    equ ebp+8
BuffR   equ ebp-16
BuffH   equ ebp-22
endTablI equ ebp-26

        push    ebp
        mov     ebp,esp
        add     esp,-26

        push    ebx edi esi

;--------------------------- проверяем - загружена ли этот файл с иконкой ------
        xor     ebx,ebx
   .SearchIco:
        mov     edi,[path]
        mov     esi,[IcoTable+ebx]
    @@: lodsb
        scasb
        jne     @f
        test    al,al
        jnz     @b

        mov     eax,[IcoTable+ebx+4]
        pop     esi edi ebx
        leave
        ret     4
    @@:
        add     ebx,8
        cmp     ebx,50*8
        jne     @f

        xor     eax,eax
        pop     esi edi ebx
        leave
        ret     4
    @@:
        cmp     [IcoTable+ebx],0
        jnz     .SearchIco

;-------------------------------------------------------------------------------
        mov     [endTablI],ebx

        m2m     dword[fiIcon.path],dword[path]
        mcall   70,fiIcon
        test    eax,eax
        jz      @f

        xor     eax,eax
        pop     esi edi ebx
        leave
        ret     4

     @@:
        xor     ecx,ecx
        mov     cx,[BuffH+4]
        xor     edi,edi
     .TestRecord:
        cmp     word[BuffR],2020h
        jne     .NextRec
        cmp     byte[BuffR+2],0
        jne     .NextRec

        lea     eax,[BuffR]
        stdcall LoadIco32b, [path],eax


        pop     esi edi ebx
        leave
        ret     4

     .NextRec:
        inc     edi

        mov     eax,edi
        shl     eax,4
        add     eax,6
        mov     dword[fiIcon.pos],eax
        mcall   70,fiIcon

        loop    .TestRecord

        xor     eax,eax
        pop     esi edi ebx
        leave
        ret     4
restore path
restore BuffH
restore BuffR

proc LoadIco32b, path,bufHeader
local ImgBuff:DWORD
        mov     edi,[bufHeader]
        cmp     dword[edi+8],10A8h
        je      @f

        xor     eax,eax
        ret

     @@:
        mcall   68,12,32*32*4*2
        mov     ebx,[endTablI]
        mov     [IcoTable+ebx],eax
        add     eax,4096
        mov     [IcoTable+ebx+4],eax
        mov     [ImgBuff],eax
        m2m     dword[fiIcon.pos],dword[edi+12]
        mov     dword[fiIcon.size],32*32*4
        mcall   70,fiIcon

        mov     eax,[ImgBuff]
        ret
endp



fiIcon:
        dd 0
 .pos   dd 0
        dd 0
 .size  dd 22
 .point dd 0
        db 0
 .path  dd 0


IcoTable rd 2*50       ;(name(4), data(4)). End - dword 0
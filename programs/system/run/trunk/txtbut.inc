;--------------------------------------------------------------------
;--- ??ᮢ???? ?????? ? ⥪?⮬ ? ??।??? --------------------------
;--------------------------------------------------------------------
macro use_txt_button
{
txt_button:
.draw:
pusha
;--- ?????ᮢ?? ᠬ?? ?????? ---
        mov     eax,8
        mov     ebx,[edi] ; [but.width]
        mov     ecx,[edi+4] ;[but.height]
        mov     edx,[edi+8];[but.id]
        mov     esi,[edi+12];[but.color]
        mcall
;--- ⥪?? ?? ??? ---
        ; ???⠥? ???????⢮ ᨬ?????
        mov     edx,[edi+20]
        xor     esi,esi
.check_next_symbol:
        cmp     [edx+esi],byte 0
        je      .str_end
        inc     esi      ; ? esi ????? ??ப? ? ᨬ?????
        jmp     .check_next_symbol
.str_end:

        ; ??????塞 ???न????
        lea     ebx,[esi*2+esi]
        lea     ebx,[ebx*2] ; ? ebx ????? ??ப? ? ???ᥫ??

        ;not     ebx
        neg     ebx
        add     bx,[edi] ; [but.width]
        sar     bx,1
        inc     bx
        add     bx,[edi+2];[but.left]  ; ? bx ??????? ᫥??
        shl     ebx,16


        mov     bx,[edi+4];[but.height]
        sub     bx,8-1
        sar     bx,1
        add     bx,[edi+6];[but.top]   ; ? bx ??????? ᢥ???

        ; ???㥬 ??ப? ⥪???
        mov     ecx,[edi+16] ;[but.text_color]
        shr     eax,1
        mcall
popa
ret
}

struc txt_button width,left,height,top,id,color,text_color,text_ptr,\
proc_addres
{
.width dw width ;+0
.left dw left   ;+2
.height dw height ;+4
.top dw top       ;+6
.id dd id         ;+8
.color dd color   ;+12
.text_color dd text_color  ;+16
.text_ptr dd text_ptr      ;+20
.proc_addres dd proc_addres+0 ;+24
}
but_struc_size=24+4
macro draw_txt_buttons start,end
{
        mov     edi,start
        mov     ecx,((end-start)/but_struc_size)
@@:
        call    txt_button.draw
        add     edi,but_struc_size
        loop    @b
}

macro txt_but_set_sys_color start,end,color_table
{
        mov     edi,start
        mov     esi,color_table
        mov     ecx,((end-start)/but_struc_size)
@@:
        push    dword [esi+24]
        pop     dword [edi+12]
        push    dword [esi+28]
        pop     dword [edi+16]
        add     edi,but_struc_size
        loop    @b
}

macro txt_but_ev start,end
{
        mov     edi,start
        mov     ecx,((end-start)/but_struc_size)
.txt_but_next:
        cmp     ah,[edi+8]
        jne     @f
        cmp     dword [edi+24],0
        je      @f
        pusha
        call    dword [edi+24]
        popa
@@:
        add     edi,but_struc_size
        loop    .txt_but_next
}

macro draw_txt_button ptr
{
        mov     edi,ptr
        call    txt_button.draw
}

macro set_sys_colors_txt_button but,sc
{
        mov     esi,sc
        mov     edi,but
        push    dword [esi+24]
        pop     dword [edi+12]
        push    dword [esi+28]
        pop     dword [edi+16]
}

macro event_txt_button but,sc
{

}
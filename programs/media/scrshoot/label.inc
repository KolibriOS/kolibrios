macro use_label
{
draw_label:
pusha
        xor     esi,esi
        mov     edx,[edi+8]
.check_next:
        mov     ah,[edx+esi]
        inc     esi
        test    ah,ah
        jz      @f
        jmp     .check_next
@@:
        mov     eax,4
        mov     ebx,[edi]
        mov     ecx,[edi+4]
        mcall
popa
ret
}

struc label left,top,color,text
{
.top: dw top ; +0
.left: dw left ; +2
.color: dd color; +4
.text: dd text  ; +8
}

l_struc_size=12
macro draw_labels start,end
{
        mov     edi,start
        mov     ecx,((end-start)/l_struc_size)
@@:
        call    draw_label
        add     edi,l_struc_size
        loop    @r
}

macro labels_set_sys_color start,end,color_table
{
        mov     edi,start
        mov     ecx,((end-start)/l_struc_size)
        mov     esi,color_table
@@:
        push    dword [esi+32]
        pop     dword [edi+4]
        add     edi,l_struc_size
        loop    @r
}
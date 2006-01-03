blit_8_to_32:
    mov esi,virtual_screen_8		; ESI=src
    mov edi,virtual_screen_32		; EDI=dst
    mov ecx,WND_SIZE_X*WND_SIZE_Y	; ECX=size
.blit_next:
    xor eax,eax				; EAX=byte to blit
    mov al,[esi]
    shl eax,2
    mov eax,[_palette+eax]
    mov [edi],ax
    shr eax,16
    mov [edi+2],al
    inc esi
    add edi,3
    loop .blit_next
    ret

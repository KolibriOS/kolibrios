;**************************************
;* IRQ HANDLER FOR PS/2 MOUSE         *
;**************************************

proc irq_handler

;        call    Wait8042BufferEmpty  ;clear buffer
        in      al,0x60              ;get scan-code

        cmp     [mouse_byte],0
        je      .byte1
        cmp     [mouse_byte],1
        je      .byte2
        cmp     [mouse_byte],2
        je      .byte3
        cmp     [mouse_byte],3
        je      .byte4
        jmp     .error

  .byte1:
        test    al,1000b             ;first byte?
        jz      .error               
        mov     [first_byte],al
        inc     [mouse_byte]
        jmp     .exit

  .byte2:
        mov     [second_byte],al
        inc     [mouse_byte]
        jmp     .exit

  .byte3:
        mov     [third_byte],al
        cmp     [MouseType],MT_3B
        je      .full_packet
        inc     [mouse_byte]
        jmp     .exit
        
  .byte4:
        mov     [fourth_byte],al
        

  .full_packet:
        mov     [mouse_byte],0
        mov     al,byte [first_byte]
        and     eax,7
        mov     byte [ButtonState],al
        
        cmp     [MouseType],MT_3B
        je      .xy_moving
        mov     al,[fourth_byte]
        cmp     [MouseType],MT_3BScroll
        je      .z_moving
        
        mov     ah,al
        and     ah,00110000b
        shr     ah,1
        or      byte [ButtonState],ah
        and     al,00001111b
        bt      eax,3
        jnc     .z_moving
        or      al,11110000b

  .z_moving:
        movsx   eax,al
        mov     [ZMoving],eax

  .xy_moving:
        mov     ah,0   
        mov     al,[first_byte]
        test    al,10000b
        jz      @f
        mov     ah,0FFh

    @@:
        mov     al,[second_byte]
        cwde
        mov     [XMoving],eax

        mov     ah,0   
        mov     al,[first_byte]
        test    al,100000b
        jz      @f
        mov     ah,0FFh

    @@:
        mov     al,[third_byte]
        cwde

    @@:
        mov     [YMoving],eax
        
        mov     eax,[ZMoving]
        test    eax,1
        jnz     .vert
        
        sar     eax,1
        push    eax
        push    0
        jmp     @f
    
    .vert:
        push    0
        push    eax
        
    @@:

        invoke  SetMouseData, [ButtonState], [XMoving], [YMoving]
        
        jmp   .exit

  .error:
        mov   [mouse_byte],0

  .exit:
        xor     eax, eax
        inc     eax
        ret
endp


;***********************************************
;*   Waiting for clearing I8042 buffer         *
;* Retutned state:                             *
;* ZF is set - good ending,                    *
;* ZF is cleared - time-out error.             *
;***********************************************
;Wait8042BufferEmpty:
;        push ecx
;        xor  ecx,ecx
;      @@:
;        in     al,64h
;        test   al,00000010b
;        loopnz @b
;        pop    ecx
;
;        ret

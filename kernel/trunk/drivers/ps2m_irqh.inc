;**************************************
;* ОБРАБОТЧИК ПРЕРЫВАНИЯ ОТ МЫШИ PS/2 *
;**************************************

proc irq_handler

        call    Wait8042BufferEmpty  ;очистка буфера
        in      al,0x60              ;получить скэн-код

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
        test    al,1000b             ;первый байт посылки?
        jz      .error               ;сбой синхронизации
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
        mov     ah,0   ;дублируем знак во все разряды AH
        mov     al,[first_byte]
        test    al,10000b
        jz      @f
        mov     ah,0FFh

    @@:
        mov     al,[second_byte]
        cwd
        mov     [XMoving],eax

        mov     ah,0   ;дублируем знак во все разряды AH
        mov     al,[first_byte]
        test    al,100000b
        jz      @f
        mov     ah,0FFh

    @@:
        mov     al,[third_byte]
        cwd

    @@:
        mov     [YMoving],eax
        stdcall SetMouseData, [ButtonState], [XMoving], [YMoving], [ZMoving], 0

        
        jmp   .exit

  .error:
        mov   [mouse_byte],0

  .exit:
        ret
endp


;***********************************************
;*   ОЖИДАНИЕ ОЧИСТКИ ВХОДНОГО БУФЕРА I8042    *
;* При выходе из процедуры:                    *
;* флаг ZF установлен - нормальное завершение, *
;* флаг ZF сброшен - ошибка тайм-аута.         *
;***********************************************
Wait8042BufferEmpty:
        push ecx
        xor  ecx,ecx
      @@:
        in     al,64h
        test   al,00000010b
        loopnz @b
        pop    ecx

        ret                   ;возврат в подпрограмму

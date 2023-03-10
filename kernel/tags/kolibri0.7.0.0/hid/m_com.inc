$Revision$
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

struc COM_MOUSE_DATA {
; ????? ???????????? ?? ???? ?????
        .MouseByteNumber        db      ?
; ???????????? ????????? ??????, ???????????? ?????
        .FirstByte              db      ?
        .SecondByte             db      ?
        .ThirdByte              db      ?
        .timer_ticks_com        dd      ?
}
virtual at 0
 COM_MOUSE_DATA COM_MOUSE_DATA
end virtual

uglobal
com1_mouse COM_MOUSE_DATA
com2_mouse COM_MOUSE_DATA
endg

;***************************************
;* ????? ?????????? ?????????? ?? ???? *
;***************************************
check_mouse_data_com:
; in: esi -> COM_MOUSE_DATA struc, dx = base port (xF8h)
        add     edx, 5          ; xFDh
        in      al, dx
        test    al, 1           ; ?????? ???????
        jz      .Error
; ?????? ??????
        sub     edx, 5
        in      al, dx
; ???????? ??????? ?????????? ???
        and     al, 01111111b

; ?????????? ?????????? ????? ???????????? ?????
        cmp     [esi+COM_MOUSE_DATA.MouseByteNumber], 2
        ja      .Error
        jz      .ThirdByte
        jp      .SecondByte
; ????????? ?????? ???? ??????
.FirstByte:
        test    al, 1000000b    ; ?????? ???? ????????
        jz      .Error
        mov     [esi+COM_MOUSE_DATA.FirstByte], al
        inc     [esi+COM_MOUSE_DATA.MouseByteNumber]
        jmp     .EndMouseInterrupt
; ????????? ?????? ???? ??????
.SecondByte:
        test    al, 1000000b
        jnz     .Error
        mov     [esi+COM_MOUSE_DATA.SecondByte], al
        inc     [esi+COM_MOUSE_DATA.MouseByteNumber]
        jmp     .EndMouseInterrupt
; ????????? ?????? ???? ??????
.ThirdByte:
        test    al, 1000000b
        jnz     .Error
        mov     [esi+COM_MOUSE_DATA.ThirdByte], al
        mov     [esi+COM_MOUSE_DATA.MouseByteNumber], 0
; (????? ?????? ?? ???? ?????? ?????????).
; ???????? ????? ???????? ????????? ?????? ????
        mov     al, [esi+COM_MOUSE_DATA.FirstByte]
        mov     ah, al
        shr     al, 3
        and     al, 2
        shr     ah, 5
        and     ah, 1
        add     al, ah
        mov     [BTN_DOWN], al
        mov     [mouse_active], 1
; ????????? ??????????? ?? X ? ?????????? X
        mov     al, [esi+COM_MOUSE_DATA.FirstByte]
        shl     al, 6
        or      al, [esi+COM_MOUSE_DATA.SecondByte]
        call    mouse_acceleration_com
; mouse acceleration
        mov     ecx, [timer_ticks]
        sub     ecx, [esi+COM_MOUSE_DATA.timer_ticks_com]
        cmp     ecx, [mouse_delay]
        ja      @f
        imul    ax, [mouse_speed_factor]
@@:
        add     ax, [MOUSE_X]
; ?????? ?? ?????? ???????? ?? ????? ??? ?????? ??????? ??????
        js      .x1
        cmp     ax, [ScreenWidth]
        jb      .x2
; ?????????? ?????????? X ?? ?????? ???????
        mov     ax, [ScreenWidth]
        jmp     .x2
.x1:
; ?????????? ?????????? X ?? ????? ???????
        xor     eax, eax
.x2:
        mov     [MOUSE_X], ax
; ????????? ??????????? ?? Y ? ?????????? Y
        mov     al, [esi+COM_MOUSE_DATA.FirstByte]
        and     al, 00001100b
        shl     al, 4
        or      al, [esi+COM_MOUSE_DATA.ThirdByte]
        call    mouse_acceleration_com
        add     ax, [MOUSE_Y]
; ?????? ?? ?????? ???????? ?? ??????? ??? ?????? ??????? ??????
        js      .y1
        cmp     ax, [ScreenHeight]
        jb      .y2
; ?????????? ?????????? Y ?? ?????? ???????
        mov     ax, [ScreenHeight]
        jmp     .y2
.y1:
; ?????????? ?????????? Y ?? ??????? ???????
        xor     eax, eax
.y2:
        mov     [MOUSE_Y], ax
        mov     eax, [timer_ticks]
        mov     [esi+COM_MOUSE_DATA.timer_ticks_com], eax
        jmp     .EndMouseInterrupt

.Error:
; ????????? ???? ? ??????? ???????? ?????????? ??
; ????, ???????? ??????? ?????? ?????? ??????
        mov     [esi+COM_MOUSE_DATA.MouseByteNumber],0
.EndMouseInterrupt:
        jmp     ready_for_next_irq

mouse_acceleration_com:
        cbw
        mov     ecx, [timer_ticks]
        sub     ecx, [esi+COM_MOUSE_DATA.timer_ticks_com]
        cmp     ecx, [mouse_delay]
        ja      @f
        imul    ax, [mouse_speed_factor]
@@:
        ret

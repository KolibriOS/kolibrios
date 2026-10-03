; The monitoring chips of older boards, ported from GMON: Winbond W836xx and
; ITE IT87xx at ports 0x295/0x296, ABIT uGuru at 0xE0/0xE4. Their first
; temperature stands in for a CPU without a sensor.

HWM_PORT        = 0x295                 ; index, data at + 1
SIO_PORT        = 0x2E                  ; Super I/O config, data at + 1
UGURU_CMD       = 0xE0
UGURU_DATA      = 0xE4

WB_BANK         = 0x4E                  ; the registers of Winbond: the bank,
WB_VENDOR       = 0x4F                  ; the vendor, 0x80 in WB_BANK: its
WB_ID           = 0x5CA3                ; high byte
WB_TEMP         = 0x50                  ; in bank 1
SIO_CHIP_ID     = 0x20                  ; of the Super I/O, then of ITE
ITE_ID          = 0x87                  ; the high byte
ITE_TEMP        = 0x29
UGURU_TEMP      = 0x2100                ; bank, sensor

; The chip of the board: hwm_read; out: eax = it, 0 if none: the ports
; given back
hwm_init:
        mcall   SF_SET_PORTS, 0, HWM_PORT, HWM_PORT + 1
        test    eax, eax
        jnz     .uguru
        call    wb_probe
        je      .found

        mcall   SF_SET_PORTS, 0, SIO_PORT, SIO_PORT + 1
        test    eax, eax
        jnz     .uguru
        call    ite_probe
        je      .found

.uguru:
        mcall   SF_SET_PORTS, 0, UGURU_CMD, UGURU_DATA
        test    eax, eax
        jnz     .none

        in      al, UGURU_CMD
        test    al, al
        jz      @f
        cmp     al, 0xAC
        jne     .none

@@:
        in      al, UGURU_DATA
        test    al, al
        jz      @f
        cmp     al, 8
        jne     .none

@@:
        mov     eax, uguru_read

.found:
        mov     [hwm_read], eax
        ret

.none:
        mcall   SF_SET_PORTS, 1, HWM_PORT, HWM_PORT + 1
        mcall   SF_SET_PORTS, 1, UGURU_CMD, UGURU_DATA
        xor     eax, eax
        ret

; al = bank (or 0x80: the high byte of the vendor), ah = register; out: eax
wb_get:
        push    eax
        mov     ah, WB_BANK
        call    hwm_index
        pop     eax
        out     dx, al                  ; then hwm_get

; ah = register; out: eax
hwm_get:
        call    hwm_index
        in      al, dx
        movzx   eax, al
        ret

hwm_index:                              ; out: dx = the data port
        mov     dx, HWM_PORT
        mov     al, ah
        out     dx, al
        inc     edx
        ret

; out: ZF = 1 and eax = the read procedure if found
wb_probe:
        mov     ax, WB_VENDOR shl 8     ; a byte at a time
        call    wb_get
        xchg    ebx, eax
        mov     ax, WB_VENDOR shl 8 + 0x80
        call    wb_get
        mov     bh, al

        cmp     bx, WB_ID
        jne     .done

        mov     eax, wb_read
        cmp     eax, eax
.done:
        ret

; out: ZF = 1 and eax = the read procedure if found
ite_probe:
        mov     eax, 0x55550187         ; MB PnP mode
        mov     ecx, 4
@@:
        out     SIO_PORT, al
        shr     eax, 8
        loop    @b

        mov     al, SIO_CHIP_ID
        out     SIO_PORT, al
        in      al, SIO_PORT + 1
        xchg    eax, esi

        mov     al, 2                   ; leave the mode
        out     SIO_PORT, al
        out     SIO_PORT + 1, al
        mcall   SF_SET_PORTS, 1, SIO_PORT, SIO_PORT + 1

        xchg    eax, esi
        cmp     al, ITE_ID
        mov     eax, ite_read
        ret

; out: eax = the first temperature
wb_read:
        mov     ax, WB_TEMP shl 8 + 1
        jmp     wb_get

ite_read:
        mov     ah, ITE_TEMP
        jmp     hwm_get

uguru_read:
        mov     ax, UGURU_TEMP          ; then uguru_sensor

; ax = sensor; out: eax
uguru_sensor:
        push    ebx
        mov     ebx, eax
        mov     al, ah                  ; the bank
        out     UGURU_DATA, al
        mov     ah, 8
        call    .wait

        mov     al, bl
        out     UGURU_CMD, al
        mov     ah, 1
        call    .wait

        in      al, UGURU_CMD
        movzx   ebx, al

        mov     ecx, 1000               ; back to the ready state
.ready:
        in      al, UGURU_DATA
        cmp     al, 8
        je      .done
        xor     eax, eax
        out     UGURU_DATA, al
        mov     ah, 9
        call    .wait
        loop    .ready

.done:
        xchg    eax, ebx
        pop     ebx
        ret

.wait:                                  ; for ah at the data port
        push    ecx
        mov     ecx, 1000
@@:
        in      al, UGURU_DATA
        cmp     al, ah
        loopne  @b
        pop     ecx
        ret

hwm_read        dd      nothing         ; the procedure of the chip

uglobal
align 4
hwm_temp        dd      ?               ; degrees
endg

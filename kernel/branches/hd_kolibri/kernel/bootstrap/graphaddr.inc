; GRAPHICS ADDRESSES

        ;mov     eax,0x100000*8                    ; LFB address
        ;cmp     [0xfe0c],word 0x13
        ;je      no_d_lfb
        ;cmp     [0xfe0c],word 0x12
        ;je      no_d_lfb
        ;cmp     [0x2f0000+0x901e],byte 1
        ;jne     no_d_lfb
        mov     byte [0x2f0000+0x901e],0x0
        mov     eax,[0x2f0000+0x9018]
      ;no_d_lfb:
        mov     [LFBAddress],eax

        cmp     [SCR_MODE],word 0100000000000000b
        jge     setvesa20
        cmp     [SCR_MODE],word 0x13
        je      v20ga32
        mov     [PUTPIXEL],dword Vesa12_putpixel24  ; Vesa 1.2
        mov     [0xe024],dword Vesa12_getpixel24
        cmp     [ScreenBPP],byte 24
        jz      ga24
        mov     [PUTPIXEL],dword Vesa12_putpixel32
        mov     [0xe024],dword Vesa12_getpixel32
      ga24:
        jmp     v20ga24
      setvesa20:
        mov     [PUTPIXEL],dword Vesa20_putpixel24  ; Vesa 2.0
        mov     [0xe024],dword Vesa20_getpixel24
        cmp     [ScreenBPP],byte 24
        jz      v20ga24
      v20ga32:
        mov     [PUTPIXEL],dword Vesa20_putpixel32
        mov     [0xe024],dword Vesa20_getpixel32
      v20ga24:
        cmp     [SCR_MODE],word 0x12                ; 16 C VGA 640x480
        jne     no_mode_0x12
        mov     [PUTPIXEL],dword VGA_putpixel
        mov     [0xe024],dword Vesa20_getpixel32
      no_mode_0x12:

           call test_cpu
;           btr [cpu_caps], CAPS_SSE    ;test: dont't use sse code
;           btr [cpu_caps], CAPS_SSE2   ;test: don't use sse2

;           btr [cpu_caps], CAPS_FXSR   ;test: disable sse support
                                        ;all sse commands rise #UD exption
;           btr [cpu_caps], CAPS_PSE    ;test: don't use large pages
;           btr [cpu_caps], CAPS_PGE    ;test: don't use global pages
;           btr [cpu_caps], CAPS_MTRR   ;test: don't use MTRR
           bts [cpu_caps], CAPS_TSC     ;force use rdtsc

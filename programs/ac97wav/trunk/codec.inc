
NAMBAR_read_byte:
        add     dx, [NAMBAR]
        in      al, dx
ret


NAMBAR_read_word:
        add     dx, [NAMBAR]
        in      ax, dx
ret


NAMBAR_read_dword:
        add     dx, [NAMBAR]
        in      eax, dx
ret

NAMBAR_write_byte:
        add     dx, [NAMBAR]
        out     dx, al
ret

NAMBAR_write_word:
        add     dx, [NAMBAR]
        out     dx, ax
ret

NAMBAR_write_dword:
        add     dx, [NAMBAR]
        out     dx, eax
ret



NABMBAR_read_byte:
        add     dx, [NABMBAR]
        in      al, dx
ret

NABMBAR_read_word:
        add     dx, [NABMBAR]
        in      ax, dx
ret

NABMBAR_read_dword:
        add     dx, [NABMBAR]
        in      eax, dx
ret

NABMBAR_write_byte:
        add     dx, [NABMBAR]
        out     dx, al
ret

NABMBAR_write_word:
        add     dx, [NABMBAR]
        out     dx, ax
ret

NABMBAR_write_dword:
        add     dx, [NABMBAR]
        out     dx, eax
ret



semaphore:
        push    ecx edx

        mov     edx, GLOB_STS_REG               ; 0x30 global status register
        call    NABMBAR_read_dword
        and     eax, PRI_CODEC_RDY              ; 100h primary codec ready
        jz      .success                        ; exit if codec not ready !!!

;       mov     ecx, 1024                       ; try 1024 times
        mov     ecx, 0ffffh                     ; try 65535 times
  .wait:
        mov     edx, ACC_SEMA_REG               ; 0x34 codec write semaphore
        call    NABMBAR_read_byte
        and     al, CODEC_BUSY                  ; 01h codec access semaphore
        jz      .success                        ; exit if codec not busy !!!

        dec     ecx
        jnz     .wait

        pop     edx ecx
        mov     eax, 0
        jmp     .exit

  .success:
        pop     edx ecx
        mov     eax, 1
  .exit:
ret





codecStop:
        push    eax ebx edx

        mov     edx, PO_CR_REG          ; 0x1B control register
        mov     al, 0                   ; stop all PCM out data
        call    NABMBAR_write_byte

        mcall   MF_DELAY, eax           ; ebx = (eax = MF_DELAY = 5); wait 50 ms

        mov     edx, PO_CR_REG          ; 0x1B control register
        mov     al, RR                  ; reset PCM out regs
        call    NABMBAR_write_byte

        mcall   MF_DELAY, eax

        pop     edx ebx eax
ret




; set voulme
; in ax = volume level
setVolume:
        push     eax edx

        push     eax
        call     semaphore
        mov      edx, CODEC_RESET_REG           ; 0
        xor      eax, eax                       ; register reset the codec
        call     NAMBAR_write_word

        call     semaphore
        pop      eax
        imul     ax, 0101h                      ; set volume for both chn
        mov      edx, CODEC_MASTER_VOL_REG      ; 2
        call     NAMBAR_write_word
        push     eax

        call     semaphore
        pop      eax                            ; set volume for both chn
        mov      edx, CODEC_HP_VOL_REG          ; 4
        call     NAMBAR_write_word
        push     eax

        call     semaphore
        mov      edx, CODEC_CD_VOL_REG          ; 12h
        pop      eax                            ; set volume for both chn
        shr      eax, 2                         ; adjust CD VOL
        call     NAMBAR_write_word

        call     semaphore
        mov      edx, CODEC_PCM_OUT_REG         ; 18h
        mov      ax, 0808h                      ; standard PCM out volume
        call     NAMBAR_write_word

        pop      edx eax
ret




samplerate dw 0



; enable codec, unmute stuff, set output to desired rate
; in : ax = desired sample rate
; out: ax = true or false
;
codecConfig:
        pushad
        mov [samplerate], ax                    ; save sample rate


;        mov     edx, GLOB_STS_REG               ; 30h global status register
;        call    NABMBAR_read_dword
;        and     eax, PRI_CODEC_RDY              ; 0100h primary codec ready
;        jnz     skip_init                       ; skip init if codec ready !!!

        ; stop the codec if currently playing
;;;        call    codecStop

;        mov     edx, GLOB_STS_REG
;        call    NABMBAR_read_dword
;        dps     "GLOB_STA = "
;        dph     eax
;        newline

;        mov     edx, GLOB_CNT_REG
;        call    NABMBAR_read_dword
;        dps     "GLOB_CNT = "
;        dph     eax
;        newline

;        mcall 5, 10

;;        test    eax, ACCOLD_RESET
;;        jnz     .skip_cold_reset

;        print   "cold reset"
        ; do a cold reset
        mov     edx, GLOB_CNT_REG               ; 2ch global control register
        xor     eax, eax
        call    NABMBAR_write_dword             ; enable (AC Link off clear)

;        print   "wait"
        mcall   5, 5
;        print   "alive!"

;;    .skip_cold_reset:

        mov     edx, GLOB_CNT_REG               ; 2ch global control register
        mov     eax, ACCOLD_RESET + PRI_RES_EN  ; cold reset + primary resume
        call    NABMBAR_write_dword             ; 2 channels & 16 bit samples

        mov     edx, GLOB_CNT_REG               ; 2ch global control register
        call    NABMBAR_read_dword
        and     eax, ACCOLD_RESET               ; cold reset
        jz      init_error                      ; INIT FAILED !!!

;        print   "cold reset finished"

        ; wait for primary codec ready status
        mov     ecx, 128
codec_ready_loop:
        mov     edx, GLOB_STS_REG               ; 30h global status register
        call    NABMBAR_read_dword
        and     eax, PRI_CODEC_RDY              ; 0100h primary codec ready
        jnz     codec_ready_exit                ; move on if codec ready !!!
        mcall   5, 1
        dec     ecx
        jnz     codec_ready_loop
        ;dps "~"
codec_ready_exit:

        ; wait until codec init ready (*** replaces warm reset wait ***)
        mcall  5, 60

        ; test if codec ready bit is finally set
        mov     edx, GLOB_STS_REG               ; 30h global status register
        call    NABMBAR_read_dword
        and     eax, PRI_CODEC_RDY              ; 0100h primary codec ready
        jnz     codec_ready_bit_set             ; move on if codec ready !!!
        cmp     [AC97ICH4], 1
        jne     init_error
;       je      codec_ready_bit_set             ; ignore codec ready for ICH4
;       jmp     init_error                      ; codec ready bit not set !!!
codec_ready_bit_set:

        ; clear semaphore flag
        mov     edx, CODEC_RESET_REG            ; 0h codec reset register
        call    NAMBAR_read_word


        ; check if codec sections ready
        call    semaphore
        test    eax, eax
        jz      init_error
        mov     edx, CODEC_POWER_CTRL_REG       ; 26h codec powerdown ctrl
        call    NAMBAR_read_word
        and     eax, 01111b
        cmp     eax, 01111b
        jne     init_error                      ; codec sections not ready



        ; disable interrupts
        mov     al, 0

        mov     edx, PI_CR_REG                  ; 0Bh PCM in control register
        call    NABMBAR_write_byte

        mov     edx, PO_CR_REG                  ; 1Bh PCM out control register
        call    NABMBAR_write_byte

        mov     edx, MC_CR_REG                  ; 2Bh MIC in control register
        call    NABMBAR_write_byte

        ; reset channels
        mov     al, RR                          ; 02h reset Bus master regs

        mov     edx, PI_CR_REG                  ; 0Bh PCM in control register
        call    NABMBAR_write_byte

        mov     edx, PO_CR_REG                  ; 1Bh PCM out control register
        call    NABMBAR_write_byte

        mov     edx, MC_CR_REG                  ; 2Bh MIC in control register
        call    NABMBAR_write_byte

        ; set default volume
        mov     eax, 15                         ; set average volume level
        call    setVolume

        ; set VRA and clear DRA (if not supported will be skipped)
        call    semaphore
        test    eax, eax
        jz      init_error
        mov     edx, CODEC_EXT_AUDIO_CTRL_REG   ; register 2ah
        call    NAMBAR_read_word                ; get ext audio ctl

        mov     ebx, eax
        call    semaphore
        test    eax, eax
        jz      init_error
        mov     eax, ebx
        and     eax, 0FFFFh - BIT1              ; clear DRA (BIT1)
        or      eax, BIT0                       ; set VRA (BIT0)
        mov     edx, CODEC_EXT_AUDIO_CTRL_REG   ; register 2ah
        call    NAMBAR_write_word               ; write ext audio ctl

        ; set desired sample rate
skip_init:
        call    semaphore
        test    eax, eax
        jz      init_error

       ; mov     edx, CODEC_PCM_FRONT_DACRATE_REG
       ; call    NAMBAR_read_word
       ; and     eax, 0xFFFF
       ; newline
       ; dps     "old PCM OUT RATE: "
       ; dpd     eax
       ; newline

        mov     ax, [samplerate]                 ; restore sample rate
;        mov     edx, CODEC_PCM_FRONT_DACRATE_REG ; register 2ch
;        call    NAMBAR_write_word
        call    set_sample_rate

        popad
        mov    eax, 1                           ; exit with success
        jmp    exit_config
init_error:
        popad
        xor    eax, eax                           ; exit with error
exit_config:
        ret

set_sample_rate: ; rate in ax
        mov    edx, CODEC_PCM_FRONT_DACRATE_REG ; 0x2C reg
        call   NAMBAR_write_word
ret

;
;    Ok, this is the sceleton that MENUET 0.01 understands.
;    Do not change the header bits for now. Compile with nasm.
;

include '..\..\..\macros.inc'

use32
 org    0x0
 db     'MENUET01'    ; header
 dd     0x01          ; header version
 dd     START         ; entry point
 dd     I_END         ; image size
 dd     0x1000        ; required memory
 dd     0x1000        ; esp
 dd     0x0 , 0x0     ; I_Param , I_Path


MIDI_PORT = 0x330

midisp: dw      MIDI_PORT + 1
mididp: dw      MIDI_PORT


START:
        mov     dx, word[midisp]
        mov     cx, word[mididp]
        mcall   46, 0

        test    eax, eax
        jne     exit


  red:
    call  draw_window

still:

        mov     eax, 10          ; redraw ?
        mcall

        cmp     eax, 1
        jz      red
        cmp     eax, 3
        jnz     still

        ;button:
        mov     eax, 17
        mcall

        cmp     al, 0
        jnz  still

        cmp     ah, 1
        jz      exit

        cmp     ah, 2
        jz      .play_note

        ; reset midi device
        call    midi_reset

        cmp     eax, 0    ; check error code
        jz      still

        call    printerror

        jmp     still

.play_note:

        mov     eax, 50
.nn:
        mov     ebx, 100
        call    noteout
        pusha

        mov     eax, 5
        mov     ebx, 8
        mcall

        popa
        mov     ebx, 0
;    call noteout

        add     eax, 3

        mov     ebx, 100
        call    noteout
        pusha

        mov     eax, 5
        mov     ebx, 8
        mcall

        popa
        mov     ebx, 0
;    call noteout

        add     eax, 4

        inc     eax
        cmp     eax, 90
        jbe     .nn

        jmp     still

exit:
        mcall   -1


draw_window:

        pusha

        mov     eax, 12                    ; tell os about redraw
        mov     ebx, 1
        mcall

        mov     eax, 0                     ; define and draw window
        mov     ebx, 20*65536+250
        mov     ecx, 20*65536+120
        mov     edx, 0x14ffffff
        mov     edi, title
        mcall

        mov     eax, 8
        mov     ebx, 10*65536+200          ; button start x & size
        mov     ecx, 40 *65536+17          ; button start y & size
        mov     edx, 2                     ; button number
        mov     esi, 0x4060b0              ; button color
        mcall

        mov     ecx, 60 *65536+17          ; button start y & size
        mov     edx, 3                     ; button number
        mcall

        mov     eax, 4
        mov     ebx, 25*65536+45
        mov     ecx, dword 0xffffff
        mov     edx, buttont
        mov     esi, buttontlen-buttont
        mcall

        mov     ebx, 25*65536+65
        mov     edx, buttont2
        mov     esi, buttontlen2-buttont2
        mcall

        mov     eax, 12                    ; tell os about redraw end
        mov     ebx, 2
        mcall

        popa
        ret


noteout:
        pusha

        push    ebx
        push    eax

        mov     ecx, 0x9f
        call    midi_output_byte

        pop     ecx
        call    midi_output_byte

        pop     ecx
        call    midi_output_byte

        cmp     eax, 0
        jz      @f

        call    printerror

@@:
        popa
        ret

printerror:

        mov     eax, 4
        mov     ebx, 15*65536+85
        mov     ecx, 0x000000
        mov     edx, error1
        mov     esi, errorlen1-error1
        mcall

        mov     eax, 4
        mov     ebx, 15*65536+95
        mov     ecx, 0x000000
        mov     edx, error2
        mov     esi, errorlen2-error2
        mcall

        ret

; MPU401 interface

midi_reset:
@@:
        ; reset device
        call    is_output
        test    al, al
        jnz     @b
        mov     dx, word [midisp]
        mov     al, 0xff
        out     dx, al
@@:
        mov     dx, word [midisp]
        mov     al, 0xff
        out     dx, al
        call    is_input
        test    al, al
        jnz     @b
        call    get_mpu_in
        cmp     al, 0xfe
        jnz     @b
@@:
        call    is_output
        test    al, al
        jnz     @b
        mov     dx, word [midisp]
        mov     al, 0x3f
        out     dx, al
        ret

midi_output_byte:
        ; output byte
@@:
        call    get_mpu_in
        call    is_output
        test    al, al
        jnz     @b
        mov     al, cl
        call    put_mpu_out
        ret

is_input:
        push    edx
        mov     dx, word [midisp]
        in      al, dx
        and     al, 0x80
        pop     edx
        ret

is_output:
        push    edx
        mov     dx, word [midisp]
        in      al, dx
        and     al, 0x40
        pop     edx
        ret

get_mpu_in:
        push    edx
        mov     dx, word [mididp]
        in      al, dx
        pop     edx
        ret

put_mpu_out:
        push    edx
        mov     dx, word [mididp]
        out     dx, al
        pop     edx
        ret





; DATA AREA


title    db   'MIDI TEST',0

buttont:
    db   'PLAY A FEW NOTES'
buttontlen:
buttont2:
    db   'RESET MIDI DEVICE'
buttontlen2:

error1:
    db   'NO BASE DEFINED FOR MPU-401'
errorlen1:

error2:
    db   'USE SETUP AND RESET MIDI DEVICE.'
errorlen2:

base db 0x0

I_END:




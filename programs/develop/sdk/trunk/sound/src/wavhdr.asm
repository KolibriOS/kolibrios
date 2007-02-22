format MS COFF

include "snd.inc"
include "proc32.inc"

section '.text' align 16 code readable executable

public _test_wav@4

; convert WAVEHEADER into PCM_x_xx_xx constant

align 4
proc _test_wav@4 stdcall, hdr:dword

           mov eax, [hdr]
           cmp dword [eax], 0x46464952
           jne .fail

           cmp dword [eax+8], 0x45564157
           jne .fail

           cmp word [eax+20], 1
           jne .fail

           mov ecx, dword [eax+24]
           mov edx, 22050
           cmp ecx, edx
           ja .high
           je .l_22

           cmp ecx, 8000
           je .l_8

           cmp ecx, 11025
           je .l_11

           cmp ecx, 12000
           je .l_12

           cmp ecx, 16000
           je .l_16
.fail:
           xor eax, eax
           ret
.high:
           cmp ecx, 24000
           je .LN56
           cmp ecx, 32000
           je .LN65
           cmp ecx, 44100
           je .LN74
           cmp ecx, 48000
           jne .fail

           movzx ecx, word [eax+22]
           dec ecx
           je .LN79
           dec ecx
           jne .LN74

           mov edx, 19
           jmp .done
.LN79:
           mov edx, 20
           jmp .done
.LN74:
           movzx ecx, word [eax+22]
           dec ecx
           je .LN70
           dec ecx
           jne .LN65

           mov edx, 21
           jmp .done
.LN70:
           mov edx, 22
           jmp .done
.LN65:
           movzx ecx, word [eax+22]
           dec ecx
           je .LN61
           dec ecx
           jne .LN56

           mov edx, 23
           jmp .done
.LN61:
           mov edx, 24
           jmp .done
.LN56:
           movzx ecx, word [eax+22]
           dec ecx
           je .LN52
           dec ecx
           je .LN50
.l_22:
           movzx ecx, word [eax+22]
           dec ecx
           je .LN43
           dec ecx
           je .LN41
.l_16:
           movzx ecx, word [eax+22]
           dec ecx
           je .LN34
           dec ecx
           je .LN32
.l_12:
           movzx ecx, word [eax+22]
           dec ecx
           je .LN25
           dec ecx
           je .LN23
.l_11:
           movzx ecx, word [eax+22]
           dec ecx
           je .LN16
           dec ecx
           je .LN14
.l_8:
           movzx ecx, word [eax+22]
           dec ecx
           je .LN7
           dec ecx
           jne .fail

           mov edx, 35
           jmp .done
.LN7:
           mov edx, 36
           jmp .done
.LN14:
           mov edx, 33
           jmp .done
.LN16:
           mov edx, 34
           jmp .done
.LN23:
           mov edx, 31
           jmp .done
.LN25:
           mov edx, 32
           jmp .done
.LN32:
           mov edx, 29
           jmp .done
.LN34:
           mov edx, 30
           jmp .done
.LN41:
           mov edx, 27
           jmp .done
.LN43:
           mov edx, 28
           jmp .done
.LN50:
           mov edx, 25
           jmp .done
.LN52:
           mov edx, 26
.done:
           xor ecx, ecx
           cmp word [eax+34], 16
           setne cl
           dec ecx
           and ecx, -18
           add ecx, edx
           mov eax, ecx
           ret
endp

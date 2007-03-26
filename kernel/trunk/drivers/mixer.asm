;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;   (C) copyright Serge 2006
;   email: infinity_sound@mail.ru



align 4

mix_list rq 32

align 4
proc new_mix stdcall, output:dword
           locals
             main_count   rd 1
             fpu_state    rb 528   ;512+16
           endl

           mov [main_count], 32
           call prepare_playlist
           cmp [play_count], 0
           je .clear

           lea eax, [fpu_state+16]
           and eax, -16             ;must be 16b aligned
           call FpuSave

           call update_stream
.mix:
           lea eax, [mix_list]
           call do_mix_list
           test eax, eax
           je .done

  ;         cmp eax, 1
  ;         je .copy

           lea ebx, [mix_list]
           stdcall mix_all, [output], ebx, eax
@@:
           add [output], 512
           dec [main_count]
           jnz .mix
.exit:
           lea eax, [fpu_state+16]
           and eax, -16
           call FpuRestore
           ret
.copy:
           lea eax, [mix_list]
           stdcall copy_mem, [output], [eax]
           jmp @B
.done:
           mov ecx, [main_count]
           shl ecx, 7     ;ecx*= 512/4

           mov edi, [output]
           xor eax, eax
           cld
           rep stosd
           jmp .exit
.clear:
           mov edi, [output]
           mov ecx, 4096
           xor eax, eax
           cld
           rep stosd
           ret
endp

align 4
proc update_stream
           locals
             stream_index  dd ?
             ev_code       dd ?  ;EVENT
             ev_offs       dd ?
                           rd 4
           endl

           mov [stream_index], 0
.l1:
           mov edx, [stream_index]
           mov esi, [play_list+edx*4]

           mov eax, [esi+STREAM.out_rp]
           cmp eax, [esi+STREAM.out_top]
           jb @f
           sub eax, 64*1024
@@:
           mov [esi+STREAM.out_rp], eax

           cmp word [esi+STREAM.format], PCM_2_16_48
           je .copy

           cmp [esi+STREAM.out_count], 16384
           ja .skip

           test [esi+STREAM.format], PCM_RING
           jnz .ring

           stdcall refill, esi
.skip:
           inc [stream_index]
           dec [play_count]
           jnz .l1
           ret

.ring:
           stdcall refill_ring, esi
           jmp .skip
.copy:
           mov ebx, esi
           mov edi, [ebx+STREAM.out_wp]
           cmp edi, [ebx+STREAM.out_top]
           jb @f

           sub edi, 64*1024
           mov [ebx+STREAM.out_wp], edi
@@:
           mov esi, [ebx+STREAM.in_rp]
           mov ecx, 16384/4
           cld
           rep movsd

           mov [ebx+STREAM.out_wp], edi

           cmp esi, [ebx+STREAM.in_top]
           jb @f

           sub esi, 0x10000
@@:
           mov [ebx+STREAM.in_rp], esi

           test eax, eax
           jz .l_end
           mov eax, [ebx+STREAM.notify_event]
           mov ebx, [ebx+STREAM.notify_id]
           mov ecx, EVENT_WATCHED
           xor edx, edx
           call RaiseEvent     ;eax, ebx, ecx, edx
.l_end:
           inc [stream_index]
           dec [play_count]
           jnz .l1
           ret
endp

align 4
proc refill stdcall, str:dword
           locals
             r_size    rd 1
             event     rd 6
           endl

           mov ebx, [str]
           mov edi, [ebx+STREAM.out_wp]
           cmp edi, [ebx+STREAM.out_top]
           jb @F
           sub edi, 0x10000
           mov [ebx+STREAM.out_wp], edi
@@:
           mov eax, [ebx+STREAM.in_count]
           test eax, eax
           jz .done

           mov ecx, [ebx+STREAM.r_size]
           cmp eax, ecx
           jle @F

           mov eax, ecx
@@:
           mov ecx, eax
           cmp word [ebx+STREAM.format], PCM_1_16_8
           ja @F

           shr eax, 1                   ;two channles
@@:
           test [ebx+STREAM.format], 1  ;even formats mono
           jz @F

           shr eax, 1                   ;eax= samples
@@:
           shl eax, 15    ;eax*=32768 =r_end

           mov [r_size], ecx

           mov esi, [ebx+STREAM.in_rp]
           mov edi, [ebx+STREAM.out_wp]

           stdcall [ebx+STREAM.resample], edi, esi, \
           [ebx+STREAM.r_dt], ecx, eax

           mov ebx, [str]

           add [ebx+STREAM.out_count], eax;
           add [ebx+STREAM.out_wp], eax;

           mov eax, [ebx+STREAM.in_rp]
           mov ecx, [r_size]
           add eax, ecx
           add [ebx+STREAM.in_free], ecx
           sub [ebx+STREAM.in_count], ecx

           cmp eax, [ebx+STREAM.in_top]
           jb @f

           sub eax, [ebx+STREAM.in_size]
@@:
           mov [ebx+STREAM.in_rp], eax

.done:
           mov eax, [ebx+STREAM.notify_event]
           test eax, eax
           jz .exit

           mov ebx, [ebx+STREAM.notify_id]
           mov ecx, EVENT_WATCHED
           xor edx, edx
           call RaiseEvent   ;eax, ebx, ecx, edx
.exit:
           ret
endp

align 4
proc refill_ring stdcall, str:dword
           locals
             event     rd 6
           endl

           mov ebx, [str]
           mov edi, [ebx+STREAM.out_wp]
           cmp edi, [ebx+STREAM.out_top]
           jb @F
           sub edi, 0x10000
           mov [ebx+STREAM.out_wp], edi
@@:
           mov ecx, [ebx+STREAM.r_size]
           mov eax, ecx
           cmp word [ebx+STREAM.format], PCM_1_16_8
           ja @F

           shr eax, 1                   ;two channles
@@:
           test [ebx+STREAM.format], 1  ;even formats mono
           jz @F

           shr eax, 1                   ;eax= samples
@@:
           shl eax, 15    ;eax*=32768 =r_end

           mov esi, [ebx+STREAM.in_rp]
           mov edi, [ebx+STREAM.out_wp]

           stdcall [ebx+STREAM.resample], edi, esi, \
           [ebx+STREAM.r_dt], ecx, eax

           mov ebx, [str]

           add [ebx+STREAM.out_count], eax;
           add [ebx+STREAM.out_wp], eax;

           mov eax, [ebx+STREAM.in_rp]
           mov ecx, [ebx+STREAM.r_size]
           add eax, ecx
           add [ebx+STREAM.in_free], ecx
           sub [ebx+STREAM.in_count], ecx

           cmp eax, [ebx+STREAM.in_top]
           jb @f

           sub eax, [ebx+STREAM.in_size]
@@:
           mov [ebx+STREAM.in_rp], eax

           sub eax, [ebx+STREAM.in_base]
           sub eax, 128
           lea edx, [event]

           mov dword [edx], RT_INP_EMPTY
           mov dword [edx+4], 0
           mov dword [edx+8], ebx
           mov dword [edx+12], eax

           mov eax, [ebx+STREAM.notify_event]
           test eax, eax
           jz .exit

           mov ebx, [ebx+STREAM.notify_id]
           xor ecx, ecx
           call RaiseEvent   ;eax, ebx, ecx, edx
.exit:
           ret
endp

align 4
proc mix_all stdcall, dest:dword, list:dword, count:dword

           mov edi, [dest]
           mov ebx, 64
.mix:
           mov edx, [list]
           mov ecx, [count]

           mov eax, [edx]
           movq mm0, [eax]
           movd mm1, [edx+4]
           punpckldq mm1,mm1
           pmulhw mm0, mm1
           psllw  mm0, 1

.mix_loop:
           add dword [edx], 8
           add edx, 8
           dec ecx
           jz @F

           mov eax, [edx]
           movq mm1, [eax]
           movd mm2, [edx+4]
           punpckldq mm2,mm2
           pmulhw mm1, mm2
           psllw  mm1, 1
           paddsw mm0, mm1
           jmp .mix_loop
@@:
           movq [edi], mm0
           add edi, 8
           dec ebx
           jnz .mix

           ret
endp

align 4
proc resample_1 stdcall, dest:dword,src:dword,\
                       r_dt:dword, r_size:dword,r_end:dword

; dest equ esp+8
; src  equ esp+12
; r_dt equ esp+16
; r_size equ esp+20
; r_end equ esp+24

           mov edi, [dest]
           mov edx, [src]
           sub edx, 32*2
           mov eax, 16

align 4
.l1:
           mov ecx, eax
           mov esi, eax
           and ecx, 0x7FFF
           shr esi, 15
           lea esi, [edx+esi*2]

           movsx ebp, word [esi]
           movsx esi, word [esi+2]
           mov ebx, 32768
           imul esi, ecx
           sub ebx, ecx
           imul ebx, ebp
           lea ecx, [ebx+esi+16384]
           sar ecx, 15
           cmp ecx, 32767         ; 00007fffH
           jle @f
           mov ecx, 32767         ; 00007fffH
           jmp .write
@@:
           cmp ecx, -32768        ; ffff8000H
           jge .write
           mov ecx, -32768        ; ffff8000H
.write:
           mov ebx, ecx
           shl ebx, 16
           mov bx, cx
           mov [edi], ebx
           add edi, 4

           add eax, [esp+16]
           cmp eax, [esp+24]
           jb .l1

           mov ebp, esp

           sub edi, [dest]
           mov eax, edi
           ret
endp

align 4
proc resample_18 stdcall, dest:dword,src:dword,\
                       r_dt:dword, r_size:dword,r_end:dword


           mov edi, [dest]
           mov edx, [src]
           sub edx, 32

           mov esi, 16

align 4
.l1:
           mov ecx, esi
           mov eax, esi
           and ecx, 0x7FFF
           shr eax, 15
           lea eax, [edx+eax]

           mov bx, word [eax]
           sub bh, 0x80
           sub bl, 0x80
           movsx eax, bh
           shl eax,8
           movsx ebp, bl
           shl ebp,8
           mov ebx, 32768
           imul eax, ecx
           sub ebx, ecx
           imul ebx, ebp
           lea ecx, [ebx+eax+16384]
           sar ecx, 15
           cmp ecx, 32767         ; 00007fffH
           jle @f
           mov ecx, 32767         ; 00007fffH
           jmp .write
@@:
           cmp ecx, -32768        ; ffff8000H
           jge .write
           mov ecx, -32768        ; ffff8000H
.write:
           mov ebx, ecx
           shl ebx, 16
           mov bx, cx
           mov [edi], ebx
           add edi, 4

           add esi, [esp+16]
           cmp esi, [esp+24]
           jb .l1

           mov ebp, esp
           sub edi, [dest]
           mov eax, edi
           ret
endp

align 4
proc copy_stream stdcall, dest:dword,src:dword,\
                       r_dt:dword, r_size:dword,r_end:dword

           mov ecx, [r_size]
           mov eax, ecx
           shr ecx, 2
           mov esi, [src]
           mov edi, [dest]
           rep movsd
           mov eax, 16384
           ret
endp

align 4
proc resample_2 stdcall, dest:dword,src:dword,\
                       r_dt:dword, r_size:dword,r_end:dword

           mov edx, [src]
           sub edx, 32*4
           mov edi, [dest]
           mov ebx, [r_dt]
           mov eax, 16
           emms

align 4
.l1:
           mov ecx, eax
           mov esi, eax
           and ecx, 0x7FFF
           shr esi, 15
           lea esi, [edx+esi*4]

           movq mm0, [esi]
           movq mm1, mm0

           movd mm2, ecx
           punpcklwd mm2, mm2
           movq mm3, qword [m7]    ;0x8000

           psubw mm3, mm2 ;        ;0x8000 - iconst
           punpckldq mm3, mm2

           pmulhw mm0, mm3
           pmullw mm1, mm3

           movq mm4, mm1
           punpcklwd mm1, mm0
           punpckhwd mm4, mm0
           paddd mm1, mm4
           psrad  mm1, 15
           packssdw mm1, mm1
           movd [edi], mm1
           add edi, 4

           add eax, ebx
           cmp eax, [r_end]
           jb .l1
           emms

           sub edi, [dest]
           mov eax, edi
           ret
endp

align 4
proc resample_28 stdcall, dest:dword,src:dword,\
                       r_dt:dword, r_size:dword,r_end:dword

           mov edx, [src]
           sub edx, 32*2
           mov edi, [dest]
           mov ebx, [r_dt]
           mov eax, 16
           emms
           movq mm7,[mm80]
           movq mm6,[mm_mask]

align 4
.l1:
           mov ecx, eax
           mov esi, eax
           and ecx, 0x7FFF
           shr esi, 15
           lea esi, [edx+esi*2]

           movq mm0, [esi]
           psubb mm0,mm7
           punpcklbw mm0,mm0
           pand mm0,mm6

           movq mm1, mm0

           movd mm2, ecx
           punpcklwd mm2, mm2
           movq mm3, qword [m7] ;                  // 0x8000

           psubw mm3, mm2       ;         // 0x8000 - iconst
           punpckldq mm3, mm2

           pmulhw mm0, mm3
           pmullw mm1, mm3

           movq mm4, mm1
           punpcklwd mm1, mm0
           punpckhwd mm4, mm0
           paddd mm1, mm4
           psrad  mm1, 15
           packssdw mm1, mm1
           movd [edi], mm1
           add edi, 4

           add eax, ebx
           cmp eax, [r_end]
           jb .l1
           emms


           sub edi, [dest]
           mov eax, edi
           ret
endp


proc m16_stereo stdcall, dest:dword,src:dword,\
                       r_dt:dword, r_size:dword,r_end:dword

           mov esi, [src]
           mov edi, [dest]
           mov ecx, [r_size]
           shr ecx,8
@@:
           call m16_s_mmx
           add edi, 128
           add esi, 64
           call m16_s_mmx
           add edi, 128
           add esi, 64
           call m16_s_mmx
           add edi, 128
           add esi, 64
           call m16_s_mmx
           add edi, 128
           add esi, 64
           dec ecx
           jnz @b

           mov eax, [r_size]
           add eax, eax
           ret
endp

align 4
proc s8_stereo stdcall, dest:dword,src:dword,\
                       r_dt:dword, r_size:dword,r_end:dword

           mov esi, [src]
           mov edi, [dest]
           mov ecx, [r_size]
           shr ecx, 7

           movq mm7, [mm80]
           movq mm6, [mm_mask]
@@:
           call s8_s_mmx
           add edi, 64
           add esi, 32
           call s8_s_mmx
           add edi, 64
           add esi, 32
           call s8_s_mmx
           add edi, 64
           add esi, 32
           call s8_s_mmx
           add edi, 64
           add esi, 32
           dec ecx
           jnz @b

           mov eax, [r_size]
           add eax, eax
           ret
endp

proc m8_stereo stdcall, dest:dword,src:dword,\
                       r_dt:dword, r_size:dword,r_end:dword

           mov esi, [src]
           mov edi, [dest]
           mov ecx, [r_size]
           shr ecx, 6

           movq mm7, [mm80]
           movq mm6, [mm_mask]
@@:
           call m8_s_mmx
           add edi, 64
           add esi, 16
           call m8_s_mmx
           add edi, 64
           add esi, 16
           call m8_s_mmx
           add edi, 64
           add esi, 16
           call m8_s_mmx
           add edi, 64
           add esi, 16
                  dec ecx
           jnz @b

           mov eax, [r_size]
           add eax, eax
           add eax, eax
           ret
endp

align 4
proc alloc_mix_buff

           bsf eax, [mix_buff_map]
           jnz .find
           xor eax, eax
           ret
.find:
           btr [mix_buff_map], eax
           shl eax, 9
           add eax, [mix_buff]
           ret
endp

align 4
proc m16_s_mmx

           movq    mm0, [esi]
           movq    mm1, mm0
           punpcklwd mm0, mm0
           punpckhwd mm1, mm1
           movq    [edi], mm0
           movq    [edi+8], mm1

           movq    mm0, [esi+8]
           movq    mm1, mm0
           punpcklwd mm0, mm0
           punpckhwd mm1, mm1
           movq    [edi+16], mm0
           movq    [edi+24], mm1

           movq    mm0, [esi+16]
           movq    mm1, mm0
           punpcklwd mm0, mm0
           punpckhwd mm1, mm1
           movq    [edi+32], mm0
           movq    [edi+40], mm1

           movq    mm0, [esi+24]
           movq    mm1, mm0
           punpcklwd mm0, mm0
           punpckhwd mm1, mm1
           movq    [edi+48], mm0
           movq    [edi+56], mm1

           movq    mm0, [esi+32]
           movq    mm1, mm0
           punpcklwd mm0, mm0
           punpckhwd mm1, mm1
           movq    [edi+64], mm0
           movq    [edi+72], mm1

           movq    mm0, [esi+40]
           movq    mm1, mm0
           punpcklwd mm0, mm0
           punpckhwd mm1, mm1
           movq    [edi+80], mm0
           movq    [edi+88], mm1


           movq    mm0, [esi+48]
           movq    mm1, mm0
           punpcklwd mm0, mm0
           punpckhwd mm1, mm1
           movq    [edi+96], mm0
           movq    [edi+104], mm1

           movq    mm0, [esi+56]
           movq    mm1, mm0
           punpcklwd mm0, mm0
           punpckhwd mm1, mm1
           movq    [edi+112], mm0
           movq    [edi+120], mm1

           ret
endp

align 4
proc s8_s_mmx

           movq    mm0, [esi]
           psubb   mm0, mm7
           movq    mm1, mm0
           punpcklbw mm0, mm0
           pand mm0, mm6
           punpckhbw mm1, mm1
           pand mm1, mm6
           movq    [edi], mm0
           movq    [edi+8], mm1

           movq    mm0, [esi+8]
           psubb   mm0, mm7
           movq    mm1, mm0
           punpcklbw mm0, mm0
           pand mm0, mm6
           punpckhbw mm1, mm1
           pand mm1, mm6
           movq    [edi+16], mm0
           movq    [edi+24], mm1

           movq    mm0, [esi+16]
           psubb   mm0, mm7
           movq    mm1, mm0
           punpcklbw mm0, mm0
           pand mm0, mm6
           punpckhbw mm1, mm1
           pand mm1, mm6
           movq    [edi+32], mm0
           movq    [edi+40], mm1

           movq    mm0, [esi+24]
           psubb   mm0, mm7
           movq    mm1, mm0
           punpcklbw mm0, mm0
           pand    mm0, mm6
           punpckhbw mm1, mm1
           pand    mm1, mm6
           movq    [edi+48], mm0
           movq    [edi+56], mm1

           ret

endp

align 4
proc m8_s_mmx

           movq    mm0, [esi]
           psubb   mm0, mm7
           movq    mm1, mm0
           punpcklbw mm0, mm0
           pand mm0, mm6
           punpckhbw mm1, mm1
           pand mm1, mm6
           movq mm2, mm0
           punpcklwd mm0, mm0
           punpckhwd mm2, mm2

           movq mm3, mm1
           punpcklwd mm1, mm1
           punpckhwd mm3, mm3

           movq    [edi], mm0
           movq    [edi+8], mm2
           movq    [edi+16], mm1
           movq    [edi+24], mm3

           movq    mm0, [esi+8]
           psubb   mm0, mm7
           movq    mm1, mm0
           punpcklbw mm0, mm0
           pand mm0, mm6
           punpckhbw mm1, mm1
           pand mm1, mm6
           movq mm2, mm0
           punpcklwd mm0, mm0
           punpckhwd mm2, mm2

           movq mm3, mm1
           punpcklwd mm1, mm1
           punpckhwd mm3, mm3

           movq    [edi+32], mm0
           movq    [edi+40], mm2
           movq    [edi+48], mm1
           movq    [edi+56], mm3

           ret
endp

align 4
proc mix_2_1 stdcall, output:dword, str0:dword, str1:dword

           mov edi, [output]
           mov eax, [str0]
           mov ebx, [str1]
           mov esi, 128
           call [mix_2_core]   ;edi, eax, ebx

           add edi, esi
           add eax, esi
           add ebx, esi
           call [mix_2_core]   ;edi, eax, ebx

           add edi, esi
           add eax, esi
           add ebx, esi
           call [mix_2_core]   ;edi, eax, ebx

           add edi, esi
           add eax, esi
           add ebx, esi
           call [mix_2_core]   ;edi, eax, ebx
           ret
endp

align 4
proc mix_3_1 stdcall, output:dword, str0:dword, str1:dword, str2:dword

           mov edi, [output]
           mov eax, [str0]
           mov ebx, [str1]
           mov ecx, [str2]
           mov esi, 128
           call [mix_3_core]

           add edi, esi
           add eax, esi
           add ebx, esi
           add ecx, esi
           call [mix_3_core]

           add edi, esi
           add eax, esi
           add ebx, esi
           add ecx, esi
           call [mix_3_core]

           add edi, esi
           add eax, esi
           add ebx, esi
           add ecx, esi
           call [mix_3_core]
           ret
endp

align 4
proc mix_4_1 stdcall, str0:dword, str1:dword,\
                      str2:dword, str3:dword

           local output:DWORD

           call alloc_mix_buff
           and eax, eax
           jz .err

           mov [output], eax

           mov edi, eax
           mov eax, [str0]
           mov ebx, [str1]
           mov ecx, [str2]
           mov edx, [str3]
           mov esi, 128
           call [mix_4_core]  ;edi, eax, ebx, ecx, edx

           add edi, esi
           add eax, esi
           add ebx, esi
           add ecx, esi
           add edx, esi
           call [mix_4_core]  ;edi, eax, ebx, ecx, edx

           add edi, esi
           add eax, esi
           add ebx, esi
           add ecx, esi
           add edx, esi
           call [mix_4_core]  ;edi, eax, ebx, ecx, edx

           add edi, esi
           add eax, esi
           add ebx, esi
           add ecx, esi
           add edx, esi
           call [mix_4_core]  ;edi, eax, ebx, ecx, edx
           mov eax, [output]
           ret
.err:
           xor eax, eax
           ret
endp


align 4
proc final_mix stdcall, output:dword, str0:dword, str1:dword,\
                        str2:dword, str3:dword

           mov edi, [output]

           mov eax, [str0]
           mov ebx, [str1]
           mov ecx, [str2]
           mov edx, [str3]
           mov esi, 128
           call [mix_4_core]  ;edi, eax, ebx, ecx, edx

           add edi, esi
           add eax, esi
           add ebx, esi
           add ecx, esi
           add edx, esi
           call [mix_4_core]  ;edi, eax, ebx, ecx, edx

           add edi, esi
           add eax, esi
           add ebx, esi
           add ecx, esi
           add edx, esi
           call [mix_4_core]  ;edi, eax, ebx, ecx, edx

           add edi, esi
           add eax, esi
           add ebx, esi
           add ecx, esi
           add edx, esi
           call [mix_4_core]  ;edi, eax, ebx, ecx, edx
           ret
endp

align 4
proc copy_mem stdcall, output:dword, input:dword

           mov edi, [output]
           mov esi, [input]
           mov ecx, 0x80
.l1:
           mov eax, [esi]
           mov [edi], eax
           add esi, 4
           add edi, 4
           loop .l1

           ret
endp

proc memcpy
@@:
           mov eax, [esi]
           mov [edi], eax
           add esi, 4
           add edi, 4
           dec ecx
           jnz @B
           ret
endp

if 0

align 4
proc new_mix stdcall, output:dword
           locals
             mixCounter  dd ?
             mixIndex  dd ?
             streamIndex dd ?
             inputCount  dd ?
             main_count  dd ?
             blockCount  dd ?
             mix_out  dd ?
           endl

           call prepare_playlist

           cmp [play_count], 0
           je .exit
           call FpuSave
           mov [main_count], 32;
.l00:
           mov [mix_buff_map], 0x0000FFFF;
           xor eax, eax
           mov [mixCounter], eax
           mov [mixIndex],eax
           mov [streamIndex], eax;
           mov ebx, [play_count]
           mov [inputCount], ebx
.l0:
           mov ecx, 4
.l1:
           mov ebx, [streamIndex]
           mov esi, [play_list+ebx*4]
           mov eax, [esi+STREAM.work_read]
           add [esi+STREAM.work_read], 512

           mov ebx, [mixIndex]
           mov [mix_input+ebx*4], eax
           inc [mixCounter]
           inc [mixIndex]
           inc [streamIndex]
           dec [inputCount]
           jz .m2

           dec ecx
           jnz .l1

           cmp [mixCounter], 4
           jnz .m2

           stdcall mix_4_1, [mix_input],[mix_input+4],[mix_input+8],[mix_input+12]
           sub [mixIndex],4
           mov ebx, [mixIndex]
           mov [mix_input+ebx*4], eax
           inc [mixIndex]
           mov [mixCounter], 0

           cmp [inputCount], 0
           jnz .l0
.m2:
           cmp [mixIndex], 1
           jne @f
           stdcall copy_mem, [output], [mix_input]
           jmp .m3
@@:
           cmp [mixIndex], 2
           jne @f
           stdcall mix_2_1, [output], [mix_input], [mix_input+4]
           jmp .m3
@@:
           cmp [mixIndex], 3
           jne @f
           stdcall mix_3_1, [output],[mix_input],[mix_input+4],[mix_input+8]
           jmp .m3
@@:
           stdcall final_mix, [output],[mix_input],[mix_input+4],[mix_input+8], [mix_input+12]
.m3:
           add [output],512

           dec [main_count]
           jnz .l00

           call update_stream
           emms
           call FpuRestore
           ret
.exit:
           mov edi, [output]
           mov ecx, 0x1000
           xor eax, eax
           cld
           rep stosd
           ret
endp

end if


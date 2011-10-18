;
;
CP866 db '€‚ƒ„…†‡‰‹‘’“”•–—™› ΅Ά£¤¥¦§¨©ª«¬­®―ΰαβγδεζηθικλμνξο'

;
;  ¥ΰ¥ª®¤¨ΰ®Άª  ¨§ cp1251 Ά cp866
;  
;  [eax] <-- Null-terminated string
; 

  win2dos:
    pushad
    pushf

    xor ebx, ebx
    xor ecx, ecx
    ;xor edx, edx

  w2d_loop:
    mov bl, [eax+ecx]
    cmp bl, 0
    jz  w2d_end
    cmp bl, 0A8h              ; π
    jz  w2d_yo1
    cmp bl, 0B8h              ; ρ
    jz  w2d_yo2
    cmp bl, 0C0h              ; γααª ο ΅γªΆ 
    jnc w2d_rchar
    inc ecx
    jmp w2d_loop

  w2d_yo1:
    mov [eax+ecx], byte 0F0h
    inc ecx
    jmp w2d_loop

  w2d_yo2:
    mov [eax+ecx], byte 0F1h
    inc ecx
    jmp w2d_loop

  w2d_rchar:
    sub bl, 0C0h
    mov bl, [CP866+ebx]
    mov [eax+ecx], bl
    inc ecx
    jmp w2d_loop


  w2d_end:

    popf
    popad
  ret


CP1251 db 'ΐΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠΡÒΣΤΥΦΧΨΩΪΫάέήίΰαβγδεζηθικλμνξοπρςστυφχψωϊϋόύώÿ'

;
; ¥ΰ¥ª®¤¨ΰ®Άª  ¨§ CP866 Ά CP1251
; [eax] <-- Null termainated string
;

   dos2win:
     pushf
     pushad

     xor ebx, ebx
     xor ecx, ecx

     dec ecx

   d2w_loop:
     inc ecx
     mov bl, [eax+ecx]
     cmp bl, 0
     jz  d2w_end
     cmp bl, 80h
     jnc d2w_rchar
     
     ;inc ecx
     jmp d2w_loop

   d2w_yo1:
     mov byte [eax+ecx], 0A8h
     ;inc ecx
     jmp d2w_loop

   d2w_yo2:
     mov byte [eax+ecx], 0B8h
     ;inc ecx
     jmp d2w_loop

   d2w_rchar:
     cmp bl, 0B0h
     jnc d2w_rchar2
     
     sub bl, 80h
     mov bl, [CP1251+ebx]
     mov [eax+ecx], bl
     jmp d2w_loop

   d2w_rchar2:
     cmp bl, 0E0h
     jc d2w_loop
     cmp bl, 0F0h
     jz d2w_yo1
     cmp bl, 0F1h
     jz d2w_yo2
     cmp bl, 0F2h
     jnc d2w_loop
     add bl, 10h
     mov [eax+ecx], bl
     jmp d2w_loop


     d2w_end:

     popad
     popf
   ret

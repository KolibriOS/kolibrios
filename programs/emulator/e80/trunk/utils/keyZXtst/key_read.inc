; Text CodePage = cp1251

; ?????????????? ?????????? ?????????? ????? ????? ?????????
;set_ckancode_mode:
;         xor      ecx, ecx
;         inc ecx
;         mcall 66,1                    ; ????????? ?????? ????? ?????????
;         ret





;-----------------------------------------------------
load_port_FE:           ;    ?????????? ???? FE - ?????????? Spectruma

                                  ;    ???? ax - ????? ?????  al = 0xFE   ah = [??????? ???? ?????? ?????]
                                  ;    ?????   - al - ?????????? ????? ?????????? ????????? ????? ???????????
                                  ;    ???????????? ?????? ?????????? ????? c ????????????? ?????????? 
                                  ;    ???????? ????? (??? ? ????????? ??????)
            cmp   al, 0xFE
            jne     .ret
   push   esi
   push   ebx
            mov  esi, zx_keys    ; ????? ?????????? ?? ?????? ?????
            mov  bl, 0xFF
            cld
rept  8
{
            lodsb
            shr   ah, 1
            jc  @f
            and  bl, al
@@:
}
           mov  al, bl

   pop    ebx
   pop    esi

.ret:
           ret







;------------------------------------------------------
;  ?????? ?????????? PC ? ??????? ?? ????????? ? ????? ????? FE ?????????

load_keys:                    ; ???????? ?? ?????????

         mov     edi, scancodes_tbl             ; ????? ??????? ????????? ?????????
         mov     ebx, 0xFFFFFFE0                ; ??????????????? ?????

         mov     edx,  dword [zx_keys]         ; ????? ?????????? ?????????
         mov     esi,   dword [zx_keys+4]    ; 



.loop:                ; ???? ?????? ?????? ?? ??????
         mov     ecx, ebx                       ; ????????? ebx
         mcall   66,3                             ; ????????? ??????????? ???????
         mov     ebx, ecx                       ; ???????????? ebx
         and      al, 0x80                       ; ????????? NumLock
         mov     byte [num_lock_flag], al
.loop_1:
         mcall   2                                  ; ??????? ??? ??????? ???????
         cmp     eax, 1
         jne        .load                              ; ???? ????? ?? ????

.exit:
        mov   dword [zx_keys], edx
        mov   dword [zx_keys+4], esi
.ret:
        ret



.ext_key:
              mov byte [ext_flag], 1
              jmp  .loop

.load:
        cmp  ah, 250
        jne   @f
        or  edx, 0x02000001       ; ??????? CAPS LOCK  {, NumLock, ScrollLock  - ?}
;        or  esi,  0x00000000
        mov  byte [ ext_flag], 0
        jmp  .loop_1
@@:
        cmp  ah,   224
        je       .ext_key                         ; ???? ??????????? ???????

        mov   cl, ah
        and    cl, 0x7F
        cmp   cl, 71                             ; ???? ??????? ?? NumPad
        jc       .l1
        cmp   cl, 84
        jnc     .l1

        cmp  byte [ ext_flag], 1           ;    ???? ??????? ?????? ????? EXT
        je      .l1
       ; ???? ??????? ??? ext - 
        cmp  byte [num_lock_flag], 0     ;   ???? ?????????? ????? ???????
        jne      .lnorm
       ; ?????????????? ?????????? ??? ????? ???? ?? NumPade
        sub   cl, 71
        and   eax, 0x00008000
        mov  al, cl
        jz      .l1_2
        xor    ah, ah
        jz      ._1_2
.l1:
       mov   byte [ext_flag], 0
.lnorm:
        mov     al, ah                            ; scancode ? al
        and     eax, 0x000000FF          ;????????? ??????? ????? ?? al
        or        al, al
        js  ._1
  ;  ???? ????? ??????? ??????? ???????
.l1_1:    add  al, 13
.l1_2:
       shl       eax, 1                        ; *2
       mov     ax, word [ edi+eax]     ;   al - 1 ???? , ah - 2 ????
       mov     ecx, eax
     call     .L_proc_set                    ; 1 - ???????
     shr    ecx, 8
     mov   eax, ecx
     call     .L_proc_set                    ; 2 - ???????
       jmp   .loop_1                               ; ?????????? ????? ?? ??????

._1:              ;???? ????? ??????? ?????????? ???????
       and      al, 0x7f
       add      al, 13
._1_2:
       shl       eax, 1                        ; *2
       mov     ax, word [ edi+eax]     ;   al - 1 ???? , ah - 2 ????
       mov     ecx, eax
     call     .L_proc_res                    ; 1 - ???????
     shr    ecx, 8
     mov   eax, ecx
     call     .L_proc_res                    ; 2 - ???????
       jmp   .loop_1




.L_proc_set:
       or        cl, cl
       jz       .ret
       or        eax, ebx
       and     cl, bl                              ; ????? ????? *32
       or        cl, cl
       js   @f
                                                         ;  ???? ?????????? ? edx
       ror    cl, 2                                 ; ??????????? ????????
       rol    eax, cl
       and  edx, eax                           ; ?????????? ???????
       ret
@@:                                         ; ???? ?????????? ? esi
      and   cl, 0x60                 ; ?????????? ?????
      ror    cl, 2
      rol    eax, cl
      and  esi, eax
      ret

.L_proc_res:
       or        cl, cl
       jz        .ret
       or        eax, ebx
       xor      eax, -1
       and     cl, bl                              ; ????? ????? *32
       or        cl, cl
       js   @f
                                                         ;  ???? ?????????? ? edx
       ror    cl, 2                                 ; ??????????? ????????
       rol    eax, cl
       or     edx, eax                           ; ???????? ???????
       ret
@@:                                         ; ???? ?????????? ? esi
      and   cl, 0x60                 ; ?????????? ?????
      ror    cl, 2
      rol    eax, cl
      or     esi, eax
      ret
;--------------------------------------------------------



;   ?????????? ??? ?????????
;-----------------------------------------------------
;  SHIFT             -   CAPS SHIFT        (CS)
;  CTRL              -    SIMBOL SHIFT   (SS)
;  ALT                -    EXTEND MODE   (CS/SS) ; ??????? ???????? CS/SS ?? ??????????
;
;  BACKSPACE  -   DELETE                (CS/0)
;  ESC                -   BREAK                  (CS/Space)
;  TAB                -   EDIT                     (CS/1)
;  '~'                  -   GRAPH                 (CS/9)
;  PAGE UP        -  TRUE VIDEO        (CS/3)
;  PAGE DOWN  -  INV. VIDEO          (CS/4)
;  UP                  -   UP                        (CS/7)
;  DOWN            -  DOWN                  (CS/6)
;  LEFT              -  LEFT                    (CS/5)
;  RIGHT            -  RIGHT                 (CS/8)
;  CAPS LOCK    - /CAPS LOCK/ - ?? ???????? ???????? ?????? ???  (CS/2)




not_used_code     equ     0x1F1F           ; ??? ??? ??????????????? ????????

; ??? ???????? - 0x[???? ????????][???? ???????]
; [???? ??????? / ????????] - ????=PPPKKKKK
; ???   PPP -  ??? ?????
;          KKKKK - ??? ??????? 





               ; ??????? ???????????? ????????? ????? ????????? ?????? - 256 - ????

scancodes_tbl     dw 0x0097                          ; 71 - Num7
   dw 0x009B                          ; 72 - Num8
   dw 0x009D                          ; 73 - Num9
   dw 0xFDD7                         ; 74 - Num-
   dw 0x0077                          ; 75 - Num4
   dw 0x006F                          ; 76 - Num5
   dw 0x008F                          ; 77 - Num6
   dw 0xFDDB                         ; 78 - Num+
   dw 0x007E                           ; 79 - Num1

   dw 0x007D                          ; 80 - Num2
   dw 0x007B                          ; 81 - Num3
   dw 0x009E                          ; 82 - Num0
   dw 0xFDFB                         ; 83 - Num.
   dw not_used_code             ;  0              ; -------------------------------------------
   dw 0x1EFE                          ; 1 - ESC  -  Break (CS/Space)
   dw 0x007E                           ; 2 - 1
   dw 0x007D                           ; 3 - 2
   dw 0x007B                           ; 4 - 3
   dw 0x0077                           ; 5 - 4
   dw 0x006F                          ; 6 - 5
   dw 0x008F                          ; 7 - 6
   dw 0x0097                          ; 8 - 7
   dw 0x009B                          ; 9 - 8

   dw 0x009D                         ; 10 - 9
   dw 0x009E                          ; 11 - 0
   dw 0xFDD7                         ; 12 - -
   dw 0xFDDD                        ; 13 - =
   dw 0x1E9E                          ; 14 - BackSpace   -  Delete(backspace) (CS/0)
   dw 0x1E7E                          ; 15 - TAB       -     EDIT (CS/1)
   dw 0x005E                          ; 16 - q
   dw 0x005D                         ; 17 - w
   dw 0x005B                         ; 18 - e
   dw 0x0057                         ; 19 - r

   dw 0x004F                          ; 20 - t
   dw 0x00AF                           ; 21 - y
   dw 0x00B7                          ; 22 - u
   dw 0x00BB                         ; 23 - i
   dw 0x00BD                         ; 24 - o
   dw 0x00BE                         ; 25 - p
   dw 0xFDAF                        ; 26 - [            -     SS/Y
   dw 0xFDB7                        ; 27 - ]            -     SS/U
   dw 0x00DE                         ; 28 - Enter
   dw 0x00FD                         ; 29 - L_Control   -  Simbol Shift (SS)

   dw 0x003E                          ; 30 - a
   dw 0x003D                          ; 31 - s
   dw 0x003B                          ; 32 - d
   dw 0x0037                          ; 33 - f
   dw 0x002F                         ; 34 - g
   dw 0x00CF                         ; 35 - h
   dw 0x00D7                         ; 36 - j
   dw 0x00DB                         ; 37 - k
   dw 0x00DD                         ; 38 - l
   dw 0xFDBD                        ; 39 - ;         -   ;  (SS/O)

   dw 0xFD97                         ; 40 - '       -    '  (SS/7)
   dw 0x1E9D                         ; 41 - ~     -   GRAPH (CS/9)
   dw 0x001E                          ; 42 - L_Shift (?? caps )
   dw 0xFD3B                         ; 43 - \      -   SS/D
   dw 0x001D                          ; 44 - z
   dw 0x001B                          ; 45 - x
   dw 0x0017                          ; 46 - c
   dw 0x000F                          ; 47 - v
   dw 0x00EF                          ; 48 - b
   dw 0x00F7                          ; 49 - n

   dw 0x00FB                          ; 50 - m
   dw 0xFDF7                         ; 51 - ,
   dw 0xFDFB                         ; 52 - .
   dw 0xFD0F                         ; 53 - /
   dw 0x001E                          ; 54 - R_Shift  -    CAPS   (CS)
   dw 0xFDEF                         ; 55 - Num *    -  *  (SS/B)
   dw 0xFD1E                         ; 56 - ALT     -    Extend Mode     (CS/SS)
   dw 0x00FE                          ; 57 - Space
   dw 0x1E7D                         ; 58 - Caps Lock   -  Caps Lock  (CS/2)
   dw not_used_code             ; 59 - F1

   dw not_used_code             ; 60 - F2
   dw not_used_code             ; 61 - F3
   dw not_used_code             ; 62 - F4
   dw not_used_code             ; 63 - F5
   dw not_used_code             ; 64 - F6
   dw not_used_code             ; 65 - F7
   dw not_used_code             ; 66 - F8
   dw not_used_code             ; 67 - F9
   dw not_used_code             ; 68 - F10
   dw not_used_code             ; 69 - NumLock (?????? M 250)
                                                       ; BREAK ????????? (225,29,69) (225 - EXT2) (29 -L_Control)

   dw not_used_code             ; 70 - Scroll Lock ( ?????? ? 250)
   dw not_used_code             ; 71 - Home  (????? EXT)      (?? 
   dw 0x1E97                          ; 72 - Up (????? EXT)     -    UP (CS/7)
   dw 0x1E7B                         ; 73 - Page Up (????? EXT - 224)   -  True video (CS/3)
   dw 0xFDD7                         ; 74 - Num-     -        -     (SS/J)
   dw 0x1E6F                          ; 75 - Left (????? EXT)     -    <-- (CS/5)
   dw not_used_code             ; 76 - Num5
   dw 0x1E9B                          ; 77 - Right (????? EXT)  -    --> (CS/8)
   dw 0xFDDB                         ; 78 - Num+
   dw not_used_code             ; 79 - END (????? EXT)

   dw 0x1E8F                          ; 80 - Down (????? EXT)   -    DOWN (CS/6)
   dw 0x1E77                          ; 81 - Page Down (????? EXT)  - Inv. Video (CS/4)
   dw not_used_code             ; 82 - Insert (????? EXT)
   dw not_used_code             ; 83 - Delete (????? EXT)
   dw not_used_code             ; 84
   dw not_used_code             ; 85
   dw not_used_code             ; 86
   dw not_used_code             ; 87 - F11
   dw not_used_code             ; 88 - F12
   dw not_used_code             ; 89

   dw not_used_code             ; 90
   dw not_used_code             ; 91
   dw not_used_code             ; 92
   dw not_used_code             ; 93 - ???? 3 (????? ext)
   dw not_used_code             ; 94
   dw not_used_code             ; 95
   dw not_used_code             ; 96 - (224) - ext - ???. ??????? ?????????
   dw not_used_code             ; 97 - (225) - ext2
   dw not_used_code             ; 98
   dw not_used_code             ; 99

   dw not_used_code             ; 100
   dw not_used_code             ; 101
   dw not_used_code             ; 102
   dw not_used_code             ; 103
   dw not_used_code             ; 104
   dw not_used_code             ; 105
   dw not_used_code             ; 106
   dw not_used_code             ; 107
   dw not_used_code             ; 108
   dw not_used_code             ; 109

   dw not_used_code             ; 110
   dw not_used_code             ; 111
   dw not_used_code             ; 112
   dw not_used_code             ; 113
   dw not_used_code             ; 114
   dw not_used_code             ; 115
   dw not_used_code             ; 116
   dw not_used_code             ; 117
   dw not_used_code             ; 118
   dw not_used_code             ; 119

   dw not_used_code             ; 120
   dw not_used_code             ; 121
   dw not_used_code             ; 122 - (250) M ?????? (Capslock, Numlock ? ?.?.)
   dw not_used_code             ; 123
   dw not_used_code             ; 124
   dw not_used_code             ; 125
   dw not_used_code             ; 126
   dw not_used_code             ; 127
   dw 0




; ????? ??????????
ext_flag                 db 0                 ; ???? ???????????? ???????? 
num_lock_flag      db 0                 ;  ???? num ?????


;????? ZX-Spectrum ??????????.

zx_keys     dd   0xFFFFFFFF, 0xFFFFFFFF
;??????? ?????/??????? ???? ?????:

;   +0 - FE
;   +1 - FD
;   +2 - FB
;   +3 - F7
;   +4 - EF
;   +5 - DF
;   +6 - BF
;   +7 - 7F




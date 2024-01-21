;
;   ZKEY. Version 0.51.
;   Virtual keyboard for KolibriOS.
;
;   Author:         Asper
;   Date of issue:  26.08.2009
;   Compiler:       FASM
;   Target:         KolibriOS
;

use32
        org     0x0

        db     'MENUET01'
        dd     1                ; version
        dd     STARTAPP         ; program start
        dd     I_END            ; program image size
        dd     MEM              ; size memory for program
        dd     STACKTOP         ; base stack pointer
        dd     0
        dd     0

include '../../../macros.inc'
include "ASPAPI.INC"

N_KEYCOLOR equ 0x00677889 ; Normal button color
C_KEYCOLOR equ 0x00344556 ; Control button color
L_KEYCOLOR equ 0x00258778 ; Lock button color
TEXTCOLOR  equ 0x00FFFFFF ; Button caption color

ASCII_KEYMAP_SIZE equ 128

STARTAPP:
        mov     ecx, 1     ; to send scancodes.
        call    set_keyboard_mode

        call    reload_ascii_keymap
        call    draw_window
        jmp     get_zkey_window_slot_number

draw_window:
  test   byte [red_type], 1
  jnz    @f
  test   byte [red_type], 2
  jnz    .draw_switch_button
  start_draw_window 100,370,600,210,0x14224466,labelt

  stdcall draw_button, 15,50,23,20,2,N_KEYCOLOR,keyText,3,TEXTCOLOR             ; Esc

  stdcall draw_button, 70,50,23,20,3,N_KEYCOLOR,keyText+3,2,TEXTCOLOR   ; F1
  stdcall draw_button, 95,50,23,20,4,N_KEYCOLOR,keyText+5,2,TEXTCOLOR   ; F2
  stdcall draw_button, 120,50,23,20,5,N_KEYCOLOR,keyText+7,2,TEXTCOLOR  ; F3
  stdcall draw_button, 145,50,23,20,6,N_KEYCOLOR,keyText+9,2,TEXTCOLOR  ; F4

  stdcall draw_button, 183,50,23,20,7,N_KEYCOLOR,keyText+11,2,TEXTCOLOR ; F5
  stdcall draw_button, 208,50,23,20,8,N_KEYCOLOR,keyText+13,2,TEXTCOLOR ; F6
  stdcall draw_button, 233,50,23,20,9,N_KEYCOLOR,keyText+15,2,TEXTCOLOR ; F7
  stdcall draw_button, 258,50,23,20,10,N_KEYCOLOR,keyText+17,2,TEXTCOLOR ; F8

  stdcall draw_button, 296,50,23,20,11,N_KEYCOLOR,keyText+19,2,TEXTCOLOR ; F9
  stdcall draw_button, 321,50,23,20,12,N_KEYCOLOR,keyText+21,3,TEXTCOLOR ; F10
  stdcall draw_button, 346,50,23,20,13,N_KEYCOLOR,keyText+24,3,TEXTCOLOR ; F11
  stdcall draw_button, 371,50,23,20,14,N_KEYCOLOR,keyText+27,3,TEXTCOLOR ; F12
@@:
  stdcall draw_button, 15,80,23,20,15,N_KEYCOLOR,ascii_keymap+41,1,TEXTCOLOR ;keyText+30,2,TEXTCOLOR     ; ~
  stdcall draw_button, 40,80,23,20,16,N_KEYCOLOR,ascii_keymap+2,1,TEXTCOLOR ;keyText+32,2,TEXTCOLOR      ; 1!
  stdcall draw_button, 65,80,23,20,17,N_KEYCOLOR,ascii_keymap+3,1,TEXTCOLOR ;keyText+34,2,TEXTCOLOR      ; 2@
  stdcall draw_button, 90,80,23,20,18,N_KEYCOLOR,ascii_keymap+4,1,TEXTCOLOR ;keyText+36,2,TEXTCOLOR      ; 3#
  stdcall draw_button, 115,80,23,20,19,N_KEYCOLOR,ascii_keymap+5,1,TEXTCOLOR ;keyText+38,2,TEXTCOLOR     ; 4$
  stdcall draw_button, 140,80,23,20,20,N_KEYCOLOR,ascii_keymap+6,1,TEXTCOLOR ;keyText+40,2,TEXTCOLOR     ; 5%
  stdcall draw_button, 165,80,23,20,21,N_KEYCOLOR,ascii_keymap+7,1,TEXTCOLOR ;keyText+42,2,TEXTCOLOR     ; 6^
  stdcall draw_button, 190,80,23,20,22,N_KEYCOLOR,ascii_keymap+8,1,TEXTCOLOR ;keyText+44,2,TEXTCOLOR     ; 7&
  stdcall draw_button, 215,80,23,20,23,N_KEYCOLOR,ascii_keymap+9,1,TEXTCOLOR ;keyText+46,2,TEXTCOLOR     ; 8*
  stdcall draw_button, 240,80,23,20,24,N_KEYCOLOR,ascii_keymap+10,1,TEXTCOLOR ;keyText+48,2,TEXTCOLOR    ; 9(
  stdcall draw_button, 265,80,23,20,25,N_KEYCOLOR,ascii_keymap+11,1,TEXTCOLOR ;keyText+50,2,TEXTCOLOR    ; 0)
  stdcall draw_button, 290,80,23,20,26,N_KEYCOLOR,ascii_keymap+12,1,TEXTCOLOR ;keyText+52,2,TEXTCOLOR    ; -_
  stdcall draw_button, 315,80,23,20,27,N_KEYCOLOR,ascii_keymap+13,1,TEXTCOLOR ;keyText+54,2,TEXTCOLOR    ; =+
  stdcall draw_button, 340,80,23,20,28,N_KEYCOLOR,ascii_keymap+43,1,TEXTCOLOR ;keyText+56,2,TEXTCOLOR    ; \|
  stdcall draw_button, 365,80,30,20,29,N_KEYCOLOR,keyText+58,4,TEXTCOLOR ; bksp

  stdcall draw_button, 15 ,103,33,20,30,N_KEYCOLOR,keyText+62,3,TEXTCOLOR       ; Tab
  stdcall draw_button, 50 ,103,23,20,31,N_KEYCOLOR,ascii_keymap+16,1,TEXTCOLOR ;keyText+65,2,TEXTCOLOR   ; Qâ
  stdcall draw_button, 75 ,103,23,20,32,N_KEYCOLOR,ascii_keymap+17,1,TEXTCOLOR ;keyText+67,2,TEXTCOLOR   ; Wñ
  stdcall draw_button, 100,103,23,20,33,N_KEYCOLOR,ascii_keymap+18,1,TEXTCOLOR ;keyText+69,2,TEXTCOLOR   ; Eì
  stdcall draw_button, 125,103,23,20,34,N_KEYCOLOR,ascii_keymap+19,1,TEXTCOLOR ;keyText+71,2,TEXTCOLOR   ; Rä
  stdcall draw_button, 150,103,23,20,35,N_KEYCOLOR,ascii_keymap+20,1,TEXTCOLOR ;keyText+73,2,TEXTCOLOR   ; TÖ
  stdcall draw_button, 175,103,23,20,36,N_KEYCOLOR,ascii_keymap+21,1,TEXTCOLOR ;keyText+75,2,TEXTCOLOR   ; Yç
  stdcall draw_button, 200,103,23,20,37,N_KEYCOLOR,ascii_keymap+22,1,TEXTCOLOR ;keyText+77,2,TEXTCOLOR   ; UÉ
  stdcall draw_button, 225,103,23,20,38,N_KEYCOLOR,ascii_keymap+23,1,TEXTCOLOR ;keyText+79,2,TEXTCOLOR   ; Iò
  stdcall draw_button, 250,103,23,20,39,N_KEYCOLOR,ascii_keymap+24,1,TEXTCOLOR ;keyText+81,2,TEXTCOLOR   ; Oô
  stdcall draw_button, 275,103,23,20,40,N_KEYCOLOR,ascii_keymap+25,1,TEXTCOLOR ;keyText+83,2,TEXTCOLOR   ; Pá
  stdcall draw_button, 300,103,23,20,41,N_KEYCOLOR,ascii_keymap+26,1,TEXTCOLOR ;keyText+85,3,TEXTCOLOR   ; [{ï
  stdcall draw_button, 325,103,23,20,42,N_KEYCOLOR,ascii_keymap+27,1,TEXTCOLOR ;keyText+88,3,TEXTCOLOR   ; ]}ö
  stdcall draw_button, 350,103,45,43,43,N_KEYCOLOR,keyText+91,5,TEXTCOLOR       ; Enter

  stdcall draw_button, 15 ,126,43,20,44,L_KEYCOLOR,keyText+96 ,4,TEXTCOLOR      ; lock
  stdcall draw_button, 60 ,126,23,20,45,N_KEYCOLOR,ascii_keymap+30,1,TEXTCOLOR ;keyText+100,2,TEXTCOLOR  ; Aî
  stdcall draw_button, 85 ,126,23,20,46,N_KEYCOLOR,ascii_keymap+31,1,TEXTCOLOR ;keyText+102,2,TEXTCOLOR  ; Sõ
  stdcall draw_button, 110,126,23,20,47,N_KEYCOLOR,ascii_keymap+32,1,TEXTCOLOR ;keyText+104,2,TEXTCOLOR  ; DÇ
  stdcall draw_button, 135,126,23,20,48,N_KEYCOLOR,ascii_keymap+33,1,TEXTCOLOR ;keyText+106,2,TEXTCOLOR  ; FÄ
  stdcall draw_button, 160,126,23,20,49,N_KEYCOLOR,ascii_keymap+34,1,TEXTCOLOR ;keyText+108,2,TEXTCOLOR  ; Gè
  stdcall draw_button, 185,126,23,20,50,N_KEYCOLOR,ascii_keymap+35,1,TEXTCOLOR ;keyText+110,2,TEXTCOLOR  ; Hê
  stdcall draw_button, 210,126,23,20,51,N_KEYCOLOR,ascii_keymap+36,1,TEXTCOLOR ;keyText+112,2,TEXTCOLOR  ; Jé
  stdcall draw_button, 235,126,23,20,52,N_KEYCOLOR,ascii_keymap+37,1,TEXTCOLOR ;keyText+114,2,TEXTCOLOR  ; Kã
  stdcall draw_button, 260,126,23,20,53,N_KEYCOLOR,ascii_keymap+38,1,TEXTCOLOR ;keyText+116,2,TEXTCOLOR  ; LÑ
  stdcall draw_button, 285,126,23,20,54,N_KEYCOLOR,ascii_keymap+39,1,TEXTCOLOR ;keyText+118,3,TEXTCOLOR  ; ;:Ü
  stdcall draw_button, 310,126,23,20,55,N_KEYCOLOR,ascii_keymap+40,1,TEXTCOLOR ;keyText+121,3,TEXTCOLOR  ; '"ù

  stdcall draw_button, 15 ,149,53,20,56,C_KEYCOLOR,keyText+124,5,TEXTCOLOR      ; Shift
  stdcall draw_button, 70 ,149,23,20,57,N_KEYCOLOR,ascii_keymap+44,1,TEXTCOLOR ;keyText+129,2,TEXTCOLOR  ; Zü
  stdcall draw_button, 95 ,149,23,20,58,N_KEYCOLOR,ascii_keymap+45,1,TEXTCOLOR ;keyText+131,2,TEXTCOLOR  ; Xó
  stdcall draw_button, 120,149,23,20,59,N_KEYCOLOR,ascii_keymap+46,1,TEXTCOLOR ;keyText+133,2,TEXTCOLOR  ; Cë
  stdcall draw_button, 145,149,23,20,60,N_KEYCOLOR,ascii_keymap+47,1,TEXTCOLOR ;keyText+135,2,TEXTCOLOR  ; Vå
  stdcall draw_button, 170,149,23,20,61,N_KEYCOLOR,ascii_keymap+48,1,TEXTCOLOR ;keyText+137,2,TEXTCOLOR  ; Bà
  stdcall draw_button, 195,149,23,20,62,N_KEYCOLOR,ascii_keymap+49,1,TEXTCOLOR ;keyText+139,2,TEXTCOLOR  ; Ní
  stdcall draw_button, 220,149,23,20,63,N_KEYCOLOR,ascii_keymap+50,1,TEXTCOLOR ;keyText+141,2,TEXTCOLOR  ; Mú
  stdcall draw_button, 245,149,23,20,64,N_KEYCOLOR,ascii_keymap+51,1,TEXTCOLOR ;keyText+143,3,TEXTCOLOR  ; ,<Å
  stdcall draw_button, 270,149,23,20,65,N_KEYCOLOR,ascii_keymap+52,1,TEXTCOLOR ;keyText+146,3,TEXTCOLOR  ; .>û
  stdcall draw_button, 295,149,23,20,66,N_KEYCOLOR,ascii_keymap+53,1,TEXTCOLOR ;keyText+149,2,TEXTCOLOR  ; /?

  test    byte [red_type], 1
  jnz     @f
  stdcall draw_button, 320,149,75,20,67,C_KEYCOLOR,keyText+151,5,TEXTCOLOR      ; Shift

  stdcall draw_button, 15 ,172,30,20,68,C_KEYCOLOR,keyText+156,4,TEXTCOLOR      ; Ctrl
  stdcall draw_button, 47 ,172,30,20,69,C_KEYCOLOR,keyText+160,3,TEXTCOLOR      ; Win
  stdcall draw_button, 79 ,172,30,20,70,C_KEYCOLOR,keyText+163,3,TEXTCOLOR      ; Alt
  stdcall draw_button, 111,172,156,20,71,N_KEYCOLOR,keyText+166,1,TEXTCOLOR     ;  Space
  stdcall draw_button, 269,172,30,20,72,C_KEYCOLOR,keyText+167,3,TEXTCOLOR      ; Alt
  stdcall draw_button, 301,172,30,20,73,C_KEYCOLOR,keyText+170,3,TEXTCOLOR      ; Win
  stdcall draw_button, 333,172,30,20,74,C_KEYCOLOR,keyText+173,3,TEXTCOLOR      ; [=]
  stdcall draw_button, 365,172,30,20,75,C_KEYCOLOR,keyText+176,4,TEXTCOLOR      ; Ctrl

  stdcall draw_button, 405,50 ,23,20,76,N_KEYCOLOR,keyText+180,3,TEXTCOLOR      ; psc
  stdcall draw_button, 430,50 ,23,20,77,L_KEYCOLOR,keyText+183,3,TEXTCOLOR      ; slk
  stdcall draw_button, 455,50 ,23,20,78,N_KEYCOLOR,keyText+186,3,TEXTCOLOR      ; brk
  stdcall draw_button, 405,80 ,23,20,79,N_KEYCOLOR,keyText+189,3,TEXTCOLOR      ; Ins
  stdcall draw_button, 430,80 ,23,20,80,N_KEYCOLOR,keyText+192,2,TEXTCOLOR      ; Hm
  stdcall draw_button, 455,80 ,23,20,81,N_KEYCOLOR,keyText+194,3,TEXTCOLOR      ; Pup
  stdcall draw_button, 405,103 ,23,20,82,N_KEYCOLOR,keyText+197,3,TEXTCOLOR     ; Del
  stdcall draw_button, 430,103 ,23,20,83,N_KEYCOLOR,keyText+200,3,TEXTCOLOR     ; End
  stdcall draw_button, 455,103 ,23,20,84,N_KEYCOLOR,keyText+203,3,TEXTCOLOR     ; Pdn
  stdcall draw_button, 430,149,23,20,85,C_KEYCOLOR,keyText+206,1,TEXTCOLOR      ; ^
  stdcall draw_button, 405,172,23,20,86,C_KEYCOLOR,keyText+207,1,TEXTCOLOR      ; v
  stdcall draw_button, 430,172,23,20,87,C_KEYCOLOR,keyText+208,1,TEXTCOLOR      ; <
  stdcall draw_button, 455,172,23,20,88,C_KEYCOLOR,keyText+209,2,TEXTCOLOR      ;  >

  stdcall draw_button, 488,80 ,23,20,89,L_KEYCOLOR,keyText+211,3,TEXTCOLOR      ; nlk
  stdcall draw_button, 513,80 ,23,20,90,N_KEYCOLOR,keyText+214,1,TEXTCOLOR      ; /
  stdcall draw_button, 538,80 ,23,20,91,N_KEYCOLOR,keyText+215,1,TEXTCOLOR      ; *
  stdcall draw_button, 563,80 ,23,20,92,N_KEYCOLOR,keyText+216,1,TEXTCOLOR      ; -
  stdcall draw_button, 488,103 ,23,20,93,N_KEYCOLOR,keyText+217,1,TEXTCOLOR     ; 7
  stdcall draw_button, 513,103 ,23,20,94,N_KEYCOLOR,keyText+218,1,TEXTCOLOR     ; 8
  stdcall draw_button, 538,103 ,23,20,95,N_KEYCOLOR,keyText+219,1,TEXTCOLOR     ; 9
  stdcall draw_button, 488,126,23,20,96,N_KEYCOLOR,keyText+220,1,TEXTCOLOR      ; 4
  stdcall draw_button, 513,126,23,20,97,N_KEYCOLOR,keyText+221,1,TEXTCOLOR      ; 5
  stdcall draw_button, 538,126,23,20,98,N_KEYCOLOR,keyText+222,1,TEXTCOLOR      ; 6
  stdcall draw_button, 488,149,23,20,99,N_KEYCOLOR,keyText+223,1,TEXTCOLOR      ; 1
  stdcall draw_button, 513,149,23,20,100,N_KEYCOLOR,keyText+224,1,TEXTCOLOR     ; 2
  stdcall draw_button, 538,149,23,20,101,N_KEYCOLOR,keyText+225,1,TEXTCOLOR     ; 3
  stdcall draw_button, 488,172,48,20,102,N_KEYCOLOR,keyText+226,1,TEXTCOLOR     ; 0
  stdcall draw_button, 538,172,23,20,103,N_KEYCOLOR,keyText+227,1,TEXTCOLOR     ; .
  stdcall draw_button, 563,103 ,23,43,104,N_KEYCOLOR,keyText+228,1,TEXTCOLOR    ; +
  stdcall draw_button, 563,149,23,43,105,N_KEYCOLOR,keyText+229,3,TEXTCOLOR     ; Ent

 .draw_switch_button:
  mov   eax, 6
  mul   byte [keyboard_mode]
  add   eax, SwitchText
  stdcall draw_button, 513,28,46,20,106,0x00700000,eax,6,TEXTCOLOR       ; Scan/ASCII switch
  
  xor   al, al
  xchg  al, [red_type]
  test  al, 2
  jnz   .skip_end_draw_window ; if we draw switch button without redraw window then end_draw_window not need
  
@@:
 .draw_indicators_area:
  call lights_on

  end_draw_window
  
.skip_end_draw_window:  
ret

get_zkey_window_slot_number:
        mov     eax, 18
        mov     ebx, 7
        int     0x40
        mov     [zkey_window], eax
set_event_mask:
        mov     eax, 40
        mov     ebx, EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE ; 39 ; 100111b
        int     0x40

still:
        call    get_receiver_window_slot_number
        call    get_receiver_window_keyboard_mode ; return in ecx keyboard_mode for set_keyboard_mode
        call    set_keyboard_mode
; delete_switch_button 
        mov eax, 8 
        mov edx, BT_DEL + 0x00700000
        int     0x40 

        mov    byte [red_type], 2
        call    draw_window.draw_switch_button
 
        mov     eax, 10               ; Wait for an event in the queue.
        int     0x40

        cmp     al, EV_REDRAW         ; redraw request ?
        jz      red
        cmp     al, EV_KEY            ; key in buffer ?
        jz      key
        cmp     al, EV_BUTTON         ; button in buffer ?
        jz      button

        jmp     still

red:
        call    draw_window
        jmp     still

key:
        mov     eax, 2
        int     0x40

;        push    ax
        call    get_control_keys_state
        cmp     [locks], ax
        je      @f

      .reload:
        call    reload_ascii_keymap
        mov     byte [red_type], 1
        call    draw_window
       @@:
;        pop     dx
;        jmp     button.select_key_code
        jmp     still

button:
        mov     eax, 17             ; Get pressed button code
        int     0x40
        cmp     ah, 1               ; Test x button
        je      close_app

        cmp     ah, 106             ; Test Scan/ASCII switch button
        jne     .keyboard_keys
        mov     cl, 1
        sub     cl, byte [keyboard_mode]
        call    set_keyboard_mode
        mov     byte [red_type], 2
        jmp     red

      .keyboard_keys:
        cmp     ah, 2               ; Test if pressed buttons
        jb      still               ; is a ZKEY keyboard key...
        cmp     ah, 106
        jnb     still               ; ...so, if not then still,
        mov     [butt], ah          ; if yes then save pressed button ID
      .test_slot:
        mov     eax, [zkey_window]              ; Test is receiver ZKEY window
        mov     ecx, [receiver_window]
        cmp     eax, ecx
        je      still                           ; if yes still.
      .activate_receiver_window:
        mov     eax, 18
        mov     ebx, 3
        int     0x40

      call reload_ascii_keymap

      .read_key_scancode:
        xor     edx, edx
        movzx   si, byte [butt]                 ; Read scan code of the key from the table.
        sub     si, 2
        mov     dl, [keyCodes+si]               ; Set dl = scancode of the key corresponding to the pressed button.
        cmp     dl, 0xA0
        jb      .select_key_code                ; If not extended key then select ascii or scan to send and then send it to the receiver.

        cmp     dl, 0xA6                        ; Test if it's extended2 key
        jne     .read_ext_key                   ; if not go to prepare to send simply extended key.
        mov     dl, 0xE1                        ; Send extended2 scancode...
        call    send_key
        mov     dl, 0x1D                        ; ...and extended scancode,
        call    send_key                        ;
        mov     dl, [keyCodes.ext2]             ; (There is only one ext2 key - 'Break')
        jmp     .send_key_code                          ; so... send it's normal scan code.
      .read_ext_key:
        sub     dl, 0xA0                        ; Read extended key scan code from the table.
        movzx   si, dl
        mov     dl, [keyCodes.ext+si]
     .send_ext:
        push    dx                              ; Send extended scancode to the receiver's window.
        mov     dl, 0xE0
        call    send_key
        pop     dx
;        jmp     .send_key_code
      .select_key_code:                 ; Select what to send ASCII or scan.
        cmp     [keyboard_mode], 0      ; Test if ZKEY must send ASCII.
        jne     .send_key_code

;        mov     esi, ext0               ; Set esi to the beginning of the ext0 table.
        and     edx, 0x7F
;      @@:
;        cmp     dl, byte [esi]
;        je      .send_key_code
;        inc     esi
;        cmp     esi, ext0end
;        jng     @b
      .read_ascii:
        mov     dl, byte [ascii_keymap+edx]     ; Read ASCII from the keymap table.
      .send_key_code:
        call    send_key                ; Finally send the key to the receiver's window.
      .wait:
        mov     eax, 5
        mov     ebx, 1                  ; Wait 0.01 second.
        int     0x40
      .activate_zkey_window:
        mov     eax, 18
        mov     ebx, 3
        mov     ecx, [zkey_window]
        int     0x40

        jmp     still


close_app:
    mov  eax,-1                  ; close this program
    int  0x40


;**********************************
;*  input: ecx = type of mode     *
;**********************************

set_keyboard_mode:
        mov     eax, 66
        mov     ebx, 1     ; Set keyboard mode
        int     0x40
        mov     [keyboard_mode], cl
ret

get_control_keys_state:    ; This routine works only in Scan keyboard mode.
        mov     eax, 66
        mov     ebx, 3
        int     0x40
ret


;**********************************
;*  input: ecx = type of map      *
;**********************************

get_keyboard_map:
        mov     eax, 26
        mov     ebx, 2
        mov     edx, ascii_keymap
        int     0x40
ret


reload_ascii_keymap:
        call    get_control_keys_state
        mov     ecx, 1

        test    ax, 3                           ; Shift pressed ?
        jnz      @f
        test    ax, 0x40                        ; Caps  Lock  on ?
        jz      .load_ascii_keymap
      @@:
        mov     ecx, 2
      .load_ascii_keymap:
        call    get_keyboard_map
  ret



get_receiver_window_slot_number:
        mov     eax, 18
        mov     ebx, 7
        int     0x40
        mov     ebx, [zkey_window]
        cmp     eax, ebx
        je      @f
        mov     [receiver_window], eax
      @@:
  ret

get_receiver_window_keyboard_mode:
        mov     eax, 9
        mov     ebx, PROCINFO_BUFF
        mov     ecx, [receiver_window]
        int     0x40
        movzx   ecx, byte [ebx + process_information.keyboard_mode]
        ret

;************************************
;*  input: edx = code of the key    *
;************************************

send_key:
        mov     eax, 72
        mov     ebx, 1
        mov     ecx, 2
        int     0x40
  ret

lights_on:
   bar 491,51,95,15,0                      ; indicators area
   rectangle 490,50,96,16,TEXTCOLOR

   call    get_control_keys_state
   mov     [locks], ax
   test ax, 0x40
   jz @f
   ;stdcall bar, 532, 53, 10, 11, 0x0000FF00
   bar 532, 53, 10, 11, 0x0000FF00
 @@:
   test ax, 0x80
   jz @f
   ;stdcall bar, 500, 53, 10, 11, 0x0000FF00
   bar 500, 53, 10, 11, 0x0000FF00
 @@:
   test ax, 0x100
   jz @f
   ;stdcall bar, 565, 53, 10, 11, 0x0000FF00
   bar 565, 53, 10, 11, 0x0000FF00
 @@:
ret

; DATA AREA

; Application Title
labelt db 'ZKEY v0.51 - keyboard for KolibriOS', 0

SwitchText db ' ASCII SCAN '

;Button names
keyText db 'EscF1F2F3F4F5F6F7F8F9F10F11F12'
        db '~1!2@3#4$5%6^7&8*9(0)-_=+\|bksp'
        db 'TabQâWñEìRäTÖYçUÉIòOôPá{[ï}]öEnter'
        db 'lockAîSõDÇFÄGèHêJéKãLÑ;:Ü''"ù'
        db 'ShiftZüXóCëVåBàNíMú,<Å.>û/?Shift'
        db 'CtrlWinAlt AltWin[=]Ctrl'
        db 'pscslkbrkInsHmPupDelEndPdn^<v >'
        db 'nlk/*-7894561230.+Ent'

;Key scan codes
keyCodes:
        db 0x01, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43, 0x44, 0x57, 0x58
        db 0x29, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x2B, 0x0E
        db 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C
        db 0x3A, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28
        db 0x2A, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36
        db 0x1D, 0xA0, 0x38, 0x39, 0xA1, 0xA2, 0xA3, 0xA4
        db 0xA5, 0x46, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB0
        db 0x45, 0xB1, 0x37, 0x4A, 0x47, 0x48, 0x49, 0x4B, 0x4C, 0x4D, 0x4F, 0x50, 0x51, 0x52, 0x53, 0x4E, 0xB2
      .ext:
        db 0x5B, 0x38, 0x5C, 0x5D, 0x1D
        db 0x37, 0x00, 0x52, 0x47, 0x49, 0x53, 0x4F, 0x51, 0x48, 0x4B, 0x50, 0x4D
        db 0x35, 0x1C
      .ext2  db 0x45      ; Just Break :)

;ext0:
;    db   0x1D, 0x29, 0x2A, 0x36, 0x37, 0x38, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40
;    db   0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D
;    db   0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53, 0x57, 0x58
;ext0end:

receiver_window dd 0      ; Slot number of the receiver
zkey_window dd 0          ; Slot number of ZKEY
butt    db 0              ; Pressed button ID
locks   dw 0              ; NumLock, CapsLock, ScrollLock lights mask

keyboard_mode db 0        ; Scan or ASCII keys to send ?  0 - ASCII , 1 - Scan
red_type      db 0        ; Type of window redraw. 0 - default , 1 - on ASCII_keymap change , 2 - Switch button pressed.

I_END:                    ; End of application code and data marker

ascii_keymap:   rb ASCII_KEYMAP_SIZE

PROCINFO_BUFF process_information

        rb 4096
STACKTOP:
MEM:


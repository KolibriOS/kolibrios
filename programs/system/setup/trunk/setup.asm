;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                               ;;
;;          DEVICE SETUP         ;;
;;                               ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Authors: Ville       - original version
;          A. Ivushkin - autostart (w launcher)
;          M. Lisovin  - added many feauters (apply all, save all, set time...)
;          I. Poddubny - fixed russian keymap
;14/08/06  Mario79 - added regulation of mouse features

;******************************************************************************
  use32
  org        0x0
  db      'MENUET01'   ; 8 byte identifier
  dd      0x01           ; title version
  dd      START        ; pointer to program start
  dd      I_END        ; size of image
  dd      0x4000      ; reguired amount of memory
  dd      0x4000      ; stack pointer (esp)
  dd      I_PARAM,0    ; parameters, reserved
  include '..\..\..\macros.inc'
;******************************************************************************

LLL equ (56+3)
BBB equ 25

;******************************************************************************
apply_all:

    call _midibase    ;1
    call _sound_dma    ;10
    call _pci_acc    ;12
    call _sb16        ;4
    call _syslang    ;5
    call _keyboard    ;2
    call _mouse_speed
    call _mouse_delay
    call get_disk_info
    cmp  [cd],0
    jne  no_cd
    call _cdbase    ;3
  no_cd:
    cmp  [hd],0
    jne  no_hd
    call _lba_read    ;11
    call _hdbase    ;7
    call _f32p        ;8
  no_hd:
ret
;-------------------------------------------------------------------------------
get_disk_info:
    mov  [hd],1
    mov  [cd],1
    mov  [hdbase],0
    mov  [cdbase],0
    mcall 18,11,1,table_area

  ide_0:
    mov  al,[table_area+1]
    shr  al,6
    cmp  al,0
    je        ide_1
    cmp  al,01b
    jnz  ide_0_cd
    mov  [hdbase],1
    mov  [hd],0
    jmp  ide_1

  ide_0_cd:
    cmp  al,10b
    jnz  ide_1
    mov  [cdbase],1
    mov  [cd],0
    cmp  [hd],0
    je         all_device

  ide_1:
    mov  al,[table_area+1]
    shl  al,2
    shr  al,6
    cmp  al,0
    je        ide_2
    cmp  al,01b
    jnz  ide_1_cd
    cmp  [hd],0
    je         ide_11
    mov  [hdbase],2
    mov  [hd],0
  ide_11:
    cmp  [cd],0
    je         all_device
    jmp  ide_2

  ide_1_cd:
    cmp  al,10b
    jnz  ide_2
    cmp  [cd],0
    je         ide_11_cd
    mov  [cdbase],2
    mov  [cd],0
  ide_11_cd:
    cmp  [hd],0
    je         all_device

 ide_2:
    mov  al,[table_area+1]
    shl  al,4
    shr  al,6
    cmp  al,0
    je        ide_3
    cmp  al,01b
    jnz  ide_2_cd
    cmp  [hd],0
    je         ide_21
    mov  [hdbase],3
    mov  [hd],0
  ide_21:
    cmp  [cd],0
    je         all_device
    jmp  ide_3

  ide_2_cd:
    cmp  al,10b
    jnz  ide_3
    cmp  [cd],0
    je         ide_21_cd
    mov  [cdbase],3
    mov  [cd],0
  ide_21_cd:
    cmp  [hd],0
    je         all_device

  ide_3:
    mov  al,[table_area+1]
    shl  al,6
    shr  al,6
    cmp  al,0
    je        not_device
    cmp  al,01b
    jnz  ide_3_cd
    cmp  [hd],0
    je         ide_31
    mov  [hdbase],4
    mov  [hd],0
  ide_31:
    cmp  [cd],0
    jmp   all_device

  ide_3_cd:
    cmp  al,10b
    jnz  not_device
    cmp  [cd],0
    je         all_device
    mov  [cdbase],4
    mov  [cd],0

  all_device:
  not_device:
    ret

hd db 0
cd db 0
;******************************************************************************
apply_all_and_exit:
    mcall 70,read_fileinfo
    call apply_all
    jmp  close

;******************************************************************************
set_language_and_exit:
    mov  eax,26
    mov  ebx,2
    mov  ecx,9
    mcall
;    cmp  eax,5
;    jne  @f
;    xor  eax,eax
;@@: mov  [keyboard],eax
    cmp  eax,1
    je        russian
    xor  eax,eax
set_lang_now:
    mov  [keyboard],eax
    call _keyboard
    jmp  close
russian:
    mov  eax,3
    jmp  set_lang_now

set_syslanguage_and_exit:
    mov  eax,26
    mov  ebx,5
;    mov  ecx,9
    mcall
    cmp  eax,6
    jne  temp      ;@f
    xor  eax,eax
;@@: inc  eax
temp: inc  eax
    mov  [syslang],eax
    call _syslang
    jmp  close

get_setup_values:
    mcall 26,1
    mov [midibase],eax
    mcall 26,2,9
    dec  eax
    mov [keyboard],eax
    mcall 26,3
    mov [cdbase],eax
    mcall 26,4
    mov [sb16],eax
    mcall 26,5
    mov [syslang],eax
    mcall 26,7
    mov [hdbase],eax
    mcall 26,8
    mov [f32p],eax
    mcall 26,10
    mov [sound_dma],eax
    mcall 26,11
    mov [lba_read],eax
    mcall 26,12
    mov [pci_acc],eax
    mcall 18,19,0
    mov [mouse_speed],eax
    mcall 18,19,2
    mov [mouse_delay],eax
    ret

;******************************************************************************

START:
    cmp  [I_PARAM], 'SLAN'
    je         set_syslanguage_and_exit

    cmp  [I_PARAM], 'LANG'
    je         set_language_and_exit

    cmp  [I_PARAM], 'BOOT'
    je         apply_all_and_exit

    call get_setup_values
    call loadtxt
red:
    call draw_window

still:

    cmp  word [blinkpar],0
    jne  blinker
    mov  eax,29     ;get system date
    mcall
    cmp  eax,[date]
    je         gettime
    mov  [date],eax
 gettime:
    mov  eax,3        ;get system time
    mcall
    cmp  ax,[time]
    je         sysevent
    mov  [time],ax
    call drawtime

 sysevent:
    mov  eax,23
    mov  ebx,8        ; wait here for event with timeout
    mcall

    cmp  eax,1
    jz         red
    cmp  eax,2
    jz         key
    cmp  eax,3
    jz         button

    jmp  still

 blinker:
    cmp  byte [count],6
    jb         noblink
    btc  dword [blinkpar],16
    mov  byte [count],0
    call drawtime
 noblink:
    inc  byte [count]
    jmp  sysevent

incdectime:
    cmp byte [blinkpar],0
    je        still
    mov esi,time
    mov bl,0x23  ;border
    cmp byte [blinkpar],1
    je        hours
    mov bl,0x59       ;minutes
    inc esi
  hours:
    mov al,byte [esi]
    cmp ah,112
    je        dectime
    cmp al,bl
    je        noinctime
     inc al
     daa
    jmp incdectime1
  noinctime:
    xor al,al
  incdectime1:
    mov byte [esi],al
    jmp still
  dectime:
    cmp al,0
    je        nodectime
    dec al
    das
    jmp incdectime1
  nodectime:
    mov al,bl
    jmp incdectime1

incdecdate:
    cmp byte [blinkpar+1],0
    je        still
    mov esi,date
    mov bl,0      ;border of years
    cmp byte [blinkpar+1],1
    jne days
    mov bl,0x12     ;months
    inc esi
  days:
    cmp byte [blinkpar+1],2
    jne nodays
    mov bl,0x31
    add esi,2
  nodays:
    mov al,byte [esi]
    cmp ah,122
    je        decdate
    cmp al,bl
    je        noincdate
    inc al ;add al,1
    daa
    jmp incdecdate1
  noincdate:
    mov al,1
  incdecdate1:
    mov byte [esi],al
    jmp still
  decdate:
    cmp al,1
    je        nodecdate
    dec al
    das
    jmp incdecdate1
  nodecdate:
    mov al,bl
    jmp incdecdate1


  key:
    ;mov  eax,2
    mcall
    cmp  ah,27
    jne  still
    mov  dword [blinkpar],0
    call drawtime
    jmp  still

  button:

    mov  eax,17
    mcall

    cmp  ah,112
    je         incdectime
    cmp  ah,113
    je         incdectime
    cmp  ah,122
    je         incdecdate
    cmp  ah,123
    je         incdecdate
    cmp  ah,111
    jne  noseltime
    mov  al, [blinkpar]
    cmp  al,2
    jae  seltime
    inc  al
    jmp  seltime1
  seltime:
    xor  al,al
  seltime1:
    mov  [blinkpar],al
    call drawtime
    jmp  still
noseltime:
    cmp  ah,121
    jne  noseldate
    mov  al,byte [blinkpar+1]
    cmp  al,3
    jae  seldate
    inc  al
    jmp  seldate1
 seldate:
    xor  al,al
 seldate1:
    mov  [blinkpar+1],al
    call drawtime
    jmp  still
noseldate:
    cmp  ah,99
    jne  nosaveall
    mcall 70,save_fileinfo
    mov  dword [blinkpar],0
    call drawtime
    jmp  still
nosaveall:
    cmp  ah,100
    jne  no_apply_all
    call apply_all
    jmp  still
no_apply_all:

    cmp  ah,1           ; CLOSE APPLICATION
    jne  no_close
close:
    or         eax,-1
    mcall
  no_close:

    cmp  ah,11         ; SET MIDI BASE
    jnz  nosetbase1
    call _midibase
   nosetbase1:
    cmp  ah,12
    jnz  nomm
    sub  [midibase],2
    call draw_infotext
  nomm:
    cmp  ah,13
    jnz  nomp
    add  [midibase],2
    call draw_infotext
  nomp:


    cmp  ah,4           ; SET KEYBOARD
    jnz  nokm
    mov  eax,[keyboard]
    test eax,eax
    je         downuplbl
    dec  eax
    jmp  nodownup
   downuplbl:
    mov  eax,5
   nodownup:
    mov  [keyboard],eax
    call draw_infotext
  nokm:
    cmp  ah,5
    jnz  nokp
    mov  eax,[keyboard]
    cmp  eax,5
    je         updownlbl
    inc  eax
    jmp  noupdown
   updownlbl:
    xor  eax,eax
   noupdown:
    mov  [keyboard],eax
    call draw_infotext
  nokp:


    cmp  ah,22          ; SET CD BASE
    jnz  nocm
    mov  eax,[cdbase]
    sub  eax,2
    and  eax,3
    inc  eax
    mov  [cdbase],eax
    call draw_infotext
  nocm:
    cmp  ah,23
    jnz  nocp
    mov  eax,[cdbase]
    and  eax,3
    inc  eax
    mov  [cdbase],eax
    call draw_infotext
  nocp:
    cmp  ah,21
    jnz  nocs
    call _cdbase
  nocs:

    cmp  ah,62        ; SET HD BASE
    jnz  hnocm
    mov  eax,[hdbase]
    sub  eax,2
    and  eax,3
    inc  eax
    mov  [hdbase],eax
    call draw_infotext
  hnocm:
    cmp  ah,63
    jnz  hnocp
    mov  eax,[hdbase]
    and  eax,3
    inc  eax
    mov  [hdbase],eax
    call draw_infotext
  hnocp:
    cmp  ah,61
    jnz  hnocs
    call _hdbase
  hnocs:

    cmp  ah,82        ; SET SOUND DMA
    jne  no_sdma_d
    mov  eax,[sound_dma]
    dec  eax
   sdmal:
    and  eax,3
    mov  [sound_dma],eax
    call draw_infotext
    jmp  still
  no_sdma_d:
    cmp  ah,83
    jne  no_sdma_i
    mov  eax,[sound_dma]
    inc  eax
    jmp  sdmal
  no_sdma_i:
    cmp  ah,81
    jne  no_set_sound_dma
    call _sound_dma
    jmp  still
  no_set_sound_dma:

    cmp  ah,92         ; SET LBA READ
    jne  no_lba_d
  slbal:
    btc  [lba_read],0
    call draw_infotext
    jmp  still
   no_lba_d:
    cmp  ah,93
    jne  no_lba_i
    jmp  slbal
  no_lba_i:
    cmp  ah,91
    jne  no_set_lba_read
    call _lba_read
    jmp  still
   no_set_lba_read:


    cmp  ah,102       ; SET PCI ACCESS
    jne  no_pci_d
  pcip:
    btc  [pci_acc],0
    call draw_infotext
    jmp  still
  no_pci_d:
    cmp  ah,103
    jne  no_pci_i
    jmp  pcip
   no_pci_i:
    cmp  ah,101
    jne  no_set_pci_acc
    call _pci_acc
    jmp  still
  no_set_pci_acc:


  set_partition:
    cmp  ah,72        ; SET FAT32 PARTITION
    jnz  .nominus
    mov  eax,[f32p]
    sub  eax,2
;   and  eax,15          ; 3 - four partitions, 7 - eight p., 15 - sixteen, etc.
    cmp  eax,15
    jb        @f
    mov  eax,14
@@:
    inc  eax
    mov  [f32p],eax
    call draw_infotext
  .nominus:
    cmp  ah,73
    jnz  .noplus
    mov  eax,[f32p]
;   and  eax,15          ; 3 - four partitions, 7 - eight p., 15 - sixteen, etc.
    cmp  eax,15
    jb        @f
    mov  eax,0
@@:
    inc  eax
    mov  [f32p],eax
    call draw_infotext
  .noplus:
    cmp  ah,71
    jnz  .noapply
    call _f32p
  .noapply:

    cmp  ah,32        ; SET SOUND BLASTER 16 BASE
    jnz  nosbm
    sub  [sb16],2
    call draw_infotext
  nosbm:
    cmp  ah,33
    jnz  nosbp
    add  [sb16],2
    call draw_infotext
  nosbp:
    cmp  ah,31
    jnz  nosbs
    call _sb16
  nosbs:

    cmp  ah,42          ; SET SYSTEM LANGUAGE BASE
    jnz  nosysm
    mov  eax,[syslang]
    dec  eax
    jz         still
    mov  [syslang],eax
    call draw_infotext
  nosysm:
    cmp  ah,43
    jnz  nosysp
    mov  eax,[syslang]
    cmp  eax,6
    je         nosysp
    inc  eax
    mov  [syslang],eax
    call draw_infotext
  nosysp:
    cmp  ah,41
    jnz  nosyss
    call _syslang
    call cleantxt
    call loadtxt
    call draw_window
    call drawtime
  nosyss:
    cmp  ah,132        ; SET MOUSE SPEED
    jnz  .nominus
    mov  eax,[mouse_speed]
    sub  eax,2
    cmp  eax,9
    jb        @f
    mov  eax,8
@@:
    inc  eax
    mov  [mouse_speed],eax
    call draw_infotext
  .nominus:
    cmp  ah,133
    jnz  .noplus
    mov  eax,[mouse_speed]
    cmp  eax,9
    jb        @f
    mov  eax,0
@@:
    inc  eax
    mov  [mouse_speed],eax
    call draw_infotext
  .noplus:
    cmp  ah,131
    jnz  .noapply
    call _mouse_speed
  .noapply:
 mousedelay:
    cmp  ah,142        ; SET MOUSE DELAY
    jnz  .nominus
    mov  eax,[mouse_delay]
    sub  eax,2
    cmp  eax,0xfff
    jb        @f
    mov  eax,0xffe
@@:
    inc  eax
    mov  [mouse_delay],eax
    call draw_infotext
  .nominus:
    cmp  ah,143
    jnz  .noplus
    mov  eax,[mouse_delay]
    cmp  eax,0xfff
    jb        @f
    mov  eax,0
@@:
    inc  eax
    mov  [mouse_delay],eax
    call draw_infotext
  .noplus:
    cmp  ah,141
    jnz  .noapply
    call _mouse_delay
  .noapply:

    cmp  ah,3         ; SET KEYMAP
    jne  still
    call _keyboard
    jmp  still

  _keyboard:
    cmp [keyboard],0
    jnz  nosetkeyle
    mov  eax,21       ; english
    mov  ebx,2
    mov  ecx,1
    mov  edx,en_keymap
    mcall
    mov  eax,21
    inc  ecx
    mov  edx,en_keymap_shift
    mcall
    mov  eax,21
    mov  ecx,9
    mov  edx,1
    mcall
    call alt_gen
  nosetkeyle:
    cmp  [keyboard],1
    jnz  nosetkeylfi
    mov  eax,21       ; finnish
    mov  ebx,2
    mov  ecx,1
    mov  edx,fi_keymap
    mcall
    mov  eax,21
    inc  ecx
    mov  edx,fi_keymap_shift
    mcall
    mov  eax,21
    mov  ecx,9
    mov  edx,2
    mcall
    call alt_gen
  nosetkeylfi:
    cmp  [keyboard],2
    jnz  nosetkeylge
    mov  eax,21       ; german
    mov  ebx,2
    mov  ecx,1
    mov  edx,ge_keymap
    mcall
    mov  eax,21
    inc  ecx
    mov  edx,ge_keymap_shift
    mcall
    mov  eax,21
    mov  ecx,9
    mov  edx,3
    mcall
    call alt_gen
  nosetkeylge:
    cmp  [keyboard],3
    jnz  nosetkeylru
    mov  eax,21       ; russian
    mov  ebx,2
    mov  ecx,1
    mov  edx,ru_keymap
    mcall
    mov  eax,21
    inc  ecx
    mov  edx,ru_keymap_shift
    mcall
    call alt_gen
    mov  eax,21
    mov  ecx,9
    mov  edx,4
    mcall
  nosetkeylru:
    cmp  [keyboard],4        ;french
    jnz  nosetkeylfr
    mov  eax,21
    mov  ebx,2
    mov  ecx,1
    mov  edx,fr_keymap
    mcall
    mov  eax,21
    inc  ecx
    mov  edx,fr_keymap_shift
    mcall
    mov  eax,21
    inc  ecx
    mov  edx,fr_keymap_alt_gr
    mcall
    mov  eax,21
    mov  ecx,9
    mov  edx,5
    mcall
  nosetkeylfr:
    cmp  [keyboard],5
    jnz  nosetkeylet
    mov  eax,21       ; estonian
    mov  ebx,2
    mov  ecx,1
    mov  edx,et_keymap
    mcall
    mov  eax,21
    inc  ecx
    mov  edx,et_keymap_shift
    mcall
    mov  eax,21
    mov  ecx,9
    mov  edx,6
    mcall
    call alt_gen
  nosetkeylet:
    ret

 alt_gen:
   mov eax,21
   mov ecx,3
   mov edx,alt_general
   mcall
   ret



draw_buttons:

    pusha

    shl  ecx,16
    add  ecx,12
    mov  ebx,(350-50)*65536+46+BBB

    mov  eax,8
    mcall

    mov  ebx,(350-79)*65536+9
    inc  edx
    mcall

    mov  ebx,(350-67)*65536+9
    inc  edx
    mcall

    popa
    ret



; ********************************************
; ******* WINDOW DEFINITIONS AND DRAW  *******
; ********************************************


draw_window:

    pusha

    mov  eax,12
    mov  ebx,1
    mcall

    xor  eax,eax       ; DRAW WINDOW
    mov  ebx,40*65536+355+BBB
    mov  ecx,40*65536+320
    mov  edx,0x93111199
    mov  edi,title
    mcall

    mov  eax,8             ; APPLY ALL
    mov  ebx,(350-79)*65536+100
    mov  ecx,282*65536+12
    mov  edx,100
    mov  esi,0x005588dd
    mcall
    add  ecx,16*65536         ; SAVE ALL
    dec  edx
    mcall

    mov  esi,0x5580c0

    mov  edx,11
    mov  ecx,43
    call draw_buttons

    mov  edx,41
    mov  ecx,43+8*8
    call draw_buttons

    mov  edx,21
    mov  ecx,43+4*8
    call draw_buttons

    mov  edx,31
    mov  ecx,43+2*8
    call draw_buttons

    mov  edx,3
    mov  ecx,43+10*8
    call draw_buttons

    mov  edx,61
    mov  ecx,43+6*8
    call draw_buttons

    mov  edx,91
    mov  ecx,43+16*8
    call draw_buttons

    mov  edx,71
    mov  ecx,43+12*8
    call draw_buttons

    mov  edx,81
    mov  ecx,43+14*8
    call draw_buttons

    mov  edx,101
    mov  ecx,43+18*8
    call draw_buttons

    mov  edx,111
    mov  ecx,43+20*8 ; 22
    call draw_buttons

    mov  edx,121
    mov  ecx,43+22*8 ; 24
    call draw_buttons

    mov  edx,131
    mov  ecx,43+24*8 ; 26
    call draw_buttons

    mov  edx,141
    mov  ecx,43+26*8 ; 26
    call draw_buttons

    call draw_infotext

    mov  eax,12
    mov  ebx,2
    mcall

    popa
    ret



draw_infotext:

    pusha

    mov  eax,[keyboard]       ; KEYBOARD
    test eax,eax
    jnz  noen
    mov  [text00+LLL*5+28],dword 'ENGL'
    mov  [text00+LLL*5+32],dword 'ISH '
  noen:
    cmp  eax,1
    jnz  nofi
    mov  [text00+LLL*5+28],dword 'FINN'
    mov  [text00+LLL*5+32],dword 'ISH '
  nofi:
    cmp  eax,2
    jnz  noge
    mov  [text00+LLL*5+28],dword 'GERM'
    mov  [text00+LLL*5+32],dword 'AN  '
  noge:
    cmp  eax,3
    jnz  nogr
    mov  [text00+LLL*5+28],dword 'RUSS'
    mov  [text00+LLL*5+32],dword 'IAN '
  nogr:
    cmp  eax,4
    jnz  nofr
    mov  [text00+LLL*5+28],dword 'FREN'
    mov  [text00+LLL*5+32],dword 'CH  '
  nofr:
    cmp  eax,5
    jnz  noet
    mov  [text00+LLL*5+28],dword 'ESTO'
    mov  [text00+LLL*5+32],dword 'NIAN'
  noet:

    mov  eax,[syslang]            ; SYSTEM LANGUAGE
    dec  eax
    test eax,eax
    jnz  noen5
    mov  [text00+LLL*4+28],dword 'ENGL'
    mov  [text00+LLL*4+32],dword 'ISH '
  noen5:
    cmp  eax,1
    jnz  nofi5
    mov  [text00+LLL*4+28],dword 'FINN'
    mov  [text00+LLL*4+32],dword 'ISH '
  nofi5:
    cmp  eax,2
    jnz  noge5
    mov  [text00+LLL*4+28],dword 'GERM'
    mov  [text00+LLL*4+32],dword 'AN  '
  noge5:
    cmp  eax,3
    jnz  nogr5
    mov  [text00+LLL*4+28],dword 'RUSS'
    mov  [text00+LLL*4+32],dword 'IAN '
  nogr5:
    cmp  eax,4
    jne  nofr5
    mov  [text00+LLL*4+28],dword 'FREN'
    mov  [text00+LLL*4+32],dword 'CH  '
  nofr5:
    cmp  eax,5
    jne  noet5
    mov  [text00+LLL*4+28],dword 'ESTO'
    mov  [text00+LLL*4+32],dword 'NIAN'
  noet5:

    mov  eax,[midibase]
    mov  esi,text00+LLL*0+32
    call hexconvert           ; MIDI BASE


    mov  eax,[sb16]           ; SB16 BASE
    mov  esi,text00+LLL*1+32
    call hexconvert


    mov  eax,[cdbase]          ; CD BASE
    cmp  eax,1
    jnz  noe1
    mov  [text00+LLL*2+28],dword 'PRI.'
    mov  [text00+LLL*2+32],dword 'MAST'
    mov  [text00+LLL*2+36],dword 'ER  '
  noe1:
    cmp  eax,2
    jnz  nof1
    mov  [text00+LLL*2+28],dword 'PRI.'
    mov  [text00+LLL*2+32],dword 'SLAV'
    mov  [text00+LLL*2+36],dword 'E   '
  nof1:
    cmp  eax,3
    jnz  nog1
    mov  [text00+LLL*2+28],dword 'SEC.'
    mov  [text00+LLL*2+32],dword 'MAST'
    mov  [text00+LLL*2+36],dword 'ER  '
  nog1:
    cmp  eax,4
    jnz  nog2
    mov  [text00+LLL*2+28],dword 'SEC.'
    mov  [text00+LLL*2+32],dword 'SLAV'
    mov  [text00+LLL*2+36],dword 'E   '
  nog2:


    mov  eax,[hdbase]            ; HD BASE
    cmp  eax,1
    jnz  hnoe1
    mov  [text00+LLL*3+28],dword 'PRI.'
    mov  [text00+LLL*3+32],dword 'MAST'
    mov  [text00+LLL*3+36],dword 'ER  '
  hnoe1:
    cmp  eax,2
    jnz  hnof1
    mov  [text00+LLL*3+28],dword 'PRI.'
    mov  [text00+LLL*3+32],dword 'SLAV'
    mov  [text00+LLL*3+36],dword 'E   '
  hnof1:
    cmp  eax,3
    jnz  hnog1
    mov  [text00+LLL*3+28],dword 'SEC.'
    mov  [text00+LLL*3+32],dword 'MAST'
    mov  [text00+LLL*3+36],dword 'ER  '
  hnog1:
    cmp  eax,4
    jnz  hnog2
    mov  [text00+LLL*3+28],dword 'SEC.'
    mov  [text00+LLL*3+32],dword 'SLAV'
    mov  [text00+LLL*3+36],dword 'E   '
  hnog2:


    mov  eax,[f32p]        ; FAT32 PARTITION
    add  al,48
    mov  [text00+LLL*6+28],al

    mov  eax,[sound_dma]      ; SOUND DMA
    add  eax,48
    mov  [text00+LLL*7+28],al

    mov  eax,[lba_read]
    call onoff          ; LBA READ
    mov  [text00+LLL*8+28],ebx

    mov  eax,[pci_acc]
    call onoff          ; PCI ACCESS
    mov  [text00+LLL*9+28],ebx

    mov  eax,[mouse_speed]      ; MOUSE SPEED
    add  al,48
    mov  [text00+LLL*12+28],al

    mov  eax,[mouse_delay]
    mov  esi,text00+LLL*13+32
    call hexconvert           ; MOUSE DELAY

    call text_out

    popa
    ret

  drawtime:
    mov  ax,[time]      ;hours 22
    mov  cl,1
    call unpacktime
    mov  [text00+LLL*10+28],word bx
    mov  al,ah          ;minutes
    inc  cl
    call unpacktime
    mov  [text00+LLL*10+31],word bx
    mov  eax,[date]
    mov  ch,3
    call unpackdate
    mov  [text00+LLL*11+34],word bx    ;year   24
    mov  al,ah
    mov  ch,1
    call unpackdate
    mov  [text00+LLL*11+28],word bx    ;month
    bswap eax
    mov  al,ah
    inc  ch
    call unpackdate
    mov  [text00+LLL*11+31],word bx    ;day

text_out:
    mov  eax,13
    mov  ebx,175*65536+85
    mov  ecx,40*65536+225
    mov  edx,0x80111199-19
    mcall

    mov  edx,text00
    mov  ebx,10*65536+45
    mov  eax,4
    mov  ecx,0xffffff
    mov  esi,LLL
    mov  ebp,text1_strings
  newline:
    mcall
    add  ebx,8+8
    add  edx,esi
    dec  ebp
    jnz  newline
    mov  ebp,text2_strings
    add  ebx,8+8
  @@:
    mcall
    add  ebx,8+8
    add  edx,esi
    dec  ebp
    jnz  @b
    ret

  unpacktime:
    cmp  byte [blinkpar],cl    ;translate packed number to ascii
    jne  unpack1
  chkblink:
    bt dword [blinkpar],16
    jnc  unpack1
    xor  bx,bx
    ret
  unpackdate:
    cmp  byte [blinkpar+1],ch
    je         chkblink
  unpack1:
    xor  bx,bx
    mov  bh,al
    mov  bl,al
    and  bh,0x0f
    shr  bl,4
    add  bx,0x3030
    ret

  hexconvert:        ;converting dec to hex in ascii
    xor  ebx,ebx
    mov  bl,al
    and  bl,15
    add  ebx,hex
    mov  cl,[ebx]
    mov  [esi],cl
    shr  eax,4
    xor  ebx,ebx
    mov  bl,al
    and  bl,15
    add  ebx,hex
    mov  cl,[ebx]
    dec  esi
    mov  [esi],cl
    shr  eax,4
    xor  ebx,ebx
    mov  bl,al
    and  bl,15
    add  ebx,hex
    mov  cl,[ebx]
    dec  esi
    mov  [esi],cl
    ret

onoff:
    cmp [syslang],4
    jne norus1
    mov ebx,'ÑÄ  '
    cmp eax,1
    je        exitsub
    mov ebx,'çÖí '
    ret
 norus1:
    mov ebx,'ON  '
    cmp eax,1
    je        exitsub
    mov ebx,'OFF '
 exitsub:
    ret

_midibase:
    mov  eax,21
    mov  ebx,1
    mov  ecx,[midibase]
    mcall
 ret

_cdbase:
    mov  eax,21
    mov  ebx,3
    mov  ecx,[cdbase]
    mcall
 ret

_hdbase:
    mov  eax,21
    mov  ebx,7
    mov  ecx,[hdbase]
    mcall
    ret

_sound_dma:
    mov  eax,21
    mov  ebx,10
    mov  ecx,[sound_dma]
    mcall
    ret

_lba_read:
    mov  eax,21
    mov  ebx,11
    mov  ecx,[lba_read]
    mcall
    ret

_pci_acc:
    mov  eax,21
    mov  ebx,12
    mov  ecx,[pci_acc]
    mcall
    ret

_f32p:
    mov  eax,21
    mov  ebx,8
    mov  ecx,[f32p]
    mcall
 ret

_sb16:
    mov  eax,21
    mov  ebx,4
    mov  ecx,[sb16]
    mcall
    ret

_syslang:
    mov  eax,21
    mov  ebx,5
    mov  ecx,[syslang]
    mcall
 ret

_mouse_speed:
    mov  eax,18
    mov  ebx,19
    mov  ecx,1
    mov  edx,[mouse_speed]
    mcall
 ret

_mouse_delay:
    mov  eax,18
    mov  ebx,19
    mov  ecx,3
    mov  edx,[mouse_delay]
    mcall
 ret

loadtxt:
    cld
    mov  edi,text00
    mov  ecx,488 ;28
    cmp  [syslang],4
    jne  norus
    mov  esi,textrus
    jmp  sload
  norus:
    mov  esi,texteng
  sload:
    rep  movsd
    ret

cleantxt:
    xor  eax,eax
    mov  ecx,428
    cld
    mov  edi,text00
    rep stosd
    mov  [text00+1711],byte 'x'
    ret

; DATA AREA
count:          db 0x0
blinkpar: dd 0x0
time:        dw 0x0
date:        dd 0x0

textrus:

    db 'Å†ß† MIDI ROLAND MPU-401  : 0x320           - +   è‡®¨•≠®‚Ï'
    db 'Å†ß† SoundBlaster 16      : 0x240           - +   è‡®¨•≠®‚Ï'
    db 'Å†ß† CD-ROM†              : PRI.SLAVE       - +   è‡®¨•≠®‚Ï'
    db 'Å†ß† ÜÑ-1                 : PRI.MASTER      - +   è‡®¨•≠®‚Ï'
    db 'üßÎ™ ·®·‚•¨Î              : ENGLISH         - +   è‡®¨•≠®‚Ï'
    db 'ê†·™´†§™† ™´†¢®†‚„‡Î      : ENGLISH         - +   è‡®¨•≠®‚Ï'
    db 'ê†ß§•´ FAT32 ≠† ÜÑ-1      : 1               - +   è‡®¨•≠®‚Ï'
    db 'á¢„™Æ¢Æ© ™†≠†´ DMA        : 1               - +   è‡®¨•≠®‚Ï'
    db 'Ç™´ÓÁ®‚Ï LBA              : OFF             - +   è‡®¨•≠®‚Ï'
    db 'ÑÆ·‚„Ø ™ Ë®≠• PCI         : OFF             - +   è‡®¨•≠®‚Ï'
    db 'ë®·‚•¨≠Æ• ¢‡•¨Ô           :  0:00           - +     ÇÎ°Æ‡  '
    db 'ë®·‚•¨≠†Ô §†‚† (¨,§,£)    : 00/00/00        - +     ÇÎ°Æ‡  '
    db 'ë™Æ‡Æ·‚Ï ™„‡·Æ‡† ¨ÎË®     : 1               - +   è‡®¨•≠®‚Ï'
    db 'á†§•‡¶™† „·™Æ‡•≠®Ô ¨ÎË®   : 0x00a           - +   è‡®¨•≠®‚Ï'

    db 'ÇçàåÄçàÖ:                                    è‡®¨•≠®‚Ï ¢·• '
    db 'çÖ áÄÅìÑúíÖ ëéïêÄçàíú çÄëíêéâäà              ëÆÂ‡†≠®‚Ï ¢·• '

texteng:

    db 'MIDI: ROLAND MPU-401 BASE : 0x320           - +     APPLY  '
    db 'SOUND: SB16 BASE          : 0x240           - +     APPLY  '
    db 'CD-ROM BASE               : PRI.SLAVE       - +     APPLY  '
    db 'HARDDISK-1 BASE           : PRI.MASTER      - +     APPLY  '
    db 'SYSTEM LANGUAGE           : ENGLISH         - +     APPLY  '
    db 'KEYBOARD LAYOUT           : ENGLISH         - +     APPLY  '
    db 'FAT32-1 PARTITION IN HD-1 : 1               - +     APPLY  '
    db 'SOUND DMA CHANNEL         : 1               - +     APPLY  '
    db 'LBA READ ENABLED          : OFF             - +     APPLY  '
    db 'PCI ACCESS FOR APPL.      : OFF             - +     APPLY  '
    db 'SYSTEM TIME               :  0:00           - +    SELECT  '
    db 'SYSTEM DATE (M,D,Y)       : 00/00/00        - +    SELECT  '
    db 'Mouse pointer speed       : 1               - +     APPLY  '
    db 'Mouse pointer delay       : 0x00a           - +     APPLY  '
text1_strings = 14

    db 'NOTE:                                           APPLY ALL  '
    db 'SAVE YOUR SETTINGS BEFORE QUIT KOLIBRI          SAVE ALL   '
text2_strings = 2

title  db 'SETUP',0

hex db         '0123456789ABCDEF'

alt_general:

;     db   ' ',27
;     db   ' @ $  {[]}\ ',8,9
;     db   '            ',13
;     db   '             ',0,'           ',0,'4',0,' '
;     db   '             ',180,178,184,'6',176,'7'
;     db   179,'8',181,177,183,185,182
;     db   'ABCD',255,'FGHIJKLMNOPQRSTUVWXYZ'
;     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
;     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
;     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'


en_keymap:

     db   '6',27
     db   '1234567890-=',8,9
     db   'qwertyuiop[]',13
     db   '~asdfghjkl;',39,96,0,'\zxcvbnm,./',0,'45 '
     db   '@234567890123',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'AB<D',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'


en_keymap_shift:

     db   '6',27
     db   '!@#$%^&*()_+',8,9
     db   'QWERTYUIOP{}',13
     db   '~ASDFGHJKL:"~',0,'|ZXCVBNM<>?',0,'45 '
     db   '@234567890123',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'AB>D',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'


fr_keymap:

     db   '6',27
     db   '&é"',39,'(-ç_ìê)=',8,9
     db   'azertyuiop^$',13
     db   '~qsdfghjklmí',0,0,'*wxcvbn,;:!',0,'45 '
     db   '@234567890123',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'AB<D',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'



fr_keymap_shift:


     db   '6',27
     db   '1234567890+',8,9
     db   'AZERTYUIOPïî',13
     db   '~QSDFGHJKLM%',0,'ñWXCVBN?./',0,'45 '
     db   '@234567890123',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'AB>D',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'


fr_keymap_alt_gr:


     db   '6',27
     db   28,'~#{[|ò\^@]}',8,9
     db   'azertyuiop^$',13
     db   '~qsdfghjklmí',0,0,'*wxcvbn,;:!',0,'45 '
     db   '@234567890123',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'AB<D',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'




fi_keymap:

     db   '6',27
     db   '1234567890+[',8,9
     db   'qwertyuiop',192,'~',13
     db   '~asdfghjkl',194,193,'1',0,39,'zxcvbnm,.-',0,'45 '
     db   '@234567890123',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'AB<D',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'


fi_keymap_shift:

     db   '6',27
     db   '!"#è%&/()=?]',8,9
     db   'QWERTYUIOP',200,'~',13
     db   '~ASDFGHJKL',202,201,'1',0,'*ZXCVBNM;:_',0,'45 '
     db   '@234567890123',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'AB>D',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'



ge_keymap:

     db   '6',27
     db   '1234567890?[',8,9
     db   'qwertzuiop',203,'~',13
     db   '~asdfghjkl',194,193,'1',0,39,'yxcvbnm,.-',0,'45 '
     db   '@234567890123',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'AB<D',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'


ge_keymap_shift:

     db   '6',27
     db   '!"#$%&/()=',197,']',8,9
     db   'QWERTZUIOP',195,'~',13
     db   '~ASDFGHJKL',202,201,'1',0,'*YXCVBNM;:_',0,'45 '
     db   '@234567890123',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'AB>D',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'

ru_keymap:

     db   '6',27
     db   '1234567890-=',8,9
     db   '©Ê„™•≠£ËÈßÂÍ',13
     db   0,"‰Î¢†Ø‡Æ´§¶Ì"
     db   0xf1, '-/'
     db   "ÔÁ·¨®‚Ï°Ó",'.-','45 '
     db   '@234567890123',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'AB<D',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'



ru_keymap_shift:

     db   '6',27
     db   '!"N;%:?*()_+',8,0
     db   "âñìäÖçÉòôáïö",13
     db   0,"îõÇÄèêéãÑÜù"
     db   0xf0, '-\'
     db   "üóëåàíúÅû",',-','45 '
     db   '@234567890123',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'AB>D',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'

et_keymap:

     db   '6',27
     db   '1234567890+¥',8,9
     db   'qwertyuiop¸ı',13
     db   '~asdfghjklˆ‰','1',0,'<zxcvbnm,.-',0,'45 '
     db   '@234567890123',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'AB<D',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'


et_keymap_shift:

     db   '6',27
     db   '!"#§%&/()=?`',8,9
     db   'QWERTYUIOP‹’',13
     db   '~ASDFGHJKL÷ƒ','1',0,'>ZXCVBNM;:_',0,'45 '
     db   '@234567890123',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'AB>D',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'

read_fileinfo:
       dd 0
       dd 0
       dd 0
       dd 48
       dd keyboard
       db 0
       dd file_name

save_fileinfo:
       dd 2
       dd 0
       dd 0
       dd 48
       dd keyboard
file_name:   db '/rd/1/setup.dat',0

I_PARAM   dd 0

keyboard     dd 0x0
midibase     dd 0x320
cdbase         dd 0x2
sb16         dd 0x220
syslang      dd 0x1
hdbase         dd 0x1
f32p         dd 0x1
sound_dma    dd 0x1
lba_read     dd 0x1
pci_acc      dd 0x1
mouse_speed  dd 0x3
mouse_delay  dd 0x10
text00:

I_END:
table_area:

;-------------------------------------------------------------------------
;
;    Файл конфигурации
;
;
;     SPraid
;
;-------------------------------------------------------------------------

conf_file_loaded: db 0			; флаг загрузки конфига

; загрузка ини файла в библиотеку  
proc load_conf_file
  pushad
  invoke ini.load,.fname
  mov [conf_file_loaded],byte 1
  popad
  ret
  .fname db '%sys%/sys.conf',0
endp


proc set_kernel_conf
locals
  par db 30 dup(?)
endl
  pushad
  ;[gui]
  ;mouse_speed
  mov eax,ebp
  add eax,par-ebp
  push eax
  invoke ini.get_str, ugui, ugui_mouse_speed, eax, ugui_mouse_speed_def        
  pop eax
  stdcall strtoint,eax 
  push eax
  pop edx
  call _mouse_speed

  popad
  ret
  
  ;mouse_delay
  mov eax,ebp
  add eax,par-ebp
  push eax
  invoke ini.get_str, ugui, ugui_mouse_delay, eax, ugui_mouse_delay_def
  pop eax
  stdcall strtoint,eax
  push eax
  pop edx
  call _mouse_delay

  ;[dev]
  ;sb16
  mov eax,ebp
  add eax,par-ebp
  push eax
  invoke ini.get_str, udev, udev_sb16, eax, udev_sb16_def
  pop eax
  stdcall strtoint,eax
  push eax
  pop ecx
  call _sb16
  
  ;sound_dma
  mov eax,ebp
  add eax,par-ebp
  push eax
  invoke ini.get_str, udev, udev_sound_dma, eax, udev_sound_dma_def
  pop eax
  stdcall strtoint,eax
  push eax
  pop ecx
  call _sound_dma  
  
  
  ;midibase
  mov eax,ebp
  add eax,par-ebp
  push eax
  invoke ini.get_str, udev, udev_midibase, eax, udev_midibase_def
  pop eax
  stdcall strtoint,eax
  push eax
  pop ecx
  call _midibase

  popad
  ret
endp

ugui db 'gui',0
ugui_mouse_speed db 'mouse_speed',0
ugui_mouse_speed_def db '2',0
ugui_mouse_delay db 'mouse_delay',0
ugui_mouse_delay_def db '0x00A',0

udev db 'dev',0
udev_sb16 db 'sb16',0
udev_sb16_def db '0x220',0
udev_sound_dma db 'sound_dma',0
udev_sound_dma_def db '1',0
udev_midibase db 'midibase',0
udev_midibase_def db '0x320',0

; конверчение строки в DWord в eax (по второму символу определяет систему счисления)
     
proc strtoint stdcall,strs
  pushad
  
  mov eax,[strs]
  inc eax
  mov bl,[eax]
  cmp bl,'x'
  je .hex
  cmp bl,'X'
  je .hex
  jmp .dec
.hex:
  inc eax
  stdcall strtoint_hex,eax
  jmp .exit
.dec:
  dec eax
  stdcall strtoint_dec,eax
.exit:
  mov [esp+28],eax
  popad
  ret  
endp

; конверчение строки в DWord в eax для десятичного
proc strtoint_dec stdcall,strs
  pushad
  xor edx,edx
  ; поиск конца
  mov esi,[strs]
@@:
  lodsb
  or al,al
  jnz @b
  mov ebx,esi
  mov esi,[strs]
  dec ebx
  sub ebx,esi
  mov ecx,1

@@:
  dec ebx
  or ebx,ebx
  jz @f
  imul ecx,ecx,10  ; порядок
  jmp @b
@@:

 xchg ebx,ecx

  
  xor ecx,ecx
  

@@:  
  xor eax,eax
  lodsb
  cmp al,0
  je .eend
  
  sub al,30h
  imul ebx
  add ecx,eax
  push ecx
  xchg eax,ebx
  mov ecx,10
  div ecx
  xchg eax,ebx
  pop ecx
  jmp @b
  
.eend:
  mov [esp+28],ecx
  popad
  ret
endp

; конверчение строки в DWord в eax для шеснадцатиричного
proc strtoint_hex stdcall,strs
  pushad
  xor edx,edx

  mov esi,[strs]
  mov ebx,1
  add esi,1

@@:
  lodsb
  or al,al
  jz @f
  shl ebx,4
  jmp @b
@@:
  xor ecx,ecx
  mov esi,[strs]

@@:  
  xor eax,eax
  lodsb
  cmp al,0
  je .eend
  
  cmp al,'a'
  jae .bm
  cmp al,'A'
  jae .bb
  jmp .cc
.bm:    ; 57h
  sub al,57h
  jmp .do
 
.bb:    ; 37h
  sub al,37h
  jmp .do

.cc:    ; 30h
  sub al,30h
  
.do:  
  imul ebx
  add ecx,eax
  shr ebx,4

  jmp @b

.eend:
  mov [esp+28],ecx
  popad
  ret
endp    


; установки из setup

_mouse_speed:
    mov  eax,18
    mov  ebx,19
    mov  ecx,1
    int 0x40
 ret

_mouse_delay:
    mov  eax,18
    mov  ebx,19
    mov  ecx,3
    int 0x40
 ret
 
_sb16:
    mov  eax,21
    mov  ebx,4
    int 0x40
 ret
    
_sound_dma:
    mov  eax,21
    mov  ebx,10
    int 0x40
 ret
 
 
_midibase:
    mov  eax,21
    mov  ebx,1
    int 0x40
 ret
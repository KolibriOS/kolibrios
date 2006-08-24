SYS equ meos
midi_parse:
    and  [max_note],0
    and  [cur_track],0
    mov  [min_note],0xff
    mov  edi,[midi_limit]
    cmp  dword[workarea],'MThd'
    je   .head_ok
    mov  edx,sHeadInv
    call debug_outstr
    jmp  clearpath
  .head_ok:
    cmp  dword[workarea+4],0x6000000
    je   .heads_ok
    mov  edx,sHSizeInv
    call debug_outstr
    jmp  clearpath
  .heads_ok:
    cmp  dword[workarea+8],0x1000000
    jmp   .headt_ok
    mov  edx,sTypeUnsup
    call debug_outstr
    jmp  clearpath
  .headt_ok:
    movzx eax,word[workarea+12]
    rol  ax,8
    mov  [quarter],eax
    mov  [tempo],50
    mov  esi,workarea+0xe
skip_sections:
    lodsd
    cmp  eax,'MTrk'
    je   track_found
if SYS eq meos
;    dps  <'What?',13,10>
end if
    lodsd
    add  esi,eax
    cmp  esi,[midi_limit]
    jbe  skip_sections
 if NONCRITICAL_MSG eq 1
    dps  'No more tracks'
 end if
    and  word[edi],0
;    ud2
    jmp  decode_end
track_found:
if SYS eq meos1
    dps  <13,10,'Track '>
    push esi
    sub  esi,workarea
;    dpd  esi
    pop  esi
end if
    lodsd
    bswap eax
    mov  bl,[cur_track]
    cmp  bl,[sel_track]
    je   .trk_fnd
    add  esi,eax
    jmp  next_event.eot
  .trk_fnd:
    mov  [track_len],eax
    dps  'TRK'
next_event:
    call readvar
;    dpd  eax
    add  [delta],eax
    lodsw             ; al -event, ah - next byte
if SYS eq meos1
    dph  eax
    dps  <' ',13,10>
    newline
end if
    test al,0x80      ; check if a valid event
    jnz  .valid_evt2
    dec  esi
    shl  ax,8
    mov  al,[prev_cmd]
    jmp  .valid_evt
  .valid_evt2:
    mov  [prev_cmd],al
  .valid_evt:
    cmp  al,0xf0
    jne  .nosysex
    dec  esi
    call readvar
    add  esi,eax
    jmp  next_event
  .nosysex:
    cmp  al,0xff
    jne  .no_meta

  ; meta events
    cmp  ah,0x51
    jne  .notempo
    push eax edx
    mov  eax,[esi]
    xor  al,al
    bswap eax
    xor  edx,edx
    mov  ebx,10000
    div  ebx
    pop  edx eax
    jmp  .no_eot
  .notempo:
    cmp  ah,0x2f      ; end of track
    jne  .no_eot
    inc  esi
if SYS eq meos1
    dps  <13,10,'EOT'>
    push esi
    sub  esi,workarea
    dpd  esi
    pop  esi
;    mcall 5,200
;    dph  eax
;    ud2
end if
  .eot:
;    dps  'EOT '
    inc  [cur_track]
    jmp  skip_sections;decode_end
  .no_eot:
    lodsb
    movzx ecx,al    ; ecx - length of metadata
    add  esi,ecx
    jmp  next_event
  .no_meta:
    cmp  al,0xfa    ; system ctl events
    jb   .no_sys
  .dec_esi:
    dec  esi
    jmp  next_event
  .no_sys:
    cmp  al,0xf8
    je   .dec_esi
    movzx ecx,al    ; ecx - MIDI Event Command
    and  ecx,0xf    ; cl - channel
    and  al,0xf0    ; al - event code

    cmp  al,0xe0    ; Pitch wheel change
    je   .inc_esi
    cmp  al,0xb0
    ja   .no_inc
  .inc_esi:
    inc  esi
  .no_inc:
    cmp  ecx,[channel]     ; Channel must be 0 !!!
    jz   .chan_ok
 if NONCRITICAL_MSG eq 1
    dps  'C'         ; Reference to unsupported channel !!!
    dpd  ecx
    mov  ecx,esi
    dps  '-'
    sub  ecx,workarea+1
    dpd  ecx
 end if
    jmp  next_event
  .chan_ok:
    cmp  al,0x90     ; Note On
    jne  .no_noon
    add  al,[octave]
    cmp  [curnote],0x80
    je   .note_ok
 if NONCRITICAL_MSG eq 1
    dps  'N!'        ; Note On without Off !!!
 end if
  .note_ok:
;    dps  'N+'
;    movzx  ecx,ah
;    dpd  ecx
    call insert_pause
    mov  [curnote],ah ; [curnote]=note number
    jmp  next_event
  .no_noon:
    cmp  al,0x80     ; Note Off
    jne  .no_nooff
    add  ah,[octave]
    cmp  ah,[curnote]
    je   .off_ok
 if NONCRITICAL_MSG eq 1
    dps  'n!'        ; Note Off mismatch !!!
 end if
  .off_ok:
;    dps  'N-'
    cmp  ah,[max_note]
    jbe  .nomax
    mov  [max_note],ah
  .nomax:
    cmp  ah,[min_note]
    jae  .ins
    mov  [min_note],ah
  .ins:
    call insert_note
    mov  [curnote],al

  .no_nooff:         ; No more supported events
    jmp  next_event
prev_cmd db ?
max_note db ?
min_note db ?
;   *********************************************
;   *******  READ VARIABLE BYTES ****************
;   *********************************************

readvar:
; in:  esi - pointer;
; out: esi - new pointer; eax - value;
    push ebx ecx
    movzx ebx,byte[esi]
    inc  esi
    btr  ebx,7
    jnc  .exit
  .next:
    shl  ebx,7
    lodsb
    mov  ecx,eax
    and  eax,0x7f
    add  ebx,eax
    cmp  cl,0x7f
    ja   .next
  .exit:
    mov  eax,ebx
    pop  ecx ebx
    ret

;   *********************************************
;   *******  INSERT PAUSE ***********************
;   *********************************************

insert_pause:
    cmp  [delta],0
    jz  return
    push eax ebx ecx
    mov  ah,0xff
    jmp  write_note

;   *********************************************
;   *******  INSERT NOTE ************************
;   *********************************************

insert_note:    ; ah - note code
    push eax ebx ecx
    movzx eax,ah
    mov  ebx,12
    div  bl
    shl  al,4
    add  ah,al
    sub  ah,0x1f
write_note:
    push eax
    mov  eax,[delta]
    mov  edx,[tempo]
    mul  edx
    mov  ecx,[quarter]
    div  ecx    ; ax - note delay
    cmp  eax,0x7f
    jb   .ok
    mov  eax,0x7f
  .ok:
    movzx ecx,al
    or   ecx,0x80
    pop  eax
    mov  al,cl
    stosw
    xor  eax,eax
    mov  [delta],eax
    pop ecx ebx eax
return:
    ret

sHeadInv:
if lang eq ru
  db "Неверный заголовок",0
else
  db "Header invalid",0
end if

sHSizeInv:
if lang eq ru
  db 'Неверный размер заголовка',0
else
  db 'Header size invalid',0
end if

sTypeUnsup:
if lang eq ru
  db 'Тип MIDI не поддерживается',0
else
  db 'MIDI type not supported',0
end if

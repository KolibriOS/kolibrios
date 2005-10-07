;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;;  SB16.INC                                                       ;;
;;                                                                 ;;
;;  Sound Blaster 16 functions for MenuetOS                        ;;
;;                                                                 ;;
;;  Copyright 2002 Paolo Minazzi, paolo.minazzi@inwind.it          ;;
;;                                                                 ;;
;;  See file COPYING for details                                   ;;
;;                                                                 ;;
;;  - 11.07.2002 8 bit stereo mode - Ville Turjanmaa               ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


SB16_load_music  equ   0xc0000000
SB16_play_music  equ   0xc0000001
DMAPage          equ   0x2A
Rate             equ   44100
SB16Buffer       equ   0x2A0000
SB16_Status      equ   SB16Buffer+65536

iglobal
  sound_data_format   dd    0x1
  sound_data_length   dd    65536
  sound_data_freq     dd    44100
endg

sound_interface:

   cmp  eax,0                     ; Load data
   jne  no_SB16_load_music
   mov  edi,[0x3010]
   add  edi,0x10
   add  ebx,[edi]
   call code_SB16_load_music
   ret
 no_SB16_load_music:

   cmp  eax,1                     ; Play data
   jne  no_SB16_play_music
   call code_SB16_play_music
   ret
 no_SB16_play_music:

   cmp  eax,2                     ; Set data formats
   jne  no_SB16_data_format
   cmp  ebx,0                     ; ebx=0 play format
   jne  no_sound_format
   mov  [sound_data_format],ecx     ; 1=8b mono, 2=8b stereo
   ret
  no_sound_format:
   cmp  ebx,1                     ; ebx=1 data length
   jne  no_sound_length
   mov  [sound_data_length],ecx     ;
   ret
 no_sound_length:
   cmp  ebx,2                     ; ebx=2 sound data frequency
   jne  no_sound_freq
   mov  [sound_data_freq],ecx
   ret
 no_sound_freq:
   ret

 no_SB16_data_format:

;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   cmp   eax, edi                      ; this is subfunction #55 ?
    jne  retFunc55                     ; if no then return.
   cmp   byte [sound_flag],0           
   jne   retFunc55
   movzx eax, byte [countDelayNote]
   or    al, al                        ; player is busy ?
    jnz  retFunc55                     ; return counter delay Note
;   mov   eax, [0x3010]
;   mov   eax, [eax+0x10]               ; address application im memory
;   add   eax, edx                      ; add offset Delay-Note string
;   mov   [memAdrNote], eax
   mov   [memAdrNote],edx
   mov   eax,[0x3010]
   mov   eax,[eax+4]
   mov   [pidProcessNote],eax
   xor   eax, eax                      ; Ok!  EAX = 0
 retFunc55:
   mov   [esp+36], eax                 ; return value EAX for application
;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   ret




code_SB16_play_music:

        cmp  [sound_data_format],1
        jne  no_sound_8bm
        call sb_play_8b_mono
        ret
      no_sound_8bm:

        cmp  [sound_data_format],2
        jne  no_sound_8bs
        call sb_play_8b_stereo
        ret
      no_sound_8bs:

        ret





Blaster_command:

  push eax
  push ecx
  push edx

  mov  dx,word [sb16]
  add  dx,0xc
  mov  cx,1000
 bcl1:
  in   al,dx
  and  al,128
  jz   bcl2
  loop bcl1
 bcl2:
  mov  al,[esp+8]
  mov  dx,[esp+0]
  add  dx,word [sb16]
  out  dx,al

  pop  edx
  pop  ecx
  pop  eax

  ret


sb_play_8b_stereo:

  pusha

  call  sb_set_dma

  call  sb_set_stereo

  mov   dx,0xc
  mov   al,0xa8
  call  Blaster_command

  mov   al,0x40
  call  Blaster_command

  mov   al,245
  call  Blaster_command

  mov   al,0x48
  call  Blaster_command

  mov   al,0xff
  call  Blaster_command
  call  Blaster_command

  mov   al,0x91
  call  Blaster_command

  popa
  ret



sb_set_stereo:

  push eax
  push edx

  call sb_wait

  mov  dx,word [sb16]
  add  dx,0x4
  mov  al,0xe
  out  dx,al
  inc  dx
  in   al,dx
  and  al,253
  or   al,2     ; stereo
  out  dx,al

  pop  edx
  pop  eax
  ret



code_SB16_load_music:

     cmp byte [SB16_Status],1
     je  nol
     mov edi,SB16Buffer
     mov esi,ebx
     mov ecx,65536/4
     cld
     rep movsd
nol: ret


iglobal
  dma_table  db   0x87,0x83,0x81,0x82
endg



;--------------------------------
;        program dma
;--------------------------------

sb_set_dma:

         pusha

         mov eax,[sound_dma]
         add eax,4
         out 0xa,al

         mov al,0
         out 0xc,al

         mov eax,[sound_dma]
         add eax,0x48
         out 0xb,al

         mov edx,[sound_dma]
         shl edx,1
         mov al,0
         out dx,al

         mov al,0
         out dx,al

         mov   edx,[sound_dma]
         add   edx,dma_table
         movzx edx,byte [edx]
         mov   al,DMAPage
         out   dx,al

         mov edx,[sound_dma]
         shl edx,1
         inc edx
         mov eax,[sound_data_length]
         dec eax
         and eax,0xff
         ; mov al,(DataLength-1) and 0xff
         out dx,al

         mov eax,[sound_data_length]
         dec eax
         shr eax,8
         ; mov al,(DataLength-1) shr 8
         out dx,al

         mov eax,[sound_dma] ; DMA
         out 0xa,al

         popa
         ret



sb_play_8b_mono:


        call sb_set_dma

        cmp byte [SB16_Status],1
        jne contsb16
        jmp retserve
      contsb16:

        mov dx,word [sb16]
        add dx,4
        mov ecx,[sound_dma]
        mov ax,0x01
        shl ax,cl
        shl ax,8
        add ax,0x81
        out dx,ax

        mov ax,0f280h   ;enable irq5
        out dx,ax


adr1_SB:   mov dx,word [sb16]
           add dx,0ch
           in al,dx
           and al,080h
           jnz adr1_SB

           call sb_set_stereo

           mov al,0d1h
           out dx,al


        mov   dx,word [sb16]
        add   dx,0ch

        call  sb_wait

        mov   al,40h                              ; Rate
        out   dx,al
        call  sb_wait
        mov   al,256-1000000/Rate
        out   dx,al

        call  sb_wait

        mov   al,14h                              ; Datalength
        out   dx,al
        call  sb_wait

        mov eax,[sound_data_length]
        dec eax
        and eax,0xff
        ;mov   al,(DataLength-1) and 0xff
        out   dx,al
        call  sb_wait
        mov eax,[sound_data_length]
        dec eax
        shr eax,8
        ;mov   al,(DataLength-1) shr 8
        out   dx,al

    retserve:

        ret


sb_wait: in al,dx                                ;wait
         and al,080h
         jnz sb_wait

         ret




;****************************************
; END CODE SB16 by Minazzi Paolo
;***************************************

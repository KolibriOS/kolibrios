;--------------------------------
;        program dma
;--------------------------------
sb_set_dma:
	 mov ebx,[sound_dma]
	 lea eax,[ebx+4] ;mask required channel
	 cmp bl,4
	 ja  .use_second_dma_controller
	 jb  @f
.dma_setup_error:
if DEBUG
	 mov esi,msgErrDMAsetup
	 call SysMsgBoardStr
end if
	 mov dword[esp],START.stop
	 ret
@@:
if use_cli_sti
	 cli	    ;here to minimize time with disabled ints
end if
	 out 0xA,al ;mask required channel

	 xor eax,eax
	 out 0xC,al ;clear byte pointer flip-flop register

	 lea eax,[ebx+0x58] ;auto-init mode for channel (ebx)
	 out 0xB,al ;DMA channel 0-3 mode register

	 movzx edx,byte[ebx+dma_table] ;page register
	 mov   al,DMAPage
	 out   dx,al

	 lea edx,[ebx*2] ;DMA channel 0-3 base address

	 mov al,0   ;LSB is 0
	 out dx,al

;        mov al,0   ;MSB is 0 too
	 out dx,al

	 inc edx    ;DMA channel 0-3 byte count

	 mov al,((sb_buffer_size-1) and 0xff)
	 out dx,al

         mov al,((sb_buffer_size-1) shr 8) ;it is the same
	 out dx,al

	 mov eax,ebx ;unmask DMA channel
	 out 0xA,al

if use_cli_sti
	 sti
end if
	 ret

.use_second_dma_controller:
	 cmp bl,7
	 ja .dma_setup_error

	 sub bl,4
	 sub al,4
if use_cli_sti
	 cli	    ;here to minimize time with disabled ints
end if
	 out 0xD4,al ;mask required channel

	 xor eax,eax
	 out 0xD8,al ;clear byte pointer flip-flop register

	 lea eax,[ebx+0x58] ;auto-init mode for channel (ebx+4)
	 out 0xD6,al ;DMA channel 4-7 mode register

	 movzx edx,byte[ebx+dma_table+4] ;page register
	 mov   al,DMAPage
	 out   dx,al

	 lea edx,[ebx*4+0xC0] ;DMA channel 4-7 base address

	 mov al,0   ;LSB is 0 ;for 16bit DMA this contains
	 out dx,al  ;A1-A8 lines of address bus, A0 is zero

;        mov al,0   ;MSB is 0 too ;for 16bit DMA this contains
	 out dx,al  ;A9-A16 lines of address bus

	 inc edx
	 inc edx    ;DMA channel 4-7 16bit word count

	 mov al,(((sb_buffer_size/2)-1) and 0xff)
	 out dx,al

	 mov al,(((sb_buffer_size/2)-1) shr 8)
	 out dx,al

	 mov eax,ebx ;unmask DMA channel
	 out 0xD4,al

if use_cli_sti
	 sti
end if
	 ret
;-------------------------------------------------------------------------------
;       out byte to SB DSP's write port
;-------------------------------------------------------------------------------
macro	sb_out data_to_out {
@@:
	in   al,dx
	test al,al  ;is DSP busy?
	js   @b     ;it's busy
	mov  al,data_to_out ;it's free
	out  dx,al
}
;-------------------------------------------------------------------------------
;       stop playing
;-------------------------------------------------------------------------------
proc sb_stop
	mov  edx,[sb_base_port]
	add  dl,0xC
	sb_out 0xD3 ;turn the speaker off
	sb_out 0xDA ;exit 8bit DMA
	sb_out 0xD9 ;exit 16bit DMA
	ret
endp
;-------------------------------------------------------------------------------
;       start playing
;-------------------------------------------------------------------------------
proc sb_play
	and  [int_flip_flop],0
	mov  edx,[sb_base_port]
	add  dl,0xC
	sb_out 0xD1 ;turn speaker on
;        sb_out 0x48 ;set DSP transfer size  ;for older cards

;        mov  ax,32767 ;(64k)/2-1
;@@:                 ;out the low byte...
;        in   al,dx
;        test al,al  ;is DSP busy?
;        js   @b     ;it's busy
;        out  dx,al

;        mov  al,ah  ;...then the high byte
;@@:
;        in   al,dx
;        test al,al  ;is DSP busy?
;        js   @b     ;it's busy
;        out  dx,al

;        sb_out 0x1C ;auto-init 8bit playback

;              0xBXh - 16 bit DMA mode
;              ||||
	sb_out 10110110b ;bCommand
;                  ||||
;                  |||+-reserved
;                  ||+--turn FIFO on (0 for off)
;                  |+---auto-init mode on (0 for off)
;                  +----A/D: 0-output, 1-input
;                +------stereo on
;                |+-----unsigned (1 for signed)
;                ||
	sb_out 00110000b ;bMode
;              ||  ||||
;              ---------reserved
;wSize is a number of 16bit samples less 1. For auto-init mode each half
;buffer is (64k)/2 bytes long and, obviously, contains ((64k)/2)/2 bytes
	sb_out (((sb_buffer_size/2/2)-1) and 0xFF) ;wSize.LowByte
	sb_out (((sb_buffer_size/2/2)-1) shr 8)    ;wSize.HighByte
	ret
endp
;-------------------------------------------------------------------------------
;       reset DSP
;-------------------------------------------------------------------------------
proc sb_reset
	and  [int_flip_flop],0
	mov  edx,[sb_base_port]
	add  dl,6
	mov  al,1   ;start DSP reset

if use_cli_sti
	cli	    ;here to minimize time with disabled ints
end if
	out  dx,al
	mov  ecx,40 ;wait at least 3 microsec.
@@:
	in   al,dx
	loop @b

	xor  eax,eax	;stop DSP reset
if use_cli_sti
	sti
end if
	out  dx,al
	ret
endp
;-------------------------------------------------------------------------------
;       set the rate for playing, enable stereo
;-------------------------------------------------------------------------------
proc sb_setup
	mov  edx,[sb_base_port]
	add  dl,0xC
	sb_out 40h  ;set time constant, this is for old cards
	sb_out sb_tc

	sb_out 41h  ;set sound rate, this can only SB16
	sb_out (sb_out_rate shr 8)    ;first high byte (MSB)
	sb_out (sb_out_rate and 0xff) ;then low byte (LSB)
;        mov  al,0xE
;        sub  dl,(0xC-4) ;talk to SB's mixer
;        out  dx,al      ;select this register of the mixer
;        mov  ecx,6      ;wait for the chip
;@@:
;        in   al,dx
;        loop @b

;        inc  edx        ;now read the data port
;        in   al,dx
;        or   al,22h     ;turn on stereo
;        mov  ah,al

;        mov  al,0xE
;        dec  edx        ;talk to SB's mixer
;        out  dx,al      ;select this register of the mixer

;        mov  ecx,6      ;wait for the chip
;@@:
;        in   al,dx
;        loop @b

;        inc  edx        ;now send data to the data port
;        mov  al,ah
;        out  dx,al

;        dec   edx
;        mov   ecx,35     ;wait for the chip
;@@:
;        in   al,dx
;        loop @b
	ret
endp
use32
	org	0x0

	db	'MENUET01'	; 8 byte id
	dd	38		; required os
	dd	STARTAPP	; program start
	dd	I_END		; program image size
	dd	0x100000	; required amount of memory
	dd	0x100000	; reserved= extended header
	dd	filename, 0x0	; I_Param , I_Icon

include "aspAPI.inc"

;Clock_Hz       equ     4608    ; Frequency of clock
;Monitor_Hz     equ     70      ; Frequency of monitor
;Clock_Scale    equ     Clock_Hz / Monitor_Hz
CData		equ	0x40	; Port number of timer 0
CMode		equ	0x43	; Port number of timer control word
BufSize 	equ	65528	; Frame buffer size - Must be even

struc	MainHeaderRec
{
	.Padding1	dd	?	; size of the file
	.ID		dw	?	; magic
	.Frames 	dw	?	; frames
	.Padding2	dd	?	; width/height
	.Padding3	dd	?	; depth/flags
	.Speed		dw	?	; speed
	.Reserv 	dw	?	; always = 0
	; Only for FLC
	.Created	dd	?	; date file attribute
	.Creator	dd	?	; creator serial number
	.Updated	dd	?	; date of the last change
	.Updater	dd	?	; updater serial number
	.Aspectx	dw	?	; scale by x
	.Aspecty	dw	?	; scale by Y
	.Padding4	db	38     dup(?)  ; Reserved. All zeroes.
	.Oframe1	dd	?	; offset to the 1st frame
	.Oframe2	dd	?	; offset to the 2nd frame
	.Padding5	db	40     dup(?)  ; Reserved. All zeroes.
}

struc	FrameHeaderRec
{
	.Size	dd	?	; size
	.Padding1	dw	?	; magic
	.Chunks dw	?	; chunks
	.Padding2	db	8	dup(?)	;Pad to 16 Bytes.       All zeroes.
}


; in:           esi = pointer to the buffer

DrawFrame:
	; this is the routine that takes a frame and put it on the screen
  @Fli_Loop:					; main loop that goes through all the chunks in a frame
    cmp  word [Chunks],0	      ;are there any more chunks to draw?
    je	 @Exit
    dec  word [Chunks]		      ;decrement Chunks For the chunk to process now
	
	mov  ax, word [esi+4]		; let AX have the ChunkType
	add  esi, 6		     ; skip the ChunkHeader
	cmp  ax, 0Bh				; is it a FLI_COLor chunk?
	je   @Fli_Color
	cmp  ax, 0Ch				; is it a FLI_LC chunk?
	je   @Fli_Lc
	cmp  ax, 0Dh				; is it a FLI_BLACK chunk?
	je   @Fli_Black
	cmp  ax, 0Fh				; is it a FLI_BRUN chunk?
	je   @Fli_Brun
	cmp  ax, 10h				; is it a FLI_COPY chunk?
	je   @Fli_Copy
	jmp  @Fli_Loop				; This command should not be necessary

  @Fli_Color:
	mov  bx, word [esi]			; number of packets in this chunk (always 1?)
	add  esi, 2					; skip the NumberofPackets
;_________________________________________________________________________________________
	mov  al, 0					; start at color 0
	xor  cx, cx					; reset CX
  @Color_Loop:
	or   bx, bx					; set flags
	jz   @Fli_Loop				; Exit if no more packages
	dec  bx 					; decrement NumberofPackages For the package to process now
	mov  cl, byte [esi]			; first Byte in packet tells how many colors to skip
	add  al, cl					; add the skiped colors to the start to get the new start
	mov  edi, pal
	add	di,	cx

		;push  cx       ;----- save how many colors to skip ----->
	mov  cl, byte [esi+1]		; next Byte in packet tells how many colors to change
	or   cl, cl					; set the flags
	jnz  @Jump_Over 			; if NumberstoChange=0 then NumberstoChange=256
	inc  ch 					; CH=1 and CL=0 => CX=256
  @Jump_Over:
	;add  al, cl                                     ; update the color to start at
	add  esi, 2					; skip the NumberstoSkip and NumberstoChange Bytes
	.set_next_color:
	     lodsb	   ; B
	     shl     al, 2 ; Enlight the color r*4, g*4, b*4
	     push    ax

	     lodsb	   ; G
	     shl     al, 2 ; Enlight the color r*4, g*4, b*4
	     push    ax

	     lodsb	   ; R
	     shl     al, 2 ; Enlight the color r*4, g*4, b*4

	     stosb	   ; R
	     pop     ax
	     stosb	   ; G
	     pop     ax
	     stosb	   ; B
	     xor     al, al
	     stosb	   ; 0
	  loop	.set_next_color

	jmp  @Color_Loop			; finish With this packet - jump back
  @Fli_Lc:
	mov  di, word [esi]			; put LinestoSkip into DI -
	mov  ax, di					; - to get the offset address to this line we have to multiply With 320 -
	shl  ax, 8					; - DI = old_DI shl 8 + old_DI shl 6 -
	shl  di, 6					; - it is the same as DI = old_DI*256 + old_DI*64 = old_DI*320 -
	add  di, ax					; - but this way is faster than a plain mul
		add  edi, dword [image]
	mov  bx, word [esi+2]		; put LinestoChange into BX
	add  esi, 4					; skip the LinestoSkip and LinestoChange Words
	xor  cx, cx					; reset cx
  @Line_Loop:
	or   bx, bx					; set flags
	jz   @Fli_Loop				; Exit if no more lines to change
	dec  bx
	mov  dl, byte [esi]			; put PacketsInLine into DL
	inc  esi						; skip the PacketsInLine Byte
	push edi						; save the offset address of this line
  @Pack_Loop:
	or   dl, dl					; set flags
	jz   @Next_Line 			; Exit if no more packets in this line

	dec  dl
	mov  cl, byte [esi]			; put BytestoSkip into CL
	add  di, cx					; update the offset address
	mov  cl, byte [esi+1]		; put BytesofDatatoCome into CL
	or   cl, cl					; set flags
	jns  @Copy_Bytes			; no SIGN means that CL number of data is to come -
								; - else the next data should be put -CL number of times
	mov  al, byte [esi+2]		; put the Byte to be Repeated into AL
	add  esi, 3					; skip the packet
	neg  cl 					; Repeat -CL times
	rep  stosb
	jmp  @Pack_Loop 			; finish With this packet
  @Copy_Bytes:
	add  esi, 2					; skip the two count Bytes at the start of the packet
	rep  movsb
	jmp  @Pack_Loop 			; finish With this packet
  @Next_Line:
	pop  edi						; restore the old offset address of the current line
	add  edi, 320				; offset address to the next line
	jmp  @Line_Loop
  @Fli_Black:
	mov  edi, dword [image]
	mov  cx, 32000				; number of Words in a screen
	xor  ax, ax					; color 0 is to be put on the screen
	rep  stosw
	jmp  @Fli_Loop				; jump back to main loop
  @Fli_Brun:
	mov  edi, dword [image]
	mov  bx, 200				; numbers of lines in a screen
	xor  cx, cx
  @Line_Loop2:
	mov  dl, byte [esi]			; put PacketsInLine into DL
	inc  esi						; skip the PacketsInLine Byte
	push edi						; save the offset address of this line
  @Pack_Loop2:
	or   dl, dl					; set flags
	jz   @Next_Line2			; Exit if no more packets in this line
	dec  dl
	mov  cl, byte [esi]			; put BytesofDatatoCome into CL
	or   cl, cl					; set flags
	js   @Copy_Bytes2			; SIGN meens that CL number of data is to come -
								; - else the next data should be put -CL number of times
	mov  al, byte [esi+1]		; put the Byte to be Repeated into AL
	add  esi, 2					; skip the packet
	rep  stosb
	jmp  @Pack_Loop2			; finish With this packet
  @Copy_Bytes2:
	inc  esi						; skip the count Byte at the start of the packet
	neg  cl 					; Repeat -CL times
	rep  movsb
	jmp  @Pack_Loop2			; finish With this packet
  @Next_Line2:
	pop  edi						; restore the old offset address of the current line
	add  edi, 320				; offset address to the next line
	dec  bx 					; any more lines to draw?
	jnz  @Line_Loop2
	jmp  @Fli_Loop				; jump back to main loop
  @Fli_Copy:
	mov  edi, dword [image]
	mov  cx, 32000				; number of Words in a screen
	rep  movsw
	jmp  @Fli_Loop				; jump back to main loop
  @Exit:

		mov	eax, 65
		mov	ebx, dword [image]
		mov	ecx, 320*65536+200
		mov	edx, 10*65536+100
		mov	esi, 8
		mov	edi, pal
		xor	ebp, ebp
		int	0x40
	
	ret	; DrawFrame end.

TFliPlayer_Init:
	mov	eax, 68 	; Init process memory
	mov	ebx, 11
	int	0x40
	cmp	eax, BufSize	
	jl	.fail	
  .ok:
    ;GetMem(Buffer,BufSize);
	mov	eax, 68
	mov	ebx, 12
	mov	ecx, BufSize
	int	0x40
	mov	dword [Buffer], eax

		;GetMem(image,32000);
		mov	eax, 68
		mov	ebx, 12
		mov	ecx, 320*200*4		;32000
		int	0x40
		mov	dword [image], eax

	
    mov word [Interval],  -1	; ClearSpeed
	mov	ax,	1
	ret
  .fail:
	xor	ax, ax
	ret	; TFliPlayer_Init end.

TFliPlayer_Done:
    ;FreeMem(Buffer,BufSize);
	mov	eax, 68
	mov	ebx, 13
	mov	ecx, dword [Buffer]
	int	0x40

		;FreeMem(image,32000);
	mov	eax, 68
	mov	ebx, 13
	mov	ecx, dword [image]
	int	0x40

	ret	; TFliPlayer_Done end.

;       in:     ax = Speed
;TFliPlayer_SetSpeed:
;       mov     bl,     byte Clock_Scale
;       mul     bl
;    mov        word [Interval], ax     ;= Speed * Clock_Scale;
;       ret     ; TFliPlayer_SetSpeed end.



ReadHeader: ;Boolean;
	;BlockRead(FliFile,MainHeader,SizeOf(MainHeader))       ; Read header record
	mov	eax, dword [filepos]
	mov	dword [InfoStructure+4], eax
	mov	dword [InfoStructure+12], 128
	mov	dword [InfoStructure+16], MainHeader
	mov	eax, 70
	mov	ebx, InfoStructure
	
	int	0x40
	
	mov	eax, 128
	mov	dword [filepos], eax
	
    mov ax, word [MainHeader.ID]
	cmp	ax, 0xAF11 ; FLI ?
	je	.fli

	cmp	ax, 0xAF12 ; FLC ?
	je	.flc

	xor	ax,	ax	; Not a .FLI File
	ret
  .fli:
	mov	byte [filetype], 1
	jmp	.ok
  .flc:
	mov	byte [filetype], 2
  .ok:
;       cmp     word [Interval], -1
;       jne     @f
;       ; Read speed from header
;       mov     ax,     word [MainHeader.Speed]
;       mov     bl,     byte Clock_Scale
;       mul     bl
;    mov        word [Interval], ax     ;= Speed * Clock_Scale; 
	mov	ax, 1
;  @@:
	ret	; ReadHeader end.

ReadFrame:
	;BlockRead(FliFile,FrameHeader,SizeOf(FrameHeader));
	mov	eax, dword [filepos]
	mov	dword [InfoStructure+4], eax
	mov	dword [InfoStructure+12], 16
	mov	dword [InfoStructure+16], FrameHeader
	mov	eax, 70
	mov	ebx, InfoStructure
	
	int	0x40

	add	dword [filepos], 16
	;FrameSize := FrameHeader.Size - SizeOf(FrameHeader);
	mov	eax, dword [FrameHeader.Size]
	sub	eax, 16
	mov dword [FrameSize], eax
    xor ecx, ecx

	ret	; ReadFrame end.

ProcessFrame:
	;BlockRead(FliFile,Buffer^,FrameSize);
	mov	eax, dword [filepos]
	mov	dword [InfoStructure+4], eax
	mov	eax,	dword [FrameSize]
	mov	dword [InfoStructure+12], eax
	mov	eax, dword [Buffer]
	mov	dword [InfoStructure+16], eax
	mov	eax, 70
	mov	ebx, InfoStructure
	
	int	0x40

	mov	eax,	dword [FrameSize]
	add	dword [filepos], eax
	mov	esi, dword [Buffer]
	mov	dx, word [FrameHeader.Chunks]
	mov	word [Chunks], dx
	call DrawFrame
	
	ret	; ProcessFrame end.
	
	
;       in:     esi = pointer to the filename
TFliPlayer_Play:
	mov	eax, 70
	mov	ebx, InfoStructure
	mov	dword [InfoStructure+12], 1
	mov	dword [InfoStructure+16], FrameHeader
	int	0x40
	test	ax,	ax
	jnz	.err

	call	ReadHeader
	test	ax,	ax
	jz	.err

	call	ReadFrame

	mov	eax, dword [FrameSize]
	add	eax, 128+16
	mov	dword [RestartPos], eax
	call	ProcessFrame


  .play_again:
	mov	word [Frame], 1
	  .show_next_frame:
			
			call	ReadFrame
			cmp	dword [FrameSize], 0
			je	@f
			call	ProcessFrame
		  @@:

		  ;REPEAT UNTIL GetClock > Timeout;
			mov	eax, 5
			mov	ebx, 2
			int	0x40
			
			push	edi		
			call	process_events
			pop	edi

			mov	al, byte [stop]
			cmp	al, 1
			je	.end
			
			mov	ax, word [Frame]
			inc	ax
			mov	word [Frame], ax

			cmp	ax, word [MainHeader.Frames]
			jng	.show_next_frame
		mov	eax,	dword [RestartPos]
		mov	dword [InfoStructure+4], eax
		mov	eax, 128
		mov	dword [filepos], eax

	jmp	.play_again
  .end:
	mov	dword [filepos], 0
	xor	ax, ax
  .err:
  
	ret	; TFliPlayer_Play end.


;       in:     esi = pointer to filename
AAPlay:
	mov	byte [pausetime], 0
	mov	byte [stop], 0
	cmp	byte [playtime], 1
	je	.end
	call	unprint_error

	mov	byte [playtime], 1
	call	TFliPlayer_Play
	test	ax, ax
	jz	.end
	call	print_error
  .end:
	mov	byte [playtime], 0
	ret	; AAPlay end.


	
N_KEYCOLOR equ 0x00444444 ; Normal button color
TEXTCOLOR  equ 0x80FFFFFF ; Text color
BUTCAPCOLOR  equ 0x0000FF00 ; Button caption color


unprint_error:
	mov eax, 13
	mov ebx, 10*65536+320
	mov ecx, 50*65536+15
	mov edx, 0 ; color
	int 0x40
ret

print_error:
	;call   unprint_error
	
	mov	ebx, 15*65536+54
	mov	edx, error_text
	mov	ecx, TEXTCOLOR - 0x0000FFFF
	mov	eax, 4
	int	0x40

ret

print_filename:
	mov eax, 13
	mov ebx, 10*65536+320
	mov ecx, 30*65536+15
	mov edx, 0 ; color
	int 0x40

	mov	ebx, 15*65536+34
	mov	edx, filename
	mov	ecx, TEXTCOLOR - 0x00FF00FF
	mov	eax, 4
	int	0x40
	
ret

;**********************************
;*  input: ecx = type of map      *
;**********************************
get_keyboard_map:
	mov	eax, 26
	mov	ebx, 2
	mov	edx, keymap
	int	0x40
ret

;**********************************
;*  input: ecx = type of mode    *
;**********************************
set_keyboard_mode:
	mov	eax, 66
	mov	ebx, 1	   ; Set keyboard mode
	int	0x40
ret

get_control_keys_state:
	mov	eax, 66
	mov	ebx, 3
	int	0x40
ret

reload_ascii_keymap:
	pusha
	call	get_control_keys_state
	mov	ecx, 1

	test	ax, 1				; Left  Shift pressed ?
	jnz	 @f
	test	ax, 2				; Right Shift pressed ?
	jnz	 @f
	test	ax, 0x40			; Caps  Lock  on ?
	jz	.load_ascii_keymap
      @@:
	mov	ecx, 2
      .load_ascii_keymap:
	call	get_keyboard_map
	popa
  ret


STARTAPP:

	call	TFliPlayer_Init
	test	ax, ax
	jz	close_app

	mov	ecx, 1	   ; to send scancodes.
	call	set_keyboard_mode
	call	reload_ascii_keymap
	call	draw_window
	call	print_filename

	cmp  [filename], byte 0
	je   .noparam
	call	AAPlay
  .noparam:
	jmp	still


draw_window:
	start_draw_window 20,170,340,310,0x14224466,labelt
	draw_button 15,70,20,20,2,N_KEYCOLOR,keyText,1,BUTCAPCOLOR		; Play

	mov	ecx, BUTCAPCOLOR

	mov	edx, keyText
	mov	ebx, 21*65536+77	       ; |
	call	out_symbol

	mov	edx, keyText+1
	mov	ebx, 22*65536+75	       ; -
	call	out_symbol

	mov	ebx, 24*65536+76	       ; -
      @@:
	call	out_symbol
	inc	ebx
	cmp	ebx, 24*65536+79
	jl	@b
		
	mov	ebx, 22*65536+79	       ; -
	call	out_symbol

	mov	ebx, 26*65536+77	       ; -
	call	out_symbol
		
	draw_button 45,70,20,20,3,N_KEYCOLOR,keyText,1,BUTCAPCOLOR		; Pause

	mov	ecx, BUTCAPCOLOR

	mov	edx, keyText
	mov	ebx, 49*65536+77	       ; |
	call	out_symbol

	mov	ebx, 51*65536+77	       ; |
	call	out_symbol

	mov	ebx, 54*65536+77	       ; |
	call	out_symbol

	mov	ebx, 55*65536+77	       ; |
	call	out_symbol

	mov	ebx, 56*65536+77	       ; |
	call	out_symbol

	draw_button 75,70,20,20,4,N_KEYCOLOR,keyText,1,BUTCAPCOLOR		; Stop

	bar	81,77,8,7,BUTCAPCOLOR

	end_draw_window
	ret

process_events:
	mov	eax, 11 	    ; Test if there is an event in the queue.
	int	0x40

	cmp	eax,1		       ; redraw request ?
	jnz	@f
	call	red
	ret
      @@:		
	cmp	eax,3		       ; button in buffer ?
	jnz	@f
	call	button
	ret
      @@:
	cmp	eax,2		       ; key in buffer ?
	jnz	@f

	call	key
      @@:
	;jz      key
      ;  cmp     eax, 2
      ;  jnz     @f
      ;
      ;  cmp     byte [playtime], 0
      ;  je      @f
	;int     0x40
      ;@@:

	ret	
	
still:
	call	process_events
	jmp	still

red:
	call	draw_window
	ret	;jmp    still

key:
	mov	eax, 2
	int	0x40

	test	al, al
	jnz	.end

	cmp	ah, 1 ; Esc
	jne	@f
	jmp	close_app
  @@:
	cmp	ah, 28 ; Enter
	jne	@f
	call	AAPlay
	jmp	.end
  @@:
	cmp	ah, 15 ; Tab
	je	.end
	cmp	ah, 29 ; Left Control
	je	.end
	cmp	ah, 42 ; Left Shift
	je	.end

	cmp	ah, 14 ; BackSpace
	jne	@f
	
	; strlen(filename)
	mov	edi, filename
	xor	al, al	; search for the end of the filename
	repne	scasb
	cmp	edi, filename+2
	jl	.end

	cmp	edi, filename+2
	jl	.end
	mov	byte [edi-2], 0
	
	call	print_filename
	jmp	.end

  @@:
	cmp	ah, 57 ; Space
	je	.input_symbol
	cmp	ah, 2
	jl	.end
	cmp	ah, 54
	jg	.end

  .input_symbol:
	mov	byte [stop], 1
	call	reload_ascii_keymap
	; strlen(filename)
	mov	edi, filename
	xor	al, al	; search for the end of the filename
	repne	scasb
	cmp	edi, filename+52
	jg	.end
	
	shr	ax, 8
	and	eax, 0x7F
	mov	al, byte [keymap+eax]	; Read ASCII from the keymap table.
	mov	byte [edi-1], al
	mov	byte [edi], 0
	
	call	print_filename
  .end:
	ret
	
button:
	mov	eax, 17 	    ; Get pressed button code
	int	0x40

	cmp	ah, 1		    ; Test x button
	je	close_app

	cmp	ah, 2		    ; Test "Play" button
	jne	@f
	call	AAPlay
	jmp	.end
  @@:
	
	cmp	ah, 3		    ; Test "Pause" button
	jne	@f
	cmp	byte [playtime], 1
	jne	@f
	
	not	byte [pausetime]
      .pause:
	call	process_events
	cmp	byte [pausetime], 0
	jne	.pause

  @@:
	cmp	ah, 4		    ; Test "Stop" button
	jne	@f
	mov	byte [stop], 1
	jne	@f
  @@:

  .end:
	ret
close_app:

    cmp     dword [image], 0
    je	    @f
    call    TFliPlayer_Done
  @@:
    mov  eax,-1 		 ; close this program
    int  0x40
	
	
; DATA  
; Application Title
labelt		db	'FLI Player v0.31',0
keyText 	db	'|-_'


error_text	db	"Can't load file.",0
filepos 	dd	0x0
filetype	db	0     ; 0 - unsupported, 1 - fli, 2 - flc

playtime	db	0
pausetime	db	0
stop		db	0

pal		rb	256*4
image		dd	0
InfoStructure:	
			dd	0x0	; subfunction number
			dd	filepos ; position in the file in bytes
			dd	0x0	; upper part of the position address
			dd	0x0	; number of     bytes to read
			dd	0x0	; pointer to the buffer to write data
			db	0
			dd	filename

keymap	rb	128
			
I_END:

Buffer		dd	?
Interval	dw	?

Chunks		dw	?

FrameSize	dd	?
RestartPos	dd	?
Frame		dw	?

MainHeader	MainHeaderRec
FrameHeader	FrameHeaderRec

filename	db	'/hd0/2/',0
		rb	46


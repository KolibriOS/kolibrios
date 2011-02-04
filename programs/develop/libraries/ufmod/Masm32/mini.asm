; MINI.ASM
; --------
; Shows how to place data and code inside (!!!) the XM track.

.386
.model flat

include ufmod.inc ; uFMOD API

.CODE

PE_BASE   db "MENUET01"
          dd 1
          dd OFFSET _START    ; Entry point
          dd OFFSET BSS_START ; End of code and initialized data
          dd OFFSET BSS_END   ; End of uninitialized (BSS) data
          dd OFFSET STACK_B   ; Bottom of the stack
          dd 0                ; Args
          dd 0                ; Reserved

err_txt   db "Error"
err_txt_l EQU $ - err_txt

_START:
	; Start playback.
	push XM_MEMORY
	push xm_length
	push BYTE PTR xm

; Let's place the stream right inside the code section.
xm_length EQU 905
xm        EQU $ - PE_BASE
xm_unused_000 LABEL BYTE

; *** The following 60 bytes are not used. So, we'll place
; *** some code here.
; (the actual size and location of such gaps may be
; found out using the Eff utility)

	call uFMOD_LoadSong
	xor ebp,ebp             ; global 0

	; Stack fixing is required here, but in this simple
	; example leaving ESP as it is won't harm. In a real
	; application you should uncomment the following line:
	; add esp,12

	test eax,eax
	jz error

	; Wait for user input.
	push uFMOD_WaveOut      ; cbProc <- continue fetching data!
	push BYTE PTR msg_txt_l ; cbString
	push OFFSET msg_txt     ; lpString
	push OFFSET msg_cap     ; szCap
	call MessageBoxCB
	; add esp,16

	; Stop playback.
	call uFMOD_StopSong

r:      ; Exit.
	lea eax,[ebp-1]
	int 40h

error:
	push ebp                          ; cbProc <- no callback
	push BYTE PTR err_txt_l           ; cbString
	push BYTE PTR (err_txt - PE_BASE) ; lpString
	push OFFSET err_cap               ; szCap
	call MessageBoxCB
	; add esp,16
	jmp r

org xm_unused_000 + 60
	db 034h,000h,000h,000h,020h,000h,000h,000h,002h,000h,00Dh,000h,001h,000h,001h,000h
	db 00Ah,000h,091h,000h,000h,001h,002h,003h,004h,005h,006h,007h,000h,001h,002h,003h
	db 004h,005h,006h,007h,008h,009h,00Ah,00Bh,008h,009h,00Ch,00Bh,008h,009h,00Ah,00Bh
	db 008h,009h,00Ch,00Bh,009h,000h,000h,000h,000h,004h,000h,001h,000h,083h,016h,001h
	db 080h,080h,02Eh,001h,000h,00Eh,060h,080h,03Ah,001h,000h,00Eh,062h,081h,061h,083h
	db 035h,001h,009h,000h,000h,000h,000h,004h,000h,001h,000h,083h,016h,001h,080h,080h
	db 02Eh,001h,000h,00Eh,060h,080h,035h,001h,000h,00Eh,062h,081h,061h,083h,038h,001h
	db 009h,000h,000h,000h,000h,004h,000h,001h,000h,083h,016h,001h,080h,080h,02Eh,001h
	db 000h,00Eh,060h,080h,038h,001h,000h,00Eh,062h,080h,083h,033h,001h,009h,000h,000h
	db 000h,000h,006h,000h,001h,000h,083h,016h,001h,080h,080h,02Eh,001h,000h,00Eh,060h
	db 080h,033h,001h,000h,00Eh,061h,081h,061h,083h,035h,001h,083h,00Dh,001h,083h,036h
	db 001h,080h,083h,036h,001h,009h,000h,000h,000h,000h,004h,000h,001h,000h,083h,00Fh
	db 001h,080h,080h,02Eh,001h,000h,00Eh,060h,080h,036h,001h,000h,00Eh,062h,081h,061h
	db 083h,033h,001h,009h,000h,000h,000h,000h,006h,000h,001h,000h,083h,00Fh,001h,080h
	db 080h,02Eh,001h,000h,00Eh,060h,080h,033h,001h,000h,00Eh,061h,081h,061h,083h,02Eh
	db 001h,083h,012h,001h,083h,033h,001h,080h,083h,035h,001h,009h,000h,000h,000h,000h
	db 006h,000h,001h,000h,083h,016h,001h,080h,080h,02Eh,001h,000h,00Eh,060h,080h,035h
	db 001h,000h,00Eh,061h,081h,061h,083h,02Eh,001h,083h,00Dh,001h,083h,031h,001h,080h
	db 083h,02Eh,001h,009h,000h,000h,000h,000h,008h,000h,001h,000h,083h,012h,001h,098h
	db 00Ah,001h,083h,019h,001h,088h,00Ah,083h,01Eh,001h,081h,061h,083h,012h,001h,080h
	db 083h,014h,001h,080h,083h,01Bh,001h,080h,083h,020h,001h,080h,083h,014h,001h,080h
	db 009h,000h,000h,000h,000h,008h,000h,001h,000h,083h,012h,001h,081h,061h,083h,019h
	db 001h,080h,083h,01Eh,001h,080h,083h,012h,001h,080h,083h,019h,001h,083h,031h,001h
	db 083h,01Eh,001h,080h,083h,012h,001h,083h,031h,001h,083h,019h,001h,080h,009h,000h
	db 000h,000h,000h,008h,000h,001h,000h,083h,014h,001h,083h,033h,001h,083h,01Bh,001h
	db 080h,083h,020h,001h,083h,031h,001h,083h,014h,001h,080h,083h,01Bh,001h,083h,030h
	db 001h,083h,020h,001h,080h,083h,014h,001h,083h,031h,001h,083h,01Bh,001h,080h,009h
	db 000h,000h,000h,000h,008h,000h,001h,000h,083h,016h,001h,083h,030h,001h,083h,01Dh
	db 001h,083h,031h,001h,083h,022h,001h,083h,035h,001h,083h,016h,001h,098h,00Ah,001h
	db 083h,01Dh,001h,088h,00Ah,083h,022h,001h,081h,061h,083h,016h,001h,080h,083h,01Dh
	db 001h,080h,009h,000h,000h,000h,000h,008h,000h,001h,000h,083h,016h,001h,080h,083h
	db 01Dh,001h,080h,083h,022h,001h,080h,083h,016h,001h,080h,083h,018h,001h,080h,083h
	db 01Dh,001h,080h,083h,011h,001h,080h,083h,018h,001h,080h,009h,000h,000h,000h,000h
	db 008h,000h,001h,000h,083h,016h,001h,083h,030h,001h,083h,01Dh,001h,083h,031h,001h
	db 083h,019h,001h,083h,02Eh,001h,083h,016h,001h,098h,00Ah,001h,083h,01Dh,001h,088h
	db 00Ah,083h,019h,001h,081h,061h,083h,016h,001h,080h,083h,01Dh,001h,080h,0F1h,000h
	db 000h,000h
xm_unused_001 LABEL BYTE

; The following 23 bytes are not used.
; So, let's place the MessageBox text and caption instead.
; UI text messages.
msg_txt   db "uFMOD ruleZ!"
msg_txt_l equ $ - msg_txt
msg_cap   db "MASM32",0
err_cap   db ":-(",0

org xm_unused_001 + 23
	db 001h,000h,012h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h
	db 000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h
	db 000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h
	db 000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h
	db 000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h
	db 000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h
	db 000h,000h,000h,000h,000h,000h,000h,000h,040h,000h,008h,000h,02Ch,000h,00Eh,000h
	db 008h,000h,018h,000h,016h,000h,020h,000h,008h,000h,02Dh,000h,00Dh,000h,032h,000h
	db 004h,000h,03Ch,000h,007h,000h,044h,000h,004h,000h,05Ah,000h,000h,000h,064h,000h
	db 000h,000h,06Eh,000h,000h,000h,000h,000h,020h,000h,00Ah,000h,028h,000h,01Eh,000h
	db 018h,000h,032h,000h,020h,000h,03Ch,000h,020h,000h,046h,000h,020h,000h,050h,000h
	db 020h,000h,05Ah,000h,020h,000h,064h,000h,020h,000h,06Eh,000h,020h,000h,078h,000h
	db 020h,000h,082h,000h,020h,000h,009h,006h,001h,002h,004h,002h,003h,005h,001h,000h
	db 000h,000h,000h,000h,080h,000h,00Ch,000h,000h,000h,000h,000h,000h,000h,00Ch,000h
	db 000h,000h,040h,000h,001h,080h,0F9h,000h,0BFh,000h,0C3h,000h,00Ah,000h,057h,000h
	db 06Eh,000h,023h,000h

; ----------------------------------------------------------
; void MessageBoxCB(szCap, lpString, cbString, cbProc);
; ----------------------------------------------------------

; This is similar to a Win32 MessageBox. The box is centered
; on screen. It contains a single-line text message and an
; "OK" button. This function returns when user closes the
; box (via the X button or via the OK button). An optional
; callback subroutine may be specified to be called when no
; events are pending in the event queue.

; NOTE: Doesn't work if you already have defined a window
; in the current process! Doesn't modify the event mask. So,
; make sure keyboard events are enabled before calling this
; function. This function doesn't check the validity of the
; supplied parameters!

; Parameters:
; szCap     - A pointer to the ASCIIz string containing the
;             caption. A trailing zero char IS required.
; lpString  - A pointer to an ASCII string containing a single
;             line message to pop up in the box. No trailing
;             zero char is required.
; cbString  - number of characters in string.
; cbProc    - Address of the callback subroutine. Can be NULL.

sOK db "OK"
MessageBoxCB:
; EBP = 0
	mov esi,[esp+12] ; cbString
	mov edi,[esp+4]  ; szCap

	; Get screen metrics.
	lea eax,[ebp+14]
	int 40h
	mov ecx,eax
	movzx eax,ax
	shr ecx,16         ; screen w
	xchg eax,edx       ; screen h
	lea ebx,[esi*2+esi]
	lea ebx,[ebx*2+28] ; w = string len * 6 + 28
	sub ecx,ebx
	shr ecx,1
	shl ecx,16
	or ebx,ecx
	lea ecx,[ebp+52h]  ; h = 52h
	sub edx,ecx
	shr edx,1
	shl edx,16
	or ecx,edx         ; y = (screen h - window h) / 2
	mov edx,ebx        ; x = (screen w - window w) / 2

_MessageBoxCB_redraw:
	; Start redraw.
	push edx
	lea eax,[ebp+12]
	lea ebx,[ebp+1]
	int 40h

	; Define and draw window.
	xor eax,eax
	mov ebx,edx        ; x, w (ECX: y, h)
	mov edx,34C0C0C0h  ; style and BG color
	int 40h

	; Define the OK button.
	push esi
	lea eax,[ebp+8]
	sub ebx,28+0Ah
	shr bx,1
	shl ebx,16         ; x = (window w - button w) / 2
	mov bx,18+0Ah      ; w = 18 + 0Ah
	mov ecx,001C0012h  ; y = 1Ch, h = 12h
	lea edx,[ebp+1]    ; ID = close
	mov esi,0C0C0C0h   ; color
	int 40h

	; Draw the OK label.
	lea eax,[ebp+4]
	add ebx,90000h     ; x = button x + 9
	mov bx,22h         ; y = 22h
	xor ecx,ecx        ; style, font and color
	mov edx,OFFSET sOK ; string
	lea esi,[ebp+2]    ; length
	int 40h
	pop esi

	; Draw text string.
	lea eax,[ebp+4]
	mov ebx,000A000Ah  ; x = 0Ah, y = 0Ah
	xor ecx,ecx        ; style, font and color
	mov edx,[esp+12]   ; lpString
	int 40h

	; End redraw.
	lea eax,[ebp+12]
	lea ebx,[ebp+2]
	int 40h

_MessageBoxCB_eventloop:
	mov edx,[esp+20]   ; cbProc
	test edx,edx
	lea eax,[ebp+10]
	jz _MessageBoxCB_peekevent

	; Invoke the callback.
	call edx

	lea eax,[ebp+23]
	lea ebx,[ebp+10] ; wait for at most 0.1 sec
_MessageBoxCB_peekevent:
	int 40h
	dec eax
	js _MessageBoxCB_eventloop

	pop edx
	jz _MessageBoxCB_redraw
	ret

.DATA?
BSS_START LABEL BYTE
          db 1020 dup (?)
STACK_B   dd ? ; Stack bottom
BSS_END   LABEL BYTE

END _START
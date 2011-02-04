; MINI.ASM
; --------
; Minimalistic uFMOD usage example.

; Shows how to play an XM track in memory,
; including proper error handling.

BITS 32
org 0
db "MENUET01"
dd 1
dd START         ; Entry point
dd uFMOD_IMG_END ; End of code and initialized data
dd MEMORY_END    ; End of uninitialized (BSS) data
dd STACK_B       ; Bottom of the stack
dd 0             ; Args
dd 0             ; Reserved

; uFMOD setup:
%define f48000   ; Set sampling rate to 48KHz  (22050, 44100, 48000)
%define STRONG   ; Select STRONG interpolation (NONE, WEAK, STRONG)
%define UNSAFE   ; Select UNSAFE mode          (NORMAL, UNSAFE)
%define NODEBUG  ; Skip debug-board messages
%define NOLINKER ; Select "no linker" mode

; uFMOD constants:
%define uFMOD_MIN_VOL     0
%define uFMOD_MAX_VOL     25
%define uFMOD_DEFAULT_VOL 25

; The XM track.
xm        incbin "..\ufmodlib\media\mini.xm"
xm_length equ $ - xm

; Optimization:
; This header file is suitable for mini.xm track only!
; If you change the track, update the optimization header.
; (Use the standart eff.inc file for a general purpose player app.)
%include "..\ufmodlib\media\mini.eff.inc"

; UI text messages.
msg_txt   db "uFMOD ruleZ!"
msg_txt_l equ $ - msg_txt
msg_cap   db "NASM",0
err_txt   db "Error"
err_txt_l equ $ - err_txt
err_cap   db ":-(",0

START:
	; Start playback.
	push XM_MEMORY
	push xm_length
	push xm
	call _uFMOD_LoadSong

	; Stack fixing is required here, but in this simple
	; example leaving ESP as it is won't harm. In a real
	; application you should uncomment the following line:
	; add esp,12

	test eax,eax
	jz error

	; Wait for user input.
	push _uFMOD_WaveOut ; cbProc <- continue fetching data!
	push msg_txt_l      ; cbString
	push msg_txt        ; lpString
	push msg_cap        ; szCap
	call _MessageBoxCB
	; add esp,16

	; Stop playback.
	call _uFMOD_StopSong

r:      ; Exit.
	xor eax,eax
	dec eax
	int 40h

error:
	push 0              ; cbProc <- no callback
	push err_txt_l      ; cbString
	push err_txt        ; lpString
	push err_cap        ; szCap
	call _MessageBoxCB
	; add esp,16
	jmp r

; ---------------------------------------------------------------
; void _cdecl _MessageBoxCB(szCap, lpString, cbString, cbProc);
; ---------------------------------------------------------------

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
_MessageBoxCB:
	push ebp
	push esi
	push edi
	push ebx
	xor ebp,ebp      ; global 0
	mov esi,[esp+28] ; cbString
	mov edi,[esp+20] ; szCap

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
	mov edx,sOK        ; string
	lea esi,[ebp+2]    ; length
	int 40h
	pop esi

	; Draw text string.
	lea eax,[ebp+4]
	mov ebx,000A000Ah  ; x = 0Ah, y = 0Ah
	xor ecx,ecx        ; style, font and color
	mov edx,[esp+28]   ; lpString
	int 40h

	; End redraw.
	lea eax,[ebp+12]
	lea ebx,[ebp+2]
	int 40h

_MessageBoxCB_eventloop:
	mov edx,[esp+36]   ; cbProc
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

	pop ebx
	pop edi
	pop esi
	pop ebp
	ret

	; Include the whole uFMOD sources here. (Right after
	; your main code to avoid naming conflicts, but still
	; inside your code section.)
	%include "nasm.asm"

alignb 4
	resb 1020
STACK_B resd 1 ; Stack bottom
MEMORY_END:    ; End of uninitialized (BSS) data

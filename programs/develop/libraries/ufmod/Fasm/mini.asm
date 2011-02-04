; MINI.ASM
; --------
; Minimalistic uFMOD usage example.

; Shows how to play an XM track in memory,
; including proper error handling.

; A precompiled version (not packed or whatever) is
; available in bin\

use32
org 0
db 'MENUET01'
dd 1
dd START         ; Entry point
dd uFMOD_IMG_END ; End of code and initialized data
dd MEMORY_END    ; End of uninitialized (BSS) data
dd STACK_B       ; Bottom of the stack
dd 0             ; Args
dd 0             ; Reserved

; uFMOD setup:
UF_FREQ  equ 48000  ; Set sampling rate to 48KHz  (22050, 44100, 48000)
UF_RAMP  equ STRONG ; Select STRONG interpolation (NONE, WEAK, STRONG)
UD_MODE  equ UNSAFE ; Select UNSAFE mode          (NORMAL, UNSAFE)
DEBUG    equ 0      ; Skip debug-board messages
NOLINKER equ 1      ; Select "no linker" mode

; uFMOD constants:
XM_MEMORY         = 1
XM_FILE           = 2
XM_NOLOOP         = 8
XM_SUSPENDED      = 16
uFMOD_MIN_VOL     = 0
uFMOD_MAX_VOL     = 25
uFMOD_DEFAULT_VOL = 25

; The XM track.
xm        file '..\ufmodlib\media\mini.xm'
xm_length = $ - xm

; Optimization:
; This header file is suitable for mini.xm track only!
; If you change the track, update the optimization header.
; (Use the standart eff.inc file for a general purpose player app.)
include '..\ufmodlib\media\mini.eff.inc'

; Include the GUI framework.
FRMWRK_CALLBACK_ON equ 1 ; Enable callback
include 'frmwrk.asm'

; UI text messages.
msg_txt   db "uFMOD ruleZ!"
msg_txt_l = $ - msg_txt
msg_cap   db "FASM",0
err_txt   db "Error"
err_txt_l = $ - err_txt
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

	; Include the whole uFMOD sources here. (Right after
	; your main code to avoid naming conflicts, but still
	; inside your code section.)
	macro PUBLIC symbol {} ; hide all publics
	include '..\ufmodlib\src\fasm.asm'

align 4
	rb 1020
STACK_B dd ? ; Stack bottom
MEMORY_END:  ; End of uninitialized (BSS) data

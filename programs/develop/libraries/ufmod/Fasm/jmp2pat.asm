; JMP2PAT.ASM
; -----------
; Sometimes it makes sense merging various XM tracks
; sharing the same instruments in a single XM file.
; This example program uses such an XM file actually
; containing 3 tracks and the _uFMOD_Jump2Pattern
; function to play all 3 tracks in the same file.

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

; BLITZXMK.XM tracked by Kim (aka norki):
;   [00:07] - track #1
;   [08:10] - track #2
;   [11:13] - track #3
xm        file '..\ufmodlib\media\BLITZXMK.XM'
xm_length = $ - xm

; Optimization:
; This header file is suitable for blitzxmk.xm track only!
; If you change the track, update the optimization header.
; (Use the standart eff.inc file for a general purpose player app.)
include '..\ufmodlib\media\blitz.eff.inc'

; Include the GUI framework.
FRMWRK_CALLBACK_ON equ 0 ; Disable callback
include 'frmwrk.asm'

; UI text messages.
vals       dd 0,8,11 ; Preset pattern indexes
wnd_btns1  db "1    2    3    Pause "
wnd_btns2  db "1    2    3    Resume"
wnd_btns_l = $ - wnd_btns2
wnd_cap    db "Jump2Pattern",0
err_txt    db "Error"
err_txt_l  = $ - err_txt
err_cap    db ":-(",0

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
	xor ebp,ebp        ; global 0
	mov [wnd_btns],wnd_btns1

	; Switch keyboard mode to SCANCODE.
	lea ebx,[ebp+1]
	lea eax,[ebp+66]
	mov ecx,ebx
	int 40h

	; Get screen metrics.
	lea eax,[ebp+14]
	int 40h
	mov ecx,eax
	movzx eax,ax
	shr ecx,16         ; screen w
	xchg eax,edx       ; screen h
	mov ebx,wnd_btns_l*6+42
	sub ecx,ebx
	shr ecx,1
	shl ecx,16
	or ebx,ecx
	lea ecx,[ebp+40h]  ; h = 40h
	sub edx,ecx
	shr edx,1
	shl edx,16
	or ecx,edx         ; y = (screen h - window h) / 2
	mov edx,ebx        ; x = (screen w - window w) / 2

redraw:
	; Start redraw.
	push edx
	lea eax,[ebp+12]
	lea ebx,[ebp+1]
	int 40h

	; Define and draw window.
	xor eax,eax
	mov ebx,edx        ; x, w (ECX: y, h)
	mov edx,34C0C0C0h  ; style and BG color
	mov edi,wnd_cap
	int 40h

	; Define the 1 2 3 Pause/Resume buttons.
	lea eax,[ebp+8]
	mov ebx,0A0012h    ; x = 0Ah, w = 12h
	mov ecx,0A0012h    ; y = 0Ah, h = 10h
	lea edx,[ebp+10]   ; ID = #10
	mov esi,0C0C0C0h   ; color
	int 40h
	mov ebx,280012h    ; x = 28h, w = 12h
	inc edx            ; ID = #11
	int 40h
	mov ebx,460012h    ; x = 46h, w = 12h
	inc edx            ; ID = #12
	int 40h
	mov ebx,640030h    ; x = 64h, w = 30h
	inc edx            ; ID = #13
	int 40h

	; Draw the labels.
	lea eax,[ebp+4]
	mov ebx,120011h    ; x = 12h, y = 11h
	xor ecx,ecx        ; style, font and color
	mov edx,[wnd_btns] ; string
	lea esi,[ebp+wnd_btns_l] ; length
	int 40h

	; End redraw.
	lea eax,[ebp+12]
	lea ebx,[ebp+2]
	int 40h

eventloop:
	; Update the PCM buffer.
	call _uFMOD_WaveOut

	lea eax,[ebp+23]
	lea ebx,[ebp+10] ; wait for at most 0.1 sec
	int 40h
	dec eax
	js eventloop ; 0 = idle
	jz redraw    ; 1 = redraw

	dec eax      ; 2 = keyboard event
	jnz chk_eventbutton

	; Get key scancode.
	lea eax,[ebp+2]
	int 40h
	cmp ah,19h   ; P
	je do_PauseResume
	cmp ah,13h   ; R
	je do_PauseResume
chk_kb123:
	movzx eax,ah
	sub eax,2
	jmp do_Jump2Pat123

chk_eventbutton:     ; 3 = button event
	lea eax,[ebp+17]
	int 40h
	cmp ah,1     ; Close
	je break_loop
	cmp ah,13    ; Pause/Resume
	jne chk_btn123

do_PauseResume:
	cmp BYTE [paused],1
	mov edx,_uFMOD_Resume
	mov ebx,wnd_btns1
	je do_Resume
	mov edx,_uFMOD_Pause
	mov ebx,wnd_btns2
do_Resume:
	call edx
	xor BYTE [paused],1
	mov [wnd_btns],ebx
	jmp redraw

chk_btn123:          ; 1 2 3
	movzx eax,ah
	sub eax,10

do_Jump2Pat123:
	cmp eax,3
	jae eventloop
	push DWORD [vals+eax*4]
	call _uFMOD_Jump2Pattern
	pop eax ; fix stack
	jmp eventloop
break_loop:

	; Stop playback.
	call _uFMOD_StopSong

r:      ; Exit.
	xor eax,eax
	dec eax
	int 40h

error:
	push err_txt_l      ; cbString
	push err_txt        ; lpString
	push err_cap        ; szCap
	call _MessageBox
	; add esp,16
	jmp r

	; Include the whole uFMOD sources here. (Right after
	; your main code to avoid naming conflicts, but still
	; inside your code section.)
	macro PUBLIC symbol {} ; hide all publics
	include '..\ufmodlib\src\fasm.asm'

wnd_btns dd ?
paused   db ?
align 4
	 rb 1020
STACK_B  dd ? ; Stack bottom
MEMORY_END:   ; End of uninitialized (BSS) data

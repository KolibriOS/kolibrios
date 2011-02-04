; FRMWRK.ASM
; ----------
; A set of common GUI code used in uFMOD examples for KolibriOS.
; Feel free to reuse it in your own projects if you like it ;)

; ---------------------------------------------------------------
; void _cdecl _MessageBox  (szCap, lpString, cbString);
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
if FRMWRK_CALLBACK_ON
_MessageBoxCB:
else
_MessageBox:
end if
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

if FRMWRK_CALLBACK_ON
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
else
	lea eax,[ebp+10]
	int 40h
	dec eax
end if
	pop edx
	jz _MessageBoxCB_redraw

	pop ebx
	pop edi
	pop esi
	pop ebp
	ret

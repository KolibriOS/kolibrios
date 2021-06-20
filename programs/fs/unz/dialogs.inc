
;SayErr  int num_strings, const char* strings[],
;                      int num_buttons, const char* buttons[]);

proc SayErr num_strings:dword, strings:dword,num_buttons:dword, buttons:dword
	pushad
	cmp	[num_strings],1
	je	@f
	m2m	[errmess0], strErrorExc
	jmp	.l1
       @@:
	mov	ebx,[strings]
	m2m	[errmess0], dword [ebx]
       .l1:
	mcall	51,1,threadSayErr,stackDlg
	popad
	mov	eax,1
	ret
endp

proc SimpleSayErr str:dword
	pushad
	m2m	[errmess0],[str]
	mcall	51,1,threadSayErr,stackDlg
	popad
	ret
endp


proc threadSayErr
	mcall	40, 000111b+0C000000h

.wm_redraw:
	mcall	12, 1
	mcall	48, 3, sc, sizeof.system_colors
	mov	edx, [sc.work]

	or	edx, 0x33000000
	mcall 0, <220,420>, <220,110>, , , title

	mov	ecx,[sc.work_text]
	or	ecx,90000000h
	mov	edx,[errmess0]
	mcall 4, <15,11>

	mcall 8, <105,100>,<45,25>,1,[sc.work_button]
	mov	ecx,[sc.work_button_text]
	or	ecx,90000000h
	mcall 4, <147,51>, , strOk

	mcall 12, 2

.still:
	mcall	10
	cmp	eax, 1
	je	.wm_redraw
	cmp	eax, 2
	je	.wm_key
	cmp	eax, 3
	je	.wm_button
	jmp	.still

.wm_button:
	mcall	17

	cmp	ah, 1
	je	.exit
	jmp	.still

.wm_key:
	mcall	2
	jmp	.still
.exit:
	mcall	-1
endp

;-------------------------------------------------------------------------------
;-------------------------------------------------------------------------------
;-------------------------------------------------------------------------------
    ; "enter password" dialog for KFar
;password_dlg:
;        dd      1       ; use standard dialog colors
;        dd      -1      ; center window by x
;        dd      -1      ; center window by y
;.width  dd      ?       ; width (will be filled according to current console width)
;        dd      2       ; height
;        dd      4, 2    ; border size
;        dd      aEnterPasswordTitle     ; title
;        dd      ?       ; colors (will be set by KFar)
;        dd      0       ; used internally by dialog manager, ignored
;        dd      0, 0    ; reserved for DlgProc
;        dd      2       ; 2 controls
;; the string "enter password"
;        dd      1       ; type: static
;        dd      1,0     ; upper-left position
;.width1 dd      ?,0     ; bottom-right position
;        dd      aEnterPassword  ; data
;        dd      0       ; flags
;; editbox for password
;        dd      3       ; type: edit
;        dd      1,1     ; upper-left position
;.width2 dd      ?,0     ; bottom-right position
;        dd      password_data   ; data
;        dd      2Ch     ; flags



proc DialogBox dlgInfo:dword
	pushad
	mov	ebx,[dlgInfo]
	mov	ebx,[ebx+25*4]
	add	ebx, 12	; password_ansi
	mov	[edtPassword+9*4], ebx	; text
	mov	[stateDlg], 0
	mcall	51,1,threadDialogBox,stackDlg

	;wait thread...
    @@: cmp	[stateDlg],0
	jne	@f
	mcall	5,1
	jmp	@b
     @@:
	popad
	cmp	[stateDlg], 1
	jne	@f
	xor	eax, eax
	ret
    @@:
	or	eax, -1
	ret
endp

proc threadDialogBox

	mcall	40, 100111b+0C000000h
	xor	eax,eax
	mov	dword[edtPassword.size], eax
	mov	dword[edtPassword.pos], eax

.wm_redraw:
	mcall	12, 1
	mcall	48, 3, sc, sizeof.system_colors
	mov	edx, [sc.work]
	or	edx, 0x33000000
	mcall	0, <200,320>, <200,140>, , , title

	edit_boxes_set_sys_color edtPack,endEdits,sc
	stdcall [edit_box_draw],edtPassword


	mov	ecx,[sc.work_text]
	or	ecx,90000000h
	mcall 4, <56,12>, , strGetPass

	mcall 8, <70,80>,<74,22>,2,[sc.work_button]
	mov	ecx,[sc.work_button_text]
	or	ecx,90000000h
	mcall 4, <103,79>, , strOk

	mcall 8, <165,80>,<74,22>,1,[sc.work_button]
	mov	ecx,[sc.work_button_text]
	or	ecx,90000000h
	mcall 4, <182,79>, , strCancel


	mcall 12, 2

.still:
	mcall	10
	cmp	eax, 1
	je	.wm_redraw
	cmp	eax, 2
	je	.wm_key
	cmp	eax, 3
	je	.wm_button
	cmp	eax, 6
	je	.wm_mouse

	jmp	.still

.wm_key:
	mcall	2
	stdcall [edit_box_key],edtPassword
	jmp	.still


.wm_button:
	mcall	17

	cmp	ah, 2		;OK
	jne	@f
	mov	[stateDlg],1
	jmp	.exit
    @@:

	cmp	ah, 1		;Close window or Cancel
	jne	 .still
	mov	[stateDlg],2
	jmp	.exit

.wm_mouse:
	stdcall [edit_box_mouse],edtPassword


	jmp	.still

.exit:
	mcall	-1
endp

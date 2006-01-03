;******************************************************************************
; project name:      TINYPAD
; compiler:          flat assembler 1.60
; memory to compile: 2 Mbytes +
; version:           3.78a
; last update:       13/05/2005
; maintained by:     Ivan Poddubny, Mike Semenyako (aka mike.dld)
; e-mail:            ivan-yar@bk.ru, mike.dld@tut.by
;******************************************************************************
; HISTORY:
; 3.79 (just some ideas for further releases)
;   optimize drawing (reduce flickering)
;   optimize memory usage (allocate only needed amount, not static 3 Mbytes)
;   improve keyboard handling (use 66th function)
;   introduce real selection capabilities (including block selection)
;   and of course other small speed and size optimizations ;)
; 3.78a (mike.dld)
;   fixed termination while typing in x positions higher than (line_length+10)
;   improved drawing on small heights
;     don't draw window while its height equals 0
; 3.78 (mike.dld)
;   now lines may be of ANY length;
;     optimized memory usage (less memory for internal file representation)
;       after loading file, it's internal size equals to its real size
;       plus 14 bytes for each line (4 bytes for line length
;         and 10 spaced to the end - to reduce data relocations)
;     completely rewritten keyboard handling;
;     added horizontal scrollbar;
;   all line feed formats are supported: WIN(CRLF),*NIX(LF),MAC(CR);
;   etc.
; 3.77 (mike.dld)
;   changed save_string to collapse SPACEs into TABs;
;   rewrote drawfile from scratch (speed++)
;     through some drawing improvements still needed
;     (some checkups to reduce flickering);
;   writepos (size--);
;   fixed drawing window while height < 100px, and for non-asm files;
;   several small fixes; speed/size optimizations
; 3.76 (mike.dld)
;   changed loadfile/loadhdfile to expand TABs into SPACEs;
;   changed TAB,ENTER,DELETE,BSPACE keys behaviour (rewritten from scratch);
;   vertical scrollbar;
;   extra window resizing capabilities (added a couple of constants);
;   completely new text cursor management & moving text cursor with mouse;
;   improved search function, moving cursor to beginning of text found;
;   adjustable max line width (change LINE_WIDTH & recompile) // (obsolete)
; 3.75a
;   fixed converting char to upper case in read_string
; 3.75
;   rewrote save_file from scratch; bugfix in loadfile;
; 3.74
;   optimisation
; 3.73
;   completly new load_file function
; 3.72
;   speed++
; 3.71
;   error beep
; 3.6,3.7:
;   many bugs fixed
;   simple toolbar
;   compile, run applications from TINYPAD, all fasm output is in debug board
;   TAB button
;   auto-indent
;   Ctrl+L - insert comment string
;******************************************************************************
; Memory 0x300000:
;   stack            0x00eff0 -  стэк
;   stack for help   0x00fff0 -
;   load position    0x010000 +  адрес загрузки файла
;   screen comp      0x078000 +  содержимое экрана (obsolete)
;   edit area        0x080000 +  документ
;   copy/paste area  0x2f0000 +  область для копирования/вставки
;******************************************************************************

include 'lang.inc'
include 'macros.inc' ; useful stuff
purge mov	     ;  SPEED

macro mov op1,op2 {
 if (op1 in __regs) & (op2 eq -1)
  or  op1,-1
 else
  mov op1,op2
 end if
}

 header '01',1,@CODE,TINYPAD_END,0x300000,0xeff0,@PARAMS,0

ASEPC = '-'
RBTNW = rstr.size*6/3
LBTNW = lstr.size*6/2
ABTNH = 16
ATOPH = 24
OLEFT = 5+1
SCRLW = 16
ATABW = 8
PATHL = 260

AMINS = 8

;-----------------------------------------------------------------------------
section @CODE ;///////////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------

;******************************************************************************
; INITIALIZING

	cld
	mov	esi,s_example
	mov	edi,s_fname
	mov	ecx,s_example.size
	mov	[s_fname.size],ecx
	rep	movsb
	mov	esi,s_still
	mov	edi,s_search
	mov	ecx,s_still.size
	mov	[s_search.size],ecx
	rep	movsb
	cmp	byte[@PARAMS],0
	jz	no_params

    ; parameters are at @PARAMS
	mov	esi,@PARAMS
	mov	edi,s_fname
	mov	ecx,PATHL
	rep	movsb
	mov	edi,s_fname
	mov	ecx,PATHL
	xor	al,al
	repne	scasb
	sub	edi,s_fname+1
	mov	[s_fname.size],edi

  no_params:
	mcall	40,00100111b
	jmp	do_load_file

;******************************************************************************
; MAIN LOOP
still:
	call	writepos ; write current position & number of strings

  .skip_write:
	mcall	10;23,50; wait here until event
	dec	eax	; redraw ?
	jz	red
	dec	eax	; key ?
	jz	key
	dec	eax	; button ?
	jz	button
	sub	eax,3	; mouse ?
	jz	mouse

	;d

	jmp	still.skip_write
;******************************************************************************

  mouse:
	mcall	9,p_info,-1
	cmp	ax,[p_info.window_stack_position]
	jne	still

	mcall	37,2
	test	al,1
	jz	.capture_off

	mcall	37,1
	mov	ebx,eax
	and	ebx,0x0000FFFF
	shr	eax,16

	mov	ecx,[top_ofs]
	inc	ecx
	pushd	OLEFT ecx [p_info.x_size] ecx
	popd	[__rc+0xC] [__rc+0x8] [__rc+0x4] [__rc+0x0]
	sub	[__rc+0x8],SCRLW+5+3
	imul	ecx,[slines],10
	dec	ecx
	add	[__rc+0xC],ecx
	mov	ecx,__rc
	call	pt_in_rect
	jnc	.check_vscroll
	sub	eax,OLEFT
	sub	ebx,[__rc+0x4]
	push	eax
	mov	eax,ebx
	xor	edx,edx
	mov	ecx,10
	div	ecx
    @@: add	eax,[top_line]
	mov	ebx,eax
	pop	eax
	xor	edx,edx
	mov	ecx,6
	div	ecx
    @@: add	eax,[left_col]

	cmp	eax,[columns]
	jb	@f
	mov	eax,[columns]
    @@: cmp	ebx,[lines]
	jb	@f
	mov	ebx,[lines]
	dec	ebx
    @@:
	cmp	[posx],eax
	jne	.change_cur_pos
	cmp	[posy],ebx
	je	still.skip_write

  .change_cur_pos:
	mov	[posx],eax
	mov	eax,[posy]
	pushd	ebx
	popd	[posy]
	cmp	eax,ebx
	je	@f
	push	ebx
	mov	ebx,eax
	call	drawfile.ex
	pop	eax
    @@: mov	ebx,eax
	call	drawfile.ex
	jmp	still

  .check_vscroll:
	mov	ecx,[p_info.x_size]
	sub	ecx,SCRLW+5-2
	pushd	ecx [top_ofs] ecx [bot_ofs]
	popd	[__rc+0xC] [__rc+0x8] [__rc+0x4] [__rc+0x0]
	add	[__rc+0x8],SCRLW-2
	add	[__rc+0x4],SCRLW+1
	sub	[__rc+0xC],SCRLW*2+3
	mov	ecx,__rc
	call	pt_in_rect
	jnc	.check_hscroll

	sub	ebx,[__rc+0x4]
	cmp	[vscrl_capt],0
	jge	.vcaptured
	mov	eax,[vscrl_top]
	cmp	ebx,eax
	jb	.center_vcapture
	add	eax,[vscrl_size]
	cmp	ebx,eax
	jae	.center_vcapture
	mov	eax,ebx
	sub	eax,[vscrl_top]
	dec	eax
	mov	[vscrl_capt],eax
	dec	ebx
	jmp	.vcaptured
  .center_vcapture:
	mov	eax,[vscrl_size]
	shr	eax,1
	mov	[vscrl_capt],eax
  .vcaptured:
	sub	ebx,[vscrl_capt]
	jns	@f
	xor	ebx,ebx
    @@:
	mov	[vscrl_top],ebx
	mov	eax,[lines]
	sub	eax,[slines]
	mul	ebx
	mov	ebx,[bot_ofs]
	sub	ebx,[top_ofs]
	sub	ebx,SCRLW*3+2+2 	;**
	sub	ebx,[vscrl_size]
	div	ebx
	cmp	eax,[top_line]
	je	still.skip_write
	mov	[top_line],eax
	call	check_bottom_right
	call	drawfile
	jmp	still.skip_write

  .check_hscroll:
	pushd	(OLEFT+SCRLW+1) [bot_ofs] [p_info.x_size] [bot_ofs]
	popd	[__rc+0xC] [__rc+0x8] [__rc+0x4] [__rc+0x0]
	add	[__rc+0x8],-SCRLW*2-10-1
	add	[__rc+0x4],-SCRLW
	add	[__rc+0xC],-2
	mov	ecx,__rc
	call	pt_in_rect
	jnc	.capture_off

	mov	ebx,eax
	sub	ebx,[__rc+0x0]
	cmp	[hscrl_capt],0
	jge	.hcaptured
	mov	eax,[hscrl_top]
	cmp	ebx,eax
	jb	.center_hcapture
	add	eax,[hscrl_size]
	cmp	ebx,eax
	jae	.center_hcapture
	mov	eax,ebx
	sub	eax,[hscrl_top]
	dec	eax
	mov	[hscrl_capt],eax
	dec	ebx
	jmp	.hcaptured
  .center_hcapture:
	mov	eax,[hscrl_size]
	shr	eax,1
	mov	[hscrl_capt],eax
  .hcaptured:
	sub	ebx,[hscrl_capt]
	jns	@f
	xor	ebx,ebx
    @@:
	mov	[hscrl_top],ebx
	mov	eax,[columns]
	sub	eax,[scolumns]
	mul	ebx
	mov	ebx,[p_info.x_size]
	sub	ebx,SCRLW*3+10+2	;**
	sub	ebx,[hscrl_size]
	div	ebx
	cmp	eax,[left_col]
	je	still.skip_write
	mov	[left_col],eax
	call	check_bottom_right
	call	drawfile
	jmp	still.skip_write

  .capture_off:
	or	[vscrl_capt],-1
	or	[hscrl_capt],-1
	jmp	still.skip_write

func pt_in_rect
	cmp	eax,[ecx+0x0]
	jl	@f
	cmp	ebx,[ecx+0x4]
	jl	@f
	cmp	eax,[ecx+0x8]
	jg	@f
	cmp	ebx,[ecx+0xC]
	jg	@f
	stc
	ret
    @@: clc
	ret
endf

func check_bottom_right
	push	eax
	mov	eax,[top_line]
	add	eax,[slines]
	cmp	eax,[lines]
	jbe	.lp1
	mov	eax,[lines]
	sub	eax,[slines]
	jns	@f
	xor	eax,eax
    @@: mov	[top_line],eax
  .lp1: mov	eax,[left_col]
	add	eax,[scolumns]
	cmp	eax,[columns]
	jbe	.exit
	mov	eax,[columns]
	sub	eax,[scolumns]
	jns	@f
	xor	eax,eax
    @@: mov	[left_col],eax
  .exit:
	pop	eax
	ret
endf

; *********************************
; *         BUTTON HANDLER        *
; *********************************

  button:

	mcall	17
	shr	eax,8

	cmp	eax,'UP'
	jne	not_up
	dec	[top_line]
	jns	check_inv_all.skip_check
	mov	[top_line],0
	jmp	still.skip_write
  not_up:

	cmp	eax,'DN'
	jne	not_down
	inc	[top_line]
	mov	eax,[lines]
	sub	eax,[slines]
	cmp	eax,[top_line]
	jae	check_inv_all.skip_check
	dec	eax
	mov	[top_line],eax
	jmp	still.skip_write
  not_down:

	cmp	eax,'LT'
	jne	not_left
	dec	[left_col]
	jns	check_inv_all.skip_check
	mov	[left_col],0
	jmp	still.skip_write
  not_left:

	cmp	eax,'RT'
	jne	not_right
	inc	[left_col]
	mov	eax,[columns]
	sub	eax,[scolumns]
	cmp	eax,[left_col]
	jae	check_inv_all.skip_check
	dec	eax
	mov	[left_col],eax
	jmp	still.skip_write
  not_right:

; SEARCH {
  search:
	cmp	al,50
	jne	no_search

  .skip_check:

	cld
	mov	ecx,[posy]
	mov	edx,ecx
	call	get_line_offset
	cmp	dword[esi],0
	je	still
	call	get_real_length
	add	esi,4
	or	eax,eax
	jz	.end_line.2
	mov	ecx,eax
	sub	ecx,[posx]
	push	esi
	add	esi,[posx]
	dec	ecx
	inc	esi
	jmp	@f

  .next_line:
	push	esi
    @@:
	sub	ecx,[s_search.size]
	inc	ecx

  .next_char:
	dec	ecx
	js	.end_line
	xor	edi,edi

  .next_ok:
	movzx	eax,byte[edi+esi]
	movzx	ebx,byte[edi+s_search]

	cmp	al,$61
	jb	@f
	add	al,[eax+add_table-$61]
    @@: cmp	bl,$61
	jb	@f
	add	bl,[ebx+add_table-$61]
    @@:

	cmp	al,bl
	je	@f

	inc	esi
	jmp	.next_char
    @@:
	inc	edi
	cmp	edi,[s_search.size]
	jne	.next_ok

  .found:
	add	esp,4
	mov	[posy],edx
	mov	ecx,edx
	lea	eax,[esi-4]
	call	get_line_offset
	sub	eax,esi
	mov	[posx],eax
	jmp	check_inv_all

  .end_line:
	pop	esi
  .end_line.2:
	add	esi,[esi-4]
	inc	edx
	call	get_real_length
	mov	ecx,eax
	lodsd
	or	eax,eax
	jnz	.next_line
	jmp	still

; SEARCH }

  no_search:

; TOOLBAR {
	cmp	eax,10000
	jb	no_toolbar

	add	eax,-10000
	jnz	@f
	mov	bl,0;[run_outfile],0
	call	start_fasm
	jmp	still
    @@: dec	eax
	jnz	@f
	mov	bl,1;[run_outfile],1
	call	start_fasm
	jmp	still
    @@: dec	eax
	jnz	@f
	call	open_debug_board
	jmp	still
    @@: dec	eax
	jnz	still
	call	open_sysfuncs_txt
	jmp	still
; TOOLBAR }

  no_toolbar:

	cmp	al,4
	jne	noid4

; LOAD_FILE {
  do_load_file:
	cmp	[s_fname],'/'
	jne	@f
	call	loadhdfile
	jmp	.restorecursor
    @@: call	loadfile

    .restorecursor:
	xor	eax,eax
	mov	[top_line],eax
	mov	[posx],eax
	mov	[posy],eax

; enable color syntax for ASM and INC files:
	mov	[asm_mode],al

	mov	eax,[s_fname.size]
	add	eax,s_fname
	mov	byte[eax],0
	cmp	dword[eax-3],'ASM'
	jne	@f
	inc	[asm_mode]
	jmp	.nocol
    @@: cmp	dword[eax-3],'INC'
	jne	.nocol
	inc	[asm_mode]
    .nocol:

; if the header is the same as previous,
; just redraw the text area
; else redraw the window

	mov	ecx, [s_fname.size]
	add	ecx, 10 	    ; strlen(" - TINYPAD");
	cmp	ecx, [s_title.size]
	jne	@f
	add	ecx, -10
	mov	esi, s_fname	   ; strcmp(s_fname,header);
	mov	edi, s_title
	rep	cmpsb
	jne	@f
	call	drawfile
	jmp	still
    @@:

; set window title:
	mov	esi,s_fname
	mov	edi,s_title
	mov	ecx,[s_fname.size]
	lea	eax,[ecx+10]
	mov	[s_title.size],eax
	cld
	rep	movsb

	mov	dword[edi],' - '
	add	edi,3
	mov	esi,htext
	mov	ecx,htext.size
	rep	movsb

	call	drawwindow
	jmp	still
; LOAD_FILE }

  noid4:

	cmp	al, 2
	jz	yessave

	dec	al	 ; close if butid == 1
	jnz	nosave
; EXIT:
	mov	[main_closed],1
	mcall	-1

; SAVE_FILE {
  yessave:
	call	save_file
	jmp	still
; SAVE_FILE }

  nosave:
	inc	al
	call	read_string
	jmp	still

;**********************************
;*         REDRAW HANDLER         *
;**********************************

func red
; перерисовка окна
	call	drawwindow
	jmp	check_inv_all.skip_check
endf

;**********************************
;*          KEY HANDLER           *
;**********************************

key:
	mcall	2		; GET KEY
	cmp	al,1
	je	still

	shr	eax,8

; HELP_WND {
	cmp	al,0xD2;210  ; Ctrl + F1
	jne	no_help_text

	mcall	51,1,help_thread_start,0xfff0
	jmp	still
; HELP_WND }

  no_help_text:
; LOAD_FILE {
	cmp	al,211	     ; Ctrl + F2
	je	do_load_file
; LOAD_FILE }

; SEARCH {
	cmp	al,212	     ; Ctrl + F3
	je	search.skip_check
; SEARCH }

; SAVE_FILE {
	cmp	al,213	     ; Ctrl + F4
	je	yessave
; SAVE_FILE }

; ENTER_FILENAME {
	cmp	al,214	     ; Ctrl + F5
	jne	@f
	mov	al,5
	call	read_string
	jmp	still
    @@:
; ENTER_FILENAME }

; ENTER_SEARCH {
	cmp	al,215	     ; Ctrl + F6
	jne	@f
	mov	al,51
	call	read_string
	jmp	still
    @@:
; ENTER_SEARCH }

; CHANGE_LANG_LAYOUT {
	cmp	al,217	     ; Ctrl + F8
	jne	@f
	call	layout
	jmp	still
    @@:
; CHANGE_LANG_LAYOUT }

; 3 times english -> русский
; 2 times русский -> english

; COPY START {
	cmp	al,19
	jne	no_copy_start
	push	[posy]
	pop	[copy_start]
	jmp	still
; COPY START }

  no_copy_start:

; COPY END {
	cmp	al,5
	jne	no_copy_end
	mov	eax,[posy]
	cmp	eax,[copy_start]
	jae	@f
	xchg	eax,[copy_start]
    @@: sub	eax,[copy_start]
	inc	eax
	mov	[copy_count],eax

	mov	ecx,[copy_start]
	call	get_line_offset
	push	esi
	mov	ecx,eax
    @@: lodsd
	add	esi,eax
	loop	@b
	mov	ecx,esi
	pop	esi
	sub	ecx,esi
	mov	[copy_size],ecx

	mov	edi,0x2f0000
	cld
	rep	movsb
	jmp	still
; COPY END }

  no_copy_end:

; PASTE {
	cmp	al,16
	jne	no_copy_paste
	mov	ebx,[copy_count]
	or	ebx,ebx
	jz	still
	add	[lines],ebx

	mov	ecx,[posy]
	call	get_line_offset
	mov	ebx,esi
	mov	edi,0x2E0000
	mov	esi,edi
	mov	ecx,[copy_size]
	sub	esi,ecx
	lea	ecx,[esi+4]
	sub	ecx,ebx
	std
	rep	movsb
	mov	edi,ebx
	mov	esi,0x2F0000
	mov	ecx,[copy_size]
	cld
	rep	movsb

	jmp	check_inv_all
; PASTE }

  no_copy_paste:

; INSERT_SEPARATOR {
	cmp	al,0x0C       ; Ctrl+L
	jne	no_insert_separator

	mov	ecx,[posy]
	call	get_line_offset
	mov	ebx,esi

	mov	ecx,[lines]
	call	get_line_offset
	lea	edi,[esi+90+4]
	lea	ecx,[esi+4]
	sub	ecx,ebx
	std
	rep	movsb

	lea	edi,[ebx+5]
	mov	dword[ebx],90
	mov	al,ASEPC
	mov	ecx,79
	cld
	rep	stosb
	mov	al,' '
	mov	ecx,10
	rep	stosb
	mov	byte[ebx+4],';'

	inc	[lines]
	inc	[posy]

	jmp	check_inv_all
; INSERT_SEPARATOR }

  no_insert_separator:

; DEL_LINE {
	cmp	al,4
	jne	no_delete_line

	mov	eax,[posy]
	inc	eax
	cmp	eax,[lines]
	jge	still

	mov	ecx,[posy]
	call	get_line_offset
	mov	edi,esi
	lodsd
	add	esi,eax

	dec	[lines]
	mov	ecx,0x2e0000
	sub	ecx,esi
	shr	ecx,2		       ;// fixed (was 4)
	cld
	rep	movsd
	jmp	check_inv_all
; DEL_LINE }

  no_delete_line:

; ENTER {
	cmp	al,13
	jnz	noenter

	mov	ecx,[posy]
	call	get_line_offset

	mov	ebx,[posx]
	cmp	ebx,[esi]
	jb	@f
	mov	ebx,[esi]
	dec	ebx
	jns	@f
	xor	ebx,ebx
    @@:

	cld

	mov	edi,0x10000
	mov	ebp,esi
	mov	ecx,ebx
	inc	ecx
    @@: dec	ecx
	jz	@f
	cmp	byte[esi+ecx+4-1],' '
	je	@b
    @@: lea	eax,[ecx+10]
	stosd
	jecxz	@f
	push	esi
	add	esi,4
	rep	movsb
	pop	esi
    @@: mov	al,' '
	mov	ecx,10
	rep	stosb

	mov	ecx,[esi]
	sub	ecx,ebx;[posx]
	add	esi,ebx;[posx]
	add	esi,4
	inc	ecx
    @@: dec	ecx
	jz	@f
	cmp	byte[esi+ecx-1],' '
	je	@b
    @@: jz	.lp1
    @@: cmp	byte[esi],' '
	jne	.lp1
	inc	esi
	loop	@b
  .lp1: push	edi ecx
	mov	ecx,[ebp]
	lea	edi,[ebp+4]
	mov	al,' '
	repe	scasb
	mov	eax,ecx
	pop	ecx edi
	je	.lp2
	neg	eax
	add	eax,[ebp]
	dec	eax
	jmp	@f
  .lp2: xor	eax,eax
    @@: mov	edx,edi
	add	edi,4
	mov	[posx],eax
	jecxz	@f
	push	ecx
	mov	ecx,eax
	mov	al,' '
	rep	stosb
	pop	ecx
    @@: jecxz	@f
	rep	movsb
    @@: mov	ecx,10
	mov	al,' '
	rep	stosb

	lea	eax,[edi-4]
	sub	eax,edx
	mov	[edx],eax

	lea	ecx,[edi-0x10000]

	push	ecx
	mov	edi,0x2E0000
	lea	esi,[edi+4]
	sub	esi,ecx
	add	esi,[ebp]
	lea	ecx,[esi-4]
	sub	ecx,ebp
	std
	cmp	esi,edi
	jb	@f
	je	.lp3
	lea	esi,[ebp+4]
	mov	eax,[esp]
	lea	edi,[esi+eax-4]
	add	esi,[ebp]
	mov	ecx,0x2E0000
	sub	ecx,esi
	cld
    @@: rep	movsb
  .lp3: pop	ecx
	mov	esi,0x10000
	mov	edi,ebp
	cld
	rep	movsb

	inc	[posy]
	inc	[lines]

	jmp	check_inv_all
; ENTER }

  noenter:

; UP {
	cmp	al,130+48
	jnz	noup

	mov	eax,[posy]
	dec	eax
	jns	@f
	xor	eax,eax
    @@: mov	ecx,[top_line]
	cmp	eax,ecx
	jae	@f
	dec	ecx
	jns	@f
	xor	ecx,ecx
    @@: jmp	check_inv_all.skip_init
; UP }

  noup:

; DOWN {
	cmp	al,129+48
	jnz	nodown

	mov	eax,[posy]
	inc	eax
	cmp	eax,[lines]
	jb	@f
	dec	eax
    @@: mov	ecx,[top_line]
	mov	edx,eax
	sub	edx,ecx
	cmp	edx,[slines]
	jb	@f
	inc	ecx
    @@: jmp	check_inv_all.skip_init
; DOWN }

  nodown:

; LEFT {
	cmp	al,128+48
	jnz	noleft
	mov	eax,[posx]
	dec	eax
	jns	@f
	inc	eax
    @@: mov	[posx],eax
	jmp	check_inv_all
; LEFT }

  noleft:

; RIGHT {
	cmp	al,131+48
	jnz	noright
	mov	eax,[posx]
	inc	eax
	cmp	eax,[columns]
	jbe	@f
	dec	eax
    @@: mov	[posx],eax
	jmp	check_inv_all
; RIGHT }

  noright:

; PAGE_UP {
  page_up:
	cmp	al,136+48
	jnz	nopu
	mov	edx,[slines]
	dec	edx
	mov	eax,[posy]
	mov	ecx,[top_line]
	sub	eax,edx
	jns	@f
	xor	eax,eax
    @@: sub	ecx,edx
	jns	@f
	xor	ecx,ecx
    @@: jmp	check_inv_all.skip_init
; PAGE_UP }

  nopu:

; PAGE_DOWN {
  page_down:
	cmp	al,135+48
	jnz	nopd
	mov	edx,[slines]
	dec	edx
	mov	eax,[posy]
	mov	ecx,[top_line]
	add	eax,edx
	add	ecx,edx
	cmp	eax,[lines]
	jb	@f
	mov	eax,[lines]
	dec	eax
    @@: jmp  check_inv_all.skip_init
; PAGE_DOWN }

  nopd:

; HOME {
	cmp	al,132+48
	jnz	nohome
	mov	[posx],0
	jmp	check_inv_all
; HOME }

  nohome:

; END {
  end_key:
	cmp	al,133+48
	jnz	noend

	mov	ecx,[posy]
	call	get_line_offset
	call	get_real_length
	mov	[posx],eax
	jmp	check_inv_all
; END }

  noend:

; GO_START {
	cmp	al,251	       ; Ctrl + [
	jnz	no_go_to_start
	xor	eax,eax
	mov	[top_line],eax
	mov	[posy],eax
	jmp	check_inv_all.skip_check
; GO_START }

  no_go_to_start:

; GO_END {
	cmp	al,253	       ; Ctrl + ]
	jnz	no_go_to_end
	mov	eax,[lines]    ; eax = lines in the file
	mov	[posy],eax
	sub	eax,[slines]   ; eax -= lines on the screen
	jns	@f
	xor	eax,eax
    @@: mov	[top_line],eax
	dec	[posy]
	jmp	check_inv_all.skip_check
; GO_END }

  no_go_to_end:

; DELETE {
  __key_delete:
	cmp	al,134+48
	jne	nodel

	mov	ecx,[posy]
	call	get_line_offset
	lea	ebx,[esi+4]
	mov	ebp,esi

	call	get_real_length
	or	eax,eax
	je	.line_up

	mov	ecx,[posx]
	cmp	ecx,eax
	jae	.line_up
	lea	edi,[ebx+ecx]
	neg	ecx
	add	ecx,[ebp]
	repe	scasb
	je	.line_up

	mov	edi,ebx
	mov	ecx,[posx]
	add	edi,ecx
	lea	esi,[edi+1]
	neg	ecx
	add	ecx,[ebp]
	dec	ecx
	rep	movsb
	mov	byte[edi],' '

	jmp	check_inv_all

  .line_up:
	mov	eax,[lines]
	dec	eax
	cmp	eax,[posy]
	je	still;.ok
	mov	edi,0x10004
	mov	esi,ebx
	mov	ecx,[posx]
	rep	movsb
	mov	ecx,[posx]
	mov	[0x10000],ecx
	cmp	ecx,[ebp]
	jbe	@f
	sub	ecx,[ebp]
	sub	edi,ecx
	mov	al,' '
	rep	stosb
    @@: lea	esi,[ebx+4]
	add	esi,[ebp]
	mov	ecx,[esi-4]
	add	[0x10000],ecx
	rep	movsb

	lea	ecx,[edi-0x10000]

	mov	esi,0x10000
	call	get_real_length
	cmp	eax,[columns]
	jbe	@f
	mov	[columns],eax
    @@:

	push	ecx
	mov	edi,0x2E0000
	lea	esi,[edi+8]
	sub	esi,ecx
	add	esi,[ebp]
	lea	eax,[ebp+4]
	add	eax,[ebp]
	add	esi,[eax]
	lea	ecx,[esi-4]
	sub	ecx,ebp
	std
	cmp	esi,edi
	jb	@f
	jz	.lp1
	mov	edi,ebp
	add	edi,[esp]
	lea	esi,[ebp+8]
	add	esi,[esi-8]
	add	esi,[esi-4]
	mov	ecx,0x2E0000
	sub	ecx,esi
	cld
    @@: rep	movsb
  .lp1: pop	ecx
	mov	esi,0x10000
	mov	edi,ebp
	cld
	rep	movsb

  .ok.dec.lines:
	dec	[lines]
	mov	eax,[lines]
	cmp	[posy],eax
	jb	check_inv_all
	dec	eax
	mov	[posy],eax
	jmp	check_inv_all
; DELETE }

  nodel:

; INSERT {
	cmp	al,137+48
	jnz	noins
	;// ... toggle insert/overwrite mode here ...
	jmp	still;check_inv_str
; INSERT }

  noins:

; BACKSPACE {
	cmp	al,8
	jnz	nobs

	mov	eax,[posx]
	dec	eax
	js	.line_up

	dec	[posx]
	mov	ecx,[posy]
	call	get_line_offset

	mov	ebx,eax
	call	get_real_length
	cmp	eax,[posx]
	jb	check_inv_all

	lea	edi,[esi+4+ebx]
	mov	ecx,ebx
	neg	ecx
	add	ecx,[esi]
	dec	ecx
	lea	esi,[edi+1]
	cld
	rep	movsb
	mov	byte[edi],' '

	jmp	check_inv_str

  .line_up:
	cmp	[posy],0
	je	still
	mov	ecx,[posy]
	dec	ecx
	call	get_line_offset
	mov	ebp,esi
	lea	ebx,[esi+4]
	mov	ecx,[ebp]
    @@: cmp	byte[ebx+ecx-1],' '
	jne	@f
	dec	ecx
	jg	@b
    @@: mov	[posx],ecx
	dec	[posy]
	cld
	jmp	__key_delete.line_up
; BACKSPACE }

  nobs:

; TAB {
  __key_tab:
	cmp	eax,9  ; Tab
	jne	notab

	mov	eax,[posx]

	mov	ecx,eax
	add	eax,ATABW
	and	eax,not(ATABW-1)
	push	eax ' '
	sub	eax,ecx
  .direct:
	mov	ecx,[posy]
	call	get_line_offset

	xchg	eax,ecx

	call	get_real_length
	cmp	eax,[posx]
	jae	@f
	mov	eax,[posx]
    @@: mov	edx,[esi]
	sub	edx,eax
	cmp	ecx,edx
	jl	@f
	pushad; esi ecx eax
	mov	ecx,0x2E0000-10+1
	lea	eax,[esi+4]
	add	eax,[esi]
	sub	ecx,eax
	mov	edi,0x2E0000
	mov	esi,0x2E0000-10
	std
	rep	movsb
	mov	ecx,10
	mov	al,' '
	rep	stosb
	popad;  eax ecx esi
	add	dword[esi],10
	jmp	@b
    @@: lea	ebx,[esi+4]
	push	ecx
	lea	edi,[ebx-1]
	add	edi,[esi]
	mov	esi,edi
	sub	esi,ecx
	lea	ecx,[esi+1]
	sub	ecx,ebx
	sub	ecx,[posx]
	std
	rep	movsb
  .ok:	pop	ecx
	pop	eax
	rep	stosb

	cld

	pop	[posx]

	lea	esi,[ebx-4]
	call	get_real_length
	cmp	eax,[posx]
	jae	@f
	mov	eax,[posx]
    @@: cmp	eax,[columns]
	jbe	@f
	mov	[columns],eax
    @@:
	jmp	check_inv_all
; TAB }

  notab:

; ADD_KEY {
	push	[posx] eax
	inc	dword[esp+4]
	mov	eax,1
	jmp	__key_tab.direct
; ADD_KEY }

check_inv_str = check_inv_all
@^
func check_inv_str
	mov	eax,[posy]
	mov	ecx,[top_line]
  .skip_init:
	call	check_cur_vis
	mov	[posy],eax
	mov	[top_line],ecx
  .skip_check:
;       call    invalidate_string
	call	drawfile
	jmp	still
endf
^@
func check_inv_all
	mov	eax,[posy]
	mov	ecx,[top_line]
  .skip_init:
	call	check_cur_vis
	mov	[posy],eax
	mov	[top_line],ecx
  .skip_check:
;       call    clear_screen
	call	drawfile
	jmp	still
endf

func check_cur_vis
	cmp	eax,ecx
	jb	.low
	mov	edx,ecx
	add	edx,[slines]
	cmp	edx,[lines]
	jbe	@f
	mov	edx,[lines]
    @@: cmp	eax,edx
	jb	@f
	lea	ecx,[eax+1]
	sub	ecx,[slines]
	jns	@f
	xor	ecx,ecx
	jmp	@f
  .low: mov	ecx,eax
    @@:
	mov	edx,ecx
	add	edx,[slines]
	cmp	edx,[lines]
	jbe	@f
	mov	ecx,[lines]
	sub	ecx,[slines]
	jns	@f
	xor	ecx,ecx
    @@:;mov     [top_line],eax

	pushad
	mov	eax,[posx]
	mov	ebx,[left_col]
	mov	ecx,ebx
	add	ecx,[scolumns]
	cmp	eax,ebx
	jb	.lp1
	cmp	eax,ecx
	jb	.exit.2
	lea	ebx,[eax]
	sub	ebx,[scolumns]
	jmp	@f
  .lp1: mov	ebx,eax
    @@: mov	[left_col],ebx
  .exit.2:
	mov	[posx],eax
	popad

	ret
endf

func get_real_length
	mov	eax,[esi]
    @@: cmp	byte[esi+eax+4-1],' '
	jne	@f
	dec	eax
	jnz	@b
    @@: ret
endf

;******************************************************************************

;----------------------------------------------------------------------------
func start_fasm ;////////////////////////////////////////////////////////////
;----------------------------------------------------------------------------
; BL = run after compile
;----------------------------------------------------------------------------
	cmp	[asm_mode],0
	jne	@f
	ret
    @@: mov	esi,s_fname
	mov	edi,fasm_parameters

	cmp	byte[esi],'/'
	je	.yes_systree

	mov	ecx,[s_fname.size]
	rep	movsb

	mov	al,','
	stosb

	mov	ecx,[s_fname.size]
	add	ecx,-4
	mov	esi,s_fname
	rep	movsb

	mov	al,','
	stosb

	mov	dword[edi],'/RD/'
	mov	word[edi+4],'1/'
	add	edi,6

	mov	al,0
	stosb

	jmp	.run

 .yes_systree:
	add	esi,[s_fname.size]
	dec	esi

	xor	ecx,ecx
	mov	al,'/'
    @@: cmp	[esi],al
	je	@f
	dec	esi
	inc	ecx
	jmp	@b
    @@: inc	esi

	push	esi esi ecx

	rep	movsb

	mov	al,','
	stosb

	pop	ecx esi

	add	ecx,-4
	rep	movsb

	mov	al,','
	stosb

	pop	ecx
	sub	ecx,s_fname
	mov	esi,s_fname

	rep	movsb

	mov	al,0
	stosb

 .run:
	cmp	bl,0 ; run outfile ?
	je	@f
	mov	dword[edi-1],',run'
	mov	byte[edi+3],0
    @@: mcall	19,fasm_filename,fasm_parameters
	ret
endf

func open_debug_board
	mcall	19,debug_filename,0
	ret
endf

func open_sysfuncs_txt
	mcall	19,tinypad_filename,sysfuncs_filename
	ret
endf

func layout
; сменить раскладку клавиатуры
	mcall	19,setup,param_setup
	mcall	5,eax
;       call    activate_me
;       ret
;endf

;func activate_me
	mcall	9,p_info,-1
	inc	eax
	inc	eax
	mov	ecx,eax
	mov	edi,[p_info.PID]
	mov	ebx,p_info
    @@: dec	ecx
	jz	@f    ; counter=0 => not found? => return
	mcall	9
	cmp	edi,[p_info.PID]
	jne	@b
	mcall	18,3
	mcall	5,eax
    @@: ret
endf

; *******************************************************************
; **************************  DRAW WINDOW  **************************
; *******************************************************************

func drawwindow

	mcall	48,3,sc,sizeof.system_colors

	mcall	12,1

	push	[color_tbl+4*5]
	pop	[sc.work]

	mov	edx,[sc.work]
	add	edx,0x03000000
	mov	esi,[sc.grab]
	or	esi,0x80000000
	mcall	0,<100,6*80+6+OLEFT+SCRLW>,<75,402>,,,[sc.frame]

	mcall	48,4
	mov	[skinh],eax
	push	eax

	mcall	9,p_info,-1
	pop	eax
	cmp	[p_info.y_size],0
	je	.exit.2

	mov	[top_ofs],eax
	dec	[top_ofs]
	cmp	[asm_mode],0
	je	@f
	add	[top_ofs],ATOPH+1
    @@:
; header string
	mov	ebx,eax
	shr	ebx,1
	adc	ebx,1+0x000A0000-4
	mcall	4,,[sc.grab_text],s_title,[s_title.size]

	mov	eax,[p_info.x_size]
	sub	eax,5*2+SCRLW+2
	cdq
	mov	ebx,6
	div	ebx
	mov	[scolumns],eax

	mov	eax,[p_info.y_size] ; calculate buttons position
	add	eax,-5-ABTNH*3-2-2
	mov	[bot_ofs],eax

;       mov     eax,[procinfo.y_size]
	mov	[do_not_draw],1 ; do_not_draw = true

	mov	ebx,eax
	sub	ebx,[skinh]
	sub	ebx,ATOPH+SCRLW*3+AMINS+5
	js	.no_draw

;        cmp     eax,100
;        jb      .no_draw        ; do not draw text & buttons if height < 100

	mov	[do_not_draw],0 ; do_not_draw = false
	sub	eax,SCRLW+2+2
	sub	eax,[top_ofs]
	cdq
	mov	ebx,10
	div	ebx
	mov	[slines],eax

	mov	ebx,[p_info.x_size]
	add	ebx,5*65536-5
	mov	ecx,[top_ofs-2]
	mov	cx,word[top_ofs]
	mcall	38,,,[sc.work_text]
	mov	ecx,[bot_ofs-2]
	mov	cx,word[bot_ofs]
	sub	ecx,0x00010001
	push	ecx
	mcall
	add	ecx,(ABTNH+2)*65536+ABTNH+2
	mcall
	add	ecx,(ABTNH+2)*65536+ABTNH+2
	mcall
	pop	ecx
	add	cx,(ABTNH+2)*3
	mov	ebx,[p_info.x_size]
	sub	ebx,RBTNW+5+1
	push	bx
	shl	ebx,16
	pop	bx
	mcall
	mov	ebx,(5+LBTNW+1)*65536+(5+LBTNW+1)
	add	ecx,(ABTNH+2)*65536
	mcall

	inc	[top_ofs]

	mov	ebx,5*65536
	mov	bx,word[p_info.x_size]
	sub	bx,9
	push	bx
	sub	bx,RBTNW+2
	mov	ecx,[bot_ofs-2]
	mov	cx,ABTNH+1
	mcall	13,,,[sc.work_graph]		; BAR STRIPE

	pop	bx
	cmp	[asm_mode],0
	je	.skip_top_btns
	mov	ecx,[skinh-2]
	mov	cx,ATOPH
	mcall					; UPPER BAR

	mov	eax,8
	mov	ebx,6*65536+52
	mov	ecx,[skinh]
	inc	ecx
	shl	ecx,16
	add	ecx,ATOPH-3
	mov	edx,10000
	mov	esi,[sc.work_button]
    @@:
	mcall					; TOOLBAR BUTTONS
	add	ebx,54*65536
	inc	edx
	cmp	edx,10004
	jb	@b

  .skip_top_btns:
	mov	ebx,5*65536+LBTNW
	mov	ecx,[bot_ofs]
	shl	ecx,16
	add	ecx,(ABTNH+2)*65536+ABTNH
	mcall	8,,,5,[sc.work_button]		; FILE BUTTON

	add	ecx,(ABTNH+2)*65536
	mcall	,,,51				; STRING BUTTON

	push	ebx ecx

	mov	ebx,[p_info.x_size]
	shl	ebx,16
	add	ebx,(-5-RBTNW)*65536+RBTNW
	mov	ecx,[bot_ofs]
	shl	ecx,16
	add	ecx,ABTNH
	mov	edx,2
	mcall	,,,2				; SAVE BUTTON

	add	ecx,(ABTNH+2)*65536
	mcall	,,,4				; FILE BUTTON

	add	ecx,(ABTNH+2)*65536
	mcall	,,,50				; SEARCH BUTTON

	shr	ecx,16
	mov	bx,cx
	add	ebx,-ABTNH-2-ABTNH/2-2-3
	mcall	4,,[sc.work_button_text],rstr,rstr.size/3
	add	ebx,ABTNH+2
	add	edx,rstr.size/3
	mcall
	add	ebx,ABTNH+2
	add	edx,rstr.size/3
	mcall

	pop	edi ebx
	shr	edi,16
	mov	bx,di
	add	ebx,-1-ABTNH/2-2-2
	mcall	,,,lstr,lstr.size/2
	add	ebx,ABTNH+2
	add	edx,lstr.size/2
	mcall

	cmp	[asm_mode],0
	je	@f
	mov	ebx,[skinh]
	add	ebx,0x000C0000+ATOPH/2-4
	mcall	,,,toolbar_btn_text,toolbar_btn_text.size
    @@:
	mov	eax,[bot_ofs]
	add	eax,ABTNH+2
	mov	[ya],eax
	mov	[addr],s_fname
	call	print_text

	add	eax,ABTNH+2
	mov	[ya],eax
	mov	[addr],s_search
	call	print_text
	jmp	.exit

  .no_draw:
	mov	ebx,[skinh]
	mov	[top_ofs],ebx
	mov	eax,[p_info.y_size]
	add	eax,-3
	mov	[bot_ofs],eax
	sub	eax,ebx
	push	eax
	add	eax,-2-SCRLW
	cdq
	mov	ebx,10
	idiv	ebx
	mov	[slines],eax
	pop	eax

	mov	ebx,[p_info.y_size]
	sub	ebx,[skinh]
	sub	ebx,SCRLW*3++AMINS+7
	jns	@f

	inc	[do_not_draw]

	add	eax,-2
	cdq
	mov	ebx,10
	idiv	ebx
	mov	[slines],eax

	mov	eax,[p_info.x_size]
	sub	eax,5*2
	cdq
	mov	ebx,6
	idiv	ebx
	mov	[scolumns],eax
    @@:

  .exit:
	call	drawfile
  .exit.2:
	mcall	12,2
	ret
endf

;--------------------------------------------
func get_line_offset
;--------------------------------------------
; Input:
;  ECX = line number
; Output:
;  ESI = line data offset
;--------------------------------------------
	push	eax
	mov	esi,0x80000
	jecxz	.exit
    @@: lodsd
	add	esi,eax
	loop	@b
  .exit:
	pop	eax
	ret
endf

; **********************************
; ***********  DRAWFILE  ***********
; **********************************

;---------------------------------------------------
func drawfile.ex
;---------------------------------------------------
; Input:
;  EAX = start line
;  EBX = end line
;---------------------------------------------------
	cmp	[p_info.y_size],0
	jne	@f
	ret
    @@:
	call	check_bottom_right

	pushad

	cmp	[slines],0
	jle	drawfile.exit

	cmp	eax,ebx
	jae	@f
	xchg	eax,ebx
    @@: cmp	eax,[top_line]
	jae	@f
	mov	eax,[top_line]
    @@: mov	ecx,[top_line]
	add	ecx,[slines]
	cmp	ebx,ecx
	jb	@f
	dec	ecx
	mov	ebx,ecx
    @@: cmp	eax,ebx
	ja	drawfile.exit

	mov	ecx,eax
	call	get_line_offset

  .start:
	mov	ecx,ebx
	sub	ecx,eax

	mov	ebx,[top_ofs]
	add	ebx,OLEFT*65536+1
	sub	eax,[top_line]
	imul	eax,10
	add	ebx,eax

	imul	ebp,[left_col],6*65536

	jmp	drawfile.next_line
endf

func drawfile
	cmp	[p_info.y_size],0
	jne	@f
	ret
    @@:
	call	check_bottom_right

	pushad

	mov	ebx,[top_ofs]
	add	ebx,OLEFT*65536+1

	mov	ecx,[top_line]
	call	get_line_offset

  .start:
	mov	ecx,[slines]
	or	ecx,ecx
	jle	.exit

	imul	ebp,[left_col],6*65536

  .next_line:

	push	ecx ebx

	mov	ecx,ebx
	shl	ecx,16
	mov	cl,10
	mov	ebx,[p_info.x_size]
	add	ebx,(OLEFT-1)*65536-10-SCRLW ; <OLEFT-1,LINE_WIDTH*6+2>
	mcall	13,,,[color_tbl+4*5]

	xor	ecx,ecx
	lodsd
	mov	[cur_line_len],eax

	or	eax,eax
	ja	.next_block
	add	esp,4*2
	jmp	.draw_cursor

  .next_block:

	push	esi
	push	ecx
	call	get_next_part
	pop	ebx
	push	ecx
	mov	ecx,eax

	push	esi ebx
	mov	eax,ebx
	sub	ebx,[left_col]
	cmp	ebx,[scolumns]
	jge	@f
	add	ebx,esi
	jle	@f
	mov	ebx,[esp+8+4*2] ;// 4*2=esi+ebx
	sub	eax,[left_col]
	jge	.qqq
	sub	edx,eax
	add	esi,eax
	mov	eax,OLEFT*65536
	jmp	.qqq2
  .qqq:
	inc	eax
	imul	eax,6*65536
  .qqq2:
	and	ebx,0x0000FFFF
	add	ebx,eax

	mov	eax,[esp]
	add	eax,[esp+4];esi
	sub	eax,[left_col]
	sub	eax,[scolumns]
	jle	.qweqwe
	sub	esi,eax
  .qweqwe:

	mcall	4;[esp+8]
    @@:
	pop	eax eax ; ebx esi
	imul	eax,6
	add	[esp+10],ax
	pop	ecx esi
	cmp	ecx,[cur_line_len];LINE_WIDTH
	jl	.next_block

	pop	ebx ecx
	and	ebx,0x0000FFFF
	add	ebx,OLEFT*65536+10
	add	esi,[cur_line_len];LINE_WIDTH
	dec	ecx
	jg	.next_line

;--------------------------------------------------------------------------
  .draw_cursor:
	mov	eax,[posy]
	sub	eax,[top_line]
	js	@f
	mov	ecx,[slines]
	sub	ecx,eax
	jle	@f
	imul	eax,10
	add	eax,[top_ofs]
	inc	eax
	shl	eax,16
	add	eax,10-1
	mov	ecx,eax
	mov	ebx,[posx]
	sub	ebx,[left_col]
	js	@f
	cmp	ebx,[scolumns]
	ja	@f
	imul	ebx,6
	add	ebx,OLEFT-1
	shl	ebx,16
	inc	ebx
	inc	ebx
	mov	edx,[color_tbl+4*5]
	not	edx
	and	edx,0x00FFFFFF
	mcall	13
    @@:
;--------------------------------------------------------------------------
	cmp	[do_not_draw],2
	je	.exit

	mov	ebx,[p_info.x_size]
	shl	ebx,16
	add	ebx,(-SCRLW-5+2)*65536+SCRLW-2
	mov	ecx,[top_ofs-2]
	mov	cx,SCRLW-1
	mcall	8,,,'UP',[sc.work_button]
	pushad
	push	0x18
	shr	ecx,16
	mov	bx,cx
	add	ebx,(SCRLW/2-3)*65536+SCRLW/2-4
	mcall	4,,[sc.work_button_text],esp,1
	add	esp,4
	popad
	mov	ecx,[bot_ofs]
	shl	ecx,16
	add	ecx,(-SCRLW*2-1)*65536+SCRLW-1
	mcall	,,,'DN'
	pushad
	push	0x19
	shr	ecx,16
	mov	bx,cx
	add	ebx,(SCRLW/2-3)*65536+SCRLW/2-4
	mcall	4,,[sc.work_button_text],esp,1
	add	esp,4
	popad
	sub	ebx,1*65536-2

	push	ebx
	mov	eax,[lines]
	mov	ebx,[slines]
	mov	ecx,[top_line]
	mov	edx,[bot_ofs]
	sub	edx,[top_ofs]
	add	edx,-(SCRLW*3+2)
	call	get_scroll_vars
	mov	[vscrl_top],eax
	mov	[vscrl_size],ebx
	pop	ebx

	mov	ecx,eax
	add	ecx,[top_ofs]
	shl	ecx,16
	mov	cx,word[vscrl_size]
	add	ecx,(SCRLW+1)*65536
	mcall	13,,,[sc.work_button]
	mov	ecx,[top_ofs-2]
	mov	cx,word[vscrl_top]
	add	ecx,(SCRLW+1)*65536
	mov	edx,[color_tbl+4*5]
	or	cx,cx
	jle	@f
	mcall
    @@:
	mov	ecx,[top_ofs]
	add	ecx,[vscrl_top]
	add	ecx,[vscrl_size]
	add	ecx,SCRLW+1
	mov	di,cx
	shl	ecx,16
	mov	cx,word[bot_ofs]
	sub	cx,di
	sub	cx,(SCRLW+1)*2
	jle	@f
	mcall
    @@:
;-------------------------------
@^
	pushad
	mov	eax,ebx
	shr	eax,16
	mov	bx,ax
	add	ebx,0x00010001
	mov	ecx,[top_ofs]
	add	ecx,[vscrl_top]
	add	ecx,SCRLW+1
	mov	eax,ecx
	shl	ecx,16
	add	eax,[vscrl_size]
	dec	eax
	add	ecx,eax
	mov	edx,[sc.work_button]
	add	edx,0x00202020
	mcall	38
	add	ebx,(SCRLW-2)*65536+(SCRLW-2)
	mov	edx,[sc.work_button]
	sub	edx,0x00202020
	mcall	38
	sub	ebx,(SCRLW-2)*65536
	mov	ax,cx
	shl	ecx,16
	mov	cx,ax
	mcall	38
	mov	eax,[vscrl_size-2]
	mov	ax,word[vscrl_size]
	sub	eax,0x00010001
	sub	ecx,eax
	mov	edx,[sc.work_button]
	add	edx,0x00202020
	mcall	38
	popad
^@
;-------------------------------
	mov	eax,ebx
	shr	eax,16
	add	bx,ax
	mov	ecx,[top_ofs-2]
	mov	cx,word[top_ofs]
	add	ecx,SCRLW*65536+SCRLW
	mcall	38,,,[sc.work_text]
	mov	ecx,[bot_ofs-2]
	mov	cx,word[bot_ofs]
	sub	ecx,(SCRLW*2+2)*65536+(SCRLW*2+2)
	mcall
	rol	ebx,16
	push	bx
	rol	ebx,16
	pop	bx
	mov	ecx,[top_ofs-2]
	mov	cx,word[bot_ofs]
	add	ecx,-2
	mcall
;--------------------------------------------------------------------------
	mov	ebx,5*65536+SCRLW-1
	mov	ecx,[bot_ofs]
	shl	ecx,16
	add	ecx,(-SCRLW)*65536+SCRLW-2
	mcall	8,,,'LT',[sc.work_button]
	pushad
	push	0x1B
	shr	ecx,16
	mov	bx,cx
	add	ebx,(SCRLW/2-3)*65536+SCRLW/2-4
	mcall	4,,[sc.work_button_text],esp,1
	add	esp,4
	popad
	mov	ebx,[p_info.x_size]
	shl	ebx,16
	add	ebx,(-SCRLW*2-5)*65536+SCRLW
	mcall	,,,'RT'
	pushad
	push	0x1A
	shr	ecx,16
	mov	bx,cx
	add	ebx,(SCRLW/2-2)*65536+SCRLW/2-4
	mcall	4,,[sc.work_button_text],esp,1
	add	esp,4
	popad
	inc	ecx

	push	ecx
	mov	eax,[columns]
	mov	ebx,[scolumns]
	mov	ecx,[left_col]
	mov	edx,[p_info.x_size]
	add	edx,-(SCRLW*3+10)
	call	get_scroll_vars
	mov	[hscrl_top],eax
	mov	[hscrl_size],ebx
	pop	ecx

	mov	ebx,eax
	add	ebx,OLEFT+SCRLW
	shl	ebx,16
	mov	bx,word[hscrl_size]
	mcall	13,,,[sc.work_button]
	mov	ebx,(OLEFT+SCRLW)*65536
	mov	bx,word[hscrl_top]
	mcall	,,,[color_tbl+4*5]
	mov	ebx,OLEFT+SCRLW-1
	add	ebx,[hscrl_top]
	add	ebx,[hscrl_size]
	mov	di,bx
	shl	ebx,16
	mov	bx,word[p_info.x_size]
	sub	bx,di
	sub	bx,SCRLW*2+6
	jle	@f
	mcall
    @@:
	mov	eax,ebx
	shr	eax,16
	add	bx,ax
	mov	ecx,[bot_ofs-2]
	mov	cx,word[bot_ofs]
	sub	ecx,SCRLW*65536+2
	mcall	38,<OLEFT+SCRLW-1,OLEFT+SCRLW-1>,,[sc.work_text]
	mov	ebx,[p_info.x_size-2]
	mov	bx,word[p_info.x_size]
	sub	ebx,(SCRLW*2+6)*65536+(SCRLW*2+6)
	mcall
	mov	ebx,[p_info.x_size]
	add	ebx,5*65536-5
	mov	ecx,[bot_ofs-2]
	mov	cx,word[bot_ofs]
	sub	ecx,(SCRLW+1)*65536+(SCRLW+1)
	mcall
;--------------------------------------------------------------------------

.exit:
	popad
	ret
endf

;--------------------------------------------
func get_scroll_vars
;--------------------------------------------
; Input:
;  EAX = maximum data size      (units)
;  EBX = visible data size      (units)
;  ECX = current data position  (units)
;  EDX = scrolling area size    (pixels)
; Output:
;  EAX = srcoller offset        (pixels)
;  EBX = scroller size          (pixels)
;--------------------------------------------
	push	eax ebx ecx edx
;       sub     eax,ebx
	mov	esi,eax
	mov	eax,edx
	mul	ebx
	idiv	esi
	cmp	eax,[esp]
	jae	.null
	cmp	eax,AMINS
	jae	@f
	neg	eax
	add	eax,AMINS
	sub	[esp],eax
	mov	eax,AMINS
    @@: mov	[esp+8],eax	; scroller size
	mov	eax,[esp]
	mul	ecx
	idiv	esi
	mov	[esp+12],eax	; scroller offset
	add	eax,[esp+8]
	cmp	eax,[esp]
	jbe	@f
	dec	dword[esp+12]
    @@:
	pop	edx ecx ebx eax
	ret
  .null:
	mov	dword[esp+8],0
	mov	dword[esp+12],0
	jmp	@b
endf

;--------------------------------------------
func get_next_part
;--------------------------------------------
; Input:
;  ECX = current letter
;  ESI = string
; Output:
;  ECX = color
;  EDX = string
;  ESI = length
;--------------------------------------------
	cmp	[asm_mode],0
	je	.plain.text
	xor	ebx,ebx
	mov	edx,ecx
	add	esi,ecx
	mov	edi,symbols
	mov	al,[esi]
	cmp	al,';'
	je	.comment
	mov	ecx,symbols.size
	repne	scasb
	je	.symbol
	cmp	al,'$'
	jne	@f
	mov	edi,symbols
	mov	al,[esi+1]
	mov	ecx,symbols.size
	repne	scasb
	je	.not_symbol
	jmp	.number
    @@: cmp	al,'0'
	jb	@f
	cmp	al,'9'
	jbe	.number
    @@: cmp	al,"'"
	je	.string
	cmp	al,'"'
	je	.string
  .not_symbol:
	inc	ebx
	inc	edx
	cmp	edx,[cur_line_len];LINE_WIDTH
	jge	@f
	mov	edi,symbols
	mov	al,[esi+ebx]
	cmp	al,';'
	je	@f
	mov	ecx,symbols.size
	repne	scasb
	jne	.not_symbol
    @@: mov	ecx,edx
	mov	edx,esi
	mov	esi,ebx
	mov	eax,[color_tbl+4*0]
	ret
  .symbol:
	inc	ebx
	inc	edx
	cmp	edx,[cur_line_len];LINE_WIDTH
	jge	@f
	mov	edi,symbols
	mov	al,[esi+ebx]
	mov	ecx,symbols.size
	repne	scasb
	je	.symbol
    @@: mov	ecx,edx
	mov	edx,esi
	mov	esi,ebx
	mov	eax,[color_tbl+4*4]
	ret
  .comment:
	neg	edx
	add	edx,[cur_line_len];LINE_WIDTH
	xchg	edx,esi
	mov	ecx,[cur_line_len];LINE_WIDTH
	mov	eax,[color_tbl+4*3]
	ret
  .number:
	inc	ebx
	inc	edx
	cmp	edx,[cur_line_len];LINE_WIDTH
	jge	@f
	mov	edi,symbols
	mov	al,[esi+ebx]
	cmp	al,';'
	je	@f
	mov	ecx,symbols.size
	repne	scasb
	jne	.number
    @@: mov	ecx,edx
	mov	edx,esi
	mov	esi,ebx
	mov	eax,[color_tbl+4*1]
	ret
  .string:
	inc	ebx
	inc	edx
	cmp	edx,[cur_line_len];LINE_WIDTH
	jge	@f
	cmp	[esi+ebx],al
	jne	.string
	inc	ebx
	inc	edx
    @@:
	mov	ecx,edx
	mov	edx,esi
	mov	esi,ebx
	mov	eax,[color_tbl+4*2]
	ret
  .plain.text:
	mov	edx,[cur_line_len];LINE_WIDTH
	xchg	edx,esi
	mov	ecx,[cur_line_len];LINE_WIDTH
	mov	eax,[color_tbl+4*0]
	ret
endf

; ********************************************
; ****************  SAVEFILE  ****************
; ********************************************
func save_file
	mov	esi,0x80000	; 0x70000 = 448 Kbytes (maximum)
	mov	edi,0x10000

  .new_string:
	call	save_string
	cmp	dword[esi],0
	jne	.new_string
	sub	edi,0x10000+2	; minus last CRLF
;!      mov     [filelen],edi
	cmp	byte[s_fname],'/'
	je	.systree_save
	mcall	33,s_fname,0x10000,edi,0;[filelen],0
	test	eax,eax
	je	.exit
	call	file_not_found
	jmp	.exit

  .systree_save:
;!      mov     eax,[filelen]
	mov	[f_info+8],edi ;! eax
	mov	[f_info+0],1
	mov	esi,s_fname
	mov	edi,f_info.path
	mov	ecx,PATHL
	cld
	rep	movsb
	mcall	58,f_info

  .exit:
	ret
endf

func save_string
	lodsd
	mov	ecx,eax

    @@: cmp	byte[esi+ecx-1],' '
	jne	@f
	loop	@b
    @@: jecxz	.endcopy
	xor	edx,edx
	mov	ebx,edx
	mov	ah,dl

  .next_char:
	mov	al,[esi+ebx]
	inc	ebx
	test	ah,00000001b
	jnz	.char
	cmp	al,'"'
	jne	@f
	xor	ah,00000100b
	jmp	.char
    @@: cmp	al,"'"
	jne	@f
	xor	ah,00000010b
	jmp	.char
    @@: test	ah,00000110b
	jnz	.char
	cmp	al,';'
	jne	@f
	test	ah,00000001b
	jnz	.char
	xor	ah,00000001b
	jmp	.char
    @@: cmp	al,' '
	jne	.char
	inc	edx
	test	ebx,ATABW-1
	jnz	@f
	dec	edx
	jle	.put
	mov	al,9
	xor	edx,edx
	jmp	.put
  .char:
	or	edx,edx
	jz	.put
	push	ecx eax
	mov	ecx,edx
	mov	al,' '
	rep	stosb
	pop	eax ecx
	xor	edx,edx
  .put:
	stosb
    @@: loop	.next_char

  .endcopy:
	mov	eax,0x0A0D
	stosw
	add	esi,[esi-4]
	ret
endf

; ********************************************
; ****************  LOADFILE  ****************
; ********************************************

func loadhdfile
	mov	[f_info+0],0
	mov	[f_info+8],300000/512
	mov	esi,s_fname
	mov	edi,f_info.path;pathfile_read
	mov	ecx,PATHL
	cld
	rep	movsb
	mcall	58,f_info ; fileinfo_read
	xchg	eax,ebx
	inc	eax
	test	ebx,ebx
	je	file_found
	cmp	ebx,6		 ;// ATV driver fix (6 instead of 5)
	je	file_found
	call	file_not_found
	ret
endf

func loadfile
	mcall	6,s_fname,0,16800,0x10000 ; 6 = open file
	inc	eax	     ; eax = -1 -> file not found
	jnz	file_found
	call	file_not_found
	ret

  file_found:
	dec	eax
	mov	[filesize],eax
	mov	[lines],1
	mov	[columns],0
	mov	esi,0x10000
	mov	edi,0x80000
	mov	edx,eax

  .next_line:
	mov	ebx,edi
	add	edi,4
  .next_char:
	or	edx,edx
	jle	.exit
	lodsb
	dec	edx
	cmp	al,13
	je	.CR
	cmp	al,10
	je	.LF
	cmp	al,9
	je	.TB
	cmp	al,0
	je	.exit
	stosb
	jmp	.next_char

  .exit:
	mov	ecx,10
	mov	al,' '
	rep	stosb
	lea	eax,[edi-4]
	sub	eax,ebx
	mov	[ebx],eax
	mov	dword[ebx+eax+4],0
	sub	eax,10
	jnz	@f
	inc	eax
    @@: cmp	eax,[columns]
	jbe	@f
	mov	[columns],eax
    @@: ret

  .CR:	cmp	byte[esi],10
	jne	.LF
	lodsb
	dec	edx
  .LF:	mov	ecx,10
	mov	al,' '
	rep	stosb
	lea	eax,[edi-4]
	sub	eax,ebx
	mov	[ebx],eax
	inc	[lines]
	add	eax,-10
	cmp	eax,[columns]
	jbe	.next_line
	mov	[columns],eax
	jmp	.next_line

  .TB:	lea	eax,[edi-4]
	sub	eax,ebx
	mov	ecx,eax
	add	ecx,ATABW
	and	ecx,not(ATABW-1)
	sub	ecx,eax
	mov	al,' '
	rep	stosb
	jmp	.next_char

  file_not_found:
	mcall	55,eax,error_beep   ; beep
	mov	[lines],1	    ; open empty document
	mov	[columns],1
	xor	eax,eax
	mov	[top_line],eax
	mov	[posx],eax
	mov	[posy],eax
	mov	edi,0x80000+4
	mov	ecx,10
	mov	[edi-4],ecx
	mov	[edi+10],eax
	mov	al,' '
	cld
	rep	stosb
	ret
endf

; *****************************
; ******  WRITE POSITION ******
; *****************************

func writepos
	cmp	[do_not_draw],1  ; return if drawing is not permitted
	jae	.exit
	pusha
	mov	eax,[posx]
	inc	eax
	mov	ebx,5
	mov	ecx,10
	mov	edi,htext2.pos1
	cld
	call	uint2strz
	mov	eax,[posy]
	inc	eax
	mov	ebx,5
	mov	edi,htext2.pos2
	call	uint2strz
	mov	eax,[lines]	; number of lines
	mov	ebx,5
	mov	edi,htext2.pos3
	call	uint2strz
	mov	ebx,5*65536+htext2.size*6
	mov	ecx,[bot_ofs]
	shl	ecx,16
	add	ecx,ABTNH
	mcall	13,,,[sc.work_graph] ; draw bar
	mov	ebx,12*65536
	mov	bx,word[bot_ofs]
	add	ebx,(ABTNH+2)/2-4
	mcall	4,,[sc.work_button_text],htext2,htext2.size ; write position
	popa

  .exit:
	ret
endf

;-----------------------------------------------------------------------------
func uint2strz ;//////////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	dec	ebx
	jz	@f
	xor	edx,edx
	div	ecx
	push	edx
	call	uint2strz
	pop	eax
    @@: cmp	al,10
	sbb	al,$69
	das
	stosb
	ret
endf

; ****************************
; ******* READ STRING ********
; ****************************

func read_string
	cmp	al,5
	jz	.f1
	cmp	al,51
	jz	.f2
	ret

  .f1:	mov	[addr],s_fname
	mov	eax,[bot_ofs]
	add	eax,ABTNH+2
	mov	[ya],eax
	push	0		; case insensitive
	jmp	.rk
  .f2:	mov	[addr],s_search
	mov	eax,[bot_ofs]
	add	eax,(ABTNH+2)*2
	mov	[ya],eax
	push	1		; case sensitive
  .rk:
	mov	edi,[addr]
	mov	ecx,PATHL
	sub	ecx,[edi-4]
	add	edi,[edi-4]
	mov	al,$1C
	cld
	rep	stosb

	mov	edi,[addr]
	mov	eax,[edi-4]
	mov	[temp],eax
	add	edi,eax
	call	print_text
	mcall	40,00000111b

  .waitev:

	mcall	10
	cmp	eax,2
	jne	.read_done
	mcall	;2
	shr	eax,8
	cmp	al,13
	je	.read_done
	cmp	al,8
	jne	.nobs
	cmp	edi,[addr]
	je	.waitev
	dec	[temp]
	mov	eax,[addr]
	dec	dword[eax-4]
	sub	edi,1
	mov	byte[edi],$1C
	call	print_text
	jmp	.waitev
  .nobs:
	movzx	ebx,al
	sub	ebx,$20
	jle	.waitev
	cmp	byte[esp],0
	jne	.keyok
	sub	ebx,$40
	jl	.keyok
	add	al,[ebx+add_table]
  .keyok:
	mov	ecx,[addr]
	add	ecx,PATHL
	cmp	edi,ecx
	jae	.waitev
	mov	[edi],al
	inc	[temp]
	mov	eax,[addr]
	inc	dword[eax-4]

	call	print_text
	inc	edi
	jmp	.waitev

  .read_done:
	add	esp,4
	mov	ecx,PATHL
	sub	ecx,[temp]
	mov	edi,[addr]
	add	edi,[temp]
	mov	al,' '
	cld
	rep	stosb
	mov	[temp],987
	call	print_text
	mcall	40,00100111b
	ret
endf

func print_text
	pusha
	mov	ebx,(LBTNW+5+2)*65536
	mov	bx,word[p_info.x_size]
	sub	bx,LBTNW+RBTNW+10+3
	mov	ecx,[ya-2]
	mov	cx,ABTNH+1
	mcall	13,,,[sc.work]
	mov	ebx,(LBTNW+5+2+4)*65536+ABTNH/2-3
	add	ebx,[ya]
	mov	eax,[p_info.x_size]
	sub	eax,LBTNW+RBTNW+10+8
	push	eax
	cdq
	mov	ecx,6
	div	ecx
	cmp	eax,PATHL
	jbe	@f
	mov	eax,PATHL
    @@: mov	esi,eax
	mcall	4,,[color_tbl+0],[addr]

	mov	eax,[ya]
	mov	ebx,eax
	add	eax,ABTNH/2-6
	shl	eax,16
	add	eax,ebx
	add	eax,ABTNH/2-6+11
	mov	ecx,eax
	imul	eax,[temp],6
	pop	ebx
	cmp	eax,ebx
	jae	@f
	add	eax,LBTNW+5+2+4
	mov	ebx,eax
	shl	eax,16
	add	ebx,eax
	mcall	38,,,[color_tbl+0]

    @@: popa
	ret
endf

include 'helpwnd.asm'

;-----------------------------------------------------------------------------
section @DATA ;///////////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------

addr	   dd s_fname  ; address of input string
temp	   dd 0xabc	; used in read_string
vscrl_capt dd -1
hscrl_capt dd -1

label color_tbl dword
   dd 0x00000000 ;0x00FFFF00 ; text
   dd 0x00009000 ;0x0000FF00 ; numbers
   dd 0x00A00000 ;0x0000FFFF ; strings
   dd 0x00909090 ;0x00C0C0C0 ; comments
   dd 0x003030f0 ;0x00FFFFFF ; symbols
   dd 0x00E0E0E0 ;0x00000080 ; background

add_table:
; times $61 db -$00
  times $1A db -$20
  times $25 db -$00
  times $10 db -$20
  times $30 db -$00
  times $10 db -$50
  times $04 db -$00,-$01
  times $08 db -$00

error_beep	db 0xA0,0x30,0

sz s_example,'README.TXT'
sz s_still  ,'still'

;sz param_setup,<'LANG',0> ; parameter for SETUP
param_setup db 'LANG',0

sz fasm_filename   ,'FASM       '
sz debug_filename  ,'BOARD      '
sz tinypad_filename,'TINYPAD    '
sz setup	   ,'SETUP      ' ; to change keyboard layout

lsz sysfuncs_filename,\
  ru,<'SYSFUNCR.TXT',0>,\
  en,<'SYSFUNCS.TXT',0>

sz htext,'TINYPAD'
sz toolbar_btn_text,'COMPILE    RUN     BOARD   SYSFUNC'

lsz lstr,\
  ru,<'   ФАЙЛ > ',' СТРОКА > '>,\
  en,<'   FILE > ',' STRING > '>
lsz rstr,\
  ru,<' СОХРАНИТЬ ',' ЗАГРУЗИТЬ ',' ПОИСК     '>,\
  en,<' SAVE   '   ,' LOAD   '	 ,' SEARCH '   >

lsz htext2,\
  ru,'ПОЗИЦИЯ 00000:00000   ДЛИНА 00000 СТРОК',\
  en,'POSITION 00000:00000   LENGTH 00000 LINES'
if lang eq ru
 htext2.pos1 = htext2+8
 htext2.pos2 = htext2+14
 htext2.pos3 = htext2+28
else
 htext2.pos1 = htext2+9
 htext2.pos2 = htext2+15
 htext2.pos3 = htext2+30
end if

lszc help_text,b,\
  ru,'КОМАНДЫ:',\
  ru,' ',\
  ru,'  CTRL+F1 : Это окно',\
  ru,'  CTRL+S  : Первая строка для копирования',\
  ru,'  CTRL+E  : Последняя строка для копирования',\
  ru,'  CTRL+P  : Вставить выбранное на текущую позицию',\
  ru,'  CTRL+D  : Удалить строку',\
  ru,'  CTRL+L  : Вставить строку-разделитель',\
  ru,'  CTRL+[  : Перейти в начало файла',\
  ru,'  CTRL+]  : Перейти в конец файла',\
  ru,'  CTRL+F2 : Загрузить файл',\
  ru,'  CTRL+F3 : Поиск',\
  ru,'  CTRL+F4 : Сохранить файл',\
  ru,'  CTRL+F5 : Ввести имя файла',\
  ru,'  CTRL+F6 : Ввести строку для поиска',\
  ru,'  CTRL+F8 : Сменить раскладку клавиатуры',\
  ru,'',\
  en,'COMMANDS:',\
  en,' ',\
  en,'  CTRL+F1 : SHOW THIS WINDOW',\
  en,'  CTRL+S  : SELECT FIRST STRING TO COPY',\
  en,'  CTRL+E  : SELECT LAST STRING TO COPY',\
  en,'  CTRL+P  : PASTE SELECTED TO CURRENT POSITION',\
  en,'  CTRL+D  : DELETE CURRENT LINE',\
  en,'  CTRL+L  : INSERT SEPARATOR LINE',\
  en,'  CTRL+[  : GO TO THE BEGINNING OF FILE',\
  en,'  CTRL+]  : GO TO THE END OF FILE',\
  en,'  CTRL+F2 : LOAD FILE',\
  en,'  CTRL+F3 : SEARCH',\
  en,'  CTRL+F4 : SAVE FILE',\
  en,'  CTRL+F5 : ENTER FILENAME',\
  en,'  CTRL+F6 : ENTER SEARCH STRING',\
  en,'  CTRL+F8 : CHANGE KEYBOARD LAYOUT',\
  en,''

lsz help_title,\
  ru,'ПОМОЩЬ',\
  en,'TINYPAD HELP'

sz symbols,'#&*\:/<>|{}()[]=+-, '; %.'

align 4
label f_info dword
    dd	?
    dd	0
    dd	?
    dd	0x10000
    dd	0x70000
.path:
    times PATHL+1 db ?

TINYPAD_END:	 ; end of file

;-----------------------------------------------------------------------------
section @UDATA ;//////////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------

posx	      dd ?    ; global X position (text cursor)
posy	      dd ?    ; global Y position (text cursor)
lines	      dd ?    ; number of lines in file
slines	      dd ?    ; number of lines on the screen
columns       dd ?    ; number of columns in file
scolumns      dd ?    ; number of columns on the screen
top_ofs       dd ?    ; height occupied by top buttons
bot_ofs       dd ?    ; height occupied by bottom buttons
top_line      dd ?    ; topmost visible line on screen
left_col      dd ?    ; leftmost visible char on line
vscrl_top     dd ?
vscrl_size    dd ?
hscrl_top     dd ?
hscrl_size    dd ?
skinh	      dd ?    ; skin height
__rc	      dd ?,?,?,?
;filelen       dd ?    ; file size (on save) ???
filesize      dd ?    ; file size (on load) ???
ya	      dd ?    ; for read_string
copy_start    dd ?    ; first line for copying (Ctrl+S)
copy_count    dd ?    ; number of lines for copying (Ctrl+E)
copy_size     dd ?    ; size of data to copy
s_title.size  dd ?    ; caption length

cur_line_len  dd ?

asm_mode      db ?    ; ASM highlight?
do_not_draw   db ?    ; draw top and bottom buttons?
main_closed   db ?    ; main window closed? (to close help window)

align 4
s_fname.size  dd ?
s_fname       rb PATHL+1
align 4
s_search.size dd ?
s_search      rb PATHL+1

s_title       rb 256  ; window caption

;-----------------------------------------------------------------------------
section @PARAMS ;/////////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------

fasm_parameters:

p_info process_information
sc     system_colors
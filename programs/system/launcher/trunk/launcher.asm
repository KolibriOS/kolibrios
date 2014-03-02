;-----------------------------------------------------------------------------
;
;   LAUNCHER - startup of programs
;
;   Compile with FASM 1.52 or newer
;
;-----------------------------------------------------------------------------
; last update:  02/03/2014
; changed by:   Marat Zakiyanov aka Mario79, aka Mario
; changes:      Reducing the consumption of RAM, 4 KB instead of 32 KB.
;               Output to BOARD information about running applications.
;               Cleaning of the source code.
; notice:       Only 2 KB of memory for AUTORUN.DAT - be careful!
;-----------------------------------------------------------------------------
	use32
	org 0x0
	db 'MENUET01'	; 8 byte id
	dd 0x01		; header version
	dd START	; start of code
	dd IM_END	; size of image
	dd I_END	; memory for app
	dd stack_top	; esp
	dd 0x0		; I_Param
	dd 0x0		; I_Icon
;-----------------------------------------------------------------------------
include "../../../macros.inc"

define __DEBUG__ 1
define __DEBUG_LEVEL__ 1
include "../../../debug-fdo.inc"
;-----------------------------------------------------------------------------
START:                           ; start of execution
	mcall	70,autorun_dat_info	; load AUTORUN.DAT
	add	ebx,file_data
	mov	[fileend],ebx
;-----------------------------------------------------------------------------
; this cycle does not contain an obvious exit condition,
; but auxiliary procedures (like "get_string") will exit
; at EOF
start_program:
	call	skip_spaces
	cmp	al,byte '#'
	jz	skip_this_string
	call	clear_strings
	mov	edi,program
	call	get_string
	mov	edi,parameters
	call	get_string
	call	get_number
	call	run_program
;--------------------------------------
skip_this_string:
	call	next_line
	jmp	start_program
;-----------------------------------------------------------------------------
exit:
	or	eax,-1
	mcall
;-----------------------------------------------------------------------------
run_program:     ; time to delay in eax
	DEBUGF 1, "Launch: %s Parameter: %s\n",program,parameters
	push	eax
	mcall	70,start_info
	pop	ebx
; if delay is negative, wait for termination
;   of the spawned process
	test	ebx,ebx
	js	must_wait_for_termination
; otherwise, simply wait
	mcall	5
	ret
;-----------------------------------------------------------------------------
must_wait_for_termination:
	mov	esi,eax  ; save slot for the future
; get process slot
	mov	ecx,eax
	mcall	18,21
; if an error has occured, exit
	test	eax,eax
	jz	child_exited

	mov	ecx,eax
;--------------------------------------
wait_for_termination:
	mcall	5,1
	mov	ebx, processinfo
	mcall	9
	cmp	[ebx+50],word 9 ; the slot was freed?
	jz	child_exited

	cmp	[ebx+30],dword esi ; the slot is still occupied by our child?
	jz	wait_for_termination
;--------------------------------------
child_exited:
	ret
;-----------------------------------------------------------------------------
clear_strings:   ; clears buffers
	pushad
	mov	ecx,60+1
	mov	edi,program
	xor	al,al
	rep	stosb
	mov	ecx,60+1
	mov	edi,parameters
	rep	stosb
	popad
	ret
;-----------------------------------------------------------------------------
get_string: ; pointer to destination buffer in edi
	pushad
	call	skip_spaces
	mov	esi,[position]
	add	esi,file_data
	cmp	[esi],byte '"'
	jz	.quoted
;--------------------------------------
.start:
	cmp	esi,[fileend]
	jae	exit

	lodsb
	cmp	al,byte ' '
	jbe	.finish

	stosb
	inc	dword [position]
	jmp	.start
;--------------------------------------
.finish:
	popad
	ret
;--------------------------------------
.quoted:
	inc	esi
	inc	dword [position]
;--------------------------------------
.quoted.start:
	cmp	esi,[fileend]
	jae	exit

	lodsb
	inc	dword [position]
	cmp	al,byte '"'
	je	.finish

	stosb
	jmp	.quoted.start
;-----------------------------------------------------------------------------
get_number:
	push	ebx esi
	call	skip_spaces
	mov	esi,[position]
	add	esi,file_data
	xor	eax,eax
	cmp	[esi],byte '-'
	jnz	@f

	inc	eax
	inc	esi
	inc	dword [position]
;--------------------------------------
@@:
	push	eax
	xor	eax,eax
	xor	ebx,ebx
;--------------------------------------
.start:
	cmp	esi,[fileend]
	jae	.finish

	lodsb
	sub	al,byte '0'
	cmp	al,9
	ja	.finish

	lea	ebx,[ebx*4+ebx]
	lea	ebx,[ebx*2+eax]
	inc	dword [position]
	jmp	.start
;--------------------------------------
.finish:
	pop	eax
	dec	eax
	jnz	@f

	neg	ebx
;--------------------------------------
@@:
	mov	eax,ebx
	pop	esi ebx
	ret
;-----------------------------------------------------------------------------
skip_spaces:
	push	esi
	xor	eax,eax
	mov	esi,[position]
	add	esi,file_data
;--------------------------------------
.start:
	cmp	esi,[fileend]
	jae	.finish

	lodsb
	cmp	al,byte ' '
	ja	.finish

	inc	dword [position]
	jmp	.start
;--------------------------------------
.finish:
	pop	esi
	ret
;-----------------------------------------------------------------------------
next_line:
	mov	esi,[position]
	add	esi,file_data
;--------------------------------------
.start:
	cmp	esi,[fileend]
	jae	exit

	lodsb
	cmp	al,13
	je	.finish

	cmp	al,10
	je	.finish

	inc	dword [position]
	jmp	.start
;--------------------------------------
.finish:
	inc	dword [position]
	cmp	esi,[fileend]
	jae	exit

	lodsb
	cmp	al,13
	je	.finish

	cmp	al,10
	je	.finish

	ret
;-----------------------------------------------------------------------------
; DATA:
;-----------------------------------------------------------------------------
include_debug_strings
;-----------------------------------------------------------------------------
autorun_dat_info:	; AUTORUN.DAT
	.mode		dd 0            ; read file
	.start_block	dd 0            ; block to read
			dd 0
	.blocks		dd 4*512       ; 2 Kb max for AUTORUN.DAT
	.address	dd file_data
	db "/SYS/SETTINGS/AUTORUN.DAT",0
;-----------------------------------------------------------------------------
start_info:
	.mode	dd 7
		dd 0
	.params	dd parameters
		dd 0
		dd 0
	.path:
;-----------------------------------------------------------------------------
IM_END:
;-----------------------------------------------------------------------------
align 4
processinfo:
;-----------------------------------------------------------------------------
program:
	rb 61	; 60 + [0] char
;-----------------------------------------------------------------------------
parameters:
	rb 61
;-----------------------------------------------------------------------------
	rb 1024-61*2
;-----------------------------------------------------------------------------
position:
	rd 1	; position in file
;-----------------------------------------------------------------------------
fileend:
	rd 1
;-----------------------------------------------------------------------------
align 4
file_data:
	rb 4*512 ; 2 Kb for AUTORUN.DAT
;-----------------------------------------------------------------------------
align 4
	rb 256
stack_top:
;-----------------------------------------------------------------------------
I_END:
;-----------------------------------------------------------------------------

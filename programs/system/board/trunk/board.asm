;
;   DEBUG BOARD for APPLICATIONS and KERNEL DEVELOPMENT
;
;   See f63
;
;   Compile with FASM for KolibriOS
;
WRITE_LOG    equ 1
P_LEN		 equ 11
include 'lang.inc'

   use32
   org	  0x0
   db	  'MENUET01'		  ; 8 byte id
   dd	  0x01			  ; header version
   dd	  START 		  ; start of code
   dd	  I_END 		  ; size of image
   dd	  mem			; memory for app
   dd	  mem			; esp
   dd	  filename , 0x0	  ; I_Param , I_Icon
include '../../../macros.inc'
include '../../../debug.inc'
purge newline
MAXSTRINGS = 16
TMP = 80*(MAXSTRINGS+1)
;------------------------------------------------------------------------------
START:				; start of execution

	call CheckUnique

	mov	edi,filename
	cmp	[edi],byte 0
	jnz	param
	mov	esi,default_filename
@@:
	lodsb
	stosb
	test	al,al
	jnz	@b
param:

; allow user to see messages written before start
;	mov	ecx,4096
;flush:
;	mcall	63,2
;	loop	flush

	mov	ecx,TMP
	xor	eax,eax
	mov	edi,[targ]
	rep	stosb

	mov	[tmp1],'x'
	mov	[tmp2],'x'

	mcall	14
	and	eax,0xffff0000
	sub	eax,399 shl 16
	add	eax,399
	mov	[xstart],eax
	mcall	48,3,sc,sizeof.system_colors

	mov	esi,filename
	call	CreateFile
;------------------------------------------------------------------------------
red:
	call draw_window
;------------------------------------------------------------------------------
still:
	cmp	[buffer_length],0
	je	@f
	call	write_buffer
@@:
	mcall	23,50		; wait here for event
	cmp	eax,1			; redraw request ?
	je	red

	cmp	eax,2			; key in buffer ?
	je	key

	cmp	eax,3			; button in buffer ?
	je	button

	mcall	63,2
	cmp	ebx,1
	jne	still

new_data:
	cmp	[buffer_length],255
	jne	@f
	call	write_buffer
@@:
	movzx	ebx,byte[buffer_length]
	mov	[ebx+tmp],al
	inc	[buffer_length]

	mov	ebp,[targ]
.no4:
	cmp	al,13
	jne	no13
	and	[ebp-8],dword 0
	jmp	new_check
;------------------------------------------
write_buffer:
	pusha
	mov	edx,tmp
	movzx	ecx,byte[buffer_length]	;1
	mov	esi,filename
.write_to_logfile:
	call	WriteToFile
	cmp	eax,5
	jne	@f
	mov	esi,filename
	mov	[filepos],0
	call	CreateFile
	jnc	.write_to_logfile
@@:
	movzx	eax,byte[buffer_length]
	add	[filepos],eax
	xor	eax,eax
	mov	[buffer_length],al
	popa
	ret
;------------------------------------------
no13:
	cmp	al,10
	jne	no10
	and	[ebp-8],dword 0
	inc	dword [ebp-4]
	cmp	[ebp-4],dword MAXSTRINGS
	jbe	.noypos
	mov	[ebp-4],dword MAXSTRINGS
	lea	esi,[ebp+80]
	mov	edi,ebp
	mov	ecx,80*(MAXSTRINGS)
	cld
	rep	movsb

	mov	esi,[ebp-4]
	imul	esi,80
	add	esi,[ebp-8]
	add	esi,ebp
	mov	ecx,80
	xor	al,al
	rep	stosb
.noypos:
	mov	[targ],text2
	and	[krnl_cnt],0
	jmp	new_check
;------------------------------------------
no10:
	cmp	ebp,text1
	je	add2
	mov	ecx,[krnl_cnt]
	cmp	al,[krnl_msg+ecx]
	jne	.noknl
	inc	[krnl_cnt]
	cmp	[krnl_cnt],4
	jne	new_check
	mov	[targ],text1
.noknl:
	mov	ebp,[targ]
	jecxz	.add
	push	eax
	mov	esi,krnl_msg
.l1:
	lodsb
	call	add_char
	loop	.l1
	pop	eax
.add:
	and	[krnl_cnt],0
add2:
	call	add_char

new_check:
	mcall	63,2
	cmp	ebx,1
	je	new_data
	call	draw_text
	jmp	still
;------------------------------------------------------------------------------
key:
	mcall	2
	cmp	ah,' '
	je	button.noclose
	jmp	still
;------------------------------------------------------------------------------
button:
	mcall	17	; get id
	cmp	ah,1		; button id=1 ?
	jne	.noclose
	or	eax,-1 		; close this program
	mcall
.noclose:
	xor	[vmode],1
	jmp	red
;------------------------------------------------------------------------------
add_char:
	push	esi
	mov	esi,[ebp-4]
	imul	esi,80
	add	esi,[ebp-8]
	mov	[ebp+esi],al
	inc	dword[ebp-8]
	cmp	dword[ebp-8],80
	jb	.ok
	mov	dword[ebp-8],79
.ok:
	pop	esi
	ret
	
;------------------------------------------------------------------------------
;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************
draw_window:
	mcall	12,1	; 1, start of draw
	mcall 48, 5     ; GetClientTop
    shr ebx, 16
	mov	ecx,ebx
	shl ecx,16
	add ecx,MAXSTRINGS*10+45	   ; [y start] *65536 + [y size]
	xor	eax,eax		     ; function 0 : define and draw window
;	mov	edx,[sc.work]		   ; color of work area RRGGBB,8->color gl
	mov	edx,0xffffff
	or	edx,0x14000000
	xor	esi,esi
	mcall	,[xstart],,,,title
	mov	ebx,296 shl 16+5*6
	mcall	8,,<5,12>,3,[sc.work]
	mov	edx,[vmode]
	lea	edx,[edx*4+duk]
	mcall	4,<300,8>,,,4
	call	draw_text
	mcall	12,2	; 2, end of draw
	ret
;------------------------------------------------------------------------------
draw_text:
	mov	ebx,15*65536+30	   ; draw info text with function 4
;	mov	ecx,[sc.work_text]
	xor	ecx,ecx
	or	ecx,0x40000000
;	mov	edi,[sc.work]
	mov	edi,0xffffff
	mov	edx,text1
	cmp	[vmode],0
	je	.kern
	mov	edx,text2
.kern:
	push	ebx ecx edx
	mcall	9,procinfo,-1
	mov	eax,[ebx+42]
	xor	edx,edx
	mov	ebx,6
	div	ebx
	pop	edx ecx ebx
	mov	esi,80
	cmp	eax,esi
	ja	@f
	mov	esi,eax
@@:
	cmp	esi,5
	ja	@f
	mov	esi,5
@@:
	sub	esi,4
	mov	eax,4
newline:
	mcall
	add	ebx,10
	add	edx,80
	cmp	[edx],byte 'x'
	jne	newline
	ret
;------------------------------------------------------------------------------
;********************************************
;*  input:  esi = pointer to the file name  *
;********************************************

CreateFile:
	pusha
	mov	dword [InfoStructure],2		; create file
	mov	dword [InfoStructure+4],0	; reserved
	mov	dword [InfoStructure+8],0	; reserved
	mov	dword [InfoStructure+12],0	; 0 bytes to write (just create)
	mov	dword [InfoStructure+16],0	; NULL data pointer (no data)
	mov	dword [InfoStructure+20],0	; reserved
	mov	dword [InfoStructure+21],esi	; pointer to the file name
	mcall	70, InfoStructure
	test	eax,eax
	jz	.out
	stc
.out:
	popa
	ret
;------------------------------------------------
;********************************************
;*  input:  esi = pointer to the file name  *
;*          edx = pointer to data buffer    *
;*          ecx = data length               *
;********************************************

WriteToFile:
	push	ebx
	mov	dword [InfoStructure],3		; write to file
	mov	eax,  [filepos]
	mov	dword [InfoStructure+4],eax	; lower position addr
	mov	dword [InfoStructure+8],0	; upper position addr (0 for FAT)
	mov	dword [InfoStructure+12],ecx	; number of bytes to write
	mov	dword [InfoStructure+16],edx	; pointer to data buffer
	mov	dword [InfoStructure+20],0	; reserved
	mov	dword [InfoStructure+21],esi	; pointer to the file name
	mcall	70, InfoStructure
	clc
	test	eax,eax
	jz	.out
	stc
.out:
	pop      ebx
	ret

;-------------------------------------------------
;********************************************
;*  input:  esi = pointer to string         *
;*          edi = pointer to string         *
;*          ecx = data length               *
;********************************************
StrCmp:
	repe cmpsb
	ja .a_greater_b
	jb .a_less_b
.equal:
	mov eax, 0
	jmp .end
.a_less_b:
	mov eax, 1
	jmp .end
.a_greater_b:
	mov eax, -1
.end:
	ret

;-------------------------------------------------
;********************************************
;*  input:  edi = pointer to string         *
;*          ecx = data length			    *
;********************************************
; 'a' - 'A' = 32 -> 'A'|32 = 'a'
ToLower:
	xor eax, eax
.cycle:
	or byte[edi+eax], 32
	inc eax
	loop .cycle
.end:
	ret


;-------------------------------------------------
CheckUnique:
;get info on current thread, save pid/tid
;look for another process with same name and different pid/tid
;if found, close self
;else continue normally

.get_thread_info:
	mov ebx, procinfo
	mov ecx, -1
	mcall 9
.get_pid:
; check_buffer
	mov [process_count], eax
	mov eax, [ebx+process_information.PID]
	mov [pid_tid], eax
	mov ecx, 2

.check_threads:
	cmp ecx, [process_count]
	ja .leave_check
	mov eax, 9
	mcall

.check_slot_free:
	cmp dword [ebx+process_information.slot_state], 9
	je .next_thread

.check_pid:
	mov eax, [pid_tid]
	cmp [ebx+process_information.PID], eax
	je .next_thread

.get_proc_name:
	lea edi, [ebx+process_information.process_name]
	push ecx
	mov ecx, my_name_size-1
.lower_case:
	call ToLower
	lea esi, [my_name]

	mov ecx, my_name_size
	call StrCmp

	pop ecx

	cmp eax, 0
	je .close_program

.next_thread:
	inc ecx
	jmp .check_threads

.close_program:
	mcall 18,3 ; restore and active window of previous thread
	mov eax, -1
	mcall

.leave_check:
	ret



;--------------------------------------------------
InfoStructure:
	dd 0x0	; subfunction number
	dd 0x0	; position in the file in bytes
	dd 0x0	; upper part of the position address
	dd 0x0	; number of bytes to read
	dd 0x0	; pointer to the buffer to write data
	db 0
	dd 0	; pointer to the filename

filepos	dd 0
default_filename db '/sys/boardlog.txt',0
;------------------------------------------------------------------------------
krnl_msg db 'K : '
duk db 'KernUser'

; DATA AREA

; 11,11 > 0,-1
; 5,11  > 0,-1
if lang eq ru
 title	db 'Доска отладки и сообщений',0
else if lang eq it
 title	db 'Notifiche e informazioni generiche per il debug',0
else if lang eq ge
 title	db 'Allgemeines debug- & nachrichtenboard',0
else
 title	db 'General debug & message board',0
end if
krnl_cnt	dd 0
vmode		dd 1
targ		dd text2

my_name 	   db 'board',0
my_name_size   = $-my_name
process_count  dd 0x0
pid_tid		   dd 0x0
;------------------------------------------------------------------------------
I_END:
;------------------------------------------------------------------------------
offs	dd ?
flag	rb 1
	rd 2
;x1pos	dd ?
;y1pos	dd ?
text1	rb 80*(MAXSTRINGS+1)
tmp1	db ?
	rd 2
;x2pos	dd ?
;y2pos	dd ?
text2	rb 80*(MAXSTRINGS+1)
tmp2	db ?
xstart	dd ?

sc system_colors

i_end:
buffer_length	rb 1
;------------------------------------------------------------------------------
tmp	rb	256
;------------------------------------------------------------------------------
filename	rb 256
;------------------------------------------------------------------------------
align 4
procinfo:
	rb 1024
;------------------------------------------------------------------------------
align 4
stackbuf	rb 2000h
;------------------------------------------------------------------------------
mem:
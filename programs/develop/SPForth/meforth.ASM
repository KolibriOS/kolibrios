; KolSPForth v12

;Memory Map
;
;0
;0x02C7D	;I_END
;0x05430	;CTOP

;0x3F800	;Data stack
;0x3FA00	;return stack
;0x3FC00	;User
;0x3FE00	;TIB
;0x30000	;FILE BUFF
;0x40000 (256K)

   ;; Version control

   VER             EQU     1                       ;major release version
   EXT             EQU     0                       ;minor extension

   ;; Constants

   TRUEE		EQU	-1			;true flag

   COMPO		EQU	040H			;lexicon compile only bit
   IMEDD		EQU	080H			;lexicon immediate bit
   MASKK		EQU	01FH			;lexicon bit mask

   CELLL		EQU	4			;size of a cell
   BASEE		EQU	10			;default radix
   VOCSS		EQU	8			;depth of vocabulary stack

   BKSPP		EQU	8			;back space
   LF		EQU	10			;line feed
   CRR		EQU	13			;carriage return
   ERR		EQU	27			;error escape
   TIC		EQU	39			;tick

   CALLL		EQU	0E8H			;CALL opcodes

   ROWH   EQU 13
   KEY_DELAY EQU 20
   FW_WIDTH equ 500
   FW_HEIGHT equ 352


   ;; Memory allocation

   EM		EQU	256*1024		;top of memory
   FILE_BS	EQU	64*1024			;file buff size
   US		EQU	128*CELLL		;user area size in cells
   RTS		EQU	1024*CELLL		;return stack/TIB size

   FILE_B	EQU	EM-FILE_BS		;terminal input buffer (TIB)
   TIBB		EQU	FILE_B-RTS		;terminal input buffer (TIB)
;   UPP		EQU	TIBB-US                 ;start of user area (UP0)
   RPP		EQU	UPP-RTS                 ;start of return stack (ESP0)
   SPP		EQU	RPP-RTS                 ;start of data stack (EBP0)

LastNFA = 0

LastCFA EQU INIT



macro AHEADER FLAG,ID,F_CFA {
	db	FLAG
	DD	F_CFA
	DD	LastNFA
The_Nfa = $
	DB	ID,0
LastNFA = The_Nfa
F_CFA:
}

macro cfa_AHEADER FLAG,ID,F_CFA {
	db	FLAG
	DD	cfa_#F_CFA
	DD	LastNFA
The_Nfa = $
	DB	ID,0
LastNFA = The_Nfa
cfa_#F_CFA:
}


;; Main entry points and COLD start data

 use32
 format binary as ""

 	org    0x0

 	db     'MENUET01'
 	dd     0x01
 	dd     ORIG         ; start of code
 	dd     I_END        ; size of image
MEMS: 	dd     EM       ; memory for app
 	dd     SPP          ; esp
	dd     params
	dd     cur_dir_path

 lang fix ru
 include 'MACROS.INC'
 include '..\..\proc32.inc'

align 4
proc strncmp stdcall, s1:dword, s2:dword, n:dword

	   push esi
	   push edi
	   mov ecx, [n]
	   test ecx, ecx         ; Max length is zero?
	   je .done

	   mov esi, [s1]         ; esi = string s1
	   mov edi, [s2]         ; edi = string s2
	   cld
.compare:
	   cmpsb                 ; Compare two bytes
	   jne .done
	   cmp byte [esi-1], 0   ; End of string?
	   je .done
	   dec ecx               ; Length limit reached?
	   jne .compare
.done:
	   seta al               ; al = (s1 > s2)
	   setb ah               ; ah = (s1 < s2)
	   sub al, ah
	   movsx eax, al         ; eax = (s1 > s2) - (s1 < s2), i.e. -1, 0, 1
	   pop edi
	   pop esi
	   ret
endp

align 4
proc GetPr stdcall, exp:dword, sz_name:dword
           mov edx, [exp]
.next:
           push edx
           stdcall strncmp, edx, [sz_name], 16
           pop edx
           test eax, eax
           jz .ok
           mov edx, [edx-4]
           test edx, edx
           jnz .next
	mov eax,edx
           ret
.ok:
           mov eax, [edx-8]
           ret
endp

AHEADER 0 ,'GETPR',cfa_GETPR
 JMP GetPr


   ORIG:

   	MOV	EBP,RPP			;initialize RP
   	CLD				;ESI gets incremented
   	finit
 	call draw_window
 	call  calc_lines
	XCHG	ESP,EBP
	CALL amain
BYE:	mcall -1
        

   ULAST:    ;      DD      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0


 draw_cursor:

 draw_window:
	pusha
	mcall 12,1
	mov   [fRed],ebx
	mcall 0, <0,FW_WIDTH>, <0,FW_HEIGHT>, 0x54000000
	; mcall 0,,,COL_WINDOW_BG, ,title ; define window
	mcall 71, 1, header

	mcall 9,pinfo,-1
	mov   eax,[pinfo.x_size]
	cdq
	sub   eax,20
	mov   ebx,6
	div   ebx
	mov   [pinfo.x_start],eax

	mov   eax,[pinfo.y_size]
	cdq
	sub   eax,40
	mov   ebx,ROWH
	div   ebx

	mov   [pinfo.y_start],eax
	call  erase_screen
	mcall 12, 2
	mcall 67,-1,-1,FW_WIDTH,FW_HEIGHT
	popa
	ret


macro GetSkinHeight
{
	mov  eax,48
	mov  ebx,4
	int 0x40
}

erase_screen:
	GetSkinHeight
	mov ecx,eax
	shl ecx,16
	add ecx,[pinfo.y_size]
	sub ecx,eax
	sub ecx,4		
	mov   ebx,[pinfo.x_size]
	add   ebx,5 shl 16 - 9
	mcall 13,,,0;xff
	ret


AHEADER 0,"CC_LINES",cfa_CC_LINES
 calc_lines:
	 cmp dword[UPP+10*4],0
	 je  .ex
 		pusha
 		mov  ebp,os_work
 		mov  al,0xd
 		mov  edi,screen_buf
; 		mov  esi,[cursor]
; 		mov  byte[esi],'_'
; 		inc  [cursor]
 	.again:
 		mov  [ebp],edi
 		mov  esi,[cursor]
 		sub  esi,edi
 		mov  ecx,[pinfo.x_start]
;   test ecx,ecx
;   jnz  .no0
;   inc  ecx
;  .no0:
 		cmp  ecx,esi
 		jbe  .ok
 		mov  ecx,esi
 	.ok:
 		repne scasb
 		jecxz .nocrlf
 		cmp  byte[edi],10
 		jne  .nocrlf
 		inc  edi
 	.nocrlf:
 		mov  ecx,edi
 		sub  ecx,[ebp]
 		add  ebp,8
 		mov  [ebp-4],ecx
 		cmp  edi,[cursor]
 		jb   .again
 		and  dword[ebp],0
 ;		dpd  esi
 		mov  eax,[pinfo.y_start]
 		shl  eax,3
 		sub  ebp,eax
 		cmp  ebp,os_work
 		jae  .ok2
 		mov  ebp,os_work
 	.ok2:
 	 cmp  ebp,[last_ebp]
   je   .ok3
   mov  [last_ebp],ebp
   call erase_screen
  .ok3:
 		mov  ebx,10 shl 16 +30
 ;		ud2
 	.newl:
 		mcall 4,,0xffffff,[ebp],[ebp+4]
 		add  ebp,8
 		add  ebx,ROWH
 		cmp  dword[ebp],0
 		jnz  .newl
		SUB   ebx,ROWH
		call set_cur
 		popa
 .ex:
 		ret


set_cur:
	MOV	ecx,EBX
	shl	ecx,16
	add	ecx,EBX
	MOV 	EAX,[ebp+4-8]
	add 	EAX,2
	imul	EAX,6
	mov 	EBX,EAX
	shl	ebx,16
	add 	EBX,EAX
	mov	[lastcur],ecx
	mov	[lastcur+4],ebx
	ret

 e_calc_lines:
	 cmp dword[UPP+10*4],0
	 je  e_.ex
 		pusha
 		mov  ebp,os_work
 		mov  al,0xd
 		mov  edi,screen_buf
; 		mov  esi,[cursor]
; 		mov  byte[esi],'_'
; 		inc  [cursor]
 	e_.again:
 		mov  [ebp],edi
 		mov  esi,[cursor]
 		sub  esi,edi
 		mov  ecx,[pinfo.x_start]
;   test ecx,ecx
;   jnz  .no0
;   inc  ecx
;  .no0:
 		cmp  ecx,esi
 		jbe  e_.ok
 		mov  ecx,esi
 	e_.ok:
 		repne scasb
 		jecxz e_.nocrlf
 		cmp  byte[edi],10
 		jne  e_.nocrlf
 		inc  edi
 	e_.nocrlf:
 		mov  ecx,edi
 		sub  ecx,[ebp]
 		add  ebp,8
 		mov  [ebp-4],ecx
 		cmp  edi,[cursor]
 		jb   e_.again
 		and  dword[ebp],0
 ;		dpd  esi
 		mov  eax,[pinfo.y_start]
 		shl  eax,3
 		sub  ebp,eax
 		cmp  ebp,os_work
 		jae  e_.ok2
 		mov  ebp,os_work
 	e_.ok2:
 	 cmp  ebp,[last_ebp]
   je   e_.ok3
   mov  [last_ebp],ebp
	cmp  byte[edi],10
	jne  e_.ok3

   call erase_screen
  e_.ok3:
 		mov  ebx,10 shl 16+30
 ;		ud2
 	e_.newl:
; 		mcall 4,,0xffffff,[ebp],[ebp+4]
 		add  ebp,8
 		add  ebx,ROWH
 		cmp  dword[ebp],0
 		jnz  e_.newl
		SUB   ebx,ROWH
 		mcall 4,,0x00ffffff,[ebp-8],[ebp+4-8]
		call set_cur
 		popa
 e_.ex:
 		ret


AHEADER 0,"?KEY",cfa_queKEY
	PUSH	EDI
	XCHG	EBP,ESP
	PUSH	EAX
       POP  EBX
       mov  eax,10
       test ebx,ebx
       jz   QRX0
       inc  eax
    QRX0:
   		XOR	ECX,ECX			;EBX=0 setup for false flag
       mcall
       cmp  eax,1
       jne  QRX_
       call draw_window
       call calc_lines
    QRX_:
       cmp  eax,3
       je  BYE
       cmp  eax,2
       jne QRX3

       mcall 2
   QRX1:		MOVZX	ECX,AH
   QRX2:		PUSH	ECX			;save character
   QRX_TRUE:
   		MOV	ECX,TRUEE		;true flag
   QRX3:		PUSH	ECX
	POP	eax
	XCHG	EBP,ESP
	POP	EDI
	ret

   ;   ?RX		( -- c T | F )
   ;		Return input character and true, or a false if no input.


AHEADER 0,"EMIT_N",cfa_EMIT_N

	PUSH	EDI
	XCHG	EBP,ESP
			;char in AL
   		CMP	AL,0FFH			;0FFH is interpreted as input
   		JNZ	TX2			;do NOT allow input
   		MOV	AL,32			;change to blank
   TX2:
   		mov  ebx,[cursor]
   		mov  [ebx],AL
   		inc  [cursor]

	POP	eax
	XCHG	EBP,ESP
	POP	EDI
RET


 ;; Basic I/O

cfa_AHEADER 0,'CL_CUR',CL_CUR
	PUSH	EAX
	mcall 38,[lastcur+4],[lastcur],0
	POP	EAX
	RET

	cfa_AHEADER 0,'DR_CUR',DR_CUR
	PUSH	EAX
	mcall 38,[lastcur+4],[lastcur],0x00FF00FF
	POP	EAX
	RET
	

COLOR_ORDER equ MENUETOS
include 'gif_lite.inc'

cfa_AHEADER 0,'READ_GIF',READ_GIF ;( gif_ptr, out_ptr -- result )
	push esi
	push edi
	push ebp
	mov  edi, eax
	mov  esi,[ebp]
	mov  eax,os_work
	call ReadGIF
	pop  ebp
	pop  edi
	pop  esi
	add  ebp,4
	RET

   ;===============================================================
 cursor  dd screen_buf
  fRed  dd 1
 last_ebp dd 0

 include 'amain.asm'
 
 header db   'Kolibri Forth v12.1',0
 params: rb 256
 cur_dir_path: rb 4096


 FINFO:
 	.mode dd 0
		  dd 0
 	.blk  dd 1
 	.buf  dd 0
 	.work dd os_work


lastcur dd 0,0

I_END:
 squote_buf rb 1024
 sys_v rd 6
 screen_buf:
; sc_end:
 rb 4096
 pinfo process_information
 os_work rb 16*1024

   CTOP            =     $   ;next available memory in code dictionary
   ;=============================================================
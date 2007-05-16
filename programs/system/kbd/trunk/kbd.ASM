;
; Kolibri Bus Disconnect
;   Test for bus disconnect
;
;   Compile with FASM for Menuet
;
;

include '..\..\..\macros.inc'

memsize = 1000h
               org 0
PARAMS  = memsize - 1024

appname equ 'Kolibri Bus Disconnect'
version equ ' 1.1 '
  
               use32              ; включить 32-битный режим ассемблера

               db     'MENUET01'              ; 8 byte id
               dd     0x01                    ; header version
               dd     START                   ; start of code
               dd     I_END                   ; size of image
               dd     memsize                 ; memory for app
               dd     memsize - 1024          ; esp
               dd     PARAMS , 0x0            ; I_Param , I_Icon



;---------------------------------------------------------------------
;---  НАЧАЛО ПРОГРАММЫ  ----------------------------------------------
;---------------------------------------------------------------------

START:
    cmp     [PARAMS], byte 0
    jne     check_parameters

no_params:
    call    find_north_bridg

drawwindow:	
      mov  eax,48
      mov  ebx,3
      mov  ecx,sc
      mov  edx,sizeof.system_colors
      mcall

      mov	eax, 12
	mov	ebx, 1 ; start redraw
	mcall

	mov	eax, 0 ; window
	mov	ebx, 100 shl 16 + 300
	mov	ecx, 100 shl 16 + 90
	mov   edx, [sc.work]
      or    edx, 0x13000000
	mov	edi, title
	mcall

	mov	eax, 4
	mov	ebx, 17 shl 16 + 30
      mov   ecx, [sc.work_text]
	mov	edx, msg_nb
	mov	esi, 14
	mcall
	
	mov	ebx, 105 shl 16 + 30
	mov	edx, [nb_name]
	movzx	esi, byte[edx]
	inc	edx
	mcall
	
	mov	ebx, 17 shl 16 + 40
	mov	edx, msg_stat
	mov	esi, 8
	mcall
	
	mov	ebx, 102 shl 16 + 50
	mov	edx, msg_divs
	mov	esi, 32
	mcall
	mov	ebx, 17 shl 16 + 62
	mov	edx, msg_hdd
	mov	esi, 10
	mcall
	mov	ebx, 17 shl 16 + 72
	mov	edx, msg_sgd
	mcall
	
	call	get_divs
	mov	eax, 47
	mov	ebx, 0x30000
	mov	ecx, [val_hdd]
	mov	edx, 8
	shl	edx, cl
	mov	ecx, edx
	mov	edx, 80 shl 16 + 62
	mov	esi, [sc.work_text]
	mcall
	
	mov	ecx, [val_sgd]
	mov	edx, 8
	shl	edx, cl
	mov	ecx, edx
	mov	edx, 80 shl 16 + 72
	mcall

	
	call	get_bd_stat
	mov	ecx, [sc.work_text]
	mov	esi, 9
	mov	ebx, 105 shl 16 + 40
	mov	edx, msg_nf
	mov	al, [bd_stat]
	test	al, al
	jz	@f
	mov	edx, msg_dis
	dec	al
	jz	@f
	mov	edx, msg_en
@@:	mov	eax, 4
	mcall

	; Создаём кнопку
	mov	ecx, 27 shl 16 + 20
	mov	eax, 8
	mov	ebx, 220 shl 16 + 70
	mov	esi, [sc.work_button]
	mov	edx, 2
	mcall
	
	; И ещё 14 ;)
	mov	edi, 7
	mov	ecx, 60 shl 16 + 10
	mov	eax, 8
	mov	ebx, 105 shl 16 + 25
	mov	edx, 3
	
@@:	mcall
	inc	edx
	add	ebx, 27 shl 16 
	dec	edi
	jnz	@b

	add	ecx, 12 shl 16 
	mov	ebx, 105 shl 16 + 25
	mov	edi, 7
@@:	mcall
	inc	edx
	add	ebx, 27 shl 16 
	dec	edi
	jnz	@b
	
end_dr:	mov	eax, 12
	mov	ebx, 2 ; end redraw
	mcall

; Wait for event ...
	mov	eax, 10
	mcall
	
	cmp	al, 3
	jne	not_bt
	
	mov	eax, 17			; get id
	mcall
	cmp	ah, 1
	jne	no_exit
	mov	eax, -1		; close this program
	mcall
no_exit:
	cmp	ah, 2
	jne	no_ch_bt
	mov	dl, [bd_stat]
	test	dl, dl
	jz	drawwindow
	xor	eax, eax
	dec	dl
	jnz	@f
	inc	eax
@@:	call	set_bd_stat
	jmp	drawwindow
	
no_ch_bt:
	cmp	ah, 9
	jg	no_hdd_bt
	sub	ah, 3
	movzx	esi, ah
	mov	edi, [val_sgd]
	call	set_divs
	jmp	drawwindow
no_hdd_bt:
	sub	ah, 10
	movzx	edi, ah
	mov	esi, [val_hdd]
	call	set_divs
	jmp	drawwindow

	
not_bt:	cmp	al, 2
	jne	drawwindow
	mov	eax, 2			; пока клава не поддерживается - читаем и забываем
	mcall
	jmp	drawwindow	
;--------------------------------------------------------------------------
bus_num:	db	2	; Номер шины
devfn:		db	255
bd_id:		dd	0	; Идентификатор устройства
bd_stat:	db	0	; 0 - не найден, 1 - выключен, 2 - включен

nb_name		dd	nb_nf
bd_msk		dd	msk_i440

nb_nf		db	9, 'Not found'
msk_i440	db	0
nb_i440		db	4, 'i440'
msk_nforce	db	0x6D, 0x80, 0xE7, 0x06, 0	; номер регистра, маска, номер регистра, маска, ... , 0
nb_nforce	db	6, 'nForce'
msk_nforce2	db	0x6F, 0x10, 0
nb_nforce2	db	8, 'nForce 2'
msk_sis730	db	0x6B, 0x01, 0
nb_sis730	db	7, 'SiS 730'
nb_sis733	db	7, 'SiS 733'
msk_sis735	db	0x6A, 0x03, 0
nb_sis735	db	7, 'SiS 735'
nb_sis740	db	7, 'SiS 740'
nb_sis741	db	7, 'SiS 741'
nb_sis745	db	7, 'SiS 745'
msk_sis746	db	0x6C, 0x01, 0
nb_sis746	db	7, 'SiS 746'
nb_sis748	db	7, 'SiS 748'
msk_amd751	db	0x62, 0x06, 0
nb_amd751	db	7, 'AMD 751'
nb_amd751s	db	8, 'AMD 751S'
nb_amd761	db	7, 'AMD 761'
msk_amd762	db	0x62, 0x02, 0x6A, 0x02, 0
nb_amd762	db	7, 'AMD 762'
msk_viakt133	db	0x52, 0x80, 0x70, 0x08, 0
nb_viakt133	db	30, 'VIA KT133(A)/KM133/KL133/KN133'
nb_viakx133	db	9, 'VIA KX133'
nb_viakle133	db	10, 'VIA KLE133'
msk_viakt266	db	0x92, 0x80, 0x95, 0x02, 0x70, 0x08, 0
nb_viakt266	db	18, 'VIA KT266(A)/KT333'
nb_viakm266	db	21, 'VIA KM266/KL266/KM333'
nb_viakn266	db	9, 'VIA KN266'
msk_viakt400	db	0xD2, 0x80, 0xD5, 0x02, 0x70, 0x08, 0
nb_viakt400	db	18, 'VIA KT400(A)/KT600'
nb_viakm400	db	9, 'VIA KM400'
msk_viakt880	db	0x82, 0x80, 0x85, 0x02, 0
nb_viakt880	db	9, 'VIA KT880'


bd_table:	dd	0x70061022		; AMD 751  ----
		dd	nb_amd751
		dd	msk_amd751
		
		dd	0x70041022		; AMD 751S
		dd	nb_amd751s
		dd	msk_amd751
		
		dd	0x700E1022		; AMD 761
		dd	nb_amd761
		dd	msk_amd751
		
		dd	0x700C1022		; AMD 762
		dd	nb_amd762
		dd	msk_amd762
		
		dd	0x71908086		; i440 ---
		dd	nb_i440
		dd	msk_i440
		
		dd	0x01A410DE		; nForce  ----
		dd	nb_nforce
		dd	msk_nforce
		
		dd	0x01E010DE		; nForce 2
		dd	nb_nforce2
		dd	msk_nforce2
		
		dd	0x07301039		; SiS 730  ----
		dd	nb_sis730
		dd	msk_sis730
		
		dd	0x07331039		; SiS 733
		dd	nb_sis733
		dd	msk_sis730
		
		dd	0x07351039		; SiS 735
		dd	nb_sis735
		dd	msk_sis735
		
		dd	0x07401039		; SiS 740
		dd	nb_sis740
		dd	msk_sis735
		
		dd	0x07411039		; SiS 741
		dd	nb_sis741
		dd	msk_sis735
		
		dd	0x07451039		; SiS 745
		dd	nb_sis745
		dd	msk_sis735
		
		dd	0x07461039		; SiS 746
		dd	nb_sis746
		dd	msk_sis746
		
		dd	0x07481039		; SiS 748
		dd	nb_sis748
		dd	msk_sis746
		
		dd	0x03051106		; VIA KT133(A)/KM133/KL133/KN133 ----
		dd	nb_viakt133
		dd	msk_viakt133
		
		dd	0x03911106		; VIA KX133
		dd	nb_viakx133
		dd	msk_viakt133
		
		dd	0x06911106		; VIA KLE133
		dd	nb_viakle133
		dd	msk_viakt133
		
		dd	0x30991106		; VIA KT266(A)/KT333
		dd	nb_viakt266
		dd	msk_viakt266
		
		dd	0x31161106		; VIA KM266/KL266/KM333
		dd	nb_viakm266
		dd	msk_viakt266
		
		dd	0x31561106		; VIA KN266
		dd	nb_viakn266
		dd	msk_viakt266
		
		dd	0x31891106		; VIA KT400(A)/KT600
		dd	nb_viakt400
		dd	msk_viakt400
		
		dd	0x32051106		; VIA KM400
		dd	nb_viakm400
		dd	msk_viakt400
		
		dd	0x22691106		; VIA KT880
		dd	nb_viakt880
		dd	msk_viakt880
bd_table_end:


find_north_bridg:
	mov	bl, 6
	xor	cl, cl
nbus:	mov	bh, [bus_num]
ndevfn:	mov	ch, [devfn]
	mov	eax, 62
	mcall
	cmp	eax, 0xffffffff
	je	bd_next
	;---------
	mov	esi, bd_table_end - bd_table - 12
@@:	cmp	eax, [bd_table + esi]
	je	bd_found
	test	esi, esi
	jz	bd_next
	sub	esi, 12
	jmp	@b
	;---------
bd_next:dec	byte[devfn]
	jns	ndevfn
	mov	byte[devfn], 0
	dec	byte[bus_num]
	jns	nbus
	ret
bd_found:
	add	esi, bd_table + 4
	mov	edi, nb_name
	mov	ecx, 2
	rep	movsd

	mov	[bd_id], eax
	ret

;----------------------
;bd_stat:	db	0	; 0 - не найден, 1 - выключен, 2 - включен
get_bd_stat:
	mov	byte[bd_stat], 1
	cld
	mov	esi, [bd_msk]
	lodsw
	test	al, al
	jnz	@f
	mov	byte[bd_stat], 0
	ret
@@:	push	eax
	mov	bh, [bus_num]
	mov	bl, 4
	mov	ch, [devfn]
	mov	cl, al
	mov	eax, 62
	mcall
	pop	edx
	and	al, dh
	jnz	@f
	lodsw
	test	al, al
	jnz	@b
	ret	
@@:	mov	byte[bd_stat], 2
	ret
;----------------------
set_bd_stat:
	cmp	dword[bd_id], 0x01E010DE	; специально для nForce2 400
	je	set_stat_nforce2
	
	mov	edi, eax
	cld
	mov	esi, [bd_msk]
bd_ss_nxt:
	lodsw
	test	al, al
	jz	bd_ss_end
	mov	dl, ah		; маска
	mov	bh, [bus_num]
	mov	bl, 4
	mov	ch, [devfn]
	mov	cl, al
	mov	eax, 62
	mcall
	mov	bl, 8
	test	edi, edi
	jz	@f
	or	al, dl
	mov	dl, al
	mov	eax, 62
	mcall
	jmp	bd_ss_nxt
@@:	not	dl
	and	al, dl
	mov	dl, al
	mov	eax, 62
	mcall	
	jmp	bd_ss_nxt
bd_ss_end:
	ret
;------- nForce 2 -----------
set_stat_nforce2:
; IN : eax = 0 - disable, !0 - enable
	push	eax
	mov	bh, [bus_num]
	mov	bl, 4
	mov	ch, [devfn]
	mov	cl, 0x6f
	mov	eax, 62
	mcall
	and	al, 0x1F
	mov	dl, al
	mov	bl, 8
	mov	eax, 62
	mcall
	pop	eax
	test	eax, eax
	jz	@f
	or	dl, 0x10
	mov	eax, 62
	mcall
	ret
@@:	and	dl, 0xef
	mov	eax, 62
	mcall
	ret
;--------------------------------------------------------------------------
;                        x8    x16   x32    x64  x128  x256  x512
div_hdd:	db	0x23, 0x27, 0x2B, 0x2F, 0x63, 0x67, 0x6B ; Halt Disconnect Divisor
div_sgd:	db	0x12, 0x52, 0x92, 0xD2, 0x12, 0x52, 0x92 ; Stop Grand Divisor
; low word of 0xC001001B MSR
; HDD\SGD     8     16     32     64       128    256    512
; 8        0x1223 0x5223 0x9223 0xD223 | 0x1223 0x5223 0x9223
; 16       0x1227                      |
; 32       0x122B                      |
; 64       0x122F                      |      0x522F
; 128      0x1263                      |
; 256      0x1267   & bit 18 is clear  |  & bit 18 is set
; 512      0x126B                      |
           ; ^^^^
           ; ||||_HDD
           ; ||_SGD
set_divs:
; IN : ESI - hdd (0 = x8, 1 = x16 ..)
;      EDI - sgd (0 = x8, 1 = x16 ..)
	mov	eax, 68
	mov	ebx, 3
	mov	edx, 0xC001001b
	mcall
	mov	al, [div_hdd + esi]
	mov	ah, [div_sgd + edi]
	and	eax, 0xFFFBFFFF
	cmp	edi, 3
	jle	@f
	or	eax, 0x40000
@@:	mov	edi, eax
	mov	esi, ebx
	mov	eax, 68
	mov	ebx, 4
	mcall
	ret

get_divs:
; OUT : val_hdd - hdd (0 = x8, 1 = x16 ..)
;       val_sgd - sgd ...
	mov	eax, 68
	mov	ebx, 3
	mov	edx, 0xC001001b
	mcall
	mov	ecx, 7
@@:	cmp	[div_hdd + ecx - 1], al
	je	@f
	loop	@b
@@:	dec	ecx
	mov	[val_hdd], ecx
	mov	ecx, 4
@@:	cmp	[div_sgd + ecx - 1], ah
	je	@f
	loop	@b
@@:	dec	ecx
	test	eax, 0x40000
	jz	@f
	add	ecx, 4
@@:	mov	[val_sgd], ecx
	ret

;******************************************************************************

check_parameters:
    cmp     [PARAMS], dword "BOOT" ; received BOOT parameter -> goto handler
    je      boot_bd_enable
    jmp     no_params

;******************************************************************************

boot_bd_enable:

   call    find_north_bridg
   call    set_bd_stat
   mcall   -1

;******************************************************************************


;--------------------------------------------------------------------------

title	db appname,version,0

msg_divs	db ' x8  x16  x32 x64 x128 x256 x512'
msg_hdd	db 'Hatl Disc.'
msg_sgd	db 'Stop Grand'
msg_nb	db 'North bridge :';14
msg_stat	db 'Status :'      ;8
msg_en	db 'Enabled  '     ;9
msg_dis	db 'Disabled '     ;9
msg_nf	db 'Not found'     ;9

I_END:
sc     system_colors
val_hdd:	dd	?
val_sgd:	dd	?


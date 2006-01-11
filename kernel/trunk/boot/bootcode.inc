;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                        ;;
;;  BOOTCODE.INC                                          ;;
;;                                                        ;;
;;  KolibriOS 16-bit loader,                              ;;
;;                        based on bootcode for MenuetOS  ;;
;;                                                        ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



;==========================================================================
;
;                           16 BIT FUNCTIONS
;
;==========================================================================

putchar:
; in: al=character
	mov	ah, 0Eh
	mov	bh, 0
	int	10h
	ret

print:
; in: si->string
	mov	al, 186
	call	putchar
	mov	al, ' '
	call	putchar

printplain:
; in: si->string
	pusha
	lodsb
@@:
	call	putchar
	lodsb
	cmp	al, 0
	jnz	@b
	popa
	ret

; Now int 16 is used for keyboard support.
; This is shorter, simpler and more reliable.
if 0
getkey:      push  ecx
             push  edx
             add   ebx,0x0101
             xor   eax,eax

           gk1:
             in    al,0x60
             mov   cl,al
           gk0:
             in    al,0x60
             cmp   al,cl
             je    gk0
             cmp   ax,11
             jg    gk0
             gk0_1:
             mov   cl,al

;             add   al,47
;             mov   [ds:keyinbs-0x10000],al
;             mov   si,keyinbs-0x10000
;             call  printplain

           gk12:
             in    al,0x60
             cmp   al,cl
             je    gk12
             cmp   ax,240
             jne   gk13
             mov   al,cl
             jmp   gk14
           gk13:
             add   cl,128
             cmp   al,cl
             jne   gk1
             sub   al,128
           gk14:

             movzx edx,bl
             cmp   eax,edx
             jb    gk1
             movzx edx,bh
             cmp   eax,edx
             jg    gk1
             test  ebx,0x010000
             jnz   gk3
             mov   cx,0x1000
             mov   dx,cx
             add   eax,47
             mov   cx,ax
             cmp   cx,58
             jb    gk_nozero
             sub   cx,10
           gk_nozero:
             mov   [ds:keyin-0x10000],cl
             mov   si,keyin-0x10000
             call  printplain
           gk3:
             sub   eax,48
             pop   edx
             pop   ecx
             ret
end if

getkey:
; get number in range [bl,bh] (bl,bh in ['0'..'9'])
; in: bx=range
; out: ax=digit (1..9, 10 for 0)
	mov	ah, 0
	int	16h
	cmp	al, bl
	jb	getkey
	cmp	al, bh
	ja	getkey
	push	ax
	call	putchar
	pop	ax
	and	ax, 0Fh
	jnz	@f
	mov	al, 10
@@:
	ret

setcursor:
; in: dl=column, dh=row
	mov	ah, 2
	mov	bh, 0
	int	10h
	ret

macro _setcursor row,column
{
	mov	dx, row*256 + column
	call	setcursor
}

; 16-bit data
; videomodes table
gr_table:
	dw      0x112+0100000000000000b ,  640 ,  480		; 1
	dw      0x115+0100000000000000b ,  800 ,  600		; 2
	dw      0x118+0100000000000000b , 1024 ,  768		; 3
	dw      0x11B+0100000000000000b , 1280 , 1024		; 4
	dw      0x112 ,  640 , 480				; 5
	dw      0x115 ,  800 , 600				; 6
	dw      0x118 , 1024 , 768				; 7
	dw      0x11B , 1280 ,1024				; 8
	dw	0x13, 320, 200					; 9
	dw	0x12, 640, 480					; 0

; table for move to extended memory (int 15h, ah=87h)
       movedesc:
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0

        db      0xff,0xff,0x0,0xa0,0x00,0x93,0x0,0x0
        db      0xff,0xff,0x0,0x00,0x10,0x93,0x0,0x0

        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0

;=========================================================================
;
;                           16 BIT CODE
;
;=========================================================================


start_of_code:
	cld
; \begin{diamond}[02.12.2005]
	cmp	ax, 'KL'
	jnz	@f
	mov	word [cs:cfgmanager.loader_block-0x10000], si
	mov	word [cs:cfgmanager.loader_block+2-0x10000], ds
@@:
; \end{diamond}[02.12.2005]

; set up stack
	mov	ax, 3000h
	mov	ss, ax
	mov	sp, 0EC00h
; set up segment registers
	push	cs
	pop	ds
	push	cs
	pop	es

; set videomode
	mov	ax, 3
        int  0x10

 ; Load & set russian VGA font (RU.INC)
        mov  bp,RU_FNT1-10000h   ; RU_FNT1 - First part
          mov  bx,1000h            ; 768 bytes
          mov  cx,30h              ; 48 symbols
          mov  dx,80h              ; 128 - position of first symbol
        mov  ax,1100h
          int  10h

        mov  bp,RU_FNT2-10000h   ; RU_FNT2 -Second part
        mov  bx,1000h            ; 512 bytes
        mov  cx,20h              ; 32 symbols
        mov  dx,0E0h             ; 224 - position of first symbol
        mov  ax,1100h
          int  10h
 ; End set VGA russian font

; draw frames
	push	0xb800
	pop	es
	xor	di, di
;        mov  si,d80x25-0x10000
;        mov  cx,80*25
;        mov  ah,1*16+15
;       dfl1:
;        lodsb
;        stosw
;        loop dfl1
	mov	ah, 1*16+15
; draw top
	mov	si, d80x25_top - 0x10000
	mov	cx, d80x25_top_num * 80
@@:
	lodsb
	stosw
	loop	@b
; draw spaces
	mov	si, space_msg - 0x10000
	mov	cx, 25 - d80x25_top_num - d80x25_bottom_num
dfl1:
	push	cx
	push	si
	mov	cx, 80
@@:
	lodsb
	stosw
	loop	@b
	pop	si
	pop	cx
	loop	dfl1
; draw bottom
	mov	si, d80x25_bottom - 0x10000
	mov	cx, d80x25_bottom_num * 80
@@:
	lodsb
	stosw
	loop	@b

	mov	byte [space_msg-0x10000+80], 0	; now space_msg is null terminated

	_setcursor d80x25_top_num,0


; TEST FOR 386+

	mov	bx, 0x4000
        pushf
        pop     ax
        mov     dx,ax
        xor     ax,bx
        push    ax
        popf
        pushf
        pop     ax
        and     ax,bx
        and     dx,bx
        cmp     ax,dx
        jnz     cpugood
        mov     si,not386-0x10000
        call    print
        jmp     $
     cpugood:

; set up esp
	movzx	esp, sp

; FLUSH 8042 KEYBOARD CONTROLLER

;// mike.dld [
 ;       mov     al,0xED
 ;       out     0x60,al
 ;       or      cx,-1
 ;     @@:
 ;       in      al,0x64
 ;       test    al,2
 ;       jz      @f
 ;       loop    @b
 ;     @@:
 ;       mov     al,0
 ;       out     0x60,al
 ;       or      cx,-1
 ;     @@:
 ;       in      al,0x64
 ;       test    al,2
 ;       jz      @f
 ;       loop    @b
 ;     @@:
;// mike.dld ]

;       mov     ecx,10000
;      fl1:
;       in      al,0x64
;       loop    fl1
;       test    al,1
;       jz      fl2
;       in      al,0x60
;       jmp     fl1
;      fl2:

;****************************************************************
; The function is modified Mario79
;*****************************************************************
; wait_kbd:        ; variant 1
;       mov      cx,2500h  ;задержка порядка 10 мсек
; test_kbd:
;       in       al,64h    ;читаем состояние клавиатуры
;       test     al,2      ;проверка бита готовности
;       loopnz   test_kbd

    mov   al,0xf6         ; Сброс клавиатуры, разрешить сканирование
    out   0x60,al
    xor   cx,cx
wait_loop:       ; variant 2
; reading state of port of 8042 controller
        in      al,64h 
        and     al,00000010b  ; ready flag
; wait until 8042 controller is ready 
        loopnz  wait_loop

; DISPLAY VESA INFORMATION

	push	0
	pop	es
        mov     ax,0x4f00
        mov     di,0xa000
        int     0x10
        cmp     ax,0x004f
        mov	si, novesa-0x10000
        jnz	@f
        mov     ax,[es:di+4]
        add     ax,'0'*256+'0'
        mov     [vervesa+vervesa_off-0x10000], ah
        mov     [vervesa+vervesa_off+2-0x10000], al
        mov     si,vervesa-0x10000
@@:     call    print

; \begin{diamond}[30.11.2005]
cfgmanager:
; settings:
; a) preboot_graph = graphical mode
;    preboot_gprobe = probe this mode?
; b) preboot_mtrr = use hardware acceleration?
; c) preboot_vrrm = use VRR?
; d) preboot_device = from what boot?
	mov	si, preboot_graph-0x10000
; check bootloader block
	cmp	[.loader_block-0x10000], 0
	jz	.noloaderblock
	les	bx, [.loader_block-0x10000]
	cmp	byte [es:bx], 1
	jz	@f
	mov	si, loader_block_error-0x10000
	call	print
	jmp	$
@@:
	test	byte [es:bx+1], 1
	jz	@f
; image in memory present
	cmp	[si+preboot_device-preboot_graph], 0
	jnz	@f
	mov	[si+preboot_device-preboot_graph], 3
@@:
.noloaderblock:
; determine default settings
	mov	[.bSettingsChanged-0x10000], 0
	cmp	byte [si], 0
	jnz	.preboot_gr_end
	mov	[si+preboot_gprobe-preboot_graph], 0
	mov	al, [vervesa+vervesa_off-0x10000]
	cmp	al, 'x'
	jz	.novesa
	cmp	al, '1'
	jz	.vesa12
	mov	[si+preboot_gprobe-preboot_graph], 2
	mov	al, 3
	jmp	@f
.vesa12:
	mov	al, 7
	jmp	@f
.novesa:
	mov	al, 10
@@:
	mov	[si], al
.preboot_gr_end:
	cmp	[si+preboot_mtrr-preboot_graph], 1
	adc	[si+preboot_mtrr-preboot_graph], 0
	cmp	[si+preboot_vrrm-preboot_graph], 1
	adc	[si+preboot_vrrm-preboot_graph], 0
	cmp	[si+preboot_device-preboot_graph], 1
	adc	[si+preboot_device-preboot_graph], 0
; notify user
	mov	si, linef-0x10000
	call	print
	mov	si, start_msg-0x10000
	call	print
	mov	si, time_msg-0x10000
	call	print
; get start time
	call	.gettime
	mov	[.starttime-0x10000], eax
	mov	word [.timer-0x10000], .newtimer-0x10000
	mov	word [.timer-0x10000+2], cs
.printcfg:
	_setcursor 9,0
	mov	si, current_cfg_msg-0x10000
	call	print
	mov	si, curvideo_msg-0x10000
	call	print
	mov	al, [preboot_graph-0x10000]
	cmp	al, 8
	ja	.pnovesa
	mov	dl, al
	and	eax, 3
	mov	si, [modes_msg-0x10000+eax*2]
	call	printplain
	mov	si, modevesa20-0x10000
	cmp	dl, 4
	jbe	@f
	mov	si, modevesa12-0x10000
@@:
	call	printplain
	cmp	dl, 4
	ja	.x
	mov	si, probeno_msg-0x10000
	cmp	[preboot_gprobe-0x10000], 2
	jnz	@f
	mov	si, probeok_msg-0x10000
@@:
	call	printplain
.x:
	jmp	.c
.pnovesa:
	cmp	al, 9
	mov	si, mode9-0x10000
	jz	@b
	mov	si, mode10-0x10000
	jmp	@b
.c:
	mov	si, linef-0x10000
	call	printplain
	mov	si, mtrr_msg-0x10000
	call	print
	mov	si, on_msg-0x10000
	cmp	[preboot_mtrr-0x10000], 1
	jz	@f
	mov	si, off_msg-0x10000
@@:
	call	printplain
	mov	si, vrrm_msg-0x10000
	call	print
	mov	si, on_msg-0x10000
	cmp	[preboot_vrrm-0x10000], 1
	jz	@f
	mov	si, off_msg-0x10000
@@:
	call	printplain
	mov	si, preboot_device_msg-0x10000
	call	print
	mov	al, [preboot_device-0x10000]
	and	eax, 3
	mov	si, [preboot_device_msgs-0x10000+eax*2]
	call	printplain
.wait:
; empty BIOS keyboard buffer
	_setcursor 25,0		; out of screen
; set timer interrupt handler
	cli
	push	0
	pop	es
	mov	eax, [es:8*4]
	mov	[.oldtimer-0x10000], eax
	mov	eax, [.timer-0x10000]
	mov	[es:8*4], eax
	sti
; wait for keypressed
	mov	ah, 0
	int	16h
	push	ax
; restore timer interrupt
	push	0
	pop	es
	mov	ecx, [.oldtimer-0x10000]
	mov	[es:8*4], ecx
	mov	[.timer-0x10000], ecx
	_setcursor 7,0
	mov	si, space_msg-0x10000
	call	printplain
	pop	ax
; switch on key
	cmp	al, 13
	jz	.continue
	or	al, 20h
	cmp	al, 'a'
	jz	.change_a
	cmp	al, 'b'
	jz	.change_b
	cmp	al, 'c'
	jz	.change_c
	cmp	al, 'd'
	jnz	.wait
	_setcursor 15,0
	mov     si,bdev-0x10000
	call    printplain
	mov     bx,'13'
	call    getkey
	mov	[preboot_device-0x10000], al
	_setcursor 13,0
.d:
	mov	[.bSettingsChanged-0x10000], al
	mov	si, space_msg-0x10000
	call	printplain
	_setcursor 15,0
	mov	cx, 6
@@:
	call	printplain
	loop	@b
	jmp	.printcfg
.change_a:
	_setcursor 15,0
	mov	si, gr_mode-0x10000
	call    printplain
	mov     bx, '09'
	call    getkey
	mov	[preboot_graph-0x10000], al
	cmp	al, 4
	ja	@f
	mov	si, probetext-0x10000
	call	printplain
	mov	bx, '12'
	call	getkey
	mov	[preboot_gprobe-0x10000], al
@@:
	_setcursor 10,0
	jmp	.d
.change_b:
	_setcursor 15,0
	mov	si, gr_acc-0x10000
	call	printplain
	mov	bx, '12'
	call	getkey
	mov	[preboot_mtrr-0x10000], al
	_setcursor 11,0
	jmp	.d
.change_c:
	_setcursor 15,0
	mov	si, vrrmprint-0x10000
	call	printplain
	mov	bx, '12'
	call	getkey
	mov	[preboot_vrrm-0x10000], al
	_setcursor 12,0
	jmp	.d
.oldtimer dd ?
.starttime dd ?
.bSettingsChanged db ?
.timer dd ?
.loader_block dd 0
.gettime:
	mov	ah, 0
	int	1Ah
	xchg	ax, cx
	shl	eax, 10h
	xchg	ax, dx
	ret
.newtimer:
	push	ds
	push	cs
	pop	ds
	pushf
	call	[.oldtimer-0x10000]
	pushad
	call	.gettime
	sub	eax, [.starttime-0x10000]
	sub	ax, 18*5
	jae	.timergo
	neg	ax
	add	ax, 18-1
	mov	bx, 18
	xor	dx, dx
	div	bx
if lang eq ru
; Ї®¤®¦¤ЁвҐ 5 бҐЄг­¤, 4/3/2 бҐЄг­¤л, 1 бҐЄг­¤г
	cmp	al, 5
	mov	cl, ' '
	jae	@f
	cmp	al, 1
	mov	cl, 'г'
	jz	@f
	mov	cl, 'л'
@@:	mov	[time_str+9-0x10000], cl
else
; wait 5/4/3/2 seconds, 1 second
	cmp	al, 1
	mov	cl, 's'
	ja	@f
	mov	cl, ' '
@@:	mov	[time_str+9-0x10000], cl
end if
	add	al, '0'
	mov	[time_str+1-0x10000], al
	mov	si, time_msg-0x10000
	_setcursor 7,0
	call	print
	_setcursor 25,0
	popad
	pop	ds
	iret
.timergo:
	push	0
	pop	es
	mov	eax, [.oldtimer-0x10000]
	mov	[es:8*4], eax
	mov	sp, 0EC00h
.continue:
	sti
	_setcursor 6,0
	mov	si, space_msg-0x10000
	call	printplain
	call	printplain
	_setcursor 6,0
	mov	si, loading_msg-0x10000
	call	print
	_setcursor 15,0
	cmp	[.bSettingsChanged-0x10000], 0
	jz	.load
	cmp	[.loader_block-0x10000], 0
	jz	.load
	les	bx, [.loader_block-0x10000]
	mov	eax, [es:bx+3]
	push	ds
	pop	es
	test	eax, eax
	jz	.load
	push	eax
	mov	si, save_quest-0x10000
	call	print
.waityn:
	mov	ah, 0
	int	16h
	or	al, 20h
	cmp	al, 'n'
	jz	.loadc
	cmp	al, 'y'
	jnz	.waityn
	call	putchar
	mov	byte [space_msg-0x10000+80], 186
	pop	eax
	push	cs
	push	.cont-0x10000
	push	eax
	retf
.loadc:
	pop	eax
.cont:
	push	cs
	pop	ds
	mov	byte [space_msg-0x10000+80], 0
	_setcursor 15,0
	mov	si, space_msg-0x10000
	call	printplain
	_setcursor 15,0
.load:
; \end{diamond}[02.12.2005]

; ASK GRAPHICS MODE

	movzx	ax, [preboot_graph-0x10000]
	test	ax, ax
        jne     pre_graph
        mov     si,gr_mode-0x10000
        call    printplain
        mov     bx,'09'
        call    getkey
       pre_graph:
	push	0
	pop	es
; address is gr_table+6*(ax-1)-0x10000
        add	ax, ax
        lea	si, [gr_table-0x10000 + eax + eax*2 - 6]
        mov     bx,[si+0]
        mov     cx,[si+2]
        mov     dx,[si+4]
        cmp	al, 9*2
        jb	@f
        mov	byte [es:0x9000], 32
        mov	dword [es:0x9018], 0x800000
       @@:
        mov     [es:0x9008],bx
        mov     [es:0x900A],cx
        mov     [es:0x900C],dx
        mov     ax,32
        test	bh, bh
        jz	nov

; USE DEFAULTS OR PROBE

; bx - mode : cx - x size : dx - y size

     ;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        movzx   ax,[preboot_gprobe-0x10000]
        test    ax,ax
        jne     pre_probe
     ;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

        test    bx,0100000000000000b
        jz      noprobe

        mov     si,probetext-0x10000
        call    printplain
        push    bx
        mov	bx, '12'
        call    getkey
        pop     bx

     pre_probe:
	dec	ax
        je      noprobe

        push    cx dx

        mov     bx,0x100

     newprobe:

        inc     bx
        cmp     bx,0x17f
        jne     probemore

        mov     si,prnotfnd-0x10000
        call    printplain

        jmp     $

     probemore:

        mov     ax,0x4f01
        mov     cx,bx
        and     cx,0xfff
        mov     di,0xa000
        int     0x10

	test	byte [es:di], 80h	; lfb?
	jz	newprobe
	mov	ax, [es:di+0x12]	; x size?
	cmp	ax, [esp+2]
	jnz	newprobe
	cmp	[es:di+0x14], dx	; y size?
	jnz	newprobe
	cmp	byte [es:di+0x19], 32 ;24
	jb	newprobe

        add     bx,0100000000000000b
        mov     [es:0x9008],bx
        pop	dx cx

     noprobe:


; FIND VESA 2.0 LFB & BPP

        mov     ax,0x4f01
        mov     cx,bx
        and     cx,0xfff
        mov     di,0xa000
        int     0x10
        ; LFB
        mov     ecx,[es:di+0x28]
        mov     [es:0x9018],ecx
        ; ---- vbe voodoo
        BytesPerScanLine equ 0x10
        mov ax, [es:di+BytesPerScanLine]
        mov [es:0x9001],ax
        ; BPP
        movzx   ax,byte [es:di+0x19]
        mov     [es:0x9000],al
       nov:
        cmp     ax,24
        mov	si,bt24-0x10000
        jz	bppl
        cmp     ax,32
        mov     si,bt32-0x10000
        jz     bppl
        mov     si,btns-0x10000
        call    print
        jmp     $
       bppl:
        call    printplain


; FIND VESA 1.2 PM BANK SWITCH ADDRESS

	push	es
        mov     ax,0x4f0A
        xor	bx, bx
        int     0x10
        xor     eax,eax
        mov     ax,es
        shl     eax,4
        movzx   ebx,di
        add     eax,ebx
        mov     bx,[es:di]
        add     eax,ebx
        pop     es
        mov     [es:0x9014],eax


; GRAPHICS ACCELERATION

        mov     al, [preboot_mtrr-0x10000]
        test    al,al
        jne     pre_mtrr
        mov     si,gr_acc-0x10000
        call    printplain
        mov     bx,'12'
        call    getkey
      pre_mtrr:
        mov     [es:0x901C],al
; commented by diamond - newlines are not used now
;        mov     si,linef-0x10000
;        call    printplain


; VRR_M USE

        mov     al,[preboot_vrrm-0x10000]
        test    al,al
        jne     pre_vrrm
        mov     si,vrrmprint-0x10000
        call    print
        mov     bx,'13'
        call    getkey
      pre_vrrm:
        mov     [es:0x9030],al
; commented by diamond - newlines are not used now
;        mov     si,linef2-0x10000
;        call    printplain


; MEMORY MODEL

;        movzx   eax,byte [es:preboot_memory-0x10000]
;        cmp     eax,0
;        jne     pre_mem
;;;;;;;;;;;;;;;;;;;;;;;;;
; mario79 - memory size ;
;;;;;;;;;;;;;;;;;;;;;;;;;
;           mov ax,0E801h
;;;           xor bx,bx    ; thanks to Alexei for bugfix [18.07.2004]
;           xor cx, cx
;           xor dx, dx
;           int 0x15
;           movzx ebx, dx ;bx
;           movzx eax, cx ;ax
;           shl   ebx,6   ; перевод в килобайты (x64)
;           add   eax,ebx
;           add eax, 1000h ;440h
;           cmp eax,40000h ; 256?
;           jge mem_256_z
;           cmp eax,20000h ; 128?
;           jge mem_128_z
;           cmp eax,10000h ; 64?
;           jge mem_64_z
;           cmp eax,8000h ; 32?
;           jge mem_32_z
;           jmp mem_16_z
;
;mem_256_z: mov     si,memokz256-0x10000
;           call    printplain
;           mov eax,5
;           jmp pre_mem
;mem_128_z: mov     si,memokz128-0x10000
;           call    printplain
;           mov eax,4
;           jmp pre_mem
;mem_64_z:  mov     si,memokz64-0x10000
;           call    printplain
;           mov eax,3
;           jmp pre_mem
;mem_32_z:  mov     si,memokz32-0x10000
;           call    printplain
;           mov eax,2
;           jmp pre_mem
;mem_16_z:  mov     si,memokz16-0x10000
;           call    printplain
;           mov eax,1
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;      pre_mem:
;        push    word 0x0000
;        pop     es
;        mov     [es:0x9030],al
;        push    word 0x1000
;        pop     es
;        mov     si,linef-0x10000
;        call    printplain




; DIRECT WRITE TO LFB, PAGING DISABLED

;        movzx   eax,byte [es:preboot_lfb-0x10000]
;        mov     eax,1                             ; paging disabled
;        cmp     eax,0
;        jne     pre_lfb
;        mov     si,gr_direct-0x10000
;        call    printplain
;        mov     ebx,'12'
;        call    getkey
;      pre_lfb:
;        push    word 0x0000
;        pop     es
;        mov     [es:0x901E],al
;        mov     ax,0x1000
;        mov     es,ax
;        mov     si,linef-0x10000
;        call    printplain
        mov     [es:0x901E],byte 1



; BOOT DEVICE

	mov	al, [preboot_device-0x10000]
	test	al, al
	jnz	pre_device
        mov     si,bdev-0x10000
        call    printplain
        mov     bx,'13'
        call    getkey
      pre_device:
        dec     ax
        mov     [boot_dev-0x10000],al
; commented by diamond - newlines are not used now
;        mov     si,linef-0x10000
;        call    printplain



; READ DISKETTE TO MEMORY

        cmp     [boot_dev-0x10000],0
        jne     no_sys_on_floppy
        mov     si,diskload-0x10000
        call    print
        xor	ax, ax			; reset drive
        xor	dx, dx
        int     0x13
        mov     cx,0x0001               ; startcyl,startsector
        xor	dx, dx			; starthead,drive
        push    word 80*2               ; read no of sect
       reads:
        pusha
        xor     si,si
       newread:
        mov     bx,0xa000               ; es:bx -> data area
        mov     ax,0x0200+18            ; read, no of sectors to read
        int     0x13
        test	ah, ah
        jz      goodread
        inc	si
        cmp     si,10
        jnz     newread
        mov     si,badsect-0x10000
        call    printplain
        jmp     $
       goodread:
        ; move -> 1mb
        mov     si,movedesc-0x10000
        push	es
        push	ds
        pop     es
        mov     cx,256*18
        mov     ah,0x87
        int     0x15
        pop	es

        test    ah,ah                  ; was the move successfull ?
        je      goodmove
        mov     dx,0x3f2              ; floppy motor off
        mov     al,0
        out     dx,al
        mov     si,memmovefailed-0x10000
        call    print
        jmp     $
      goodmove:

	add	dword [movedesc-0x10000+0x18+2], 512*18
        popa
        inc     dh
        cmp     dh,2
        jnz     bb2
        mov     dh,0
        inc     ch
        pusha                        ; print prosentage
        mov     si,pros-0x10000
	shr	ch, 2
	mov	al, '5'
	test	ch, 1
	jnz	@f
	mov	al, '0'
@@:
	mov	[si+1], al
	shr	ch, 1
	add	ch, '0'
	mov	[si], ch
        call    printplain
        popa
       bb2:
        pop     ax
        dec     ax
        push    ax
        jnz     reads
       readdone:
        pop     ax
        mov     si,backspace-0x10000
        call    printplain
        call    printplain
        mov     si,okt-0x10000
        call    printplain
       no_sys_on_floppy:
        xor	ax, ax		; reset drive
        xor	dx, dx
        int     0x13
       mov dx,0x3f2 ; floppy motor off
       mov al,0
       out dx,al

	push	es
; PAGE TABLE

	push	dword [es:0x9018]

        map_mem equ 64                ; amount of memory to map

        push	0x6000
        pop	es                    ; es:di = 6000:0
        xor     di,di
        mov     cx,256*map_mem         ; Map (mapmem) M
        mov     eax,7
       pt2:
        cmp     cx,256*(map_mem-8)     ; 8 M map to LFB
        jnz     pt3
        pop     eax
        add     eax,7
       pt3:
        cmp     cx,256*(map_mem-12)    ; 12 M back to linear = physical
        jnz     pt4
        mov     eax,12*0x100000 + 7
       pt4:
        stosd
        add     eax,4096
        loop    pt2
        
	push	0x7100
	pop	es
        xor     di,di
        mov     eax,8*0x100000+7
        mov     cx,256*4
      pt5:
        stosd
        add     eax,0x1000
        loop    pt5
        
; 4 KB PAGE DIRECTORY

	push	0x7F00
	pop	es                ; es:di = 7F00:0
        xor     di, di
        mov     cx, 64 / 4
        mov     eax, 0x60007            ; for 0 M
      pd4k:
        stosd
        add     eax, 0x1000
        loop    pd4k
        mov     dword [es:0x800],0x71007   ;map region 0x80000000-0x803FFFFF to 0x800000-0xCFFFFF
        xor     si,si
        mov     di,second_base_address shr 20
        mov     cx,64/4
        rep     movs dword [es:di], [es:si]
       
        mov     eax, 0x7F000 +8+16      ; Page directory and enable caches
        mov     cr3, eax

; SET GRAPHICS

	pop	es
        mov     bx,[es:0x9008]		; vga & 320x200
        mov	ax, bx
	push	ax
        cmp     ax,0x13
        je      setgr
        cmp     ax,0x12
        je      setgr
        mov     ax,0x4f02            ; Vesa
       setgr:
        int     0x10
        test	ah,ah
        jz      gmok
        mov     si,fatalsel-0x10000
        call    print
        jmp     $
        
       gmok:
; set mode 0x12 graphics registers:
        cmp     bx,0x12
        jne     gmok2

        mov     al,0x05
        mov     dx,0x03ce
        out     dx,al      ; select GDC mode register
        mov     al,0x02
        inc	dx
        out     dx,al      ; set write mode 2

        mov     al,0x02
        mov     dx,0x03c4
        out     dx,al      ; select VGA sequencer map mask register
        mov     al,0x0f
        inc	dx
        out     dx,al      ; set mask for all planes 0-3

        mov     al,0x08
        mov     dx,0x03ce
        out     dx,al      ; select GDC bit mask register
                           ; for writes to 0x03cf

       gmok2:
        push	ds
        pop	es

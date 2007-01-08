Advanced Power Management

SYSTEM CALL

eax = 49
dx = номер функции APM BIOS (аналогичен ax в реальном режиме)
остальные (bx, cx) регистры по спецификации (см. apm.txt)
результат : по спецификации (включая CF), старшая часть 32 битных регистров не определена


MEMORY MAP

Boot:
	0x9040 - dword - entry point of APM BIOS
	0x9044 -  word - version (BCD)
	0x9046 -  word - flags


ИЗМЕНЕНИЯ

bootcode.inc
syscall.inc
kernel.asm
bootcode.inc
shutdown.inc

##############[boot\bootcode.inc]##################

Три новых дескриптора

.............
.............

; GDT TABLE

gdts:

        dw     gdte-$-1
        dd     gdts
        dw     0

int_code_l:
os_code_l:

        dw     0xffff
        dw     0x0000
        db     0x00
        dw     11011111b *256 +10011010b
        db     0x00

int_data_l:
os_data_l:

        dw     0xffff
        dw     0x0000
        db     0x00
        dw     11011111b *256 +10010010b
        db     0x00
; --------------- APM ---------------------
apm_code_32:
        dw     0x0f        ; limit 64kb
        db     0, 0, 0
        dw     11010000b *256 +10011010b
        db     0x00
apm_code_16:
        dw     0x0f
        db     0, 0, 0
        dw     10010000b *256 +10011010b
        db     0x00
apm_data_16:
        dw     0x0f
        db     0, 0, 0
        dw     10010000b *256 +10010010b
        db     0x00
; -----------------------------------------
app_code_l:
      dw ((0x80000000-std_application_base_address) shr 12) and 0xffff
      dw 0
      db 0
      dw 11010000b*256+11111010b+256*((0x80000000-std_application_base_address) shr 28)
      db std_application_base_address shr 24

app_data_l:
      dw (0x80000000-std_application_base_address) shr 12 and 0xffff
      dw 0
      db 0
      dw 11010000b*256+11110010b+256*((0x80000000-std_application_base_address) shr 28)
      db std_application_base_address shr 24

graph_data_l:

        dw     0x3ff
        dw     0x0000
        db     0x00
        dw     11010000b *256 +11110010b
        db     0x00

tss0_l:

.............
.............

##############[core\syscall.inc]###################

.............
.............


      dd display_number          ; 47-WriteNum
      dd display_settings        ; 48-SetRedrawType and SetButtonType
      dd sys_apm                 ; 49-Advanced Power Management (APM)
      dd random_shaped_window    ; 50-Window shape & scale
      dd syscall_threads         ; 51-Threads


.............
.............

##############[kernel.asm]#########################


Часть 1 (после метки "; SAVE REAL MODE VARIABLES"):

.............
.............

; SAVE REAL MODE VARIABLES

; --------------- APM ---------------------
	mov	eax, [0x2f0000 + 0x9040]	; entry point
	mov	dword[apm_entry], eax
	mov	word [apm_entry + 4], apm_code_32 - gdts

	mov	eax, [0x2f0000 + 0x9044]	; version & flags
	mov	[apm_vf], eax
; -----------------------------------------

.............
.............

Часть 2 (системный вызов, расположение не критично,
я расположил перед меткой "undefined_syscall:")

.............
.............

; --------------- APM ---------------------
apm_entry	dp	0
apm_vf		dd	0
align 4
sys_apm:
	cmp	word [apm_vf], 0	; Check APM BIOS enable
	jne	@f
	or	[esp + 56], byte 1	; error
	mov	[esp + 36], dword 8	; 32-bit protected-mode interface not supported
	ret
	
@@:	xchg	eax, ecx
	xchg	ebx, ecx
	
	cmp	al, 3
	ja	@f
	and	[esp + 56], byte 0xfe	; emulate func 0..3 as func 0
	mov	eax, [apm_vf]
	mov	[esp + 36], eax
	shr	eax, 16
	mov	[esp + 32], eax
	ret
	
@@:	call	pword [apm_entry]	; call APM BIOS
	mov	[esp + 8 ], edi
	mov	[esp + 12], esi
	mov	[esp + 24], ebx
	mov	[esp + 28], edx
	mov	[esp + 32], ecx
	mov	[esp + 36], eax
	setc	al
	and	[esp + 56], byte 0xfe
	or	[esp + 56], al
	ret
; -----------------------------------------

align 4

undefined_syscall:                      ; Undefined system call

.............
.............

##############[boot\bootcode.inc]##################

Перед меткой "; DISPLAY VESA INFORMATION"

.............
.............

; --------------- APM ---------------------
	push	0
	pop	es
	mov	word [es : 0x9044], 0		; ver = 0.0 (APM not found)
	mov	ax, 0x5300
	xor	bx, bx
	int	0x15
	jc	apm_end				; APM not found
	test	cx, 2
	jz	apm_end				; APM 32-bit protected-mode interface not supported
	mov	[es : 0x9044], ax		; Save APM Version
	mov	[es : 0x9046], cx		; Save APM flags
	
	; Write APM ver ----
	jmp	@f
msg_apm:db	' APM x.x ', 0
@@:	and	ax, 0xf0f
	add	ax, '00'
	mov	si, msg_apm - 0x10000
	mov	[si + 5], ah
	mov	[si + 7], al
	_setcursor 0, 3
	call	printplain
	_setcursor d80x25_top_num,0
	; ------------------
	
	mov	ax, 0x5304			; Disconnect interface
	xor	bx, bx
	int	0x15
	mov	ax, 0x5303			; Connect 32 bit mode interface
	xor	bx, bx
	int	0x15
	; init selectors
	movzx	eax, ax ; real-mode segment base address of protected-mode 32-bit code segment
	shl	eax, 4
	mov	[apm_code_32 - 0x10000 + 2], ax
	shr	eax, 16
	mov	[apm_code_32 - 0x10000 + 4], al
	movzx	ecx, cx ; real-mode segment base address of protected-mode 16-bit code segment
	shl	ecx, 4
	mov	[apm_code_16 - 0x10000 + 2], cx
	shr	ecx, 16
	mov	[apm_code_16 - 0x10000 + 4], cl
	movzx	edx, dx ; real-mode segment base address of protected-mode 16-bit data segment
	shl	edx, 4
	mov	[apm_data_16 - 0x10000 + 2], dx
	shr	edx, 16
	mov	[apm_data_16 - 0x10000 + 4], dl
	mov	[es : 0x9040], ebx              ; offset of APM entry point
apm_end:
; -----------------------------------------


; DISPLAY VESA INFORMATION

.............
.............

##############[boot\shutdown.inc]##################

Решение проблемы APM poweroff

.............
.............

APM_PowerOff:
        mov     ax, 5304h
        xor     bx, bx
        int     15h
;!!!!!!!!!!!!!!!!!!!!!!!!
mov ax,0x5300
xor bx,bx
int 0x15
push ax

.............
.............

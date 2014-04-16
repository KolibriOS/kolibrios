; PRIVATE FIELDS

 __shell.buf__	rd 1
 __shell.name__ rb 16
 __shell.str__	rb 16

;-----------------------------------
; a = 0
;-----------------------------------

 macro clear a {
    xor 	a, a
 }

;-----------------------------------
; if a == b then c
;-----------------------------------

 macro cmpe a, b, c {
    cmp 	a, b
    je		c
 }

;-----------------------------------
; if a != b then c
;-----------------------------------

 macro cmpne a, b, c {
    cmp 	a, b
    jne 	c
 }

;-----------------------------------
; Shell`s initialization
; without arguments
;-----------------------------------

 shell.init:
    pusha

  ; Get PID
    mov 	eax, 68
    mov 	ebx, 11
    int 	64

    mov 	eax, 68
    mov 	ebx, 12
    mov 	ecx, 1024
    int 	64

    mov 	esi, eax

    mov 	eax, 9
    mov 	ebx, esi
    mov 	ecx, -1
    int 	64

    mov 	eax, dword [esi + 30]
    mov 	[@pid], eax

    mov 	eax, 68
    mov 	ebx, 13
    mov 	ecx, esi
    int 	64

  ; Convert PID to STR
    mov 	eax, [@pid]
    mov 	ebx, 10
    clear	ecx
  @@:
    clear	edx
    div 	ebx
    push	edx
    inc 	ecx
    cmpne	eax, 0, @b

    mov 	ebx, __shell.name__
  @@:
    pop 	eax
    add 	al, "0"
    mov 	[ebx], al
    inc 	ebx
    loop	@b

  ; Add postfix
    mov 	[ebx + 0], dword "-SHE"
    mov 	[ebx + 4], word "LL"
    mov 	[ebx + 6], byte 0

  ; Open buffer
    mov 	eax, 68
    mov 	ebx, 22
    mov 	ecx, __shell.name__
    mov 	edx, 4096
    mov 	esi, 101b
    int 	64

    mov 	[__shell.buf__], eax

    mov 	eax, 5
    mov 	ebx, 5
    int 	64

    popa
    ret

;-----------------------------------
; Wait answer from shell
;
; INPUT:
; edi - shell-buffer
;-----------------------------------

 shell.wait:
    pusha

  @@:
    mov 	eax, 5
    mov 	ebx, 1
    int 	64

    cmpne	[edi], byte 0, @b

    popa
    ret

;-----------------------------------
; Exit from shell
; without arguments
;-----------------------------------

 shell.destroy:
    pusha

    mov 	edi, [__shell.buf__]
    mov 	[edi], byte 1
    call	shell.wait

    mov 	eax, 68
    mov 	ebx, 23
    mov 	ecx, __shell.name__
    int 	64

    popa
    ret

;-----------------------------------
; Print char to shell
;
; INPUT:
; al - char
;-----------------------------------

 shell.print_char:
    pusha

    mov 	edi, [__shell.buf__]
    mov 	[edi], byte 2
    mov 	[edi + 1], al
    call	shell.wait

    popa
    ret

;-----------------------------------
; Print string to shell
;
; INPUT:
; eax - string
;-----------------------------------

 shell.print_string:
    pusha

    mov 	edi, [__shell.buf__]
    mov 	[edi], byte 3
    inc 	edi

  @@:
    mov 	bl, [eax]
    mov 	[edi], bl
    inc 	edi
    inc 	eax
    cmpne	[eax], byte 0, @b

    mov 	[edi], byte 0
    mov 	edi, [__shell.buf__]
    call	shell.wait

    popa
    ret

;-----------------------------------
; Print number to shell
;
; INPUT:
; eax - number
;-----------------------------------

 shell.print_num:
    pusha

    mov 	ecx, eax
    and 	ecx, 1 shl 31
    cmpe	ecx, 0, @f
    mov 	[__shell.str__], "-"
    not 	eax
    inc 	eax

  @@:
    mov 	ebx, 10
    clear	ecx
  @@:
    clear	edx
    div 	ebx
    push	edx
    inc 	ecx
    cmpne	eax, 0, @b

    mov 	ebx, __shell.str__
    inc 	ebx

  @@:
    pop 	eax
    add 	al, "0"
    mov 	[ebx], al
    inc 	ebx
    loop	@b

    mov 	[ebx], byte 0
    mov 	eax, __shell.str__
    cmpe	[eax], byte "-", @f
    inc 	eax
  @@:
    call	shell.print_string

    popa
    ret

;-----------------------------------
; Get char from shell
;
; OUTPUT:
; al - char
;-----------------------------------

 shell.get_char:
    push	ebx ecx edx esi edi

    mov 	edi, [__shell.buf__]
    mov 	[edi], byte 4
    call	shell.wait
    mov 	al, [edi + 1]

    pop 	edi esi edx ecx ebx
    ret

;-----------------------------------
; Get string from shell
;
; INPUT:
; eax - addres of memory for str
;-----------------------------------

 shell.get_string:
    pusha

    mov 	edi, [__shell.buf__]
    mov 	[edi], byte 5
    call	shell.wait

    inc 	edi
  @@:
    mov 	bl, [edi]
    mov 	[eax], bl
    inc 	eax
    inc 	edi
    cmpne	[edi], byte 0, @b

    mov 	[eax], byte 0

    popa
    ret

;-----------------------------------
; Get number from shell
;
; OUTPUT:
; eax - number
;-----------------------------------

 shell.get_num:
    push	ebx ecx edx esi edi

    mov 	eax, __shell.str__
    call	shell.get_string

    mov 	ebx, eax
    clear	eax
    clear	edi
    cmpne	[ebx], byte "-", @f
    mov 	edi, 1
    inc 	ebx

  @@:
    sub 	[ebx], byte "0"
    imul	eax, 10
    add 	al, [ebx]
    inc 	ebx
    cmpne	[ebx], byte 10, @b

    cmpe	edi, 0, @f
    not 	eax
    inc 	eax

  @@:
    pop 	edi esi edx ecx ebx
    ret


;-----------------------------------
; Clear shell
; without arguments
;-----------------------------------

 shell.clear:
    pusha

    mov 	edi, [__shell.buf__]
    mov 	[edi], byte 6
    call	shell.wait

    popa
    ret

;-----------------------------------
; Print char to shell
;
; INPUT:
; arg1 - char
;-----------------------------------

 macro shpc [char] {
    push	eax
    mov 	al, char
    call	shell.print_char
    pop 	eax
 }

;-----------------------------------
; Go to new line in shell
; without arguments
;-----------------------------------

 macro shln {
    shpc	10
    shpc	13
 }

;-----------------------------------
; Print string to shell
;
; INPUT:
; arg1 - string
;-----------------------------------

 macro shps [string] {
    local	..string, ..label
    jmp 	..label
 ..string	db string, 0
 ..label:

    push	eax
    mov 	eax, ..string
    call	shell.print_string
    pop 	eax
 }

;-----------------------------------
; Print string to shell
;
; INPUT:
; arg1 - addres of string
;-----------------------------------

 macro shpsa [addr] {
    push	eax
    mov 	eax, addr
    call	shell.print_string
    pop 	eax
 }

;-----------------------------------
; Print number to shell
;
; INPUT:
; arg1 - number
;-----------------------------------

 macro shpn [num] {
    push	eax
    mov 	eax, num
    call	shell.print_num
    pop 	eax
 }


;-----------------------------------
; Get char from shell
;
; OUTPUT:
; al - char
;-----------------------------------

 macro shgc {
    call	shell.get_char
 }

;-----------------------------------
; Get string from shell
;
; INPUT:
; arg1 - addres of memory for str
;-----------------------------------

 macro shgs [addr] {
    mov 	eax, addr
    call	shell.get_string
 }

;-----------------------------------
; Get number from shell
;
; INPUT:
; arg1 - addres of memory for num
;-----------------------------------

 macro shgn [addr] {
    push	eax
    call	shell.get_num
    mov 	[addr], eax
    pop 	eax
 }


;-----------------------------------
; Wait any char from shell
; without arguments
;-----------------------------------

 macro shw {
    push	eax
    call	shell.get_char
    pop 	eax
 }

;-----------------------------------
; Clear shell
; without arguments
;-----------------------------------

 macro shc {
    call	shell.clear
 }

;-----------------------------------
; Choice
;
; INPUT:
; arg1 - question
; arg2 - asnwers
;
; OUTPUT:
; al - answer
;-----------------------------------

 macro shchs str, chs {
    local	..loop1, ..loop2, ..chs, ..start, ..res
    jmp 	..start
  ..chs 	db chs, 0
  ..start:
    push	ebx

    shps	str, " ["
    shpsa	..chs
    shps	"]: "

  ..loop1:
    call	shell.get_char

    mov 	ebx, ..chs
  ..loop2:
    cmpe	al, [ebx], ..res
    inc 	ebx
    cmpe	[ebx], byte 0, ..loop1
    jmp 	..loop2

  ..res:
    mov 	al, [ebx]
    call	shell.print_char
    shln

    mov 	eax, ebx
    sub 	eax, ..chs

    pop 	ebx
 }
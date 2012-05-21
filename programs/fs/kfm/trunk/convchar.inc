;---------------------------------------------------------------------
char_toupper:
; convert character to uppercase, using cp866 encoding
; in: al=symbol
; out: al=converted symbol
    cmp     al, 'a'
    jb	    .ret
    cmp     al, 'z'
    jbe     .az
    cmp     al, ' '
    jb	    .ret
    cmp     al, 'à'
    jb	    .rus1
    cmp     al, 'ï'
    ja	    .ret
; 0xE0-0xEF -> 0x90-0x9F
    sub     al, 'à'-''
.ret:
    ret
.rus1:
; 0xA0-0xAF -> 0x80-0x8F
.az:
    and     al, not 0x20
    ret
;---------------------------------------------------------------------
char_todown:
; convert character to lowercase, using cp866 encoding
; in: al=symbol
; out: al=converted symbol
	cmp	al, 'A'
	jb	.ret
	cmp	al, 'Z'
	jbe	.az
	cmp	al, '€'
	jb	.ret
	cmp	al, ''
	jb	.rus1
	cmp	al, 'Ÿ'
	ja	.ret
; 0x90-0x9F -> 0xE0-0xEF
	add	al, 'à'-''
.ret:
	ret
.rus1:
; 0x80-0x8F -> 0xA0-0xAF
.az:
	add	al, 0x20
	ret
;---------------------------------------------------------------------
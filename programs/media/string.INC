;****************************************
;*  input:  esi = pointer to string     *
;*  output: ecx = length of the string  *
;****************************************
strlen:
      push	eax esi
      xor	ecx, ecx
      @@:
	lodsb
	or	al, al
	jz	@f
	inc	ecx
	jmp	@b
      @@:
      pop      esi eax
  ret

;*************************************************
;*  input:  esi = pointer to the src string      *
;*          edi = pointer to the dest string     *
;*          ecx = number of bytes to copy        *
;*************************************************
strncpy:
      push	eax ecx esi edi
      @@:
	lodsb
	stosb
	or	al, al
	jz	@f
	dec	ecx
	jz	@f
	jmp	@b
      @@:
      pop	edi esi ecx eax
  ret

;*************************************************
;*  input:  esi = pointer to the src string      *
;*          edi = pointer to the dest string     *
;*************************************************
strcpy:
     push  esi edi
     rep   movsb
     pop   edi esi
  ret

;*************************************************
;*  input:  esi = pointer to the src string      *
;*          edi = pointer to the dest string     *
;*************************************************
strcat:
     push  esi
     call  strlen
     add   esi, ecx
     call  strcpy
     pop   esi
  ret

;*************************************************
;*  input:  esi = pointer to the src string      *
;*          edi = pointer to the dest string     *
;*          ecx = number of bytes to copy        *
;*************************************************
strncat:
     push  edi
     push  ecx esi
     mov   esi, edi
     call  strlen
     add   edi, ecx
     pop   esi ecx
     call  strncpy
     pop   edi
  ret

;*************************************************
;*  input:  edi = pointer to the dest string     *
;*           al = byte to set the string to      *
;*************************************************
;strset:
;     push  edi
;     rep   stosb
;     pop   edi
;  ret

;*************************************************
;*  input:  edi = pointer to the dest string     *
;*           al = byte to set the string to      *
;*          ecx = number of bytes to set         *
;*************************************************
strnset:
     push    edi ecx
     repe    stosb
     pop     ecx edi
  ret

;
; Formatted Debug Output (FDO)
; Copyright (c) 2005-2006, mike.dld
; Created: 2005-01-29, Changed: 2006-07-20
;
; For questions and bug reports, mail to mike.dld@gmail.com
;
; Available format specifiers are: %s, %d, %u, %x (with partial width support)
;

; to be defined:
;   __DEBUG__ equ 1
;   __DEBUG_LEVEL__ equ 5

macro debug_func name {
 if used name
  name@of@func equ name
}

macro debug_beginf {
 align 4
 name@of@func:
}

debug_endf fix end if

macro DEBUGS _sign,[_str] {
 common
  pushf
  pushad
  local ..str,..label,is_str
  is_str = 0
 forward
  if _str eqtype ''
   is_str = 1
  end if
 common
  if is_str = 1
   jmp ..label
   ..str db _str,0
   ..label:
   add	esp,4*8+4
   mov	edx,..str
   sub	esp,4*8+4
   call fdo_debug_outstr
  else
   mov	edx,_str
   call fdo_debug_outstr
  end if
  popad
  popf
}

macro DEBUGD _sign,_dec {
 pushf
 pushad
 if _dec eqtype eax
  if _dec in <ebx,ecx,edx,esi,edi,ebp,esp>
   mov	eax,_dec
  else if ~_dec eq eax
   if _sign = 1
    movsx eax,_dec
   else
    movzx eax,_dec
   end if
  end if
 else if _dec eqtype 0
  mov eax,_dec
 else
  add  esp,4*8+4
  local tp
  tp equ 0
  match _num[_arg],_dec \{
   if _num = 1
    if _sign = 1
     movsx eax,byte[_arg]
    else
     movzx eax,byte[_arg]
    end if
   else if _num = 2
    if _sign = 1
     movsx eax,word[_arg]
    else
     movzx eax,word[_arg]
    end if
   else
    mov   eax,dword[_arg]
   end if
   tp equ 1
  \}
  match =0 [_arg],tp _dec \{
   mov	 eax,dword[_arg]
  \}
  sub  esp,4*8+4
 end if
 mov  cl,_sign
 call fdo_debug_outdec
 popad
 popf
}

macro DEBUGH _sign,_hex {
 pushf
 pushad
 if _hex eqtype eax
  if _hex in <eax,ebx,ecx,edx,esi,edi,ebp,esp>
   if ~_hex eq eax
    mov   eax,_hex
   end if
   mov	 edx,8
  else if _hex in <ax,bx,cx,dx,si,di,bp,sp>
   if ~_hex eq ax
    movzx eax,_hex
   end if
   shl	 eax,16
   mov	 edx,4
  else if _hex in <al,ah,bl,bh,cl,ch,dl,dh>
   if ~_hex eq al
    movzx eax,_hex
   end if
   shl	 eax,24
   mov	 edx,2
  end if
 else if _hex eqtype 0
  mov eax,_hex
  mov edx,8
 else
  add  esp,4*8+4
  local tp
  tp equ 0
  match _num[_arg],_hex \{
   mov	 eax,dword[_arg]
   mov	edx,_num
   tp equ 1
  \}
  match =0 [_arg],tp _hex \{
   mov	 eax,dword[_arg]
   mov	 edx,8
  \}
  sub  esp,4*8+4
 end if
 call fdo_debug_outhex
 popad
 popf
}

;-----------------------------------------------------------------------------

debug_func fdo_debug_outchar
debug_beginf
	pushad
	mov	cl,al
	mov	ebx,1
	mov	eax,63
	int	0x40
	popad
	ret
debug_endf

debug_func fdo_debug_outstr
debug_beginf
	mov	eax,63
	mov	ebx,1
  .l1:	mov	cl,[edx]
	or	cl,cl
	jz	.l2
	int	0x40
	inc	edx
	jmp	.l1
  .l2:	ret
debug_endf

debug_func fdo_debug_outdec
debug_beginf
	or	cl,cl
	jz	@f
	or	eax,eax
	jns	@f
	neg	eax
	push	eax
	mov	al,'-'
	call	fdo_debug_outchar
	pop	eax
    @@: push	10
	pop	ecx
	push	-'0'
  .l1:	xor	edx,edx
	div	ecx
	push	edx
	test	eax,eax
	jnz	.l1
  .l2:	pop	eax
	add	al,'0'
	jz	.l3
	call	fdo_debug_outchar
	jmp	.l2
  .l3:	ret
debug_endf

debug_func fdo_debug_outhex
  __fdo_hexdigits db '0123456789ABCDEF'
debug_beginf
	mov	cl,dl
	neg	cl
	add	cl,8
	shl	cl,2
	rol	eax,cl
  .l1:	rol	eax,4
	push	eax
	and	eax,0x0000000F
	mov	al,[__fdo_hexdigits+eax]
	call	fdo_debug_outchar
	pop	eax
	dec	edx
	jnz	.l1
	ret
debug_endf

;-----------------------------------------------------------------------------

macro DEBUGF _level,_format,[_arg] {
 common
 if __DEBUG__ = 1 & _level >= __DEBUG_LEVEL__
  local ..f1,f2,a1,a2,c1,c2,c3,..lbl
  _debug_str_ equ __debug_str_ # a1
  a1 = 0
  c2 = 0
  c3 = 0
  f2 = 0
  repeat ..lbl-..f1
   virtual at 0
    db _format,0,0
    load c1 word from %-1
   end virtual
   if c1 = '%s'
    virtual at 0
     db _format,0,0
     store word 0 at %-1
     load c1 from f2-c2
    end virtual
    if c1 <> 0
     DEBUGS 0,_debug_str_+f2-c2
    end if
    c2 = c2 + 1
    f2 = %+1
    DEBUGF_HELPER S,a1,0,_arg
   else if c1 = '%x'
    virtual at 0
     db _format,0,0
     store word 0 at %-1
     load c1 from f2-c2
    end virtual
    if c1 <> 0
     DEBUGS 0,_debug_str_+f2-c2
    end if
    c2 = c2 + 1
    f2 = %+1
    DEBUGF_HELPER H,a1,0,_arg
   else if c1 = '%d' | c1 = '%u'
    local c4
    if c1 = '%d'
     c4 = 1
    else
     c4 = 0
    end if
    virtual at 0
     db _format,0,0
     store word 0 at %-1
     load c1 from f2-c2
    end virtual
    if c1 <> 0
     DEBUGS 0,_debug_str_+f2-c2
    end if
    c2 = c2 + 1
    f2 = %+1
    DEBUGF_HELPER D,a1,c4,_arg
   else if c1 = '\n'
    c3 = c3 + 1
   end if
  end repeat
  virtual at 0
   db _format,0,0
   load c1 from f2-c2
  end virtual
  if (c1<>0)&(f2<>..lbl-..f1-1)
   DEBUGS 0,_debug_str_+f2-c2
  end if
  virtual at 0
   ..f1 db _format,0
   ..lbl:
   __debug_strings equ __debug_strings,_debug_str_,<_format>,..lbl-..f1-1-c2-c3
  end virtual
 end if
}

macro __include_debug_strings dummy,[_id,_fmt,_len] {
 common
  local c1,a1,a2
 forward
  if defined _len & ~_len eq
   _id:
   a1 = 0
   a2 = 0
   repeat _len
    virtual at 0
     db _fmt,0,0
     load c1 word from %+a2-1
    end virtual
    if (c1='%s')|(c1='%x')|(c1='%d')|(c1='%u')
     db 0
     a2 = a2 + 1
    else if (c1='\n')
     dw $0A0D
     a1 = a1 + 1
     a2 = a2 + 1
    else
     db c1 and 0x0FF
    end if
   end repeat
   db 0
  end if
}

macro DEBUGF_HELPER _letter,_num,_sign,[_arg] {
 common
  local num
  num = 0
 forward
  if num = _num
    DEBUG#_letter _sign,_arg
  end if
  num = num+1
 common
  _num = _num+1
}

macro include_debug_strings {
 if __DEBUG__ = 1
  match dbg_str,__debug_strings \{
   __include_debug_strings dbg_str
  \}
 end if
}

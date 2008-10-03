;-----------------------------------------------------------------------------
;///// PART OF ATi RADEON 9000 DRIVER ////////////////////////////////////////
;-----------------------------------------------------------------------------
; Copyright (c) 2004, mike.dld
; Using BeOS driver - Copyright (c) 2002, Thomas Kurschel
;-----------------------------------------------------------------------------
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
; FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
; DEALINGS IN THE SOFTWARE.
;-----------------------------------------------------------------------------

macro __include_debug_strings dummy,[_id,_fmt,_len] {
 common
  local c1,a1,a2
 forward
  _id rb _len+1
  db 0
  a1 = 0
  a2 = 0
  if ~ _len eq
   repeat _len-1
    virtual at 0
     db _fmt,13,10,0,0
     load c1 word from %+a2-1
    end virtual
    if (c1='%s')|(c1='%x')|(c1='%d')
     store byte 0 at _id+%+a1-1
     a2 = a2 + 1
    else if (c1='\n')
     store word $0A0D at _id+%+a1-1
     a1 = a1 + 1
     a2 = a2 + 1
    else
     store word c1 at _id+%+a1-1
    end if
   end repeat
  end if
 common
}

macro include_debug_strings { __include_debug_strings __debug_strings }

SHINFO0 fix SHINFO

macro SHINFO _num,_format,[_arg] {
 common
  SHFLOW _format,_arg
}

macro SHFLOW _format,[_arg] {
 common
 if __DEBUG__ = 1
  local ..f1,f2,a1,a2,c1,c2,..lbl
  _debug_str_ fix __debug_str_ # a1
  a1 = 0
  c2 = 0
  f2 = 0
  repeat ..lbl-..f1-1
   virtual at 0
    db _format,13,10,0
    load c1 word from %-1
   end virtual
   if c1 = '%s'
    virtual at 0
     db _format,13,10,0
     store word 0 at %-1
     load c1 from f2-c2
    end virtual
    if c1 <> 0
     DEBUGS _debug_str_+f2-c2
    end if
    c2 = c2 + 1
    f2 = %+1
    a2 = 0
    forward
     if a2 = a1
      DEBUGS _arg
     end if
     a2 = a2+1
    common
    a1 = a1+1
   else if c1 = '%x'
    virtual at 0
     db _format,13,10,0
     store word 0 at %-1
     load c1 from f2-c2
    end virtual
    if c1 <> 0
     DEBUGS _debug_str_+f2-c2
    end if
    c2 = c2 + 1
    f2 = %+1
    a2 = 0
    forward
     if a2 = a1
      DEBUGH _arg
     end if
     a2 = a2+1
    common
    a1 = a1+1
   else if c1 = '%d'
    virtual at 0
     db _format,13,10,0
     store word 0 at %-1
     load c1 from f2-c2
    end virtual
    if c1 <> 0
     DEBUGS _debug_str_+f2-c2
    end if
    c2 = c2 + 1
    f2 = %+1
    a2 = 0
    forward
     if a2 = a1
      DEBUGD _arg
     end if
     a2 = a2+1
    common
    a1 = a1+1
   end if
  end repeat
  virtual at 0
   db _format,13,10,0
   load c1 from f2-c2
  end virtual
   if c1 <> 0
   DEBUGS _debug_str_+f2-c2
  end if
  virtual at 0
   ..f1 db _format,13,10,0
   ..lbl:
   __debug_strings fix __debug_strings,_debug_str_,_format,..lbl-..f1-1-c2
  end virtual
 end if
pushad
mov  edi,sys_delay
mov  eax,1
call sys_int
popad
}

;-----------------------------------------------------------------------------

macro jif _op1,_cond,_op2,_label,_op {
 if _op eq
  cmp _op1,_op2
 else
  if _op2 eq
   _op _op1
  else
   _op _op1,_op2
  end if
 end if
 j#_cond _label
}

macro m2m op1,op2 {
 pushd op2
 popd  op1
}

macro BITFIELD _bfnum,_size,[_name,_nbits] {
 common
  local _shift,_mask
  _shift = 0
  .bit_field#_bfnum rb _size
  if B_HOST eq HOST_BENDIAN
 forward
  _mask = 0
  repeat _nbits
   _mask = (_mask shl 1) or 1
  end repeat
  _name = _mask shl _shift
  _name#_shift = _shift
  _shift = _shift + _nbits
 common
  else
 forward
  _mask = 0
  repeat _nbits
   _mask = (_mask shl 1) or 1
  end repeat
  _name = _mask shl (_size-_shift-_nbits)
  _name#_shift = _size-_shift-_nbits
  _shift = _shift + _nbits
 common
  end if
}

;-----------------------------------------------------------------------------

sys_delay	=  5*4
sys_pci 	= 62*4
sys_debug_board = 63*4

;-----------------------------------------------------------------------------

func sys_int
begin
	pushad
	push	eax
	add	edi,[systlb]
	call	dword[edi]
	pop	eax
	popad
	ret
endf

func debug_outstr
begin
	mov	eax,1
    .l1:
	mov	bl,[edx]
	test	bl,bl
	jz	.l2
	mov	edi,sys_debug_board
	call	sys_int
	inc	edx
	jmp	.l1
    .l2:
	ret
endf

macro DEBUGS [_str] {
 common
  pushf
  pushad
  local ..str,..label,..is_str
  ..is_str = 0
 forward
  if _str eqtype ''
   ..is_str = 1
  end if
 common
  if ..is_str = 1
   jmp ..label
   ..str db _str,0
   ..label:
   mov  edx,..str
   call debug_outstr
  else
   mov  edx,_str
   call debug_outstr
  end if
  popad
  popf
}

macro DEBUGF [_str] {
 common
  DEBUGS _str,13,10
}

func debug_outchar
begin
	pushad
	mov	bl,al
	mov	eax,1
	mov	edi,sys_debug_board
	call	sys_int
	popad
	ret
endf

func debug_outdec
begin
	mov	ecx,10
	push	-'0'
    .l1:
	xor	edx,edx
	div	ecx
	push	edx
	test	eax,eax
	jnz	.l1
    .l2:
	pop	eax
	add	al,'0'
	jz	.l3
	call	debug_outchar
	jmp	.l2
    .l3:
	ret
endf

macro DEBUGD _dec {
 pushf
 pushad
 if _dec eqtype eax
  if _dec in <ebx,ecx,edx,esi,edi,ebp,esp>
   mov  eax,_dec
  else if _dec in <ax,bx,cx,dx,si,di,bp,sp,al,ah,bl,bh,cl,ch,dl,dh>
   movzx eax,_dec
  end if
 else
  if (_dec eqtype word[])|(_dec eqtype byte[])
   movzx eax,_dec
  else
   mov eax,_dec
  end if
 end if
 call debug_outdec
 popad
 popf
}

func debug_outhex
  __hexdigits db '0123456789ABCDEF'
begin
	mov	edx,8
    .l1:
	rol	eax,4
	push	eax
	and	eax,0x0000000F
	mov	al,[__hexdigits+eax]
	call	debug_outchar
	pop	eax
	dec	edx
	jnz	.l1
	ret
endf

macro DEBUGH _hex {
 pushf
 pushad
 if ~_hex eq eax
  mov  eax,_hex
 end if
 call debug_outhex
 popad
 popf
}

macro diff16 title,l1,l2
 {
  local s,d
  s = l2-l1
  display title,': 0x'
  repeat 8
   d = '0' + s shr ((8-%) shl 2) and $0F
   if d > '9'
    d = d + 'A'-'9'-1
   end if
   display d
  end repeat
  display 13,10
 }

macro diff10 title,l1,l2
 {
  local s,d,z,m
  s = l2-l1
  z = 0
  m = 1000000000
  display title,': '
  repeat 10
   d = '0' + s / m
   s = s - (s/m)*m
   m = m / 10
   if d <> '0'
    z = 1
   end if
   if z <> 0
    display d
   end if
  end repeat
  display 13,10
 }

;-----------------------------------------------------------------------------
;///// END ///////////////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
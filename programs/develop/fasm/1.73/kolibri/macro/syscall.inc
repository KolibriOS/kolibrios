macro reg_props type,size,[reg]
{
	typeof.#reg = type
	sizeof.#reg = size
}
?isReg32	equ in <eax,ebx,ecx,edx,esi,edi,ebp,esp>
?isReg32w8	equ in <eax,ebx,ecx,edx>
?isReg		equ eqtype eax
?isInteger	equ eqtype 0
?isFlatMem	equ eqtype []
?isSegMem	equ eqtype fs:[]
?isString	equ eqtype ''

macro madd reg,a1,a2 { add reg,(a1) shl 16 + (a2) }
macro msub reg,a1,a2 { sub reg,(a1) shl 16 + (a2) }

macro mcr_mov arg1*,arg2*
{
	if (arg1 ?isReg32) & ((arg2 ?isInteger) | (arg2 ?isString))
		if ~(arg2)
			xor arg1,arg1
		else if (arg2) = 1
			xor arg1,arg1
			inc arg1
		else if (arg2) = -1
			or  arg1,-1
		else
			mov arg1,arg2
		end if
	else
		mov arg1,arg2
	end if
}

macro mpack dest, hsrc, lsrc
{
	if (hsrc ?isInteger)
		if (lsrc ?isReg32)
			lea dest, [(hsrc) shl 16 + lsrc]
		else if (+lsrc ?isInteger) ;everething except memory opnd
			mov dest, (hsrc) shl 16 + lsrc
		else ;if (lsrc ?isFlatMem) | (lsrc ?isSegMem)
			mov dest, lsrc
			add dest, (hsrc) shl 16
		end if
	else
		mov dest, hsrc
		shl dest, 16
		add dest, lsrc
	end if
}

macro __mov reg,a,b {	    ; mike.dld
 match any,a \{
   if (~b eq)
     mpack reg,a,b
   else if ~(a eq reg)
     mcr_mov reg,a
   end if \}
}

macro regcall proc,a,b,c,d,e,f,g {
	__mov	eax,a
	__mov	ebx,b
	__mov	ecx,c
	__mov	edx,d
	__mov	esi,e
	__mov	edi,f
	__mov	ebp,g
	call	proc
}

macro reginvk proc,a,b,c,d,e,f,g {
	__mov	eax,a
	__mov	ebx,b
	__mov	ecx,c
	__mov	edx,d
	__mov	esi,e
	__mov	edi,f
	__mov	ebp,g
	call	[proc]
}



include 'config.inc'
;__CPU_type equ p5
SYSENTER_VAR	equ 0

macro mcallb a,b,c,d,e,f {   ; mike.dld, updated by Ghost for Fast System Calls
 local	..ret_point
 __mov ebx,b
 __mov ecx,c
 __mov edx,d
 __mov esi,e
 __mov edi,f

 if __CPU_type eq p5
	int	0x40
 else
  if __CPU_type eq p6
	push	ebp
	mov	ebp, esp
	push	..ret_point	; it may be 2 or 5 byte
	sysenter
 ..ret_point:
	pop	edx
	pop	ecx

  else
   if __CPU_type eq k6
	push	ecx
	syscall
	pop	ecx
   else
	display 'ERROR : unknown CPU type (set to p5)', 10, 13
	__CPU_type equ p5
	int	0x40
   end if
  end if
 end if
}

macro mcall a,b& {   ; mike.dld, updated by Ghost for Fast System Calls
	__mov	eax,a
	mcallb	,b
}
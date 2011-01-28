;// fast life generator: ~2.8 pixel*generation/tact

macro live_shl x,do_shl
{
	if do_shl eq yes
		psllq x,1
	end if
}

macro live_shr x,do_shr
{
	if do_shr eq yes
		psrlq x,1
	end if
}

macro live_mov  x,reg,how
{
	if how eq low
		xorps     x,x
		movlps    x,[reg+24]
	else if how eq high
		xorps     x,x
		movhps    x,[reg+edx]
	else
		movaps    x,[reg+ecx]
	end if
}

macro live_load x,y,z,t,shl_edi,shr_esi,how
{
	live_mov  y,edi,how
	live_mov  x,ebx,how
	live_shl  y,shl_edi
	movaps    t,y
	xorps     y,x
	live_mov  z,esi,how
	andps     x,t
	live_shr  z,shr_esi
	movaps    t,y
	xorps     y,z
	andps     t,z
	orps      x,t
}

macro live_operation a,A,b,B,c,C,d,D
{
	movaps    D,A
	xorps     A,B
	andps     D,B
	movaps    d,a
	xorps     a,D
	andps     d,D
	movaps    D,a
	xorps     a,b
	andps     D,b
	orps      d,D
	movaps    D,a
	xorps     a,c
	andps     D,c
	xorps     d,D
	xorps     a,d
	movaps    D,A
	orps      D,C
	xorps     A,C
	xorps     d,D
	orps      A,[ebx+ecx+16]
	andps     a,d
	andps     a,A
	movaps    [ebp+ecx],a
}

macro live_cycle shl_edi,shr_esi
{
	local cycle
	local cycle_entry
	mov  ecx,edx
	live_load      xmm2,xmm3,xmm4,xmm5,shl_edi,shr_esi,low
	live_load      xmm4,xmm5,xmm6,xmm7,shl_edi,shr_esi
	sub  ecx,eax
	jmp  cycle_entry
cycle:
	live_load      xmm4,xmm5,xmm6,xmm7,shl_edi,shr_esi
	live_operation xmm0,xmm1,xmm2,xmm3,xmm4,xmm5,xmm6,xmm7
	sub  ecx,eax
cycle_entry:
	live_load      xmm6,xmm7,xmm0,xmm1,shl_edi,shr_esi
	live_operation xmm2,xmm3,xmm4,xmm5,xmm6,xmm7,xmm0,xmm1
	sub  ecx,eax
	live_load      xmm0,xmm1,xmm2,xmm3,shl_edi,shr_esi
	live_operation xmm4,xmm5,xmm6,xmm7,xmm0,xmm1,xmm2,xmm3
	sub  ecx,eax
	live_load      xmm2,xmm3,xmm4,xmm5,shl_edi,shr_esi
	live_operation xmm6,xmm7,xmm0,xmm1,xmm2,xmm3,xmm4,xmm5
	sub  ecx,eax
	jg   cycle
	live_load      xmm4,xmm5,xmm6,xmm7,shl_edi,shr_esi,high
	live_operation xmm0,xmm1,xmm2,xmm3,xmm4,xmm5,xmm6,xmm7
}

OneGeneration_Flag12:
	push edi
	lea  esi,[eax+1]
	bt   dword [esp+48],1
	jnc  OneGeneration_flag2_end
	bt   dword [esp+48],3
	jc   OneGeneration_flag2_end
	mov  edi,[esp+36]
	shl  edi,4
	cmp  edi,edx
	jb   OneGeneration_flag2_uphalf
	sub  edi,edx
	cmp  edi,edx
	jnb  OneGeneration_flag2_end
	add  edi,8
OneGeneration_flag2_uphalf:
	mov  ecx,esi
	lea  edi,[edi+ebx+16]
	pxor mm0,mm0
OneGeneration_flag2_cycle:
	movq [edi],mm0
	add  edi,edx
	loop OneGeneration_flag2_cycle
OneGeneration_flag2_end:
	bt   dword [esp+48],0
	jnc  OneGeneration_flag1_end
	bt   dword [esp+48],2
	jc   OneGeneration_flag1_end
	push edx
	mov  eax,[esp+36]
	xor  edx,edx
	div  esi
	mov  esi,edx
	pop  edx
	cmp  eax,64
	jnb  OneGeneration_flag1_end
	imul esi,edx
	lea  esi,[esi+ebx+16]
	btr  eax,5
	jnc  OneGeneration_flag1_noadd4
	add  esi,4
OneGeneration_flag1_noadd4:
	lea  ecx,[edx-8]
	mov  edi,8
OneGeneration_flag1_cycle:
	btr  dword [esi+ecx],eax
	sub  ecx,edi
	btr  dword [esi+ecx],eax
	sub  ecx,edi
	btr  dword [esi+ecx],eax
	sub  ecx,edi
	btr  dword [esi+ecx],eax
	sub  ecx,edi
	jnl  OneGeneration_flag1_cycle
OneGeneration_flag1_end:
	pop  edi
	ret

@OneGeneration$qqsiipvpxvi:
	push ebp
	push ebx
	push esi
	push edi
	mov  eax,[esp+20]
	mov  edx,[esp+24]
	mov  ebp,[esp+28]
	mov  ebx,[esp+32]
	dec  eax
	jl   OneGeneration_end
	add  edx,7
	add  ebp,15
	dec  ebx
	shr  eax,6
	shl  edx,3
	and  ebp,not 15
	and  ebx,not 15
	and  edx,not 63
	jng  OneGeneration_end
	test eax,eax
	jz   OneGeneration_single
	mov  edi,edx
	imul edi,eax
	jo   OneGeneration_end
	push eax
	add  edi,ebx
	call OneGeneration_Flag12
	lea  esi,[ebx+edx]
	push dword [esp]
	mov  eax,16
	live_cycle yes,no
	jmp  OneGeneration_cycle_fin
OneGeneration_cycle:
	mov  edi,ebx
	mov  ebx,esi
	add  ebp,edx
	add  esi,edx
	live_cycle no,no
OneGeneration_cycle_fin:
	dec  dword [esp]
	jg   OneGeneration_cycle
	mov  edi,ebx
	pop  ecx
	mov  ebx,esi
	mov  esi,edx
	add  ebp,edx
	imul esi,[esp]
	neg  esi
	add  esi,ebx
	live_cycle no,yes
	jmp  OneGeneration_flag48
OneGeneration_single:
	push eax
	mov  edi,ebx
	call OneGeneration_Flag12
	mov  esi,ebx
	mov  eax,16
	live_cycle yes,yes
OneGeneration_flag48:
	pop  ebp
	inc  ebp
	bt   dword [esp+36],3
	jnc  OneGeneration_flag8_end
	mov  edi,[esp+24]
	mov  ebx,[esp+28]
	dec  edi
	add  ebx,15
	shl  edi,4
	lea  esi,[edi-16]
	and  ebx,not 15
	cmp  edi,edx
	jb   OneGeneration_flag8_uphalf
	sub  edi,edx
	add  edi,8
	cmp  esi,edx
	jb   OneGeneration_flag8_uphalf
	sub  esi,edx
	add  esi,8
OneGeneration_flag8_uphalf:
	mov  ecx,ebp
OneGeneration_flag8_cycle:
	movq mm0,[ebx+esi]
	movq [ebx],mm0
	movq mm0,[ebx+16]
	movq [ebx+edi],mm0
	add  ebx,edx
	loop OneGeneration_flag8_cycle
OneGeneration_flag8_end:
	bt   dword [esp+36],2
	jnc  OneGeneration_flag4_end
	mov  eax,[esp+20]
	push edx
	dec  eax
	xor  edx,edx
	mov  ebx,[esp+32]
	div  ebp
	add  ebx,15
	mov  esi,eax
	mov  edi,edx
	and  ebx,not 15
	dec  edx
	jl   OneGeneration_flag4_dec0
	mov  ebp,edx
	jmp  OneGeneration_flag4_after_dec
OneGeneration_flag4_dec0:
	dec  ebp
	dec  eax
OneGeneration_flag4_after_dec:
	pop  edx
	imul edi,edx
	imul ebp,edx
	add  edi,ebx
	add  ebp,ebx
	btr  esi,5
	jnc  OneGeneration_flag4_noadd4f
	add  edi,4
OneGeneration_flag4_noadd4f:
	btr  eax,5
	jnc  OneGeneration_flag4_noadd4s
	add  ebp,4
OneGeneration_flag4_noadd4s:
	mov  ecx,edx
	jmp  OneGeneration_flag4_cycle0_entry
OneGeneration_flag4_cycle0:
	btr  dword [ebx+ecx],0
OneGeneration_flag4_cycle0_entry:
	sub  ecx,8
	jl   OneGeneration_flag4_cycle0_end
	bt   dword [ebp+ecx],eax
	jnc  OneGeneration_flag4_cycle0
	bts  dword [ebx+ecx],0
	jmp  OneGeneration_flag4_cycle0_entry
OneGeneration_flag4_cycle0_end:
	xor  eax,eax
	cmp  dword [esp+20],64
	jng  OneGeneration_flag4_single
	add  ebx,edx
	jmp  OneGeneration_flag4_cycle1_entry
OneGeneration_flag4_single:
	inc  eax
	jmp  OneGeneration_flag4_cycle1_entry
OneGeneration_flag4_cycle1:
	btr  dword [edi+edx],esi
OneGeneration_flag4_cycle1_entry:
	sub  edx,8
	jl   OneGeneration_flag4_end
	bt   dword [ebx+edx],eax
	jnc  OneGeneration_flag4_cycle1
	bts  dword [edi+edx],esi
	jmp  OneGeneration_flag4_cycle1_entry
OneGeneration_flag4_end:
	emms
OneGeneration_end:
	pop  edi
	pop  esi
	pop  ebx
	pop  ebp
	ret  20


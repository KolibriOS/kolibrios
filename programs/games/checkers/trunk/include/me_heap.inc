;/***

MenuetHeapInit = @@Menuet@_HeapInit$qpvt1t1

MenuetHeapAlloc = @@Menuet@Alloc$qui

MenuetHeapReAlloc = @@Menuet@ReAlloc$qpvui

MenuetHeapFree = @@Menuet@Free$qpv

MenuetHeapFreeAndThreadFinish = @Menuet@_FreeAndThreadFinish$qpvpi

define @Menuet@_SetUseMemory$qui
  push ebx
  mov  eax,64
  mov  ebx,1
  mov  ecx,[esp+8]
  int  0x40
  pop  ebx
  test eax,eax
  jnz  Menuet_set_use_memory_nomem
  push ecx
  push dword [@Menuet@_MenuetMemBlock]
  call @@MemoryHeap@ResizeBlock$q20MemoryHeap@TMemBlockpv
  add  esp,8
  mov  al,1
  ret
Menuet_set_use_memory_nomem:
  xor  al,al
  ret
enddef

define @Menuet@_RecalculateUseMemory$qui
  mov  eax,dword [esp+4]
  mov  ecx,(U_END + 3) and not 3
  cmp  eax,ecx
  jna  Menuet_recalculate_use_memory_min
  push ebx
  sub  eax,ecx
  mov  ebx,6
  mul  ebx
  dec  ebx
  div  ebx
  add  eax,((U_END + 3) and not 3) + 3
  and  eax,not 3
  pop  ebx
  ret
Menuet_recalculate_use_memory_min:
  mov  eax,ecx
  ret
enddef

define @Menuet@_FreeAndThreadFinish$qpvpi
  mov  ebx,1
  mov  ecx,[esp+8]
  jmp  Menuet_heap_free_tf_wait
Menuet_heap_free_tf_wait_loop:
  mov  eax,5
  int  0x40
  shl  ebx,1
  cmp  ebx,MENUET_MUTEX_MAX_TIME_WAIT
  jna  Menuet_heap_free_tf_wait
  mov  ebx,MENUET_MUTEX_MAX_TIME_WAIT
Menuet_heap_free_tf_wait:
  cmp  dword [ecx],0
  jnz  @Menuet@ExitProcess$qv
  lock bts dword [@Menuet@_MemHeapMutex],0
  jc   Menuet_heap_free_tf_wait_loop
  push dword [esp+4]
  push @Menuet@_MenuetFreeSpace
  call @@MemoryHeap@Free$qr21MemoryHeap@TFreeSpacepv
  add  esp,8
  mov  byte [@Menuet@_MemHeapMutex],0x40
  or   eax,-1
  int  0x40
enddef

macro call func
{
  if func eq @MemoryHeap@_FirstNotZeroBit$qui
    bsf  eax,[esp]
  else if func eq @MemoryHeap@_CopyMemItemArray$quiuiui
    xchg edi,[esp]
    xchg esi,[esp+4]
    mov  ecx,[esp+8]
    cld
    sub  ecx,esi
    shr  ecx,2
    rep movs dword [edi],[esi]
    xchg edi,[esp]
    xchg esi,[esp+4]
  else
    call func
  end if
}

@$bnwa$qui = @@Menuet@Alloc$qui
@$bnew$qui = @@Menuet@Alloc$qui

@$bdla$qpv = @@Menuet@Free$qpv
@$bdele$qpv = @@Menuet@Free$qpv

define @_vector_new_ldtc_$qpvuiuiuit1uit1
.var_2C = -0Ch
.var_28 = -8
.var_24 = -4
.arg_0 = 8
.arg_4 = 0Ch
.arg_8 = 10h
.arg_C = 14h
.arg_10 = 18h
	push	ebp
	mov	ebp, esp
	add	esp, -0Ch
	push	ebx
	push	esi
	push	edi
	mov	edi, [ebp+.arg_10]
	mov	esi, [ebp+.arg_C]
	cmp	dword [ebp+.arg_0], 0
	jnz	.loc_10070
	mov	edx, [ebp+.arg_4]
	imul	edx, [ebp+.arg_8]
	test	esi, 10h
	jz	@f
	add	edx, 4
@@:
	push	edx
	call	@$bnwa$qui
	pop	ecx
	mov	[ebp+.arg_0], eax
	test	eax, eax
	jz	.ret
	mov	dword [ebp+.var_2C], 1
	jmp	@f
.loc_10070:
	and	dword [ebp+.var_2C], 0
@@:
	test	esi, 10h
	jz	@f
	mov	edx, [ebp+.arg_0]
	mov	ecx, [ebp+.arg_8]
	mov	[edx], ecx
	add	dword [ebp+.arg_0], 4
@@:
	mov	eax, [ebp+.arg_0]
	test	edi, edi
	jz	.ret
	mov	ecx, [ebp+.arg_8]
	mov	[ebp+.var_28], ecx
	mov	ebx, eax
	jmp	.loc_100EF
.loc_100B0:
	mov	eax, esi
	and	eax, 7
	dec	eax
	jz	.loc_100CB
	dec	eax
	jz	.loc_100D1
	dec	eax
	jz	.loc_100D6
	dec	eax
	dec	eax
	jz	.loc_100DC
	jmp	$
.loc_100CB:
	push	ebx
	call	edi
	pop	ecx
	jmp	.loc_100EC
.loc_100D1:
.loc_100DC:
	push	ebx
	call	edi
	jmp	.loc_100EC
.loc_100D6:
	mov	eax, ebx
	call	edi
.loc_100EC:
	add	ebx, [ebp+.arg_4]
.loc_100EF:
	dec	dword [ebp+.var_28]
	jns	.loc_100B0
	mov	eax, [ebp+.arg_0]
.ret:
	pop	edi
	pop	esi
	pop	ebx
	mov	esp, ebp
	pop	ebp
	ret
enddef

define @_vector_delete_ldtc_$qpvuiuiuit1
.arg_0 = 8
.arg_4 = 0Ch
.arg_8 = 10h
.arg_C = 14h
.arg_10 = 18h
	push	ebp
	mov	ebp, esp
	push	ebx
	push	esi
	push	edi
	mov	edi, [ebp+.arg_C]
	mov	esi, edi
	and	esi, 10h
	and	edi, 8
	cmp	dword [ebp+.arg_0], 0
	jz	.ret
	test	esi, esi
	jz	@f
	mov	ecx, [ebp+.arg_0]
	mov	eax, [ecx-4]
	mov	[ebp+.arg_8], eax
@@:
	mov	ebx, [ebp+.arg_8]
	dec	ebx
	imul	ebx, [ebp+.arg_4]
	add	ebx, [ebp+.arg_0]
	jmp	.loc_100D3
.loc_1008B:
	mov	eax, [ebp+.arg_C]
	and	eax, 7
	dec	eax
	jz	.loc_1009E
	dec	eax
	jz	.loc_100A9
	dec	eax
	jz	.loc_100B1
	dec	eax
	dec	eax
	jz	.loc_100BD
	jmp	$
.loc_1009E:
	push	2
	push	ebx
	call	dword [ebp+.arg_10]
	add	esp, 8
	jmp	.loc_100D0
.loc_100A9:
	push	2
	push	ebx
	call	dword [ebp+.arg_10]
	jmp	.loc_100D0
.loc_100B1:
	mov	edx, 2
	mov	eax, ebx
	call	dword [ebp+.arg_10]
	jmp	.loc_100D0
.loc_100BD:
	push	2
	push	ebx
	call	dword [ebp+.arg_10]
.loc_100D0:
	sub	ebx, [ebp+.arg_4]
.loc_100D3:
	dec	dword [ebp+.arg_8]
	jns	.loc_1008B
	test	esi, esi
	jz	@f
	sub	dword [ebp+.arg_0], 4
@@:
	test	edi, edi
	jz	@f
	push	dword [ebp+.arg_0]
	call	@$bdla$qpv
	pop	ecx
	jmp	.ret
@@:
	mov	eax, [ebp+.arg_0]
	jmp	.reteax
.ret:
	xor	eax, eax
.reteax:
	pop	edi
	pop	esi
	pop	ebx
	mov	esp, ebp
	pop	ebp
	ret
enddef

;/**/

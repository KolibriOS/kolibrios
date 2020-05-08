;testing speed of RSA coding (256 bit)
;for program CPUID (WildWest)
;author: halyavin

;structure of long number:
;rd 1 : length of number
;rd 39: digits of number
;length of zero number is equal to 1.

number_digits=39
number_size=4*(number_digits+1)

;calling convention:
;procedures can change eax,ecx,edx

init_rsatest:
;to do: add initialization which depends on processor.
  ret


set_zero:
;eax - address of number
;[eax]:=0
;result: eax isn't changed
  xor  ecx,ecx
  xor  edx,edx
  inc  edx
  mov  dword [eax],edx
  mov  edx,39
  add  eax,4
.loop:
  mov  dword [eax],ecx
  add  eax,4
  dec  edx
  jnz  .loop
  sub  eax,160 ;size of number
  ret

ladd:
;eax - first number
;edx - second number
;[eax]:=[eax]+[edx]
  pushad
  mov  ecx,dword [eax]
  mov  esi,dword [edx]
  mov  ebp,eax
  cmp  ecx,esi
  jg   .l1
  mov  ecx,esi
.l1:
  add  eax,4
  add  edx,4
  xor  ebx,ebx
  xor  esi,esi
  mov  edi,ecx
.l2:
  add  ebx,dword [eax]
  adc  ebx,dword [edx]
  adc  esi,esi
  mov  dword [eax],ebx
  mov  ebx,esi
  add  eax,4
  xor  esi,esi
  add  edx,4
  dec  ecx
  jnz  .l2
  test ebx,ebx
  jz   .lend
  inc  dword  [eax]
  inc  edi
.lend:
  mov  eax,ebp
  mov  dword [eax],edi
  popad
  ret

lsub:
;eax - first number
;edx - second number
;[eax]=[eax]-[edx]
;[eax]>=[edx]
  pushad
  mov  ecx,dword [eax]
  mov  edi,ecx
  mov  ebp,eax
  add  eax,4
  add  edx,4
  xor  ebx,ebx
  xor  esi,esi
.l1:
  mov  esi,dword [eax]
  sub  esi,ebx
  sbb  esi,dword [edx]
  mov  dword [eax],esi
  sbb  esi,esi
  mov  ebx,esi
  neg  ebx
  add  eax,4
  add  edx,4
  dec  ecx
  jnz  .l1
  dec  edi
  jz   .lend
  sub  eax,4
.l2:
  mov  ebx,dword [eax]
  test ebx,ebx
  jnz  .lend
  sub  eax,4
  dec  edi
  jnz  .l2
.lend:
  inc  edi
  mov  eax,ebp
  mov  [eax],edi
  popad
  ret

lcmp:
;eax - address of number
;edx - address of number
;result: ecx=1 if eax>edx
;        ecx=0 if eax=edx
;        ecx=-1 if eax<edx
  push	ebx
  mov	ebx,dword [eax]
  cmp	ebx,dword [edx]
  jg	.l1
  jl	.l2
  push	esi
  push	eax
  push	edx
  lea	eax,[eax+4*ebx]
  lea	edx,[edx+4*ebx]
.l3:
  mov	esi,dword [eax]
  cmp	esi,dword [edx]
  ja	.l4
  jb	.l5
  sub	eax,4
  sub	edx,4
  dec	ebx
  jnz	.l3
  xor	ecx,ecx
.lend:
  pop	edx
  pop	eax
  pop	esi
  pop	ebx
  ret
.l4:
  mov	ecx,1
  jmp	.lend
.l5:
  mov	ecx,-1
  jmp	.lend
.l1:
  mov	ecx,1
  pop	ebx
  ret
.l2:
  mov	ecx,-1
  pop	ebx
  ret

lshr:
;eax - number
;edx - shift
;[eax]=[eax] shr edx
  cmp  edx,32
  jl   .general
  mov  ecx,edx
  shr  ecx,5
  and  edx,31
  push esi
  mov  esi,ecx
  cmp  ecx,dword [eax]
  jng  .norm
  xor  esi,esi
  mov  ecx,dword [eax]
.norm:
  push edi
  push ebp
  mov  ebp,eax
  add  eax,4
.loop1:
  test esi,esi
  jz   .endloop1
  mov  edi,dword [eax+4*ecx]
  mov  dword [eax],edi
  add  eax,4
  dec  esi
  jmp  .loop1
.endloop1:
  mov  eax,ebp
  mov  esi,dword [eax]
  lea  eax,[eax+4*esi]
  xor  edi,edi
  mov  esi,ecx
.loop2:
  mov  dword [eax],edi
  sub  eax,4
  dec  ecx
  jnz  .loop2
  mov  eax,ebp
  sub  dword [eax],esi
  jz   .len0
.lend1:
  pop  ebp
  pop  edi
  pop  esi
  jmp  .general

.len0:
  inc dword [eax]
  jmp .lend1

.general:
  test edx,edx
  jz  .lend
  push ebx
  push esi
  push edi
  push ebp
  push eax
  mov  ebx,dword [eax]
  lea  eax,[eax+4*ebx]
  mov  ecx,edx
  sub  ecx,32
  neg  ecx
  mov  esi,1
  shl  esi,cl
  dec  esi
  mov  ecx,edx
  xor  ebp,ebp
.loop3:
  mov  edi,dword [eax]
  ror  edi,cl
  mov  edx,edi
  and  edx,esi
  or   edx,ebp
  mov  dword [eax],edx
  mov  edx,esi
  not  edx
  and  edi,edx
  mov  ebp,edi
  sub  eax,4
  dec  ebx
  jnz  .loop3
  pop  eax
  mov  ebx,dword [eax]
  mov  ecx,dword [eax+4*ebx]
  dec  ebx
  jz   .norm1
  test ecx,ecx
  jnz  .norm1
  dec  dword [eax]
.norm1:
  pop  ebp
  pop  edi
  pop  esi
  pop  ebx
.lend:
  ret

lshl:
;eax - number
;edx - shift
;[eax]:=[eax] shl edx
  cmp  edx,32
  jl   .general
  cmp  dword [eax],1
  jnz  .l1
  cmp  dword [eax+4],0
  jnz	.l1
  ret
.l1:
  push esi
  push ebx
  mov  ecx,edx
  shr  ecx,5
  and  edx,31
  mov  ebx,dword [eax]
  add  dword [eax],ecx
  lea  eax,[eax+4*ebx]
.loop1:
  mov  esi,dword [eax]
  mov  dword [eax+4*ecx],esi
  sub  eax,4
  dec  ebx
  jnz  .loop1
  xor  esi,esi
  lea  eax,[eax+4*ecx]
.loop2:
  mov  dword [eax],esi
  sub  eax,4
  dec  ecx
  jnz  .loop2
  pop  ebx
  pop  esi
  jmp  .general

.general:
  test edx,edx
  jz   .lend
  pushad
  mov  ecx,edx
  mov  ebx,1
  shl  ebx,cl
  dec  ebx
  mov  edx,dword [eax]
  xor  esi,esi
  lea  eax,[eax+4*edx]
.loop3:
  mov  edi,dword [eax]
  rol  edi,cl
  mov  ebp,edi
  and  ebp,ebx
  add  esi,ebp
  mov  dword [eax+4],esi
  mov  ebp,ebx
  not  ebp
  and  edi,ebp
  mov  esi,edi
  sub  eax,4
  dec  edx
  jnz  .loop3
  mov  dword [eax+4],esi
  mov  edx,dword [eax]
  cmp  dword [eax+4*edx+4],0
  jz   .l2
  inc  dword [eax]
.l2:
  popad
.lend:
  ret

lwrite_bit:
;eax - address of number
;edx - number of bit
;set bit to 1. First bit has number 0.
  mov  ecx,edx
  shr  edx,5
  inc  edx
  cmp  dword [eax],edx
  jg   .norm
  mov  dword [eax],edx
.norm:
  push ebx
  mov  ebx,1
  shl  ebx,cl
  or   dword [eax+4*edx],ebx
  pop  ebx
  ret

lcopy:
;eax - address of first number
;edx - address of second number
;[eax]:=[edx]
  push ebx
  push esi
  push eax
  push edx
  push dword [eax]
  mov  ebx,dword [edx]
  mov  ecx,ebx
  add  edx,4
  add  eax,4
.loop1:
  mov  esi,dword [edx]
  mov  dword [eax],esi
  add  eax,4
  add  edx,4
  dec  ecx
  jnz  .loop1
  pop  ecx
  sub  ecx,ebx
  jng  .l1
  xor  esi,esi
.loop2:
  mov  dword [eax],esi
  add  eax,4
  dec  ecx
  jnz  .loop2
.l1:
  pop  edx
  pop  eax
  mov  dword [eax],ebx
  pop  esi
  pop  ebx
  ret

lmul:
;eax - first number
;edx - second number
;ecx - third number
;[eax]:=[ecx]*[edx]
  push ecx
  push edx
  call set_zero
  pop  edx
  pop  ecx
  pushad
  sub  esp,16 ; local variables
  mov  ebp,edx
  mov  ebx,dword [edx]
  mov  dword [esp],ebx
  mov  ebx,dword [ecx]
  mov  dword [esp+4],ebx
  mov  ebx,eax
  add  ebp,4
  add  ecx,4
  add  ebx,4
  xor  esi,esi
  mov  dword [esp+8],esi
.loop1:
  xor  esi,esi
  mov  dword [esp+12],esi
.loop2:
  mov  eax,dword [esp+12]
  mov  eax,dword [ecx+4*eax]
  mov  edi,dword [esp+8]
  mov  edi,dword [ebp+4*edi]
  mul  edi
  add  eax,esi
  adc  edx,0
  mov  edi,dword [esp+12]
  add  edi,dword [esp+8]
  add  eax,dword [ebx+4*edi]
  adc  edx,0
  mov  dword [ebx+4*edi],eax
  mov  esi,edx
  inc  dword [esp+12]
  mov  eax,dword [esp+4]
  cmp  dword [esp+12],eax
  jnz  .loop2
  test esi,esi
  jz   .loop1end
  mov  eax,dword [esp+12]
  add  eax,dword [esp+8]
  mov  [ebx+4*eax],esi
.loop1end:
  inc  dword [esp+8]
  mov  eax,dword [esp]
  cmp  dword [esp+8],eax
  jnz  .loop1
  add  esp,16
  sub  ebx,4
  mov  ecx,39
  xor  edx,edx
.loop3:
  cmp  dword [ebx+4*ecx],edx
  jnz  .loop3end
  dec  ecx
  jnz  .loop3
.loop3end:
  test ecx,ecx
  jnz  .l1
  inc  ecx
.l1:
  mov  dword [ebx],ecx
  popad
  ret

lmod:
;eax - address of first number
;edx - address of second number
;[eax]:=[eax] mod [edx]
  mov  ecx,dword [eax]
  sub  ecx,dword [edx]
  jl   .lend
  pushad
  inc  ecx
  mov  ebp,eax
  mov  eax,edx
  mov  edi,eax
  shl  ecx,5
  mov  edx,ecx
  mov  esi,ecx
  call lshl
.loop:
  mov  eax,ebp
  mov  edx,edi
  call lcmp
  test ecx,ecx
  jl   .l1
  call lsub
.l1:
  dec  esi
  js   .endloop
  mov  eax,edx
  mov  edx,1
  call lshr
  jmp  .loop
.endloop:
  popad
.lend:
  ret

rsacode:
;eax - address of message
;edx - open key
;ecx - address of module
;esi - temparary number1
;edi - temparary number2
;esi - [eax]^edx mod (ecx)
;[eax] is changed.
  push eax
  push edx
  push ecx
  xchg eax,esi
  call set_zero
  mov  dword [eax+4],1
  mov  esi,eax
  mov  eax,edi
  call set_zero
.loop:
  mov  ecx,dword [esp+4]
  test ecx,1
  jz   .l1
  mov  eax,edi
  mov  edx,esi
  mov  ecx,dword [esp+8]
  call lmul
  mov  edx,dword [esp]
  call lmod
  mov  eax,esi
  mov  edx,edi
  call lcopy
.l1:
  shr  dword [esp+4],1
  jz   .endloop
  mov  edx,dword [esp+8]
  mov  ecx,edx
  mov  eax,edi
  call lmul
  mov  edx,dword [esp]
  call lmod
  mov  eax,dword [esp+8]
  mov  edx,edi
  call lcopy
  jmp  .loop
.endloop:
  pop  ecx
  pop  edx
  pop  eax
  ret

rsa_test:
  mov  eax,num1
  mov  dword [eax],16
  mov  ecx,1
  mov  edx,16
.loop:
  add  eax,4
  mov  dword [eax],ecx
  dec  edx
  jnz  .loop
  mov  eax,num1
  mov  ebx,[iter]
  mov  dword [eax+4],ebx
  inc  dword [iter]
  mov  edx,[openkey]
  mov  ecx,module
  mov  esi,num2
  mov  edi,num3
  call rsacode

  ret

init_test:
  xor  eax,eax
  mov  [iter],eax
  mov  [openkey],10007
  mov  eax,num2
  call set_zero
  mov  eax,num3
  call set_zero
  ret

module:
;module=p*q.
;where
;p=
;dd 9
;dd 147,2,3,4,5,6,7,8,9
;times 30 dd 0
;q=
;dd 9
;dd 163,4,5,6,7,8,9,10,11
;times 30 dd 0
  dd 17
  dd 23961,914,1232,1556,1887,2226,2574,2932,3301,288,308,314,305,280,238,178,99
  times 22 dd 0
;in decimal
;p=1042128803351525332193283695592003066548124262686459610611886292768993621049491
;q=1273712981880077616387281148672409277231717442781838063285512054053473668300963
;n=1327372985619988354987062708438042005329282516404896732667039640816200186465366322016844458439816997285872910403676793109807015096535910981266920474905959833

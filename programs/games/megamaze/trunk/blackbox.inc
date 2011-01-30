RAY_NUM equ   100000b
RAY_ABS equ  1000000b
RAY_REF equ 10000000b
BBSIZE   equ 10
ATOM_COUNT equ BBSIZE/2

CELL_EMPTY_CENTER equ 10100000b
CELL_ABSORBED     equ 10000000b
CELL_REFLECTED    equ 01000000b
CELL_EMPTY_EDGE   equ 11000000b
CELL_FULL_EDGE    equ 00100000b
MASK_ATOM         equ 00000001b
MASK_GUESS        equ 00000010b
MASK_ALL          equ 11100000b
MASK_DIR          equ 00000011b

BB_levelp:
		mcall 40,100111b
    mov  byte[fx],BBSIZE
    mov  byte[fy],BBSIZE
		and  [cur_step],0
		and  [pause_time],0
    call get_xy_sf.count
    call erase_field
    mov  edi,field+1
    mov  ecx,BBSIZE-2
    mov  eax,CELL_EMPTY_EDGE+1
    rep  stosb
    mov  eax,CELL_EMPTY_CENTER
    mov  ecx,BBSIZE-2
  .lp:
    push ecx
    mov  word[edi],(CELL_EMPTY_EDGE+3) shl 8+CELL_EMPTY_EDGE
    add  edi,2
    mov  ecx,BBSIZE-2
    rep  stosb
    pop  ecx
    loop .lp
    mov  word[edi],(CELL_EMPTY_EDGE+3) shl 8+CELL_EMPTY_EDGE
    add  edi,2
    mov  eax,CELL_EMPTY_EDGE+2
    mov  ecx,BBSIZE-2
    rep  stosb

    mov  ecx,BBSIZE/2
    mov  ebx,BBSIZE*BBSIZE
  .again:
    call get_rnd
    xor  edx,edx
    div  ebx
    cmp  byte[field+edx],CELL_EMPTY_CENTER
    jne  .again
    or   byte[field+edx],MASK_ATOM
    loop .again
    mov  [win_flag],5
    ret

BB_key:
		cmp  eax,-1
		je   BB_mouse
		cmp  eax,9
		jne  .ex
		xor  edx,edx
		xor  edi,edi
		mov  esi,field
		mov  ecx,BBSIZE*BBSIZE
	.lp:
		lodsb
		mov  ebx,eax
		and  eax,MASK_ALL;+MASK_ATOM
		cmp  eax,CELL_EMPTY_CENTER
		jne  .nxt
		cmp  ebx,CELL_EMPTY_CENTER+MASK_ATOM+MASK_GUESS
		jne  .nomatch
		add  dx,0x101
		jmp  .nxt
	.nomatch:
		test ebx,MASK_GUESS
		jz   .nxt
		inc  dh
	.nxt:
		loop .lp
		cmp  dh,ATOM_COUNT
		je   .chk
		mov  [win_flag],9
		jmp  .ex
	.chk:
		mov  [win_flag],10
		mov  ebx,(BBSIZE-2)*4
		mov  ecx,ATOM_COUNT
		sub  cl,dl
		imul  ecx,ATOM_COUNT
		sub  ebx,ecx
		sub  ebx,[pause_time]
		mov  [score],ebx
	.ex:
    ret

BB_drawm:
		mov  eax,47
		mov  esi,0x100000ff
		cmp  [win_flag],5
		jne  .no5
    mcall ,0x10000,ATOM_COUNT,<XFOFS+MSGXO,42>
	.no5:
		cmp  [win_flag],10
		jne  .no10
    mcall ,0x20000,[score],<XFOFS+MSGXO+12*8,42>
	.no10:
		cmp  [win_flag],9
		jne  .no9
    mcall ,0x10000,ATOM_COUNT,<XFOFS+MSGXO+11*8,42>
	.no9:
    mov  ecx,[cell_count]
  .lp:
  	push ecx
  	lea  eax,[ecx-1]
  	movzx ebx,byte[field+eax]
  	call get_xy
  	mov  eax,ebx
  	and  eax,MASK_ALL
  	cmp  eax,CELL_EMPTY_CENTER
  	jne  .nocenter
  	test ebx,MASK_ATOM
  	jz   .noatom
  	cmp  [win_flag],10
  	jne  .noatom
  	mov  edx,0xff0000
  	push ebx
  	mcall 13,[lx],[ly]
  	pop  ebx
  .noatom:
  	test ebx,MASK_GUESS
  	jz   .nxtlp
  	add  [lx],2 shl 16-4
  	add  [ly],2 shl 16-4
  	mov  edx,0xffff00
  .bar:
  	mcall 13,[lx],[ly]
  .nxtlp:
  	pop  ecx
  	loop .lp
  .ex2:
    ret
  .nocenter:
  	mov  edx,0xff
  	cmp  eax,CELL_ABSORBED
  	je   .bar
  	mov  edx,0x8000
  	cmp  eax,CELL_REFLECTED
  	je   .bar
  	cmp  eax,CELL_FULL_EDGE
  	jne  .nxtlp
  	and  ebx,11111b
  	mov  ecx,ebx
  	push word[ly+2]
  	pop  word[lx]
  	mov  eax,[Ces]
  	shr  eax,1
  	add  word[lx],ax
  	add  word[lx+2],ax
  	sub  [lx],8 shl 16+6
  	mcall 47,0x20000,,[lx],0x10000000
  	jmp  .nxtlp

corners db 0,BBSIZE-1,BBSIZE*BBSIZE-1,BBSIZE*(BBSIZE-1)

BB_mouse:
		cmp  [win_flag],1
		je   .ex
    mov  [jump],still
    mov  edx,eax
		call get_last_mclick
		mov  eax,ebx
		mov  edi,corners
		mov  ecx,4
		repne scasb
		je   .ex
		movzx ebx,byte[field+eax]
		mov  ecx,ebx
		and  ecx,MASK_ALL
		cmp  ecx,CELL_EMPTY_CENTER
		jne  .nocenter
		xor  byte[field+eax],MASK_GUESS
    mov  [win_flag],11
		jmp  .red
	.nocenter:
		cmp  ecx,CELL_EMPTY_EDGE
		jne  .ex

		mov  [player],eax
		and  ebx,MASK_DIR
	.step:
		mov  [finish],eax
		add  eax,[dirs+ebx*4] ; 1 step
		mov  [stepptr],eax
		movzx edx,byte[field+eax] ; cell ahead - edx
		and  edx,MASK_ALL+MASK_ATOM
		cmp  edx,CELL_EMPTY_CENTER+MASK_ATOM
		jne  .noabsorb
		mov  eax,[player]
		mov  byte[field+eax],CELL_ABSORBED
		inc  [pause_time]
    mov  [win_flag],7
		jmp  .red
	.noabsorb:
		lea  ecx,[ebx-2]
		and  ecx,MASK_DIR
		add  eax,[dirs+ecx*4]
		movzx edx,byte[field+eax] ; cell aside - edx
		xor  ecx,11b
		and  edx,MASK_ALL+MASK_ATOM
		cmp  edx,CELL_EMPTY_CENTER+MASK_ATOM
		jne  .noreflect1
	.reflect:
		mov  eax,[finish]
		cmp  eax,[player]
		je   .endref
		mov  ebx,ecx ;new direction
		jmp  .nextstep
	.noreflect1:
		mov  eax,[stepptr]
		add  eax,[dirs+ecx*4]
		movzx edx,byte[field+eax] ; cell aside - edx
		xor  ecx,11b
		and  edx,MASK_ALL+MASK_ATOM
		cmp  edx,CELL_EMPTY_CENTER+MASK_ATOM
		je   .reflect
	.noreflect2:
		mov  eax,[stepptr]
		movzx edx,byte[field+eax]
		and  edx,MASK_ALL
		cmp  edx,CELL_EMPTY_CENTER
		je   .nextstep

		cmp  eax,[player]
		je   .endref
		mov  ebx,[cur_step]
		add  ebx,CELL_FULL_EDGE
		mov  byte[field+eax],bl
		mov  eax,[player]
		mov  byte[field+eax],bl
		inc  [cur_step]
    mov  [win_flag],6
		add  [pause_time],2
		jmp  .red
	.endref:
		mov  eax,[player]
		mov  byte[field+eax],CELL_REFLECTED
    mov  [win_flag],8
 		inc  [pause_time]
		jmp  .red
	.nextstep:
		jmp  .step

	.red:
    mov  [jump],drw
	.ex:
		ret

BB_level:
	db 0

if lang eq ru
	BB_help mstr \
	'Ваша цель состоит в том, чтобы обнаружить',\
	'положение пяти атомов в черном ящике, посылая',\
	'лучи света в коробку и наблюдая, где лучи',\
	'появляются. Атом или поглотит луч, если луч',\
	'упадет на него в упор, или отклонит луч на 90',\
	'градусов, если он пройдет по смежному ряду или',\
	'колонке). Могут произойти 2 отклонения, вынуждая',\
	'луч возвратиться во входной точке,- это считается',\
	'отражением. Если желаете проверить себя и узнать',\
	'счет, отметьте 5 клеток желтым цветом и нажмите',\
	'TAB. Пять скрытых атомов будут показаны красными',\
	'квадратиками.',\
	' Луч поглощен - синий квадрат;',\
	' Луч отражен  - зеленый квадрат.',\
	' Числами отмечены точки входа и выхода лучей.','',\
	'http://puzzleprograms.narod.ru'
else
	BB_help mstr \
	'Your goal is to discover the whereabouts of five',\
	'atoms in a black box by sending rays of light',\
	'into the box and observing if and where the rays',\
	'emerge. An atom will either absorb a ray if the',\
	'ray hits it head on, or deflect a ray through 90',\
	'if it comes close (passes through an adjacent row',\
	'or column). Two deflections can occur at the same',\
	'time forcing a ray to return and emerge at its',\
	'entry point, this is referred to as reflection.','',\
	'The challenge is to pin point the location of all',\
	'atoms using the least number of rays. If you wish',\
	'to commit your solution and to discover your',\
	'score you must have exactly five guesses marked,',\
	'then press TAB. The five hidden atoms will appear',\
	'as red squares.','',\
	'http://puzzleprograms.narod.ru'
end if

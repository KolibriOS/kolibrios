; level format
; [fx|fy]..[field cells x2]..[worm_count]..[worm_len][start cell #][body dirs x2]

; internal format
; [stepptr]= worms #
; area: [worm_len][start_cell #][end_cell #]..[dirs]..
; [cur_step]: dragged worm pointer
; [finish]: 0 - if head dragged, 1- if tail

OR_levelp:
		mov  [Ces],40
		mcall 40,100111b
		inc  [drag_flag]
		mov  eax,OR_strip
		call need_image
    inc  esi
    movzx ecx,byte[esi]
    inc  esi
    push esi
    mov  esi,OR_maze
    jecxz .sk
  .nxtmz:
    movzx eax,byte[esi]
    add  esi,eax
    loop .nxtmz
  .sk:
    call get_xy_sf
    call unpack_level
    pop  esi
    movzx ecx,byte[esi]
    inc  esi
    mov  [cur_step],ecx
    jecxz .sk2
    mov  edi,area
    rep  movsb
  .sk2:
    mov  ecx,[cell_count]
    mov  edi,area+16
    rep  movsb
    mov  [stepptr],2
;    call erase_field
;    ud2
    ret

OR_key:
		cmp  eax,0
		jl   OR_mouse
    ret

OR_drawf:
    mov  ecx,[cell_count]
    push [sq_size]
		mov  [sq_size],0
  .lp:
    push ecx
    movzx ebx,byte[area+16+ecx-1]
    cmp  bl,0xf
    je   .no
    lea  eax,[ecx-1]
    call out_image
  .no:
    pop  ecx
    loop .lp
		pop  [sq_size]
		ret

OR_drawm:
    push [sq_size]
		mov  [sq_size],0
    mov  eax,[player]
    mov  ebx,[stepptr]
    add  ebx,8
    call out_image
    mov  ecx,[cur_step]
    jecxz .skipb
    mov  ebx,7
		mov  esi,area
		xor  eax,eax
	.nxtb:
		lodsb
		call out_image
		loop .nxtb
	.skipb:
		mov  eax,[player]
	.again:
    mov  ebx,[stepptr]
    call check_move
    jc   .ex
		add  eax,[dirs+ebx*4]
		cmp  byte[area+16+eax],6
		je   .ex
		mov  ecx,[cur_step]
		jecxz .skbchk
		mov  edi,area
		repne scasb
		je   .again
	.skbchk:
		mov  [sq_size],1
		push eax
    call get_xy
		mov  ebx,[lx]
		sub  bx,2
		mov  ecx,[ly]
		mov  cx,3
		mcall 13,,,0xFFC633
		xchg bx,cx
		mcall
		mov  esi,[Ces]
		sub  esi,3+2
		shl  esi,16
		push ebx
		add  ebx,esi
		mcall
		pop  ebx
		add  ecx,esi
		xchg bx,cx
		add  bx,2
		mcall
		pop  eax
		jmp  .again
	.ex:
		pop  [sq_size]
    ret

OR_mouse:
		cmp  [win_flag],1
		je   .ex
    mov  [jump],still
    mov  edx,eax
		call get_last_mclick
    cmp  edx,-2
    jne  .ex
		test eax,eax
		jnz  .noempty
	.drw:
    mov  [jump],drw
    ret
	.noempty:

; First Click at ebx cell
		mov  eax,[player]
    cmp  eax,ebx
    je   OR_drawf
		mov  ecx,[cur_step]
		jecxz .skbchk
		mov  edi,area
		xchg eax,ebx
		repne scasb
		xchg eax,ebx
		je   OR_drawf
	.skbchk:
  	cmp  byte[area+16+ebx],0xf
  	je   .ex
  	cmp  byte[area+16+ebx],6
  	je   .ex
    push eax
    mov  edx,ebx
    call get_offset
    mov  ebx,eax
    pop  eax
    cmp  ebx,[stepptr]
    jne  .ex

    call check_move
    jc   .ex
    movzx ecx,byte[area+16+edx]
		cmp  ecx,4
		jae  .noarr
		mov  ebx,ecx
		call check_move
		jc   .ex
		mov  edx,eax
		add  eax,[dirs+ebx*4]
		mov  [player],eax
		mov  ecx,[cur_step]
		jecxz .skbchk2
		mov  edi,area
		repne scasb
		jne  .skbchk2
		mov  [edi-1],dl
	.skbchk2:
		cmp  eax,[finish]
		jne  .drw
		mov  [win_flag],1
    jmp  .drw
  .noarr:
  	mov  edx,[stepptr]
  	cmp  ecx,4
  	jne  .nocw
  	movzx edx,byte[dir_rotate+edx]
  	jmp  .ex2
  .nocw:
  	cmp  ecx,5
  	jne  .ex
  	movzx edx,byte[dir_rotate+4+edx]
  .ex2:
  	mov  [stepptr],edx
    jmp  .drw
	.ex:
		ret

OR_maze:
file  'omaze.bin'

OR_level:
file 'or2.bin'

if lang eq ru
	OR_help mstr \
	'Щелчок на выделенной клетке передвигает или',\
	'вращает красную стрелку. Синий блок действует',\
	'как препятствие, скрывая одну клетку. При',\
	'переходе стрелки на клетку синего блока они',\
	'поменяются местами. Задача - провести красную',\
	'стрелку из нижней клетки в верхнюю.','',\
	'http://www.clickmazes.com'
else
	OR_help mstr \
	'Click on a highlighted square to move or rotate',\
	'the red pointer. The blue switcher acts as an',\
	'obstacle, hiding one square. The red pointer and',\
	'blue switcher will swap places if the red pointer',\
	'is pushed onto a square occupied by a blue',\
	'switcher. Your challenge is to negotiate the red',\
	'arrow from the bottom to the top square.','',\
	'http://www.clickmazes.com'
end if

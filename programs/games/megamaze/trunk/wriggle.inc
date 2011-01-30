; level format
; [fx|fy]..[field cells x2]..[worm_count]..[worm_len][start cell #][body dirs x2]

; internal format
; [stepptr]= worms #
; area: [worm_len][start_cell #][end_cell #]..[dirs]..
; [cur_step]: dragged worm pointer
; [finish]: 0 - if head dragged, 1- if tail
WG_levelp:
		mcall 40,100111b
		inc  [drag_flag]
    call get_xy_sf
    sub  esi,2
    call LP_levelp.bit2
    cmp edx,4
    jne  .ok
    dec  esi
  .ok:
    movzx ecx,byte[esi]
    mov  [stepptr],ecx
    inc  esi
    xor  eax,eax
    mov  edi,area
  .lp1:
    push ecx
    movzx ecx,byte[esi]
    inc  esi
    mov  [edi],cl
    inc  edi
    lodsb
		dec  ecx
    movzx ebx,al
    stosb
    push edi
    inc  edi
    mov  edx,8/2
    lodsb
  .lp:
  	rol  al,2
  	push ecx
  	mov  cl,al
  	and  ecx,11b
  	add  ebx,[dirs+ecx*4]
  	mov  [edi],cl
  	pop  ecx
  	inc  edi
  	dec  edx
  	test edx,edx
  	jnz  .nxt
    mov  edx,8/2
		lodsb
	.nxt:
		loop .lp
    cmp edx,4
    jne  .ok2
    dec  esi
  .ok2:
		pop  ecx
		mov  [ecx],bl
    pop  ecx
    loop .lp1
    mov  esi,[stepptr]
    ret

WG_key:
		cmp  eax,0
		jl   WG_mouse
    ret

WG_drawm:
		mov  [win_flag],1
    mov  ecx,[cell_count]
;		mov  [sq_size],3
  .lp:
    push ecx
    movzx ebx,byte[field+ecx-1]
    and  byte[field+ecx-1],11000000b
    shr  ebx,6
    test ebx,ebx
    jz   .no
    dec  ebx
    lea  eax,[ecx-1]
    call get_xy
    add  [lx],5-2 shl 16
    add  [ly],5-2 shl 16
    mov  edx,[f_colors+ebx*4]
    mcall 13,[lx],[ly]
  .no:
    pop  ecx
    loop .lp

		add  [sq_size],3

    mov  ecx,[stepptr]
    xor  eax,eax
    mov  esi,area
    push ebp
    mov  edi,w_colors
    mov  dword[player],1
  .lp2:
    push ecx
    movzx ecx,byte[esi]
    inc  esi
    xor  eax,eax
    lodsb
    mov  ebp,esi
  .lp1:
    inc  esi
    push ecx eax
    call get_xy
    mcall 13,[lx],[ly],[edi]
		pop  eax
    mov  ecx,[player]
    mov  bl,[field+eax]
    shr  bl,6
    add  [field+eax],cl
    cmp  bl,cl
    je   .match
    and  [win_flag],0
  .match:
		pop  ecx
		push esi edi
    movzx ebx,byte[esi]
		mov  esi,eax
    mov  [ebp],al
    add  eax,[dirs+ebx*4]

    cmp  ecx,1
    je   .skip
    mov  edi,eax
    call getline
    call bold_line
  .skip:
    pop  edi esi
		loop .lp1
		pop  ecx
		add  edi,4
		inc  dword[player]
		loop .lp2
		pop  ebp
		sub  [sq_size],3

    ret

WG_mouse:
		cmp  [win_flag],1
		je   .ex
    mov  [jump],still
    mov  edx,eax
		call get_last_mclick
    cmp  edx,-2
    jne  .no1st
		test eax,eax
		jz   .ex
	.noempty:

; First Click at ebx cell
;    dpd  ebx
    mov  [cur_step],0
    mov  dword[finish],0
    mov  esi,area
    mov  ecx,[stepptr]
  .fndlp:
    movzx edx,byte[esi]
    cmp  bl,[esi+1]
    je   .fnd
    cmp  bl,[esi+2]
    jne  .nxt
    inc  dword[finish]
  .fnd:
    mov  [cur_step],esi
    mov  [jump],drw;red
    jmp  .ex
  .nxt:
  	lea  esi,[esi+edx+2]
  	loop .fndlp
    jmp  .ex

  .no1st:
		test eax,eax
		jz   .ex
; While dragging
    mov  esi,[cur_step]
		test esi,esi
		jz   .ex
    lea  edi,[esi+1]
    add  edi,[finish]
		movzx eax,byte[edi]
		cmp  eax,ebx
		je   .ex
		push ebx
		cmp  dword[finish],0
		jne  .noswap
		xchg eax,ebx
	.noswap:
		call get_offset
		cmp  ebx,1
		pop  ebx
		jne  .ex
		cmp  eax,-1
		je   .ex
		test byte[field+ebx],11b
		jnz  .ex
		movzx ecx,byte[esi]
		cmp  dword[finish],0
		jne  .tail
		mov  [esi+1],bl
		mov  [esi+2],al
		add  esi,ecx
		std
		lea  edi,[esi+1]
		rep  movsb
		jmp  .redex

	.tail:
		mov  [esi+2],bl
		movzx ebx,byte[esi+3]
		movzx edx,byte[esi+1]
		add  edx,[dirs+ebx*4]
		mov  [esi+1],dl
		add  esi,3
		lea  edi,[esi-1]
		rep  movsb
		mov  [edi-1],al
;		ud2
	.redex:
		cld
    mov  [jump],drw;red
    mov  esi,area
	.ex:
		ret

WG_level:
file 'wriggle.bin'

if lang eq ru
	WG_help mstr \
	'Ваша задача - тянуть червяков за голову или',\
	'хвост, пока каждый из них не покроет клетки',\
	'своего цвета, оставив незакрытыми белые квадраты.',\
	'Червяки не могут перекрывать и пересекать друг',\
	'друга.','',\
	'http://www.clickmazes.com'
else
	WG_help mstr \
	'Your aim is to drag the wriggley worms by head or',\
	'tail until each worm covers the squares of its',\
	'own colour leaving only white squares exposed.',\
	'Worms cannot cross or overlap.','',\
	'http://www.clickmazes.com'
end if

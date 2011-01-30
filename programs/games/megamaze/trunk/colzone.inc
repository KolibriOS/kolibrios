; level format
; [fx|fy]..[field cells x2]..[worm_count]..[worm_len][start cell #][body dirs x2]

; internal format
; [stepptr]= worms #
; area: [worm_len][start_cell #][end_cell #]..[dirs]..
; [cur_step]: dragged worm pointer
; [finish]: 0 - if head dragged, 1- if tail
CZ_levelp:
    call get_xy_sf
    call LP_levelp.bit2
    mov  dword[area],5;0
    ret

CZ_key:
		mov  [jump],still
    cmp  eax,176
    jb   .ex
    cmp  eax,179
    ja   .ex
    mov  ecx,dword[area]
    lea  ebx,[eax-176]
    mov  eax,[player]
    call check_bounds
    je   .ex
    cmp  ecx,5
    jne  .novert
  .vert:
		add  eax,[dirs+ebx*4]
		call check_bounds
    je   .ex
		mov  ecx,eax
		add  ecx,[dirs+ebx*4]
		mov  cl,[field+ecx]
		cmp  cl,[field+eax]
		jne  .ex
		mov  dword[area],ebx
  .ok:
    mov  [player],eax
    cmp  eax,[finish]
    jne  .jm
    cmp  dword[area],5
    jne  .jm
    mov  [win_flag],1
  .jm:
    mov  [jump],drw
  .ex:
    ret
  .novert:
    mov  edx,ebx
    cmp  edx,ecx ; the same dir
    jne  .nosame
    add  eax,[dirs+ebx*4]
    call check_bounds
    je   .ex
  .set5:
    mov  dword[area],5
    add  eax,[dirs+ebx*4]
    jmp  .ok
  .nosame:
    xor  edx,11b
    cmp  edx,ecx ; the opposite dir
    je   .set5
    add  eax,[dirs+ebx*4]
    mov  ebx,ecx
    xor  ecx,11b
    add  eax,[dirs+ecx*4]
    jmp  .vert

CZ_drawm:
    mov  ecx,[cell_count]
		mov  esi,field
		mov  [sq_size],3
  .lp:
    push ecx
    movzx ebx,byte[field+ecx-1]
    shr  ebx,6
    lea  eax,[ecx-1]
    call get_xy
    add  [lx],5-2 shl 16
    add  [ly],5-2 shl 16
    mov  edx,[f_colors+ebx*4]
    mcall 13,[lx],[ly]
  .no:
    pop  ecx
    loop .lp

		inc  [sq_size]
		mov  eax,[player]
    call get_xy
    xor  edx,edx
    cmp  dword[area],5
    je   .vert
    push [lx]
    push [ly]
    mov  edx,dword[area]
    add  eax,[dirs+edx*4]
    call get_xy
    mcall 13,[lx],[ly]
    pop  [ly]
    pop  [lx]
  .vert:
    mcall 13,[lx],[ly]
		add  [sq_size],5
		mov  eax,[finish]
    call get_xy
    mcall 13,[lx],[ly],0xffffff

    ret


CZ_level:
file 'colzone.bin'

if lang eq ru
	CZ_help mstr \
	'Черный блок из 2 кубиков стоит на раскрашенной',\
	'клетчатой доске. Ваша задача - перекатить его на',\
	'клетку с белым квадратиком. Блок начинает стоя и',\
	'должен закончить тоже в вертикальном положении.',\
	'Нельзя выкатывать его за пределы сетки и сверх',\
	'того - кубики блока должны всегда ложиться на',\
	'клетки одинакового цвета. Блок не отображен в 3D,',\
	'поэтому Вы видите лишь его проекцию.','',\
	'http://www.clickmazes.com'
else
	CZ_help mstr \
	'A black block, made of two black cubes stuck',\
	'together, sits on a coloured grid. Your challenge',\
	'is to roll the black block to its target',\
	'position, which is marked with a small white',\
	'square. The block starts on one end (vertical)',\
	'and must also finish in this position. The block',\
	'is not permitted to roll off the grid and, in',\
	'addition, the block must always lie entirely',\
	'within one colour-zone. Note the block is not',\
	'displayed in 3D, in effect you see only its',\
	'shadow.','',\
	'http://www.clickmazes.com'
end if

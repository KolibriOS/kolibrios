FH_levelp:
		mcall 40,100111b
    call get_xy_sf
    call erase_field
    sub  esi,2
    mov  [levptr],esi
    movzx ecx,byte[esi-2]
    sub  ecx,2
    xor  eax,eax
  .lp:
    lodsb
    mov  byte[field+eax],-1
    loop .lp
    mov  dword[player],-1
    mov  [win_flag],3
    ret

FH_key:
		cmp  eax,-1
		je   FH_mouse
		cmp  eax,8
		jne  .nobsp
		cmp  [stepptr],0
		jne  .no1st
		mov  [jump],run.game
		ret
	.no1st:
		mov  [win_flag],0
		sub  [cur_step],2
		mov  esi,[cur_step]
		mov  edi,[finish]
		mov  eax,3
		movzx ebx,byte[esi]
		sub  eax,ebx
		movzx ecx,byte[esi+1]
	.lp0:
		and  byte[field+edi],0
		add  edi,[dirs+eax*4]
		loop .lp0
		mov  [finish],edi
		dec  [stepptr]
		ret
	.nobsp:
    cmp  eax,176
    jb   .ex
    cmp  eax,179
    ja   .ex
    sub  eax,176
    jmp  FH_mouse.fromkey
  .ex:
    ret

FH_drawm:
    mov  ecx,[cell_count]
  .lp:
    push ecx
    movzx ebx,byte[field+ecx-1]
    test ebx,ebx
    jz   .no
    lea  eax,[ecx-1]
    call get_xy
    mov  edx,0
    cmp  ebx,0xf
    jne  .stable
    mov  edx,0x909090
  .stable:
    mcall 13,[lx],[ly]
  .no:
    pop  ecx
    loop .lp

    mov  eax,[player]
    cmp  eax,-1
    je   .ex
    call get_xy
    mcall 13,[lx],[ly],0xff00
    mov  eax,[finish]
    call get_xy
    mcall 13,[lx],[ly],0xff0000

    mov  ecx,[stepptr]
    jecxz .ex
    mov  ebx,area
    mov  edi,[player]
    mov  edx,0xffffff
  .lp3:
    push ecx
    mov  esi,edi
    movzx eax,byte[ebx]
    movzx ecx,byte[ebx+1]
    add  ebx,2
  .lp2:
    add  edi,[dirs+eax*4]
    loop .lp2
		call getline
    pop  ecx
    loop .lp3
  .ex:
    ret

FH_mouse:
    mov  [jump],drw;red
		call get_last_mclick
		cmp  byte[field+ebx],0
		jne  .ex
		cmp  dword[player],-1
		jne  .nostart
		mov  dword[player],ebx
		mov  byte[field+ebx],0xf
		mov  [finish],ebx
		and  [win_flag],0
    mov  [cur_step],area
    and  [stepptr],0
    ret
	.nostart:
		mov  eax,[finish]
		call get_offset
		cmp  eax,-1
		je   .ex
	.fromkey:
		xor  ecx,ecx
		mov  edx,eax
		mov  esi,[finish]
	.lp:
		add  esi,[dirs+edx*4]
		cmp  byte[field+esi],0
		jnz  .exlp
		mov  eax,[finish]
		mov  ebx,esi
		call get_offset
		cmp  eax,edx
		jne  .exlp
		mov  byte[field+esi],0xf
		inc  ecx
		mov  [finish],esi
		jmp  .lp

	.exlp:
		jecxz .ex
		mov  eax,edx
		mov  edi,[cur_step]
		stosb
		mov  [edi],cl
		inc  edi
		mov  [cur_step],edi
		inc  [stepptr]
		mov  edi,field
		mov  ecx,[cell_count]
		xor  eax,eax
		repne scasb
		je   .ch_deadend
		mov  [win_flag],1
	.ex:
		ret
	.ch_deadend:
		mov  ecx,4
		mov  edx,dirs
	.lpp:
		mov  eax,[finish]
		mov  ebx,[edx]
		add  ebx,eax
		push ebx
		call get_offset
		pop  ebx
		cmp  eax,-1
		je   .nxtlp
		cmp  byte[field+ebx],0
		jz   .ex
	.nxtlp:
		add  edx,4
		loop .lpp
		mov  [win_flag],4
		ret

FH_level:
file 'fhouse.bin'

if lang eq ru
	FH_help mstr \
	'В задании найдите начальную клетку и проведите',\
	'линию, которая проходит через пустые клетки по',\
	'одному разу.',\
	' Каждый раз красный квадрат двигается прямо,',\
	'останавливаясь только у края, перед черными',\
	'квадратами или уже посещенными клетками.','',\
	'http://www.clickmazes.com'
else
	FH_help mstr \
	'For each puzzle find the starting square and draw',\
	'a path moving horizontally and vertically that',\
	'passes through each open square exactly once.','',\
	'At each move the red runner will always run',\
	'straight, as far as possible, stopping only when',\
	"blocked by the grid's edge, a black square, or a",\
	'square already visited.','',\
	'http://www.clickmazes.com'
end if

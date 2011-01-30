LP_levelp:
		mcall 40,100111b
		inc  [drag_flag]
		lodsb
		lodsw
    mov  byte[fx],al
    mov  byte[fy],ah
		and  dword[player],0
		and  dword[finish],0
    call get_xy_sf.count
    call erase_field
;    mov  [levptr],esi
	.bit2:
    mov  edx,8/2
    mov  edi,field
    lodsb
    mov  ecx,[cell_count]
  .lp:
  	mov  bl,al
  	rol  al,2
  	and  bl,11000000b
  	mov  [edi],bl
  	inc  edi
  	dec  edx
  	test edx,edx
  	jnz  .nxt
    mov  edx,8/2
		lodsb
	.nxt:
		loop .lp
    ret

LP_key:
		cmp  eax,0
		jl   LP_mouse
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
  .ex:
    ret

LP_drawm:
		and  [pause_time],0
    mov  ecx,[cell_count]
  .lp:
    push ecx
    mov  bl,byte[field+ecx-1]
    push ebx
    lea  eax,[ecx-1]
    mov  ecx,eax
    call get_xy

    mov  al,bl
    and  al,0xf
    test bl,11000000b
    jnz  .noempty
    mov  esi,LP_aw
    mov  ecx,11
    call LP_draw_alert
    jmp  .no
  .noempty:
    mov  edx,0
    test bl,10000000b
    je   .stable
    mov  edx,0xffffff
    mov  esi,LP_aw
    mov  ecx,7
    jmp  .d13
  .stable:
    mov  esi,LP_em
    mov  ecx,9
	.d13:
    call LP_draw_alert
    mcall 13,[lx],[ly]
  .no:
  	mov  esi,[esp+4]
  	dec  esi
  	mov  edi,esi
  	mov  edx,0x0000FF
  ; ecx - cell #
  	test byte[esp],10b
  	je   .no_dn
  	add  edi,[dirs+4*1]
  	call getline
  .no_dn:
  	test byte[esp],1000b
  	je   .no_right
  	mov  edi,esi
  	add  edi,[dirs+4*3]
  	call getline
	.no_right:
  	pop  ebx
    pop  ecx
  .valid:
    loop .lp2
    jmp  .exlp
  .lp2:
  	jmp  .lp
  .exlp:

    mov  esi,[player]
    mov  edi,[finish]
    cmp  esi,edi
    je   .ex
    mov  edx,0x7777FF
    call getline
  .ex:
  	cmp  [pause_time],0
  	jne   .ex2
  	xor  ecx,ecx
  	xor  ebx,ebx
  	mov  edx,5
; eax - cur cell content, ebx - cell #, ecx - cell counter, edx - dir
	.chwin:
  	movzx eax,byte[field+ebx]
  	and  eax,0xf
  	btr  eax,edx
  	bsf  edx,eax
  	je   .ex3
  	inc  ecx
  	add  ebx,[dirs+edx*4]
  	jz   .ex3
  	xor  edx,11b
  	jmp  .chwin
  .ex3:
  	cmp  ecx,[cell_count]
  	jne  .ex2
  	mov  [win_flag],1
  .ex2:
    ret

LP_mouse:
		cmp  [win_flag],1
		je   .ex
    mov  [jump],still
    mov  edx,eax
		call get_last_mclick
    cmp  edx,-2
    jne  .no1st
		test eax,eax
		jnz  .noempty
		mov  eax,[player]
		mov  ebx,[finish]
		call get_offset
		mov  ecx,eax
		mov  edx,0x101
		shl  dl,cl
		xor  ecx,11b
		shl  dh,cl
		mov  ecx,ebx
		jecxz LP_drawm.ex2
		mov  esi,[player]
	.lp:
		xor  [field+esi],dl
		add  esi,[dirs+eax*4]
		xor  [field+esi],dh
		loop .lp
		xor  ebx,ebx
	.noempty:
		mov  [player],ebx
    jmp  .exx
  .no1st:
		test eax,eax
		jz   .ex
		mov  eax,[player]
		push ebx
		call get_offset
		pop  ebx
		cmp  eax,-1
		je   .ex
	.exx:
		mov  [finish],ebx
    mov  [jump],drw
;		ud2
	.ex:
		ret


LP_aw db 1001b,0110b
LP_em db 0000b,0001b,0010b,0100b,1000b
LP_ab	db 0011b,0101b,1100b,1010b,1001b,0110b

LP_draw_alert:
; al-cell, esi - ptr,ecx - len
	.lp:
		cmp  al,[esi]
		je   .ex
		inc  esi
		loop .lp
		pusha
		mov  ebx,[lx]
		mov  ecx,[ly]
		add  ebx,4-2 shl 16
		add  ecx,4-2 shl 16
		mcall 13,,,0xFFAAAA
		mov  [pause_time],eax
		popa
	.ex:
		ret

LP_level:
file 'loops.bin'

if lang eq ru
	LP_help mstr \
	'Через все клетки прямоугольника необходимо',\
	'провести замкнутую ломаную. Проходя через черный',\
	'кружок, ломаная обязана повернуть, а проходя',\
	'через белый кружок, ломаная поворачивать не',\
	'может. В пустых же клетках ломаная ведет себя как',\
	'захочет.','',\
	'http://puzzleprograms.narod.ru'
else
	LP_help mstr \
	'You have to draw closed line over every grid',\
	'cell. On black square line must turn, and it must',\
	'go straight when over white square. The line may',\
	'behave anyhow elsewhere.','',\
	'http://puzzleprograms.narod.ru'
end if

; level format
; [fx|fy][hole xy][balls #]..[1-red,0-black xy]..[maze walls]

; internal format
; [stepptr]= balls #
; [finish]= hole cell
; area: ..[1-red,0-black xy].. ( -1 if empty)

MAR_levelp:
    call get_xy_sf
    movzx eax,byte[esi-2]
    mov  [finish],eax
    movzx ecx,byte[esi-1]
    mov  [stepptr],ecx
    mov  edi,area
    rep  movsb
    mov  [pause_time],10
    call unpack_level
    ret

MAR_key:
    cmp  eax,176
    jb   .ex
    cmp  eax,179
    ja   .ex
    mov  esi,area-1
    lea  ebx,[eax-176]
  .again:
    and  [cur_step],0
    mov  ecx,[stepptr]
  .lp:
    movzx eax,byte[esi+ecx]
    cmp  al,-1
    je   .ex1
    mov  byte[player],al
    and  byte[player],0x80
    and  eax,0x7f
    call check_move
    jc   .ex1
    add  eax,[dirs+ebx*4]
    cmp  eax,[finish]
    jne  .nohole
    mov  byte[esi+ecx],-1
    jmp  .ex2
  .nohole:
  	push ecx
    mov  ecx,[stepptr]
    mov  edi,area
  .lp2:
    mov  dl,[edi]
    and  dl,0x7f
    cmp  al,dl
    je   .exlp2
    inc  edi
    loop .lp2
  .exlp2:
    pop  ecx
    je   .ex1
    or   al,byte[player]
    mov  [esi+ecx],al
  .ex2:
    inc  [cur_step]
  .ex1:
  	loop .lp
    cmp  [cur_step],0
    je   .exx
    call delay
    call drwfld
    jmp  .again
  .exx:
    mov  ecx,[stepptr]
    inc  esi
  .lp3:
    lodsb
    cmp  al,-1
    je   .nxt
    test al,0x80
    jne  .ex
  .nxt:
		loop .lp3
    mov  [win_flag],1
  .ex:
    ret

MAR_drawm:
		shl  [sq_size],1
    mov  eax,[finish]
    call get_xy
		shr  [sq_size],1
    mcall 13,[lx],[ly],0xa0a0a0
    movzx ecx,byte[stepptr]
    mov  edi,area
  .lp:
    push ecx
    movzx eax,byte[edi]
    mov  edx,0x0
    cmp  al,-1
    je   .exlp
    test eax,0x80
    je   .no
    mov  edx,0xff00000
  .no:
    and  eax,0x7f
    call get_xy
    mcall 13,[lx],[ly]
  .exlp:
    pop  ecx
    inc  edi
    loop .lp
  .ex:
    ret

MAR_level:
file 'marble.bin'

if lang eq ru
	MAR_help mstr \
	'Несколько красных и черных плашек лежат на доске,',\
	'дырка обозначена серой клеткой. Когда плашка',\
	'проходит над дыркой, она падает туда и исчезает.',\
	'Используйте стрелки для наклона доски.',\
	' Ваша задача - сбросить все красные плашки.',\
	'Количество упавших черных плашек роли не играет.','',\
	'http://www.clickmazes.com'
else
	MAR_help mstr \
  'Several red and black marbles sit on a flat tray,',\
  'a grey square indicate a hole in the tray. If a',\
  'marble rolls onto or over the hole it drops',\
  'through and disappears. Use the cursor keys to',\
  'tilt the tray and start the marbles rolling, the',\
  'marbles roll in a straight line until they come',\
  'to rest, you can then roll again. Your challenge',\
  'is to drop all the red marbles, it does not',\
  'matter how many black marbles you lose in the',\
  'process.','',\
	'http://www.clickmazes.com'
end if

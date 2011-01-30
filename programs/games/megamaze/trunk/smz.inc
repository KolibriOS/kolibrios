SMZ_levelp:
    call get_xy_sf
    xor  eax,eax
    mov  [stepptr],esi
    lodsb
    mov  [cur_step],esi
    add  esi,eax
    mov  [levptr],esi
    call unpack_level
    ret

SMZ_key:
    cmp  eax,176
    jb   .ex
    cmp  eax,179
    ja   .ex
    mov  ecx,[cur_step]
    movzx ecx,byte[ecx]
    lea  ebx,[eax-176]
    mov  eax,[player]
  .lp:
    call check_move
    jc   .exl
    add  eax,[dirs+ebx*4]
    loop .lp
  .exl:
    test ecx,ecx
    jne  .ex
    mov  [player],eax
    cmp  eax,[finish]
    jne  .nowin
    mov  [win_flag],1
  .nowin:
    mov  esi,[stepptr]
    movzx eax,byte[esi]
    inc  esi
    lea  ebx,[esi+eax]
    inc  [cur_step]
    cmp  [cur_step],ebx
    jb   .ex
    mov  [cur_step],esi
  .ex:
    ret

SMZ_drawm:
    mov  eax,[player]
    call get_xy
    mcall 13,[lx],[ly],0xff00
    mov  eax,[finish]
    call get_xy
    mcall 13,[lx],[ly],0xff0000
    cmp  [win_flag],1
    je   .ex
    mov  edi,[stepptr]
    movzx ecx,byte[edi]
    inc  edi
    mov  edx,(XFOFS+MSGXO) shl 16+42
    mov  ebx,0x010000
    mov  eax,47
  .lp:
    push ecx
    mov  esi,0x10000000
    movzx ecx,byte[edi]
    cmp  edi,[cur_step]
    jne  .nocur
    mov  esi,0x100000ff
    mcall
    add  edx,1 shl 16
  .nocur:
    mcall
    pop  ecx
    add  edx,10 shl 16
    inc  edi
    loop .lp
  .ex:
    ret

SMZ_level:
file 'smz.bin'

if lang eq ru
	SMZ_help mstr \
	'Повторяйте последовательность ходов, двигая',\
	'зеленый квадрат на подсвеченное количество клеток',\
	'по прямой, пока не достигнете красного квадрата.','',\
  'http://puzzleprograms.narod.ru'
else
	SMZ_help mstr \
	'Repeat sequence shown above the field moving the',\
	'green square by highlighted count of spaces as',\
	'many times as you like, until you finish at the',\
	'red square.','',\
  'http://puzzleprograms.narod.ru'
end if

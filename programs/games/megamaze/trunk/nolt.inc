NLT_levelp:
		mov  eax,OR_strip
		call need_image
		xor  eax,eax
		lodsb
		lodsb
		mov  [fx],eax
		lodsb
		mov  [fy],eax
		lodsw
		mov  [player],eax
		mov  dword[area],5
		lodsw
		mov  [finish],eax
    call get_xy_sf.count
    mov  [levptr],esi
    call unpack_level
    ret

NLT_key:
		mov  [jump],still
    cmp  eax,176
    jb   .ex
    cmp  eax,179
    ja   .ex
    lea  ebx,[eax-176]
    mov  eax,[player]
    call check_move
    jc   .ex
    mov  ecx,ebx
    mov  edx,ebx
		movzx ecx,byte[dir_rotate+ecx]
		cmp  ecx,dword[area]
		je   .ex
		xor  edx,11b
		cmp  edx,dword[area]
		je   .ex
    add  eax,[dirs+ebx*4]
    mov  dword[area],ebx
    mov  [player],eax
    cmp  eax,[finish]
    jne  .nowin
    mov  [win_flag],1
  .nowin:
		mov  [jump],drw
  .ex:
    ret

NLT_drawm:
    push [sq_size]
    mov  [sq_size],0
    mov  eax,[finish]
    mov  ebx,6
    call out_image
;    mcall 13,[lx],[ly],0xff0000
    mov  ebx,dword[area]
    cmp  ebx,5
    jne  .skip
    mov  eax,[player]
    mov  edi,eax
    pop  [sq_size]
    call get_xy
    mcall 13,[lx],[ly],0x8000
    jmp  .ex
  .skip:
    add  ebx,8
    mov  eax,[player]
    call out_image
    pop  [sq_size]
  .ex:
    ret

NLT_level:
file 'nolt.bin'

if lang eq ru
	NLT_help mstr \
	'Ваша задача - провести зеленый квадрат к красному',\
	'без поворота налево и движения назад. Это труднее',\
	'чем кажется. Внимание: если Вы примете плохое',\
	'решение, можете попасть в тупик и будете',\
	'вынуждены начать сначала.','',\
  'http://puzzleprograms.narod.ru'
else
	NLT_help mstr \
  'Your challenge is to manouvre the green dot to',\
  'the red square without ever turning left, or',\
  'making a U-turn. Harder than it looks. Beware, if',\
  'you make a bad decision in this maze you may get',\
  'completely stuck and have to restart.','',\
	'http://www.clickmazes.com'
end if

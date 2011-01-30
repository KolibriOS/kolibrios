TM_levelp:
    call get_xy_sf
    mov  [levptr],esi
    call unpack_level
    ret

TM_key:
    cmp  eax,' '
    je   .mm
    cmp  eax,176
    jb   .still
    cmp  eax,179
    ja   .still
    lea  ebx,[eax-176]
    mov  eax,[teseus]
    call check_move
    jc   .still
    call move_teseus
    cmp  [win_flag],0
    jne  .ex
  .nowin:
    call drwfld
  .mm:
    call move_minotaur
    cmp  eax,[teseus]
    jne  .still
    mov  [win_flag],2
    jmp  .ex
  .still:
    mov  [jump],still
  .ex:
    ret

move_minotaur:
    xor  esi,esi
    mov  eax,[teseus]
    mov  ebx,[fx]
    div  bl
    mov  ecx,eax  ;teseus: ch-x, cl-y
    mov  eax,[minotaur]
  .again:
    call mino_xy
    xor  ebx,ebx
    cmp  dh,ch
    je   .stand2
    ja   .ok1
    add  ebx,3
  .ok1:
    call check_move
    jc   .stand2
    add  eax,[dirs+ebx*4]
    mov  [minotaur],eax
    inc  esi
    call delay
    call drwfld
    cmp  esi,2
    jb   .again
    jmp  .ex
  .stand1:
    call mino_xy
  .stand2:
    mov  ebx,2
    cmp  dl,cl
    je   .ex
    ja   .ok2
    dec  ebx
  .ok2:
    call check_move
    jc   .ex
    add  eax,[dirs+ebx*4]
    mov  [minotaur],eax
    inc  esi
    call delay
    call drwfld
    cmp  esi,2
    jb   .again
  .ex:
    ret

mino_xy:
    push eax
    div  byte[fx]
    mov  edx,eax  ;minotaur: dh-x, dl-y
    pop  eax
    ret

move_teseus:
    pusha
    cdq
    mov  ecx,[fx]
    div  cl
    add  ax,1 shl 8+1
    test ebx,ebx
    jne  .no0
    cmp  ah,1
    jne  .move
  .win:
    inc  [win_flag]
    jmp  .ex
  .no0:
    cmp  ebx,1
    jne  .no1
    cmp  al,byte[fy]
    je   .win
    jmp  .move
  .no1:
    cmp  ebx,2
    jne  .no2
    cmp  al,1
    je   .win
    jmp  .move
  .no2:
    cmp  ebx,3
    jne  .move
    cmp  ah,byte[fx]
    je   .win
  .move:
    mov  eax,[esp+28]
    add  eax,[dirs+ebx*4]
    mov  [teseus],eax
  .ex:
    popa
    ret

TM_drawm:
    mov  eax,[teseus]
    call get_xy
    mcall 13,[lx],[ly],0xff00
    mov  eax,[minotaur]
    call get_xy
    mcall 13,[lx],[ly],0xff0000
    ret

TM_level:
file 'tam.bin'

if lang eq ru
	TM_help mstr \
	' Тесей (зеленый квадрат) должен убежать из',\
	'лабиринта. За ним гонится механический Минотавр',\
	'(красный квадрат). На каждый ход Тесей Минотавр',\
	'делает 2 хода по такой схеме:',\
	' Сначала он пытается приблизиться к Тесею по',\
	'горизонтали на 1 квадрат. Если это невозможно,',\
	'он пытается подойти по вертикали на 1 квадрат.',\
	'Если невозможно сделать ни того, ни другого,',\
	'Минотавр пропускает данный ход.','',\
  'http://puzzleprograms.narod.ru'
else
	TM_help mstr \
  ' Theseus (the green dot) must escape from a maze.',\
  'There is also a mechanical Minotaur (the red',\
  'dot) in each maze. For every turn that Theseus',\
  'takes, the Minotaur takes two turns.',\
  ' Each turn he decides following:',\
  'First he tests if he can move horizontally and',\
  'get closer to Theseus. If he can, he will move',\
  "one square horizontally. If he can't, he will",\
  'test if he could move vertically and get closer',\
  'to Theseus. If he can, he will move one square',\
  "vertically. If he can't move either horizontally",\
  'or vertically, then he just skips that turn.','',\
  'http://puzzleprograms.narod.ru'
end if

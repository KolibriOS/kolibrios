; level format
; [fx|fy][blue xy][red xy][blue point xy][red point xy]..[maze walls]

; internal format
; points db=[stepptr]
; [player]= blue
; [finish]= red
; area: ..[1-red,0-black xy].. ( -1 if empty)

TILT2_levelp:
    call get_xy_sf
    mov  [stepptr],esi
    add  esi,2
    call unpack_level
    mov  [pause_time],10
    ret

TILT2_key:
    cmp  eax,176
    jb   .ex
    cmp  eax,179
    ja   .ex
    mov  esi,area
    inc  esi
    lea  ebx,[eax-176]
  .lp:
    xor  edx,edx
    mov  eax,[player]
    call check_move
    jc   .nom1
    inc  edx
    add  eax,[dirs+ebx*4]
    mov  [player],eax
  .nom1:
    mov  eax,[finish]
    call check_move
    jc   .nom2
    inc  edx
    add  eax,[dirs+ebx*4]
    mov  [finish],eax
  .nom2:
  	test edx,edx
  	jz   .ex1
    call delay
    call drwfld
    jmp  .lp
  .ex1:
    mov  eax,[finish]
    cmp  eax,[player]
    jnz  .ex2
    mov  [win_flag],2
    jmp  TILT2_drawm.skip
  .ex2:
  	mov  esi,[stepptr]
  	cmp  al,[esi+1]
  	jne  .ex
  	mov  eax,[player]
  	cmp  al,[esi]
  	jne  .ex
    mov  [win_flag],1
  .ex:
    ret

TILT2_drawm:
		cmp  [win_flag],2
		je   .skip
;		mov  [sq_size],3
    mov  eax,[player]
    call get_xy
    mcall 13,[lx],[ly],0xff
    mov  eax,[finish]
    call get_xy
    mcall 13,[lx],[ly],0xff0000
  .skip:
		shl  [sq_size],1
    mov  esi,[stepptr]
    lodsb
    call get_xy
    mcall 13,[lx],[ly],0xff
    lodsb
    call get_xy
		shr  [sq_size],1
    mcall 13,[lx],[ly],0xff0000
    ret

TILT2_level:
file 'tilt2.bin'

if lang eq ru
	TILT2_help mstr \
	'Красный и синий блоки должны достичь клеток',\
	'соответствующего цвета. Внимание! Блоки сделаны',\
	'из агрессивного материала, и при столкновении',\
	'мгновенно испаряются. В этом случае Вам придется',\
	'начать уровень сначала.','',\
	'http://www.clickmazes.com'
else
	TILT2_help mstr \
  'The red block must reach the red target and the',\
  'blue block the blue target. Beware! The two',\
  'blocks are made of highly reactive material and,',\
  'if they collide, will instantly evaporate. At',\
  'this point you must start all over again.','',\
	'http://www.clickmazes.com'
end if

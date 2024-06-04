use32
    org 0x0
    db	'MENUET01'
    dd	0x01
    dd	_preSTART
    dd	I_END
    dd	0x4000
    dd	0x4000
    dd	0x0,0x0

include '../../macros.inc'
include 'lang.inc' ; Language support for locales: ru_RU (CP866), en_US.
include 'draw_window.inc'
include 'ball_operations.inc'
include 'basic_alg.inc'

_preSTART:
	mcall	66, 1, 1	;�ਭ����� ᪠�-����
	mcall	3				;randomize
	ror	eax, 16
	mov	[TC_U_SYSTEM_RANDSEED], eax	;from PPro "system.inc"

new_game:
	;���㫥��� ������
	mov	eax, 0
	mov	ecx, 81 / 4
	mov	edi, lineBall
	rep	stosd
	stosb

	mov	[score], 0
	mov	[countAllBall], 0
	mov	[current], 0
	@@:
	   mov	ebx, 7
	   call	random
	   inc	dl
	   call	add_new_ball
	   call test_new_ball
	   cmp  [countAllBall], 5
	   jb   @b
	call generate_new___new_color

START:
	draw_window
   still:
	mcall	10

	dec	eax
	jz	START	; ����ᮢ��� ����
	dec	eax
	jnz	button	;�᫨ �� ������ - ����� ������

   key:			; ����⨥ ������
	mcall	2	; ����砥� ��� ������
	cmp	ah, 60
	je	new_game
	jmp	still

   button:		; ����⨥ ������
	mcall	17
	shr	eax, 8
	cmp	ax, 1	; �᫨ ������ ��室�?
	je	.exit

	;�஢�ਬ, �� ���� �� ���⪠
	mov	edx, [current]
	test	byte [eax + lineBall - 2], 0x7
	jz	.blank

	;�஢�ਬ �� �뫠 �� �� ���⪠ 㦥 ��࠭�
	cmp	eax, edx
	je	.double_choice

	mov	[current], eax
	call	choice_ball
	call	paint_ball	;᭠砫� ����㥬 ���� ��࠭�� �ਪ

	test	edx, edx	;cmp [current], 0
	jz	still
	mov	eax, edx

   @@:
	call	redraw_cell	;� 㦥 ��⮬ 㡥६ ࠬ�� �� ��ண�
	call	paint_ball
	jmp	still

   .double_choice:
	mov	[current], 0
	jmp	@B

   .blank:	;������ ������ - ����
   	test	edx, edx	;cmp [current], 0
	jz	still
	mov	[dest], eax	;������ �����祭�� -> � [dest]
	mov	eax, edx	;��ࠬ��� � eax
	call	test_path	;��뢠�� ४���� test_path
	call	zero_cheked
	jnc	still

;^^^^^^^^^^^^^^^^^ move_ball	proc
	mov	eax, [current]
	mov	cl, byte [eax + lineBall - 2]
	mov	byte [eax + lineBall - 2], 0
	mov	eax, [dest]
	mov	byte [eax + lineBall - 2], cl

	mov	eax, [current]
	call	redraw_cell
	mov	eax, [dest]
	call	paint_ball
	mov	[current], 0
;^^^^^^^^^^^^^^^^^ move_ball	endp

	call	find_line
	call	vanish_balls
	call	zero_cheked

	cmp	[countVanishBall], 0
	je	new_3_balls
	movzx	eax, [countVanishBall]
	inc	eax
	add	[score], eax
	sub	[countAllBall], al
	call	redraw_score
	jmp	still

   .exit:
	mcall	-1	;��室��

new_3_balls:

	rept	3 num
	{
	   mov	dl, [newColor#num]
	   call	add_new_ball
	   call	test_new_ball
	   cmp	[countAllBall],81
	   je	.record_li
	}

	call	generate_new___new_color
	call	paint_new_color
	jmp	still

	.record_li:
	   mov	eax, [score]
	   cmp	eax, [record]
	   jbe	still
	   mov	[record], eax
	   jmp	START

zero_cheked:	;���㫥��� ������ cheked
	pushfd
	mov	eax, lineBall
	mov	ecx, 81
	@@:
	   and	byte [eax], 0x7
	   inc	eax
	   loop	@B
	popfd
	ret

random:			; edx := random [0..(ebx-1)]
	mov	eax, 134775813
	mul	[TC_U_SYSTEM_RANDSEED]
	inc	eax
	mov     [ TC_U_SYSTEM_RANDSEED], eax
	xor	edx, edx
	div	ebx
	ret

generate_new___new_color:
	rept	3 num
	{
	   mov	ebx, 7
	   call	random
	   inc	dl
	   mov	[newColor#num], dl
	}
	ret

add_new_ball:	;� dl - 梥� �ਪ�
	mov	ebp, edx
	mov	ebx, 81
	sub	bl, [countAllBall]
	call	random
	mov	ecx, edx
	mov	eax, lineBall-1
	inc	ecx
	@@:
	   inc	eax
	   test	byte [eax], 0x7
	   jnz	@B
	   loop	@B

	;⥯��� � ��� � eax - ���� ��襣� �ਪ�
	mov	edx, ebp
	or	byte [eax], dl
	inc	[countAllBall]
	;����稬 ID ������
	sub	eax, lineBall-2
	mov	[dest], eax
	ret

test_new_ball:
	call	paint_ball

	call	find_line
	call	vanish_balls
	call	zero_cheked

	cmp	[countVanishBall], 0
	je	@F
	movzx	eax, [countVanishBall]
	inc	eax
	add	[score], eax
	sub	[countAllBall], al
	call	redraw_score
	@@:
	ret

if lang eq ru_RU
	szTitle  db '����� ����� v 0.3',0
	szNewGame db 'F2 - ����� ���',0
	szRecord db '�����',0
	szScore  db '�窨',0
else ; Default to en_US
	szTitle  db 'Color lines v 0.3',0
	szNewGame db 'F2 - new game',0
	szRecord db 'Record',0
	szScore  db 'Score',0
end if


blank	= 0xB8C2D3
brown	= 0x804000
red	= 0xff0000
yellow	= 0xffff00
green	= 0x008000
cyan	= 0x00ffff
blue	= 0x0000ff
purple	= 0x800080

tableColor	dd blank,brown,red,yellow,green,cyan,blue,purple

lineCoord:
		rept	9 coory:0
		{
		   rept	9 coorx:0
		   \{
		   	dw coorx*256+coory

		   \}
		}

record  dd 25

lineBall db 81 dup ?
score   dd ?
baseAddr dd ?
current dd ?
dest	dd ?

newColor1 db ?
newColor2 db ?
newColor3 db ?

countVanishBall	db ?
countAllBall	db ?

TC_U_SYSTEM_RANDSEED	dd	?
bitID	dd ?

I_END:

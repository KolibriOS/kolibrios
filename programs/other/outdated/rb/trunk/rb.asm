;
;   DESKTOP CONTEXT MENU
;   written by Ivan Poddubny
;
;   ���� - ���� ����㡭�
;   e-mail: ivan-yar@bk.ru
;
;   Compile with flat assembler
;
;------------------------------------------------------------------------------
; version:	1.1
; last update:  27/03/2012
; changed by:   Marat Zakiyanov aka Mario79, aka Mario
; changes:      The program uses only 3404 bytes memory is now.
;               Optimisations and code refactoring.
;------------------------------------------------------------------------------
include 'lang.inc'
include '..\..\..\..\macros.inc'
;------------------------------------------------------------------------------
	use32
	org 0x0
	db 'MENUET01'	; 8 byte id
	dd 0x01		; header version
	dd START	; start of code
	dd IM_END	; size of image
	dd I_END	; memory for app
	dd stack_area	; esp
	dd 0		; boot parameters
	dd 0		; path
;------------------------------------------------------------------------------
START:
; ������� ��⥬�� 梥�
	mcall	48,3,sc,sizeof.system_colors
; ��⠭���� ���� ᮡ�⨩ - ��� ������� ⮫쪮 ����
	mcall	40,100000b
;------------------------------------------------------------------------------
align 4
still:		     ; ������ 横� �᭮����� �����
	mcall	10	; ��� ᮡ���

	mcall	37,2	; ����� ������ ������?
	cmp	eax,ebx	; �᫨ �� �ࠢ��, ������
	jne	still
;--------------------------------------
; �� ��� �⫠��� - �᫨ ���� � �窥 (0;0), ���஥���
;	xor	ebx,ebx
;	mcall	37
;	test	eax,eax	; ����� � �窥 (0;0), �.�. eax = 0
;	je	exit
;--------------------------------------
; ���न���� �����
	xor	ebx,ebx
	mcall	37

	mov	ebx,eax		; eax = cursor_x
	shr	eax,16		; ebx = cursor_y
	and	ebx,0xffff
	mov	[curx1],eax	; curx1 = cursor_x
	mov	[cury1],ebx	; cury1 = cursor_y
; ���� �ਭ������� �窠?
	mcall	34,[curx1],[cury1]
	cmp	al,1 ; 1 - ��
	jne	still
;--------------------------------------
align 4
@@:		; �������, ���� ���짮��⥫� �� ����⨫ �ࠢ�� ������ ���
	mcall	37,2    ;   ������ �� ������ ���?
	test	eax,ebx ; �᫨ ����⨫, (eax != 2)
	jz	@f	;   ��� � ��砫� �������� 横��

	mcall	68,1	; ���� ��४��稬�� �� ᫥���騩 ��⮪ ��⥬� � �����
	jmp	@b	; �믮������ ��୥��� �⮬� ��⮪�, �஢�ਬ ���� �����
;--------------------------------------
align 4
@@:
; �᫨ 㦥 �뫮 ����� ����, �㦭� ���������, ���� ��� ���஥���:
	cmp	[menu_opened],0
	je	@f

	mcall	68,1	; ��४��稬�� �� ᫥���騩 ��⮪ ��⥬�
			; ����� ��䥪⨢�� ᯮᮡ ����প� 祬 mcall 5
	jmp	@b
;--------------------------------------
align 4
@@:
; � ⥯��� ����� ᬥ�� ����᪠�� ����� (��⮪) ����
	mcall	51,1,start_wnd,stack_wnd
	jmp	still
;------------------------------------------------------------------------------
align 4
exit_menu:	      ; �᫨ ��室�� �� ����, ���� ������� � [menu_opened] 0
	mov	[menu_opened],0
;--------------------------------------
align 4
exit:		      ; � �� ���, ����� ��室�� �� �᭮����� �����
	or	eax,-1	      ; eax = -1
	mcall
;------------------------------------------------------------------------------
; ����� ������ ����� ����
;------------------------------------------------------------------------------
align 4
start_wnd:
	mov	[menu_opened],1
; ��⠭���� ���� �������� ᮡ�⨩: ���� + ������ + ����ᮢ��
	mcall	40,100101b
;------------------------------------------------------------------------------
align 4
red:
	call	draw_window
;------------------------------------------------------------------------------
align 4
still2: 	    ; ������ 横� ����� ����
	mcall	10	; ��� ᮡ���

	cmp	eax,1	    ; ����ᮢ��?
	je	red

	cmp	eax,3	    ; ������?
	je	button

	cmp	eax,6	    ; ����?
	je	mouse

	jmp	still2	    ; ������ � ��砫� �������� 横��
;------------------------------------------------------------------------------
align 4
; ���������� ����
mouse:		  ; ����� ���짮��⥫� ������ ������ ���, ���஥���
	mcall	37,2	; ����� ������ ������?
	test	eax,eax   ; �������? - ⮣�� �४�᭮! ������ � ������ 横�
	jz	still2

        mcall   37,0

        mov     esi, eax
        shr     esi, 16
        movzx   edi, ax
        mcall   9, procinfo, -1

        mov     eax, [procinfo.box.left]
        cmp     esi, eax
        jl      exit_menu

        add     eax, [procinfo.box.width]
        cmp     esi, eax
        jge     exit_menu

        mov     eax, [procinfo.box.top]
        cmp     edi, eax
        jl      exit_menu

        add     eax, [procinfo.box.height]
        cmp     edi, eax
        jge     exit_menu

        jmp     still2
;------------------------------------------------------------------------------
align 4
; ������ ������
button:
	mcall	17	; ������� �����䨪��� ����⮩ ������

	sub	ah,10	      ; �ࠢ������ � 10
	jl	nofuncbtns    ; �᫨ ����� - ����뢠�� ����

	movzx	ebx,ah	      ; ����稫� ����� �ணࠬ�� � ᯨ᪥ � ebx
	mov	esi,[startapps + ebx*4]
	mov	edi,start_info.path
	cld
;--------------------------------------
align 4
@@:
	lodsb
	stosb
	test	al,al
	jnz	@b
	mcall	70, start_info

;	mov	eax,5         ; �������, ���� �ணࠬ�� ����������
;	mov	ebx,1         ; � � �� ���� �� �㤥� ���ᮢ��� (��� � ��???)
;	mcall          ; �᪮�������� �� ��ப�, �᫨ � ��� �஡����
		       ; � ���ᮢ���
;--------------------------------------
align 4
nofuncbtns:	      ; ����뢠�� ����
	jmp	exit_menu
;------------------------------------------------------------------------------
_BTNS_		  = 6	  ; ������⢮ ������ ("�㭪⮢ ����")

if lang eq ru_RU
  font		  = 0x00000000
  string_length   = 20		; ����� ��ப�
  wnd_x_size	  = 133 	; �ਭ� ����
  title_pos	 = 36 shl 16 + 7
else
  font		  = 0x10000000
  string_length   = 12		; string length
  wnd_x_size	  = 105 	; window width
  title_pos	 = 23 shl 16 + 7
end if
;------------------------------------------------------------------------------
;*******************************
;********  ������ ����  ********
;*******************************
draw_window:
	mcall	12,1	; ��稭��� "�ᮢ���"

	mov	eax,[curx1]	 ; ⥪�騥 ���न���� �����
	mov	[curx],eax	 ; ����襬 � ���न���� ����
	mov	eax,[cury1]
	mov	[cury],eax
; ⥯��� �㤥� ����� ���न���� ����, �⮡� ��� �� �ࠩ �࠭� �� �뫥���
	mcall	14		; ����稬 ࠧ��� �࠭�

	mov	ebx,eax
	shr	eax,16			; � eax - x_screen
	and	ebx,0xffff		; � ebx - y_screen
	add	eax,-wnd_x_size		; eax = [x_screen - �ਭ� ����]
	add	ebx,-_BTNS_*15-21	; ebx = [y_screen - ���� ����]

	cmp	eax,[curx]
	jg	.okx			; �᫨ ���� ᫨誮� ������ � �ࠢ��� ���,
	add	[curx],-wnd_x_size	; ᤢ���� ��� ����� �� 100
;--------------------------------------
align 4
.okx:
	cmp	ebx, [cury]
	jg	.oky			; �� ���⨪��� �筮 ⠪��
	add	[cury], -_BTNS_*15-21
;--------------------------------------
align 4
.oky:
	xor	eax, eax	   ; �㭪�� 0 - ᮧ���� ����
	mov	ebx, [curx]	   ;  ebx = [���न��� �� x] shl 16 + [�ਭ�]
	shl	ebx, 16
	add	ebx, wnd_x_size
	mov	ecx, [cury]	   ;  ecx = [���न��� �� y] shl 16 + [����]
	shl	ecx, 16
	add	ecx, _BTNS_*15+21
	mov	edx, [sc.work]	   ;  梥� ࠡ�祩 ������
	mov	esi, [sc.grab]	   ;  梥� ���������
	or	esi, 0x81000000
	mov	edi, [sc.frame]    ;  梥� ࠬ��
	mcall

	mov	eax, 4		   ; ���������
	mov	ebx, title_pos	  ;  [x] shl 16 + [y]
	mov	ecx, [sc.grab_text];  ���� � 梥� (���)
	or	ecx, 0x10000000

	push	ecx
	push	ecx
	xor	edx,edx
;--------------------------------------
align 4
.dec_color:
	sub	byte [esp+edx], 0x33
	jae	@f
	mov	byte [esp+edx], 0
;--------------------------------------
align 4
@@:
	inc	edx
	jnp	.dec_color
	pop	ecx
	mov	edx, title	  ;  ���� ���������
	mov	esi, title.size   ;  ����� ��������� ("M E N U")
	mcall
	pop	ecx
	add	ebx, 1 shl 16	   ;  ᤢ���� ��ࠢ� �� 1
	mcall

	mov	ebx, 1*65536+wnd_x_size-2  ; ��稭��� ������ ������
	mov	ecx, 20*65536+15
	mov	edx, 10 or 0x40000000 ; ��� 30 ��⠭����� => ������ �� ������

	mov	edi,_BTNS_	     ; ������⢮ ������ (����稪)
;--------------------------------------
align 4
newbtn:		     ; ��砫� 横��
	mcall	8		;  ᮧ��� ������

			     ;  ��襬 ⥪�� �� ������
	pushad		     ;   ᯠᠥ� ॣ�����
	shr	ecx, 16
	and	ebx, 0xffff0000
	add	ebx, ecx	     ;   ebx = [x] shl 16 + [y];
	add	ebx, 10*65536+4      ;   ebx += ᬥ饭�� �⭮�⥫쭮 ��� ������;
	mov	ecx, [sc.work_text]  ;   ���� � 梥�
	or	ecx, font
	add	edx, -10	     ;   edx = ����� ������;
	imul	edx, string_length   ;   edx *= ����� ��ப�;
	add	edx, text	     ;   edx += text;  ⥯��� � edx ���� ��ப�
	mov	esi, string_length   ;   � esi - ����� ��ப�
	mcall	4
	popad

	inc	edx		     ;  ����� ������++;
	add	ecx,15*65536	     ;  㢥��稬 ᬥ饭�� �� y
	dec	edi		     ;  㬥��訬 ����稪
	jnz	newbtn		     ; �᫨ �� ����, ����ਬ ��� ��� ࠧ

	mcall	12,2	; �����稫� "�ᮢ���"
	ret			     ; ������
;------------------------------------------------------------------------------
align 4
; PROGRAM DATA

  macro strtbl name, [string]
  {
   common
     label name dword
   forward
     local str
     dd str
   forward
     str db string
  }

  strtbl startapps	 ,\
    <"/sys/PIC4",0>	,\
    <"/sys/DESKTOP",0>	,\
    <"/sys/ICON",0>,\
    <"/sys/SETUP",0>	,\
    <"/sys/DEVELOP/BOARD",0> ,\
    <"/sys/CPU",0>

  sz title, "KolibriOS"

  lsz text,\
    en_US, 'Background  ',\
    en_US, 'Desktop     ',\
    en_US, 'Icon manager',\
    en_US, 'Device setup',\
    en_US, 'Debug board ',\
    en_US, 'Processes   ',\
    \
    ru_RU, '������� �����     ',\
    ru_RU, '����ன�� ����      ',\
    ru_RU, '��ࠢ����� �������� ',\
    ru_RU, '����ன�� ���ன�� ',\
    ru_RU, '������ �⫠���      ',\
    ru_RU, '������            ',\
    \
    et_EE, 'Taust       ',\
    et_EE, 'T��laud     ',\
    et_EE, 'Ikooni hald.',\
    et_EE, 'Seadme hald.',\
    et_EE, 'Silumis aken',\
    et_EE, 'Protsessid  '

;------------------------------------------------------------------------------
align 4
start_info:
	.mode	dd 7
		dd 0
	.params dd 0
		dd 0
		dd 0
		db 0
		dd start_info.path
;------------------------------------------------------------------------------
IM_END:
align 4
; �������������������� ������
  curx1		dd ?	; ���न���� �����
  cury1		dd ?
  curx		dd ?	; ���न���� ���� ����
  cury		dd ?

  menu_opened	db ?	; ����� ���� ��� ���? (1-��, 0-���)
;------------------------------------------------------------------------------
align 4
start_info.path	rb 256
;------------------------------------------------------------------------------
align 4
sc	system_colors	; ��⥬�� 梥�
;------------------------------------------------------------------------------
align 4
procinfo process_information	; ���ଠ�� � �����
;------------------------------------------------------------------------------
align 4
	rb 512			; ��� ��� ���� ���� - 墠�� � 1 ��
stack_wnd:
;------------------------------------------------------------------------------
align 4
	rb 512
stack_area:
;------------------------------------------------------------------------------
I_END:
;------------------------------------------------------------------------------
; ����� ���������
;------------------------------------------------------------------------------

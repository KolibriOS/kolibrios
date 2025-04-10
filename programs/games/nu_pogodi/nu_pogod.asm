use32
	org 0
	db 'MENUET01' ;������. �ᯮ��塞��� 䠩�� �ᥣ�� 8 ����
	dd 1, start, i_end, mem, stacktop, 0, sys_path

include '../../macros.inc'
include '../../proc32.inc'
include '../../KOSfuncs.inc'
include '../../load_lib.mac'
include '../../develop/libraries/box_lib/trunk/box_lib.mac' ;������ ��� ������� ������⮢ box_lib
include '../../dll.inc'

@use_library mem.Alloc,mem.Free,mem.ReAlloc, dll.Load

;䫠��, ��� �㭪樨 ��१���� ����
BUF2D_OPT_CROP_TOP equ 1 ;��१�� ᢥ���
BUF2D_OPT_CROP_LEFT equ 2 ;��१�� ᫥��
BUF2D_OPT_CROP_BOTTOM equ 4 ;��१�� ᭨��
BUF2D_OPT_CROP_RIGHT equ 8 ;��१�� �ࠢ�
BUF2D_BIT_OPT_CROP_TOP equ 0
BUF2D_BIT_OPT_CROP_LEFT equ 1
BUF2D_BIT_OPT_CROP_BOTTOM equ 2
BUF2D_BIT_OPT_CROP_RIGHT equ 3

BUF_STRUCT_SIZE equ 21
buf2d_data equ dword[edi] ;����� ���� ����ࠦ����
buf2d_w equ dword[edi+8] ;�ਭ� ����
buf2d_h equ dword[edi+12] ;���� ����
buf2d_l equ word[edi+4] ;����� ᫥��
buf2d_t equ word[edi+6] ;����� ᢥ���
buf2d_size_lt equ dword[edi+4] ;����� ᫥�� � �ࠢ� ��� ����
buf2d_color equ dword[edi+16] ;梥� 䮭� ����
buf2d_bits equ byte[edi+20] ;������⢮ ��� � 1-� �窥 ����ࠦ����

struct FileInfoBlock
	Function dd ?
	Position dd ?
	Flags	 dd ?
	Count	 dd ?
	Buffer	 dd ?
		db ?
	FileName dd ?
ends

displ_w dd ? ;�ਭ� ����
displ_h dd ? ;���� ����
displ_bytes dd ? ;ࠧ��� 1-�� 䠩�� � ����ࠦ�����
offs_shadow_x dd ? ;ᤢ�� ⥭�� �� �� 'x'
offs_shadow_y dd ? ;ᤢ�� ⥭�� �� �� 'y'

IMAGE_FONT_SIZE equ 128*144*3

BUT1_T equ 10 ;����� ᢥ���
BUT1_L equ 15 ;����� ᫥��
BUT1_W equ 50 ;�ਭ�
BUT1_H equ 20 ;����
BUT1_NEXT_TOP equ (BUT1_T+BUT1_H)*65536

game_select_mode db 1 ;०�� �롮� ����

FILE_NAME_MAX equ 20 ;���ᨬ��쭠� ������ ����� 䠩�� (��� �����, �⭮�⥫쭮 ⥪�饩)
;���祭�� ���� �� 㬮�砭��
ini_def_decorat_file db 'curici.png',0
ini_def_unit_file db 'wolf.png',0
ini_def_objects_file db 'eggs.png',0
ini_def_lost_file db 'chi.png',0
;����� 䠩���
fn_icon0 rb FILE_NAME_MAX ;��� 䠩�� � ������ﬨ
fn_icon1 rb FILE_NAME_MAX ;��� 䠩�� � ������ � ���楬
fn_icon2 rb FILE_NAME_MAX ;��� 䠩�� � �栬�
fn_icon3 rb FILE_NAME_MAX ;��� 䠩�� � 樯��⠬�

TREE_ICON_SYS16_BMP_SIZE equ 256*3*11+54 ;ࠧ��� bmp 䠩�� � ��⥬�묨 ��������

ini_m_name db 'main.ini',0
key_count db 'count',0
key_game db 'g'
	key_game_ind db 0,0,0

ini_name rb FILE_NAME_MAX ;��� ini 䠩�� c ��࠭��� ��ன
ini_sec_files db 'Files',0
key_file_decorat db 'file_decorat',0
key_file_unit db 'file_unit',0
key_file_objects db 'file_objects',0
key_file_lost db 'file_lost',0
key_displ_w db 'displ_w',0
key_displ_h db 'displ_h',0
key_shadow_x db 'offs_shadow_x',0
key_shadow_y db 'offs_shadow_y',0

ini_sec_game db 'Game',0
key_delay_a db 'delay_a',0
key_delay_b db 'delay_b',0
key_delay_min db 'delay_min',0

ini_sec_color db 'Colors',0
;���� ��� ���뢠��� 梥⮢ �� *.ini 䠩��
key_color_fon db 'background',0
key_color_shadows db 'shadows',0
key_color_egg db 'objects',0
key_color_chick db 'lost',0
key_color_decorat: db 'dec'
	.ind: db '?',0
key_color_unit db 'unit',0

;梥� � ���
color_fon dd ? ;梥� 䮭�
color_shadows dd ? ;梥� ⥭��
color_wolf dd ? ;梥� ����� � ����
color_egg dd ? ;梥� ��
color_chick dd ? ;梥� 樯�����
color_decorat dd ?,?,? ;梥� �����権 (�����, ��ਫ�, ...)

;梥� ����䥩�
color_but_sm dd 0x808080 ;梥� �����쪨� ������
color_but_te dd 0xffffff ;梥� ⥪�� �� �������

macro load_image_file path,buf,size { ;����� ��� ����㧪� ����ࠦ����
	;path - ����� ���� ��६����� ��� ��ப��� ��ࠬ��஬
	if path eqtype '' ;�஢��塞 ����� �� ��ப�� ��ࠬ��� path
		jmp @f
			local .path_str
			.path_str db path ;�ନ�㥬 �������� ��६�����
			db 0
		@@:
		;32 - �⠭����� ���� �� ���஬� ������ ���� ���� � ��⥬�� ��⥬
		copy_path .path_str,[32],file_name,0
	else
		copy_path path,[32],file_name,0 ;�ନ�㥬 ����� ���� � 䠩�� ����ࠦ����, ���ࠧ㬥���� �� �� � ����� ����� � �ணࠬ���
	end if

	stdcall mem.Alloc, dword size ;�뤥�塞 ������ ��� ����ࠦ����
	mov [buf],eax

	mov eax,SF_FILE
	mov [run_file_70.Function], SSF_READ_FILE
	mov [run_file_70.Position], 0
	mov [run_file_70.Flags], 0
	mov [run_file_70.Count], dword size
	m2m [run_file_70.Buffer], [buf]
	mov byte[run_file_70+20], 0
	mov [run_file_70.FileName], file_name
	mov ebx,run_file_70
	int 0x40 ;����㦠�� 䠩� ����ࠦ����
	cmp ebx,0xffffffff
	je @f
		;��।��塞 ��� ����ࠦ���� � ��ॢ���� ��� �� �६���� ���� image_data
		stdcall [img_decode], [buf],ebx,0
		mov [image_data],eax
		;�८�ࠧ㥬 ����ࠦ���� � �ଠ�� rgb
		stdcall [img_to_rgb2], [image_data],[buf]
		;㤠�塞 �६���� ���� image_data
		stdcall [img_destroy], [image_data]
	@@:
}

;����� ����
bit_zaac equ 2 ;��� ����
val_zaac equ 4 ;��� �⢥���� �� ��� ����
bit_mig equ 3 ;��� �������
val_mig equ 8 ;��� �⢥���� �� ��� �������
mask_lot_lu    equ 1b ;��᪠ ��� ������ ���孥�� ��⪠
mask_lot_ld    equ 100000b ;��᪠ ��� ������ ������� ��⪠
mask_lot_ru    equ 10000000000b ;��᪠ ��� �ࠢ��� ���孥�� ��⪠
mask_lot_rd    equ 1000000000000000b ;��᪠ ��� �ࠢ��� ������� ��⪠
mask_clear_all equ 11111011111011110111101111011110b ;��᪠ ��� ���⪨ ������� �� � �ਡ������ 樯���
mask_move_eggs equ 11111111111111111111b ;��᪠ ��������� ��
mask_fail_eggs equ 100001000010000100000b ;��᪠ ������� ��
mask_chi_left  equ 11111000000000000000000000b ;��᪠ ����� 樯���
mask_chi_right equ 11111000000000000000000000000000b ;��᪠ �ࠢ�� 樯���
mask_chi_cr_l  equ 1000000000000000000000b ;��᪠ ��� ᮧ����� ������
mask_chi_cr_r  equ 1000000000000000000000000000b ;��᪠ ��� ᮧ����� �ࠢ���
bit_chi_left  equ 21 ;1-� ��� ����� �⢥砥� �� ����饣� ᫥��
bit_chi_right equ 27 ;1-� ��� ����� �⢥砥� �� ����饣� �ࠢ�
val_zaac_time_y equ 5 ;�������⢮ ⠪⮢, ���஥ ��易⥫쭮 ������ �஢���� ����
val_zaac_time_n equ 7 ;�������⢮ ⠪⮢, ���஥ ��易⥫쭮 ������ ���� ���⠭�� ����

txt_game_a db '��� �',0
txt_game_b db '��� �',0

zaac_status db 0 ;�᫮ ⠪⮢, ���஥ ���� �� ������ ������ ᢮� ���������
pos_wolf db 0 ;������ ����� 0-� ��� ᫥��/�ࠢ�, 1-� ��� ᢥ���/����
;rb 1
pos_eggs dd 0 ;����樨 �ᯮ������� �� � 樯���
eggs_count dw 0 ;�������⢮ �������� ��
game_text db '��� _',13
some_text db '0'
	rb 8 ;⥪�� � �᫮� ��������� ��
count_last db 0 ;���稪 �ய�饭��� ��
game_spd dd 0 ;����প� ����
game_delay_a dd ? ;��ࢮ��砫쭠� ����প� ��� ���� �
game_delay_b dd ? ;��ࢮ��砫쭠� ����প� ��� ���� �
game_delay_min dd ? ;�������쭠� ����প�

;��� �����樨 ��砩��� �ᥫ
rand_x dd 0

align 4
rand_next:
;x(k+1) = (a*x(k)+c) mod m
; a=22695477, c=1, m=2^32
push eax
	mov eax,[rand_x]
	imul eax,22695477
	inc eax
	mov [rand_x],eax
pop eax
	ret

;ᮧ���� �஧��� ���� �� 8 ��� (������), ��� �ᮢ����
;��� buf - ���� �� �᭮�� ���ண� �㤥� ᮧ��� �����
;䮭��� 梥� ���� ������ ���� 0xffffff, ���� �� �ந������ ��१��
;�� 䮭����� 梥�� � ����� �㤥� ������� 쬭��� ���� � �����
align 4
proc CreateTrapharetBuffer uses eax edi, buf:dword, img_data:dword
	mov edi,[buf]

	;���������� ������ ����
	mov buf2d_size_lt,0
	mov eax,[displ_w]
	mov buf2d_w,eax
	mov eax,[displ_h]
	mov buf2d_h,eax
	mov buf2d_color,0xffffff
	mov buf2d_bits,24

	stdcall [buf2d_create_f_img], edi,[img_data] ;ᮧ���� ����
	stdcall [buf2d_conv_24_to_8], edi,1 ;������ ���� �஧�筮�� 8���
	;��१��� ��譨� ��� ����, ��� ����� ����ண� �ᮢ����
	stdcall [buf2d_crop_color], edi,buf2d_color,BUF2D_OPT_CROP_TOP+BUF2D_OPT_CROP_BOTTOM+BUF2D_OPT_CROP_RIGHT+BUF2D_OPT_CROP_LEFT
	ret
endp

align 4
InitBackgroundBuffer: ;ᮧ����� 䮭����� ����ࠦ����
	pushad
	mov edi,buf_fon
	mov eax,[color_fon]
	mov buf2d_color,eax
	stdcall [buf2d_clear], edi,eax
	mov esi,edi

	xor eax,eax
	cld
	cmp dword[offs_shadow_x],0
	jne @f
	cmp dword[offs_shadow_y],0
	jne @f
		jmp .end_shadows
	@@:
		mov edi,buf_decor
		xor ebx,ebx
		mov ecx,3
		@@:
			mov ax,buf2d_t
			add eax,[offs_shadow_y]
			mov bx,buf2d_l
			add ebx,[offs_shadow_x]
			stdcall [buf2d_bit_blt_alpha], esi, ebx,eax, edi,[color_shadows] ;��㥬 ⥭� �����権
			add edi,BUF_STRUCT_SIZE
			loop @b
	.end_shadows:

	mov edi,buf_decor
	mov ebx,color_decorat
	mov ecx,3
	@@:
		mov ax,buf2d_t
		stdcall [buf2d_bit_blt_alpha], esi, 0,eax, edi,[ebx] ;��㥬 �����樨
		add edi,BUF_STRUCT_SIZE
		add ebx,4
		loop @b
	popad
	ret

;���� ���⨭�� � ���� buf �� ���ᨢ� ���஢ buf_img � �����ᮬ ind
;�. �. buf <- buf_img[ind]
align 4
proc DrawArrayImg, buf:dword, buf_img:dword, ind:dword, color:dword
	pushad
		mov edi,dword[ind]
		imul edi,BUF_STRUCT_SIZE
		add edi,dword[buf_img]

		mov esi,dword[buf]
		xor eax,eax
		mov ax,buf2d_t
		xor ebx,ebx
		mov bx,buf2d_l
		stdcall [buf2d_bit_blt_alpha], esi, ebx,eax, edi,[color]
	popad
	ret
endp

align 4
DrawZaac: ;�ᮢ���� ����
	push eax
	bt word[pos_wolf],bit_zaac
	jae .end_mig

	stdcall DrawArrayImg, buf_displ,buf_wolf,6,[color_wolf] ;��㥬 ⥫� ����
	mov eax,dword[pos_eggs]
	and eax,mask_chi_right+mask_chi_left
	cmp eax,0
	je .end_mig ;�᫨ ��� ������� 樯���, � ���� �㪮� �� ����
	bt word[pos_wolf],bit_mig
	jc @f
		stdcall DrawArrayImg, buf_displ,buf_wolf,7,[color_wolf] ;��㥬 ��� ���� ������
		jmp .end_mig
	@@:
		stdcall DrawArrayImg, buf_displ,buf_wolf,8,[color_wolf] ;��㥬 ��� ���� �����
	.end_mig:
	pop eax
	ret

align 4
DrawWolf: ;���� ����� � ��২����
	bt word[pos_wolf],0
	jc @f
		stdcall DrawArrayImg, buf_displ,buf_wolf,0,[color_wolf]
		bt word[pos_wolf],1
		jc .corz_ldn
			stdcall DrawArrayImg, buf_displ,buf_wolf,1,[color_wolf]
			jmp .corz_lend
		.corz_ldn:
			stdcall DrawArrayImg, buf_displ,buf_wolf,2,[color_wolf]
		.corz_lend:
		jmp .wolf_b
	@@:
		stdcall DrawArrayImg, buf_displ,buf_wolf,3,[color_wolf]
		bt word[pos_wolf],1
		jc .corz_rdn
			stdcall DrawArrayImg, buf_displ,buf_wolf,4,[color_wolf]
			jmp .corz_rend
		.corz_rdn:
			stdcall DrawArrayImg, buf_displ,buf_wolf,5,[color_wolf]
		.corz_rend:
	.wolf_b:
	ret

align 4
DrawEggs: ;���� ��
	pushad
	cld

	mov eax,dword[pos_eggs]
	xor ebx,ebx
	mov ecx,20 ;横� ��� �ᮢ���� �������� ��
	@@:
		bt eax,ebx
		jae .draw_egg
			stdcall DrawArrayImg, buf_displ,buf_egg,ebx,[color_egg]
		.draw_egg:
		inc ebx
		loop @b

	;mov eax,dword[pos_eggs]
	and eax,mask_chi_left
	cmp eax,0
	je @f
		stdcall DrawArrayImg, buf_displ,buf_egg,20,[color_egg] ;ࠧ��⮥ �� ᫥��
	@@:
	mov eax,dword[pos_eggs]
	and eax,mask_chi_right
	cmp eax,0
	je @f
		stdcall DrawArrayImg, buf_displ,buf_egg,21,[color_egg] ;ࠧ��⮥ �� �ࠢ�
	@@:

	mov eax,dword[pos_eggs]
	xor edx,edx
	mov ebx,bit_chi_left
	mov ecx,5 ;横� ��� �ᮢ���� ����� 樯���
	@@:
		bt eax,ebx
		jae .draw_chick_l
			stdcall DrawArrayImg, buf_displ,buf_chi,edx,[color_chick]
		.draw_chick_l:
		inc ebx
		inc edx
		loop @b
	mov ebx,bit_chi_right
	mov ecx,5 ;横� ��� �ᮢ���� �ࠢ�� 樯���
	@@:
		bt eax,ebx
		jae .draw_chick_r
			stdcall DrawArrayImg, buf_displ,buf_chi,edx,[color_chick]
		.draw_chick_r:
		inc ebx
		inc edx
		loop @b

	xor eax,eax
	mov al,byte[count_last]
	mov ecx,eax ;横� ��� �ᮢ���� ������ �窮�
	shr ecx,1
	mov edx,10 ;������ ��砫� ������ ������ � ���� buf_chi
	cmp ecx,0
	je .no_unit_last
	@@:
		stdcall DrawArrayImg, buf_displ,buf_chi,edx,[color_chick]
		inc edx
		loop @b
	.no_unit_last:

	bt ax,0 ;�஢��塞 �����饥 �� �⭮� �窮
	jae @f
		bt word[pos_wolf],bit_mig
		jc @f
			stdcall DrawArrayImg, buf_displ,buf_chi,edx,[color_chick] ;�����饥 ���䭮� �窮
	@@:

	popad
	ret

align 4
CountEggsInc: ;㢥��稢��� ���稪 �� �� 1
	push eax edi
		xor eax,eax
		inc word[eggs_count] ;㢥��稢��� ���稪 ��
		cmp word[eggs_count],200 ;������ �窨
		je @f
		cmp word[eggs_count],500 ;������ �窨
		je @f
			jmp .no_bonus
		@@:
			mov byte[count_last],0
		.no_bonus:

		mov ax,word[eggs_count]
		mov edi,some_text
		call convert_to_str ;������塞 ⥪�⮢�� ��ப�
		and ax,0xf ;�१ ����� 16 �� 㬥��蠥� �६�
		cmp ax,0
		jne @f
			mov edi,[game_delay_min] ;�������쭠� ����প�
			cmp dword[game_spd],edi
			jle @f
				dec dword[game_spd]
		@@:
	pop edi eax
	ret

;input:
; eax - ��᪠, 㪠�뢠��� � ����� ��஭� 㯠�� ��
align 4
CountLastInc: ;���᫥��� ������ �窮�
	inc byte[count_last] ;��������� ���䭮�� �窠
	bt word[pos_wolf],bit_zaac
	jc @f
		inc byte[count_last] ;�᫨ ��� ���� � �� 1-�� �������� ���䭮�� �窠
	@@:
	cmp byte[count_last],6
	jle @f
		mov byte[count_last],6 ;�⠢�� ��࠭�祭� �� �᫮ ������ �窮�
	@@:
	or dword[pos_eggs],eax ;ᮧ���� ����饣� 樯�����
	ret

align 4
MoveEggs:
	pushad
	xor byte[pos_wolf],val_mig ;��� ��� �������

	cmp byte[count_last],6 ;���ᨬ��쭮� �᫮ ������ �窮�
	je .end_fun

	rol dword[pos_eggs],1
	mov ecx,dword[pos_eggs]
	and ecx,mask_fail_eggs
	cmp ecx,0
	je .no_fail ;��� ������� ��
		;���᫥��� �� ������� � ��২��
		;��� ���᫥��� ������ �窮�
		xor ebx,ebx
		mov bl,byte[pos_wolf] ;��६ ��ࠬ���� (������) �����
		and bl,3 ;�� ��直� ��砩

		bt ecx,5 ;�஢��塞 ���孥� ����� ��ਫ�
		jae .perilo_lu
			cmp bx,0 ;�஢��塞 ����稥 ��২��
			jne @f
				call CountEggsInc
				jmp .perilo_lu
			@@:
				mov eax,mask_chi_cr_l
				call CountLastInc
			.perilo_lu:
		bt ecx,10 ;�஢��塞 ������ ����� ��ਫ�
		jae .perilo_ld
			cmp bx,2 ;�஢��塞 ����稥 ��২��
			jne @f
				call CountEggsInc
				jmp .perilo_ld
			@@:
				mov eax,mask_chi_cr_l
				call CountLastInc
		.perilo_ld:
		bt ecx,15 ;�஢��塞 ���孥� �ࠢ�� ��ਫ�
		jae .perilo_ru
			cmp bx,1 ;�஢��塞 ����稥 ��২��
			jne @f
				call CountEggsInc
				jmp .perilo_ru
			@@:
				mov eax,mask_chi_cr_r
				call CountLastInc
		.perilo_ru:
		bt ecx,20 ;�஢��塞 ������ �ࠢ�� ��ਫ�
		jae .perilo_rd
			cmp bx,3 ;�஢��塞 ����稥 ��২��
			jne @f
				call CountEggsInc
				jmp .perilo_rd
			@@:
				mov eax,mask_chi_cr_r
				call CountLastInc
		.perilo_rd:
	.no_fail:

	and dword[pos_eggs],mask_clear_all ;���⪠ 㯠��� �� � ��������� ��३

	call rand_next
	cmp byte[zaac_status],0
	jle @f
		dec byte[zaac_status]
		jmp .no_zaac_move ;���� ���� �� ���������
	@@:
	
	bt dword[rand_x],6 ;���� �� 䮭��� ����� �����
	jc @f
		xor byte[pos_wolf],val_zaac ;��ᮢ뢠��/��ᮢ뢠�� ����
		bt word[pos_wolf],val_zaac
		jc .zaac_n
			mov byte[zaac_status],val_zaac_time_y ;�⠢�� �������쭮� �६� ��� ᬥ�� �����
			jmp @f
		.zaac_n:
			mov byte[zaac_status],val_zaac_time_n ;�⠢�� �������쭮� �६� ��� ᬥ�� �����
	@@:
	.no_zaac_move:

	;ᮧ����� ����� ��
	cmp word[eggs_count],5 ;���� 5 �� ������� �� 1-��
	jge @f
		mov ecx,dword[pos_eggs]
		and ecx,mask_move_eggs
		;cmp ecx,0
		jnz .end_creat
	@@:

	bt dword[rand_x],4 ;�஢��塞 �㤥� �� ᮧ������ ����� ��
	jc .end_creat
	bt dword[rand_x],5 ;�஢��塞 � ����� ��஭� �㤥� ᮧ������ ����� ��
	jc .creat_r
		bt dword[rand_x],7
		jc @f
			or dword[pos_eggs],mask_lot_lu
			jmp .end_creat
		@@:
			or dword[pos_eggs],mask_lot_ld
			jmp .end_creat
	.creat_r:
		bt dword[rand_x],7
		jc @f
			or dword[pos_eggs],mask_lot_ru
			jmp .end_creat
		@@:
			or dword[pos_eggs],mask_lot_rd
			;jmp .end_creat
	.end_creat:

	.end_fun:
	popad
	ret

align 4
proc InitGame, b:dword ;��ࢮ��砫�� ����ன�� ����
	mov word[eggs_count],0 ;�������⢮ �������� ��
	mov byte[some_text],'0'
	mov byte[some_text+1],0 ;⥪�� � �᫮� ��������� ��
	mov byte[count_last],0 ;����� �窨
	mov dword[pos_eggs],0
	mov byte[zaac_status],0

	push eax ebx
	cmp dword[b],0
	jne @f
		mov byte[game_text+5],'�'
		mov eax,dword[game_delay_a]
		mov dword[game_spd],eax ;����প� ����
		jmp .end_init
	@@:
		mov byte[game_text+5],'�'
		mov eax,dword[game_delay_b]
		mov dword[game_spd],eax ;����প� ����
	.end_init:

	mcall SF_SYSTEM_GET,SSF_TIME_COUNT
	mov [rand_x],eax ;������塞 1-� ��砩��� �᫮
	pop ebx eax

	ret
endp

align 4
proc LoadArrayBuffer, f_name:dword, buf_start:dword, count:dword
	pushad
	mov edx,[displ_bytes]
	mov ecx,edx
	imul ecx,[count]
	mov eax,[f_name]
	load_image_file eax,image_data_gray,ecx
		mov edx,[displ_bytes]
		mov eax,[image_data_gray]
		mov edi,[buf_start]
		mov ecx,[count]
		cld
		@@: ;���뢠�� 3 ���� � ������ﬨ
			stdcall CreateTrapharetBuffer,edi,eax
			add eax,edx
			add edi,BUF_STRUCT_SIZE
			loop @b
	stdcall mem.Free,[image_data_gray] ;�᢮������� ������
	popad
	ret
endp

txt_tile_type_0 rb FILE_NAME_MAX

align 4
user_is_select:
	push eax ecx esi edi
	stdcall [tl_node_get_data], tree1
	or eax,eax ;�᫨ ��� ���� ���⮥
	jz @f
		mov esi,eax	
		mov edi,ini_name
		mov ecx,FILE_NAME_MAX
		cld
		rep movsb

		mov byte[game_select_mode],0
		call InitAll
	@@:
	pop edi esi ecx eax

	call draw_window
	ret

;******************************************************************************
; �㭪��, ����� 1 ࠧ ������ �� ����ன�� �㦭� ��� ����
;******************************************************************************
align 4
InitAll:
	pushad
	;ࠡ�� � 䠩��� ����஥�
	copy_path ini_name,sys_path,file_name,0
	stdcall [ini_get_int],file_name,ini_sec_files,key_displ_w,210
	mov	[displ_w],eax
	stdcall [ini_get_int],file_name,ini_sec_files,key_displ_h,140
	mov	[displ_h],eax
	stdcall [ini_get_str],file_name,ini_sec_files,key_file_decorat,fn_icon0,FILE_NAME_MAX,ini_def_decorat_file
	stdcall [ini_get_str],file_name,ini_sec_files,key_file_unit,fn_icon1,FILE_NAME_MAX,ini_def_unit_file
	stdcall [ini_get_str],file_name,ini_sec_files,key_file_objects,fn_icon2,FILE_NAME_MAX,ini_def_objects_file
	stdcall [ini_get_str],file_name,ini_sec_files,key_file_lost,fn_icon3,FILE_NAME_MAX,ini_def_lost_file

	stdcall [ini_get_int],file_name,ini_sec_files,key_shadow_x,2
	mov	[offs_shadow_x],eax
	stdcall [ini_get_int],file_name,ini_sec_files,key_shadow_y,2
	mov	[offs_shadow_y],eax

	;���뢠��� ����஥� ������� �� ᪮���� ����
	stdcall [ini_get_int],file_name,ini_sec_game,key_delay_a,65
	mov	[game_delay_a],eax
	stdcall [ini_get_int],file_name,ini_sec_game,key_delay_b,35
	mov	[game_delay_b],eax
	stdcall [ini_get_int],file_name,ini_sec_game,key_delay_min,15
	mov	[game_delay_min],eax
	
	stdcall [ini_get_color],file_name,ini_sec_color,key_color_fon,0xffffff
	mov	[color_fon],eax
	stdcall [ini_get_color],file_name,ini_sec_color,key_color_shadows,0xd0d0d0
	mov	[color_shadows],eax
	stdcall [ini_get_color],file_name,ini_sec_color,key_color_unit,0
	mov	[color_wolf],eax
	stdcall [ini_get_color],file_name,ini_sec_color,key_color_egg,0x404080
	mov	[color_egg],eax
	stdcall [ini_get_color],file_name,ini_sec_color,key_color_chick,0x00d0d0
	mov	[color_chick],eax

	mov ebx,color_decorat
	mov byte[key_color_decorat.ind],'0'
	mov ecx,3
	cld
	@@:
		push ecx ;�㭪�� ini_get_color ����� �ࠢ� ������ ॣ���� ecx
		stdcall [ini_get_color],file_name,ini_sec_color,key_color_decorat,0x000080
		pop ecx
		mov [ebx],eax
		add ebx,4
		inc byte[key_color_decorat.ind]
		loop @b

	mov edx,[displ_w]
	imul edx,[displ_h]
	lea edx,[edx+edx*2]
	mov [displ_bytes],edx ;����塞 ࠧ��� ��஢��� ����

	stdcall LoadArrayBuffer, fn_icon0, buf_decor,3 ;���뢠�� 3 ���� � ������ﬨ
	stdcall LoadArrayBuffer, fn_icon1, buf_wolf,9 ;���뢠�� 9 ���஢ � ������ � ���楬
	stdcall LoadArrayBuffer, fn_icon2, buf_egg,22 ;���뢠�� 22 ���஢ � �栬�
	stdcall LoadArrayBuffer, fn_icon3, buf_chi,13 ;���뢠�� 13 ���஢ � 樯��⠬�

	load_image_file 'font8x9.bmp', image_data_gray,IMAGE_FONT_SIZE
	stdcall [buf2d_create_f_img], buf_font,[image_data_gray] ;ᮧ���� ����
	stdcall mem.Free,[image_data_gray] ;�᢮������� ������

	stdcall [buf2d_conv_24_to_8], buf_font,1 ;������ ���� �஧�筮�� 8 ���
	stdcall [buf2d_convert_text_matrix], buf_font

;�஢�ઠ ����������� ���஬ ����
;mov edi,buf_wolf
;add edi,BUF_STRUCT_SIZE ;���室�� �� ���� ��২��
;stdcall [buf2d_clear],edi,0x808080 ;�������� ��� ��� 梥⮬

	mov ebx,dword[displ_w]
	mov edx,dword[displ_h]

	mov edi,buf_displ
	mov buf2d_w,ebx
	mov buf2d_h,edx
	stdcall [buf2d_create], buf_displ ;ᮧ���� ���� ��� �뢮�� �� ��࠭

	mov edi,buf_fon
	mov buf2d_w,ebx
	mov buf2d_h,edx
	stdcall [buf2d_create], buf_fon ;ᮧ���� ���� � 䮭��묨 ������ﬨ

	call InitBackgroundBuffer ;������塞 ���� � 䮭��묨 ������ﬨ
	stdcall InitGame,0
	popad
	ret

align 4
start:
	load_libraries l_libs_start,load_lib_end

	;�஢�ઠ �� ᪮�쪮 㤠筮 ���㧨���� ������⥪�
	cmp	dword [lib0+ll_struc_size-4],0
	jnz @f
	cmp	dword [lib1+ll_struc_size-4],0
	jnz @f
	cmp	dword [lib2+ll_struc_size-4],0
	jnz @f
	cmp	dword [lib3+ll_struc_size-4],0
	jnz @f
	jmp .lib
	@@:
		mcall SF_TERMINATE_PROCESS
	.lib:

	mcall SF_SET_EVENTS_MASK,0x27
	mcall SF_STYLE_SETTINGS,SSF_GET_COLORS,sc,sizeof.system_colors

;******************************************************************************
; �����⮢�� ᯨ᪠ ���
;******************************************************************************
	stdcall dword[tl_data_init], tree1

	load_image_file 'tl_sys_16.png', image_data_gray,TREE_ICON_SYS16_BMP_SIZE
	stdcall [buf2d_create_f_img], buf_tree_sys,[image_data_gray] ;ᮧ���� ����
	stdcall mem.Free,[image_data_gray] ;�᢮������� ������

	mov edi,buf_tree_sys
	m2m dword[tree1.data_img_sys],buf2d_data

	;ࠡ�� � ������ 䠩��� ����஥�
	copy_path ini_m_name,sys_path,file_name,0

	stdcall [ini_get_int],file_name,ini_sec_files,key_count,1
	mov	ecx,eax
	mov dl,'0'
	cld
	@@:
		mov byte[key_game_ind],dl
		inc dl
		push ecx edx
		stdcall [ini_get_str],file_name,ini_sec_files,key_game,txt_tile_type_0,FILE_NAME_MAX,ini_def_decorat_file
		stdcall [tl_node_add], tree1, 0, txt_tile_type_0 ;������塞 �������� ����
		stdcall [tl_cur_next], tree1 ;��७�ᨬ ����� ����, ��-�� �� �������� ���冷� ���
		pop edx ecx
	loop @b
	stdcall [tl_cur_beg], tree1 ;��७�ᨬ ����� �����

	mcall SF_SYSTEM_GET,SSF_TIME_COUNT
	mov [last_time],eax



align 4
red_win:
	call draw_window

align 4
still: ;������ 横�

	cmp byte[game_select_mode],0
	jne .select_mode
	
	mcall SF_SYSTEM_GET,SSF_TIME_COUNT
	mov ebx,[last_time]
	add ebx,[game_spd] ;delay
	sub ebx,eax
	cmp ebx,[game_spd] ;delay
	ja it_is_time_now
	test ebx,ebx
	jz it_is_time_now
	mcall SF_WAIT_EVENT_TIMEOUT
	or eax,eax
	jz it_is_time_now

	jmp @f
	.select_mode:
		mcall SF_WAIT_EVENT
	@@:

	cmp al,1 ;���������� ��������� ����
	jz red_win
	cmp al,2
	jz key
	cmp al,3
	jz button
	cmp al,6
	jz mouse

	jmp still

align 4
mouse:
	cmp byte[game_select_mode],0
	je @f
		stdcall [tl_mouse], tree1 ;�᫨ ��� �� �� ��࠭�
	@@:
	jmp still

align 4
it_is_time_now:
	mcall SF_SYSTEM_GET,SSF_TIME_COUNT
	mov [last_time],eax

	;cmp byte[game_select_mode],0
	;jne still

	;...����� ���� ����⢨�, ��뢠��� ����� delay ���� ����� ᥪ㭤...
	call MoveEggs
	call draw_display
	jmp still

align 4
key:
	push eax ebx
	mcall SF_GET_KEY

	cmp byte[game_select_mode],0
	je @f
		stdcall [tl_key], tree1
	@@:

	cmp ah,176 ;Left
	jne @f
		and byte[pos_wolf],0xfe
	@@:
	cmp ah,179 ;Right
	jne @f
		or byte[pos_wolf],1
	@@:
	cmp ah,178 ;Up
	jne @f
		and byte[pos_wolf],0xff-2
	@@:
	cmp ah,177 ;Down
	jne @f
		or byte[pos_wolf],2
	@@:

	cmp ah,97 ;a
	jne @f
		and byte[pos_wolf],0xff-3 ;00
	@@:
	cmp ah,122 ;z
	jne @f
		and byte[pos_wolf],0xff-1
		or byte[pos_wolf],2 ;10
	@@:
	cmp ah,39 ;'
	jne @f
		and byte[pos_wolf],0xff-2
		or byte[pos_wolf],1 ;01
	@@:
	cmp ah,47 ;/
	jne @f
		or byte[pos_wolf],3 ;11
	@@:

	pop ebx eax
	jmp still


align 4
draw_window:
	pushad
	mcall SF_REDRAW,SSF_BEGIN_DRAW

	xor eax,eax
	mov ebx,20*65536+480
	mov ecx,20*65536+270
	mov edx,[sc.work]
	;or edx,0x33000000
	or edx,0x73000000
	mov edi,hed
	mcall ;ᮧ����� ����
	mcall SF_THREAD_INFO,procinfo,-1

	cmp byte[game_select_mode],0
	jne .select_mode

	mov edi,buf_displ
	mov eax,[procinfo.client_box.width]
	cmp eax,[displ_w]
	jle @f
		sub eax,[displ_w]
		shr eax,1
		mov buf2d_l,ax ;��ࠢ������ ���� �� 業��� ����
	@@:

	call draw_display

	mov eax,SF_DRAW_RECT
	mov edx,[sc.work]
	xor esi,esi
	mov si,buf2d_l
	add esi,[displ_w]
	mov ebx,[procinfo.client_box.width]
	inc ebx
	cmp esi,ebx
	jge @f
		sub ebx,esi
		rol ebx,16
		mov bx,si
		rol ebx,16
		mov ecx,[procinfo.client_box.height]
		inc ecx
		int 0x40 ;�ᮢ���� �ࠢ��� �������� ����
		jmp .draw_s
	@@:
		mov esi,[procinfo.client_box.width] ;����� �� �ਭ� �� ������
		inc esi
	.draw_s:

	; *** �ᮢ���� ������ ***
push esi
	movzx ebx,buf2d_l
	add ebx,buf2d_w
	add ebx,BUT1_L
	shl ebx,16
	mov bx,BUT1_W
	mov ecx,BUT1_T*65536+BUT1_H
	mov esi,[color_but_sm]
	mcall SF_DEFINE_BUTTON,,,5

	inc edx
	add ecx,BUT1_NEXT_TOP
	int 0x40
pop esi

	mov bx,BUT1_H-4
	add ebx,8 shl 16
	mov ecx,[color_but_te]
	or  ecx,0x80000000
	mov edx,txt_game_a
	mcall SF_DRAW_TEXT

	ror ebx,16
	add ebx,BUT1_NEXT_TOP
	ror ebx,16
	mov edx,txt_game_b
	int 0x40

	; *** ����⠭������� ��ࠬ��஢ ***
	mov eax,SF_DRAW_RECT
	mov edx,[sc.work]

	mov ebx,esi
	mov ecx,[procinfo.client_box.height]
	inc ecx
	mov esi,[displ_h]
	cmp esi,ebx
	jge @f
		sub ecx,esi
		rol ecx,16
		mov cx,si
		rol ecx,16
		int 0x40 ;�ᮢ���� ������� ����
	@@:
	
	xor ebx,ebx
	mov bx,buf2d_l
	mov ecx,[displ_h]
	int 0x40 ;�ᮢ���� ������ �������� ����

	jmp @f
	.select_mode:
		stdcall [tl_draw], tree1
		mov edi,tree1
		add edi,tl_offs_box
		stdcall draw_rect_border, procinfo.client_box, edi
	@@:

	mcall SF_REDRAW,SSF_END_DRAW
	popad
	ret

; �㭪�� ���� ���� ����� ��אַ㣮�쭨�� user_box
; ࠧ��� ����� �������� ��室� �� ࠧ��஢ client_rect
; �।����������, �� � ����設�⢥ ��砥� client_rect > user_box
align 4
proc draw_rect_border, client_rect:dword, user_box:dword
	pushad
	mov esi,[user_box]
	cmp esi,0
	je @f
		mov edi,[client_rect]
		mov ebx,dword[edi+8] ;+8 = width
		inc bx
		mov ecx,dword[esi+4] ;+4 = top
		mov edx,[sc.work]
		mcall SF_DRAW_RECT ;top

		mov eax,dword[esi+4] ;+4 = top
		add eax,dword[esi+12] ;+12 = height
		cmp eax,dword[edi+12]
		jge .no_bottom
			mov ecx,eax
			shl ecx,16
			mov cx,word[edi+12] ;+12 = bottom
			inc cx
			sub cx,ax
			mcall SF_DRAW_RECT ;bottom
		.no_bottom:

		mov ebx,dword[esi] ;+0 left
		mov ecx,dword[esi+4] ;+4 = top
		shl ecx,16
		mov cx,word[esi+12] ;+12 = height
		inc cx 
		mcall SF_DRAW_RECT ;left

		mov eax,dword[esi] ;+0 left
		add eax,dword[esi+8] ;+8 = width
		mov ebx,eax
		shl ebx,16
		mov bx,word[edi+8] ;+8 = right
		sub bx,ax
		inc bx
		mcall SF_DRAW_RECT ;right
	@@:
	popad
	ret
endp

align 4
draw_display:

	stdcall mem_copy, [buf_fon],[buf_displ],[displ_bytes] ;����஢���� ����ࠦ���� �� 䮭����� ����
	call DrawZaac ;��㥬 ����
	call DrawWolf ;��㥬 �����
	call DrawEggs ;��㥬 ��

push eax
	mov eax,[displ_w]
	shr eax,1
	stdcall [buf2d_draw_text], buf_displ, buf_font,game_text,eax,[offs_shadow_x],[color_wolf] ;��㥬 ��ப� � ⥪�⮬
pop eax
	stdcall [buf2d_draw], buf_displ
	ret

align 4
button:
	mcall SF_GET_BUTTON

	cmp ah,5
	jne @f
		stdcall InitGame,0
	@@:
	cmp ah,6
	jne @f
		stdcall InitGame,1
	@@:
	cmp ah,1
	jne still
.exit:
	stdcall [buf2d_delete],buf_fon ;㤠�塞 ����
	stdcall [buf2d_delete],buf_displ ;㤠�塞 ����

	stdcall [buf2d_delete],buf_tree_sys
	stdcall [buf2d_delete],buf_font

	cld
	mov ecx,3
	mov edi,buf_decor
	@@: ;㤠�塞 3 ����
		stdcall [buf2d_delete],edi
		add edi,BUF_STRUCT_SIZE
		loop @b
	mov ecx,9
	mov edi,buf_wolf
	@@: ;㤠�塞 9 ���஢ � ������ � ���楬
		stdcall [buf2d_delete],edi
		add edi,BUF_STRUCT_SIZE
		loop @b
	mov ecx,13
	mov edi,buf_chi
	@@: ;㤠�塞 13 ���஢
		stdcall [buf2d_delete],edi
		add edi,BUF_STRUCT_SIZE
		loop @b
	mov ecx,22
	mov edi,buf_egg
	@@: ;㤠�塞 22 ����
		stdcall [buf2d_delete],edi
		add edi,BUF_STRUCT_SIZE
		loop @b

	mov dword[tree1.data_img_sys],0 ;��⨬ 㪠��⥫� �� ����ࠦ����
	stdcall dword[tl_data_clear], tree1
	mcall SF_TERMINATE_PROCESS

system_dir0 db '/sys/lib/'
lib0_name db 'buf2d.obj',0
system_dir1 db '/sys/lib/'
lib1_name db 'libimg.obj',0
system_dir2 db '/sys/lib/'
lib2_name db 'libini.obj',0
system_dir3 db '/sys/lib/'
lib3_name db 'box_lib.obj',0

;library structures
l_libs_start:
	lib0 l_libs lib0_name, file_name, system_dir0, import_buf2d_lib
	lib1 l_libs lib1_name, file_name, system_dir1, import_libimg
	lib2 l_libs lib2_name, file_name, system_dir2, import_libini
	lib3 l_libs lib3_name, file_name, system_dir3, import_box_lib
load_lib_end:

align 4
proc mem_copy uses ecx esi edi, source:dword, destination:dword, len:dword
	cld
	mov esi, [source]
	mov edi, [destination]
	mov ecx, [len]
	rep movsb
	ret
endp

;input:
; eax - �᫮
; edi - ���� ��� ��ப�
align 4
convert_to_str:
	pushad
	lea esi,[edi+8] ;8 - ������ ���� -1
	call .str
	popad
	ret

align 4
.str:
	mov ecx,10
	cmp eax,ecx
	jb @f
		xor edx,edx
		div ecx
		push edx
		;dec edi  ;ᬥ饭�� ����室���� ��� ����� � ���� ��ப�
		call .str
		pop eax
	@@:
	cmp edi,esi
	jge @f
		or al,0x30
		stosb
		mov byte[edi],0 ;� ����� ��ப� �⠢�� 0, ��-�� �� �뫠��� ����
	@@:
	ret


last_time dd 0
image_data dd 0 ;������ ��� �८�ࠧ������ ���⨭�� �㭪�ﬨ libimg
image_data_gray dd 0 ;������ � �६���묨 ��묨 ����ࠦ���ﬨ � �ଠ� 24-bit, �� ������ ���� ᮧ�������� ������

run_file_70 FileInfoBlock
hed db 'Nu pogodi 10.12.20',0 ;������� ����
sc system_colors  ;��⥬�� 梥�

count_of_dir_list_files equ 10
el_focus dd tree1
tree1 tree_list FILE_NAME_MAX,count_of_dir_list_files+2, tl_key_no_edit+tl_draw_par_line+tl_list_box_mode,\
	16,16, 0x8080ff,0x0000ff,0xffffff, 10,10,140,100, 0,0,0, el_focus,\
	0,user_is_select

align 4
buf_tree_sys:
	dd 0 ;㪠��⥫� �� ���� ����ࠦ����
	dw 0 ;+4 left
	dw 0 ;+6 top
	dd 16 ;+8 w
	dd 16*11 ;+12 h
	dd 0 ;+16 color
	db 24 ;+20 bit in pixel

align 4
buf_font: ;���� � ���⮬
	dd 0 ;㪠��⥫� �� ���� ����ࠦ����
	dw 25 ;+4 left
	dw 25 ;+6 top
	dd 128 ;+8 w
	dd 144 ;+12 h
	dd 0 ;+16 color
	db 24 ;+20 bit in pixel

align 4
buf_displ:
	dd 0 ;㪠��⥫� �� ���� ����ࠦ����
	dw 25,0
	dd ? ;+8 w
	dd ? ;+12 h
	dd 0 ;+16 color
	db 24 ;+20 bit in pixel

align 4
buf_fon: ;䮭��� ����
	dd 0 ;㪠��⥫� �� ���� ����ࠦ����
	dw 0 ;+4 left
	dw 0 ;+6 top
	dd ? ;+8 w
	dd ? ;+12 h
	dd 0xffffff ;+16 color
	db 24 ;+20 bit in pixel

align 4
buf_decor: ;���� � ������ﬨ: �������� � ३����; � ���栬�; � ��⥭�ﬨ
	rb 3*BUF_STRUCT_SIZE

align 4
buf_wolf:
	rb 9*BUF_STRUCT_SIZE

align 4
buf_egg:
	rb 22*BUF_STRUCT_SIZE

align 4
buf_chi:
	rb 13*BUF_STRUCT_SIZE



align 4
import_libini:
	dd alib_init0
	ini_get_str   dd aini_get_str
	ini_get_int   dd aini_get_int
	ini_get_color dd aini_get_color
dd 0,0
	alib_init0     db 'lib_init',0
	aini_get_str   db 'ini_get_str',0
	aini_get_int   db 'ini_get_int',0
	aini_get_color db 'ini_get_color',0

align 4
import_libimg:
  dd alib_init1
  img_is_img  dd aimg_is_img
  img_info    dd aimg_info
  img_from_file dd aimg_from_file
  img_to_file dd aimg_to_file
  img_from_rgb dd aimg_from_rgb
  img_to_rgb  dd aimg_to_rgb
  img_to_rgb2 dd aimg_to_rgb2
  img_decode  dd aimg_decode
  img_encode  dd aimg_encode
  img_create  dd aimg_create
  img_destroy dd aimg_destroy
  img_destroy_layer dd aimg_destroy_layer
  img_count   dd aimg_count
  img_lock_bits dd aimg_lock_bits
  img_unlock_bits dd aimg_unlock_bits
  img_flip    dd aimg_flip
  img_flip_layer dd aimg_flip_layer
  img_rotate  dd aimg_rotate
  img_rotate_layer dd aimg_rotate_layer
  img_draw    dd aimg_draw

  dd 0,0
  alib_init1   db 'lib_init',0
  aimg_is_img  db 'img_is_img',0 ;��।���� �� �����, ����� �� ������⥪� ᤥ���� �� ��� ����ࠦ����
  aimg_info    db 'img_info',0
  aimg_from_file db 'img_from_file',0
  aimg_to_file db 'img_to_file',0
  aimg_from_rgb db 'img_from_rgb',0
  aimg_to_rgb  db 'img_to_rgb',0 ;�८�ࠧ������ ����ࠦ���� � ����� RGB
  aimg_to_rgb2 db 'img_to_rgb2',0
  aimg_decode  db 'img_decode',0 ;��⮬���᪨ ��।���� �ଠ� ����᪨� ������
  aimg_encode  db 'img_encode',0
  aimg_create  db 'img_create',0
  aimg_destroy db 'img_destroy',0
  aimg_destroy_layer db 'img_destroy_layer',0
  aimg_count   db 'img_count',0
  aimg_lock_bits db 'img_lock_bits',0
  aimg_unlock_bits db 'img_unlock_bits',0
  aimg_flip    db 'img_flip',0
  aimg_flip_layer db 'img_flip_layer',0
  aimg_rotate  db 'img_rotate',0
  aimg_rotate_layer db 'img_rotate_layer',0
  aimg_draw    db 'img_draw',0

align 4
import_buf2d_lib:
	dd sz_lib_init
	buf2d_create dd sz_buf2d_create
	buf2d_create_f_img dd sz_buf2d_create_f_img
	buf2d_clear dd sz_buf2d_clear
	buf2d_draw dd sz_buf2d_draw
	buf2d_delete dd sz_buf2d_delete
	buf2d_line dd sz_buf2d_line
	buf2d_circle dd sz_buf2d_circle
	buf2d_img_hdiv2 dd sz_buf2d_img_hdiv2
	buf2d_img_wdiv2 dd sz_buf2d_img_wdiv2
	buf2d_conv_24_to_8 dd sz_buf2d_conv_24_to_8
	buf2d_conv_24_to_32 dd sz_buf2d_conv_24_to_32
	buf2d_bit_blt dd sz_buf2d_bit_blt
	buf2d_bit_blt_transp dd sz_buf2d_bit_blt_transp
	buf2d_bit_blt_alpha dd sz_buf2d_bit_blt_alpha
	buf2d_curve_bezier dd sz_buf2d_curve_bezier
	buf2d_convert_text_matrix dd sz_buf2d_convert_text_matrix
	buf2d_draw_text dd sz_buf2d_draw_text
	buf2d_crop_color dd sz_buf2d_crop_color
	buf2d_offset_h dd sz_buf2d_offset_h	
	dd 0,0
	sz_lib_init db 'lib_init',0
	sz_buf2d_create db 'buf2d_create',0
	sz_buf2d_create_f_img db 'buf2d_create_f_img',0
	sz_buf2d_clear db 'buf2d_clear',0
	sz_buf2d_draw db 'buf2d_draw',0
	sz_buf2d_delete db 'buf2d_delete',0
	sz_buf2d_line db 'buf2d_line',0
	sz_buf2d_circle db 'buf2d_circle',0 ;�ᮢ���� ���㦭���
	sz_buf2d_img_hdiv2 db 'buf2d_img_hdiv2',0
	sz_buf2d_img_wdiv2 db 'buf2d_img_wdiv2',0
	sz_buf2d_conv_24_to_8 db 'buf2d_conv_24_to_8',0
	sz_buf2d_conv_24_to_32 db 'buf2d_conv_24_to_32',0
	sz_buf2d_bit_blt db 'buf2d_bit_blt',0
	sz_buf2d_bit_blt_transp db 'buf2d_bit_blt_transp',0
	sz_buf2d_bit_blt_alpha db 'buf2d_bit_blt_alpha',0
	sz_buf2d_curve_bezier db 'buf2d_curve_bezier',0
	sz_buf2d_convert_text_matrix db 'buf2d_convert_text_matrix',0
	sz_buf2d_draw_text db 'buf2d_draw_text',0
	sz_buf2d_crop_color db 'buf2d_crop_color',0
	sz_buf2d_offset_h db 'buf2d_offset_h',0

align 4
import_box_lib:
	dd alib_init2

	;scrollbar_ver_draw  dd aScrollbar_ver_draw

	tl_data_init dd sz_tl_data_init
	tl_data_clear dd sz_tl_data_clear
	tl_info_clear dd sz_tl_info_clear
	tl_key dd sz_tl_key
	tl_mouse dd sz_tl_mouse
	tl_draw dd sz_tl_draw
	tl_info_undo dd sz_tl_info_undo
	tl_info_redo dd sz_tl_info_redo
	tl_node_add dd sz_tl_node_add
	tl_node_set_data dd sz_tl_node_set_data
	tl_node_get_data dd sz_tl_node_get_data
	tl_node_delete dd sz_tl_node_delete
	tl_cur_beg dd sz_tl_cur_beg
	tl_cur_next dd sz_tl_cur_next
	tl_cur_perv dd sz_tl_cur_perv
	tl_node_close_open dd sz_tl_node_close_open
	tl_node_lev_inc dd sz_tl_node_lev_inc
	tl_node_lev_dec dd sz_tl_node_lev_dec

dd 0,0
	alib_init2 db 'lib_init',0

	;aScrollbar_ver_draw  db 'scrollbar_v_draw',0

	sz_tl_data_init db 'tl_data_init',0
	sz_tl_data_clear db 'tl_data_clear',0
	sz_tl_info_clear db 'tl_info_clear',0
	sz_tl_key db 'tl_key',0
	sz_tl_mouse db 'tl_mouse',0
	sz_tl_draw db 'tl_draw',0
	sz_tl_info_undo db 'tl_info_undo',0
	sz_tl_info_redo db 'tl_info_redo',0
	sz_tl_node_add db 'tl_node_add',0
	sz_tl_node_set_data db 'tl_node_set_data',0
	sz_tl_node_get_data db 'tl_node_get_data',0
	sz_tl_node_delete db 'tl_node_delete',0
	sz_tl_cur_beg db 'tl_cur_beg',0
	sz_tl_cur_next db 'tl_cur_next',0
	sz_tl_cur_perv db 'tl_cur_perv',0
	sz_tl_node_close_open db 'tl_node_close_open',0
	sz_tl_node_lev_inc db 'tl_node_lev_inc',0
	sz_tl_node_lev_dec db 'tl_node_lev_dec',0


align 16
i_end:
	procinfo process_information
	rb 1024
stacktop:
	sys_path rb 4096
	file_name:
		rb 4096
	openfile_path:
		rb 4096
	filename_area:
		rb 256
mem:

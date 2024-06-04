use32
	org 0
	db 'MENUET01'
	dd 1,start,i_end,mem,stacktop,0,sys_path

include '../../../macros.inc'
include '../../../proc32.inc'
include '../../../KOSfuncs.inc'
include '../../../load_img.inc'
include '../../../load_lib.mac'

;include 'lang.inc'  ; Language support for locales: ru_RU (CP866), en_US.

@use_library mem.Alloc,mem.Free,mem.ReAlloc,dll.Load
hed db 'Life 18.11.20',0 ;������� ����

run_file_70 FileInfoBlock
image_data dd 0 ;㪠��⥫� �� �६����� ������. ��� �㦥� �८�ࠧ������ ����ࠦ����

ICONSIZE = 21
TBTNSIZE = ICONSIZE-2

IMAGE_TOOLBAR_ICON_SIZE equ ICONSIZE*ICONSIZE*3
image_data_toolbar dd 0


;--------------------------------------
struct Cell
	x dd ? ;+0
	y dd ? ;+4
	tc  dd ? ;+8 ��������� � ���஬ த����� �ᮡ�
	liv db ? ;+12 ����� �祩�� ��� ���
	so  db ? ;+13 �᫮ �ᥤ��
ends

MAX_CELL equ 90000 ;������ ���� ��⭮ 4
COL_MEM equ 64 ;�᫮ 梥⮢ ��� �⫨�� ������� � �����

cell dd 0 ;㪠��⥫� �� ������ � ������ࠬ� �祥�
memCell dd 0
CellColors dd 0

macro get_cell_offset reg,ind
{
	mov reg,ind
	imul reg,sizeof.Cell
	add reg,dword[cell]
}

er_oom db 0 ;�� ��砩 ���௠��� �����
tim_ch db 0 ;��⮬���᪨ �����뢠�� ���������
poc_stop dd 1 ;����� �� �᫮ ���������
Cor_x dd 0
Cor_y dd 0
tim dd 0 ;�६� (���������)
b_sort dd 0 ;�࠭�� ��� ���஢����� �祥�
osob dd 0 ;�᫮ �ᮡ��
zoom db 3 ;����⠡ ����

if lang eq ru_RU
txt_zoom db '   ����⠡:',0
txt_gen db '  ���������:',0
txt_osob db '    �ᮡ��:',0
else ; Default to en_US
txt_zoom db '      Zoom:',0
txt_gen db  'Generation:',0
txt_osob db 'Population:',0
end if

;����ன�� ���ᨢ� � 梥⠬�
; col_pole - 梥� ����
; col_cell_n - 梥� ����� �祩��
; col_cell_o - 梥� ��ன �祩��
align 4
proc pole_init_colors uses eax ebx ecx edx esi edi, col_pole:dword, col_cell_n:dword, col_cell_o:dword
	mov esi,[CellColors]
	mov ebx,[col_pole]
	mov [esi],ebx

	lea edi,[esi+4*COL_MEM]
	add esi,4
	; esi - 㪠��⥫� �� 1-� �ࠤ����� 梥�
	; edi - 㪠��⥫� �� ��᫥���� �ࠤ����� 梥�
	mov eax,[col_cell_n]
	mov ebx,[col_cell_o]

	mov [esi],eax
	mov [edi],ebx
	;need save ecx edx
	stdcall middle_colors, esi,edi

	ret
endp

;�ᯮ����⥫쭠� �㭪�� ��� ��室����� �।���� 梥� � ����� ��� � ���ᨢ
;input:
; eax - 梥� ��砫��
; ebx - 梥� ������
;�����������: ecx, edx
align 4
proc middle_colors uses edi esi, i0:dword, i1:dword
	mov esi,[i0]
	mov edi,[i1]
	;��। �맮��� �㭪樨
	;dword[esi]=eax
	;dword[edi]=ebx
	sub edi,esi
	shr edi,1
	btr edi,1 ;���㣫塞 �� 4-�, �. �. �㦭� ������� ���� (���㣫���� ��஥ - 㡨ࠥ� ���� ��� ����� �।���������� 2-�)
	add edi,esi
	cmp edi,esi
	je @f
		push eax ebx

		mov ecx,eax
		mov edx,ebx

		;��室�� �।��� 梥� ����� eax � ebx
		and ebx,111111101111111011111110b ;㡨ࠥ� ��᫥���� ���� � 梥�� r, g, b
		and eax,111111101111111011111110b
		add eax,ebx ;�㬨�㥬 梥� �� r, g, b
		shr eax,1   ;����� �� 2
		mov dword[edi],eax

		;४��ᨢ�� �맮� ��� �஡����� ���孥� ��������
		mov ebx,eax
		mov eax,ecx
		stdcall middle_colors, [i0],edi

		;४��ᨢ�� �맮� ��� �஡����� ������ ��������
		mov eax,ebx
		mov ebx,edx
		stdcall middle_colors, edi,[i1]

		pop ebx eax
	@@:
	ret
endp

align 4
proc pole_clear uses eax ecx edi
	xor eax,eax
	mov dword[tim],eax
	mov dword[osob],eax
	mov  byte[tim_ch],al
	mov dword[Cor_x],eax
	mov dword[Cor_y],eax
	mov dword[b_sort],eax
	mov  byte[er_oom],al
	cld
	mov ecx,(MAX_CELL*sizeof.Cell)/4
	mov edi,[cell]
	rep stosd ;memset(cell,0,sizeof(Cell)*MAX_CELL);
	mov edi,[memCell]
	mov ecx,MAX_CELL
	@@:
		stosd ;for(i=0;i<MAX_CELL;i++) memCell[i]=i;
		inc eax
		loop @b
	ret
endp

align 4
proc pole_cell_creat, x:dword, y:dword, li:dword
	pushad ;eax ebx ecx edx edi

	; *** �᫨ ���⪠ 㦥 �뫠 ᮧ����
	stdcall pole_cell_find, [x],[y]
	or eax,eax
	jz @f ;if(i){
		get_cell_offset ebx,eax
		cmp dword[li],0
		jne .else_if ;if(!li)
			; �᫨ ᮧ������ ����� ���⪠
			inc byte[ebx+Cell.so]
			jmp .fun_e
		.else_if: ;else if(!(cell[i].liv&1) ){
		bt word[ebx+Cell.liv],0
			; �᫨ ᮧ������ ����� ���⪠
			; � ࠭�� ���⪠ �뫠 ᮧ���� �� ��������� ���⮩
			jae .creat_border_cells
		jmp .fun_e
	@@:

	; *** ᮧ����� ����� �祩��
	; ��室�� ����� ᢮������ �祩�� (i) ��� ���������� �����
	mov edi,[memCell]
	inc dword[edi]
	cmp dword[edi],MAX_CELL
	jne @f
		dec dword[edi]
		mov byte[tim_ch],0
		;... need call message: "eror out of memory" ...
		;... �뢮� ᮮ�饭�� ��९������� ���� ��������  ...
		mov byte[er_oom],0
		jmp .fun_e ;return;
	@@:
	mov eax,[edi]
	shl eax,2
	add eax,[memCell] ;eax -> memCell[firstC]
	get_cell_offset ebx,[eax]

	mov ecx,[x]
	mov dword[ebx],ecx ;+0 = .x
	mov edx,[y]
	mov dword[ebx+Cell.y],edx
	mov eax,[tim]
	mov dword[ebx+Cell.tc],eax
	mov byte[ebx+Cell.liv],0

	cmp dword[li],0
	jne @f
		mov byte[ebx+Cell.so],1
		jmp .fun_e
	@@:
	mov byte[ebx+Cell.so],0

	.creat_border_cells:
		inc dword[osob]
		or byte[ebx+Cell.liv],1
		mov ecx,[x]
		dec ecx
		mov edx,[y]
		dec edx
		stdcall pole_cell_creat,ecx,edx,0
		inc edx
		stdcall pole_cell_creat,ecx,edx,0
		inc edx
		stdcall pole_cell_creat,ecx,edx,0
		inc ecx
		stdcall pole_cell_creat,ecx,edx,0
		sub edx,2
		stdcall pole_cell_creat,ecx,edx,0
		inc ecx
		stdcall pole_cell_creat,ecx,edx,0
		inc edx
		stdcall pole_cell_creat,ecx,edx,0
		inc edx
		stdcall pole_cell_creat,ecx,edx,0
	.fun_e:
	popad ;edi edx ecx ebx eax
	ret
endp

;output:
; eax - index
align 4
proc pole_cell_find, x:dword, y:dword
	mov eax,[memCell]
	cmp dword[eax],0
	jne @f
		xor eax,eax ;if(!fristC) return 0;
		jmp .fun_e
	@@:

	xor eax,eax ;fnd=0;
	cmp dword[b_sort],0
	je @f
		stdcall pole_bin_find, [memCell], [x],[y], [b_sort] ;i=BinFind(memCell, x,y, b_sort);
		cmp eax,0
		je @f
			shl eax,2
			add eax,[memCell]
			mov eax,[eax] ;if(i) fnd=memCell[i];
			jmp .fun_e
	@@:

	cmp eax,0
	jne @f ;if(!fnd){ // ���� �祩�� �� ������ ��ॢ��
		push ebx ecx edx edi esi
		;ebx -> i
		;ecx -> firstC
		;edx -> &memCell[i]
		;edi -> cell[memCell[i]]
		mov ecx,[memCell]
		mov ebx,[b_sort]
		lea edx,[ebx*4]
		add edx,ecx
		mov ecx,[ecx]
		.cycle_b: ;for(i=b_sort+1;i<=fristC;i++)
			inc ebx
			cmp ebx,ecx
			jg .cycle_e
			add edx,4
			get_cell_offset edi,[edx]
			mov esi,[x]
			cmp dword[edi],esi ;+0 = .x
			jne .if_e
			mov esi,[y]
			cmp dword[edi+Cell.y],esi
			jne .if_e
				;if(cell[memCell[i]].x==x && cell[memCell[i]].y==y){
				mov eax,[edx] ;fnd=memCell[i];
				jmp .cycle_e ;break;
			.if_e:
			jmp .cycle_b
		.cycle_e:
		pop esi edi edx ecx ebx
	@@:
	.fun_e:
	ret
endp

;output:
; eax - index
align 4
proc pole_bin_find, mas:dword, fx:dword, fy:dword, k:dword
	push ebx ecx edx edi
	xor eax,eax
	mov ebx,1 ;ebx - ���ᨬ���� ���冷� ��� ��ॢ�
	@@:
	cmp dword[k],ebx
	jle @f ;while(k>por)
		shl ebx,1 ;por<<=1;
		jmp @b
	@@:
	cmp dword[k],ebx
	jge @f ;if(k<por)
		shr ebx,1 ;por>>=1;
	@@:
	mov ecx,ebx ;i=por;

	;ecx -> i
	;edi -> mas[i]
	.cycle_b: ;do{
		shr ebx,1 ;por>>=1;

		lea edi,[ecx*4]
		add edi,[mas]
		;if(compare_cells_mb(mas[i],fx,fy)){
		stdcall pole_compare_cells_mb_coords, dword[edi],[fx],[fy]
		cmp dl,0
		je .if_u0_e
			@@: ;while(i+por>k)
			mov edx,ecx
			add edx,ebx
			cmp edx,[k] ;i+por>k
			jle @f
				shr ebx,1 ;por>>=1;
				jmp @b
			@@:
			add ecx,ebx ;i+=por;
			jmp .if_e
		.if_u0_e:
		;else if(compare_cells_bm(mas[i],fx,fy))i-=por;
		stdcall pole_compare_cells_bm_coords, dword[edi],[fx],[fy]
		cmp dl,0
		je .if_u1_e
			sub ecx,ebx
			jmp .if_e
		.if_u1_e:
		;else { m=i; por=0; }
			mov eax,ecx
			xor ebx,ebx
		.if_e:
	cmp ebx,0
	jne .cycle_b ;}while(por);

	pop edi edx ecx ebx
	ret
endp

;description:
; �ᯮ����⥫쭠� �㭪�� ��� ���஢�� �祥�� �� ���न��⠬
;output:
; dl
align 4
proc pole_compare_cells_bm_coords, i0:dword, fx:dword, fy:dword
	push eax ebx ecx
	get_cell_offset eax,[i0]
	;eax -> cell[i0]
	mov ebx,[fx]
	cmp dword[eax],ebx
	jle @f
		mov dl,1
		jmp .fun_e
	@@:
	mov ecx,[fy]
	cmp dword[eax+Cell.y],ecx
	jle @f
	cmp dword[eax],ebx
	jne @f
		mov dl,1
		jmp .fun_e
	@@:
	xor dl,dl
	.fun_e:
	pop ecx ebx eax
	ret
endp

;description:
; �ᯮ����⥫쭠� �㭪�� ��� ���஢�� �祥�� �� ���न��⠬
;output:
; dl
align 4
proc pole_compare_cells_mb_coords, i0:dword, fx:dword, fy:dword
	push eax ebx ecx
	get_cell_offset eax,[i0]
	;eax -> cell[i0]
	mov ebx,[fx]
	cmp dword[eax],ebx
	jge @f
		mov dl,1
		jmp .fun_e
	@@:
	mov ecx,[fy]
	cmp dword[eax+Cell.y],ecx
	jge @f
	cmp dword[eax],ebx
	jne @f
		mov dl,1
		jmp .fun_e
	@@:
	xor dl,dl
	.fun_e:
	pop ecx ebx eax
	ret
endp

;output:
; dl
align 4
proc pole_compare_cells_bm, i0:dword, i1:dword
	push eax ebx ecx
	get_cell_offset eax,[i0] ;eax -> cell[i0]
	get_cell_offset ebx,[i1] ;ebx -> cell[i1]
	mov ecx,[ebx] ;+0 = .x
	cmp dword[eax],ecx
	jle @f ;x0>x1
		mov dl,1
		jmp .fun_e
	@@:
	jne @f ;x0==x1
	mov ecx,[ebx+Cell.y]
	cmp dword[eax+Cell.y],ecx
	jle @f ;y0>y1
		mov dl,1
		jmp .fun_e
	@@:
	xor dl,dl
	.fun_e:
	pop ecx ebx eax
	ret
endp

align 4
pole_paint:
	pushad
	;eax -> firstC
	;ebx -> i
	;ecx -> cell[memCell[i]]
	;edx -> color
	;edi -> coord_x
	;esi -> coord_y
	mov eax,[memCell]
	cmp dword[eax],0
	je .no_draw

	mov eax,[eax]
	mov ebx,1

;---
	@@: ;while(i<b_sort && Cor_x+cell[memCell[i]].x<0)
		cmp ebx,[b_sort]
		jge @f ;���室�� �� ��砫� ������� 横��
		lea ecx,[ebx*4]
		add ecx,[memCell]
		get_cell_offset ecx,[ecx]
		mov edx,[ecx] ;+0 = .x
		add edx,[Cor_x]
		cmp edx,0
		jge @f ;���室�� �� ��砫� ������� 横��
			inc ebx ;i++; // ��� �ய�᪠ �祥� �� ����� ᫥��
		jmp @b
	@@:

	cmp byte[zoom],2
	jge .zoom2
	@@: ;for(;i<=fristC;i++){
		lea ecx,[ebx*4]
		add ecx,[memCell]
		get_cell_offset ecx,[ecx]
;...
		mov edi,[Cor_x]
		add edi,[ecx] ;+0 = .x
		mov esi,[Cor_y]
		add esi,[ecx+Cell.y]
		bt word[ecx+Cell.liv],0
		jc .cell_1
			;�� ����� �祩��
			mov edx,[CellColors]
			mov edx,[edx]
			jmp .cell_0
		.cell_1:
			;����� �祩��
			mov edx,[tim]
			inc edx
			sub edx,[ecx+Cell.tc]
			cmp edx,COL_MEM
			jle .in_color
				mov edx,COL_MEM
			.in_color:
			shl edx,2
			add edx,[CellColors]
			mov edx,[edx]
		.cell_0:
		stdcall [buf2d_set_pixel], buf_0, edi, esi, edx
;...
		inc ebx
		cmp ebx,eax
		jle @b

	jmp .no_draw
	.zoom2:

	@@: ;for(;i<=fristC;i++){
		lea ecx,[ebx*4]
		add ecx,[memCell]
		get_cell_offset ecx,[ecx]

		xor edx,edx
		mov dl,byte[zoom] ;edx �ᯮ������ ��� ���ᥭ�� zoom � 4 ���⭮� �᫮
		mov edi,[ecx] ;+0 = .x
		add edi,[Cor_x]
		imul edi,edx
		mov esi,[ecx+Cell.y]
		add esi,[Cor_y]
		imul esi,edx
		bt word[ecx+Cell.liv],0
		jc .z2_cell_1
			;�� ����� �祩��
			mov edx,[CellColors]
			mov edx,[edx]
			jmp .z2_cell_0
		.z2_cell_1:
			;����� �祩��
			mov edx,[tim]
			inc edx
			sub edx,[ecx+Cell.tc]
			cmp edx,COL_MEM
			jle .z2_in_color
				mov edx,COL_MEM
			.z2_in_color:
			shl edx,2
			add edx,[CellColors]
			mov edx,[edx]
		.z2_cell_0:
		xor ecx,ecx
		mov cl,byte[zoom] ;ecx �ᯮ������ ��� ���ᥭ�� zoom � 4 ���⭮� �᫮
		;;;dec ecx
		stdcall [buf2d_filled_rect_by_size], buf_0, edi, esi, ecx, ecx, edx
		inc ebx
		cmp ebx,eax
		jle @b

	.no_draw:
	popad
	ret

align 4
pole_next_gen:
	pushad
	;eax -> firstC
	;ebx -> i
	;ecx -> &memCell[i]
	;edx -> cell[memCell[i]]

	mov eax,[memCell]
	mov ecx,eax
	mov eax,[eax]
	cmp eax,1
	jl .fun_e
	inc dword[tim]
	mov ebx,1
	@@: ;for(i=1;i<=firstC;i++)
		add ecx,4
		get_cell_offset edx,[ecx]
		bt word[edx+Cell.liv],0
		jae .if_0_e
			; ��࠭���� �祩�� (�ᥤ�� 2 ��� 3)
			cmp byte[edx+Cell.so],2
			je .if_2_e
			cmp byte[edx+Cell.so],3
			je .if_2_e
			jmp .change
		.if_0_e:
			; ᮧ����� �祩�� (�ᥤ�� 3)
			cmp byte[edx+Cell.so],3
			jne .if_1_e
			.change:
				or byte[edx+Cell.liv],2
				jmp .if_2_e
		.if_1_e:
			; 㤠����� ���⮩ �祩�� ��� �᢮�������� �����
			cmp byte[edx+Cell.so],0
			jne .if_2_e
			mov edi,[edx+Cell.tc]
			add edi,5 ; 5 - �६� ��࠭���� ���⮩ �祩��, �� �� 㤠�����
			cmp edi,[tim]
			jge .if_2_e
				lea edi,[eax*4]
				add edi,[memCell] ;edi -> &memCell[fristC]
				mov esi,[edi] ;swp=memCell[fristC];
				mov edx,[ecx] ;edx - 㦥 �� �ᯮ��㥬, ��⮬� ����� ������
				mov [edi],edx ;memCell[fristC]=memCell[i];
				mov [ecx],esi ;memCell[i]=swp;
				dec eax
				dec ebx
				sub ecx,4
		.if_2_e:

		inc ebx
		cmp ebx,eax
		jle @b
	mov ebx,[memCell]
	mov [ebx],eax ;firstC <- eax

	mov [b_sort],eax
	stdcall pole_fl_sort, [memCell],eax

	mov ecx,[memCell]
	mov ebx,1
	@@: ;for(i=1;i<=firstC;i++)
		add ecx,4
		get_cell_offset edx,[ecx]
		bt word[edx+Cell.liv],1
		jae .no_change
			xor byte[edx+Cell.liv],3
			mov edi,[tim]
			mov [edx+Cell.tc],edi
			bt word[edx+Cell.liv],0
			jc .new_cell
				push eax
				mov edi,[edx]
				dec edi
				mov esi,[edx+Cell.y]
				dec esi
				dec dword[osob]
				;����� ���祭�� edx �������
				stdcall pole_cell_find,edi,esi
				get_cell_offset edx,eax
				dec byte[edx+Cell.so]
				inc esi
				stdcall pole_cell_find,edi,esi
				get_cell_offset edx,eax
				dec byte[edx+Cell.so]
				inc esi
				stdcall pole_cell_find,edi,esi
				get_cell_offset edx,eax
				dec byte[edx+Cell.so]
				inc edi
				stdcall pole_cell_find,edi,esi
				get_cell_offset edx,eax
				dec byte[edx+Cell.so]
				sub esi,2
				stdcall pole_cell_find,edi,esi
				get_cell_offset edx,eax
				dec byte[edx+Cell.so]
				inc edi
				stdcall pole_cell_find,edi,esi
				get_cell_offset edx,eax
				dec byte[edx+Cell.so]
				inc esi
				stdcall pole_cell_find,edi,esi
				get_cell_offset edx,eax
				dec byte[edx+Cell.so]
				inc esi
				stdcall pole_cell_find,edi,esi
				get_cell_offset edx,eax
				dec byte[edx+Cell.so]
				pop eax
				jmp .no_change
			.new_cell: ; ������ ����� �祩��
				inc dword[osob]
				mov edi,[edx]
				dec edi
				mov esi,[edx+Cell.y]
				dec esi
				stdcall pole_cell_creat,edi,esi,0
				inc esi
				stdcall pole_cell_creat,edi,esi,0
				inc esi
				stdcall pole_cell_creat,edi,esi,0
				inc edi
				stdcall pole_cell_creat,edi,esi,0
				sub esi,2
				stdcall pole_cell_creat,edi,esi,0
				inc edi
				stdcall pole_cell_creat,edi,esi,0
				inc esi
				stdcall pole_cell_creat,edi,esi,0
				inc esi
				stdcall pole_cell_creat,edi,esi,0
		.no_change:
		inc ebx
		cmp ebx,eax
		jle @b
	.fun_e:
	popad
	ret

;����஢�� ����� a[1..n] ��⮤�� ������
align 4
proc pole_fl_sort, a:dword, n:dword
	pushad
	mov ecx,[a]
	;��ନ஢��� ��室��� ���筮 㯮�冷祭��� ��ॢ�
	mov eax,[n]
	shr eax,1
	@@: ;for(i=n>>1; i>=2; i--)
		stdcall pole_fl_surface, ecx,eax,[n] ;(a,i,n)
		dec eax
		cmp eax,2
		jge @b
	;�믮����� ��楤��� �ᯫ��� ������ ��� ������� �����ॢ�
	mov eax,[n]
	@@: ;for(i=n; i>=2; i--){
		stdcall pole_fl_surface, ecx,1,eax ;(a,1,i)
		;�������� �������� ���ᨬ���� ����� � ����� ᯨ᪠
		lea edi,[eax*4]
		add edi,ecx ;edi -> &a[i]
		mov esi,[edi] ;w=a[i];
		mov edx,[ecx+4]
		mov [edi],edx ;a[i]=a[1];
		mov [ecx+4],esi ;a[1]=w;

		dec eax
		cmp eax,2
		jge @b
	popad
	ret
endp

;��楤�� �ᯫ��� ������ �� ��ॢ� a[1..k]
align 4
proc pole_fl_surface, a:dword, i:dword, k:dword
locals
	copy dd ?
endl
	pushad
	;edx -> ...
	;edi -> m
	;esi -> j
	mov eax,[a]
	mov ebx,[i]
	mov ecx,[k]

	lea edx,[ebx*4]
	add edx,eax
	mov edx,[edx]
	mov [copy],edx ;copy=a[i];
	mov edi,ebx
	shl edi,1 ;m=i<<1;
	.cycle_b: ;while (m<=k) {
		cmp edi,ecx
		jg .cycle_e
		jne @f ;if (m==k) j=m;
			mov esi,edi
			jmp .else_e
		@@: ;else if (pole_compare_cells_bm(a[m],a[m+1])) j=m;
		lea edx,[edi*4]
		add edx,eax
		stdcall pole_compare_cells_bm, dword[edx],dword[edx+4]
		cmp dl,0
		je @f
			mov esi,edi
			jmp .else_e
		@@: ;else j=m+1;
			mov esi,edi
			inc esi
		.else_e:

		;if (pole_compare_cells_bm(a[j],copy)) {
		lea edx,[esi*4]
		add edx,eax
		stdcall pole_compare_cells_bm, dword[edx],[copy]
		cmp dl,0
		je .cycle_e ;} else break; //��室 �� 横��

		lea edx,[esi*4]
		add edx,eax
		push dword[edx] ;push a[j];
		lea edx,[ebx*4]
		add edx,eax
		pop dword[edx] ;a[i]=a[j];
		mov ebx,esi ;i=j;
		mov edi,ebx
		shl edi,1 ;m=i<<1;

		jmp .cycle_b
	.cycle_e:

	;���祭�� ������ ॣ���஢ 㦥 �� ����� �. �. ����� �㭪樨
	shl ebx,2
	add eax,ebx
	mov edx,[copy]
	mov [eax],edx ;a[i]=copy;

	popad
	ret
endp
;--------------------------------------


align 4
start:
	load_libraries l_libs_start,l_libs_end
	;�஢�ઠ �� ᪮�쪮 㤠筮 ���㧨���� ��� ����
	mov	ebp,lib2
	cmp	dword [ebp+ll_struc_size-4],0
	jz	@f
		mcall SF_TERMINATE_PROCESS ;exit not correct
	@@:
	mcall SF_STYLE_SETTINGS,SSF_GET_COLORS,sc,sizeof.system_colors
	mcall SF_SET_EVENTS_MASK,0x27
	stdcall [OpenDialog_Init],OpenDialog_data ;�����⮢�� �������

	stdcall [buf2d_create], buf_0 ;ᮧ����� ����

	stdcall mem.Alloc,MAX_CELL*sizeof.Cell
	mov [cell],eax
	stdcall mem.Alloc,MAX_CELL*4
	mov [memCell],eax
	stdcall mem.Alloc,(COL_MEM+1)*4
	mov [CellColors],eax
	include_image_file 'life3tb.png', image_data_toolbar

	;����ன�� 梥⮢ �祥�
	stdcall pole_init_colors, 0xffffd0,0xff0000,0x0000ff
	call pole_clear
	call pole_paint ;�ᮢ���� ���� � ���� (�� �� �࠭�)

	mcall SF_SYSTEM_GET,SSF_TIME_COUNT
	mov [last_time],eax

align 4
red_win:
	call draw_window

align 4
still:
	mcall SF_SYSTEM_GET,SSF_TIME_COUNT
	mov ebx,[last_time]
	add ebx,10 ;����প�
	cmp ebx,eax
	jge @f
		mov ebx,eax
	@@:
	sub ebx,eax
	mcall SF_WAIT_EVENT_TIMEOUT
	cmp eax,0
	je timer_funct

	cmp al,1
	jz red_win
	cmp al,2
	jz key
	cmp al,3
	jz button
	cmp al,6
	jne @f
		mcall SF_THREAD_INFO,procinfo,-1
		cmp ax,word[procinfo.window_stack_position]
		jne @f ;���� �� ��⨢��
		call mouse
	@@:
	jmp still

align 4
mouse:
	push eax ebx ecx
	mcall SF_MOUSE_GET,SSF_BUTTON_EXT
	bt eax,8
	jnc @f
		;mouse l. but. press
		;call mouse_left_d
		;jmp .end_l
	@@:
	bt eax,16
	jnc .end_l
		;mouse l. but. up
		;call mouse_left_u
		;jmp .end_l
	.end_l:

	call buf_get_mouse_coord
	cmp eax,-1
	je .end0
		shl eax,1
		sub eax,[buf_0.w]
		sar eax,1
		;mov [mouse_prop_x],eax
		shl ebx,1
		sub ebx,[buf_0.h]
		sar ebx,1
		;mov [mouse_prop_y],ebx

		mcall SF_MOUSE_GET,SSF_SCROLL_DATA
		test ax,ax
		jz .end0

		test ax,0x8000
		jnz .decr
			;㢥��祭�� ����⠡�
			call but_zoom_p
			jmp .end0
		.decr:
			;㬥��襭�� ����⠡�
			call but_zoom_m
	.end0:

	pop ecx ebx eax
	ret

;output:
; eax - buffer coord X (�᫨ ����� �� ���஬ -1)
; ebx - buffer coord Y (�᫨ ����� �� ���஬ -1)
align 4
proc buf_get_mouse_coord
	mcall SF_MOUSE_GET,SSF_WINDOW_POSITION
	cmp ax,word[buf_0.t]
	jl .no_buf ;�� ������ � ���� ���� �� �� y
	mov ebx,eax
	shr ebx,16
	cmp bx,word[buf_0.l]
	jl .no_buf ;�� ������ � ���� ���� �� �� x

	and eax,0xffff ;��⠢�塞 ���न���� y
	sub ax,word[buf_0.t]
	cmp eax,[buf_0.h]
	jg .no_buf
	sub bx,word[buf_0.l]
	cmp ebx,[buf_0.w]
	jg .no_buf
	xchg eax,ebx
	jmp .end_f
	.no_buf:
		xor eax,eax
		not eax
		xor ebx,ebx
		not ebx
	.end_f:
	ret
endp

align 4
timer_funct:
	pushad
	mcall SF_SYSTEM_GET,SSF_TIME_COUNT
	mov [last_time],eax

	cmp byte[tim_ch],0
	je @f
		;call but_next_gen
		cld
		mov ecx,[poc_stop]
		cmp ecx,1
		jg .clear
			mov ecx,1 ;��ࠢ����� ecx �� ��砩 �ᥫ ������ 1
			jmp .cycle
		.clear: ;��⨬ ���� �᫨ ���� ����� �� ��᪮�쪮 ��������� �� 1 ⠪� ⠩���
			stdcall [buf2d_clear], buf_0, [buf_0.color]
		.cycle:
			call pole_next_gen
			loop .cycle
		call pole_paint
		stdcall [buf2d_draw], buf_0
		call draw_pok
	@@:
	popad
	jmp still

align 4
draw_window:
pushad
	mcall SF_REDRAW,SSF_BEGIN_DRAW
	mcall SF_CREATE_WINDOW,(50 shl 16)+485,(50 shl 16)+415,0x73000000,0,hed

	mcall SF_THREAD_INFO,procinfo,-1
	mov eax,[procinfo.box.height]
	cmp eax,120
	jge @f
		mov eax,120 ;min size
	@@:
	sub eax,63
	mov ebx,[procinfo.box.width]
	cmp ebx,270
	jge @f
		mov ebx,270
	@@:
	sub ebx,9
	cmp eax,[buf_0.h] ;ᬮ�ਬ ࠧ��� ����
	jne @f
	cmp ebx,[buf_0.w]
	jne @f
		jmp .end0
	@@:
		stdcall [buf2d_resize], buf_0, ebx,eax,1
		stdcall [buf2d_clear], buf_0, [buf_0.color]
		call pole_paint
	.end0:

	mov edx,[sc.work]
	mov ebx, 0 shl 16
	add ebx, [buf_0.w]
	mcall SF_DRAW_RECT,,<0,35>

	mcall SF_DEFINE_BUTTON,(6 shl 16)+TBTNSIZE,(6 shl 16)+TBTNSIZE,4+0x40000000, [sc.work_button]

	mov ebx,(36 shl 16)+TBTNSIZE
	mov edx,6+0x40000000
	int 0x40

	mov ebx,(61 shl 16)+TBTNSIZE
	mov edx,7+0x40000000
	int 0x40

	mov ebx,(86 shl 16)+TBTNSIZE
	mov edx,8+0x40000000
	int 0x40

	mov ebx,(116 shl 16)+TBTNSIZE
	mov edx,9+0x40000000
	int 0x40

	mov ebx,(141 shl 16)+TBTNSIZE
	mov edx,10+0x40000000
	int 0x40

	mov ebx,(171 shl 16)+TBTNSIZE
	mov edx,11+0x40000000
	int 0x40

	mov ebx,(196 shl 16)+TBTNSIZE
	mov edx,12+0x40000000
	int 0x40

	mov ebx,(221 shl 16)+TBTNSIZE
	mov edx,13+0x40000000
	int 0x40

	mov ebx,(246 shl 16)+TBTNSIZE
	mov edx,14+0x40000000
	int 0x40

	mcall SF_PUT_IMAGE,[image_data_toolbar],(ICONSIZE shl 16)+ICONSIZE,(5 shl 16)+5

	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	mov edx,(35 shl 16)+5 ;run once
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	mov edx,(60 shl 16)+5 ;run auto
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	mov edx,(85 shl 16)+5 ;stop
	int 0x40

	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	mov edx,(115 shl 16)+5 ;-
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	mov edx,(140 shl 16)+5 ;+
	int 0x40

	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	mov edx,(170 shl 16)+5 ;move up
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	mov edx,(195 shl 16)+5 ;move doun
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	mov edx,(220 shl 16)+5 ;move left
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	mov edx,(245 shl 16)+5 ;move right
	int 0x40

	; add ebx,IMAGE_TOOLBAR_ICON_SIZE
	; mov edx,(270 shl 16)+5
	; int 0x40
	; add ebx,IMAGE_TOOLBAR_ICON_SIZE
	; mov edx,(295 shl 16)+5
	; int 0x40

	mov eax,SF_DRAW_TEXT
	mov ebx,295*65536+5
	mov ecx,[sc.work_text]
	or  ecx,0x80000000 ;or (1 shl 30)
	mov edx,txt_zoom
	;mov edi,[sc.work]
	int 0x40
	add bx,9
	mov edx,txt_gen
	int 0x40
	add bx,9
	mov edx,txt_osob
	int 0x40

	call draw_pok

	stdcall [buf2d_draw], buf_0

	mcall SF_REDRAW,SSF_END_DRAW
popad
	ret

align 4
draw_pok:
	mov eax,SF_DRAW_NUMBER
	movzx ecx,byte[zoom]
	mov ebx,(2 shl 16)
	mov edx,(295+8*9)*65536+5
	mov esi,[sc.work_text]
	or  esi,(1 shl 30)
	mov edi,[sc.work]
	int 0x40 ;����⠡
	mov ebx,(5 shl 16)
	mov ecx,[tim]
	add edx,9
	int 0x40 ;�६�
	mov ebx,(5 shl 16)
	mov ecx,[osob]
	add edx,9
	int 0x40 ;�������
	ret

align 4
key:
	mcall SF_GET_KEY
	jmp still


align 4
button:
	mcall SF_GET_BUTTON
	cmp ah,3
	jne @f
		call but_new_file
		jmp still
	@@:
	cmp ah,4
	jne @f
		call but_open_file
		jmp still
	@@:
	cmp ah,5
	jne @f
		call but_save_file
		jmp still
	@@:
	cmp ah,6
	jne @f
		call but_next_gen
		jmp still
	@@:
	cmp ah,7
	jne @f
		call but_run
		jmp still
	@@:
	cmp ah,8
	jne @f
		call but_stop
		jmp still
	@@:
	cmp ah,9
	jne @f
		call but_zoom_p
		jmp still
	@@:
	cmp ah,10
	jne @f
		call but_zoom_m
		jmp still
	@@:
	cmp ah,11
	jne @f
		call but_pole_up
		jmp still
	@@:
	cmp ah,12
	jne @f
		call but_pole_dn
		jmp still
	@@:
	cmp ah,13
	jne @f
		call but_pole_left
		jmp still
	@@:
	cmp ah,14
	jne @f
		call but_pole_right
		jmp still
	@@:
	cmp ah,1
	jne still
.exit:
	stdcall [buf2d_delete],buf_0
	stdcall mem.Free,[open_file_lif]
	stdcall mem.Free,[cell]
	stdcall mem.Free,[memCell]
	stdcall mem.Free,[CellColors]
	stdcall mem.Free,[image_data_toolbar]
	mcall SF_TERMINATE_PROCESS


align 4
but_new_file:
	ret

align 4
open_file_lif dd 0 ;㪠��⥫� �� ������ ��� ������ 䠩���
open_file_size dd 0 ;ࠧ��� ����⮣� 䠩��

align 4
but_open_file:
	pushad
	copy_path open_dialog_name,communication_area_default_path,file_name,0
	mov [OpenDialog_data.type],0
	stdcall [OpenDialog_Start],OpenDialog_data
	cmp [OpenDialog_data.status],2
	je .end_open_file
	;��� �� 㤠筮� ����⨨ �������

	mov [run_file_70.Function], SSF_GET_INFO
	mov [run_file_70.Position], 0
	mov [run_file_70.Flags], 0
	mov dword[run_file_70.Count], 0
	mov dword[run_file_70.Buffer], open_b
	mov byte[run_file_70+20], 0
	mov dword[run_file_70.FileName], openfile_path
	mcall SF_FILE,run_file_70

	mov ecx,dword[open_b+32] ;+32 qword: ࠧ��� 䠩�� � �����
	inc ecx ;for text files
	stdcall mem.ReAlloc,[open_file_lif],ecx
	mov [open_file_lif],eax
	dec ecx ;for text files
	mov byte[eax+ecx],0 ;for text files

	mov [run_file_70.Function], SSF_READ_FILE
	mov [run_file_70.Position], 0
	mov [run_file_70.Flags], 0
	mov [run_file_70.Count], ecx
	m2m dword[run_file_70.Buffer], dword[open_file_lif]
	mov byte[run_file_70+20], 0
	mov dword[run_file_70.FileName], openfile_path
	mcall SF_FILE,run_file_70 ;����㦠�� 䠩� ����ࠦ����
	test eax,eax
	jnz .end_open_file
	cmp ebx,0xffffffff
	je .end_open_file

	mov [open_file_size],ebx
	mcall SF_SET_CAPTION,1,openfile_path

	call pole_clear
	mov eax,[buf_0.w]
	shr eax,1
	xor ecx,ecx
	mov cl,byte[zoom]
	cmp cx,2
	jl @f ;������� �� ����稭� zoom
		xor edx,edx
		div ecx
	@@:
	add [Cor_x],eax
	mov eax,[buf_0.h]
	shr eax,1
	cmp cx,2
	jl @f ;������� �� ����稭� zoom
		xor edx,edx
		div ecx
	@@:
	add [Cor_y],eax

	;eax - first x position
	;ebx - x position
	;ecx - y position
	;edx - ����� 䠩��
	mov edi,[open_file_lif]
	xor ebx,ebx
	xor ecx,ecx
	mov eax,ebx
	mov edx,edi
	add edx,[open_file_size]
	@@:
		cmp byte[edi],'*'
		jne .no_cell
			stdcall pole_cell_creat, ebx,ecx,1
			inc ebx
		.no_cell:
		cmp byte[edi],'.'
		jne .cell_move
			inc ebx
		.cell_move:
		cmp byte[edi],13
		jne .cell_nl
			mov ebx,eax
			inc ecx
		.cell_nl:
		cmp word[edi],'#P' ;ᬥ�� ����樨
		jne .pos
			inc edi ;�ய�� '#'
			.space:
				inc edi ;�ய�� 'P' � ' '
				cmp byte[edi],' '
				je .space
			stdcall conv_str_to_int,edi
			mov ebx,eax
			cmp byte[edi],'-'
			jne .digit
				inc edi
			.digit:
				cmp byte[edi],'0'
				jl .digit_no
				cmp byte[edi],'9'
				jg .digit_no
				inc edi
				jmp .digit
			.digit_no:
			;.space_1:
				inc edi ;�ய�� 'P' � ' '
				cmp byte[edi],' '
				je .digit_no ;.space_1
			stdcall conv_str_to_int,edi
			mov ecx,eax
			mov eax,ebx ;����⠭������� ������ ����㯠 � eax
		.pos:
		inc edi
		cmp edi,edx
		jl @b
	;---
	stdcall [buf2d_clear], buf_0, [buf_0.color] ;��⨬ ����
	call pole_paint ;��㥬 ���� (�� ��砩 �᫨ ���� �⪠ ��� ⥪�⮢� ������)
	stdcall [buf2d_draw], buf_0 ;������塞 ���� �� �࠭�
	.end_open_file:
	popad
	ret

align 4
but_save_file:
	ret

align 4
but_next_gen:
	call pole_next_gen
	call pole_paint
	stdcall [buf2d_draw], buf_0
	pushad
		call draw_pok
	popad
	ret

align 4
but_run:
	mov byte[tim_ch],1
	ret

align 4
but_stop:
	mov byte[tim_ch],0
	;cld
	;mov ecx,100
	;@@:
	;       call pole_next_gen
	;loop @b
	;stdcall [buf2d_clear], buf_0, [buf_0.color]
	;call pole_paint
	;stdcall [buf2d_draw], buf_0
	ret

align 4
but_zoom_p:
	cmp byte[zoom],16
	jge @f
		pushad
		;���᫥��� ᤢ���� ��� ����, ����� ���ᯥ�� 業�஢�� ���� �� 㢥��祭�� ����⠡�
		xor ecx,ecx
		mov cl,byte[zoom]
		xor edx,edx
		mov eax,[buf_0.w]
		shr eax,1 ;� eax �������� �ਭ� ����
		mov ebx,eax ;������ १�ࢭ�� ����� eax
		div ecx ;����� eax �� ⥪�騩 ����⠡
		xchg eax,ebx
		xor edx,edx
		inc ecx
		div ecx ;����� eax �� ���� ����⠡
		sub ebx,eax ;�������� ᤢ�� ���� ����� ���ᯥ�� 業�஢�� ����
		sub dword[Cor_x],ebx ;ᤢ����� ���� �७�� �� �� x
		xor ecx,ecx
		mov cl,byte[zoom]
		xor edx,edx
		mov eax,[buf_0.h]
		shr eax,1
		mov ebx,eax
		div ecx
		xchg eax,ebx
		xor edx,edx
		inc ecx
		div ecx
		sub ebx,eax
		sub dword[Cor_y],ebx ;ᤢ����� ���� �७�� �� �� y

		inc byte[zoom]
		call draw_pok
		popad

		cmp dword[poc_stop],1
		jle .buf_clear
		cmp byte[tim_ch],0
		jne @f
			.buf_clear:
			stdcall [buf2d_clear], buf_0, [buf_0.color]
			call pole_paint
			stdcall [buf2d_draw], buf_0
	@@:
	ret

align 4
but_zoom_m:
	cmp byte[zoom],1
	jle @f
		pushad
		;���᫥��� ᤢ���� ��� ����, ����� ���ᯥ�� 業�஢�� ���� �� 㬥��襭�� ����⠡�
		xor ecx,ecx
		mov cl,byte[zoom]
		xor edx,edx
		mov eax,[buf_0.w]
		shr eax,1 ;� eax �������� �ਭ� ����
		mov ebx,eax ;������ १�ࢭ�� ����� eax
		div ecx ;����� eax �� ⥪�騩 ����⠡
		xchg eax,ebx
		xor edx,edx
		dec ecx
		div ecx ;����� eax �� ���� ����⠡
		sub ebx,eax ;�������� ᤢ�� ���� ����� ���ᯥ�� 業�஢�� ����
		sub dword[Cor_x],ebx ;ᤢ����� ���� �७�� �� �� x
		xor ecx,ecx
		mov cl,byte[zoom]
		xor edx,edx
		mov eax,[buf_0.h]
		shr eax,1
		mov ebx,eax
		div ecx
		xchg eax,ebx
		xor edx,edx
		dec ecx
		div ecx
		sub ebx,eax
		sub dword[Cor_y],ebx ;ᤢ����� ���� �७�� �� �� y

		dec byte[zoom]
		call draw_pok
		popad

		cmp dword[poc_stop],1
		jle .buf_clear
		cmp byte[tim_ch],0
		jne @f
			.buf_clear:
			stdcall [buf2d_clear], buf_0, [buf_0.color]
			call pole_paint
			stdcall [buf2d_draw], buf_0
	@@:
	ret

align 4
but_pole_up:
	push eax ecx edx
	mov eax,[buf_0.h]
	shr eax,2
	xor ecx,ecx
	mov cl,byte[zoom]
	cmp cx,2
	jl @f ;������� �� ����稭� zoom
		xor edx,edx
		div ecx
	@@:
	add [Cor_y],eax
	pop edx ecx eax
	stdcall [buf2d_clear], buf_0, [buf_0.color]
	call pole_paint
	stdcall [buf2d_draw], buf_0
	ret

align 4
but_pole_dn:
	push eax ecx edx
	mov eax,[buf_0.h]
	shr eax,2
	xor ecx,ecx
	mov cl,byte[zoom]
	cmp cx,2
	jl @f ;������� �� ����稭� zoom
		xor edx,edx
		div ecx
	@@:
	sub [Cor_y],eax
	pop edx ecx eax
	stdcall [buf2d_clear], buf_0, [buf_0.color]
	call pole_paint
	stdcall [buf2d_draw], buf_0
	ret

align 4
but_pole_left:
	push eax ecx edx
	mov eax,[buf_0.w]
	shr eax,2
	movzx ecx,byte[zoom]
	cmp ecx,2
	jl @f ;������� �� ����稭� zoom
		xor edx,edx
		div ecx
	@@:
	add [Cor_x],eax
	pop edx ecx eax
	stdcall [buf2d_clear], buf_0, [buf_0.color]
	call pole_paint
	stdcall [buf2d_draw], buf_0
	ret

align 4
but_pole_right:
	push eax ecx edx
	mov eax,[buf_0.w]
	shr eax,2
	xor ecx,ecx
	mov cl,byte[zoom]
	cmp cx,2
	jl @f ;������� �� ����稭� zoom
		xor edx,edx
		div ecx
	@@:
	sub [Cor_x],eax
	pop edx ecx eax
	stdcall [buf2d_clear], buf_0, [buf_0.color]
	call pole_paint
	stdcall [buf2d_draw], buf_0
	ret

;input:
; buf - 㪠��⥫� �� ��ப�, �᫮ ������ ���� � 10 ��� 16 �筮� ����
;output:
; eax - �᫮
align 4
proc conv_str_to_int uses ebx ecx esi, buf:dword
	xor eax,eax
	xor ebx,ebx
	mov esi,[buf]
	;��।������ ����⥫��� �ᥫ
	xor ecx,ecx
	inc ecx
	cmp byte[esi],'-'
	jne @f
		dec ecx
		inc esi
	@@:

	cmp word[esi],'0x'
	je .load_digit_16

	.load_digit_10: ;���뢠��� 10-���� ���
		mov bl,byte[esi]
		cmp bl,'0'
		jl @f
		cmp bl,'9'
		jg @f
			sub bl,'0'
			imul eax,10
			add eax,ebx
			inc esi
			jmp .load_digit_10
	jmp @f

	.load_digit_16: ;���뢠��� 16-���� ���
		add esi,2
	.cycle_16:
		mov bl,byte[esi]
		cmp bl,'0'
		jl @f
		cmp bl,'f'
		jg @f
		cmp bl,'9'
		jle .us1
			cmp bl,'A'
			jl @f ;��ᥨ���� ᨬ���� >'9' � <'A'
		.us1: ;��⠢��� �᫮���
		cmp bl,'F'
		jle .us2
			cmp bl,'a'
			jl @f ;��ᥨ���� ᨬ���� >'F' � <'a'
			sub bl,32 ;��ॢ���� ᨬ���� � ���孨� ॣ����, ��� ��饭�� �� ��᫥��饩 ��ࠡ�⪨
		.us2: ;��⠢��� �᫮���
			sub bl,'0'
			cmp bl,9
			jle .cor1
				sub bl,7 ;convert 'A' to '10'
			.cor1:
			shl eax,4
			add eax,ebx
			inc esi
			jmp .cycle_16
	@@:
	cmp ecx,0 ;�᫨ �᫮ ����⥫쭮�
	jne @f
		sub ecx,eax
		mov eax,ecx
	@@:
	ret
endp

;����� ��� ������� ������ 䠩���
align 4
OpenDialog_data:
.type			dd 0 ;0 - ������, 1 - ��࠭���, 2 - ����� ��४���
.procinfo		dd procinfo	;+4
.com_area_name		dd communication_area_name	;+8
.com_area		dd 0	;+12
.opendir_path		dd plugin_path	;+16
.dir_default_path	dd default_dir ;+20
.start_path		dd file_name ;+24 ���� � ������� ������ 䠩���
.draw_window		dd draw_window	;+28
.status 		dd 0	;+32
.openfile_path		dd openfile_path	;+36 ���� � ���뢠����� 䠩��
.filename_area		dd filename_area	;+40
.filter_area		dd Filter
.x:
.x_size 		dw 420 ;+48 ; Window X size
.x_start		dw 10 ;+50 ; Window X position
.y:
.y_size 		dw 320 ;+52 ; Window y size
.y_start		dw 10 ;+54 ; Window Y position

default_dir db '/sys',0

communication_area_name:
	db 'FFFFFFFF_open_dialog',0
open_dialog_name:
	db 'opendial',0
communication_area_default_path:
	db '/sys/File managers/',0

Filter:
dd Filter.end - Filter ;.1
.1:
db 'LIF',0
db 'RLE',0
.end:
db 0


system_dir_0 db '/sys/lib/'
lib_name_0 db 'proc_lib.obj',0
system_dir_1 db '/sys/lib/'
lib_name_1 db 'libimg.obj',0
system_dir_2 db '/sys/lib/'
lib_name_2 db 'buf2d.obj',0

l_libs_start:
	lib0 l_libs lib_name_0, file_name, system_dir_0, import_proclib
	lib1 l_libs lib_name_1, file_name, system_dir_1, import_libimg
	lib2 l_libs lib_name_2, file_name, system_dir_2, import_buf2d
l_libs_end:

align 4
import_libimg:
	dd alib_init1
	img_to_rgb2 dd aimg_to_rgb2
	img_decode  dd aimg_decode
	img_destroy dd aimg_destroy
	dd 0,0
	alib_init1   db 'lib_init',0
	aimg_to_rgb2 db 'img_to_rgb2',0
	aimg_decode  db 'img_decode',0 ;��⮬���᪨ ��।���� �ଠ� ����᪨� ������
	aimg_destroy db 'img_destroy',0

align 4
import_proclib:
	OpenDialog_Init dd aOpenDialog_Init
	OpenDialog_Start dd aOpenDialog_Start
dd 0,0
	aOpenDialog_Init db 'OpenDialog_init',0
	aOpenDialog_Start db 'OpenDialog_start',0

align 4
import_buf2d:
	init dd sz_init
	buf2d_create dd sz_buf2d_create
	buf2d_clear dd sz_buf2d_clear
	buf2d_draw dd sz_buf2d_draw
	buf2d_delete dd sz_buf2d_delete
	buf2d_resize dd sz_buf2d_resize
	buf2d_filled_rect_by_size dd sz_buf2d_filled_rect_by_size
	buf2d_set_pixel dd sz_buf2d_set_pixel
	dd 0,0
	sz_init db 'lib_init',0
	sz_buf2d_create db 'buf2d_create',0
	sz_buf2d_clear db 'buf2d_clear',0
	sz_buf2d_draw db 'buf2d_draw',0
	sz_buf2d_delete db 'buf2d_delete',0
	sz_buf2d_resize db 'buf2d_resize',0
	sz_buf2d_filled_rect_by_size db 'buf2d_filled_rect_by_size',0
	sz_buf2d_set_pixel db 'buf2d_set_pixel',0

align 4
buf_0: dd 0
.l: dw 0 ;+4 left
.t: dw 35 ;+6 top
.w: dd 476 ;+8 w
.h: dd 352 ;+12 h
.color: dd 0xffffd0 ;+16 color
	db 24 ;+20 bit in pixel

align 16
i_end:
	mouse_dd rd 1
	last_time rd 1
	sc system_colors
	procinfo process_information
	sys_path rb 1024
	file_name rb 2048 ;4096
	plugin_path rb 4096
	openfile_path rb 4096
	filename_area rb 256
	rb 1024
stacktop:
mem:

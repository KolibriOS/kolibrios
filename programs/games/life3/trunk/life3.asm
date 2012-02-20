use32
	org 0x0
	db 'MENUET01' ;идентиф. исполняемого файла всегда 8 байт
	dd 0x1
	dd start
	dd i_end ;размер приложения
	dd mem
	dd stacktop
	dd 0
	dd sys_path

include '../../../macros.inc'
include '../../../proc32.inc'
include '../../../develop/libraries/box_lib/load_lib.mac'
include '../../nu_pogodi/trunk/mem.inc'
include '../../nu_pogodi/trunk/dll.inc'

@use_library_mem mem.Alloc,mem.Free,mem.ReAlloc,dll.Load
hed db 'Life 20.02.12',0 ;подпись окна

struct FileInfoBlock
	Function dd ?
	Position dd ?
	Flags	 dd ?
	Count	 dd ?
	Buffer	 dd ?
		db ?
	FileName dd ?
ends

run_file_70 FileInfoBlock
image_data dd 0 ;указатель на временную память. для нужен преобразования изображения

fn_toolbar db 'toolbar.png',0
IMAGE_TOOLBAR_ICON_SIZE equ 16*16*3
IMAGE_TOOLBAR_SIZE equ IMAGE_TOOLBAR_ICON_SIZE*9
image_data_toolbar dd 0

macro load_image_file path,buf,size { ;макрос для загрузки изображений
	;path - может быть переменной или строковым параметром
	if path eqtype '' ;проверяем задан ли строкой параметр path
		jmp @f
			local .path_str
			.path_str db path ;формируем локальную переменную
			db 0
		@@:
		;32 - стандартный адрес по которому должен быть буфер с системным путем
		copy_path .path_str,[32],file_name,0x0
	else
		copy_path path,[32],file_name,0x0 ;формируем полный путь к файлу изображения, подразумеваем что он в одной папке с программой
	end if

	stdcall mem.Alloc, dword size ;выделяем память для изображения
	mov [buf],eax

	mov eax,70 ;70-я функция работа с файлами
	mov [run_file_70.Function], 0
	mov [run_file_70.Position], 0
	mov [run_file_70.Flags], 0
	mov [run_file_70.Count], dword size
	m2m [run_file_70.Buffer], [buf]
	mov byte[run_file_70+20], 0
	mov [run_file_70.FileName], file_name
	mov ebx,run_file_70
	int 0x40 ;загружаем файл изображения
	cmp ebx,0xffffffff
	je @f
		;определяем вид изображения и переводим его во временный буфер image_data
		stdcall dword[img_decode], dword[buf],ebx,0
		mov dword[image_data],eax
		;преобразуем изображение к формату rgb
		stdcall dword[img_to_rgb2], dword[image_data],dword[buf]
		;удаляем временный буфер image_data
		stdcall dword[img_destroy], dword[image_data]
	@@:
}

;--------------------------------------
struct Cell
	x dd ? ;+0
	y dd ? ;+4
	tc dd ? ;+8
	liv db ? ;+12
	so db ? ;+13
ends

MAX_CELL equ 90000
COL_MEM equ 64 ;число цветов

cell dd 0 ;указатель на память со структурами ячеек
memCell dd 0
CellColors dd 0

macro get_cell_offset reg,ind
{
	mov reg,ind
	imul reg,sizeof.Cell
	add reg,dword[cell]
}

capt rb 50
er_oom db 0 ;на случай исчерпания памяти
tim_ch db 0 ;автоматически просчитывать поколения
poc_stop dd 1 ;просчет на число поколений
Cor_x dd 0
Cor_y dd 0
tim dd 0 ;время (поколение)
b_sort dd 0 ;граница для сортированных ячеек
osob dd 0 ;число особей
zoom db 3 ;масштаб поля
txt_zoom db 'Масштаб:',0
txt_gen db 'Поколение:',0
txt_osob db 'Особей:',0

;настройка массива с цветами
; col_pole - цвет поля
; col_cell_n - цвет новой ячейки
; col_cell_o - цвет старой ячейки
align 4
proc pole_init_colors uses eax ebx ecx edx esi edi, col_pole:dword, col_cell_n:dword, col_cell_o:dword
	mov esi,dword[CellColors]
	mov ebx,dword[col_pole]
	mov dword[esi],ebx

	add esi,4
	mov edi,COL_MEM
	dec edi
	shl edi,2
	add edi,esi
	; esi - указатель на 1-й градиентный цвет
	; edi - указатель на последний градиентный цвет
	mov eax,dword[col_cell_n]
	mov ebx,dword[col_cell_o]

	mov dword[esi],eax
	mov dword[edi],ebx
	;need save ecx edx
	stdcall middle_colors, esi,edi

	ret
endp

;вспомогательная функция для находжения среднего цвета и записи его в массив
;input:
; eax - цвет начальный
; ebx - цвет конечный
;зазрушаються: ecx, edx
align 4
proc middle_colors uses edi esi, i0:dword, i1:dword
	mov esi,dword[i0]
	mov edi,dword[i1]
	;перед вызовом функции
	;dword[esi]=eax
	;dword[edi]=ebx
	sub edi,esi
	shr edi,1
	btr edi,1 ;округляем до 4-х, т. к. нужно получить адрес (округление хитрое - убираем один бит вместо предполагаемых 2-х)
	add edi,esi
	cmp edi,esi
	je @f
		push eax ebx

		mov ecx,eax
		mov edx,ebx

		;находим средний цвет между eax и ebx
		and ebx,111111101111111011111110b ;убираем последние биты в цветах r, g, b
		and eax,111111101111111011111110b
		add eax,ebx ;сумируем цвета из r, g, b
		shr eax,1   ;делим на 2
		mov dword[edi],eax

		;рекурсивный вызов для дробления верхней половины
		mov ebx,eax
		mov eax,ecx
		stdcall middle_colors, [i0],edi

		;рекурсивный вызов для дробления нижней половины
		mov eax,ebx
		mov ebx,edx
		stdcall middle_colors, edi,[i1]

		pop ebx eax
	@@:
	ret
endp

align 4
pole_clear:
	push eax ecx edi
	xor eax,eax
	mov dword[tim],eax
	mov dword[osob],eax
	mov  byte[tim_ch],al
	mov dword[Cor_x],eax
	mov dword[Cor_y],eax
	mov dword[b_sort],eax
	mov  byte[er_oom],al
	cld
	mov ecx,MAX_CELL
	imul ecx,sizeof.Cell
	mov edi,dword[cell]
	repne stosb ;memset(cell,0,sizeof(Cell)*MAX_CELL);
	mov edi,dword[memCell]
	mov ecx,MAX_CELL
	@@:
		stosd ;for(i=0;i<MAX_CELL;i++) memCell[i]=i;
		;mov dword[edi],eax
		;add edi,4
		inc eax
		loop @b
	pop edi ecx eax
	ret

align 4
proc pole_cell_creat, x:dword, y:dword, li:dword
	pushad ;eax ebx ecx edx edi

	; *** если клетка уже была создана
	stdcall pole_cell_find, [x],[y]
	cmp eax,0
	je @f ;if(i){
		get_cell_offset ebx,eax
		cmp dword[li],0
		jne .else_if ;if(!li)
			; если создается пустая клетка
			inc byte[ebx+13] ;+13 = .so
			jmp .fun_e
		.else_if: ;else if(!(cell[i].liv&1) ){
		bt word[ebx+12],0 ;+12 = .liv
			; если создается живая клетка
			; и раньше клетка была создана но оказалась пустой
			jae .creat_border_cells
		jmp .fun_e
	@@:

	; *** создание новой ячейки
	; находим номер свободной ячейки (i) для добавления новой
	mov edi,dword[memCell]
	inc dword[edi]
	cmp dword[edi],MAX_CELL
	jne @f
		dec dword[edi]
		mov byte[tim_ch],0
		;... need call message: "eror out of memory" ...
		;... вывод сообщения переполнения надо добавить  ...
		mov byte[er_oom],0
		jmp .fun_e ;return;
	@@:
	mov eax,dword[edi]
	shl eax,2
	add eax,dword[memCell] ;eax -> memCell[firstC]
	get_cell_offset ebx,dword[eax]

	mov ecx,dword[x]
	mov dword[ebx],ecx ;+0 = .x
	mov edx,dword[y]
	mov dword[ebx+4],edx ;+4 = .y
	mov eax,dword[tim]
	mov dword[ebx+8],eax ;+8 = .tc
	mov byte[ebx+12],0 ;+12 = .liv

	cmp dword[li],0
	jne @f
		mov byte[ebx+13],1 ;+13 = .so
		jmp .fun_e
	@@:
	mov byte[ebx+13],0 ;+13 = .so

	.creat_border_cells:
		inc dword[osob]
		or byte[ebx+12],1 ;+12 = .liv
		mov ecx,dword[x]
		dec ecx
		mov edx,dword[y]
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
	mov eax,dword[memCell]
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
			add eax,dword[memCell]
			mov eax,dword[eax] ;if(i) fnd=memCell[i];
			jmp .fun_e
	@@:

	cmp eax,0
	jne @f ;if(!fnd){ // поиск ячейки за бинарным деревом
		push ebx ecx edx edi esi
		;ebx -> i
		;ecx -> firstC
		;edx -> &memCell[i]
		;edi -> cell[memCell[i]]
		mov ecx,dword[memCell]
		mov ebx,dword[b_sort]
		mov edx,ebx
		shl edx,2
		add edx,ecx
		inc ebx
		mov ecx,dword[ecx]
		.cycle_b: ;for(i=b_sort+1;i<=fristC;i++)
			add edx,4
			get_cell_offset edi,dword[edx]
			mov esi,dword[x]
			cmp dword[edi],esi ;+0 = .x
			jne .if_e
			mov esi,dword[y]
			cmp dword[edi+4],esi ;+4 = .y
			jne .if_e
				;if(cell[memCell[i]].x==x && cell[memCell[i]].y==y){
				mov eax,dword[edx] ;fnd=memCell[i];
				jmp .cycle_e ;break;
			.if_e:
			inc ebx
			cmp ebx,ecx
			jle .cycle_b
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
	mov ebx,1 ;ebx - максимальный порядок для дерева
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

		mov edi,ecx
		shl edi,2
		add edi,dword[mas]
		;if(compare_cells_mb(mas[i],fx,fy)){
		stdcall pole_compare_cells_mb_coords, dword[edi],[fx],[fy]
		cmp dl,0
		je .if_u0_e
			@@: ;while(i+por>k)
			mov edx,ecx
			add edx,ebx
			cmp edx,dword[k] ;i+por>k
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

;output:
; dl
align 4
proc pole_compare_cells_bm_coords, i0:dword, fx:dword, fy:dword
	push eax ebx ecx
	get_cell_offset eax,[i0]
	;eax -> cell[i0]
	mov ebx,dword[fx]
	cmp dword[eax],ebx
	jle @f
		mov dl,1
		jmp .fun_e
	@@:
	mov ecx,dword[fy]
	cmp dword[eax+4],ecx
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

;output:
; dl
align 4
proc pole_compare_cells_mb_coords, i0:dword, fx:dword, fy:dword
	push eax ebx ecx
	get_cell_offset eax,[i0]
	;eax -> cell[i0]
	mov ebx,dword[fx]
	cmp dword[eax],ebx
	jge @f
		mov dl,1
		jmp .fun_e
	@@:
	mov ecx,dword[fy]
	cmp dword[eax+4],ecx
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
	mov ecx,dword[ebx] ;+0 = .x
	cmp dword[eax],ecx
	jle @f ;x0>x1
		mov dl,1
		jmp .fun_e
	@@:
	jne @f ;x0==x1
	mov ecx,dword[ebx+4] ;+4 = .y
	cmp dword[eax+4],ecx
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
	mov eax,dword[memCell]
	cmp dword[eax],0
	je .no_draw

	mov eax,dword[eax]
	mov ebx,1

;---
	@@: ;while(i<b_sort && Cor_x+cell[memCell[i]].x<0)
		cmp ebx,dword[b_sort]
		jge @f ;переходим на начало нижнего цикла
		mov ecx,ebx
		shl ecx,2
		add ecx,dword[memCell]
		get_cell_offset ecx,dword[ecx]
		mov edx,dword[ecx] ;+0 = .x
		add edx,dword[Cor_x]
		cmp edx,0
		jge @f ;переходим на начало нижнего цикла
			inc ebx ;i++; // для пропуска ячеек за окном слева
		jmp @b
	@@:

	cmp byte[zoom],2
	jge .zoom2
	@@: ;for(;i<=fristC;i++){
		mov ecx,ebx
		shl ecx,2
		add ecx,dword[memCell]
		get_cell_offset ecx,dword[ecx]
;...
		mov edi,dword[Cor_x]
		add edi,dword[ecx] ;+0 = .x
		mov esi,dword[Cor_y]
		add esi,dword[ecx+4] ;+4 = .y
		bt word[ecx+12],0 ;+12 = .liv
		jc .cell_1
			;не живая ячейка
			mov edx,dword[CellColors]
			mov edx,dword[edx]
			jmp .cell_0
		.cell_1:
			;живая ячейка
			mov edx,dword[tim]
			inc edx
			sub edx,dword[ecx+8] ;+8 = .tc
			cmp edx,COL_MEM
			jle .in_color
				mov edx,COL_MEM
			.in_color:
			shl edx,2
			add edx,dword[CellColors]
			mov edx,dword[edx]
		.cell_0:
		stdcall [buf2d_set_pixel], buf_0, edi, esi, edx
;...
		inc ebx
		cmp ebx,eax
		jle @b

	jmp .no_draw
	.zoom2:

	@@: ;for(;i<=fristC;i++){
		mov ecx,ebx
		shl ecx,2
		add ecx,dword[memCell]
		get_cell_offset ecx,dword[ecx]

		xor edx,edx
		mov dl,byte[zoom] ;edx используется для внесения zoom в 4 байтное число
		mov edi,dword[ecx] ;+0 = .x
		add edi,dword[Cor_x]
		imul edi,edx
		mov esi,dword[ecx+4] ;+4 = .y
		add esi,dword[Cor_y]
		imul esi,edx
		bt word[ecx+12],0 ;+12 = .liv
		jc .z2_cell_1
			;не живая ячейка
			mov edx,dword[CellColors]
			mov edx,dword[edx]
			jmp .z2_cell_0
		.z2_cell_1:
			;живая ячейка
			mov edx,dword[tim]
			inc edx
			sub edx,dword[ecx+8] ;+8 = .tc
			cmp edx,COL_MEM
			jle .z2_in_color
				mov edx,COL_MEM
			.z2_in_color:
			shl edx,2
			add edx,dword[CellColors]
			mov edx,dword[edx]
		.z2_cell_0:
		xor ecx,ecx
		mov cl,byte[zoom] ;ecx используется для внесения zoom в 4 байтное число
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

	mov eax,dword[memCell]
	mov ecx,eax
	mov eax,dword[eax]
	cmp eax,1
	jl .fun_e
	inc dword[tim]
	mov ebx,1
	@@: ;for(i=1;i<=firstC;i++)
		add ecx,4
		get_cell_offset edx,dword[ecx]
		bt word[edx+12],0 ;+12 = .liv
		jae .if_0_e
			; сохранение ячейки (соседей 2 или 3)
			cmp byte[edx+13],2 ;+13 = .so
			je .if_2_e
			cmp byte[edx+13],3 ;+13 = .so
			je .if_2_e
			jmp .change
		.if_0_e:
			; создание ячейки (соседей 3)
			cmp byte[edx+13],3 ;+13 = .so
			jne .if_1_e
			.change:
				or byte[edx+12],2 ;+12 = .liv
				jmp .if_2_e
		.if_1_e:
			; удаление пустой ячейки для освобождения памяти
			cmp byte[edx+13],0 ;+13 = .so
			jne .if_2_e
			mov edi,dword[edx+8] ;+8 = .tc
			add edi,5 ; 5 - время сохранения пустой ячейки, до её удаления
			cmp edi,dword[tim]
			jge .if_2_e
				mov edi,eax
				shl edi,2
				add edi,dword[memCell] ;edi -> &memCell[fristC]
				mov esi,dword[edi] ;swp=memCell[fristC];
				mov edx,dword[ecx] ;edx - уже не используем, потому можем портить
				mov dword[edi],edx ;memCell[fristC]=memCell[i];
				mov dword[ecx],esi ;memCell[i]=swp;
				dec eax
				dec ebx
				sub ecx,4
		.if_2_e:

		inc ebx
		cmp ebx,eax
		jle @b
	mov ebx,dword[memCell]
	mov dword[ebx],eax ;firstC <- eax

	mov dword[b_sort],eax
	stdcall pole_fl_sort, dword[memCell],eax

	mov ecx,dword[memCell]
	mov ebx,1
	@@: ;for(i=1;i<=firstC;i++)
		add ecx,4
		get_cell_offset edx,dword[ecx]
		bt word[edx+12],1 ;+12 = .liv
		jae .no_change
			xor byte[edx+12],3 ;+12 = .liv
			mov edi,dword[tim]
			mov dword[edx+8],edi ;+8 = .tc
			bt word[edx+12],0 ;+12 = .liv
			jc .new_cell
				push eax
				mov edi,dword[edx]
				dec edi
				mov esi,dword[edx+4]
				dec esi
				dec dword[osob]
				;дальше значение edx портится
				stdcall pole_cell_find,edi,esi
				get_cell_offset edx,eax
				dec byte[edx+13] ;+13 = .so
				inc esi
				stdcall pole_cell_find,edi,esi
				get_cell_offset edx,eax
				dec byte[edx+13] ;+13 = .so
				inc esi
				stdcall pole_cell_find,edi,esi
				get_cell_offset edx,eax
				dec byte[edx+13] ;+13 = .so
				inc edi
				stdcall pole_cell_find,edi,esi
				get_cell_offset edx,eax
				dec byte[edx+13] ;+13 = .so
				sub esi,2
				stdcall pole_cell_find,edi,esi
				get_cell_offset edx,eax
				dec byte[edx+13] ;+13 = .so
				inc edi
				stdcall pole_cell_find,edi,esi
				get_cell_offset edx,eax
				dec byte[edx+13] ;+13 = .so
				inc esi
				stdcall pole_cell_find,edi,esi
				get_cell_offset edx,eax
				dec byte[edx+13] ;+13 = .so
				inc esi
				stdcall pole_cell_find,edi,esi
				get_cell_offset edx,eax
				dec byte[edx+13] ;+13 = .so
				pop eax
				jmp .no_change
			.new_cell: ; появилась новая ячейка
				inc dword[osob]
				mov edi,dword[edx]
				dec edi
				mov esi,dword[edx+4]
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

;Сортировка вектора a[1..n] методом Флойда
align 4
proc pole_fl_sort, a:dword, n:dword
	pushad
	mov ecx,dword[a]
	;Формировать исходное частично упорядоченное дерево
	mov eax,dword[n]
	shr eax,1
	@@: ;for(i=n>>1; i>=2; i--)
		stdcall pole_fl_surface, ecx,eax,[n] ;(a,i,n)
		dec eax
		cmp eax,2
		jge @b
	;Выполнить процедуру всплытия Флойда для каждого поддерева
	mov eax,dword[n]
	@@: ;for(i=n; i>=2; i--){
		stdcall pole_fl_surface, ecx,1,eax ;(a,1,i)
		;Поместить найденный максимальный элемент в конец списка
		mov edi,eax
		shl edi,2
		add edi,ecx ;edi -> &a[i]
		mov esi,dword[edi] ;w=a[i];
		mov edx,dword[ecx+4]
		mov dword[edi],edx ;a[i]=a[1];
		mov dword[ecx+4],esi ;a[1]=w;

		dec eax
		cmp eax,2
		jge @b
	popad
	ret
endp

;Процедура всплытия Флойда по дереву a[1..k]
align 4
proc pole_fl_surface, a:dword, i:dword, k:dword
locals
	copy dd ?
endl
	pushad
	;edx -> ...
	;edi -> m
	;esi -> j
	mov eax,dword[a]
	mov ebx,dword[i]
	mov ecx,dword[k]

	mov edx,ebx
	shl edx,2
	add edx,eax
	mov edx,dword[edx]
	mov dword[copy],edx ;copy=a[i];
	mov edi,ebx
	shl edi,1 ;m=i<<1;
	.cycle_b: ;while (m<=k) {
		cmp edi,ecx
		jg .cycle_e
		jne @f ;if (m==k) j=m;
			mov esi,edi
			jmp .else_e
		@@: ;else if (pole_compare_cells_bm(a[m],a[m+1])) j=m;
		mov edx,edi
		shl edx,2
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
		mov edx,esi
		shl edx,2
		add edx,eax
		stdcall pole_compare_cells_bm, dword[edx],dword[copy]
		cmp dl,0
		je .cycle_e ;} else break; //выход из цикла

		mov edx,esi
		shl edx,2
		add edx,eax
		push dword[edx] ;push a[j];
		mov edx,ebx
		shl edx,2
		add edx,eax
		pop dword[edx] ;a[i]=a[j];
		mov ebx,esi ;i=j;
		mov edi,ebx
		shl edi,1 ;m=i<<1;

		jmp .cycle_b
	.cycle_e:

	;значения многих регистров уже не важны т. к. конец функции
	shl ebx,2
	add eax,ebx
	mov edx,dword[copy]
	mov dword[eax],edx ;a[i]=copy;

	popad
	ret
endp
;--------------------------------------


align 4
start:
	load_libraries l_libs_start,l_libs_end
	;проверка на сколько удачно загузилась наша либа
	mov	ebp,lib_7
	cmp	dword [ebp+ll_struc_size-4],0
	jz	@f
		mcall -1 ;exit not correct
	@@:
	mcall 48,3,sc,sizeof.system_colors
	mcall 40,0x27
	stdcall [OpenDialog_Init],OpenDialog_data ;подготовка диалога

	stdcall [buf2d_create], buf_0 ;создание буфера

	stdcall mem.Alloc,MAX_CELL*sizeof.Cell
	mov [cell],eax
	stdcall mem.Alloc,MAX_CELL*4
	mov [memCell],eax
	stdcall mem.Alloc,(COL_MEM+1)*4
	mov [CellColors],eax
	load_image_file fn_toolbar, image_data_toolbar,IMAGE_TOOLBAR_SIZE

	stdcall pole_init_colors, 0xffffff,0xff0000,0x0000ff
	call pole_clear
	call pole_paint ;рисование поля в буфере (не на экране)

	;xor eax,eax
	;mov edi,txt_zoom.zi
	;mov al,byte[zoom]
	;call tl_convert_to_str

	mcall 26,9
	mov [last_time],eax

align 4
red_win:
	call draw_window

align 4
still:
	mcall 26,9
	mov ebx,[last_time]
	add ebx,10 ;задержка
	cmp ebx,eax
	jge @f
		mov ebx,eax
	@@:
	sub ebx,eax
	;cmp ebx,10 ;задержка
	;ja timer_funct
	;test ebx,ebx
	;jz timer_funct
	mcall 23
	cmp eax,0
	je timer_funct

	cmp al,1
	jz red_win
	cmp al,2
	jz key
	cmp al,3
	jz button

	jmp still

align 4
timer_funct:
	pushad
	mcall 26,9
	mov [last_time],eax

	cmp byte[tim_ch],0
	je @f
		;call but_next_gen
		cld
		mov ecx,dword[poc_stop]
		cmp ecx,1
		jg .clear
			mov ecx,1 ;исправление ecx на случай чисел меньших 1
			jmp .cycle
		.clear: ;чистим поле если есть просчет на несколько поколений за 1 такт таймера
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
	mcall 12,1
	xor eax,eax
	mov ebx,(20 shl 16)+485
	mov ecx,(20 shl 16)+415
	mov edx,[sc.work]
	or  edx,(3 shl 24)+0x10000000+0x20000000
	mov edi,hed
	int 0x40

	mov eax,8
	mov ebx,(5 shl 16)+20
	mov ecx,(5 shl 16)+20
	mov edx,3
	mov esi,[sc.work_button]
	int 0x40

	mov ebx,(30 shl 16)+20
	mov ecx,(5 shl 16)+20
	mov edx,4
	int 0x40

	mov ebx,(55 shl 16)+20
	mov ecx,(5 shl 16)+20
	mov edx,5
	int 0x40

	mov ebx,(85 shl 16)+20
	mov ecx,(5 shl 16)+20
	mov edx,6
	int 0x40

	mov ebx,(110 shl 16)+20
	mov ecx,(5 shl 16)+20
	mov edx,7
	int 0x40

	mov ebx,(135 shl 16)+20
	mov ecx,(5 shl 16)+20
	mov edx,8
	int 0x40

	mov ebx,(165 shl 16)+20
	mov ecx,(5 shl 16)+20
	mov edx,9
	int 0x40

	mov ebx,(190 shl 16)+20
	mov ecx,(5 shl 16)+20
	mov edx,10
	int 0x40

	mov ebx,(220 shl 16)+20
	mov ecx,(5 shl 16)+20
	mov edx,11
	int 0x40

	mov ebx,(245 shl 16)+20
	mov ecx,(5 shl 16)+20
	mov edx,12
	int 0x40

	mov ebx,(270 shl 16)+20
	mov ecx,(5 shl 16)+20
	mov edx,13
	int 0x40

	mov ebx,(295 shl 16)+20
	mov ecx,(5 shl 16)+20
	mov edx,14
	int 0x40

	mov eax,7
	mov ebx,[image_data_toolbar]
	mov ecx,(16 shl 16)+16
	mov edx,(32 shl 16)+7
	int 0x40

	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	mov edx,(87 shl 16)+7 ;run once
	int 0x40

	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	mov edx,(112 shl 16)+7 ;run auto
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	mov edx,(137 shl 16)+7 ;stop
	int 0x40

	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	mov edx,(167 shl 16)+7 ;-
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	mov edx,(192 shl 16)+7 ;+
	int 0x40

	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	mov edx,(222 shl 16)+7 ;move up
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	mov edx,(247 shl 16)+7 ;move doun
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	mov edx,(272 shl 16)+7 ;move left
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	mov edx,(297 shl 16)+7 ;move up
	int 0x40

	call draw_pok

	stdcall [buf2d_draw], buf_0

	mcall 12,2
popad
	ret

align 4
draw_pok:
	mov eax,4 ;рисование текста
	mov ebx,325*65536+5
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

	mov eax,47
	xor ecx,ecx
	mov cl,byte[zoom]
	mov ebx,(2 shl 16)
	mov edx,(325+6*9)*65536+5
	mov esi,[sc.work_button_text]
	or  esi,(1 shl 30)
	mov edi,[sc.work_button]
	int 0x40 ;масштаб
	mov ebx,(5 shl 16)
	mov ecx,[tim]
	add edx,(6*2)*65536+9
	int 0x40 ;время
	mov ebx,(5 shl 16)
	mov ecx,[osob]
	add edx,(6*0)*65536+9
	int 0x40 ;популяция
	ret

align 4
key:
	mcall 2
	jmp still


align 4
button:
	mcall 17
	cmp ah,3
	jne @f
		call but_new_file
	@@:
	cmp ah,4
	jne @f
		call but_open_file
	@@:
	cmp ah,5
	jne @f
		call but_save_file
	@@:
	cmp ah,6
	jne @f
		call but_next_gen
	@@:
	cmp ah,7
	jne @f
		call but_run
	@@:
	cmp ah,8
	jne @f
		call but_stop
	@@:
	cmp ah,9
	jne @f
		call but_zoom_p
	@@:
	cmp ah,10
	jne @f
		call but_zoom_m
	@@:
	cmp ah,11
	jne @f
		call but_pole_up
	@@:
	cmp ah,12
	jne @f
		call but_pole_dn
	@@:
	cmp ah,13
	jne @f
		call but_pole_left
	@@:
	cmp ah,14
	jne @f
		call but_pole_right
	@@:
	cmp ah,1
	jne still
.exit:
	stdcall [buf2d_delete],buf_0
	stdcall mem.Free,[cell]
	stdcall mem.Free,[memCell]
	stdcall mem.Free,[CellColors]
	stdcall mem.Free,[image_data_toolbar]
	mcall -1


align 4
but_new_file:
	ret

align 4
open_file_lif:
	rb 4096 ;область для открытия файлов
.end:

align 4
but_open_file:
	pushad
	copy_path open_dialog_name,communication_area_default_path,file_name,0
	mov [OpenDialog_data.type],0
	stdcall [OpenDialog_Start],OpenDialog_data
	cmp [OpenDialog_data.status],2
	je .end_open_file
	;код при удачном открытии диалога

	mov eax,70 ;70-я функция работа с файлами
	mov [run_file_70.Function], 0
	mov [run_file_70.Position], 0
	mov [run_file_70.Flags], 0
	mov dword[run_file_70.Count], open_file_lif.end-open_file_lif
	m2m [run_file_70.Buffer], open_file_lif
	mov byte[run_file_70+20], 0
	mov dword[run_file_70.FileName], openfile_path
	mov ebx,run_file_70
	int 0x40 ;загружаем файл изображения
	cmp ebx,0xffffffff
	je .end_open_file

	add ebx,open_file_lif
	mov byte[ebx],0 ;на случай если ранее был открыт файл большего размера чистим конец буфера с файлом
	mcall 71,1,openfile_path

	call pole_clear
	mov eax,dword[buf_0.w]
	shr eax,1
	xor ecx,ecx
	mov cl,byte[zoom]
	cmp cx,2
	jl @f ;деление на величину zoom
		xor edx,edx
		div ecx
	@@:
	add dword[Cor_x],eax
	mov eax,dword[buf_0.h]
	shr eax,1
	cmp cx,2
	jl @f ;деление на величину zoom
		xor edx,edx
		div ecx
	@@:
	add dword[Cor_y],eax

	;eax - first x position
	;ebx - x position
	;ecx - y position
	mov edi,open_file_lif
	xor ebx,ebx
	xor ecx,ecx
	mov eax,ebx
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
		cmp word[edi],'#P' ;смена позиции
		jne .pos
			inc edi ;пропуск '#'
			.space:
				inc edi ;пропуск 'P' и ' '
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
				inc edi ;пропуск 'P' и ' '
				cmp byte[edi],' '
				je .digit_no ;.space_1
			stdcall conv_str_to_int,edi
			mov ecx,eax
			mov eax,ebx ;восстановление левого отступа в eax
		.pos:
		inc edi
		cmp byte[edi],0
		jne @b
	;---
	stdcall [buf2d_clear], buf_0, [buf_0.color] ;чистим буфер
	call pole_paint ;рисуем поле (на случай если есть сетка или текстовые подписи)
	stdcall [buf2d_draw], buf_0 ;обновляем буфер на экране
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
		;вычисление сдвигов для поля, которые обеспечат центровку поля при увеличении масштаба
		xor ecx,ecx
		mov cl,byte[zoom]
		xor edx,edx
		mov eax,dword[buf_0.w]
		shr eax,1 ;в eax половина ширины поля
		mov ebx,eax ;делаем резервную копию eax
		div ecx ;делим eax на текущий масштаб
		xchg eax,ebx
		xor edx,edx
		inc ecx
		div ecx ;делим eax на новый масштаб
		sub ebx,eax ;вычисляется сдвиг поля который обеспечит центровку поля
		sub dword[Cor_x],ebx ;сдвигаем поле зрения по оси x
		xor ecx,ecx
		mov cl,byte[zoom]
		xor edx,edx
		mov eax,dword[buf_0.h]
		shr eax,1
		mov ebx,eax
		div ecx
		xchg eax,ebx
		xor edx,edx
		inc ecx
		div ecx
		sub ebx,eax
		sub dword[Cor_y],ebx ;сдвигаем поле зрения по оси y

		inc byte[zoom]
		;xor eax,eax
		;mov edi,txt_zoom.zi
		;mov al,byte[zoom]
		;call tl_convert_to_str
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
		;вычисление сдвигов для поля, которые обеспечат центровку поля при уменьшении масштаба
		xor ecx,ecx
		mov cl,byte[zoom]
		xor edx,edx
		mov eax,dword[buf_0.w]
		shr eax,1 ;в eax половина ширины поля
		mov ebx,eax ;делаем резервную копию eax
		div ecx ;делим eax на текущий масштаб
		xchg eax,ebx
		xor edx,edx
		dec ecx
		div ecx ;делим eax на новый масштаб
		sub ebx,eax ;вычисляется сдвиг поля который обеспечит центровку поля
		sub dword[Cor_x],ebx ;сдвигаем поле зрения по оси x
		xor ecx,ecx
		mov cl,byte[zoom]
		xor edx,edx
		mov eax,dword[buf_0.h]
		shr eax,1
		mov ebx,eax
		div ecx
		xchg eax,ebx
		xor edx,edx
		dec ecx
		div ecx
		sub ebx,eax
		sub dword[Cor_y],ebx ;сдвигаем поле зрения по оси y

		dec byte[zoom]
		;xor eax,eax
		;mov edi,txt_zoom.zi
		;mov al,byte[zoom]
		;call tl_convert_to_str
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
	mov eax,dword[buf_0.h]
	shr eax,2
	xor ecx,ecx
	mov cl,byte[zoom]
	cmp cx,2
	jl @f ;деление на величину zoom
		xor edx,edx
		div ecx
	@@:
	add dword[Cor_y],eax
	pop edx ecx eax
	stdcall [buf2d_clear], buf_0, [buf_0.color]
	call pole_paint
	stdcall [buf2d_draw], buf_0
	ret

align 4
but_pole_dn:
	push eax ecx edx
	mov eax,dword[buf_0.h]
	shr eax,2
	xor ecx,ecx
	mov cl,byte[zoom]
	cmp cx,2
	jl @f ;деление на величину zoom
		xor edx,edx
		div ecx
	@@:
	sub dword[Cor_y],eax
	pop edx ecx eax
	stdcall [buf2d_clear], buf_0, [buf_0.color]
	call pole_paint
	stdcall [buf2d_draw], buf_0
	ret

align 4
but_pole_left:
	push eax ecx edx
	mov eax,dword[buf_0.w]
	shr eax,2
	xor ecx,ecx
	mov cl,byte[zoom]
	cmp cx,2
	jl @f ;деление на величину zoom
		xor edx,edx
		div ecx
	@@:
	add dword[Cor_x],eax
	pop edx ecx eax
	stdcall [buf2d_clear], buf_0, [buf_0.color]
	call pole_paint
	stdcall [buf2d_draw], buf_0
	ret

align 4
but_pole_right:
	push eax ecx edx
	mov eax,dword[buf_0.w]
	shr eax,2
	xor ecx,ecx
	mov cl,byte[zoom]
	cmp cx,2
	jl @f ;деление на величину zoom
		xor edx,edx
		div ecx
	@@:
	sub dword[Cor_x],eax
	pop edx ecx eax
	stdcall [buf2d_clear], buf_0, [buf_0.color]
	call pole_paint
	stdcall [buf2d_draw], buf_0
	ret

;align 4
;but_bru_clear:
;        ret

;input:
; buf - указатель на строку, число должно быть в 10 или 16 ричном виде
;output:
; eax - число
align 4
proc conv_str_to_int, buf:dword
	xor eax,eax
	push ebx ecx esi
	xor ebx,ebx
	mov esi,[buf]
	;определение отрицательных чисел
	xor ecx,ecx
	inc ecx
	cmp byte[esi],'-'
	jne @f
		dec ecx
		inc esi
	@@:

	cmp word[esi],'0x'
	je .load_digit_16

	.load_digit_10: ;считывание 10-тичных цифр
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

	.load_digit_16: ;считывание 16-ричных цифр
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
			jl @f ;отсеиваем символы >'9' и <'A'
		.us1: ;составное условие
		cmp bl,'F'
		jle .us2
			cmp bl,'a'
			jl @f ;отсеиваем символы >'F' и <'a'
			sub bl,32 ;переводим символы в верхний регистр, для упрощения их последущей обработки
		.us2: ;составное условие
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
	cmp ecx,0 ;если число отрицательное
	jne @f
		sub ecx,eax
		mov eax,ecx
	@@:
	pop esi ecx ebx
	ret
endp

;данные для диалога открытия файлов
align 4
OpenDialog_data:
.type			dd 0 ;0 - открыть, 1 - сохранить, 2 - выбрать дтректорию
.procinfo		dd procinfo	;+4
.com_area_name		dd communication_area_name	;+8
.com_area		dd 0	;+12
.opendir_path		dd plugin_path	;+16
.dir_default_path	dd default_dir ;+20
.start_path		dd file_name ;+24 путь к диалогу открытия файлов
.draw_window		dd draw_window	;+28
.status 		dd 0	;+32
.openfile_path		dd openfile_path	;+36 путь к открываемому файлу
.filename_area		dd filename_area	;+40
.filter_area		dd Filter
.x:
.x_size 		dw 420 ;+48 ; Window X size
.x_start		dw 10 ;+50 ; Window X position
.y:
.y_size 		dw 320 ;+52 ; Window y size
.y_start		dw 10 ;+54 ; Window Y position

default_dir db '/rd/1',0

communication_area_name:
	db 'FFFFFFFF_open_dialog',0
open_dialog_name:
	db 'opendial',0
communication_area_default_path:
	db '/rd/1/File managers/',0

Filter:
dd Filter.end - Filter ;.1
.1:
db 'LIF',0
db 'RLE',0
.end:
db 0



head_f_i:
head_f_l db 'Системная ошибка',0

system_dir_0 db '/sys/lib/'
lib_name_0 db 'proc_lib.obj',0
err_message_found_lib_0 db 'Не найдена библиотека ',39,'proc_lib.obj',39,0
err_message_import_0 db 'Ошибка при импорте библиотеки ',39,'proc_lib.obj',39,0

system_dir_1 db '/sys/lib/'
lib_name_1 db 'libimg.obj',0
err_message_found_lib_1 db 'Не найдена библиотека ',39,'libimg.obj',39,0
err_message_import_1 db 'Ошибка при импорте библиотеки ',39,'libimg.obj',39,0

system_dir_7 db '/sys/lib/'
lib_name_7 db 'buf2d.obj',0
err_msg_found_lib_7 db 'Не найдена библиотека ',39,'buf2d.obj',39,0
err_msg_import_7 db 'Ошибка при импорте библиотеки ',39,'buf2d',39,0

l_libs_start:
	lib0 l_libs lib_name_0, sys_path, file_name, system_dir_0,\
		err_message_found_lib_0, head_f_l, proclib_import,err_message_import_0, head_f_i
	lib1 l_libs lib_name_1, sys_path, file_name, system_dir_1,\
		err_message_found_lib_1, head_f_l, import_libimg, err_message_import_1, head_f_i
	lib_7 l_libs lib_name_7, sys_path, library_path, system_dir_7,\
		err_msg_found_lib_7,head_f_l,import_buf2d,err_msg_import_7,head_f_i
l_libs_end:

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
	aimg_is_img  db 'img_is_img',0 ;определяет по данным, может ли библиотека сделать из них изображение
	aimg_info    db 'img_info',0
	aimg_from_file db 'img_from_file',0
	aimg_to_file db 'img_to_file',0
	aimg_from_rgb db 'img_from_rgb',0
	aimg_to_rgb  db 'img_to_rgb',0 ;преобразование изображения в данные RGB
	aimg_to_rgb2 db 'img_to_rgb2',0
	aimg_decode  db 'img_decode',0 ;автоматически определяет формат графических данных
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
proclib_import: ;описание экспортируемых функций
	OpenDialog_Init dd aOpenDialog_Init
	OpenDialog_Start dd aOpenDialog_Start
dd 0,0
	aOpenDialog_Init db 'OpenDialog_init',0
	aOpenDialog_Start db 'OpenDialog_start',0

align 4
import_buf2d:
	init dd sz_init
	buf2d_create dd sz_buf2d_create
	buf2d_create_f_img dd sz_buf2d_create_f_img
	buf2d_clear dd sz_buf2d_clear
	buf2d_draw dd sz_buf2d_draw
	buf2d_delete dd sz_buf2d_delete
	buf2d_line dd sz_buf2d_line
	buf2d_rect_by_size dd sz_buf2d_rect_by_size
	buf2d_filled_rect_by_size dd sz_buf2d_filled_rect_by_size
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
	buf2d_flood_fill dd sz_buf2d_flood_fill
	buf2d_set_pixel dd sz_buf2d_set_pixel
	dd 0,0
	sz_init db 'lib_init',0
	sz_buf2d_create db 'buf2d_create',0
	sz_buf2d_create_f_img db 'buf2d_create_f_img',0
	sz_buf2d_clear db 'buf2d_clear',0
	sz_buf2d_draw db 'buf2d_draw',0
	sz_buf2d_delete db 'buf2d_delete',0
	sz_buf2d_line db 'buf2d_line',0
	sz_buf2d_rect_by_size db 'buf2d_rect_by_size',0
	sz_buf2d_filled_rect_by_size db 'buf2d_filled_rect_by_size',0
	sz_buf2d_circle db 'buf2d_circle',0
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
	sz_buf2d_flood_fill db 'buf2d_flood_fill',0
	sz_buf2d_set_pixel db 'buf2d_set_pixel',0

mouse_dd dd 0x0
sc system_colors 
last_time dd 0

align 16
procinfo process_information 

align 4
buf_0: dd 0 ;a?азаa?лi на ?aa?a ?зо?aаж?н?i
	dw 5 ;+4 left
	dw 35 ;+6 top
.w: dd 460 ;+8 w
.h: dd 340 ;+12 h
.color: dd 0xffffd0 ;+16 color
	db 24 ;+20 bit in pixel

;этот код не мой, он преобразует число в строку
;input:
; eax = value
; edi = string buffer
;output:
align 4
tl_convert_to_str:
	pushad
		mov dword[edi+1],0
		call .str
	popad
	ret

align 4
.str:
	mov ecx,0x0a ;задается система счисления изменяются регистры ebx,eax,ecx,edx входные параметры eax - число
    ;преревод числа в ASCII строку взодные данные ecx=система счисленя edi адрес куда записывать, будем строку, причем конец переменной 
	cmp eax,ecx  ;сравнить если в eax меньше чем в ecx то перейти на @@-1 т.е. на pop eax
	jb @f
		xor edx,edx  ;очистить edx
		div ecx      ;разделить - остаток в edx
		push edx     ;положить в стек
		;dec edi             ;смещение необходимое для записи с конца строки
		call .str ;перейти на саму себя т.е. вызвать саму себя и так до того момента пока в eax не станет меньше чем в ecx
		pop eax
	@@: ;cmp al,10 ;проверить не меньше ли значение в al чем 10 (для системы счисленя 10 данная команда - лишная))
	or al,0x30  ;данная команда короче  чем две выше
	stosb	    ;записать элемент из регистра al в ячеку памяти es:edi
	ret	      ;вернуться чень интересный ход т.к. пока в стеке храниться кол-во вызовов то столько раз мы и будем вызываться

i_end:
	rb 1024
stacktop:
	sys_path rb 1024
	file_name:
		rb 1024 ;4096 
	library_path rb 1024
	plugin_path rb 4096
	openfile_path rb 4096
	filename_area rb 256
mem:

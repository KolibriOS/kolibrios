;
; Функции нужные для отображения воксельного объекта через библиотеку tinygl
;

normal_gran_z1 equ -1.0
normal_gran_z0 equ  1.0
normal_gran_y1 equ -1.0
normal_gran_y0 equ  1.0
normal_gran_x1 equ -1.0
normal_gran_x0 equ  1.0

;марос коректировки вектора нормали для закругления крайних вокселей
macro normal_gran param, gran
{
    mov dword[param],0.0
    bt dword[edi+vox_ogl_planes],vox_ogl_gran_#gran
    jnc @f
        mov dword[param],normal_gran_#gran
    @@:
}

;марос коректировки вектора нормали для диагонального сглаживания вокселей
;диагональные воксели после этого сглаживания смотрятся не степеньками а сплошной плоскостью
macro normal_gran_2 param, gran2, gran1
{
    bt dword[edi+vox_ogl_planes],vox_ogl_gran_#gran2
    jnc @f
        mov dword[param],normal_gran_#gran1 ;поставить 0.0 в dword[param] если нужно не сильное сглаживание
    @@:
}

vox_ogl_x0 equ 0
vox_ogl_y0 equ 4
vox_ogl_z0 equ 8
vox_ogl_x1 equ 12
vox_ogl_y1 equ 16
vox_ogl_z1 equ 20
vox_ogl_color equ 24
vox_ogl_zoom equ 28
vox_ogl_planes equ 30
vox_ogl_size equ 34

def_del_planes equ 63

;номера битов, которые указывают на то какие есть соседние воксели
;нужно для отсечения внутренних граней между соседними вокселями
;а еще нужно для задания векторов нормалей, что-бы воксели были сглаженными (закругленными)
vox_ogl_gran_z1 equ 1
vox_ogl_gran_z0 equ 0
vox_ogl_gran_y1 equ 3
vox_ogl_gran_y0 equ 2
vox_ogl_gran_x1 equ 5
vox_ogl_gran_x0 equ 4
vox_ogl_gran_y1z1 equ 7
vox_ogl_gran_y0z0 equ 6
vox_ogl_gran_x1z1 equ 9
vox_ogl_gran_x0z0 equ 8
vox_ogl_gran_y0z1 equ 11
vox_ogl_gran_y1z0 equ 10
vox_ogl_gran_x0z1 equ 13
vox_ogl_gran_x1z0 equ 12
vox_ogl_gran_x1y1 equ 15
vox_ogl_gran_x0y0 equ 14
vox_ogl_gran_x0y1 equ 17
vox_ogl_gran_x1y0 equ 16


vox_offs_tree_table equ 4
vox_offs_data equ 12


;description:
; создание воксельного объекта для показа в 3d графике
;input:
align 4
proc buf_vox_obj_create_3d, v_obj:dword, p_mem:dword, coord_x:dword,\
coord_y:dword, k_scale:dword
	cmp [k_scale],0
	jl .end_f
pushad
	mov edi,[p_mem]
    mov dword[edi],0 ;count voxels

	mov ecx,[k_scale]
	mov ebx,[coord_x]
	mov edx,[coord_y]
	mov edi,[v_obj]
	add edi,vox_offs_data
	xor esi,esi
	stdcall create_sub_vox_obj_3d, [v_obj],[p_mem],[k_scale]

    ; (1)
    ; сортировка вокселей по координатам x,y,z
    ;
    mov edi,[p_mem]
    mov ecx,dword[edi]
    ;inc ecx
    add edi,4
    sub edi,vox_ogl_size
    stdcall pole_fl_sort, edi, ecx

    ;отсечение соседних граней для ускорения отрисовки
    mov edi,[p_mem]
    mov ecx,dword[edi]
    dec ecx
    add edi,4

align 4
    .cycle_0:
        mov ax,word[edi+vox_ogl_zoom]
        cmp ax,word[edi+vox_ogl_size+vox_ogl_zoom]
        jne @f
        mov eax,dword[edi+vox_ogl_x0]
        cmp eax,dword[edi+vox_ogl_size+vox_ogl_x0]
        jne @f
        mov eax,dword[edi+vox_ogl_y0]
        cmp eax,dword[edi+vox_ogl_size+vox_ogl_y0]
        jne @f
        mov eax,dword[edi+vox_ogl_z0]
        inc eax ;увеличиваем высоту, для будущего сравнения
        cmp eax,dword[edi+vox_ogl_size+vox_ogl_z0]
        jne @f
        ;если по высоте воксели различается на 1 координату, то соседние высоту и низ отсекаем
        btr dword[edi+vox_ogl_planes],vox_ogl_gran_z1 ;верх отсекаем
        btr dword[edi+vox_ogl_size+vox_ogl_planes],vox_ogl_gran_z0 ;низ отсекаем
    @@:
        add edi,vox_ogl_size
        loop .cycle_0

push ebx edx esi
    ;диагональные соседние воксели
    mov edi,[p_mem]
    mov ecx,dword[edi]
    dec ecx
    add edi,4

    mov ebx,ecx
    ;inc ebx ;??? cmp esi,ebx -> jge @f
    imul ebx,vox_ogl_size
    add ebx,edi

align 4
    .cycle_3:
        mov dx,word[edi+vox_ogl_zoom]
        mov esi,edi
align 4
        .cycle_4:
        add esi,vox_ogl_size
        cmp esi,ebx
        jg @f
        cmp dx,word[esi+vox_ogl_zoom]
        jne .cycle_4
        
        mov eax,dword[edi+vox_ogl_x0]
        cmp eax,dword[esi+vox_ogl_x0]
        jne @f
        mov eax,dword[edi+vox_ogl_y0]
        inc eax ; y+
        cmp eax,dword[esi+vox_ogl_y0]
        jl @f
        jne .cycle_4
        mov eax,dword[edi+vox_ogl_z0]
        inc eax ; z+
        cmp eax,dword[esi+vox_ogl_z0]
        jne .cycle_4
        ;если по высоте воксели различается на 1 координату
        bts dword[edi+vox_ogl_planes],vox_ogl_gran_y1z1 ;
        bts dword[esi+vox_ogl_planes],vox_ogl_gran_y0z0 ;
    @@:
        add edi,vox_ogl_size
        loop .cycle_3

; ***
    mov edi,[p_mem]
    mov ecx,dword[edi]
    dec ecx
    add edi,4

align 4
    .cycle_7:
        mov dx,word[edi+vox_ogl_zoom]
        mov esi,edi
align 4
        .cycle_8:
        add esi,vox_ogl_size
        cmp esi,ebx
        jg @f
        cmp dx,word[esi+vox_ogl_zoom]
        jne .cycle_8
        
        mov eax,dword[edi+vox_ogl_x0]
        cmp eax,dword[esi+vox_ogl_x0]
        jne @f
        mov eax,dword[edi+vox_ogl_y0]
        inc eax ; y+
        cmp eax,dword[esi+vox_ogl_y0]
        jl @f
        jne .cycle_8
        mov eax,dword[edi+vox_ogl_z0]
        dec eax ; z-
        cmp eax,dword[esi+vox_ogl_z0]
        jne .cycle_8
        ;если по высоте воксели различается на 1 координату
        bts dword[edi+vox_ogl_planes],vox_ogl_gran_y1z0 ;
        bts dword[esi+vox_ogl_planes],vox_ogl_gran_y0z1 ;
    @@:
        add edi,vox_ogl_size
        loop .cycle_7
pop esi edx ebx

    ; (2)
    ; сортировка вокселей по координатам x,z,y
    ;
    mov edi,[p_mem]
    mov ecx,dword[edi]
    add edi,4
    sub edi,vox_ogl_size
    stdcall pole_fl_sort_zxy, edi, ecx

    ;отсечение соседних граней для ускорения отрисовки
    mov edi,[p_mem]
    mov ecx,dword[edi]
    dec ecx
    add edi,4

align 4
    .cycle_1:
        mov ax,word[edi+vox_ogl_zoom]
        cmp ax,word[edi+vox_ogl_size+vox_ogl_zoom]
        jne @f
        mov eax,dword[edi+vox_ogl_z0]
        cmp eax,dword[edi+vox_ogl_size+vox_ogl_z0]
        jne @f
        mov eax,dword[edi+vox_ogl_x0]
        cmp eax,dword[edi+vox_ogl_size+vox_ogl_x0]
        jne @f
        mov eax,dword[edi+vox_ogl_y0]
        inc eax ;увеличиваем высоту, для будущего сравнения
        cmp eax,dword[edi+vox_ogl_size+vox_ogl_y0]
        jne @f
        ;если по высоте воксели различается на 1 координату, то соседние высоту и низ отсекаем
        btr dword[edi+vox_ogl_planes],vox_ogl_gran_y1 ;верх отсекаем
        btr dword[edi+vox_ogl_size+vox_ogl_planes],vox_ogl_gran_y0 ;низ отсекаем
    @@:
        add edi,vox_ogl_size
        loop .cycle_1

push ebx edx esi
    ;диагональные соседние воксели
    mov edi,[p_mem]
    mov ecx,dword[edi]
    dec ecx
    add edi,4

    mov ebx,ecx
    ;inc ebx ;??? cmp esi,ebx -> jge @f
    imul ebx,vox_ogl_size
    add ebx,edi

align 4
    .cycle_11:
        mov dx,word[edi+vox_ogl_zoom]
        mov esi,edi
align 4
        .cycle_12:
        add esi,vox_ogl_size
        cmp esi,ebx
        jg @f
        cmp dx,word[esi+vox_ogl_zoom]
        jne .cycle_12
        
        mov eax,dword[edi+vox_ogl_z0]
        cmp eax,dword[esi+vox_ogl_z0]
        jne @f
        mov eax,dword[edi+vox_ogl_x0]
        inc eax ; x+
        cmp eax,dword[esi+vox_ogl_x0]
        jl @f
        jne .cycle_12
        mov eax,dword[edi+vox_ogl_y0]
        inc eax ; y+
        cmp eax,dword[esi+vox_ogl_y0]
        jne .cycle_12
        ;если по высоте воксели различается на 1 координату
        bts dword[edi+vox_ogl_planes],vox_ogl_gran_x1y1 ;
        bts dword[esi+vox_ogl_planes],vox_ogl_gran_x0y0 ;
    @@:
        add edi,vox_ogl_size
        loop .cycle_11

; ***
    mov edi,[p_mem]
    mov ecx,dword[edi]
    dec ecx
    add edi,4

align 4
    .cycle_15:
        mov dx,word[edi+vox_ogl_zoom]
        mov esi,edi
align 4
        .cycle_16:
        add esi,vox_ogl_size
        cmp esi,ebx
        jg @f
        cmp dx,word[esi+vox_ogl_zoom]
        jne .cycle_16
        
        mov eax,dword[edi+vox_ogl_z0]
        cmp eax,dword[esi+vox_ogl_z0]
        jne @f
        mov eax,dword[edi+vox_ogl_x0]
        inc eax ; x+
        cmp eax,dword[esi+vox_ogl_x0]
        jl @f
        jne .cycle_16
        mov eax,dword[edi+vox_ogl_y0]
        dec eax ; y-
        cmp eax,dword[esi+vox_ogl_y0]
        jne .cycle_16
        ;если по высоте воксели различается на 1 координату
        bts dword[edi+vox_ogl_planes],vox_ogl_gran_x1y0 ;
        bts dword[esi+vox_ogl_planes],vox_ogl_gran_x0y1 ;
    @@:
        add edi,vox_ogl_size
        loop .cycle_15

pop esi edx ebx

    ; (3)
    ; сортировка вокселей по координатам y,z,x
    ;
    mov edi,[p_mem]
    mov ecx,dword[edi]
    add edi,4
    sub edi,vox_ogl_size
    stdcall pole_fl_sort_yzx, edi, ecx

    ;отсечение соседних граней для ускорения отрисовки
    mov edi,[p_mem]
    mov ecx,dword[edi]
    dec ecx
    add edi,4

align 4
    .cycle_2:
        mov ax,word[edi+vox_ogl_zoom]
        cmp ax,word[edi+vox_ogl_size+vox_ogl_zoom]
        jne @f
        mov eax,dword[edi+vox_ogl_y0]
        cmp eax,dword[edi+vox_ogl_size+vox_ogl_y0]
        jne @f
        mov eax,dword[edi+vox_ogl_z0]
        cmp eax,dword[edi+vox_ogl_size+vox_ogl_z0]
        jne @f
        mov eax,dword[edi+vox_ogl_x0]
        inc eax ;увеличиваем высоту, для будущего сравнения
        cmp eax,dword[edi+vox_ogl_size+vox_ogl_x0]
        jne @f
        ;если по высоте воксели различается на 1 координату, то соседние высоту и низ отсекаем
        btr dword[edi+vox_ogl_planes],vox_ogl_gran_x1 ;верх отсекаем
        btr dword[edi+vox_ogl_size+vox_ogl_planes],vox_ogl_gran_x0 ;низ отсекаем
    @@:
        add edi,vox_ogl_size
        loop .cycle_2

push ebx edx esi
    ;диагональные соседние воксели
    mov edi,[p_mem]
    mov ecx,dword[edi]
    dec ecx
    add edi,4

    mov ebx,ecx
    ;inc ebx ;??? cmp esi,ebx -> jge @f
    imul ebx,vox_ogl_size
    add ebx,edi

align 4
    .cycle_5:
        mov dx,word[edi+vox_ogl_zoom]
        mov esi,edi
align 4
        .cycle_6:
        add esi,vox_ogl_size
        cmp esi,ebx
        jg @f
        cmp dx,word[esi+vox_ogl_zoom]
        jne .cycle_6
        
        mov eax,dword[edi+vox_ogl_y0]
        cmp eax,dword[esi+vox_ogl_y0]
        jne @f
        mov eax,dword[edi+vox_ogl_z0]
        inc eax ; z+
        cmp eax,dword[esi+vox_ogl_z0]
        jl @f
        jne .cycle_6
        mov eax,dword[edi+vox_ogl_x0]
        inc eax ; x+
        cmp eax,dword[esi+vox_ogl_x0]
        jne .cycle_6
        ;если по высоте воксели различается на 1 координату
        bts dword[edi+vox_ogl_planes],vox_ogl_gran_x1z1 ;
        bts dword[esi+vox_ogl_planes],vox_ogl_gran_x0z0 ;
    @@:
        add edi,vox_ogl_size
        loop .cycle_5

; ***
    mov edi,[p_mem]
    mov ecx,dword[edi]
    dec ecx
    add edi,4

align 4
    .cycle_9:
        mov dx,word[edi+vox_ogl_zoom]
        mov esi,edi
align 4
        .cycle_10:
        add esi,vox_ogl_size
        cmp esi,ebx
        jg @f
        cmp dx,word[esi+vox_ogl_zoom]
        jne .cycle_10
        
        mov eax,dword[edi+vox_ogl_y0]
        cmp eax,dword[esi+vox_ogl_y0]
        jne @f
        mov eax,dword[edi+vox_ogl_z0]
        inc eax ; z+
        cmp eax,dword[esi+vox_ogl_z0]
        jl @f
        jne .cycle_10
        mov eax,dword[edi+vox_ogl_x0]
        dec eax ; x-
        cmp eax,dword[esi+vox_ogl_x0]
        jne .cycle_10
        ;если по высоте воксели различается на 1 координату
        bts dword[edi+vox_ogl_planes],vox_ogl_gran_x0z1 ;
        bts dword[esi+vox_ogl_planes],vox_ogl_gran_x1z0 ;
    @@:
        add edi,vox_ogl_size
        loop .cycle_9

pop esi edx ebx

    ;преобразование координат во float
    stdcall vox_obj_3d_recalc, [p_mem]
popad
	.end_f:
	ret
endp

;input:
; ebx - coord_x
; edx - coord_y
; esi - coord_z
; ecx - уровень текушего узла 
; edi - указатель на данные воксельного объекта
align 4
proc create_sub_vox_obj_3d, v_obj:dword, p_mem:dword, k_scale:dword
	cmp byte[edi+3],0 ;смотрим есть ли поддеревья
	je .sub_trees

		;добавляем узел, который содержит дочерние узлы, при этом дочерние мелкие и не попадают на вывод
		cmp ecx,0
		jne @f
			push ecx
                mov eax,[p_mem]
                inc dword[eax] ;увеличиваем счетчик вокселей на 1
                mov eax,[eax]
                imul eax,vox_ogl_size ;умножаем на размер данных о вокселе
                add eax,4-vox_ogl_size ;число вокселей 4 байта
                add eax,[p_mem]
                mov [eax+vox_ogl_x0],ebx
                mov [eax+vox_ogl_y0],edx
                mov [eax+vox_ogl_z0],esi
                sub ecx,[k_scale]
                mov word[eax+vox_ogl_zoom],cx
                mov dword[eax+vox_ogl_planes],def_del_planes
				mov ecx,dword[edi]
				and ecx,0xffffff
                mov [eax+vox_ogl_color],ecx
			pop ecx
		@@:

		;рекурсивный перебор поддеревьев
		push edx
		;вход внутрь узла
		dec ecx

		mov eax,1
		cmp ecx,1
		jl @f
			shl eax,cl
		@@:

		add edx,eax ;коректировка высоты под воксель нижнего уровня

		mov ah,byte[edi+3]
		add edi,4
		mov al,8
		.cycle:
			bt ax,8 ;тестируем только ah
			jnc .c_next
				push eax ebx edx esi
				stdcall vox_corect_coords_pl, [v_obj],1
				stdcall create_sub_vox_obj_3d, [v_obj], [p_mem],[k_scale]
				pop esi edx ebx eax
			.c_next:
			shr ah,1
			dec al
			jnz .cycle
		;выход из узла
		inc ecx
		pop edx
		jmp .end_f
	.sub_trees:
		cmp ecx,0
		jl .end_0 ;не рисуем очень маленькие воксели

			;cmp ecx,1
			;jl @f
				;квадрат больше текущего масштаба
				;stdcall vox_draw_square_1g, [buf_i],[buf_z],eax
				;jmp .end_0
			;@@:
				;квадрат текущего масштаба
				push ecx
                mov eax,[p_mem]
                inc dword[eax] ;увеличиваем счетчик вокселей на 1
                mov eax,[eax]
                imul eax,vox_ogl_size ;умножаем на размер данных о вокселе
                add eax,4-vox_ogl_size ;число вокселей 4 байта
                add eax,[p_mem]
                mov [eax+vox_ogl_x0],ebx
                mov [eax+vox_ogl_y0],edx
                mov [eax+vox_ogl_z0],esi
                cmp ecx,1
                jl @f
                    ;квадрат больше текущего масштаба
                    shr dword[eax+vox_ogl_x0],cl
                    shr dword[eax+vox_ogl_y0],cl
                    shr dword[eax+vox_ogl_z0],cl
                @@:
                sub ecx,[k_scale]
                mov word[eax+vox_ogl_zoom],cx
                mov dword[eax+vox_ogl_planes],def_del_planes
				mov ecx,dword[edi]
				and ecx,0xffffff
                mov [eax+vox_ogl_color],ecx
				pop ecx
		.end_0:
		add edi,4
	.end_f:
	ret
endp

;description:
; в OpenGL требуются координаты float
; преобразование координат из int во float,
align 4
proc vox_obj_3d_recalc uses eax ebx ecx, p_mem:dword

;получаем координаты в пределах от 0 до 1 в дробном виде
	mov eax,[p_mem]
	mov ecx,dword[eax]
	add eax,4

	finit
align 4
	@@:
		fild word[eax+vox_ogl_zoom]
		fld1 ; размер кубической грани без учета масштаба
		fscale ; st0=1*2^st1 или размер кубической грани с учетом масштаба
		fxch

		;coord x
		fild dword[eax+vox_ogl_x0]
		fscale
		fst dword[eax+vox_ogl_x0]
		fadd st0,st2 ;добавляем ко второй точке размер масштабированной кубической грани
		fstp dword[eax+vox_ogl_x1]
		;coord y
		fild dword[eax+vox_ogl_y0]
		fscale
		fst dword[eax+vox_ogl_y0]
		fadd st0,st2
		fstp dword[eax+vox_ogl_y1]
		;coord z
		fild dword[eax+vox_ogl_z0]
		fscale
		fst dword[eax+vox_ogl_z0]
		fadd st0,st2
		fstp dword[eax+vox_ogl_z1]

		;goto next voxel
		add eax,vox_ogl_size
		ffree st0
		fincstp
		ffree st0
		fincstp
		loop @b

;отнимаем от каждой координаты по 0.5 для центровки объекта по центру экрана
	mov eax,[p_mem]
	mov ecx,dword[eax]
	add eax,4
	fld1 ;st0=1
	fchs ;st0=-1
	fld1 ;st0=1 st1=-1
	fscale ;st0=1*2^st1 или 1*2^-1=1/2=0.5
align 4
	@@:
		;coord x
		fld dword[eax+vox_ogl_x0]
		fsub st0,st1 ;-0.5
		fstp dword[eax+vox_ogl_x0]
		fld dword[eax+vox_ogl_x1]
		fsub st0,st1 ;-0.5
		fstp dword[eax+vox_ogl_x1]
		;coord y
		fld dword[eax+vox_ogl_y0]
		fsub st0,st1 ;-0.5
		fstp dword[eax+vox_ogl_y0]
		fld dword[eax+vox_ogl_y1]
		fsub st0,st1 ;-0.5
		fstp dword[eax+vox_ogl_y1]
		;coord z
		fld dword[eax+vox_ogl_z0]
		fsub st0,st1 ;-0.5
		fstp dword[eax+vox_ogl_z0]
		fld dword[eax+vox_ogl_z1]
		fsub st0,st1 ;-0.5
		fstp dword[eax+vox_ogl_z1]

		;goto next voxel
		add eax,vox_ogl_size
		loop @b
	ffree st0
	fincstp
	ffree st0
	fincstp
	ret
endp

;Сортировка вектора a[1..n] методом Флойда
align 4
proc pole_fl_sort, a:dword, n:dword
	pushad
	mov ecx,dword[a]
	;Формировать исходное частично упорядоченное дерево
	mov eax,dword[n]
	shr eax,1
	@@: ;for(i=n/2; i>=2; i--)
		stdcall pole_fl_surface, ecx,eax,[n] ;(a,i,n)
		dec eax
		cmp eax,2
		jge @b
	;Выполнить процедуру всплытия Флойда для каждого поддерева
	mov eax,dword[n]
	@@: ;for(i=n; i>=2; i--){
		stdcall pole_fl_surface, ecx,1,eax ;(a,1,i)
		;Поместить найденный максимальный элемент в конец списка
        stdcall swap_cell, ecx,1,eax ;меняем местами a[1] <-> a[i]
		dec eax
		cmp eax,2
		jge @b
	popad
	ret
endp

;Процедура всплытия Флойда по дереву a[1..k]
align 4
proc pole_fl_surface, a:dword, i:dword, k:dword
	pushad
	;edx -> ...
	;edi -> m
	;esi -> j
	mov eax,dword[a]
	mov ebx,dword[i]
	mov ecx,dword[k]

	stdcall copy_cell, eax, -1,ebx ;copy=a[i];
	mov edi,ebx
	shl edi,1 ;m=2*i где edi=m
	.cycle_b: ;while (m<=k) {
		cmp edi,ecx
		jg .cycle_e
		jne @f ;if (m==k) j=m;
			mov esi,edi
			jmp .else_e
		@@: ;else if (pole_compare_cells_bm(a[m],a[m+1])) j=m;
		mov edx,edi
        inc edx
		stdcall pole_compare_cells_bm, eax, edi,edx
		cmp dl,0
		je @f
			mov esi,edi
			jmp .else_e
		@@: ;else j=m+1;
			mov esi,edi
			inc esi
		.else_e:

		;if (pole_compare_cells_bm(a[j],copy)) {
		stdcall pole_compare_cells_bm, eax, esi,-1
		cmp dl,0
		je .cycle_e ;} else break; //выход из цикла

		stdcall copy_cell, eax, ebx,esi ;a[i]=a[j];
		mov ebx,esi ;i=j;
		mov edi,ebx
		shl edi,1 ;m=2*i;

		jmp .cycle_b
	.cycle_e:

	;значения многих регистров уже не важны т. к. конец функции
	stdcall copy_cell, eax, ebx,-1 ;a[i]=copy;

	popad
	ret
endp

;output:
; dl
align 4
proc pole_compare_cells_bm uses eax ebx ecx, p_mem:dword, i0:dword, i1:dword

	mov eax,[i0] ;eax -> cell[i0]
    imul eax,vox_ogl_size
    add eax,[p_mem]

	mov ebx,[i1] ;ebx -> cell[i1]
    cmp ebx,0
    jl .copy
    imul ebx,vox_ogl_size
    add ebx,[p_mem]
    jmp @f
    .copy:
        mov ebx,mem_copy_32
    @@:

	mov cx,word[ebx+vox_ogl_zoom] ;zoom
	cmp word[eax+vox_ogl_zoom],cx
	jle @f ;zoom0>zoom1
		mov dl,1
		jmp .fun_e
	@@:
	jne .r_0 ;if zoom0<zoom1 return 0

	mov ecx,dword[ebx+vox_ogl_x0] ;coord x
	cmp dword[eax+vox_ogl_x0],ecx
	jle @f ;x0>x1
		mov dl,1
		jmp .fun_e
	@@:
	jne .r_0 ;if x0<x1 return 0

	mov ecx,dword[ebx+vox_ogl_y0] ;coord y
	cmp dword[eax+vox_ogl_y0],ecx
	jle @f ;y0>y1
		mov dl,1
		jmp .fun_e
	@@:
	jne .r_0 ;if y0<y1 return 0

	mov ecx,dword[ebx+vox_ogl_z0] ;coord z
	cmp dword[eax+vox_ogl_z0],ecx
	jle @f ;z0>z1
		mov dl,1
		jmp .fun_e
	@@:

    .r_0:
	xor dl,dl
	.fun_e:
	ret
endp

if 0 ;пока не нужно, но работало правильно
;Сортировка вектора a[1..n] методом Флойда
align 4
proc pole_fl_sort_xzy, a:dword, n:dword
	pushad
	mov ecx,dword[a]
	;Формировать исходное частично упорядоченное дерево
	mov eax,dword[n]
	shr eax,1
	@@: ;for(i=n/2; i>=2; i--)
		stdcall pole_fl_surface_xzy, ecx,eax,[n] ;(a,i,n)
		dec eax
		cmp eax,2
		jge @b
	;Выполнить процедуру всплытия Флойда для каждого поддерева
	mov eax,dword[n]
	@@: ;for(i=n; i>=2; i--){
		stdcall pole_fl_surface_xzy, ecx,1,eax ;(a,1,i)
		;Поместить найденный максимальный элемент в конец списка
        stdcall swap_cell, ecx,1,eax ;меняем местами a[1] <-> a[i]
		dec eax
		cmp eax,2
		jge @b
	popad
	ret
endp

;Процедура всплытия Флойда по дереву a[1..k]
align 4
proc pole_fl_surface_xzy, a:dword, i:dword, k:dword
	pushad
	;edx -> ...
	;edi -> m
	;esi -> j
	mov eax,dword[a]
	mov ebx,dword[i]
	mov ecx,dword[k]

	stdcall copy_cell, eax, -1,ebx ;copy=a[i];
	mov edi,ebx
	shl edi,1 ;m=2*i где edi=m
	.cycle_b: ;while (m<=k) {
		cmp edi,ecx
		jg .cycle_e
		jne @f ;if (m==k) j=m;
			mov esi,edi
			jmp .else_e
		@@: ;else if (pole_compare_bm_xzy(a[m],a[m+1])) j=m;
		mov edx,edi
        inc edx
		stdcall pole_compare_bm_xzy, eax, edi,edx
		cmp dl,0
		je @f
			mov esi,edi
			jmp .else_e
		@@: ;else j=m+1;
			mov esi,edi
			inc esi
		.else_e:

		;if (pole_compare_bm_xzy(a[j],copy)) {
		stdcall pole_compare_bm_xzy, eax, esi,-1
		cmp dl,0
		je .cycle_e ;} else break; //выход из цикла

		stdcall copy_cell, eax, ebx,esi ;a[i]=a[j];
		mov ebx,esi ;i=j;
		mov edi,ebx
		shl edi,1 ;m=2*i;

		jmp .cycle_b
	.cycle_e:

	;значения многих регистров уже не важны т. к. конец функции
	stdcall copy_cell, eax, ebx,-1 ;a[i]=copy;

	popad
	ret
endp

;output:
; dl
align 4
proc pole_compare_bm_xzy uses eax ebx ecx, p_mem:dword, i0:dword, i1:dword

	mov eax,[i0] ;eax -> cell[i0]
    imul eax,vox_ogl_size
    add eax,[p_mem]

	mov ebx,[i1] ;ebx -> cell[i1]
    cmp ebx,0
    jl .copy
    imul ebx,vox_ogl_size
    add ebx,[p_mem]
    jmp @f
    .copy:
        mov ebx,mem_copy_32
    @@:

	mov cx,word[ebx+vox_ogl_zoom] ;zoom
	cmp word[eax+vox_ogl_zoom],cx
	jle @f ;zoom0>zoom1
		mov dl,1
		jmp .fun_e
	@@:
	jne .r_0 ;if zoom0<zoom1 return 0

	mov ecx,dword[ebx+vox_ogl_x0] ;coord x
	cmp dword[eax+vox_ogl_x0],ecx
	jle @f ;x0>x1
		mov dl,1
		jmp .fun_e
	@@:
	jne .r_0 ;if x0<x1 return 0

	mov ecx,dword[ebx+vox_ogl_z0] ;coord z
	cmp dword[eax+vox_ogl_z0],ecx
	jle @f ;z0>z1
		mov dl,1
		jmp .fun_e
	@@:
	jne .r_0 ;if z0<z1 return 0

	mov ecx,dword[ebx+vox_ogl_y0] ;coord y
	cmp dword[eax+vox_ogl_y0],ecx
	jle @f ;y0>y1
		mov dl,1
		jmp .fun_e
	@@:

    .r_0:
	xor dl,dl
	.fun_e:
	ret
endp
end if

;Сортировка вектора a[1..n] методом Флойда
align 4
proc pole_fl_sort_zxy, a:dword, n:dword
	pushad
	mov ecx,dword[a]
	;Формировать исходное частично упорядоченное дерево
	mov eax,dword[n]
	shr eax,1
	@@: ;for(i=n/2; i>=2; i--)
		stdcall pole_fl_surface_zxy, ecx,eax,[n] ;(a,i,n)
		dec eax
		cmp eax,2
		jge @b
	;Выполнить процедуру всплытия Флойда для каждого поддерева
	mov eax,dword[n]
	@@: ;for(i=n; i>=2; i--){
		stdcall pole_fl_surface_zxy, ecx,1,eax ;(a,1,i)
		;Поместить найденный максимальный элемент в конец списка
        stdcall swap_cell, ecx,1,eax ;меняем местами a[1] <-> a[i]
		dec eax
		cmp eax,2
		jge @b
	popad
	ret
endp

;Процедура всплытия Флойда по дереву a[1..k]
align 4
proc pole_fl_surface_zxy, a:dword, i:dword, k:dword
	pushad
	;edx -> ...
	;edi -> m
	;esi -> j
	mov eax,dword[a]
	mov ebx,dword[i]
	mov ecx,dword[k]

	stdcall copy_cell, eax, -1,ebx ;copy=a[i];
	mov edi,ebx
	shl edi,1 ;m=2*i где edi=m
	.cycle_b: ;while (m<=k) {
		cmp edi,ecx
		jg .cycle_e
		jne @f ;if (m==k) j=m;
			mov esi,edi
			jmp .else_e
		@@: ;else if (pole_compare_bm_xzy(a[m],a[m+1])) j=m;
		mov edx,edi
        inc edx
		stdcall pole_compare_bm_zxy, eax, edi,edx
		cmp dl,0
		je @f
			mov esi,edi
			jmp .else_e
		@@: ;else j=m+1;
			mov esi,edi
			inc esi
		.else_e:

		;if (pole_compare_bm_xzy(a[j],copy)) {
		stdcall pole_compare_bm_zxy, eax, esi,-1
		cmp dl,0
		je .cycle_e ;} else break; //выход из цикла

		stdcall copy_cell, eax, ebx,esi ;a[i]=a[j];
		mov ebx,esi ;i=j;
		mov edi,ebx
		shl edi,1 ;m=2*i;

		jmp .cycle_b
	.cycle_e:

	;значения многих регистров уже не важны т. к. конец функции
	stdcall copy_cell, eax, ebx,-1 ;a[i]=copy;

	popad
	ret
endp

;output:
; dl
align 4
proc pole_compare_bm_zxy uses eax ebx ecx, p_mem:dword, i0:dword, i1:dword

	mov eax,[i0] ;eax -> cell[i0]
    imul eax,vox_ogl_size
    add eax,[p_mem]

	mov ebx,[i1] ;ebx -> cell[i1]
    cmp ebx,0
    jl .copy
    imul ebx,vox_ogl_size
    add ebx,[p_mem]
    jmp @f
    .copy:
        mov ebx,mem_copy_32
    @@:

	mov cx,word[ebx+vox_ogl_zoom] ;zoom
	cmp word[eax+vox_ogl_zoom],cx
	jle @f ;zoom0>zoom1
		mov dl,1
		jmp .fun_e
	@@:
	jne .r_0 ;if zoom0<zoom1 return 0

	mov ecx,dword[ebx+vox_ogl_z0] ;coord z
	cmp dword[eax+vox_ogl_z0],ecx
	jle @f ;z0>z1
		mov dl,1
		jmp .fun_e
	@@:
	jne .r_0 ;if z0<z1 return 0

	mov ecx,dword[ebx+vox_ogl_x0] ;coord x
	cmp dword[eax+vox_ogl_x0],ecx
	jle @f ;x0>x1
		mov dl,1
		jmp .fun_e
	@@:
	jne .r_0 ;if x0<x1 return 0

	mov ecx,dword[ebx+vox_ogl_y0] ;coord y
	cmp dword[eax+vox_ogl_y0],ecx
	jle @f ;y0>y1
		mov dl,1
		jmp .fun_e
	@@:

    .r_0:
	xor dl,dl
	.fun_e:
	ret
endp

;Сортировка вектора a[1..n] методом Флойда
align 4
proc pole_fl_sort_yzx, a:dword, n:dword
	pushad
	mov ecx,dword[a]
	;Формировать исходное частично упорядоченное дерево
	mov eax,dword[n]
	shr eax,1
	@@: ;for(i=n/2; i>=2; i--)
		stdcall pole_fl_surface_yzx, ecx,eax,[n] ;(a,i,n)
		dec eax
		cmp eax,2
		jge @b
	;Выполнить процедуру всплытия Флойда для каждого поддерева
	mov eax,dword[n]
	@@: ;for(i=n; i>=2; i--){
		stdcall pole_fl_surface_yzx, ecx,1,eax ;(a,1,i)
		;Поместить найденный максимальный элемент в конец списка
        stdcall swap_cell, ecx,1,eax ;меняем местами a[1] <-> a[i]
		dec eax
		cmp eax,2
		jge @b
	popad
	ret
endp

;Процедура всплытия Флойда по дереву a[1..k]
align 4
proc pole_fl_surface_yzx, a:dword, i:dword, k:dword
	pushad
	;edx -> ...
	;edi -> m
	;esi -> j
	mov eax,dword[a]
	mov ebx,dword[i]
	mov ecx,dword[k]

	stdcall copy_cell, eax, -1,ebx ;copy=a[i];
	mov edi,ebx
	shl edi,1 ;m=2*i где edi=m
	.cycle_b: ;while (m<=k) {
		cmp edi,ecx
		jg .cycle_e
		jne @f ;if (m==k) j=m;
			mov esi,edi
			jmp .else_e
		@@: ;else if (pole_compare_bm_yzx(a[m],a[m+1])) j=m;
		mov edx,edi
        inc edx
		stdcall pole_compare_bm_yzx, eax, edi,edx
		cmp dl,0
		je @f
			mov esi,edi
			jmp .else_e
		@@: ;else j=m+1;
			mov esi,edi
			inc esi
		.else_e:

		;if (pole_compare_bm_yzx(a[j],copy)) {
		stdcall pole_compare_bm_yzx, eax, esi,-1
		cmp dl,0
		je .cycle_e ;} else break; //выход из цикла

		stdcall copy_cell, eax, ebx,esi ;a[i]=a[j];
		mov ebx,esi ;i=j;
		mov edi,ebx
		shl edi,1 ;m=2*i;

		jmp .cycle_b
	.cycle_e:

	;значения многих регистров уже не важны т. к. конец функции
	stdcall copy_cell, eax, ebx,-1 ;a[i]=copy;

	popad
	ret
endp

;output:
; dl
align 4
proc pole_compare_bm_yzx uses eax ebx ecx, p_mem:dword, i0:dword, i1:dword

	mov eax,[i0] ;eax -> cell[i0]
    imul eax,vox_ogl_size
    add eax,[p_mem]

	mov ebx,[i1] ;ebx -> cell[i1]
    cmp ebx,0
    jl .copy
    imul ebx,vox_ogl_size
    add ebx,[p_mem]
    jmp @f
    .copy:
        mov ebx,mem_copy_32
    @@:

	mov cx,word[ebx+vox_ogl_zoom] ;zoom
	cmp word[eax+vox_ogl_zoom],cx
	jle @f ;zoom0>zoom1
		mov dl,1
		jmp .fun_e
	@@:
	jne .r_0 ;if zoom0<zoom1 return 0

	mov ecx,dword[ebx+vox_ogl_y0] ;coord y
	cmp dword[eax+vox_ogl_y0],ecx
	jle @f ;y0>y1
		mov dl,1
		jmp .fun_e
	@@:
	jne .r_0 ;if y0<y1 return 0

	mov ecx,dword[ebx+vox_ogl_z0] ;coord z
	cmp dword[eax+vox_ogl_z0],ecx
	jle @f ;z0>z1
		mov dl,1
		jmp .fun_e
	@@:
	jne .r_0 ;if z0<z1 return 0

	mov ecx,dword[ebx+vox_ogl_x0] ;coord x
	cmp dword[eax+vox_ogl_x0],ecx
	jle @f ;x0>x1
		mov dl,1
		jmp .fun_e
	@@:

    .r_0:
	xor dl,dl
	.fun_e:
	ret
endp

align 4
proc copy_cell uses ecx edi esi, p_mem:dword, i0:dword, i1:dword

    mov esi,[i1]
    cmp esi,0
    jl .copy_0
    imul esi,vox_ogl_size
    add esi,[p_mem]
    jmp @f
    .copy_0:
        mov esi,mem_copy_32
    @@:

    mov edi,[i0]
    cmp edi,0
    jl .copy_1
    imul edi,vox_ogl_size
    add edi,[p_mem]
    jmp @f
    .copy_1:
        mov edi,mem_copy_32
    @@:

if vox_ogl_size eq 34
    mov ecx,8 ;ecx=32/4
    cld
    rep movsd
    movsw
else
    vox_ogl_size не равно 34 !
end if
    ret
endp

align 4
proc swap_cell uses eax ebx ecx edi esi, p_mem:dword, i0:dword, i1:dword
    mov esi,[i0]
    imul esi,vox_ogl_size
    add esi,[p_mem]
    mov edi,[i1]
    imul edi,vox_ogl_size
    add edi,[p_mem]
if vox_ogl_size eq 34
    ;vox_ogl_size = 34 = 32 + 2
    mov ecx,8 ;ecx=32/4
    cld
    @@:
        mov eax,dword[edi]
        mov ebx,dword[esi]
        mov dword[edi],ebx
        mov dword[esi],eax
        add esi,4
        add edi,4
        loop @b
    mov ax,word[edi]
    mov bx,word[esi]
    mov word[edi],bx
    mov word[esi],ax
else
    vox_ogl_size не равно 34 !
end if
    ret
endp

align 4
mem_copy_32 rb vox_ogl_size

;функция для коректировки координат
;направления осей координат в вокселе:
;*z
;|
;+-* x
;input:
;  al - номер узла в дереве (от 1 до 8)
; ebx - координата x
; edx - координата y
; esi - координата z
; ecx - уровень текушего узла
;output:
; ebx - новая координата x
; edx - новая координата y
; esi - новая координата z
align 4
proc vox_corect_coords_pl, v_obj:dword, v_size:dword
	cmp ecx,0
	jl .end_f ;для ускорения отрисовки

	push eax edi
	and eax,15 ;выделяем номер узла в дереве
	mov edi,[v_obj]
	add edi,vox_offs_tree_table
	add edi,8
	sub edi,eax

	mov eax,[v_size]
	cmp ecx,1
	jl @f
		shl eax,cl
	@@:

	bt word[edi],0 ;test voxel coord x
	jnc @f
		add ebx,eax
	@@:
	bt word[edi],2 ;test voxel coord z
	jnc @f
		sub edx,eax
	@@:
	bt word[edi],1 ;test voxel coord y
	jc @f
		mov eax,1
		cmp ecx,1
		jl .end_0
			shl eax,cl
		.end_0:
		add esi,eax ;меняем глубину для буфера z
	@@:
	pop edi eax
	.end_f:
	ret
endp

align 4
p1 dd ?
p2 dd ?

align 4
proc draw_voxels_3d uses ebx ecx edx edi, p_mem:dword
locals
    v_count dd ?
endl
    mov edi,[p_mem]
    cmp edi,0
    je .end_f
    mov eax,dword[edi]
    mov dword[v_count],eax
    add edi,4

bt word[opt_cube_box],0
jnc @f
;рисование рамки
stdcall [glColor3ub],128,128,128
stdcall [glBegin],GL_LINE_STRIP
    stdcall [glVertex3f], -0.5, -0.5, -0.5
    stdcall [glVertex3f], -0.5,  0.5, -0.5
    stdcall [glVertex3f],  0.5,  0.5, -0.5
    stdcall [glVertex3f],  0.5, -0.5, -0.5
    stdcall [glVertex3f], -0.5, -0.5, -0.5
    stdcall [glVertex3f], -0.5, -0.5,  0.5
    stdcall [glVertex3f], -0.5,  0.5,  0.5
    stdcall [glVertex3f],  0.5,  0.5,  0.5
    stdcall [glVertex3f],  0.5, -0.5,  0.5
    stdcall [glVertex3f], -0.5, -0.5,  0.5
stdcall [glEnd]
stdcall [glBegin],GL_LINES
    stdcall [glVertex3f], -0.5,  0.5, -0.5
    stdcall [glVertex3f], -0.5,  0.5,  0.5
    stdcall [glVertex3f],  0.5,  0.5, -0.5
    stdcall [glVertex3f],  0.5,  0.5,  0.5
    stdcall [glVertex3f],  0.5, -0.5, -0.5
    stdcall [glVertex3f],  0.5, -0.5,  0.5
stdcall [glEnd]
@@:

;рисование объекта
    stdcall [glBegin],GL_QUADS
    .cycle_0:
        cmp dword[v_count],1
        jl .cycle_1
        stdcall [glColor3ub],[edi+vox_ogl_color+2],[edi+vox_ogl_color+1],[edi+vox_ogl_color]

        bt word[edi+vox_ogl_planes],vox_ogl_gran_z1
        jnc .e_gran_z1
        ;передняя грань Normal(0.0,0.0,normal_gran_z1)
            normal_gran p1,x0
            normal_gran_2 p1,x0z1,x1
            normal_gran p2,y0
            normal_gran_2 p2,y0z1,y1
            stdcall [glNormal3f], [p1],[p2],normal_gran_z1
        stdcall [glVertex3f],[edi+vox_ogl_x0],[edi+vox_ogl_y0],[edi+vox_ogl_z1]
            normal_gran p1,x1
            normal_gran_2 p1,x1z1,x0
            ;normal_gran p2,y0
            ;normal_gran_2 p2,y0z1,y1
            stdcall [glNormal3f], [p1],[p2],normal_gran_z1
        stdcall [glVertex3f],[edi+vox_ogl_x1],[edi+vox_ogl_y0],[edi+vox_ogl_z1]
            ;normal_gran p1,x1
            ;normal_gran_2 p1,x1z1,x0
            normal_gran p2,y1
            normal_gran_2 p2,y1z1,y0
            stdcall [glNormal3f], [p1],[p2],normal_gran_z1
        stdcall [glVertex3f],[edi+vox_ogl_x1],[edi+vox_ogl_y1],[edi+vox_ogl_z1]
            normal_gran p1,x0
            normal_gran_2 p1,x0z1,x1
            ;normal_gran p2,y1
            ;normal_gran_2 p2,y1z1,y0            
            stdcall [glNormal3f], [p1],[p2],normal_gran_z1
        stdcall [glVertex3f],[edi+vox_ogl_x0],[edi+vox_ogl_y1],[edi+vox_ogl_z1]
        .e_gran_z1:

        bt word[edi+vox_ogl_planes],vox_ogl_gran_y0
        jnc .e_gran_y0
        ;верхняя грань Normal(0.0,normal_gran_y0,0.0)
            normal_gran p1,x0
            normal_gran_2 p1,x0y0,x1
            normal_gran p2,z0
            normal_gran_2 p2,y0z0,z1
            stdcall [glNormal3f], [p1],normal_gran_y0,[p2]
        stdcall [glVertex3f],[edi+vox_ogl_x0],[edi+vox_ogl_y0],[edi+vox_ogl_z0]
            normal_gran p1,x1
            normal_gran_2 p1,x1y0,x0
            ;normal_gran p2,z0
            ;normal_gran_2 p2,y0z0,z1
            stdcall [glNormal3f], [p1],normal_gran_y0,[p2]
        stdcall [glVertex3f],[edi+vox_ogl_x1],[edi+vox_ogl_y0],[edi+vox_ogl_z0]
            ;normal_gran p1,x1
            ;normal_gran_2 p1,x1y0,x0
            normal_gran p2,z1
            normal_gran_2 p2,y0z1,z0
            stdcall [glNormal3f], [p1],normal_gran_y0,[p2]
        stdcall [glVertex3f],[edi+vox_ogl_x1],[edi+vox_ogl_y0],[edi+vox_ogl_z1]
            normal_gran p1,x0
            normal_gran_2 p1,x0y0,x1
            ;normal_gran p2,z1
            ;normal_gran_2 p2,y0z1,z0
            stdcall [glNormal3f], [p1],normal_gran_y0,[p2]
        stdcall [glVertex3f],[edi+vox_ogl_x0],[edi+vox_ogl_y0],[edi+vox_ogl_z1]
        .e_gran_y0:

        bt word[edi+vox_ogl_planes],vox_ogl_gran_y1
        jnc .e_gran_y1
        ;нижняя грань Normal(0.0,normal_gran_y1,0.0)
            normal_gran p1,x0
            normal_gran_2 p1,x0y1,x1
            normal_gran p2,z1
            normal_gran_2 p2,y1z1,z0
            stdcall [glNormal3f], [p1],normal_gran_y1,[p2]
        stdcall [glVertex3f],[edi+vox_ogl_x0],[edi+vox_ogl_y1],[edi+vox_ogl_z1]
            normal_gran p1,x1
            normal_gran_2 p1,x1y1,x0
            ;normal_gran p2,z1
            ;normal_gran_2 p2,y1z1,z0
            stdcall [glNormal3f], [p1],normal_gran_y1,[p2]
        stdcall [glVertex3f],[edi+vox_ogl_x1],[edi+vox_ogl_y1],[edi+vox_ogl_z1]
            ;normal_gran p1,x1
            ;normal_gran_2 p1,x1y1,x0
            normal_gran p2,z0
            normal_gran_2 p2,y1z0,z1
            stdcall [glNormal3f], [p1],normal_gran_y1,[p2]
        stdcall [glVertex3f],[edi+vox_ogl_x1],[edi+vox_ogl_y1],[edi+vox_ogl_z0]
            normal_gran p1,x0
            normal_gran_2 p1,x0y1,x1
            ;normal_gran p2,z0
            ;normal_gran_2 p2,y1z0,z1
            stdcall [glNormal3f], [p1],normal_gran_y1,[p2]
        stdcall [glVertex3f],[edi+vox_ogl_x0],[edi+vox_ogl_y1],[edi+vox_ogl_z0]
        .e_gran_y1:

        bt word[edi+vox_ogl_planes],vox_ogl_gran_x0
        jnc .e_gran_x0
        ;левая грань Normal(normal_gran_x0,0.0,0.0)
            normal_gran p1,y0
            normal_gran_2 p1,x0y0,y1
            normal_gran p2,z1
            normal_gran_2 p2,x0z1,z0
            stdcall [glNormal3f], normal_gran_x0,[p1],[p2]
        stdcall [glVertex3f],[edi+vox_ogl_x0],[edi+vox_ogl_y0],[edi+vox_ogl_z1]
            normal_gran p1,y1
            normal_gran_2 p1,x0y1,y0
            ;normal_gran p2,z1
            ;normal_gran_2 p2,x0z1,z0
            stdcall [glNormal3f], normal_gran_x0,[p1],[p2]
        stdcall [glVertex3f],[edi+vox_ogl_x0],[edi+vox_ogl_y1],[edi+vox_ogl_z1]
            ;normal_gran p1,y1
            ;normal_gran_2 p1,x0y1,y0
            normal_gran p2,z0
            normal_gran_2 p2,x0z0,z1
            stdcall [glNormal3f], normal_gran_x0,[p1],[p2]
        stdcall [glVertex3f],[edi+vox_ogl_x0],[edi+vox_ogl_y1],[edi+vox_ogl_z0]
            normal_gran p1,y0
            normal_gran_2 p1,x0y0,y1
            ;normal_gran p2,z0
            ;normal_gran_2 p2,x0z0,z1
            stdcall [glNormal3f], normal_gran_x0,[p1],[p2]
        stdcall [glVertex3f],[edi+vox_ogl_x0],[edi+vox_ogl_y0],[edi+vox_ogl_z0]
        .e_gran_x0:

        bt word[edi+vox_ogl_planes],vox_ogl_gran_x1
        jnc .e_gran_x1
        ;правая грань Normal(normal_gran_x1,0.0,0.0)
            normal_gran p1,y0
            normal_gran_2 p1,x1y0,y1
            normal_gran p2,z1
            normal_gran_2 p2,x1z1,z0
            stdcall [glNormal3f], normal_gran_x1,[p1],[p2]
        stdcall [glVertex3f],[edi+vox_ogl_x1],[edi+vox_ogl_y0],[edi+vox_ogl_z1]
            ;normal_gran p1,y0
            ;normal_gran_2 p1,x1y0,y1
            normal_gran p2,z0
            normal_gran_2 p2,x1z0,z1
            stdcall [glNormal3f], normal_gran_x1,[p1],[p2]
        stdcall [glVertex3f],[edi+vox_ogl_x1],[edi+vox_ogl_y0],[edi+vox_ogl_z0]
            normal_gran p1,y1
            normal_gran_2 p1,x1y1,y0
            ;normal_gran p2,z0
            ;normal_gran_2 p2,x1z0,z1
            stdcall [glNormal3f], normal_gran_x1,[p1],[p2]
        stdcall [glVertex3f],[edi+vox_ogl_x1],[edi+vox_ogl_y1],[edi+vox_ogl_z0]
            ;normal_gran p1,y1
            ;normal_gran_2 p1,x1y1,y0
            normal_gran p2,z1
            normal_gran_2 p2,x1z1,z0
            stdcall [glNormal3f], normal_gran_x1,[p1],[p2]
        stdcall [glVertex3f],[edi+vox_ogl_x1],[edi+vox_ogl_y1],[edi+vox_ogl_z1]
        .e_gran_x1:

        bt word[edi+vox_ogl_planes],vox_ogl_gran_z0
        jnc .e_gran_z0
        ;задняя грань Normal(0.0,0.0,normal_gran_z0)
            normal_gran p1,x0
            normal_gran_2 p1,x0z0,x1
            normal_gran p2,y1
            normal_gran_2 p2,y1z0,y0
            stdcall [glNormal3f], [p1],[p2],normal_gran_z0
        stdcall [glVertex3f],[edi+vox_ogl_x0],[edi+vox_ogl_y1],[edi+vox_ogl_z0]
            normal_gran p1,x1
            normal_gran_2 p1,x1z0,x0
            ;normal_gran p2,y1
            ;normal_gran_2 p2,y1z0,y0
            stdcall [glNormal3f], [p1],[p2],normal_gran_z0
        stdcall [glVertex3f],[edi+vox_ogl_x1],[edi+vox_ogl_y1],[edi+vox_ogl_z0]
            ;normal_gran p1,x1
            ;normal_gran_2 p1,x1z0,x0
            normal_gran p2,y0
            normal_gran_2 p2,y0z0,y1
            stdcall [glNormal3f], [p1],[p2],normal_gran_z0
        stdcall [glVertex3f],[edi+vox_ogl_x1],[edi+vox_ogl_y0],[edi+vox_ogl_z0]
            normal_gran p1,x0
            normal_gran_2 p1,x0z0,x1
            ;normal_gran p2,y0
            ;normal_gran_2 p2,y0z0,y1
            stdcall [glNormal3f], [p1],[p2],normal_gran_z0
        stdcall [glVertex3f],[edi+vox_ogl_x0],[edi+vox_ogl_y0],[edi+vox_ogl_z0]
        .e_gran_z0:

        ;goto next voxel
        add edi,vox_ogl_size
        dec dword[v_count]
        jmp .cycle_0
    .cycle_1:

    stdcall [glEnd]

    .end_f:
    ret
endp

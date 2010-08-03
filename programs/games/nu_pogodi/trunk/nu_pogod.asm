use32
	org 0x0
	db 'MENUET01' ;идентиф. исполняемого файла всегда 8 байт
	dd 0x1
	dd start
	dd i_end ;размер приложения
	dd mem
	dd stacktop
	dd 0x0
	dd sys_path

include '../../../macros.inc'
include '../../../proc32.inc'
include '../../../develop/libraries/box_lib/load_lib.mac'
include 'mem.inc'
include 'dll.inc'

@use_library_mem mem.Alloc,mem.Free,mem.ReAlloc, dll.Load

;флаги, для функции обрезания буфера
BUF2D_OPT_CROP_TOP equ 1 ;обрезка сверху
BUF2D_OPT_CROP_LEFT equ 2 ;обрезка слева
BUF2D_OPT_CROP_BOTTOM equ 4 ;обрезка снизу
BUF2D_OPT_CROP_RIGHT equ 8 ;обрезка справа
BUF2D_BIT_OPT_CROP_TOP equ 0
BUF2D_BIT_OPT_CROP_LEFT equ 1
BUF2D_BIT_OPT_CROP_BOTTOM equ 2
BUF2D_BIT_OPT_CROP_RIGHT equ 3

BUF_STRUCT_SIZE equ 21
buf2d_data equ dword[edi] ;данные буфера изображения
buf2d_w equ dword[edi+8] ;ширина буфера
buf2d_h equ dword[edi+12] ;высота буфера
buf2d_l equ word[edi+4] ;отступ слева
buf2d_t equ word[edi+6] ;отступ сверху
buf2d_size_lt equ dword[edi+4] ;отступ слева и справа для буфера
buf2d_color equ dword[edi+16] ;цвет фона буфера
buf2d_bits equ byte[edi+20] ;количество бит в 1-й точке изображения

struct FileInfoBlock
	Function dd ?
	Position dd ?
	Flags	 dd ?
	Count	 dd ?
	Buffer	 dd ?
		db ?
	FileName dd ?
ends

displ_w dd ? ;ширина поля
displ_h dd ? ;высота поля
displ_bytes dd ? ;размер 1-го файла с изображением
;displ_bytes equ 315*210*3 ;размер 1-го файла с изображением

OFFS_SHADOW_X equ 2 ;сдвиг теней по оси 'x'
OFFS_SHADOW_Y equ 2 ;сдвиг теней по оси 'y'
IMAGE_FONT_SIZE equ 128*144*3

use_but equ 1

if use_but eq 1
BUT1_T equ 10 ;отступ сверху
BUT1_L equ 15 ;отступ слева
BUT1_W equ 50 ;ширина
BUT1_H equ 20 ;высота
BUT1_NEXT_TOP equ (BUT1_T+BUT1_H)*65536
end if

fn_icon0 db 'curici.png',0 ;имя файла с декорациями
fn_icon1 db 'wolf.png',0 ;имя файла с волком и зайцем
fn_icon2 db 'eggs.png',0 ;имя файла с яйцами
fn_icon3 db 'chi.png',0 ;имя файла с циплятами
fn_font db 'font8x9.bmp',0

ini_name db 'nu_pogod.ini',0
ini_sec_files db 'Files',0
key_displ_w db 'displ_w',0
key_displ_h db 'displ_h',0
ini_sec_color db 'Colors',0
key_color_unit db 'unit',0

;цвета в игре
color_fon dd 0xffffff
color_shadows dd 0xd0d0d0 ;цвет теней
color_trees dd 0x008000 ;цвет травы
color_wolf dd 0x800000 ;цвет волка и зайца
color_egg dd 0x404080 ;цвет яйца
color_chick dd 0x00d0d0 ;цвет ципленка
color_curici dd 0x8080d0 ;цвет курицы
color_perilo dd 0x000080 ;цвет перила (гребня)
;цвета интерфейса
color_but_sm dd 0x808080 ;цвет маленьких кнопок
color_but_te dd 0xffffff ;цвет текста на кнопках

macro load_image_file path,buf,size { ;макрос для загрузки изображений
	copy_path path,sys_path,file_name,0x0 ;формируем полный путь к файлу изображения, подразумеваем что он в одной папке с программой

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

;данные игры
bit_zaac equ 2 ;бит зайца
val_zaac equ 4 ;цифра отвечающая за бит зайца
bit_mig equ 3 ;бит мигания
val_mig equ 8 ;цифра отвечающая за бит мигания
mask_lot_lu    equ 1b ;маска для левого верхнего лотка
mask_lot_ld    equ 100000b ;маска для левого нижнего лотка
mask_lot_ru    equ 10000000000b ;маска для правого верхнего лотка
mask_lot_rd    equ 1000000000000000b ;маска для правого нижнего лотка
mask_clear_all equ 11111011111011110111101111011110b ;маска для очистки падающих яиц и прибежавших циплят
mask_fail_eggs equ 100001000010000100000b ;маска падающих яиц
mask_chi_left  equ 11111000000000000000000000b ;маска левых циплят
mask_chi_right equ 11111000000000000000000000000000b ;маска правых циплят
mask_chi_cr_l  equ 1000000000000000000000b ;маска для создания левого
mask_chi_cr_r  equ 1000000000000000000000000000b ;маска для создания правого
bit_chi_left  equ 21 ;1-й бит который отвечает за бегущего слева
bit_chi_right equ 27 ;1-й бит который отвечает за бегущего права
val_zaac_time_y equ 5 ;колличество тактов, которое обязательно должен провисеть заяц
val_zaac_time_n equ 7 ;колличество тактов, которое обязательно должен быть спрятанным заяц

txt_game_a db 'Игра А',0
txt_game_b db 'Игра Б',0

zaac_status db 0
pos_wolf db 0 ;позиция волка 0-й бит слева/справа, 1-й бит сверху/вниз
;rb 1
pos_eggs dd 0 ;позиции расположения яиц и циплят
eggs_count dw 0 ;колличество пойманых яиц
game_text db 'Игра _',13
some_text db '0'
	rb 8 ;текст с числом пойманных яиц
count_last db 0 ;счетчик пропущенных яиц
game_spd dd 0 ;задержка игры

;для генерации случайных чисел
rand_x dd 0

align 4
rand_next:
;x(k+1) = (a*x(k)+c) mod m
; a=22695477, c=1, m=2^32
push eax
	mov eax,dword[rand_x]
	imul eax,22695477
	inc eax
	mov dword[rand_x],eax
pop eax
	ret

;создаем прозрачные буфера по 8 бит (трафареты), для рисования
;где buf - буфер на основе которого будет создан трафарет
;фоновый цвет буфера должен быть 0xffffff, иначе не произойдет обрезка
;по фоновому цвету и трафарет будет занимат ьмного места в памяти
align 4
proc CreateTrapharetBuffer, buf:dword, img_data:dword
	push eax edi
	mov edi,dword[buf]

	;заполнение данных буфера
	mov buf2d_size_lt,0
	mov eax,dword[displ_w]
	mov buf2d_w,eax
	mov eax,dword[displ_h]
	mov buf2d_h,eax
	mov buf2d_color,0xffffff
	mov buf2d_bits,24

	stdcall [buf2d_create_f_img], edi,[img_data] ;создаем буфер
	stdcall [buf2d_conv_24_to_8], edi,1 ;делаем буфер прозрачности 8бит
	;обрезаем лишние края буфера, для более быстрого рисования
	stdcall [buf2d_crop_color], edi,buf2d_color,BUF2D_OPT_CROP_TOP+BUF2D_OPT_CROP_BOTTOM+BUF2D_OPT_CROP_RIGHT+BUF2D_OPT_CROP_LEFT
	pop edi eax
	ret
endp

align 4
InitBackgroundBuffer: ;создание фонового изображения
	pushad
	mov edi,buf_fon
	mov eax,[color_fon]
	mov buf2d_color,eax
	stdcall [buf2d_clear], edi,eax
	mov esi,edi

	xor eax,eax
	xor ebx,ebx

	mov edi,buf_decor
	mov ax,buf2d_t
	add eax,OFFS_SHADOW_Y
	mov bx,buf2d_l
	add ebx,OFFS_SHADOW_X
	stdcall [buf2d_bit_blt_alpha], esi, ebx,eax, edi,[color_shadows] ;рисуем тени домиков
	add edi,BUF_STRUCT_SIZE
	mov ax,buf2d_t
	add eax,OFFS_SHADOW_Y
	mov bx,buf2d_l
	add ebx,OFFS_SHADOW_X
	stdcall [buf2d_bit_blt_alpha], esi, ebx,eax, edi,[color_shadows] ;рисуем тени куриц
	add edi,BUF_STRUCT_SIZE
	mov ax,buf2d_t
	add eax,OFFS_SHADOW_Y
	mov bx,buf2d_l
	add ebx,OFFS_SHADOW_X
	stdcall [buf2d_bit_blt_alpha], esi, ebx,eax, edi,[color_shadows] ;рисуем тени деревьев

	mov edi,buf_decor
	mov ax,buf2d_t
	stdcall [buf2d_bit_blt_alpha], esi, 0,eax, edi,[color_perilo] ;рисуем домики
	add edi,BUF_STRUCT_SIZE
	mov ax,buf2d_t
	stdcall [buf2d_bit_blt_alpha], esi, 0,eax, edi,[color_curici] ;рисуем курицы
	add edi,BUF_STRUCT_SIZE
	mov ax,buf2d_t
	stdcall [buf2d_bit_blt_alpha], esi, 0,eax, edi,[color_trees] ;рисуем деревья
	popad
	ret

;рисует картинку в буфере buf из массива буферов buf_img с индексом ind
;т. е. buf <- buf_img[ind]
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
DrawZaac: ;рисование зайца
	push eax
	bt word[pos_wolf],bit_zaac
	jae .end_mig

	stdcall DrawArrayImg, buf_displ,buf_wolf,6,[color_wolf] ;рисуем тело зайца
	mov eax,dword[pos_eggs]
	and eax,mask_chi_right+mask_chi_left
	cmp eax,0
	je .end_mig ;если нет бегающих циплят, то заяц рукой не машет
	bt word[pos_wolf],bit_mig
	jc @f
		stdcall DrawArrayImg, buf_displ,buf_wolf,7,[color_wolf] ;рисуем руку зайца вверху
		jmp .end_mig
	@@:
		stdcall DrawArrayImg, buf_displ,buf_wolf,8,[color_wolf] ;рисуем руку зайца внизу
	.end_mig:
	pop eax
	ret

align 4
DrawWolf: ;рисует волка с корзинами
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
DrawEggs: ;рисует яйца
	pushad
	cld

	mov eax,dword[pos_eggs]
	xor ebx,ebx
	mov ecx,20 ;цикл для рисования котящихся яиц
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
		stdcall DrawArrayImg, buf_displ,buf_egg,20,[color_egg] ;разбитое яйцо слева
	@@:
	mov eax,dword[pos_eggs]
	and eax,mask_chi_right
	cmp eax,0
	je @f
		stdcall DrawArrayImg, buf_displ,buf_egg,21,[color_egg] ;разбитое яйцо справа
	@@:

	mov eax,dword[pos_eggs]
	xor edx,edx
	mov ebx,bit_chi_left
	mov ecx,5 ;цикл для рисования левых циплят
	@@:
		bt eax,ebx
		jae .draw_chick_l
			stdcall DrawArrayImg, buf_displ,buf_chi,edx,[color_chick]
		.draw_chick_l:
		inc ebx
		inc edx
		loop @b
	mov ebx,bit_chi_right
	mov ecx,5 ;цикл для рисования правых циплят
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
	mov ecx,eax ;цикл для рисования штрафных очков
	shr ecx,1
	mov edx,10 ;индекс начала штрафных иконок в буфере buf_chi
	cmp ecx,0
	je .no_unit_last
	@@:
		stdcall DrawArrayImg, buf_displ,buf_chi,edx,[color_chick]
		inc edx
		loop @b
	.no_unit_last:

	bt ax,0 ;проверяем мигающее не четное очко
	jae @f
		bt word[pos_wolf],bit_mig
		jc @f
			stdcall DrawArrayImg, buf_displ,buf_chi,edx,[color_chick] ;мигающее штрафное очко
	@@:

	popad
	ret

align 4
CountEggsInc: ;увеличиваем счетчик яиц на 1
	push eax edi
		xor eax,eax
		inc word[eggs_count] ;увеличиваем счетчик яиц
		cmp word[eggs_count],200 ;бонусные очки
		je @f
		cmp word[eggs_count],500 ;бонусные очки
		je @f
			jmp .no_bonus
		@@:
			mov byte[count_last],0
		.no_bonus:

		mov ax,word[eggs_count]
		mov edi,some_text
		call convert_to_str ;обновляем текстовую строку
		and ax,0xf ;через каждые 16 яиц уменьшаем время
		cmp ax,0
		jne @f
			cmp dword[game_spd],15 ;минимальная задержка
			jle @f
				dec dword[game_spd]
		@@:
	pop edi eax
	ret

;input:
; eax - маска, указывающая с какой строны упало яйцо
align 4
CountLastInc: ;начисление штрафных очков
	inc byte[count_last] ;половинао штрафного очка
	bt word[pos_wolf],bit_zaac
	jc @f
		inc byte[count_last] ;если нет зайца то еще 1-на половина штрафного очка
	@@:
	cmp byte[count_last],6
	jle @f
		mov byte[count_last],6 ;ставим ограничени на число штрафных очков
	@@:
	or dword[pos_eggs],eax ;создаем бегущего ципленка
	ret

align 4
MoveEggs:
	pushad
	xor byte[pos_wolf],val_mig ;бит для мигания

	cmp byte[count_last],6 ;максимальное число штрафных очков
	je .end_fun

	rol dword[pos_eggs],1
	mov ecx,dword[pos_eggs]
	and ecx,mask_fail_eggs
	cmp ecx,0
	je .no_fail ;нет падающих яиц
		;начисление яиц попавших в корзину
		;или начисление штрафных очков
		xor ebx,ebx
		mov bl,byte[pos_wolf] ;берем параметры (позицию) волка
		and bl,3 ;на всякий случай

		bt ecx,5 ;проверяем верхнее левое перило
		jae .perilo_lu
			cmp bx,0 ;проверяем наличие корзины
			jne @f
				call CountEggsInc
				jmp .perilo_lu
			@@:
				mov eax,mask_chi_cr_l
				call CountLastInc
			.perilo_lu:
		bt ecx,10 ;проверяем нижнее левое перило
		jae .perilo_ld
			cmp bx,2 ;проверяем наличие корзины
			jne @f
				call CountEggsInc
				jmp .perilo_ld
			@@:
				mov eax,mask_chi_cr_l
				call CountLastInc
		.perilo_ld:
		bt ecx,15 ;проверяем верхнее правое перило
		jae .perilo_ru
			cmp bx,1 ;проверяем наличие корзины
			jne @f
				call CountEggsInc
				jmp .perilo_ru
			@@:
				mov eax,mask_chi_cr_r
				call CountLastInc
		.perilo_ru:
		bt ecx,20 ;проверяем нижнее правое перило
		jae .perilo_rd
			cmp bx,3 ;проверяем наличие корзины
			jne @f
				call CountEggsInc
				jmp .perilo_rd
			@@:
				mov eax,mask_chi_cr_r
				call CountLastInc
		.perilo_rd:
	.no_fail:

	and dword[pos_eggs],mask_clear_all ;очистка упавших яиц и добежавших курей

	call rand_next
	cmp byte[zaac_status],0
	jle @f
		dec byte[zaac_status]
		jmp .no_zaac_move ;заяц пока не двигается
	@@:
	
	bt dword[rand_x],6 ;заяц от фонаря меняет статус
	jc @f
		xor byte[pos_wolf],val_zaac ;высовываем/засовываем зайца
		bt word[pos_wolf],val_zaac
		jc .zaac_n
			mov byte[zaac_status],val_zaac_time_y ;ставим минимальное время для смены статуса
			jmp @f
		.zaac_n:
			mov byte[zaac_status],val_zaac_time_n ;ставим минимальное время для смены статуса
	@@:
	.no_zaac_move:

	;создание новых яиц
	bt dword[rand_x],4 ;проверяем будем ли создавать новое яйцо
	jc .end_creat
	bt dword[rand_x],5 ;проверяем с какой стороны будем создавать новое яйцо
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
proc InitGame, b:dword ;первоначальные настройки игры
	mov word[eggs_count],0 ;колличество пойманых яиц
	mov byte[some_text],'0'
	mov byte[some_text+1],0 ;текст с числом пойманных яиц
	mov byte[count_last],0
	mov dword[pos_eggs],0
	mov byte[zaac_status],0

	cmp dword[b],0
	jne @f
		mov byte[game_text+5],'А'
		mov dword[game_spd],65 ;задержка игры
		jmp .end_init
	@@:
		mov byte[game_text+5],'Б'
		mov dword[game_spd],35 ;задержка игры
	.end_init:

	push eax ebx
		mcall 26,9
		mov dword[rand_x],eax ;заполняем 1-е случайное число
	pop ebx eax

	ret
endp

align 4
proc LoadArrayBuffer, f_name:dword, buf_start:dword, count:dword
	pushad
	mov edx,dword[displ_bytes]
	mov ecx,edx
	imul ecx,dword[count]
	mov eax,dword[f_name]
	load_image_file eax,image_data_gray,ecx
		mov edx,dword[displ_bytes]
		mov eax,[image_data_gray]
		mov edi,dword[buf_start]
		mov ecx,dword[count]
		cld
		@@: ;считываем 3 буфера с декорациями
			stdcall CreateTrapharetBuffer,edi,eax
			add eax,edx
			add edi,BUF_STRUCT_SIZE
			loop @b
	stdcall mem.Free,[image_data_gray] ;освобождаем память
	popad
	ret
endp

align 4
start:
	load_libraries l_libs_start,load_lib_end

	;проверка на сколько удачно загузилась наша либа
	mov	ebp,lib0
	cmp	dword [ebp+ll_struc_size-4],0
	jz	@f
		mcall -1 ;exit not correct
	@@:
	mov	ebp,lib1
	cmp	dword [ebp+ll_struc_size-4],0
	jz	@f
		mcall -1 ;exit not correct
	@@:

	mcall 40,0x27
	mcall 48,3,sc,sizeof.system_colors ;получаем системные цвета

	;работа с файлом настроек
	copy_path ini_name,sys_path,file_name,0x0
	stdcall dword[ini_get_int],file_name,ini_sec_files,key_displ_w,210
	mov	dword[displ_w],eax
	stdcall dword[ini_get_int],file_name,ini_sec_files,key_displ_h,140
	mov	dword[displ_h],eax
	stdcall dword[ini_get_color],file_name,ini_sec_color,key_color_unit,0
	mov	dword[color_wolf],eax

	mov edx,dword[displ_w]
	imul edx,dword[displ_h]
	lea edx,[edx+edx*2]
	mov dword[displ_bytes],edx ;вычисляем размер игрового поля

	stdcall LoadArrayBuffer, fn_icon0, buf_decor,3 ;считываем 3 буфера с декорациями
	stdcall LoadArrayBuffer, fn_icon1, buf_wolf,9 ;считываем 9 буферов с волком и зайцем
	stdcall LoadArrayBuffer, fn_icon2, buf_egg,22 ;считываем 22 буферов с яйцами
	stdcall LoadArrayBuffer, fn_icon3, buf_chi,13 ;считываем 13 буферов с циплятами

	load_image_file fn_font, image_data_gray,IMAGE_FONT_SIZE
	stdcall [buf2d_create_f_img], buf_font,[image_data_gray] ;создаем буфер
	stdcall mem.Free,[image_data_gray] ;освобождаем память

	stdcall [buf2d_conv_24_to_8], buf_font,1 ;делаем буфер прозрачности 8 бит
	stdcall [buf2d_convert_text_matrix], buf_font

;проверка занимаемого буфером места
;mov edi,buf_wolf
;add edi,BUF_STRUCT_SIZE ;переходим на буфер корзины
;stdcall [buf2d_clear],edi,0x808080 ;заливаем его серым цветом

	mov ebx,dword[displ_w]
	mov edx,dword[displ_h]

	mov edi,buf_displ
	mov buf2d_w,ebx
	mov buf2d_h,edx
	stdcall [buf2d_create], buf_displ ;создаем буфер для вывода на экран

	mov edi,buf_fon
	mov buf2d_w,ebx
	mov buf2d_h,edx
	stdcall [buf2d_create], buf_fon ;создаем буфер с фоновыми декорациями

	call InitBackgroundBuffer ;заполняем буфер с фоновыми декорациями
	stdcall InitGame,0
	mcall 26,9
	mov [last_time],ebx



align 4
red_win:
	call draw_window

align 4
still: ;главный цикл
	mcall 26,9
	mov ebx,[last_time]
	add ebx,dword[game_spd] ;delay
	sub ebx,eax
	cmp ebx,dword[game_spd] ;delay
	ja it_is_time_now
	test ebx,ebx
	jz it_is_time_now
	mcall 23

	cmp eax,0
	je it_is_time_now

	;mcall 10

	cmp al,0x1 ;изменилось положение окна
	jz red_win
	cmp al,0x2
	jz key
	cmp al,0x3
	jz button

	jmp still

align 4
it_is_time_now:
	mcall 26,9
	mov [last_time],eax

	;...здесь идут действия, вызываемые каждые delay сотых долей секунд...
	call MoveEggs
	call draw_display
	jmp still

align 4
key:
	push eax ebx
	mcall 2

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
	mcall 12,1

	xor eax,eax
	mov ebx,20*65536+480
	mov ecx,20*65536+270
	mov edx,[sc.work]
	;or edx,0x33000000
	or edx,0x73000000
	mov edi,hed
	mcall ;создание окна

	mcall 9,procinfo,-1
	mov edi,buf_displ
	mov eax,dword[procinfo.client_box.width]
	cmp eax,dword[displ_w]
	jle @f
		sub eax,dword[displ_w]
		shr eax,1
		mov buf2d_l,ax ;выправниваем буфер по центру окна
	@@:

	call draw_display

	mov eax,13 ;рисование прямоугольника
	mov edx,[sc.work]
	xor esi,esi
	mov si,buf2d_l
	add esi,dword[displ_w]
	mov ebx,dword[procinfo.client_box.width]
	inc ebx
	cmp esi,ebx
	jge @f
		sub ebx,esi
		rol ebx,16
		mov bx,si
		rol ebx,16
		mov ecx,dword[procinfo.client_box.height]
		inc ecx
		int 0x40 ;рисование правого бокового поля
		jmp .draw_s
	@@:
		mov esi,dword[procinfo.client_box.width] ;когда по ширине не влазит
		inc esi
	.draw_s:

if use_but eq 1
	; *** рисование кнопок ***
push esi
	mov eax,8
	xor ebx,ebx
	mov bx,buf2d_l
	add ebx,buf2d_w
	add ebx,BUT1_L
	shl ebx,16
	mov bx,BUT1_W
	mov ecx,BUT1_T*65536+BUT1_H
	mov edx,5
	;or edx,0x40000000
	mov esi,dword[color_but_sm]
	int 0x40

	inc edx
	add ecx,BUT1_NEXT_TOP
	int 0x40
pop esi

	mov eax,4 ;аЁбRў -Ё? в?Єбв 
	mov bx,BUT1_H
	add ebx,3*65536;+3
	mov ecx,dword[color_but_te]
	or  ecx,0x80000000
	mov edx,txt_game_a
	int 0x40

	ror ebx,16
	add ebx,BUT1_NEXT_TOP
	ror ebx,16
	mov edx,txt_game_b
	int 0x40

	; *** восстановление параметров ***
	mov eax,13 ;рисование прямоугольника
	mov edx,[sc.work]
end if

	mov ebx,esi
	mov ecx,dword[procinfo.client_box.height]
	inc ecx
	mov esi,dword[displ_h]
	cmp esi,ebx
	jge @f
		sub ecx,esi
		rol ecx,16
		mov cx,si
		rol ecx,16
		int 0x40 ;рисование нижнего поля
	@@:
	
	xor ebx,ebx
	mov bx,buf2d_l
	mov ecx,dword[displ_h]
	int 0x40 ;рисование левого бокового поля

	mcall 12,2
	popad
	ret

align 4
draw_display:

	stdcall mem_copy, dword[buf_fon],dword[buf_displ],315*210*3;dword[displ_bytes] ;копирование изображения из фонового буфера
	call DrawZaac ;рисуем зайца
	call DrawWolf ;рисуем волка
	call DrawEggs ;рисуем яйца

push eax
	mov eax,dword[displ_w]
	shr eax,1
	stdcall [buf2d_draw_text], buf_displ, buf_font,game_text,eax,OFFS_SHADOW_X,[color_curici] ;рисуем строку с текстом
pop eax
	stdcall [buf2d_draw], buf_displ
	ret

align 4
button:
	mcall 17 ;получить код нажатой кнопки
	if use_but eq 1
	cmp ah,5
	jne @f
		stdcall InitGame,0
	@@:
	cmp ah,6
	jne @f
		stdcall InitGame,1
	@@:	end if
	cmp ah,1
	jne still
.exit:
	stdcall [buf2d_delete],buf_fon ;удаляем буфер
	stdcall [buf2d_delete],buf_displ ;удаляем буфер

	stdcall [buf2d_delete],buf_font

	cld
	mov ecx,3
	mov edi,buf_decor
	@@: ;удаляем 3 буфера
		stdcall [buf2d_delete],edi
		add edi,BUF_STRUCT_SIZE
		loop @b
	mov ecx,9
	mov edi,buf_wolf
	@@: ;удаляем 9 буферов с волком и зайцем
		stdcall [buf2d_delete],edi
		add edi,BUF_STRUCT_SIZE
		loop @b
	mov ecx,13
	mov edi,buf_chi
	@@: ;удаляем 13 буферов
		stdcall [buf2d_delete],edi
		add edi,BUF_STRUCT_SIZE
		loop @b
	mov ecx,22
	mov edi,buf_egg
	@@: ;удаляем 22 буфера
		stdcall [buf2d_delete],edi
		add edi,BUF_STRUCT_SIZE
		loop @b

	mcall -1 ;выход из программы

head_f_i:
head_f_l  db 'Системная ошибка',0

system_dir0 db '/sys/lib/'
name_buf2d db 'buf2d.obj',0
err_message_found_lib0 db 'Не удалось найти библиотеку buf2d.obj',0
err_message_import0 db 'Ошибка при импорте библиотеки buf2d.obj',0

system_dir1 db '/sys/lib/'
name_libimg db 'libimg.obj',0
err_message_found_lib1 db 'Не удалось найти библиотеку libimg.obj',0
err_message_import1 db 'Ошибка при импорте библиотеки libimg.obj',0

system_dir2 db '/sys/lib/'
libini_name db 'libini.obj',0
err_message_found_lib2 db 'Не удалось найти библиотеку libini.obj',0
err_message_import2 db 'Ошибка при импорте библиотеки libini.obj',0

;library structures
l_libs_start:
	lib0 l_libs name_buf2d,  sys_path, file_name, system_dir0, err_message_found_lib0, head_f_l, import_buf2d_lib, err_message_import0, head_f_i
	lib1 l_libs name_libimg, sys_path, file_name, system_dir1, err_message_found_lib1, head_f_l, import_libimg, err_message_import1, head_f_i
	lib2 l_libs libini_name, sys_path, file_name, system_dir2, err_message_found_lib2, head_f_l, libini_import, err_message_import2, head_f_i
load_lib_end:

align 4
proc mem_copy, source:dword, destination:dword, len:dword
  push ecx esi edi
    cld
    mov esi, dword[source]
    mov edi, dword[destination]
    mov ecx, dword[len]
    rep movsb
  pop edi esi ecx
  ret
endp

align 4
convert_to_str:
	pushad
	;mov eax,dword[value]
	;mov edi,dword[text]
	mov dword[edi+1],0
	cld
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
  call .str;перейти на саму себя т.е. вызвать саму себя и так до того момента пока в eax не станет меньше чем в ecx
  pop eax
  @@: ;cmp al,10 ;проверить не меньше ли значение в al чем 10 (для системы счисленя 10 данная команда - лишная))
  ;sbb al,$69  ;- честно данная инструкция меня заставляет задуматься т.е. я не знаю как это работает
  ;das        ;после данной команды как бы происходит уменьшение al на 66h  (в книге написано другое)
  or al,0x30  ;данная команда короче  чем две выше 
  stosb       ;записать элемент из регистра al в ячеку памяти es:edi
  ret	      ;вернуться чень интересный ход т.к. пока в стеке храниться кол-во вызовов то столько раз мы и будем вызываться


last_time dd ?
image_data dd 0 ;память для преобразования картинки функциями libimg
image_data_gray dd 0 ;память с временными серыми изображениями в формате 24-bit, из которых будут создаваться трафареты

run_file_70 FileInfoBlock
hed db 'Nu pogodi 03.08.10',0 ;подпись окна
sc system_colors  ;системные цвета

align 4
buf_font: ;буфер со шрифтом
	dd 0 ;указатель на буфер изображения
	dw 25 ;+4 left
	dw 25 ;+6 top
	dd 128 ;+8 w
	dd 144 ;+12 h
	dd 0 ;+16 color
	db 24 ;+20 bit in pixel

align 4
buf_displ:
	dd 0 ;указатель на буфер изображения
	dw 25,0
	dd ? ;+8 w
	dd ? ;+12 h
	dd 0 ;+16 color
	db 24 ;+20 bit in pixel

align 4
buf_fon: ;фоновый буфер
	dd 0 ;указатель на буфер изображения
	dw 0 ;+4 left
	dw 0 ;+6 top
	dd ? ;+8 w
	dd ? ;+12 h
	dd 0xffffff ;+16 color
	db 24 ;+20 bit in pixel

align 4
buf_decor: ;буфера с декорациями: домиками и рейками; с курицами; с растениями
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
libini_import:
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
	buf2d_cruve_bezier dd sz_buf2d_cruve_bezier
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
	sz_buf2d_circle db 'buf2d_circle',0 ;рисование окружности
	sz_buf2d_img_hdiv2 db 'buf2d_img_hdiv2',0
	sz_buf2d_img_wdiv2 db 'buf2d_img_wdiv2',0
	sz_buf2d_conv_24_to_8 db 'buf2d_conv_24_to_8',0
	sz_buf2d_conv_24_to_32 db 'buf2d_conv_24_to_32',0
	sz_buf2d_bit_blt db 'buf2d_bit_blt',0
	sz_buf2d_bit_blt_transp db 'buf2d_bit_blt_transp',0
	sz_buf2d_bit_blt_alpha db 'buf2d_bit_blt_alpha',0
	sz_buf2d_cruve_bezier db 'buf2d_cruve_bezier',0
	sz_buf2d_convert_text_matrix db 'buf2d_convert_text_matrix',0
	sz_buf2d_draw_text db 'buf2d_draw_text',0
	sz_buf2d_crop_color db 'buf2d_crop_color',0
	sz_buf2d_offset_h db 'buf2d_offset_h',0

i_end:
	rb 1024
	align 16
	procinfo process_information
stacktop:
	sys_path rb 4096
	file_name:
		rb 4096
	plugin_path:
		rb 4096
	openfile_path:
		rb 4096
	filename_area:
		rb 256
mem:
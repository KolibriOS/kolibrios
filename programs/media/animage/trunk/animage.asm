;******************************************************
;***GRAPHICS EDIT ANIMAGE creted by andrew_programmer**
;******************************************************

use32
org 0x0

  db  'MENUET01'
  dd  0x1
  dd  START
  dd  I_END
  dd  0x19000;100 kb
  dd  0x19000;
  dd  0x0
  dd  0x0
  include 'giflib.inc'
  include 'bmplib.inc'
  include 'dialog.inc'
  include 'dialog2.inc'
  include 'design.inc'
  include 'graphlib.inc'
START:

;--------------------------------------------------------
;---------------set events mask--------------------------
;--------------------------------------------------------
   mov eax,40
   mov ebx,1100111b
   int 0x40

;---------------------------------------------------------
;--------initializate data and params of program----------
;---------------------------------------------------------
   mov [Scroll1CoordinatX],8+1
   mov [Scroll2CoordinatY],20+15+1+46+3
   mov [Window_SizeX],640
   mov [Window_SizeY],400
   mov [Window_CordinatX],50
   mov [Window_CordinatY],50
   mov [WorkScreen_SizeX],100
   mov [WorkScreen_SizeY],100
   mov [MaxWorkScreen_SizeX],100
   mov [MaxWorkScreen_SizeY],100
   mov [ScreenPointer],0x19000
   and [PosX],0
   and [PosY],0
   mov [Picture_SizeX],640;400
   mov [Picture_SizeY],400;280
   mov [k],1
   mov [PointerToPicture],0x19000+(1200*1000*3)+30*(20*20*3)+500000
   mov [PointerToCopyPicture],0x19000+(1200*1000*3)+30*(20*20*3)+500000+(640*400*3)
   mov [PointerToCopyPicture2],0x19000+(1200*1000*3)+30*(20*20*3)+500000+(640*400*3)*2
   mov [PointerToEditBufer],0x19000+(1200*1000*3)+30*(20*20*3)+500000+(640*400*3)*3
   mov [PointerToSpriteBufer],0x19000+(1200*1000*3)+30*(20*20*3)+500000+(640*400*3)*4
   mov [PointerToPalette],0x19000+(1200*100*3)+30*(20*20*3)+1
   mov [ReserveArray],0x19000+(1200*1000)*3+30*(20*20*3)+8
   and [save_flag],0
   mov [line_width],1
   mov [lastik_is_active],0
   and [crossing],0
   and [finishing_crossing],0
   and [number_undo],0
   and [instrument_used],0
   and [DrawSprite_flag],0
   and [extended_memory],0

   finit
;----------------------------------------------------------
;--------get memory and draw window of program-------------
;----------------------------------------------------------
   call GetMemory

   call cleare_work_arrea
   call load_icons
   call drawwin
;----------------------------------------------------------
;---------------------MAIN LOOP----------------------------
;----------------------------------------------------------
main_loop:
   still:

   call event

   cmp eax,1
   jne no_redraw_window
   call drawwin
   call drawwin
   jmp still
   no_redraw_window:
   cmp eax,3
   je buttons
   cmp eax,6
   je mouse
   cmp eax,2
   je keys
   jmp main_loop

;------------------------------------------
;-------------KEYS-------------------------
;------------------------------------------
   keys:

   mov eax,2
   int 0x40
   shr eax,8

   cmp eax,176
   jne key2
   sub [PosX],20
   jmp change_screen
 key2:
   cmp eax,179
   jne key3
   add [PosX],20
   jmp change_screen
 key3:
   cmp eax,177
   jne key4
   add [PosY],20
   jmp change_screen
 key4:
  cmp eax,178
   jne key5
   sub [PosY],20
   jmp change_screen
 key5:
   cmp eax,49
   jne key6
   mov [k],1
   mov [Scroll1CoordinatX],9
   mov [Scroll2CoordinatY],85
   call drawwin
   jmp change_screen
 key6:
   cmp eax,50
   jne key7
   mov [k],2
   mov [Scroll1CoordinatX],9
   mov [Scroll2CoordinatY],85
   call drawwin
   jmp change_screen
 key7:
   cmp eax,52
   jne key8
   mov [k],4
   mov [Scroll1CoordinatX],9
   mov [Scroll2CoordinatY],85
   call drawwin
   jmp change_screen
 key8:
   cmp eax,56
   jne key9
   mov [k],8
   mov [Scroll1CoordinatX],9
   mov [Scroll2CoordinatY],85
   call drawwin
   jmp change_screen
 key9:
   cmp eax,48
   jne key10
   mov [k],16
   mov [Scroll1CoordinatX],9
   mov [Scroll2CoordinatY],85
   call drawwin
   jmp change_screen
 key10:
   cmp eax,255
   jne key11
   call analizing_picture_to_palette
   call drawwin
   jmp change_screen
 key11:
   cmp eax,27
   jne still
    mov eax,-1
    int 0x40
   jmp still

   change_screen:

   call CalculatePositionScreen
   call MovePictureToWorkScreen
   call draw_scrollers
   call PrintMousePos

   jmp still
;---------------------------------------------
;-----------panel BUTTONS---------------------
;---------------------------------------------
buttons:

	cmp [Current_instrument],30
	jne no_finish_instrument_button

	cmp [instrument_used],1
	jne no_finish_instrument_button

	cmp [Activate_instrument],0
	jne no_finish_instrument_button

	cmp [crossing],1
	jne no_finish_instrument_button

	mov [finishing_crossing],1
	call TakeButtonInstruments

	no_finish_instrument_button:

	mov eax,17
	int 0x40

	shr eax,8

	cmp eax,1
	jne no_exit

	mov eax,-1
	int 0x40

	no_exit:

	mov [Current_instrument],eax

	cmp eax,10
	jl still

	cmp eax,15
	je still

	cmp eax,17
	je still

	cmp eax,18
	je still

	cmp eax,19
	je still

	cmp eax,20
	je still

	cmp eax,21
	jne no_palette_

	call TakeButtonInstruments
	jmp still

	no_palette_:

	cmp eax,23
	je still

	cmp eax,30
	jne no_allocation__

	and [Activate_instrument],0
	jmp still

	no_allocation__:

	cmp eax,38
	jne no_kontur__

	and [instrument_used],0
	jmp still

	no_kontur__:

	call TakeButtonInstruments
	jmp still
;---------------------------------------------
;-------------interraction MOUSE--------------
;---------------------------------------------
mouse:

	call GetMouseCoordinats

	;panel
	mov edx,[Window_SizeX]
	sub edx,5

	cmp ebx,20
	jle no_panel

	cmp ebx,20+15
	jae no_panel

	cmp eax,5
	jle no_panel

	cmp eax,edx ;585
	jae no_panel

	call GetMouseCoordinats

	mov [counter],7
	mov edi,panel_text
	call panel_interraction

	jmp still
	no_panel:

	cmp [Panel_flag],0
	jz no_redraw_panel

	mov [counter],7
	mov edi,panel_text
	call panel_interraction

	no_redraw_panel:

	call GetMouseCoordinats
	mov eax,[MouseX]
	mov ebx,[MouseY]
	mov ecx,[WorkScreen_SizeX]
	mov edx,[WorkScreen_SizeY]
	add ecx,9
	add edx,85

	cmp eax,ecx
	jae mouse_scroll

	cmp ebx,edx
	jae mouse_scroll

	jmp no_scrollers

	mouse_scroll:

	mov [exit_from_work_arrea],1  ;mouse situated after work arrea

	;scrollers
	call GetMouseClick

	cmp eax,1
	jne no_click

	call GetMouseCoordinats
	;interraction with horizontal scroller
	mov eax,[Scroll1CoordinatX]
	mov ebx,[Scroll1CoordinatY]
	mov ecx,[MouseX]
	mov edx,[MouseY]
	mov esi,[Scroll1MaxSizeX]
	mov edi,[Scroll1MaxSizeY]
	call columnus

	test eax,eax
	jz no_horizontal


	mov eax,9
	mov ebx,[Scroll1CoordinatY]
	mov ecx,[Scroll1MaxSizeX]
	mov edx,14+10
	mov esi,7
	inc ecx
	call draw_volume_rectangle

	mov eax,[MouseX]
	mov ebx,[Scroll1SizeX]
	shr ebx,1
	sub eax,ebx
	mov ecx,[MouseX]
	add ecx,ebx
	mov edx,8+1
	add edx,[Scroll1MaxSizeX]
	mov [Scroll1CoordinatX],eax

	sub eax,9
	jns no_min_scroll

	mov [Scroll1CoordinatX],9

	no_min_scroll:

	cmp ecx,edx
	jl no_max_scroll

	sub edx,ebx
	sub edx,ebx
	mov [Scroll1CoordinatX],edx

	no_max_scroll:
	;
	mov eax,[Scroll1CoordinatX]
	sub eax,9
	mov ebx,[Picture_SizeX]
	imul eax,ebx
	mov ebx,[Scroll1MaxSizeX]
	cdq
	idiv ebx
	mov [PosX],eax

	jmp no_vertical

	no_horizontal:

	mov eax,[Scroll2CoordinatX]
	mov ebx,[Scroll2CoordinatY]
	mov ecx,[MouseX]
	mov edx,[MouseY]
	mov esi,[Scroll2MaxSizeX]
	mov edi,[Scroll2MaxSizeY]
	call columnus

	test eax,eax
	jz no_vertical

	mov eax,[Scroll2CoordinatX]
	mov ebx,85
	mov ecx,14+10
	mov edx,[Scroll2MaxSizeY]
	mov esi,7
	inc edx
	call draw_volume_rectangle

	mov eax,[MouseY]
	mov ebx,[Scroll2SizeY]
	shr ebx,1
	sub eax,ebx
	mov ecx,[MouseY]
	add ecx,ebx
	mov edx,85
	add edx,[Scroll2MaxSizeY]
	mov [Scroll2CoordinatY],eax

	sub eax,85
	jns no_min_scroll2

	mov [Scroll2CoordinatY],85

	no_min_scroll2:

	cmp ecx,edx
	jl no_max_scroll2

	sub edx,ebx
	sub edx,ebx
	mov [Scroll2CoordinatY],edx

	no_max_scroll2:
	;
	mov eax,[Scroll2CoordinatY]
	sub eax,85
	mov ebx,[Picture_SizeY]
	imul eax,ebx
	mov ebx,[Scroll2MaxSizeY]
	cdq
	idiv ebx
	mov [PosY],eax

	no_vertical:

	call CalculatePositionScreen

	call draw_scrollers
	call MovePictureToWorkScreen

	no_click:

	jmp still
	no_scrollers:


	mov eax,[MouseX]
	mov ebx,[MouseY]
	mov ecx,[Window_SizeX]
	mov edx,[Window_SizeY]
	sub ecx,36
	sub edx,35

	cmp eax,9
	jle not_work_arrea

	cmp eax,ecx
	jae not_work_arrea

	cmp ebx,20+15+1+46
	jle not_work_arrea

	cmp ebx,edx
	jae not_work_arrea

	jmp mouse_in_work_arrea

	not_work_arrea:

	mov [exit_from_work_arrea],1

	jmp still

	mouse_in_work_arrea:

	call GetScreenCordinats
	call PrintMousePos
	call GetMouseClick

	test eax,eax
	jz no_use_instruments

	cmp [Activate_instrument],0

	jnz no_undo___
	;------------begin copy for undo-------------
	inc [number_undo]

	cmp [number_undo],1
	jne no_one__

	mov edi,[PointerToCopyPicture]

	no_one__:

	cmp [number_undo],2
	jne no_two__

	mov edi,[PointerToCopyPicture2]

	no_two__:

	cmp [number_undo],3
	jne no_three__

	;copy bufer number two to bufer number one
	mov esi,[PointerToCopyPicture2]
	mov edi,[PointerToCopyPicture]
	mov ecx,[Picture_SizeX]
	imul ecx,[Picture_SizeY]
	lea ecx,[ecx+ecx*2]
	add ecx,4
	shr ecx,2
	inc ecx
	cld
	rep movsd
	;end copy
	dec [number_undo]
	mov edi,[PointerToCopyPicture2]

	no_three__:

	mov esi,[PointerToPicture]
	mov ecx,[Picture_SizeX]
	imul ecx,[Picture_SizeY]
	lea ecx,[ecx+ecx*2]
	add ecx,4
	shr ecx,2
	inc ecx
	cld
	rep movsd
	;--------------end copy for undo-------------
	no_undo___:

	call TakeButtonInstruments
	no_use_instruments:

	mov eax,[Current_instrument]
	and [Activate_instrument],0

	jmp still
;-----------------------------------------------
;---------get mouse cordinats-------------------
;-----------------------------------------------
GetMouseCoordinats:

	mov eax,37
	mov ebx,1
	int 0x40

	mov ebx,eax
	shr eax,16
	and ebx,0xffff

	mov [MouseX],eax
	mov [MouseY],ebx

	ret
;------------------------------------------------
;-------get mouse attributs----------------------
;------------------------------------------------
GetMouseClick:
	 mov eax,37
	 mov ebx,2
	 int 0x40

	 ret
;-------------------------------------------------
;-----interraction panel with user----------------
;-------------------------------------------------
panel_interraction:
	;>>>>>>>>>>>>>>>>>>>>>>>>>>
	mov eax,18
	mov ebx,14
	int 0x40

	and [Panel_flag],0
	;collision with text on panel
      next_columnus_text:

	mov eax,[edi]
	mov ebx,[edi+4]
	sub eax,10
	sub ebx,3
	mov [Icon_text_x],eax
	mov [Icon_text_y],ebx
	mov esi,[edi+8]
	add edi,esi
	add edi,3*4
	mov ecx,[MouseX]
	mov edx,[MouseY]
	push edi
	mov edi,13
	mov esi,80
	call columnus

	pop edi
	mov esi,7

	test eax,eax
	jz no_columnus_text

	mov esi,1
	mov [Panel_flag],1
	call GetMouseClick

	test eax,eax
	jz no_mouse_pressed

	call print_panel_menu

	no_mouse_pressed:

	no_columnus_text:

	mov eax,[Icon_text_x]
	mov ebx,[Icon_text_y]
	mov ecx,79
	mov edx,13
	call draw_volume_rectangle

	push [counter]
	push edi
	mov [counter],7
	mov edi,panel_text
	call print_panel_text

	pop edi
	pop [counter]
	dec [counter]
	jnz next_columnus_text

	ret
;---------------------------------------------------------
;-----draw panel menu(main engin of panel)----------------
;---------------------------------------------------------
print_panel_menu:


	push [counter]
	;delit icon buttons(some time)
	mov [counter],10
	next_icon_delit:
	mov eax,8
	mov edx,[counter]
	add edx,11100000000000000000000000000000b
	int 0x40
	inc [counter]

	cmp [counter],30
	jl next_icon_delit

	pop [counter]

	push esi
	mov esi,[counter]
	dec esi
	shl esi,4

	mov eax,[menu_rectangles+esi]
	mov ebx,[menu_rectangles+esi+4]
	mov ecx,[menu_rectangles+esi+8]
	mov edx,[menu_rectangles+esi+12]
	mov esi,1
	mov [menu_coordinat_x],eax
	mov [menu_size_x],ecx
	call draw_volume_rectangle
	pop esi

    calculate_counter_menu:


	;calculate menu counter
	mov eax,[counter]
	dec eax
	shl eax,2
	mov ebx,[menu_counters+eax]
	mov [counter_menu],ebx

	call GetMouseCoordinats

	and [menu_counter],0
	push esi
	push edi
	;сглаживание анимации >>>>>>>>>>>>>>>>>>
	mov eax,18
	mov ebx,14
	int 0x40

    menu_loop:

	mov eax,[menu_coordinat_x]
	mov ebx,36
	mov ecx,[MouseX]
	mov edx,[MouseY]
	mov esi,[menu_size_x]
	mov edi,18
	add ebx,[menu_counter]
	add eax,2
	sub esi,3
	inc ebx
	call columnus
	mov esi,7

	test eax,eax
	jz no_columnus_menu_text

	mov esi,1

	no_columnus_menu_text:

	mov eax,[menu_coordinat_x]
	mov ebx,36
	mov ecx,[menu_size_x]
	mov edx,18
	add ebx,[menu_counter]
	add eax,2
	sub ecx,3
	inc ebx
	call draw_volume_rectangle
	add [menu_counter],20


	dec [counter_menu]
	jnz menu_loop

	;print menu text
	push [counter]
	mov esi,[counter]
	dec esi
	shl esi,2
	mov edi,[menu_text_en+esi]
	mov eax,[menu_counters+esi]
	mov [counter],eax
	call print_panel_text
	pop [counter]
	pop edi
	pop esi


	menu_still:

	mov eax,10
	int 0x40
	mov eax,2
	int 0x40
	mov eax,17
	int 0x40

	call GetMouseClick

	test eax,eax
	jz calculate_counter_menu

	call GetMouseCoordinats

	;calculate menu counter
	mov eax,[counter]
	dec eax
	shl eax,2
	mov ebx,[menu_counters+eax]
	mov [counter_menu],ebx

	and [menu_counter],0
	and [counter_11],0

	push esi
	push edi

    menu_loop2:


	mov eax,[menu_coordinat_x]
	mov ebx,36
	mov ecx,[MouseX]
	mov edx,[MouseY]
	mov esi,[menu_size_x]
	mov edi,18
	add ebx,[menu_counter]
	add eax,2
	sub esi,3
	inc ebx
	call columnus

	inc [counter_11]

	test eax,eax
	jz no_columnus_menu_text2

	mov esi,[counter_11]
	mov [number_menu],esi

	no_columnus_menu_text2:

	add [menu_counter],20

	dec [counter_menu]
	jnz menu_loop2

	pop edi
	pop esi

	mov eax,5
	mov ebx,15
	int 0x40

	push [counter]
	pushad

	call drawwin

	popad
	pop [counter]

	mov eax,[counter]
	mov [number_panel],eax

	call TakeInstruments
	jmp still

;---------------------------------------------------------
;------print mouse position on panel----------------------
;---------------------------------------------------------
PrintMousePos:

	pushad

	mov eax,385
	mov ebx,20+15+6
	mov ecx,52
	mov edx,18
	mov esi,4
	call draw_volume_rectangle

	mov eax,385
	mov ebx,20+15+6+18+1
	mov ecx,52
	mov edx,18
	mov esi,4
	call draw_volume_rectangle

	mov eax,390
	mov ebx,20+15+6+6
	and ecx,0
	mov edx,mouse_pos_x
	mov esi,2
	call print_text

	mov eax,390
	mov ebx,20+15+6+6+18+1
	and ecx,0
	mov edx,mouse_pos_y
	mov esi,2
	call print_text

	mov eax,47
	mov ebx,4*65536
	mov ecx,[ScreenX]
	add ecx,[PosX]
	mov edx,405*65536+20+15+6+6
	and esi,0
	int 0x40

	mov eax,47
	mov ebx,4*65536
	mov ecx,[ScreenY]
	add ecx,[PosY]
	mov edx,405*65536+20+15+6+18+1+6
	and esi,0
	int 0x40

	popad
	ret

;---------------------------------------------------------
;---------- get time in 1/100 secunds---------------------
;---------------------------------------------------------
Clock:
	  mov eax,26
	  mov ebx,9
	  int 0x40
	  mov [time],eax
	  ret
;----------------------------------------------------------
;-------------draw window of program-----------------------
;----------------------------------------------------------
drawwin:

	mov eax,12
	mov ebx,1
	int 0x40

	and eax,0
	mov ebx,[Window_CordinatX]
	mov ecx,[Window_CordinatY]
	shl ebx,16
	shl ecx,16
	add ebx,[Window_SizeX]
	add ecx,[Window_SizeY]
	mov edx,0x03aabbcc
	mov esi,0x805080d0
	mov edi,0x005080d0
	int 0x40

	call draw_panel
	call PrintMousePos

	mov [counter],7
	mov edi,panel_text
	call print_panel_text

	mov eax,10
	mov ebx,5
	mov ecx,0xffffff
	mov edx,name_of_program
	mov esi,34
	call print_text

	mov eax,13
	mov ebx,447*65536+35
	mov ecx,42*65536+36
	mov edx,[Color]
	int 0x40

	mov eax,12
	mov ebx,2
	int 0x40

	mov eax,9
	mov ebx,IPC_table
	or ecx,-1
	int 0x40

	mov eax,[IPC_table+34]
	mov ebx,[IPC_table+38]
	mov ecx,[IPC_table+42]
	mov edx,[IPC_table+46]
	mov [Window_CordinatX],eax
	mov [Window_CordinatY],ebx
	mov [Window_SizeX],ecx
	mov [Window_SizeY],edx

	cmp [Window_SizeX],585
	jae no_minimum_size_x

	mov [Window_SizeX],585
	mov ecx,[Window_SizeX]

	no_minimum_size_x:

	cmp [Window_SizeY],320
	jae no_minimum_size_y

	mov [Window_SizeY],320
	mov edx,[Window_SizeY]

	no_minimum_size_y:

	mov [MaxWorkScreen_SizeX],ecx
	mov [MaxWorkScreen_SizeY],edx
	sub [MaxWorkScreen_SizeX],20+10+5+10
	sub [MaxWorkScreen_SizeY],20+10+15+1+45+20+10

	cmp [MaxWorkScreen_SizeX],0
	jns no_znak1

	mov [MaxWorkScreen_SizeX],ecx

	no_znak1:
	cmp [MaxWorkScreen_SizeY],0
	jns no_znak2

	mov [MaxWorkScreen_SizeY],ecx

	no_znak2:

	mov ecx,[k]

	and [PosX],0
	and [PosY],0
	call MovePictureToWorkScreen
	call draw_scrollers
	call draw_icons

	ret
;----------------------------------------------------------
;---------------draw panel in window of program------------
;----------------------------------------------------------
draw_panel:

	mov eax,5
	mov ebx,20
	mov ecx,[Window_SizeX]
	mov edx,15
	mov esi,6
	sub ecx,10
	call draw_volume_rectangle

	mov eax,5
	mov ebx,20
	mov ecx,[Window_SizeX]
	mov edx,15
	mov esi,6
	sub ecx,10
	call draw_volume_rectangle

	mov eax,5
	mov ebx,20+15+1
	mov ecx,[Window_SizeX]
	mov edx,46
	mov esi,1
	sub ecx,10
	call draw_volume_rectangle

	mov eax,5
	mov ebx,20+15+1+46+1
	mov ecx,[Window_SizeX]
	mov edx,[Window_SizeY]
	mov esi,1
	sub ecx,10+20
	sub edx,20+15+1+46+1+5+20
	call draw_volume_rectangle

	mov eax,5
	mov ebx,[Window_SizeY]
	mov ecx,[Window_SizeX]
	mov edx,20+10
	mov esi,3
	sub ecx,30
	sub ebx,25+10
	call draw_volume_rectangle

	mov eax,[Window_SizeX]
	mov ebx,20+15+1+45+2
	mov ecx,20+10
	mov edx,[Window_SizeY]
	mov esi,3
	sub eax,25+10
	sub edx,20+15+1+45+5+20
	call draw_volume_rectangle

	mov eax,[Window_SizeX]
	mov ebx,[Window_SizeY]
	mov ecx,20+10
	mov edx,20+10
	mov esi,6
	sub eax,25+10
	sub ebx,25+10
	call draw_volume_rectangle

	mov eax,445
	mov ebx,20+15+6
	mov ecx,37
	mov edx,37
	mov esi,4
	call draw_volume_rectangle

	ret
;----------------------------------------------------------
;---------------------system events------------------------
;----------------------------------------------------------
event:
	mov eax,10
	int 0x40
	ret

;----------------------------------------------------------
;----------print text on the panel and menu----------------
;----------------------------------------------------------
print_panel_text:
       next_panel_text:

	mov eax,[edi]
	mov ebx,[edi+4]
	xor ecx,ecx
	mov edx,edi
	add edx,12
	mov esi,[edi+8]
	add edi,esi
	add edi,3*4

	push edi
	call print_text
	pop edi

	dec [counter]
	jnz next_panel_text
	ret
;----------------------------------------------------------
;--------Move picture from array to work screeen-----------
;----------------------------------------------------------
MovePictureToWorkScreen:

	call cleare_screen

	mov eax,[Picture_SizeX]
	mov ebx,[Picture_SizeY]
	mov ecx,[MaxWorkScreen_SizeX]
	mov edx,[MaxWorkScreen_SizeY]
	mov esi,[k]
	imul eax,esi
	imul ebx,esi

	cmp eax,ecx
	jle lab1
	mov eax,[MaxWorkScreen_SizeX]
	mov [WorkScreen_SizeX],eax
	jmp lab2
	lab1:
	mov [WorkScreen_SizeX],eax
	lab2:

	cmp ebx,edx
	jle lab3
	mov ebx,[MaxWorkScreen_SizeY]
	mov [WorkScreen_SizeY],ebx
	jmp lab4
	lab3:
	mov [WorkScreen_SizeY],ebx
	lab4:

	mov eax,[WorkScreen_SizeX]
	mov ebx,[k]
	cdq
	idiv ebx
	mov [CounterX],eax
	mov eax,[WorkScreen_SizeY]
	cdq
	idiv ebx
	mov [CounterY],eax

	mov eax,[WorkScreen_SizeX]
	mov ecx,eax
	mov ebx,[k]
	cdq
	idiv ebx
	imul eax,ebx
	sub ecx,eax
	lea ecx,[ecx+ecx*2]
	;
	mov eax,[WorkScreen_SizeX]
	mov ebx,[k]
	dec ebx
	imul eax,ebx
	lea eax,[eax+eax*2]
	add eax,ecx
	mov [OffsetYWorkScreen],eax


	mov ebx,[Picture_SizeX]
	mov eax,[CounterX]
	sub ebx,eax
	lea ebx,[ebx+ebx*2]
	mov [OffsetYPicture],ebx

	mov eax,[WorkScreen_SizeX]
	mov ebx,[k]
	sub eax,ebx
	lea eax,[eax+eax*2]
	mov [OffsetYBigPixel],eax

	mov eax,[PosX]
	mov ebx,[PosY]
	mov ecx,[Picture_SizeX]
	imul ecx,ebx
	add eax,ecx
	lea eax,[eax+eax*2]
	add eax,[PointerToPicture]


	mov ebx,[ScreenPointer]

	mov edi,[CounterY]

	;if size of picture natural(mastab is 1) than move picture to work screen
	cmp [k],1
	jne no_zoom_1_
	screen_y_1:
	    mov esi,[CounterX]
	    screen_x_1:
	      mov ecx,[eax]
	      and ecx,0xffffff
	      mov ebp,ecx
	      shr ecx,16
		   mov [ebx],bp
		   mov [ebx+2],cl
	      add ebx,3;
	      add eax,3
	    dec esi
	    jnz screen_x_1
	    add eax,[OffsetYPicture]
	    add ebx,[OffsetYWorkScreen]
	dec edi
	jnz screen_y_1
	jmp fps
	no_zoom_1_:

	cmp [k],2
	jne no_zoom_2

	screen_y_2:
	    mov esi,[CounterX]
	    screen_x_2:
	      mov ecx,[eax]
	      and ecx,0xffffff
	      mov ebp,ecx
	      shr ecx,16
	      mov edx,ebx
		   mov [edx],bp
		   mov [edx+2],cl
		   mov [edx+3],bp
		   mov [edx+3+2],cl
		   add edx,3*2
		   add edx,[OffsetYBigPixel]
		   mov [edx],bp
		   mov [edx+2],cl
		   mov [edx+3],bp
		   mov [edx+3+2],cl
		   add edx,3*2
		   add edx,[OffsetYBigPixel]
	      add ebx,3*2
	      add eax,3
	    dec esi
	    jnz screen_x_2
	    add eax,[OffsetYPicture]
	    add ebx,[OffsetYWorkScreen]
	dec edi
	jnz screen_y_2
	jmp fps
	no_zoom_2:

	cmp [k],4
	jne no_zoom_4
	screen_y_4:
	    mov esi,[CounterX]
	    screen_x_4:
	      mov ecx,[eax]
	      and ecx,0xffffff
	      mov ebp,ecx
	      shr ecx,16
	      mov edx,ebx
		   mov [edx],bp
		   mov [edx+2],cl
		   mov [edx+3],bp
		   mov [edx+3+2],cl
		   mov [edx+6],bp
		   mov [edx+6+2],cl
		   mov [edx+9],bp
		   mov [edx+9+2],cl
		   add edx,3*4
		   add edx,[OffsetYBigPixel]
		   mov [edx],bp
		   mov [edx+2],cl
		   mov [edx+3],bp
		   mov [edx+3+2],cl
		   mov [edx+6],bp
		   mov [edx+6+2],cl
		   mov [edx+9],bp
		   mov [edx+9+2],cl
		   add edx,3*4
		   add edx,[OffsetYBigPixel]
		   mov [edx],bp
		   mov [edx+2],cl
		   mov [edx+3],bp
		   mov [edx+3+2],cl
		   mov [edx+6],bp
		   mov [edx+6+2],cl
		   mov [edx+9],bp
		   mov [edx+9+2],cl
		   add edx,3*4
		   add edx,[OffsetYBigPixel]
		   mov [edx],bp
		   mov [edx+2],cl
		   mov [edx+3],bp
		   mov [edx+3+2],cl
		   mov [edx+6],bp
		   mov [edx+6+2],cl
		   mov [edx+9],bp
		   mov [edx+9+2],cl
		   add edx,3*4
		   add edx,[OffsetYBigPixel]
	      add ebx,4*3
	      add eax,3
	    dec esi
	    jnz screen_x_4
	    add eax,[OffsetYPicture]
	    add ebx,[OffsetYWorkScreen]
	dec edi
	jnz screen_y_4
	jmp fps
	no_zoom_4:
	;if zoom more than 4
	screen_y:
	    mov esi,[CounterX]
	    screen_x:
	      mov ecx,[eax]
	      and ecx,0xffffff
	      mov ebp,ecx
	      shr ecx,16
	      push ebx
	      push eax
	      mov edx,[k]
	      big_pixel_y:
		 mov eax,[k]
		 big_pixel_x:
		   mov [ebx],bp
		   mov [ebx+2],cl
		   add ebx,3
		 dec eax
		 jnz big_pixel_x
		 add ebx,[OffsetYBigPixel]
	      dec edx
	      jnz big_pixel_y
	      pop eax
	      pop ebx
	      mov edx,[k]
	      lea edx,[edx+edx*2]
	      add ebx,edx
	      add eax,3
	    dec esi
	    jnz screen_x
	    add eax,[OffsetYPicture]
	    add ebx,[OffsetYWorkScreen]
	dec edi
	jnz screen_y

	 fps:
	 mov eax,18
	 mov ebx,14
	 int 0x40

	 mov eax,7
	 mov ebx,[ScreenPointer]
	 mov ecx,[WorkScreen_SizeX]
	 mov edx,[WorkScreen_SizeY]
	 shl ecx,16
	 add ecx,edx
	 mov edx,8*65536+20+15+45+5
	 int 0x40

	 ret
;----------------------------------------------------------
;--------------------clear screen--------------------------
;----------------------------------------------------------
cleare_screen:

	mov eax,[ScreenPointer]
	mov ebx,[WorkScreen_SizeX]
	imul ebx,[WorkScreen_SizeY]
	lea ebx,[ebx+ebx*2]
	shr ebx,3
	inc ebx
	mov esi,0xffffff

	clear_screen_loop:

	  mov [eax],esi
	  mov [eax+3],esi
	  mov [eax+6],si
	  add eax,4+4

	dec ebx
	jnz clear_screen_loop
	ret
;----------------------------------------------------------
;-------------cleare work arrea(work screen)---------------
;----------------------------------------------------------
cleare_work_arrea:

	mov eax,[PointerToPicture]
	mov ebx,[Picture_SizeX]
	imul ebx,[Picture_SizeY]
	lea ebx,[ebx+ebx*2]
	shr ebx,3
	inc ebx
	mov esi,0xffffff

	clear_work_arrea_loop:

	  mov [eax],esi
	  mov [eax+3],esi
	  mov [eax+6],si
	  add eax,4+4

	dec ebx
	jnz clear_work_arrea_loop
	ret
;----------------------------------------------------------
;------------draw lines of scoll---------------------------
;----------------------------------------------------------
draw_scrollers:

	 mov edi,[CounterX]
	 mov eax,[Scroll1CoordinatX]
	 mov ebx,[Window_SizeY]
	 sub ebx,22+10
	 mov ecx,[Window_SizeX]
	 sub ecx,10+5+20+10+1
	 mov edx,14+10
	 mov esi,1
	 mov [Scroll1CoordinatX],eax
	 mov [Scroll1CoordinatY],ebx
	 mov [Scroll1MaxSizeY],edx
	 mov [Scroll1MaxSizeX],ecx
	 imul ecx,edi
	 push eax
	 push ebx
	 push edx
	 mov eax,ecx
	 mov ebx,[Picture_SizeX]
	 cdq
	 idiv ebx
	 mov ecx,eax
	 pop edx
	 pop ebx
	 pop eax
	 mov [Scroll1SizeX],ecx
	 mov edi,[Scroll1MaxSizeX]
	 sub edi,ecx
	 mov [Scroll1FreeX],edi
	 call draw_volume_rectangle

	 mov eax,[Window_SizeX]
	 sub eax,22+10
	 mov ebx,[Scroll2CoordinatY]
	 mov ecx,14+10
	 mov edx,[Window_SizeY]
	 sub edx,20+15+1+46+10+20+11
	 mov esi,1
	 mov [Scroll2CoordinatX],eax
	 mov [Scroll2CoordinatY],ebx
	 mov [Scroll2MaxSizeX],ecx
	 mov [Scroll2MaxSizeY],edx

	 imul edx,[CounterY]
	 push eax
	 push ebx
	 mov eax,edx
	 mov ebx,[Picture_SizeY]
	 cdq
	 idiv ebx
	 mov edx,eax
	 pop ebx
	 pop eax
	 mov [Scroll2SizeY],edx
	 mov edi,[Scroll2MaxSizeY]
	 sub edi,edx
	 mov [Scroll2FreeY],edi

	 call draw_volume_rectangle

	 ret
;----------------------------------------------------------
;--------change size of memory which use program-----------
;----------------------------------------------------------
GetMemory:
	 pushad
	 mov eax,64
	 mov ebx,1
	 mov ecx,[Picture_SizeX]
	 mov edx,[Picture_SizeY]
	 imul ecx,edx
	 lea ecx,[ecx+ecx*2]
	 lea ecx,[ecx+ecx*4]		;(Picture_SizeX*Picture_SizeY*3)*5
	 add ecx,[ScreenPointer]
	 add ecx,(1200*1000)*3+30*(20*20*3)+500000+16000+0x4000
	 add ecx,[extended_memory]
	 int 0x40

	 test eax,eax
	 jz have_new_memory

	    mov esi,sound_havent_memory
	    call sound

	    jmp still

	 have_new_memory:
	 popad
	 ret
;-----------------------------------------------------------
;-----------instruments of menu-----------------------------
;-----------------------------------------------------------
TakeInstruments:

	 ;|||||||||||||||||||FILE||||||||||||||||||||||
	 cmp [number_panel],7
	 jne no_file

	 ;<<<<<NEW>>>>>>>
	 cmp [number_menu],1
	 jne no_new

	 mov eax,80
	 mov ebx,100
	 mov ecx,150
	 mov edx,90
	 mov  esi,1
	 call draw_volume_rectangle

	 mov eax,177
	 mov ebx,115
	 mov ecx,40
	 mov edx,14
	 mov  esi,4
	 call draw_volume_rectangle

	 mov eax,177
	 mov ebx,115+24
	 mov ecx,40
	 mov edx,14
	 mov  esi,4
	 call draw_volume_rectangle

	 mov eax,175
	 mov ebx,170
	 mov ecx,40
	 mov edx,15
	 mov  esi,1
	 call draw_volume_rectangle

	 mov eax,90
	 mov ebx,118
	 mov ecx,0xffffff
	 mov edx,new_text1
	 mov esi,14
	 call print_text

	 mov eax,90
	 mov ebx,118+24
	 mov ecx,0xffffff
	 mov edx,new_text2
	 mov esi,14
	 call print_text

	 mov eax,185
	 mov ebx,175
	 mov ecx,0xffffff
	 mov edx,ok_text
	 mov esi,2
	 call print_text

	 mov eax,8
	 mov ebx,177*65536+40
	 mov ecx,115*65536+14
	 mov edx,1000000000000000000000000000000b
	 add edx,1000
	 int 0x40

	 mov eax,8
	 mov ebx,177*65536+40
	 mov ecx,139*65536+14
	 mov edx,1000000000000000000000000000000b
	 add edx,1001
	 int 0x40

	 mov eax,8
	 mov ebx,175*65536+40
	 mov ecx,170*65536+15
	 mov edx,1000000000000000000000000000000b
	 add edx,1002
	 int 0x40

	 no_activate_space:

	 mov eax,10
	 int 0x40

	 cmp eax,1
	 je exit_new

	 cmp eax,3
	 jne no_activate_space

	 mov eax,17
	 int 0x40
	 shr eax,8

	 cmp eax,1000
	 jl no_activate_space

	 cmp eax,1000
	 jne no_picture_size_x

	 mov eax,180
	 mov ebx,119
	 mov ecx,5
	 call dialog_line

	 mov eax,string_
	 mov ebx,13
	 call find_symvol

	 dec eax
	 mov [length_number],eax
	 mov eax,string_
	 call value

	 mov [Picture_SizeX],eax

	 jmp no_activate_space

	 no_picture_size_x:

	 cmp eax,1001
	 jne no_picture_size_y

	 mov eax,180
	 mov ebx,119+24
	 mov ecx,5
	 call dialog_line

	 mov eax,string_
	 mov ebx,13
	 call find_symvol

	 dec eax
	 mov [length_number],eax
	 mov eax,string_
	 call value

	 mov [Picture_SizeY],eax

	 jmp no_activate_space

	 no_picture_size_y:

	 cmp eax,1002
	 jne no_activate_space

	 exit_new:

	 mov eax,8
	 mov ebx,175*65536+40
	 mov ecx,170*65536+15
	 mov edx,11100000000000000000000000000000b
	 add edx,1002
	 int 0x40

	 mov eax,8
	 mov ebx,177*65536+40
	 mov ecx,139*65536+14
	 mov edx,11100000000000000000000000000000b
	 add edx,1001
	 int 0x40

	 mov eax,8
	 mov ebx,177*65536+40
	 mov ecx,115*65536+14
	 mov edx,11100000000000000000000000000000b
	 add edx,1000
	 int 0x40

	 ;get memory for new picture
	 mov eax,[Picture_SizeX]
	 mov ebx,[Picture_SizeY]

	 imul eax,ebx
	 lea eax,[eax+eax*2]

	 mov ebx,[ScreenPointer]
	 add ebx,(1200*1000*3)+30*(20*20*3)+500000

	 mov [PointerToPicture],ebx
	 mov [PointerToCopyPicture],ebx
	 mov [PointerToCopyPicture2],ebx
	 mov [PointerToEditBufer],ebx
	 mov [PointerToSpriteBufer],ebx

	 add [PointerToCopyPicture],eax

	 add [PointerToCopyPicture2],eax
	 add [PointerToCopyPicture2],eax

	 add [PointerToEditBufer],eax
	 add [PointerToEditBufer],eax
	 add [PointerToEditBufer],eax

	 add [PointerToSpriteBufer],eax
	 add [PointerToSpriteBufer],eax
	 add [PointerToSpriteBufer],eax
	 add [PointerToSpriteBufer],eax

	 call GetMemory

	 and [save_flag],0
	 and [number_panel],0
	 and [number_menu],0

	 call cleare_work_arrea
	 call drawwin

	 jmp still
	 no_new:

	 ;<<<<<OPEN>>>>>>>
	 cmp [number_menu],2
	 jne no_open

	 opendialog drawwin,open_1,open_2,file_path
	 open_1:

	 mov eax,40
	 mov ebx,100111b
	 int 0x40

	 mov eax,[ScreenPointer]
	 add eax,0x10000
	 mov [file_info],dword 0
	 mov [file_info+8],dword 1
	 mov [file_info+12],eax
	 mov eax,58
	 mov ebx,file_info
	 int 0x40

	 mov esi,[ScreenPointer]
	 add esi,0x10000
	 ;-------------is this BMP file ?----------------
	 xor eax,eax
	 mov ax,[esi]
	 mov [type],ax

	 cmp [type],word 'BM'
	 jne no_bmp_file

	 xor eax,eax
	 xor ebx,ebx
	 mov eax,[esi+18]
	 mov ebx,[esi+22]
	 mov [Picture_SizeX],eax
	 mov [Picture_SizeY],ebx

	 jmp exit_type

	 no_bmp_file:

	 ;-------------is this GIF file ?----------------
	 xor eax,eax
	 mov ax,[esi]
	 mov [type],ax

	 cmp [type],'GI'
	 jne no_gif_file

	 add esi,6
	 xor eax,eax
	 xor ebx,ebx
	 mov ax,word[esi]
	 mov bx,word[esi+2]
	 mov [Picture_SizeX],eax
	 mov [Picture_SizeY],ebx

	 jmp exit_type

	 no_gif_file:

	 jmp no_unpakcing_gif_file

	 exit_type:
	 ;----------------------------------------------------------
	 ;Get momory for unpacking picture and for picture's bufers
	 ;----------------------------------------------------------
	 mov eax,[Picture_SizeX]
	 mov ebx,[Picture_SizeY]

	 imul eax,ebx
	 lea eax,[eax+eax*2]

	 mov ebx,[ScreenPointer]
	 add ebx,(1200*1000*3)+30*(20*20*3)+500000

	 mov [PointerToPicture],ebx
	 mov [PointerToCopyPicture],ebx
	 mov [PointerToCopyPicture2],ebx
	 mov [PointerToEditBufer],ebx
	 mov [PointerToSpriteBufer],ebx

	 add [PointerToCopyPicture],eax

	 add [PointerToCopyPicture2],eax
	 add [PointerToCopyPicture2],eax

	 add [PointerToEditBufer],eax
	 add [PointerToEditBufer],eax
	 add [PointerToEditBufer],eax

	 add [PointerToSpriteBufer],eax
	 add [PointerToSpriteBufer],eax
	 add [PointerToSpriteBufer],eax
	 add [PointerToSpriteBufer],eax

	 call GetMemory
	 ;----------------------------------------------------------
	 ;--------------------Load file in memory-------------------
	 ;----------------------------------------------------------

	 mov eax,[PointerToCopyPicture]
	 mov ebx,[ScreenPointer]
	 add eax,1000
	 mov [file_info],dword 0
	 mov [file_info+8],dword 1
	 mov [file_info+12],dword eax
	 mov [file_info+16],dword ebx

	 call load_file

	 ;----------------------------------------------------------
	 ;-------------------Unpacking picture----------------------
	 ;----------------------------------------------------------
	 mov esi,[PointerToCopyPicture]
	 add esi,1000
	 mov edi,[PointerToPicture]
	 mov eax,[ScreenPointer]

	 cmp [type],'BM'
	 jne no_unpakcing_bmp_file
	    ;BMP DECODER
	    call bmptoimg

	 no_unpakcing_bmp_file:


	 cmp [type],'GI'
	 jne no_unpakcing_gif_file
	   ;GIF DECODER
	   sub edi,12
	   call ReadGIF

	 no_unpakcing_gif_file:

	 mov [Scroll1CoordinatX],9
	 mov [Scroll2CoordinatY],85
	 call load_icons
	 call drawwin
	 and [number_panel],0
	 and [number_menu],0
	 mov [save_flag],1

	 open_2:

	 jmp still

	 no_open:

	 ;<<<<<<<<<<<SAVE>>>>>>>>>>>>
	 cmp [number_menu],3
	 jne no_save_

	 mov al,[save_flag]

	 test al,al
	 jz no_save_file

	 jmp save_enter

	 no_save_file:

	 and [number_panel],0
	 and [number_menu],0

	 jmp still

	 no_save_:

	 ;<<<<<<<<<SAVE AS>>>>>>>>>>>
	 cmp [number_menu],4
	 jne no_save

	 savedialog drawwin,save1,save2,file_path

	 save1:

	 save_enter:
	 mov eax,40
	 mov ebx,100111b
	 int 0x40

	 call analizing_picture_to_palette
	 ;eax => number of colors in picture
	 mov ebx,[PointerToPicture]
	 mov ecx,[PointerToEditBufer]
	 mov edx,[PointerToPalette]
	 mov esi,[Picture_SizeX]
	 mov edi,[Picture_SizeY]

	 call coding_bmp
	 mov edi,[PointerToEditBufer]
	 mov [file_info],dword 1
	 mov [file_info+8],dword ebx
	 mov [file_info+12],edi

	 mov eax,58
	 mov ebx,file_info
	 int 0x40

	 mov [save_flag],1
	 and [number_panel],0
	 and [number_menu],0
	 call drawwin

	 jmp still

	 save2:

	 and [number_panel],0
	 and [number_menu],0

	 jmp still
	 no_save:

	 ;<<<<<<EXIT>>>>>>>
	 cmp [number_menu],5
	 jne no_exit_program

	 mov eax,-1
	 int 0x40

	 no_exit_program:

	 no_file:

	 ;|||||||||||||||||||||||||||||EDIT|||||||||||||||||||||||||

	 ;<<<<<<<<<<UNDO>>>>>>>>>>
	 cmp [number_panel],6
	 jne no_edit

	 cmp [number_menu],1
	 jne no_undo

	 cmp [number_undo],1
	 jne no_one____

	 mov esi,[PointerToCopyPicture]

	 no_one____:

	 cmp [number_undo],2
	 jne no_two____

	 mov esi,[PointerToCopyPicture2]

	 no_two____:

	 mov edi,[PointerToPicture]
	 mov ecx,[Picture_SizeX]
	 imul ecx,[Picture_SizeY]
	 lea ecx,[ecx+ecx*2]
	 add ecx,4
	 shr ecx,2
	 cld
	 rep movsd
	 call MovePictureToWorkScreen

	 dec [number_undo]
	 jnz no_null_undo

	 mov [number_undo],1

	 no_null_undo:

	 and [number_panel],0
	 and [number_menu],0
	 jmp still
	 no_undo:

	 ;<<<<<<<<<<COPY>>>>>>>>>>

	 cmp [number_menu],2
	 jne no_copy

	 cmp [instrument_used],1
	 jne no_copy
	 cmp [Activate_instrument],0
	 jne no_copy

	mov eax,[OldX]
	mov ebx,[OldY]

	cmp eax,[rectangular_shade_x]
	jl no_remove_x_copy

	mov ecx,[rectangular_shade_x]
	mov [OldX],ecx			  ; OldX <-----> rectangulare_shade_x
	mov [rectangular_shade_x],eax

	no_remove_x_copy:

	cmp ebx,[rectangular_shade_y]
	jl no_remove_y_copy

	mov ecx,[rectangular_shade_y]
	mov [OldY],ecx			  ; OldY <-----> rectangulare_shade_y
	mov [rectangular_shade_y],ebx

	no_remove_y_copy:

	 mov eax,[OldX]
	 mov ebx,[OldY]
	 mov ecx,[rectangular_shade_x]
	 mov edx,[rectangular_shade_y]
	 inc eax
	 inc ebx
	 dec ecx
	 dec edx

	 mov [x],eax
	 mov [y],ebx
	 mov esi,eax
	 mov edi,ebx
	 mov [Dx_],1
	 mov [Dy_],1

	 sub ecx,eax
	 jnz no_signum_fill_r_x_copy

	 mov [Dx_],-1

	 no_signum_fill_r_x_copy:

	 sub edx,ebx
	 jnz no_signum_fill_r_y_copy

	 mov [Dy_],-1

	 no_signum_fill_r_y_copy:

	 mov ebx,[rectangular_shade_y]
	 sub ebx,edi

	 mov edx,[PointerToEditBufer]
	 mov [y],edi
	 loop_fill_rectangle_y_copy:

	 mov [x],esi
	 loop_fill_rectangle_x_copy:

	 push esi
	 push edi

	 mov eax,[PointerToPicture]
	 mov ebx,[Picture_SizeX]
	 mov esi,[x]
	 mov edi,[y]
	 call GetColorOfPixel

	 mov [edx],ax
	 shr eax,16
	 mov [edx+2],al

	 pop edi
	 pop esi

	 add edx,3

	 mov eax,[x]
	 add eax,[Dx_]
	 mov [x],eax

	 cmp eax,[rectangular_shade_x]
	 jl loop_fill_rectangle_x_copy

	 mov eax,[y]
	 add eax,[Dy_]
	 mov [y],eax

	 cmp eax,[rectangular_shade_y]
	 jl loop_fill_rectangle_y_copy

	 call MovePictureToWorkScreen

	 and [number_panel],0
	 and [number_menu],0
	 mov [DrawSprite_flag],1

	 jmp still

	 no_copy:

	 ;<<<<<<<<<<PASTE>>>>>>>>
	 cmp [number_menu],3
	 jne no_paste

	 cmp [instrument_used],1
	 jne no_paste

	 cmp [Activate_instrument],0
	 jne no_paste

	 mov eax,[OldX]
	 mov ebx,[OldY]

	 cmp eax,[rectangular_shade_x]
	 jl no_remove_x_paste

	 mov ecx,[rectangular_shade_x]
	 mov [OldX],ecx 		   ; OldX <-----> rectangulare_shade_x
	 mov [rectangular_shade_x],eax

	 no_remove_x_paste:

	 cmp ebx,[rectangular_shade_y]
	 jl no_remove_y_paste

	 mov ecx,[rectangular_shade_y]
	 mov [OldY],ecx 		   ; OldY <-----> rectangulare_shade_y
	 mov [rectangular_shade_y],ebx

	 no_remove_y_paste:

	 mov eax,[OldX]
	 mov ebx,[OldY]
	 mov ecx,[rectangular_shade_x]
	 mov edx,[rectangular_shade_y]
	 inc eax
	 inc ebx
	 dec ecx
	 dec edx

	 mov [x],eax
	 mov [y],ebx
	 mov esi,eax
	 mov edi,ebx
	 mov [Dx_],1
	 mov [Dy_],1

	 sub ecx,eax
	 jnz no_signum_fill_r_x_paste

	 mov [Dx_],-1

	 no_signum_fill_r_x_paste:

	 sub edx,ebx
	 jnz no_signum_fill_r_y_paste

	 mov [Dy_],-1

	 no_signum_fill_r_y_paste:

	 mov edx,[PointerToEditBufer]
	 mov [y],edi

	 loop_fill_rectangle_y_paste:

	 mov [x],esi
	 loop_fill_rectangle_x_paste:

	 push esi
	 push edi

	 mov ecx,[edx]
	 and ecx,0xffffff ;color

	 mov eax,[PointerToPicture]
	 mov ebx,[Picture_SizeX]
	 mov esi,[x]
	 mov edi,[y]
	 call PutPixel

	 pop edi
	 pop esi

	 add edx,3

	 mov eax,[x]
	 add eax,[Dx_]
	 mov [x],eax
	 cmp eax,[rectangular_shade_x]
	 jl loop_fill_rectangle_x_paste

	 mov eax,[y]
	 add eax,[Dy_]
	 mov [y],eax
	 cmp eax,[rectangular_shade_y]

	 jl loop_fill_rectangle_y_paste

	 call MovePictureToWorkScreen
	 and [number_panel],0
	 and [number_menu],0
	 mov [Paste_flag],1

	 jmp still

	 no_paste:

	 ;<<<<<<<<<<CUT>>>>>>>>>>
	 cmp [number_menu],4
	 jne no_cut

	 cmp [instrument_used],1
	 jne no_cut

	 cmp [Activate_instrument],0
	 jne no_cut

	 mov eax,[OldX]
	 mov ebx,[OldY]

	 cmp eax,[rectangular_shade_x]
	 jl no_remove_x_cut

	 mov ecx,[rectangular_shade_x]
	 mov [OldX],ecx 		   ; OldX <-----> rectangulare_shade_x
	 mov [rectangular_shade_x],eax

	 no_remove_x_cut:

	 cmp ebx,[rectangular_shade_y]
	 jl no_remove_y_cut

	 mov ecx,[rectangular_shade_y]
	 mov [OldY],ecx 		   ; OldY <-----> rectangulare_shade_y
	 mov [rectangular_shade_y],ebx

	 no_remove_y_cut:

	 mov eax,[OldX]
	 mov ebx,[OldY]
	 mov ecx,[rectangular_shade_x]
	 mov edx,[rectangular_shade_y]
	 inc eax
	 inc ebx
	 dec ecx
	 dec edx

	 mov [x],eax
	 mov [y],ebx
	 mov esi,eax
	 mov edi,ebx
	 mov [Dx_],1
	 mov [Dy_],1

	 sub ecx,eax
	 jnz no_signum_fill_r_x

	 mov [Dx_],-1

	 no_signum_fill_r_x:

	 sub edx,ebx
	 jnz no_signum_fill_r_y

	 mov [Dy_],-1

	 no_signum_fill_r_y:

	 mov [y],edi
	 loop_fill_rectangle_y:

	 mov [x],esi
	 loop_fill_rectangle_x:

	 push esi
	 push edi

	 mov eax,[PointerToPicture]
	 mov ebx,[Picture_SizeX]
	 mov ecx,dword 0xffffff
	 mov esi,[x]
	 mov edi,[y]
	 call PutPixel

	 pop edi
	 pop esi

	 mov eax,[x]
	 add eax,[Dx_]
	 mov [x],eax

	 cmp eax,[rectangular_shade_x]
	 jl loop_fill_rectangle_x

	 mov eax,[y]
	 add eax,[Dy_]
	 mov [y],eax

	 cmp eax,[rectangular_shade_y]
	 jl loop_fill_rectangle_y

	 call MovePictureToWorkScreen

	 and [number_panel],0
	 and [number_menu],0

	 jmp still
	 no_cut:

	 ;<<<<<<CLEARE ALL>>>>>>
	 cmp [number_menu],5
	 jne no_cleare_all

	 call cleare_work_arrea

	 call MovePictureToWorkScreen

	 and [number_panel],0
	 and [number_menu],0

	 jmp still
	 no_cleare_all:
	 ;<<<<<<TO ALLOCATE ALL>>>>>>

	 cmp [number_menu],6
	 jne no_to_allocate_all

	 mov [OldX],1
	 mov [OldY],1
	 mov eax,[Picture_SizeX]
	 mov ebx,[Picture_SizeY]
	 dec eax
	 dec ebx
	 mov [rectangular_shade_x],eax
	 mov [rectangular_shade_y],ebx
	 mov [instrument_used],1
	 mov [Activate_instrument],1
	 mov [Current_instrument],30
	 mov [crossing],0
	 and [number_panel],0
	 and [number_menu],0
	 ;call TakeButtonInstruments
	 ;call MovePictureToWorkScreen
	 jmp still

	 no_to_allocate_all:

	 no_edit:


	 jmp still

	 ret
;-----------------------------------------------------------
;-----instruments of panel(icon's instruments)--------------
;-----------------------------------------------------------
TakeButtonInstruments:

	mov eax,[Current_instrument]

	;*************************brush 1***********************
	cmp eax,10
	jne no_brush1

	mov [Brush_SizeX],4
	mov [Brush_SizeY],4
	mov [Number_Brush],0
	mov [Current_instrument],18

	jmp still
	no_brush1:

	;*************************brush 2***********************
	cmp eax,11
	jne no_brush2

	mov [Brush_SizeX],6
	mov [Brush_SizeY],4
	mov [Number_Brush],1
	mov [Current_instrument],18

	jmp still
	no_brush2:

	;*************************brush 3***********************
	cmp eax,12
	jne no_brush3

	mov [Brush_SizeX],8
	mov [Brush_SizeY],7
	mov [Number_Brush],2
	mov [Current_instrument],18

	jmp still
	no_brush3:

	;************************brush 4************************
	cmp eax,13
	jne no_brush4

	mov [Brush_SizeX],14
	mov [Brush_SizeY],14
	mov [Number_Brush],3
	mov [Current_instrument],18

	jmp still
	no_brush4:

	;************************brush 5************************
	cmp eax,14
	jne no_brush5

	mov [Brush_SizeX],6
	mov [Brush_SizeY],6
	mov [Number_Brush],4
	mov [Current_instrument],18

	jmp still
	no_brush5:

	;*************************pensil************************
	cmp eax,15
	jne no_pensil

	mov al,[exit_from_work_arrea]

	test al,al
	jz no_exit_from_work_arrea

	and [Activate_instrument],0
	and [exit_from_work_arrea],0

	no_exit_from_work_arrea:

	mov al,[Activate_instrument]

	test al,al
	jnz no_activated_later

	call GetScreenCordinats
	mov eax,[ScreenX]
	mov ebx,[ScreenY]
	add eax,[PosX]
	add ebx,[PosY]
	mov [OldX],eax
	mov [OldY],ebx

	no_activated_later:

	mov eax,[MaxWorkScreen_SizeX]
	mov ebx,[MaxWorkScreen_SizeY]

	mov eax,[PointerToPicture]
	mov ebx,[ReserveArray]
	mov ecx,[Picture_SizeX]
	mov edx,[OldX]
	shl edx,16
	add edx,[OldY]
	mov esi,[ScreenX]
	mov edi,[ScreenY]
	add esi,[PosX]
	add edi,[PosY]
	call calculate_line

	mov ecx,eax
	mov edx,[Color]
	mov esi,[Color]
	shr edx,16
	xor ebx,ebx
	mov eax,[ReserveArray]

	pensil_next_pixel_draw:

	  mov ebx,[eax]
	  mov [ebx],si
	  mov [ebx+2],dl
	  add eax,4

	dec ecx
	jnz pensil_next_pixel_draw

	mov eax,[ScreenX]
	mov ebx,[ScreenY]
	add eax,[PosX]
	add ebx,[PosY]
	mov [OldX],eax
	mov [OldY],ebx

	call MovePictureToWorkScreen
	mov [Activate_instrument],1
	jmp still
	no_pensil:

	;**********************pipetka**************************
	cmp eax,16
	jne no_pipetka
	mov eax,[ScreenY]
	mov ebx,[Picture_SizeX]
	add eax,[PosY]
	imul eax,ebx
	add eax,[ScreenX]
	add eax,[PosX]
	lea eax,[eax+eax*2]
	add eax,[PointerToPicture]
	mov ebx,[eax]
	and ebx,0xffffff
	mov [Color],ebx
	mov eax,13
	mov ebx,447*65536+35
	mov ecx,42*65536+36
	mov edx,[Color]
	int 0x40
	jmp still
	no_pipetka:

	;**********************draw brush***********************
	cmp eax,18
	jne no_brush

	jmp no_lastik_

	lastik_in:
	mov eax,[Color]
	mov [SColor],eax
	mov [Color],0xffffff
	no_lastik_:

	mov eax,[ScreenX]
	mov ebx,[ScreenY]
	mov ecx,[Brush_SizeX]
	mov edx,[Brush_SizeY]
	add eax,[PosX]
	add ebx,[PosY]
	add eax,ecx
	add ebx,edx

	cmp eax,[Picture_SizeX]
	jl no_max_pos_x
	mov eax,[Picture_SizeX]
	no_max_pos_x:

	cmp ebx,[Picture_SizeY]
	jl no_max_pos_y
	mov ebx,[Picture_SizeY]
	no_max_pos_y:

	cmp eax,ecx
	ja no_min_pos_x
	mov eax,[Brush_SizeX]
	no_min_pos_x:

	cmp ebx,edx
	ja no_min_pos_y
	mov ebx,[Brush_SizeY]
	no_min_pos_y:

	sub eax,[Brush_SizeX]
	sub ebx,[Brush_SizeY]

	mov [x],eax
	mov [y],ebx

	mov al,[exit_from_work_arrea]
	test al,al
	jz no_exit_from_work_arrea_brush
	and [Activate_instrument],0
	and [exit_from_work_arrea],0
	no_exit_from_work_arrea_brush:

	mov al,[Activate_instrument]
	test al,al
	jnz no_new_brush_xy
	mov eax,[x]
	mov ebx,[y]
	mov [OldX],eax
	mov [OldY],ebx
	mov [Activate_instrument],1
	no_new_brush_xy:

	mov eax,[PointerToPicture]
	mov ebx,[ReserveArray]
	add ebx,4
	mov ecx,[Picture_SizeX]
	mov edx,[OldX]
	shl edx,16
	add edx,[OldY]
	mov esi,[x]
	mov edi,[y]

	call calculate_line

	mov ebx,[ReserveArray]
	mov [ebx],eax

	;procedure drawing of brush
	mov ebx,[ReserveArray]
	mov ecx,[ebx]
	mov edx,[Color]
	mov esi,[Color]
	shr edx,16
	add ebx,4
	mov edi,[Number_Brush]
	imul edi,20*20

	next_pixel_put_brush:

	mov eax,[ebx]
	push eax
	push ecx
	xor ebp,ebp
	and [counter2],0

	vertical_width_brush:
	   and [counter],0

	   horizontal_width_brush:
	   xor ecx,ecx
	   mov cl,byte[Brush_color+edi+ebp]
	   test cl,cl
	     jz no_draw_pixel_brush
	      mov [eax],si
	      mov [eax+2],dl
	     no_draw_pixel_brush:
	   add eax,3
	   inc ebp
	   inc [counter]
	   cmp [counter],20
	   jne horizontal_width_brush

	   mov ecx,[Picture_SizeX]
	   sub ecx,20
	   lea ecx,[ecx+ecx*2]
	   add eax,ecx
	inc [counter2]
	cmp [counter2],20
	jne vertical_width_brush

	pop ecx
	pop eax
	add ebx,4
	dec ecx
	jnz next_pixel_put_brush

	mov eax,[x]
	mov ebx,[y]
	mov [OldX],eax
	mov [OldY],ebx

	mov al,[lastik_is_active]
	test al,al
	jz no_lastik_active

	mov eax,[SColor]
	mov [Color],eax
	and [lastik_is_active],0
	no_lastik_active:

	call MovePictureToWorkScreen
	jmp still
	no_brush:

	;************************Flood Fill*******************
	cmp eax,17
	jne no_FloodFill

	mov eax,[PointerToPicture]
	mov ebx,[PointerToEditBufer]

	mov ecx,[Picture_SizeX]
	imul ecx,[Picture_SizeY]
	lea ecx,[ecx+ecx*2]
	shl ecx,1

	cmp ecx,500000
	ja normal_size_of_bufer

	mov ebx,[ReserveArray]

	normal_size_of_bufer:

	add ebx,4
	mov ecx,[Picture_SizeX]
	mov edx,[ScreenX]
	add edx,[PosX]
	shl edx,16
	add edx,[ScreenY]
	add edx,[PosY]
	mov esi,[Picture_SizeX]
	dec esi
	shl esi,16
	add esi,[Picture_SizeY]
	dec esi
	mov edi,[Color]

	call flood_fill
	call MovePictureToWorkScreen

	jmp still
	no_FloodFill:

	;************************lastik*************************
	cmp eax,19
	jne no_lastik

	 mov [lastik_is_active],1
	jmp lastik_in

	no_lastik:

	;******************************************************
	cmp eax,20
	jne  no_spray

	cmp [Activate_instrument],0
	jne no_null_spray
	mov [Activate_instrument],1
	jmp still
	no_null_spray:

	mov eax,[ScreenX]
	mov ebx,[ScreenY]
	mov ecx,[Brush_SizeX]
	mov edx,[Brush_SizeY]
	add eax,[PosX]
	add ebx,[PosY]
	add eax,ecx
	add ebx,edx

	cmp eax,[Picture_SizeX]
	jl no_max_pos_x_spray
	mov eax,[Picture_SizeX]
	no_max_pos_x_spray:

	cmp ebx,[Picture_SizeY]
	jl no_max_pos_y_spray
	mov ebx,[Picture_SizeY]
	no_max_pos_y_spray:

	cmp eax,ecx
	ja no_min_pos_x_spray
	mov eax,[Brush_SizeX]
	no_min_pos_x_spray:

	cmp ebx,edx
	ja no_min_pos_y_spray
	mov ebx,[Brush_SizeY]
	no_min_pos_y_spray:

	sub eax,[Brush_SizeX]
	sub ebx,[Brush_SizeY]
	mov edi,0;[Number_Brush]
	imul edi,20*20

	mov [x],eax
	mov [y],ebx
	mov ebp,[Picture_SizeX]
	xor edx,edx
	brush_y_spray:
	  xor ecx,ecx
	  brush_x_spray:
	   ;calculate position in array of spray
	   mov esi,edx
	   lea esi,[esi+esi*4] ;esi=esi*5
	   shl esi,2	       ;esi=(esi*3)*4
	   add esi,ecx
	   add esi,edi
	   ;read byte from array
	   xor eax,eax
	   mov al,[Spray_color+esi]
	   test eax,eax
	   jz no_color_spray
	   mov eax,[x]
	   mov ebx,[y]
	   add eax,ecx
	   add ebx,edx
	   imul ebx,ebp; ebp=[Picture_SizeX]
	   add eax,ebx
	   lea eax,[eax+eax*2]
	   add eax,[PointerToPicture]
	   mov ebx,[Color]
	   mov [eax],bx
	   shr ebx,16
	   mov [eax+2],bl
	   no_color_spray:
	   inc ecx
	   cmp ecx,20
	  jl brush_x_spray
	inc edx
	cmp edx,20
	jl brush_y_spray

	call MovePictureToWorkScreen
	jmp still

	no_spray:

	;***********************palette*************************
	cmp eax,21
	jne no_palette

	mov eax,20
	mov ebx,100
	mov ecx,32*10+32*5+7
	mov edx,8*10+7*10
	mov  esi,1
	call draw_volume_rectangle
	mov [x],20+5
	mov [y],100+10
	mov edi,256
	xor esi,esi
	next_rectangle:
	mov eax,13
	mov ebx,[x]
	mov ecx,[y]
	mov edx,[palitra+esi]
	and edx,0xffffff
	shl ebx,16
	shl ecx,16
	add ebx,13
	add ecx,13
	int 0x40
	add [x],15
	cmp [x],20+15*32
	jl no_new_line
	mov [x],20+5
	add [y],15
	no_new_line:
	add esi,4
	dec edi
	jnz next_rectangle

	wait_events:
	call event

	cmp eax,1
	je still
	cmp eax,2
	jne no_keys
	mov eax,2
	int 0x40
	no_keys:
	cmp eax,3
	jne no_buttons
	mov eax,17
	int 0x40
	no_buttons:
	cmp eax,6
	jne wait_events

	call GetMouseClick

	test eax,eax
	jz wait_events
	call GetMouseCoordinats
	mov [x],20+5
	mov [y],100+10
	mov [counter],0
	next_rectangle_column:
	mov eax,[x]
	mov ebx,[y]
	mov ecx,[MouseX]
	mov edx,[MouseY]
	mov esi,13
	mov edi,13
	call columnus
	test eax,eax
	jz no_columnus_color
	mov eax,[counter]
	shl eax,2
	mov ebx,[palitra+eax]
	and ebx,0xffffff
	mov [Color],ebx
	no_columnus_color:
	add [x],15
	cmp [x],20+15*32
	jl no_new_line_column
	mov [x],20+5
	add [y],15
	no_new_line_column:
	inc [counter]
	cmp [counter],257
	jne next_rectangle_column
	mov eax,5
	mov ebx,10
	int 0x40
	call drawwin
	and [Current_instrument],0
	jmp still
	no_palette:

	;************************draw line**********************
	cmp eax,22
	jne no_line

	mov al,[Activate_instrument]
	test al,al
	jnz no_new_line_xy
	mov eax,[ScreenX]
	mov ebx,[ScreenY]
	add eax,[PosX]
	add ebx,[PosY]
	mov [OldX],eax
	mov [OldY],ebx
	mov [Activate_instrument],1
	mov eax,[ReserveArray]
	mov ecx,60000
	clear_array_line:
	mov [eax],dword 0
	add eax,4
	dec ecx
	jnz clear_array_line
	jmp still
	no_new_line_xy:

	;put saved pixels
	mov ebx,[ReserveArray]
	mov eax,[ebx]
	test eax,eax
	jz no_put_line_to_screen_line
	mov ecx,[ebx]
	add ebx,4
	xor ebp,ebp
	next_color_put_line:
	;put saved pixels in ReserveArray
	push ecx
	and [counter2],0
	mov edi,[ebx]
	push edi
	vertical_width_put:
	   and [counter],0

	   horizontal_width_put:
	      mov edx,[ebx+4807*4+ebp]
	      and edx,0xffffff
	      mov [edi],dx
	      shr edx,16
	      mov [edi+2],dl

	   add edi,3
	   add ebp,4
	   inc [counter]
	   cmp [counter],5
	   jne horizontal_width_put

	   mov ecx,[Picture_SizeX]
	   sub ecx,5
	   lea ecx,[ecx+ecx*2]
	   add edi,ecx
	inc [counter2]
	cmp [counter2],5
	jne vertical_width_put
	pop edi
	pop ecx
	add ebx,4
	dec ecx
	jnz next_color_put_line

	no_put_line_to_screen_line:

	;calculate line
	mov ebx,[ReserveArray]
	add ebx,4
	mov eax,[PointerToPicture]
	mov ecx,[Picture_SizeX]
	mov edx,[OldX]
	shl edx,16
	add edx,[OldY]
	mov esi,[ScreenX]
	mov edi,[ScreenY]
	add esi,[PosX]
	add edi,[PosY]

	mov ebp,[Picture_SizeX]
	sub ebp,[line_width]
	cmp esi,ebp
	jl no_minimum_x_line
	mov esi,ebp
	no_minimum_x_line:

	mov ebp,[Picture_SizeY]
	sub ebp,[line_width]
	cmp edi,ebp
	jl no_minimum_y_line
	mov edi,ebp
	no_minimum_y_line:

	call calculate_line
	;call calculate_rectangle
	mov [counter],eax

	;save color pixels in ReserveArray
	mov eax,[counter]
	mov ebx,[ReserveArray]
	mov [ebx],eax

	mov ecx,[ebx]
	add ebx,4

	xor ebp,ebp
	next_color_save_line:
	;save color of pixel in ReserveArray
	push ecx
	and [counter2],0
	mov edi,[ebx]
	vertical_width_save:
	   and [counter],0

	   horizontal_width_save:
	      mov eax,edi
	      mov edx,[eax]
	      and edx,0xffffff
	      mov [ebx+4807*4+ebp],dx
	      shr edx,16
	      mov [ebx+4807*4+2+ebp],dl

	   add edi,3
	   add ebp,4
	   inc [counter]
	   cmp [counter],5
	   jne horizontal_width_save

	   mov ecx,[Picture_SizeX]
	   sub ecx,5
	   lea ecx,[ecx+ecx*2]
	   add edi,ecx
	inc [counter2]
	cmp [counter2],5
	jne vertical_width_save
	pop ecx
	add ebx,4
	dec ecx
	jnz next_color_save_line

	;draw calculated pixels on work arrea
	mov ebx,[ReserveArray]
	mov ecx,[ebx]
	mov edx,[Color]
	mov esi,[Color]
	shr edx,16
	add ebx,4
	mov edi,[line_width]
	dec edi
	imul edi,25
	next_pixel_put_line:

	mov eax,[ebx]
	push eax
	push ecx
	xor ebp,ebp
	and [counter2],0

	vertical_width:
	   and [counter],0

	   horizontal_width:
	   xor ecx,ecx
	   mov cl,byte[width_pixels+edi+ebp]
	   test cl,cl
	     jz no_draw_pixel_line
	      mov [eax],si
	      mov [eax+2],dl
	     no_draw_pixel_line:
	   add eax,3
	   inc ebp
	   inc [counter]
	   cmp [counter],5
	   jne horizontal_width

	   mov ecx,[Picture_SizeX]
	   sub ecx,5
	   lea ecx,[ecx+ecx*2]
	   add eax,ecx
	inc [counter2]
	cmp [counter2],5
	jne vertical_width

	pop ecx
	pop eax
	add ebx,4
	dec ecx
	jnz next_pixel_put_line

	call MovePictureToWorkScreen

	jmp still
	no_line:

	;*********************************DRAW RECTANGLE****************************
	cmp eax,23
	jne no_rectangle

	mov al,[Activate_instrument]
	test al,al
	jnz no_new_rectangle_xy
	mov eax,[ScreenX]
	mov ebx,[ScreenY]
	add eax,[PosX]
	add ebx,[PosY]
	mov [OldX],eax
	mov [OldY],ebx
	mov [Activate_instrument],1
	mov eax,[ReserveArray]
	mov ecx,60000
	clear_array_rectangle:
	mov [eax],dword 0
	add eax,4
	dec ecx
	jnz clear_array_rectangle
	jmp still
	no_new_rectangle_xy:

	;put saved pixels
	mov ebx,[ReserveArray]
	mov eax,[ebx]
	test eax,eax
	jz no_put_rectangle_to_screen_line
	mov ecx,[ebx]
	add ebx,4
	xor ebp,ebp
	next_color_put_rectangle:
	;put saved pixels in ReserveArray
	push ecx
	and [counter2],0
	mov edi,[ebx]
	push edi
	vertical_width_put_rectangle:
	   and [counter],0

	   horizontal_width_put_rectangle:
	      mov edx,[ebx+4807*4+ebp]
	      and edx,0xffffff
	      mov [edi],dx
	      shr edx,16
	      mov [edi+2],dl

	   add edi,3
	   add ebp,4
	   inc [counter]
	   cmp [counter],5
	   jne horizontal_width_put_rectangle

	   mov ecx,[Picture_SizeX]
	   sub ecx,5
	   lea ecx,[ecx+ecx*2]
	   add edi,ecx
	inc [counter2]
	cmp [counter2],5
	jne vertical_width_put_rectangle
	pop edi
	pop ecx
	add ebx,4
	dec ecx
	jnz next_color_put_rectangle

	no_put_rectangle_to_screen_line:

	;calculate line
	mov ebx,[ReserveArray]
	add ebx,4
	mov eax,[PointerToPicture]
	mov ecx,[Picture_SizeX]
	mov edx,[OldX]
	shl edx,16
	add edx,[OldY]
	mov esi,[ScreenX]
	mov edi,[ScreenY]
	add esi,[PosX]
	add edi,[PosY]

	mov ebp,[Picture_SizeX]
	sub ebp,[line_width]
	cmp esi,ebp
	jl no_minimum_x_rectangle
	mov esi,ebp
	no_minimum_x_rectangle:

	mov ebp,[Picture_SizeY]
	sub ebp,[line_width]
	cmp edi,ebp
	jl no_minimum_y_rectangle
	mov edi,ebp
	no_minimum_y_rectangle:

	call calculate_rectangle
	mov [counter],eax

	;save color pixels in ReserveArray
	mov eax,[counter]
	mov ebx,[ReserveArray]
	mov [ebx],eax

	mov ecx,[ebx]
	add ebx,4

	xor ebp,ebp
	next_color_save_rectangle:
	;save color of pixel in ReserveArray
	push ecx
	and [counter2],0
	mov edi,[ebx]
	vertical_width_save_rectangle:
	   and [counter],0

	   horizontal_width_save_rectangle:
	      mov eax,edi
	      mov edx,[eax]
	      and edx,0xffffff
	      mov [ebx+4807*4+ebp],dx
	      shr edx,16
	      mov [ebx+4807*4+2+ebp],dl

	   add edi,3
	   add ebp,4
	   inc [counter]
	   cmp [counter],5
	   jne horizontal_width_save_rectangle

	   mov ecx,[Picture_SizeX]
	   sub ecx,5
	   lea ecx,[ecx+ecx*2]
	   add edi,ecx
	inc [counter2]
	cmp [counter2],5
	jne vertical_width_save_rectangle
	pop ecx
	add ebx,4
	dec ecx
	jnz next_color_save_rectangle

	;draw calculated pixels on work arrea
	mov ebx,[ReserveArray]
	mov ecx,[ebx]
	mov edx,[Color]
	mov esi,[Color]
	shr edx,16
	add ebx,4
	mov edi,[line_width]
	dec edi
	imul edi,25
	next_pixel_put_rectangle:

	mov eax,[ebx]
	push eax
	push ecx
	xor ebp,ebp
	and [counter2],0

	vertical_width_rectangle:
	   and [counter],0

	   horizontal_width_rectangle:
	   xor ecx,ecx
	   mov cl,byte[width_pixels_rectangle+edi+ebp]
	   test cl,cl
	     jz no_draw_pixel_rectangle
	      mov [eax],si
	      mov [eax+2],dl
	     no_draw_pixel_rectangle:
	   add eax,3
	   inc ebp
	   inc [counter]
	   cmp [counter],5
	   jne horizontal_width_rectangle

	   mov ecx,[Picture_SizeX]
	   sub ecx,5
	   lea ecx,[ecx+ecx*2]
	   add eax,ecx
	inc [counter2]
	cmp [counter2],5
	jne vertical_width_rectangle

	pop ecx
	pop eax
	add ebx,4
	dec ecx
	jnz next_pixel_put_rectangle

	call MovePictureToWorkScreen
	jmp still
	no_rectangle:

	;*********************************DRAW CIRCLE****************************
	cmp eax,24
	jne no_circle

	mov al,[Activate_instrument]
	test al,al
	jnz no_new_circle_xy
	mov eax,[ScreenX]
	mov ebx,[ScreenY]
	add eax,[PosX]
	add ebx,[PosY]
	mov [OldX],eax
	mov [OldY],ebx
	mov [Activate_instrument],1
	mov eax,[ReserveArray]
	mov ecx,60000
	clear_array_circle:
	mov [eax],dword 0
	add eax,4
	dec ecx
	jnz clear_array_circle
	jmp still
	no_new_circle_xy:

	;put saved pixels
	mov ebx,[ReserveArray]
	mov eax,[ebx]
	test eax,eax
	jz no_put_line_to_screen_circle
	mov ecx,[ebx]
	add ebx,4
	xor ebp,ebp
	next_color_put_circle:
	;put saved pixels in ReserveArray
	push ecx
	and [counter2],0
	mov edi,[ebx]
	push edi
	vertical_width_put_circle:
	   and [counter],0

	   horizontal_width_put_circle:
	      mov edx,[ebx+4807*4+ebp]
	      and edx,0xffffff
	      mov [edi],dx
	      shr edx,16
	      mov [edi+2],dl

	   add edi,3
	   add ebp,4
	   inc [counter]
	   cmp [counter],5
	   jne horizontal_width_put_circle

	   mov ecx,[Picture_SizeX]
	   sub ecx,5
	   lea ecx,[ecx+ecx*2]
	   add edi,ecx
	inc [counter2]
	cmp [counter2],5
	jne vertical_width_put_circle
	pop edi
	pop ecx
	add ebx,4
	dec ecx
	jnz next_color_put_circle

	no_put_line_to_screen_circle:

	mov esi,[ScreenX]
	mov edi,[ScreenY]
	add esi,[PosX]
	add edi,[PosY]

	sub esi,[OldX]
	jns no_sign_x_circle
	neg esi
	shr esi,1
	neg esi
	add esi,[OldX]
	jmp no_plus_sign_x_circle
	no_sign_x_circle:

	shr esi,1
	add esi,[OldX]
	no_plus_sign_x_circle:

	sub edi,[OldY]
	jns no_sign_y_circle
	neg edi
	shr edi,1
	neg edi
	add edi,[OldY]
	jmp no_plus_sign_y_circle
	no_sign_y_circle:

	shr edi,1
	add edi,[OldY]
	no_plus_sign_y_circle:

	mov [x],esi
	mov [y],edi

	mov esi,[ScreenX]
	mov edi,[ScreenY]
	add esi,[PosX]
	add edi,[PosY]

	sub esi,[OldX]
	jns no_sign_x_circle_r
	neg esi
	no_sign_x_circle_r:

	sub edi,[OldY]
	jns no_sign_y_circle_r
	neg edi
	no_sign_y_circle_r:

	mov [Dx_],esi
	mov [Dy_],edi

	;finit
	fild [Dx_]
	fmul st0,st0
	fild [Dy_]
	fmul st0,st0
	fadd st0,st1
	fsqrt
	fistp [Radius]
	fistp [Icon_X]
	mov esi,[Radius]
	shr esi,1
	cmp esi,0
	jne no_null_radius
	mov [Radius],1
	no_null_radius:

	mov [Radius],esi

	mov edi,[x]
	mov ebp,[y]
	add edi,esi
	add ebp,esi

	mov edx,[Picture_SizeX]
	sub edx,[line_width]
	cmp edi,edx
	jl no_limit_x_circle
	sub edi,edx
	sub [Radius],edi
	no_limit_x_circle:

	mov edx,[Picture_SizeY]
	sub edx,[line_width]
	cmp ebp,edx
	jl no_limit_y_circle
	sub ebp,edx
	sub [Radius],ebp
	no_limit_y_circle:


	mov edi,[x]
	mov ebp,[y]

	sub edi,[Radius]
	jns no_minimum_x_circle
	add [Radius],edi
	no_minimum_x_circle:

	sub ebp,[Radius]
	jns no_minimum_y_circle
	add [Radius],ebp
	no_minimum_y_circle:

	;calculate circle
	mov ebx,[ReserveArray]
	add ebx,4
	mov eax,[PointerToPicture]
	mov ecx,[Picture_SizeX]
	mov edx,[x]
	shl edx,16
	add edx,[y]
	mov esi,[Radius]
	call calculate_circle

	mov [counter],eax

	;save color pixels in ReserveArray
	mov eax,[counter]
	mov ebx,[ReserveArray]
	mov [ebx],eax

	mov ecx,[ebx]
	add ebx,4

	xor ebp,ebp
	next_color_save_circle:
	;save color of pixel in ReserveArray
	push ecx
	and [counter2],0
	mov edi,[ebx]
	vertical_width_save_circle:
	   and [counter],0

	   horizontal_width_save_circle:
	      mov eax,edi
	      mov edx,[eax]
	      and edx,0xffffff
	      mov [ebx+4807*4+ebp],dx
	      shr edx,16
	      mov [ebx+4807*4+2+ebp],dl

	   add edi,3
	   add ebp,4
	   inc [counter]
	   cmp [counter],5
	   jne horizontal_width_save_circle

	   mov ecx,[Picture_SizeX]
	   sub ecx,5
	   lea ecx,[ecx+ecx*2]
	   add edi,ecx
	inc [counter2]
	cmp [counter2],5
	jne vertical_width_save_circle
	pop ecx
	add ebx,4
	dec ecx
	jnz next_color_save_circle

	;draw calculated pixels on work arrea
	mov ebx,[ReserveArray]
	mov ecx,[ebx]
	mov edx,[Color]
	mov esi,[Color]
	shr edx,16
	add ebx,4
	mov edi,[line_width]
	dec edi
	imul edi,25
	next_pixel_put_circle:

	mov eax,[ebx]
	push eax
	push ecx
	xor ebp,ebp
	and [counter2],0

	vertical_width_circle:
	   and [counter],0

	   horizontal_width_circle:
	   xor ecx,ecx
	   mov cl,byte[width_pixels+edi+ebp]
	   test cl,cl
	     jz no_draw_pixel_circle
	      mov [eax],si
	      mov [eax+2],dl
	     no_draw_pixel_circle:
	   add eax,3
	   inc ebp
	   inc [counter]
	   cmp [counter],5
	   jne horizontal_width_circle

	   mov ecx,[Picture_SizeX]
	   sub ecx,5
	   lea ecx,[ecx+ecx*2]
	   add eax,ecx
	inc [counter2]
	cmp [counter2],5
	jne vertical_width_circle

	pop ecx
	pop eax
	add ebx,4
	dec ecx
	jnz next_pixel_put_circle

	call MovePictureToWorkScreen
	jmp still
	no_circle:

	;************************zoom 1*************************
	cmp eax,25
	jne no_1_
	mov [k],1
	mov [Scroll1CoordinatX],9
	mov [Scroll2CoordinatY],85
	and [Current_instrument],0
	call drawwin
	jmp still
	no_1_:

	;*************************zoom 2************************
	cmp eax,26
	jne no_2_
	mov [k],2
	call drawwin
	mov [Scroll1CoordinatX],9
	mov [Scroll2CoordinatY],85
	and [Current_instrument],0
	jmp still
	no_2_:

	;*************************zoom 4************************
	cmp eax,27
	jne no_4_
	mov [k],4
	call drawwin
	mov [Scroll1CoordinatX],9
	mov [Scroll2CoordinatY],85
	and [Current_instrument],0
	jmp still
	no_4_:

	;************************zoom 8*************************
	cmp eax,28
	jne no_8_
	mov [k],8
	call drawwin
	mov [Scroll1CoordinatX],9
	mov [Scroll2CoordinatY],85
	and [Current_instrument],0
	jmp still
	no_8_:

	;************************zoom 16************************
	cmp eax,29
	jne no_16_
	mov [k],16
	call drawwin
	mov [Scroll1CoordinatX],9
	mov [Scroll2CoordinatY],85
	and [Current_instrument],0
	jmp still
	no_16_:

	;***************allocation of a countour*********************
	cmp eax,30
	jne no_allocation

	cmp [instrument_used],0
	jnz instrument_not_finished_work

	mov al,[Activate_instrument]
	test al,al
	jnz no_new_allocation_xy
	mov eax,[ScreenX]
	mov ebx,[ScreenY]
	add eax,[PosX]
	add ebx,[PosY]
	mov [OldX],eax
	mov [OldY],ebx
	inc eax
	inc ebx
	mov [rectangular_shade_x],eax
	mov [rectangular_shade_y],ebx
	mov [Activate_instrument],1
	mov [instrument_used],1
	and [crossing],0
	and [finishing_crossing],0
	mov eax,[ReserveArray]
	mov ecx,60000
	clear_array_allocation:
	mov [eax],dword 0
	add eax,4
	dec ecx
	jnz clear_array_allocation
	jmp still
	no_new_allocation_xy:
	instrument_not_finished_work:

	mov al,[instrument_used]
	test al,al
	jz no_change_coordinats

	cmp [Activate_instrument],0
	jnz no_change_coordinats

	;save coordinates as old if crossing=0
	cmp [crossing],0
	jnz no_save_coordinate_of_crossing

	mov eax,[OldX]
	mov ebx,[OldY]

	cmp eax,[rectangular_shade_x]
	jl no_remove_x
	mov ecx,[rectangular_shade_x]
	mov [OldX],ecx			  ; OldX <-----> rectangulare_shade_x
	mov [rectangular_shade_x],eax
	no_remove_x:

	cmp ebx,[rectangular_shade_y]
	jl no_remove_y
	mov ecx,[rectangular_shade_y]
	mov [OldY],ecx			  ; OldY <-----> rectangulare_shade_y
	mov [rectangular_shade_y],ebx
	no_remove_y:

	mov eax,[ScreenX]
	mov ebx,[ScreenY]
	add eax,[PosX]
	add ebx,[PosY]
	mov [crossing_old_x],eax
	mov [crossing_old_y],ebx
	mov [crossing],1


	mov eax,[OldX]
	mov ebx,[OldY]
	inc eax
	inc ebx
	mov [SpriteCoordinatX],eax
	mov [SpriteCoordinatY],ebx
	mov [SpriteOldCoordinatX],eax
	mov [SpriteOldCoordinatY],ebx
	mov esi,[rectangular_shade_x]
	mov edi,[rectangular_shade_y]

	sub esi,eax
	jns no_sign_sprite_size_x
	neg esi
	no_sign_sprite_size_x:

	sub edi,ebx
	jns no_sign_sprite_size_y
	neg edi
	no_sign_sprite_size_y:

	test esi,esi
	jnz no_null_sprite_x
	mov esi,1
	no_null_sprite_x:

	test edi,edi
	jnz no_null_sprite_y
	mov edi,1
	no_null_sprite_y:

	mov [SpriteSizeX],esi
	mov [SpriteSizeY],edi

	call SaveFonForSprite

	no_save_coordinate_of_crossing:

	cmp [crossing],0
	je no_test_crossing_with_work_arrea
	;if mouse situatad after allocation than exit
	push [ScreenX]
	push [ScreenY]

	call GetScreenCordinats

	mov eax,[OldX]
	mov ebx,[OldY]
	mov ecx,[ScreenX]
	mov edx,[ScreenY]
	mov esi,[SpriteSizeX]
	mov edi,[SpriteSizeY]
	add ecx,[PosX]
	add edx,[PosY]
	inc esi;eax
	inc edi;ebx
	call columnus

	test eax,eax
	jnz crossing_with_work_area
	mov [finishing_crossing],1
	mov [register],1
	crossing_with_work_area:

	pop [ScreenY]
	pop [ScreenX]

	no_test_crossing_with_work_arrea:

	mov eax,[ScreenX]
	mov ebx,[ScreenY]
	add eax,[PosX]
	add ebx,[PosY]
	mov ecx,eax
	mov edx,ebx
	sub eax,[crossing_old_x]  ;dx=(x-oldx)
	sub ebx,[crossing_old_y]  ;dy=(y-oldy)
	mov [crossing_old_x],ecx
	mov [crossing_old_y],edx

	add [OldX],eax		       ;x1=x1+dx
	add [OldY],ebx		       ;y1=y1+dy
	add [rectangular_shade_x],eax  ;x2=x2+dx
	add [rectangular_shade_y],ebx  ;y2+y2+dy

	mov eax,[OldX]
	mov ebx,[OldY]
	inc eax
	inc ebx
	mov [SpriteCoordinatX],eax
	mov [SpriteCoordinatY],ebx

	cmp [SpriteCoordinatX],0
	jns no_null_sprite_coordinat_x
	mov [SpriteCoordinatX],1
	no_null_sprite_coordinat_x:

	cmp [SpriteCoordinatY],0
	jns no_null_sprite_coordinat_y
	mov [SpriteCoordinatY],1
	no_null_sprite_coordinat_y:

	mov esi,[rectangular_shade_x]
	mov edi,[rectangular_shade_y]

	sub esi,[OldX]
	jns no_znak_size_of_rectangulare_crossing_x
	neg esi
	no_znak_size_of_rectangulare_crossing_x:

	sub edi,[OldY]
	jns no_znak_size_of_rectangulare_crossing_y
	neg edi
	no_znak_size_of_rectangulare_crossing_y:

	mov ecx,[OldX]
	mov edx,[OldY]

	sub ecx,[PosX]
	jns no_minimum_x_crossing
	mov ecx,0
	add ecx,[PosX]
	mov [OldX],ecx
	add ecx,esi
	mov [rectangular_shade_x],ecx
	no_minimum_x_crossing:

	sub edx,[PosY]
	jns no_minimum_y_crossing
	mov edx,0
	add edx,[PosY]
	mov [OldY],edx
	add edx,edi
	mov [rectangular_shade_y],edx
	no_minimum_y_crossing:

	mov ecx,[Picture_SizeX]
	sub ecx,esi
	cmp [OldX],ecx
	jl no_maximum_x_crossing
	dec ecx
	mov [OldX],ecx
	add ecx,esi
	mov [rectangular_shade_x],ecx
	no_maximum_x_crossing:

	mov edx,[Picture_SizeY]
	sub edx,edi
	cmp [OldY],edx
	jl no_maximum_y_crossing
	dec edx
	mov [OldY],edx
	add edx,edi
	mov [rectangular_shade_y],edx
	no_maximum_y_crossing:

	mov eax,[rectangular_shade_x]
	mov ebx,[rectangular_shade_y]
	sub eax,[PosX]
	sub ebx,[PosY]
	mov [ScreenX],eax
	mov [ScreenY],ebx
	no_change_coordinats:

	;put saved pixels
	mov ebx,[ReserveArray]
	mov eax,[ebx]
	test eax,eax
	jz no_put_line_to_screen_allocation
	mov ecx,[ebx]
	add ebx,4
	xor ebp,ebp
	next_color_put_allocation:
	;put saved pixels in ReserveArray
	push ecx
	and [counter2],0
	mov edi,[ebx]
	push edi
	vertical_width_put_allocation:
	   and [counter],0

	   horizontal_width_put_allocation:
	      mov edx,[ebx+4807*4+ebp]
	      and edx,0xffffff
	      mov [edi],dx
	      shr edx,16
	      mov [edi+2],dl

	   add edi,3
	   add ebp,4
	   inc [counter]
	   cmp [counter],1;5
	   jne horizontal_width_put_allocation

	   mov ecx,[Picture_SizeX]
	   sub ecx,1;5
	   lea ecx,[ecx+ecx*2]
	   add edi,ecx
	inc [counter2]
	cmp [counter2],1;5
	jne vertical_width_put_allocation
	pop edi
	pop ecx
	add ebx,4
	dec ecx
	jnz next_color_put_allocation

	no_put_line_to_screen_allocation:

	cmp [DrawSprite_flag],1
	jne no_activate_put_fon_

	cmp [Paste_flag],1
	je no_put_fon___
	call PutFonForSprite
	no_put_fon___:
	and [Paste_flag],0
	no_activate_put_fon_:

	cmp [finishing_crossing],0
	jz not_finish_from_instrument_crossing
	and [Activate_instrument],0
	and [crossing],0
	and [instrument_used],0
	and [DrawSprite_flag],0
	call MovePictureToWorkScreen
	jmp still
	not_finish_from_instrument_crossing:

	;calculate line
	mov ebx,[ReserveArray]
	add ebx,4
	mov eax,[PointerToPicture]
	mov ecx,[Picture_SizeX]
	mov edx,[OldX]
	shl edx,16
	add edx,[OldY]
	mov esi,[ScreenX]
	mov edi,[ScreenY]
	add esi,[PosX]
	add edi,[PosY]

	mov ebp,[Picture_SizeX]
	dec ebp
	cmp esi,ebp
	jl no_minimum_x_allocation
	mov esi,ebp
	no_minimum_x_allocation:

	mov ebp,[Picture_SizeY]
	dec ebp
	cmp edi,ebp
	jl no_minimum_y_allocation
	mov edi,ebp
	no_minimum_y_allocation:

	call calculate_rectangle
	mov [counter],eax

	;save color pixels in ReserveArray
	mov eax,[counter]
	mov ebx,[ReserveArray]
	mov [ebx],eax

	mov ecx,[ebx]
	add ebx,4

	xor ebp,ebp
	next_color_save_allocation:
	;save color of pixel in ReserveArray
	push ecx
	and [counter2],0
	mov edi,[ebx]
	vertical_width_save_allocation:
	   and [counter],0

	   horizontal_width_save_allocation:
	      mov eax,edi
	      mov edx,[eax]
	      and edx,0xffffff
	      mov [ebx+4807*4+ebp],dx
	      shr edx,16
	      mov [ebx+4807*4+2+ebp],dl

	   add edi,3
	   add ebp,4
	   inc [counter]
	   cmp [counter],1;5
	   jne horizontal_width_save_allocation

	   mov ecx,[Picture_SizeX]
	   sub ecx,1;5
	   lea ecx,[ecx+ecx*2]
	   add edi,ecx
	inc [counter2]
	cmp [counter2],1;5
	jne vertical_width_save_allocation
	pop ecx
	add ebx,4
	dec ecx
	jnz next_color_save_allocation

	cmp [DrawSprite_flag],1
	jne no_save_fon_for_sprite_
	;save current coordinats as old
	mov eax,[SpriteCoordinatX]
	mov ebx,[SpriteCoordinatY]
	mov [SpriteOldCoordinatX],eax
	mov [SpriteOldCoordinatY],ebx

	call SaveFonForSprite

	no_save_fon_for_sprite_:

	;draw calculated pixels on work arrea
	mov ebx,[ReserveArray]
	mov ecx,[ebx]
	add ebx,4
	next_pixel_put_allocation:

	mov edx,0x1f3fff
	mov esi,edx
	shr edx,16

	mov ebp,ecx
	and ebp,8
	cmp ebp,8
	jne black_color
	mov si,0xffff
	mov dl,0xff
	black_color:

	mov eax,[ebx]
	mov [eax],si
	mov [eax+2],dl

	add ebx,4
	dec ecx
	jnz next_pixel_put_allocation

	cmp [DrawSprite_flag],1
	jne no_activate_draw_sprite_

	call DrawSprite

	no_activate_draw_sprite_:

	mov al,[instrument_used]
	test al,al
	jz no_change_coordinats
	cmp [Activate_instrument],0
	jz no_save_shades
	mov eax,[ScreenX]
	mov ebx,[ScreenY]
	add eax,[PosX]
	add ebx,[PosY]
	mov [rectangular_shade_x],eax
	mov [rectangular_shade_y],ebx
	no_save_shades:

	call MovePictureToWorkScreen
	jmp still
	no_allocation:

	;*************reflection from left to right******************
	cmp eax,35
	jne no_reflection_from_left_to_right

	mov ebp,[PointerToPicture]
	mov edx,[ReserveArray]
	mov esi,[Picture_SizeX]
	mov ebx,[Picture_SizeX]
	lea ebx,[ebx+ebx*2]
	shr esi,1
	next_line_reflection_x:
	;copy vertical line to array
	mov ecx,[Picture_SizeX]
	shr ecx,1
	sub ecx,esi
	lea ecx,[ecx+ecx*2]
	add ecx,[PointerToPicture]
	mov edx,[ReserveArray]
	and edi,0
	copy_to_array_line_reflection_y:
	xor eax,eax
	mov eax,[ecx]
	mov ebp,edi
	lea ebp,[ebp+ebp*2]
	mov [edx+ebp],ax
	shr eax,16
	mov [edx+ebp+2],al
	add ecx,ebx
	inc edi
	cmp edi,[Picture_SizeY]
	jne copy_to_array_line_reflection_y

	mov ecx,[Picture_SizeX]
	shr ecx,1
	mov ebp,ecx
	add ecx,esi
	dec ecx
	lea ecx,[ecx+ecx*2]
	add ecx,[PointerToPicture]
	sub ebp,esi
	lea ebp,[ebp+ebp*2]
	add ebp,[PointerToPicture]
	and edi,0
	next_line_reflection_y:
	mov eax,[ecx]
	and eax,0xffffff
	mov [ebp],ax
	shr eax,16
	mov [ebp+2],al
	add ebp,ebx
	add ecx,ebx
	inc edi
	cmp edi,[Picture_SizeY]
	jnz next_line_reflection_y

	;copy vertical line  from array to screen
	mov ecx,[Picture_SizeX]
	shr ecx,1
	add ecx,esi
	dec ecx
	lea ecx,[ecx+ecx*2]
	add ecx,[PointerToPicture]
	mov edx,[ReserveArray]
	and edi,0
	copy_from_array_to_screen_reflection_y:
	mov ebp,edi
	lea ebp,[ebp+ebp*2]
	xor eax,eax
	mov eax,[edx+ebp]
	mov [ecx],ax
	shr eax,16
	mov [ecx+2],al
	add ecx,ebx
	inc edi
	cmp edi,[Picture_SizeY]
	jne copy_from_array_to_screen_reflection_y

	dec esi
	jnz next_line_reflection_x

	call MovePictureToWorkScreen
	jmp still
	no_reflection_from_left_to_right:

	;*************reflection from up to down******************
	cmp eax,36
	jne no_reflection_from_up_to_down
	mov esi,[Picture_SizeX]
	mov edi,esi
	lea esi,[esi+esi*2]
	lea edi,[edi+edi*2]
	imul edi,[Picture_SizeY]
	mov edx,[ReserveArray]
	mov ecx,[Picture_SizeY]
	shr ecx,1
	add edi,[PointerToPicture]
	sub edi,esi
	mov ebp,[PointerToPicture]
	next_lines_reflection:
	;copy line
	xor ebx,ebx
	copy_line_1:
	xor eax,eax
	mov al,[edi+ebx]
	mov [edx+ebx],al
	inc ebx
	cmp ebx,esi
	jne copy_line_1

	xor ebx,ebx
	copy_line_2:
	xor eax,eax
	mov al,[ebp+ebx]
	mov [edi+ebx],al
	inc ebx
	cmp ebx,esi
	jne copy_line_2

	xor ebx,ebx
	copy_line_3:
	xor eax,eax
	mov al,[edx+ebx]
	mov [ebp+ebx],al
	inc ebx
	cmp ebx,esi
	jne copy_line_3

	add ebp,esi
	sub edi,esi
	dec ecx
	jnz next_lines_reflection

	call MovePictureToWorkScreen
	jmp still
	no_reflection_from_up_to_down:

	;*********************draw hard contour*******************
	cmp eax,38
	jne no_kontur_

	mov al,[Activate_instrument]
	test al,al
	jnz no_new_konture_xy

	mov al,[instrument_used]
	test al,al
	jz instrument_not_used
	mov eax,[used_OldX]
	mov ebx,[used_OldY]
	mov [OldX],eax
	mov [OldY],ebx
	jmp exit_used_instrument
	instrument_not_used:

	mov eax,[ScreenX]
	mov ebx,[ScreenY]
	add eax,[PosX]
	add ebx,[PosY]
	mov [OldX],eax
	mov [OldY],ebx
	exit_used_instrument:

	mov al,[instrument_used]
	test al,al
	jnz instrument_used_true
	mov [instrument_used],byte 1
	instrument_used_true:

	mov [Activate_instrument],1
	mov eax,[ReserveArray]
	mov ecx,60000
	clear_array_konture:
	mov [eax],dword 0
	add eax,4
	dec ecx
	jnz clear_array_line

	no_new_konture_xy:

	;put saved pixels
	mov ebx,[ReserveArray]
	mov eax,[ebx]
	test eax,eax
	jz no_put_line_to_screen_konture
	mov ecx,[ebx]
	add ebx,4
	xor ebp,ebp
	next_color_put_konture:
	;put saved pixels in ReserveArray
	push ecx
	and [counter2],0
	mov edi,[ebx]
	push edi
	vertical_width_put_konture:
	   and [counter],0

	   horizontal_width_put_konture:
	      mov edx,[ebx+4807*4+ebp]
	      and edx,0xffffff
	      mov [edi],dx
	      shr edx,16
	      mov [edi+2],dl

	   add edi,3
	   add ebp,4
	   inc [counter]
	   cmp [counter],5
	   jne horizontal_width_put_konture

	   mov ecx,[Picture_SizeX]
	   sub ecx,5
	   lea ecx,[ecx+ecx*2]
	   add edi,ecx
	inc [counter2]
	cmp [counter2],5
	jne vertical_width_put_konture
	pop edi
	pop ecx
	add ebx,4
	dec ecx
	jnz next_color_put_konture

	no_put_line_to_screen_konture:

	;calculate line
	mov ebx,[ReserveArray]
	add ebx,4
	mov eax,[PointerToPicture]
	mov ecx,[Picture_SizeX]
	mov edx,[OldX]
	shl edx,16
	add edx,[OldY]
	mov esi,[ScreenX]
	mov edi,[ScreenY]
	add esi,[PosX]
	add edi,[PosY]

	mov ebp,[Picture_SizeX]
	sub ebp,[line_width]
	cmp esi,ebp
	jl no_minimum_x_konture
	mov esi,ebp
	no_minimum_x_konture:

	mov ebp,[Picture_SizeY]
	sub ebp,[line_width]
	cmp edi,ebp
	jl no_minimum_y_konture
	mov edi,ebp
	no_minimum_y_konture:

	call calculate_line
	;call calculate_rectangle
	mov [counter],eax

	;save color pixels in ReserveArray
	mov eax,[counter]
	mov ebx,[ReserveArray]
	mov [ebx],eax

	mov ecx,[ebx]
	add ebx,4

	xor ebp,ebp
	next_color_save_konture:
	;save color of pixel in ReserveArray
	push ecx
	and [counter2],0
	mov edi,[ebx]
	vertical_width_save_konture:
	   and [counter],0

	   horizontal_width_save_konture:
	      mov eax,edi
	      mov edx,[eax]
	      and edx,0xffffff
	      mov [ebx+4807*4+ebp],dx
	      shr edx,16
	      mov [ebx+4807*4+2+ebp],dl

	   add edi,3
	   add ebp,4
	   inc [counter]
	   cmp [counter],5
	   jne horizontal_width_save_konture

	   mov ecx,[Picture_SizeX]
	   sub ecx,5
	   lea ecx,[ecx+ecx*2]
	   add edi,ecx
	inc [counter2]
	cmp [counter2],5
	jne vertical_width_save_konture
	pop ecx
	add ebx,4
	dec ecx
	jnz next_color_save_konture

	;draw calculated pixels on work arrea
	mov ebx,[ReserveArray]
	mov ecx,[ebx]
	mov edx,[Color]
	mov esi,[Color]
	shr edx,16
	add ebx,4
	mov edi,[line_width]
	dec edi
	imul edi,25
	next_pixel_put_konture:

	mov eax,[ebx]
	push eax
	push ecx
	xor ebp,ebp
	and [counter2],0

	vertical_width_konture:
	   and [counter],0

	   horizontal_width_konture:
	   xor ecx,ecx
	   mov cl,byte[width_pixels+edi+ebp]
	   test cl,cl
	     jz no_draw_pixel_konture
	      mov [eax],si
	      mov [eax+2],dl
	     no_draw_pixel_konture:
	   add eax,3
	   inc ebp
	   inc [counter]
	   cmp [counter],5
	   jne horizontal_width_konture

	   mov ecx,[Picture_SizeX]
	   sub ecx,5
	   lea ecx,[ecx+ecx*2]
	   add eax,ecx
	inc [counter2]
	cmp [counter2],5
	jne vertical_width_konture

	pop ecx
	pop eax
	add ebx,4
	dec ecx
	jnz next_pixel_put_konture

	mov eax,[ScreenX]
	mov ebx,[ScreenY]
	add eax,[PosX]
	add ebx,[PosY]
	mov [used_OldX],eax
	mov [used_OldY],ebx

	call MovePictureToWorkScreen

	jmp still

	no_kontur_:
	;************************Draw ellips***********************
	cmp eax,39
	jne no_ellips

	mov al,[Activate_instrument]
	test al,al
	jnz no_new_ellips_xy
	mov eax,[ScreenX]
	mov ebx,[ScreenY]
	add eax,[PosX]
	add ebx,[PosY]
	mov [OldX],eax
	mov [OldY],ebx
	mov [Activate_instrument],1
	mov eax,[ReserveArray]
	mov ecx,60000
	clear_array_ellips:
	mov [eax],dword 0
	add eax,4
	dec ecx
	jnz clear_array_ellips
	jmp still
	no_new_ellips_xy:

	;put saved pixels
	mov ebx,[ReserveArray]
	mov eax,[ebx]
	test eax,eax
	jz no_put_line_to_screen_ellips
	mov ecx,[ebx]
	add ebx,4
	xor ebp,ebp
	next_color_put_ellips:
	;put saved pixels in ReserveArray
	push ecx
	and [counter2],0
	mov edi,[ebx]
	push edi
	vertical_width_put_ellips:
	   and [counter],0

	   horizontal_width_put_ellips:
	      mov edx,[ebx+4807*4+ebp]
	      and edx,0xffffff
	      mov [edi],dx
	      shr edx,16
	      mov [edi+2],dl

	   add edi,3
	   add ebp,4
	   inc [counter]
	   cmp [counter],5
	   jne horizontal_width_put_ellips

	   mov ecx,[Picture_SizeX]
	   sub ecx,5
	   lea ecx,[ecx+ecx*2]
	   add edi,ecx
	inc [counter2]
	cmp [counter2],5
	jne vertical_width_put_ellips
	pop edi
	pop ecx
	add ebx,4
	dec ecx
	jnz next_color_put_ellips

	no_put_line_to_screen_ellips:

	mov esi,[ScreenX]
	mov edi,[ScreenY]
	add esi,[PosX]
	add edi,[PosY]

	sub esi,[OldX]
	jns no_sign_x_ellips
	neg esi
	shr esi,1
	neg esi
	add esi,[OldX]
	jmp no_plus_sign_x_ellips
	no_sign_x_ellips:

	shr esi,1
	add esi,[OldX]
	no_plus_sign_x_ellips:

	sub edi,[OldY]
	jns no_sign_y_ellips
	neg edi
	shr edi,1
	neg edi
	add edi,[OldY]
	jmp no_plus_sign_y_ellips
	no_sign_y_ellips:

	shr edi,1
	add edi,[OldY]
	no_plus_sign_y_ellips:

	mov [x],esi
	mov [y],edi

	mov esi,[ScreenX]
	mov edi,[ScreenY]
	add esi,[PosX]
	add edi,[PosY]

	sub esi,[OldX]
	jns no_sign_x_ellips_r
	neg esi
	no_sign_x_ellips_r:

	sub edi,[OldY]
	jns no_sign_y_ellips_r
	neg edi
	no_sign_y_ellips_r:

	cmp edi,0
	jnz no_null_a_ellips
	mov edi,1
	no_null_a_ellips:

	shr esi,1
	shr edi,1
	mov [a_ellips],esi
	mov [b_ellips],edi

	mov edi,[x]
	mov ebp,[y]
	add edi,esi
	add ebp,esi

	mov edx,[Picture_SizeX]
	sub edx,[line_width]
	cmp edi,edx
	jl no_limit_x_ellips
	sub edi,edx
	sub [a_ellips],edi
	no_limit_x_ellips:

	mov edx,[Picture_SizeY]
	sub edx,[line_width]
	cmp ebp,edx
	jl no_limit_y_ellips
	sub ebp,edx
	sub [b_ellips],ebp
	no_limit_y_ellips:


	mov edi,[x]
	mov ebp,[y]

	sub edi,[a_ellips]
	jns no_minimum_x_ellips
	add [a_ellips],edi
	no_minimum_x_ellips:

	sub ebp,[b_ellips]
	jns no_minimum_y_ellips
	add [b_ellips],ebp
	no_minimum_y_ellips:

	;calculate circle
	mov ebx,[ReserveArray]
	add ebx,4
	mov eax,[PointerToPicture]
	mov ecx,[Picture_SizeX]
	mov edx,[x]
	shl edx,16
	add edx,[y]
	mov esi,[a_ellips]
	shl esi,16
	add esi,[b_ellips]
	call calculate_ellips

	mov [counter],eax

	;save color pixels in ReserveArray
	mov eax,[counter]
	mov ebx,[ReserveArray]
	mov [ebx],eax

	mov ecx,[ebx]
	add ebx,4

	xor ebp,ebp
	next_color_save_ellips:
	;save color of pixel in ReserveArray
	push ecx
	and [counter2],0
	mov edi,[ebx]
	vertical_width_save_ellips:
	   and [counter],0

	   horizontal_width_save_ellips:
	      mov eax,edi
	      mov edx,[eax]
	      and edx,0xffffff
	      mov [ebx+4807*4+ebp],dx
	      shr edx,16
	      mov [ebx+4807*4+2+ebp],dl

	   add edi,3
	   add ebp,4
	   inc [counter]
	   cmp [counter],5
	   jne horizontal_width_save_ellips

	   mov ecx,[Picture_SizeX]
	   sub ecx,5
	   lea ecx,[ecx+ecx*2]
	   add edi,ecx
	inc [counter2]
	cmp [counter2],5
	jne vertical_width_save_ellips
	pop ecx
	add ebx,4
	dec ecx
	jnz next_color_save_ellips

	;draw calculated pixels on work arrea
	mov ebx,[ReserveArray]
	mov ecx,[ebx]
	mov edx,[Color]
	mov esi,[Color]
	shr edx,16
	add ebx,4
	mov edi,[line_width]
	dec edi
	imul edi,25
	next_pixel_put_ellips:

	mov eax,[ebx]
	push eax
	push ecx
	xor ebp,ebp
	and [counter2],0

	vertical_width_ellips:
	   and [counter],0

	   horizontal_width_ellips:
	   xor ecx,ecx
	   mov cl,byte[width_pixels+edi+ebp]
	   test cl,cl
	     jz no_draw_pixel_ellips
	      mov [eax],si
	      mov [eax+2],dl
	     no_draw_pixel_ellips:
	   add eax,3
	   inc ebp
	   inc [counter]
	   cmp [counter],5
	   jne horizontal_width_ellips

	   mov ecx,[Picture_SizeX]
	   sub ecx,5
	   lea ecx,[ecx+ecx*2]
	   add eax,ecx
	inc [counter2]
	cmp [counter2],5
	jne vertical_width_ellips

	pop ecx
	pop eax
	add ebx,4
	dec ecx
	jnz next_pixel_put_ellips

	call MovePictureToWorkScreen
	jmp still
	no_ellips:

	;*************************Line width 1*********************
	cmp eax,41
	jne no_line_width_1
	mov [line_width],1
	jmp still
	no_line_width_1:
	;*************************Line width 2*********************
	cmp eax,42
	jne no_line_width_2
	mov [line_width],2
	jmp still
	no_line_width_2:
	;*************************Line width 3*********************
	cmp eax,43
	jne no_line_width_3
	mov [line_width],3
	jmp still
	no_line_width_3:
	;*************************Line width 4*********************
	cmp eax,44
	jne no_line_width_4
	mov [line_width],4
	jmp still
	no_line_width_4:
	;*************************Line width 5*********************
	cmp eax,45
	jne no_line_width_5
	mov [line_width],5
	jmp still
	no_line_width_5:

	jmp still
	ret
;-----------------------------------------------------------
;-----------calculate cordinats on work picture--------------
;-----------------------------------------------------------
GetScreenCordinats:

	mov eax,[MouseX]
	mov ebx,[MouseY]
	sub eax,9
	sub ebx,83

	mov ecx,[k]
	cdq
	idiv ecx
	mov [ScreenX],eax
	mov eax,ebx
	cdq
	idiv ecx
	mov [ScreenY],eax

	ret
;-----------------------------------------------------------
;------------------SaveFonForSprite-------------------------
;-----------------------------------------------------------
SaveFonForSprite:

	mov edi,[PointerToSpriteBufer]
	mov ecx,[SpriteSizeX]
	mov edx,[Picture_SizeX]
	sub edx,[SpriteSizeX]
	mov esi,[SpriteCoordinatY]
	imul esi,[Picture_SizeX]
	add esi,[SpriteCoordinatX]
	lea edx,[edx+edx*2]
	lea esi,[esi+esi*2]
	add esi,[PointerToPicture]
	mov ebx,[SpriteSizeY]
	mov [counter],ecx

	next_line_sprite_save:
	mov ecx,[counter]

	next_pixel_sprite_save:
	mov eax,[esi]
	and eax,0xffffff
	mov [edi],ax
	shr eax,16
	mov [edi+2],al
	add esi,3
	add edi,3
	dec ecx
	jnz next_pixel_sprite_save

	add esi,edx
	dec ebx
	jnz next_line_sprite_save

	ret
;-----------------------------------------------------------
;-------------------PutFonForSprite-------------------------
;-----------------------------------------------------------
PutFonForSprite:

	mov esi,[PointerToSpriteBufer]
	mov ecx,[SpriteSizeX]
	mov edx,[Picture_SizeX]
	sub edx,[SpriteSizeX]
	mov edi,[SpriteOldCoordinatY]
	imul edi,[Picture_SizeX]
	add edi,[SpriteOldCoordinatX]
	lea edx,[edx+edx*2]
	lea edi,[edi+edi*2]
	add edi,[PointerToPicture]
	mov ebx,[SpriteSizeY]
	mov [counter],ecx

	next_line_sprite_put:
	mov ecx,[counter]

	next_pixel_sprite_put:
	mov eax,[esi]
	and eax,0xffffff
	mov [edi],ax
	shr eax,16
	mov [edi+2],al
	add esi,3
	add edi,3
	dec ecx
	jnz next_pixel_sprite_put

	add edi,edx
	dec ebx
	jnz next_line_sprite_put

	ret
;-----------------------------------------------------------
;------------------DrawFonForSprite-------------------------
;-----------------------------------------------------------
DrawSprite:

	mov esi,[PointerToEditBufer]
	mov ecx,[SpriteSizeX]
	mov edx,[Picture_SizeX]
	sub edx,[SpriteSizeX]
	mov edi,[SpriteCoordinatY]
	imul edi,[Picture_SizeX]
	add edi,[SpriteCoordinatX]
	lea edx,[edx+edx*2]
	lea edi,[edi+edi*2]
	add edi,[PointerToPicture]
	mov ebx,[SpriteSizeY]
	mov [counter],ecx

	next_line_sprite_draw:
	mov ecx,[counter]

	next_pixel_sprite_draw:
	mov eax,[esi]
	and eax,0xffffff
	mov [edi],ax
	shr eax,16
	mov [edi+2],al
	add esi,3
	add edi,3
	dec ecx
	jnz next_pixel_sprite_draw

	add edi,edx
	dec ebx
	jnz next_line_sprite_draw

	ret
;-----------------------------------------------------------
;-------find simvole in string------------------------------
;-----------------------------------------------------------
find_symvol:
	;eax,string
	;ebx,symvol
	mov esi,eax
	next_symvol:
	xor ecx,ecx
	mov cl,[eax]
	cmp cl,bl
	je symvol_fined
	inc eax
	jmp next_symvol

	symvol_fined:
	sub eax,esi
	ret
;-----------------------------------------------------------
;--------load file in memory--------------------------------
;-----------------------------------------------------------
load_file:
	 mov eax,58
	 mov ebx,file_info
	 int 0x40
	 test eax,eax
	 jnz no_open_
	 mov [length_file],ebx
	 shr ebx,9
	 inc ebx
	 mov [file_info+8],ebx
	 mov eax,58
	 mov ebx,file_info
	 int 0x40
	 no_open_:
	 ret
;-----------------------------------------------------------
;---load icons  in memory and draw icons on panel-----------
;-----------------------------------------------------------
load_icons:
	 mov esi,panel_picture
	 mov edi,[ScreenPointer]
	 mov eax,edi
	 add edi,(1200*1000*3)
	 call ReadGIF

	 ret

draw_icons:
	 mov [Icon_X],10
	 mov [Icon_Y],20+15+4
	 and [counter],0
   next_icon:
	 mov eax,8
	 mov ebx,[Icon_X]
	 mov ecx,[Icon_Y]
	 dec ebx
	 dec ecx
	 shl ebx,16
	 shl ecx,16
	 add ebx,20
	 add ecx,20
	 mov edx,[counter]
	 add edx,10
	 add edx,1000000000000000000000000000000b
	 int 0x40


	 mov edx,[Icon_X]
	 shl edx,16
	 add edx,[Icon_Y]
	 mov ebx,[counter]
	 imul ebx,20*20*3
	 add ebx,[ScreenPointer]
	 add ebx,(1200*1000*3)+12
	 mov eax,7
	 mov ecx,20*65536+20
	 int 0x40

	 add [Icon_X],25
	 cmp [Icon_X],10+25*15
	 jl no_next_line_icons
	 mov [Icon_X],10
	 mov [Icon_Y],20+15+4+22
	 no_next_line_icons:
	 inc [counter]
	 cmp [counter],30
	 jl next_icon

	 and [counter],0
	 mov [Icon_X],475
	 mov [Icon_Y],20+15+7
	 next_button_line:

	 mov eax,8
	 mov ebx,[Icon_X]
	 mov ecx,[Icon_Y]
	 dec ebx
	 dec ecx
	 shl ebx,16
	 shl ecx,16
	 add ebx,10
	 add ecx,35
	 mov edx,[counter]
	 add edx,40
	 add edx,1000000000000000000000000000000b
	 int 0x40

	 mov eax,13
	 mov ebx,[Icon_X]
	 mov ecx,[Icon_Y]
	 add ebx,4
	 shl ebx,16
	 shl ecx,16
	 add ebx,[counter]
	 add ecx,35
	 ;xor edx,edx
	 mov edx,0xb1d8ff
	 int 0x40

	 add [Icon_X],15
	 inc [counter]
	 cmp [counter],5
	 jle next_button_line
	 ret
;-----------------------------------------------------------
;calculate position work screen on a picture
;-----------------------------------------------------------
CalculatePositionScreen:

	 mov eax,[Picture_SizeX]
	 mov ebx,[Picture_SizeY]
	 mov ecx,[CounterX]
	 mov edx,[CounterY]
	 sub eax,ecx
	 sub ebx,edx

	 cmp [PosX],eax
	 jle no_limit_screen_x
	 mov [PosX],eax
	 no_limit_screen_x:

	 cmp [PosY],ebx
	 jle no_limit_screen_y
	 mov [PosY],ebx
	 no_limit_screen_y:

	 cmp [PosX],0
	 jns no_minimum_screen_x
	 mov [PosX],0
	 no_minimum_screen_x:

	 cmp [PosY],0
	 jns no_minimum_screen_y
	 mov [PosY],0
	 no_minimum_screen_y:

	 ret
;-----------------------------------------------------------
;-------analizing picture to palette------------------------
;-----------------------------------------------------------
analizing_picture_to_palette:

	 mov eax,[Picture_SizeX]
	 mov ebx,[Picture_SizeY]
	 imul eax,ebx
	 mov ecx,eax
	 mov edx,[PointerToPicture]
	 mov esi,1 ;counter colors in palette

	 ;put firs color of picture how one color in palette
	 mov ebp,[PointerToPalette]
	 mov eax,[edx]
	 and eax,0xffffff
	 mov [ebp],eax

	 analizing_to_palette:
	 mov eax,[edx]
	 and eax,0xffffff

	 mov edi,esi
	 mov ebp,[PointerToPalette]
	 next_color_in_palette:
	 mov ebx,[ebp]
	 and ebx,0xffffff
	 cmp ebx,eax	     ;this is color have in palette
	 je exit_loop_palette
	 add ebp,4
	 dec edi
	 jnz next_color_in_palette

	 inc esi
	 cmp esi,256 ;256 colors
	 ja more_than_256_colors
	 mov ebp,[PointerToPalette]
	 mov ebx,esi
	 dec ebx
	 shl ebx,2
	 add ebp,ebx
	 mov [ebp],ax
	 shr eax,16
	 mov [ebp+2],al

	 exit_loop_palette:

	 add edx,3
	 dec ecx
	 jnz analizing_to_palette


	 more_than_256_colors:
	 mov eax,esi
	 ret
;---------------------------------------------------------
;---------------SOUND of EVENTS---------------------------
;---------------------------------------------------------
sound:
	  mov eax,55
	  mov ebx,eax
	  int 0x40
	  ret
;----------------------------------------------------------
;-------------file info structure--------------------------
;----------------------------------------------------------
file_info:
		     dd 0
		     dd 0
		     dd 1
		     dd 0x19000+0x10000
		     dd 0x19000

		    file_path:
		     times 256 db 0
;-----------------------------------------------------------
;------------variables and data of program------------------
;-----------------------------------------------------------
length_file	     dd 0

time		     dd 0
sound_havent_memory  db 150,64,0
PosX		     dd 0
PosY		     dd 0
ScreenPointer	     dd 0
PointerToPicture     dd 0
PointerToCopyPicture dd 0
PointerToCopyPicture2 dd 0
PointerToEditBufer   dd 0
PointerToSpriteBufer dd 0
PointerToPalette     dd 0
Color		     dd 0
Number_Brush	     dd 0
Brush_SizeX	     dd 0
Brush_SizeY	     dd 0
Current_instrument   dd 0
Activate_instrument  db 0
SColor		     dd 0
OldX		     dd 0
OldY		     dd 0

MouseX		     dd 0
MouseY		     dd 0
Window_SizeX	     dd 0
Window_SizeY	     dd 0
Window_CordinatX     dd 0
Window_CordinatY     dd 0
Picture_SizeX	     dd 0
Picture_SizeY	     dd 0
ScreenX 	     dd 0
ScreenY 	     dd 0
WorkScreen_SizeX     dd 0
WorkScreen_SizeY     dd 0
MaxWorkScreen_SizeX  dd 0
MaxWorkScreen_SizeY  dd 0
k		     dd 0
IPC_table	     rd 256
ReserveArray	     dd 0
register	     dd 0
CounterX	     dd 0
CounterY	     dd 0
OffsetYPicture	     dd 0
OffsetYWorkScreen    dd 0
OffsetYBigPixel      dd 0

Icon_X		     dd 0
Icon_Y		     dd 0
counter 	     dd 0
counter2	     dd 0
Icon_text_x	     dd 0
Icon_text_y	     dd 0
Panel_flag	     db 0
counter_menu	     dd 0
menu_coordinat_x     dd 0
menu_size_x	     dd 0
menu_counter	     dd 0
counter_11	     dd 0
number_panel	     dd 0
number_menu	     dd 0

Scroll1CoordinatX    dd 0
Scroll1CoordinatY    dd 0
Scroll1MaxSizeX      dd 0
Scroll1MaxSizeY      dd 0
Scroll1SizeX	     dd 0
Scroll1FreeX	     dd 0

Scroll2CoordinatX    dd 0
Scroll2CoordinatY    dd 0
Scroll2MaxSizeX      dd 0
Scroll2MaxSizeY      dd 0
Scroll2SizeY	     dd 0
Scroll2FreeY	     dd 0

extended_memory      dd 0
type		     dw 0
x		     dd 0
y		     dd 0
save_flag	     db 0
exit_from_work_arrea db 0

Radius		     dd 0
Dx_		     dd 0
Dy_		     dd 0
line_width	     dd 0
lastik_is_active     db 0
a_ellips	     dd 0
b_ellips	     dd 0
instrument_used      db 0
used_OldX	     dd 0
used_OldY	     dd 0
rectangular_shade_x  dd 0
rectangular_shade_y  dd 0
crossing_old_x	     dd 0
crossing_old_y	     dd 0
crossing	     dd 0
finishing_crossing   dd 0
number_undo	     dd 0
DrawSprite_flag      db 0
Paste_flag	     db 0
SpriteSizeX	     dd 0
SpriteSizeY	     dd 0
SpriteCoordinatX     dd 0
SpriteCoordinatY     dd 0
SpriteOldCoordinatX  dd 0
SpriteOldCoordinatY  dd 0
;**********************************************************
;------------------TEXT DATA-------------------------------
;**********************************************************
name_of_program      db 'Graphics editor <<ANIMAGE>> V1.00 '
mouse_pos_x	     db 'X='
mouse_pos_y	     db 'Y='
new_text1	     db 'PICTURE SIZE X'
new_text2	     db 'PICTURE SIZE Y'
ok_text 	     db 'OK'

panel_text	     dd 15,24,4
		     db 'FILE'
		     dd 95,24,4
		     db 'EDIT'
		     dd 175,24,11
		     db 'INSTRUMENTS'
		     dd 255,24,7
		     db 'PALETTE'
		     dd 335,24,7
		     db 'FILTERS'
		     dd 415,24,9
		     db 'ANIMATION'
		     dd 495,24,4
		     db 'HELP'
menu_text_en:
		     dd menu_text7
		     dd menu_text6
		     dd menu_text5
		     dd menu_text4
		     dd menu_text3
		     dd menu_text2
		     dd menu_text1
		     dd 0,0,0

menu_rectangles      dd 485,36,100,100
		     dd 405,36,100,100
		     dd 325,36,100,100
		     dd 245,36,100,100
		     dd 165,36,100,100
		     dd 85,36,100,100;20
		     dd 5,36,100,100


menu_text1:
		     dd 15,45,3
		     db 'NEW'
		     dd 15,65,4
		     db 'OPEN'
		     dd 15,85,4
		     db 'SAVE'
		     dd 15,105,7
		     db 'SAVE AS'
		     dd 15,125,4
		     db 'EXIT'

menu_text2:
		     dd 95,45,4
		     db 'UNDO'
		     dd 95,65,4
		     db 'COPY'
		     dd 95,85,5
		     db 'PASTE'
		     dd 95,105,3
		     db 'CUT'
		     dd 95,125,13
		     db 'CLEARE SCREEN'
		     ;dd 95,145,12
		     ;db 'ALLOCATE ALL'
menu_text3:
		     dd 175,45,3
		     db 0,0,0;'PENCIL'
		     dd 175,65,3
		     db 0,0,0;'BRUSHES'
		     dd 175,85,3
		     db 0,0,0;'COLORS'
		     dd 175,105,3
		     db 0,0,0;'PIPETKA'
menu_text4:
		     dd 255,45,1
		     db 0,0,0
menu_text5:
		     dd 335,45,1
		     db 0,0,0
menu_text6:
		     dd 415,45,1
		     db 0,0,0
menu_text7:
		     dd 495,45,1
		     db 0,0,0,0


menu_counters:
		     dd 1,1,1,1,1,5,5

;----------------------------------------------------------
palitra:		 ;three bytes for one color
db 0,0,0,0,0,0,128,0
db 0,128,0,0,0,128,128,0
db 128,0,0,0,128,0,128,0
db 128,128,0,0,128,128,128,0
db 192,220,192,0,240,202,166,0
db 170,63,42,0,255,63,42,0
db 0,95,42,0,85,95,42,0
db 170,95,42,0,255,95,42,0
db 0,127,42,0,85,127,42,0
db 170,127,42,0,255,127,42,0
db 0,159,42,0,85,159,42,0
db 170,159,42,0,255,159,42,0
db 0,191,42,0,85,191,42,0
db 170,191,42,0,255,191,42,0
db 0,223,42,0,85,223,42,0
db 170,223,42,0,255,223,42,0
db 0,255,42,0,85,255,42,0
db 170,255,42,0,255,255,42,0
db 0,0,85,0,85,0,85,0
db 170,0,85,0,255,0,85,0
db 0,31,85,0,85,31,85,0
db 170,31,85,0,255,31,85,0
db 0,63,85,0,85,63,85,0
db 170,63,85,0,255,63,85,0
db 0,95,85,0,85,95,85,0
db 170,95,85,0,255,95,85,0
db 0,127,85,0,85,127,85,0
db 170,127,85,0,255,127,85,0
db 0,159,85,0,85,159,85,0
db 170,159,85,0,255,159,85,0
db 0,191,85,0,85,191,85,0
db 170,191,85,0,255,191,85,0
db 0,223,85,0,85,223,85,0
db 170,223,85,0,255,223,85,0
db 0,255,85,0,85,255,85,0
db 170,255,85,0,255,255,85,0
db 0,0,127,0,85,0,127,0
db 170,0,127,0,255,0,127,0
db 0,31,127,0,85,31,127,0
db 170,31,127,0,255,31,127,0
db 0,63,127,0,85,63,127,0
db 170,63,127,0,255,63,127,0
db 0,95,127,0,85,95,127,0
db 170,95,127,0,255,95,127,0
db 0,127,127,0,85,127,127,0
db 170,127,127,0,255,127,127,0
db 0,159,127,0,85,159,127,0
db 170,159,127,0,255,159,127,0
db 0,191,127,0,85,191,127,0
db 170,191,127,0,255,191,127,0
db 0,223,127,0,85,223,127,0
db 170,223,127,0,255,223,127,0
db 0,255,127,0,85,255,127,0
db 170,255,127,0,255,255,127,0
db 0,0,170,0,85,0,170,0
db 170,0,170,0,255,0,170,0
db 0,31,170,0,85,31,170,0
db 170,31,170,0,255,31,170,0
db 0,63,170,0,85,63,170,0
db 170,63,170,0,255,63,170,0
db 0,95,170,0,85,95,170,0
db 170,95,170,0,255,95,170,0
db 0,127,170,0,85,127,170,0
db 170,127,170,0,255,127,170,0
db 0,159,170,0,85,159,170,0
db 170,159,170,0,255,159,170,0
db 0,191,170,0,85,191,170,0
db 170,191,170,0,255,191,170,0
db 0,223,170,0,85,223,170,0
db 170,223,170,0,255,223,170,0
db 0,255,170,0,85,255,170,0
db 170,255,170,0,255,255,170,0
db 0,0,212,0,85,0,212,0
db 170,0,212,0,255,0,212,0
db 0,31,212,0,85,31,212,0
db 170,31,212,0,255,31,212,0
db 0,63,212,0,85,63,212,0
db 170,63,212,0,255,63,212,0
db 0,95,212,0,85,95,212,0
db 170,95,212,0,255,95,212,0
db 0,127,212,0,85,127,212,0
db 170,127,212,0,255,127,212,0
db 0,159,212,0,85,159,212,0
db 170,159,212,0,255,159,212,0
db 0,191,212,0,85,191,212,0
db 170,191,212,0,255,191,212,0
db 0,223,212,0,85,223,212,0
db 170,223,212,0,255,223,212,0
db 0,255,212,0,85,255,212,0
db 170,255,212,0,255,255,212,0
db 85,0,255,0,170,0,255,0
db 0,31,255,0,85,31,255,0
db 170,31,255,0,255,31,255,0
db 0,63,255,0,85,63,255,0
db 170,63,255,0,255,63,255,0
db 0,95,255,0,85,95,255,0
db 170,95,255,0,255,95,255,0
db 0,127,255,0,85,127,255,0
db 170,127,255,0,255,127,255,0
db 0,159,255,0,85,159,255,0
db 170,159,255,0,255,159,255,0
db 0,191,255,0,85,191,255,0
db 170,191,255,0,255,191,255,0
db 0,223,255,0,85,223,255,0
db 170,223,255,0,255,223,255,0
db 85,255,255,0,170,255,255,0
db 255,204,204,0,255,204,255,0
db 255,255,51,0,255,255,102,0
db 255,255,153,0,255,255,204,0
db 0,127,0,0,85,127,0,0
db 170,127,0,0,255,127,0,0
db 0,159,0,0,85,159,0,0
db 170,159,0,0,255,159,0,0
db 0,191,0,0,85,191,0,0
db 170,191,0,0,255,191,0,0
db 0,223,0,0,85,223,0,0
db 170,223,0,0,255,223,0,0
db 85,255,0,0,170,255,0,0
db 0,0,42,0,85,0,42,0
db 170,0,42,0,255,0,42,0
db 0,31,42,0,85,31,42,0
db 170,31,42,0,255,31,42,0
db 0,63,42,0,170,174,160,0
db 240,251,255,0,164,160,160,0
db 128,128,128,0,0,0,255,0
db 0,255,0,0,0,255,255,0
db 255,0,0,0,255,0,255,0
db 255,255,0,0xff,0xff,0xff,0xff
;----------------------------------------------------------
;--------------------COLOR BRUSHES-------------------------
;----------------------------------------------------------
Brush_color:
;
db 0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

;
db 0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

;
db 0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0
db 1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0
db 1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0
db 1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0
db 0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

;
db 0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0
db 0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0
db 0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0
db 0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0
db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0
db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0
db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0
db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0
db 0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0
db 0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0
db 0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0
db 0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

;
db 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

;
db 0
;------------------------------------------
Spray_color:
;
db 0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0
db 0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
;
db 0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0
db 0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0
db 0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0
db 0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0
db 0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0
db 0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0
db 0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0
db 0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0
db 0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0
db 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0
db 0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
;
db 0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0
db 0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0
db 0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0
db 0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0
db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0
db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0
db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0
db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0
db 0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0
db 0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0
db 0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0
db 0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

;------------------------------------------
width_pixels:

db 1,0,0,0,0
db 0,0,0,0,0
db 0,0,0,0,0
db 0,0,0,0,0
db 0,0,0,0,0
;
db 1,1,0,0,0
db 1,1,0,0,0
db 0,0,0,0,0
db 0,0,0,0,0
db 0,0,0,0,0
;
db 0,1,0,0,0
db 1,1,1,0,0
db 0,1,0,0,0
db 0,0,0,0,0
db 0,0,0,0,0
;
db 0,1,1,0,0
db 1,1,1,1,0
db 1,1,1,1,0
db 0,1,1,0,0
db 0,0,0,0,0
;
db 0,0,1,0,0
db 0,1,1,1,0
db 1,1,1,1,1
db 0,1,1,1,0
db 0,0,1,0,0
;-----------------------------------------
width_pixels_rectangle:

db 1,0,0,0,0
db 0,0,0,0,0
db 0,0,0,0,0
db 0,0,0,0,0
db 0,0,0,0,0
;
db 1,1,0,0,0
db 1,1,0,0,0
db 0,0,0,0,0
db 0,0,0,0,0
db 0,0,0,0,0
;
db 1,1,1,0,0
db 1,1,1,0,0
db 1,1,1,0,0
db 0,0,0,0,0
db 0,0,0,0,0
;
db 1,1,1,1,0
db 1,1,1,1,0
db 1,1,1,1,0
db 1,1,1,1,0
db 0,0,0,0,0
;
db 1,1,1,1,1
db 1,1,1,1,1
db 1,1,1,1,1
db 1,1,1,1,1
db 1,1,1,1,1

;----------------------------------------------------------
;-------------------ICON"S picture-------------------------
;----------------------------------------------------------
panel_picture:
file 'worktab1.gif'

;----------------------------------------------------------

I_END:



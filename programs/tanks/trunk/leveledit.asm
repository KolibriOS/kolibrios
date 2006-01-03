use32
org 0x0

  db  'MENUET01'
  dd  0x1
  dd  START
  dd  I_END
  dd  0x3000+640*400*3+16*20*20*3+512+640+1
  dd  0x3000
  dd  0x0
  dd  0x0
include 'lang.inc'
include 'pixengin.inc'
START:
   mov eax,40
   mov ebx,111b
   int 0x40
   call drawwin
;main cycle(fps)
 fpst:
   call clock
   mov eax,[time]
   mov [old_time],eax
   mov [old_frame_time],eax
;----------------------------------------------------------
;---------load all sprites from arrays to memory-----------
;----------------------------------------------------------
   mov [number_sprite],0
   xor eax,eax
nextfile:
   mov ebx,[spisok+4*eax]
   mov ecx,50
   mov esi,ebx
   add esi,2
   mov edi,0x3000+(640*400*3)+(16*20*20*3)+10
   rep movsb ;copy palitra to memory
   mov esi,ebx
   add esi,52
   xor ecx,ecx
   mov cl,byte [ebx]
   mov edi,0x3000+(640*400*3)+(16*20*20*3)+512
   push eax
   call unpakin
   pop eax
   mov ecx,20*20
   mov esi,0x3000+(640*400*3)+(16*20*20*3)+512
   mov edi,[number_sprite]
   imul edi,3*20*20
   add edi,0x3000+(640*400*3)
   un:
      xor ebx,ebx
      mov bl,byte[esi]
      lea ebx,[ebx+ebx*2]
      add ebx,0x3000+(640*400*3)+(16*20*20*3)+10
      mov edx,[ebx]
      mov [edi],edx
      add esi,1
      add edi,3
      sub ecx,1
   jnz un
   add [number_sprite],1
   add eax,1
   cmp [number_sprite],10;total number of pictures equal 10
   jne nextfile
;--------------------------------------------------------
   mov [total_time],0
   mov [phas],0
   mov [x],100
   mov [y],100
animation:
   call clock
   mov eax,[time]
   sub eax,[old_time]
   cmp eax,5
   jl next_action
   mov eax,[time]
   mov [old_time],eax
   mov eax,[phas]
   mov [black],eax
   mov [phas],15
   call PutSprite
   call putimage
   mov eax,5
   mov ebx,2
   int 0x40
   mov eax,[black]
   mov [phas],eax
   call PutSprite
   call putimage
   call print_pos
   add [frames],2
next_action:
	 call clock
	 mov eax,[time]
	 sub eax,[old_frame_time]
	 cmp eax,100
	 jl no_frames
	 mov eax,[time]
	 mov [old_frame_time],eax
	 mov eax,13
	 mov ebx,420*65536+30
	 mov ecx,425*65536+15
	 mov edx,0xffffff
	 int 0x40
	 mov eax,47
	 mov ebx,3*65536
	 mov ecx,[frames]
	 mov edx,425*65536+427
	 mov esi,0
	 int 0x40
	 mov [frames],0
no_frames:
	 ;----------------------
	 mov ebx,20
	 mov eax,[x]
	 cdq
	 idiv ebx
	 mov ecx,eax
	 mov eax,[y]
	 cdq
	 idiv ebx
	 imul eax,32
	 add eax,ecx
	 add eax,0x3000+(640*400*3)+(16*20*20*3)+512
	 xor ebx,ebx
	 mov bl,byte [phas]
	 mov [eax],bl;write number of shablon to the map-array
	 ;-----------------------
	 ;mov eax,11
	 ;int 0x40
	 mov eax,23
	 mov ebx,2
	 int 0x40
	 cmp eax,2
	 jne animation
	 mov eax,2
	 int 0x40
	 shr eax,8
	 cmp eax,32
	 jne key2
	 add [phas],1
	 cmp [phas],10
	 jne animation
	 mov [phas],0
	 jmp animation
    key2:cmp eax,176
	 jne key3
	 sub [x],20
	 jmp animation
    key3:cmp eax,179
	 jne key4
	 add [x],20
	 jmp animation
    key4:cmp eax,178
	 jne key5
	 sub [y],20
	 jmp animation
    key5:cmp eax,177
	 jne key6
	 add [y],20
	 jmp animation
    key6: cmp eax,51
	 jne key7
	 call save_level
	 jmp animation
    key7:cmp eax,50
	 jne key8
	  call load_level
	 jmp animation
    key8:cmp eax,27
	 jne animation
	 mov eax,-1
	 int 0x40
;-------------------------------------------
drawwin:
 mov eax,12
	mov ebx,1
	int 0x40
	;рисуем окно задавая все необходимые цвета
	mov eax,0
	mov ebx,50*65536+640
	mov ecx,50*65536+450
	mov edx,0x02AABBCC
	mov esi,0x805080d0
	mov edi,0x005080d0
	int 0x40
	;пишем заголовок окна
	mov eax,4
	mov ebx,5*65536+5
	mov ecx,0x10ddeeff
	mov edx,name
	mov esi,7
	int 0x40
	;рисуем кнопку закрытия окна
	mov eax,8
	mov ebx,(640-19)*65536+12
	mov ecx,5*65536+12
	mov edx,1
	mov esi,0x6688dd
	int 0x40
	ret
;----------------------------------------------------------
counter     dd 0
;----------------------------------------------------
;draw sprite in video memory
PutSprite:
	mov ebx,[x]
	imul ebx,3
	mov eax,[y]
	imul eax,640*3
	add eax,0x3000
	add eax,ebx
	cld
	mov [counter],20
	mov esi,[phas]
	imul esi,1200
	add esi,0x3000+(640*400*3)
	mov ebx,esi
   draw:
	mov esi,ebx
	mov edi,eax
	mov ecx,20*3
	rep movsb
	add eax,640*3
	add ebx,3*20
	dec [counter]
	jnz draw
      ret
;-------------------------------------------------------------------------
;get time in 1/100 sec
clock:	mov eax,26
	mov ebx,9
	int 0x40
	mov [time],eax
	ret
;---------------------------------------------------------
putimage:
	 mov eax,7
	 mov ebx,0x3000
	 mov ecx,640*65536+400
	 mov edx,1*65536+20
	 int 0x40
	 ret
;---------------------------------------------------------
print_pos:mov eax,13
	  mov ebx,490*65536+60
	  mov ecx,425*65536+15
	  mov edx,0xffffff
	  int 0x40
	  mov eax,47
	  mov ebx,3*65536
	  mov ecx,[x]
	  mov edx,495*65536+430
	  mov esi,0
	  int 0x40
	  mov eax,47
	  mov ebx,3*65536
	  mov ecx,[y]
	  mov edx,530*65536+430
	  mov esi,0
	  int 0x40
	  ret
;-----------------------------------------------------------
;-------------------------load files------------------------
;-----------------------------------------------------------
loadfile:
	 ;куда записывать первый блок
	 mov [file_read+12],dword 0x3000+(640*400*3)+(16*20*20*3)
	 ;загружаем первый блок для того чтобы узнат размер файла
	 mov eax,58
	 mov ebx,file_read
	 int 0x40
	 ;вычисляем сколько блоков по 512 байт нужно использовать
	 ;для загрузки файла
	 mov ecx,ebx
	 shr ecx,9
	 add ecx,1
	 ;записываем адрес куда нужно записать первый блок
	 mov [file_read+12],dword 0x3000+(640*400*3)+(16*20*20*3)
   ;заносим в структуру file_read номер блока с которого нужно загружать файл
	 mov [file_read+4],dword 0
	 ;заносим в структуру file_read число блоков которые нужно загрузить
	 mov [file_read+8],ecx
	 ;загружаем блок
	 mov eax,58
	 mov ebx,file_read
	 int 0x40
	 cmp eax,0
	 jne scock2
	 mov esi,0x3000+(640*400*3)+(16*20*20*3)
	 mov ecx,[esi+8]
	 add esi,60
	 mov edi,0x3000+(640*400*3)+(16*20*20*3)+512
	 call unpakin
	 mov [addr_palitra],dword 0x3000+(640*400*3)+(16*20*20*3)+10
	 mov ecx,400
	 mov esi,0x3000+(640*400*3)+(16*20*20*3)+512
	 mov edi,[number_sprite]
	 imul edi,3*20*20
	 add edi,0x3000+(640*400*3)
    unp:
	xor eax,eax
	mov al,byte[esi]
	imul eax,3
	add eax,[addr_palitra]
	mov ebx,[eax]
	and ebx,0xffffff
	mov [edi],ebx
	add esi,1
	add edi,3
	sub ecx,1
	jnz unp
  scock2:
	 ret
;---------------------------------------------------------------------------
;-----------------save level in file----------------------------------------
;---------------------------------------------------------------------------
save_level:
	   call input_path
	   xor ebx,ebx
  copy_path:
	   xor eax,eax
	   mov al,byte [string+ebx]
	   cmp al,13
	   je end_copy
	   mov [file_write+20+ebx],eax
	   add ebx,1
	   jmp copy_path
   end_copy:
	   mov esi,0x3000+(640*400*3)+(16*20*20*3)+512;0x1000+0x1000
	   mov edi,0x3000+(640*400*3)+(16*20*20*3);0x1000
	   mov ecx,32*20
	   call save_image
	   mov eax,[SizeFile]
	   mov [file_write+8],eax
	   mov [file_write+12],dword 0x3000+(640*400*3)+(16*20*20*3);+(0x1000)
	   mov eax,58
	   mov ebx,file_write
	   int 0x40
	   cmp eax,0
	   jne scok
	   xor esi,esi
	   mov eax,55
	   mov ebx,eax
	   mov esi,sound
	   int 0x40
       scok:
	  ret
;-----------------------------------------------------------------------------
;-----------------load level to memory----------------------------------------
;-----------------------------------------------------------------------------
load_level:
	   call input_path
	   xor ebx,ebx
 copy_load:xor eax,eax
	   mov al,byte [string+ebx]
	   cmp al,13
	   je end_load
	   mov [file_read+20+ebx],eax
	   add ebx,1
	   jmp copy_load
  end_load:
	   mov [file_read+12],dword 0x3000+(640*400*3)+(16*20*20*3)
	   mov eax,58
	   mov ebx,file_read
	   int 0x40
	   mov ecx,ebx
	   shr ebx,9
	   add ebx,1
	   mov [file_read+8],ebx
	   mov eax,58
	   mov ebx,file_read
	   int 0x40
	   cmp eax,0
	   jne nosound
	   mov esi,0x3000+(640*400*3)+(16*20*20*3)
	   mov edi,0x3000+(640*400*3)+(16*20*20*3)+512;0x1000+0x1000
	   call unpakin
	   xor esi,esi
	   mov eax,55
	   mov ebx,55
	   mov esi,sound
	   int 0x40
	   xor esi,esi
	   mov [x_l],0
	   mov [y_l],0
       c_y:
	   mov [x_l],0
       c_x:
	   mov eax,[x_l]
	   mov ebx,[y_l]
	   imul ebx,32
	   add eax,ebx
	   add eax,0x3000+(640*400*3)+(16*20*20*3)+512;0x1000+0x1000
	   mov ecx,eax
	   xor eax,eax
	   mov al,byte [ecx]
	   imul eax,1200
	   add eax,0x3000+(640*400*3)
	   mov ebx,[x_l]
	   imul ebx,20*3
	   mov ecx,[y_l]
	   imul ecx,20*3*640
	   add ebx,ecx
	   add ebx,0x3000
	   mov esi,eax
	   mov edi,ebx
	   ;----------------------------
	   xor edx,edx
  next_line:
	   mov esi,eax
	   mov edi,ebx
	   mov ecx,60
	   rep movsb
	   add edx,1
	   add eax,20*3
	   add ebx,(640*3)
	   cmp edx,20
	   jne next_line
	   ;----------------------------
	   add [x_l],1
	   cmp [x_l],32
	   jne c_x
	   add [y_l],1
	   cmp [y_l],20
	   jne c_y
    nosound:
	   jmp animation
;-----------------------------------------------------------------------------
input_path:
	    mov [position],byte -1
	    call print_line
	    mov eax,13
	    mov ebx,25*65536+6
	    mov ecx,433*65536+12
	    mov edx,0xff6c58
	    int 0x40
      opros:mov eax,10
	    int 0x40
	    cmp eax,2
	    jne opros
	    mov eax,2
	    int 0x40
	    shr eax,8
	    cmp eax,13
	    je exit_cycle
	    cmp eax,8
	    je backspace
	    cmp eax,176
	    je left
	    cmp eax,179
	    je right
	    cmp eax,32
	    je probel
	    add [position],1
	    xor ebx,ebx
	    mov bl,[position]
	    mov [string+ebx],byte al
	    call print_line
	    call print_cursor
	    jmp  opros
  backspace:xor ebx,ebx
	    mov bl,[position]
	    mov [string+ebx],byte ' '
	    sub [position],1
	    call print_line
	    call print_cursor
	    jmp opros
       left:sub [position],1
	    call print_line
	    call print_cursor
	    jmp opros
      right:add [position],1
	    call print_line
	    call print_cursor
	    jmp opros
     probel:add [position],1
	     call print_line
	     call print_cursor
	    jmp opros
 exit_cycle:add [position],1
	     xor ebx,ebx
	     mov bl,[position]
	     mov [string+ebx],byte 13
	     mov eax,13
	     mov ebx,20*65536+(64*6)+5
	     mov ecx,430*65536+15
	     mov edx,0xffffff
	     int 0x40
	    ret
;---------------------------------------------------------------
print_line:
	     mov eax,13
	     mov ebx,20*65536+(64*6)+5
	     mov ecx,430*65536+15
	     mov edx,0xffffff
	     int 0x40
	     mov eax,4
	     mov ebx,25*65536+435
	     mov ecx,0x1
	     mov edx,string
	     mov esi,64
	     int 0x40
	     ret
print_cursor:
	     mov eax,13
	     xor ebx,ebx
	     mov bl,[position]
	     imul ebx,6
	     add ebx,25+6
	     shl ebx,16
	     add ebx,6
	     mov ecx,433*65536+12
	     mov edx,0xff6c58
	     int 0x40
	     ret
;------------------------------------------------------------------------
string:
       db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
       db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
position	db 0
total_time	dd 0
time		dd 0
old_time	dd 0
frame		dd 0
addr_palitra	dd 0
;addr_array      dd 0
number_sprite	dd 0
offs		dd 0
phas		dd 0
black		dd 0
x		dd 0
y		dd 0
x_l		dd 0
y_l		dd 0
name	  db 'testfps'
frames		dd 0
old_frame_time	dd 0
sound	  db 0x90,0x30,0
file_read:
       dd 0
       dd 0
       dd 1
       dd 0
       dd 0x3000
       db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
file_write:
	dd 1
	dd 0
	dd 0
	dd 0
	dd 0x3000
	db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
water:
db 247,0,0,0,0,0,255,0

db 0,128,0,255,128,0,128,255

db 0,255,0,0,255,255,0,0

db 0,0,0,0,0,0,0,0

db 0,0,0,0,0,0,0,0

db 0,0,0,0,0,0,0,0

db 0,0,7,0,2,1,19,2

db 1,2,1,2,17,2,1,18

db 35,49,19,2,49,2,17,2

db 17,3,6,5,19,2,19,17

db 18,35,18,1,2,51,5,1

db 2,5,3,6,18,1,19,5

db 3,17,2,19,6,5,19,6

db 1,3,33,19,6,3,5,3

db 2,17,51,2,1,19,18,3

db 6,67,18,1,2,3,5,35

db 1,2,17,19,5,6,5,6

db 3,1,18,51,2,3,18,3

db 6,3,5,6,19,5,19,1

db 19,33,18,1,18,3,6,3

db 6,67,1,2,1,2,65,2

db 1,19,5,3,21,3,17,34

db 1,66,1,18,67,2,17,2

db 33,2,1,18,1,2,1,34

db 1,2,1,18,1,18,17,2

db 33,34,17,50,17,2,17,66

db 1,2,1,19,5,3,2,17

db 2,17,50,17,34,19,6,3

db 6,19,1,2,65,19,6,3

db 17,3,5,19,5,35,18,17

db 3,6,3,5,19,18,3,6

db 5,19,5,19,18,17,3,21

db 3,21,2,1,35,21,19,17

db 2,1,2,3,6,3,6,5

db 3,1,18,1,35,2,17,50

db 1,3,5,6,35,1,2,1

db 2,1,2,17,2,17,2,17

db 2,51,16
voda1:
db 137,0,0,0,0,255,255,0

db 255,128,0,0,0,0,0,0

db 0,0,0,0,0,0,0,0

db 0,0,0,0,0,0,0,0

db 0,0,0,0,0,0,0,0

db 0,0,0,0,0,0,0,0

db 0,0,3,0,2,17,18,17

db 18,65,18,17,2,33,2,97

db 2,1,2,33,18,17,18,49

db 18,65,2,65,18,17,2,17

db 2,33,18,33,2,113,18,17

db 18,17,2,49,18,17,18,145

db 34,33,18,49,18,17,18,145

db 2,65,2,65,18,17,18,17

db 2,17,34,49,18,97,2,113

db 2,49,18,1,2,49,2,1

db 18,17,2,1,34,65,2,97

db 2,113,2,65,18,17,2,49

db 18,1,2,33,18,1,2,81

db 18,193,18,81,18,17,18,129

db 18,145,18,1,2,81,34,17

db 18,65,34,1,18,145,2,129

db 18,49,18,33,2,33,34,81

db 18,97,2,1,16
trava:
db 214,0,0,0,0,0,255,0

db 0,128,0,0,0,0,0,0

db 0,0,0,0,0,0,0,0

db 0,0,0,0,0,0,0,0

db 0,0,0,0,0,0,0,0

db 0,0,0,0,0,0,0,0

db 0,0,3,0,2,33,2,1

db 2,1,2,17,2,1,34,17

db 18,1,18,1,2,49,2,17

db 2,33,2,1,2,33,2,17

db 34,17,18,1,2,1,18,33

db 2,17,34,17,18,33,18,17

db 50,97,2,33,2,33,18,1

db 18,17,50,1,2,1,50,1

db 18,65,2,33,18,1,18,17

db 18,17,82,33,2,1,2,1

db 2,33,2,49,18,1,18,1

db 2,1,2,33,2,17,2,1

db 2,65,2,1,18,1,2,17

db 2,17,34,1,66,1,18,17

db 2,17,2,17,2,33,2,1

db 18,1,2,1,34,1,2,1

db 18,1,18,17,2,33,34,17

db 50,17,2,17,66,1,2,17

db 2,17,2,17,2,17,50,17

db 50,1,34,33,2,81,2,49

db 2,33,34,1,18,33,18,1

db 66,33,2,1,50,17,2,1

db 2,1,2,1,2,49,34,33

db 2,1,18,1,18,33,18,1

db 2,17,2,17,50,17,2,1

db 2,1,2,1,2,1,2,1

db 2,17,2,17,2,17,18,17

db 2,16
sten3:
db 104,0,0,0,0,128,128,128

db 192,192,192,0,0,0,0,0

db 0,0,0,0,0,0,0,0

db 0,0,0,0,0,0,0,0

db 0,0,0,0,0,0,0,0

db 0,0,0,0,0,0,0,0

db 0,0,3,0,226,66,65,2

db 81,2,65,2,81,2,81,2

db 65,2,81,2,81,2,65,2

db 1,226,66,33,2,81,2,81

db 2,65,2,81,2,81,2,65

db 2,81,2,81,2,17,226,66

db 1,2,81,2,81,2,65,2

db 81,2,81,2,65,2,81,2

db 81,2,49,226,66,65,2,97

db 2,65,2,65,2,97,2,65

db 2,65,2,97,2,65,226,82

db 33,2,81,2,81,2,65,2

db 81,2,81,2,65,2,81,2

db 81,2,17,16
sten1:
db 111,0,0,0,0,128,128,128

db 192,192,192,0,0,255,255,255

db 255,64,128,255,0,0,0,0

db 0,0,0,0,0,0,0,0

db 0,0,0,0,0,0,0,0

db 0,0,0,0,0,0,0,0

db 0,0,6,0,227,67,53,3

db 133,3,69,34,5,3,5,98

db 5,3,5,98,5,3,5,98

db 5,3,5,50,53,3,133,3

db 69,227,67,133,3,133,3,114

db 5,3,5,98,5,3,114,5

db 3,5,98,5,3,133,3,133

db 227,83,69,3,133,3,53,50

db 5,3,5,98,5,3,5,98

db 5,3,5,98,5,3,5,34

db 69,3,133,3,53,227,67,101

db 3,133,3,21,82,5,3,5

db 98,5,3,5,98,5,3,5

db 98,5,3,5,2,101,3,133

db 3,21,16
pesok:
db 120,0,0,0,0,0,255,255

db 0,64,128,0,128,128,64,128

db 128,64,128,255,0,0,0,0

db 0,0,0,0,0,0,0,0

db 0,0,0,0,0,0,0,0

db 0,0,0,0,0,0,0,0

db 0,0,6,0,225,81,2,33

db 2,65,2,3,33,2,1,5

db 49,5,49,2,129,3,225,97

db 3,17,2,33,2,1,5,33

db 2,1,5,129,5,65,2,65

db 2,225,97,2,33,3,17,2

db 49,3,145,2,177,3,49,5

db 81,5,1,2,33,2,65,3

db 17,2,17,3,145,2,209,3

db 81,2,49,5,33,2,177,3

db 145,2,17,2,5,113,2,225

db 1,2,33,3,17,3,49,3

db 17,3,161,2,1,5,97,2

db 177,2,49,2,33,5,33,2

db 193,2,49,3,33,2,81,3

db 1,5,1,16
palma:
db 209,0,0,0,0,0,255,0

db 0,128,0,0,255,255,64,128

db 128,64,128,255,0,0,0,0

db 0,0,0,0,0,0,0,0

db 0,0,0,0,0,0,0,0

db 0,0,0,0,0,0,0,0

db 0,0,6,0,2,33,2,1

db 2,1,2,17,2,1,34,17

db 18,1,18,1,51,1,2,17

db 2,1,35,1,2,33,115,2

db 51,21,19,1,2,35,69,35

db 53,35,18,3,21,67,5,3

db 21,19,1,18,19,2,5,3

db 50,1,3,5,3,5,67,17

db 3,1,19,17,67,5,99,34

db 3,2,35,5,3,5,3,5

db 3,5,3,5,35,1,18,1

db 2,19,5,35,5,3,5,35

db 21,19,33,19,5,35,5,35

db 5,51,5,3,34,35,17,51

db 20,3,5,35,5,3,2,1

db 35,18,3,5,3,36,5,19

db 1,35,1,18,3,17,2,3

db 2,52,35,50,1,2,17,2

db 17,2,1,52,3,34,17,50

db 1,34,33,36,49,2,49,2

db 33,34,1,36,17,18,1,66

db 33,2,1,2,1,36,1,2

db 1,2,1,2,1,2,49,66

db 36,18,1,18,33,18,1,2

db 17,84,2,17,2,1,2,1

db 2,1,2,1,2,1,84,2

db 17,18,17,2,16
kamni:
db 210,0,0,0,0,0,255,0

db 0,128,0,64,128,255,64,128

db 128,0,0,0,0,0,0,0

db 0,0,0,0,0,0,0,0

db 0,0,0,0,0,0,0,0

db 0,0,0,0,0,0,0,0

db 0,0,5,0,2,33,2,1

db 2,35,1,2,1,34,17,18

db 1,18,1,2,17,51,1,2

db 33,2,1,2,33,2,1,67

db 4,3,2,1,2,1,18,33

db 2,17,2,83,4,17,18,17

db 50,17,131,2,33,18,1,18

db 17,83,20,3,18,1,18,65

db 2,67,4,19,4,3,18,17

db 82,99,4,35,2,49,18,1

db 18,1,2,1,2,17,35,4

db 2,1,2,65,2,1,18,1

db 2,17,2,17,34,1,66,1

db 18,17,2,17,2,17,2,33

db 2,1,18,1,2,1,2,19

db 1,2,1,18,1,18,19,2

db 33,18,35,4,3,18,17,2

db 1,67,2,1,2,99,1,2

db 17,2,35,4,3,34,35,4

db 3,4,3,1,2,17,83,1

db 67,20,19,4,3,2,17,51

db 4,115,4,35,2,1,83,4

db 67,34,33,2,35,1,3,4

db 19,1,18,1,2,17,2,17

db 50,17,2,1,2,1,2,1

db 2,1,2,1,2,17,2,17

db 2,17,18,17,2,16
drevo:
db 240,0,0,0,0,0,255,0

db 0,128,0,255,0,128,0,64

db 128,64,128,128,128,0,128,255

db 128,128,0,0,0,0,0,0

db 0,0,0,0,0,0,0,0

db 0,0,0,0,0,0,0,0

db 0,0,8,0,2,33,2,1

db 2,19,1,35,34,17,18,1

db 18,1,2,17,3,38,7,22

db 3,1,2,1,2,33,2,1

db 19,7,6,23,6,23,3,6

db 18,33,2,17,2,3,7,19

db 6,3,54,7,19,50,33,3

db 22,7,6,3,6,7,22,3

db 23,2,1,18,17,22,7,6

db 7,3,7,3,7,22,7,6

db 35,17,35,54,55,6,23,22

db 19,18,19,7,3,7,6,7

db 22,7,3,22,3,22,7,3

db 18,19,23,3,182,3,33,3

db 22,3,7,6,7,6,7,22

db 7,6,7,22,3,34,7,22

db 7,22,3,39,38,7,6,7

db 19,2,1,3,7,6,23,38

db 7,38,39,6,7,3,1,2

db 19,54,3,38,3,70,3,2

db 1,2,1,3,7,6,7,19

db 4,5,4,19,6,7,6,3

db 1,50,1,34,3,1,36,17

db 3,22,2,49,2,33,34,21

db 4,33,18,1,66,33,2,1

db 52,17,2,1,2,1,2,1

db 2,49,18,20,5,4,5,4

db 18,1,18,33,18,1,2,1

db 132,1,2,1,2,1,2,1

db 2,1,36,5,20,5,4,21

db 36,17,2,16
baza1:
db 192,0,0,0,0,0,255,0

db 0,128,0,255,0,128,255,128

db 128,255,0,0,255,128,0,255

db 255,0,128,0,255,0,0,255

db 0,255,255,0,0,0,0,0

db 0,0,0,0,0,0,0,0

db 0,0,11,0,2,33,2,1

db 2,1,2,17,2,1,34,17

db 18,1,18,1,2,49,2,17

db 2,33,2,1,2,17,232,9

db 4,17,2,8,201,8,3,9

db 4,2,1,8,41,150,8,19

db 9,2,1,57,6,101,7,6

db 8,3,10,3,9,2,8,41

db 6,5,84,7,6,8,35,9

db 2,8,41,6,5,84,7,6

db 8,35,9,2,8,41,6,5

db 84,7,6,8,3,21,9,1

db 8,41,6,5,84,7,6,8

db 3,21,9,2,8,41,6,5

db 84,7,6,8,35,9,2,8

db 41,6,5,103,6,8,3,10

db 3,9,1,8,57,134,8,35

db 9,1,8,201,8,3,10,3

db 9,2,232,35,9,2,9,131

db 10,19,10,3,9,4,3,9

db 1,2,25,19,22,131,9,4

db 9,33,9,35,22,131,25,1

db 18,1,233,9,2,1,2,1

db 2,1,2,17,2,17,2,17

db 18,17,2,16
spisok:
   dd sten1
   dd sten3
   dd trava
   dd kamni
   dd palma
   dd water
   dd voda1
   dd baza1
   dd drevo
   dd pesok
I_END:
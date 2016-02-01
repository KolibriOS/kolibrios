;include '../../../../../KOSfuncs.inc'

;x,y - координаты вывода FPS на окно
;возвращает время в сотых долях секунды затрачиваемое на 1 цикл

time1 dd 0 ;int
time2 dd 0 ;int
fps1 dd 0 ;int
timerend dd 0 ;int

;функция определения FPS
proc Fps uses ebx ecx edx esi, x:dword, y:dword
locals
	tr dd ?
endl

	;определяем время прошедшее с момента запуска системы
	mcall SF_SYSTEM_GET,SSF_TIME_COUNT
	mov [time1],eax

	cmp dword[timerend],0
	jne @f
		mov eax,[time1]
		mov [time2],eax
		mov [timerend],eax
	@@:
	
	mov eax,[time1]
	sub eax,[timerend]
	mov [tr],eax

	mov eax,[time1]
	sub eax,[time2]
	cmp eax,100 ;если прошло менее 1 секунды 
	jge .else
		inc dword[fps1] ;увеличиваем счетчик fps
		jmp @f
	.else:
		;выводим число fps
		mov ebx,[x]
		shl ebx,16
		add ebx,23
		mov ecx,[y]
		shl ecx,16
		add ecx,7
		mcall SF_DRAW_RECT,,,0x00555555
		mov edx,[x]
		shl edx,16
		add edx,[y]
		mcall SF_DRAW_NUMBER,4 shl 16,[fps1],,0xfafafa
		mov dword[fps1],0
		mov eax,[time1]
		mov [time2],eax
	@@:
	
	mov eax,[time1]
	mov [timerend],eax
	mov eax,[tr]
	ret
endp

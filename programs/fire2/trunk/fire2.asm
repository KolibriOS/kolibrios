use32
      org     0x0
   
	db 'MENUET01'
	dd 0x01
	dd START
	dd I_END
	dd 0x100000
	dd 0x7fff0
	dd 0x0, 0x0

include 'lang.inc'   
START:
   
	call draw_window
   
 mov edi,my_img
	mov ecx,64001*3/4
	xor eax,eax
	rep stosd
   
	mov esi,my_img
	mov [count1],esi
   
	mov eax,3
	int 0x40
	mov [curtime],eax
   
still:
   
	mov eax,11
	int 0x40
	
	cmp eax,1
	je red
	cmp eax,2
	je key
	cmp eax,3
	je button
	call image
	mov eax,3
	int 0x40
	mov ebx,[curtime]
	cmp eax,ebx
	jz still
	mov [curtime],eax
	call print_fps
	jmp still
   
red:
	call draw_window
	jmp still
   
key:
	mov eax,2
	int 0x40
	cmp ah,1
	jne still
	mov eax,0xffffffff
	int 0x40
	jmp still
   
button:
	mov eax,17
	int 0x40
   
	cmp ah,1
	jne still
	mov eax,-1
	int 0x40
	retn                                                                           
   
;Window
   
draw_window:
   
	mov eax,12
	mov ebx,1
	int 0x40
   
	mov eax,0
	mov ebx,200*65536+325
	mov ecx,150*65536+225
	mov edx,0x00000000
	mov esi,edx
	mov edi,0x00100000
	int 0x40
   
	mov eax,4
	mov ebx,8*65536+8
	mov ecx,0x00402020
	mov edx,fire_label
	mov esi,len00
	int 0x40
   
	mov eax,8
	mov ebx,(325-19)*65536+12
	mov ecx,5*65536+12
	mov edx,1
	mov esi,0x00400000
	int 0x40
   
	mov eax,12
	mov ebx,2
	int 0x40
   
	retn
   
penta:
; Рисуем пентагpамму
   
	mov edi,my_img
	mov ebx,(320)*3
DrawP: mov     eax,0x00fc0000
       add     edi,0x540*3
       push    edi
       mov     ecx,0x78
ll_3:  mov [edi],eax
       mov [edi+3],eax
       mov [edi-3],eax
       mov [edi+ebx],eax
       add edi,6
       add     edi,ebx
       loop    ll_3
       pop     edi
       mov     ecx,0x60
ll_4:  mov     [ebx+edi],eax
       mov [edi],eax
       mov [edi+3],eax
       mov [edi-3],eax
       add edi,3
       add     edi,ebx
       add     edi,ebx
       loop    ll_4
       mov     ecx,0x61
ll_5:  mov     [ebx+edi],eax
       mov [edi],eax
       mov [edi+3],eax
       mov [edi-3],eax
       add edi,3
       sub     edi,ebx
       sub     edi,ebx
       loop    ll_5
       add     edi,0x98D1*3
       push    edi
       mov     ecx,0x78
ll_6:  mov [edi],eax
       mov [edi+ebx],eax
       mov [edi+3],eax
       mov [edi-3],eax
       add edi,6
       sub     edi,ebx
       loop    ll_6
       pop     edi
       mov     ecx,0x8F*2
ll_7:  mov [edi],eax
       mov [edi+ebx],eax
       add edi,3
       loop ll_7
   
       retn
   
   
fire:
; _ВHИМАHИЕ_! Здесь самое интеpесное.
; Алгоpитм гоpения.
	mov esi,[count1]
 mov edx,[count2]
	mov ebx,320*3
	mov ecx,0xffff
Flame: cmp     esi,0xFA00*3+my_img       ; Псевдослучайная точка в пpедалах экp
       jae     NxtP      ; если HЕТ - беpем следующую.
       lodsd
       dec esi                   ; Считываем ее цвет.
       and      eax,0x00ff0000
                                ; Точка чеpная? (гоpеть нечему?)
       jz      NxtP      ; если ДА - беpем следующую.
       sub     eax,0x00040000
                                   ; Цветом на единицу меньшим
       mov     [esi-2*3],eax       ;    ставим точку слева,
       mov     [esi],eax         ;    спpава,
       mov     [ebx+esi-1*3],eax    ;    снизу
       mov     [esi-0x141*3],eax ;    и свеpху.
NxtP:
       add     esi,edx
       cmp esi,0xffff*3+my_img
       jbe loc_cont
       sub esi,0xfffe*3
loc_cont:
        													          ; Беpем следующую
       add     edx,3
       cmp edx,0x10000*3
       jbe loc_cont2
       sub edx,0xfffe*3
loc_cont2:
                               ;    псевдослучайную точку.
       ;jnz     Flame           ; И так 65536 pаз.
	loop Flame
	mov [count1],esi
 mov [count2],edx
	retn
   
image:
	call penta
	call fire
   
	mov ebx,my_img
	mov ecx,320*65536+200
	mov edx,2*65536+23
	mov eax,7
	int 0x40
   
	inc [fps]
	retn
   
print_fps:
	mov eax,13
	mov ebx,(8+27*6)*65536+18
	mov ecx,8*65536+8
	mov edx,0x00000000
	int 0x40
	mov eax,47
	xor ebx,ebx
	mov bx,3
	shl ebx,16
	mov ecx,[fps]
	mov edx,(8+27*6)*65536+8
	mov esi,0x00400000
	int 0x40
	xor eax,eax
	mov [fps],eax
	retn
   
;DATA
   
count1	dd 0
count2 dd 0
curtime dd 0
fps	dd 0
fire_label	db 'Pentagramm in FIRE     FPS:'
len00=$-fire_label
	org 320*3*2
my_img:
   
I_END:
   
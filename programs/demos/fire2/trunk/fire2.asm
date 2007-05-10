include '..\..\..\macros.inc'
use32
        org     0x0

        db      'MENUET01'
        dd      0x01
        dd      START
        dd      I_END
        dd      mem_end
        dd      mem_end
        dd      0x0, 0x0

START:

        mov     edi, my_img
        mov     ecx, 64001*3/4+1
        xor     eax, eax
        rep     stosd

        mov     al, 3
        mcall
        mov     [curtime], eax

red:
        push    12
        pop     eax
        push    1
        pop     ebx
        mcall

        xor     eax, eax
        mov     ebx, 200*65536+325
        mov     ecx, 150*65536+225
        xor     edx, edx
        xor     esi, esi
        mov     edi, 0x00100000
        mcall

        mov     al, 8
        mov     ebx, (325-19)*65536+12
        mov     ecx, 5*65536+12
        inc     edx
        mov     esi, 0x00400000
        mcall

        mov     al, 4
        mov     ebx, 8*65536+8
        mov     ecx, 0x00FFFFFF;0x00402020
        mov     edx, fire_label
        push    len00
        pop     esi
        mcall

        mov     al, 12
        push    2
        pop     ebx
        mcall

still:

        push    11
        pop     eax
        mcall

        dec     eax
        jz      red
        dec     eax
        jz      key
        dec     eax
        jz      button

        call    image
        push    3
        pop     eax
        mcall
        cmp     eax, [curtime]
        jz      still
        mov     [curtime], eax
print_fps:
        push    13
        pop     eax
        mov     ebx, (8+27*6)*65536+18
        mov     ecx, 8*65536+8
        xor     edx, edx
        mcall
        lea     edx, [ebx-10]
        mov     al, 47
        mov     ebx, 30000h
        xor     ecx, ecx
        xchg    ecx, [fps]
        mov     esi, 0x00FFFFFF ;0x00400000
        mcall
        jmp     still

key:
        mov     al, 2
        mcall
        cmp     ah, 1Bh   ;<Esc>
        jne     still
button:
; we have only one button, close
        or      eax, -1
        mcall

image:

; Рисуем пентагpамму
penta:
	mov edi,my_img+0x540*3
	mov ebx,(320)*3
       mov     eax,0x00fc0000
       push    edi
        push     0x78
        pop     ecx
ll_3:
        call    put_big_point
        lea      edi, [edi+ebx+6]
        loop    ll_3
       pop     edi
        mov     cl, 0x60
ll_4:
        call    put_big_point
       lea      edi, [edi+ebx*2+3]
       loop    ll_4
        mov     cl, 0x61
ll_5:
        call    put_big_point
        sub     edi, 2*320*3-3
       loop    ll_5
        mov     edi, my_img+0x1D4F6
       push    edi
        mov     cl, 0x78
ll_6:
        call    put_big_point
       add edi,6
       sub     edi,ebx
       loop    ll_6
       pop     edi
       mov     ecx,0x8F*2
ll_7:  stosd
       mov [edi+ebx-4],eax
       dec edi
       loop ll_7

fire:
; _ВHИМАHИЕ_! Здесь самое интеpесное.
; Алгоpитм гоpения.
	mov esi,[count1]
        mov edx,[count2]
        mov     ecx, 10000h
Flame: cmp     esi,0xFA00*3+my_img       ; Псевдослучайная точка в пpедалах экp
       jae     NxtP      ; если HЕТ - беpем следующую.
       lodsd
       dec esi                   ; Считываем ее цвет.
;       and      eax,0x00ff0000
        test    eax, eax
                                ; Точка чеpная? (гоpеть нечему?)
       jz      NxtP      ; если ДА - беpем следующую.
       sub     eax,0x00040000
                                   ; Цветом на единицу меньшим
       mov     [esi-2*3],eax       ;    ставим точку слева,
       mov     [esi],eax         ;    спpава,
       mov     [320*3+esi-1*3],eax    ;    снизу
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

	mov ebx,my_img
	mov ecx,320*65536+200
	mov edx,2*65536+23
        push    7
        pop     eax
	int 0x40

	inc [fps]
	retn

put_big_point:
        mov     [edi], eax
        mov     [edi+3], eax
        mov     [edi-3], eax
        mov     [edi+ebx], eax
        ret

;DATA
align 4
   
count1	dd my_img
count2 dd 0
curtime dd 0
fps	dd 0
fire_label	db 'Pentagramm in FIRE     FPS:'
len00=$-fire_label
I_END:

align 16
        rb      320*3
my_img  rb      320*200*3
        rb      320*3

; stack
        align   256
        rb      256
mem_end:

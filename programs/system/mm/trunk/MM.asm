; Programa Mostra memoria
; Projeto: Clovis Bombardelli e Felipe Gustavo Bombardelli
; Usar Fasm Assembler
; Dez/2007


; Infos:
; keys: w - add 0x200 to current memory address
;	s - sub 0x200
;	e - add 0x10
;	d - sub 0x10
;

use32

  org    0x0

STACK_SIZE=1024

  db     'MENUET01'              ; 8 byte id
  dd     0x01                    ; header version
  dd     START                   ; start of code
  dd     I_END                   ; size of image
  dd     I_END+STACK_SIZE        ; memory for app
  dd     I_END+STACK_SIZE        ; esp
  dd     0x0 , 0x0               ; I_Param , I_Icon

include 'lang.inc' ;language support



START:


main:

	call monta_quadro

.a:	mov eax, 11		; espera por evento
	int 0x40			; 0= -> redraw
	or al,al
	jz .h
	cmp al, 2			; 1= -> REDRAW
	jb main			; 2= -> tecla pressionada
	jz .c                         ; 3= -> botão pressionado

; botão
	mov  eax,17      		;
	int 0x40                   	; OBSERVAÇÕES
	cmp  ah,1        		; se o botão apertado for fechar, minimizar, maximizar...todos devem ser do sistema
	jnz  .b     		; mover...etc
	or   eax,-1   		; saida do aplicativo   **************************deve ser do sistema
	int 0x40            	; 					nunca do aplicativo

.b:	jmp .a   	; loop para novo evento


; tecla
.c:	mov  eax,2		; getkey
	int 0x40
	;cmp al, 1
	;jnz .a
	cmp ah, 'w'
      	jne .a1
      	add [end_base], 0x200
	jmp main			;
.a1:  	cmp ah, 's'
      	jne .a2
      	cmp [end_base], dword 0x80000000
      	jbe .a2
      	sub [end_base], 0x200
	jmp main			;
.a2:	cmp ah, 'e'
      	jne .a3
      	add [end_base], 0x10
	jmp main			;
.a3:  	cmp ah, 'd'
      	jne .b1
      	cmp [end_base], dword 0x80000000
      	jbe .b1
      	sub [end_base], 0x10
.b1:	jmp main


; atualiza apenas o codigo hex / ascii
.h:
	call hex_loop
;	inc [contador]		; usado apenas para testar a dinamica
	jmp .a		; retorna para buscar novo evento


monta_quadro:

  ; posição inicial, subfunção 1

    	mov  eax,12
    	mov  ebx,1
    	int 0x40

  ; Desenha o quadro

    	mov  eax,0                     ; função 0 : define e monta o quadro
    	mov  ebx,100*65536+480         ; [x pos] *65536 + [x lar]
    	mov  ecx,100*65536+440         ; [y pos] *65536 + [y alt]
    	mov  edx,[cor_janela]        	 ; cor area RRGGBB
    	mov  edi,titulo                ; Titulo
    	int 0x40

  ; escreve texto no quadro (barra superior)

  	mov edi,[cor_janela]
  	mov  eax,4
	mov ecx, [cor_texto]
	mov  ebx,8*65536+8
	mov  edx,teste
	int 0x40

  	add  ebx,11		; adiciona espaço na coord y

  	mov  eax,4
	mov ecx, [cor_texto]
	mov  edx,barra1
	int 0x40

 	call hex_loop

  ; posição final, subfunção 2

.d:	mov  eax,12
    	mov  ebx,2
    	int 0x40

	ret

  ; buscar conteudo da memoria para mostrar

hex_loop:
   	xor eax, eax
   	mov ebx, [end_base]	; endereço base

  ; imprime o endereço

   	mov edx, 8*65536+40 ; posição inicial hex
   	mov [pos_ascii], edx
   	add [pos_ascii], 360*65536	; posição inicial ascii
   	mov word [pos_x], 0
   	mov al, [quant_linhas]
	mov edi,[cor_janela]

.c:   	push eax		;
   	push edx		;

   	push ebx		; salva endereço base
   	mov al, 47
   	mov ecx, ebx	; imprime o endereço
   	mov ebx, 0x00080100
   	mov esi, [cor_endereco]
   	int 0x40
	pop ebx		;

	add edx, 60*65536	; pula 8 espaços mais alguma coisa

  ; busca hexcode

	mov esi, [cor_hexcode]
	mov al, 4

.b:	push eax		; ----------------------------------------------------------ok

 	mov al, 72	; ebx= endereço de leitura (nao funcionou...) change this service to another
	int 0x40		; retorna ecx= dword da posição ebx

   	push ebx	 	; ----------------------------------------------------------ok
 	mov ebx, 0x00020100	; 2 digitos, modo hex
 	mov al, 4

.a:	push eax		; controle -------------------------------------------------ok
 	push ecx		; codigo
 	push edx		; posição
 	and ecx, 0xff
 	mov al, 47
 	int 0x40
 	call imprime_car$
 	pop edx		;
 	pop ecx		;
 	shr ecx, 8
 	add edx, (18)*65536
 	pop eax		; ----------------------------------------------------------ok
 	dec al
 	jnz .a

 	pop ebx		; ----------------------------------------------------------ok
 	add ebx, 4	; soma 4 ao endereço base
 	pop eax		; ----------------------------------------------------------ok
 	dec al		; imprimir mais 4 vezes incrementando ebx em 4
 	jnz .b

	pop edx		;
 	add edx, 10	;

 ; imprime a string ascii

 	push ebx
 	push edx
 	mov al, 4
      	mov ebx, [pos_ascii]	; endereço na tela
      	mov edx, ascii_string	; endereço da string
      	or ecx, [cor_ascii]		; cor
      	int 0x40
 	mov [ascii_x], byte 0
 	add [pos_ascii], 10
 	pop edx
 	pop ebx

 	pop eax		;
 	dec al
 	jnz .c
	ret

imprime_car$:		; ecx tem o caractere a imprimir

	push ebx
	xor ebx, ebx
	mov bl, [ascii_x]
	or cl, cl
      	jnz .c
      	mov cl, 0x20
.c:	mov [ebx+ascii_string], cl
	inc [ascii_x]
	pop ebx
	ret


; para acessar a memoria do sistema:
;
;	acessar a base das tabelas de paginaçao
;	com o endereço buscar o descritor de pagina
; 	ler o endereço físico ndo descritor
; ou 	buscar a pagina do sistema e ler nele o endereço fisico

; DATA AREA
align 0x10
;contador		dd 0			; para testar a dinamica
cor_janela  	dd 0x346f72c3		; atrib, tipo, corRGB
cor_texto 	dd 0xc0fffffe		; fonte variavel, cor branca
cor_endereco	dd 0xc0fffffc
cor_hexcode	dd 0xc0fffffd
cor_ascii		dd 0xc0fffffa
pos_x		db 0
pos_y		db 0
quant_linhas	db 0x20
ascii_x		db 0
end_base		dd 0x80000000		; ok
pos_ascii		dd 0
ascii_string	rb 0x11
reservado		rb 3

if lang eq it
	titulo    db 'Visualizza la memoria del sistema operativo',0
	teste	db '--------  ----------------- Codice Hex  -----------------   -- A S C I I  --',0
	barra1	db 'Indirizzo:00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f   0123456789abcdef',0

else if lang eq es
	titulo    db 'Mostra a memoria do sistema operacional',0
	teste	db '--------  ------------------ Hex Code -------------------   -- A S C I I  --',0
	barra1	db 'Address:  00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f   0123456789abcdef',0

else
	titulo    db 'Displays the operating system memory',0
	teste	db '--------  ------------------ Hex Code -------------------   -- A S C I I  --',0
	barra1	db 'Address:  00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f   0123456789abcdef',0
end if

align 0x10
I_END:


; service 72 from system calls
;
; must contain:
;       mov eax, [eax]
;	mov [esp+32], eax
;	ret



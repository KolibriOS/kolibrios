; CTV!

bits 32
section .text
;extern "C" int blur_bitmap_16
;  (unsigned char *dest,int len);

global blur_bitmap_16
global test_ctv
times ($$-$) & 3 db 0
blur_bitmap_16:
push edi ; point to screen
push edx ; temporary for unwrapping
push ecx ; count
push ebx ; last pixel

; ax = current pixel

xor ebx,ebx ; Last pixel

mov edi,[esp+20] ; edi is a pointer to the bitmap
mov ecx,[esp+24] ; ecx is the amount of pixels to blur

blur16_loop:
mov ax,word[edi]

mov dx,ax
and ax,0x07e0
and dx,0xf81f
rol eax,16
mov ax,dx
; Now we have unwrapped the green middle out of eax
mov edx,eax ; ebx=this pixel (unwrapped) before we changed it
add eax,ebx ; add last pixel to this one
mov ebx,edx ; ebx=this pixel for next time

sar eax,1

; wrap up again
mov edx,eax
ror edx,16
and dx,0x07e0
and ax,0xf81f
or  ax,dx
;finished pixel in ax

;mov word[edi],ax
;add edi,2
stosw
loop blur16_loop

pop  ebx
pop  ecx
pop  edx
pop  edi
xor eax,eax
ret


; --------------------------------------

bits 32
section .text
;extern "C" int blur_bitmap_15
;  (unsigned char *dest,int len);

global blur_bitmap_15
times ($$-$) & 3 db 0
blur_bitmap_15:
push edi ; point to screen
push edx ; temporary for unwrapping
push ecx ; count
push ebx ; last pixel

; ax = current pixel

xor ebx,ebx ; Last pixel

mov edi,[esp+20] ; edi is a pointer to the bitmap
mov ecx,[esp+24] ; ecx is the amount of pixels to blur

blur15_loop:
mov ax,word[edi]

mov dx,ax
and ax,0x03e0
and dx,0x7c1f
rol eax,16
mov ax,dx
; Now we have unwrapped the green middle out of eax
mov edx,eax ; ebx=this pixel (unwrapped) before we changed it
add eax,ebx ; add last pixel to this one
mov ebx,edx ; ebx=this pixel for next time

sar eax,1

; wrap up again
mov edx,eax
ror edx,16
and dx,0x03e0
and ax,0x7c1f
or  ax,dx
;finished pixel in ax

;mov word[edi],ax
;add edi,2
stosw
loop blur15_loop

pop  ebx
pop  ecx
pop  edx
pop  edi
xor eax,eax
ret

; ----------------------------

bits 32
section .text
;extern "C" int test_ctv
;  (unsigned char *dest,int len);

global _test_ctv
times ($$-$) & 3 db 0
test_ctv:
push edi ; point to screen
push edx ; temporary for unwrapping
push ecx ; count
push ebx ; last pixel

; ax = current pixel

xor ebx,ebx ; Last pixel

mov edi,[esp+20] ; edi is a pointer to the bitmap
mov ecx,[esp+24] ; ecx is the amount of pixels to blur

test_loop:
mov ax,word[edi]

mov dx,ax
rol eax,16
mov ax,dx
mov edx,eax ; ebx=this pixel (unwrapped) before we changed it
mov ebx,edx ; ebx=this pixel for next time

sar eax,1

mov edx,eax
ror edx,16
;finished pixel in ax

;mov word[edi],ax
;add edi,2
stosw
loop test_loop

pop  ebx
pop  ecx
pop  edx
pop  edi
xor eax,eax
ret

%ifdef NASM_STACK_NOEXEC
section .note.GNU-stack noalloc noexec nowrite progbits
%endif

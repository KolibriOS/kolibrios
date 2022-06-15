bits 32
section .text
;extern "C" int asm_memcpy
;  (unsigned char *dest, unsigned char *src, int len);

global asm_memcpy

times ($$-$) & 3 db 0

asm_memcpy:

pushad			; save registers
mov edi,[esp+36]	; get 1st argument
mov esi,[esp+40]	; ...2nd
mov eax,[esp+44]	; ...3rd

mov edx, eax
shr eax, 2		; figure out how many 4 byte chunks we have
and edx, 3		; also figure out slack
test eax, eax		; Do we have any big chunks?
push edx
jz .slack		; If not, just do slack

mov ecx,eax

rep movsd		; move 4 byte chunks

.slack:
pop ecx
rep movsb		; move 1 byte slack

popad			; clean up
ret

; --------------------------------------

%ifdef NASM_STACK_NOEXEC
section .note.GNU-stack noalloc noexec nowrite progbits
%endif

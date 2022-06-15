bits 32
section .text
;extern "C" int mmx_memcpy
;  (unsigned char *dest, unsigned char *src, int len);

global mmx_memcpy

times ($$-$) & 3 db 0

mmx_memcpy:

pushad			; save registers
mov edi,[esp+36]	; get 1st argument
mov esi,[esp+40]	; ...2nd
mov eax,[esp+44]	; ...3rd

mov edx, eax
shr eax, byte 3		; figure out how many 8 byte chunks we have
and edx, byte 7		; also figure out slack
test eax, eax		; Do we have any big chunks?
push edx
jz .slack		; If not, let's just do slack

mov ecx,eax

.mmx_move:
  movq mm0,qword[esi]	; move 8 byte blocks using MMX
  movq qword[edi],mm0
  add esi, byte 8	; increment pointers
  add edi, byte 8
loopnz .mmx_move	; continue until CX=0

.slack:
pop ecx
rep movsb		; move 1 byte slack

emms			; Free up for the FPU

popad			; clean up
ret

; --------------------------------------

%ifdef NASM_STACK_NOEXEC
section .note.GNU-stack noalloc noexec nowrite progbits
%endif

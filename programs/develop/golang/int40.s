
SECTION .text

[GLOBAL __start]
extern go.kernel.Load

global go.os.Sleep
global go.os.Event
global go.os.Button
global go.os.Exit
global go.os.Redraw
global go.os.Window

__start:
  call  go.kernel.Load
  ret

go.os.Sleep:
    push ebp
    mov ebp,esp
	mov eax, 5
	mov ebx, [ebp+8]
	int 0x40
    mov esp,ebp
    pop ebp
    ret


go.os.Event:
	push ebp
    mov ebp,esp
	mov eax, 10
	int 0x40
	mov esp,ebp
    pop ebp
	ret

go.os.Button:
    push ebp
    mov ebp,esp
	mov eax, 17
	int 0x40
    mov esp,ebp
    pop ebp
    ret

go.os.Exit:
    push ebp
    mov ebp,esp
	mov eax, -1
	int 0x40
    mov esp,ebp
    pop ebp
    ret


go.os.Redraw:
    push ebp
    mov ebp,esp
	mov eax, 12
	mov ebx, [ebp+8]
	int 0x40
    mov esp,ebp
    pop ebp
    ret

go.os.Window:
    push ebp
    mov ebp,esp
	mov ebx, [ebp+8]
	shl ebx, 16
	or ebx, [ebp+16]
	mov ecx, [ebp+12]
	shl ecx, 16
	or ecx, [ebp+20]
	mov edx, 0x14
	shl edx, 24
	or edx, 0xFFFFFF
	mov esi, 0x808899ff
	mov edi, [ebp+24]
	xor eax, eax
	int 0x40
    mov esp,ebp
    pop ebp
    ret


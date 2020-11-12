
SECTION .text

[GLOBAL __start]
extern go.kernel.Load

global go.os.Sleep
global go.os.Event
global go.os.GetButtonID
global go.os.CreateButton
global go.os.Exit
global go.os.Redraw
global go.os.Window
global go.os.WriteText
global go.os.GetTime
global go.os.DrawLine
global go.os.DrawBar
global go.os.DebugOutHex
global go.os.DebugOutChar
global go.os.DebugOutStr

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

go.os.GetButtonID:
  mov   eax,17
  int   0x40
  test  al,al
  jnz   .no_button
  shr   eax,8
  ret
.no_button:
  xor   eax,eax
  dec   eax
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

go.os.WriteText:
    mov eax,4
    mov ebx,[esp+4]
    shl ebx,16
    mov bx,[esp+8]
    mov ecx,[esp+12]
    mov edx,[esp+16]
    mov esi,[esp+20]
    int 0x40
    ret

go.os.DrawLine:  
    push ebx
    mov ebx,[esp+8]
    shl ebx,16
    mov bx,[esp+16]
    mov ecx,[esp+12]
    shl ecx,16
    mov cx,[esp+20]
    mov edx,[esp+24]
    mov eax,38
    int 0x40
    pop ebx
    ret

go.os.DrawBar:
    push  ebx
    mov   eax,13
    mov   ebx,[esp+8]
    shl   ebx,16
    mov   bx,[esp+16]
    mov   ecx,[esp+12]
    shl   ecx,16
    mov   cx,[esp+20]
    mov   edx,[esp+24]
    int   0x40
    pop   ebx
    ret

go.os.GetTime:
    mov eax, 3
    int 0x40
    ret

go.os.DebugOutHex: 
    mov eax, [esp+4]
    mov   edx, 8
    .new_char:
    rol   eax, 4
    movzx ecx, al
    and   cl,  0x0f
    mov   cl,  [__hexdigits + ecx]
    pushad
    mov eax, 63
    mov ebx, 1
    int 0x40
    popad
    dec   edx
    jnz   .new_char
    ret

go.os.DebugOutChar:        
   mov al, [esp+4]
   pushf
   pushad
   mov  cl,al
   mov  eax,63
   mov  ebx,1
   int  0x40
   popad
   popf
   ret

go.os.DebugOutStr:
   mov  edx,[esp+4]
   mov  eax,63
   mov  ebx,1
 m2:
   mov  cl, [edx]
   test cl,cl
   jz   m1
   int  40h
   inc  edx
   jmp  m2
 m1:
   ret

go.os.CreateButton:
  push  ebx
  push  esi
  mov   ebx,[esp+12]
  shl   ebx,16
  mov   bx,[esp+20]
  mov   ecx,[esp+16]
  shl   ecx,16
  mov   cx,[esp+24]
  mov   edx,[esp+28]
  mov   esi,[esp+32]
  mov   eax,8
  int   0x40
  pop   esi 
  pop   ebx
  ret

SECTION .data
__hexdigits:
  db '0123456789ABCDEF'

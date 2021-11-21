
SECTION .text

global go.kos.Sleep
global go.kos.Event
global go.kos.GetButtonID
global go.kos.CreateButton
global go.kos.Exit
global go.kos.Redraw
global go.kos.Window
global go.kos.WriteText
global go.kos.GetTime
global go.kos.DrawLine
global go.kos.DrawBar
global go.kos.DebugOutHex
global go.kos.DebugOutChar
global go.kos.DebugOutStr
global go.kos.WriteText2

global runtime.memequal32..f
runtime.memequal32..f:
    ret

global runtime.memequal8..f
runtime.memequal8..f:
    ret

global runtime.memequal
runtime.memequal:
    ret

global go.kos.SetByteString
go.kos.SetByteString:
  push ebp
  mov ebp, esp
  mov eax, [ebp+8]
  mov ebx, [ebp+12]
  mov ecx, [ebp+16]
  mov dh, [ebp+20]
  mov byte[eax+ecx], dh
  mov esp, ebp
  pop ebp
  ret

global __go_runtime_error
global __go_register_gc_roots
global __unsafe_get_addr

__unsafe_get_addr:
  push ebp
  mov ebp, esp
  mov eax, [ebp+8]
  mov esp, ebp
  pop ebp
  ret

__go_register_gc_roots:
__go_runtime_error:
  ret

global runtime.writeBarrier
global runtime.gcWriteBarrier
runtime.writeBarrier:
    mov eax, [esp+8]
    mov ebx, [esp+12]
    mov dword[eax], ebx
    ret

global runtime.strequal..f
runtime.strequal..f:
    mov eax,[esp+8]
    mov ebx,[esp+16]
    mov ecx,0
    strcmp_loop:
        mov byte dl,[eax+ecx]
        mov byte dh,[ebx+ecx]
        inc ecx
        cmp dl,0
        je strcmp_end_0
        cmp byte dl,dh
        je strcmp_loop
        jl strcmp_end_1
        jg strcmp_end_2
    strcmp_end_0:
        cmp dh,0
        jne strcmp_end_1
        xor ecx,ecx
        ret
    strcmp_end_1:
        mov ecx,1
        ret
    strcmp_end_2:
        mov ecx,-1
        ret

runtime.gcWriteBarrier:
    mov eax, [esp+8]
    mov ebx, [esp+12]
    mov dword[eax], ebx
    ret

global runtime.goPanicIndex
runtime.goPanicIndex:
    ret

global runtime.registerGCRoots
runtime.registerGCRoots:
    ret

global memcmp
memcmp:
    push ebp
    mov ebp,esp
    mov     esi, [ebp+8]    ; Move first pointer to esi
    mov     edi, [ebp+12]    ; Move second pointer to edi
    mov     ecx, [ebp+16]    ; Move length to ecx

    cld                         ; Clear DF, the direction flag, so comparisons happen
                                ; at increasing addresses
    cmp     ecx, ecx            ; Special case: If length parameter to memcmp is
                                ; zero, don't compare any bytes.
    repe cmpsb                  ; Compare bytes at DS:ESI and ES:EDI, setting flags
                                ; Repeat this while equal ZF is set
    setz    al
    mov esp,ebp
    pop ebp
    ret


go.kos.Sleep:
    push ebp
    mov ebp,esp
	mov eax, 5
	mov ebx, [ebp+8]
	int 0x40
    mov esp,ebp
    pop ebp
    ret


go.kos.Event:
	mov eax, 10
	int 0x40
	ret

go.kos.GetButtonID:
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

go.kos.Exit:
	mov eax, -1
	int 0x40
    ret

go.kos.Redraw:
    push ebp
    mov ebp,esp
	mov eax, 12
	mov ebx, [ebp+8]
	int 0x40
    mov esp,ebp
    pop ebp
    ret

go.kos.Window:
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

go.kos.WriteText:
    push ebp
    mov ebp,esp
    mov eax,4
    mov ebx,[ebp+8]
    shl ebx,16
    mov bx,[ebp+12]
    mov ecx,[ebp+16]
    mov edx,[ebp+20]
    mov esi,[ebp+24]
    int 0x40
    mov esp,ebp
    pop ebp
    ret

go.kos.WriteText2:
    push ebp
    mov ebp,esp
    mov eax,47
    mov ebx,[ebp+8]
    shl ebx,16
    mov ecx,[ebp+12]
    mov edx,[ebp+20]
    shl edx,16
    add edx, [ebp+24]
    mov esi,[ebp+28]
    int 0x40
    mov esp,ebp
    pop ebp
    ret

go.kos.DrawLine:
    push ebp
    mov ebp,esp
    mov ebx,[ebp+8]
    shl ebx,16
    mov bx,[ebp+16]
    mov ecx,[ebp+12]
    shl ecx,16
    mov cx,[ebp+20]
    mov edx,[ebp+24]
    mov eax,38
    int 0x40
    mov esp,ebp
    pop ebp
    ret

go.kos.DrawBar:
    push ebp
    mov ebp,esp
    mov   eax,13
    mov   ebx,[ebp+8]
    shl   ebx,16
    mov   bx,[ebp+16]
    mov   ecx,[ebp+12]
    shl   ecx,16
    mov   cx,[ebp+20]
    mov   edx,[ebp+24]
    int   0x40
    mov esp,ebp
    pop ebp
    ret

go.kos.GetTime:
    mov eax, 3
    int 0x40
    ret

go.kos.DebugOutHex:
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

go.kos.DebugOutChar:
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

go.kos.DebugOutStr:
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

go.kos.CreateButton:
  push  ebp
  mov   ebp,esp
  mov   eax, 8
  mov ebx, [ebp+8]
  shl ebx, 16
  mov bx, [ebp+16]
  mov ecx, [ebp+12]
  shl ecx, 16
  mov cx, [ebp+20]
  mov edx, [ebp+24]
  mov esi, [ebp+28]
  int   0x40
  mov   esp,ebp
  pop   ebp
  ret

SECTION .data
__hexdigits:
  db '0123456789ABCDEF'

__test:
    dd __hexdigits
    dd 15
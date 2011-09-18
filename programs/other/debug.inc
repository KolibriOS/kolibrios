macro debug_print str
{
   local ..string, ..label

   jmp ..label
   ..string db str,0
  ..label:

   pushf
   pushad
   mov  edx,..string
   call debug_outstr
   popad
   popf
}

dps fix debug_print

macro debug_print_dec arg
{
   pushf
   pushad
   if ~arg eq eax
     mov  eax,arg
   end if
   call debug_outdec
   popad
   popf
}

dpd fix debug_print_dec

;---------------------------------
debug_outdec:           ;(eax - num, edi-str)
        push 10         ;2
        pop ecx         ;1
        push -'0'       ;2
    .l0:
        xor edx,edx     ;2
        div ecx         ;2
        push edx        ;1
        test eax,eax    ;2
        jnz .l0         ;2
    .l1:
        pop eax         ;1
        add al,'0'      ;2
        call debug_outchar ; stosb
        jnz .l1         ;2
        ret             ;1
;---------------------------------

debug_outchar:          ; al - char
   pushf
   pushad
   mov  cl,al
   mov  eax,63
   mov  ebx,1
   mcall
   popad
   popf
ret

debug_outstr:
   mov  eax,63
   mov  ebx,1
 @@:
   mov  cl,[edx]
   test cl,cl
   jz   @f
   mcall
   inc  edx
   jmp  @b
 @@:
   ret

_debug_crlf db 13, 10, 0

macro newline
{
  pushf
  pushad
  mov edx, _debug_crlf
  call debug_outstr
  popad
  popf
}

macro print message
{
  dps message
  newline
}

macro pregs
{
  dps "EAX: "
  dpd eax
  dps "   EBX: "
  dpd ebx
  newline
  dps "ECX: "
  dpd ecx
  dps "   EDX: "
  dpd edx
  newline
}

macro debug_print_hex arg
{
    pushf
    pushad
    if ~arg eq eax
      mov eax, arg
    end if
    call debug_outhex
    popad
    popf
}
dph fix debug_print_hex

debug_outhex:
    ;  eax - number
    mov   edx, 8
  .new_char:
    rol   eax, 4
    movzx ecx, al
    and   cl,  0x0f
    mov   cl,  [__hexdigits + ecx]
    pushad
    mcall 63, 1
    popad
    dec   edx
    jnz   .new_char
ret

__hexdigits:
  db '0123456789ABCDEF'
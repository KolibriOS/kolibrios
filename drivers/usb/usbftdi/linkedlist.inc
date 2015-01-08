;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2015. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

head    dd 0

linkedlist_add:
    push    ebx
    mov     ebx, [head]                
    mov     [head], eax
    mov     [eax + node.next], ebx    
    pop     ebx
    ret

linkedlist_unlink:
    push    ebx ecx
    test    eax, eax
    jz      .invalid_pointer
    mov     ebx, eax     ; eax - pointer to node for unlink
    cmp     eax, [head]
    jz      .unlink_head
  .getnext:
    cmp     [ebx+node.next], eax
    jz      .unlink
    cmp     [ebx+node.next], 0
    jz      .invalid_pointer
    mov     ebx, [ebx+node.next]
    jmp     .getnext
  .unlink:
    mov     ecx, [eax+node.next]
    mov     [ebx+node.next], ecx
    jmp     @f    
  .unlink_head:
    mov     ebx, [eax+node.next]
    mov     [head], ebx
  @@:
  .invalid_pointer:
    pop     ecx ebx
    ret
    
linkedlist_isvalid:       ;eax - pointer to check; returns zero if valid
    push    ebx ecx
    xor     ecx, ecx
    test    eax, eax
    jz      .invalid_pointer 
    mov     ebx, [head]
  .next_check:
    cmp     eax, ebx
    jz      .valid_pointer
    mov     ebx, [ebx + node.next]
    test    ebx, ebx
    jz      .invalid_pointer
    jmp     .next_check      
  .invalid_pointer:  
    inc     ecx
  .valid_pointer:
    mov     eax, ecx
    pop     ecx ebx
    ret

linkedlist_gethead:
    mov     eax, [head]
    ret

restore node
restore node.next
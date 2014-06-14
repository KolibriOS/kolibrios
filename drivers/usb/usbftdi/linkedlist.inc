head dd 1
node equ ftdi_context
node.next equ ftdi_context.next_context
linkedlist:
  .init:
        push   eax
        xor    eax, eax
        mov    [head], eax
        pop    eax
        ret

  .add:
        push   ebx
        mov	   ebx, [head]				
        mov	   [head], eax
        mov	   [eax + node.next], ebx	
        pop    ebx
        ret

  .delete:
        push   ebx ecx
        mov    ebx, eax	    ; eax - pointer to node for delete
        cmp    eax, [head]
        jz     .unlink_head

  .getnext:
        cmp    [ebx+node.next], eax
        jz     .unlink
        cmp    [ebx+node.next], 0
        jz     .invalid_pointer
        mov    ebx, [ebx+node.next]
        jmp    .getnext

  .unlink:
        mov    ecx, [eax+node.next]
        mov    [ebx+node.next], ecx
        jmp    @f


  .unlink_head:
        mov    ebx, [eax+node.next]
        mov    [head], ebx
  @@:
        mov    ecx, eax
        call   Kfree
  .invalid_pointer:
        pop    ecx ebx
        ret

  .gethead:
        mov	   eax, [head]
        ret

restore node
restore next
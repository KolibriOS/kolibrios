;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2014. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

head dd 0
size dd 0

linkedlist_add:
    push ebx
	mov	ebx, [head]				
	mov	[head], eax
	mov	[eax + node.next], ebx	
    pop ebx
	ret

linkedlist_delete:
    push ebx ecx
	mov ebx, eax	 ; eax - pointer to node for delete
	cmp eax, [head]
	jz  .unlink_head

  .getnext:
	cmp [ebx+node.next], eax
	jz  .unlink
	cmp [ebx+node.next], 0
	jz  .invalid_pointer
	mov ebx, [ebx+node.next]
	jmp .getnext

  .unlink:
	mov ecx, [eax+node.next]
	mov [ebx+node.next], ecx
	jmp @f
    
  .unlink_head:
	mov ebx, [eax+node.next]
	mov [head], ebx
  @@:
	mov   ecx, eax
	call  Kfree
  .invalid_pointer:
    pop   ecx ebx
	ret
    
linkedlist_isvalid:
    push  ebx ecx
    xor   ecx, ecx
    mov   ebx, [head]
  .next_check:
    cmp   eax, ebx
    jz    .valid_pointer
    mov   ebx, [ebx + node.next]
    test  ebx, ebx
    jz    .invalid_pointer
    jmp   .next_check      
  .invalid_pointer:  
    inc   ecx
  .valid_pointer:
    mov   eax, ecx
    pop   ecx ebx
    ret

linkedlist_gethead:
  	mov	eax, [head]
  	ret

restore node
restore node.next
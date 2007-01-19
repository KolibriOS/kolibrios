
HEAP_BASE equ 0x00C00000
;HEAP_SIZE equ 0x01000000

struc MEM_BLOCK
{  .next_block  dd ?
   .prev_block  dd ? ;+4
   .list_next   dd ? ;+8
   .list_prev   dd ? ;+12
   .base        dd ? ;+16
   .size        dd ? ;+20
   .flags       dd ? ;+24
   .handle      dd ? ;+28
}

FREE_BLOCK      equ  4
USED_BLOCK      equ  8

virtual at 0
  MEM_BLOCK MEM_BLOCK
end virtual

MEM_BLOCK_SIZE equ 8*4

block_next   equ MEM_BLOCK.next_block
block_prev   equ MEM_BLOCK.prev_block
list_next    equ MEM_BLOCK.list_next
list_prev    equ MEM_BLOCK.list_prev
block_base   equ MEM_BLOCK.base
block_size   equ MEM_BLOCK.size
block_flags  equ MEM_BLOCK.flags

macro calc_index op
{          shr op, 12
           dec op
           cmp op, 63
           jna @f
           mov op, 63
@@:
}

macro remove_from_list op
{          mov edx, [op+list_next]
           mov ecx, [op+list_prev]
           test edx, edx
           jz @f
           mov [edx+list_prev], ecx
@@:
           test ecx, ecx
           jz @f
           mov [ecx+list_next], edx
@@:
           mov [op+list_next],0
           mov [op+list_prev],0
}

macro remove_from_free op
{
           remove_from_list op

           mov eax, [op+block_size]
           calc_index eax
           cmp [mem_block_list+eax*4], op
           jne @f
           mov [mem_block_list+eax*4], edx
@@:
           cmp [mem_block_list+eax*4], 0
           jne @f
           btr [mem_block_mask], eax
@@:
}

macro remove_from_used op
{
           remove_from_list op
           cmp [mem_used_list], op
           jne @f
           mov [mem_used_list], edx
@@:
}

align 4
proc init_kernel_heap

           mov ecx, 64/4
           mov edi, mem_block_list
           xor eax, eax
           cld
           rep stosd

           mov ecx, 512/4
           mov edi, mem_block_map
           not eax
           rep stosd

           mov [mem_block_start], mem_block_map
           mov [mem_block_end], mem_block_map+512
           mov [mem_block_arr], HEAP_BASE

           stdcall alloc_pages, dword 32
           mov ecx, 32
           mov edx, eax
           mov edi, HEAP_BASE
.l1:
           stdcall map_page,edi,edx,PG_SW
           add edi, 0x1000
           add edx, 0x1000
           dec ecx
           jnz .l1

           mov edi, HEAP_BASE
           mov ebx, edi
           add ebx, MEM_BLOCK_SIZE
           xor eax, eax
           mov [edi+block_next], ebx
           mov [edi+block_prev], eax
           mov [edi+list_next], eax
           mov [edi+list_prev], eax
           mov [edi+block_base], HEAP_BASE
           mov [edi+block_size], 4096*MEM_BLOCK_SIZE
           mov [edi+block_flags], USED_BLOCK

           mov [ebx+block_next], eax
           mov [ebx+block_prev], eax
           mov [ebx+list_next], eax
           mov [ebx+block_base], HEAP_BASE+4096*MEM_BLOCK_SIZE

           mov ecx, [MEM_AMOUNT]
           sub ecx, HEAP_BASE + 4096*MEM_BLOCK_SIZE
           mov [heap_size], ecx
           mov [heap_free], ecx
           mov [ebx+block_size], ecx
           mov [ebx+block_flags], FREE_BLOCK

           mov [mem_block_mask], eax
           mov [mem_block_mask+4],0x80000000

           mov [mem_used_list], eax
           mov [mem_block_list+63*4], ebx
           mov byte [mem_block_map], 0xFC
           and [heap_mutex], 0
           mov [heap_blocks], 4095
           mov [free_blocks], 4095
           ret
endp

align 4
proc get_block stdcall, index:dword

           mov eax, 63
           mov ecx, [index]
           cmp ecx, eax
           jna @f
           ;cmova ecx, eax
           mov ecx, eax
@@:
           xor esi, esi
           xor ebx, ebx
           xor edx, edx
           not edx

           cmp ecx, 32
           jb .bit_test

           sub ecx, 32
           add ebx, 32
           add esi, 4

.bit_test:
           shl edx, cl
           and edx, [mem_block_mask+esi]
           jz .high_mask
           bsf eax, edx
           add ebx, eax
           mov eax, [mem_block_list+ebx*4]
           ret

.high_mask:

           add esi, 4
           add ebx, 32
           test esi, 0xFFFFFFF8
           jnz .big_error
           mov edx, [mem_block_mask+esi]
           and edx, edx
           jz .high_mask
           bsf eax, edx
           add ebx, eax
           mov eax, [mem_block_list+ebx*4]
           ret

.big_error:
           xor eax, eax
           ret
endp

align 4
proc alloc_mem_block

           mov ebx, [mem_block_start]
           mov ecx, [mem_block_end]
.l1:
           bsf eax,[ebx];
           jnz found
           add ebx,4
           cmp ebx, ecx
           jb .l1
           xor eax,eax
           ret

found:
           btr [ebx], eax
           mov [mem_block_start],ebx
           sub ebx, mem_block_map
           lea eax,[eax+ebx*8]
           shl eax, 5
           add eax, [mem_block_arr]
           dec [free_blocks]
           ret
endp

proc free_mem_block
           sub eax, [mem_block_arr]
           shr eax, 5

           mov ebx, mem_block_map
           bts [ebx], eax
           inc [free_blocks]
           shr eax, 3
           and eax, not 3
           add eax, ebx
           cmp [mem_block_start], eax
           ja @f
           ret
@@:
           mov [mem_block_start], eax
	   ret
.err:
           xor eax, eax
	   ret
endp

align 4
proc alloc_kernel_space stdcall, size:dword
           local block_ind:DWORD

           mov eax, [size]
           add eax, 4095
           and eax, not 4095
           mov [size], eax

           mov ebx, heap_mutex
           call wait_mutex    ;ebx

           cmp eax, [heap_free]
           ja .error

           shr eax, 12
           sub eax, 1

           mov [block_ind], eax

           stdcall get_block, eax
           test eax, eax
           jz .error

           mov edi, eax           ;edi - pBlock

           cmp [edi+block_flags], FREE_BLOCK
           jne .error

           mov [block_ind], ebx   ;index of allocated block

           mov eax, [edi+block_size]
           cmp eax, [size]
           je .m_eq_size

           call alloc_mem_block
           and eax, eax
           jz .error

           mov esi, eax           ;esi - splitted block

           mov [esi+block_next], edi
           mov eax, [edi+block_prev]
           mov [esi+block_prev], eax
           mov [edi+block_prev], esi
           mov [esi+list_next], 0
           mov [esi+list_prev], 0
           and eax, eax
           jz @f
           mov [eax+block_next], esi
@@:
           mov ebx, [edi+block_base]
           mov [esi+block_base], ebx
           mov edx, [size]
           mov [esi+block_size], edx
           add [edi+block_base], edx
           sub [edi+block_size], edx

           mov eax, [edi+block_size]
           shr eax, 12
           sub eax, 1
           cmp eax, 63
           jna @f
           mov eax, 63
@@:
           cmp eax, [block_ind]
           je .m_eq_ind

           remove_from_list edi

           mov ecx, [block_ind]
           mov [mem_block_list+ecx*4], edx

           test edx, edx
           jnz @f
           btr [mem_block_mask], ecx
@@:
           mov edx, [mem_block_list+eax*4]
           mov [edi+list_next], edx
           test edx, edx
           jz @f
           mov [edx+list_prev], edi
@@:
           mov [mem_block_list+eax*4], edi
           bts [mem_block_mask], eax
.m_eq_ind:
           mov ebx, [mem_used_list]
           mov [esi+list_next], ebx
           test ebx, ebx
           jz @f
           mov [ebx+list_prev], esi
@@:
           mov [esi+block_flags], USED_BLOCK
           mov [mem_used_list], esi
           mov eax, [esi+block_base]
           mov ebx, [size]
           sub [heap_free], ebx
           and [heap_mutex], 0
           ret
.m_eq_size:
           remove_from_list edi
           mov [mem_block_list+ebx*4], edx
           and edx, edx
           jnz @f
           btr [mem_block_mask], ebx
@@:
           mov ecx, [mem_used_list]
           mov [edi+list_next], ecx
           test ecx, ecx
           jnz @f
           mov [ecx+list_prev], edi
@@:
           mov [mem_used_list], edi
           mov [edi+block_flags], USED_BLOCK
           mov eax, [edi+block_base]
           mov ebx, [size]
           sub [heap_free], ebx
           and [heap_mutex], 0
           ret
.error:
           xor eax, eax
           mov [heap_mutex], eax
           ret
endp

align 4
proc free_kernel_space stdcall, base:dword

           mov ebx, heap_mutex
           call wait_mutex    ;ebx

           mov eax, [base]
           mov esi, [mem_used_list]
@@:
           test esi, esi
           jz .fail

           cmp [esi+block_base], eax
           je .found
           mov esi, [esi+list_next]
           jmp @b
.found:
           cmp [esi+block_flags], USED_BLOCK
           jne .fail

           mov eax, [esi+block_size]
           add [heap_free], eax

           mov edi, [esi+block_next]
           test edi, edi
           jz .prev

           cmp [edi+block_flags], FREE_BLOCK
           jne .prev

           remove_from_free edi

           mov edx, [edi+block_next]
           mov [esi+block_next], edx
           test edx, edx
           jz @f

           mov [edx+block_prev], esi
@@:
           mov ecx, [edi+block_size]
           add [esi+block_size], ecx

           mov eax, edi
           call free_mem_block
.prev:
           mov edi, [esi+block_prev]
           test edi, edi
           jz .insert

           cmp [edi+block_flags], FREE_BLOCK
           jne .insert

           remove_from_used esi

           mov edx, [esi+block_next]
           mov [edi+block_next], edx
           test edx, edx
           jz @f
           mov [edx+block_prev], edi
@@:
           mov eax, esi
           call free_mem_block

           mov ecx, [edi+block_size]
           mov eax, [esi+block_size]
           add eax, ecx
           mov [edi+block_size], eax

           calc_index eax
           calc_index ecx
           cmp eax, ecx
           je .m_eq

           push ecx
           remove_from_list edi
           pop ecx

           cmp [mem_block_list+ecx*4], edi
           jne @f
           mov [mem_block_list+ecx*4], edx
@@:
           cmp [mem_block_list+ecx*4], 0
           jne @f
           btr [mem_block_mask], ecx
@@:
           mov esi, [mem_block_list+eax*4]
           mov [mem_block_list+eax*4], edi
           mov [edi+list_next], esi
           test esi, esi
           jz @f
           mov [esi+list_prev], edi
@@:
           bts [mem_block_mask], eax
.m_eq:
           xor eax, eax
           mov [heap_mutex], eax
           not eax
           ret
.insert:
           remove_from_used esi

           mov eax, [esi+block_size]
           calc_index eax

           mov edi, [mem_block_list+eax*4]
           mov [mem_block_list+eax*4], esi
           mov [esi+list_next], edi
           test edi, edi
           jz @f
           mov [edi+list_prev], esi
@@:
           bts [mem_block_mask], eax
           mov [esi+block_flags],FREE_BLOCK
           xor eax, eax
           mov [heap_mutex], eax
           not eax
           ret
.fail:
           xor eax, eax
           mov [heap_mutex], eax
           ret
endp

align 4
proc kernel_alloc stdcall, size:dword
           locals
             lin_addr    dd ?
             pages_count dd ?
           endl

           mov eax, [size]
           add eax, 4095
           and eax, not 4095;
           mov [size], eax
           and eax, eax
           jz .error
           mov ebx, eax
           shr ebx, 12
           mov [pages_count], ebx

           stdcall alloc_kernel_space, eax
           and eax, eax
           jz .error
           mov [lin_addr], eax

           mov ecx, [pages_count]
           mov edx, eax
           mov ebx, ecx

           shr ecx, 3
           jz .next

           and ebx, not 7
           push ebx
           stdcall alloc_pages, ebx
           pop ecx                   ; yes ecx!!!
           and eax, eax
           jz .error

           mov edi, eax
           mov edx, [lin_addr]
@@:
           stdcall map_page,edx,edi,dword PG_SW
           add edx, 0x1000
           add edi, 0x1000
           dec ecx
           jnz @B
.next:
           mov ecx, [pages_count]
           and ecx, 7
           jz .end

@@:        push ecx
           call alloc_page
           pop ecx
           test eax, eax
           jz .error

           stdcall map_page,edx,eax,dword PG_SW
           add edx, 0x1000
           dec ecx
           jnz @B
.end:
           mov eax, [lin_addr]
           ret
.error:
           xor eax, eax
           ret
endp

align 4
proc kernel_free stdcall, base:dword

           mov ebx, heap_mutex
           call wait_mutex    ;ebx

           mov eax, [base]
           mov esi, [mem_used_list]
@@:
           test esi, esi
           jz .fail

           cmp [esi+block_base], eax
           je .found
           mov esi, [esi+list_next]
           jmp @b
.found:
           cmp [esi+block_flags], USED_BLOCK
           jne .fail

           and [heap_mutex], 0

           mov ecx, [esi+block_size];
           shr ecx, 12
           call release_pages   ;eax, ecx
           stdcall free_kernel_space, [base]
           ret
.fail:
           and [heap_mutex], 0
           ret
endp

restore block_next
restore block_prev
restore block_list
restore block_base
restore block_size
restore block_flags

;;;;;;;;;;;;;;      USER     ;;;;;;;;;;;;;;;;;

HEAP_TOP  equ 0x5FC00000

align 4
proc init_heap

           mov ebx,[CURRENT_TASK]
           shl ebx,8
           mov eax, [PROC_BASE+APPDATA.heap_top+ebx]
           test eax, eax
           jz @F
           sub eax,[PROC_BASE+APPDATA.heap_base+ebx]
           sub eax, 4096
           ret
@@:
           mov esi, [PROC_BASE+APPDATA.mem_size+ebx]
           add esi, 4095
           and esi, not 4095
           mov eax, HEAP_TOP
           mov [PROC_BASE+APPDATA.heap_base+ebx], esi
           mov [PROC_BASE+APPDATA.heap_top+ebx], eax

           sub eax, esi
           add esi, new_app_base
           shr esi, 10
           mov ecx, eax
           sub eax, 4096
           or ecx, FREE_BLOCK
           mov [pages_tab+esi], ecx
           ret
.exit:
           xor eax, eax
           ret
endp

align 4
proc user_alloc stdcall, alloc_size:dword

           mov ecx, [alloc_size]
           add ecx, (4095+4096)
           and ecx, not 4095

           mov ebx, [CURRENT_TASK]
           shl ebx, 8
           mov esi, dword [ebx+PROC_BASE+APPDATA.heap_base]; heap_base
           mov edi, dword [ebx+PROC_BASE+APPDATA.heap_top]; heap_top
           add esi, new_app_base
           add edi, new_app_base
l_0:
           cmp esi, edi
           jae m_exit

           mov ebx, esi
           shr ebx, 12
           mov eax, [pages_tab+ebx*4]
           test eax, FREE_BLOCK
           jz test_used
           and eax, 0xFFFFF000
           cmp eax, ecx    ;alloc_size
           jb  m_next
	   jz  @f

           mov edx, esi
           add edx, ecx
           sub eax, ecx;
           or eax, FREE_BLOCK
           shr edx, 12
           mov [pages_tab+edx*4], eax

@@:
           or ecx, USED_BLOCK
           mov [pages_tab+ebx*4], ecx
           shr ecx, 12
           dec ecx
           inc ebx
@@:
           mov dword [pages_tab+ebx*4], 2
           inc ebx
           dec ecx
           jnz @B

           mov eax, esi
           add eax, 4096
           sub eax, new_app_base
           ret
m_next:
           add esi, eax
           jmp l_0
test_used:
           test eax, USED_BLOCK
           jz m_exit

           and eax, 0xFFFFF000
           add esi, eax
           jmp l_0
m_exit:
           xor eax, eax
           ret
endp

align 4
proc user_free stdcall, base:dword

           mov esi, [base]
           test esi, esi
           jz .exit

           sub esi, 4096
           shr esi, 12
           mov eax, [pages_tab+esi*4]
           test eax, USED_BLOCK
           jz .not_used

           and eax, not 4095
           mov ecx, eax
           or eax, FREE_BLOCK
           mov [pages_tab+esi*4], eax
           inc esi
           sub ecx, 4096
           shr ecx, 12
.release:
           xor eax, eax
           xchg eax, [pages_tab+esi*4]
           test eax, 1
           jz @F
           call free_page
@@:
           inc esi
           dec ecx
           jnz .release
.not_used:
           mov ebx, [CURRENT_TASK]
           shl ebx, 8
           mov esi, dword [ebx+PROC_BASE+APPDATA.heap_base]; heap_base
           mov edi, dword [ebx+PROC_BASE+APPDATA.heap_top]; heap_top
           add esi, new_app_base
           add edi, new_app_base
           shr esi, 12
           shr edi, 12
@@:
           mov eax, [pages_tab+esi*4]
           test eax, USED_BLOCK
           jz .test_free
           shr eax, 12
           add esi, eax
           jmp @B
.test_free:
           test eax, FREE_BLOCK
           jz .err
           mov edx, eax
           shr edx, 12
           add edx, esi
           cmp edx, edi
           jae .exit

           mov ebx, [pages_tab+edx*4]
           test ebx, USED_BLOCK
           jz .next_free

           shr ebx, 12
           add edx, ebx
           mov esi, edx
           jmp @B
.next_free:
           test ebx, FREE_BLOCK
           jz .err
           and dword [pages_tab+edx*4], 0
           add eax, ebx
           and eax, not 4095
           or eax, FREE_BLOCK
           mov [pages_tab+esi*4], eax
           jmp @B
.exit:
           xor eax, eax
           inc eax
           ret
.err:
           xor eax, eax
           ret
endp

if 0
align 4
proc alloc_dll
           pushf
           cli
           bsf eax, [dll_map]
           jnz .find
           popf
           xor eax, eax
           ret
.find:
           btr [dll_map], eax
           popf
           shl eax, 5
           add eax, dll_tab
           ret
endp

align 4
proc alloc_service
           pushf
           cli
           bsf eax, [srv_map]
           jnz .find
           popf
           xor eax, eax
           ret
.find:
           btr [srv_map], eax
           popf
           shl eax,0x02
           lea eax,[srv_tab+eax+eax*8]   ;srv_tab+eax*36
           ret
endp

end if

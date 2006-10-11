if ~ used memalloc_inc
memalloc_inc_fix:
memalloc_inc fix memalloc_inc_fix
;kglobals.inc required.
;multithread:  ;uncomment this for thread-safe version
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Memory allocator for MenuetOS                         ;;
;; Halyavin Andrey halyavin@land.ru, 2006                ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; allocated mem block structure                         ;;
;; +0:   bit 0 - used flag                               ;;
;;       bits 31..1 - block size                         ;;
;; +4: address of prev block                             ;;
;; +8 .. +(blocksize) - allocated memory                 ;;
;; +(blocksize) - next block                             ;;
;;                                                       ;;
;; free mem block structure                              ;;
;; +0:   bit 0 - used flag                               ;;
;;       bits 31..1 - block size                         ;;
;; +4:  address of prev block                            ;;
;; +8:  prev free block                                  ;;
;; +12: next free block                                  ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
memblock.size=0
memblock.prevblock=4
memblock.prevfreeblock=8
memblock.nextfreeblock=12
uglobal
    heapsmallblocks rd 1
    heapstart	    rd 1
    heapend	    rd 1
    heapfreelist    rd 1
    heapmutex	    rd 1
    heaplastblock   rd 1
endg

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; mf_init                                               ;;
;; Initialize memory map for dynamic use                 ;;
;; input: eax: starting address or 0                     ;;
;; output: none                                          ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
mf_init:
  push ebx
  push ecx
  test eax,eax
  jnz  .noautodet
  sub  esp,1024
  mov  ebx,esp
  mov  ecx,-1
  mov  eax,9
  int  0x40
  mov  eax,[esp+26]
  add  esp,1024
.noautodet:
  add  eax,15
  and  eax,not 15
  mov  [heapsmallblocks],eax
  add  eax,2048
  mov  [heapstart],eax
  mov  [heapfreelist],eax
  mov  [heaplastblock],eax

  mov  ecx,eax
if defined heapstartsize
  add  ecx,heapstartsize
else
  add  ecx,4096
end if
  add  ecx,4095
  and  ecx,not 4095
  push eax
  mov  eax,64
  mov  ebx,1
  int  0x40
  pop  eax
  mov  [eax+memblock.prevblock],dword 0  
  mov  [heapend],ecx
  mov  [eax+memblock.size],ecx
  sub  [eax+memblock.size],eax
  xor  ebx,ebx
  mov  dword [eax+memblock.prevfreeblock],heapfreelist-memblock.nextfreeblock
  mov  [eax+memblock.nextfreeblock],ebx
  mov  [heapmutex],ebx
  push edi
  mov  edi,[heapsmallblocks]
  mov  ecx,512
  xor  eax,eax
  rep  stosd
  pop  edi
  pop  ecx
  pop  ebx
  ret

if defined multithread
heaplock:
  push eax
  push ebx
  mov  eax,68
  mov  ebx,1
.loop:
  xchg eax,[heapmutex]
  test eax,eax
  jz   .endloop
  int  0x40	;change task
  jmp  .loop
.endloop:
  pop  ebx
  pop  eax
  ret

heapunlock:
  mov  [heapmutex],dword 0
  ret
end if

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; heap_split_block                                      ;;
;; Split free block to allocated block and free one.     ;;
;; input:                                                ;;
;;   eax - size of allocated block                       ;;
;;   ebx - block                                         ;;
;; output:                                               ;;
;;   eax - real size of allocated block                  ;;
;;   ebx - pointer to new block                          ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
heap_split_block:
  push ecx
  mov  ecx,[ebx+memblock.size]
  sub  ecx,16
  cmp  ecx,eax
  jge  .norm
  inc  dword [ebx+memblock.size]
  mov  eax,ecx
  xor  ebx,ebx
  pop  ecx
  ret
.norm:
  add  ecx,16
  mov  [ebx+memblock.size],eax
  inc  dword [ebx+memblock.size]
  mov  [ebx+eax+memblock.prevblock],ebx
  add  ebx,eax
  sub  ecx,eax
  mov  [ebx+memblock.size],ecx
  mov  ecx,eax
  mov  eax,ebx
  call heap_fix_right
  mov  eax,ecx
  pop  ecx
  ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; heap_add_free_block                                   ;;
;; Add free block to one of free block lists.            ;;
;; input:                                                ;;
;;   eax - address of free block                         ;;
;; output:                                               ;;
;;   none                                                ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
heap_add_free_block:
  cmp  dword [eax+memblock.size],4096
  push ebx
  jge  .bigblock
  mov  ebx,[eax+memblock.size]
  shr  ebx,1
  add  ebx,[heapsmallblocks]
  push dword [ebx]
  pop  dword [eax+memblock.nextfreeblock]
  mov  [ebx],eax
  mov  dword [eax+memblock.prevfreeblock],ebx
  sub  dword [eax+memblock.prevfreeblock],memblock.nextfreeblock
  mov  ebx,[eax+memblock.nextfreeblock]
  test ebx,ebx
  jz   .no_next_block
  mov  [ebx+memblock.prevfreeblock],eax
.no_next_block:
  pop  ebx
  ret
.bigblock:
  mov  ebx,[heapfreelist]
  mov  [eax+memblock.nextfreeblock],ebx
  mov  [heapfreelist],eax
  mov  dword [eax+memblock.prevfreeblock],heapfreelist-memblock.nextfreeblock
;  mov  ebx,[eax+memblock.nextfreeblock]
  test ebx,ebx
  jz   .no_next_big_block
  mov  [ebx+memblock.prevfreeblock],eax
.no_next_big_block:
  pop  ebx
  ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; heap_remove_block                                     ;;
;; Remove free block from the list of free blocks.       ;;
;; input:                                                ;;
;;   eax - free block                                    ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
heap_remove_block:
  push	ebx
  push	ecx
  mov	ecx,[eax+memblock.prevfreeblock]
  mov	ebx,[eax+memblock.nextfreeblock]
  mov	[ecx+memblock.nextfreeblock],ebx
  test	ebx,ebx
  jz	.no_next_block
  mov	[ebx+memblock.prevfreeblock],ecx
.no_next_block:
  pop	ecx
  pop	ebx
  ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; mf_alloc
;; allocates a block of memory in heap
;; intput: eax: size of block
;; output: eax: address of allocated memory block or 0 if there's no mem.
;; allocator will not create new nodes that contain less that 8b of space,
;; and minimal allocation is actually 16 bytes - 8 for node and 8 for user.
;; allocator will never create non-aligned memory blocks.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
mf_alloc:
  test eax,eax
  jg   .not_null  ; test that we are not allocating null size block
  xor  eax,eax
  ret
.not_null:
if defined multithread
  call heaplock
end if
  push edi
;  push edx
  push ecx
  push ebx
  add  eax,7
  and  eax,not 7   ; make sure that block size is aligned

  lea  edi,[eax+8] ; desired block size
  cmp  edi,4096
  jge  .general_cycle

  mov  ebx,[heapsmallblocks]
  xor  ecx,ecx
  shr  edi,1

.smallloop:
  cmp  [ebx+edi],ecx
  jnz  .smallblockfound
  add  edi,4
  cmp  edi,2048
  jl   .smallloop
  lea  edi,[eax+8]
  jmp  .general_cycle

.smallblockfound:
  lea  ecx,[eax+8]
  mov  eax,[ebx+edi]
  call heap_remove_block
  mov  ebx,eax
  xchg eax,ecx
  call heap_split_block
  test ebx,ebx
  jz   .perfect_small_block
  mov  eax,ebx
  call heap_add_free_block
.perfect_small_block:
  lea  eax,[ecx+8]
  jmp  .ret

.general_cycle:
;edi - size needed
  mov  eax,[heapfreelist]

.loop:
  test eax,eax
  jz   .new_mem
  cmp  [eax+memblock.size],edi
  jge  .blockfound
  mov  eax,[eax+memblock.nextfreeblock]
  jmp  .loop

.blockfound:
  call heap_remove_block
  mov  ebx,eax
  mov  ecx,eax
  mov  eax,edi
  call heap_split_block
  test ebx,ebx
  jz   .perfect_block
  mov  eax,ebx
  call heap_add_free_block
.perfect_block:
  lea  eax,[ecx+8]
.ret:
if defined multithread
  call heapunlock
end if
  pop  ebx
  pop  ecx
;  pop  edx
  pop  edi
  ret

.new_mem:
  mov  eax,edi
  add  eax,4095
  and  eax,not 4095
  mov  ecx,[heapend]
  add  [heapend],eax
  push eax
  mov  eax,64
  push ebx
  push ecx
  mov  ecx,[heapend]
  mov  ebx,1
  int  0x40
  pop  ecx
  pop  ebx
  pop  eax
  mov  [ecx+memblock.size],eax
  mov  eax,[heaplastblock]
  mov  [ecx+memblock.prevblock],eax
  mov  [heaplastblock],ecx
  mov  eax,ecx
  call heap_add_free_block
  jmp  .general_cycle

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; heap_fix_right                                        ;;
;; input:                                                ;;
;;   eax - pointer to free block                         ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
heap_fix_right:
  push ebx
  mov  ebx,eax
  add  ebx,[eax+memblock.size]
  cmp  ebx,[heapend]
  jz   .endblock
  mov  [ebx+memblock.prevblock],eax
  pop  ebx
  ret
.endblock:
  mov  [heaplastblock],eax
  pop  ebx
  ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; heap_merge_left                                       ;;
;; input:                                                ;;
;;   eax - pointer to free block                         ;;
;; output:                                               ;;
;;   eax - pointer to merged block                       ;; 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
heap_merge_left:
  push ebx
  mov  ebx,[eax+memblock.prevblock]
  test ebx,ebx
  jz   .ret
  test byte [ebx+memblock.size],1
  jnz  .ret
  xchg eax,ebx
  call heap_remove_block
  mov  ebx,[ebx+memblock.size]
  add  [eax+memblock.size],ebx
  call heap_fix_right
.ret:
  pop  ebx
  ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; heap_merge_right                                      ;;
;; input:                                                ;;
;;   eax - pointer to free block                         ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
heap_merge_right:
  push ebx
  mov  ebx,eax
  add  ebx,[eax+memblock.size]
  cmp  ebx,[heapend]
  jz   .ret
  test byte [ebx+memblock.size],1
  jnz  .ret
  xchg eax,ebx
  call heap_remove_block
  xchg eax,ebx
  mov  ebx,[ebx+memblock.size]
  add  [eax+memblock.size],ebx
  call heap_fix_right
.ret:
  pop  ebx
  ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; mf_free                                               ;;
;; input:                                                ;;
;;   eax - pointer                                       ;; 
;; output:                                               ;;
;;   eax=1 - ok                                          ;;
;;   eax=0 - failed                                      ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
mf_free:
  test eax,eax
  jnz  .no_null
  inc  eax
  ret
.no_null:
if defined multithread
  call heaplock
end if
  sub  eax,8
  dec  dword [eax+memblock.size]
  call heap_merge_left
  call heap_merge_right
  call heap_add_free_block
.ret:
if defined multithread
  call heapunlock
end if
  ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; heap_try_reloc
;; input:
;;   eax - address
;;   ebx - new size
;; output:
;;   ebx=1 - ok
;;   ebx=0 - failed
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
heap_try_reloc:
  push eax
  sub  eax,8
  add  ebx,15
  dec  dword [eax+memblock.size]
  and  ebx,not 7
  cmp  [eax+memblock.size],ebx
  jge  .truncate
  push ebx
  mov  ebx,eax
  add  ebx,[eax+memblock.size]
  cmp  ebx,[heapend]
  jz   .fail  ;todo: we can allocate new mem here
  test [ebx+memblock.size],byte 1
  jnz  .fail
  xchg eax,ebx
  call heap_remove_block
  mov  eax,[eax+memblock.size]
  add  [ebx+memblock.size],eax
  mov  eax,ebx
  call heap_fix_right
  pop  ebx
.truncate:
  xchg eax,ebx
  call heap_split_block
  test ebx,ebx
  jz   .no_last_block
  mov  eax,ebx
  call heap_add_free_block
  call heap_merge_right
.no_last_block:
  xor  ebx,ebx
  pop  eax
  inc  ebx
  ret
.fail:
  pop  ebx
  xor  ebx,ebx
  pop  eax
  inc  dword [eax-8+memblock.size]
  ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; mf_realloc
;; input:
;;   eax - pointer
;;   ebx - new size
;; output:
;;   eax - new pointer
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
mf_realloc:
  push ebx
if defined multithread
  call heaplock
end if
  call heap_try_reloc
  test ebx,ebx
  jnz  .ret
;allocate new memory
  push eax
  mov  eax,[esp+4]
  call mf_alloc
  test eax,eax
  jz   .fail
  push esi
  push edi
  push ecx
  mov  edi,eax
  mov  esi,[esp+12]
  mov  ecx,[esi-8+memblock.size]
  shr  ecx,2
  rep  movsd
  pop  ecx
  pop  edi
  pop  esi
  xchg eax,[esp]
  call mf_free
.fail:
  pop  eax
.ret:
if defined multithread
  call heapunlock
end if
  pop  ebx
  ret
end if

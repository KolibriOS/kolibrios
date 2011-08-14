;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2008. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


MEM_WB	   equ 6	       ;write-back memory
MEM_WC	   equ 1	       ;write combined memory
MEM_UC	   equ 0	       ;uncached memory

; ======================================================================
align 4
preinit_mem:

; clear [CLEAN_ZONE..HEAP_BASE]
	   xor	 eax,eax
	   mov	 edi,CLEAN_ZONE 		; 0x280000 = ramdisk FAT ?
	   mov	 ecx,(HEAP_BASE-OS_BASE-CLEAN_ZONE) / 4
	   cld
	   rep	 stosd

; clear [0x40000..0x90000]
	   mov	 edi,0x50000			; 0x50000 is somewhere inside kernel code?
	   mov	 ecx,(0x90000-0x50000)/4
	   rep	 stosd

; clear undefined kernel globals
	   mov	 edi, endofcode-OS_BASE
	   mov	 ecx, (uglobals_size/4)+4
	   rep	 stosd

; save [0..0xffff]
	   xor	 esi, esi
	   mov	 edi, (BOOT_VAR - OS_BASE)		; low mem storage area
	   mov	 ecx, 0x10000 / 4
	   rep	 movsd
; clear [0x1000..0x0ffff]
	   mov	 edi,0x1000
	   mov	 ecx,0xf000 / 4
	   rep	 stosd

; clear <sys_pgdir> table
	   mov edi, sys_pgdir-OS_BASE
	   mov ecx, 4096/4
	   rep stosd
	ret

; ======================================================================
align 4
proc init_mem

	   mov	ecx, 0xC001001A 			; Top of Memory MSR
	   xor	edi, edi
	   rdmsr
	   mov	esi, eax				; esi = total amount of memory
	   mov	ecx, 0x0200
.read_mtrr:
	   rdmsr
	   and	eax, 0xFFF00000 			; not just bitcleaning
	   jz	.next_mtrr				; ignore the main memory and free MTRRs
	   cmp	esi, eax
	   jb	.next_mtrr				; ignore MMIO blocks
	   mov	esi, eax
.next_mtrr:
	   add	cl, 2
	   cmp	cl, 0x10
	   jb	.read_mtrr

	   mov	eax, USER_DMA_SIZE
	   sub	esi, eax				; exclude the Global DMA block...
	   and	esi, 0xFF800000 			; ...and the hole above it
	   mov	eax, esi
	   mov	[MEM_AMOUNT-OS_BASE], eax
	   mov	[pg_data.mem_amount-OS_BASE], eax	; the true MEMTOP
	   mov	[UserDMAaddr-OS_BASE], eax

	   mov edx, esi 				; edx will hold maximum allocatable address
	   shr esi, 12
	   mov [pg_data.pages_count-OS_BASE], esi	; max number of PTEs   ?

	   shr edx, 12
	   add edx, 31
	   and edx, not 31
	   shr edx, 3
	   mov [pg_data.pagemap_size-OS_BASE], edx	; size of sys_pgmap structure

	   add edx, (sys_pgmap-OS_BASE)+4095
	   and edx, not 4095
	   mov [tmp_page_tabs], edx			; free zone to build PTEs

	   mov edx, (HEAP_BASE-OS_BASE+HEAP_MIN_SIZE)/4096
	   mov [pg_data.kernel_pages -OS_BASE], edx
	   shr edx, 10
	   mov [pg_data.kernel_tables-OS_BASE], edx

	   mov edx, (sys_pgdir-OS_BASE)+ 0x800		; (0x800 = OS_BASE shr 20)

	   mov ebx, cr4
	   or  ebx, CR4_PSE
	   mov eax, PG_LARGE+PG_SW
	   mov cr4, ebx
	   dec [pg_data.kernel_tables-OS_BASE]

	   mov [edx], eax				; map first (physical) 4M bytes
	   add edx, 4

	   mov edi, [tmp_page_tabs]
	   mov ecx, [pg_data.kernel_pages -OS_BASE]	; safety cleaning of already-zeroed space
	   xor eax, eax
	   rep stosd

	   mov ecx, [pg_data.kernel_tables-OS_BASE]	; build some PDEs to hold empty PTEs
	   mov eax, [tmp_page_tabs]
	   or  eax, PG_SW
	   mov edi, edx 		; edi = sys_pgdir+0x804

.map_kernel_tabs:
	   stosd
	   add eax, 0x1000
	   dec ecx
	   jnz .map_kernel_tabs

; map pagetables to linear space
	   mov dword [sys_pgdir-OS_BASE+(page_tabs shr 20)], sys_pgdir+PG_SW-OS_BASE

	   mov edi, (sys_pgdir-OS_BASE)
	   lea esi, [edi+(OS_BASE shr 20)]
	   movsd
	   movsd
	   ret
endp

align 4
proc init_page_map
; mark all memory as unavailable
	   mov edi, sys_pgmap-OS_BASE
	   mov ecx, [pg_data.pagemap_size-OS_BASE]
	   shr ecx, 2
	   xor eax, eax
	   cld
	   rep stosd

; scan through memory map and mark free areas as available
	   mov ebx, BOOT_VAR-OS_BASE + 0x9104
	   mov edx, [ebx-4]
.scanmap:
	   mov ecx, [ebx+8]
	   shr ecx, 12 ; ecx = number of pages
	   jz .next
	   mov edi, [ebx]
	   shr edi, 12 ; edi = first page
	   mov eax, edi
	   shr edi, 5
	   shl edi, 2
	   add edi, sys_pgmap-OS_BASE
	   and eax, 31
	   jz .startok
	   add ecx, eax
	   sub ecx, 32
	   jbe .onedword
	   push ecx
	   mov ecx, eax
	   or eax, -1
	   shl eax, cl
	   or [edi], eax
	   add edi, 4
	   pop ecx
.startok:
	   push ecx
	   shr ecx, 5
	   or eax, -1
	   rep stosd
	   pop ecx
	   and ecx, 31
	   neg eax
	   shl eax, cl
	   dec eax
	   or [edi], eax
	   jmp .next
.onedword:
	   add ecx, 32
	   sub ecx, eax
@@:
	   bts [edi], eax
	   inc eax
	   loop @b
.next:
	   add ebx, 20
	   dec edx
	   jnz .scanmap

; mark kernel memory as allocated (unavailable)
	   mov ecx, [tmp_page_tabs]
	   mov edx, [pg_data.pages_count-OS_BASE]
	   shr ecx, 12
	   add ecx, [pg_data.kernel_tables-OS_BASE]
	   sub edx, ecx
	   mov [pg_data.pages_free-OS_BASE], edx

	   mov edi, sys_pgmap-OS_BASE
	   mov ebx, ecx
	   shr ecx, 5
	   xor eax, eax
	   rep stosd

	   not eax
	   mov ecx, ebx
	   and ecx, 31
	   shl eax, cl
	   and [edi], eax
	   add edi, OS_BASE
	   mov [page_start-OS_BASE], edi;

	   mov ebx, sys_pgmap
	   add ebx, [pg_data.pagemap_size-OS_BASE]
	   mov [page_end-OS_BASE], ebx

	   mov [pg_data.pg_mutex-OS_BASE], 0
	   ret
endp

align 4

init_BIOS32:
	   mov edi, 0xE0000
.pcibios_nxt:
	   cmp dword[edi], '_32_' ; "magic" word
	   je .BIOS32_found
.pcibios_nxt2:
	   add edi, 0x10
	   cmp edi, 0xFFFF0
	   je .BIOS32_not_found
	   jmp .pcibios_nxt
.BIOS32_found:			; magic word found, check control summ

	   movzx ecx, byte[edi + 9]
	   shl ecx, 4
	   mov esi, edi
	   xor eax, eax
	   cld	 ; paranoia
@@:	lodsb
	   add ah, al
	   loop @b
	   jnz .pcibios_nxt2 ; control summ must be zero
    ; BIOS32 service found !
	   mov ebp, [edi + 4]
	   mov [bios32_entry], ebp
    ; check PCI BIOS present
	   mov eax, '$PCI'
	   xor ebx, ebx
	   push cs  ; special for 'ret far' from  BIOS
	   call ebp
	   test al, al
	   jnz .PCI_BIOS32_not_found

 ; здесь создаются дискрипторы для PCI BIOS

	   add ebx, OS_BASE
	   dec ecx
	   mov [(pci_code_32-OS_BASE)], cx    ;limit 0-15
	   mov [(pci_data_32-OS_BASE)], cx    ;limit 0-15

	   mov [(pci_code_32-OS_BASE)+2], bx  ;base  0-15
	   mov [(pci_data_32-OS_BASE)+2], bx  ;base  0-15

	   shr ebx, 16
	   mov [(pci_code_32-OS_BASE)+4], bl  ;base  16-23
	   mov [(pci_data_32-OS_BASE)+4], bl  ;base  16-23

	   shr ecx, 16
	   and cl, 0x0F
	   mov ch, bh
	   add cx, D32
	   mov [(pci_code_32-OS_BASE)+6], cx  ;lim   16-19 &
	   mov [(pci_data_32-OS_BASE)+6], cx  ;base  24-31

	   mov [(pci_bios_entry-OS_BASE)], edx
	 ; jmp .end
.PCI_BIOS32_not_found:
	; здесь должна заполнятся pci_emu_dat
.BIOS32_not_found:
.end:
	   ret

align 4
test_cpu:	; only AMD machines supported

	   xor eax, eax
	   mov [cpu_caps-OS_BASE], eax
	   mov [cpu_caps+4-OS_BASE], eax

	   pushfd
	   pop eax
	   mov ecx, eax
	   xor eax, 0x40000
	   push eax
	   popfd
	   pushfd
	   pop eax
	   xor eax, ecx
	   jz $ 		; 386
	   push ecx
	   popfd

	   mov eax, ecx
	   xor eax, 0x200000
	   push eax
	   popfd
	   pushfd
	   pop eax
	   xor eax, ecx
	   je $ 		; 486

	   xor eax, eax
	   cpuid

	   mov [cpu_vendor-OS_BASE],   ebx
	   mov [cpu_vendor+4-OS_BASE], edx
	   mov [cpu_vendor+8-OS_BASE], ecx

	   cmp ebx, dword [AMD_str-OS_BASE]
	   jne $
	   cmp edx, dword [AMD_str+4-OS_BASE]
	   jne $
	   cmp ecx, dword [AMD_str+8-OS_BASE]
	   jne $
	   cmp eax, 1
	   jl $
	   mov eax, 1
	   cpuid
	   mov [cpu_sign-OS_BASE],  eax
	   mov [cpu_info-OS_BASE],  ebx
	   mov [cpu_caps-OS_BASE],  edx
	   mov [cpu_caps+4-OS_BASE],ecx
	   shr eax, 8
	   and eax, 0x0f
	   ret




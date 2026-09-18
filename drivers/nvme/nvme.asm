;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2024. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format PE DLL native 0.05
entry START

API_VERSION = 0  ;debug
SRV_GETVERSION = 0
__DEBUG__ = 1
__DEBUG_LEVEL__ = 1
DRIVER_VERSION = 1
DBG_INFO = 1
NULLPTR = 0
FALSE = 0
TRUE = 1

section ".flat" code readable writable executable
include "../proc32.inc"
include "../struct.inc"
include "../macros.inc"
include "../fdo.inc"
include "../pci.inc"
include "../peimport.inc"
include "nvme.inc"
include "lib.inc"
include "command.inc"

struct DISKMEDIAINFO
	flags 		dd ?
	sectorsize  	dd ?
	capacity 	dq ?
ends

proc START c, reason:dword, cmdline:dword
local   AnythingLoadedSuccessfully db 0

	push 	ebx esi edi
	cmp     [reason], DRV_ENTRY
	jne     .err

.entry:
	DEBUGF  DBG_INFO, "Detecting NVMe device...\n"
	call    detect_nvme
	test 	eax, eax
	jz      .err
	xor 	ebx, ebx
	mov 	esi, dword [p_nvme_devices]
	test 	esi, esi
	jz 	.err
	sub 	esi, sizeof.pcidev

.loop:
	add 	esi, sizeof.pcidev
	push 	ebx esi
	stdcall device_is_compat, esi
	test 	eax, eax
	jz 	.pop
	stdcall nvme_init, esi
	test 	eax, eax
	jz 	.pop
	pop 	esi ebx
	stdcall add_nvme_disk, esi
	jmp 	.next

.pop:
	pop 	esi ebx

.next:
	test 	eax, eax
	setne	[AnythingLoadedSuccessfully]
	inc 	ebx
	cmp 	ebx, dword [num_pcidevs]
	jne 	.loop
	cmp 	[AnythingLoadedSuccessfully], 0
	jz 	.err
	invoke  RegService, my_service, service_proc
	pop 	edi esi ebx
	ret

.err:
	call    nvme_cleanup
	pop 	edi esi ebx
      	ret

endp

proc service_proc stdcall, ioctl:dword

      	mov     esi, [ioctl]
      	mov     eax, [esi + IOCTL.io_code]
      	cmp     eax, SRV_GETVERSION
      	jne     .ret

      	mov     eax, [esi + IOCTL.output]
      	cmp     [esi + IOCTL.out_size], 4
      	jne     .ret
      	mov     dword [eax], API_VERSION
      	xor     eax, eax
      	ret

.ret:
	or      eax, -1
	ret

endp

; Registers the NVMe disk into KolibriOS. This requires that the
; device was successfully initialized by nvme_init, otherwise this
; will have undefined behavior.
proc add_nvme_disk stdcall, pci:dword

	push 	esi
	mov 	esi, [pci]

	; NOTE: If the pcidev.num or pcidev.nsid is more than 9 then
	; this fails to build the string correctly. Ignoring this issue
	; for now since who has more than 9 NVMe SSDs on a desktop computer
	; and a NSID bigger than 9 is also unlikely.
	;
	; Still, will address this problem in the future.
	push 	0 ; null terminator
	movzx 	eax, byte [esi + pcidev.nsid]
	add 	al, "0"
	mov 	byte [esp], al
	dec 	esp
	mov 	byte [esp], "n"
	dec 	esp
	movzx 	eax, byte [esi + pcidev.num]
	add 	al, "0"
	mov 	byte [esp], al
	push 	"nvme"
	mov 	eax, esp
	invoke  DiskAdd, disk_functions, eax, [esi + pcidev.nsinfo], 0
	add 	esp, 10
	test 	eax, eax
	jz 	@f
	invoke  DiskMediaChanged, eax, 1
	DEBUGF  DBG_INFO, "nvme%un%u: Successfully registered disk\n", [esi + pcidev.num], [esi + pcidev.nsid]
	xor 	eax, eax
	inc 	eax
	pop 	esi
	ret

@@:
	DEBUGF  DBG_INFO, "nvme%un%u: Failed to register disk\n", [esi + pcidev.num], [esi + pcidev.nsid]
	xor 	eax, eax
	pop 	esi
	ret

endp

proc nvme_query_media stdcall, userdata:dword, info:dword

	push 	ebx esi edi
	mov 	esi, [userdata]
	mov 	ebx, dword [esi + NSINFO.pci]
	mov 	edi, [info]
	mov 	dword [edi + DISKMEDIAINFO.flags], 0
	mov 	cl, byte [esi + NSINFO.lbads]
	xor 	eax, eax
	inc 	eax
	shl 	eax, cl
	DEBUGF  DBG_INFO, "nvme%un%u (Query Media): Sector size = %u\n", [ebx + pcidev.num], [esi + NSINFO.nsid], eax
	mov 	dword [edi + DISKMEDIAINFO.sectorsize], eax
	mov 	eax, dword [esi + NSINFO.capacity]
	mov 	dword [edi + DISKMEDIAINFO.capacity], eax
	mov 	eax, dword [esi + NSINFO.capacity + 4]
	mov 	dword [edi + DISKMEDIAINFO.capacity + 4], eax
	DEBUGF  DBG_INFO, "nvme%un%u (Query Media): Capacity = %u + %u sectors\n", [ebx + pcidev.num], [esi + NSINFO.nsid], [esi + NSINFO.capacity], [esi + NSINFO.capacity + 4]
	xor 	eax, eax
	pop 	edi esi ebx
	ret

endp

; returns 1 if the given NSID is a an active NSID, returns
; 0 otherwise
proc is_active_namespace stdcall, pci:dword, nsid:dword
	
	push 	esi edi
	invoke  KernelAlloc, 0x1000
	test 	eax, eax
	jnz 	@f
	pop 	edi esi
	ret

@@:
	mov 	esi, eax
	invoke  GetPhysAddr
	stdcall nvme_identify, [pci], [nsid], eax, CNS_IDNS
	test 	eax, eax
	jz 	.not_active_nsid
	xor 	ecx, ecx

@@:
	mov 	eax, dword [esi + ecx * 4]
	test 	eax, eax
	jnz 	.is_active_nsid
	inc 	ecx
	cmp 	ecx, 0x1000 / 4
	jne 	@b
	
.not_active_nsid:
	invoke  KernelFree, esi
	pop 	edi esi
	xor 	eax, eax
	ret

.is_active_nsid:
	invoke  KernelFree, esi
	pop 	edi esi
	xor 	eax, eax
	inc 	eax
	ret

endp

; See page 248 of the NVMe 1.4 specification for reference
; Returns the number of namespaces that are active, note this
; doesn't mean if EAX = 5, then namespaces 1-5 will be active.
; This also sets [pci + pcidev.nn] and [pci + pcidev.nsids] 
; to their appropriate values.
proc determine_active_nsids stdcall, pci:dword

	push 	ebx esi
	mov 	esi, [pci]
	xor 	ebx, ebx
	xor 	ecx, ecx
	inc 	ecx

.loop:
	cmp 	ecx, dword [esi + pcidev.nn]
	ja 	.ret
	push 	ecx
	stdcall is_active_namespace, [pci], ecx
	pop 	ecx
	test 	eax, eax
	jz 	.not_active_namespace
	mov 	ebx, ecx
	jmp 	.ret

.not_active_namespace:
	inc 	ecx
	jmp 	.loop

.ret:
	pop 	edi esi
	mov 	eax, ebx
	ret

endp

; Allocates prp_list_ptr and creates a PRP list there. nprps should
; be set appropriately to the number of PRPs the caller wants to create.
;
; This function should only be called if the conditions for building
; a PRP list are met (see page 68 of the NVMe 1.4.0 spec).
;
; TODO: Currently the code for building recursive PRP lists is untested.
; If you want to test it, do a read/write with a sector count equivalant
; to more than 4MiB. Will test in the future.
proc build_prp_list stdcall, nprps:dword, buf:dword, prp_list_ptr:dword

	push 	esi ebx edi
	sub 	esp, 4

	; stack:
	; 	[esp]: virtual pointer to first PRP list
	; here, we store the pointer to the very first
	; PRP list so that free_prp_list can free the
	; entire PRP list if something goes wrong, it
	; also serves as our return value placeholder
	mov 	dword [esp], 0

	xor 	edi, edi
	xor 	esi, esi
	mov 	ecx, [nprps]
	shl 	ecx, 3 ; multiply by 8 since each PRP pointer is a QWORD

	; we'll store consecutive PRP list buffers here, for example
	; given 2 PRP lists, we allocate 2 continuous pages
	push 	ecx
	invoke  KernelAlloc, ecx ; store pointers to the PRP entries here
	pop 	ecx
	test 	eax, eax
	jz 	.err
	mov 	dword [esp], eax
	mov 	edi, eax
	mov 	eax, [prp_list_ptr]
	mov 	dword [eax], edi
	shr 	ecx, 2 ; ECX holds the size in bytes, memsetdz wants it in DWORDs
	stdcall memsetdz, edi, ecx
	
	; note we assume buf is page-aligned
	mov 	esi, [buf]

.build_prp_list:
	; ensure we don't cross a page boundary
	mov 	ebx, [nprps]
	cmp 	ebx, PAGE_SIZE / 8
	jb 	@f
	mov 	ebx, PAGE_SIZE / 8
	sub 	[nprps], ebx

@@:
	xor 	ecx, ecx
	cmp 	dword [esp], edi
	je 	.loop

	; we need to store the pointer of the next
	; PRP list to the previous PRP list last entry
	mov 	eax, edi
	invoke  GetPhysAddr
	mov 	dword [edi - 8], eax
	mov 	dword [edi - 4], 0

.loop:
	mov 	eax, esi
	invoke  GetPhysAddr
	mov 	dword [edi + ecx * 8], eax
	mov 	dword [edi + ecx * 8 + 4], 0
	add 	esi, PAGE_SIZE
	inc 	ecx
	cmp 	ecx, ebx
	jne 	.loop

	; check if we we need to build another PRP list
	add 	edi, PAGE_SIZE
	cmp 	ebx, PAGE_SIZE / 8
	je 	.build_prp_list

	; PRP list successfully created
	mov 	eax, dword [esp]
	invoke  GetPhysAddr
	add 	esp, 4
	pop 	edi ebx esi
	ret

.err:
	add 	esp, 4
	pop 	edi ebx esi
	xor 	eax, eax
	ret
	
endp

; Allocates PRP1/PRP2. Note that it is not required to call this function
; unless you're doing read and writes with an arbitrary buffer that the
; kernel passes to driver. In most other cases, it's better to just allocate a 
; page-aligned buffer.
;
; ns: Pointer to the device's respective namespace struct
;
; prps_ptr: should be a pointer to at least 2 DWORDS (PRP1 and PRP2 respectively), 
; the caller is allowed to not initialize PRP1, however PRP2 should explicitly be 
; initialized to 0.
;
; prp_list_ptr: pointer to 1 DWORD, the caller must initialize this value to 0.
; If a PRP list is allocated, then prp_list_ptr shall contain the pointer to
; the PRP list. The caller is required to free the allocated memory afterwards.
; 
; buf: Pointer to the buffer
;
; On success, the function will return 1 and the PRPs will be initialized. If an
; error occurs (most likely due to memory allocation), the function returns 0.
proc alloc_dptr stdcall, ns:dword, prps_ptr:dword, numsectors:dword, prp_list_ptr:dword, buf:dword

	push 	ebx esi edi
	mov 	esi, [ns]
	mov 	edi, [prps_ptr]
	mov 	eax, [buf]
	invoke  GetPhysAddr
	mov 	dword [edi], eax ; PRP1 runs from the buffer to the end of its page

	; What decides the shape of PRP2 is how many memory pages the transfer
	; touches, not how many sectors it carries: the controller reads PRP2 as a
	; plain data pointer whenever a single page holds everything PRP1 could not,
	; and only as a PRP List pointer beyond that. Counting sectors instead got
	; this wrong for an unaligned buffer carrying more than a page worth of
	; sectors while still landing inside two pages - the controller then took the
	; list itself for the data, which is the file corruption of GSoC issue #7.
	mov 	eax, [numsectors]
	mov 	cl, byte [esi + NSINFO.lbads]
	shl 	eax, cl ; bytes to transfer
	mov 	ebx, [buf]
	and 	ebx, PAGE_SIZE - 1 ; offset into the first page
	add 	eax, ebx
	add 	eax, PAGE_SIZE - 1
	shr 	eax, LOG2 PAGE_SIZE ; pages the transfer touches

	; whatever PRP1 does not cover starts at the next page boundary
	mov 	ebx, [buf]
	and 	ebx, not (PAGE_SIZE - 1)
	add 	ebx, PAGE_SIZE

	cmp 	eax, 1
	jbe 	.success ; it all fits in the first page, PRP2 is unused
	cmp 	eax, 2
	ja 	.build_prp_list

	; exactly two pages: PRP2 is the second page itself
	mov 	eax, ebx
	invoke  GetPhysAddr
	mov 	dword [edi + 4], eax
	jmp 	.success

.build_prp_list:
	dec 	eax ; PRP1 already covers the first page
	stdcall build_prp_list, eax, ebx, [prp_list_ptr]
	test 	eax, eax
	jz 	.err
	mov 	dword [edi + 4], eax

.success:
	xor 	eax, eax
	inc 	eax
	pop 	edi esi ebx
	ret
	
.err:
	xor 	eax, eax
	pop 	edi esi ebx
	ret

endp

nvme_read:
	mov 	edx, NVM_CMD_READ
	jmp 	nvme_readwrite

nvme_write:
	mov 	edx, NVM_CMD_WRITE

; Reads from/writes to the disk. The request is split into chunks of at most
; pcidev.max_sectors, so that no single command exceeds the controller's
; Maximum Data Transfer Size and a plain, non-chained PRP list always suffices.
proc nvme_readwrite stdcall, ns:dword, buf:dword, start_sector:qword, numsectors_ptr:dword
locals
	prp1 		dd ? ; PRP1 of the command in flight
	prp2 		dd ? ; PRP2 of the command in flight
	prp_list 	dd ? ; virtual pointer to its PRP list, 0 if none was needed
	opcode 		dd ?
	chunk 		dd ? ; sectors carried by the command in flight
	left 		dd ? ; sectors still to transfer
	cur_buf 	dd ?
	cur_lba_lo 	dd ?
	cur_lba_hi 	dd ?
endl
; alloc_dptr fills PRP1 and PRP2 through one pointer, so they must stay adjacent
assert prp2 - prp1 = 4

	push 	ebx esi edi
	mov 	[opcode], edx
	mov 	esi, [ns]
	mov 	ebx, dword [esi + NSINFO.pci]

	mov 	eax, [numsectors_ptr]
	mov 	eax, dword [eax]
	mov 	[left], eax
	mov 	eax, [buf]
	mov 	[cur_buf], eax
	mov 	eax, dword [start_sector]
	mov 	[cur_lba_lo], eax
	mov 	eax, dword [start_sector + 4]
	mov 	[cur_lba_hi], eax

.next_chunk:
	mov 	eax, [left]
	test 	eax, eax
	jz 	.done
	mov 	ecx, dword [ebx + pcidev.max_sectors]
	cmp 	eax, ecx
	jbe 	@f
	mov 	eax, ecx

@@:
	mov 	[chunk], eax
	mov 	[prp2], 0 ; PRP2 entry (0 by default)
	mov 	[prp_list], 0 ; no PRP list allocated by default
	lea 	ecx, [prp_list]
	lea 	edx, [prp1]
	stdcall alloc_dptr, esi, edx, eax, ecx, [cur_buf]
	test 	eax, eax
	jz 	.fail

	; According to the NVMe specification, the NLB field in the I/O read and write
	; commands is a 0-based value (i.e., 0 is equivalant to 1, 1 is equivalant to 2, ...)
	; As far as I know, KolibriOS doesn't follow this mechanism so let's just decrement the
	; value and it should have the same effect.
	mov 	ecx, [chunk]
	dec 	ecx

	; TODO: add non-blocking mechanisms later on
	mov 	dword [ebx + pcidev.spinlock], 1
	stdcall nvme_io_rw, ebx, 	                    1, 	                    [esi + NSINFO.nsid], 	                    [prp1], 	                    [prp2], 	                    [cur_lba_lo], 	                    [cur_lba_hi], 	                    ecx, 	                    [opcode]

	; TODO: add non-blocking mechanisms later on
	stdcall nvme_poll, ebx
	test 	eax, eax
	jz 	.fail

	; free PRP list (if allocated)
	mov 	eax, [prp_list]
	test 	eax, eax
	jz 	@f
	invoke  KernelFree, eax
	mov 	[prp_list], 0

@@:
	; advance to the next chunk
	mov 	eax, [chunk]
	sub 	[left], eax
	add 	[cur_lba_lo], eax
	adc 	[cur_lba_hi], 0
	mov 	cl, byte [esi + NSINFO.lbads]
	shl 	eax, cl ; sectors -> bytes
	add 	[cur_buf], eax
	jmp 	.next_chunk

.done:
	xor 	eax, eax
	pop 	edi esi ebx
	ret

.fail:
	; free PRP list (if allocated)
	mov 	eax, [prp_list]
	test 	eax, eax
	jz 	@f
	invoke  KernelFree, eax

@@:
	; report how much of the request did make it to the disk
	mov 	eax, [numsectors_ptr]
	mov 	ecx, dword [eax]
	sub 	ecx, [left]
	mov 	dword [eax], ecx
	pop 	edi esi ebx
	or 	eax, -1 ; generic disk error
	ret

endp

; Detects NVMe devices on the PCI bus and stores them into 
; [p_nvme_devices] and sets [num_pcidevs] to the appropriate
; size based off how many NVMe devices there are.
proc detect_nvme

      	invoke  GetPCIList
	mov 	esi, eax
	mov 	ebx, eax

.check_dev:
	mov 	eax, dword [esi + PCIDEV.class]
	and	eax, 0x00ffff00 ; retrieve class/subclass code only
	cmp 	eax, 0x00010800 ; Mass Storage Controller - Non-Volatile Memory Controller
	je	.found_dev

.next_dev:
	mov	esi, dword [esi + PCIDEV.fd]
	cmp	esi, ebx
	jne 	.check_dev

.exit_success:
	xor 	eax, eax
	inc 	eax
	ret

.found_dev:
	; skip PCIDEV.owner check if the PCI device pointer has already been
	; allocated (without this check, more than 1 NVMe device cannot be
	; registered)
	mov 	eax, dword [p_nvme_devices]
	test 	eax, eax
	jnz 	@f
	cmp 	dword [esi + PCIDEV.owner], 0
	jnz 	.err

@@:
	cmp 	dword [num_pcidevs], TOTAL_PCIDEVS
	jne 	@f
	DEBUGF  DBG_INFO, "Can't add any more NVMe devices...\n"
	jmp 	.exit_success

@@:
	inc 	dword [num_pcidevs]
	add 	dword [num_pcidevs_sz], sizeof.pcidev
	cmp     dword [p_nvme_devices], 0
	jnz	@f ; was the pointer already allocated?
	invoke  KernelAlloc, sizeof.pcidev * TOTAL_PCIDEVS
	test 	eax, eax
	jz 	.err
	mov     dword [p_nvme_devices], eax 
	; clear it: a slot for a controller that has not been initialised yet is
	; still walked by the interrupt handler
	stdcall memsetdz, eax, sizeof.pcidev * TOTAL_PCIDEVS / 4
	mov 	eax, dword [p_nvme_devices]
	mov 	dword [esi + PCIDEV.owner], eax
	DEBUGF  DBG_INFO, "nvme: Allocated memory for PCI devices at: 0x%x\n", eax

@@:
	mov 	ecx, dword [num_pcidevs]
	dec 	ecx
	mov 	edi, dword [p_nvme_devices]
	mov 	edx, ecx
	imul 	edx, sizeof.pcidev
	lea 	edi, [edi + edx]

	movzx 	eax, byte [esi + PCIDEV.bus]
	mov 	byte [edi + pcidev.bus], al
	movzx 	eax, byte [esi + PCIDEV.devfn]
	mov 	byte [edi + pcidev.devfn], al
	mov 	dword [edi + pcidev.num], ecx

	jmp	.next_dev

.err:
	xor 	eax, eax
	ret

endp

; Returns 1 if the NVMe device is compatible. 0 otherwise. In practice, the driver
; is compatible with (hopefully) most compliant controllers. This also does some
; initialization for some reason, due to bad design decisions made in the beginning
; but since the code works I haven't felt inclined to change it.
proc device_is_compat stdcall, pci:dword

	push  	 esi edx ecx
	mov 	 esi, [pci]
	invoke 	 PciRead8, dword [esi + pcidev.bus], dword [esi + pcidev.devfn], PCI_header00.interrupt_line
	mov 	 byte [esi + pcidev.iline], al
	invoke   PciRead32, dword [esi + pcidev.bus], dword [esi + pcidev.devfn], PCI_header00.base_addr_0
	and      eax, 0xfffffff0
	test     eax, eax
	jz       .failure
	mov 	 edx, eax

	invoke   MapIoMem, eax, 0x2000, PG_SW+PG_NOCACHE
	test     eax, eax
	jz       .failure
	mov 	 dword [esi + pcidev.io_addr], eax
	mov 	 eax, dword [eax + NVME_MMIO.CAP + 4]
	and 	 eax, CAP_DSTRD
	mov 	 byte [esi + pcidev.dstrd], al
	mov 	 eax, dword [esi + pcidev.io_addr]
	mov 	 eax, dword [eax + NVME_MMIO.VS]
	DEBUGF   DBG_INFO, "nvme%u: Controller version: 0x%x\n", [esi + pcidev.num], eax
	mov 	 dword [esi + pcidev.version], eax
	pop 	 ecx edx esi
	xor 	 eax, eax
	inc 	 eax
	ret

.failure:
	DEBUGF  DBG_INFO, "nvme%u: something went wrong checking NVMe device compatibility\n", [esi + pcidev.num]
	pop 	 ecx edx esi
	xor      eax, eax
	ret
	
endp

; nvme_init: Initializes the NVMe controller, I/O queues, and namespaces.
proc nvme_init stdcall, pci:dword

	push 	 ebx esi edi
	mov 	 esi, dword [pci]

	; Check the PCI header to see if interrupts are disabled, if so
	; we have to re-enable them
	invoke   PciRead16, dword [esi + pcidev.bus], dword [esi + pcidev.devfn], PCI_header00.command
	and 	 eax, not (1 shl 10)
	; Enable Bus Master bit, memory space access, and I/O space access. QEMU automatically sets the
	; bus master bit, but Virtualbox does not. Not sure about the other bits though, but let's set them
	; to 1 to anyway just to be extra cautious.
	; See: https://git.kolibrios.org/GSoC/kolibrios-nvme-driver/issues/1#issuecomment-467
	or 	 eax, (1 shl 2) or (1 shl 1) or 1
	invoke   PciWrite16, dword [esi + pcidev.bus], dword [esi + pcidev.devfn], PCI_header00.command, eax

	; Check if the device has a pointer to the capabilities list (status register bit 4 set to 1)
	; though this check is probably unnecessary since all PCIe devices should have this bit set to 1
	invoke   PciRead16, dword [esi + pcidev.bus], dword [esi + pcidev.devfn], PCI_header00.status
	test 	 ax, (1 shl 4)
	jz 	 .exit_fail

	invoke   PciRead8, dword [esi + pcidev.bus], dword [esi + pcidev.devfn], PCI_header00.cap_ptr
	and 	 eax, 0xfc ; bottom two bits are reserved, so mask them before we access the configuration space
	mov 	 edi, eax
	DEBUGF   DBG_INFO, "nvme%u: Checking capabilities...\n", [esi + pcidev.num]

; We need to check if there are any MSI/MSI-X capabilities, and if so, make sure they're disabled since
; we're using old fashioned pin-based interrupts (for now). A controller may expose both of them, so
; the whole list is walked instead of stopping at the first one found. EBX keeps the next capability
; pointer because the PCI accesses below clobber EAX.
	push 	 PCI_MAX_CAPS ; guard against a malformed (circular) capability list

.read_cap:
	invoke   PciRead32, dword [esi + pcidev.bus], dword [esi + pcidev.devfn], edi
	movzx 	 ecx, ah
	and 	 ecx, 0xfc ; bottom two bits of the next pointer are reserved
	push 	 ecx
	add 	 edi, 2 ; -> Message Control register of this capability
	cmp 	 al, MSICAP_CID
	je 	 .got_msi_cap
	cmp 	 al, MSIXCAP_CID
	je 	 .got_msix_cap

.next_cap:
	pop 	 edi
	dec 	 dword [esp]
	jz 	 .end_cap_parse_pop
	test 	 edi, edi
	jnz 	 .read_cap

.end_cap_parse_pop:
	add 	 esp, 4
	jmp 	 .end_cap_parse

; EDI points to the Message Control register of the capability (CAP + 2),
; so a 16-bit access is enough and it never touches the neighbouring fields.
.got_msi_cap:
	DEBUGF   DBG_INFO, "nvme%u: Found MSI capability, disabling it\n", [esi + pcidev.num]
	invoke   PciRead16, dword [esi + pcidev.bus], dword [esi + pcidev.devfn], edi
	and      eax, not MSICAP_MSIE
	invoke   PciWrite16, dword [esi + pcidev.bus], dword [esi + pcidev.devfn], edi, eax
	jmp 	 .next_cap

.got_msix_cap:
	DEBUGF   DBG_INFO, "nvme%u: Found MSI-X capability, disabling it\n", [esi + pcidev.num]
	invoke   PciRead16, dword [esi + pcidev.bus], dword [esi + pcidev.devfn], edi
	and 	 eax, not MSIXCAP_MXE
	invoke   PciWrite16, dword [esi + pcidev.bus], dword [esi + pcidev.devfn], edi, eax
	jmp 	 .next_cap

.end_cap_parse:
	mov      edi, dword [esi + pcidev.io_addr]

	; check maximum queue entries supported
	mov 	eax, dword [edi + NVME_MMIO.CAP]
	DEBUGF  DBG_INFO, "nvme%u: Maximum queue entries available is %u (required: %u)\n", [esi + pcidev.num], ax, SQ_ENTRIES
	cmp 	ax, SQ_ENTRIES - 1 ; CAP.MQES is a 0's based value
	jb 	.exit_fail

	if __DEBUG__
		test 	eax, CAP_CQR
		setnz 	al
		DEBUGF  DBG_INFO, "nvme%u: Contiguous queues required: %u\n", [esi + pcidev.num], al
	end if
	
	; Check if NVM command set is supported
	mov      eax, dword [edi + NVME_MMIO.CAP + 4]
	DEBUGF   DBG_INFO, "nvme%u: Checking if NVM command set is supported...\n", [esi + pcidev.num]
	test 	 eax, CAP_CSS_NVM_CMDSET
	jz       .exit_fail
	DEBUGF   DBG_INFO, "nvme%u: OK... NVM command set supported\n", [esi + pcidev.num]

	stdcall nvme_disable_ctrl, esi
	test 	eax, eax
	jz 	.exit_fail
	DEBUGF  DBG_INFO, "nvme%u: Checking if memory page size is supported...\n", [esi + pcidev.num]
	mov 	eax, dword [edi + NVME_MMIO.CAP + 4]
	mov 	edx, eax
	and 	edx, CAP_MPSMIN
	shr 	edx, 16
	cmp 	edx, NVM_MPS
	ja 	.exit_fail
	and 	eax, CAP_MPSMAX
	shr 	eax, 20
	cmp 	eax, NVM_MPS
	jb	.exit_fail
	DEBUGF  DBG_INFO, "nvme%u: OK... memory page size supported\n", [esi + pcidev.num]

	; Configure IOSQES, IOCQES, AMS, MPS, CSS
	; 	CSS = 0 (NVM Command Set)
	; 	AMS = 0 (Round Robin)
	; 	MPS = 0 (4KiB Pages)
	; 	IOSQES = 6 (64B)
	; 	IOCQES = 4 (16B)
	xor 	eax, eax
	or 	eax, CC_DEFAULT_IOSQES or CC_DEFAULT_IOCQES
	mov 	dword [edi + NVME_MMIO.CC], eax
	DEBUGF  DBG_INFO, "nvme%u: OK... controller is configured to appropriate settings\n", [esi + pcidev.num]

	; Configure Admin Queue Attributes
	xor 	eax, eax
	or 	eax, (NVM_ASQS - 1) or ((NVM_ACQS - 1) shl 16) ; both fields are 0's based
	mov 	dword [edi + NVME_MMIO.AQA], eax
	DEBUGF  DBG_INFO, "nvme%u: Admin queue attributes: 0x%x\n", [esi + pcidev.num], eax

	; Allocate list of queues
	DEBUGF  DBG_INFO, "nvme%u: Allocating Administrator and I/O queues...\n",, [esi + pcidev.num]
	invoke  KernelAlloc, sizeof.NVM_QUEUE_ENTRY * (LAST_QUEUE_ID + 1)
	test 	eax, eax
	jz 	.exit_fail
	mov 	dword [esi + pcidev.queue_entries], eax
	mov 	edi, eax
	stdcall memsetdz, eax, sizeof.NVM_QUEUE_ENTRY * (LAST_QUEUE_ID + 1) / 4

	; Allocate submission/completion queue pointers
	xor 	ebx, ebx

.init_queues:
	invoke  KernelAlloc, QUEUE_ALLOC_SIZE
	test 	eax, eax
	jz 	.exit_fail
	DEBUGF  DBG_INFO, "nvme%u: Allocated queue at offset %u: 0x%x\n", [esi + pcidev.num], ebx, eax
	mov 	dword [edi + ebx + NVM_QUEUE_ENTRY.cq_ptr], eax
	mov 	edx, eax
	add 	eax, CQ_ALLOC_SIZE
	mov 	dword [edi + ebx + NVM_QUEUE_ENTRY.sq_ptr], eax
	mov 	byte [edi + ebx + NVM_QUEUE_ENTRY.phase], CQ_PHASE_TAG
	stdcall memsetdz, edx, QUEUE_ALLOC_SIZE / 4

	; Initialize command entries
	invoke  KernelAlloc, sizeof.NVMQCMD * CQ_ENTRIES
	test 	eax, eax
	jz 	.exit_fail
	mov 	dword [edi + ebx + NVM_QUEUE_ENTRY.cmd_ptr], eax
	push 	ebx esi
	mov  	esi, eax
	xor 	ebx, ebx

.init_cmd_entries:
	invoke  KernelAlloc, sizeof.MUTEX
	test 	eax, eax
	jz 	.exit_fail_cleanup
	mov 	dword [esi + NVMQCMD.mutex_ptr], eax
	mov 	dword [esi + NVMQCMD.cid], ebx
	mov 	ecx, eax
	invoke  MutexInit
	inc 	ebx
	add 	esi, sizeof.NVMQCMD
	cmp 	ebx, CQ_ENTRIES
	jne 	.init_cmd_entries

	pop 	esi ebx
	add 	ebx, sizeof.NVM_QUEUE_ENTRY
	cmp 	ebx, (LAST_QUEUE_ID + 1) * sizeof.NVM_QUEUE_ENTRY
	jne 	.init_queues

	; Configure Admin Completion Queue Base Address
	mov 	esi, [pci]
	mov 	esi, dword [esi + pcidev.io_addr]
	mov 	eax, dword [edi + NVM_QUEUE_ENTRY.cq_ptr]
	invoke  GetPhysAddr
	mov 	dword [esi + NVME_MMIO.ACQ], eax
	mov 	dword [esi + NVME_MMIO.ACQ + 4], 0
	if __DEBUG__
		push 	esi
		mov 	esi, [pci]
		DEBUGF  DBG_INFO, "nvme%u: Admin completion queue base address: 0x%x\n", [esi + pcidev.num], eax
		pop 	esi
	end if

	; Configure Admin Submission Queue Base Address
	mov 	eax, dword [edi + NVM_QUEUE_ENTRY.sq_ptr]
	invoke  GetPhysAddr
	mov 	dword [esi + NVME_MMIO.ASQ], eax
	mov 	dword [esi + NVME_MMIO.ASQ + 4], 0
	if __DEBUG__
		push 	esi
		mov 	esi, [pci]
		DEBUGF  DBG_INFO, "nvme%u: Admin submission queue base address: 0x%x\n", [esi + pcidev.num], eax
		pop 	esi
	end if

	; Attach interrupt handler
	mov 	esi, [pci]
	movzx 	eax, byte [esi + pcidev.iline]
	DEBUGF  DBG_INFO, "nvme%u: Attaching interrupt handler to IRQ %u\n", [esi + pcidev.num], eax
	invoke  AttachIntHandler, eax, irq_handler, 0
	test 	eax, eax
	jz 	.exit_fail
	DEBUGF  DBG_INFO, "nvme%u: Successfully attached interrupt handler\n", [esi + pcidev.num]

	; Restart the controller
	stdcall nvme_enable_ctrl, esi
	test 	eax, eax
	jz 	.exit_fail

	invoke  KernelAlloc, 0x1000
	test 	eax, eax
	jz 	.exit_fail
	mov 	edi, eax
	invoke  GetPhysAddr
	;                      pci:dword, nsid:dword, dptr:dword, cns:byte
	stdcall nvme_identify, [pci],     0,          eax,        CNS_IDCS
	test 	eax, eax
	jz 	.exit_fail
	mov 	eax, dword [edi + IDENTC.nn]
	mov 	dword [esi + pcidev.nn], eax
	DEBUGF  DBG_INFO, "nvme%u: Namespace Count: %u\n", [esi + pcidev.num], eax
	mov 	al, byte [edi + IDENTC.mdts]
	mov 	byte [esi + pcidev.mdts], al
	DEBUGF  DBG_INFO, "nvme%u: IDENTC.MDTS = %u\n", [esi + pcidev.num], al

	; Note that the specification only allows ASCII strings that contain code
	; values between 0x20 (' ') and 0x7E ('~'). Strings are left justified and
	; padded with spaces (at least according to the 1.4.0 spec) which means there
	; is no null terminator anywhere. To prevent garbage or repeated values from
	; being printed to the debug log, I have inserted a 0 byte at the end of each
	; string.
	lea 	ebx, byte [edi + IDENTC.sn]
	mov 	byte [ebx + 19], 0
	DEBUGF  DBG_INFO, "nvme%u: Serial Number: %s\n", [esi + pcidev.num], ebx
	add 	ebx, 20
	mov 	byte [ebx + 39], 0
	DEBUGF  DBG_INFO, "nvme%u: Model Number: %s\n", [esi + pcidev.num], ebx
	add 	ebx, 40
	mov 	byte [ebx + 7], 0
	DEBUGF  DBG_INFO, "nvme%u: Firmware Revision: %s\n", [esi + pcidev.num], ebx
	mov 	edx, dword [esi + pcidev.version]
	
	cmp 	edx, VS140
	jb 	@f
	; This is a reserved field in pre-1.4 controllers
	mov 	al, byte [edi + IDENTC.cntrltype]
	cmp 	al, CNTRLTYPE_IO_CONTROLLER
	jne 	.exit_fail 	
	;DEBUGF  DBG_INFO, "nvme%u: I/O controller detected...\n", [esi + pcidev.num]

@@:
	; TODO: check IDENTC.AVSCC
	mov 	al, byte [edi + IDENTC.sqes]
	and 	al, 11110000b
	DEBUGF  DBG_INFO, "nvme%u: IDENTC.SQES = %u\n", [esi + pcidev.num], al
	cmp 	al, 0x60 ; maximum submission queue size should at least be 64 bytes
	jb 	.exit_fail
	mov 	al, byte [edi + IDENTC.cqes]
	and 	al, 11110000b
	DEBUGF  DBG_INFO, "nvme%u: IDENTC.CQES = %u\n", [esi + pcidev.num], al
	cmp 	al, 0x40 ; maximum completion queue entry size should at least be 16 bytes
	jb 	.exit_fail
	invoke  KernelFree, edi

	mov 	eax, 1 or (1 shl 16) ; CDW11 (set the number of queues we want)
	mov 	esi, [pci]
	mov 	dword [esi + pcidev.spinlock], 1
	stdcall set_features, [pci], NULLPTR, FID_NUMBER_OF_QUEUES, eax
	stdcall nvme_poll, esi
	test 	eax, eax
	jz 	.exit_fail
	mov 	esi, dword [esi + pcidev.queue_entries]
	mov 	esi, dword [esi + NVM_QUEUE_ENTRY.cq_ptr]
	mov 	eax, dword [esi + sizeof.CQ_ENTRY + CQ_ENTRY.cdw0]
	;DEBUGF  DBG_INFO, "nvme%u: Set Features CDW0: 0x%x\n", [esi + pcidev.num], eax
	test 	ax, ax ; Number of I/O Submission Queues allocated
	jz 	.exit_fail
	shl 	eax, 16
	test 	ax, ax ; Number of I/O Completion Queues allocated
	jnz	.exit_fail

	; Create I/O Queues
	; (TODO: create N queue pairs for N CPU cores, see page 8 of NVMe 1.4 spec for an explaination)
	mov 	esi, [pci]
	mov 	edi, esi
	mov 	esi, dword [esi + pcidev.queue_entries]
	add 	esi, sizeof.NVM_QUEUE_ENTRY
	mov 	eax, dword [esi + NVM_QUEUE_ENTRY.cq_ptr]
	invoke 	GetPhysAddr
	stdcall create_io_completion_queue, [pci], eax, 1, IEN_ON
	test 	eax, eax
	jz 	.exit_fail
	;DEBUGF  DBG_INFO, "nvme%u: Successfully created I/O completion queue 1\n", [edi + pcidev.num]
	mov 	eax, dword [esi + NVM_QUEUE_ENTRY.sq_ptr]
	invoke 	GetPhysAddr
	stdcall create_io_submission_queue, [pci], eax, 1, 1
	test 	eax, eax
	jz 	.exit_fail
	;DEBUGF  DBG_INFO, "nvme%u: Successfully created I/O submission queue 1\n", [edi + pcidev.num]

	; TODO: This only registers a single namespace. Add support for more
	stdcall determine_active_nsids, [pci]
	test 	eax, eax
	jz 	.exit_fail ; No active NSIDS
	mov 	esi, [pci]
	mov 	dword [esi + pcidev.nsid], eax
	DEBUGF  DBG_INFO, "nvme%u: Found active NSID: %u\n", [esi + pcidev.num], eax

	invoke  KernelAlloc, 0x1000
	test 	eax, eax
	jz 	.exit_fail
	mov 	edi, eax
	invoke  GetPhysAddr
	stdcall nvme_identify, [pci], [esi + pcidev.nsid], eax, CNS_IDNS
	test 	eax, eax
	jz 	.exit_fail
	invoke  KernelAlloc, sizeof.NSINFO
	test 	eax, eax
	jz 	.exit_fail
	mov 	ebx, eax
	mov 	dword [esi + pcidev.nsinfo], eax
	mov 	al, byte [edi + IDENTN.nsfeat]
	mov 	byte [ebx + NSINFO.features], al
	;DEBUGF  DBG_INFO, "nvme%un%u: Namespace Features: 0x%x\n", [esi + pcidev.num], [esi + pcidev.nsid], al
	mov 	eax, dword [esi + pcidev.nsid]
	mov 	dword [ebx + NSINFO.nsid], eax
	mov 	dword [ebx + NSINFO.pci], esi
	mov 	eax, dword [edi + IDENTN.nsze]
	mov 	dword [ebx + NSINFO.size], eax
	mov 	eax, dword [edi + IDENTN.nsze + 4]
	mov 	dword [ebx + NSINFO.size + 4], eax
	mov 	eax, dword [edi + IDENTN.ncap]
	mov 	dword [ebx + NSINFO.capacity], eax 
	mov 	eax, dword [edi + IDENTN.ncap + 4]
	mov 	dword [ebx + NSINFO.capacity + 4], eax 
	;DEBUGF  DBG_INFO, "nvme%un%u: Namespace Size: %u + %u logical blocks\n", [esi + pcidev.num], [esi + pcidev.nsid], [edi + IDENTN.nsze], [edi + IDENTN.nsze + 4]
	;DEBUGF  DBG_INFO, "nvme%un%u: Namespace Capacity: %u + %u logical blocks\n", [esi + pcidev.num], [esi + pcidev.nsid], [edi + IDENTN.ncap], [edi + IDENTN.ncap + 4]
	mov 	eax, dword [edi + IDENTN.lbaf0]
	shr 	eax, 16 ; Get LBADS

	; KolibriOS only supports a LBADS of 512, so if it's a higher value then we
	; have to ignore this namespace
	cmp 	al, SUPPORTED_LBADS
	jne 	.exit_fail

	mov 	byte [ebx + NSINFO.lbads], al
	invoke  KernelFree, edi

	; Work out the largest transfer a single I/O command may carry. MDTS is given in
	; 2^(12 + CAP.MPSMIN) byte units (MPSMIN is known to be 0 here, it was checked
	; above) and a value of 0 means the controller imposes no limit. Whatever the
	; controller allows, stay below MAX_PRP_PAGES so one plain PRP list always fits.
	movzx 	ecx, byte [esi + pcidev.mdts]
	mov 	eax, MAX_PRP_PAGES
	test 	ecx, ecx
	jz 	@f
	cmp 	ecx, 31
	jae 	@f ; nonsensical value, keep our own cap
	mov 	edx, 1
	shl 	edx, cl
	cmp 	edx, eax
	jae 	@f
	mov 	eax, edx

@@:
	mov 	cl, byte [ebx + NSINFO.lbads]
	mov 	edx, PAGE_SIZE
	shr 	edx, cl ; sectors per memory page
	imul 	eax, edx
	mov 	dword [esi + pcidev.max_sectors], eax
	DEBUGF  DBG_INFO, "nvme%u: Maximum transfer size: %u sectors\n", [esi + pcidev.num], eax
	if 0
		invoke  KernelAlloc, 0x6000
		test 	eax, eax
		jz 	.exit_fail
		mov 	edi, eax
		invoke  KernelAlloc, 0x8
		test 	eax, eax
		jz 	.exit_fail
		mov 	edx, NVM_CMD_READ
		mov 	dword [eax], 6
		add 	edi, 0x5
		mov 	dword [esi + pcidev.spinlock], 1
		stdcall nvme_readwrite, [esi + pcidev.nsinfo], edi, 0x0, 0, eax
		stdcall nvme_poll, esi
		test 	eax, eax
		jz 	.exit_fail
	        DEBUGF  DBG_INFO, "STRING: %s\n", edi
		add 	edi, 0x2000
		DEBUGF  DBG_INFO, "STRING: %s\n", edi
	end if
	DEBUGF  DBG_INFO, "nvme%u: Successfully initialized driver\n", [esi + pcidev.num]
      	xor     eax, eax
      	inc     eax
	pop 	edi esi ebx
      	ret

.exit_fail_cleanup:
	add 	esp, 8

.exit_fail:
	mov 	esi, [pci]
	DEBUGF  DBG_INFO, "nvme%u: Failed to initialize controller\n", [esi + pcidev.num]
	mov 	edi, dword [esi + pcidev.io_addr]
	mov 	eax, dword [edi + NVME_MMIO.CSTS]
	test 	eax, CSTS_CFS
	jz 	@f
	DEBUGF  DBG_INFO, "nvme%u: A fatal controller error has occurred\n", [esi + pcidev.num]

@@:
	xor 	eax, eax
	pop 	edi esi ebx
	ret

endp

; Returns a new CID for queue #y
proc get_new_cid stdcall, pci:dword, y:dword

	mov 	eax, [pci]
	mov 	eax, dword [eax + pcidev.queue_entries]
	mov 	ecx, [y]
	shl 	ecx, LOG2 sizeof.NVM_QUEUE_ENTRY
	movzx 	eax, word [eax + ecx + NVM_QUEUE_ENTRY.head]
	ret

endp

; Waits until CSTS.RDY reads back as 'ready' (0 or CSTS_RDY). The controller
; advertises its own worst case in CAP.TO, in 500ms units; never wait longer than
; that, so a controller which never comes up cannot hang the boot.
; Returns 1 if the state was reached, 0 on timeout.
proc nvme_wait_ready stdcall, pci:dword, ready:dword

	push 	ebx esi edi
	mov 	esi, [pci]
	mov 	edi, dword [esi + pcidev.io_addr]
	mov 	ebx, dword [edi + NVME_MMIO.CAP]
	shr 	ebx, 24 ; CAP.TO
	inc 	ebx ; a controller reporting 0 still deserves one round
	imul 	ebx, 500 / CTRL_POLL_MS

.wait:
	mov 	eax, dword [edi + NVME_MMIO.CSTS]
	and 	eax, CSTS_RDY
	cmp 	eax, [ready]
	je 	.ready
	mov 	esi, CTRL_POLL_MS
	invoke 	Sleep
	dec 	ebx
	jnz 	.wait
	xor 	eax, eax
	pop 	edi esi ebx
	ret

.ready:
	xor 	eax, eax
	inc 	eax
	pop 	edi esi ebx
	ret

endp

; Returns 1 if the controller went idle, 0 if it timed out.
proc nvme_disable_ctrl stdcall, pci:dword

	push 	esi edi
	mov 	esi, [pci]
	DEBUGF  DBG_INFO, "nvme%u: Disabling Controller...\n", [esi + pcidev.num]
	mov 	edi, dword [esi + pcidev.io_addr]
	and 	dword [edi + NVME_MMIO.CC], not CC_EN

; Wait for controller to be brought to idle state, CSTS.RDY should be cleared to 0 when this happens
	stdcall nvme_wait_ready, esi, 0
	test 	eax, eax
	jz 	@f
	DEBUGF  DBG_INFO, "nvme%u: Successfully disabled controller\n", [esi + pcidev.num]
	pop 	edi esi
	xor 	eax, eax
	inc 	eax
	ret

@@:
	DEBUGF  DBG_INFO, "nvme%u: Timed out waiting for the controller to go idle\n", [esi + pcidev.num]
	pop 	edi esi
	xor 	eax, eax
	ret

endp

; Returns 1 if the controller became ready, 0 if it timed out.
proc nvme_enable_ctrl stdcall, pci:dword

	push 	esi edi
	mov 	esi, [pci]
	DEBUGF  DBG_INFO, "nvme%u: Enabling Controller...\n", [esi + pcidev.num]
	mov 	edi, dword [esi + pcidev.io_addr]
	or 	dword [edi + NVME_MMIO.CC], CC_EN

; Wait for controller to be brought into active state, CSTS.RDY should be set to 1 when this happens
	stdcall nvme_wait_ready, esi, CSTS_RDY
	test 	eax, eax
	jz 	@f
	DEBUGF  DBG_INFO, "nvme%u: Successfully enabled controller\n", [esi + pcidev.num]
	pop 	edi esi
	xor 	eax, eax
	inc 	eax
	ret

@@:
	DEBUGF  DBG_INFO, "nvme%u: Timed out waiting for the controller to become ready\n", [esi + pcidev.num]
	pop 	edi esi
	xor 	eax, eax
	ret

endp

; Waits for the command in flight to complete.
;
; TODO: this whole arrangement - one command in flight per device, a lock spun on
; by the thread that submitted it - should give way to the kernel's asynchronous
; API, so that a thread issuing disk I/O sleeps on an event and the queues can
; hold more than one command at a time. It was left as a spinlock because the
; driver had enough other bugs at the time that changing the completion path as
; well would have made regressions impossible to attribute. The queues, their
; doorbells and the phase tracking are already per queue, so what has to change
; is this procedure, the claim in nvme_drain_cq, and giving each NVMQCMD entry
; its own event instead of sharing pcidev.spinlock.
;
; The interrupt handler releases the lock as soon as it takes the completion, but
; the driver does not depend on that: plenty of machines give an NVMe controller
; no usable pin interrupt at all (the controller is MSI-X only, or the firmware
; left the interrupt line at 0xFF), so after a short grace period the completion
; queues are drained from here instead.
;
; Returns 1 when the command completed without error, 0 on failure or timeout.
proc nvme_poll stdcall, pci:dword
locals
	busy_left 	dd ? ; rounds still to spend spinning
	slow_left 	dd ? ; rounds still to spend sleeping
endl

	push 	ebx esi edi
	mov 	edi, [pci]
	mov 	dword [busy_left], IO_BUSY_ROUNDS
	mov 	dword [slow_left], IO_TIMEOUT_MS / CTRL_POLL_MS
	mov 	ecx, POLL_SPINS

.wait:
	xor 	eax, eax
	inc 	eax
	xchg 	eax, dword [edi + pcidev.spinlock]
	test 	eax, eax
	jz 	.completed
	dec 	ecx
	jnz 	.wait

	; look at the completion queues ourselves rather than trust the interrupt
	stdcall nvme_drain_cq, edi
	mov 	ecx, POLL_SPINS
	dec 	dword [busy_left]
	jns 	.wait

	; nothing yet after a good while: stop burning CPU and wait in slices
	mov 	dword [busy_left], 0
	mov 	esi, CTRL_POLL_MS
	invoke 	Sleep
	dec 	dword [slow_left]
	jnz 	.wait

	DEBUGF  DBG_INFO, "nvme%u: Timed out waiting for a command to complete\n", [edi + pcidev.num]
	pop 	edi esi ebx
	xor 	eax, eax
	ret

.completed:
	mov 	eax, dword [edi + pcidev.last_status]
	test 	eax, eax
	jnz 	.failed
	pop 	edi esi ebx
	xor 	eax, eax
	inc 	eax
	ret

.failed:
	DEBUGF  DBG_INFO, "nvme%u: Command failed, status 0x%x\n", [edi + pcidev.num], eax
	pop 	edi esi ebx
	xor 	eax, eax
	ret

endp


; Writes to completion queue 'y' head doorbell. 'cqh' should
; be the new head value that will be stored in the register.
proc cqyhdbl_write stdcall, pci:dword, y:dword, cqh:dword

	push 	esi edi
	mov 	esi, [pci]

	; 1000h + ((2y + 1) * (4 << CAP.DSTRD))
	mov 	eax, [y]
	shl 	al, 1
	inc 	al
	mov 	edx, 4
	mov 	cl, byte [esi + pcidev.dstrd]
	shl 	dx, cl
	imul 	dx, ax
	add 	dx, 0x1000
	mov 	ecx, [y]
	shl 	ecx, LOG2 sizeof.NVM_QUEUE_ENTRY
	mov 	edi, dword [esi + pcidev.queue_entries]
	lea 	edi, dword [edi + ecx]
	mov 	eax, [cqh]
	mov 	esi, dword [esi + pcidev.io_addr]
	mov 	word [esi + edx], ax ; Write to CQyHDBL
	mov 	word [edi + NVM_QUEUE_ENTRY.head], ax

	; NOTE: Currently commented out since we're just using
	; plain spinlocks for notifying when a command has been
	; completed, but this will be uncommented later and use
	; semaphores instead of mutexes once the polling code
	; has been replaced with the asynchronous API.

	; Unlock the mutex now that the command is complete
	;mov 	edi, dword [edi + NVM_QUEUE_ENTRY.cmd_ptr]
	;mov 	ecx, [cqh]
	;shl 	ecx, SIZEOF_NVMQCMD
	;add 	edi, ecx
	;mov 	ecx, dword [edi + NVMQCMD.mutex_ptr]
	;invoke  MutexUnlock

	pop 	edi esi
	ret

endp

; Writes to submission queue 'y' tail doorbell. 'cmd' should
; be a pointer to the submission queue struct.
proc sqytdbl_write stdcall, pci:dword, y:word, cmd:dword

	push 	ebx esi edi
	mov 	edi, [pci]
	mov 	edi, dword [edi + pcidev.queue_entries]
	movzx 	ebx, [y]
	shl 	ebx, LOG2 sizeof.NVM_QUEUE_ENTRY
	lea 	edi, [edi + ebx]
	;mov 	eax, dword [edi + NVM_QUEUE_ENTRY.cmd_ptr]
	mov 	edx, dword [edi + NVM_QUEUE_ENTRY.sq_ptr]
	mov 	esi, [cmd]

	; The command goes into the slot the tail doorbell is about to point past,
	; which is the one the controller will fetch. (This used to index by CID,
	; which only happened to agree with the tail while exactly one command was
	; ever in flight.)
	movzx 	ecx, word [edi + NVM_QUEUE_ENTRY.tail]
	shl 	ecx, LOG2 sizeof.SQ_ENTRY
	lea 	edx, [edx + ecx]
	stdcall memcpyd, edx, esi, sizeof.SQ_ENTRY / 4
	mov 	esi, [pci]
	mov 	dword [esi + pcidev.last_status], 0
	;mov 	ecx, dword [ebx + NVMQCMD.mutex_ptr]
	;invoke  MutexLock

	mov 	esi, [pci]
	mov 	ax, word [edi + NVM_QUEUE_ENTRY.tail]
	inc 	ax
	cmp 	ax, NVM_ASQS ; valid indices are 0..NVM_ASQS-1
	jb	@f
	xor 	ax, ax
	
@@:
	; 1000h + (2y * (4 << CAP.DSTRD))
	movzx 	ebx, [y]
	shl 	ebx, 1
	mov 	edx, 4
	mov 	cl, byte [esi + pcidev.dstrd]
	shl 	edx, cl
	imul 	edx, ebx
	add 	edx, 0x1000
	mov 	word [edi + NVM_QUEUE_ENTRY.tail], ax
	mov 	esi, dword [esi + pcidev.io_addr]
	mov 	word [esi + edx], ax
	pop 	edi esi ebx
	ret

endp

proc is_queue_full stdcall, tail:word, head:word
	
	push 	bx
	mov 	ax, [tail]
	mov 	bx, [head]
	cmp 	ax, bx
	je 	.not_full
	test 	bx, bx
	jnz 	@f
	cmp 	ax, NVM_ASQS - 1
	jne 	@f
	pop 	bx
	xor 	eax, eax
	inc 	eax
	ret

@@:
	cmp 	ax, bx
	jae 	.not_full
	sub 	ax, bx
	cmp 	ax, 1
	jne 	.not_full
	pop 	bx
	xor 	eax, eax
	inc 	eax
	ret
	
.not_full:
	pop 	bx
	xor 	eax, eax
	ret

endp

; Returns 1 if the controller has posted a completion in the given queue that the
; driver has not taken yet. The phase tag is what says so: the controller flips it
; every time it wraps around the queue, so an entry carrying the tag we expect next
; is one it has just written.
proc cq_entry_pending stdcall, pci:dword, queue:dword

	push 	esi edi
	mov 	esi, [pci]
	mov 	ecx, [queue]
	shl 	ecx, LOG2 sizeof.NVM_QUEUE_ENTRY
	mov 	esi, dword [esi + pcidev.queue_entries]
	test 	esi, esi
	jz 	.no_queues
	lea 	esi, [esi + ecx]
	movzx 	ecx, word [esi + NVM_QUEUE_ENTRY.head]
	shl 	ecx, LOG2 sizeof.CQ_ENTRY
	mov 	edi, dword [esi + NVM_QUEUE_ENTRY.cq_ptr]
	movzx 	eax, word [edi + ecx + CQ_ENTRY.status]
	and 	eax, CQ_PHASE_TAG
	movzx 	edx, byte [esi + NVM_QUEUE_ENTRY.phase]
	cmp 	eax, edx
	je 	.pending

.no_queues:
	pop 	edi esi
	xor 	eax, eax
	ret

.pending:
	pop 	edi esi
	xor 	eax, eax
	inc 	eax
	ret

endp

; Takes every completion the controller has posted in the given queue, remembers
; whether any of them reported an error, and tells the controller how far the
; driver has got by writing the completion queue head doorbell. Returns the number
; of entries taken.
proc consume_cq_entries stdcall, pci:dword, queue:dword

	push 	ebx esi edi
	xor 	ebx, ebx ; number of entries taken

.next:
	stdcall cq_entry_pending, [pci], [queue]
	test 	eax, eax
	jz 	.done
	mov 	esi, [pci]
	mov 	ecx, [queue]
	shl 	ecx, LOG2 sizeof.NVM_QUEUE_ENTRY
	mov 	esi, dword [esi + pcidev.queue_entries]
	lea 	esi, [esi + ecx]
	movzx 	ecx, word [esi + NVM_QUEUE_ENTRY.head]
	mov 	edi, dword [esi + NVM_QUEUE_ENTRY.cq_ptr]
	shl 	ecx, LOG2 sizeof.CQ_ENTRY
	movzx 	eax, word [edi + ecx + CQ_ENTRY.status]
	shr 	eax, 1 ; drop the phase tag, what is left is the status
	mov 	edi, [pci]
	or 	dword [edi + pcidev.last_status], eax
	inc 	ebx

	movzx 	ecx, word [esi + NVM_QUEUE_ENTRY.head]
	inc 	ecx
	cmp 	ecx, NVM_ACQS ; valid indices are 0..NVM_ACQS-1
	jb 	@f
	xor 	ecx, ecx
	xor 	byte [esi + NVM_QUEUE_ENTRY.phase], CQ_PHASE_TAG ; the queue wrapped

@@:
	mov 	word [esi + NVM_QUEUE_ENTRY.head], cx
	stdcall cqyhdbl_write, [pci], [queue], ecx
	jmp 	.next

.done:
	test 	ebx, ebx
	jz 	@f

	; a command finished, wake whoever is waiting for it
	mov 	esi, [pci]
	xor 	eax, eax
	xchg 	eax, dword [esi + pcidev.spinlock]

@@:
	mov 	eax, ebx
	pop 	edi esi ebx
	ret

endp

; Drains both completion queues of a device. The waiting thread and the interrupt
; handler both call this, so a claim keeps them from taking the same entry twice.
;
; The controller's interrupts stay masked for the duration. That is what makes the
; handoff safe: an interrupt that arrives while the other side is already draining
; finds the claim taken and returns without consuming anything, and because it left
; the interrupts masked, the controller stops re-asserting a pin that nobody is
; going to service - otherwise it would re-enter the handler forever and the thread
; holding the claim would never run again. Whoever holds the claim unmasks on its
; way out, which also re-raises the interrupt if any completion is still waiting.
proc nvme_drain_cq stdcall, pci:dword

	push 	esi edi
	mov 	esi, [pci]
	mov 	edi, dword [esi + pcidev.io_addr]
	mov 	dword [edi + NVME_MMIO.INTMS], 0x3
	xor 	eax, eax
	inc 	eax
	xchg 	eax, dword [esi + pcidev.cqlock]
	test 	eax, eax
	jnz 	.busy
	stdcall consume_cq_entries, esi, ADMIN_QUEUE
	stdcall consume_cq_entries, esi, 1
	mov 	dword [esi + pcidev.cqlock], 0
	mov 	dword [edi + NVME_MMIO.INTMC], 0x3

.busy:
	pop 	edi esi
	ret

endp

; Our interrupt handler. Once the controller finishes a command,
; it should generate an interrupt (assuming that no fatal error
; occurred). If an interrupt isn't being generated when it is expected
; to, check the CSTS register to make sure that the error bit isn't being
; set. The controller doesn't generate any interrupts in such cases.
;
; Once a command has complete (successfully or not), the controller will
; add a new completion queue entry and it is the interrupt handler's 
; responsibility to write to the appropriate completion queue's head doorbell
; register and update it correctly, otherwise the controller will continue
; to generate interrupts (the most common causes for freezes with the driver,
; in my experience).
proc irq_handler

	push 	ebx esi edi
	mov 	esi, dword [p_nvme_devices]
	mov 	ebx, dword [num_pcidevs_sz]
	add 	ebx, esi

.check_who_raised_irq:
	stdcall device_generated_interrupt, esi
	test 	eax, eax
	jnz 	@f
	add 	esi, sizeof.pcidev
	cmp 	esi, ebx
	jb 	.check_who_raised_irq

	; Interrupt not handled by driver, return 0
	pop 	edi esi ebx
	xor 	eax, eax
	ret

@@:
	stdcall nvme_drain_cq, esi

	; Interrupt handled by driver, return 1
	pop 	edi esi ebx
	mov 	eax, 1
	ret

endp

; The IRQ line may be shared, so only claim the interrupt when one of this
; device's completion queues really does hold something new.
proc device_generated_interrupt stdcall, pci:dword

	push 	ebx
	xor 	ebx, ebx

@@:
	stdcall cq_entry_pending, [pci], ebx
	test 	eax, eax
	jnz 	.ours
	inc 	ebx
	cmp 	ebx, LAST_QUEUE_ID
	jbe 	@b
	pop 	ebx
	xor 	eax, eax
	ret

.ours:
	pop 	ebx
	mov 	eax, 1
	ret

endp

; Deletes the allocated I/O queues for all of the NVMe devices,
; and shuts down all of the controllers. See page 295-297 of
; the NVMe 1.4.0 spec for details on how shutdown processing
; should occur.
;
; Currently shutdown still has problems on VMWare.
; See: https://git.kolibrios.org/GSoC/kolibrios-nvme-driver/issues/5
proc nvme_cleanup

	DEBUGF  DBG_INFO, "nvme: Cleaning up...\n"
	push 	ebx esi edi
	mov 	esi, dword [p_nvme_devices]
	test 	esi, esi
	jnz 	@f
	pop 	edi esi ebx
	ret

@@:
	sub 	esi, sizeof.pcidev
	xor 	ebx, ebx

.get_pcidev:
	add 	esi, sizeof.pcidev

	; Free the queues
	mov 	edi, dword [esi + pcidev.queue_entries]
	test 	edi, edi
	jz 	.ret
	sub 	edi, sizeof.NVM_QUEUE_ENTRY
	push 	ebx
	xor 	ebx, ebx

.get_queue:
	add 	edi, sizeof.NVM_QUEUE_ENTRY

	; TODO: Check if I/O completion and submission queue exist
	; before deleting?
	test 	ebx, ebx
	jz 	@f ; we don't want to delete the admin queue
	stdcall delete_io_submission_queue, esi, ebx
	stdcall delete_io_completion_queue, esi, ebx

@@:
	inc 	ebx
	cmp 	ebx, LAST_QUEUE_ID
	jbe 	.get_queue
	pop 	ebx

	; Shutdown the controller
	mov 	edi, dword [esi + pcidev.io_addr]
	mov 	eax, dword [edi + NVME_MMIO.CC]
	and 	eax, not CC_SHN
	or 	eax, CC_SHN_NORMAL_SHUTDOWN
	mov 	dword [edi + NVME_MMIO.CC], eax

	; Wait for shutdown processing to finish (CSTS.SHST = 10b) before the controller
	; is reset. The old code span while the bit was set rather than until it was, and
	; it could not give up; do neither.
	push 	ebx
	mov 	ebx, SHUTDOWN_WAIT_MS / CTRL_POLL_MS

.wait_shutdown:
	mov 	eax, dword [edi + NVME_MMIO.CSTS]
	and 	eax, CSTS_SHST
	cmp 	eax, CSTS_SHST_SHUTDOWN_COMPLETE
	je 	.shutdown_complete
	push 	esi
	mov 	esi, CTRL_POLL_MS
	invoke 	Sleep
	pop 	esi
	dec 	ebx
	jnz 	.wait_shutdown
	DEBUGF  DBG_INFO, "nvme%u: Timed out waiting for shutdown processing\n", [esi + pcidev.num]

.shutdown_complete:
	pop 	ebx
	stdcall nvme_disable_ctrl, esi

	inc 	ebx
	cmp 	ebx, dword [num_pcidevs]
	jne 	.get_pcidev

.ret:
	pop 	edi esi ebx
	ret

endp

;all initialized data place here
align 4
	p_nvme_devices dd 0 ; Pointer to array of NVMe devices
	num_pcidevs    dd 0 ; Number of NVMe devices
	num_pcidevs_sz dd 0 ; Size in bytes
	my_service   db "nvme",0  ;max 16 chars include zero
	disk_functions:
		dd 	 disk_functions.end - disk_functions
		dd 	 0 ; no close function
		dd 	 0 ; no closemedia function
		dd 	 nvme_query_media
		dd 	 nvme_read
		dd 	 nvme_write
		dd 	 0 ; no flush function
		dd 	 0 ; use default cache size
	.end:
	if __DEBUG__
	include_debug_strings
	end if

align 4
data fixups
end data


; Disk driver to create FAT16/FAT32 memory-based temporary disk aka RAM disk.
; (c) CleverMouse

; Note: in the ideal world, a disk driver should not care about a file system
; on it. In the current world, however, there is no way to format a disk in
; FAT, so this part of file-system-specific operations is included in the
; driver.

; When this driver is loading, it registers itself in the system and does
; nothing more. When loaded, this driver controls pseudo-disk devices
; named /tmp#/, where # is a digit from 0 to 9. The driver does not create
; any device by itself, waiting for instructions from an application.
; The driver responds to the following IOCTLs from a control application:
SRV_GETVERSION          equ 0 ; input ignored,
                              ; output = dword API_VERSION
DEV_ADD_DISK            equ 1 ; input = structure add_disk_struc,
                              ; no output
DEV_DEL_DISK            equ 2 ; input = structure del_disk_struc,
                              ; no output
; For all IOCTLs the driver returns one of the following error codes:
NO_ERROR                equ 0
ERROR_INVALID_IOCTL     equ 1 ; unknown IOCTL code, wrong input/output size...
ERROR_INVALID_ID        equ 2 ; .DiskId must be from 0 to 9
ERROR_SIZE_TOO_LARGE    equ 3 ; .DiskSize is too large
ERROR_SIZE_TOO_SMALL    equ 4 ; .DiskSize is too small
ERROR_NO_MEMORY         equ 5 ; memory allocation failed

include '../struct.inc'

API_VERSION             equ 1
; Input structures:
struct add_disk_struc
DiskSize        dd      ? ; disk size in sectors, 1 sector = 512 bytes
; Note: DiskSize is the full size, including FAT service data.
; Size for useful data is slightly less than this number.
DiskId          db      ? ; from 0 to 9
ends
struct del_disk_struc
DiskId          db      ? ; from 0 to 9
ends

max_num_disks   equ     10

; standard driver stuff; version of driver model = 5
format PE DLL native 0.05

DEBUG equ 0

section '.flat' code readable writable executable
data fixups
end data
entry START
include '../proc32.inc'
include '../peimport.inc'
include '../macros.inc'
; the start procedure (see the description above)
proc START
; This procedure is called in two situations:
; when the driver is loading and when the system is shutting down.
; 1. Check that the driver is loading; do nothing unless so.
        xor     eax, eax ; set return value in case we will do nothing
        cmp     dword [esp+4], 1
        jne     .nothing
; 2. Register the driver in the system.
        invoke  RegService, my_service, service_proc
; 3. Return the value returned by RegService back to the system.
.nothing:
        retn
endp

; Service procedure for the driver - handle all IOCTL requests for the driver.
; The description of handled IOCTLs is located in the start of this file.
proc service_proc
; 1. Save used registers to be stdcall.
; Note: this shifts esp, so the first parameter [esp+4] becomes [esp+16].
; Note: edi is used not by this procedure itself, but by worker procedures.
        push    ebx esi edi
; 2. Get parameter from the stack: [esp+16] is the first parameter,
;    pointer to IOCTL structure.
        mov     edx, [esp+16]    ; edx -> IOCTL
; 3. Set the return value to 'invalid IOCTL'.
; Now, if one of conditions for IOCTL does not met, the code
; can simply return the value already loaded.
        mov     al, ERROR_INVALID_IOCTL
; 4. Get request code and select a handler for the code.
        mov     ecx, [edx+IOCTL.io_code]
        test    ecx, ecx        ; check for SRV_GETVERSION
        jnz     .no.srv_getversion
; 4. This is SRV_GETVERSION request, no input, 4 bytes output, API_VERSION.
; 4a. Output size must be at least 4 bytes.
        cmp     [edx+IOCTL.out_size], 4
        jl      .return
; 4b. Write result to the output buffer.
        mov     eax, [edx+IOCTL.output]
        mov     dword [eax], API_VERSION
; 4c. Return success.
        xor     eax, eax
        jmp     .return
.no.srv_getversion:
        dec     ecx     ; check for DEV_ADD_DISK
        jnz     .no.dev_add_disk
; 5. This is DEV_ADD_DISK request, input is add_disk_struc, output is 1 byte
; 5a. Input size must be exactly sizeof.add_disk_struc bytes.
        cmp     [edx+IOCTL.inp_size], sizeof.add_disk_struc
        jnz     .return
; 5b. Load input parameters and call the worker procedure.
        mov     eax, [edx+IOCTL.input]
        movzx   ebx, [eax+add_disk_struc.DiskId]
        mov     esi, [eax+add_disk_struc.DiskSize]
        call    add_disk
; 5c. Return back to the caller the value from the worker procedure.
        jmp     .return
.no.dev_add_disk:
        dec     ecx     ; check for DEV_DEL_DISK
        jnz     .return
; 6. This is DEV_DEL_DISK request, input is del_disk_struc
; 6a. Input size must be exactly sizeof.del_disk_struc bytes.
        cmp     [edx+IOCTL.inp_size], sizeof.del_disk_struc
        jnz     .return
; 6b. Load input parameters and call the worker procedure.
        mov     eax, [edx+IOCTL.input]
        movzx   ebx, [eax+del_disk_struc.DiskId]
        call    del_disk
; 6c. Return back to the caller the value from the worker procedure.
.return:
; 7. Exit.
; 7a. The code above returns a value in al for efficiency,
; propagate it to eax.
        movzx   eax, al
; 7b. Restore used registers to be stdcall.
        pop     edi esi ebx
; 7c. Return, popping one argument.
        retn    4
endp

; The worker procedure for DEV_ADD_DISK request.
; Creates a memory-based disk of given size and formats it in FAT16/32.
; Called with ebx = disk id, esi = disk size,
; returns error code in al.
proc add_disk
; 1. Check that disk id is correct and free.
; Otherwise, return the corresponding error code.
        mov     al, ERROR_INVALID_ID
        cmp     ebx, max_num_disks
        jae     .return
        cmp     [disk_pointers+ebx*4], 0
        jnz     .return
; 2. Check that the size is reasonable.
; Otherwise, return the corresponding error code.
        mov     al, ERROR_SIZE_TOO_LARGE
        cmp     esi, MAX_SIZE
        ja      .return
        mov     al, ERROR_SIZE_TOO_SMALL
        cmp     esi, MIN_FAT16_SIZE
        jb      .return
; 3. Allocate memory for the disk, store the pointer in edi.
; If failed, return the corresponding error code.
        mov     eax, esi
        shl     eax, 9
        invoke  KernelAlloc, eax
        mov     edi, eax
        test    eax, eax
        mov     al, ERROR_NO_MEMORY
        jz      .return
; 4. Store the pointer and the size in the global variables.
; It is possible, though very unlikely, that two threads
; have called this function in parallel with the same id,
; so [disk_pointers+ebx*4] could be filled by another thread.
; Play extra safe and store new value only if old value is zero.
        xor     eax, eax
        lock cmpxchg [disk_pointers+ebx*4], edi
        jz      @f
; Otherwise, free the allocated memory and return the corresponding error code.
        invoke  KernelFree, edi
        mov     al, ERROR_INVALID_ID
        jmp     .return
@@:
        mov     [disk_sizes+ebx*4], esi
; 5. Call the worker procedure for formatting this disk.
; It should not fail.
        call    format_disk
; 6. Register the disk in the system.
; 6a. Generate name as /tmp#, where # = ebx + '0'. Use two dwords in the stack.
        push    0
        push    'tmp'
        mov     eax, esp ; eax points to 'tmp' + zero byte + zero dword
        lea     ecx, [ebx+'0'] ; ecx = digit
        mov     [eax+3], cl ; eax points to 'tmp#' + zero dword
; 6b. Call the kernel API. Use disk id as 'userdata' parameter for callbacks.
        invoke  DiskAdd, disk_functions, eax, ebx, 0
; 6c. Restore the stack after 6a.
        pop     ecx ecx
; 6c. Check the result. If DiskAdd has failed, cleanup and return
; ERROR_NO_MEMORY, this is the most probable or even the only reason to fail.
        test    eax, eax
        jnz     @f
        mov     [disk_sizes+ebx*4], 0
        mov     [disk_pointers+ebx*4], 0
        invoke  KernelFree, edi
        mov     al, ERROR_NO_MEMORY
        jmp     .return
@@:
        push    eax
; 6d. Notify the kernel that media is inserted.
        invoke  DiskMediaChanged, eax, 1
; 6e. Disk is fully configured; store its handle in the global variable
; and return success.
        pop     [disk_handles+ebx*4]
        xor     eax, eax
; 7. Return.
.return:
        retn
endp

; The worker procedure for DEV_DEL_DISK request.
; Deletes a previously created memory-based disk.
; Called with ebx = disk id,
; returns error code in al.
proc del_disk
; 1. Check that disk id is correct.
; Otherwise, return the corresponding error code.
        mov     al, ERROR_INVALID_ID
        cmp     ebx, max_num_disks
        jae     .return
; 2. Get the disk handle, simultaneously clearing the global variable.
        xor     edx, edx
        xchg    edx, [disk_handles+ebx*4]
; 3. Check that the handle is non-zero.
; Otherwise, return the corresponding error code.
        test    edx, edx
        jz      .return
; 4. Delete the disk from the system.
        invoke  DiskDel, edx
; 5. Return success.
; Note that we can't free memory yet; it will be done in tmpdisk_close.
        xor     eax, eax
.return:
        retn
endp

; Include implementation of tmpdisk_* callbacks.
include 'tmpdisk_work.inc'
; Include FAT-specific code.
include 'tmpdisk_fat.inc'

; initialized data
align 4
disk_functions:
        dd      disk_functions_end - disk_functions
        dd      tmpdisk_close
        dd      0 ; no need in .closemedia
        dd      tmpdisk_querymedia
        dd      tmpdisk_read
        dd      tmpdisk_write
        dd      0 ; no need in .flush
        dd      tmpdisk_adjust_cache_size
disk_functions_end:
; disk_handles = array of values for Disk* kernel functions
label disk_handles dword
times max_num_disks dd 0
; disk_pointers = array of pointers to disk data
label disk_pointers dword
times max_num_disks dd 0
; disk_sizes = array of disk sizes
label disk_sizes dword
times max_num_disks dd 0

my_service      db      'tmpdisk',0

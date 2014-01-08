;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2013. All rights reserved.      ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;; RAMDISK functions                                            ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 4273 $

iglobal
align 4
ramdisk_functions:
        dd      .size
        dd      0       ; no close() function
        dd      0       ; no closemedia() function
        dd      ramdisk_querymedia
        dd      ramdisk_read
        dd      ramdisk_write
        dd      0       ; no flush() function
        dd      ramdisk_adjust_cache_size
.size = $ - ramdisk_functions
endg

; See memmap.inc.
; Currently size of memory allocated for the ramdisk is fixed.
; This should be revisited when/if memory map would become more dynamic.
RAMDISK_CAPACITY = 2880 ; in sectors

iglobal
align 4
ramdisk_actual_size     dd      RAMDISK_CAPACITY
endg

; This function is called early in boot process.
; It creates filesystem /rd/1 based on raw image data loaded by somebody before
; to memory named as RAMDISK with max size RAMDISK_CAPACITY, may be less.
proc ramdisk_init
iglobal
ramdisk_name    db      'rd',0
endg
        push    ebx esi ; save used registers to be stdcall
; 1. Register the device and the (always inserted) media in the disk subsystem.
        stdcall disk_add, ramdisk_functions, ramdisk_name, 0, 0
        test    eax, eax
        jz      .fail
        mov     ebx, eax
        stdcall disk_media_changed, eax, 1
; 2. We don't know actual size of loaded image,
; so try to calculate it using partition structure,
; assuming that file systems fill the real size based on contents of the partition.
; 2a. Prepare for loop over partitions.
        xor     ecx, ecx
        xor     edx, edx
; 2b. Check that at least one partition was recognized.
        cmp     [ebx+DISK.NumPartitions], ecx
        jz      .fail
; 2c. Loop over partitions.
.partitions:
; For every partition, set edx to maximum between edx and end of partition.
        mov     esi, [ebx+DISK.Partitions]
        mov     esi, [esi+ecx*4]
        mov     eax, dword [esi+PARTITION.FirstSector]
        add     eax, dword [esi+PARTITION.Length]
        cmp     eax, edx
        jb      @f
        mov     edx, eax
@@:
        inc     ecx
        cmp     ecx, [ebx+DISK.NumPartitions]
        jb      .partitions
; 3. Reclaim unused memory, if any.
        mov     [ramdisk_actual_size], edx
        add     edx, 7  ; aligning up
        shr     edx, 3  ; 512-byte sectors -> 4096-byte pages
        mov     esi, RAMDISK_CAPACITY / 8       ; aligning down
        sub     esi, edx
        jbe     .no_reclaim
        shl     edx, 12
        add     edx, RAMDISK - OS_BASE
@@:
        mov     eax, edx
        call    free_page
        add     edx, 0x1000
        dec     esi
        jnz     @b
.no_reclaim:
        pop     esi ebx ; restore used registers to be stdcall
        ret
.fail:
        dbgstr 'Failed to initialize ramdisk'
        pop     esi ebx ; restore used registers to be stdcall
        ret
endp

; Returns information about disk media.
proc ramdisk_querymedia
  virtual at esp+4
    .userdata dd ?
    .info dd ?
  end virtual
; Media is always present, sector size is always 512 bytes.
        mov     edx, [.userdata]
        mov     ecx, [.info]
        mov     [ecx+DISKMEDIAINFO.Flags], 0
        mov     [ecx+DISKMEDIAINFO.SectorSize], 512
        mov     eax, [ramdisk_actual_size]
        mov     dword [ecx+DISKMEDIAINFO.Capacity], eax
        mov     dword [ecx+DISKMEDIAINFO.Capacity+4], 0
; Return zero as an indicator of success.
        xor     eax, eax
        retn    8
endp

; Common procedure for reading and writing.
; operation = 0 for reading, operation = 1 for writing.
; Arguments of ramdisk_read and ramdisk_write are the same.
macro ramdisk_read_write operation
{
        push    esi edi         ; save used registers to be stdcall
        mov     esi, [userdata]
        mov     edi, [numsectors_ptr]
; 1. Determine number of sectors to be transferred.
; This is either the requested number of sectors or number of sectors
; up to the disk boundary, depending of what is less.
        xor     ecx, ecx
; 1a. Test whether [start_sector] is less than RAMDISK_CAPACITY.
; If so, calculate number of sectors between [start_sector] and RAMDISK_CAPACITY.
; Otherwise, the actual number of sectors is zero.
        cmp     dword [start_sector+4], ecx
        jnz     .got_number
        mov     eax, [ramdisk_actual_size]
        sub     eax, dword [start_sector]
        jbe     .got_number
; 1b. Get the requested number of sectors.
        mov     ecx, [edi]
; 1c. If it is greater than number of sectors calculated in 1a, use the value
; from 1a.
        cmp     ecx, eax
        jb      .got_number
        mov     ecx, eax
.got_number:
; 2. Compare the actual number of sectors with requested. If they are
; equal, set eax (it will be the returned value) to zero. Otherwise,
; use DISK_STATUS_END_OF_MEDIA.
        xor     eax, eax
        cmp     ecx, [edi]
        jz      @f
        mov     al, DISK_STATUS_END_OF_MEDIA
@@:
; 3. Store the actual number of sectors.
        mov     [edi], ecx
; 4. Calculate source and destination addresses.
if operation = 0 ; reading?
        mov     esi, dword [start_sector]
        shl     esi, 9
        add     esi, RAMDISK
        mov     edi, [buffer]
else ; writing?
        mov     edi, dword [start_sector]
        shl     edi, 9
        add     edi, RAMDISK
        mov     esi, [buffer]
end if
; 5. Calculate number of dwords to be transferred.
        shl     ecx, 9-2
; 6. Copy data.
        rep movsd
; 7. Return. The value in eax was calculated in step 2.
        pop     edi esi         ; restore used registers to be stdcall
}

; Reads one or more sectors from the device.
proc ramdisk_read userdata:dword, buffer:dword, start_sector:qword, numsectors_ptr:dword
        ramdisk_read_write 0
        ret
endp

; Writes one or more sectors to the device.
proc ramdisk_write userdata:dword, buffer:dword, start_sector:qword, numsectors_ptr:dword
        ramdisk_read_write 1
        ret
endp

; The kernel calls this function when initializing cache subsystem for
; the media. This call allows the driver to adjust the cache size.
proc ramdisk_adjust_cache_size
  virtual at esp+4
    .userdata dd ?
    .suggested_size dd ?
  end virtual
; Since ramdisk does not need cache, just return 0.
        xor     eax, eax
        retn    8
endp

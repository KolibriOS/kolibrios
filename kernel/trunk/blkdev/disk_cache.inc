;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2011-2014. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

; Read/write functions try to do large operations,
; it is significantly faster than several small operations.
; This requires large buffers.
; We can't use input/output buffers directly - they can be controlled
; by user-mode application, so they can be modified between the operation
; and copying to/from cache, giving invalid data in cache.
; It is unclear how to use cache directly, currently cache items are
; allocated/freed sector-wise, so items for sequential sectors can be
; scattered over all the cache.
; So read/write functions allocate a temporary buffer which is
; 1) not greater than half of free memory and
; 2) not greater than the following constant.
CACHE_MAX_ALLOC_SIZE = 4 shl 20

; Legacy interface for filesystems fs_{read,write}32_{sys,app}
; gives only one sector for FS. However, per-sector reading is inefficient,
; so internally fs_read32_{sys,app} reads to the cache several sequential
; sectors, hoping that they will be useful.
; Total number of sectors is given by the following constant.
CACHE_LEGACY_READ_SIZE = 16

; This structure describes one item in the cache.
struct CACHE_ITEM
SectorLo        dd      ?       ; low 32 bits of sector
SectorHi        dd      ?       ; high 32 bits of sector
Status          dd      ?       ; one of CACHE_ITEM_*
ends

; Possible values for CACHE_ITEM_*
CACHE_ITEM_EMPTY = 0
CACHE_ITEM_COPY = 1
CACHE_ITEM_MODIFIED = 2

; Read several sequential sectors using cache #1.
; in: edx:eax = start sector, relative to start of partition
; in: ecx = number of sectors to read
; in: ebx -> buffer
; in: ebp -> PARTITION
; out: eax = error code, 0 = ok
; out: ecx = number of sectors that were read
fs_read64_sys:
; Save ebx, set ebx to SysCache and let the common part do its work.
        push    ebx
        mov     ebx, [ebp+PARTITION.Disk]
        add     ebx, DISK.SysCache
        jmp     fs_read64_common

; Read several sequential sectors using cache #2.
; in: edx:eax = start sector, relative to start of partition
; in: ecx = number of sectors to read
; in: edi -> buffer
; in: ebp -> PARTITION
; out: eax = error code, 0 = ok
; out: ecx = number of sectors that were read
fs_read64_app:
; Save ebx, set ebx to AppCache and let the common part do its work.
        push    ebx
        mov     ebx, [ebp+PARTITION.Disk]
        add     ebx, DISK.AppCache

; Common part of fs_read64_{app,sys}:
; read several sequential sectors using the given cache.
fs_read64_common:
; 1. Setup stack frame.
        push    esi edi         ; save used registers to be stdcall
        push    0               ; initialize .error_code
        push    ebx edx eax ecx ecx     ; initialize stack variables
virtual at esp
.local_vars:
.num_sectors_orig dd    ?
; Number of sectors that should be read. Used to generate output value of ecx.
.num_sectors    dd      ?
; Number of sectors that remain to be read. Decreases from .num_sectors_orig to 0.
.sector_lo      dd      ?       ; low 32 bits of the current sector
.sector_hi      dd      ?       ; high 32 bits of the current sector
.cache          dd      ?       ; pointer to DISKCACHE
.error_code     dd      ?       ; current status
.local_vars_size = $ - .local_vars
.saved_regs     rd      2
.buffer         dd      ?       ; filled by fs_read64_{sys,app}
end virtual
; 2. Validate parameters against partition length:
; immediately return error if edx:eax are beyond partition end,
; decrease .num_sectors and .num_sectors_orig, if needed,
; so that the entire operation fits in the partition limits.
        mov     eax, dword [ebp+PARTITION.Length]
        mov     edx, dword [ebp+PARTITION.Length+4]
        sub     eax, [.sector_lo]
        sbb     edx, [.sector_hi]
        jb      .end_of_media
        jnz     .no_end_of_media
        cmp     ecx, eax
        jbe     .no_end_of_media
; If .num_sectors got decreased, set status to DISK_STATUS_END_OF_MEDIA;
; if all subsequent operations would be successful, this would become the final
; status, otherwise this would be rewritten by failed operation.
        mov     [.num_sectors], eax
        mov     [.num_sectors_orig], eax
        mov     [.error_code], DISK_STATUS_END_OF_MEDIA
.no_end_of_media:
; 3. If number of sectors to read is zero, either because zero-sectors operation
; was requested or because it got decreased to zero due to partition limits,
; just return the current status.
        cmp     [.num_sectors], 0
        jz      .return
; 4. Shift sector from partition-relative to absolute.
        mov     eax, dword [ebp+PARTITION.FirstSector]
        mov     edx, dword [ebp+PARTITION.FirstSector+4]
        add     [.sector_lo], eax
        adc     [.sector_hi], edx
; 5. If the cache is disabled, pass the request directly to the driver.
        mov     edi, [.buffer]
        cmp     [ebx+DISKCACHE.pointer], 0
        jz      .nocache
; 6. Look for sectors in the cache, sequentially from the beginning.
; Stop at the first sector that is not in the cache
; or when all sectors were read from the cache.
; 6a. Acquire the lock.
        mov     ecx, [ebp+PARTITION.Disk]
        add     ecx, DISK.CacheLock
        call    mutex_lock
.lookup_in_cache_loop:
; 6b. For each sector, call the lookup function without adding to the cache.
        mov     eax, [.sector_lo]
        mov     edx, [.sector_hi]
        call    cache_lookup_read
; 6c. If it has failed, the sector is not in cache;
; release the lock and go to 7.
        jc      .not_found_in_cache
; The sector is found in cache.
; 6d. Copy data for the caller.
; Note that buffer in edi is advanced automatically.
        mov     esi, ecx
        shl     esi, 9
        add     esi, [ebx+DISKCACHE.data]
        mov     ecx, 512/4
        rep movsd
; 6e. Advance the sector.
        add     [.sector_lo], 1
        adc     [.sector_hi], 0
; 6f. Decrement number of sectors left.
; If all sectors were read, release the lock and return.
        dec     [.num_sectors]
        jnz     .lookup_in_cache_loop
; Release the lock acquired at 6a.
        mov     ecx, [ebp+PARTITION.Disk]
        add     ecx, DISK.CacheLock
        call    mutex_unlock
.return:
        mov     eax, [.error_code]
        mov     ecx, [.num_sectors_orig]
        sub     ecx, [.num_sectors]
.nothing:
        add     esp, .local_vars_size
        pop     edi esi ebx     ; restore used registers to be stdcall
        ret
.not_found_in_cache:
; Release the lock acquired at 6a.
        mov     ecx, [ebp+PARTITION.Disk]
        add     ecx, DISK.CacheLock
        call    mutex_unlock
; The current sector is not present in the cache.
; Ask the driver to read all requested not-yet-read sectors,
; put results in the cache.
; Also, see the comment before the definition of CACHE_MAX_ALLOC_SIZE.
; 7. Allocate buffer for operations.
; Normally, create buffer that is sufficient for all remaining data.
; However, for extra-large requests make an upper limit:
; do not use more than half of the free memory
; or more than CACHE_MAX_ALLOC_SIZE bytes.
        mov     ebx, [pg_data.pages_free]
        shr     ebx, 1
        jz      .nomemory
        cmp     ebx, CACHE_MAX_ALLOC_SIZE shr 12
        jbe     @f
        mov     ebx, CACHE_MAX_ALLOC_SIZE shr 12
@@:
        shl     ebx, 12 - 9
        cmp     ebx, [.num_sectors]
        jbe     @f
        mov     ebx, [.num_sectors]
@@:
        mov     eax, ebx
        shl     eax, 9
        stdcall kernel_alloc, eax
; If failed, return the appropriate error code.
        test    eax, eax
        jz      .nomemory
        mov     esi, eax
; Split the request to chunks that fit in the allocated buffer.
.read_loop:
; 8. Get iteration size: either size of allocated buffer in sectors
; or number of sectors left, select what is smaller.
        cmp     ebx, [.num_sectors]
        jbe     @f
        mov     ebx, [.num_sectors]
@@:
; 9. Create second portion of local variables.
; Note that variables here and above are esp-relative;
; it means that all addresses should be corrected when esp is changing.
        push    ebx esi esi
        push    ebx
; In particular, num_sectors is now [.num_sectors+.local_vars2_size].
virtual at esp
.local_vars2:
.current_num_sectors    dd      ?       ; number of sectors that were read
.current_buffer         dd      ?
; pointer inside .allocated_buffer that points
; to the beginning of not-processed data
.allocated_buffer       dd      ?       ; saved in safe place
.iteration_size         dd      ?       ; saved in safe place
.local_vars2_size = $ - .local_vars2
end virtual
; 10. Call the driver, reading the next chunk.
        push    esp     ; numsectors
        push    [.sector_hi+.local_vars2_size+4] ; startsector
        push    [.sector_lo+.local_vars2_size+8] ; startsector
        push    esi     ; buffer
        mov     esi, [ebp+PARTITION.Disk]
        mov     al, DISKFUNC.read
        call    disk_call_driver
; If failed, save error code.
        test    eax, eax
        jz      @f
        mov     [.error_code+.local_vars2_size], eax
@@:
; 11. Copy data for the caller.
; Note that buffer in edi is advanced automatically.
        cmp     [.current_num_sectors], 0
        jz      .copy_done
        mov     ecx, [.current_num_sectors]
        shl     ecx, 9-2
        mov     esi, [.allocated_buffer]
        rep movsd
; 12. Copy data to the cache.
; 12a. Acquire the lock.
        mov     ebx, [.cache+.local_vars2_size]
        mov     ecx, [ebp+PARTITION.Disk]
        add     ecx, DISK.CacheLock
        call    mutex_lock
; 12b. Prepare for the loop: save edi and create a local variable that
; stores number of sectors to be copied.
        push    edi
        push    [.current_num_sectors+4]
.store_to_cache:
; 12c. For each sector, call the lookup function with adding to the cache, if not yet.
        mov     eax, [.sector_lo+.local_vars2_size+8]
        mov     edx, [.sector_hi+.local_vars2_size+8]
        call    cache_lookup_write
        test    eax, eax
        jnz     .cache_error
; 12d. If the sector was already present in the cache as modified,
; data that were read at step 10 for this sector are obsolete,
; so rewrite data for the caller from the cache.
        cmp     [esi+CACHE_ITEM.Status], CACHE_ITEM_MODIFIED
        jnz     .not_modified
        mov     esi, ecx
        shl     esi, 9
        add     esi, [ebx+DISKCACHE.data]
        mov     edi, [esp+4]
        mov     ecx, [esp]
        shl     ecx, 9-2
        sub     edi, ecx
        mov     ecx, 512/4
        rep movsd
        add     [.current_buffer+8], 512
        jmp     .sector_done
.not_modified:
; 12e. For each not-modified sector,
; copy data, mark the item as not-modified copy of the disk,
; advance .current_buffer and .sector_hi:.sector_lo to the next sector.
        mov     [esi+CACHE_ITEM.Status], CACHE_ITEM_COPY
        mov     esi, [.current_buffer+8]
        mov     edi, ecx
        shl     edi, 9
        add     edi, [ebx+DISKCACHE.data]
        mov     ecx, 512/4
        rep movsd
        mov     [.current_buffer+8], esi
.sector_done:
        add     [.sector_lo+.local_vars2_size+8], 1
        adc     [.sector_hi+.local_vars2_size+8], 0
; 12f. Continue the loop 12c-12e until all sectors are read.
        dec     dword [esp]
        jnz     .store_to_cache
.cache_error:
; 12g. Restore after the loop: pop the local variable and restore edi.
        pop     ecx
        pop     edi
; 12h. Release the lock.
        mov     ecx, [ebp+PARTITION.Disk]
        add     ecx, DISK.CacheLock
        call    mutex_unlock
.copy_done:
; 13. Remove portion of local variables created at step 9.
        pop     ecx
        pop     esi esi ebx
; 14. Continue iterations while number of sectors read by the driver
; is equal to number of sectors requested and there are additional sectors.
        cmp     ecx, ebx
        jnz     @f
        sub     [.num_sectors], ebx
        jnz     .read_loop
@@:
; 15. Free the buffer allocated at step 7 and return.
        stdcall kernel_free, esi
        jmp     .return

; Special branches:
.nomemory:
; memory allocation failed at step 7: return the corresponding error
        mov     [.error_code], DISK_STATUS_NO_MEMORY
        jmp     .return
.nocache:
; step 5, after correcting number of sectors to fit in partition limits
; and advancing partition-relative sector to absolute,
; sees that cache is disabled: pass corrected request to the driver
        lea     eax, [.num_sectors]
        push    eax             ; numsectors
        push    [.sector_hi+4]  ; startsector
        push    [.sector_lo+8]  ; startsector
        push    edi     ; buffer
        mov     esi, [ebp+PARTITION.Disk]
        mov     al, DISKFUNC.read
        call    disk_call_driver
        test    eax, eax
        jnz     @f
        mov     eax, [.error_code]
@@:
        mov     ecx, [.num_sectors]
        jmp     .nothing
.end_of_media:
; requested sector is beyond the partition end: return the corresponding error
        mov     [.error_code], DISK_STATUS_END_OF_MEDIA
        jmp     .return

; Write several sequential sectors using cache #1.
; in: edx:eax = start sector
; in: ecx = number of sectors to write
; in: ebx -> buffer
; in: ebp -> PARTITION
; out: eax = error code, 0 = ok
; out: ecx = number of sectors that were written
fs_write64_sys:
; Save ebx, set ebx to SysCache and let the common part do its work.
        push    ebx
        mov     ebx, [ebp+PARTITION.Disk]
        add     ebx, DISK.SysCache
        jmp     fs_write64_common

; Write several sequential sectors using cache #2.
; in: edx:eax = start sector
; in: ecx = number of sectors to write
; in: ebx -> buffer
; in: ebp -> PARTITION
; out: eax = error code, 0 = ok
; out: ecx = number of sectors that were written
fs_write64_app:
; Save ebx, set ebx to AppCache and let the common part do its work.
        push    ebx
        mov     ebx, [ebp+PARTITION.Disk]
        add     ebx, DISK.AppCache

; Common part of fs_write64_{app,sys}:
; write several sequential sectors using the given cache.
fs_write64_common:
; 1. Setup stack frame.
        push    esi edi         ; save used registers to be stdcall
        push    0               ; initialize .error_code
        push    edx eax ecx ecx ; initialize stack variables
        push    [.buffer-4]     ; copy [.buffer] to [.cur_buffer]
                                ; -4 is due to esp-relative addressing
virtual at esp
.local_vars:
.cur_buffer     dd      ?       ; pointer to data that are currently copying
.num_sectors_orig dd    ?
; Number of sectors that should be written. Used to generate output value of ecx.
.num_sectors    dd      ?
; Number of sectors that remain to be written.
.sector_lo      dd      ?       ; low 32 bits of the current sector
.sector_hi      dd      ?       ; high 32 bits of the current sector
.error_code     dd      ?       ; current status
.local_vars_size = $ - .local_vars
.saved_regs     rd      2
.buffer         dd      ?       ; filled by fs_write64_{sys,app}
end virtual
; 2. Validate parameters against partition length:
; immediately return error if edx:eax are beyond partition end,
; decrease .num_sectors and .num_sectors_orig, if needed,
; so that the entire operation fits in the partition limits.
        mov     eax, dword [ebp+PARTITION.Length]
        mov     edx, dword [ebp+PARTITION.Length+4]
        sub     eax, [.sector_lo]
        sbb     edx, [.sector_hi]
        jb      .end_of_media
        jnz     .no_end_of_media
        cmp     ecx, eax
        jbe     .no_end_of_media
; If .num_sectors got decreased, set status to DISK_STATUS_END_OF_MEDIA;
; if all subsequent operations would be successful, this would become the final
; status, otherwise this would be rewritten by failed operation.
        mov     [.num_sectors], eax
        mov     [.num_sectors_orig], eax
        mov     [.error_code], DISK_STATUS_END_OF_MEDIA
.no_end_of_media:
; 3. If number of sectors to write is zero, either because zero-sectors operation
; was requested or because it got decreased to zero due to partition limits,
; just return the current status.
        cmp     [.num_sectors], 0
        jz      .return
; 4. Shift sector from partition-relative to absolute.
        mov     eax, dword [ebp+PARTITION.FirstSector]
        mov     edx, dword [ebp+PARTITION.FirstSector+4]
        add     [.sector_lo], eax
        adc     [.sector_hi], edx
; 5. If the cache is disabled, pass the request directly to the driver.
        cmp     [ebx+DISKCACHE.pointer], 0
        jz      .nocache
; 6. Store sectors in the cache, sequentially from the beginning.
; 6a. Acquire the lock.
        mov     ecx, [ebp+PARTITION.Disk]
        add     ecx, DISK.CacheLock
        call    mutex_lock
.lookup_in_cache_loop:
; 6b. For each sector, call the lookup function with adding to the cache, if not yet.
        mov     eax, [.sector_lo]
        mov     edx, [.sector_hi]
        call    cache_lookup_write
        test    eax, eax
        jnz     .cache_error
; 6c. For each sector, copy data, mark the item as modified and not saved,
; advance .current_buffer to the next sector.
        mov     [esi+CACHE_ITEM.Status], CACHE_ITEM_MODIFIED
        mov     esi, [.cur_buffer]
        mov     edi, ecx
        shl     edi, 9
        add     edi, [ebx+DISKCACHE.data]
        mov     ecx, 512/4
        rep movsd
        mov     [.cur_buffer], esi
; 6d. Remove the sector from the other cache.
; Normally it should not be there, but prefetching could put to the app cache
; data that normally should belong to the sys cache and vice versa.
; Note: this requires that both caches must be protected by the same lock.
        mov     eax, [.sector_lo]
        mov     edx, [.sector_hi]
        push    ebx
        sub     ebx, [ebp+PARTITION.Disk]
        xor     ebx, DISK.SysCache xor DISK.AppCache
        add     ebx, [ebp+PARTITION.Disk]
        call    cache_lookup_read
        jc      @f
        mov     [esi+CACHE_ITEM.Status], CACHE_ITEM_EMPTY
@@:
        pop     ebx
; 6e. Advance .sector_hi:.sector_lo to the next sector.
        add     [.sector_lo], 1
        adc     [.sector_hi], 0
; 6f. Continue the loop at 6b-6e until all sectors are processed.
        dec     [.num_sectors]
        jnz     .lookup_in_cache_loop
.unlock_return:
; 6g. Release the lock and return.
        mov     ecx, [ebp+PARTITION.Disk]
        add     ecx, DISK.CacheLock
        call    mutex_unlock
.return:
        mov     eax, [.error_code]
        mov     ecx, [.num_sectors_orig]
        sub     ecx, [.num_sectors]
.nothing:
        add     esp, .local_vars_size
        pop     edi esi ebx
        ret

; Special branches:
.cache_error:
; error at flushing the cache while adding sector to the cache:
; return the error from the lookup function
        mov     [.error_code], eax
        jmp     .unlock_return
.end_of_media:
; requested sector is beyond the partition end: return the corresponding error
        mov     eax, DISK_STATUS_END_OF_MEDIA
        xor     ecx, ecx
        jmp     .nothing
.nocache:
; step 5, after correcting number of sectors to fit in partition limits
; and advancing partition-relative sector to absolute,
; sees that cache is disabled: pass corrected request to the driver
        lea     eax, [.num_sectors]
        push    eax             ; numsectors
        push    [.sector_hi+4]  ; startsector
        push    [.sector_lo+8]  ; startsector
        push    [.buffer+12]    ; buffer
        mov     esi, [ebp+PARTITION.Disk]
        mov     al, DISKFUNC.write
        call    disk_call_driver
        mov     ecx, [.num_sectors]
        jmp     .nothing

; Legacy. Use fs_read64_sys instead.
; This function is intended to replace the old 'hd_read' function when
; [hdd_appl_data] = 0, so its input/output parameters are the same, except
; that it can't use the global variables 'hd_error' and 'hdd_appl_data'.
; in: eax = sector, ebx = buffer, ebp = pointer to PARTITION structure
; eax is relative to partition start
; out: eax = error code; 0 = ok
fs_read32_sys:
; Save ebx, set ebx to SysCache and let the common part do its work.
        push    ebx
        mov     ebx, [ebp+PARTITION.Disk]
        add     ebx, DISK.SysCache
        jmp     fs_read32_common

; Legacy. Use fs_read64_app instead.
; This function is intended to replace the old 'hd_read' function when
; [hdd_appl_data] = 1, so its input/output parameters are the same, except
; that it can't use the global variables 'hd_error' and 'hdd_appl_data'.
; in: eax = sector, ebx = buffer, ebp = pointer to PARTITION structure
; eax is relative to partition start
; out: eax = error code; 0 = ok
fs_read32_app:
; Save ebx, set ebx to AppCache and let the common part do its work.
        push    ebx
        mov     ebx, [ebp+PARTITION.Disk]
        add     ebx, DISK.AppCache

; This label is the common part of fs_read32_sys and fs_read32_app.
fs_read32_common:
; 1. Check that the required sector is inside the partition. If no, return
; DISK_STATUS_END_OF_MEDIA.
        cmp     dword [ebp+PARTITION.Length+4], 0
        jnz     @f
        cmp     dword [ebp+PARTITION.Length], eax
        ja      @f
        mov     eax, DISK_STATUS_END_OF_MEDIA
        pop     ebx
        ret
@@:
; 2. Get the absolute sector on the disk.
        push    ecx edx esi edi
        xor     edx, edx
        add     eax, dword [ebp+PARTITION.FirstSector]
        adc     edx, dword [ebp+PARTITION.FirstSector+4]
; 3. If there is no cache for this disk, just pass the request to the driver.
        cmp     [ebx+DISKCACHE.pointer], 0
        jnz     .scancache
        push    1
        push    esp     ; numsectors
        push    edx     ; startsector
        push    eax     ; startsector
        pushd   [esp+32]; buffer
        mov     esi, [ebp+PARTITION.Disk]
        mov     al, DISKFUNC.read
        call    disk_call_driver
        pop     ecx
        pop     edi esi edx ecx
        pop     ebx
        ret
.scancache:
        push    ebx edx eax
virtual at esp
.local_vars:
.sector_lo      dd      ?
.sector_hi      dd      ?
.cache          dd      ?
.local_vars_size = $ - .local_vars
.saved_regs     rd      4
.buffer         dd      ?
end virtual
; 4. Scan for the requested sector in the cache.
; If found, copy the data and return.
; 4a. Acquire the lock.
        mov     ecx, [ebp+PARTITION.Disk]
        add     ecx, DISK.CacheLock
        call    mutex_lock
; 4b. Call the lookup function without adding to the cache.
        mov     eax, [.sector_lo]
        mov     edx, [.sector_hi]
        call    cache_lookup_read
; If not found, go to 5.
        jc      .not_found_in_cache
.found_in_cache:
; 4c. Copy the data.
        mov     edi, [.buffer]
        mov     esi, ecx
        shl     esi, 9
        add     esi, [ebx+DISKCACHE.data]
        mov     ecx, 512/4
        rep movsd
; 4d. Release the lock and return success.
        mov     ecx, [ebp+PARTITION.Disk]
        add     ecx, DISK.CacheLock
        call    mutex_unlock
.return:
        xor     eax, eax
.return_eax:
        add     esp, .local_vars_size
        pop     edi esi edx ecx
        pop     ebx
        ret
.not_found_in_cache:
; 5. Decide whether we need to prefetch further sectors.
; If so, advance to 6. If not, go to 13.
; Assume that devices < 3MB are floppies which are slow
; (ramdisk does not have a cache, so we don't even get here for ramdisk).
; This is a dirty hack, but the entire function is somewhat hacky. Use fs_read64*.
        mov     eax, [ebp+PARTITION.Disk]
        cmp     dword [eax+DISK.MediaInfo.Capacity+4], 0
        jnz     @f
        cmp     dword [eax+DISK.MediaInfo.Capacity], 3 shl (20-9)
        jb      .floppy
@@:
; We want to prefetch CACHE_LEGACY_READ_SIZE sectors.
; 6. Release the lock acquired at step 4a.
        mov     ecx, [ebp+PARTITION.Disk]
        add     ecx, DISK.CacheLock
        call    mutex_unlock
; 7. Allocate buffer for CACHE_LEGACY_READ_SIZE sectors.
        stdcall kernel_alloc, CACHE_LEGACY_READ_SIZE shl 9
; If failed, return the corresponding error code.
        test    eax, eax
        jz      .nomemory
; 8. Create second portion of local variables.
        push    eax eax
        push    CACHE_LEGACY_READ_SIZE
virtual at esp
.local_vars2:
.num_sectors            dd      ?       ; number of sectors left
.current_buffer         dd      ?       ; pointer to data that are currently copying
.allocated_buffer       dd      ?       ; saved at safe place
.local_vars2_size = $ - .local_vars2
end virtual
; 9. Call the driver to read CACHE_LEGACY_READ_SIZE sectors.
        push    esp     ; numsectors
        push    [.sector_hi+.local_vars2_size+4]        ; startsector
        push    [.sector_lo+.local_vars2_size+8]        ; startsector
        push    eax     ; buffer
        mov     esi, [ebp+PARTITION.Disk]
        mov     al, DISKFUNC.read
        call    disk_call_driver
; Note: we're ok if at least one sector is read,
; read error somewhere after that just limits data to be put in cache.
        cmp     [.num_sectors], 0
        jz      .read_error
; 10. Copy data for the caller.
        mov     esi, [.allocated_buffer]
        mov     edi, [.buffer+.local_vars2_size]
        mov     ecx, 512/4
        rep movsd
; 11. Store all sectors that were successfully read to the cache.
; 11a. Acquire the lock.
        mov     ecx, [ebp+PARTITION.Disk]
        add     ecx, DISK.CacheLock
        call    mutex_lock
.store_to_cache:
; 11b. For each sector, call the lookup function with adding to the cache, if not yet.
        mov     eax, [.sector_lo+.local_vars2_size]
        mov     edx, [.sector_hi+.local_vars2_size]
        call    cache_lookup_write
        test    eax, eax
        jnz     .cache_error
; 11c. Ignore sectors marked as modified: for them the cache is more recent that disk data.
        cmp     [esi+CACHE_ITEM.Status], CACHE_ITEM_MODIFIED
        jnz     .not_modified
        add     [.current_buffer], 512
        jmp     .sector_done
.not_modified:
; 11d. For each sector, copy data, mark the item as not-modified copy of the disk,
; advance .current_buffer and .sector_hi:.sector_lo to the next sector.
        mov     [esi+CACHE_ITEM.Status], CACHE_ITEM_COPY
        mov     esi, [.current_buffer]
        mov     edi, ecx
        shl     edi, 9
        add     edi, [ebx+DISKCACHE.data]
        mov     ecx, 512/4
        rep movsd
        mov     [.current_buffer], esi
.sector_done:
        add     [.sector_lo+.local_vars2_size], 1
        adc     [.sector_hi+.local_vars2_size], 0
; 11e. Continue the loop at 11b-11d until all sectors are processed.
        dec     [.num_sectors]
        jnz     .store_to_cache
.cache_error:
; 11f. Release the lock.
        mov     ecx, [ebp+PARTITION.Disk]
        add     ecx, DISK.CacheLock
        call    mutex_unlock
.copy_done:
; 12. Remove portion of local variables created at step 8,
; free the buffer allocated at step 7 and return.
        pop     ecx ecx
        stdcall kernel_free
        jmp     .return
.read_error:
; If no sectors were read, free the buffer allocated at step 7
; and pass the error to the caller.
        push    eax
        stdcall kernel_free, [.allocated_buffer+4]
        pop     eax
        add     esp, .local_vars2_size
        jmp     .return_eax
.nomemory:
        mov     eax, DISK_STATUS_NO_MEMORY
        jmp     .return_eax
.floppy:
; We don't want to prefetch anything, just read one sector.
; We are still holding the lock acquired at step 4a.
; 13. Call the lookup function adding sector to the cache.
        call    cache_lookup_write
        test    eax, eax
        jnz     .floppy_cache_error
        push    ecx

; 14. Call the driver to read one sector.
        push    1
        push    esp
        push    edx
        push    [.sector_lo+16]
        shl     ecx, 9
        add     ecx, [ebx+DISKCACHE.data]
        push    ecx
        mov     esi, [ebp+PARTITION.Disk]
        mov     al, DISKFUNC.read
        call    disk_call_driver
        pop     ecx
        dec     ecx
        jnz     .floppy_read_error
; 15. Get the slot and pointer to the cache item,
; change the status to not-modified copy of the disk
; and go to 4c.
        pop     ecx
        lea     esi, [ecx*sizeof.CACHE_ITEM/4]
        shl     esi, 2
        add     esi, [ebx+DISKCACHE.pointer]
        mov     [esi+CACHE_ITEM.Status], CACHE_ITEM_COPY
        jmp     .found_in_cache

; On error at steps 13-14, release the lock
; and pass the error to the caller.
.floppy_read_error:
        pop     ecx
.floppy_cache_error:
        mov     ecx, [ebp+PARTITION.Disk]
        add     ecx, DISK.CacheLock
        push    eax
        call    mutex_unlock
        pop     eax
        jmp     .return_eax

; This function is intended to replace the old 'hd_write' function when
; [hdd_appl_data] = 0, so its input/output parameters are the same, except
; that it can't use the global variables 'hd_error' and 'hdd_appl_data'.
; in: eax = sector, ebx = buffer, ebp = pointer to PARTITION structure
; eax is relative to partition start
; out: eax = error code; 0 = ok
fs_write32_sys:
; Just call the advanced function.
        push    ecx edx
        xor     edx, edx
        mov     ecx, 1
        call    fs_write64_sys
        pop     edx ecx
        ret

; This function is intended to replace the old 'hd_write' function when
; [hdd_appl_data] = 1, so its input/output parameters are the same, except
; that it can't use the global variables 'hd_error' and 'hdd_appl_data'.
; in: eax = sector, ebx = buffer, ebp = pointer to PARTITION structure
; eax is relative to partition start
; out: eax = error code; 0 = ok
fs_write32_app:
; Just call the advanced function.
        push    ecx edx
        xor     edx, edx
        mov     ecx, 1
        call    fs_write64_app
        pop     edx ecx
        ret

; Lookup for the given sector in the given cache.
; If the sector is not present, return error.
; The caller must acquire the cache lock.
; in: edx:eax = sector
; in: ebx -> DISKCACHE structure
; out: CF set if sector is not in cache
; out: ecx = index in cache
; out: esi -> sector:status
proc cache_lookup_read
        mov     esi, [ebx+DISKCACHE.pointer]
        add     esi, sizeof.CACHE_ITEM

        mov     ecx, 1

.hdreadcache:

        cmp     [esi+CACHE_ITEM.Status], CACHE_ITEM_EMPTY
        je      .nohdcache

        cmp     [esi+CACHE_ITEM.SectorLo], eax
        jne     .nohdcache
        cmp     [esi+CACHE_ITEM.SectorHi], edx
        jne     .nohdcache
        clc
        ret

.nohdcache:

        add     esi, sizeof.CACHE_ITEM
        inc     ecx
        cmp     ecx, [ebx+DISKCACHE.sad_size]
        jbe     .hdreadcache
        stc
        ret
endp

; Lookup for the given sector in the given cache.
; If the sector is not present, allocate space for it,
; possibly flushing data.
; in: edx:eax = sector
; in: ebx -> DISKCACHE structure
; in: ebp -> PARTITION structure
; out: eax = error code
; out: ecx = index in cache
; out: esi -> sector:status
proc cache_lookup_write
        call    cache_lookup_read
        jnc     .return0
        push    edx eax
;-----------------------------------------------------------
; find empty or read slot, flush cache if next 12.5% is used by write
; output : ecx = cache slot
;-----------------------------------------------------------
; Note: the code is essentially inherited, so probably
; no analysis of efficiency were done.
; However, it works.
.search_again:
        mov     eax, [ebx+DISKCACHE.sad_size]
        mov     ecx, [ebx+DISKCACHE.search_start]
        shr     eax, 3
        lea     esi, [ecx*sizeof.CACHE_ITEM/4]
        shl     esi, 2
        add     esi, [ebx+DISKCACHE.pointer]
.search_for_empty:
        inc     ecx
        add     esi, sizeof.CACHE_ITEM
        cmp     ecx, [ebx+DISKCACHE.sad_size]
        jbe     .inside_cache
        mov     ecx, 1
        mov     esi, [ebx+DISKCACHE.pointer]
        add     esi, sizeof.CACHE_ITEM
.inside_cache:
        cmp     [esi+CACHE_ITEM.Status], CACHE_ITEM_MODIFIED
        jb      .found_slot             ; it's empty or read
        dec     eax
        jnz     .search_for_empty
        stdcall write_cache64, [ebp+PARTITION.Disk] ; no empty slots found, write all
        test    eax, eax
        jne     .found_slot_access_denied
        jmp     .search_again           ; and start again
.found_slot:
        mov     [ebx+DISKCACHE.search_start], ecx
        popd    [esi+CACHE_ITEM.SectorLo]
        popd    [esi+CACHE_ITEM.SectorHi]
        mov     [esi+CACHE_ITEM.Status], CACHE_ITEM_EMPTY
.return0:
        xor     eax, eax        ; success
        ret
.found_slot_access_denied:
        add     esp, 8
        ret
endp

; Flush the given cache.
; The caller must acquire the cache lock.
; in: ebx -> DISKCACHE
; in: first argument in stdcall convention -> PARTITION
proc write_cache64
; 1. Setup stack frame.
        push    esi edi         ; save used registers to be stdcall
        sub     esp, .local_vars_size   ; reserve space for local vars
virtual at esp
.local_vars:
.cache_end      dd      ?       ; item past the end of the cache
.size_left      dd      ?       ; items left to scan
.current_ptr    dd      ?       ; pointer to the current item
;
; Write operations are coalesced in chains,
; one chain describes a sequential interval of sectors,
; they can be sequential or scattered in the cache.
.sequential     dd      ?
; boolean variable, 1 if the current chain is sequential in the cache,
; 0 if additional buffer is needed to perform the operation
.chain_start_pos dd     ?       ; slot of chain start item
.chain_start_ptr dd     ?       ; pointer to chain start item
.chain_size     dd      ?       ; chain size (thanks, C.O.)
.iteration_size dd      ?
; If the chain size is too large, split the operation to several iterations.
; This is size in sectors for one iterations.
.iteration_buffer dd    ?       ; temporary buffer for non-sequential chains
.local_vars_size = $ - .local_vars
                rd      2       ; saved registers
                dd      ?       ; return address
.disk           dd      ?       ; first argument
end virtual
; 1. If there is no cache for this disk, nothing to do, just return zero.
        cmp     [ebx+DISKCACHE.pointer], 0
        jz      .return0
; 2. Prepare for the loop: initialize current pointer and .size_left,
; calculate .cache_end.
        mov     ecx, [ebx+DISKCACHE.sad_size]
        mov     [.size_left], ecx
        lea     ecx, [ecx*sizeof.CACHE_ITEM/4]
        shl     ecx, 2
        mov     esi, [ebx+DISKCACHE.pointer]
        add     esi, sizeof.CACHE_ITEM
        add     ecx, esi
        mov     [.cache_end], ecx
; 3. Main loop: go over all items, go to 5 for every modified item.
.look:
        cmp     [esi+CACHE_ITEM.Status], CACHE_ITEM_MODIFIED
        jz      .begin_write
.look_next:
        add     esi, sizeof.CACHE_ITEM
        dec     [.size_left]
        jnz     .look
; 4. Return success.
.return0:
        xor     eax, eax
.return:
        add     esp, .local_vars_size
        pop     edi esi         ; restore used registers to be stdcall
        ret     4               ; return popping one argument
.begin_write:
; We have found a modified item.
; 5. Prepare for chain finding: save the current item, initialize chain variables.
        mov     [.current_ptr], esi
; Initialize chain as sequential zero-length starting at the current item.
        mov     [.chain_start_ptr], esi
        mov     eax, [ebx+DISKCACHE.sad_size]
        sub     eax, [.size_left]
        inc     eax
        mov     [.chain_start_pos], eax
        mov     [.chain_size], 0
        mov     [.sequential], 1
; 6. Expand the chain backward.
; Note: the main loop in step 2 looks for items sequentially,
; so the previous item is not modified. If the previous sector
; is present in the cache, it automatically makes the chain scattered.
; 6a. Calculate sector number: one before the sector for the current item.
        mov     eax, [esi+CACHE_ITEM.SectorLo]
        mov     edx, [esi+CACHE_ITEM.SectorHi]
        sub     eax, 1
        sbb     edx, 0
.find_chain_start:
; 6b. For each sector where the previous item does not expand the chain,
; call the lookup function without adding to the cache.
        call    cache_lookup_read
; 6c. If the sector is not found in cache or is not modified, stop expanding
; and advance to step 7.
        jc      .found_chain_start
        cmp     [esi+CACHE_ITEM.Status], CACHE_ITEM_MODIFIED
        jnz     .found_chain_start
; 6d. We have found a new block that expands the chain backwards.
; It makes the chain non-sequential.
; Normally, sectors come in sequential blocks, so try to look at previous items
; before returning to 6b; if there is a sequential block indeed, this saves some
; time instead of many full-fledged lookups.
        mov     [.sequential], 0
        mov     [.chain_start_pos], ecx
.look_backward:
; 6e. For each sector, update chain start pos/ptr, decrement sector number,
; look at the previous item.
        mov     [.chain_start_ptr], esi
        inc     [.chain_size]
        sub     eax, 1
        sbb     edx, 0
        sub     esi, sizeof.CACHE_ITEM
; If the previous item exists...
        cmp     esi, [ebx+DISKCACHE.pointer]
        jbe     .find_chain_start
; ...describes the correct sector...
        cmp     [esi+CACHE_ITEM.SectorLo], eax
        jnz     .find_chain_start
        cmp     [esi+CACHE_ITEM.SectorHi], edx
        jnz     .find_chain_start
; ...and is modified...
        cmp     [esi+CACHE_ITEM.Status], CACHE_ITEM_MODIFIED
        jnz     .found_chain_start
; ...expand the chain one sector backwards and continue the loop at 6e.
; Otherwise, advance to step 7 if the previous item describes the correct sector
; but is not modified, and return to step 6b otherwise.
        dec     [.chain_start_pos]
        jmp     .look_backward
.found_chain_start:
; 7. Expand the chain forward.
; 7a. Prepare for the loop at 7b:
; set esi = pointer to current item, edx:eax = current sector.
        mov     esi, [.current_ptr]
        mov     eax, [esi+CACHE_ITEM.SectorLo]
        mov     edx, [esi+CACHE_ITEM.SectorHi]
.look_forward:
; 7b. First, look at the next item. If it describes the next sector:
; if it is modified, expand the chain with that sector and continue this step,
; if it is not modified, the chain is completed, so advance to step 8.
        inc     [.chain_size]
        add     eax, 1
        adc     edx, 0
        add     esi, sizeof.CACHE_ITEM
        cmp     esi, [.cache_end]
        jae     .find_chain_end
        cmp     [esi+CACHE_ITEM.SectorLo], eax
        jnz     .find_chain_end
        cmp     [esi+CACHE_ITEM.SectorHi], edx
        jnz     .find_chain_end
        cmp     [esi+CACHE_ITEM.Status], CACHE_ITEM_MODIFIED
        jnz     .found_chain_end
        jmp     .look_forward
.find_chain_end:
; 7c. Otherwise, call the lookup function.
        call    cache_lookup_read
; 7d. If the next sector is present in the cache and is modified,
; mark the chain as non-sequential and continue to step 7b.
        jc      .found_chain_end
        cmp     [esi+CACHE_ITEM.Status], CACHE_ITEM_MODIFIED
        jnz     .found_chain_end
        mov     [.sequential], 0
        jmp     .look_forward
.found_chain_end:
; 8. Decide whether the chain is sequential or scattered.
; Advance to step 9 for sequential chains, go to step 10 for scattered chains.
        cmp     [.sequential], 0
        jz      .write_non_sequential
.write_sequential:
; 9. Write a sequential chain to disk.
; 9a. Pass the entire chain to the driver.
        mov     eax, [.chain_start_ptr]
        mov     edx, [.chain_start_pos]
        shl     edx, 9
        add     edx, [ebx+DISKCACHE.data]
        lea     ecx, [.chain_size]
        push    ecx     ; numsectors
        pushd   [eax+CACHE_ITEM.SectorHi]       ; startsector
        pushd   [eax+CACHE_ITEM.SectorLo]       ; startsector
        push    edx     ; buffer
        mov     esi, [ebp+PARTITION.Disk]
        mov     al, DISKFUNC.write
        call    disk_call_driver
; 9b. If failed, pass the error code to the driver.
        test    eax, eax
        jnz     .return
; 9c. If succeeded, mark all sectors in the chain as not-modified,
; advance current item and number of items left to skip the chain.
        mov     esi, [.current_ptr]
        mov     eax, [.chain_size]
        sub     [.size_left], eax
@@:
        mov     [esi+CACHE_ITEM.Status], CACHE_ITEM_COPY
        add     esi, sizeof.CACHE_ITEM
        dec     eax
        jnz     @b
; 9d. Continue the main loop at step 2 if there are more sectors.
; Return success otherwise.
        cmp     [.size_left], 0
        jnz     .look
        jmp     .return0
.write_non_sequential:
; Write a non-sequential chain to the disk.
; 10. Allocate a temporary buffer.
; Use [.chain_size] sectors, but
; not greater than CACHE_MAX_ALLOC_SIZE bytes
; and not greater than half of free memory.
        mov     eax, [pg_data.pages_free]
        shr     eax, 1
        jz      .nomemory
        cmp     eax, CACHE_MAX_ALLOC_SIZE shr 12
        jbe     @f
        mov     eax, CACHE_MAX_ALLOC_SIZE shr 12
@@:
        shl     eax, 12 - 9
        cmp     eax, [.chain_size]
        jbe     @f
        mov     eax, [.chain_size]
@@:
        mov     [.iteration_size], eax
        shl     eax, 9
        stdcall kernel_alloc, eax
        test    eax, eax
        jz      .nomemory
        mov     [.iteration_buffer], eax
.write_non_sequential_iteration:
; 11. Split the chain so that each iteration fits in the allocated buffer.
; Iteration size is the minimum of chain size and allocated size.
        mov     eax, [.chain_size]
        cmp     eax, [.iteration_size]
        jae     @f
        mov     [.iteration_size], eax
@@:
; 12. Prepare arguments for the driver.
        mov     esi, [.chain_start_ptr]
        mov     edi, [.iteration_buffer]
        push    [.iteration_size]
        push    esp     ; numsectors
        push    [esi+CACHE_ITEM.SectorHi]       ; startsector
        push    [esi+CACHE_ITEM.SectorLo]       ; startsector
        push    edi     ; buffer
; 13. Copy data from the cache to the temporary buffer,
; advancing chain_start pos/ptr and marking sectors as not-modified.
; 13a. Prepare for the loop: push number of sectors to process.
        push    [.iteration_size+20]    ; temporary variable
.copy_loop:
; 13b. For each sector, copy the data.
; Note that edi is advanced automatically.
        mov     esi, [.chain_start_pos+24]
        shl     esi, 9
        add     esi, [ebx+DISKCACHE.data]
        mov     ecx, 512/4
        rep movsd
; 13c. Mark the item as not-modified.
        mov     esi, [.chain_start_ptr+24]
        mov     [esi+CACHE_ITEM.Status], CACHE_ITEM_COPY
; 13d. Check whether the next sector continues the chain.
; If so, advance to 13e. Otherwise, go to 13f.
        mov     eax, [esi+CACHE_ITEM.SectorLo]
        mov     edx, [esi+CACHE_ITEM.SectorHi]
        add     esi, sizeof.CACHE_ITEM
        add     eax, 1
        adc     edx, 0
        cmp     esi, [.cache_end+24]
        jae     .no_forward
        cmp     [esi+CACHE_ITEM.SectorLo], eax
        jnz     .no_forward
        cmp     [esi+CACHE_ITEM.SectorHi], edx
        jnz     .no_forward
; 13e. Increment position/pointer to the chain and
; continue the loop.
        inc     [.chain_start_pos+24]
        mov     [.chain_start_ptr+24], esi
        dec     dword [esp]
        jnz     .copy_loop
        jmp     .copy_done
.no_forward:
; 13f. Call the lookup function without adding to the cache.
; Update position/pointer with returned value.
; Note: for the last sector in the chain, ecx/esi may contain
; garbage; we are not going to use them in this case.
        call    cache_lookup_read
        mov     [.chain_start_pos+24], ecx
        mov     [.chain_start_ptr+24], esi
        dec     dword [esp]
        jnz     .copy_loop
.copy_done:
; 13g. Restore the stack after 13a.
        pop     ecx
; 14. Call the driver.
        mov     esi, [ebp+PARTITION.Disk]
        mov     al, DISKFUNC.write
        call    disk_call_driver
        pop     ecx     ; numsectors
; 15. If the driver has returned an error, free the buffer allocated at step 10
; and pass the error to the caller.
; Otherwise, remove the processed part from the chain and continue iterations
; starting in step 11 if there are more data to process.
        test    eax, eax
        jnz     .nonsequential_error
        sub     [.chain_size], ecx
        jnz     .write_non_sequential_iteration
; 16. The chain is written. Free the temporary buffer
; and continue the loop at step 2.
        stdcall kernel_free, [.iteration_buffer]
        mov     esi, [.current_ptr]
        jmp     .look_next
.nonsequential_error:
        push    eax
        stdcall kernel_free, [.iteration_buffer+4]
        pop     eax
        jmp     .return
.nomemory:
        mov     eax, DISK_STATUS_NO_MEMORY
        jmp     .return
endp

; This internal function is called from disk_add to initialize the caching for
; a new DISK.
; The algorithm is inherited from getcache.inc: take 1/32 part of the available
; physical memory, round down to 8 pages, limit by 128K from below and by 1M
; from above. Reserve 1/8 part of the cache for system data and 7/8 for app
; data.
; After the size is calculated, but before the cache is allocated, the device
; driver can adjust the size. In particular, setting size to zero disables
; caching: there is no sense in a cache for a ramdisk. In fact, such action
; is most useful example of a non-trivial adjustment.
; esi = pointer to DISK structure
disk_init_cache:
; 1. Calculate the suggested cache size.
; 1a. Get the size of free physical memory in pages.
        mov     eax, [pg_data.pages_free]
; 1b. Use the value to calculate the size.
        shl     eax, 12 - 5     ; 1/32 of it in bytes
        and     eax, -8*4096    ; round down to the multiple of 8 pages
; 1c. Force lower and upper limits.
        cmp     eax, 1024*1024
        jb      @f
        mov     eax, 1024*1024
@@:
        cmp     eax, 128*1024
        ja      @f
        mov     eax, 128*1024
@@:
; 1d. Give a chance to the driver to adjust the size.
        push    eax
        mov     al, DISKFUNC.adjust_cache_size
        call    disk_call_driver
; Cache size calculated.
        mov     [esi+DISK.cache_size], eax
        test    eax, eax
        jz      .nocache
; 2. Allocate memory for the cache.
; 2a. Call the allocator.
        stdcall kernel_alloc, eax
        test    eax, eax
        jnz     @f
; 2b. If it failed, say a message and return with eax = 0.
        dbgstr 'no memory for disk cache'
        jmp     .nothing
@@:
; 3. Fill two DISKCACHE structures.
        mov     [esi+DISK.SysCache.pointer], eax
        lea     ecx, [esi+DISK.CacheLock]
        call    mutex_init
; The following code is inherited from getcache.inc.
        mov     edx, [esi+DISK.SysCache.pointer]
        and     [esi+DISK.SysCache.search_start], 0
        and     [esi+DISK.AppCache.search_start], 0
        mov     eax, [esi+DISK.cache_size]
        shr     eax, 3
        mov     [esi+DISK.SysCache.data_size], eax
        add     edx, eax
        imul    eax, 7
        mov     [esi+DISK.AppCache.data_size], eax
        mov     [esi+DISK.AppCache.pointer], edx

        mov     eax, [esi+DISK.SysCache.data_size]
        push    ebx
        call    calculate_for_hd64
        pop     ebx
        add     eax, [esi+DISK.SysCache.pointer]
        mov     [esi+DISK.SysCache.data], eax
        mov     [esi+DISK.SysCache.sad_size], ecx

        push    edi
        mov     edi, [esi+DISK.SysCache.pointer]
        lea     ecx, [(ecx+1)*3]
        xor     eax, eax
        rep stosd
        pop     edi

        mov     eax, [esi+DISK.AppCache.data_size]
        push    ebx
        call    calculate_for_hd64
        pop     ebx
        add     eax, [esi+DISK.AppCache.pointer]
        mov     [esi+DISK.AppCache.data], eax
        mov     [esi+DISK.AppCache.sad_size], ecx

        push    edi
        mov     edi, [esi+DISK.AppCache.pointer]
        lea     ecx, [(ecx+1)*3]
        xor     eax, eax
        rep stosd
        pop     edi

; 4. Return with nonzero al.
        mov     al, 1
; 5. Return.
.nothing:
        ret
; No caching is required for this driver. Zero cache pointers and return with
; nonzero al.
.nocache:
        mov     [esi+DISK.SysCache.pointer], eax
        mov     [esi+DISK.AppCache.pointer], eax
        mov     al, 1
        ret

calculate_for_hd64:
        push    eax
        mov     ebx, eax
        shr     eax, 9
        lea     eax, [eax*3]
        shl     eax, 2
        sub     ebx, eax
        shr     ebx, 9
        mov     ecx, ebx
        shl     ebx, 9
        pop     eax
        sub     eax, ebx
        dec     ecx
        ret


; This internal function is called from disk_media_dereference to free the
; allocated cache, if there is one.
; esi = pointer to DISK structure
disk_free_cache:
; The algorithm is straightforward.
        mov     eax, [esi+DISK.SysCache.pointer]
        test    eax, eax
        jz      .nothing
        stdcall kernel_free, eax
.nothing:
        ret

; This function flushes all modified data from both caches for the given DISK.
; esi = pointer to DISK
disk_sync:
; The algorithm is straightforward.
        cmp     [esi+DISK.SysCache.pointer], 0
        jz      .nothing
        lea     ecx, [esi+DISK.CacheLock]
        call    mutex_lock
        push    ebx
        push    esi     ; for second write_cache64
        push    esi     ; for first write_cache64
        lea     ebx, [esi+DISK.SysCache]
        call    write_cache64
        add     ebx, DISK.AppCache - DISK.SysCache
        call    write_cache64
        pop     ebx
        lea     ecx, [esi+DISK.CacheLock]
        call    mutex_unlock
.nothing:
        mov     al, DISKFUNC.flush
        call    disk_call_driver
        ret

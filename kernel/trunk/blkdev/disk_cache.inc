;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2011-2012. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

; This function is intended to replace the old 'hd_read' function when
; [hdd_appl_data] = 0, so its input/output parameters are the same, except
; that it can't use the global variables 'hd_error' and 'hdd_appl_data'.
; in: eax = sector, ebx = buffer, ebp = pointer to PARTITION structure
; eax is relative to partition start
; out: eax = error code; 0 = ok
fs_read32_sys:
; Compatibility hack: if PARTITION.Disk is 'old', there is no DISK structure,
; this request should be processed by hd_read.
        cmp     [ebp+PARTITION.Disk], 'old'
        jnz     @f
        add     eax, dword [ebp+PARTITION.FirstSector]
        mov     [hdd_appl_data], 0
        call    hd_read
        mov     [hdd_appl_data], 1      ; restore to default state
        mov     eax, [hd_error]
        ret
@@:
; In the normal case, save ecx, set ecx to SysCache and let the common part
; do its work.
        push    ecx
        mov     ecx, [ebp+PARTITION.Disk]
        add     ecx, DISK.SysCache
        jmp     fs_read32_common

; This function is intended to replace the old 'hd_read' function when
; [hdd_appl_data] = 1, so its input/output parameters are the same, except
; that it can't use the global variables 'hd_error' and 'hdd_appl_data'.
; in: eax = sector, ebx = buffer, ebp = pointer to PARTITION structure
; eax is relative to partition start
; out: eax = error code; 0 = ok
fs_read32_app:
; Compatibility hack: if PARTITION.Disk is 'old', there is no DISK structure,
; this request should be processed by hd_read.
        cmp     [ebp+PARTITION.Disk], 'old'
        jnz     @f
        add     eax, dword [ebp+PARTITION.FirstSector]
        mov     [hdd_appl_data], 1
        call    hd_read
        mov     eax, [hd_error]
        ret
@@:
; In the normal case, save ecx, set ecx to AppCache and let the common part
; do its work.
        push    ecx
        mov     ecx, [ebp+PARTITION.Disk]
        add     ecx, DISK.AppCache

; This label is the common part of fs_read32_sys and fs_read32_app.
fs_read32_common:
; 1. Check that the required sector is inside the partition. If no, return
; DISK_STATUS_END_OF_MEDIA.
        cmp     dword [ebp+PARTITION.Length+4], 0
        jnz     @f
        cmp     dword [ebp+PARTITION.Length], eax
        ja      @f
        mov     eax, DISK_STATUS_END_OF_MEDIA
        pop     ecx
        ret
@@:
; 2. Get the absolute sector on the disk.
        push    edx esi
        xor     edx, edx
        add     eax, dword [ebp+PARTITION.FirstSector]
        adc     edx, dword [ebp+PARTITION.FirstSector+4]
; 3. If there is no cache for this disk, just pass the request to the driver.
        cmp     [ecx+DISKCACHE.pointer], 0
        jnz     .scancache
        push    1
        push    esp     ; numsectors
        push    edx     ; startsector
        push    eax     ; startsector
        push    ebx     ; buffer
        mov     esi, [ebp+PARTITION.Disk]
        mov     al, DISKFUNC.read
        call    disk_call_driver
        pop     ecx
        pop     esi edx
        pop     ecx
        ret
.scancache:
; 4. Scan the cache.
        push    edi ecx ; scan cache
        push    edx eax
virtual at esp
.sector_lo      dd      ?
.sector_hi      dd      ?
.cache          dd      ?
end virtual
; The following code is inherited from hd_read. The differences are:
; all code is protected by the cache lock; instead of static calls
; to hd_read_dma/hd_read_pio/bd_read the dynamic call to DISKFUNC.read is used;
; sector is 64-bit, not 32-bit.
        call    mutex_lock
        mov     eax, [.sector_lo]
        mov     edx, [.sector_hi]
        mov     esi, [ecx+DISKCACHE.pointer]
        mov     ecx, [ecx+DISKCACHE.sad_size]
        add     esi, 12

        mov     edi, 1

.hdreadcache:

        cmp     dword [esi+8], 0        ; empty
        je      .nohdcache

        cmp     [esi], eax      ; correct sector
        jne     .nohdcache
        cmp     [esi+4], edx    ; correct sector
        je      .yeshdcache

.nohdcache:

        add     esi, 12
        inc     edi
        dec     ecx
        jnz     .hdreadcache

        mov     esi, [.cache]
        call    find_empty_slot64       ; ret in edi
        test    eax, eax
        jnz     .read_done

        push    1
        push    esp
        push    edx
        push    [.sector_lo+12]
        mov     ecx, [.cache+16]
        mov     eax, edi
        shl     eax, 9
        add     eax, [ecx+DISKCACHE.data]
        push    eax
        mov     esi, [ebp+PARTITION.Disk]
        mov     al, DISKFUNC.read
        call    disk_call_driver
        pop     ecx
        dec     ecx
        jnz     .read_done

        mov     ecx, [.cache]
        lea     eax, [edi*3]
        mov     esi, [ecx+DISKCACHE.pointer]
        lea     esi, [eax*4+esi]

        mov     eax, [.sector_lo]
        mov     edx, [.sector_hi]
        mov     [esi], eax      ; sector number
        mov     [esi+4], edx    ; sector number
        mov     dword [esi+8], 1; hd read - mark as same as in hd

.yeshdcache:

        mov     esi, edi
        mov     ecx, [.cache]
        shl     esi, 9
        add     esi, [ecx+DISKCACHE.data]

        mov     edi, ebx
        mov     ecx, 512/4
        rep movsd               ; move data
        xor     eax, eax        ; successful read
.read_done:
        mov     ecx, [.cache]
        push    eax
        call    mutex_unlock
        pop     eax
        add     esp, 12
        pop     edi esi edx ecx
        ret

; This function is intended to replace the old 'hd_write' function when
; [hdd_appl_data] = 0, so its input/output parameters are the same, except
; that it can't use the global variables 'hd_error' and 'hdd_appl_data'.
; in: eax = sector, ebx = buffer, ebp = pointer to PARTITION structure
; eax is relative to partition start
; out: eax = error code; 0 = ok
fs_write32_sys:
; Compatibility hack: if PARTITION.Disk is 'old', there is no DISK structure,
; this request should be processed by hd_write.
        cmp     [ebp+PARTITION.Disk], 'old'
        jnz     @f
        add     eax, dword [ebp+PARTITION.FirstSector]
        mov     [hdd_appl_data], 0
        call    hd_write
        mov     [hdd_appl_data], 1      ; restore to default state
        mov     eax, [hd_error]
        ret
@@:
; In the normal case, save ecx, set ecx to SysCache and let the common part
; do its work.
        push    ecx
        mov     ecx, [ebp+PARTITION.Disk]
        add     ecx, DISK.SysCache
        jmp     fs_write32_common

; This function is intended to replace the old 'hd_write' function when
; [hdd_appl_data] = 1, so its input/output parameters are the same, except
; that it can't use the global variables 'hd_error' and 'hdd_appl_data'.
; in: eax = sector, ebx = buffer, ebp = pointer to PARTITION structure
; eax is relative to partition start
; out: eax = error code; 0 = ok
fs_write32_app:
; Compatibility hack: if PARTITION.Disk is 'old', there is no DISK structure,
; this request should be processed by hd_write.
        cmp     [ebp+PARTITION.Disk], 'old'
        jnz     @f
        add     eax, dword [ebp+PARTITION.FirstSector]
        mov     [hdd_appl_data], 1
        call    hd_write
        mov     eax, [hd_error]
        ret
@@:
; In the normal case, save ecx, set ecx to AppCache and let the common part
; do its work.
        push    ecx
        mov     ecx, [ebp+PARTITION.Disk]
        add     ecx, DISK.AppCache

; This label is the common part of fs_read32_sys and fs_read32_app.
fs_write32_common:
; 1. Check that the required sector is inside the partition. If no, return
; DISK_STATUS_END_OF_MEDIA.
        cmp     dword [ebp+PARTITION.Length+4], 0
        jnz     @f
        cmp     dword [ebp+PARTITION.Length], eax
        ja      @f
        mov     eax, DISK_STATUS_END_OF_MEDIA
        pop     ecx
        ret
@@:
        push    edx esi
; 2. Get the absolute sector on the disk.
        xor     edx, edx
        add     eax, dword [ebp+PARTITION.FirstSector]
        adc     edx, dword [ebp+PARTITION.FirstSector+4]
; 3. If there is no cache for this disk, just pass request to the driver.
        cmp     [ecx+DISKCACHE.pointer], 0
        jnz     .scancache
        push    1
        push    esp     ; numsectors
        push    edx     ; startsector
        push    eax     ; startsector
        push    ebx     ; buffer
        mov     esi, [ebp+PARTITION.Disk]
        mov     al, DISKFUNC.write
        call    disk_call_driver
        pop     ecx
        pop     esi edx
        pop     ecx
        ret
.scancache:
; 4. Scan the cache.
        push    edi ecx ; scan cache
        push    edx eax
virtual at esp
.sector_lo      dd      ?
.sector_hi      dd      ?
.cache          dd      ?
end virtual
; The following code is inherited from hd_write. The differences are:
; all code is protected by the cache lock;
; sector is 64-bit, not 32-bit.
        call    mutex_lock

        ; check if the cache already has the sector and overwrite it
        mov     eax, [.sector_lo]
        mov     edx, [.sector_hi]
        mov     esi, [ecx+DISKCACHE.pointer]
        mov     ecx, [ecx+DISKCACHE.sad_size]
        add     esi, 12

        mov     edi, 1

.hdwritecache:
        cmp     dword [esi+8], 0        ; if cache slot is empty
        je      .not_in_cache_write

        cmp     [esi], eax      ; if the slot has the sector
        jne     .not_in_cache_write
        cmp     [esi+4], edx    ; if the slot has the sector
        je      .yes_in_cache_write

.not_in_cache_write:

        add     esi, 12
        inc     edi
        dec     ecx
        jnz     .hdwritecache

        ; sector not found in cache
        ; write the block to a new location

        mov     esi, [.cache]
        call    find_empty_slot64       ; ret in edi
        test    eax, eax
        jne     .hd_write_access_denied

        mov     ecx, [.cache]
        lea     eax, [edi*3]
        mov     esi, [ecx+DISKCACHE.pointer]
        lea     esi, [eax*4+esi]

        mov     eax, [.sector_lo]
        mov     edx, [.sector_hi]
        mov     [esi], eax      ; sector number
        mov     [esi+4], edx    ; sector number

.yes_in_cache_write:

        mov     dword [esi+8], 2        ; write - differs from hd

        shl     edi, 9
        mov     ecx, [.cache]
        add     edi, [ecx+DISKCACHE.data]

        mov     esi, ebx
        mov     ecx, 512/4
        rep movsd               ; move data
        xor     eax, eax        ; success
.hd_write_access_denied:
        mov     ecx, [.cache]
        push    eax
        call    mutex_unlock
        pop     eax
        add     esp, 12
        pop     edi esi edx ecx
        ret

; This internal function is called from fs_read32_* and fs_write32_*. It is the
; analogue of find_empty_slot for 64-bit sectors.
find_empty_slot64:
;-----------------------------------------------------------
; find empty or read slot, flush cache if next 12.5% is used by write
; output : edi = cache slot
;-----------------------------------------------------------
.search_again:
        mov     ecx, [esi+DISKCACHE.sad_size]
        mov     edi, [esi+DISKCACHE.search_start]
        shr     ecx, 3
.search_for_empty:
        inc     edi
        cmp     edi, [esi+DISKCACHE.sad_size]
        jbe     .inside_cache
        mov     edi, 1
.inside_cache:
        lea     eax, [edi*3]
        shl     eax, 2
        add     eax, [esi+DISKCACHE.pointer]
        cmp     dword [eax+8], 2
        jb      .found_slot             ; it's empty or read
        dec     ecx
        jnz     .search_for_empty
        stdcall write_cache64, [ebp+PARTITION.Disk] ; no empty slots found, write all
        test    eax, eax
        jne     .found_slot_access_denied
        jmp     .search_again           ; and start again
.found_slot:
        mov     [esi+DISKCACHE.search_start], edi
        xor     eax, eax        ; success
.found_slot_access_denied:
        ret

; This function is intended to replace the old 'write_cache' function.
proc write_cache64 uses ecx edx esi edi, disk:dword
locals
cache_chain_started     dd      0
cache_chain_size        dd      ?
cache_chain_pos         dd      ?
cache_chain_ptr         dd      ?
endl
saved_esi_pos = 16+12 ; size of local variables + size of registers before esi
; If there is no cache for this disk, nothing to do.
        cmp     [esi+DISKCACHE.pointer], 0
        jz      .flush
;-----------------------------------------------------------
; write all changed sectors to disk
;-----------------------------------------------------------

        ; write difference ( 2 ) from cache to DISK
        mov     ecx, [esi+DISKCACHE.sad_size]
        mov     esi, [esi+DISKCACHE.pointer]
        add     esi, 12
        mov     edi, 1
.write_cache_more:
        cmp     dword [esi+8], 2        ; if cache slot is not different
        jne     .write_chain
        mov     dword [esi+8], 1        ; same as in hd
        mov     eax, [esi]
        mov     edx, [esi+4]            ; edx:eax = sector to write
; Объединяем запись цепочки последовательных секторов в одно обращение к диску
        cmp     ecx, 1
        jz      .nonext
        cmp     dword [esi+12+8], 2
        jnz     .nonext
        push    eax edx
        add     eax, 1
        adc     edx, 0
        cmp     eax, [esi+12]
        jnz     @f
        cmp     edx, [esi+12+4]
@@:
        pop     edx eax
        jnz     .nonext
        cmp     [cache_chain_started], 1
        jz      @f
        mov     [cache_chain_started], 1
        mov     [cache_chain_size], 0
        mov     [cache_chain_pos], edi
        mov     [cache_chain_ptr], esi
@@:
        inc     [cache_chain_size]
        cmp     [cache_chain_size], 16
        jnz     .continue
        jmp     .write_chain
.nonext:
        call    .flush_cache_chain
        test    eax, eax
        jnz     .nothing
        mov     [cache_chain_size], 1
        mov     [cache_chain_ptr], esi
        call    .write_cache_sector
        test    eax, eax
        jnz     .nothing
        jmp     .continue
.write_chain:
        call    .flush_cache_chain
        test    eax, eax
        jnz     .nothing
.continue:
        add     esi, 12
        inc     edi
        dec     ecx
        jnz     .write_cache_more
        call    .flush_cache_chain
        test    eax, eax
        jnz     .nothing
.flush:
        mov     esi, [disk]
        mov     al, DISKFUNC.flush
        call    disk_call_driver
.nothing:
        ret

.flush_cache_chain:
        xor     eax, eax
        cmp     [cache_chain_started], eax
        jz      @f
        call    .write_cache_chain
        mov     [cache_chain_started], 0
@@:
        retn

.write_cache_sector:
        mov     [cache_chain_size], 1
        mov     [cache_chain_pos], edi
.write_cache_chain:
        pusha
        mov     edi, [cache_chain_pos]
        mov     ecx, [ebp-saved_esi_pos]
        shl     edi, 9
        add     edi, [ecx+DISKCACHE.data]
        mov     ecx, [cache_chain_size]
        push    ecx
        push    esp     ; numsectors
        mov     eax, [cache_chain_ptr]
        pushd   [eax+4]
        pushd   [eax]   ; startsector
        push    edi     ; buffer
        mov     esi, [ebp]
        mov     esi, [esi+PARTITION.Disk]
        mov     al, DISKFUNC.write
        call    disk_call_driver
        pop     ecx
        mov     [esp+28], eax
        popa
        retn
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
        lea     ecx, [esi+DISK.SysCache.mutex]
        call    mutex_init
        lea     ecx, [esi+DISK.AppCache.mutex]
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
        lea     ecx, [ecx*3]
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
        lea     ecx, [ecx*3]
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
; Compatibility hack: if PARTITION.Disk is 'old', there is no DISK structure,
; this request should be processed by write_cache.
        cmp     esi, 'old'
        jnz     @f
        mov     [hdd_appl_data], 0
        call    write_cache
        mov     [hdd_appl_data], 1
        call    write_cache
        mov     eax, [hd_error]
        ret
@@:
; The algorithm is straightforward.
        push    esi
        push    esi     ; for second write_cache64
        push    esi     ; for first write_cache64
        add     esi, DISK.SysCache
        call    write_cache64
        add     esi, DISK.AppCache - DISK.SysCache
        call    write_cache64
        pop     esi
        ret

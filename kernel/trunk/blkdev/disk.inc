;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2011-2012. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

; =============================================================================
; ================================= Constants =================================
; =============================================================================
; Error codes for callback functions.
DISK_STATUS_OK              = 0 ; success
DISK_STATUS_GENERAL_ERROR   = -1; if no other code is suitable
DISK_STATUS_INVALID_CALL    = 1 ; invalid input parameters
DISK_STATUS_NO_MEDIA        = 2 ; no media present
DISK_STATUS_END_OF_MEDIA    = 3 ; end of media while reading/writing data
; Driver flags. Represent bits in DISK.DriverFlags.
DISK_NO_INSERT_NOTIFICATION = 1
; Media flags. Represent bits in DISKMEDIAINFO.Flags.
DISK_MEDIA_READONLY = 1

; If too many partitions are detected,there is probably an error on the disk.
; 256 partitions should be enough for any reasonable use.
; Also, the same number is limiting the number of MBRs to process; if
; too many MBRs are visible,there probably is a loop in the MBR structure.
MAX_NUM_PARTITIONS = 256

; =============================================================================
; ================================ Structures =================================
; =============================================================================
; This structure defines all callback functions for working with the physical
; device. They are implemented by a driver. Objects with this structure reside
; in a driver.
struct  DISKFUNC
        strucsize       dd ?
; Size of the structure. This field is intended for possible extensions of
; this structure. If a new function is added to this structure and a driver
; implements an old version, the caller can detect this by checking .strucsize,
; so the driver remains compatible.
        close           dd ?
; The pointer to the function which frees all driver-specific resources for
; the disk.
; Optional, may be NULL.
; void close(void* userdata);
        closemedia      dd ?
; The pointer to the function which informs the driver that the kernel has
; finished all processing with the current media. If media is removed, the
; driver should decline all requests to that media with DISK_STATUS_NO_MEDIA,
; even if new media is inserted, until this function is called. If media is
; removed, a new call to 'disk_media_changed' is not allowed until this
; function is called.
; Optional, may be NULL (if media is not removable).
; void closemedia(void* userdata);
        querymedia      dd ?
; The pointer to the function which determines capabilities of the media.
; int querymedia(void* userdata, DISKMEDIAINFO* info);
; Return value: one of DISK_STATUS_*
        read            dd ?
; The pointer to the function which reads data from the device.
; int read(void* userdata, void* buffer, __int64 startsector, int* numsectors);
; input: *numsectors = number of sectors to read
; output: *numsectors = number of sectors which were successfully read
; Return value: one of DISK_STATUS_*
        write           dd ?
; The pointer to the function which writes data to the device.
; Optional, may be NULL.
; int write(void* userdata, void* buffer, __int64 startsector, int* numsectors);
; input: *numsectors = number of sectors to write
; output: *numsectors = number of sectors which were successfully written
; Return value: one of DISK_STATUS_*
        flush           dd ?
; The pointer to the function which flushes the internal device cache.
; Optional, may be NULL.
; int flush(void* userdata);
; Return value: one of DISK_STATUS_*
; Note that read/write are called by the cache manager, so a driver should not
; create a software cache. This function is implemented for flushing a hardware
; cache, if it exists.
        adjust_cache_size       dd ?
; The pointer to the function which returns the cache size for this device.
; Optional, may be NULL.
; unsigned int adjust_cache_size(unsigned int suggested_size);
; Return value: 0 = disable cache, otherwise = used cache size in bytes.
ends

; This structure holds information on a medium.
; Objects with this structure are allocated by the kernel as a part of the DISK
; structure and are filled by a driver in the 'querymedia' callback.
struct  DISKMEDIAINFO
        Flags           dd ?
; Combination of DISK_MEDIA_* bits.
        SectorSize      dd ?
; Size of the sector.
        Capacity        dq ?
; Size of the media in sectors.
ends

; This structure represents the disk cache. To follow the old implementation,
; there are two distinct caches for a disk, one for "system" data,and the other
; for "application" data.
struct  DISKCACHE
        mutex           MUTEX
; Lock to protect the cache.
; The following fields are inherited from data32.inc:cache_ideX.
        pointer         dd ?
        data_size       dd ?    ; unused
        data            dd ?
        sad_size        dd ?
        search_start    dd ?
ends

; This structure represents a disk device and its media for the kernel.
; This structure is allocated by the kernel in the 'disk_add' function,
; freed in the 'disk_dereference' function.
struct  DISK
; Fields of disk object
        Next            dd ?
        Prev            dd ?
; All disk devices are linked in one list with these two fields.
; Head of the list is the 'disk_list' variable.
        Functions       dd ?
; Pointer to the 'DISKFUNC' structure with driver functions.
        Name            dd ?
; Pointer to the string used for accesses through the global filesystem.
        UserData        dd ?
; This field is passed to all callback functions so a driver can decide which
; physical device is addressed.
        DriverFlags     dd ?
; Bitfield. Currently only DISK_NO_INSERT_NOTIFICATION bit is defined.
; If it is set, the driver will never issue 'disk_media_changed' notification
; with argument set to true, so the kernel must try to detect media during
; requests from the file system.
        RefCount        dd ?
; Count of active references to this structure. One reference is kept during
; the lifetime of the structure between 'disk_add' and 'disk_del'.
; Another reference is taken during any filesystem operation for this disk.
; One reference is added if media is inserted.
; The structure is destroyed when the reference count decrements to zero:
; this usually occurs in 'disk_del', but can be delayed to the end of last
; filesystem operation, if one is active.
        MediaLock       MUTEX
; Lock to protect the MEDIA structure. See the description after
; 'disk_list_mutex' for the locking strategy.
; Fields of media object
        MediaInserted   db ?
; 0 if media is not inserted, nonzero otherwise.
        MediaUsed       db ?
; 0 if media fields are not used, nonzero otherwise. If .MediaRefCount is
; nonzero, this field is nonzero too; however, when .MediaRefCount goes
; to zero, there is some time interval during which media object is still used.
                        dw ? ; padding
; The following fields are not valid unless either .MediaInserted is nonzero
; or they are accessed from a code which has obtained the reference when
; .MediaInserted was nonzero.
        MediaRefCount   dd ?
; Count of active references to the media object. One reference is kept during
; the lifetime of the media between two calls to 'disk_media_changed'.
; Another reference is taken during any filesystem operation for this media.
; The callback 'closemedia' is called when the reference count decrements to
; zero: this usually occurs in 'disk_media_changed', but can be delayed to the
; end of the last filesystem operation, if one is active.
        MediaInfo       DISKMEDIAINFO
; This field keeps information on the current media.
        NumPartitions   dd ?
; Number of partitions on this media.
        Partitions      dd ?
; Pointer to array of .NumPartitions pointers to PARTITION structures.
        cache_size      dd ?
; inherited from cache_ideX_size
        SysCache        DISKCACHE
        AppCache        DISKCACHE
; Two caches for the disk.
ends

; This structure represents one partition for the kernel. This is a base
; template, the actual contents after common fields is determined by the
; file system code for this partition.
struct  PARTITION
        FirstSector     dq ?
; First sector of the partition.
        Length          dq ?
; Length of the partition in sectors.
        Disk            dd ?
; Pointer to parent DISK structure.
        FSUserFunctions dd ?
; Handlers for the sysfunction 70h. This field is a pointer to the following
; array. The first dword is a number of supported subfunctions, other dwords
; point to handlers of corresponding subfunctions.
; This field is 0 if file system is not recognized.
; ...fs-specific data may follow...
ends

; This is an external structure, it represents an entry in the partition table.
struct  PARTITION_TABLE_ENTRY
        Bootable        db ?
; 80h = bootable partition, 0 = non-bootable partition, other values = invalid
        FirstHead       db ?
        FirstSector     db ?
        FirstTrack      db ?
; Coordinates of first sector in CHS.
        Type            db ?
; Partition type, one of predefined constants. 0 = empty, several types denote
; extended partition (see process_partition_table_entry), we are not interested
; in other values.
        LastHead        db ?
        LastSector      db ?
        LastTrack       db ?
; Coordinates of last sector in CHS.
        FirstAbsSector  dd ?
; Coordinate of first sector in LBA.
        Length          dd ?
; Length of the partition in sectors.
ends

; =============================================================================
; ================================ Global data ================================
; =============================================================================
iglobal
; The pseudo-item for the list of all DISK structures.
; Initialized to the empty list.
disk_list:
        dd      disk_list
        dd      disk_list
endg
uglobal
; This mutex guards all operations with the global list of DISK structures.
disk_list_mutex MUTEX
; * There are two dependent objects, a disk and a media. In the simplest case,
;   disk and media are both non-removable. However, in the general case both
;   can be removed at any time, simultaneously or only media,and this makes things
;   complicated.
; * For efficiency, both disk and media objects are located in the one
;   structure named DISK. However, logically they are different.
; * The following operations use data of disk object: adding (disk_add);
;   deleting (disk_del); filesystem (fs_lfn which eventually calls
;   dyndisk_handler or dyndisk_enum_root).
; * The following operations use data of media object: adding/removing
;   (disk_media_changed); filesystem (fs_lfn which eventually calls
;   dyndisk_handler; dyndisk_enum_root doesn't work with media).
; * Notifications disk_add, disk_media_changed, disk_del are synchronized
;   between themselves, this is a requirement for the driver. However, file
;   system operations are asynchronous, can be issued at any time by any
;   thread.
; * We must prevent a situation when a filesystem operation thinks that the
;   object is still valid but in fact the notification has destroyed the
;   object. So we keep a reference counter for both disk and media and destroy
;   the object when this counter goes to zero.
; * The driver must know when it is safe to free driver-allocated resources.
;   The object can be alive even after death notification has completed.
;   We use special callbacks to satisfy both assertions: 'close' for the disk
;   and 'closemedia' for the media. The destruction of the object includes
;   calling the corresponding callback.
; * Each filesystem operation keeps one reference for the disk and one
;   reference for the media. Notification disk_del forces notification on the
;   media death, so the reference counter for the disk is always not less than
;   the reference counter for the media.
; * Two operations "get the object" and "increment the reference counter" can
;   not be done simultaneously. We use a mutex to guard the consistency here.
;   It must be a part of the container for the object, so that this mutex can
;   be acquired as a part of getting the object from the container. The
;   container for disk object is the global list, and this list is guarded by
;   'disk_list_mutex'. The container for media object is the disk object, and
;   the corresponding mutex is DISK.MediaLock.
; * Notifications do not change the data of objects, they can only remove
;   objects. Thus we don't need another synchronization at this level. If two
;   filesystem operations are referencing the same filesystem data, this is
;   better resolved at the level of the filesystem.
endg

iglobal
; The function 'disk_scan_partitions' needs three 512-byte buffers for
; MBR, bootsector and fs-temporary sector data. It can not use the static
; buffers always, since it can be called for two or more disks in parallel.
; However, this case is not typical. We reserve three static 512-byte buffers
; and a flag that these buffers are currently used. If 'disk_scan_partitions'
; detects that the buffers are currently used, it allocates buffers from the
; heap.
; The flag is implemented as a global dword variable. When the static buffers
; are not used, the value is -1. When the static buffers are used, the value
; is normally 0 and temporarily can become greater. The function increments
; this value. If the resulting value is zero, it uses the buffers and
; decrements the value when the job is done. Otherwise, it immediately
; decrements the value and uses buffers from the heap, allocated in the
; beginning and freed in the end.
partition_buffer_users  dd      -1
endg
uglobal
; The static buffers for MBR, bootsector and fs-temporary sector data.
align 16
mbr_buffer      rb      512
bootsect_buffer rb      512
fs_tmp_buffer   rb      512
endg

iglobal
; This is the array of default implementations of driver callbacks.
; Same as DRIVERFUNC structure except for the first field; all functions must
; have the default implementations.
align 4
disk_default_callbacks:
        dd      disk_default_close
        dd      disk_default_closemedia
        dd      disk_default_querymedia
        dd      disk_default_read
        dd      disk_default_write
        dd      disk_default_flush
        dd      disk_default_adjust_cache_size
endg

; =============================================================================
; ================================= Functions =================================
; =============================================================================

; This function registers a disk device.
; This includes:
; - allocating an internal structure describing this device;
; - registering this structure in the global filesystem.
; The function initializes the disk as if there is no media. If a media is
; present, the function 'disk_media_changed' should be called after this
; function succeeds.
; Parameters:
; [esp+4] = pointer to DISKFUNC structure with the callbacks
; [esp+8] = pointer to name (ASCIIZ string)
; [esp+12] = userdata to be passed to the callbacks as is.
; [esp+16] = flags, bitfield. Currently only DISK_NO_INSERT_NOTIFICATION bit
;            is defined.
; Return value:
; NULL = operation has failed
; non-NULL = handle of the disk. This handle can be used
; in the operations with other Disk* functions.
; The handle is the pointer to the internal structure DISK.
disk_add:
        push    ebx esi         ; save used registers to be stdcall
; 1. Allocate the DISK structure.
; 1a. Call the heap manager.
        push    sizeof.DISK
        pop     eax
        call    malloc
; 1b. Check the result. If allocation failed, return (go to 9) with eax = 0.
        test    eax, eax
        jz      .nothing
; 2. Copy the disk name to the DISK structure.
; 2a. Get length of the name, including the terminating zero.
        mov     ebx, [esp+8+8]  ; ebx = pointer to name
        push    eax             ; save allocated pointer to DISK
        xor     eax, eax        ; the argument of malloc() is in eax
@@:
        inc     eax
        cmp     byte [ebx+eax-1], 0
        jnz     @b
; 2b. Call the heap manager. Note that it can change ebx.
        push    ebx
        call    malloc
        pop     ebx
; 2c. Check the result. If allocation failed, go to 7.
        pop     esi             ; restore allocated pointer to DISK
        test    eax, eax
        jz      .free
; 2d. Store the allocated pointer to the DISK structure.
        mov     [esi+DISK.Name], eax
; 2e. Copy the name.
@@:
        mov     dl, [ebx]
        mov     [eax], dl
        inc     ebx
        inc     eax
        test    dl, dl
        jnz     @b
; 3. Copy other arguments of the function to the DISK structure.
        mov     eax, [esp+4+8]
        mov     [esi+DISK.Functions], eax
        mov     eax, [esp+12+8]
        mov     [esi+DISK.UserData], eax
        mov     eax, [esp+16+8]
        mov     [esi+DISK.DriverFlags], eax
; 4. Initialize other fields of the DISK structure.
; Media is not inserted, reference counter is 1.
        lea     ecx, [esi+DISK.MediaLock]
        call    mutex_init
        xor     eax, eax
        mov     dword [esi+DISK.MediaInserted], eax
        mov     [esi+DISK.MediaRefCount], eax
        inc     eax
        mov     [esi+DISK.RefCount], eax
; The DISK structure is initialized.
; 5. Insert the new structure to the global list.
; 5a. Acquire the mutex.
        mov     ecx, disk_list_mutex
        call    mutex_lock
; 5b. Insert item to the tail of double-linked list.
        mov     edx, disk_list
        list_add_tail esi, edx     ;esi= new edx= list head
; 5c. Release the mutex.
        call    mutex_unlock
; 6. Return with eax = pointer to DISK.
        xchg    eax, esi
        jmp     .nothing
.free:
; Memory allocation for DISK structure succeeded, but for disk name failed.
; 7. Free the DISK structure.
        xchg    eax, esi
        call    free
; 8. Return with eax = 0.
        xor     eax, eax
.nothing:
; 9. Return.
        pop     esi ebx         ; restore used registers to be stdcall
        ret     16              ; purge 4 dword arguments to be stdcall

; This function deletes a disk device from the global filesystem.
; This includes:
; - removing a media including all partitions;
; - deleting this structure from the global filesystem;
; - dereferencing the DISK structure and possibly destroying it.
; Parameters:
; [esp+4] = handle of the disk, i.e. the pointer to the DISK structure.
; Return value: none.
disk_del:
        push    esi         ; save used registers to be stdcall
; 1. Force media to be removed. If the media is already removed, the
; call does nothing.
        mov     esi, [esp+4+4]  ; esi = handle of the disk
        stdcall disk_media_changed, esi, 0
; 2. Delete the structure from the global list.
; 2a. Acquire the mutex.
        mov     ecx, disk_list_mutex
        call    mutex_lock
; 2b. Delete item from double-linked list.
        mov     eax, [esi+DISK.Next]
        mov     edx, [esi+DISK.Prev]
        mov     [eax+DISK.Prev], edx
        mov     [edx+DISK.Next], eax
; 2c. Release the mutex.
        call    mutex_unlock
; 3. The structure still has one reference created in disk_add. Remove this
; reference. If there are no other references, disk_dereference will free the
; structure.
        call    disk_dereference
; 4. Return.
        pop     esi             ; restore used registers to be stdcall
        ret     4               ; purge 1 dword argument to be stdcall

; This is an internal function which removes a previously obtained reference
; to the disk. If this is the last reference, this function lets the driver
; finalize all associated data, and afterwards frees the DISK structure.
; esi = pointer to DISK structure
disk_dereference:
; 1. Decrement reference counter. Use atomic operation to correctly handle
; possible simultaneous calls.
        lock dec [esi+DISK.RefCount]
; 2. If the result is nonzero, there are other references, so nothing to do.
; In this case, return (go to 4).
        jnz     .nothing
; 3. If we are here, we just removed the last reference and must destroy the
; disk object.
; 3a. Call the driver.
        mov     al, DISKFUNC.close
        stdcall disk_call_driver
; 3b. Free the structure.
        xchg    eax, esi
        push    ebx
        call    free
        pop     ebx
; 4. Return.
.nothing:
        ret

; This is an internal function which removes a previously obtained reference
; to the media. If this is the last reference, this function calls 'closemedia'
; callback to signal the driver that the processing has finished and it is safe
; to inform about a new media.
; esi = pointer to DISK structure
disk_media_dereference:
; 1. Decrement reference counter. Use atomic operation to correctly handle
; possible simultaneous calls.
        lock dec [esi+DISK.MediaRefCount]
; 2. If the result is nonzero, there are other references, so nothing to do.
; In this case, return (go to 4).
        jnz     .nothing
; 3. If we are here, we just removed the last reference and must destroy the
; media object.
; Note that the same place inside the DISK structure is reused for all media
; objects, so we must guarantee that reusing does not happen while freeing.
; Reusing is only possible when someone processes a new media. There are two
; mutually exclusive variants:
; * driver issues media insert notifications (DISK_NO_INSERT_NOTIFICATION bit
;   in DISK.DriverFlags is not set). In this case, we require from the driver
;   that such notification (except for the first one) can occur only after a
;   call to 'closemedia' callback.
; * driver does not issue media insert notifications. In this case, the kernel
;   itself must sometimes check whether media is inserted. We have the flag
;   DISK.MediaUsed, visible to the kernel. This flag signals to the other parts
;   of kernel that the way is free.
; In the first case other parts of the kernel do not use DISK.MediaUsed, so it
; does not matter when this flag is cleared. In the second case this flag must
; be cleared after all other actions, including call to 'closemedia'.
; 3a. Free all partitions.
        push    esi edi
        mov     edi, [esi+DISK.NumPartitions]
        mov     esi, [esi+DISK.Partitions]
        test    edi, edi
        jz      .nofree
.freeloop:
        lodsd
        call    free
        dec     edi
        jnz     .freeloop
.nofree:
        pop     edi esi
; 3b. Free the cache.
        call    disk_free_cache
; 3c. Call the driver.
        mov     al, DISKFUNC.closemedia
        stdcall disk_call_driver
; 3d. Clear the flag.
        mov     [esi+DISK.MediaUsed], 0
.nothing:
        ret

; This function is called by the driver and informs the kernel that the media
; has changed. If the media is non-removable, it is called exactly once
; immediately after 'disk_add' and once from 'disk_del'.
; Parameters:
; [esp+4] = handle of the disk, i.e. the pointer to the DISK structure.
; [esp+8] = new status of the media: zero = no media, nonzero = media inserted.
disk_media_changed:
        push    ebx esi edi             ; save used registers to be stdcall
; 1. Remove the existing media, if it is present.
        mov     esi, [esp+4+12]         ; esi = pointer to DISK
; 1a. Check whether it is present. Since DISK.MediaInserted is changed only
; in this function and calls to this function are synchronized, no lock is
; required for checking.
        cmp     [esi+DISK.MediaInserted], 0
        jz      .noremove
; We really need to remove the media.
; 1b. Acquire mutex.
        lea     ecx, [esi+DISK.MediaLock]
        call    mutex_lock
; 1c. Clear the flag.
        mov     [esi+DISK.MediaInserted], 0
; 1d. Release mutex.
        call    mutex_unlock
; 1e. Remove the "lifetime" reference and possibly destroy the structure.
        call    disk_media_dereference
.noremove:
; 2. Test whether there is new media.
        cmp     dword [esp+8+12], 0
        jz      .noinsert
; Yep, there is.
; 3. Process the new media. We assume that all media fields are available to
; use, see comments in 'disk_media_dereference' (this covers using by previous
; media referencers) and note that calls to this function are synchronized
; (this covers using by new media referencers).
; 3a. Call the 'querymedia' callback.
; .Flags are set to zero for possible future extensions.
        lea     edx, [esi+DISK.MediaInfo]
        and     [edx+DISKMEDIAINFO.Flags], 0
        mov     al, DISKFUNC.querymedia
        stdcall disk_call_driver, edx
; 3b. Check the result of the callback. Abort if it failed.
        test    eax, eax
        jnz     .noinsert
; 3c. Allocate the cache unless disabled by the driver. Abort if failed.
        call    disk_init_cache
        test    al, al
        jz      .noinsert
; 3d. Acquire the lifetime reference for the media object.
        inc     [esi+DISK.MediaRefCount]
; 3e. Scan for partitions. Ignore result; the list of partitions is valid even
; on errors.
        call    disk_scan_partitions
; 3f. Media is inserted and available for use.
        inc     [esi+DISK.MediaInserted]
.noinsert:
; 4. Return.
        pop     edi esi ebx             ; restore used registers to be stdcall
        ret     8                       ; purge 2 dword arguments to be stdcall

; This function is a thunk for all functions of a disk driver.
; It checks whether the referenced function is implemented in the driver.
; If so, this function jumps to the function in the driver.
; Otherwise, it jumps to the default implementation.
; al = offset of function in the DISKFUNC structure;
; esi = pointer to the DISK structure;
; stack is the same as for the corresponding function except that the
; first parameter (void* userdata) is prepended automatically.
disk_call_driver:
        movzx   eax, al ; eax = offset of function in the DISKFUNC structure
; 1. Prepend the first argument to the stack.
        pop     ecx     ; ecx = return address
        push    [esi+DISK.UserData]     ; add argument
        push    ecx     ; save return address
; 2. Check that the required function is inside the table. If not, go to 5.
        mov     ecx, [esi+DISK.Functions]
        cmp     eax, [ecx+DISKFUNC.strucsize]
        jae     .default
; 3. Check that the required function is implemented. If not, go to 5.
        mov     ecx, [ecx+eax]
        test    ecx, ecx
        jz      .default
; 4. Jump to the required function.
        jmp     ecx
.default:
; 5. Driver does not implement the required function; use default implementation.
        jmp     dword [disk_default_callbacks+eax-4]

; The default implementation of DISKFUNC.querymedia.
disk_default_querymedia:
        push    DISK_STATUS_INVALID_CALL
        pop     eax
        ret     8

; The default implementation of DISKFUNC.read and DISKFUNC.write.
disk_default_read:
disk_default_write:
        push    DISK_STATUS_INVALID_CALL
        pop     eax
        ret     20

; The default implementation of DISKFUNC.close, DISKFUNC.closemedia and
; DISKFUNC.flush.
disk_default_close:
disk_default_closemedia:
disk_default_flush:
        xor     eax, eax
        ret     4

; The default implementation of DISKFUNC.adjust_cache_size.
disk_default_adjust_cache_size:
        mov     eax, [esp+8]
        ret     8

; This is an internal function called from 'disk_media_changed' when a new media
; is detected. It creates the list of partitions for the media.
; If media is not partitioned, then the list consists of one partition which
; covers all the media.
; esi = pointer to the DISK structure.
disk_scan_partitions:
; 1. Initialize .NumPartitions and .Partitions fields as zeros: empty list.
        and     [esi+DISK.NumPartitions], 0
        and     [esi+DISK.Partitions], 0
; 2. Currently we can work only with 512-bytes sectors. Check this restriction.
; The only exception is 2048-bytes CD/DVD, but they are not supported yet by
; this code.
        cmp     [esi+DISK.MediaInfo.SectorSize], 512
        jz      .doscan
        DEBUGF 1,'K : sector size is %d, only 512 is supported\n',[esi+DISK.MediaInfo.SectorSize]
        ret
.doscan:
; 3. Acquire the buffer for MBR and bootsector tests. See the comment before
; the 'partition_buffer_users' variable.
        mov     ebx, mbr_buffer         ; assume the global buffer is free
        lock inc [partition_buffer_users]
        jz      .buffer_acquired        ; yes, it is free
        lock dec [partition_buffer_users]       ; no, we must allocate
        stdcall kernel_alloc, 512*3
        test    eax, eax
        jz      .nothing
        xchg    eax, ebx
.buffer_acquired:
; MBR/EBRs are organized in the chain. We use a loop over MBR/EBRs, but no
; more than MAX_NUM_PARTITION times.
; 4. Prepare things for the loop.
; ebp will hold the sector number for current MBR/EBR.
; [esp] will hold the sector number for current extended partition, if there
; is one.
; [esp+4] will hold the counter that prevents long loops.
        push    ebp             ; save ebp
        push    MAX_NUM_PARTITIONS      ; the counter of max MBRs to process
        xor     ebp, ebp        ; start from sector zero
        push    ebp             ; no extended partition yet
.new_mbr:
; 5. Read the current sector.
; Note that 'read' callback operates with 64-bit sector numbers, so we must
; push additional zero as a high dword of sector number.
        mov     al, DISKFUNC.read
        push    1
        stdcall disk_call_driver, ebx, ebp, 0, esp
        pop     ecx
; 6. If the read has failed, abort the loop.
        dec     ecx
        jnz     .mbr_failed
; 7. Check the MBR/EBR signature. If it is wrong, abort the loop.
; Soon we will access the partition table which starts at ebx+0x1BE,
; so we can fill its address right now. If we do it now, then the addressing
; [ecx+0x40] is shorter than [ebx+0x1fe]: one-byte offset vs 4-bytes offset.
        lea     ecx, [ebx+0x1be]        ; ecx -> partition table
        cmp     word [ecx+0x40], 0xaa55
        jnz     .mbr_failed
; 8. The MBR is treated differently from EBRs. For MBR we additionally need to
; execute step 9 and possibly step 10.
        test    ebp, ebp
        jnz     .mbr
; The partition table can be present or not present. In the first case, we just
; read the MBR. In the second case, we just read the bootsector for a
; filesystem.
; The following algorithm is used to distinguish between these cases.
; A. If at least one entry of the partition table is invalid, this is
;    a bootsector. See the description of 'is_partition_table_entry' for
;    definition of validity.
; B. If all entries are empty (filesystem type field is zero) and the first
;    byte is jmp opcode (0EBh or 0E9h), this is a bootsector which happens to
;    have zeros in the place of partition table.
; C. Otherwise, this is an MBR.
; 9. Test for MBR vs bootsector.
; 9a. Check entries. If any is invalid, go to 10 (rule A).
        call    is_partition_table_entry
        jc      .notmbr
        add     ecx, 10h
        call    is_partition_table_entry
        jc      .notmbr
        add     ecx, 10h
        call    is_partition_table_entry
        jc      .notmbr
        add     ecx, 10h
        call    is_partition_table_entry
        jc      .notmbr
; 9b. Check types of the entries. If at least one is nonzero, go to 11 (rule C).
        mov     al, [ecx-30h+PARTITION_TABLE_ENTRY.Type]
        or      al, [ecx-20h+PARTITION_TABLE_ENTRY.Type]
        or      al, [ecx-10h+PARTITION_TABLE_ENTRY.Type]
        or      al, [ecx+PARTITION_TABLE_ENTRY.Type]
        jnz     .mbr
; 9c. Empty partition table or bootsector with many zeroes? (rule B)
        cmp     byte [ebx], 0EBh
        jz      .notmbr
        cmp     byte [ebx], 0E9h
        jnz     .mbr
.notmbr:
; 10. This is not an  MBR. The media is not partitioned. Create one partition
; which covers all the media and abort the loop.
        stdcall disk_add_partition, 0, 0, \
                dword [esi+DISK.MediaInfo.Capacity], dword [esi+DISK.MediaInfo.Capacity+4]
        jmp     .done
.mbr:
; 11. Process all entries of the new MBR/EBR
        lea     ecx, [ebx+0x1be]        ; ecx -> partition table
        push    0       ; assume no extended partition
        call    process_partition_table_entry
        add     ecx, 10h
        call    process_partition_table_entry
        add     ecx, 10h
        call    process_partition_table_entry
        add     ecx, 10h
        call    process_partition_table_entry
        pop     ebp
; 12. Test whether we found a new EBR and should continue the loop.
; 12a. If there was no next EBR, return.
        test    ebp, ebp
        jz      .done
; Ok, we have EBR.
; 12b. EBRs addresses are relative to the start of extended partition.
; For simplicity, just abort if an 32-bit overflow occurs; large disks
; are most likely partitioned with GPT, not MBR scheme, since the precise
; calculation here would increase limit just twice at the price of big
; compatibility problems.
        pop     eax     ; load extended partition
        add     ebp, eax
        jc      .mbr_failed
; 12c. If extended partition has not yet started, start it.
        test    eax, eax
        jnz     @f
        mov     eax, ebp
@@:
; 12c. If the limit is not exceeded, continue the loop.
        dec     dword [esp]
        push    eax     ; store extended partition
        jnz     .new_mbr
.mbr_failed:
.done:
; 13. Cleanup after the loop.
        pop     eax     ; not important anymore
        pop     eax     ; not important anymore
        pop     ebp     ; restore ebp
; 14. Release the buffer.
; 14a. Test whether it is the global buffer or we have allocated it.
        cmp     ebx, mbr_buffer
        jz      .release_partition_buffer
; 14b. If we have allocated it, free it.
        xchg    eax, ebx
        call    free
        jmp     .nothing
; 14c. Otherwise, release reference.
.release_partition_buffer:
        lock dec [partition_buffer_users]
.nothing:
; 15. Return.
        ret

; This is an internal function called from disk_scan_partitions. It checks
; whether the entry pointed to by ecx is a valid entry of partition table.
; The entry is valid if the first byte is 0 or 80h, the first sector plus the
; length is less than twice the size of media. Multiplication by two is
; required since the size mentioned in the partition table can be slightly
; greater than the real size.
is_partition_table_entry:
; 1. Check .Bootable field.
        mov     al, [ecx+PARTITION_TABLE_ENTRY.Bootable]
        and     al, 7Fh
        jnz     .invalid
; 3. Calculate first sector + length. Note that .FirstAbsSector is relative
; to the MBR/EBR, so the real sum is ebp + .FirstAbsSector + .Length.
        mov     eax, ebp
        xor     edx, edx
        add     eax, [ecx+PARTITION_TABLE_ENTRY.FirstAbsSector]
        adc     edx, 0
        add     eax, [ecx+PARTITION_TABLE_ENTRY.Length]
        adc     edx, 0
; 4. Divide by two.
        shr     edx, 1
        rcr     eax, 1
; 5. Compare with capacity. If the subtraction (edx:eax) - .Capacity does not
; overflow, this is bad.
        sub     eax, dword [esi+DISK.MediaInfo.Capacity]
        sbb     edx, dword [esi+DISK.MediaInfo.Capacity+4]
        jnc     .invalid
.valid:
; 5. Return success: CF is cleared.
        clc
        ret
.invalid:
; 6. Return fail: CF is set.
        stc
        ret

; This is an internal function called from disk_scan_partitions. It processes
; the entry pointed to by ecx.
; * If the entry is invalid, just ignore this entry.
; * If the type is zero, just ignore this entry.
; * If the type is one of types for extended partition, store the address
;   of this partition as the new MBR in [esp+4].
; * Otherwise, add the partition to the list of partitions for this disk.
;   We don't use the type from the entry to identify the file system;
;   fs-specific checks do this more reliably.
process_partition_table_entry:
; 1. Check for valid entry. If invalid, return (go to 5).
        call    is_partition_table_entry
        jc      .nothing
; 2. Check for empty entry. If invalid, return (go to 5).
        mov     al, [ecx+PARTITION_TABLE_ENTRY.Type]
        test    al, al
        jz      .nothing
; 3. Check for extended partition. If extended, go to 6.
irp type,\
    0x05,\                 ; DOS: extended partition
    0x0f,\                 ; WIN95: extended partition, LBA-mapped
    0xc5,\                 ; DRDOS/secured: extended partition
    0xd5                   ; Old Multiuser DOS secured: extended partition
{
        cmp     al, type
        jz      .extended
}
; 4. If we are here, that is a normal partition. Add it to the list.
; Note that the first sector is relative to MBR/EBR.
        mov     eax, ebp
        xor     edx, edx
        add     eax, [ecx+PARTITION_TABLE_ENTRY.FirstAbsSector]
        adc     edx, 0
        push    ecx
        stdcall disk_add_partition, eax, edx, \
                [ecx+PARTITION_TABLE_ENTRY.Length], 0
        pop     ecx
.nothing:
; 5. Return.
        ret
.extended:
; 6. If we are here, that is an extended partition. Store the address.
        mov     eax, [ecx+PARTITION_TABLE_ENTRY.FirstAbsSector]
        mov     [esp+4], eax
        ret

; This is an internal function called from disk_scan_partitions and
; process_partition_table_entry. It adds one partition to the list of
; partitions for the media.
proc disk_add_partition stdcall uses ebx edi, start:qword, length:qword
; 1. Check that this partition will not exceed the limit on total number.
        cmp     [esi+DISK.NumPartitions], MAX_NUM_PARTITIONS
        jae     .nothing
; 2. Check that this partition does not overlap with any already registered
; partition. Since any file system assumes that the disk data will not change
; outside of its control, such overlap could be destructive.
; Since the number of partitions is usually very small and is guaranteed not
; to be large, the simple linear search is sufficient.
; 2a. Prepare the loop: edi will point to the current item of .Partitions
; array, ecx will be the current item, ebx will hold number of items left.
        mov     edi, [esi+DISK.Partitions]
        mov     ebx, [esi+DISK.NumPartitions]
        test    ebx, ebx
        jz      .partitionok
.scan_existing:
; 2b. Get the next partition.
        mov     ecx, [edi]
        add     edi, 4
; The range [.FirstSector, .FirstSector+.Length) must be either entirely to
; the left of [start, start+length) or entirely to the right.
; 2c. Subtract .FirstSector - start. The possible overflow distinguish between
; cases "to the left" (2e) and "to the right" (2d).
        mov     eax, dword [ecx+PARTITION.FirstSector]
        mov     edx, dword [ecx+PARTITION.FirstSector+4]
        sub     eax, dword [start]
        sbb     edx, dword [start+4]
        jb      .less
; 2d. .FirstSector is greater than or equal to start. Check that .FirstSector
; is greater than or equal to start+length; the subtraction
; (.FirstSector-start) - length must not cause overflow. Go to 2g if life is
; good or to 2f in the other case.
        sub     eax, dword [length]
        sbb     edx, dword [length+4]
        jb      .overlap
        jmp     .next_existing
.less:
; 2e. .FirstSector is less than start. Check that .FirstSector+.Length is less
; than or equal to start. If the addition (.FirstSector-start) + .Length does
; not cause overflow, then .FirstSector + .Length is strictly less than start;
; since the equality is also valid, use decrement preliminarily. Go to 2g or
; 2f depending on the overflow.
        sub     eax, 1
        sbb     edx, 0
        add     eax, dword [ecx+PARTITION.Length]
        adc     edx, dword [ecx+PARTITION.Length+4]
        jnc     .next_existing
.overlap:
; 2f. The partition overlaps with previously registered partition. Say warning
; and return with nothing done.
        dbgstr 'two partitions overlap, ignoring the last one'
        jmp     .nothing
.next_existing:
; 2g. The partition does not overlap with the current partition. Continue the
; loop.
        dec     ebx
        jnz     .scan_existing
.partitionok:
; 3. The partition has passed tests. Reallocate the partitions array for a new
; entry.
; 3a. Call the allocator.
        mov     eax, [esi+DISK.NumPartitions]
        inc     eax     ; one more entry
        shl     eax, 2  ; each entry is dword
        call    malloc
; 3b. Test the result. If failed, return with nothing done.
        test    eax, eax
        jz      .nothing
; 3c. Copy the old array to the new array.
        mov     edi, eax
        push    esi
        mov     ecx, [esi+DISK.NumPartitions]
        mov     esi, [esi+DISK.Partitions]
        rep movsd
        pop     esi
; 3d. Set the field in the DISK structure to the new array.
        xchg    [esi+DISK.Partitions], eax
; 3e. Free the old array.
        call    free
; 4. Recognize the file system.
; 4a. Call the filesystem recognizer. It will allocate the PARTITION structure
; with possible filesystem-specific fields.
        call    disk_detect_partition
; 4b. Check return value. If zero, return with list not changed; so far only
; the array was reallocated, this is ok for other code.
        test    eax, eax
        jz      .nothing
; 5. Insert the new partition to the list.
        stosd
        inc     [esi+DISK.NumPartitions]
; 6. Return.
.nothing:
        ret
endp

; This is an internal function called from disk_add_partition.
; It tries to recognize the file system on the partition and allocates the
; corresponding PARTITION structure with filesystem-specific fields.
disk_detect_partition:
; This function inherits the stack frame from disk_add_partition. In stdcall
; with ebp-based frame arguments start from ebp+8, since [ebp]=saved ebp
; and [ebp+4]=return address.
virtual at ebp+8
.start  dq      ?
.length dq      ?
end virtual
; When disk_add_partition is called, ebx contains a pointer to
; a two-sectors-sized buffer. This function saves ebx in the stack
; immediately before ebp.
virtual at ebp-4
.buffer dd      ?
end virtual
; 1. Read the bootsector to the buffer.
        mov     al, DISKFUNC.read
        mov     ebx, [.buffer]
        add     ebx, 512
        push    1
        stdcall disk_call_driver, ebx, dword [.start], dword [.start+4], esp
; 2. Run tests for all supported filesystems. If at least one test succeeded,
; go to 4.
; For tests: qword [ebp+8] = partition start, qword [ebp+10h] = partition
; length, [esp] = 0 if reading bootsector failed or 1 if succeeded,
; ebx points to the buffer for bootsector.
        call    fat_create_partition
        test    eax, eax
        jnz     .success
; 3. No file system has recognized the volume, so just allocate the PARTITION
; structure without extra fields.
        push    sizeof.PARTITION
        pop     eax
        call    malloc
        test    eax, eax
        jz      .nothing
        mov     edx, dword [.start]
        mov     dword [eax+PARTITION.FirstSector], edx
        mov     edx, dword [.start+4]
        mov     dword [eax+PARTITION.FirstSector+4], edx
        mov     edx, dword [.length]
        mov     dword [eax+PARTITION.Length], edx
        mov     edx, dword [.length+4]
        mov     dword [eax+PARTITION.Length+4], edx
        mov     [eax+PARTITION.Disk], esi
        and     [eax+PARTITION.FSUserFunctions], 0
.success:
.nothing:
; 4. Return with eax = pointer to PARTITION or NULL.
        pop     ecx
        ret

; This function is called from file_system_lfn.
; This handler gets the control each time when fn 70 is called
; with unknown item of root subdirectory.
; in: esi -> name
;     ebp = 0 or rest of name relative to esi
; out: if the handler processes path, it must not return in file_system_lfn,
;      but instead pop return address and return directly to the caller
;      otherwise simply return
dyndisk_handler:
        push    ebx edi         ; save registers used in file_system_lfn
; 1. Acquire the mutex.
        mov     ecx, disk_list_mutex
        call    mutex_lock
; 2. Loop over the list of DISK structures.
; 2a. Initialize.
        mov     ebx, disk_list
.scan:
; 2b. Get the next item.
        mov     ebx, [ebx+DISK.Next]
; 2c. Check whether the list is done. If so, go to 3.
        cmp     ebx, disk_list
        jz      .notfound
; 2d. Compare names. If names match, go to 5.
        mov     edi, [ebx+DISK.Name]
        push    esi
@@:
; esi points to the name from fs operation; it is terminated by zero or slash.
        lodsb
        test    al, al
        jz      .eoin_dec
        cmp     al, '/'
        jz      .eoin
; edi points to the disk name.
        inc     edi
; edi points to lowercase name, this is a requirement for the driver.
; Characters at esi can have any register. Lowercase the current character.
; This lowercasing works for latin letters and digits; since the disk name
; should not contain other symbols, this is ok.
        or      al, 20h
        cmp     al, [edi-1]
        jz      @b
.wrongname:
; 2f. Names don't match. Continue the loop.
        pop     esi
        jmp     .scan
.notfound:
; The loop is done and no name matches.
; 3. Release the mutex.
        call    mutex_unlock
; 4. Return normally.
        pop     edi ebx         ; restore registers used in file_system_lfn
        ret
; part of 2d: the name matches partially, but we must check that this is full
; equality.
.eoin_dec:
        dec     esi
.eoin:
        cmp     byte [edi], 0
        jnz     .wrongname
; We found the addressed DISK structure.
; 5. Reference the disk.
        lock inc [ebx+DISK.RefCount]
; 6. Now we are sure that the DISK structure is not going to die at least
; while we are working with it, so release the global mutex.
        call    mutex_unlock
        pop     ecx             ; pop from the stack saved value of esi
; 7. Acquire the mutex for media object.
        pop     edi             ; restore edi
        lea     ecx, [ebx+DISK.MediaLock]
        call    mutex_lock
; 8. Get the media object. If it is not NULL, reference it.
        xor     edx, edx
        cmp     [ebx+DISK.MediaInserted], dl
        jz      @f
        mov     edx, ebx
        inc     [ebx+DISK.MediaRefCount]
@@:
; 9. Now we are sure that the media object, if it exists, is not going to die
; at least while we are working with it, so release the mutex for media object.
        call    mutex_unlock
        mov     ecx, ebx
        pop     ebx eax         ; restore ebx, pop return address
; 10. Check whether the fs operation wants to enumerate partitions (go to 11)
; or work with some concrete partition (go to 12).
        cmp     byte [esi], 0
        jnz     .haspartition
; 11. The fs operation wants to enumerate partitions.
; 11a. Only "list directory" operation is applicable to /<diskname> path. Check
; the operation code. If wrong, go to 13.
        cmp     dword [ebx], 1
        jnz     .access_denied
; 11b. If the media is inserted, use 'fs_dyndisk_next' as an enumeration
; procedure. Otherwise, use 'fs_dyndisk_next_nomedia'.
        mov     esi, fs_dyndisk_next_nomedia
        test    edx, edx
        jz      @f
        mov     esi, fs_dyndisk_next
@@:
; 11c. Let the procedure from fs_lfn.inc do the job.
        jmp     file_system_lfn.maindir_noesi
.haspartition:
; 12. The fs operation has specified some partition.
; 12a. Store parameters for callback functions.
        push    edx
        push    ecx
; 12b. Store callback functions.
        push    dyndisk_cleanup
        push    fs_dyndisk
        mov     edi, esp
; 12c. Let the procedure from fs_lfn.inc do the job.
        jmp     file_system_lfn.found2
.access_denied:
; 13. Fail the operation with the appropriate code.
        mov     dword [esp+32], ERROR_ACCESS_DENIED
.cleanup:
; 14. Cleanup.
        mov     esi, ecx        ; disk*dereference assume that esi points to DISK
.cleanup_esi:
        test    edx, edx        ; if there are no media, we didn't reference it
        jz      @f
        call    disk_media_dereference
@@:
        call    disk_dereference
; 15. Return.
        ret

; This is a callback for cleaning up things called from file_system_lfn.found2.
dyndisk_cleanup:
        mov     esi, [edi+8]
        mov     edx, [edi+12]
        jmp     dyndisk_handler.cleanup_esi

; This is a callback for enumerating partitions called from
; file_system_lfn.maindir in the case of inserted media.
; It just increments eax until DISK.NumPartitions reached and then
; cleans up.
fs_dyndisk_next:
        cmp     eax, [ecx+DISK.NumPartitions]
        jae     .nomore
        inc     eax
        clc
        ret
.nomore:
        pusha
        mov     esi, ecx
        call    disk_media_dereference
        call    disk_dereference
        popa
        stc
        ret

; This is a callback for enumerating partitions called from
; file_system_lfn.maindir in the case of missing media.
; In this case we create one pseudo-partition.
fs_dyndisk_next_nomedia:
        cmp     eax, 1
        jae     .nomore
        inc     eax
        clc
        ret
.nomore:
        pusha
        mov     esi, ecx
        call    disk_dereference
        popa
        stc
        ret

; This is a callback for doing real work with selected partition.
; Currently this is just placeholder, since no file systems are supported.
; edi = esp -> {dd fs_dyndisk, dd dyndisk_cleanup, dd pointer to DISK, dd media object}
; ecx = partition number, esi+ebp = ASCIIZ name
fs_dyndisk:
        dec     ecx     ; convert to zero-based partition index
        pop     edx edx edx eax ; edx = pointer to DISK, eax = NULL or edx
        test    eax, eax
        jz      .nomedia
.main:
        cmp     ecx, [edx+DISK.NumPartitions]
        jae     .notfound
        mov     eax, [edx+DISK.Partitions]
        mov     eax, [eax+ecx*4]
        mov     edi, [eax+PARTITION.FSUserFunctions]
        test    edi, edi
        jz      .nofs
        mov     ecx, [ebx]
        cmp     [edi], ecx
        jbe     .unsupported
        push    edx
        push    ebp
        mov     ebp, eax
        call    dword [edi+4+ecx*4]
        pop     ebp
        pop     edx
        mov     dword [esp+32], eax
        mov     dword [esp+20], ebx
.cleanup:
        mov     esi, edx
        call    disk_media_dereference
        call    disk_dereference
        ret
.nofs:
        mov     dword [esp+32], ERROR_UNKNOWN_FS
        jmp     .cleanup
.notfound:
        mov     dword [esp+32], ERROR_FILE_NOT_FOUND
        jmp     .cleanup
.unsupported:
        mov     dword [esp+32], ERROR_UNSUPPORTED_FS
        jmp     .cleanup
.nomedia:
        test    ecx, ecx
        jnz     .notfound
        test    byte [edx+DISK.DriverFlags], DISK_NO_INSERT_NOTIFICATION
        jz      .deverror
; if the driver does not support insert notifications and we are the only fs
; operation with this disk, issue the fake insert notification; if media is
; still not inserted, 'disk_media_changed' will detect this and do nothing
        lea     ecx, [edx+DISK.MediaLock]
        call    mutex_lock
        cmp     [edx+DISK.MediaRefCount], 1
        jnz     .noluck
        call    mutex_unlock
        push    edx
        stdcall disk_media_changed, edx, 1
        pop     edx
        lea     ecx, [edx+DISK.MediaLock]
        call    mutex_lock
        cmp     [edx+DISK.MediaInserted], 0
        jz      .noluck
        lock inc [edx+DISK.MediaRefCount]
        call    mutex_unlock
        xor     ecx, ecx
        jmp     .main
.noluck:
        call    mutex_unlock
.deverror:
        mov     dword [esp+32], ERROR_DEVICE
        mov     esi, edx
        call    disk_dereference
        ret

; This function is called from file_system_lfn.
; This handler is called when virtual root is enumerated
; and must return all items which can be handled by this.
; It is called several times, first time with eax=0
; in: eax = 0 for first call, previously returned value for subsequent calls
; out: eax = 0 => no more items
;      eax != 0 => buffer pointed to by edi contains name of item
dyndisk_enum_root:
        push    edx             ; save register used in file_system_lfn
        mov     ecx, disk_list_mutex    ; it will be useful
; 1. If this is the first call, acquire the mutex and initialize.
        test    eax, eax
        jnz     .notfirst
        call    mutex_lock
        mov     eax, disk_list
.notfirst:
; 2. Get next item.
        mov     eax, [eax+DISK.Next]
; 3. If there are no more items, go to 6.
        cmp     eax, disk_list
        jz      .last
; 4. Copy name from the DISK structure to edi.
        push    eax esi
        mov     esi, [eax+DISK.Name]
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        pop     esi eax
; 5. Return with eax = item.
        pop     edx             ; restore register used in file_system_lfn
        ret
.last:
; 6. Release the mutex and return with eax = 0.
        call    mutex_unlock
        xor     eax, eax
        pop     edx             ; restore register used in file_system_lfn
        ret

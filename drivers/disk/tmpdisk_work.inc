; Callbacks which implement tmpdisk-specific disk functions for tmpdisk.asm.

; The first argument of every callback is .userdata = userdata arg of AddDisk.
; For tmpdisk, .userdata is the disk id, one of 0,...,max_num_disks-1.

DISK_STATUS_OK              = 0 ; success
DISK_STATUS_GENERAL_ERROR   = -1; if no other code is suitable
DISK_STATUS_INVALID_CALL    = 1 ; invalid input parameters
DISK_STATUS_NO_MEDIA        = 2 ; no media present
DISK_STATUS_END_OF_MEDIA    = 3 ; end of media while reading/writing data

; The last function that is called for the given disk. The kernel calls it when
; the kernel has finished all operations with the disk and it is safe to free
; all driver-specific data identified by 'userdata'.
proc tmpdisk_close
  virtual at esp+4
    .userdata dd ?
  end virtual
; Free the memory for disk and zero global variables.
        mov     edx, [.userdata]
        mov     [disk_sizes+edx*4], 0
        xor     eax, eax
        xchg    eax, [disk_pointers+edx*4]
        invoke  KernelFree, eax
        retn    4
endp

struc DISKMEDIAINFO
{
  .flags      dd ?
DISK_MEDIA_READONLY = 1
  .sectorsize dd ?
  .capacity   dq ?
}
virtual at 0
DISKMEDIAINFO DISKMEDIAINFO
end virtual

; Returns information about disk media.
proc tmpdisk_querymedia
  virtual at esp+4
    .userdata dd ?
    .info dd ?
  end virtual
; Media is always present, sector size is always 512 bytes,
; the size of disk in sectors is stored in a global variable.
        mov     edx, [.userdata]
        mov     ecx, [.info]
        mov     [ecx+DISKMEDIAINFO.flags], 0
        mov     [ecx+DISKMEDIAINFO.sectorsize], 512
        mov     eax, [disk_sizes+edx*4]
        mov     dword [ecx+DISKMEDIAINFO.capacity], eax
        mov     dword [ecx+DISKMEDIAINFO.capacity+4], 0
; Return zero as an indicator of success.
        xor     eax, eax
        retn    8
endp

; Reads one or more sectors from the device.
tmpdisk_read:
        xor     edx, edx ; 0 = reading
        jmp     tmpdisk_readwrite

; Writes one or more sectors to the device.
tmpdisk_write:
        mov     dl, 1 ; 1 = writing
; Fall through to tmpdisk_readwrite.

; Common procedure for reading and writing.
; dl = 0 for reading, dl = 1 for writing.
; Arguments of tmpdisk_read and tmpdisk_write are the same,
; they continue to be stack arguments of this procedure.
proc tmpdisk_readwrite \
  userdata:dword, \
  buffer:dword, \
  start_sector:qword, \
  numsectors_ptr:dword
; 1. Save used registers to be stdcall.
        push    esi edi
        mov     esi, [userdata]
        mov     edi, [numsectors_ptr]
; 1. Determine number of sectors to be transferred.
; This is either the requested number of sectors or number of sectors
; up to the disk boundary, depending of what is less.
        xor     ecx, ecx
; 1a. Test whether [start_sector] is less than [disk_sizes] for selected disk.
; If so, calculate number of sectors between [start_sector] and [disk_sizes].
; Otherwise, the actual number of sectors is zero.
        cmp     dword [start_sector+4], ecx
        jnz     .got_number
        mov     eax, [disk_sizes+esi*4]
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
        mov     edi, dword [start_sector]
        shl     edi, 9
        add     edi, [disk_pointers+esi*4]
        mov     esi, [buffer]
; 5. Calculate number of dwords to be transferred.
        shl     ecx, 9-2
; 6. Now esi = [buffer], edi = pointer inside disk.
; This is normal for write operations;
; exchange esi and edi for read operations.
        test    dl, dl
        jnz     @f
        xchg    esi, edi
@@:
; 7. Copy data.
        rep movsd
; 8. Restore used registers to be stdcall and return.
; The value in eax was calculated in step 2.
        pop     edi esi
        ret
endp

; The kernel calls this function when initializing cache subsystem for
; the media. This call allows the driver to adjust the cache size.
proc tmpdisk_adjust_cache_size
  virtual at esp+4
    .userdata dd ?
    .suggested_size dd ?
  end virtual
; Since tmpdisk does not need cache, just return 0.
        xor     eax, eax
        retn    8
endp

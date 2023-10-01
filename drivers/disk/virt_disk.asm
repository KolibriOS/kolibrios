;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2023. All rights reserved.         ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;    Virtual disk driver for KolibriOS                            ;;
;;                                                                 ;;
;;    Written by Mikhail Frolov aka Doczom                         ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DISK_STATUS_OK              = 0 ; success
DISK_STATUS_GENERAL_ERROR   = -1; if no other code is suitable
DISK_STATUS_INVALID_CALL    = 1 ; invalid input parameters
DISK_STATUS_NO_MEDIA        = 2 ; no media present
DISK_STATUS_END_OF_MEDIA    = 3 ; end of media while reading/writing data

; For all IOCTLs the driver returns one of the following error codes:
NO_ERROR                equ 0
ERROR_INVALID_IOCTL     equ 1 ; unknown IOCTL code

maxPathLength = 1000h

include '../struct.inc'

; TODO list:
; add support VDI image

; Input structures:
Flag:
        .Ro = 1b
        .Wo = 10b
        .RW = 11b

struct DISK_DEV
        next            rd      1
        pref            rd      1
        SectorCount     rd      2
        DiskHandle      rd      1
        DiskNumber      rd      1
        Flags           rd      1 ; 1-ro 2-wo 3-rw
        TypeImage       rd      1 ; 0-raw 1-vhd 2-vdi 3-imd
        SectorSize      rd      1
        DiskPath        rb      maxPathLength
ends
struct  IMAGE_ADD_STRUCT
        Flags           rd      1 ; 1-ro 2-wo 3-rw
        TypeImage       rd      1 ; 0-raw 1-vhd 2-vdi 3-imd
        SectorSize      rd      1
        DiskPath        rb      maxPathLength
ends

struct DISKMEDIAINFO
        Flags      dd ?
        SectorSize dd ?
        Capacity   dq ?
ends

        DEBUG                   = 1
        __DEBUG__               = 1
        __DEBUG_LEVEL__         = 1  ; 1 = verbose, 2 = errors only

format PE DLL native 0.05
entry START

section '.flat' code readable writable executable

include '../proc32.inc'
include '../peimport.inc'
include '../macros.inc'

include '../fdo.inc'

proc START c, state:dword, cmdline:dword
        xor     eax, eax ; set return value in case we will do nothing
        cmp     dword [state], 1
        jne     .nothing

        mov     ecx, disk_list_lock
        invoke  MutexInit

        DEBUGF  1, "VIRT_DISK: driver loaded\n"
        invoke  RegService, my_service, service_proc
        ret
.nothing:
        ret
endp

; get version
; add disk
; del disk
; get list disks
; get disk info
proc service_proc
        push    ebx esi edi
; 2. Get parameter from the stack: [esp+16] is the first parameter,
;    pointer to IOCTL structure.
        mov     ebx, [esp + 16]    ; edx -> IOCTL

        mov     ecx, [ebx + IOCTL.io_code]
        test    ecx, ecx        ; check for SRV_GETVERSION
        jnz     .add_disk

        cmp     [ebx + IOCTL.out_size], 4
        jb      .error_ret
        mov     eax, [ebx + IOCTL.output]
        mov     dword [eax], 1 ;API_VERSION
        xor     eax, eax
        jmp     .return

.add_disk:
        dec     ecx     ; check for DEV_ADD_DISK
        jnz     .del_disk

        cmp     [ebx + IOCTL.inp_size], sizeof.IMAGE_ADD_STRUCT
        jb      .error_ret

        cmp     [ebx + IOCTL.out_size], 4
        jb      .error_ret

        invoke  KernelAlloc, sizeof.DISK_DEV
        test    eax, eax
        jz      .error_ret

        push    eax
        mov     edi, eax
        mov     ecx, sizeof.DISK_DEV/4
        xor     eax, eax
        rep stosd
        pop     eax

        mov     edi, eax
        add     edi, DISK_DEV.Flags
        mov     esi, [ebx + IOCTL.input]
        mov     ecx, sizeof.IMAGE_ADD_STRUCT/4
        rep movsd

        mov     esi, eax ;save

        cmp     byte[esi + DISK_DEV.DiskPath], '/'
        jnz     .add_disk.error

        mov     ecx, disk_list_lock
        invoke  MutexLock

        call    get_free_num
        jnz     .add_disk.error2
        mov     [esi + DISK_DEV.DiskNumber], eax
        mov     ecx,[ebx + IOCTL.output]
        mov     [ecx], eax

        ; init image
        mov     eax, [esi + DISK_DEV.TypeImage]
        cmp     eax, image_type.max_num
        ja      .add_disk.error2

        call    dword[image_type + eax*8]  ;esi - DISK_DEV*
        test    eax, eax
        jnz     .add_disk.error2

        ; creating name
        push    ebp
        mov     ebp, esp

        mov     eax, [esi + DISK_DEV.DiskNumber]
        mov     edi, esp
        dec     edi
        mov     byte[edi], 0
        mov     ecx, 10
@@:
        xor     edx, edx
        div     ecx
        add     edx,'0'
        dec     edi
        mov     byte[edi], dl
        test    eax, eax
        jnz     @b

        mov     esp, edi
        push    word 'vd'
        sub     edi, 2

        ; add disk
        mov     eax, [esi + DISK_DEV.TypeImage]
        invoke  DiskAdd, dword[image_type + eax*8 + 4] , \
                         edi, esi, 0
        mov     [esi + DISK_DEV.DiskHandle], eax
        mov     esp, ebp
        pop     ebp
        test    eax, eax
        jz      .add_disk.error2

        invoke  DiskMediaChanged, eax, 1

        ; add in list
        mov     dword[esi], disk_root_list
        mov     eax, [disk_root_list + 4]
        mov     [esi + 4], eax
        mov     [eax], esi
        mov     [disk_root_list + 4], esi
        inc     dword[disk_count]

        mov     ecx, disk_list_lock
        invoke  MutexUnlock

        xor     eax, eax
        jmp     .return

.add_disk.error2:
        mov     ecx, disk_list_lock
        invoke  MutexUnlock

.add_disk.error:
        invoke  KernelFree, esi
        jmp     .error_ret


.del_disk:
        dec     ecx     ; check for DEV_DEL_DISK
        jnz     .get_root_list

        cmp     [ebx + IOCTL.inp_size], 4
        jb      .error_ret

        mov     ecx, [ebx + IOCTL.input]
        mov     ecx, [ecx]
        call    get_disk

        test    eax, eax
        jz      .error_ret

        cmp     [eax + DISK_DEV.DiskHandle], 0
        jz      .error_ret
        mov     ecx, [eax + DISK_DEV.DiskHandle]
        mov     [eax + DISK_DEV.DiskHandle], 0
        invoke  DiskDel, ecx

        xor     eax, eax
        jmp     .return

.get_root_list:
        dec     ecx     ; check for DEV_DEL_DISK
        jnz     .get_disk_info

        cmp     [ebx + IOCTL.inp_size], 4*2 ; offset + count
        jb      .error_ret
        mov     ecx, [ebx + IOCTL.input]
        mov     edx, [ecx]     ; offset
        mov     eax, [ecx + 4] ; count

        add     edx, eax
        cmp     edx, [disk_count]
        ja      .error_ret

        xor     edx, edx
        imul    eax, sizeof.DISK_DEV - 8
        add     eax, 4
        cmp     [ebx + IOCTL.out_size], eax
        jb      .error_ret

        mov     edi, [ebx + IOCTL.output]
        mov     eax, [disk_count]
        stosd

        mov     edx, [ecx]
        mov     eax, [disk_root_list]
@@:
        test    edx, edx
        jz      @f
        mov     eax, [eax]
        dec     edx
        jmp     @b
@@:
        mov     edx, [ecx + 4]
@@:
        test    edx, edx
        jz      @f
        mov     esi, eax
        add     esi, 8
        mov     ecx, (sizeof.DISK_DEV - 8)/4
        rep movsd
        mov     eax, [eax]
        dec     edx
        jmp     @b
@@:
        xor     eax, eax
        jmp     .return

.get_disk_info:
        dec     ecx
        jnz     .error_ret

        cmp     [ebx + IOCTL.inp_size], 4
        jb      .error_ret
        cmp     [ebx + IOCTL.out_size], sizeof.DISK_DEV - 8
        jb      .error_ret
        mov     ecx, [ebx + IOCTL.input]
        mov     ecx, [ecx]

        call    get_disk
        test    eax, eax
        jz      .error_ret

        mov     esi, eax
        add     esi, 4*2
        mov     edi, [ebx + IOCTL.output]
        mov     ecx, (sizeof.DISK_DEV - 8)/4
        rep movsd

        xor     eax, eax
        jmp     .return

.error_ret:
        mov     eax, ERROR_INVALID_IOCTL
.return:
        pop     edi esi ebx
        retn    4
endp

; IN: ecx - ptr DISK_DEV
; OUT: ZF - found zF - not found
proc    disk_dev_check
        push    eax
        mov     eax, disk_root_list
@@:
        mov     eax, [eax]
        cmp     eax, disk_root_list
        jz      .nf
        cmp     eax, ecx
        jnz     @b
        pop     eax
        ret
.nf:
        test    eax, eax
        pop     eax
        ret
endp
; IN: ecx - disk number
; OUT: eax - ptr DISK_DEV
proc    get_disk
        push    ecx
        mov     ecx, disk_list_lock
        invoke  MutexLock
        pop     ecx

        mov     eax, disk_root_list
@@:
        mov     eax, [eax]
        cmp     eax, disk_root_list
        jz      .nf
        cmp     ecx, [eax + DISK_DEV.DiskNumber]
        jnz     @b

        push    eax
        mov     ecx, disk_list_lock
        invoke  MutexUnlock
        pop     eax
        ret
.nf:
        mov     ecx, disk_list_lock
        invoke  MutexUnlock

        xor     eax, eax
        ret
endp

; OUT: eax - number free disk
;      Zf - good zf - not found
proc    get_free_num
        xor    eax, eax
        mov    edx, disk_root_list
@@:
        mov     edx, [edx]
        cmp     edx, disk_root_list
        jz      @f
        cmp     eax, [edx + DISK_DEV.DiskNumber]
        jnz     @b

        inc     eax
        jnz     @b

        test    edx, edx ; random :)
@@:
        ret
endp

;; IN: ecx - number disk
;; OUT: eax - ptr to DISK_DEV or zero
;proc    get_link_disk
;
;        push    ecx
;        mov     ecx, disk_list_lock
;        invoke  MutexLock
;        pop     ecx
;
;        xor     eax, eax
;        cmp     ecx, [disk_array.len]
;        jae     .end
;
;        mov     eax, [disk_array]
;        mov     eax, [edx + ecx*4]
;
;.end:
;        push    eax
;        ; unlock disk list
;        mov     ecx, disk_list_lock
;        invoke  MutexUnlock
;        pop     eax
;        ret
;endp
;; IN: esi - ptr to DISK_DEV
;; OUT: eax - offset in array or -1
;proc    add_link_disk
;        ; find free item
;        mov     ecx, [disk_array]
;        xor     eax, eax
;        dec     eax
;@@:
;        inc     eax
;        cmp     eax, [disk_array.len]
;        jae     .not_found
;
;        cmp     dword[ecx + eax*4], 0
;        jnz     @b
;
;        mov     [ecx + eax*4], esi
;        ret
;
;.not_found:
;        inc     dword[disk_array.len]
;        ;get new memory
;        mov     eax,[disk_array.len]
;        shl     eax, 2 ;*4
;        invoke  Kmalloc
;        test    eax, eax
;        jz      .err
;        ; copy data
;        push    edi esi
;        mov     ecx, [disk_array.len]
;        mov     edi, eax
;        mov     esi, [disk_array]
;        rep movsd
;        pop     esi edi
;        ; del old array
;        xchg    [disk_array], eax
;        invoke  Kfree
;        mov     eax, [disk_array.len]
;        ret
;.err:
;        dec     dword[disk_array.len]
;        mov     eax, -1
;        ret
;endp
;
;; IN: ecx - offset in array
;proc    del_link_disk
;        mov     edx, ecx
;        dec     edx
;        cmp     edx,[disk_array.len]
;        jz      .last_item
;
;        mov     edx,[disk_array]
;        mov     [edx + ecx*4], 0
;        ret
;.last_item:
;        dec     dword[disk_array.len]
;        ;get new memory
;        mov     eax,[disk_array.len]
;        shl     eax, 2 ;*4
;        invoke  Kmalloc
;        test    eax, eax
;        jz      .err
;        ; copy data
;        push    edi esi
;        mov     ecx, [disk_array.len]
;        mov     edi, eax
;        mov     esi, [disk_array]
;        rep movsd
;        pop     esi edi
;        ; del old array
;        xchg    [disk_array], eax
;        invoke  Kfree
;        ret
;.err:
;        inc     dword[disk_array.len]
;        mov     eax, -1
;        ret
;endp

; RAW IMAGE DISK FUNCTIONS
proc    raw_disk_close stdcall, pdata
        ; del item list
        mov     ecx, disk_list_lock
        invoke  MutexLock

        mov     ecx, [pdata]
        mov     eax, [ecx]      ; eax = next
        mov     edx, [ecx + 4]  ; edx = prev
        mov     [eax + 4], edx  ; [next.prev] = prev
        mov     [edx], eax      ; [prev.next] = next

        dec     dword[disk_count]

        mov     ecx, disk_list_lock
        invoke  MutexUnlock

        invoke  KernelFree, [pdata]
        DEBUGF  1, "VIRT_DISK: disk deleted\n"
        ret
endp

proc   disk_querymedia stdcall, pdata, mediainfo
        mov     eax, [mediainfo]
        mov     edx, [pdata]
        mov     [eax + DISKMEDIAINFO.Flags], 0
        mov     ecx, [edx + DISK_DEV.SectorSize]
        mov     [eax + DISKMEDIAINFO.SectorSize], ecx
        mov     ecx, [edx + DISK_DEV.SectorCount]
        mov     dword[eax + DISKMEDIAINFO.Capacity], ecx
        mov     ecx, [edx + DISK_DEV.SectorCount + 4]
        mov     dword[eax + DISKMEDIAINFO.Capacity + 4], ecx
        xor     eax, eax
        ret
endp

proc    raw_disk_rd stdcall pdata: dword,\
                            buffer: dword,\
                            startsector: qword,\
                            numsectors_ptr:dword

        mov     ecx, [pdata]
        test    [ecx + DISK_DEV.Flags], Flag.Ro
        jz      .no_support

        pusha
        lea     eax,[ecx + DISK_DEV.DiskPath]
        push    eax
        dec     esp
        mov     byte[esp], 0
        push    dword[buffer]

        mov     eax, [numsectors_ptr]
        mov     eax, [eax]
        mul     dword[ecx + DISK_DEV.SectorSize]
        push    eax
        ; get offset for startsector
        mov     eax, dword[startsector]
        xor     edx, edx
        mul     dword[ecx + DISK_DEV.SectorSize]
        push    edx
        push    eax
        mov     eax, dword[startsector + 4]
        mul     dword[ecx + DISK_DEV.SectorSize]
        add     [esp + 4], eax
        push    dword 0 ;read file

        mov     ebx, esp
        invoke  FS_Service
        push    eax
        mov     ecx, [pdata]
        mov     eax, ebx
        xor     edx, edx
        div     dword[ecx + DISK_DEV.SectorSize]
        mov     edx, [numsectors_ptr]
        mov     [edx], eax
        pop     eax

        add     esp, 6*4+1 ; size FS struct
        test    eax, eax
        popa
        jz      @f
        mov     eax, 1
        ret
@@:
        xor     eax, eax
        ret
.no_support:
        mov     eax, DISK_STATUS_GENERAL_ERROR
        ret
endp


proc    raw_disk_wr stdcall pdata: dword,\
                            buffer: dword,\
                            startsector: qword,\
                            numsectors_ptr:dword

        mov     ecx, [pdata]
        test    [ecx + DISK_DEV.Flags], Flag.Wo
        jz      .no_support

        pusha
        lea     eax,[ecx + DISK_DEV.DiskPath]
        push    eax
        dec     esp
        mov     byte[esp],0
        push    dword[buffer]

        mov     eax, [numsectors_ptr]
        mov     eax, [eax]
        mul     dword[ecx + DISK_DEV.SectorSize]
        push    eax
        ; get offset for startsector
        mov     eax, dword[startsector]
        xor     edx, edx
        mul     dword[ecx + DISK_DEV.SectorSize]
        push    edx
        push    eax
        xor     edx, edx
        mov     eax, dword[startsector + 4]
        mul     dword[ecx + DISK_DEV.SectorSize]
        add     [esp + 4], eax
        push    dword 3 ; write file
        mov     ebx, esp
        invoke  FS_Service

        push    eax
        mov     ecx, [pdata]
        mov     eax, ebx
        xor     edx, edx
        div     dword[ecx + DISK_DEV.SectorSize]
        mov     edx, [numsectors_ptr]
        mov     [edx], eax
        pop     eax

        add     esp, 6*4+1 ; size FS struct
        test    eax, eax
        popa
        jz      @f
        mov     eax, 1
        ret
@@:
        xor     eax, eax
        ret

.no_support:
        mov     eax, DISK_STATUS_GENERAL_ERROR
        ret
endp

disk_root_list:
        dd      disk_root_list
        dd      disk_root_list
disk_count: dd 0
disk_list_lock: MUTEX

image_type:
        ;  init function, table disk function
        dd raw_image_init, raw_disk_functions
        ; vdi
.max_num = ($ - image_type - 8) / 8; 8 - item size

align 4
; esi - ptr to DISK_DEV
;WARNING: raw image size >=2tb not supported.

proc    raw_image_init
        sub     esp, 40 ; for file_info
        mov     ecx, esp

        pusha
        lea     eax,[esi + DISK_DEV.DiskPath]
        push    eax
        dec     esp
        mov     byte[esp],0
        push    ecx
        xor     eax, eax
        push    eax eax eax
        push    dword 5

        mov     ebx, esp
        invoke  FS_Service
        add     esp, 6*4+1
        test    eax, eax
        popa
        lea     esp,[esp + 40]
        jnz     .err

        ; WARNING: Not working with stack, destroys the structure!
        mov     eax, [ecx + 32]
        mov     edx, [ecx + 36]
        test    eax, eax
        jz      .err

        div     dword[esi + DISK_DEV.SectorSize]   ; undefined exeption
        mov     [esi + DISK_DEV.SectorCount], eax
        mov     [esi + DISK_DEV.SectorCount + 4], 0
        ; END WARNING

        xor     eax, eax
        ret
.err:
        or      eax, -1
        ret
endp

align 4
raw_disk_functions:
        dd      .size
        dd      raw_disk_close
        dd      0 ; no need in .closemedia
        dd      disk_querymedia
        dd      raw_disk_rd
        dd      raw_disk_wr
        dd      0 ; no need in .flush
        dd      disk_adjust_cache_size
.size = $ - raw_disk_functions

proc disk_adjust_cache_size
  virtual at esp+4
    .userdata dd ?
    .suggested_size dd ?
  end virtual
        xor     eax, eax
        retn    8
endp

my_service      db      'VIRT_DISK',0

data fixups
end data

include_debug_strings
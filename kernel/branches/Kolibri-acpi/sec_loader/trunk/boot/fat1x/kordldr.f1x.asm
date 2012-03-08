; Copyright (c) 2008-2009, diamond
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
;       * Redistributions of source code must retain the above copyright
;       notice, this list of conditions and the following disclaimer.
;       * Redistributions in binary form must reproduce the above copyright
;       notice, this list of conditions and the following disclaimer in the
;       documentation and/or other materials provided with the distribution.
;       * Neither the name of the <organization> nor the
;       names of its contributors may be used to endorse or promote products
;       derived from this software without specific prior written permission.
;
; THIS SOFTWARE IS PROVIDED BY Alexey Teplov aka <Lrz> ''AS IS'' AND ANY
; EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
; WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
; DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
; DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
; (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
; ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
; SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;*****************************************************************************

        org     0x7E00
; the KordOS FAT12/FAT16 bootsector loads first cluster of this file to 0:7E00 and transfers control to here
; ss:bp = 0:7C00
virtual at bp
                rb      3               ; BS_jmpBoot
                rb      8               ; BS_OEMName, ignored
                dw      ?               ; BPB_BytsPerSec
BPB_SecsPerClus db      ?
BPB_RsvdSecCnt  dw      ?
BPB_NumFATs     db      ?
BPB_RootEntCnt  dw      ?
BPB_TotSec16    dw      ?
                db      ?               ; BPB_Media
BPB_FATSz16     dw      ?
BPB_SecPerTrk   dw      ?
BPB_NumHeads    dw      ?
BPB_HiddSec     dd      ?
BPB_TotSec32    dd      ?
BS_DrvNum       db      ?
fat_type        db      ?               ; this is BS_Reserved1,
                                ; we use it to save FS type: 0=FAT12, 1=FAT16
                db      ?               ; BS_BootSig
num_sectors     dd      ?               ; BS_VolID
;               rb      11              ; BS_VolLab
;               rb      3               ; BS_FilSysType, first 3 bytes
read_sectors    dw      ?
read_sectors2   dw      ?
lookup_in_root_dir dw   ?
scan_for_filename dw    ?
err_             dw      ?
noloader        dw      ?
cachelimit      dw      ?
filesize:       ; will be used to save file size
                rb      5               ; BS_FilSysType, last 5 bytes
; following variables are located in the place of starting code;
; starting code is no more used at this point
sect_per_clus   dw      ?
cur_cluster     dw      ?
next_cluster    dw      ?
flags           dw      ?
cur_delta       dd      ?
end virtual

; procedures from boot sector
; LBA version
lba_read_sectors = 7CE2h
lba_read_sectors2 = 7CDCh
lba_lookup_in_root_dir = 7D4Fh
lba_scan_for_filename = 7D2Dh
lba_err = 7CB5h
lba_noloader = 7CB2h
; CHS version
chs_read_sectors = 7CDEh
chs_read_sectors2 = 7CD8h
chs_lookup_in_root_dir = 7D70h
chs_scan_for_filename = 7D4Eh
chs_err = 7CB1h
chs_noloader = 7CAEh

        push    ax cx           ; save our position on disk
        push    ss
        pop     es
; determine version of bootsector (LBA vs CHS)
;       mov     [read_sectors], chs_read_sectors
;       mov     [read_sectors2], chs_read_sectors2
;       mov     [lookup_in_root_dir], chs_lookup_in_root_dir
;       mov     [scan_for_filename], chs_scan_for_filename
;       mov     [err], chs_err
;       mov     [noloader], chs_noloader
        lea     di, [read_sectors]
        mov     si, chs_proc_addresses
        mov     cx, 6*2
        cmp     word [chs_scan_for_filename], 0xFF31    ; 'xor di,di'
        jz      @f
        add     si, cx
;       mov     [read_sectors], lba_read_sectors
;       mov     [read_sectors2], lba_read_sectors2
;       mov     [lookup_in_root_dir], lba_lookup_in_root_dir
;       mov     [scan_for_filename], lba_scan_for_filename
;       mov     [err], lba_err
;       mov     [noloader], lba_noloader
@@:
        rep movsb
        mov     cl, [BPB_SecsPerClus]
        mov     [sect_per_clus], cx
        xor     bx, bx
; determine size of cache for folders
        int     12h             ; ax = size of available base memory in Kb
        sub     ax, 94000h / 1024
        jae     @f
nomem:
        mov     si, nomem_str
        jmp     [err_]
@@:
        shr     ax, 3
        mov     [cachelimit], ax        ; size of cache - 1
; get type of file system - FAT12 or FAT16?
; calculate number of clusters
        mov     ax, [BPB_TotSec16]
        xor     dx, dx
        test    ax, ax
        jnz     @f
        mov     ax, word [BPB_TotSec32]
        mov     dx, word [BPB_TotSec32+2]
@@:
        sub     ax, [bp-8]      ; dword [bp-8] = first data sector
        sbb     dx, [bp-6]
        jb      j_noloader
        div     [sect_per_clus]
; ax = number of clusters
; note: this is loader for FAT12/FAT16, so 'div' does not overflow on correct volumes
        mov     [fat_type], ch
        cmp     ax, 0xFF5
        jb      init_fat12
        inc     [fat_type]
init_fat16:
; no sectors loaded
        mov     di, 0x8200
        xor     ax, ax
        mov     cx, 0x100/2
        rep stosw
        jmp     init_fat_done
init_fat12:
; read FAT
        push    0x6000
        pop     es
        mov     ax, [BPB_RsvdSecCnt]
        mov     cx, [BPB_FATSz16]
        cmp     cx, 12
        jb      @f
        mov     cx, 12
@@:
        xor     dx, dx
        call    [read_sectors]
init_fat_done:
; if cluster = sector, we need to read second part of our file
; (bootsector loads only first cluster of kordldr.f1x)
        pop     cx ax           ; restore our position on disk
        cmp     cx, 1
        ja      kordldr_full
        sub     ax, [bp-8]
        inc     ax
        inc     ax              ; ax = first cluster of kordldr.f12
        call    get_next_cluster
        jc      @f
j_noloader:
        jmp     [noloader]
@@:
        dec     ax
        dec     ax
        push    0x800
        pop     es
        call    [read_sectors2]
kordldr_full:
; ...continue loading...
        mov     di, secondary_loader_info
        call    load_file
        test    bx, bx
        mov     bx, [err_]
        jz      @f
        mov     si, aKernelNotFound
        jmp     bx
@@:
; for subsequent calls to callback function, hook error handler
;       mov     byte [bx], 0xE9         ; 'jmp' opcode
;       mov     ax, hooked_err - 3
;       sub     ax, bx
;       mov     word [bx+1], ax
; push hooked_err / ret
        mov     word [bx], 0x68 + ((hooked_err and 0xFF) shl 8)
        mov     word [bx+2], (hooked_err shr 8) + (0xC3 shl 8)
; set registers for secondary loader
        mov     ah, [BS_DrvNum]
        mov     al, 'f'
        test    ah, ah
        jns     @f
        sub     ah, 80h
        mov     al, 'h'
@@:
        mov     bx, '12'
        cmp     [fat_type], 0
        jz      @f
        mov     bh, '6'
@@:
        mov     si, callback    ; ds:si = far pointer to callback procedure
        jmp     far [si-callback+secondary_loader_info] ; jump to 1000:0000

nomem_str       db      'No memory',0

chs_proc_addresses:
        dw      chs_read_sectors
        dw      chs_read_sectors2
        dw      chs_lookup_in_root_dir
        dw      chs_scan_for_filename
        dw      chs_err
        dw      chs_noloader
lba_proc_addresses:
        dw      lba_read_sectors
        dw      lba_read_sectors2
        dw      lba_lookup_in_root_dir
        dw      lba_scan_for_filename
        dw      lba_err
        dw      lba_noloader

get_next_cluster:
; in: ax = cluster
; out: if there is next cluster: CF=1, ax = next cluster
; out: if there is no next cluster: CF=0
        push    si
        cmp     [fat_type], 0
        jnz     gnc16
; for FAT12
        push    ds
        push    0x6000
        pop     ds
        mov     si, ax
        shr     si, 1
        add     si, ax
        test    al, 1
        lodsw
        jz      @f
        shr     ax, 4
@@:
        and     ax, 0xFFF
        cmp     ax, 0xFF7
        pop     ds si
        ret
; for FAT16
gnc16:
; each sector contains 200h bytes = 100h FAT entries
; so ah = # of sector, al = offset in sector
        mov     si, ax
        mov     ah, 0
        shr     si, 8
; calculate segment for this sector of FAT table
; base for FAT table is 6000:0000, so the sector #si has to be loaded to (60000 + 200*si)
; segment = 6000 + 20*si, offset = 0
        push    es
        push    si
        shl     si, 5
        add     si, 0x6000
        mov     es, si
        pop     si
        cmp     byte [ss:0x8200+si], ah ; sector already loaded?
        jnz     @f
; load corresponding sector
        pusha
        push    es
        xor     bx, bx
        mov     ax, [BPB_RsvdSecCnt]
        xor     dx, dx
        add     ax, si
        adc     dx, bx
        mov     cx, 1   ; read 1 sector
        call    [read_sectors]
        pop     es
        popa
@@:
        mov     si, ax
        add     si, si
;       mov     ax, [es:si]
        lods    word [es:si]
        pop     es
        cmp     ax, 0xFFF7
        pop     si
        ret

if $ > 0x8000
error 'get_next_cluster must fit in first sector of kordldr.f1x!'
end if

load_file:
; in: ss:bp = 0:7C00
; in: ds:di -> information structure
;       dw:dw   address
;       dw      limit in 4Kb blocks (0x1000 bytes) (must be non-zero and not greater than 0x100)
;       ASCIIZ  name
; out: bx = status: bx=0 - ok, bx=1 - file is too big, only part of file was loaded, bx=2 - file not found
; out: dx:ax = file size (0xFFFFFFFF if file not found)
        xor     ax, ax  ; start from root directory
        mov     dx, -1
        mov     word [filesize], dx
        mov     word [filesize+2], dx   ; initialize file size with invalid value
        lea     si, [di+6]
parse_dir_loop:
; convert name to FAT name
        push    di
        push    ax
        push    ss
        pop     es
; convert ASCIIZ filename to FAT name
        mov     di, filename
        push    di
        mov     cx, 8+3
        mov     al, ' '
        rep stosb
        pop     di
        mov     cl, 8   ; 8 symbols per name
        mov     bl, 1
nameloop:
        lodsb
        test    al, al
        jz      namedone
        cmp     al, '/'
        jz      namedone
        cmp     al, '.'
        jz      namedot
        dec     cx
        js      badname
        cmp     al, 'a'
        jb      @f
        cmp     al, 'z'
        ja      @f
        sub     al, 'a'-'A'
@@:
        stosb
        jmp     nameloop
namedot:
        inc     bx
        jp      badname
        add     di, cx
        mov     cl, 3
        jmp     nameloop
badname:        ; do not make direct js/jp to notfound_pop:
                ; this generates long forms of conditional jumps and results in longer code
        jmp     notfound_pop
namedone:
; scan directory
        pop     ax      ; ax = cluster of directory or 0 for root
        push    ds
        push    si
        push    es
        pop     ds
        mov     si, filename    ; ds:si -> filename in FAT style
        test    ax, ax
        jnz     lookup_in_notroot_dir
; for root directory, use the subroutine from bootsector
        call    [lookup_in_root_dir]
        jmp     lookup_done
lookup_in_notroot_dir:
; for other directories, read a folder sector-by-sector and scan
; first, try to use the cache
        push    ds
        push    cs
        pop     ds
        mov     bx, [cachelimit]
        add     bx, bx
        mov     di, foldcache_mark
@@:
        mov     dx, [foldcache_clus+di-foldcache_mark+bx]
        cmp     dx, ax
        jz      cacheok
        test    dx, dx
        jz      cacheadd        ; the cache has place for new entry
        dec     bx
        dec     bx
        jns     @b
; the folder is not present in the cache, so add it
; the cache is full; find the oldest entry and replace it with the new one
        mov     dx, [cachelimit]
@@:
        inc     bx
        inc     bx
        cmp     word [di+bx], dx        ; marks have values 0 through [cachelimit]
        jnz     @b
cacheadd:
        or      word [di+bx], 0xFFFF    ; very big value, it will be changed soon
        mov     [foldcache_clus+di-foldcache_mark+bx], ax
        and     [foldcache_size+di-foldcache_mark+bx], 0        ; no folder items yet
cacheok:
; update cache marks
        mov     dx, [di+bx]
        mov     cx, [foldcache_size+di-foldcache_mark+bx]
        mov     di, [cachelimit]
        add     di, di
cacheupdate:
        cmp     [foldcache_mark+di], dx
        adc     [foldcache_mark+di], 0
        dec     di
        dec     di
        jns     cacheupdate
        and     [foldcache_mark+bx], 0
; done, bx contains (position in cache)*2
        pop     ds
;       mov     dx, bx
;       shl     dx, 8           ; dx = (position in cache)*0x2000/0x10
;       add     dx, 0x9200
        lea     dx, [bx+0x92]
        xchg    dl, dh
        mov     es, dx
        jcxz    not_in_cache
        call    [scan_for_filename]
        jz      lookup_done
not_in_cache:
; cache miss, read folder data from disk
        mov     bx, cx
        shr     bx, 4
        shl     cx, 5
        mov     di, cx          ; es:di -> free space in cache entry
; external loop: scan clusters
folder_next_cluster:
; internal loop: scan sectors in cluster
        mov     cx, [sect_per_clus]
        push    ax
        dec     ax
        dec     ax
        mul     cx
        add     ax, [bp-8]
        adc     dx, [bp-6]      ; dx:ax = absolute sector
folder_next_sector:
; skip first bx sectors
        dec     bx
        jns     folder_skip_sector
        push    cx
        push    es di
        push    0x8000
        pop     es
        xor     bx, bx
        mov     cx, 1
        push    es
        call    [read_sectors]
; copy data to the cache...
        pop     ds
        pop     di es
        cmp     di, 0x2000      ; ...if there is free space, of course
        jae     @f
        push    si di
        mov     cx, 0x100
        xor     si, si
        rep movsw
        mov     di, es
        shr     di, 8
        add     [ss:foldcache_size+di-0x92], 0x10       ; 0x10 new entries in the cache
        pop     di si
@@:
        push    es
        push    0x8000
        pop     es
        push    cs
        pop     ds
        mov     cx, 0x10
        call    [scan_for_filename]
        pop     es
        pop     cx
        jz      lookup_done_pop
folder_skip_sector:
        inc     ax
        jnz     @f
        inc     dx
@@:
        loop    folder_next_sector
        pop     ax      ; ax = current cluster
        call    get_next_cluster
        jc      folder_next_cluster
        stc
        push    ax
lookup_done_pop:
        pop     ax
lookup_done:
        pop     si
        pop     ds
; CF=1 <=> failed
        jnc     found
notfound:
        pop     di
        mov     bx, 2   ; file not found
        mov     ax, 0xFFFF
        mov     dx, ax  ; invalid file size
        ret
notfound_pop:
        pop     ax
        jmp     notfound
found:
        mov     ax, [es:di+26]  ; get cluster
        test    byte [es:di+11], 10h    ; directory?
        jz      regular_file
        cmp     byte [si-1], 0
        jz      notfound        ; don't read directories as a regular files
; ok, we have found a directory and the caller requested a file into it
        pop     di
        jmp     parse_dir_loop  ; restart with new cluster in ax
regular_file:
        cmp     byte [si-1], 0
        jnz     notfound        ; file does not contain another files
; ok, we have found a regular file and the caller requested it
; save file size
        mov     dx, [es:di+28]
        mov     [filesize], dx
        mov     dx, [es:di+30]
        mov     [filesize+2], dx
        pop     di
        mov     si, [di+4]
        shl     si, 3
        push    si              ; [ds:di+4] = limit in 4K blocks
        les     bx, [di]        ; es:bx -> buffer
clusloop:
; ax = first cluster, top of stack contains limit in sectors
        mov     si, ax  ; remember current cluster
        xor     cx, cx  ; cx will contain number of consecutive clusters
        mov     word [cur_delta], cx
        mov     word [cur_delta+2], cx
        mov     di, ax
clusfind:
        inc     di
        inc     cx
        call    get_next_cluster
        jnc     clusread
        cmp     ax, di
        jz      clusfind
        stc
clusread:
        pop     di      ; limit in sectors
        push    ax      ; save next cluster
        pushf           ; save flags
; read cx clusters, starting from si
; calculate number of sectors
        xchg    ax, cx
        mul     [sect_per_clus]
; dx:ax = number of sectors; compare with limit
        mov     word [num_sectors], ax
        mov     word [num_sectors+2], dx
        jmp     @f
continue_load_file:
        les     bx, [di]        ; es:bx -> buffer
        mov     di, [di+4]      ; ds:di = limit in 4K blocks
        shl     di, 3   ; now di = limit in sectors
        mov     ax, word [num_sectors]
        mov     dx, word [num_sectors+2]
        mov     si, [cur_cluster]
        push    [next_cluster]
        push    [flags]
        or      ax, dx
        jz      nextclus
@@:
        test    dx, dx
        jnz     clusdecrease
        push    dx      ; limit was not exceeded
        cmp     ax, di
        jbe     @f
        pop     ax
clusdecrease:
        push    1       ; limit was exceeded
        mov     ax, di
@@:
        sub     di, ax  ; calculate new limit
        sub     word [num_sectors], ax
        sbb     word [num_sectors+2], 0
; calculate starting sector
        xchg    ax, cx
        lea     ax, [si-2]
        mul     [sect_per_clus]
        add     ax, word [cur_delta]
        adc     dx, word [cur_delta+2]
        add     word [cur_delta], cx
        adc     word [cur_delta+2], 0
; read
        call    [read_sectors2]
        pop     dx
; next cluster?
nextclus:
        popf
        pop     ax
        mov     [cur_cluster], si
        mov     [next_cluster], ax
        pushf
        pop     [flags]
        jnc     @f      ; no next cluster => return
        mov     dl, 1   ; dh=0 in any case
        test    di, di
        jz      @f      ; if there is next cluster but current limit is 0 => return: limit exceeded
        push    di
        jmp     clusloop        ; all is ok, continue
hooked_err:
        mov     sp, 7C00h-12-2  ; restore stack
        mov     dx, 3           ; return: read error
@@:
        mov     bx, dx
        mov     ax, [filesize]
        mov     dx, [filesize+2]
        ret

; Callback function for secondary loader
callback:
; in: ax = function number; only functions 1 and 2 are defined for now
; save caller's stack
        mov     dx, ss
        mov     cx, sp
; set our stack (required because we need ss=0)
        xor     si, si
        mov     ss, si
        mov     sp, 7C00h-8
        mov     bp, 7C00h
        push    dx
        push    cx
; call our function
        stc     ; unsupported function
        dec     ax
        jz      callback_readfile
        dec     ax
        jnz     callback_ret
; function 2: continue loading file
; can be called only after function 1 returned value bx=1 (only part of file was loaded)
; in: ds:di -> information structure
;       dw:dw   address
;       dw      limit in 4Kb blocks (0x1000 bytes) (must be non-zero and not greater than 0x100)
; out: bx=0 - ok, bx=1 - still only part of file was loaded, bx=3 - read error
; out: dx:ax = file size
        call    continue_load_file
        jmp     callback_ret_succ
callback_readfile:
; function 1: read file
; in: ds:di -> information structure
;       dw:dw   address
;       dw      limit in 4Kb blocks (0x1000 bytes) (must be non-zero and not greater than 0x100)
;       ASCIIZ  name
; out: bx=0 - ok, bx=1 - file is too big, only part of file was loaded, bx=2 - file not found, bx=3 - read error
; out: dx:ax = file size (0xFFFFFFFF if file was not found)
        call    load_file
callback_ret_succ:
        clc     ; function is supported
callback_ret:
; restore caller's stack
        pop     cx
        pop     ss
        mov     sp, cx
; return to caller
        retf

secondary_loader_info:
        dw      0, 0x1000
        dw      0x30000 / 0x1000
        db      'kord/loader',0
aKernelNotFound db      'Fatal error: cannot load the secondary loader',0

foldcache_clus  dw      0,0,0,0,0,0,0   ; start with no folders in cache
foldcache_mark  rw      7
foldcache_size  rw      7
filename        rb      11
if $ > 0x8200
error:
       table overwritten
end if

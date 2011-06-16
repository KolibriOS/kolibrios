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
; the KordOS FAT32 bootsector loads first cluster of this file to 0:7E00 and transfers control to here
; ss:bp = 0:7C00
; ds = 0
virtual at bp
                rb      3       ; BS_jmpBoot
                rb      8       ; BS_OEMName, ignored
                dw      ?       ; BPB_BytsPerSec
BPB_SecsPerClus db      ?
BPB_RsvdSecCnt  dw      ?
BPB_NumFATs     db      ?
BPB_RootEntCnt  dw      ?
                dw      ?       ; BPB_TotSec16
                db      ?       ; BPB_Media
                dw      ?       ; BPB_FATSz16 = 0 for FAT32
BPB_SecPerTrk   dw      ?
BPB_NumHeads    dw      ?
BPB_HiddSec     dd      ?
                dd      ?       ; BPB_TotSec32
BPB_FATSz32     dd      ?
BPB_ExtFlags    dw      ?
                dw      ?       ; BPB_FSVer
BPB_RootClus    dd      ?
filesize:
                dw      ?       ; BPB_FSInfo
                dw      ?       ; BPB_BkBootSec
                rb      12      ; BPB_Reserved
BS_DrvNum       db      ?
                db      ?       ; BS_Reserved1
                db      ?       ; BS_BootSig
                dd      ?       ; BS_VolID
;               rb      11      ; BS_VolLab
;               rb      5       ; BS_FilSysType, first 5 bytes
read_sectors32  dw      ?
read_sectors2   dw      ?
err_             dw      ?
noloader        dw      ?
cachelimit      dw      ?
fatcachehead    rw      2
fatcacheend     dw      ?
                rb      3       ; BS_FilSysType, last 3 bytes
curseg          dw      ?
num_sectors     dd      ?
cur_cluster     dd      ?
next_cluster    dd      ?
flags           dw      ?
cur_delta       dd      ?
end virtual

; procedures from boot sector
; LBA version
lba_read_sectors2 = 7CD6h
lba_err = 7CAAh
lba_noloader = 7CA7h    ; = lba_err - 3
; CHS version
chs_read_sectors2 = 7CD2h
chs_err = 7CA6h
chs_noloader = 7CA3h    ; = chs_err - 3

        push    eax cx          ; save our position on disk
; determine version of bootsector (LBA vs CHS)
        mov     [read_sectors2], chs_read_sectors2
        mov     bx, chs_err
        mov     [err_], bx
;       mov     [noloader], chs_noloader
        cmp     byte [bx], 0xE8         ; [chs_err] = 0xE8 for CHS version, 0x14 for LBA version
        jz      @f
        add     [read_sectors2], lba_read_sectors2 - chs_read_sectors2
        add     [err_], lba_err - chs_err
;       mov     [noloader], lba_noloader
@@:
        xor     bx, bx
; determine size of cache for folders
        int     12h             ; ax = size of available base memory in Kb
        sub     ax, 92000h / 1024
        jae     @f
nomem:
        mov     si, nomem_str
        jmp     [err_]
@@:
        shr     ax, 3
        mov     [cachelimit], ax        ; size of cache - 1
        mov     es, bx
; no folders in cache yet
        mov     di, foldcache_clus
        mov     cx, 8*4/2 + 1
        xor     ax, ax
        rep     stosw
; bootsector code caches one FAT sector, [bp-14], in 6000:0000
; initialize our (more advanced) FAT caching from this
        mov     di, 8400h
        mov     cx, di
        lea     si, [fatcachehead]
        mov     [si], si                ; no sectors in cache:
        mov     [si+2], si              ; 'prev' & 'next' links point to self
        mov     [fatcacheend], di       ; first free item = 8400h
        stosw                   ; 'next cached sector' link
        stosw                   ; 'prev cached sector' link
        mov     eax, [bp-14]
        stosd                           ; first sector number in cache
        test    eax, eax
        js      @f
        mov     [si], cx                ; 'first cached sector' link = 8400h
        mov     [si+2], cx              ; 'next cached sector' link = 8400h
        mov     [fatcacheend], di       ; first free item = 8406h
@@:
; if cluster = sector, we need to read second part of our file
; (bootsector loads only first cluster of kordldr.f32)
        pop     cx eax          ; restore our position on disk
        cmp     cx, 1
        ja      kordldr_full
        sub     eax, [bp-10]
        inc     eax
        inc     eax             ; eax = first cluster of kordldr.f32
        call    get_next_cluster
        jc      @f
;       jmp     [noloader]
        mov     ax, [err_]
        sub     ax, 3
        jmp     ax
@@:
        dec     eax
        dec     eax
        push    0x800
        pop     es
        call    [read_sectors2]
kordldr_full:
; bootsector code has read some data of root directory to 8000:0000
; initialize our folder caching from this
        mov     eax, [BPB_RootClus]
        mov     [foldcache_clus], eax
        mov     cx, [curseg]
        mov     ax, 8000h
        sub     cx, ax          ; cx = size of data read in paragraphs (0x10 bytes)
        shr     cx, 1           ; cx = size of folder data read in entries (0x20 bytes)
        mov     [foldcache_size], cx
        shl     cx, 4
        push    ds
        mov     ds, ax
        push    0x9000
        pop     es
        xor     si, si
        xor     di, di
        rep     movsw
        pop     ds
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
; push hooked_err / ret
        mov     dword [bx], 0x68 + (hooked_err shl 8) + (0xC3 shl 24)
; set registers for secondary loader
        mov     ah, [bp-2]              ; drive id
        mov     al, 'f'
        btr     ax, 15
        jnc     @f
        mov     al, 'h'
@@:
        mov     bx, '32'
        mov     si, callback
        jmp     far [si+secondary_loader_info-callback]

nomem_str       db      'No memory',0

cluster2sector:
        sub     eax, 2
clustersz2sectorsz:
        movzx   ecx, [BPB_SecsPerClus]
        mul     ecx
        ret

get_next_cluster:
; in: eax = cluster
; out: if there is next cluster: CF=1, eax = next cluster
; out: if there is no next cluster: CF=0
        push    di bx
        push    ds es
        push    ss
        pop     ds
        push    ss
        pop     es
        push    ax
        shr     eax, 7
; eax = FAT sector number; look in cache
        mov     di, 8400h
.cache_lookup:
        cmp     di, [fatcacheend]
        jae     .not_in_cache
        scasd
        scasd
        jnz     .cache_lookup
.in_cache:
        sub     di, 8
; delete this sector from the list
        push    si
        mov     si, [di]
        mov     bx, [di+2]
        mov     [si+2], bx
        mov     [bx], si
        pop     si
        jmp     @f
.not_in_cache:
; cache miss
; cache is full?
        mov     di, [fatcacheend]
        cmp     di, 8C00h
        jnz     .cache_not_full
; yes, delete the oldest entry
        mov     di, [fatcachehead]
        mov     bx, [di]
        mov     [fatcachehead], bx
        push    word [di+2]
        pop     word [bx+2]
        jmp     .cache_append
.cache_not_full:
; no, allocate new sector
        add     [fatcacheend], 8
.cache_append:
; read FAT
        mov     [di+4], eax
        pushad
        lea     cx, [di + 0x10000 - 0x8400 + (0x6000 shr (9-4-3))]      ; +0x10000 - for FASM
        shl     cx, 9-4-3
        mov     es, cx
        xor     bx, bx
        mov     cx, 1
        add     eax, [bp-6]     ; FAT start
        sub     eax, [bp-10]
        call    [read_sectors2]
        popad
@@:
; add new sector to the end of list
        mov     bx, di
        xchg    bx, [fatcachehead+2]
        push    word [bx]
        pop     word [di]
        mov     [bx], di
        mov     [di+2], bx
; get requested item
        lea     ax, [di + 0x10000 - 0x8400 + (0x6000 shr (9-4-3))]
        pop     di
        and     di, 0x7F
        shl     di, 2
        shl     ax, 9-4-3
        mov     ds, ax
        and     byte [di+3], 0x0F
        mov     eax, [di]
        pop     es ds
        pop     bx di
        ;and    eax, 0x0FFFFFFF
        cmp     eax, 0x0FFFFFF7
        ret

if $ > 0x8000
error 'get_next_cluster must fit in first sector of kordldr.f32!'
end if

load_file:
; in: ss:bp = 0:7C00
; in: ds:di -> information structure
;       dw:dw   address
;       dw      limit in 4Kb blocks (0x1000 bytes) (must be non-zero and not greater than 0x100)
;       ASCIIZ  name
; out: bx = status: bx=0 - ok, bx=1 - file is too big, only part of file was loaded, bx=2 - file not found
; out: dx:ax = file size (0xFFFFFFFF if file not found)
        mov     eax, [BPB_RootClus]     ; start from root directory
        or      dword [filesize], -1    ; initialize file size with invalid value
        lea     si, [di+6]
parse_dir_loop:
; convert name to FAT name
        push    di
        push    ax
        push    ss
        pop     es
; convert ASCIIZ filename to FAT name
filename equ bp
        mov     di, filename
        push    di
        mov     cx, 8+3
        mov     al, ' '
        rep     stosb
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
        pop     ax      ; eax = cluster of directory
                        ; high word of eax is preserved by operations above
        push    ds
        push    si
; read a folder sector-by-sector and scan
; first, try to use the cache
        push    ss
        pop     ds
        mov     di, foldcache_mark
        xor     bx, bx
        mov     cx, [cachelimit]
@@:
        lea     si, [di+bx]
        mov     edx, dword [foldcache_clus+si-foldcache_mark+bx]
        cmp     edx, eax
        jz      cacheok
        test    edx, edx
        jz      cacheadd        ; the cache has place for new entry
        inc     bx
        inc     bx
        dec     cx
        jns     @b
; the folder is not present in the cache, so add it
; the cache is full; find the oldest entry and replace it with the new one
        mov     bx, -2
        mov     dx, [cachelimit]
@@:
        inc     bx
        inc     bx
        cmp     word [di+bx], dx        ; marks have values 0 through [cachelimit]
        jnz     @b
        lea     si, [di+bx]
cacheadd:
        or      word [di+bx], 0xFFFF    ; very big value, it will be changed soon
        and     [foldcache_size+di-foldcache_mark+bx], 0        ; no folder items yet
        mov     dword [foldcache_clus+si-foldcache_mark+bx], eax
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
        ;mov    dx, bx
        ;shl    dx, 8           ; dx = (position in cache)*0x2000/0x10
        ;add    dx, 0x9000
        lea     dx, [bx + 0x90]
        xchg    dl, dh
        mov     ds, dx
        mov     si, filename    ; ss:si -> filename in FAT style
        call    scan_for_filename
        jz      lookup_done
; cache miss, read folder data from disk
        mov     bx, cx
        shr     bx, 4
        shl     cx, 5
        mov     di, cx          ; es:di -> free space in cache entry
; external loop: scan clusters
folder_next_cluster:
; internal loop: scan sectors in cluster
        push    eax
        call    cluster2sector
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
        push    eax
        call    [read_sectors2]
        pop     eax
; copy data to the cache...
        pop     ds
        pop     di es
        cmp     di, 0x2000      ; ...if there is free space, of course
        jae     @f
        pusha
        mov     cx, 0x100
        xor     si, si
        rep     movsw
        mov     di, es
        shr     di, 8
        add     [ss:foldcache_size+di-0x90], 0x10       ; 0x10 new entries in the cache
        popa
@@:
        push    es
        mov     cl, 0x10        ; ch=0 at this point
        call    scan_for_filename
        pop     es
        pop     cx
        jz      lookup_done_pop
folder_skip_sector:
        inc     eax
        loop    folder_next_sector
        pop     eax     ; eax = current cluster
        call    get_next_cluster
        jc      folder_next_cluster
        stc
        push    eax
lookup_done_pop:
        pop     eax
lookup_done:
        pop     si
; CF=1 <=> failed
        jnc     found
        pop     ds
notfound:
        pop     di
notfound2:
        mov     bx, 2   ; file not found
        mov     ax, 0xFFFF
        mov     dx, ax  ; invalid file size
        ret
notfound_pop:
        pop     ax
        jmp     notfound
found:
        mov     eax, [di+20-2]
        mov     edx, [di+28]
        mov     ax, [di+26]     ; get cluster
        test    byte [di+11], 10h       ; directory?
        pop     ds
        pop     di
        jz      regular_file
        cmp     byte [si-1], 0
        jz      notfound2       ; don't read directories as regular files
; ok, we have found a directory and the caller requested a file into it
        jmp     parse_dir_loop  ; restart with new cluster in ax
regular_file:
        cmp     byte [si-1], 0
        jnz     notfound2       ; file does not contain another files
; ok, we have found a regular file and the caller requested it
; save file size
        mov     [filesize], edx
        mov     si, [di+4]      ; [ds:di+4] = limit in 4K blocks
        shl     si, 3
        push    si
        les     bx, [di]        ; es:bx -> buffer
clusloop:
; eax = first cluster, top of stack contains limit in sectors
        mov     esi, eax        ; remember current cluster
        xor     ecx, ecx        ; ecx will contain number of consecutive clusters
        mov     [cur_delta], ecx
        mov     edi, eax
clusfind:
        inc     edi
        inc     ecx
        call    get_next_cluster
        jnc     clusread
        cmp     eax, edi
        jz      clusfind
        stc
clusread:
        pop     di      ; limit in sectors
        movzx   edi, di
        push    eax     ; save next cluster
        pushf           ; save flags
; read cx clusters, starting from si
; calculate number of sectors
        xchg    eax, ecx
        call    clustersz2sectorsz
        mov     [num_sectors], eax
        jmp     @f
continue_load_file:
        les     bx, [di]        ; es:bx -> buffer
        movzx   edi, word [di+4]        ; di = limit in 4K blocks
        shl     di, 3   ; now di = limit in sectors
        mov     eax, [num_sectors]
        mov     esi, [cur_cluster]
        push    [next_cluster]
        push    [flags]
        test    eax, eax
        jz      nextclus
@@:
; eax = number of sectors; compare with limit
        cmp     eax, edi
        seta    dl
        push    dx      ; limit was exceeded?
        jbe     @f
        mov     eax, edi
@@:
        sub     di, ax  ; calculate new limit
        sub     [num_sectors], eax
        mov     [cur_cluster], esi
; calculate starting sector
        push    ax
        xchg    eax, esi
        call    cluster2sector
        pop     cx
        add     eax, [cur_delta]
        add     [cur_delta], ecx
; read
        call    [read_sectors2]
        pop     dx
; next cluster?
nextclus:
        popf
        pop     eax
        mov     [next_cluster], eax
        pushf
        pop     [flags]
        jnc     @f      ; no next cluster => return
        mov     dl, 1
        test    di, di
        jz      @f      ; if there is next cluster but current limit is 0 => return: limit exceeded
        push    di
        jmp     clusloop        ; all is ok, continue
hooked_err:
        mov     sp, 7C00h-14-2  ; restore stack
        mov     dl, 3           ; return: read error
@@:
        mov     bl, dl
        mov     bh, 0
        mov     ax, [filesize]
        mov     dx, [filesize+2]
        ret

scan_for_filename:
; in: ss:si -> 11-bytes FAT name
; in: ds:0 -> part of directory data
; in: cx = number of entries
; in: bh = 0
; out: if found: CF=0, ZF=1, es:di -> directory entry
; out: if not found, but continue required: CF=1 and ZF=0
; out: if not found and zero item reached: CF=1 and ZF=1
        push    ds
        pop     es
        xor     di, di
        push    cx
        jcxz    snoent
sloop:
        cmp     byte [di], bh
        jz      snotfound
        test    byte [di+11], 8         ; volume label?
        jnz     scont                   ; ignore volume labels
        pusha
        mov     cx, 11
        repz    cmps byte [ss:si], byte [es:di]
        popa
        jz      sdone
scont:
        add     di, 0x20
        loop    sloop
snoent:
        inc     cx      ; clear ZF flag
snotfound:
        stc
sdone:
        pop     cx
lrdret:
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
        mov     sp, 7C00h-10
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
        db      'kernel.mnt',0
aKernelNotFound db      'Fatal error: cannot load the kernel',0

;if $ > 0x8200
;error 'total size of kordldr.f32 must not exceed 1024 bytes!'
;end if

;foldcache_clus dd      0,0,0,0,0,0,0,0 ; start with no folders in cache
;foldcache_mark dw      0
;               rw      7
;foldcache_size rw      8
foldcache_clus  rd      8
foldcache_mark  rw      8
foldcache_size  rw      8

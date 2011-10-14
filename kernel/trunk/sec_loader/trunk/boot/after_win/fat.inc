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

; in: ss:bp = 0:dat
; in: es:bx = address to load file
; in: ds:si -> ASCIIZ name
; in: cx = limit in sectors
; out: bx = status: bx=0 - ok, bx=1 - file is too big, only part of file has been loaded, bx=2 - file not found
; out: dx:ax = file size (0xFFFFFFFF if file not found)
load_file_fat:
        mov     eax, [bp + root_clus - dat]
        mov     [bp + cur_obj - dat], root_string
        push    es
        push    bx
        push    cx
.parse_dir_loop:
; convert name to FAT name
        push    [bp + cur_obj - dat]
        push    ax
        mov     [bp + cur_obj - dat], si
        push    ss
        pop     es
; convert ASCIIZ filename to FAT name
        mov     di, fat_filename
        push    di
        mov     cx, 8+3
        mov     al, ' '
        rep stosb
        pop     di
        mov     cl, 8   ; 8 symbols per name
        mov     bl, 1
.nameloop:
        lodsb
        test    al, al
        jz      .namedone
        cmp     al, '/'
        jz      .namedone
        cmp     al, '.'
        jz      .namedot
        dec     cx
        js      .badname
        cmp     al, 'a'
        jb      @f
        cmp     al, 'z'
        ja      @f
        sub     al, 'a'-'A'
@@:
        stosb
        jmp     .nameloop
.namedot:
        inc     bx
        jp      .badname
        add     di, cx
        mov     cl, 3
        jmp     .nameloop
.badname:
        mov     si, badname_msg
        jmp     find_error_si
.namedone:
; scan directory
        pop     ax      ; eax = cluster of directory
                        ; high word of eax is preserved by operations above
        push    ds
        push    si
; read a folder sector-by-sector and scan
; first, try to use the cache
        push    ss
        pop     ds
        mov     bx, -2
        mov     cx, [bp + rootcache_size - dat]
        cmp     [bp + root_clus - dat], eax
        jz      .lookcache_root
        mov     di, foldcache_mark
        xor     bx, bx
        mov     cx, [bp + cachelimit - dat]
@@:
        lea     si, [di+bx]
        mov     edx, dword [foldcache_clus+si-foldcache_mark+bx]
        cmp     edx, eax
        jz      .cacheok
        test    edx, edx
        jz      .cacheadd       ; the cache has place for new entry
        inc     bx
        inc     bx
        dec     cx
        js      @b
; the folder is not present in the cache, so add it
; the cache is full; find the oldest entry and replace it with the new one
        mov     bx, -2
        mov     dx, [bp + cachelimit - dat]
@@:
        inc     bx
        inc     bx
        cmp     word [di+bx], dx        ; marks have values 0 through [cachelimit]
        jnz     @b
.cacheadd:
        or      word [di+bx], 0xFFFF    ; very big value, it will be changed soon
        and     [foldcache_size+di-foldcache_mark+bx], 0        ; no folder items yet
        lea     si, [di+bx]
        mov     dword [foldcache_clus+si-foldcache_mark+bx], eax
.cacheok:
; update cache marks
        mov     dx, [di+bx]
        mov     cx, [foldcache_size+di-foldcache_mark+bx]
        mov     di, [bp + cachelimit - dat]
        add     di, di
.cacheupdate:
        cmp     [foldcache_mark+di], dx
        adc     [foldcache_mark+di], 0
        dec     di
        dec     di
        jns     .cacheupdate
        and     [foldcache_mark+bx], 0
; done, bx contains (position in cache)*2
.lookcache_root:
; bx = (position in cache)*2 for non-root folders; bx = -2 for root folder
        ;mov    dx, bx
        ;shl    dx, 8
        ;add    dx, 0x9200
        lea     dx, [bx + 0x92]
        xchg    dl, dh
        mov     ds, dx
        mov     si, fat_filename        ; ss:si -> filename in FAT style
        call    fat_scan_for_filename
        jz      .lookup_done
; cache miss, read folder data from disk
; we are reading parent directory, it can result in disk read errors; restore [cur_obj]
        mov     di, sp
        mov     bx, [bp + cur_obj - dat]
        xchg    bx, [ss:di+4]
        mov     [bp + cur_obj - dat], bx
        mov     bx, cx
        add     bx, 0xF
        shr     bx, 4
        shl     cx, 5
        mov     di, cx          ; es:di -> free space in cache entry
; external loop: scan clusters
.folder_next_cluster:
; internal loop: scan sectors in cluster
        movzx   ecx, byte [ss:0x320D]   ; BPB_SecPerClus
        push    eax
; FAT12/16 root - special handling
        test    eax, eax
        jnz     .folder_notroot
        mov     cx, [ss:0x3211]         ; BPB_RootEntCnt
        mov     dx, cx
        add     cx, 0xF
        rcr     cx, 1
        shr     cx, 3
        mov     eax, [bp + root_start - dat]
        jmp     .folder_next_sector
.folder_notroot:
        mul     ecx
        add     eax, [bp + data_start - dat]
.folder_next_sector:
        sub     dx, 0x10
; skip first bx sectors
        dec     bx
        jns     .folder_skip_sector
        push    cx
        push    es di
        push    0x8000
        pop     es
        xor     bx, bx
        mov     cx, 1
        push    es
        call    read
        jc      ..found_disk_error
; copy data to the cache...
        pop     ds
        pop     di es
        cmp     di, 0x2000      ; ...if there is free space, of course
        jae     @f
        pusha
        mov     cx, 0x100
        xor     si, si
        rep movsw
        mov     di, es
        shr     di, 8
        cmp     di, 0x90
        jz      .update_rootcache_size
        add     [ss:foldcache_size+di-0x92], 0x10       ; 0x10 new entries in the cache
        jmp     .updated_cachesize
.update_rootcache_size:
        mov     cl, 0x10
        cmp     cx, dx
        jb      @f
        mov     cx, dx
@@:
        add     [bp + rootcache_size - dat], cx
.updated_cachesize:
        popa
@@:
        push    es
        mov     cl, 0x10        ; ch=0 at this point
        cmp     cx, dx
        jb      @f
        mov     cx, dx
@@:
        call    fat_scan_for_filename
        pop     es
        pop     cx
        jz      .lookup_done_pop
.folder_skip_sector:
        inc     eax
        loop    .folder_next_sector
        pop     eax     ; eax = current cluster
        test    eax, eax
        jz      @f
        call    [bp + get_next_cluster_ptr - dat]
        jc      .folder_next_cluster
@@:
        stc
        push    eax
.lookup_done_pop:
        pop     eax
.lookup_done:
        pop     si
; CF=1 <=> failed
        jnc     .found
        pop     ds
        pop     [bp + cur_obj - dat]
        mov     si, error_not_found
        jmp     find_error_si
.found:
        mov     eax, [di+20-2]
        mov     edx, [di+28]
        mov     ax, [di+26]     ; get cluster
        test    byte [di+11], 10h       ; directory?
        pop     ds
        pop     [bp + cur_obj - dat]    ; forget old [cur_obj]
        jz      .regular_file
        cmp     byte [si-1], 0
        jnz     .parse_dir_loop
..directory_error:
        mov     si, directory_string
        jmp     find_error_si
.regular_file:
        cmp     byte [si-1], 0
        jz      @f
..notdir_error:
        mov     si, notdir_string
        jmp     find_error_si
@@:
; ok, we have found a regular file and the caller requested it
; parse FAT chunk
        push    ss
        pop     es
        push    ss
        pop     ds
        mov     di, 0x4005
        mov     byte [di-5], 1  ; non-resident attribute
        mov     dword [di-4], 1
        stosd
        pop     cx
        push    cx
.parsefat:
        call    [bp + get_next_cluster_ptr - dat]
        jnc     .done
        mov     esi, [di-8]
        add     esi, [di-4]
        cmp     eax, esi
        jz      .contc
        mov     dword [di], 1
        scasd
        stosd
        jmp     @f
.contc:
        inc     dword [di-8]
@@:
        sub     cl, [0x320D]
        sbb     ch, 0
        ja      .parsefat
.done:
        xor     eax, eax
        stosd
        mov     si, 0x4000
load_file_common_end:
        xor     ecx, ecx
        pop     cx
        pop     bx
        pop     es
        mov     [bp + filesize - dat], edx
        mov     [bp + sectors_read - dat], ecx
        add     edx, 0x1FF
        shr     edx, 9
        mov     [bp + filesize_sectors - dat], edx
        cmp     edx, ecx
        seta    al
        mov     ah, 0
        push    ax
        call    read_file_chunk
continue_load_common_end:
        mov     [bp + cur_chunk_ptr - dat], si
        pop     bx
        mov     ax, word [bp + filesize - dat]
        mov     dx, word [bp + filesize+2 - dat]
        jnc     @f
        mov     bl, 3   ; read error
@@:
        ret

continue_load_file:
; es:bx -> buffer for output, ecx = cx = number of sectors
        mov     si, [bp + cur_chunk_ptr - dat]
        push    ecx
        add     ecx, [bp + sectors_read - dat]
        mov     [bp + sectors_read - dat], ecx
        cmp     [bp + filesize_sectors - dat], ecx
        pop     ecx
        seta    al
        mov     ah, 0
        push    ax
        push    continue_load_common_end
        push    ss
        pop     ds
        cmp     [bp + cur_chunk_resident - dat], ah
        jnz     .nonresident
.resident:
        mov     ax, word [bp + num_sectors - dat]
        jmp     read_file_chunk.resident.continue
.nonresident:
        mov     eax, [bp + cur_cluster - dat]
        mov     edx, [bp + num_sectors - dat]
        add     eax, [bp + cur_delta - dat]
        jmp     read_file_chunk.nonresident.continue

fat_scan_for_filename:
; in: ss:si -> 11-bytes FAT name
; in: ds:0 -> part of directory data
; in: cx = number of entries
; out: if found: CF=0, ZF=1, es:di -> directory entry
; out: if not found, but continue required: CF=1 and ZF=0
; out: if not found and zero item reached: CF=1 and ZF=1
        push    ds
        pop     es
        xor     di, di
        push    cx
        jcxz    .noent
.loop:
        cmp     byte [di], 0
        jz      .notfound
        test    byte [di+11], 8         ; volume label?
        jnz     .cont                   ; ignore volume labels
        pusha
        mov     cx, 11
        repz cmps byte [ss:si], byte [es:di]
        popa
        jz      .done
.cont:
        add     di, 0x20
        loop    .loop
.noent:
        inc     cx      ; clear ZF flag
.notfound:
        stc
.done:
        pop     cx
        ret

fat12_get_next_cluster:
; in: ax = cluster (high word of eax is zero)
; out: if there is next cluster: CF=1, ax = next cluster
; out: if there is no next cluster: CF=0
        push    si
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

fat16_get_next_cluster:
; in: ax = cluster (high word of eax is zero)
; out: if there is next cluster: CF=1, ax = next cluster
; out: if there is no next cluster: CF=0
; each sector contains 200h bytes = 100h FAT entries
; so ah = # of sector, al = offset in sector
        push    si
        mov     si, ax
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
        cmp     byte [ss:0x3400+si], 0  ; sector already loaded?
        jnz     .noread
; load corresponding sector, try all FATs if disk read error detected
        pusha
        movzx   di, byte [ss:0x3210]    ; BPB_NumFATs
        xor     bx, bx
        mov     ax, [ss:0x320E]         ; BPB_RsvdSecCnt
        xor     dx, dx
        add     ax, si
        adc     dx, bx
@@:
        push    es
        push    dx ax
        pop     eax
        mov     cx, 1   ; read 1 sector
        call    read
        pop     es
        jnc     @f
        add     ax, [ss:0x3216]         ; BPB_FATSz16
        adc     dx, bx
        dec     di
        jnz     @b
..found_disk_error:
        mov     si, disk_error_msg
        jmp     find_error_si
@@:
        popa
.noread:
        mov     si, ax
        and     si, 0xFF
        add     si, si
        mov     ax, [es:si]
        pop     es
        cmp     ax, 0xFFF7
        pop     si
        ret

fat32_get_next_cluster:
; in: eax = cluster
; out: if there is next cluster: CF=1, eax = next cluster
; out: if there is no next cluster: CF=0
        push    di
        push    ax
        shr     eax, 7
; eax = FAT sector number; look in cache
        push    si
        mov     si, cache1head
        call    cache_lookup
        pop     si
        jnc     .noread
; read FAT, try all FATs if disk read error detected
        push    es
        pushad
        movzx   edx, word [ss:0x320E]   ; BPB_RsvdSecCnt
        add     eax, edx
        movzx   si, byte [ss:0x3210]    ; BPB_NumFATs
@@:
        lea     cx, [di - 0x3400 + (0x6000 shr (9-3))]
        shl     cx, 9-3
        mov     es, cx
        xor     bx, bx
        mov     cx, 1
        call    read
        jnc     @f
        add     eax, [ss:0x3224]        ; BPB_FATSz32
        dec     si
        jnz     @b
        jmp     ..found_disk_error
@@:
        popad
        pop     es
.noread:
; get requested item
        lea     ax, [di - 0x3400 + (0x6000 shr (9-3))]
        pop     di
        and     di, 0x7F
        shl     di, 2
        shl     ax, 9-3
        push    ds
        mov     ds, ax
        and     byte [di+3], 0x0F
        mov     eax, [di]
        pop     ds
        pop     di
        ;and    eax, 0x0FFFFFFF
        cmp     eax, 0x0FFFFFF7
        ret

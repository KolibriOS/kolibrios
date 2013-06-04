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

        jmp     far 0:real_start
; special text
org $+0x7C00
real_start:
; initialize
        xor     ax, ax
        mov     ss, ax
        mov     sp, 0x7C00
        mov     ds, ax
        mov     es, ax
        cld
        sti
        mov     [bootdrive], dl
; check LBA support
        mov     ah, 41h
        mov     bx, 55AAh
        int     13h
        mov     si, aNoLBA
        jc      err_
        cmp     bx, 0AA55h
        jnz     err_
        test    cl, 1
        jz      err_
; get file system information
; scan for Primary Volume Descriptor
        db      66h
        movi    eax, 10h-1
pvd_scan_loop:
        mov     cx, 1
        inc     eax
        mov     bx, 0x1000
        call    read_sectors
        jnc     @f
fatal_read_err:
        mov     si, aReadError
err_:
        call    out_string
        mov     si, aPressAnyKey
        call    out_string
        xor     ax, ax
        int     16h
        int     18h
        jmp     $
@@:
        push    ds
        pop     es
        cmp     word [bx+1], 'CD'
        jnz     pvd_scan_loop
        cmp     word [bx+3], '00'
        jnz     pvd_scan_loop
        cmp     byte [bx+5], '1'
        jnz     pvd_scan_loop
; we have found ISO9660 descriptor, look for type
        cmp     byte [bx], 1    ; Primary Volume Descriptor?
        jz      pvd_found
        cmp     byte [bx], 0xFF ; Volume Descriptor Set Terminator?
        jnz     pvd_scan_loop
; Volume Descriptor Set Terminator reached, no PVD found - fatal error
        mov     si, no_pvd
        jmp     err_
pvd_found:
        add     bx, 80h
        mov     ax, [bx]
        mov     [lb_size], ax
; calculate number of logical blocks in one sector
        mov     ax, 800h
        cwd
        div     word [bx]
        mov     [lb_per_sec], ax
; get location of root directory
        mov     di, root_location
        movzx   eax, byte [bx+1Dh]
        add     eax, [bx+1Eh]
        stosd
; get memory size
        int     12h
        mov     si, nomem_str
        cmp     ax, 71000h / 400h
        jb      err_
        shr     ax, 1
        sub     ax, 60000h / 800h
        mov     [size_rest], ax
        mov     [free_ptr], 60000h / 800h
; load path table
; if size > 62K => it's very strange, avoid using it
; if size > (size of cache)/2 => avoid using it too
        mov     ecx, [bx+4]
        cmp     ecx, 0x10000 - 0x800
        ja      nopathtable
        shr     ax, 1
        cmp     ax, 0x20
        jae     @f
        shl     ax, 11
        cmp     cx, ax
        ja      nopathtable
@@:
; size is ok, try to load it
        mov     [pathtable_size], cx
        mov     eax, [bx+12]
        xor     edx, edx
        div     dword [lb_per_sec]
        imul    dx, [bx]
        mov     [pathtable_start], dx
        add     cx, dx
        call    cx_to_sectors
        xor     bx, bx
        push    6000h
        pop     es
        call    read_sectors
        jc      nopathtable
; path table has been loaded
        inc     [use_path_table]
        sub     [size_rest], cx
        add     [free_ptr], cx
nopathtable:
; init cache
        mov     ax, [size_rest]
        mov     [cache_size], ax
        mov     ax, [free_ptr]
        mov     [cache_start], ax
; load secondary loader
        mov     di, secondary_loader_info
        call    load_file
        test    bx, bx
        jnz     noloader
; set registers for secondary loader
        mov     ah, [bootdrive]
        mov     al, 'c'
        mov     bx, 'is'
        mov     si, callback
        jmp     far [si-callback+secondary_loader_info] ; jump to 1000:0000

noloader:
        mov     si, aKernelNotFound
        jmp     err_

read_sectors:
; es:bx = pointer to data
; eax = first sector
; cx = number of sectors
        pushad
        push    ds
do_read_sectors:
        push    ax
        push    cx
        cmp     cx, 0x7F
        jbe     @f
        mov     cx, 0x7F
@@:
; create disk address packet on the stack
; dq starting LBA
        db      66h
        push    0
        push    eax
; dd buffer
        push    es
        push    bx
; dw number of blocks to transfer (no more than 0x7F)
        push    cx
; dw packet size in bytes
        push    10h
; issue BIOS call
        push    ss
        pop     ds
        mov     si, sp
        mov     dl, [cs:bootdrive]
        mov     ah, 42h
        int     13h
        jc      diskreaderr
; restore stack
        add     sp, 10h
; increase current sector & buffer; decrease number of sectors
        movzx   esi, cx
        mov     ax, es
        shl     cx, 7
        add     ax, cx
        mov     es, ax
        pop     cx
        pop     ax
        add     eax, esi
        sub     cx, si
        jnz     do_read_sectors
        pop     ds
        popad
        ret
diskreaderr:
        add     sp, 10h + 2*2
        pop     ds
        popad
        stc
out_string.ret:
        ret

out_string:
; in: ds:si -> ASCIIZ string
        lodsb
        test    al, al
        jz      .ret
        mov     ah, 0Eh
        mov     bx, 7
        int     10h
        jmp     out_string

aNoLBA          db      'The drive does not support LBA!',0
aReadError      db      'Read error',0
no_pvd          db      'Primary Volume Descriptor not found!',0
nomem_str       db      'No memory',0
aPressAnyKey    db      13,10,'Press any key...',13,10,0

load_file:
; in: ds:di -> information structure
;       dw:dw   address
;       dw      limit in 4Kb blocks (0x1000 bytes) (must be non-zero and not greater than 0x100)
;       ASCIIZ  name
; out: bx = status: bx=0 - ok, bx=1 - file is too big, only part of file was loaded, bx=2 - file not found
; out: dx:ax = file size (0xFFFFFFFF if file not found)
; parse path to the file
        lea     si, [di+6]
        mov     eax, [cs:root_location]
        cmp     [cs:use_path_table], 0
        jz      parse_dir
; scan for path in path table
        push    di
        push    6000h
        pop     es
        mov     di, [cs:pathtable_start]        ; es:di = pointer to current entry in path table
        mov     dx, 1   ; dx = number of current entry in path table, start from 1
        mov     cx, [cs:pathtable_size]
pathtable_newparent:
        mov     bx, dx  ; bx = number of current parent in path table: root = 1
scan_path_table_e:
        call    is_last_component
        jnc     path_table_scanned
scan_path_table_i:
        cmp     word [es:di+6], bx
        jb      .next
        ja      path_table_notfound
        call    test_filename1
        jc      .next
@@:
        lodsb
        cmp     al, '/'
        jnz     @b
        jmp     pathtable_newparent
.next:
; go to next entry
        inc     dx
        movzx   ax, byte [es:di]
        add     ax, 8+1
        and     al, not 1
        add     di, ax
        sub     cx, ax
        ja      scan_path_table_i
path_table_notfound:
        pop     di
        mov     ax, -1
        mov     dx, ax
        mov     bx, 2   ; file not found
        ret
path_table_scanned:
        movzx   eax, byte [es:di+1]
        add     eax, [es:di+2]
        pop     di
parse_dir:
; eax = logical block, ds:di -> information structure, ds:si -> file name
; was the folder already read?
        push    di ds
        push    cs
        pop     ds
        mov     [cur_desc_end], 2000h
        mov     bx, cachelist
.scan1:
        mov     bx, [bx+2]
        cmp     bx, cachelist
        jz      .notfound
        cmp     [bx+4], eax
        jnz     .scan1
.found:
; yes; delete this item from the list (the following code will append this item to the tail)
        mov     di, [bx]
        push    word [bx+2]
        pop     word [di+2]
        mov     di, [bx+2]
        push    word [bx]
        pop     word [di]
        mov     di, bx
        jmp     .scan
.notfound:
; no; load first sector of the folder to get its size
        push    eax
        push    si
        mov     si, 1
        call    load_phys_sector_for_lb_force
        mov     bx, si
        pop     si
        pop     eax
        jnc     @f
; read error - return
.readerr:
        pop     ds
.readerr2:
        pop     di
        mov     ax, -1
        mov     dx, ax
        mov     bx, 3
        ret
@@:
; first item of the folder describes the folder itself
; do not cache too big folders: size < 64K and size <= (total cache size)/2
        cmp     word [bx+12], 0
        jnz     .nocache
        mov     cx, [cache_size]        ; cx = cache size in sectors
        shr     cx, 1                   ; cx = (cache size)/2
        cmp     cx, 0x20
        jae     @f
        shl     cx, 11
        cmp     [bx+10], cx
        ja      .nocache
@@:
; we want to cache this folder; get space for it
        mov     cx, [bx+10]
        call    cx_to_sectors
        jnz     .yescache
.nocache:
        push    dword [bx+10]
        pop     dword [cur_nocache_len]
        call    lb_to_sector
        push    ds
        pop     es
        pop     ds
.nocache_loop:
        push    eax
        mov     dx, 1800h
        call    scan_for_filename_in_sector
        mov     cx, dx
        pop     eax
        jnc     .j_scandone
        sub     cx, bx
        sub     word [es:cur_nocache_len], cx
        sbb     word [es:cur_nocache_len+2], 0
        jb      .j_scandone
        ja      @f
        cmp     word [es:cur_nocache_len], 0
        jz      .j_scandone
@@:
        mov     cx, 1
        inc     eax
        push    es
        mov     bx, 1000h
        call    read_sectors
        pop     es
        jc      .readerr2
        jmp     .nocache_loop
.j_scandone:
        jmp     .scandone
.yescache:
        push    bx
        mov     bx, [cachelist.head]
.freeloop:
        cmp     cx, [size_rest]
        jbe     .sizeok
@@:
; if we are here: there is not enough free space, so we must delete old folders' data
; N.B. We know that after deleting some folders the space will be available (size <= (total cache size)/2).
; one loop iteration: delete data of one folder
        pusha
        mov     dx, [bx+10]
        mov     es, dx          ; es = segment of folder data to be deleted
        xor     di, di
        mov     ax, [bx+8]
        add     ax, 0x7FF
        rcr     ax, 1
        shr     ax, 10
        push    ax
        shl     ax, 11-4        ; get number of paragraphs in folder data to be deleted
        mov     cx, [cache_size]
        add     cx, [cache_start]
        push    ds
        push    ax
        add     ax, dx
        mov     ds, ax
        pop     ax
        shl     cx, 11-4
        sub     cx, dx          ; cx = number of paragraphs to be moved
        push    si
        xor     si, si
; move cx paragraphs from ds:si to es:di to get free space in the end of cache
@@:
        sub     cx, 1000h
        jbe     @f
        push    cx
        mov     cx, 8000h
        rep movsw
        mov     cx, ds
        add     cx, 1000h
        mov     ds, cx
        mov     cx, es
        add     cx, 1000h
        mov     es, cx
        pop     cx
        jmp     @b
@@:
        add     cx, 1000h
        shl     cx, 3
        rep movsw
        pop     si
        pop     ds
; correct positions in cache for existing items
        mov     cx, 80h
        mov     di, 8400h
.correct:
        cmp     [di+10], dx
        jbe     @f
        sub     [di+10], ax
@@:
        add     di, 12
        loop    .correct
; some additional space is free now
        pop     ax
        add     [size_rest], ax
        sub     [free_ptr], ax
; add cache item to the list of free items
        mov     dx, [bx]
        mov     ax, [free_cache_item]
        mov     [bx], ax
        mov     [free_cache_item], bx
        mov     bx, dx
; current iteration done
        popa
        jmp     .freeloop
.sizeok:
        mov     [cachelist.head], bx
        mov     word [bx+2], cachelist
; allocate new item in cache
        mov     di, [free_cache_item]
        test    di, di
        jz      .nofree
        push    word [di]
        pop     [free_cache_item]
        jmp     @f
.nofree:
        mov     di, [last_cache_item]
        add     [last_cache_item], 12
@@:
        pop     bx
        push    si di
;       mov     [di+4], eax     ; start of folder
        scasd
        stosd
        push    ax
        mov     ax, [free_ptr]
        shl     ax, 11-4
        mov     [di+10-8], ax
        mov     es, ax
        pop     ax
        add     [free_ptr], cx
        sub     [size_rest], cx
; read folder data
; first sector is already in memory, 0000:bx
        pusha
        mov     cx, [bx+10]
        mov     [di+8-8], cx    ; folder size in bytes
        mov     si, bx
        xor     di, di
        mov     cx, 0x1800
        sub     cx, si
        rep movsb
        pop     ax
        push    di
        popa
; read rest of folder
        mov     esi, dword [lb_per_sec]
        add     eax, esi
        dec     si
        not     si
        and     ax, si
        mov     si, word [bx+10]
        mov     bx, di
        pop     di
        sub     si, bx
        jbe     @f
        mov     [cur_limit], esi
        call    read_many_bytes
        pop     si
        jnc     .scan
        jmp     .readerr
@@:
        pop     si
.scan:
; now we have required cache item; append it to the end of list
        mov     bx, [cachelist.tail]
        mov     [cachelist.tail], di
        mov     [di+2], bx
        mov     word [di], cachelist
        mov     [bx], di
; scan for given filename
        mov     es, [di+10]
        mov     dx, [di+8]
        pop     ds
        xor     bx, bx
        call    scan_for_filename_in_sector
.scandone:
        push    cs
        pop     es
        mov     bx, 2000h
        cmp     bx, [es:cur_desc_end]
        jnz     filefound
j_notfound:
        jmp     path_table_notfound
filefound:
@@:
        lodsb
        test    al, al
        jz      @f
        cmp     al, '/'
        jnz     @b
@@:
        mov     cl, [es:bx+8]
        test    al, al
        jz      @f
; parse next component of file name
        test    cl, 2   ; directory?
        jz      j_notfound
        mov     eax, [es:bx]
        pop     di
        jmp     parse_dir
@@:
        test    cl, 2   ; directory?
        jnz     j_notfound      ; do not allow read directories as regular files
; ok, now load the file
        pop     di
        les     bx, [di]
        call    normalize
        movzx   esi, word [di+4]        ; esi = limit in 4K blocks
        shl     esi, 12         ; esi = limit in bytes
        push    cs
        pop     ds
        mov     [cur_limit], esi
        mov     di, 2000h
loadloop:
        and     [cur_start], 0
.loadnew:
        mov     esi, [cur_limit]
        mov     eax, [cur_start]
        add     esi, eax
        mov     [overflow], 1
        sub     esi, [di+4]
        jb      @f
        xor     esi, esi
        dec     [overflow]
@@:
        add     esi, [di+4]     ; esi = number of bytes to read
        mov     [cur_start], esi
        sub     esi, eax
        jz      .loadcontinue
        xor     edx, edx
        div     dword [lb_size] ; eax = number of logical blocks to skip,
        mov     [first_byte], dx; [first_byte] = number of bytes to skip in 1st block
        cmp     byte [di+10], 0
        jnz     .interleaved
        add     eax, [di]
; read esi bytes from logical block eax to buffer es:bx
        call    read_many_bytes.with_first
        jc      .readerr3
.loadcontinue:
        mov     [cur_chunk], di
        add     di, 11
        cmp     di, [cur_desc_end]
        jae     @f
        cmp     [cur_limit], 0
        jnz     loadloop
@@:
        mov     bx, [overflow]
.calclen:
; calculate length of file
        xor     ax, ax
        xor     dx, dx
        mov     di, 2000h
@@:
        add     ax, [di+4]
        adc     dx, [di+6]
        add     di, 11
        cmp     di, [cur_desc_end]
        jb      @b
        ret
.interleaved:
        mov     [cur_unit_limit], esi
        push    esi
; skip first blocks
        movzx   ecx, byte [di+9]        ; Unit Size
        movzx   esi, byte [di+10]       ; Interleave Gap
        add     si, cx
        mov     edx, [di]
@@:
        sub     eax, ecx
        jb      @f
        add     edx, esi
        jmp     @b
@@:
        add     ecx, eax        ; ecx = number of logical blocks to skip
        lea     eax, [ecx+edx]  ; eax = first logical block
        pop     esi
.interleaved_loop:
; get number of bytes in current file unit
        push    eax
        movzx   eax, byte [di+9]
        sub     ax, cx
        imul    eax, dword [lb_size]
        cmp     eax, esi
        ja      .i2
.i1:
        xchg    esi, eax
.i2:
        pop     eax
        sub     [cur_unit_limit], esi
        push    eax
; read esi bytes from logical block eax to buffer es:bx
        call    read_many_bytes.with_first
        pop     eax
        jnc     @f
.readerr3:
        mov     bx, 3
        jmp     .calclen
@@:
        mov     esi, [cur_unit_limit]
        test    esi, esi
        jz      .loadcontinue
        movzx   ecx, byte [di+9]        ; add Unit Size
        add     cl, byte [di+10]        ; add Interleave Gap
        adc     ch, 0
        add     eax, ecx
        xor     cx, cx
        mov     [first_byte], cx
        jmp     .interleaved_loop

cx_to_sectors:
        add     cx, 7FFh
        rcr     cx, 1
        shr     cx, 10
        ret

is_last_component:
; in: ds:si -> name
; out: CF set <=> current component is not last (=> folder)
        push    si
@@:
        lodsb
        test    al, al
        jz      @f
        cmp     al, '/'
        jnz     @b
        stc
@@:
        pop     si
        ret

test_filename1:
; in: ds:si -> filename, es:di -> path table item
; out: CF set <=> no match
        pusha
        mov     cl, [es:di]
        add     di, 8
        jmp     test_filename2.start
test_filename2:
; in: ds:si -> filename, es:bx -> directory item
; out: CF set <=> no match
        pusha
        mov     cl, [es:bx+32]
        lea     di, [bx+33]
.start:
        mov     ch, 0
@@:
        lodsb
        test    al, al
        jz      .test1
        cmp     al, '/'
        jz      .test1
        call    toupper
        mov     ah, al
        mov     al, [es:di]
        call    toupper
        inc     di
        cmp     al, ah
        loopz   @b
        jnz     .next1
; if we have reached this point: current name is done
        lodsb
        test    al, al
        jz      .ret
        cmp     al, '/'
        jz      .ret
; if we have reached this point: current name is done, but input name continues
; so they do not match
        jmp     .next1
.test1:
; if we have reached this point: input name is done, but current name continues
; "filename.ext;version" in ISO-9660 represents file "filename.ext"
; "filename." and "filename.;version" are also possible for "filename"
        cmp     byte [es:di], '.'
        jnz     @f
        inc     di
        dec     cx
        jz      .ret
@@:
        cmp     byte [es:di], ';'
        jnz     .next1
        jmp     .ret
.next1:
        stc
.ret:
        popa
        ret

toupper:
; in: al=symbol
; out: al=symbol in uppercase
        cmp     al, 'a'
        jb      .ret
        cmp     al, 'z'
        ja      .ret
        sub     al, 'a'-'A'
.ret:
        ret

scan_for_filename_in_sector:
; in: ds:si->filename, es:bx->folder data, dx=limit
; out: CF=0 if found
        push    bx
.loope:
        push    bx
.loop:
        cmp     bx, dx
        jae     .notfound
        cmp     byte [es:bx], 0
        jz      .loopd
        test    byte [es:bx+25], 4      ; ignore files with Associated bit
        jnz     .next
        call    test_filename2
        jc      .next
        push    ds es di
        push    es
        pop     ds
        push    cs
        pop     es
        mov     di, [es:cur_desc_end]
        movzx   eax, byte [bx+1]
        add     eax, [bx+2]
        stosd   ; first logical block
        mov     eax, [bx+10]
        stosd   ; length
        mov     al, [bx+25]
        stosb   ; flags
        mov     ax, [bx+26]
        stosw   ; File Unit size, Interleave Gap size
        mov     [es:cur_desc_end], di
        cmp     di, 3000h
        pop     di es ds
        jae     .done
        test    byte [es:bx+25], 80h
        jz      .done
.next:
        add     bl, [es:bx]
        adc     bh, 0
        jmp     .loop
.loopd:
        mov     ax, bx
        pop     bx
@@:
        add     bx, [cs:lb_size]
        jz      .done2
        cmp     bx, ax
        jb      @b
        jmp     .loope
.notfound:
        stc
.done:
        pop     bx
.done2:
        pop     bx
        ret

lb_to_sector:
        xor     edx, edx
        div     dword [lb_per_sec]
        ret

load_phys_sector_for_lb_force:
; in: eax = logical block, ds=0
; in: si=0 - accept 0 logical blocks, otherwise force read at least 1
; out: 0000:1000 = physical sector data; si -> logical block
; out: eax = next physical sector
; out: CF=1 if read error
; destroys cx
; this procedure reads 0-3 or 1-4 logical blocks, up to the end of physical sector
        call    lb_to_sector
        or      si, dx
        jnz     @f
        mov     si, 1800h
        jmp     .done
@@:
        mov     si, 1000h
        imul    dx, [lb_size]
        add     si, dx
        mov     cx, 1
        push    es bx
        push    ds
        pop     es
        mov     bx, 1000h
        call    read_sectors
        pop     bx es
        inc     eax
.done:
        ret

normalize:
; in: es:bx = far pointer
; out: es:bx = normalized pointer (i.e. 0 <= bx < 0x10)
        push    ax bx
        mov     ax, es
        shr     bx, 4
        add     ax, bx
        mov     es, ax
        pop     bx ax
        and     bx, 0x0F
        ret

read_many_bytes:
        and     [first_byte], 0
read_many_bytes.with_first:
; read esi bytes from logical block dx:ax to buffer es:bx
; out: CF=1 <=> disk error
        push    di
; load first physical sector
        push    bx si
        mov     si, [first_byte]
        call    load_phys_sector_for_lb_force
        jnc     @f
        pop     si bx
.ret:
        pop     di
        ret
@@:
        add     si, [first_byte]
        mov     ecx, 1800h
        sub     cx, si
        mov     ebx, esi
        pop     bx
        sub     ebx, ecx
        jnc     @f
        add     cx, bx
        xor     ebx, ebx
@@:
        pop     di
        sub     [cur_limit], ecx
        rep movsb
        mov     esi, ebx
        mov     bx, di
        call    normalize
; load other physical sectors
; read esi bytes from physical sector eax to buffer es:bx
        test    esi, esi
        jz      .ret
        push    esi
        add     esi, 0x7FF
        and     si, not 0x7FF
        cmp     esi, [cur_limit]
        jbe     .okplace
.noplace:
        sub     esi, 800h
.okplace:
        shr     esi, 11 ; si = number of sectors
        mov     cx, si
        jz      @f
        call    read_sectors
@@:
        pop     esi
        jc      .ret
        movzx   ecx, cx
        add     eax, ecx
        shl     ecx, 11
        sub     [cur_limit], ecx
        sub     esi, ecx
        jc      .big
        jz      .nopost
        push    bx es
        push    ds
        pop     es
        mov     bx, 1000h
        mov     cx, 1
        call    read_sectors
        pop     es di
        jc      .ret2
        mov     cx, si
        mov     si, 1000h
        sub     word [cur_limit], cx
        sbb     word [cur_limit+2], 0
        rep movsb
        mov     bx, di
        call    normalize
.nopost:
        clc
.ret2:
        pop     di
        ret
.big:
        mov     ax, es
        sub     ax, 80h
        mov     es, ax
        add     bx, 800h
        add     bx, si
        call    normalize
        sub     [cur_limit], esi
        jmp     .nopost

; Callback function for secondary loader
callback:
; in: ax = function number; only function 1 is defined for now
        dec     ax
        jz      callback_readfile
        dec     ax
        jz      callback_continueread
        stc     ; unsupported function
        retf

callback_readfile:
; function 1: read file
; in: ds:di -> information structure
;       dw:dw   address
;       dw      limit in 4Kb blocks (0x1000 bytes) (must be non-zero and not greater than 0x100)
;       ASCIIZ  name
; out: bx=0 - ok, bx=1 - file is too big, only part of file was loaded, bx=2 - file not found, bx=3 - read error
; out: dx:ax = file size (0xFFFFFFFF if file was not found)
        call    load_file
        clc     ; function is supported
        retf

callback_continueread:
; function 2: continue to read file
; in: ds:di -> information structure
;       dw:dw   address
;       dw      limit in 4Kb blocks (0x1000 bytes) (must be non-zero and not greater than 0x100)
; out: bx=0 - ok, bx=1 - file is too big, only part of file was loaded, bx=3 - read error
; out: dx:ax = file size
        les     bx, [di]
        call    normalize
        movzx   esi, word [di+4]        ; si = limit in 4K blocks
        shl     esi, 12                 ; bp:si = limit in bytes
        push    cs
        pop     ds
        mov     [cur_limit], esi
        mov     di, [cur_chunk]
        call    loadloop.loadnew
        clc     ; function is supported
        retf

secondary_loader_info:
        dw      0, 0x1000
        dw      0x30000 / 0x1000
        db      'kord/loader',0
aKernelNotFound db      'Fatal error: cannot load the secondary loader',0

align 2
cachelist:
.head           dw      cachelist
.tail           dw      cachelist
free_cache_item dw      0
last_cache_item dw      0x8400

use_path_table  db      0
bootdrive       db      ?
align 2
lb_size         dw      ?       ; Logical Block size in bytes
                dw      0       ; to allow access dword [lb_size]
lb_per_sec      dw      ?       ; Logical Blocks per physical sector
                dw      0       ; to allow access dword [lb_per_sec]
free_ptr        dw      ?       ; first free block in cache (cache block = sector = 0x800 bytes)
size_rest       dw      ?       ; free space in cache (in blocks)
cache_size      dw      ?
cache_start     dw      ?
pathtable_size  dw      ?
pathtable_start dw      ?
root_location   dd      ?
cur_desc_end    dw      ?
cur_nocache_len dd      ?
cur_limit       dd      ?
cur_unit_limit  dd      ?
overflow        dw      ?
cur_chunk       dw      ?
first_byte      dw      ?
cur_start       dd      ?

;times 83FEh-$  db      0
        db      43h
; just to make file 2048 bytes long :)
        db      'd' xor 'i' xor 'a' xor 'm' xor 'o' xor 'n' xor 'd'

        dw      0xAA55          ; this is not required for CD, but to be consistent...

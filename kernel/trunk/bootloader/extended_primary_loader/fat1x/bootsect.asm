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

use_lba = 0
        org     0x7C00
        jmp     start
        nop
; FAT parameters, BPB
; note: they can be changed at install, replaced with real values
; these settings are for most typical 1.44M floppies
                db      'KOLIBRI '      ; BS_OEMName, ignored
                dw      200h            ; BPB_BytsPerSec
BPB_SecsPerClus db      1
BPB_RsvdSecCnt  dw      1
BPB_NumFATs     db      2
BPB_RootEntCnt  dw      0xE0
                dw      2880            ; BPB_TotSec16
                db      0xF0            ; BPB_Media
BPB_FATSz16     dw      9
BPB_SecPerTrk   dw      18
BPB_NumHeads    dw      2
BPB_HiddSec     dd      0
                dd      0               ; BPB_TotSec32
BS_DrvNum       db      0
                db      0               ; BS_Reserved1
                db      ')'             ; BS_BootSig
                dd      12344321h       ; BS_VolID
filename:
                db      'KORD.OS    '   ; BS_VolLab
                db      'FAT12   '      ; BS_FilSysType
; Used memory map:
;       8000:0000 - current directory
;       9000:0000 - root directory data [cached]
start:
        xor     ax, ax
        mov     ss, ax
        mov     sp, 0x7C00
        mov     ds, ax
        mov     bp, sp
        cld
        sti
        mov     [bp+BS_DrvNum-0x7C00], dl
if use_lba
        mov     ah, 41h
        mov     bx, 55AAh
        int     13h
        mov     si, aNoLBA
        jc      err_
        cmp     bx, 0AA55h
        jnz     err_
        test    cx, 1
        jz      err_
else
        mov     ah, 8
        int     13h
        jc      @f              ; on error, assume that BPB geometry is valid
        mov     al, dh
        mov     ah, 0
        inc     ax
        mov     [bp+BPB_NumHeads-0x7C00], ax
        and     cx, 3Fh
        mov     [bp+BPB_SecPerTrk-0x7C00], cx
@@:
end if
; get FAT parameters
        xor     bx, bx
        mov     al, [bp+BPB_NumFATs-0x7C00]
        mov     ah, 0
        mul     [bp+BPB_FATSz16-0x7C00]
        add     ax, [bp+BPB_RsvdSecCnt-0x7C00]
        adc     dx, bx
        push    dx
        push    ax      ; root directory start = dword [bp-4]
        mov     cx, [bp+BPB_RootEntCnt-0x7C00]
        add     cx, 0xF
        rcr     cx, 1
        shr     cx, 3   ; cx = size of root directory in sectors
        add     ax, cx
        adc     dx, bx
        push    dx
        push    ax      ; data start = dword [bp-8]
; load start of root directory (no more than 0x2000 bytes = 0x10 sectors)
        cmp     cx, 0x10
        jb      @f
        mov     cx, 0x10
@@:
        mov     ax, [bp-4]
        mov     dx, [bp-2]
        push    0x9000
        pop     es
        call    read_sectors
        add     word [bp-4], cx         ; dword [bp-4] = start of non-cached root data
        adc     word [bp-2], bx
; load kordldr.f12
        mov     si, main_loader
        call    lookup_in_root_dir
        jc      noloader
        test    byte [es:di+11], 10h    ; directory?
        jz      kordldr_ok
noloader:
        mov     si, aLoaderNotFound
err_:
        call    out_string
        mov     si, aPressAnyKey
        call    out_string
        xor     ax, ax
        int     16h
        int     18h
        jmp     $
kordldr_ok:
        mov     ax, [es:di+26]          ; get file cluster
        mov     bx, 0x7E00
        xor     cx, cx
        mov     es, cx
        sub     ax, 2
        jc      noloader
        push    bx      ; save return address: bx = 7E00
        mov     cl, [bp+BPB_SecsPerClus-0x7C00]
        mul     cx
; fall through - 'ret' in read_sectors will return to 7E00

read_sectors2:
; same as read_sectors, but dx:ax is relative to start of data
        add     ax, [bp-8]
        adc     dx, [bp-6]
read_sectors:
; ss:bp = 0:7C00
; es:bx = pointer to data
; dx:ax = first sector
; cx = number of sectors
        pusha
        add     ax, word [bp+BPB_HiddSec-0x7C00]
        adc     dx, word [bp+BPB_HiddSec+2-0x7C00]
if use_lba
        push    ds
do_read_sectors:
        push    ax
        push    cx
        push    dx
        cmp     cx, 0x7F
        jbe     @f
        mov     cx, 0x7F
@@:
; create disk address packet on the stack
; dq starting LBA
        push    0
        push    0
        push    dx
        push    ax
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
        mov     dl, [bp+BS_DrvNum-0x7C00]
        mov     ah, 42h
        int     13h
        mov     si, aReadError
        jc      err_
; restore stack
        add     sp, 10h
; increase current sector & buffer; decrease number of sectors
        mov     si, cx
        mov     ax, es
        shl     cx, 5
        add     ax, cx
        mov     es, ax
        pop     dx
        pop     cx
        pop     ax
        add     ax, si
        adc     dx, 0
        sub     cx, si
        jnz     do_read_sectors
        pop     ds
        popa
        ret
else
do_read_sectors:
        pusha
        pop     di
        push    bx

; (dword in dx:ax) / (SectorsPerTrack) -> (dword in dx:ax), remainder bx
        mov     si, ax
        xchg    ax, dx
        xor     dx, dx
        div     [bp+BPB_SecPerTrk-0x7C00]
        push    ax
        mov     ax, si
        div     [bp+BPB_SecPerTrk-0x7C00]
        mov     bx, dx          ; bx=sector-1
        pop     dx

; (dword in dx:ax) / (NumHeads) -> (word in ax), remainder dx
        div     [bp+BPB_NumHeads-0x7C00]

; number of sectors: read no more than to end of track
        push    bx
        sub     bx, [bp+BPB_SecPerTrk-0x7C00]
        neg     bx
        cmp     cx, bx
        jbe     @f
        mov     cx, bx
@@:
        pop     bx

        inc     bx
; now ax=track, dl=head, dh=0, cl=number of sectors, ch=0, bl=sector; convert to int13 format
        mov     di, cx
        mov     dh, dl
        mov     dl, [bp+BS_DrvNum-0x7C00]
        shl     ah, 6
        mov     ch, al
        mov     al, cl
        mov     cl, bl
        or      cl, ah
        pop     bx
        mov     si, 3
        mov     ah, 2
@@:
        push    ax
        int     13h
        jnc     @f
        xor     ax, ax
        int     13h     ; reset drive
        pop     ax
        dec     si
        jnz     @b
        mov     si, aReadError
        jmp     err_
@@:
        pop     ax
        mov     ax, es
        mov     cx, di
        shl     cx, 5
        add     ax, cx
        mov     es, ax
        push    di
        popa
        add     ax, di
        adc     dx, 0
        sub     cx, di
        jnz     do_read_sectors
        popa
        ret
end if

scan_for_filename:
; in: ds:si -> 11-bytes FAT name
; in: es:0 -> part of directory data
; in: cx = number of entries
; out: if found: CF=0, ZF=1, es:di -> directory entry
; out: if not found, but continue required: CF=1 and ZF=0
; out: if not found and zero item reached: CF=1 and ZF=1
        xor     di, di
        push    cx
sloop:
        cmp     byte [es:di], 0
        jz      snotfound
        test    byte [es:di+11], 8      ; volume label?
        jnz     scont                   ; ignore volume labels
        pusha
        mov     cx, 11
        repz    cmpsb
        popa
        jz      sdone
scont:
        add     di, 0x20
        loop    sloop
        inc     cx      ; clear ZF flag
snotfound:
        stc
sdone:
        pop     cx
lrdret:
        ret

lookup_in_root_dir:
; ss:bp = 0:7C00
; in: ds:si -> 11-bytes FAT name
; out: if found: CF=0, es:di -> directory entry
; out: if not found: CF=1
        mov     cx, [bp+BPB_RootEntCnt-0x7C00]
        push    cx
; first, look in root directory cache
        push    0x9000
        pop     es
        test    ch, ch
        jz      @f
        mov     cx, 0x100
@@:
        mov     ax, [bp-4]
        mov     dx, [bp-2]      ; dx:ax = starting sector of not cached data of root directory
lrdloop:
        call    scan_for_filename
        pop     bx
        jz      lrdret
        sub     bx, cx
        mov     cx, bx
        stc
        jz      lrdret
; read no more than 0x10000 bytes, or 0x10000/0x20 = 0x800 entries
        push    cx
        cmp     ch, 0x8
        jb      @f
        mov     cx, 0x800
@@:
        push    0x8000
        pop     es
        push    cx
        push    es
        xor     bx, bx
        add     cx, 0xF
        shr     cx, 4
        call    read_sectors
        pop     es
        add     ax, cx
        adc     dx, bx
        pop     cx
        jmp     lrdloop

out_string:
; in: ds:si -> ASCIIZ string
        lodsb
        test    al, al
        jz      lrdret
        mov     ah, 0Eh
        mov     bx, 7
        int     10h
        jmp     out_string

aReadError      db      'Read error',0
if use_lba
aNoLBA          db      'The drive does not support LBA!',0
end if
aLoaderNotFound db      'Loader not found',0
aPressAnyKey    db      13,10,'Press any key...',13,10,0
main_loader     db      'KORDLDR F1X'

if use_lba
        db      0       ; make bootsector 512 bytes in length
end if

; bootsector signature
        dw      0xAA55

; display offsets of all procedures used by kordldr.f12.asm
macro show [procedure]
{
        bits = 16
        display `procedure,' = '
        repeat bits/4
                d = '0' + procedure shr (bits - %*4) and 0Fh
                if d > '9'
                        d = d + 'A'-'9'-1
                end if
                display d
        end repeat
        display 13,10
}

show read_sectors, read_sectors2, lookup_in_root_dir, scan_for_filename, err_, noloader

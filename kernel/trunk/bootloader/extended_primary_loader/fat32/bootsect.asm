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
; they must be changed at install, replaced with real values
                rb      8       ; BS_OEMName, ignored
                dw      200h    ; BPB_BytsPerSec
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
                dw      ?       ; BPB_FSInfo
BPB_BkBootSec   dw      ?
                rb      12      ; BPB_Reserved
BS_DrvNum       db      ?
                db      ?       ; BS_Reserved1
                db      ?       ; BS_BootSig
                dd      ?       ; BS_VolID
                rb      11      ; BS_VolLab
                rb      8       ;

curseg  dw      0x8000

start:
        xor     ax, ax
        mov     ss, ax
        mov     sp, 0x7C00
        mov     ds, ax
        mov     bp, sp
        cld
        sti
        push    dx      ; byte [bp-2] = boot drive
if use_lba
        mov     ah, 41h
        mov     bx, 55AAh
        int     13h
        mov     si, aNoLBA
        jc      err_
        cmp     bx, 0AA55h
        jnz     err_
        test    cl, 1
        jz      err_
else
        mov     ah, 8
        int     13h
        jc      @f
        movzx   ax, dh
        inc     ax
        mov     [bp+BPB_NumHeads-0x7C00], ax
        and     cx, 3Fh
        mov     [bp+BPB_SecPerTrk-0x7C00], cx
@@:
end if
; get FAT parameters
        xor     bx, bx
        movzx   eax, [bp+BPB_NumFATs-0x7C00]
        mul     [bp+BPB_FATSz32-0x7C00]
        movzx   ecx, [bp+BPB_RsvdSecCnt-0x7C00]
        push    ecx     ; FAT start = dword [bp-6]
        add     eax, ecx
        push    eax     ; data start = dword [bp-10]
        ;push   dword -1        ; dword [bp-14] = current sector for FAT cache
        db      66h
        push    -1      ; dword [bp-14] = current sector for FAT cache
        mov     eax, [bp+BPB_RootClus-0x7C00]
        mov     si, main_loader
        call    lookup_in_dir
        jnc     kordldr_ok
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
        mov     eax, [es:di+20-2]       ; hiword(eax) = hiword(cluster)
        mov     ax, [es:di+26]          ; loword(eax) = loword(cluster)
        mov     es, bx          ; es = 0
        mov     bx, 0x7E00
        push    bx      ; save return address: bx = 7E00
; fall through - 'ret' in read_cluster will return to 7E00

read_cluster:
; ss:bp = 0:7C00
; es:bx = pointer to data
; eax = cluster
        sub     eax, 2
        movzx   ecx, [bp+BPB_SecsPerClus-0x7C00]
        mul     ecx

read_sectors2:
; same as read_sectors32, but eax is relative to start of data
        add     eax, [bp-10]
read_sectors32:
; ss:bp = 0:7C00
; es:bx = pointer to data
; eax = first sector
; cx = number of sectors
; some high words of 32-bit registers are destroyed!
        pusha
        add     eax, [bp+BPB_HiddSec-0x7C00]
if use_lba
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
        push    0
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
        mov     dl, [bp-2]
        mov     ah, 42h
        int     13h
        mov     si, aReadError
        jc      err_
; restore stack
        add     sp, 10h
; increase current sector & buffer; decrease number of sectors
        movzx   esi, cx
        mov     ax, es
        shl     cx, 5
        add     ax, cx
        mov     es, ax
        pop     cx
        pop     ax
        add     eax, esi
        sub     cx, si
        jnz     do_read_sectors
        pop     ds
        popa
        ret
else
do_read_sectors:
        pusha
        pop     edi     ; loword(edi) = di, hiword(edi) = si
        push    bx

; eax / (SectorsPerTrack) -> eax, remainder bx
        movzx   esi, [bp+BPB_SecPerTrk-0x7C00]
        xor     edx, edx
        div     esi
        mov     bx, dx          ; bx=sector-1

; eax -> dx:ax
        push    eax
        pop     ax
        pop     dx
; (dword in dx:ax) / (NumHeads) -> (word in ax), remainder dx
        div     [bp+BPB_NumHeads-0x7C00]

; number of sectors: read no more than to end of track
        sub     si, bx
        cmp     cx, si
        jbe     @f
        mov     cx, si
@@:

        inc     bx
; now ax=track, dl=head, dh=0, cl=number of sectors, ch=0, bl=sector; convert to int13 format
        movzx   edi, cx
        mov     dh, dl
        mov     dl, [bp-2]
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
        push    edi
        popa
        add     eax, edi
        sub     cx, di
        jnz     do_read_sectors
        popa
        ret
end if

lookup_in_dir:
; in: ds:si -> 11-bytes FAT name
; in: eax = cluster
; in: bx = 0
; out: if found: CF=0, es:di -> directory entry
; out: if not found: CF=1
;       push    0x8000
;       pop     es
; read current cluster: first cluster goes to 8000:0000, others - to 8200:0000
        mov     es, [bp-7C00h + curseg]
        push    es
        push    eax
        call    read_cluster
        mov     ax, es
        cmp     ah, 82h
        jb      @f
        mov     ax, 8200h
@@:
        mov     [bp-7C00h + curseg], ax
        pop     eax
        pop     es
; scan for filename
        shl     cx, 4
        xor     di, di
sloop:
        cmp     byte [es:di], bl
        jz      snotfound
        test    byte [es:di+11], 8      ; volume label?
        jnz     scont                   ; ignore volume labels
        pusha
        mov     cx, 11
        repz cmpsb
        popa
        jz      sdone
scont:
        add     di, 0x20
        loop    sloop
; next cluster
        push    0x6000
        pop     es
        push    es ax
        shr     eax, 7
        cmp     eax, [bp-14]
        mov     [bp-14], eax
        jz      @f
        add     eax, [bp-6]
        mov     cx, 1
        call    read_sectors32
@@:
        pop     di es
        and     di, 0x7F
        shl     di, 2
        and     byte [es:di+3], 0x0F
        mov     eax, [es:di]
        ;and    eax, 0x0FFFFFFF
        cmp     eax, 0x0FFFFFF7
        jb      lookup_in_dir
snotfound:
        stc
sdone:
        ret

out_string:
; in: ds:si -> ASCIIZ string
        lodsb
        test    al, al
        jz      sdone
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
main_loader     db      'KORDLDR F32'

        db      56h
; just to make file 512 bytes long :)
        db      'd' xor 'i' xor 'a' xor 'm' xor 'o' xor 'n' xor 'd'

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

show read_sectors32, read_sectors2, err_, noloader

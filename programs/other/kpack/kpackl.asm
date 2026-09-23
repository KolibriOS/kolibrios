; kpackl = Kolibri Packer, native Linux port (no output, in-place compressor)
;
; Usage:  kpackl <file>
;   The file is compressed in place (overwritten).
;   If the file name is "kernel.mnt" it is packed with the "Kernel" option
;   (self-extracting KERNPACK format), otherwise the normal KPCK format is used.
;   The program prints nothing; on any problem it just leaves the file alone.
;
; Self-contained ELF32 using raw Linux int 0x80 syscalls - no libc, no linker.
; Compression core (LZMA + calltrick) and the self-extractor stub are the
; original KolibriOS kpack sources by diamond; only the OS glue is new.

format ELF executable 3
entry _start

segment readable writeable executable

use_lzma        = 1
use_calltrick1  = 40h
use_calltrick2  = 80h

; Linux i386 syscalls
sys_exit  = 1
sys_read  = 3
sys_write = 4
sys_open  = 5
sys_close = 6
sys_lseek = 19
sys_mmap2 = 192

_start:
; ---- get the file argument (argv[1]) ------------------------------
        mov     eax, [esp]              ; argc
        cmp     eax, 2
        jb      quit
        mov     esi, [esp+8]            ; argv[1]
        mov     [infilename], esi
; ---- is the base name "kernel.mnt" ? ------------------------------
        mov     ebx, esi                ; ebx = start of base name
.bn:
        mov     al, [esi]
        test    al, al
        jz      .bn_done
        cmp     al, '/'
        jne     .bn_n
        inc     esi
        mov     ebx, esi
        jmp     .bn
.bn_n:
        inc     esi
        jmp     .bn
.bn_done:
        mov     esi, ebx
        mov     edi, kernel_name
.cmpk:
        mov     al, [esi]
        mov     ah, [edi]
        test    ah, ah
        jnz     .notend
        test    al, al                  ; kernel_name ended: match iff input ended too
        jz      do_kerpack
        jmp     do_pack
.notend:
        or      al, 20h                 ; tolower (kernel_name is lowercase)
        cmp     al, ah
        jne     do_pack
        inc     esi
        inc     edi
        jmp     .cmpk

;=====================================================================
; helper: allocate ecx bytes -> eax = ptr, or eax = 0 on failure
;=====================================================================
alloc:
        push    ebx
        push    edx
        push    esi
        push    edi
        push    ebp
        mov     eax, sys_mmap2
        xor     ebx, ebx                ; addr = NULL
        mov     edx, 3                  ; PROT_READ|PROT_WRITE
        mov     esi, 22h                ; MAP_PRIVATE|MAP_ANONYMOUS
        or      edi, -1                 ; fd = -1
        xor     ebp, ebp                ; offset = 0
        int     0x80
        pop     ebp
        pop     edi
        pop     esi
        pop     edx
        pop     ebx
        cmp     eax, 0xFFFFF000         ; -4096..-1 == error
        jb      .ok
        xor     eax, eax
.ok:
        ret

;=====================================================================
; Normal packing (KPCK)
;=====================================================================
do_pack:
; open input (O_RDONLY)
        mov     eax, sys_open
        mov     ebx, [infilename]
        xor     ecx, ecx
        xor     edx, edx
        int     0x80
        test    eax, eax
        js      quit
        mov     [fd], eax
; size = lseek(fd, 0, SEEK_END)
        mov     eax, sys_lseek
        mov     ebx, [fd]
        xor     ecx, ecx
        mov     edx, 2
        int     0x80
        test    eax, eax
        jle     quit
        mov     [insize], eax
; rewind
        mov     eax, sys_lseek
        mov     ebx, [fd]
        xor     ecx, ecx
        xor     edx, edx
        int     0x80
; allocate infile
        mov     ecx, [insize]
        call    alloc
        test    eax, eax
        jz      quit
        mov     [infile], eax
; read
        mov     eax, sys_read
        mov     ebx, [fd]
        mov     ecx, [infile]
        mov     edx, [insize]
        int     0x80
; close
        mov     eax, sys_close
        mov     ebx, [fd]
        int     0x80
; maximum output size
        mov     eax, [insize]
        shr     eax, 3
        add     eax, [insize]
        add     eax, 400h
        mov     esi, eax                ; esi = maxout (preserved across alloc)
        add     eax, eax
        mov     ecx, eax
        call    alloc
        test    eax, eax
        jz      quit
        mov     [outfile], eax
        mov     [outfile1], eax
        mov     [outfilebest], eax
        add     eax, esi
        mov     [outfile2], eax
; KPCK header
        mov     eax, [outfile1]
        mov     dword [eax], 'KPCK'
        mov     ecx, [insize]
        mov     [eax+4], ecx
; LZMA dictionary size
        dec     ecx
        bsr     eax, ecx
        inc     eax
        cmp     eax, 28
        jb      @f
        mov     eax, 28
@@:
        push    eax
        push    eax
        call    lzma_set_dict_size      ; ret 4
        pop     ecx
        mov     eax, 1
        shl     eax, cl
; workmem
        imul    eax, 19
        shr     eax, 1
        add     eax, 509000h
        mov     ecx, eax
        call    alloc
        test    eax, eax
        jz      quit
        mov     [workmem], eax
; first pass: plain LZMA
        mov     eax, [outfile2]
        mov     [outfile], eax
        xchg    eax, edi
        mov     esi, [outfile1]
        movsd
        movsd
        call    pk_pack_lzma
        mov     [outsize], eax
        mov     eax, [outfile]
        mov     [outfilebest], eax
        mov     [method], use_lzma
; calltrick 1
        call    pk_preprocess_calltrick
        test    eax, eax
        jz      .noct1
        call    pk_set_outfile
        call    pk_pack_lzma
        add     eax, 5
        cmp     eax, [outsize]
        jae     .noct1
        mov     [outsize], eax
        mov     eax, [outfile]
        mov     [outfilebest], eax
        mov     [method], use_lzma or use_calltrick1
.noct1:
        call    pk_set_outfile
        push    dword [ctn]
        movzx   eax, byte [cti]
        push    eax
        call    pk_preprocess_calltrick2
        test    eax, eax
        jz      .noct2
        call    pk_set_outfile
        call    pk_pack_lzma
        add     eax, 5
        cmp     eax, [outsize]
        jae     .noct2
        mov     [outsize], eax
        mov     eax, [outfile]
        mov     [outfilebest], eax
        mov     [method], use_lzma or use_calltrick2
        pop     ecx
        pop     ecx
        push    dword [ctn]
        movzx   eax, byte [cti]
        push    eax
.noct2:
        pop     eax
        mov     [cti], al
        pop     eax
        mov     [ctn], eax
        add     dword [outsize], 12
        mov     eax, [outsize]
        cmp     eax, [insize]
        jb      .packed_ok
        jmp     quit                    ; not smaller: leave original
.packed_ok:
        movzx   eax, byte [method]
        mov     edi, [outfilebest]
        mov     [edi+8], eax
        test    al, use_calltrick1 or use_calltrick2
        jz      .save
        mov     ecx, [outsize]
        add     ecx, edi
        mov     eax, [ctn]
        mov     [ecx-5], eax
        mov     al, [cti]
        mov     [ecx-1], al
.save:
; open(name, O_WRONLY|O_CREAT|O_TRUNC, 0644)
        mov     eax, sys_open
        mov     ebx, [infilename]
        mov     ecx, 0x241
        mov     edx, 0x1A4
        int     0x80
        test    eax, eax
        js      quit
        mov     [fd], eax
        mov     eax, sys_write
        mov     ebx, [fd]
        mov     ecx, [outfilebest]
        mov     edx, [outsize]
        int     0x80
        mov     eax, sys_close
        mov     ebx, [fd]
        int     0x80
quit:
        mov     eax, sys_exit
        xor     ebx, ebx
        int     0x80

;---------------------------------------------------------------------
pk_set_outfile:
        mov     eax, [outfilebest]
        xor     eax, [outfile1]
        xor     eax, [outfile2]
        mov     [outfile], eax
        ret
;---------------------------------------------------------------------
pk_pack_lzma:
        mov     eax, [outfile]
        add     eax, 11
        push    [workmem]
        push    [insize]
        push    eax
        push    [infile]
        call    lzma_compress           ; ret 16
        mov     ecx, [outfile]
        mov     edx, [ecx+12]
        bswap   edx
        mov     [ecx+12], edx
        dec     eax
        ret
;---------------------------------------------------------------------
pk_ct_fail:
        xor     eax, eax
        mov     [ctn], eax
        ret
;---------------------------------------------------------------------
pk_preprocess_calltrick:
        mov     ecx, [insize]
        call    alloc
        test    eax, eax
        jz      pk_ct_fail
        push    eax
        xor     eax, eax
        mov     edi, ct1
        mov     ecx, 256/4
        push    edi
        rep     stosd
        pop     edi
        mov     ecx, [insize]
        mov     esi, [infile]
        xchg    eax, edx                ; edx = 0
        pop     eax
        xchg    eax, ebx                ; ebx = temp buffer
        push    ebx
.pre:
        lodsb
        sub     al, 0E8h
        cmp     al, 1
        ja      .cont
        cmp     ecx, 5
        jb      .done
        lodsd
        add     eax, esi
        sub     eax, [infile]
        cmp     eax, [insize]
        jae     .xxx
        cmp     eax, 1000000h
        jae     .xxx
        sub     ecx, 4
        bswap   eax
        mov     [esi-4], eax
        inc     edx
        mov     [ebx], esi
        add     ebx, 4
        jmp     .cont
.xxx:
        sub     esi, 4
        movzx   eax, byte [esi]
        mov     byte [eax+edi], 1
.cont:
        loop    .pre
.done:
        mov     [ctn], edx
        pop     edx                     ; edx = base of temp buffer
        xor     eax, eax
        mov     ecx, 256
        repnz   scasb
        jnz     pk_ct_fail
        not     cl
        mov     [cti], cl
@@:
        cmp     ebx, edx
        jz      @f
        sub     ebx, 4
        mov     eax, [ebx]
        mov     [eax-4], cl
        jmp     @b
@@:
        mov     al, 1
        ret
;---------------------------------------------------------------------
pk_preprocess_calltrick2:
; restore input
        mov     esi, [infile]
        mov     ecx, [ctn]
        jecxz   .pre_init
.restore:
        lodsb
        sub     al, 0E8h
        cmp     al, 1
        ja      .restore
        mov     al, [cti]
        cmp     [esi], al
        jnz     .restore
        lodsd
        mov     al, 0
        bswap   eax
        sub     eax, esi
        add     eax, [infile]
        mov     [esi-4], eax
        loop    .restore
.pre_init:
        mov     ecx, [insize]
        call    alloc
        test    eax, eax
        jz      pk_ct_fail
        mov     edi, ct1
        xchg    eax, ebx
        xor     eax, eax
        push    edi
        mov     ecx, 256/4
        rep     stosd
        pop     edi
        mov     ecx, [insize]
        mov     esi, [infile]
        xchg    eax, edx                ; edx = 0
        push    ebx
.pre:
        lodsb
.chk0f:
        cmp     al, 0Fh
        jnz     .ip1
        dec     ecx
        jz      .done
        lodsb
        cmp     al, 80h
        jb      .chk0f
        cmp     al, 90h
        jb      .take
.ip1:
        sub     al, 0E8h
        cmp     al, 1
        ja      .cont
.take:
        cmp     ecx, 5
        jb      .done
        lodsd
        add     eax, esi
        sub     eax, [infile]
        cmp     eax, [insize]
        jae     .xxx
        cmp     eax, 1000000h
        jae     .xxx
        sub     ecx, 4
        bswap   eax
        mov     [esi-4], eax
        inc     edx
        mov     [ebx], esi
        add     ebx, 4
        jmp     .cont
.xxx:
        sub     esi, 4
        movzx   eax, byte [esi]
        mov     byte [eax+edi], 1
.cont:
        loop    .pre
.done:
        mov     [ctn], edx
        pop     edx
        xor     eax, eax
        mov     ecx, 256
        repnz   scasb
        jnz     pk_ct_fail
        not     cl
        mov     [cti], cl
@@:
        cmp     ebx, edx
        jz      @f
        sub     ebx, 4
        mov     eax, [ebx]
        mov     [eax-4], cl
        jmp     @b
@@:
        mov     al, 1
        ret

;=====================================================================
; Kernel packing (self-extracting KERNPACK)
;=====================================================================
do_kerpack:
        mov     ecx, 300*1024*3 + 6A8000h
        call    alloc
        test    eax, eax
        jz      quit
        mov     [infile], eax
        add     eax, 300*1024
        mov     [inbuftmp], eax
        add     eax, 300*1024
        mov     [outfile], eax
        add     eax, 300*1024
        mov     [workmem], eax
; open + read
        mov     eax, sys_open
        mov     ebx, [infilename]
        xor     ecx, ecx
        xor     edx, edx
        int     0x80
        test    eax, eax
        js      quit
        mov     [fd], eax
        mov     eax, sys_lseek
        mov     ebx, [fd]
        xor     ecx, ecx
        mov     edx, 2
        int     0x80
        test    eax, eax
        jle     quit
        cmp     eax, 300*1024
        jbe     @f
        mov     eax, 300*1024
@@:
        mov     [insize3], eax
        mov     eax, sys_lseek
        mov     ebx, [fd]
        xor     ecx, ecx
        xor     edx, edx
        int     0x80
        mov     eax, sys_read
        mov     ebx, [fd]
        mov     ecx, [infile]
        mov     edx, [insize3]
        int     0x80
        mov     eax, sys_close
        mov     ebx, [fd]
        int     0x80
; already packed?
        mov     ebx, [insize3]
        mov     edi, [infile]
        add     edi, ebx
        cmp     dword [edi-8], 'KERN'
        jnz     .go
        cmp     dword [edi-4], 'PACK'
        jz      quit
.go:
        push    18
        call    lzma_set_dict_size
; find jump to 32-bit code
        mov     edi, [infile]
        mov     eax, edi
        add     eax, [insize3]
        dec     edi
.find:
        cmp     eax, edi
        je      quit                    ; not a kernel
        inc     edi
        cmp     dword [edi], 0xE88EE08E         ; mov fs,ax / mov gs,ax
        jnz     .find
        cmp     dword [edi+4], 0x00BCD08E       ; mov ss,ax / mov esp,00xxxxxx
        jnz     .find
        add     edi, 11
        mov     [inptr], edi
        sub     edi, [infile]
        mov     [indelta], edi
        lea     eax, [ebx+0x10000]
        mov     dword [loader_patch3+2], eax
        sub     ebx, edi
        mov     [insize1], ebx
        call    ker_preprocess_calltrick3
        mov     al, [cti]
        mov     [loader_patch5-1], al
        mov     eax, [ctn]
        mov     [loader_patch4+1], eax
        mov     eax, [inptr]
        add     eax, [outfile]
        sub     eax, [infile]
        add     eax, loader_size - 5
        push    [workmem]
        push    [insize1]
        push    eax
        push    [inptr]
        call    lzma_compress
        add     eax, loader_size - 5
        mov     [loader_patch1+6], eax
        add     eax, [indelta]
        mov     [outsize3], eax
        mov     eax, [indelta]
        mov     ecx, eax
        add     ecx, [outfile]
        mov     ecx, [ecx + loader_size - 4]
        bswap   ecx
        mov     [loader_patch2+4], ecx
        add     eax, 0x10000
        mov     [loader_patch1+1], eax
        mov     esi, [infile]
        mov     edi, [outfile]
        mov     ecx, [indelta]
        rep     movsb
        mov     esi, loader_start
        mov     ecx, loader_size
        rep     movsb
        mov     eax, [outfile]
        add     eax, [outsize3]
        mov     dword [eax], 'KERN'
        mov     dword [eax+4], 'PACK'
        add     dword [outsize3], 8
; save
        mov     eax, sys_open
        mov     ebx, [infilename]
        mov     ecx, 0x241
        mov     edx, 0x1A4
        int     0x80
        test    eax, eax
        js      quit
        mov     [fd], eax
        mov     eax, sys_write
        mov     ebx, [fd]
        mov     ecx, [outfile]
        mov     edx, [outsize3]
        int     0x80
        mov     eax, sys_close
        mov     ebx, [fd]
        int     0x80
        jmp     quit
;---------------------------------------------------------------------
ker_preprocess_calltrick3:
        mov     edi, ct1
        xor     eax, eax
        push    edi
        mov     ecx, 256/4
        rep     stosd
        pop     edi
        mov     ecx, ebx                ; ecx = insize1
        mov     esi, [inptr]
        mov     ebx, [inbuftmp]
        xchg    eax, edx                ; edx = 0
.pre:
        lodsb
.chk0f:
        cmp     al, 0Fh
        jnz     .ip1
        dec     ecx
        jz      .done
        lodsb
        cmp     al, 80h
        jb      .chk0f
        cmp     al, 90h
        jb      .take
.ip1:
        sub     al, 0E8h
        cmp     al, 1
        ja      .cont
.take:
        cmp     ecx, 5
        jb      .done
        lodsd
        add     eax, esi
        sub     eax, [inptr]
        cmp     eax, [insize1]
        jae     .xxx
        cmp     eax, 1000000h
        jae     .xxx
        sub     ecx, 4
        bswap   eax
        mov     [esi-4], eax
        inc     edx
        mov     [ebx], esi
        add     ebx, 4
        jmp     .cont
.xxx:
        sub     esi, 4
        movzx   eax, byte [esi]
        mov     byte [eax+edi], 1
.cont:
        loop    .pre
.done:
        mov     [ctn], edx
        xor     eax, eax
        mov     ecx, 256
        repnz   scasb
        jnz     .ret
        not     cl
        mov     [cti], cl
@@:
        cmp     ebx, [inbuftmp]
        jz      .ret
        sub     ebx, 4
        mov     eax, [ebx]
        mov     [eax-4], cl
        jmp     @b
.ret:
        ret

;---------------------------------------------------------------------
; LZMA compressor and the self-extractor stub (original diamond code)
;---------------------------------------------------------------------
include 'lzma_compress.inc'
include 'lzma_set_dict_size.inc'
include 'loader_lzma.inc'

;---------------------------------------------------------------------
; initialized data
;---------------------------------------------------------------------
kernel_name     db 'kernel.mnt', 0

align 4
LiteralNextStates:
        db 0,0,0,0,1,2,3,4,5,6,4,5
MatchNextStates:
        db 7,7,7,7,7,7,7,10,10,10,10,10
RepNextStates:
        db 8,8,8,8,8,8,8,11,11,11,11,11
ShortRepNextStates:
        db 9,9,9,9,9,9,9,11,11,11,11,11

align 4
method          db 1

;---------------------------------------------------------------------
; uninitialized data (BSS - must stay at the very end of the segment)
;---------------------------------------------------------------------
align 4
infilename      dd ?
fd              dd ?
infile          dd ?
outfile         dd ?
outfile1        dd ?
outfile2        dd ?
outfilebest     dd ?
inbuftmp        dd ?
workmem         dd ?
insize          dd ?
outsize         dd ?
insize3         dd ?
outsize3        dd ?
inptr           dd ?
indelta         dd ?
insize1         dd ?
ctn             dd ?
cti             db ?
                db 0,0,0
ct1             rb 256

align 4
; LZMA compressor work area (from kpack data.inc) - referenced by name
; from lzma_compress.inc; must keep this exact layout.
_lenEncoder:            rd 8451
_prices:                rd 4384
                        rd 17
_finished:              rb 1
_writeEndMark:          rb 1
_longestMatchWasFound:  rb 1
_previousByte:          rb 1
_longestMatchLength:    rd 1
g_FastPos:              rb 1024
_posSlotPrices:         rd 256
_isRep0Long:            rd 192
distances:              rd 274
_optimumCurrentIndex:   rd 1
_additionalOffset:      rd 1
_isRepG1:               rd 12
_isMatch:               rd 192
_alignPriceCount:       rd 1
_numLiteralContextBits: rd 1
_literalEncoder:        rd 114
nowPos64:               rd 2
_distancesPrices:       rd 512
_repDistances:          rd 4
_posSlotEncoder:        rd 1028
lastPosSlotFillingPos:  rd 2
_numFastBytes:          rd 1
_posStateMask:          rd 1
_isRepG0:               rd 12
_repMatchLenEncoder:    rd 8451
                        rd 4384
                        rd 17
_isRepG2:               rd 12
_dictionarySize:        rd 1
_numLiteralPosStateBits:rd 1
_distTableSize:         rd 1
_optimumEndIndex:       rd 1
state.State:            rb 1
state.Prev1IsChar:      rb 1
state.Prev2:            rb 2
state.PosPrev2:         rd 1
state.BackPrev2:        rd 1
state.Price:            rd 1
state.PosPrev:          rd 1
state.BackPrev:         rd 1
state.Backs:            rd 4
                        rd 40950
_alignPrices:           rd 16
_isRep:                 rd 12
_posAlignEncoder:       rd 256
i_01:                   rd 1
_state:                 rb 1
_cache:                 rb 1
_state.Prev2:           rb 2
_posEncoders:           rd 1
_numPrevBits:           rd 1
_numPosBits:            rd 1
_posMask:               rd 1
_posStateBits:          rd 1
_range:                 rd 1
_cacheSize:             rd 1
_cyclicBufferSize:      rd 1
low:                    rd 2
Models:                 rd 512
_matchMaxLen:           rd 1
pack_pos:               rd 1
_cutValue:              rd 1
_hash:                  rd 1
crc_table:              rd 256
_buffer:                rd 1
_pos:                   rd 1
_streamPos:             rd 1
pack_length:            rd 1

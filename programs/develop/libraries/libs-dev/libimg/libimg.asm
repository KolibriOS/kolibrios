;;============================================================================;;
;;//// libimg.asm //// (c) mike.dld, 2007-2008                                ;;
;;                //// (c) diamond, 2009,                                     ;;
;;                //// (c) dunkaist, 2011-2020                                ;;
;;============================================================================;;
;;                                                                            ;;
;; This file is part of Common development libraries (Libs-Dev).              ;;
;;                                                                            ;;
;; Libs-Dev is free software: you can redistribute it and/or modify it under  ;;
;; the terms of the GNU Lesser General Public License as published by the     ;;
;; Free Software Foundation, either version 2.1 of the License, or (at your   ;;
;; option) any later version.                                                 ;;
;;                                                                            ;;
;; Libs-Dev is distributed in the hope that it will be useful, but WITHOUT    ;;
;; ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or      ;;
;; FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public        ;;
;; License for more details.                                                  ;;
;;                                                                            ;;
;; You should have received a copy of the GNU Lesser General Public License   ;;
;; along with Libs-Dev. If not, see <http://www.gnu.org/licenses/>.           ;;
;;                                                                            ;;
;;============================================================================;;


format MS COFF

public @EXPORT as 'EXPORTS'

include 'struct.inc'
include 'proc32.inc'
include 'macros.inc'
include 'config.inc'
include 'debug-fdo.inc'
__DEBUG__ = 0
__DEBUG_LEVEL__ = 1
include 'libio.inc'
purge section,mov,add,sub

include 'libimg.inc'

section '.flat' code readable align 16

include 'bmp/bmp.asm'
include 'gif/gif.asm'
include 'jpeg/jpeg.asm'
include 'png/png.asm'
include 'tga/tga.asm'
include 'z80/z80.asm'
include 'ico_cur/ico_cur.asm'
include 'pcx/pcx.asm'
include 'xcf/xcf.asm'
include 'tiff/tiff.asm'
include 'pnm/pnm.asm'
include 'wbmp/wbmp.asm'
include 'xbm/xbm.asm'

include 'scale.asm'
include 'convert.asm'

; MMX | pretty fast and compatible
; SSE | a bit faster, but may be unsupported by some CPUs
COMPOSITE_MODE equ MMX
match =MMX, COMPOSITE_MODE {include 'blend_mmx.asm'}
match =SSE, COMPOSITE_MODE {include 'blend_sse.asm'}

;;============================================================================;;
proc lib_init ;///////////////////////////////////////////////////////////////;;
;;----------------------------------------------------------------------------;;
;? Library entry point (called after library load)                            ;;
;;----------------------------------------------------------------------------;;
;> eax = pointer to memory allocation routine                                 ;;
;> ebx = pointer to memory freeing routine                                    ;;
;> ecx = pointer to memory reallocation routine                               ;;
;> edx = pointer to library loading routine                                   ;;
;;----------------------------------------------------------------------------;;
;< eax = 1 (fail) / 0 (ok) (library initialization result)                    ;;
;;============================================================================;;
        mov     [mem.alloc], eax
        mov     [mem.free], ebx
        mov     [mem.realloc], ecx

        cmp     [dll.load], edx
        je      .ok

        mov     [dll.load], edx

        or      edx, edx
        jz      @f
        invoke  dll.load, @IMPORT
@@:

        call    img.initialize.jpeg

        xor     eax, eax
        cpuid
        cmp     ecx, 'ntel'
        jnz     @f
        mov     dword [img._.do_rgb.handlers + (Image.bpp15-1)*4], img._.do_rgb.bpp15.intel
        mov     dword [img._.do_rgb.handlers + (Image.bpp16-1)*4], img._.do_rgb.bpp16.intel
@@:

.ok:
        xor     eax, eax
        ret
endp

;;============================================================================;;
proc img.is_img _data, _length ;//////////////////////////////////////////////;;
;;----------------------------------------------------------------------------;;
;? --- TBD ---                                                                ;;
;;----------------------------------------------------------------------------;;
;> --- TBD ---                                                                ;;
;;----------------------------------------------------------------------------;;
;< --- TBD ---                                                                ;;
;;============================================================================;;
        push    ebx
        mov     ebx, img.formats_table
@@:
        stdcall [ebx + FormatsTableEntry.Is], [_data], [_length]
        or      eax, eax
        jnz     @f
        add     ebx, sizeof.FormatsTableEntry
        cmp     dword[ebx], 0
        jnz     @b
        xor     eax, eax
@@:
        pop     ebx
        ret
endp

;;============================================================================;;
proc img.info _data, _length ;////////////////////////////////////////////////;;
;;----------------------------------------------------------------------------;;
;? --- TBD ---                                                                ;;
;;----------------------------------------------------------------------------;;
;> --- TBD ---                                                                ;;
;;----------------------------------------------------------------------------;;
;< --- TBD ---                                                                ;;
;;============================================================================;;
        xor     eax, eax
        ret
endp

;;============================================================================;;
proc img.from_file _filename ;////////////////////////////////////////////////;;
;;----------------------------------------------------------------------------;;
;? load file from disk and decode it                                          ;;
;;----------------------------------------------------------------------------;;
;> [_filename] = file name as passed to libio                                 ;;
;;----------------------------------------------------------------------------;;
;< eax = 0 / pointer to image                                                 ;;
;;============================================================================;;
locals
        fd              dd ?
        img_data_len    dd ?
        img_data        dd ?    ; raw bytes
        img             dd ?    ; Image pointer
endl
        push    ebx
        mov     [img], 0
        invoke  file.open, [_filename], O_READ
        mov     [fd], eax
        test    eax, eax
        jz      .exit
        invoke  file.size, [_filename]
        test    eax, eax
        jnz     .exit_close
        mov     [img_data_len], ebx
        invoke  mem.alloc, ebx
        test    eax, eax
        jz      .exit_close
        mov     [img_data], eax
        invoke  file.read, [fd], eax, [img_data_len]
        cmp     eax, -1
        jz      .exit_free_close
        cmp     eax, [img_data_len]
        jnz     .exit_free_close
        stdcall img.decode, [img_data], [img_data_len], 0
        test    eax, eax
        jz      .exit_free_close
        mov     [img], eax
.exit_free_close:
        invoke  mem.free, [img_data]
.exit_close:
        invoke  file.close, [fd]
        mov     eax, [img]
.exit:
        pop     ebx
        ret
endp

;;============================================================================;;
proc img.to_file _img, _filename ;////////////////////////////////////////////;;
;;----------------------------------------------------------------------------;;
;? --- TBD ---                                                                ;;
;;----------------------------------------------------------------------------;;
;> --- TBD ---                                                                ;;
;;----------------------------------------------------------------------------;;
;< eax = false / true                                                         ;;
;;============================================================================;;
        xor     eax, eax
        ret
endp

;;============================================================================;;
proc img.from_rgb _rgb_data ;/////////////////////////////////////////////////;;
;;----------------------------------------------------------------------------;;
;? --- TBD ---                                                                ;;
;;----------------------------------------------------------------------------;;
;> --- TBD ---                                                                ;;
;;----------------------------------------------------------------------------;;
;< eax = 0 / pointer to image                                                 ;;
;;============================================================================;;
        xor     eax, eax
        ret
endp

;;============================================================================;;
proc img.to_rgb2 _img, _out ;/////////////////////////////////////////////////;;
;;----------------------------------------------------------------------------;;
;? decodes image data into RGB triplets and stores them where [_out] points to;;
;;----------------------------------------------------------------------------;;
;> [_img] = pointer to source image                                           ;;
;> [_out] = where to store RGB triplets                                       ;;
;;----------------------------------------------------------------------------;;
;< none                                                                       ;;
;;============================================================================;;
        push    esi edi
        mov     esi, [_img]
        stdcall img._.validate, esi
        or      eax, eax
        jnz     .ret
        mov     edi, [_out]
        call    img._.do_rgb
.ret:
        pop     edi esi
        ret
endp

;;============================================================================;;
proc img.to_rgb _img ;////////////////////////////////////////////////////////;;
;;----------------------------------------------------------------------------;;
;? decodes image data into RGB triplets and returns pointer to memory area of ;;
;? following structure:                                                       ;;
;? width  dd ?                                                                ;;
;? height dd ?                                                                ;;
;? rgb triplets                                                               ;;
;;----------------------------------------------------------------------------;;
;> [_img] = pointer to source image                                           ;;
;;----------------------------------------------------------------------------;;
;< eax = 0 / pointer to rgb_data (array of [rgb] triplets)                    ;;
;;============================================================================;;
        push    esi edi
        mov     esi, [_img]
        stdcall img._.validate, esi
        or      eax, eax
        jnz     .error

        mov     esi, [_img]
        mov     ecx, [esi + Image.Width]
        imul    ecx, [esi + Image.Height]
        lea     eax, [ecx * 3 + 4 * 3]
        invoke  mem.alloc, eax
        or      eax, eax
        jz      .error

        mov     edi, eax
        push    eax
        mov     eax, [esi + Image.Width]
        stosd
        mov     eax, [esi + Image.Height]
        stosd
        call    img._.do_rgb
        pop     eax
        pop     edi esi
        ret

.error:
        xor     eax, eax
        pop     edi esi
        ret
endp

;;============================================================================;;
proc img._.do_rgb ;///////////////////////////////////////////////////////////;;
;;----------------------------------------------------------------------------;;
;? decodes [esi + Image.Data] data into RGB triplets and stores them at [edi] ;;
;;----------------------------------------------------------------------------;;
;> esi = pointer to source image                                              ;;
;> edi = pointer to memory to store RGB triplets                              ;;
;;----------------------------------------------------------------------------;;
;< none                                                                       ;;
;;============================================================================;;
        mov     ecx, [esi + Image.Width]
        imul    ecx, [esi + Image.Height]
        mov     eax, [esi + Image.Type]
        jmp     dword [.handlers + (eax-1)*4]

align 16
.bpp8i:
; 8 BPP WITH PALETTE -> 24 BPP
        push    ebx
        mov     ebx, [esi + Image.Palette]
        mov     esi, [esi + Image.Data]
        sub     ecx, 1
        jz      .bpp8i.last
@@:
        movzx   eax, byte [esi]
        add     esi, 1
        mov     eax, [ebx + eax*4]
        mov     [edi], eax
        add     edi, 3
        sub     ecx, 1
        jnz     @b
.bpp8i.last:
        movzx   eax, byte [esi]
        mov     eax, [ebx + eax*4]
        mov     [edi], ax
        shr     eax, 16
        mov     [edi+2], al
        pop     ebx
        ret

align 16
.bpp8g:
; 8 BPP GRAYSCALE -> 24 BPP
        mov     esi, [esi + Image.Data]
@@:
        lodsb
        mov     ah, al
        stosb
        stosw
        dec     ecx
        jnz     @b
        ret
;
;align 16
;.bpp8a: ; considered application layer, may be changed in the future
;; 8a BPP -> 24 BPP
;    mov esi, [esi + Image.Data]
;@@:
;    lodsw
;    mov ah, al
;    stosb
;    stosw
;    dec ecx
;    jnz @b
;    ret

; 15 BPP -> 24 BPP
.bpp15.intel:
        push    ebx ebp
        sub     ecx, 4
        jb      .bpp15.tail
align 16
.bpp15.intel.loop:
repeat 2
        mov     ebx, [esi]
        mov     al, [esi]
        mov     ah, [esi+1]
        add     esi, 4
        and     al, 0x1F
        and     ah, 0x1F shl 2
        mov     ebp, ebx
        mov     dl, al
        mov     dh, ah
        shr     al, 2
        shr     ah, 4
        shl     dl, 3
        shl     dh, 1
        and     ebp, 0x1F shl 5
        add     al, dl
        add     ah, dh
        shr     ebp, 2
        mov     [edi], al
        mov     [edi+2], ah
        mov     eax, ebx
        mov     ebx, ebp
        shr     eax, 16
        shr     ebx, 5
        add     ebx, ebp
        mov     ebp, eax
        mov     [edi+1], bl
        and     eax, (0x1F) or (0x1F shl 10)
        and     ebp, 0x1F shl 5
        lea     edx, [eax+eax]
        shr     al, 2
        mov     ebx, ebp
        shr     ah, 4
        shl     dl, 2
        shr     ebx, 2
        shr     ebp, 7
        add     al, dl
        add     ah, dh
        mov     [edi+3], al
        add     ebx, ebp
        mov     [edi+5], ah
        mov     [edi+4], bl
        add     edi, 6
end repeat
        sub     ecx, 4
        jnb     .bpp15.intel.loop
.bpp15.tail:
        add     ecx, 4
        jz      .bpp15.done
@@:
        movzx   eax, word [esi]
        mov     ebx, eax
        add     esi, 2
        and     eax, (0x1F) or (0x1F shl 10)
        and     ebx, 0x1F shl 5
        lea     edx, [eax+eax]
        shr     al, 2
        mov     ebp, ebx
        shr     ebx, 2
        shr     ah, 4
        shl     dl, 2
        shr     ebp, 7
        add     eax, edx
        add     ebx, ebp
        mov     [edi], al
        mov     [edi+1], bl
        mov     [edi+2], ah
        add     edi, 3
        sub     ecx, 1
        jnz     @b
.bpp15.done:
        pop     ebp ebx
        ret

.bpp15.amd:
        push    ebx ebp
        sub     ecx, 4
        jb      .bpp15.tail
align 16
.bpp15.amd.loop:
repeat 4
if (% mod 2) = 1
        mov     eax, dword [esi]
        mov     ebx, dword [esi]
else
        movzx   eax, word [esi]
        mov     ebx, eax
end if
        add     esi, 2
        and     eax, (0x1F) or (0x1F shl 10)
        and     ebx, 0x1F shl 5
        lea     edx, [eax+eax]
        shr     al, 2
        mov     ebp, ebx
        shr     ebx, 2
        shr     ah, 4
        shl     dl, 2
        shr     ebp, 7
        add     eax, edx
        add     ebx, ebp
        mov     [edi], al
        mov     [edi+1], bl
        mov     [edi+2], ah
        add     edi, 3
end repeat
        sub     ecx, 4
        jnb     .bpp15.amd.loop
        jmp     .bpp15.tail

; 16 BPP -> 24 BPP
.bpp16.intel:
        push    ebx ebp
        sub     ecx, 4
        jb      .bpp16.tail
align 16
.bpp16.intel.loop:
repeat 2
        mov     ebx, [esi]
        mov     al, [esi]
        mov     ah, [esi+1]
        add     esi, 4
        and     al, 0x1F
        and     ah, 0x1F shl 3
        mov     ebp, ebx
        mov     dl, al
        mov     dh, ah
        shr     al, 2
        shr     ah, 5
        shl     dl, 3
        and     ebp, 0x3F shl 5
        add     al, dl
        add     ah, dh
        shr     ebp, 3
        mov     [edi], al
        mov     [edi+2], ah
        mov     eax, ebx
        mov     ebx, ebp
        shr     eax, 16
        shr     ebx, 6
        add     ebx, ebp
        mov     ebp, eax
        mov     [edi+1], bl
        and     eax, (0x1F) or (0x1F shl 11)
        and     ebp, 0x3F shl 5
        mov     edx, eax
        shr     al, 2
        mov     ebx, ebp
        shr     ah, 5
        shl     dl, 3
        shr     ebx, 3
        shr     ebp, 9
        add     al, dl
        add     ah, dh
        mov     [edi+3], al
        add     ebx, ebp
        mov     [edi+5], ah
        mov     [edi+4], bl
        add     edi, 6
end repeat
        sub     ecx, 4
        jnb     .bpp16.intel.loop
.bpp16.tail:
        add     ecx, 4
        jz      .bpp16.done
@@:
        movzx   eax, word [esi]
        mov     ebx, eax
        add     esi, 2
        and     eax, (0x1F) or (0x1F shl 11)
        and     ebx, 0x3F shl 5
        mov     edx, eax
        shr     al, 2
        mov     ebp, ebx
        shr     ebx, 3
        shr     ah, 5
        shl     dl, 3
        shr     ebp, 9
        add     eax, edx
        add     ebx, ebp
        mov     [edi], al
        mov     [edi+1], bl
        mov     [edi+2], ah
        add     edi, 3
        sub     ecx, 1
        jnz     @b
.bpp16.done:
        pop     ebp ebx
        ret

.bpp16.amd:
        push    ebx ebp
        sub     ecx, 4
        jb      .bpp16.tail
align 16
.bpp16.amd.loop:
repeat 4
if (% mod 2) = 1
        mov     eax, dword [esi]
        mov     ebx, dword [esi]
else
        movzx   eax, word [esi]
        mov     ebx, eax
end if
        add     esi, 2
        and     eax, (0x1F) or (0x1F shl 11)
        and     ebx, 0x3F shl 5
        mov     edx, eax
        shr     al, 2
        mov     ebp, ebx
        shr     ebx, 3
        shr     ah, 5
        shl     dl, 3
        shr     ebp, 9
        add     eax, edx
        add     ebx, ebp
        mov     [edi], al
        mov     [edi+1], bl
        mov     [edi+2], ah
        add     edi, 3
end repeat
        sub     ecx, 4
        jnb     .bpp16.amd.loop
        jmp     .bpp16.tail

align 16
.bpp24:
; 24 BPP -> 24 BPP
        lea     ecx, [ecx*3 + 3]
        mov     esi, [esi + Image.Data]
        shr     ecx, 2
        rep movsd
        ret

align 16
.bpp32:
; 32 BPP -> 24 BPP
        mov     esi, [esi + Image.Data]

@@:
        mov     eax, [esi]
        mov     [edi], ax
        shr     eax, 16
        mov     [edi+2], al
        add     esi, 4
        add     edi, 3
        sub     ecx, 1
        jnz     @b

@@:
        ret

align 16
.bpp1:
        push    ebx ebp
        mov     ebp, [esi + Image.Width]
        mov     edx, [esi + Image.Height]
        shl     edx, 16
        mov     ebx, [esi + Image.Palette]
        mov     esi, [esi + Image.Data]
.bpp1.pre:
        mov     dx, bp
        mov     ecx, 7
.bpp1.begin:
        xor     eax, eax
        bt      [esi], ecx
        adc     eax, 0
        mov     eax, [ebx + eax*4]
        mov     [edi], ax
        shr     eax, 16
        mov     [edi + 2], al
        add     edi, 3
        dec     dx
        jz      .bpp1.end_line
        dec     ecx
        jns     .bpp1.begin
        mov     ecx, 7
        inc     esi
        jmp     .bpp1.begin
.bpp1.end_line:
        sub     edx, 0x10000
        jz      .bpp1.quit
        inc     esi
        jmp     .bpp1.pre
.bpp1.quit:
        pop     ebp ebx
        ret

align 16
.bpp2i:
        push    ebx ebp
        mov     ebp, [esi + Image.Width]
        mov     edx, [esi + Image.Height]
        shl     edx, 16
        mov     ebx, [esi + Image.Palette]
        mov     esi, [esi + Image.Data]
.bpp2i.pre:
        mov     dx, bp
        mov     ecx, 3
.bpp2i.begin:
        mov     eax, 3
        shl     ecx, 1
        shl     eax, cl
        and     al, [esi]
        shr     eax, cl
        shr     ecx, 1
        mov     eax, [ebx + eax*4]
        mov     [edi], ax
        shr     eax, 16
        mov     [edi + 2], al
        add     edi, 3
        dec     dx
        jz      .bpp2i.end_line
        dec     ecx
        jns     .bpp2i.begin
        mov     ecx, 3
        inc     esi
        jmp     .bpp2i.begin
.bpp2i.end_line:
        sub     edx, 0x10000
        jz      .bpp2i.quit
        inc     esi
        jmp     .bpp2i.pre
.bpp2i.quit:
        pop     ebp ebx
        ret

align 16
.bpp4i:
        push    ebx ebp
        mov     ebp, [esi + Image.Width]
        mov     edx, [esi + Image.Height]
        shl     edx, 16
        mov     ebx, [esi + Image.Palette]
        mov     esi, [esi + Image.Data]
.bpp4i.pre:
        mov     dx, bp
        mov     ecx, 1
.bpp4i.begin:
        mov     eax, 15
        shl     ecx, 2
        shl     eax, cl
        and     al, [esi]
        shr     eax, cl
        shr     ecx, 2
        mov     eax, [ebx + eax*4]
        mov     [edi], ax
        shr     eax, 16
        mov     [edi + 2], al
        add     edi, 3
        dec     dx
        jz      .bpp4i.end_line
        dec     ecx
        jns     .bpp4i.begin
        mov     ecx, 1
        inc     esi
        jmp     .bpp4i.begin
.bpp4i.end_line:
        sub     edx, 0x10000
        jz      .bpp4i.quit
        inc     esi
        jmp     .bpp4i.pre
.bpp4i.quit:
        pop     ebp ebx
        ret

endp

;;============================================================================;;
proc img.decode _data, _length, _options ;////////////////////////////////////;;
;;----------------------------------------------------------------------------;;
;? decodes loaded into memory graphic file                                    ;;
;;----------------------------------------------------------------------------;;
;> [_data]    = pointer to file in memory                                     ;;
;> [_length]  = size in bytes of memory area pointed to by [_data]            ;;
;> [_options] = 0 / pointer to the structure of additional options            ;;
;;----------------------------------------------------------------------------;;
;< eax = 0 / pointer to image                                                 ;;
;;============================================================================;;
        push    ebx
        mov     ebx, [_length]
        or      ebx, ebx
        jz      .fail
        mov     ebx, img.formats_table
@@:
        stdcall [ebx + FormatsTableEntry.Is], [_data], [_length]
        or      eax, eax
        jnz     @f
        add     ebx, sizeof.FormatsTableEntry
        cmp     dword[ebx], eax ;0
        jnz     @b
.fail:
        xor     eax, eax
        pop     ebx
        ret
@@:
        mov     eax, [ebx + FormatsTableEntry.Decode]
        pop     ebx
        leave
        jmp     eax
endp

;;============================================================================;;
proc img.encode uses ebx, _img, _common, _specific ;//////////////////////////;;
;;----------------------------------------------------------------------------;;
;? encode image to some format                                                ;;
;;----------------------------------------------------------------------------;;
;> [_img]      = pointer to input image                                       ;;
;> [_common]   = some most important/common options                           ;;
;     0x00 :  byte : format id (defined in libimg.inc)                        ;;
;     0x01 :  byte : fast encoding (0) / best compression ratio (255)         ;;
;                    0 : store uncompressed data (if supported both by the    ;;
;                        format and libimg)                                   ;;
;                    1 - 255 : use compression, if supported                  ;;
;                        this option may be ignored if any format specific    ;;
;                        options are defined, i.e. 0 here will be ignored if  ;;
;                        particular compression algorithm is specified        ;;
;     0x02 :  byte : flags (bitfield)                                         ;;
;                    0x01 : return an error if format specific conditions     ;;
;                           cannot be met                                     ;;
;                    0x02 : preserve current bit depth. means 8bpp/16bpp/     ;;
;                           24bpp and so on                                   ;;
;                    0x04 : delete alpha channel, if any                      ;;
;                    0x08 : flush alpha channel with 0xff, if any; add it if  ;;
;                           none                                              ;;
;     0x03 :  byte : reserved, must be 0                                      ;;
;> [_specific] = 0 / pointer to the structure of format specific options      ;;
;                   see <format_name>.inc for description                     ;;
;;----------------------------------------------------------------------------;;
;< eax = 0 / pointer to encoded data                                          ;;
;< ecx = error code / the size of encoded data                                ;;
;     1 : out of memory                                                       ;;
;     2 : format is not supported                                             ;;
;     3 : specific conditions cannot be satisfied                             ;;
;     4 : bit depth cannot be preserved                                       ;;
;;============================================================================;;
        mov     ebx, [_img]

        movzx   eax, byte[_common]
        dec     eax
        imul    eax, sizeof.FormatsTableEntry
        add     eax, FormatsTableEntry.Capabilities
        add     eax, img.formats_table
        mov     eax, [eax]
        test    eax, 1          ; is encoding to this format supported at all?
        jnz     @f
        mov     ecx, LIBIMG_ERROR_FORMAT
        jmp     .error
@@:
        mov     ecx, [ebx + Image.Type]
        mov     edx, 1
        shl     edx, cl
        test    eax, edx
        jnz     .bit_depth_ok
        test    byte[_common+2], LIBIMG_ENCODE_STRICT_BIT_DEPTH
        jz      @f
        mov     ecx, LIBIMG_ERROR_BIT_DEPTH
        jmp     .error
@@:
        mov     edx, 1 SHL Image.bpp24
        test    eax, edx
        jnz     @f
        mov     ecx, LIBIMG_ERROR_BIT_DEPTH
        jmp     .error
@@:
        stdcall img.create, [ebx + Image.Width], [ebx + Image.Height], Image.bpp24
        test    eax, eax
        jnz     @f
        mov     ecx, LIBIMG_ERROR_OUT_OF_MEMORY
        jmp     .error
@@:
        push    eax
        stdcall img.to_rgb2, ebx, [eax + Image.Data]
        pop     ebx

.bit_depth_ok:
        movzx   eax, byte[_common]
        dec     eax
        imul    eax, sizeof.FormatsTableEntry
        add     eax, FormatsTableEntry.Encode
        add     eax, img.formats_table
        mov     eax, [eax]
        stdcall eax, [_img], [_common], [_specific]
        push    eax ecx
        cmp     ebx, [_img]
        je      @f
        stdcall img.destroy, ebx
@@:
        pop     ecx eax
        jmp     .quit

.error:
        xor     eax, eax
.quit:
        ret
endp

;;============================================================================;;
proc img.create _width, _height, _type ;//////////////////////////////////////;;
;;----------------------------------------------------------------------------;;
;? creates an Image structure and initializes some its fields                 ;;
;;----------------------------------------------------------------------------;;
;> [_width]  = width of an image in pixels                                    ;;
;> [_height] = height of an image in pixels                                   ;;
;> [_type]   = one of the Image.bppN constants from libimg.inc                ;;
;;----------------------------------------------------------------------------;;
;< eax = 0 / pointer to image                                                 ;;
;;============================================================================;;
        push    ecx

        stdcall img._.new
        or      eax, eax
        jz      .error

        mov     ecx, [_type]
        mov     [eax + Image.Type], ecx

        push    eax

        stdcall img.resize_data, eax, [_width], [_height]
        or      eax, eax
        jz      .error.2

        pop     eax
        jmp     .ret

.error.2:
;       pop     eax
        stdcall img._.delete; eax
        xor     eax, eax

.error:
.ret:
        pop     ecx
        ret
endp

;;============================================================================;;
proc img.destroy.layer _img ;/////////////////////////////////////////////////;;
;;----------------------------------------------------------------------------;;
;? Frees memory occupied by an image and all the memory regions its fields    ;;
;? point to. For image sequences deletes only one frame and fixes Previous/   ;;
;? Next pointers.                                                             ;;
;;----------------------------------------------------------------------------;;
;> [_img] = pointer to image                                                  ;;
;;----------------------------------------------------------------------------;;
;< eax = 0 (fail) / 1 (success)                                               ;;
;;============================================================================;;
        mov     eax, [_img]
        mov     edx, [eax + Image.Previous]
        test    edx, edx
        jz      @f
        push    [eax + Image.Next]
        pop     [edx + Image.Next]
@@:
        mov     edx, [eax + Image.Next]
        test    edx, edx
        jz      @f
        push    [eax + Image.Previous]
        pop     [edx + Image.Previous]
@@:
        stdcall img._.delete, eax
        ret
endp

;;============================================================================;;
proc img.destroy _img ;///////////////////////////////////////////////////////;;
;;----------------------------------------------------------------------------;;
;? Frees memory occupied by an image and all the memory regions its fields    ;;
;? point to. Follows Previous/Next pointers and deletes all the images in     ;;
;? sequence.                                                                  ;;
;;----------------------------------------------------------------------------;;
;> [_img] = pointer to image                                                  ;;
;;----------------------------------------------------------------------------;;
;< eax = 0 (fail) / 1 (success)                                               ;;
;;============================================================================;;
        push    1
        mov     eax, [_img]
        mov     eax, [eax + Image.Previous]
.destroy_prev_loop:
        test    eax, eax
        jz      .destroy_prev_done
        pushd   [eax + Image.Previous]
        stdcall img._.delete, eax
        test    eax, eax
        jnz     @f
        mov     byte [esp+4], 0
@@:
        pop     eax
        jmp     .destroy_prev_loop
.destroy_prev_done:
        mov     eax, [_img]
.destroy_next_loop:
        pushd   [eax + Image.Next]
        stdcall img._.delete, eax
        test    eax, eax
        jnz     @f
        mov     byte [esp+4], 0
@@:
        pop     eax
        test    eax, eax
        jnz     .destroy_next_loop
        pop     eax
        ret
endp

;;============================================================================;;
proc img.count _img ;/////////////////////////////////////////////////////////;;
;;----------------------------------------------------------------------------;;
;? Get number of images in the list (e.g. in animated GIF file)               ;;
;;----------------------------------------------------------------------------;;
;> _img = pointer to image                                                    ;;
;;----------------------------------------------------------------------------;;
;< eax = -1 (fail) / >0 (ok)                                                  ;;
;;============================================================================;;
        push    ecx edx
        mov     edx, [_img]
        stdcall img._.validate, edx
        or      eax, eax
        jnz     .error

@@:
        mov     eax, [edx + Image.Previous]
        or      eax, eax
        jz      @f
        mov     edx, eax
        jmp     @b

@@:
        xor     ecx, ecx
@@:
        inc     ecx
        mov     eax, [edx + Image.Next]
        or      eax, eax
        jz      .exit
        mov     edx, eax
        jmp     @b

.exit:
        mov     eax, ecx
        pop     edx ecx
        ret

.error:
        or      eax, -1
        pop     edx ecx
        ret
endp

;;//// image processing //////////////////////////////////////////////////////;;

;;============================================================================;;
proc img.lock_bits _img, _start_line, _end_line ;/////////////////////////////;;
;;----------------------------------------------------------------------------;;
;? --- TBD ---                                                                ;;
;;----------------------------------------------------------------------------;;
;> --- TBD ---                                                                ;;
;;----------------------------------------------------------------------------;;
;< eax = 0 / pointer to bits                                                  ;;
;;============================================================================;;
        xor     eax, eax
        ret
endp

;;============================================================================;;
proc img.unlock_bits _img, _lock ;////////////////////////////////////////////;;
;;----------------------------------------------------------------------------;;
;? --- TBD ---                                                                ;;
;;----------------------------------------------------------------------------;;
;> --- TBD ---                                                                ;;
;;----------------------------------------------------------------------------;;
;< eax = false / true                                                         ;;
;;============================================================================;;
        xor     eax, eax
        ret
endp

;;============================================================================;;
proc img.flip.layer _img, _flip_kind ;////////////////////////////////////////;;
;;----------------------------------------------------------------------------;;
;? Flip image layer                                                           ;;
;;----------------------------------------------------------------------------;;
;> _img = pointer to image                                                    ;;
;> _flip_kind = one of FLIP_* constants                                       ;;
;;----------------------------------------------------------------------------;;
;< eax = false / true                                                         ;;
;;============================================================================;;
locals
  scanline_len dd ?
endl

        push    ebx esi edi
        mov     ebx, [_img]
        stdcall img._.validate, ebx
        or      eax, eax
        jnz     .error

        mov     ecx, [ebx + Image.Height]
        mov     eax, [ebx + Image.Width]
        call    img._.get_scanline_len
        mov     [scanline_len], eax

        test    [_flip_kind], FLIP_VERTICAL
        jz      .dont_flip_vert
        cmp     dword [ebx + Image.Height], 2
        jl      .dont_flip_vert

        imul    eax, ecx
        sub     eax, [scanline_len]
        shr     ecx, 1
        mov     esi, [ebx + Image.Data]
        lea     edi, [esi + eax]
    
.next_line_vert:
        push    ecx

        mov     ecx, [scanline_len]
        push    ecx
        shr     ecx, 2
@@:
        dec     ecx
        js      @f
        mov     eax, [esi]
        xchg    eax, [edi]
        mov     [esi], eax
        add     esi, 4
        add     edi, 4
        jmp     @b
@@:

        pop     ecx
        and     ecx, 3
        jz      .cont_line_vert
@@:
        mov     al, [esi]
        xchg    al, [edi]
        mov     [esi], al
        add     esi, 1
        add     edi, 1
        dec     ecx
        jnz     @b
.cont_line_vert:

        pop     ecx
        mov     eax, [scanline_len]
        shl     eax, 1
        sub     edi, eax
        dec     ecx
        jnz     .next_line_vert

.dont_flip_vert:

        test    [_flip_kind], FLIP_HORIZONTAL
        jz      .exit
        cmp     dword [ebx + Image.Width], 2
        jl      .exit

        mov     ecx, [ebx + Image.Height]
        mov     eax, [ebx + Image.Type]
        mov     esi, [ebx + Image.Data]
        mov     edi, [scanline_len]
        add     edi, esi
        jmp     dword [.handlers_horz + (eax-1)*4]

.bpp32_horz:
        sub     edi, 4

.next_line_horz:
        push    ecx esi edi

        mov     ecx, [scanline_len]
        shr     ecx, 3
@@:
        mov     eax, [esi]
        xchg    eax, [edi]
        mov     [esi], eax
        add     esi, 4
        add     edi, -4
        sub     ecx, 1
        jnz     @b

        pop     edi esi ecx
        add     esi, [scanline_len]
        add     edi, [scanline_len]
        dec     ecx
        jnz     .next_line_horz
        jmp     .exit

.bpp1x_horz:
        sub     edi, 2
.next_line_horz1x:
        push    ecx esi edi

        mov     ecx, [ebx + Image.Width]
@@:
        mov     ax, [esi]
        mov     dx, [edi]
        mov     [edi], ax
        mov     [esi], dx
        add     esi, 2
        sub     edi, 2
        sub     ecx, 2
        ja      @b

        pop     edi esi ecx
        add     esi, [scanline_len]
        add     edi, [scanline_len]
        dec     ecx
        jnz     .next_line_horz1x
        jmp     .exit

.bpp8ig_horz:
        dec     edi
.next_line_horz8ig:
        push    ecx esi edi

        mov     ecx, [scanline_len]
        shr     ecx, 1
@@:
        mov     al, [esi]
        mov     dl, [edi]
        mov     [edi], al
        mov     [esi], dl
        add     esi, 1
        sub     edi, 1
        sub     ecx, 1
        jnz     @b

        pop     edi esi ecx
        add     esi, [scanline_len]
        add     edi, [scanline_len]
        dec     ecx
        jnz     .next_line_horz8ig
        jmp     .exit

.bpp24_horz:
        sub     edi, 3
.next_line_horz24:
        push    ecx esi edi

        mov     ecx, [ebx + Image.Width]
@@:
        mov     al, [esi]
        mov     dl, [edi]
        mov     [edi], al
        mov     [esi], dl
        mov     al, [esi+1]
        mov     dl, [edi+1]
        mov     [edi+1], al
        mov     [esi+1], dl
        mov     al, [esi+2]
        mov     dl, [edi+2]
        mov     [edi+2], al
        mov     [esi+2], dl
        add     esi, 3
        sub     edi, 3
        sub     ecx, 2
        ja      @b

        pop     edi esi ecx
        add     esi, [scanline_len]
        add     edi, [scanline_len]
        dec     ecx
        jnz     .next_line_horz24
        jmp     .exit

.bpp1_horz:
        mov     edi, [scanline_len]
        mov     edx, [ebx + Image.Width]
        and     edx, 7
        neg     edx
        add     edx, 8
        and     edx, 7
.bpp1_horz.begin:
        push    ebx edx esi
        mov     eax, 7
        add     edi, esi
        sub     edi, 1
        mov     ebx, [ebx + Image.Width]
        shr     ebx, 1
.bpp1_horz.bit:
        bt      [edi], edx
        jc      @f
        btr     [esi], eax
        jmp     .bpp1_horz.right
@@:
        bts     [esi], eax
.bpp1_horz.right:
        jnc     @f
        bts     [edi], edx
        jmp     .bpp1_horz.bit_done
@@:
        btr     [edi], edx
.bpp1_horz.bit_done:
        inc     edx
        and     edx, 7
        jnz     @f
        dec     edi
@@:
        dec     eax
        jns     @f
        mov     eax, 7
        inc     esi
@@:
        dec     ebx
        jnz     .bpp1_horz.bit

        pop     esi edx ebx
        add     esi, [scanline_len]
        mov     edi, [scanline_len]
        dec     ecx
        jnz     .bpp1_horz.begin
        jmp     .exit


.bpp2i_horz:
        mov     edi, [scanline_len]
        mov     edx, [ebx + Image.Width]
        and     edx, 3
        neg     edx
        add     edx, 4
        and     edx, 3
.bpp2i_horz.begin:
        push    ebx edx esi
        mov     eax, 3
        add     edi, esi
        sub     edi, 1
        mov     ebx, [ebx + Image.Width]
        shr     ebx, 1
.bpp2i_horz.pixel:
        push    ebx ecx
        mov     ebx, 3
        mov     ecx, edx
        shl     ebx, cl
        shl     ebx, cl
        and     bl, [edi]
        shr     ebx, cl
        shr     ebx, cl
        mov     bh, 3
        mov     ecx, eax
        shl     ebx, cl
        shl     ebx, cl
        not     bh
        and     bh, [esi]
        or      bl, bh
        mov     bh, [esi]
        mov     [esi], bl
        shr     ebx, 8
        shr     ebx, cl
        shr     ebx, cl
        and     ebx, 3
        mov     bh, 3
        mov     ecx, edx
        shl     ebx, cl
        shl     ebx, cl
        not     bh
        and     bh, [edi]
        or      bl, bh
        mov     [edi], bl
        pop     ecx ebx
.bpp2i_horz.pixel_done:
        inc     edx
        and     edx, 3
        jnz     @f
        dec     edi
@@:
        dec     eax
        jns     @f
        mov     eax, 3
        inc     esi
@@:
        dec     ebx
        jnz     .bpp2i_horz.pixel

        pop     esi edx ebx
        add     esi, [scanline_len]
        mov     edi, [scanline_len]
        dec     ecx
        jnz     .bpp2i_horz.begin
        jmp     .exit


.bpp4i_horz:
        mov     edi, [scanline_len]
        mov     edx, [ebx + Image.Width]
        and     edx, 1
        neg     edx
        add     edx, 2
        and     edx, 1
.bpp4i_horz.begin:
        push    ebx edx esi
        mov     eax, 1
        add     edi, esi
        sub     edi, 1
        mov     ebx, [ebx + Image.Width]
        shr     ebx, 1
.bpp4i_horz.pixel:
        push    ebx ecx
        mov     ebx, 15
        mov     ecx, edx
        shl     ecx, 2
        shl     ebx, cl
        and     bl, [edi]
        shr     ebx, cl
        mov     bh, 15
        mov     ecx, eax
        shl     ecx, 2
        shl     ebx, cl
        not     bh
        and     bh, [esi]
        or      bl, bh
        mov     bh, [esi]
        mov     [esi], bl
        shr     ebx, 8
        shr     ebx, cl
        and     ebx, 15
        mov     bh, 15
        mov     ecx, edx
        shl     ecx, 2
        shl     ebx, cl
        not     bh
        and     bh, [edi]
        or      bl, bh
        mov     [edi], bl
        pop     ecx ebx
.bpp4i_horz.pixel_done:
        inc     edx
        and     edx, 1
        jnz     @f
        dec     edi
@@:
        dec     eax
        jns     @f
        mov     eax, 1
        inc     esi
@@:
        dec     ebx
        jnz     .bpp4i_horz.pixel

        pop     esi edx ebx
        add     esi, [scanline_len]
        mov     edi, [scanline_len]
        dec     ecx
        jnz     .bpp4i_horz.begin
        jmp     .exit


.exit:
        xor     eax, eax
        inc     eax
        pop     edi esi ebx
        ret

.error:
        xor     eax, eax
        pop     edi esi ebx
        ret
endp

;;============================================================================;;
proc img.flip _img, _flip_kind ;//////////////////////////////////////////////;;
;;----------------------------------------------------------------------------;;
;? Flip all layers of image                                                   ;;
;;----------------------------------------------------------------------------;;
;> _img = pointer to image                                                    ;;
;> _flip_kind = one of FLIP_* constants                                       ;;
;;----------------------------------------------------------------------------;;
;< eax = false / true                                                         ;;
;;============================================================================;;
        push    1
        mov     ebx, [_img]
@@:
        mov     eax, [ebx + Image.Previous]
        test    eax, eax
        jz      .loop
        mov     ebx, eax
        jmp     @b
.loop:
        stdcall img.flip.layer, ebx, [_flip_kind]
        test    eax, eax
        jnz     @f
        mov     byte [esp], 0
@@:
        mov     ebx, [ebx + Image.Next]
        test    ebx, ebx
        jnz     .loop
        pop     eax
        ret
endp

;;============================================================================;;
proc img.rotate.layer _img, _rotate_kind ;////////////////////////////////////;;
;;----------------------------------------------------------------------------;;
;? Rotate image layer                                                         ;;
;;----------------------------------------------------------------------------;;
;> _img = pointer to image                                                    ;;
;> _rotate_kind = one of ROTATE_* constants                                   ;;
;;----------------------------------------------------------------------------;;
;< eax = false / true                                                         ;;
;;============================================================================;;
locals
  scanline_len_old    dd ?
  scanline_len_new    dd ?
  scanline_pixels_new dd ?
  line_buffer         dd ?
  pixels_ptr          dd ?
endl

        mov     [line_buffer], 0

        push    ebx esi edi
        mov     ebx, [_img]
        stdcall img._.validate, ebx
        or      eax, eax
        jnz     .error

        cmp     [_rotate_kind], ROTATE_90_CCW
        je      .rotate_ccw_low
        cmp     [_rotate_kind], ROTATE_90_CW
        je      .rotate_cw_low
        cmp     [_rotate_kind], ROTATE_180
        je      .flip
        jmp     .exit

.rotate_ccw_low:
        mov     eax, [ebx + Image.Height]
        mov     [scanline_pixels_new], eax
        call    img._.get_scanline_len
        mov     [scanline_len_new], eax

        invoke  mem.alloc, eax
        or      eax, eax
        jz      .error
        mov     [line_buffer], eax

        mov     eax, [ebx + Image.Width]
        mov     ecx, eax
        call    img._.get_scanline_len
        mov     [scanline_len_old], eax

        mov     eax, [scanline_len_new]
        imul    eax, ecx
        add     eax, [ebx + Image.Data]
        mov     [pixels_ptr], eax

        cmp     [ebx + Image.Type], Image.bpp1
        jz      .rotate_ccw1
        cmp     [ebx + Image.Type], Image.bpp2i
        jz      .rotate_ccw2i
        cmp     [ebx + Image.Type], Image.bpp4i
        jz      .rotate_ccw4i
        cmp     [ebx + Image.Type], Image.bpp8i
        jz      .rotate_ccw8ig
        cmp     [ebx + Image.Type], Image.bpp8g
        jz      .rotate_ccw8ig
        cmp     [ebx + Image.Type], Image.bpp24
        jz      .rotate_ccw24
        cmp     [ebx + Image.Type], Image.bpp32
        jz      .rotate_ccw32

.next_column_ccw_low1x:
        dec     ecx
        js      .exchange_dims
        push    ecx

        mov     edx, [scanline_len_old]
        add     [scanline_len_old], -2

        mov     ecx, [scanline_pixels_new]
        mov     esi, [ebx + Image.Data]
        mov     edi, [line_buffer]
@@:
        mov     ax, [esi]
        mov     [edi], ax
        add     esi, edx
        add     edi, 2
        sub     ecx, 1
        jnz     @b

        mov     eax, [scanline_pixels_new]
        mov     edi, [ebx + Image.Data]
        lea     esi, [edi + 2]
        mov     edx, [scanline_len_old]
@@:
        mov     ecx, edx
        shr     ecx, 2
        rep movsd
        mov     ecx, edx
        and     ecx, 3
        rep movsb
        add     esi, 1
        sub     eax, 1
        jnz     @b

        mov     eax, [scanline_len_new]
        sub     [pixels_ptr], eax
        mov     ecx, [scanline_pixels_new]
        mov     esi, [line_buffer]
        mov     edi, [pixels_ptr]
        mov     edx, ecx
        shr     ecx, 2
        rep movsd
        mov     ecx, edx
        and     ecx, 3
        rep movsb

        pop     ecx
        jmp     .next_column_ccw_low1x

.rotate_ccw32:
.next_column_ccw_low:
        dec     ecx
        js      .exchange_dims
        push    ecx

        mov     edx, [scanline_len_old]
        add     [scanline_len_old], -4

        mov     ecx, [scanline_pixels_new]
        mov     esi, [ebx + Image.Data]
        mov     edi, [line_buffer]
@@:
        mov     eax, [esi]
        stosd
        add     esi, edx
        dec     ecx
        jnz     @b

        mov     eax, [scanline_pixels_new]
        mov     edi, [ebx + Image.Data]
        lea     esi, [edi + 4]
        mov     edx, [scanline_len_old]
        shr     edx, 2
@@:
        mov     ecx, edx
        rep movsd
        add     esi, 4
        dec     eax
        jnz     @b

        mov     eax, [scanline_len_new]
        sub     [pixels_ptr], eax
        mov     ecx, [scanline_pixels_new]
        mov     esi, [line_buffer]
        mov     edi, [pixels_ptr]
        rep movsd

        pop     ecx
        jmp     .next_column_ccw_low

.rotate_ccw8ig:
.next_column_ccw_low8ig:
        dec     ecx
        js      .exchange_dims
        push    ecx

        mov     edx, [scanline_len_old]
        add     [scanline_len_old], -1

        mov     ecx, [scanline_pixels_new]
        mov     esi, [ebx + Image.Data]
        mov     edi, [line_buffer]
@@:
        mov     al, [esi]
        mov     [edi], al
        add     esi, edx
        add     edi, 1
        sub     ecx, 1
        jnz     @b

        mov     eax, [scanline_pixels_new]
        mov     edi, [ebx + Image.Data]
        lea     esi, [edi + 1]
        mov     edx, [scanline_len_old]
@@:
        mov     ecx, edx
        shr     ecx, 2
        rep movsd
        mov     ecx, edx
        and     ecx, 3
        rep movsb
        add     esi, 1
        sub     eax, 1
        jnz     @b

        mov     eax, [scanline_len_new]
        sub     [pixels_ptr], eax
        mov     ecx, [scanline_pixels_new]
        mov     esi, [line_buffer]
        mov     edi, [pixels_ptr]
        mov     edx, ecx
        shr     ecx, 2
        rep movsd
        mov     ecx, edx
        and     ecx, 3
        rep movsb

        pop     ecx
        jmp     .next_column_ccw_low8ig

.rotate_ccw24:
.next_column_ccw_low24:
        dec     ecx
        js      .exchange_dims
        push    ecx

        mov     edx, [scanline_len_old]
        add     [scanline_len_old], -3

        mov     ecx, [scanline_pixels_new]
        mov     esi, [ebx + Image.Data]
        mov     edi, [line_buffer]
@@:
        mov     al, [esi]
        mov     [edi], al
        mov     al, [esi+1]
        mov     [edi+1], al
        mov     al, [esi+2]
        mov     [edi+2], al
        add     esi, edx
        add     edi, 3
        sub     ecx, 1
        jnz     @b

        mov     eax, [scanline_pixels_new]
        mov     edi, [ebx + Image.Data]
        lea     esi, [edi + 3]
        mov     edx, [scanline_len_old]
@@:
        mov     ecx, edx
        shr     ecx, 2
        rep movsd
        mov     ecx, edx
        and     ecx, 3
        rep movsb
        add     esi, 3
        sub     eax, 1
        jnz     @b

        mov     eax, [scanline_len_new]
        sub     [pixels_ptr], eax
        mov     ecx, eax
        mov     esi, [line_buffer]
        mov     edi, [pixels_ptr]
        shr     ecx, 2
        rep movsd
        mov     ecx, eax
        and     ecx, 3
        rep movsb

        pop     ecx
        jmp     .next_column_ccw_low24

.rotate_ccw1:
        push    ecx edx

        mov     eax, [ebx + Image.Height]
        add     eax, 7
        shr     eax, 3
        imul    eax, [ebx + Image.Width]
        push    eax                             ; save new data size

        invoke  mem.alloc, eax
        or      eax, eax
        jz      .error
        push    eax                             ; save pointer to new data

        mov     ecx, [ebx + Image.Width]
        and     ecx, 7
        neg     ecx
        add     ecx, 8
        and     ecx, 7

        mov     edi, eax
        mov     esi, [ebx + Image.Data]
        mov     eax, 7
        mov     edx, [scanline_len_old]
        dec     edx
        add     esi, edx

.rotate_ccw1.begin:
        bt      [esi], ecx
        jc      .rotate_ccw1.one
.rotate_ccw1.zero:
        btr     [edi], eax
        jmp     @f
.rotate_ccw1.one:
        bts     [edi], eax
@@:
        add     esi, [scanline_len_old]
        dec     [scanline_pixels_new]
        jz      .rotate_ccw1.end_of_line
        sub     eax, 1
        adc     edi, 0
        and     eax, 7
        jmp     .rotate_ccw1.begin
.rotate_ccw1.end_of_line:
        inc     edi
        mov     eax, [ebx + Image.Height]
        mov     [scanline_pixels_new], eax
        mov     eax, 7
        inc     ecx
        and     ecx, 7
        jz      @f
        mov     esi, [ebx + Image.Data]
        add     esi, edx
        jmp     .rotate_ccw1.begin 
@@:
        dec     edx
        js      .rotate_ccw1.quit
        mov     esi, [ebx + Image.Data]
        add     esi, edx
        jmp     .rotate_ccw1.begin
.rotate_ccw1.quit:
        pop     esi                             ; get pointer to new data
        mov     edi, [ebx + Image.Data]
        pop     ecx                             ; get new data size
        rep movsb
        invoke  mem.free, esi
        pop     edx ecx
        jmp     .exchange_dims


.rotate_ccw2i:
        push    ecx edx

        mov     eax, [ebx + Image.Height]
        add     eax, 3
        shr     eax, 2
        imul    eax, [ebx + Image.Width]
        push    eax                             ; save new data size

        invoke  mem.alloc, eax
        or      eax, eax
        jz      .error
        push    eax                             ; save pointer to new data

        mov     ecx, [ebx + Image.Width]
        and     ecx, 3
        neg     ecx
        add     ecx, 4
        and     ecx, 3

        mov     edi, eax
        mov     esi, [ebx + Image.Data]
        mov     eax, 3
        mov     edx, [scanline_len_old]
        dec     edx
        add     esi, edx

.rotate_ccw2i.begin:
        push    ebx ecx
        mov     ebx, 3
        shl     ebx, cl
        shl     ebx, cl
        and     bl, [esi]
        shr     ebx, cl
        shr     ebx, cl
        mov     bh, 3
        mov     ecx, eax
        shl     ebx, cl
        shl     ebx, cl
        not     bh
        and     bh, [edi]
        or      bl, bh
        mov     [edi], bl
        pop     ecx ebx

        add     esi, [scanline_len_old]
        dec     [scanline_pixels_new]
        jz      .rotate_ccw2i.end_of_line
        sub     eax, 1
        adc     edi, 0
        and     eax, 3
        jmp     .rotate_ccw2i.begin
.rotate_ccw2i.end_of_line:
        inc     edi
        mov     eax, 3
        mov     esi, [ebx + Image.Height]
        mov     [scanline_pixels_new], esi
        inc     ecx
        and     ecx, 3
        jz      @f
        mov     esi, [ebx + Image.Data]
        add     esi, edx
        jmp     .rotate_ccw2i.begin 
@@:
        dec     edx
        js      .rotate_ccw2i.quit
        mov     esi, [ebx + Image.Data]
        add     esi, edx
        jmp     .rotate_ccw2i.begin
.rotate_ccw2i.quit:
        pop     esi                             ; get pointer to new data
        mov     edi, [ebx + Image.Data]
        pop     ecx                             ; get new data size
        rep movsb
        invoke  mem.free, esi
        pop     edx ecx
        jmp     .exchange_dims


.rotate_ccw4i:
        push    ecx edx

        mov     eax, [ebx + Image.Height]
        add     eax, 1
        shr     eax, 1
        imul    eax, [ebx + Image.Width]
        push    eax                             ; save new data size

        invoke  mem.alloc, eax
        or      eax, eax
        jz      .error
        push    eax                             ; save pointer to new data

        mov     ecx, [ebx + Image.Width]
        and     ecx, 1
        neg     ecx
        add     ecx, 2
        and     ecx, 1

        mov     edi, eax
        mov     esi, [ebx + Image.Data]
        mov     eax, 1
        mov     edx, [scanline_len_old]
        dec     edx
        add     esi, edx

.rotate_ccw4i.begin:
        push    ebx ecx
        mov     ebx, 15
        shl     ecx, 2
        shl     ebx, cl
        and     bl, [esi]
        shr     ebx, cl
        mov     bh, 15
        mov     ecx, eax
        shl     ecx, 2
        shl     ebx, cl
        not     bh
        and     bh, [edi]
        or      bl, bh
        mov     [edi], bl
        pop     ecx ebx

        add     esi, [scanline_len_old]
        dec     [scanline_pixels_new]
        jz      .rotate_ccw4i.end_of_line
        sub     eax, 1
        adc     edi, 0
        and     eax, 1
        jmp     .rotate_ccw4i.begin
.rotate_ccw4i.end_of_line:
        inc     edi
        mov     eax, 1
        mov     esi, [ebx + Image.Height]
        mov     [scanline_pixels_new], esi
        inc     ecx
        and     ecx, 1
        jz      @f
        mov     esi, [ebx + Image.Data]
        add     esi, edx
        jmp     .rotate_ccw4i.begin 
@@:
        dec     edx
        js      .rotate_ccw4i.quit
        mov     esi, [ebx + Image.Data]
        add     esi, edx
        jmp     .rotate_ccw4i.begin
.rotate_ccw4i.quit:
        pop     esi                             ; get pointer to new data
        mov     edi, [ebx + Image.Data]
        pop     ecx                             ; get new data size
        rep movsb
        invoke  mem.free, esi
        pop     edx ecx
        jmp     .exchange_dims



.rotate_cw_low:
        mov     eax, [ebx + Image.Height]
        mov     [scanline_pixels_new], eax
        call    img._.get_scanline_len
        mov     [scanline_len_new], eax

        invoke  mem.alloc, eax
        or      eax, eax
        jz      .error
        mov     [line_buffer], eax

        mov     eax, [ebx + Image.Width]
        mov     ecx, eax
        call    img._.get_scanline_len
        mov     [scanline_len_old], eax

        mov     eax, [scanline_len_new]
        imul    eax, ecx
        add     eax, [ebx + Image.Data]
        mov     [pixels_ptr], eax

        cmp     [ebx + Image.Type], Image.bpp1
        jz      .rotate_cw1
        cmp     [ebx + Image.Type], Image.bpp2i
        jz      .rotate_cw2i
        cmp     [ebx + Image.Type], Image.bpp4i
        jz      .rotate_cw4i
        cmp     [ebx + Image.Type], Image.bpp8i
        jz      .rotate_cw8ig
        cmp     [ebx + Image.Type], Image.bpp8g
        jz      .rotate_cw8ig
        cmp     [ebx + Image.Type], Image.bpp24
        jz      .rotate_cw24
        cmp     [ebx + Image.Type], Image.bpp32
        jz      .rotate_cw32

.next_column_cw_low1x:
        dec     ecx
        js      .exchange_dims
        push    ecx

        mov     edx, [scanline_len_old]
        add     [scanline_len_old], -2

        mov     ecx, [scanline_pixels_new]
        mov     esi, [pixels_ptr]
        add     esi, -2
        mov     edi, [line_buffer]
@@:
        mov     ax, [esi]
        mov     [edi], ax
        sub     esi, edx
        add     edi, 2
        sub     ecx, 1
        jnz     @b

        mov     eax, [scanline_pixels_new]
        dec     eax
        mov     edi, [ebx + Image.Data]
        add     edi, [scanline_len_old]
        lea     esi, [edi + 2]
        mov     edx, [scanline_len_old]
@@:
        mov     ecx, edx
        shr     ecx, 2
        rep movsd
        mov     ecx, edx
        and     ecx, 3
        rep movsb
        add     esi, 3
        sub     eax, 1
        jnz     @b

        mov     eax, [scanline_len_new]
        sub     [pixels_ptr], eax
        mov     ecx, eax
        mov     esi, [line_buffer]
        mov     edi, [pixels_ptr]
        shr     ecx, 2
        rep movsd
        mov     ecx, eax
        and     ecx, 3
        rep movsb

        pop     ecx
        jmp     .next_column_cw_low1x

.rotate_cw32:
.next_column_cw_low:
        dec     ecx
        js      .exchange_dims
        push    ecx

        mov     edx, [scanline_len_old]
        add     [scanline_len_old], -4

        mov     ecx, [scanline_pixels_new]
        mov     esi, [pixels_ptr]
        add     esi, -4
        mov     edi, [line_buffer]
@@:
        mov     eax, [esi]
        stosd
        sub     esi, edx
        dec     ecx
        jnz     @b

        mov     eax, [scanline_pixels_new]
        dec     eax
        mov     edi, [ebx + Image.Data]
        add     edi, [scanline_len_old]
        lea     esi, [edi + 4]
        mov     edx, [scanline_len_old]
        shr     edx, 2
@@:
        mov     ecx, edx
        rep movsd
        add     esi, 4
        dec     eax
        jnz     @b

        mov     eax, [scanline_len_new]
        sub     [pixels_ptr], eax
        mov     ecx, [scanline_pixels_new]
        mov     esi, [line_buffer]
        mov     edi, [pixels_ptr]
        rep movsd

        pop     ecx
        jmp     .next_column_cw_low

.rotate_cw8ig:
.next_column_cw_low8ig:
        dec     ecx
        js      .exchange_dims
        push    ecx

        mov     edx, [scanline_len_old]
        add     [scanline_len_old], -1

        mov     ecx, [scanline_pixels_new]
        mov     esi, [pixels_ptr]
        add     esi, -1
        mov     edi, [line_buffer]
@@:
        mov     al, [esi]
        mov     [edi], al
        sub     esi, edx
        add     edi, 1
        sub     ecx, 1
        jnz     @b

        mov     eax, [scanline_pixels_new]
        dec     eax
        mov     edi, [ebx + Image.Data]
        add     edi, [scanline_len_old]
        lea     esi, [edi + 1]
        mov     edx, [scanline_len_old]
@@:
        mov     ecx, edx
        shr     ecx, 2
        rep movsd
        mov     ecx, edx
        and     ecx, 3
        rep movsb
        add     esi, 1
        sub     eax, 1
        jnz     @b

        mov     eax, [scanline_len_new]
        sub     [pixels_ptr], eax
        mov     ecx, eax
        mov     esi, [line_buffer]
        mov     edi, [pixels_ptr]
        shr     ecx, 2
        rep movsd
        mov     ecx, eax
        and     ecx, 3
        rep movsb

        pop     ecx
        jmp     .next_column_cw_low8ig

.rotate_cw24:
.next_column_cw_low24:
        dec     ecx
        js      .exchange_dims
        push    ecx

        mov     edx, [scanline_len_old]
        add     [scanline_len_old], -3

        mov     ecx, [scanline_pixels_new]
        mov     esi, [pixels_ptr]
        add     esi, -3
        mov     edi, [line_buffer]
@@:
        mov     al, [esi]
        mov     [edi], al
        mov     al, [esi+1]
        mov     [edi+1], al
        mov     al, [esi+2]
        mov     [edi+2], al
        sub     esi, edx
        add     edi, 3
        sub     ecx, 1
        jnz     @b

        mov     eax, [scanline_pixels_new]
        dec     eax
        mov     edi, [ebx + Image.Data]
        add     edi, [scanline_len_old]
        lea     esi, [edi + 3]
        mov     edx, [scanline_len_old]
@@:
        mov     ecx, edx
        shr     ecx, 2
        rep movsd
        mov     ecx, edx
        and     ecx, 3
        rep movsb
        add     esi, 3
        sub     eax, 1
        jnz     @b

        mov     eax, [scanline_len_new]
        sub     [pixels_ptr], eax
        mov     ecx, eax
        mov     esi, [line_buffer]
        mov     edi, [pixels_ptr]
        shr     ecx, 2
        rep movsd
        mov     ecx, eax
        and     ecx, 3
        rep movsb

        pop     ecx
        jmp     .next_column_cw_low24


.rotate_cw1:
        push    ecx edx

        mov     eax, [ebx + Image.Height]
        add     eax, 7
        shr     eax, 3
        imul    [ebx + Image.Width]
        push    eax                             ; save new data size

        invoke  mem.alloc, eax
        or      eax, eax
        jz      .error
        push    eax                             ; save pointer to new data

        mov     edi, eax
        mov     esi, [ebx + Image.Data]
        mov     eax, [ebx + Image.Height]
        dec     eax
        imul    eax, [scanline_len_old]
        add     esi, eax
        mov     eax, 7
        mov     ecx, 7
        mov     edx, 0

.rotate_cw1.begin:
        bt      [esi], ecx
        jc      .rotate_cw1.one
.rotate_cw1.zero:
        btr     [edi], eax
        jmp     @f
.rotate_cw1.one:
        bts     [edi], eax
@@:
        sub     esi, [scanline_len_old]
        dec     [scanline_pixels_new]
        jz      .rotate_cw1.end_of_line
        sub     eax, 1
        adc     edi, 0
        and     eax, 7
        jmp     .rotate_cw1.begin
.rotate_cw1.end_of_line:
        inc     edi
        mov     eax, [ebx + Image.Height]
        mov     [scanline_pixels_new], eax
        mov     eax, 7
        dec     ecx
        jns     @f
        mov     ecx, 7
        inc     edx
        cmp     edx, [scanline_len_old]
        je      .rotate_cw1.quit
@@:
        mov     esi, [ebx + Image.Height]
        dec     esi
        imul    esi, [scanline_len_old]
        add     esi, [ebx + Image.Data]
        add     esi, edx
        jmp     .rotate_cw1.begin
.rotate_cw1.quit:
        pop     eax                             ; get pointer to new data
        mov     esi, eax
        mov     edi, [ebx + Image.Data]
        pop     ecx                             ; get new data size
        rep movsb
        invoke  mem.free, eax
        pop     edx ecx
        jmp     .exchange_dims


.rotate_cw2i:
        push    ecx edx

        mov     eax, [ebx + Image.Height]
        add     eax, 3
        shr     eax, 2
        imul    [ebx + Image.Width]
        push    eax                             ; save new data size

        invoke  mem.alloc, eax
        or      eax, eax
        jz      .error
        push    eax                             ; save pointer to new data

        mov     edi, eax
        mov     esi, [ebx + Image.Data]
        mov     eax, [ebx + Image.Height]
        dec     eax
        imul    eax, [scanline_len_old]
        add     esi, eax
        mov     eax, 3
        mov     ecx, 3
        mov     edx, 0

.rotate_cw2i.begin:
        push    ebx ecx
        mov     ebx, 3
        shl     ebx, cl
        shl     ebx, cl
        and     bl, [esi]
        shr     ebx, cl
        shr     ebx, cl
        mov     bh, 3
        mov     ecx, eax
        shl     ebx, cl
        shl     ebx, cl
        not     bh
        and     bh, [edi]
        or      bl, bh
        mov     [edi], bl
        pop     ecx ebx

        sub     esi, [scanline_len_old]
        dec     [scanline_pixels_new]
        jz      .rotate_cw2i.end_of_line
        sub     eax, 1
        adc     edi, 0
        and     eax, 3
        jmp     .rotate_cw2i.begin
.rotate_cw2i.end_of_line:
        inc     edi
        mov     eax, [ebx + Image.Height]
        mov     [scanline_pixels_new], eax
        mov     eax, 3
        dec     ecx
        jns     @f
        mov     ecx, 3
        inc     edx
        cmp     edx, [scanline_len_old]
        je      .rotate_cw2i.quit
@@:
        mov     esi, [ebx + Image.Height]
        dec     esi
        imul    esi, [scanline_len_old]
        add     esi, [ebx + Image.Data]
        add     esi, edx
        jmp     .rotate_cw2i.begin
.rotate_cw2i.quit:
        pop     eax                             ; get pointer to new data
        mov     esi, eax
        mov     edi, [ebx + Image.Data]
        pop     ecx                             ; get new data size
        rep movsb
        invoke  mem.free, eax
        pop     edx ecx
        jmp     .exchange_dims


.rotate_cw4i:
        push    ecx edx

        mov     eax, [ebx + Image.Height]
        add     eax, 1
        shr     eax, 1
        imul    [ebx + Image.Width]
        push    eax                             ; save new data size

        invoke  mem.alloc, eax
        or      eax, eax
        jz      .error
        push    eax                             ; save pointer to new data

        mov     edi, eax
        mov     esi, [ebx + Image.Data]
        mov     eax, [ebx + Image.Height]
        dec     eax
        imul    eax, [scanline_len_old]
        add     esi, eax
        mov     eax, 1
        mov     ecx, 1
        mov     edx, 0

.rotate_cw4i.begin:
        push    ebx ecx
        mov     ebx, 15
        shl     ecx, 2
        shl     ebx, cl
        and     bl, [esi]
        shr     ebx, cl
        mov     bh, 15
        mov     ecx, eax
        shl     ecx, 2
        shl     ebx, cl
        not     bh
        and     bh, [edi]
        or      bl, bh
        mov     [edi], bl
        pop     ecx ebx

        sub     esi, [scanline_len_old]
        dec     [scanline_pixels_new]
        jz      .rotate_cw4i.end_of_line
        sub     eax, 1
        adc     edi, 0
        and     eax, 1
        jmp     .rotate_cw4i.begin
.rotate_cw4i.end_of_line:
        inc     edi
        mov     eax, [ebx + Image.Height]
        mov     [scanline_pixels_new], eax
        mov     eax, 1
        dec     ecx
        jns     @f
        mov     ecx, 1
        inc     edx
        cmp     edx, [scanline_len_old]
        je      .rotate_cw4i.quit
@@:
        mov     esi, [ebx + Image.Height]
        dec     esi
        imul    esi, [scanline_len_old]
        add     esi, [ebx + Image.Data]
        add     esi, edx
        jmp     .rotate_cw4i.begin
.rotate_cw4i.quit:
        pop     eax                             ; get pointer to new data
        mov     esi, eax
        mov     edi, [ebx + Image.Data]
        pop     ecx                             ; get new data size
        rep movsb
        invoke  mem.free, eax
        pop     edx ecx
        jmp     .exchange_dims


.flip:
        stdcall img.flip.layer, [_img], FLIP_VERTICAL
        test    eax, eax
        jz      .error
        jmp     .exit

.exchange_dims:
        push    [ebx + Image.Width] [ebx + Image.Height]
        pop     [ebx + Image.Width] [ebx + Image.Height]

.exit:
        invoke  mem.free, [line_buffer]
        xor     eax, eax
        inc     eax
        pop     edi esi ebx
        ret

.error:
        invoke  mem.free, [line_buffer]
        xor     eax, eax
        pop     edi esi ebx
        ret
endp

;;============================================================================;;
proc img.rotate _img, _rotate_kind ;//////////////////////////////////////////;;
;;----------------------------------------------------------------------------;;
;? Rotate all layers of image                                                 ;;
;;----------------------------------------------------------------------------;;
;> _img = pointer to image                                                    ;;
;> _rotate_kind = one of ROTATE_* constants                                   ;;
;;----------------------------------------------------------------------------;;
;< eax = false / true                                                         ;;
;;============================================================================;;
        push    1
        mov     ebx, [_img]
@@:
        mov     eax, [ebx + Image.Previous]
        test    eax, eax
        jz      .loop
        mov     ebx, eax
        jmp     @b
.loop:
        stdcall img.rotate.layer, ebx, [_rotate_kind]
        test    eax, eax
        jnz     @f
        mov     byte [esp], 0
@@:
        mov     ebx, [ebx + Image.Next]
        test    ebx, ebx
        jnz     .loop
        pop     eax
        ret
endp

;;============================================================================;;
proc img.draw _img, _x, _y, _width, _height, _xpos, _ypos ;///////////////////;;
;;----------------------------------------------------------------------------;;
;? Draw image in the window                                                   ;;
;;----------------------------------------------------------------------------;;
;> _img = pointer to image                                                    ;;
;>_x = x-coordinate in the window                                             ;;
;>_y = y-coordinate in the window                                             ;;
;>_width = maximum width to draw                                              ;;
;>_height = maximum height to draw                                            ;;
;>_xpos = offset in image by x-axis                                           ;;
;>_ypos = offset in image by y-axis                                           ;;
;;----------------------------------------------------------------------------;;
;< no return value                                                            ;;
;;============================================================================;;
        push    ebx esi edi
        mov     ebx, [_img]
        stdcall img._.validate, ebx
        test    eax, eax
        jnz     .done
        mov     ecx, [ebx + Image.Width]
        sub     ecx, [_xpos]
        jbe     .done
        cmp     ecx, [_width]
        jb      @f
        mov     ecx, [_width]
@@:
        mov     edx, [ebx + Image.Height]
        sub     edx, [_ypos]
        jbe     .done
        cmp     edx, [_height]
        jb      @f
        mov     edx, [_height]
@@:
        mov     eax, [ebx + Image.Width]
        sub     eax, ecx
        call    img._.get_scanline_len
        shl     ecx, 16
        add     ecx, edx
        push    eax
        mov     eax, [ebx + Image.Width]
        imul    eax, [_ypos]
        add     eax, [_xpos]
        call    img._.get_scanline_len
        add     eax, [ebx + Image.Data]
        mov     edx, [_x - 2]
        mov     dx, word [_y]
        mov     esi, [ebx + Image.Type]
        mov     esi, [type2bpp + (esi-1)*4]
        mov     edi, [ebx + Image.Palette]
        xchg    eax, ebx
        pop     eax
        push    ebp
        push    65
        pop     ebp
        xchg    eax, ebp
        int     40h
        pop     ebp
.done:
        pop     edi esi ebx
        ret
endp


align 4
img.formats_table:
  .bmp  dd LIBIMG_FORMAT_BMP,  img.is.bmp,  img.decode.bmp,     img.encode.bmp, 1 + (1 SHL Image.bpp24) + (1 SHL Image.bpp32)
  .ico  dd LIBIMG_FORMAT_ICO,  img.is.ico,  img.decode.ico_cur, img.encode.ico, 0
  .cur  dd LIBIMG_FORMAT_CUR,  img.is.cur,  img.decode.ico_cur, img.encode.cur, 0
  .gif  dd LIBIMG_FORMAT_GIF,  img.is.gif,  img.decode.gif,     img.encode.gif, 0
  .png  dd LIBIMG_FORMAT_PNG,  img.is.png,  img.decode.png,     img.encode.png, 1 + (1 SHL Image.bpp24)
  .jpg  dd LIBIMG_FORMAT_JPEG, img.is.jpg,  img.decode.jpg,     img.encode.jpg, 0
  .tga  dd LIBIMG_FORMAT_TGA,  img.is.tga,  img.decode.tga,     img.encode.tga, 0
  .pcx  dd LIBIMG_FORMAT_PCX,  img.is.pcx,  img.decode.pcx,     img.encode.pcx, 0
  .xcf  dd LIBIMG_FORMAT_XCF,  img.is.xcf,  img.decode.xcf,     img.encode.xcf, 0
  .tiff dd LIBIMG_FORMAT_TIFF, img.is.tiff, img.decode.tiff,    img.encode.tiff,0
  .pnm  dd LIBIMG_FORMAT_PNM,  img.is.pnm,  img.decode.pnm,     img.encode.pnm, 1 + (1 SHL Image.bpp1) + (1 SHL Image.bpp8g) + (1 SHL Image.bpp24)
  .wbmp dd LIBIMG_FORMAT_WBMP, img.is.wbmp, img.decode.wbmp,    img.encode.wbmp,0
  .xbm  dd LIBIMG_FORMAT_XBM,  img.is.xbm,  img.decode.xbm,     img.encode.xbm, 0
  .z80  dd LIBIMG_FORMAT_Z80,  img.is.z80,  img.decode.z80,     img.encode.z80, 0 ;this must be the last entry as there are no signatures in z80 screens at all
        dd 0

;;============================================================================;;
;;////////////////////////////////////////////////////////////////////////////;;
;;============================================================================;;
;! Below are private procs you should never call directly from your code      ;;
;;============================================================================;;
;;////////////////////////////////////////////////////////////////////////////;;
;;============================================================================;;


;;============================================================================;;
proc img._.validate, _img ;///////////////////////////////////////////////////;;
;;----------------------------------------------------------------------------;;
;? --- TBD ---                                                                ;;
;;----------------------------------------------------------------------------;;
;> --- TBD ---                                                                ;;
;;----------------------------------------------------------------------------;;
;< --- TBD ---                                                                ;;
;;============================================================================;;
        xor     eax, eax
        ret
endp

;;============================================================================;;
proc img._.new ;//////////////////////////////////////////////////////////////;;
;;----------------------------------------------------------------------------;;
;? --- TBD ---                                                                ;;
;;----------------------------------------------------------------------------;;
;> --- TBD ---                                                                ;;
;;----------------------------------------------------------------------------;;
;< eax = 0 / pointer to image                                                 ;;
;;============================================================================;;
        invoke  mem.alloc, sizeof.Image
        test    eax, eax
        jz      @f
        push    ecx
        xor     ecx, ecx
        mov     [eax + Image.Data], ecx
        mov     [eax + Image.Type], ecx
        mov     [eax + Image.Flags], ecx
        mov     [eax + Image.Extended], ecx
        mov     [eax + Image.Previous], ecx
        mov     [eax + Image.Next], ecx
        pop     ecx
@@:
        ret
endp

;;============================================================================;;
proc img._.delete _img ;//////////////////////////////////////////////////////;;
;;----------------------------------------------------------------------------;;
;? --- TBD ---                                                                ;;
;;----------------------------------------------------------------------------;;
;> --- TBD ---                                                                ;;
;;----------------------------------------------------------------------------;;
;< eax = false / true                                                         ;;
;;============================================================================;;
        push    edx
        mov     edx, [_img]
        cmp     [edx + Image.Data], 0
        je      @f
        invoke  mem.free, [edx + Image.Data]
@@:
        cmp     [edx + Image.Extended], 0
        je      @f
        invoke  mem.free, [edx + Image.Extended]
@@:
        invoke  mem.free, edx
        pop     edx
        ret
endp

;;============================================================================;;
proc img.resize_data _img, _width, _height ;//////////////////////////////////;;
;;----------------------------------------------------------------------------;;
;? Resize data block of image. New size is calculated from _width and _height ;;
;? params and internal Image.Type value. All the internal fields are updated  ;;
;? iff succeeded. This function does not scale images, use img.scale if you   ;;
;? need to.                                                                   ;;
;;----------------------------------------------------------------------------;;
;> _img = pointer to image                                                    ;;
;> _width = new width                                                         ;;
;> _height = new height                                                       ;;
;;----------------------------------------------------------------------------;;
;< eax = 0 (fail) / pointer to the new pixels data                            ;;
;;============================================================================;;
        push    ebx esi
        mov     ebx, [_img]
        mov     eax, [_height]
; our memory is limited, [_width]*[_height] must not overflow
; image with width or height greater than 65535 is most likely bogus
        cmp     word [_width+2], 0
        jnz     .error
        cmp     word [_height+2], 0
        jnz     .error
        imul    eax, [_width]
        test    eax, eax
        jz      .error
        cmp     [ebx + Image.Type], Image.bpp1
        jz      .bpp1
        cmp     [ebx + Image.Type], Image.bpp2i
        jz      .bpp2i
        cmp     [ebx + Image.Type], Image.bpp4i
        jz      .bpp4i
        cmp     [ebx + Image.Type], Image.bpp8i
        jz      .bpp8i
        cmp     [ebx + Image.Type], Image.bpp8g
        jz      .bpp8g
        cmp     [ebx + Image.Type], Image.bpp8a
        jz      .bpp8a
        cmp     [ebx + Image.Type], Image.bpp24
        jz      .bpp24
.bpp32:
        shl     eax, 2
        jmp     @f
.bpp24:
        lea     eax, [eax*3]
        jmp     @f
.bpp8i:
        add     eax, 256*4  ; for palette
.bpp8g:
        jmp     @f
.bpp8a:
        shl     eax, 1
        jmp     @f
.bpp4i:
        mov     eax, [_width]
        add     eax, 1
        shr     eax, 1
        imul    eax, [_height]
        mov     ecx, eax
        mov     eax, [_height]
        add     eax, 1
        shr     eax, 1
        imul    eax, [_width]
        cmp     eax, ecx
        jge     .bpp4i.skip
        mov     eax, ecx
.bpp4i.skip:
        add     eax, 16*4    ; for palette
        jmp     @f
.bpp2i:
        mov     eax, [_width]
        add     eax, 3
        shr     eax, 2
        imul    eax, [_height]
        mov     ecx, eax
        mov     eax, [_height]
        add     eax, 3
        shr     eax, 2
        imul    eax, [_width]
        cmp     eax, ecx
        jge     .bpp2i.skip
        mov     eax, ecx
.bpp2i.skip:
        add     eax, 4*4    ; for palette
        jmp     @f
.bpp1:
        mov     eax, [_width]
        add     eax, 7
        shr     eax, 3
        imul    eax, [_height]
        mov     ecx, eax
        mov     eax, [_height]
        add     eax, 7
        shr     eax, 3
        imul    eax, [_width]
        cmp     eax, ecx
        jge     .bpp1.skip
        mov     eax, ecx
.bpp1.skip:

        add     eax, 2*4    ; for palette
@@:
        mov     esi, eax
        invoke  mem.realloc, [ebx + Image.Data], eax
        or      eax, eax
        jz      .error

        mov     [ebx + Image.Data], eax
        push    [_width]
        pop     [ebx + Image.Width]
        push    [_height]
        pop     [ebx + Image.Height]
        cmp     [ebx + Image.Type], Image.bpp8i
        jnz     @f
        lea     esi, [eax + esi - 256*4]
        mov     [ebx + Image.Palette], esi
        jmp     .ret
@@:
        cmp     [ebx + Image.Type], Image.bpp1
        jnz     @f
        lea     esi, [eax + esi - 2*4]
        mov     [ebx + Image.Palette], esi
        jmp     .ret
@@:
        cmp     [ebx + Image.Type], Image.bpp2i
        jnz     @f
        lea     esi, [eax + esi - 4*4]
        mov     [ebx + Image.Palette], esi
        jmp     .ret
@@:
        cmp     [ebx + Image.Type], Image.bpp4i
        jnz     .ret
        lea     esi, [eax + esi - 16*4]
        mov     [ebx + Image.Palette], esi
        jmp     .ret

  .error:
        xor     eax, eax
  .ret:
        pop     esi ebx
        ret
endp

;;============================================================================;;
img._.get_scanline_len: ;/////////////////////////////////////////////////////;;
;;----------------------------------------------------------------------------;;
;? Get scanline length of image in bytes                                      ;;
;;----------------------------------------------------------------------------;;
;> eax = width of image in pixels                                             ;;
;> ebx = image                                                                ;;
;;----------------------------------------------------------------------------;;
;< eax = scanline length in bytes                                             ;;
;;============================================================================;;
        cmp     [ebx + Image.Type], Image.bpp1
        jz      .bpp1.1
        cmp     [ebx + Image.Type], Image.bpp2i
        jz      .bpp2i.1
        cmp     [ebx + Image.Type], Image.bpp4i
        jz      .bpp4i.1
        cmp     [ebx + Image.Type], Image.bpp8i
        jz      .bpp8.1
        cmp     [ebx + Image.Type], Image.bpp8g
        jz      .bpp8.1
        cmp     [ebx + Image.Type], Image.bpp8a
        jz      .bpp8a.1
        cmp     [ebx + Image.Type], Image.bpp24
        jz      .bpp24.1
        add     eax, eax
        cmp     [ebx + Image.Type], Image.bpp32
        jnz     .quit
        add     eax, eax
        jmp     .quit
.bpp24.1:
        lea     eax, [eax*3]
        jmp     .quit
.bpp1.1:
        add     eax, 7
        shr     eax, 3
        jmp     .quit
.bpp2i.1:
        add     eax, 3
        shr     eax, 2
        jmp     .quit
.bpp4i.1:
        add     eax, 1
        shr     eax, 1
        jmp     .quit
.bpp8a.1:
        shl     eax, 1
.bpp8.1:
.quit:
        ret


;;============================================================================;;
;;////////////////////////////////////////////////////////////////////////////;;
;;============================================================================;;
;! Below is private data you should never use directly from your code         ;;
;;============================================================================;;
;;////////////////////////////////////////////////////////////////////////////;;
;;============================================================================;;

section '.data' data readable writable align 16
;include_debug_strings

align 4
type2bpp    dd  8, 24, 32, 15, 16, 1, 9, 2, 4
img._.do_rgb.handlers:
    dd  img._.do_rgb.bpp8i
    dd  img._.do_rgb.bpp24
    dd  img._.do_rgb.bpp32
    dd  img._.do_rgb.bpp15.amd  ; can be overwritten in lib_init
    dd  img._.do_rgb.bpp16.amd  ; can be overwritten in lib_init
    dd  img._.do_rgb.bpp1
    dd  img._.do_rgb.bpp8g
    dd  img._.do_rgb.bpp2i
    dd  img._.do_rgb.bpp4i

img.flip.layer.handlers_horz:
    dd  img.flip.layer.bpp8ig_horz
    dd  img.flip.layer.bpp24_horz
    dd  img.flip.layer.bpp32_horz
    dd  img.flip.layer.bpp1x_horz
    dd  img.flip.layer.bpp1x_horz
    dd  img.flip.layer.bpp1_horz
    dd  img.flip.layer.bpp8ig_horz
    dd  img.flip.layer.bpp2i_horz
    dd  img.flip.layer.bpp4i_horz

;;============================================================================;;
;;////////////////////////////////////////////////////////////////////////////;;
;;============================================================================;;
;! Exported functions section                                                 ;;
;;============================================================================;;
;;////////////////////////////////////////////////////////////////////////////;;
;;============================================================================;;


align 4
@EXPORT:

export                                      \
    lib_init           , 'lib_init'           , \
    0x00050007         , 'version'            , \
    img.is_img         , 'img_is_img'         , \
    img.info           , 'img_info'           , \
    img.from_file      , 'img_from_file'      , \
    img.to_file        , 'img_to_file'        , \
    img.from_rgb       , 'img_from_rgb'       , \
    img.to_rgb         , 'img_to_rgb'         , \
    img.to_rgb2        , 'img_to_rgb2'        , \
    img.decode         , 'img_decode'         , \
    img.encode         , 'img_encode'         , \ ;supported formats: PNG 24 32, BMP 24 32, PNM 1 8g 24
    img.create         , 'img_create'         , \
    img.destroy        , 'img_destroy'        , \
    img.destroy.layer  , 'img_destroy_layer'  , \
    img.count          , 'img_count'          , \
    img.lock_bits      , 'img_lock_bits'      , \
    img.unlock_bits    , 'img_unlock_bits'    , \
    img.flip           , 'img_flip'           , \
    img.flip.layer     , 'img_flip_layer'     , \
    img.rotate         , 'img_rotate'         , \
    img.rotate.layer   , 'img_rotate_layer'   , \
    img.draw           , 'img_draw'           , \
    img.scale          , 'img_scale'          , \
    img.get_scaled_size, 'img_get_scaled_size', \
    img.convert        , 'img_convert'        , \
    img.blend          , 'img_blend'          , \
    img.resize_data    , 'img_resize_data'    , \
    img.formats_table  , 'img_formats_table'

; import from deflate unpacker
; is initialized only when PNG loading is requested
align 16
@IMPORT:

library                           \
        archiver, 'archiver.obj', \
        libio   , 'libio.obj'

import  archiver, \
        deflate_unpack2, 'deflate_unpack2',\
        deflateInit2,    'deflateInit2',\
        deflateReset,    'deflateReset',\
        deflate,         'deflate',\
        deflateEnd,      'deflateEnd',\
        calc_crc32,      'calc_crc32'

import libio                    , \
        file.size , 'file_size' , \
        file.open , 'file_open' , \
        file.read , 'file_read' , \
        file.close, 'file_close'

align 4
; mutex for unpacker loading
deflate_loader_mutex    dd  0

; default palette for GIF - b&w
gif_default_palette:
    db  0, 0, 0
    db  0xFF, 0xFF, 0xFF

; uninitialized data - global constant tables
mem.alloc   dd ?
mem.free    dd ?
mem.realloc dd ?
dll.load    dd ?

; data for YCbCr -> RGB translation
color_table_1       rd  256
color_table_2       rd  256
color_table_3       rd  256
color_table_4       rd  256

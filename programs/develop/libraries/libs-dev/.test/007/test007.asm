use32
org 0x0
        db 'MENUET01'
        dd 0x01, START, I_END, E_END, E_END, 0, 0

;-----------------------------------------------------------------------------

include '../../../../../proc32.inc'
include '../../../../../macros.inc'
include '../../../../../dll.inc'
;include '../../../../../debug-fdo.inc'

include '../../libio/libio.inc'
include '../../libimg/libimg.inc'

;-----------------------------------------------------------------------------

START:
        mcall   68, 11

        stdcall dll.Load, @IMPORT
        or      eax, eax
        jnz     exit

        invoke  img.from_file, input_file
        test    eax, eax
        jz      exit
        mov     [image_top], eax

        invoke img.create, 100, 100, Image.bpp32
        mov     [image_bottom], eax
        mov     edi, [eax+Image.Data]
        mov     eax, 0xffffff00
        mov     ecx, 100*100
        rep stosd

still:
        mcall   10
        cmp     eax, 1
        je      .draw_window
        cmp     eax, 2
        je      .key
        cmp     eax, 3
        je      .button
        jmp     still


.draw_window:
        mcall   12, 1
        mcall   0, <200, 150>, <200, 150>, 0x73FFFFFF, 0x00000000, window_title
        call    draw_image
        mcall   12, 2
        jmp     still

.key:
        mcall   2
        jmp     still

.button:
        mcall   17
        shr     eax, 8
        cmp     eax, 1
        jne     still

exit:
        invoke  img.destroy, [image_bottom]
        invoke  img.destroy, [image_top]
        mcall   -1


proc draw_image
        mcall   9, proc_info, -1

        mov     ecx, [proc_info.client_box.height]
        inc     ecx
        mov     edx, [proc_info.client_box.width]
        inc     edx

        mov     eax, [image_bottom]
        mov     esi, [eax+Image.Data]
        invoke  img.create, [eax+Image.Width], [eax+Image.Height], Image.bpp32
        mov     [image_blended], eax
        mov     edi, [eax+Image.Data]
        mov     ecx, [eax+Image.Width]
        imul    ecx, [eax+Image.Height]
        rep movsd
        invoke  img.blend, [image_blended], [image_top], 5, 10, 20, 50, 80, 60
        invoke  img.draw, eax, 0, 0, [eax+Image.Width], [eax+Image.Height], 0, 0
        invoke  img.destroy, [image_blended]

        ret
endp

;-----------------------------------------------------------------------------

window_title    db 'img.blend example',0
input_file:
;        db '/hd0/1/bottom.jpg',0
        db '/hd0/1/top.png',0
;-----------------------------------------------------------------------------

align 4
@IMPORT:

library                           \
        libimg  , 'libimg.obj'

import  libimg                            , \
        libimg.init     , 'lib_init'      , \
        img.create      , 'img_create'    , \
        img.decode      , 'img_decode'    , \
        img.destroy     , 'img_destroy'   , \
        img.draw        , 'img_draw'      , \
        img.from_file   , 'img_from_file' , \
        img.blend       , 'img_blend'

;-----------------------------------------------------------------------------

I_END:

image_top       dd ?
image_bottom    dd ?
image_blended   dd ?

proc_info       process_information

rd 0x400        ; stack
E_END:

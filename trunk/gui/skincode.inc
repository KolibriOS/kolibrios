
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                          ;;
;;   WINDOW SKIN for MenuetOS               ;;
;;                                          ;;
;;   entryway@bkg.lt                        ;;
;;                                          ;;
;;   Bugfixes & upgrades by                 ;;
;;             Samuel Rodriguez Perez       ;;
;;             Xeoda@ciberirmandade.org     ;;
;;                                          ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



include "SKINDATA.INC"

virtual at 0
  bmp_header:
    .type       rw  1  ; "BM" signature
    .filesize   rd  1  ; size of the file
    .reserved   rd  1  ; zero
    .offbits    rd  1  ; pointer to image data
    ;----------------
    .headsize   rd  1  ; usually 40 bytes for image header
    .width      rd  1
    .height     rd  1
    .planes     rw  1  ; usually 1
    .bitcount   rw  1  ; currently 24 bits/pixel (0x18)
    .compress   rd  1  ; zero
    .sizeimage  rd  1  ; x*y*(bitcount/8)
    .unused     rd  4  ; these bits aren't used by MeOS
  bmp_data:
end virtual

virtual at 0x778000
  _bmp_bpl dd ?        ; bytes per line
  _bmp_dpl dd ?        ; dwords per line
  _bmp_zb  dd ?        ; bytes filled by zeroes at the end of a scanline
 align 32
  raw_data:
end virtual

bmp2raw:
; esi = to bmp data (source)
; edi = to raw data (destination)
    cmp   [esi+bmp_header.type],'BM'     ; check if bmp file is really loaded
    jne   .finish
    mov   edx,esi

    mov   eax,[edx+bmp_header.width]
    imul  eax,3
    push  eax
    test  eax,11b
    jz    @f
    add   eax,4
  @@:
    shr   eax,2
    mov   [_bmp_dpl],eax
    shl   eax,2
    mov   [_bmp_bpl],eax
    pop   ebx
    sub   eax,ebx
    mov   [_bmp_zb],eax

    add   esi,bmp_data
    mov   eax,[_bmp_bpl]
    imul  eax,[edx+bmp_header.height]
    add   esi,eax
    mov   ebx,[edx+bmp_header.height]    ; ebx = y
    cld
    .y_begin:
       sub   esi,[_bmp_bpl]
       push  esi
       mov   ecx,[_bmp_dpl]
       rep   movsd
       pop   esi
       sub   edi,[_bmp_zb]
    dec   ebx
    jne   .y_begin

.finish:
    ret


; BMP support by Ivan Poddubny
; 1) load LEFT.BMP
;   a) _skinleftw = bmp_width
;   b) _skinleft = 0
;   c) _refleft = 0x778000
;   d) convert
; 2) load BASE.BMP
;   a) _skinbasew = bmp_width
;   b) _skinbase = _skinleftw
;   c) _refbase = _refleft+sizeof(left_raw_converted)
;   d) convert
; 3) load OPER.BMP
;   a) _skinoper = minus width from bmp file
;   b) _skinoperw = width from bmp file
;   c) _refoper = _refbase+sizeof(oper_raw_converted)
;   d) convert
; 4) set height

load_bmp_file:
; eax = pointer to filename
    mov   ebx, 1
    or    ecx, -1
    mov   edx, 0x90000
    mov   esi, 12
    call  fileread
    ret


load_default_skin:
    pushad
    mov   eax, _fileleft
    call  load_bmp_file
    mov   eax, [0x90000+bmp_header.width]
    mov   [_skinleftw], eax
    mov   [_skinleft], 0
    mov   edi, raw_data
    mov   [_refleft], edi
    mov   esi, 0x90000
    call  bmp2raw
    mov   eax, [_bmp_bpl]
    imul  eax, [0x90000+bmp_header.height]
    push  eax

    mov   eax, _filebase
    call  load_bmp_file
    mov   eax, [0x90000+bmp_header.width]
    mov   [_skinbasew], eax
    mov   eax, [_skinleftw]
    mov   [_skinbase], eax
    pop   eax
    add   eax, [_refleft]
    ; align to 32-byte boundary
    test  eax, 11111b
    jz    @f
    shr   eax, 5
    inc   eax
    shl   eax, 5
  @@:
    ; save base address
    mov   [_refbase], eax
    ; convert
    mov   edi, eax
    mov   esi, 0x90000
    call  bmp2raw
    mov   eax, [_bmp_bpl]
    imul  eax, [0x90000+bmp_header.height]
    push  eax

    mov   eax, _fileoper
    call  load_bmp_file
    mov   eax, [0x90000+bmp_header.width]
    mov   [_skinoperw], eax
    neg   eax
    mov   [_skinoper], eax
    pop   eax
    add   eax, [_refbase]
    ; align to 32-byte boundary
    test  eax, 11111b
    jz    @f
    shr   eax, 5
    inc   eax
    shl   eax, 5
  @@:
    mov   [_refoper], eax
    mov   edi, eax
    mov   esi, 0x90000
    call  bmp2raw


;    mov   eax, [0x90000+bmp_header.height]
;    imul  eax, [0x90000+bmp_header.width]
;    imul  eax, 3
;    add   eax, raw_data  ; now eax points to the last line of image

;    mov   ecx, [eax]
;    mov   [_coloroutborder], ecx
;    mov   [_colorborder], ecx
;    sub   eax, 2*3
;    mov   ecx, [eax]
;    mov   [_colorframe], ecx


    mov   eax, [0x90000+bmp_header.height]
    mov   [_skinh], eax
    popad
    ret



drawwindow_IV:

        pusha

        push  edx

        mov   edi,[esp]                              ; RECTANGLE

        mov   eax,[edi+0]
        shl   eax,16
        mov   ax,[edi+0]
        add   ax,[edi+8]
        mov   ebx,[edi+4]
        shl   ebx,16
        mov   bx,[edi+4]
        add   bx,[edi+12]
;        mov   esi,[edi+24]
;        shr   esi,1
;        and   esi,0x007f7f7f
        mov   esi,[_coloroutborder]
        call  draw_rectangle
        mov   ecx,3
      _dw3l:
        add   eax,1*65536-1
        add   ebx,1*65536-1
        test  ax,ax
        js    no_skin_add_button
        test  bx,bx
        js    no_skin_add_button
        mov   esi,[_colorframe] ;[edi+24]
        call  draw_rectangle
        dec   ecx
        jnz   _dw3l
        mov   esi,[_colorborder]
        add   eax,1*65536-1
        add   ebx,1*65536-1
        test  ax,ax
        js    no_skin_add_button
        test  bx,bx
        js    no_skin_add_button
        call  draw_rectangle

        mov   esi,[esp]
        mov   eax,[esi+8]    ; window width
        mov   edx,[_skinleft]
        shl   edx,16
        mov   ecx,[_skinleftw]
        shl   ecx,16
        add   ecx,[_skinh]
        mov   ebx,[_refleft]
        call  sys_putimage

        mov   esi,[esp]
        mov   eax,[esi+8]
        sub   eax,[_skinleftw]
        sub   eax,[_skinoperw]
        cmp   eax,[_skinbase]
        jng   non_base
        xor   edx,edx
        mov   ebx,[_skinbasew]
        div   ebx

        inc   eax

        mov   ebx,[_refbase]
        mov   ecx,[_skinbasew]
        shl   ecx,16
        add   ecx,[_skinh]
        mov   edx,[_skinbase]
        sub   edx,[_skinbasew]
        shl   edx,16
      baseskinloop:
        shr   edx,16
        add   edx,[_skinbasew]
        shl   edx,16

        push  eax ebx ecx edx
        call  sys_putimage
        pop   edx ecx ebx eax

        dec   eax
        jnz   baseskinloop
      non_base:

        mov   esi,[esp]
        mov   edx,[esi+8]
        sub   edx,[_skinoperw]
        inc   edx
        shl   edx,16
        mov   ebx,[_refoper]
        mov   ecx,[_skinoperw]
        shl   ecx,16
        add   ecx,[_skinh]
        call  sys_putimage

        mov   esi,[esp]

        mov   edx,[esi+04]                       ; WORK AREA
        add   edx,21+5
        mov   ebx,[esi+04]
        add   ebx,[esi+12]
        cmp   edx,ebx
        jg    _noinside2
        mov   eax,5
        mov   ebx,[_skinh]
        mov   ecx,[esi+8]
        mov   edx,[esi+12]
        sub   ecx,4
        sub   edx,4
        mov   edi,[esi+16]
        call  [drawbar]
      _noinside2:

        mov   edi,[0xfe88]
        movzx eax,word [edi]
        cmp   eax,1000
        jge   no_skin_add_button
        inc   eax
        mov   [edi],ax

        shl   eax,4
        add   eax,edi

        mov   bx,[0x3000]
        mov   [eax],bx

        add   eax,2         ; save button id number
        mov   bx,1
        mov   [eax],bx
        add   eax,2         ; x start
        mov   ebx,[esp]
        mov   ebx,[ebx+8]
        cmp   [_buttonCx],0
        jg    _bCx_at_right
        mov   ebx,[_buttonCw]    ; ebx will be 0 in next instruction
      _bCx_at_right:
        sub   ebx,[_buttonCw]
        sub   ebx,[_buttonCx]
        mov   [eax],bx
        add   eax,2         ; x size
        mov   ebx,[_buttonCw]
        mov   [eax],bx
        add   eax,2         ; y start
        mov   ebx,[_buttonCy]
        mov   [eax],bx
        add   eax,2         ; y size
        mov   ebx,[_buttonCh]
        mov   [eax],bx

;* minimize button
        mov   edi,[0xfe88]
        movzx eax,word [edi]
        cmp   eax,1000
        jge   no_skin_add_button
        inc   eax
        mov   [edi],ax

        shl   eax,4
        add   eax,edi

        mov   bx,[0x3000]
        mov   [eax],bx

        add   eax,2         ; save button id number
        mov   bx,65535 ;999
        mov   [eax],bx
        add   eax,2         ; x start
        mov   ebx,[esp]
        mov   ebx,[ebx+8]
        cmp   [_buttonMx],0
        jg    _bMx_at_right
        mov   ebx,[_buttonMw]    ; ebx will be 0 in next instruction
      _bMx_at_right:
        sub   ebx,[_buttonMw]
        sub   ebx,[_buttonMx]
        mov   [eax],bx
        add   eax,2         ; x size
        mov   ebx,[_buttonMw]
        mov   [eax],bx
        add   eax,2         ; y start
        mov   ebx,[_buttonMy]
        mov   [eax],bx
        add   eax,2         ; y size
        mov   ebx,[_buttonMh]
        mov   [eax],bx
;* minimize button

      no_skin_add_button:

        add   esp,4
        popa
        ret



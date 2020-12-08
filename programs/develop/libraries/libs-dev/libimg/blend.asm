match =MMX, COMPOSITE_MODE {include 'blend_mmx.asm'}
match =SSE, COMPOSITE_MODE {include 'blend_sse.asm'}

;;============================================================================;;
proc img.blend uses ebx esi edi, _bottom, _top, _xbottom, _ybottom, \ ;///////;;
                                 _xtop, _ytop, _width, _height ;//////////////;;
;;----------------------------------------------------------------------------;;
;? Alpha blend _top image to _bottom one (both must be of type Image.bpp32)   ;;
;;----------------------------------------------------------------------------;;
;> _bottom = pointer to bottom image (will be changed)                        ;;
;> _top = pointer to top image (unchanged)                                    ;;
;> _xbottom = x coord inside _bottom image where to put _top image            ;;
;> _ybottom = y coord inside _bottom image where to put _top image            ;;
;> _xtop = x coord inside _top image to start from                            ;;
;> _ytop = y coord inside _top image to start from                            ;;
;> _width = width of _top image area to put to _bottom image                  ;;
;> _height = height of _top image area to put to _bottom image                ;;
;;----------------------------------------------------------------------------;;
;< eax = 0 (fail) / _bottom (ok)                                              ;;
;;============================================================================;;
        mov     esi, [_top]
        mov     edi, [_bottom]

        mov     eax, [esi+Image.Width]
        sub     eax, [_width]
        shl     eax, 2
        push    eax

        mov     eax, [edi+Image.Width]
        sub     eax, [_width]
        shl     eax, 2
        push    eax

        mov     eax, [_ytop]
        imul    eax, [esi+Image.Width]
        add     eax, [_xtop]
        shl     eax, 2
        mov     esi, [esi+Image.Data]
        add     esi, eax

        mov     eax, [_ybottom]
        imul    eax, [edi+Image.Width]
        add     eax, [_xbottom]
        shl     eax, 2
        mov     edi, [edi+Image.Data]
        add     edi, eax
	stdcall	xcf._.composite_rgb_00, [_width], [_height]
        mov     eax, [_bottom]
        ret
endp


xcf._.composite_table.begin:
  .p00	dd 00, xcf._.composite_rgb_00, xcf._.composite_gray_00, xcf._.composite_indexed_00	; Normal
  .p01	dd 01, xcf._.composite_rgb_01, xcf._.composite_gray_01, xcf._.composite_gray_01		; Dissolve	: random dithering to discrete alpha
;  .p02	dd 02, xcf._.composite_rgb_02, 0,			xcf._.composite_indexed_02	; Behind	: not selectable in the GIMP UI. not implemented
  .p03	dd 03, xcf._.composite_rgb_03, xcf._.composite_rgb_03, xcf._.composite_indexed_00	; Multiply
  .p04	dd 04, xcf._.composite_rgb_04, xcf._.composite_rgb_04, xcf._.composite_indexed_00	; Screen
  .p05	dd 05, xcf._.composite_rgb_05, xcf._.composite_rgb_05, xcf._.composite_indexed_00	; Overlay
  .p06	dd 06, xcf._.composite_rgb_06, xcf._.composite_rgb_06, xcf._.composite_indexed_00	; Difference
  .p07	dd 07, xcf._.composite_rgb_07, xcf._.composite_rgb_07, xcf._.composite_indexed_00	; Addition
  .p08	dd 08, xcf._.composite_rgb_08, xcf._.composite_rgb_08, xcf._.composite_indexed_00	; Subtract
  .p09	dd 09, xcf._.composite_rgb_09, xcf._.composite_rgb_09, xcf._.composite_indexed_00	; Darken Only
  .p10	dd 10, xcf._.composite_rgb_10, xcf._.composite_rgb_10, xcf._.composite_indexed_00	; Lighten Only
  .p11	dd 11, xcf._.composite_rgb_11, xcf._.composite_gray_00, xcf._.composite_indexed_00	; Hue (H of HSV)
  .p12	dd 12, xcf._.composite_rgb_12, xcf._.composite_gray_00, xcf._.composite_indexed_00	; Saturation (S of HSV)
  .p13	dd 13, xcf._.composite_rgb_13, xcf._.composite_gray_00, xcf._.composite_indexed_00	; Color (H and S of HSL)
  .p14	dd 14, xcf._.composite_rgb_14, xcf._.composite_gray_00, xcf._.composite_indexed_00	; Value (V of HSV)
  .p15	dd 15, xcf._.composite_rgb_15, xcf._.composite_rgb_15, xcf._.composite_indexed_00	; Divide
  .p16	dd 16, xcf._.composite_rgb_16, xcf._.composite_rgb_16, xcf._.composite_indexed_00	; Dodge
  .p17	dd 17, xcf._.composite_rgb_17, xcf._.composite_rgb_17, xcf._.composite_indexed_00	; Burn
  .p18	dd 18, xcf._.composite_rgb_18, xcf._.composite_rgb_18, xcf._.composite_indexed_00	; Hard Light
  .p19	dd 19, xcf._.composite_rgb_05, xcf._.composite_rgb_05, xcf._.composite_indexed_00	; Soft Light	: XCF >= 2 only ('soft light' == 'overlay')
  .p20	dd 20, xcf._.composite_rgb_20, xcf._.composite_rgb_20, xcf._.composite_indexed_00	; Grain Extract	: XCF >= 2 only
  .p21	dd 21, xcf._.composite_rgb_21, xcf._.composite_rgb_21, xcf._.composite_indexed_00	; Grain Merge	: XCF >= 2 only
xcf._.composite_table.end:

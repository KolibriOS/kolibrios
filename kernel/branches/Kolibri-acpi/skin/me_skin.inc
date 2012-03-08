;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;============================================================================
; This file should be used to generate skins of new standard
;============================================================================
; skin file structure:
;----------------------------------------------------------------------------
;  header:
;   dd 'SKIN'
;   dd = version (1 for now)
;   dd @ params
;   dd @ buttons
;   dd @ bitmaps
; ...
;----------------------------------------------------------------------------
; NOTE: order of sections listed below is insignificant
;       since they're identified by pointer in above header
;----------------------------------------------------------------------------
; ...
;  params:
;   dd = skin height
;   dw = right margin
;   dw = left margin
;   dw = bottom margin
;   dw = top margin
;   dd = inner line color
;   dd = outer line color
;   dd = frame color
;   dd = dtp file size
;   ?? = dtp file itself
; ...
;----------------------------------------------------------------------------
; ...
;  buttons:
;   dd = button type (1 = close, 2 = minimize)
;   dw = left button coord (could be negative)
;   dw = top button coord (could be negative)
;   dw = button width
;   dw = button height
;   ... etc for all buttons
;   dd = 0 (end of buttons list)
; ...
;----------------------------------------------------------------------------
; ...
;  bitmaps:
;   dw = bitmap kind (1 = left, 2 = oper, 3 = base)
;   dw = bitmap type (1 = active, 0 = inactive)
;   dd @ bitmap
;   ... etc for all bitmaps
;   dd 0 (end of bitmaps list)
; ...
;----------------------------------------------------------------------------
; ...
;  bitmap:
;   dd = bitmap width
;   dd = bitmap height
;   ?? = raw bitmap data
;   ... etc for all bitmaps
; ...
;============================================================================

dd 'SKIN',1,__params__,__buttons__,__bitmaps__

struc BITMAPFILEHEADER {
  .bfType      dw ? ; WORD
  .bfSize      dd ? ; DWORD
  .bfReserved1 dw ? ; WORD
  .bfReserved2 dw ? ; WORD
  .bfOffBits   dd ? ; DWORD
}

struc BITMAPINFOHEADER {
  .biSize          dd ? ; DWORD
  .biWidth         dd ? ; LONG
  .biHeight        dd ? ; LONG
  .biPlanes        dw ? ; WORD
  .biBitCount      dw ? ; WORD
  .biCompression   dd ? ; DWORD
  .biSizeImage     dd ? ; DWORD
  .biXPelsPerMeter dd ? ; LONG
  .biYPelsPerMeter dd ? ; LONG
  .biClrUsed       dd ? ; DWORD
  .biClrImportant  dd ? ; DWORD
}

struc _bmp {
  .h BITMAPFILEHEADER
  .i BITMAPINFOHEADER
}
virtual at 0
  _bmp _bmp
end virtual

macro BITMAP _name*,_fname*
{
  local w,h,a,r,g,b
  virtual at 0
    file _fname
    load w dword from _bmp.i.biWidth
    load h dword from _bmp.i.biHeight
  end virtual
  align 4
  label _name
    .width  = w
    .height = h
  dd w,h
  a=54+(w*3+(w mod 4))*(h-1)
  size = $
  repeat h
    repeat w
      virtual at 0
        file _fname
        load r from a+0
        load g from a+1
        load b from a+2
      end virtual
      db r,g,b
      a=a+3
    end repeat
    a=a-w*3*2-(w mod 4)
  end repeat
}

macro define_colors name,[col,val]
{
  common
    local a,b,c
  forward
    match =binner,col \{ a = val \}
    match =bouter,col \{ b = val \}
    match =bframe,col \{ c = val \}
  common
    name equ a,b,c
}

macro SKIN_PARAMS [a]
{
  common
    local _height,_margins,_colors,_colors_1,_dtp,_dtp_sz
    __params__:
  forward
    match qq == ww,a
    \{
      match =height,qq \\{ _height = ww \\}
      match =margins,qq \\{
        match [q1:q2:q3:q4],ww
        \\\{
          _margins equ q3,q1,q4,q2
        \\\}
      \\}
      match =colors =active,qq
      \\{
        match [q10==q11:q20==q21:q30==q31],ww
        \\\{
          define_colors _colors,q10,q11,q20,q21,q30,q31
        \\\}
      \\}
      match =colors =inactive,qq
      \\{
        match [q10==q11:q20==q21:q30==q31],ww
        \\\{
          define_colors _colors_1,q10,q11,q20,q21,q30,q31
        \\\}
      \\}
      match =dtp,qq \\{ _dtp equ ww \\}
    \}
  common
    dd _height
    dw _margins
    dd _colors,_colors_1
    virtual at 0
      file _dtp
      _dtp_sz = $
    end virtual
    dd _dtp_sz
    file _dtp
}

macro SKIN_BUTTONS [a]
{
  common
    local btn
    __buttons__:
  forward
    match qq == ww,a
    \{
      btn = 0
      match =close,qq    \\{ btn = 1 \\}
      match =minimize,qq \\{ btn = 2 \\}
      match [q1:q2][q3:q4],ww
      \\{
        if btn <> 0
          dd btn
          dw q1,q2,q3,q4
        end if
      \\}
    \}
  common
    dd 0
}

macro SKIN_BITMAPS [a]
{
  common
    local bmp
    __bitmaps__:
  forward
    match qq == ww,a
    \{
      bmp=-1
      match qqq =active,qq   \\{ bmp = 1 \\}
      match qqq =inactive,qq \\{ bmp = 0 \\}
      match =left qqq,qq
      \\{
        if bmp >= 0
          dw 1,bmp
          dd ww
        end if
      \\}
      match =oper qqq,qq
      \\{
        if bmp >= 0
          dw 2,bmp
          dd ww
        end if
      \\}
      match =base qqq,qq
      \\{
        if bmp >= 0
          dw 3,bmp
          dd ww
        end if
      \\}
    \}
  common
    dd 0
}

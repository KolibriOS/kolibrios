fontname equ 'font8x16.bmp'
virtual at 0
file fontname, 3Eh
; sanity check
load a1 word from 0
load a2 dword from 0xE
if (a1 <> 'BM') | (a2 <> 0x28)
error 'not BMP file!'
end if
load a1 dword from 0x12
load a2 dword from 0x16
if (a1 and 0xF) | (a2 and 0xF) | (a1 > 16*16)
error 'font: invalid width or height'
end if
font_width = a1 shr 4
font_bmp_scanline = (font_width*2 + 3) and not 3
font_height = a2 shr 4
load a1 dword from 0x1A
if a1 <> 0x10001
error 'font: not monochrome bitmap'
end if
end virtual

font:
repeat font_height
cur_scan = %
        repeat 256
                virtual at 0
                a2 = (font_width+14) shr 3
                if cur_scan=1 & %=0x10
                a2 = (font_width+7) shr 3
                end if
                file fontname:3Eh + font_bmp_scanline*(font_height*(16-((%-1) shr 4))-cur_scan) + ((((%-1) and 0xF)*font_width) shr 3), a2
                dd 0
                load a1 dword from 0
                a1 = ((a1 and 0x55555555) shl 1) or ((a1 and 0xAAAAAAAA) shr 1)
                a1 = ((a1 and 0x33333333) shl 2) or ((a1 and 0xCCCCCCCC) shr 2)
                a1 = ((a1 and 0x0F0F0F0F) shl 4) or ((a1 and 0xF0F0F0F0) shr 4)
                end virtual
                a1 = (a1 shr (((%-1)*font_width) and 7)) and ((1 shl font_width) - 1)
                a1 = a1 xor ((1 shl font_width) - 1)
                if font_width > 8
                dw      a1
                else
                db      a1
                end if
        end repeat
end repeat

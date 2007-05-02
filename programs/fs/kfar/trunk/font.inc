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
macro set_name_test a,b
{
if %=a
font_data_#a = b
end if
}
macro set_name b
{
set_name_test 1,b
set_name_test 2,b
set_name_test 3,b
set_name_test 4,b
set_name_test 5,b
set_name_test 6,b
set_name_test 7,b
set_name_test 8,b
set_name_test 9,b
set_name_test 10,b
set_name_test 11,b
set_name_test 12,b
set_name_test 13,b
set_name_test 14,b
set_name_test 15,b
set_name_test 16,b
}
macro db_name_test a
{
if %=a
db font_data_#a
end if
}
macro db_name
{
db_name_test 1
db_name_test 2
db_name_test 3
db_name_test 4
db_name_test 5
db_name_test 6
db_name_test 7
db_name_test 8
db_name_test 9
db_name_test 10
db_name_test 11
db_name_test 12
db_name_test 13
db_name_test 14
db_name_test 15
db_name_test 16
}
macro dw_name_test a
{
if %=a
dw font_data_#a
end if
}
macro dw_name
{
dw_name_test 1
dw_name_test 2
dw_name_test 3
dw_name_test 4
dw_name_test 5
dw_name_test 6
dw_name_test 7
dw_name_test 8
dw_name_test 9
dw_name_test 10
dw_name_test 11
dw_name_test 12
dw_name_test 13
dw_name_test 14
dw_name_test 15
dw_name_test 16
}
repeat font_height
cur_scan = %
        repeat 16
                virtual at 0
                file fontname:3Eh + font_bmp_scanline*(font_height*(17-%)-cur_scan), font_width*2
                dd 0
                repeat 16
                        load a1 dword from ((%-1)*font_width) shr 3
                        a1 = ((a1 and 0x55555555) shl 1) or ((a1 and 0xAAAAAAAA) shr 1)
                        a1 = ((a1 and 0x33333333) shl 2) or ((a1 and 0xCCCCCCCC) shr 2)
                        a1 = ((a1 and 0x0F0F0F0F) shl 4) or ((a1 and 0xF0F0F0F0) shr 4)
                        a1 = (a1 shr (((%-1)*font_width) and 7)) and ((1 shl font_width) - 1)
                        a1 = a1 xor ((1 shl font_width) - 1)
                        set_name a1
                end repeat
                end virtual
                if font_width > 8
                repeat 16
                dw_name
                end repeat
                else
                repeat 16
                db_name
                end repeat
                end if
        end repeat
end repeat
purge set_name_test
purge db_name_test
purge dw_name_test
purge set_name
purge db_name
purge dw_name

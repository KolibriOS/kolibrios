; Вспомогательный файл для console.inc - описание scrollbar
; Вертикальная прокрутка
virtual at 0
file 'conscrlv.bmp', 36h
; sanity check
load a1 word from 0
load a2 dword from 0xE
if (a1 <> 'BM') | (a2 <> 0x28)
error 'conscrlv.bmp: not BMP file!'
end if
load con.vscroll_width dword from 0x12
load con.vscroll_height dword from 0x16
load a1 dword from 0x1A
if a1 <> 0x180001
error 'conscrlv.bmp: must be 24-bit bitmap!'
end if
end virtual

con.vscroll_btn_height = 21
con.vscroll_bgr_height = 2
con.vscroll_bar_height1 = 2
con.vscroll_bar_height2 = 1
con.vscroll_bar_height3 = 2

if con.vscroll_btn_height*4 + con.vscroll_bgr_height*2 \
        + con.vscroll_bar_height1 + con.vscroll_bar_height2 + con.vscroll_bar_height3 \
        <> con.vscroll_height
error 'conscrlv.bmp: invalid dimensions!'
end if

; Загружаем данные BMP, на ходу преобразуя их в данные для 7-й функции
con.vscroll:
repeat con.vscroll_height
        file 'conscrlv.bmp':36h + ((con.vscroll_width*3+3) and not 3)*(con.vscroll_height - %),\
                con.vscroll_width*3
end repeat
con.vscroll_btn1 = con.vscroll
con.vscroll_btn2 = con.vscroll + con.vscroll_btn_height*con.vscroll_width*3
con.vscroll_btn3 = con.vscroll + 2*con.vscroll_btn_height*con.vscroll_width*3
con.vscroll_btn4 = con.vscroll + 3*con.vscroll_btn_height*con.vscroll_width*3
con.vscroll_bgr1 = con.vscroll + 4*con.vscroll_btn_height*con.vscroll_width*3
con.vscroll_bgr2 = con.vscroll_bgr1 + con.vscroll_bgr_height*con.vscroll_width*3
con.vscroll_bar1 = con.vscroll_bgr2 + con.vscroll_bgr_height*con.vscroll_width*3
con.vscroll_bar2 = con.vscroll_bar1 + con.vscroll_bar_height1*con.vscroll_width*3
con.vscroll_bar3 = con.vscroll_bar2 + con.vscroll_bar_height2*con.vscroll_width*3

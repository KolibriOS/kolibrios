umka_init
#disk_add ../img/kolibri.img rd -c 0
ramdisk_init ../img/kolibri.img
set_skin /sys/DEFAULT.SKN
window_redraw 1
draw_window 10 300 5 200 0x000088 1 1 1 0 1 4 hello
set_pixel 0 0 0x0000ff
set_pixel 1 1 0xff0000
set_pixel 2 2 0x00ff00
draw_line 10 510 10 510 0xff0000
draw_rect 60 20 30 20 0x00ff00
put_image chess_image.rgb 8 8 5 15
put_image_palette chess_image.rgb 12 12 5 30 9 0
write_text 10 70 0xffff00 hello 0 0 0 0 0 5 0
set_button_style 0
button 55 40 5 20 0xc0ffee 0xffffff 1 0
set_button_style 1
button 100 40 5 20 0xc1ffee 0xffffff 1 0
display_number 0 10 4 0 0 1234 5 45 0xffff00 1 1 0 0 0x0000ff
blit_bitmap chess_image.rgba 20 35 8 8  0 0 8 8  0 0 0 1  32
window_redraw 2

set_window_caption hi_there 0

get_font_smoothing
set_font_smoothing 0
get_font_smoothing

get_window_colors
set_window_colors 0 0 0 0 0 0 0 0 0 0

dump_win_stack 2
dump_win_pos 2
dump_taskdata 2
dump_appdata 2

process_info -1
get_skin_height
get_screen_area
set_screen_area 0 20 350 250
get_screen_area
get_skin_margins

get_font_size
set_font_size 16
get_font_size

get_screen_size

scrot 016_#f01_#draw_all.out.png

disk_del rd

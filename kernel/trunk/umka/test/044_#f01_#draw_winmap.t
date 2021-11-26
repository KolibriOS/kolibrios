umka_set_boot_params --x_res 44 --y_res 44
umka_init
ramdisk_init ../img/kolibri.img
set_skin /sys/DEFAULT.SKN

window_redraw 1
draw_window 2 10 4 10 0x000088 1 1 1 0 1 4 hello
window_redraw 2

set_window_caption hi_there 0

new_sys_thread
switch_to_thread 3

window_redraw 1
draw_window 4 5 8 5 0x000088 1 1 1 0 1 4 hello
window_redraw 2

dump_win_map

set redraw_background 0
move_window 6 8 5 5
dump_win_map

set redraw_background 0
move_window 6 10 5 5
dump_win_map

scrot 044_#f01_#draw_winmap.out.png

disk_del rd

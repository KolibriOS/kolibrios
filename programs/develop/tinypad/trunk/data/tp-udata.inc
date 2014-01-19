
app_start:
	   dd ?,?
 .params   dd ?
	   dd ?,?
	   db ?
 .filename dd ?

f_info.length dd ?
f_info.path:
    times PATHL+1 db ?
f_info70 rd 7

file_info FILEINFO

checker_ed    EDITOR
checker_ed_ll dd ?

tab_bar      TABCTL
virtual at tab_bar.Current
  cur_tab      TABITEM
end virtual
virtual at tab_bar.Current.Editor
  cur_editor   EDITOR
end virtual

ctx	      dd ?

lines.scr     dd ?    ; number of lines on the screen
columns.scr   dd ?    ; number of columns on the screen
top_ofs       dd ?    ; height occupied by top buttons
bot_ofs       dd ?    ; height occupied by bottom buttons
	      dw ?
left_ofs      dd ?    ;
__rc	      dd ?,?,?,?
copy_count    dd ?    ; number of lines for copying (Ctrl+E)
copy_size     dd ?    ; size of data to copy
s_title.size  dd ?    ; caption length

cur_line_len  dd ?
h_popup       dd ?
bot_dlg_handler dd ?

sel.begin.x   dd ?
sel.begin.y   dd ?
sel.end.x     dd ?
sel.end.y     dd ?
sel.selected  db ?

in_sel	      db ?

do_not_draw   db ?    ; draw top and bottom buttons?
main_closing  db ?    ; main window closing?
main_closed   db ?    ; main window closed?
tb_casesen    db ?    ; focused textbox is case-sensitive?

draw_blines   db ?

align 4
s_fname.size  dd ?
s_fname       rb PATHL+1
align 4
s_search.size dd ?
s_search      rb PATHL+1

s_title       rb PATHL+11  ; window caption

ext db ?
chr dd ?
shi dd ?

align 4
cl_3d_normal dd ?
cl_3d_pushed dd ?
cl_3d_outset dd ?
cl_3d_inset  dd ?
cl_3d_grayed dd ?

tb_color     TBOX
tb_opensave  TBOX
tb_find      TBOX
tb_replace   TBOX
tb_gotorow   TBOX
tb_gotocol   TBOX

focused_tb   dd ?

key1 rb 256

mst  rd 2
mst2 rd 2
mev  db ?
mouse_captured	db ?
just_from_popup db ?

bot_mode  db ?
bot_mode2 db ?

popup_valid_text db ?

align 4

bot_dlg_height dd ?
exit_tab_item  dd ?
exit_tab_num   dd ?

temp_buf dd ?
copy_buf dd ?
clipboard_buf dd ?

label color_tbl dword
  .text       dd ?
  .back       dd ?
  .text.sel   dd ?
  .back.sel   dd ?
  .symbol     dd ?
  .number     dd ?
  .string     dd ?
  .comment    dd ?
  .line.moded dd ?
  .line.saved dd ?

tabs_pos  db ?
secure_sel db ?
auto_braces db ?
auto_indent db ?
smart_tab db ?
optim_save db ?
line_nums db ?

mainwnd_pos:
  .x dd ?
  .y dd ?
  .w dd ?
  .h dd ?

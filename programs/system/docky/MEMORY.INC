             rb  1024
__stackend:
;-------------------------------------------------------------------------------
sc system_colors
;-------------------------------------------------------------------------------
win:
 .x          rd  1
 .y          rd  1
 .w          rd  1
 .h          rd  1
  
 .x_hdn      rd  1
 .y_hdn      rd  1
 .w_hdn      rd  1
 .h_hdn      rd  1

 .x_opn      rd  1
 .y_opn      rd  1
 .w_opn      rd  1
 .h_opn      rd  1
  
 .sid        rd  1
 .psid       rd  1
 .procinfo:  rb  1024
 .state      rb  1
 .btn_index  rd  1
 .prev_index rd  1

 .isvert     rb  1
;-------------------------------------------------------------------------------
nwin:
 .x          rd  1
 .y          rd  1
 .w          rd  1
 .h          rd  1

 .bgx        rd  1
 .bgy        rd  1
 .bgwidth    rd  1
 .bgheight   rd  1

 .visible    rd  1
 .sid        rd  1
 .swap_shape rb  1
 .close      rb  1
;-------------------------------------------------------------------------------
fi:
 .p00        rd  1
 .p04        rd  1
 .p08        rd  1
 .p12        rd  1
 .p16        rd  1
 .p20        rb  1
 .p21        rd  1
;-------------------------------------------------------------------------------
dock_items:
 .count      rd  1
 .name       rb  16  * 20
 .path       rb  256 * 20
 .param      rb  256 * 20
 .icon       rd  1   * 20
 .separator  rb  1   * 20
 .location   rd  1
 .fsize:     rb  1
 .ashow:     rb  1
;-------------------------------------------------------------------------------
sys_path     rb  4096
file_name    rb  4096
;-------------------------------------------------------------------------------
icons_resname db 'ICONS32', 0
shr_icons_ptr dd ?
shr_icons_len dd ?
icons_inactive dd ?
icons_active dd ?
;-------------------------------------------------------------------------------

__memend:
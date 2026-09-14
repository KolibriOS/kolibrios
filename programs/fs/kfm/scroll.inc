;---------------------------------------------------------------------
check_scroll_left_event:
     pusha
     mov  eax,[mouse_scroll_data]
     test eax,eax
     jz  .end
     test ax,0x8000
     jnz  .decr
     shr  eax,16
     test ax,0x8000
     jnz  .decr_1
     mov   eax,[left_folder_block]
     sub   eax,[max_panel_line]
     cmp   [left_start_draw_line],eax
     jae   .end
     movzx   eax,[mouse_scroll_data.vertical]
     test eax,eax
     jnz   @f
     movzx   eax,[mouse_scroll_data.horizontal]
@@:
     add   [left_start_draw_line],eax
     add   [sb_left.position],eax
     jmp   .draw
.decr:
     movzx   eax,[mouse_scroll_data.vertical]
     jmp    @f
.decr_1:
     movzx   eax,[mouse_scroll_data.horizontal]
@@:
     cmp   [left_start_draw_line],0
     je   .end
     mov  ebx,0xffff
     sub  ebx,eax
     inc   ebx
     sub   [left_start_draw_line],ebx
     sub   [sb_left.position],ebx
     cmp   [left_start_draw_line],0
     jae   .draw
     mov   [left_start_draw_line],0
     mov   [sb_left.position],0
.draw:
     call draw_left_panel
     mov   [scroll_bar_event],1
.end:
     popa
     ret
;---------------------------------------------------------------------
check_scroll_right_event:
     pusha
     mov  eax,[mouse_scroll_data]
     test eax,eax
     jz  .end
     test ax,0x8000
     jnz  .decr
     shr  eax,16
     test ax,0x8000
     jnz  .decr_1
     mov   eax,[right_folder_block]
     sub   eax,[max_panel_line]
     cmp   [right_start_draw_line],eax
     jae   .end
     movzx   eax,[mouse_scroll_data.vertical]
     test eax,eax
     jnz   @f
     movzx   eax,[mouse_scroll_data.horizontal]
@@:
     add   [right_start_draw_line],eax
     add   [sb_right.position],eax
     jmp   .draw
.decr:
     movzx   eax,[mouse_scroll_data.vertical]
     jmp    @f
.decr_1:
     movzx   eax,[mouse_scroll_data.horizontal]
@@:
     cmp   [right_start_draw_line],0
     je   .end
     mov  ebx,0xffff
     sub  ebx,eax
     inc   ebx
     sub   [right_start_draw_line],ebx
     sub   [sb_right.position],ebx
     cmp   [right_start_draw_line],0
     jae   .draw
     mov   [right_start_draw_line],0
     mov   [sb_right.position],0
.draw:
     call draw_right_panel
     mov   [scroll_bar_event],1
.end:
     popa

     ret
;---------------------------------------------------------------------
align 4
check_scroll_bar_left_event:
     mov   [scroll_pointer],1
     cmp   [mouse_keys_delta],0
     je    @f
     cmp   [mouse_keys],0
     jz    .exit
@@:
     sub   ecx,15
     cmp   bx,cx
     jb    .exit
@@:
     cmp  [select_panel_counter],0
     je   @f
     mov  [select_panel_counter],0
     call  draw_all_panels_for_scroll_bar
@@:
     mov   [scroll_bar_event],1
     mov   cx,[left_panel_y.start_y]
     add   cx,15
     cmp   ax,cx
     ja    .scroll
     cmp   [left_start_draw_line],0
     jne   @f
     ret
@@:
     dec   [left_start_draw_line]
     dec   [sb_left.position]
     jmp   .all
.scroll:
     add   cx,[left_panel_y.size_y]
     sub   cx,30
     cmp   ax,cx
     ja    .down
     mov   ebx,16  ;15
     add   bx,[left_panel_y.start_y]
     sub   cx,bx
     sub   ax,bx
     mov   ebx,eax
     mov   eax,[left_folder_block]
     call  calculate_for_scroll_bar
     cmp   [left_start_draw_line],eax
     je    .exit
     mov   [left_start_draw_line],eax
     mov   [sb_left.position],eax
.all:
     call draw_left_panel
.exit:
     ret
.down:
     mov   eax,[left_folder_block]
     sub   eax,[max_panel_line]
     cmp   [left_start_draw_line],eax
     jb   @f
     ret
@@:
     inc   [left_start_draw_line]
     inc   [sb_left.position]
     jmp   .all
;---------------------------------------------------------------------
draw_all_panels_for_scroll_bar:
     push eax ebx
     call draw_right_panel
     call draw_left_panel
     pop  ebx eax
     ret
;---------------------------------------------------------------------
calculate_for_scroll_bar:
     sub   eax,[max_panel_line]
     inc   eax
     shl   eax,10
     xor   edx,edx
     div   ecx
     shl   edx,1
     cmp   edx,ebx
     jb    @f
     inc   eax
@@:
     imul  eax,ebx
     shr   eax,10
     ret
;---------------------------------------------------------------------
check_scroll_bar_right_event:
     mov   [scroll_pointer],2
     cmp   [mouse_keys_delta],0
     je    @f
     cmp   [mouse_keys],0
     jz    .exit
@@:
     sub   ecx,15
     cmp   bx,cx
     jb    .exit
@@:
     cmp  [select_panel_counter],1
     je   @f
     mov  [select_panel_counter],1
     call  draw_all_panels_for_scroll_bar
@@:
     mov   [scroll_bar_event],1
     mov   cx,[right_panel_y.start_y]
     add   cx,15
     cmp   ax,cx
     ja    .scroll
     cmp   [right_start_draw_line],0
     jne   @f
     ret
@@:
     dec   [right_start_draw_line]
     jmp   .all
.scroll:
     add   cx,[right_panel_y.size_y]
     sub   cx,30
     cmp   ax,cx
     ja    .down
     mov   ebx,16  ;15
     add   bx,[right_panel_y.start_y]
     sub   cx,bx
     sub   ax,bx
     mov   ebx,eax
     mov   eax,[right_folder_block]
     call  calculate_for_scroll_bar
     cmp   [right_start_draw_line],eax
     je    .exit
     mov   [right_start_draw_line],eax
     mov   [sb_right.position],eax
.all:
     call draw_right_panel
.exit:
     ret
.down:
     mov   eax,[right_folder_block]
     sub   eax,[max_panel_line]
     cmp   [right_start_draw_line],eax
     jb   @f
     ret
@@:
     inc   [right_start_draw_line]
     jmp   .all
;---------------------------------------------------------------------
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
     cmp   [left_start_draw_line],0
     jae   .draw
     mov   [left_start_draw_line],0
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
     cmp   [right_start_draw_line],0
     jae   .draw
     mov   [right_start_draw_line],0
.draw:
     call draw_right_panel
     mov   [scroll_bar_event],1
.end:
     popa

     ret
;---------------------------------------------------------------------
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
     jmp   .all
;---------------------------------------------------------------------
draw_all_panels_for_scroll_bar:
     push eax
     push ebx
     call draw_right_panel
     call draw_left_panel
     pop  ebx
     pop  eax
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
draw_scroll_bar_left:
     pusha
     mov   eax,[left_folder_block]
     cmp   eax,0
     jne   @f
     inc   eax
@@: 
     mov   ebx,[max_panel_line]
     xor   edx,edx
     div   ebx
     cmp   eax,0
     jne   @f
     inc   eax
@@:
     mov   ebx,eax
     xor   eax,eax
     mov   ax,[left_panel_x.size_x]
     sub   eax,30
     xor   edx,edx
     div   ebx
     shr   eax,1
     cmp   eax,5
     jae    @f
     mov   eax,5
@@:
     mov   [left_scroll_compens],eax
     popa
     
     mov   [scroll_pointer_1],1
     pusha
     mov   bx,[left_panel_x.start_x]
     add   bx,[left_panel_x.size_x]
     sub   bx,15
     shl   ebx,16
     mov   bx,15
     mov   ecx,[left_panel_y]
     call  calculate_for_draw_scroll_bar
     push  ecx
     mov   ax,[left_panel_y.size_y]
     sub   eax,30  ;45
     sub   eax,[left_scroll_compens]
     sub   eax,[left_scroll_compens]
     shl   eax,10
     push  ebx
     mov   ebx,[left_folder_block]
     call  calculate_for_draw_scroll_bar_2
     mov   ebx,[left_start_draw_line]
     imul  eax,ebx
     pop   ebx
     call  calculate_for_draw_scroll_bar_1
     pop   ecx
     ror   ecx,16
     add   cx,[left_panel_y.size_y]
     call  calculate_for_draw_scroll_bar_3
     popa
     ret
;---------------------------------------------------------------------
draw_scroll_bar_right:
     pusha
     mov   eax,[right_folder_block]
     cmp   eax,0
     jne   @f
     inc   eax
@@: 
     mov   ebx,[max_panel_line]
     xor   edx,edx
     div   ebx
     cmp   eax,0
     jne   @f
     inc   eax
@@:
     mov   ebx,eax
     xor   eax,eax
     mov   ax,[right_panel_x.size_x]
     sub   eax,30
     xor   edx,edx
     div   ebx
     shr   eax,1
     cmp   eax,5
     jae    @f
     mov   eax,5
@@:
     mov   [right_scroll_compens],eax
     popa
     mov   [scroll_pointer_1],2
     pusha
     mov   bx,[right_panel_x.start_x]
     add   bx,[right_panel_x.size_x]
     sub   bx,15
     shl   ebx,16
     mov   bx,15
     mov   ecx,[right_panel_y]
     call  calculate_for_draw_scroll_bar
     push  ecx
     mov   ax,[right_panel_y.size_y]
     sub   eax,30  ;45
     sub   eax,[right_scroll_compens]
     sub   eax,[right_scroll_compens]
     shl   eax,10
     push  ebx
     mov   ebx,[right_folder_block]
     call  calculate_for_draw_scroll_bar_2
     mov   ebx,[right_start_draw_line]
     imul  eax,ebx
     pop   ebx
     call  calculate_for_draw_scroll_bar_1
     pop   ecx
     ror   ecx,16
     add   cx,[right_panel_y.size_y]
     call  calculate_for_draw_scroll_bar_3
     popa
     ret
;---------------------------------------------------------------------
calculate_for_draw_scroll_bar:
     mov   edx,0xeeeeee
     mov   eax,13
     pusha
     add   ebx,1 shl 16
     mov   bx,14
     add   ecx,16 shl 16
     sub   cx,31
     int   0x40
     popa
     xor   edx,edx
     push  ebx
     mov   bx,1
     int   0x40
     push  ecx
     add   ebx,15 shl 16
     int   0x40
     pop   ecx
     pop   ebx
     mov   [draw_scroll_character],1
     call  draw_scroll_bar_srectangle
     xor  eax,eax
     ret
;---------------------------------------------------------------------
calculate_for_draw_scroll_bar_1:
     shr   eax,10
     ror   ecx,16
     add   cx,15
     cmp   [scroll_pointer_1],1
     jne   .right
     add   ecx,[left_scroll_compens]
     jmp   @f
.right:
     add   ecx,[right_scroll_compens]
@@: 
     add   cx,ax
     rol   ecx,16
     mov   [draw_scroll_character],2
     call  draw_scroll_bar_srectangle
     ret
;---------------------------------------------------------------------
calculate_for_draw_scroll_bar_2:
     sub   ebx,[max_panel_line]
     cmp  ebx,0
     ja    @f
     mov   ebx,1
@@:
     xor   edx,edx
     div   ebx
     shl   edx,1
     cmp   edx,ebx
     jb    @f
     inc   eax
@@:
     ret
;---------------------------------------------------------------------
calculate_for_draw_scroll_bar_3:
     sub   cx,16
     rol   ecx,16
     mov   [draw_scroll_character],3
     call  draw_scroll_bar_srectangle
     ret
;---------------------------------------------------------------------
;calculate_for_draw_scroll_bar_4:
;     push  eax
;     push  ebx
;;     shr   eax,10
;     mov   ebx,eax
;     xor   eax,eax
;     mov   ax,[left_panel_y.size_y]
;     xor   edx,edx
;     div   ebx
;;     mov   esi,eax
;     pop   ebx
;     pop   eax
;     ret
;---------------------------------------------------------------------
draw_scroll_bar_srectangle:
     push  ecx
     xor   edx,edx
     mov   eax,13
     mov   cx,1
     cmp   [draw_scroll_character],2
     je   .draw_center
     int   0x40
     pusha
     add   ecx,1 shl 16
     add   ebx,1 shl 16
     mov   cx,14
     mov   bx,14
     mov   edx,0xbbddff
     int   0x40
     xor   edx,edx
     cmp   [draw_scroll_character],1
     jne   .draw_down
     add   ebx,3 shl 16
     mov   bx,7
     add   ecx,8 shl 16
     mov   cx,1
     int   0x40
     sub   ecx,1 shl 16
     add   ebx,1 shl 16
     mov   bx,5
     int   0x40
     sub   ecx,1 shl 16
     add   ebx,1 shl 16
     mov   bx,3
     int   0x40
     sub   ecx,1 shl 16
     add   ebx,1 shl 16
     mov   bx,1
     int   0x40
     jmp   .continue
.draw_down:
     add   ebx,3 shl 16
     mov   bx,7
     add   ecx,5 shl 16
     mov   cx,1
     int   0x40
     add   ecx,1 shl 16
     add   ebx,1 shl 16
     mov   bx,5
     int   0x40
     add   ecx,1 shl 16
     add   ebx,1 shl 16
     mov   bx,3
     int   0x40
     add   ecx,1 shl 16
     add   ebx,1 shl 16
     mov   bx,1
     int   0x40
.continue:
     popa
     add   ecx,15 shl 16
     int   0x40
     pop   ecx
     ret

.draw_center:
     pusha
     add   ecx,1 shl 16
     add   ebx,1 shl 16
     push  ecx
     cmp   [scroll_pointer_1],1
     jne   .right
     ror   ecx,16
     sub   ecx,[left_scroll_compens]
     rol   ecx,16
     xor   cx,cx
     add   ecx,[left_scroll_compens]
     shl   cx,1
     jmp   .draw_center_continue
.right:
     ror   ecx,16
     sub   ecx,[right_scroll_compens]
     rol   ecx,16
     xor   cx,cx
     add   ecx,[right_scroll_compens]
     shl   cx,1
.draw_center_continue:
     mov   bx,14
     mov   edx,0xbbddff
     int   0x40
     xor   edx,edx
     mov   cx,1
     sub   ecx,1 shl 16
     int   0x40
     pop   ecx
     cmp   [scroll_pointer_1],1
     jne   .right_1
     cmp   [left_scroll_compens],8
     jb    .continue_1
     jmp   .draw_center_continue_1
.right_1:
     cmp   [right_scroll_compens],8
     jb    .continue_1
.draw_center_continue_1:     
     sub   ecx,8 shl 16
     add   ebx,2 shl 16
     mov   bx,10
     add   ecx,4 shl 16
     mov   cx,1
     int   0x40
     add   ecx,3 shl 16
     sub   ebx,1 shl 16
     mov   bx,12
     int   0x40
     add   ebx,1 shl 16
     mov   bx,10
     add   ecx,3 shl 16
     int   0x40
.continue_1:
     popa
     ror   ecx,16
     cmp   [scroll_pointer_1],1
     jne   @f
     add   ecx,[left_scroll_compens]
     jmp   .draw_center_continue_2
@@:
     add   ecx,[right_scroll_compens]
.draw_center_continue_2:
     rol   ecx,16
     int   0x40
     pop   ecx
     ret
;---------------------------------------------------------------------
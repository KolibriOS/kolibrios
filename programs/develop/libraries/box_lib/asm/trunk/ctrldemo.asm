;
; Example for Box_lib: scrollbar, menubar, dinamic_button
; Copyright (c) 2009, Mario79
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
;       * Redistributions of source code must retain the above copyright
;       notice, this list of conditions and the following disclaimer.
;       * Redistributions in binary form must reproduce the above copyright
;       notice, this list of conditions and the following disclaimer in the
;       documentation and/or other materials provided with the distribution.
;       * Neither the name of the <organization> nor the
;       names of its contributors may be used to endorse or promote products
;       derived from this software without specific prior written permission.
;
; THIS SOFTWARE IS PROVIDED BY Mario79 ''AS IS'' AND ANY
; EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
; WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
; DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
; DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
; (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
; ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
; SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;*****************************************************************************
; import_boxlib procedure written by <Lrz>
;---------------------------------------------------------------------

        use32
        org     0x0

        db      'MENUET01'
        dd      0x01
        dd      START
        dd      IM_END
        dd      I_END
        dd      stacktop
        dd      0x0
        dd      path

include '../../../../../macros.inc'
include '../../load_lib.mac'
;---------------------------------------------------------------------
;---    Start of program        ----------------------------------------------
;---------------------------------------------------------------------
START:
        mcall 68, 11
        mcall 66, 1, 1
        mcall 40, 0x27
;---------------------------------------------------------------------  
; Procinfo area for function 9 in MenuBar 
        mcall   68,12,1024
        mov     [menu_data_1.procinfo],eax
        mov     [menu_data_2.procinfo],eax
;---------------------------------------------------------------------  
; loading Box_Lib library 
load_libraries l_libs_start,end_l_libs
;sys_load_library  boxlib_name, path, file_name, system_dir, \
;er_message_found_lib, ihead_f_l, myimport, er_message_import, ihead_f_i
;        test    eax,eax
;        jnz  button.exit

;----------------------------------old
;        mov     ebx,boxlib_name
;        mov     esi,path
;        mov     edi,file_name
;        call    copy_path
;
;        mcall 68,19,file_name ; load of alternative
;        test    eax,eax
;        jnz @f 
;        
;        mcall 68,19,system_dir ; load of sys directory
;        test    eax,eax
;        jz      button.exit
;@@:
;        call    import_boxlib
;---------------------------------------------------------------------
; Load and convert BMP file for DinamicButton
; This procedures use cnv_bmp.obj and this module used only for
; demonstration of use DinamicButton

;        mov     ebx,plugin_BMP_name
;        mov     esi,path
;        mov     edi,file_name
	copy_path   plugin_BMP_name,path,file_name
;
        mcall 68,19,file_name
        test    eax,eax
        jz  button.exit
        
        mov     eax,[eax+4]
        mov     [plugin],eax


;проверка на сколько удачно загузилась наша либа
        mov     ebp,library01
        cmp     dword [ebp+ll_struc_size-4],0
        jnz     button.exit
;
;        mov     ebp,library02
;        cmp     dword [ebp+ll_struc_size-4],0x2
;        jnz     button.exit
;        mov     eax,dword [ebp+36]
;        mov     eax,[eax+4]
;        mov     [plugin],eax
        
 

;        mov     ebx,icons_file_name
;        mov     esi,path
;        mov     edi,file_name
        copy_path    icons_file_name,path,file_name
        
        mcall 70, fileinfo

        mov     [fileinfo+0],dword 0

        mov     ecx,[file_info+32]
        mov     [fileinfo+12],ecx
        mov [img_size],ecx
        

        mcall 68, 12


        mov     [fileinfo+16],eax
        mov     [image_file],eax


        mcall 70, fileinfo
        
        xor     eax,eax
        mov     [return_code],eax
        mov     eax,image_file
        call    [plugin]
        
        mov     ecx,[image_file]
        mcall 68, 13,
        
        cmp     [return_code],dword 0
        jne     button.exit
        
        mov     ebx,[raw_pointer]
        mov     eax,[ebx+4]
        ; set of button size
        mov     [dinamic_button_data_1.size_x],ax
        mov     [dinamic_button_data_1.size_y],ax
        mov     [dinamic_button_data_2.size_x],ax
        mov     [dinamic_button_data_2.size_y],ax
        mov     eax,[ebx+12]
        ; set of RAW resolution to pixel
        mov     [dinamic_button_data_1.resolution_raw],eax
        mov     [dinamic_button_data_2.resolution_raw],eax
        mov     eax,[ebx+20]
        add     eax,ebx
        ; set RAW palette, use else resolution 8bit or less
        mov     [dinamic_button_data_1.palette_raw],eax
        mov     [dinamic_button_data_2.palette_raw],eax
        mov     eax,[ebx+28]
        add     eax,ebx
        ; set RAW area for passive button
        mov     [dinamic_button_data_1.passive_raw],eax
        mov     [dinamic_button_data_2.passive_raw],eax
        mov     ecx,[ebx+4]
        imul ecx,[ebx+4]
        imul ecx,[ebx+12]
        shr     ecx,3
        add     eax,ecx
        ; set  RAW area for active button
        mov     [dinamic_button_data_1.active_raw],eax
        mov     [dinamic_button_data_2.active_raw],eax
        add     eax,ecx
        ; RAW area for click button
        mov     [dinamic_button_data_1.click_raw],eax
        mov     [dinamic_button_data_2.click_raw],eax
;---------------------------------------------------------------------  
red:
        call draw_window
still:
        mcall 10

        cmp     eax,1
        je      red
        cmp     eax,2
        je      key
        cmp     eax,3
        je      button
        cmp     eax,6
        je      mouse
        jmp     still


;---------------------------------------------------------------------
        key:
        mcall 2
        jmp     still
;---------------------------------------------------------------------
        button:
        mcall 17
        cmp     ah, 1
        jne     still
        .exit:
        mcall -1
;---------------------------------------------------------------------
mouse:
;-----------------------------------------------
        cmp     [scroll_bar_data_horizontal.delta2],0
        jne     .horizontal
.vertical:
    mov   eax,[scroll_bar_data_vertical.max_area]
    cmp   eax,[scroll_bar_data_vertical.cur_area]
    jbe   .horizontal
; mouse event for Vertical ScrollBar
        push dword scroll_bar_data_vertical
        call [scrollbar_ver_mouse]
        mov  eax,scroll_bar_data_vertical.redraw
        xor  ebx,ebx
        cmp     [eax],ebx
        je      @f
        mov     [eax],ebx
        jmp     .draw_cube
@@:
        cmp     [scroll_bar_data_vertical.delta2],0
        jne     still
.horizontal:
    mov   eax,[scroll_bar_data_horizontal.max_area]
    cmp   eax,[scroll_bar_data_horizontal.cur_area]
    jbe   .other
; mouse event for Horizontal ScrollBar
        push dword scroll_bar_data_horizontal
        call [scrollbar_hor_mouse]
        mov  eax,scroll_bar_data_horizontal.redraw
        xor  ebx,ebx
        cmp     [eax],ebx       
        je      .other
        mov     [eax],ebx
.draw_cube:
        call draw_cube
        jmp     still
.other:
        cmp  [scroll_bar_data_vertical.delta2],0
        jne   still
        cmp  [scroll_bar_data_horizontal.delta2],0
        jne   still
;-----------------------------------------------
.menu_bar_1:
        call   .set_mouse_flag
@@:
; mouse event for Menu 1
        push    dword menu_data_1
        call    [menu_bar_mouse]
    cmp   [menu_data_1.click],dword 1
        jne   .menu_bar_2
        cmp   [menu_data_1.cursor_out],dword 0
        jne    analyse_out_menu_1
        jmp    .menu_bar_1
;--------------------------------------------
.menu_bar_2:
; mouse event for Menu 2
        push    dword menu_data_2
        call    [menu_bar_mouse]
    cmp   [menu_data_2.click],dword 1
        jne   .mouse_dinamic_button
        cmp   [menu_data_2.cursor_out],dword 0
        jne    analyse_out_menu_2
    jmp    .menu_bar_1
;--------------------------------------------
.mouse_dinamic_button:
; mouse event for Dinamic Button 1
        push    dword dinamic_button_data_1
        call    [dinamic_button_mouse]
        mov   eax,dinamic_button_data_1.click
    cmp   [eax],dword 1
        jne   @f
    mov   [eax],dword 0
        jmp   about
@@:
; mouse event for Dinamic Button 2
        push    dword dinamic_button_data_2
        call    [dinamic_button_mouse]
        mov   eax,dinamic_button_data_2.click
    cmp   [eax],dword 1
        jne   still  ;@f
    mov   [eax],dword 0
        jmp   button.exit
;---------------------------------------------------------------------
.set_mouse_flag:        
        xor   eax,eax
        inc   eax
        mov   [menu_data_1.get_mouse_flag],eax
        mov   [menu_data_2.get_mouse_flag],eax
        ret
;---------------------------------------------------------------------
analyse_out_menu_1:
; analyse result of Menu 1
        cmp [menu_data_1.cursor_out],dword 3
        je      button.exit     
        jmp     still
        
analyse_out_menu_2:
; analyse result of Menu 2
        cmp [menu_data_2.cursor_out],dword 2
        je      about
        jmp     still
;---------------------------------------------------------------------
        about:
        mcall 51,1,thread3,thread
        jmp     still
;---------------------------------------------------------------------
draw_window:
        mcall 12, 1
        mcall 0, <0,400>, <0,400>, 0x03AABBCC, 0x805080D0, 0x005080D0
        mcall 71, 1, header_1
;---------------------------------------------
; draw for Menu 1
        push    dword menu_data_1
        call    [menu_bar_draw] 
; draw for Menu 2
        push    dword menu_data_2
        call    [menu_bar_draw] 
;---------------------------------------------
; draw for Dinamic Button 1
        push    dword dinamic_button_data_1
        call    [dinamic_button_draw]
; draw for Dinamic Button 2
        push    dword dinamic_button_data_2
        call    [dinamic_button_draw]
;---------------------------------------------
; set all_redraw flag for draw all ScrollBar
; In some cases it is necessity to draw only the area
; of moving of a "runner", for acceleration of output - 
; in this case the flag needs to be reset to 0 (zero).
        xor     eax,eax
        inc     eax
        mov     [scroll_bar_data_vertical.all_redraw],eax
        mov     [scroll_bar_data_horizontal.all_redraw],eax
        
; draw for Vertical ScrollBar
        push     dword scroll_bar_data_vertical
        call     [scrollbar_ver_draw]
; draw for Horizontal ScrollBar
        push     dword scroll_bar_data_horizontal
        call     [scrollbar_hor_draw]
; reset all_redraw flag 
        xor     eax,eax
        mov     [scroll_bar_data_vertical.all_redraw],eax
        mov     [scroll_bar_data_horizontal.all_redraw],eax
;---------------------------------------------
        call draw_cube
        mcall 12, 2
        ret
;---------------------------------------------------------------------
draw_cube:
        mcall 13,<30,301>,<50,301>,0xafafaf
        mov     ecx,[scroll_bar_data_vertical.position]
        add     ecx,50
        shl     ecx,16
        mov     cx,30
        mov     ebx,[scroll_bar_data_horizontal.position]
        add     ebx,30
        shl     ebx,16
        mov     bx,30
        mcall 13, , , 0x0
        ret

include 'data.inc'
include 'w_about.inc'
IM_END:
        rb 1024
thread:
        rb 1024
stacktop:
path:
        rb      4096
file_name:
        rb 4096
file_info:
        rb 40
I_END:


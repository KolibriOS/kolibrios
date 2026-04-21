; Copyright (c) 2009, <Lrz>
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
; THIS SOFTWARE IS PROVIDED BY Alexey Teplov aka <Lrz> ''AS IS'' AND ANY
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

error:
.rest_value:    
        mov     di, ax;restore value after repe cmpsb
        mov     cx, bx
        jmp     ret_on_ch       ;return 

;///// ошибка при находжении длинны секции в параметре default
.error_get_size_d_sect: 
        leave   ;clear array in stack 
        mov     si, not_found_def_sect
        jmp     err_show_ini

;/////ERROR
.not_loader:
        leave   ;clear array in stack 
        mov     si, not_found_sec_loader
        jmp     err_show_ini

.default_eq_loader:     ;критическая ошибка default секция = loader
        leave
        mov     si, default_eq_loader
        jmp     err_show_ini
.correct_exit_bl:
        leave
        mov     si, point_to_default_sec_not_found
        jmp     err_show_ini
.incorect_section_def:
        leave
        mov     si, incorect_section_define
        jmp     err_show_ini
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;show message error
.LoaderModule:
        push    word 0xb800
        pop     es
        



        ret

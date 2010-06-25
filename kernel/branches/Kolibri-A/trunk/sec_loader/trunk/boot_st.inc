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

;======================================================================
;
;                           BOOT DATA
;
;======================================================================


version      db 'Secondary Loader v 0.010',0
version_end:

select_section db 'Select section:'
select_section_end:
section_description db 'Section description:'
section_description_end:
soft_mes        db 'Soft (c) 2008-2009'
soft_mes_end:

badprocessor db      '>Fatal - CPU 586+ required.',0
error_ini_f1 db      '>Error: cannot load ini file, buffer is full',0
error_ini_f2 db      '>Error: ini file not found',0
error_ini_f3 db      '>Error: cannot read ini file',0
error_ini_nf db      '>Error: unrecognized error when loading ini file',0
not_found_sec_loader db '>Not found section [loader]',0
not_found_def_sect   db '>Not found value default in section [loader]',0
default_eq_loader    db '>Error in section [loader] parametr default=loader',0
found_equal_default  db '>Found equal parametr default will be use first value',0
found_equal_timeout  db '>Found equal parametr timeout will be use first value',0
set_default_timeout_val db '>Section timeout has incorrect value, will be use default value',0
error_ini_common db  ">I will use predefined settings and try to boot. Let's hope for the best..."
             db      13,10,"Press any key to continue...",0
load_ini     db      '>Ini file loaded successfully',0
parse_ini_end db     '>End parsing ini file',0
point_to_default_sec_not_found db '>Point to default section is not found in ini file',0
incorect_section_define db ">Incorect section define not found ']'",0
default_section_name    db '"Section unname"'

start_msg db      "Press any key to change default section, press [Enter] to continue booting"
start_msg_e:
time_msg  db      "or wait 4 seconds before default continuation"
time_msg_e:
time_str  db      "seconds before default continuation"
time_str_e:

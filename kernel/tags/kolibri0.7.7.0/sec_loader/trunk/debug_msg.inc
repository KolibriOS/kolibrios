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

;??? ?????????? ??? ?????????, ??????? ????? ? ???????? ???????, ? ?????? ?? ????? ? ??????? ????? ?????????.
If DEBUG
cseg_msg	db '       - Adress of code segment',0
stack_msg	db 'Set stack & segments is have completed',0
show_string	db 'Have loaded size:'
show_decode	db '      ',0
show_db1	db '       -Message debug1',0
show_db2	db '       -Message debug2',0


lm_l_found      db '[loader] is found',0
lm_lf_timeout   db 'timeout is found',0
lm_lf_default   db 'name default is found and end parsing section',0
lm_lf_section   db 'found section [',0
lm_lf_default_f db 'found default parametr',0
lm_l_end        db 'section [loader] is end',0
show_all_sect   db 'SHOW ALL Sections',0
no_show_only_w  db 'Not show sections - only work on default sect',0
_not_found      db '[ not found',0
_found_1        db '[] found',0
_found_2        db '[ found',0
say_hello       db 'Hello $)',0
ramdiskFS_st    db 'Start use_RamdiskFS macros',0
free_memory_msg db '       -Kb availability system free memory',0
RamdiskSize_msg db '       -Kb equal RamdiskSize',0
RamdiskSector_msg  db '        -byts RamdiskSector',0
RamdiskCluster_msg db '       -RamdiskCluster',0
RamdiskFile_msg    db '       -size RamdiskFile',0
fat_create_msg  db '       -first create fat table, point to next block',0
BPB_msg         db '       -in byte, why we get data from move BPB struct',0
firstDataSect_msg  db '       -first data sector, offset to data in sectors',0
size_root_dir_msg  db '       -size root dir in sectrors',0
DataClasters_msg   db '       -size data in Clasters',0
first_entry_in_fat db '       -data segment in FIRST entry FAT',0
check_root_fat_    db '      :     --------------',0
check_name_fat_msg_y db 'Name is present that is BAD',0
check_name_fat_msg_n db 'Name is not present that is GOOD',0
name_of_seg_get_64 db '       -name of seg where we get 64 Kb of data',0
convertion_file_name_msg_y db '->Destination name of file is GOOD',0
convertion_file_name_msg_n db '->Destination name of file is BAD',0
alarm_msg 	db '%%%%%%%% WARNING: MISS THE FILE %%%%%%%%%%%',0
start_making_FAT12_msg db '>>>>>> Begin make a RAMDISK and FS after 1 Mb <<<<<<<',0
make_fat12_RFS_msg db '-Make FAT12 Ram FS',0
get_type_FS_msg    db '-End make RamDisk',0
seg_where_get_data db '      - Segment where we get data for move up file',0
return_code_af_move     db '    -return code after 0x87 int 0x15, move block',0
return_code_af_fat_m    db '    -return code after 0x87 int 0x15, move fat struc',0



end if

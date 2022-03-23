  use32
  org    0

  db     'MENUET01'
  dd     1
  dd     START
  dd     I_END
  dd     MEM
  dd     STACKTOP
  dd     PARAM
  dd     0

include '..\..\macros.inc'
START:
       mcall 40, 0x05
       mov  eax, [PARAM]
       test eax, eax
       jnz  save_data
still:
       call draw
       mcall 23, 200   ;2 seconds

       cmp  eax,1
       je   still
       cmp  eax,3
       jnz   still

       mcall 17
       cmp eax,1
       jz  still
       dec ah
       jz  exit
       dec ah
       jnz still
       ;2 - button save in file
exit:
       mcall -1
draw:
       mcall 68, 31, 1
       mov [final_pointer], ecx
       mov [start_pointer], ebx
       mcall 48,3,sc,sc,sizeof.system_colors  ;get system colors
       xor eax,eax
       mov ebx, 0x00ff017f
       mov ecx, 0x001501cc
       mov edx, 0x14000000
       add edx, [sc.work]
       mov edi,title
       mcall

       ;mov eax, 13
       ;mov ebx, 0x00080170
       ;mov ecx, 0x0037018c
       ;mov edx, [sc.work_text]
       ;mcall
        mcall 13, 0x00080170, 0x00370003, [sc.work_text]
        mcall , , 0x01c00003, ;[sc.work_text]
        mcall , 0x00080003, 0x0037018c, ;[sc.work_text]
        mcall , 0x01780003, , ;[sc.work_text]

       ;mov eax, 13
       ;mov ebx, 0x000c016a
       ;mov ecx, 0x003a0185
       ;mov edx, [sc.work]
       ;mcall

       mov eax, 4
       mov ebx, 0x000f001f
       mov ecx, 0x90000000
       add ecx, [sc.work_text]
       mov edx, table_title
       mcall

       push dword[start_pointer]
@@:
       ;call get_next_service
       mov eax, 68
       mov ebx, 31
       mov ecx, 2
       mov edx, [esp]
       mov edi, data_list
       mcall

       mov eax, 4
       mov ebx, print_1
       add ebx, [index_service]
       mov ecx, 0xd0000000  ;draw bkgr
       add ecx, [sc.work_text]
       mov edx, data_list
       mov edi, [sc.work]
       mcall

       mov eax, 47
       mov ebx, default_settings_47
       mov ecx, [esp]
       mov edx, print_2
       add edx, [index_service]
       mov esi, 0x50000000
       add esi, [sc.work_text]
       mov edi, [sc.work]
       mcall

       mov eax, 47
       mov ebx, default_settings_47
       mov ecx, [data_list.srv_proc]
       mov edx, print_3
       add edx, [index_service]
       mov esi, 0x50000000
       add esi, [sc.work_text]
       mov edi, [sc.work]
       mcall

       add dword[index_service],15
       pop eax
       cmp eax,[final_pointer]  ;eax - this_pointer
       push dword[data_list.fd]
       jnz @b
       pop eax
       and dword[index_service], 0 ; clear
       ret
;-------------------------------------------------------------------------------
;This function get 'this_pointer' in stack, stack don`t used
;get_next_service
;        mov eax, 68
;        mov ebx, 31
;        mov ecx, 2
;        mov edx, [esp+4]
;        mov edi, data_list
;        mcall
;        ret
;-------------------------------------------------------------------------------
save_data:
        mcall 70,file_str
        mov  dword[file_str], 3
        mov  dword[file_str.size], 36 ;40
        mov  dword[file_str.addr], data_list
        mcall 68, 31, 1
        mov [final_pointer], ecx
        mov [start_pointer], ebx
        push dword[start_pointer]
@@:
        mov  eax, 68
        mov  ebx, 31
        mov  ecx, 2
        mov  edx, [esp]
        mov  edi, data_list
        mcall
        mcall 70, file_str
        add  dword[file_str.pos], 36 ;40
        pop  eax
        cmp  eax,[final_pointer]  ;eax - this_pointer
        push dword[data_list.fd]
        jnz  @b
        pop  eax
        jmp  still
        ret
title:  db 'list_services',0
table_title: db '  Service_name   |  Handler  |  Service_proc',0
index_service:  dd 0
start_pointer:  dd 0
final_pointer:  dd 0

data_list:
.name:         rb 16
.fd:           rd 1
.bk:           rd 1
.base:         rd 1
.entry:        rd 1
.srv_proc:     rd 1

file_str:
        dd      2 ;rewrite this value = 3
.pos:   dd      0
        dd      0
.size:  dd      0
.addr:  dd      0
        ;db      '/tmp0/1/log_services.log',0
        db      0
        dd      PARAM
sc system_colors

default_settings_47 = 0x00080100
print_1 = 0x0015003f
print_2 = 0x00ac003f
print_3 = 0x0113003f
PARAM:  rb 256
I_END:
        rb 256
align 16
STACKTOP:
MEM:
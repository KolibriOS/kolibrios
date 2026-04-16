;---------------------------------------------------------------------
device_detect_f70:
    mov   ax,[select_disk_char]
    mov   [read_folder_2_name],ax
    mov   [read_folder_3_name],ax
;    mov   eax,left_folder_data_1
;    mov   [read_folder_2.return],eax
    mcall 70, read_folder_2
    test  eax,eax
    jz    @f
    cmp   eax,6
    jne   read_folder_error
@@:
    mov   [left_folder_block_1],ebx
    mov   [temp_counter_1],0
    mov   [retrieved_devices_table_counter],0
.start_temp_counter_1:
    imul  esi,[temp_counter_1],304
    add   esi,left_folder_data_1
    add   esi,32+40   ;[eax+32+40]
    call  copy_folder_name_3
;    mov   eax,right_folder_data_1
;    mov   [read_folder_3.return],eax
    mcall 70, read_folder_3
    test  eax,eax
    jz    @f
    cmp   eax,6
    jne   read_folder_1_error
@@:
    mov   eax,right_folder_data_1
    cmp   [eax+4],dword 0
    je    .continue
    mov   [right_folder_block_1],ebx
    mov   ebp,0
.start_copy_device_patch:
    imul  edi,[retrieved_devices_table_counter],10
    add   edi,retrieved_devices_table
    mov   [edi],byte '/'
    inc   edi
    imul  esi,[temp_counter_1],304
    add   esi,left_folder_data_1
    add   esi,32+40  ;[eax+32+40]
    call  proc_copy_patch
    imul  esi,ebp,304
    add   esi,right_folder_data_1
    add   esi,32+40   ;[eax+32+40]
    mov   [edi-1],byte '/'
    call  proc_copy_patch
    inc   [retrieved_devices_table_counter]
    inc   ebp
    cmp   ebp,[right_folder_block_1]
    jb	  .start_copy_device_patch
.continue:
    inc   [temp_counter_1]
    mov   eax,[temp_counter_1]
    cmp   eax,[left_folder_block_1]
    jb	  .start_temp_counter_1
    ret
;---------------------------------------------------------------------    
select_starting_directories:
    mov   esi,retrieved_devices_table+1
    call  copy_folder_name
    mov   esi,retrieved_devices_table+3
    mov   ecx,0
@@:
    add   esi,8
    cld
    lodsw
    inc   ecx
    cmp   ecx,[retrieved_devices_table_counter]
    ja	  @f
    cmp   ax,'hd'
    jne   @r
    sub   esi,2
    call  copy_folder_name_1
    ret
@@:
;    mov   [read_folder_1.name+1],dword 0
;    mov   [read_folder_1_name+1],dword 0
    mov   esi,retrieved_devices_table+1
    call  copy_folder_name_1
;    mov   [read_folder_1_name],dword '/rd/'
;    mov   [read_folder_1_name+4],word '1/'
;    mov   [read_folder_1_name+6],byte 0
    ret
;---------------------------------------------------------------------
proc_copy_patch:
@@:
    cld
    lodsb
    stosb
    cmp   al,0
    jne   @r
    ret
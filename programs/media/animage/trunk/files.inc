;-----------------------------------------------------------
;---------------save in file from memory--------------------
;-----------------------------------------------------------
save_file:
         mov [size_to_save],ecx
         mov [pointer_to_data],ebx
         mov [full_file_path],eax

         mov esi,[full_file_path]
         mov edi,filepath
         mov ecx,256
         rep movsd

         xor eax,eax
         mov ebx,[size_to_save]
         mov ecx,[pointer_to_data]
         mov edx,filepath

         mov dword[file_information],SSF_CREATE_FILE
         mov [file_information+4],eax
         mov [file_information+8],eax
         mov [file_information+12],ebx
         mov [file_information+16],ecx
         mov [file_information+20],al
         mov [file_information+21],edx

         mcall SF_FILE,file_information

	ret

;----------------------------------------------------------
;-------------file information structure-------------------
;----------------------------------------------------------

file_information FileInfoBlock 5,0,0,0,bufer_for_info,0,filepath

filepath rb 1040 

bufer_for_info  rd 10
full_file_path  dd 0
where_load_file dd 0
size_to_save    dd 0
pointer_to_data dd 0
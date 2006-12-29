;-----------------------------------------------------------
;-----------load heading of file in memory------------------
;-----------------------------------------------------------

load_heading_of_file:

         mov [where_load_file],ebx
         mov [full_file_path],eax

         mov esi,[full_file_path]
         mov edi,filepath
         mov ecx,256
         rep movsd

         xor eax,eax
         mov ecx,[where_load_file]
         mov edx,filepath

         mov [file_information],eax
         mov [file_information+4],eax
         mov [file_information+8],eax
         mov [file_information+12],dword 512
         mov [file_information+16],ecx
         mov [file_information+20],al
         mov [file_information+21],edx

         mov eax,70
         mov ebx,file_information
         int 0x40

         ret
;-----------------------------------------------------------
;--------load file in memory--------------------------------
;-----------------------------------------------------------
load_file:
         mov [where_load_file],ebx
         mov [full_file_path],eax

         mov esi,[full_file_path]
         mov edi,filepath
         mov ecx,256
         rep movsd

         xor eax,eax
         mov ebx,bufer_for_info
         mov ecx,filepath

         mov [file_information],dword 5
         mov [file_information+4],eax
         mov [file_information+8],eax
         mov [file_information+12],eax
         mov [file_information+16],ebx
         mov [file_information+20],al
         mov [file_information+21],ecx

         mov eax,70
	 mov ebx,file_information
         int 0x40
         
         test eax,eax
         jnz error_load_file

         xor eax,eax
         mov ebx,[bufer_for_info+32]
         mov ecx,[where_load_file]
         mov edx,filepath

         mov [file_information],eax
         mov [file_information+4],eax
         mov [file_information+8],eax
         mov [file_information+12],ebx
         mov [file_information+16],ecx
         mov [file_information+20],al
         mov [file_information+21],edx

         mov eax,70
         mov ebx,file_information
         int 0x40

         error_load_file:

	 ret
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

         mov [file_information],dword 2
         mov [file_information+4],eax
         mov [file_information+8],eax
         mov [file_information+12],ebx
         mov [file_information+16],ecx
         mov [file_information+20],al
         mov [file_information+21],edx

         mov eax,70
         mov ebx,file_information
         int 0x40

	 ret

;----------------------------------------------------------
;-------------file information structure-------------------
;----------------------------------------------------------

file_information:
 
  dd 5
  dd 0
  dd 0
  dd 0
  dd bufer_for_info
  dd 0
  dd filepath

filepath rb 1040 

bufer_for_info  rd 10
full_file_path  dd 0
where_load_file dd 0
size_to_save    dd 0
pointer_to_data dd 0
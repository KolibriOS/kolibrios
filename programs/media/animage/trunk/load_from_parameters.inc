; load picture from file to memory
align 4
open_b rb 560

load_picture:
    mov [file_information.Function], SSF_GET_INFO
    mov [file_information.Position], 0
    mov [file_information.Flags], 0
    mov dword[file_information.Count], 0
    mov dword[file_information.Buffer], open_b
    mov byte[file_information+20], 0
    mov dword[file_information.FileName], file_path
    mcall SF_FILE,file_information
	or eax,eax
	jnz .end_open

    mov ecx,dword[open_b+32] ;+32 qword: размер файла в байтах
	add ecx,mem_screen+mem_panel_b+mem_flood_f
	mcall SF_SYS_MISC,SSF_MEM_REALLOC,,[ScreenPointer]
	mov [ScreenPointer],eax
	add eax,mem_screen+mem_panel_b
	sub ecx,mem_screen+mem_panel_b+mem_flood_f
	mov [file_information.Function], SSF_READ_FILE
	mov [file_information.Position], 0
	mov [file_information.Flags], 0
	mov [file_information.Count], ecx
	mov [file_information.Buffer], eax
	mov byte[file_information+20], 0
	mov [file_information.FileName], file_path
	mcall SF_FILE,file_information ;загружаем файл изображения
	cmp ebx,0xffffffff
	je .end_open
		;определяем вид изображения и пишем его параметры
		mov eax,[ScreenPointer]
		add eax,mem_screen+mem_panel_b
		stdcall [img_decode], eax,ebx,0
		push eax
		;определяем размер декодированного изображения
		mov ecx,[eax+4] ;+4 = image width
		mov dword[Picture_SizeX],ecx
		mov eax,[eax+8] ;+8 = image height
		mov dword[Picture_SizeY],eax
		imul ecx,eax
		imul ecx,15
		add ecx,mem_screen+mem_panel_b+mem_flood_f
		mcall SF_SYS_MISC,SSF_MEM_REALLOC,,[ScreenPointer]
		mov [ScreenPointer],eax
		call prepare_data_pointers
		pop ebx
		stdcall [img_to_rgb2], ebx,[PointerToPicture] ;преобразуем изображение к формату rgb
		stdcall [img_destroy], ebx ;удаляем временный буфер с параметрами изображения
		mov [save_flag],1
	.end_open:
	ret

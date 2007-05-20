;-------------------------------------------------------------------------
;
;    Замена имен файлов
;
;
;     SPraid
;
;-------------------------------------------------------------------------

full_file_name_table dd 0		; указатель на память для таблицы путей 4К   запись 64 (32 - строка шаблон для поиска, 32 - строка шаблон для замены)

; перебирает по файлу варианты замен
proc full_file_name stdcall,sourc,dest
locals
  tmpsrc rb 260  ; временное хранение преобразования
endl
  pushad
  mov esi,[sourc]
  lea edi, [tmpsrc]

@@:
  lodsb
  stosb
  or al,al
  jnz @b

  mov al,[conf_file_loaded]					; требуется при обращении к первому файлу (конфигу)
  or al,al
  jnz @use_replace

  jmp full_file_name_exit


;--------------------------------
 @use_replace:

  xor eax,eax
.loop:
  push eax
  
  imul eax,eax,64
  add eax,[full_file_name_table] 
  mov ebx,eax
  add eax,32
  
  mov cl,[ebx]
  or cl,cl
  jz .done

  lea ecx, [tmpsrc]
  mov edx,[dest]

  stdcall full_file_name_parse, ecx,edx,ebx,eax

  mov esi,[dest]
  lea edi, [tmpsrc]
@@:
  lodsb
  stosb
  or al,al
  jnz @b

  pop eax
  inc eax
  jmp .loop

.done:
  pop eax
full_file_name_exit:

  lea eax, [tmpsrc]
  stdcall full_file_name_parse , eax , [dest], sysdir_ , sys_path
  popad
  ret
endp

;
;  Преобразовать имя в полное. Если в начале стоит
;     sourc - ссылка на строку...
;	  dest - ссылка на буфер куда результат ложить
;	  def - строка шаблон для поиска
;	  dval - к чему приравнивать шаблон
proc full_file_name_parse stdcall,sourc,dest,def,dval
  ; для одного
  pushad

  mov eax,[sourc]
  mov ebx,[def]
@@:
  mov dl,[ebx]
  mov cl,[eax]
  cmp cl,0
  je  @@bad_s
  cmp dl,0
  je  @@good

  or cl,0x20
  or dl,0x20
  cmp cl,dl
  jne  @@bad

  inc eax
  inc ebx
  jmp @b

@@bad_s:
  cmp dl,0
  je  @@good

@@bad:
  mov edi,[dest]
  mov esi,[sourc]
@@:
  lodsb
  stosb
  or al,al
  jnz	@b
  jmp @@ret_ok

@@good:
  push eax
  mov edi,[dest]
  mov esi,[dval]
@@:
  lodsb
  stosb
  or al,al
  jnz	@b

@@goodl:
  pop esi
  dec edi
@@:
  lodsb
  stosb
  or al,al
  jnz	@b

@@ret_ok:
  popad
  ret

endp

 sys_dir_mess: db 'System dir is '
 sys_path: db '/HD0/1/KOLIBRI',0,0
 sysdir_ db '%sys%',0

; берет параметры bx_from_load и исчит файл конфигурации
Parser_params:
  pushad
  mov ax,[OS_BASE+0x10000+bx_from_load]
  cmp al,'r' ; рам диск
  jnz @f
  mov [sys_path],dword '/RD/'
  mov [sys_path+4],byte ah
  mov [sys_path+5],word 0 ;0x002F
  jmp .done
@@:
  sub al,49
  mov [sys_path],dword '/HDa'
  mov [sys_path+3],byte al
  mov [sys_path+4],byte '/'
  mov [sys_path+5],byte ah
  mov [sys_path+6],dword '/KOL'
  mov [sys_path+10],dword 'IBRI'
  mov [sys_path+14],word 0 ;0x002F

.done:
  popad
  ret
  
proc load_file_parse_table
  pushad
  stdcall kernel_alloc,0x1000
  mov [full_file_name_table],eax

  invoke ini.enum_keys,conf_fname,conf_path_sect,get_every_key
  
  mov eax,[loading_counter]
  imul eax,eax,64
  add eax,[full_file_name_table]
  mov [eax],byte 0
  popad
  ret
endp

loading_counter dd 0
def_val_1 db 0

proc get_every_key stdcall,f_name, sec_name, key_name
  pushad
  mov esi,[key_name]
  mov edi,[loading_counter]
  imul edi,edi,64
  add edi,[full_file_name_table]
@@:
  lodsb
  stosb
  or al,al
  jnz	@b

  mov eax,[loading_counter]
  imul eax,eax,64
  add eax,[full_file_name_table]
  add eax,32
  mov ebx,[key_name]
  invoke ini.get_str,conf_fname,conf_path_sect,ebx,eax,32,def_val_1

  inc dword [loading_counter]
  
  popad
  mov eax,1
  ret
endp


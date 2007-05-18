;-------------------------------------------------------------------------
;
;    Замена имен файлов
;
;
;     SPraid
;
;-------------------------------------------------------------------------

; перебирает по файлу варианты замен
proc full_file_name stdcall,sourc,dest
locals
  param  rb 60
  val    rb 60
  tmpsrc rb 60	; временное хранение преобразования
endl
  pushad


  mov esi,[sourc]
;  mov edi,ebp
;  add edi,tmpsrc-ebp
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
;  mov ebx,ebp
;  add ebx,param-ebp
;  mov ecx,ebp
;  add ecx,val-ebp
  lea ebx, [param]
  lea ecx, [val]
  invoke ini.get_par,sect, ebx, ecx, eax

  mov bl,[param]
  or bl,bl
  jz .done

;  mov eax,ebp
;  add eax,tmpsrc-ebp
;  mov ebx,ebp
;  add ebx,param-ebp
;  mov ecx,ebp
;  add ecx,val-ebp
  lea eax, [tmpsrc]
  lea ebx, [param]
  lea ecx, [val]
  mov edx,[dest]

  stdcall full_file_name_parse, eax,edx,ebx,ecx

  mov esi,[dest]
;  mov edi,ebp
;  add edi,tmpsrc-ebp
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

;  mov eax,ebp
;  add eax,tmpsrc-ebp
  lea eax, [tmpsrc]
  stdcall full_file_name_parse , eax , [dest], sysdir_ , sys_path
  popad
  ret
endp

sect: db 'path',0

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


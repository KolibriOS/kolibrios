Color	       db 0
Counter        db 0
LastColor      db 0
SizeFile       dd 0
;LitlCounter    db 0
;LitlColor      db 0
save_image:
	   ;mov esi,addres_of_file_which_need_compress
	   ;mov edi,addres_where_need_to_save_compressed_file
	   ;mov ecx,size_of_file_which_need_compress
	   mov dl,byte [esi]
	   and dl,0xff
	   mov [LastColor],dl
	   mov [Counter],byte 1
	   mov [SizeFile],0
	   xor edx,edx
	   inc esi
  NextColor:;читаем новый цвет из таблицы
	   mov dl,byte [esi]
	   and dl,0xf
	   mov [Color],dl
	   cmp dl,[LastColor]
	   jne Different
	   ;увеличиваем счетчик на 1 и смотрим его значение
	   inc [Counter]
	   cmp [Counter],16
	   jl total
	   ;счетчик равен 16,значит нужно сохранить последовательность
	   dec [Counter]
	   jmp Different
	   ;Теперь текущий цвет стал старым,поэтому сохраним его как старый.
	   ;Обнулим (сделаем равным 1) счетчик повторений цвета(counter)
	   xor eax,eax
	   and [Counter],0
	   mov al,[Color]
	   mov [LastColor],al
	   xor eax,eax
	   jmp total
  Different:;прочитанный цвет не совпадает со старым
	   ;значит нужно сохранить старый и установить значения для нового
	   ;
	   ;сохраняем старый цвет
	   dec [Counter]
	   call SaveinByte
	   ;add [SizeFile],1
	   inc edi
	   ;устанавливаем значения текущего цвета как старого
	   mov dl,[Color]
	   mov [LastColor],dl
	   mov [Counter],1
     total:
	   xor edx,edx
	   mov dl,[Color]
	   mov [LastColor],dl
	   inc esi 
	   dec ecx
	   cmp	ecx,0
	   jne NextColor
	   ;счетчик может быть непустой,поэтому чтобы не терять байтов
	   ;сохраним значение счетчика и повторяемое значение
	   call SaveinByte
	   ret
SaveinByte:xor eax,eax
	   xor ebx,ebx
	   mov al,[Counter]
	   shl al,4
	   mov bl,[LastColor]
	   or al,bl
	   mov [edi],al
	   inc [SizeFile]
	   ret
;-----------------------------------------------------------------------
LitlCounter db 0
LitlColor   db 0
      unpakin:
           xor edx,edx
NextLitlColor:xor edx,edx
	   mov dl,byte[esi]
	   xor eax,eax
	   xor ebx,ebx
	   mov al,dl
	   mov bl,al
	   shr al,4
	   and al,0xf
	   mov [LitlCounter],al
	   and bl,0xf
	   mov [LitlColor],bl
	   xor eax,eax
	   xor ebx,ebx
;-------------------------------
	   mov al,[LitlColor]
       beg:mov [edi],al
	   add edi,1
	   dec [LitlCounter]
	   cmp [LitlCounter],-1
	   jne beg
	   inc esi
	   dec ecx
	   jnz NextLitlColor
	   ret
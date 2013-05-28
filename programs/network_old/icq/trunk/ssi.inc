;
;
; Поддержка контакт листа на сервере
;
;

;
; Заполняет таблицу с UIN
; и опционально таблицы с именем и доп. инфой
;


; Из comp.inc для отладки
;
;
; Массив с UIN
;
;UIN_LEN = 11            ; Длина
;UINS    = 15            ; Количество
;
;uins db UIN_LEN*UINS dup 0
; 
; массив со статусами
;
;stats dd UINS dup -1
;
; Массив с именами
;
;NAME_LEN = 30

;names db NAME_LEN*UINS dup 0


;
; Достает из item UIN 
; eax <- указатель на item
; Пропускает группы
;
 ssi_get_uin:
  push eax
  push ebx
  push ecx
  push edx

  ;
  ; Проверяем ItemID
  ;
  xor ebx, ebx
  mov bl, [eax + 1]     ; Length of the item name
  mov bh, [eax]       ;


  ;push ebx
  ;mov ebx, 128
  ;call print_mem

  ;pop ebx


  ;; FIXIT Разумнее проверять флаги
  ;; Если длина строки = 0
  ;; пропускаем item
  ;cmp ebx, 0
  ;jz ssi_get_end

  ;;
  ;;data_debug 'Item name len', ebx

  ;;+смещение до ItemID

  ;;xor ecx, ecx
  ;;mov ch, [eax + ebx + 4]
  ;;mov cl, [eax + ebx + 5]  ;	Item ID#

  ;;cmp ecx, 0
  ;;jz ssi_get_end

  ;
  ; Проверяем флаги, обрабатываем только записи UIN
  ;
  xor ecx, ecx
  mov ch, [eax + ebx + 6]
  mov cl, [eax + ebx + 7]

  
  cmp ecx, 0           ; 0x0000	  Buddy record (name: uin for ICQ and screenname for AIM)
  jz ssi_uin

  ; debug
  ;

  lea eax, [eax + ebx + 6]
  mov ebx, 2
  call print_mem


  jmp ssi_get_end


 ssi_uin:
  ;
  ; Копируем UIN в таблицу
  ;
  mov ecx, ebx  ; Длина строки
  lea eax, [eax + 2]

  mov edx, [uin_ind]
  cmp edx, UINS
  jnb ssi_get_end      ;Нет свободного места в таблице UIN

  imul edx, UIN_LEN
  mov ebx, uins
  lea ebx, [ebx + edx]

  call strcpy

  inc [uin_ind]

  ;debug
  ;mov eax, ebx
  ;xor ebx, ebx
  ;call writemsg


  ;
 ssi_get_end:
  pop edx
  pop ecx
  pop ebx
  pop eax
 ret


;
; eax <- указатель на item
; возвращает в eax указатель на следующий item
;
;

 ssi_next_item:

   push ebx
   push ecx

   xor ebx, ebx
   mov bl, [eax + 1]         ; длина UIN
   mov bh, [eax]             ;

   xor ecx, ecx
   mov cl, [eax + ebx + 9]   ; Длина дополнительных данных
   mov ch, [eax + ebx + 8]   ;

   add ebx, ecx
   add ebx, 10               ;+Длина заголовка

   lea eax, [eax + ebx]


   pop ecx
   pop ebx
 ret


;
; eax <- указатель на tlv
; возвращает в eax указатель на след tlv
;
 macro get_next_tlv {
   push ebx

   xor ebx, ebx

   mov bl, [eax + 3]
   mov bh, [eax + 2]

   ; + размер заголовка
   lea ebx, [ebx + 4]

   lea eax, [eax + ebx]

   pop ebx
 }



;
; Ищет в additional имя и др. сведения
; eax <- указатель на item
;
 ssi_get_add:
   push eax
   push ebx
   push ecx
   push edx
   push esi


   ;mov ebx, 128
   ;call print_mem




   xor ebx, ebx
   mov bl, [eax + 1]      ; Length of the item name
   mov bh, [eax]          ;

   ;;cmp ebx, 0             ; Если длина имени = 0
   ;;jz ssi_all_tlv         ; Нет смысла обрабатывать


   ;;+смещение до ItemID

   ;;xor ecx, ecx
   ;;mov ch, [eax + ebx + 4]
   ;;mov cl, [eax + ebx + 5]  ;	Item ID#
   ;
   ;;cmp ecx, 0               ; Группы пока не обрабатываются
   ;;jz ssi_all_tlv           ;
   ;
   ; Проверяем флаги, обрабатываем только записи UIN
   ;
   xor ecx, ecx
   mov ch, [eax + ebx + 6]
   mov cl, [eax + ebx + 7]
    
   cmp ecx, 0           ; 0x0000	  Buddy record (name: uin for ICQ and screenname for AIM)
   jnz ssi_all_tlv

   
   xor edx, edx
   mov dl, [eax + ebx + 9]  ;
   mov dh, [eax + ebx + 8]  ; Length of the additional data
  

   lea eax, [eax + ebx + 10] ; eax указатель на первый tlv


   ;FIXME : Iservd не присылает additional - пока не могу отладить
   ;debug
   ;push ebx
   ;mov ebx, edx

   ;data_debug 'length additional data', ebx

   ;call print_mem
   ;pop ebx
   ;



   xor esi, esi

  ssi_tlv_process:
   cmp esi, edx           ;
   jnb ssi_all_tlv        ;  additional закончилось

   xor ecx, ecx
   mov cl, [eax + 3]      ;
   mov ch, [eax + 2]      ; TLV.Length

   xor ebx, ebx
   mov bl, [eax + 1]      ; TLV.Type
   mov bh, [eax]          ;

   cmp bx, 0x0131         ;Имя пользователя
   jz ssi_name

   cmp bx, 0x0066         ;Ожидаем авторизации
   jz ssi_auth_wait

   jmp ssi_next_tlv


  ssi_auth_wait:
   ;
   ;
   ;
   jmp ssi_next_tlv


  ssi_name:

   ;
   ; Скопировать имя в массив
   ;
   push eax
   push ecx

   mov ebx, [name_ind]
   cmp ebx, UINS
   jnb ssi_name_end       ;Нет места в таблице

   lea eax, [eax + 4]     ;Указатель на строку (Прибавляем размер заголовка TLV)

   imul ebx, NAME_LEN
   lea ebx, [names + ebx]

   cmp ecx, NAME_LEN - 1     ; Если имя длиннее поля в таблице
   jna @f

   mov ecx, NAME_LEN - 1

  @@:
   call strcpy

   ;; FIXIT
   ;; Перекодировка имени
   ;;
   ;;mov eax, ebx
   ;;call win2dos

   ;
   ;debug
   ;push eax
   ;push ebx

   ;mov eax, ebx
   ;xor ebx, ebx
   ;call writemsg

   ;pop ebx
   ;pop eax
   ;
   ;
   inc [name_ind]

   pop ecx
   pop eax



  ssi_next_tlv:
   lea ecx, [ecx + 4] ; Длина данных tlv + длина заголовка
   add esi, ecx

   get_next_tlv
   jmp ssi_tlv_process


  ssi_name_end:
   pop ecx
   pop eax


  ssi_all_tlv:


   pop esi
   pop edx
   pop ecx
   pop ebx
   pop eax
 ret

;
;
; Обработка контакт листа, пришедшего от сервера
;
; в eax <- указатель на данные в пакете SNAC(13,06)
 ssi_process_data:
   push eax
   push ebx
   push ecx
   push edx

   ;
   ; Проверить версию протокола
   ;
   xor ebx, ebx
   mov bl, [eax]
   cmp bl, 0
   jnz ssi_bad_prot
   ;
   ;в ebx - количество items
   mov bl, [eax + 2]
   mov bh, [eax + 1]
   ;
   data_debug 'SSI items:', ebx

   lea eax, [eax + 3] ; Установить eax на список items

   xor ecx, ecx       ; Счетчик items


  ssi_next_uin:
   cmp ecx, ebx
   jnb ssi_all_items


   call ssi_get_uin

   call ssi_get_add

   call ssi_next_item

   inc ecx
   jmp ssi_next_uin



  ssi_bad_prot:
   write_debug "ERR: SSI protocol version mismatch"

  ssi_all_items:

   pop edx
   pop ecx
   pop ebx
   pop eax
 ret



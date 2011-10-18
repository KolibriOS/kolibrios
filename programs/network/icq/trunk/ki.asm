; compiler:     FASM 1.67.21
; name:         ICQ client for Kolibri
; version:      0.1.30
; written by:   LV
; e-mail:       lv4evil@yandex.ru


include "lang.inc"
include "MACROS.INC"
;purge mov
;include "ASCL9/ascl.inc"
;include "debug.inc"
include "editbox.inc"

MEOS_APP_START

;include "debug.inc"
include "2000.inc"
include "comp.inc"
;include "fsio.inc"
include "dos2win.inc"
include "parser.inc"
include "ssi.inc"

use_edit_box procinfo,22,5

;
; Если == 0, код для использования контакт листа
; на сервере не ассемблируется 
;
USE_SSI = 1
;
;

CODE
    
    
    ;mov eax, 40
    ;mov ebx, 47h
    ;int 40h

    ;
    ; Загрузка конфигов
    ;

    mov  eax, fname
    call    parseconf
    ;
    ; Вывод загруженной информации
    ;

    mov eax, cfg_message
    xor ebx, ebx
    call writemsg

    call showcfg

   
    ;call    loaduin
    call    draw_window 	   ; at first create and draw the window
    
    ;call    buttonbox

  wait_event:			   ; main cycle
    ;mov     eax, 23
    ;mov     ebx, 20
    ;int     0x40
    mcall 23,20

    cmp     eax, 1		   ;   if event == 1
    je	    redraw		   ;     jump to redraw handler
    cmp     eax, 2		   ;   else if event == 2
    je	    key 		   ;     jump to key handler
    cmp     eax, 3		   ;   else if event == 3
    je	    button		   ;     jump to button handler
   
    ;
    ; Ждем данных
    ;
    
    mcall 53,2,[socket]
    cmp     eax, 0
    jnz     read_socket

    mouse_edit_box inputbox
    ;
    ; Если есть соединение с сервером, посылаем пакеты - подтвеждения каждые 60 с
    ; не требуется
;    call    sendkeep
    
    jmp     wait_event		   ;   else return to the start of main cycle


  redraw:			   ; redraw event handler
    call    draw_window
    jmp     wait_event


  key:				   ; get key code
    
    mcall 2

    cmp ah, 0Dh                    ; Пробел - отправить сообщение
    jz send


    key_edit_box inputbox

    jmp     wait_event


  button:			   ; button event handler
    ;mov     eax, 17		   ;   get button identifier
    ;int     0x40
    mcall 17

    cmp     ah, 2
    jz      connect

    cmp     ah, 3
    jz      disconnect

    cmp     ah, 4
    jz      send

    ;
    ; Проверяем, не нажата ли кнопка в КЛ
    ; 100 <ID <= 100+UINS
    cmp ah, UINS+100
    jnc @f
    cmp ah, 100
    jc @f
    ;
    ; Нажата
    ;
    sub ah, 100
    mov [curruser], ah
    ;
    ; Выводим строчку, кому
    ;
    shr eax, 8
    and eax, 000000FFh
    push eax
    mov eax, CUSER
    call strlen
    mov ecx, eax
    mov eax, CUSER
    mov ebx, buff
    call strcpy
    pop eax
    mov ebx, NAME_LEN
    imul ebx, eax
    lea eax, [names + ebx]
    mov [buff + ecx], ' ' ; Пробел
    lea ebx, [buff + ecx + 1]
    mov ecx, NAME_LEN
    ;debug
     push ebx
     mov ebx, NAME_LEN
     call print_mem
     pop ebx
    ;
    call strcpy
    mov eax, buff
    xor ebx, ebx
    call writemsg




  @@:
    cmp     ah, 1
    jne     wait_event		   ;   return if button id != 1

    ;   exit application
    mcall -1


  draw_window:
    ; start drawing
    mcall 12,1

    mcall 0,(100*65536+700),(100*65536+500),0x14ffffff,0,head

    draw_edit_box inputbox

    rect 10, 30, 500, 450, 0

    draw_button 600, 460, 60, 15, 2, 'CONNECT'
    ;draw_button 600, 460, 60, 15, 3, 'Disconnect'
    draw_button 530, 460, 60, 15, 4, 'SEND'

    call    printbuff
    call    buttonbox

    ;
    ; Image
    ;
    ;mov eax, 7
    ;mov ebx, redicq
    ;mov ecx, 15*65536+15
    ;mov edx, 100*65536+100
    ;int 40h


    

   ; finish drawing
   mcall 12,2

  ret

;
;  Соединение
;
  connect:
  lea eax, [vtable + vartable.icqip]
  call ip_parser
  call htonl
  data_debug 'IP:', eax

  ;mov eax, ICQ_IP
  mov ebx, ICQ_PORT
  call srv_connect
  


  jmp wait_event


;
;
;
  disconnect:
  mov ecx, [socket]
  call closesocket

  jmp wait_event


;
;
;
  send:
  ;
  ; Определяем, не сменен ли текущий УИН
  ;
  ; Для смены используется / в начале строки и номер уина 
  ; по порядку. Если длина > 2 символов, считается, что передан
  ; сам уин - для отправки сообщений юзерам, которых нет в КЛ
  ;
    mov al, [inputbuff]
    cmp al, '/'
    jnz sd_message
    ; Смена уина
    ;mov al, [inputbuff+2]
    ;cmp al, 20h
    ;jz sd_use_kl
    mov al, [inputbuff+3]
    cmp al, 20h            ; Пробел
    jz sd_use_kl
    ;
    ; Ищем первый пробел, им должен закончиться уин
    ;
    xor ecx, ecx
  sd_loop:
    mov al, [inputbuff+ecx]
    cmp al, 20h
    jz sd_space
    cmp al, 0
    jz  wait_event
    inc ecx
    jmp sd_loop

  sd_space:
    ;
    ; Заменяем пробел на 0, отсылаем сообщение
    mov [inputbuff+ecx], byte 0
    lea ebx, [inputbuff+1]
    lea eax, [inputbuff+ecx+1]
    call sendmsg
    mov ebx, 0000FFh
    call writemsg
    jmp wait_event



  sd_use_kl:
    lea eax, [inputbuff+1]
    mov [inputbuff+3], byte 0
    call ascitoint
    lea eax, [eax-1]    ; Т.к. в КЛ отсчет с 0
    mov [curruser], al
    

  sd_message:
  ;
  ; Сообщение
    movzx eax, [curruser]
    mov ebx, UIN_LEN
    imul ebx, eax
    lea ebx, [uins+ebx]
    mov al, [inputbuff]
    cmp al, '/'
    jz @f
    mov eax, inputbuff
    jmp sd_send
  @@:
    ;mov al, [inputbuff+2]
    ;cmp al, ' '
    ;jz @f
    lea eax, [inputbuff+4]
    ;jmp sd_send
  ;@@: lea eax, [inputbuff+3]
  
  sd_send:
    call sendmsg
    mov ebx, 0000FFh
    call writemsg


  jmp wait_event


;
; Есть принятые данные
;
  read_socket:
    pushf
    pushad
    ;write_debug 'Some data in socket'
    ;
    ; Проверяем, не был ли получен заголовок отдельно от данных 
    ; в предыдущем цикле
    ;
    cmp [hrf], 1
    jz rs_head_recived

  rs_check_sock:
    ;mov eax, 53
    ;mov ebx, 2
    ;mov ecx, [socket]
    ;int 40h
    mcall 53,2,[socket]
    cmp eax, 6 ; Flap head size
    jc r_end
    ;
    ; Принимаем заголовок
    ;
    xor edx, edx

    ;mov ecx, [socket]
   rs_loop:
    ;mov eax,  53
    ;mov ebx, 3
    ;int 40h
    mcall 53,3,[socket]

    mov [mbuff+edx], bl
    inc edx
    cmp edx, 6

    jnz rs_loop
    ;
    ; Заполняем заголовок
    ;
    ;xor eax, eax

    ;
    ; Заголовок принят!
    ;
    mov [hrf], 1

    mov bl, [mbuff]
    mov [rflap.bId], bl

    mov bl, [mbuff+1]
    mov [rflap.bCh], bl

    mov bh, [mbuff+2]
    mov bl, [mbuff+3]
    mov [rflap.wSn], bx

    mov bh, [mbuff+4]
    mov bl, [mbuff+5]
    mov [rflap.wDs], bx 

    ;
    ; Принимаем данные
    ;
    ;xor edx, edx
    cmp [rflap.bId], 2Ah
    jnz rs_flap_error
    ;
    ;  Проверяем, получены ли данные
    ;
  rs_head_recived:

    ;mov eax, 53
    ;mov ebx, 2
    ;mov ecx, [socket]
    ;int 40h
    mcall 53,2,[socket]

    cmp ax, [rflap.wDs]   ; Размер данных
    jc r_end
    ;
    ;
    mov ax, [rflap.wDs]
    ;
    ; Проверяем размер данных
    ;
    cmp ax, MBUFF_SIZE+1
    jnc rs_big_flap

    xor esi, esi     
    mov esi, eax
    xor edx, edx

    ;mov ecx, [socket]

   rs_data_loop:
    cmp edx, esi
    jz rs_data_end

    ;mov eax, 53
    ;mov ebx, 3
    ;int 40h
    mcall 53,3,[socket]
    mov [mbuff+edx], bl
    inc edx
    jmp rs_data_loop

    ;
    ; Данные приняты
    ;
    rs_data_end:
    mov [hrf], 0
    ;write_debug 'Some data recived'
    ;
    ;
    ;
    cmp [login], 0
    jz rs_login
    call main_loop
    ;
    ; Есть смысл проверить сокет на наличие следующего заголовка
    ;
    ;jmp r_end
    jmp rs_check_sock


    rs_login:
    call srv_login
    ;write_debug 'Exited srv_login'
    ;jmp r_end
    jmp rs_check_sock

    rs_flap_error:
    write_debug 'Invalid Flap'
    ;
    ; FLAP.id неверный. нужно закрыть сокет
    ;

    mov ecx, [socket]
    ;call closesocket
    jmp r_end

    ;
    ;  Слишком большой пакет!
    ;
    rs_big_flap:

    write_debug 'Too BIG FLAP Recived'
    mov [hrf], 0

    ;mov ecx, [socket]
    mov ax, [rflap.wDs]
    xor esi, esi     
    mov esi, eax
    xor edx, edx

   rs_data_loop2:
    cmp edx, esi
    jz r_end

    ;mov eax, 53
    ;mov ebx, 3
    ;int 40h
    mcall 53,3,[socket]
    ;mov [mbuff+edx], bl
    inc edx
    jmp rs_data_loop2





    r_end:
    
    popad
    popf
  jmp wait_event

; Соединение с сервером, возвращает в eax - хэндл сокета
; передаем в еах IP адрес сервера
; в ebx - порт
  srv_connect:  
    push ecx 
    push edx 
    push esi 
    push edi
    push ebx
    
    mov esi, eax             ; IP - в esi
    ; find free port
    mov    ecx, 1000	     ; Определяем локальный порт, начинаем с 1000

    getlp:
    inc    ecx
    push   ecx
    ;mov    eax, 53
    ;mov    ebx, 9
    ;int    0x40
    mcall 53,9,ecx
    pop    ecx
    cmp    eax, 0	     ; этот локальный порт используется?
    jz	   getlp	     ; да - продолжаем перебирать
    ;OK ecx = port number
    ;Open Socket
    ;mov eax, 53
    ;mov ebx, 5
    xor edx, edx
    ;mov dx, ICQ_PORT
    pop edx
    ;mov esi,ICQ_IP
    ;mov edi, 1;SOCKET_ACTIVE

    ;int 40h
    mcall 53, 5, ecx, edx, esi, 1
    ;
    mov [socket], eax

    ;
    ; Ждем установки соедиения
    mov ecx, eax
  srv_loop:

    ;mov eax, 5
    ;mov ebx, 50
    ;int 40h
    mcall 5, 50


    
    ;mov eax, 53
    ;mov ebx, 6
    ;int 40h
    mcall 53, 6, ecx
    cmp eax, TCB_ESTABLISHED 
    jz fin
    cmp eax, 11
    jae c_end
    ;
    
    ;inc [timer]
    ;cmp [timer], 50
    ;jz srv_err
    jmp srv_loop



  ;srv_err:
    ;mov [timer], word 0
    ;cmp eax,-1
    ;jnz fin
    ;delay 100
    write_debug	'CONNECTION FAILED'                   ;Подключение не удалось
    jmp c_end
    ;connrcted:  
					  ;CONNECTED
    
    fin:
    write_debug 'Connected!!!!'
  c_end:
    pop edi 
    pop esi 
    pop edx 
    pop ecx 
    ;pop ebx
  ret

;
; --> ecx socket handle
;
  
  srv_login:
    pushf
    push eax
    push ebx
    ;push ecx
    push edx

    ;
    ; Определяем тип полученных данных
    ;
    movzx eax, [rflap.bCh]
    cmp eax, 01
    jz s_new_connection
    cmp eax, 04
    jz s_cookie        ; cookie
    jmp l_flap_err

  s_new_connection:
    ;
    ; Проверяем полученный пакет
    ;
    movzx eax, [rflap.wDs]
    cmp eax, 4
    jnz l_len_err
    mov eax, dword [mbuff]
    cmp eax, 01000000h      ; 00 00 00 01
    jnz l_data_err
    ;
    ;Формируем пакет для соединения
    ;
    ;mov [flap.bId], FLAP_ID
    mov [flap.bCh], NEW_CONNECTION
    
    ;mov eax, 26
    ;mov ebx, 9
    ;int 40h
    mcall 26, 9
    mov [seq], ax

    mov [flap.wSn], ax      ; Sequence number
    ;mov [buff],0
    ;mov [buff+1],0
    ;mov [buff+2],0
    mov dword [buff], 0x01000000 ;login Protokol version  00 00 00 01
    ;mov[buff+4],0
    mov word [buff+4], 0x0100; TLV.TYPE = UIN     00 01

    lea eax, [vtable + vartable.uin]
    call strlen
    mov [buff+6], ah
    mov [buff+7], al ; Length of UIN 
    mov edx, eax
    add edx, 7                 ; в edx длина заполненного буфера
    
    mov ecx, eax              ;Длина строки

    lea eax, [vtable + vartable.uin]
    lea ebx, [buff+8]         ; + размер данных в буфере + 1 

    call strcpy

    
    lea eax, [vtable + vartable.pass]
    call roast
    
    mov [buff+edx+2], 2 ; TLV.TYPE - rosted password
    call strlen
    mov [buff+edx+4], al
    mov [buff+edx+3], ah  ; Length of pass

    add edx, 4
    mov ebx, buff
    add ebx, edx               ; назначение
    add edx, eax               ; Сохраняем в EDX длину заполненного буфнра
    mov ecx, eax               ; Длина строки
    lea eax, [vtable + vartable.pass]              ; Источник
    inc ebx
    call strcpy
    
    mov [buff+edx+2], 3 ; TLV.TYPE - client id string
    mov eax, ID_STRING
    call strlen
    mov [buff+edx+4], al
    mov [buff+edx+3], ah

    add edx, 4
    mov ecx, eax
    mov ebx, buff
    add ebx, edx
    add edx, eax
    inc ebx
    mov eax, ID_STRING
    call strcpy

    xor eax, eax

    mov [buff+edx+2], 016h  ; TLV.TYPE - Client id
    mov [buff+edx+4], 2
    mov ax, ID_NUM
    call htons
    mov word [buff+edx+5], ax
    add edx, 6

    mov [buff+edx+2], 017h ; Client major version
    mov [buff+edx+4], 2
    mov ax, MAJOR
    call htons
    mov word [buff+edx+5], ax
    add edx, 6

    mov [buff+edx+2], 018h ; Client minor version
    mov [buff+edx+4], 2
    mov ax, MINOR
    call htons
    mov word [buff+edx+5], ax
    add edx, 6

    mov [buff+edx+2], 019h ; Client lesser version
    mov [buff+edx+4], 2
    mov ax, LESSER
    call htons
    mov word [buff+edx+5], ax
    add edx, 6

    mov [buff+edx+2], 01Ah ; Client build number
    mov [buff+edx+4], 2
    mov ax, BUILD
    call htons
    mov word [buff+edx+5], ax
    add edx, 6
    
    mov [buff+edx+2], 014h ; Client distribution number
    mov [buff+edx+4], 4
    mov eax, DISTR
    call htonl
    mov dword [buff+edx+5], eax
    add edx, 8

    mov [buff+edx+2], 0Fh ; Client language
    mov eax, CL_LANG
    call strlen
    mov word [buff+edx+4], ax
    add edx, 4
    mov ecx, eax
    mov ebx, buff
    add ebx, edx
    inc ebx
    add edx, eax
    mov eax, CL_LANG
    call strcpy

    mov [buff+edx+2], 0Eh ; Client country
    mov eax, CL_COUNTRY
    call strlen
    mov word [buff+edx+4], ax
    add edx, 4
    mov ecx, eax
    mov ebx, buff
    add ebx, edx
    inc ebx
    add edx, eax
    mov eax, CL_COUNTRY
    call strcpy
    
   ;write_debug 'Connect attemption'
   ; mov eax, ICQ_IP
   ; call srv_connect
   ; cmp eax, -1   ; Подключение не удалось
   ; jz l_fin
 
   ; mov ecx, eax
   ; mov eax, rflap
   ; mov ebx, lbuff
   ; call recvflap
    
  ;  cmp eax, -1
   ; jz l_flap_err
   ; cmp [rflap.bCh], 01 ; AUTH channel
   ; jnz l_ch_err
   ; cmp eax, 4
   ; jnz l_len_err
   ; cmp dword [lbuff+3], dword 1
   ; jnz l_data_err

    mov ecx, [socket]
    inc dx
    mov [flap.wDs], dx ; Data size
    mov eax, flap
    mov ebx, buff
    call sendflap
    cmp eax, 0
    jnz l_fin           ; Неуспех
    jmp l_end
    

    s_cookie:
    ;mov eax, rflap
    ;mov ebx, buff
    ;call recvflap
    ;cmp eax, -1
    ;jz l_flap_err
    ;cmp [rflap.bCh], 4
    ;jnz l_ch_err

    ;write_debug 'UIN'
    xor ebx, ebx

   uin_loop:
    xor eax, eax
    mov ax, word [mbuff+ebx]
    cmp ax, 0100h              ;  00 01 TLV.Type UIN
    jz l_uin_ok                  ;  Теперь сервер передает еще данные при соединении, а потом опять
    add ebx, 5                   ; тот же TLV 1  (новый формат пакекта)
    cmp ebx, 5
    ja l_tlvt_err
    jmp uin_loop





  l_uin_ok:
    mov eax, ebx
    xor ebx, ebx
    mov bl, [mbuff+eax+3]           ;
    mov bh, [mbuff+eax+2]           ;  Длина данных
    ;
    ;  UIN Пока не проверяется
    ;
    

    lea ebx, [ebx+eax+4]
    mov ax, word [mbuff+ebx]
    cmp ax, 0500h             ; 00 05 Bos address
    jz l_all_ok
    cmp ax, 0400h             ; UIN incorrect
    jz l_uin_err
    cmp ax, 0800h
    jz l_pass_err
    jmp l_tlvt_err
    ;
    ; если неверный UIN/ пароль, получаем TLV.TYPE 4/8 
    ;

    l_all_ok:
    xor ecx, ecx
    mov cl, [mbuff+ebx+3]       ;length
    mov ch, [mbuff+ebx+2]       ;
    
    lea eax, [mbuff+ebx+4]
    push ebx
    mov ebx, bos_address
    call strcpy
    pop ebx
    add ebx, ecx
    lea ebx, [ebx+4]                ; Размер заголовка                
    ;         
    ; cookie
    ;
    ;write_debug 'Login Cookie'

    xor eax, eax
    mov ax, word [mbuff+ebx]
    cmp ax, 0600h                  ; TLV.Type cookie
    jnz l_tlvt_err
    mov cl, [mbuff+ebx+3]           ;
    mov ch, [mbuff+ebx+2]           ; Length
    mov [cookie_len], cx
    lea eax, [mbuff+ebx+4]
    push ebx
    mov ebx, srv_cookie
    call strcpy
    pop ebx
            
    ;                         
    ;  Соединяемся с BOS    
    ;                       
    ;call srv_disconnect
    mov ecx, [socket]
    write_debug 'Closing socket'
    ;call closesocket
    ;
    ;
    ;
    ;FIXME!!!
    ;Закомменторовано из-за проблемы с сетевым стеком
    ;закрытие сокета завешивает систему
    ;mcall 53, 8, ecx


                        
    mov eax, bos_address
    call ip_parser
    
    call htonl
    data_debug 'BOS Address: ', eax
    data_debug 'BOS Port: ', ebx
    mov [bos_ip], eax       
    mov [bos_port], ebx     
    call srv_connect
    mov [login], 1                ; Соединение с основным сервером установлено
    ;mov [socket], eax
                            

    
    jmp l_end
    ; 
    ;
    ;
    l_pass_err:
    write_debug 'PASSWORD INVALID'
    jmp l_fin

    l_uin_err:
    write_debug 'UIN INVALID'
    jmp l_fin

    l_data_err:
    write_debug 'LOGIN DATA MISMATCH'
    jmp l_fin

    l_len_err:
    write_debug 'RECIVED DATA LENGTH MISMATCH'
    jmp l_fin

    l_tlvt_err:
    write_debug 'TLV TYPE MISMATCH'
    jmp l_fin

    l_ch_err:
    write_debug 'FLAP CHANNEL MISMATCH'
    jmp l_fin

    l_flap_err:
    write_debug 'FLAP ID MISMATCH / RECIVE ERROR'

    l_fin:

    ;
    ; Необходимо закрыть сокет
    ;
    ;call srv_disconnect
    call closesocket
    l_end:
    pop edx
    ;pop ecx
    pop ebx
    pop eax
    popf                          
  ret                

;
; Length of string
; input eax = offset string
; output eax = strlen
;
  strlen:
    push ebx
    push ecx
    pushf
    xor ebx, ebx
    xor ecx, ecx

    loop_s:
    mov cl, [eax+ebx]
    cmp ecx,0
    jz  nl
    inc ebx
    jmp loop_s

    nl:
    mov eax, ebx
    popf
    pop ecx
    pop ebx
  ret

;
; Roasting password
; EAX = offset password
;

  roast:
    pushf
    push ecx
    push ebx

    xor ecx, ecx
    xor ebx, ebx

    loop_r:
    mov bl, [eax+ecx] ;Символ из массива пароля
    cmp bl, 0         ;Конец строки
    jz r_fin
    
    xor bl, [ROASTING_ARRAY+ecx]
    mov [eax+ecx], bl
    inc ecx
    jmp loop_r

    r_fin:
    pop ebx
    pop ecx
    popf
  ret


;
;Copy string of bytes
;В EAX = адрес исходной строки
;В EBX = адрес назначения
;В ECX = длина строки
;
  strcpy:
    pushf
    push esi
    push edi
    push ecx

    cld      ;Обрабатываем строку от начала к концу
    mov esi, eax
    mov edi, ebx

    rep movsb

    pop ecx
    pop edi
    pop esi
    popf    
  ret


;
; Макрос для сравнения строк
;
macro strcmp first, second, len
{
   push ecx
   push esi
   push edi

   mov ecx, len
   mov esi, first
   mov edi, second
   repe cmpsb

   pop edi
   pop esi
   pop ecx

}


;
; Заполняет буфер, по адресу в ebx
; данными, по адресу eax, в
; cx  - Тип TLV
; dx  - длина данных
; 
;

  tlvstr:
    ;pushf
    push edx
    push ecx
    push ebx

    mov [ebx], ch     ; Type
    mov [ebx+1], cl

    mov [ebx+2], dh   ; Length
    mov [ebx+3], dl
    
    lea ebx, [ebx+4]
    ; EBX = offset of destination
    mov ecx, edx

    call strcpy

    pop ebx
    pop ecx
    pop edx
    ;popf
  ret

;
; eax - указатель на FLAP_head
; ebx - указатель на массив, заполненный данными
; ecx - хендл сокета
; 
; В eax возвращает результат записи в сокет
;
  sendflap:
    pushf
    push edx
    ;push ecx
    push esi
    push ebx
    push ecx

    xor edx, edx

    mov dl, [eax]          ; ID byte
    mov [sbuff], dl

    mov dl, [eax+1]        ; FLAP channel
    mov [sbuff+1], dl

    mov dl, [eax+2]        ; FLAP datagramm seq number
    mov [sbuff+3], dl      ; меняем местами байты для передачи по сети
    mov dl, [eax+3]
    mov [sbuff+2], dl

    mov dl, [eax+4]        ; FLAP data size
    mov [sbuff+5], dl
    mov dl, [eax+5]
    mov [sbuff+4], dl
    mov dx, word [eax+4]

    xchg ecx, edx           ; ecx - size edx - handle
    mov eax, ebx            ; data
    mov ebx, sbuff          ; dest
    add ebx, 6              ; + header size
    call strcpy

    xchg ecx, edx           ; ecx - handle, edx - data size

    s_wait:
    ;mov eax, 53             ; Проверяем состояние сокета. Если соедиение
    ;mov ebx, 6              ; установлено - посылаем буфер, если сокет закрыт, уходим
    ;int 40h
    mcall 53, 6, ecx
    cmp eax, TCB_ESTABLISHED ; установлено
    jz s_est
    cmp eax, TCB_CLOSED 
    jz s_fin
    cmp eax, 12            ;  У меня такое было, когда соединение устанавливалось с пустотой :-)
    jnc s_fin              ;

    
    ;mov eax, 5
    ;mov ebx, 1
    ;int 40h                ; Ждем
    mcall 5, 1
    jmp s_wait


    s_est:
    ;mov eax, 53
    ;mov ebx, 7             ; писать в сокет
  
    add edx, 6             ; + size of header
    ;mov esi, sbuff         ; data
    ;int 40h
    mcall 53, 7, ecx, edx, sbuff
 
    s_fin:
    pop ecx
    pop ebx
    pop esi
    ;pop ecx
    pop edx
    popf
  ret


;
; eax - указатель на буфер
; ebx - значение, которым необходимо затолнить. Используется только bl
; ecx - размер
;

  memset:
    pushf
    push edi
    push eax
    push ebx
    push ecx

    cld
    mov edi, eax
    mov eax, ebx
    rep stosb

    pop ecx
    pop ebx
    pop eax
    pop edi
    popf
  ret

;
; Парсим TLV
; <-- в eax адрес TLV
; <-- в ebx адрес буфера, который нужно заполнить
; --> в ebx длина полученных данных
; --> в eax тип TLV
;

  tlvpar:
    pushf
    ;push esi
    ;push edi
    push ecx
    xor ecx, ecx

    mov cl, [eax+3]  ;TLV.Length
    mov ch, [eax+2]
    call strcpy

    xor eax, eax
    mov al, [ebx+1]  ;TLV.Type
    mov ah, [ebx]
    mov ebx, ecx


    pop ecx
    ;pop edi
    ;pop esi
    popf
  ret

;
;  <-- ECX - хендл сокета, который нужно закрыть
;  --> ECX - Результат (Ненадежно)
;
  closesocket:
    push eax
    ;push ebx

    ;mov eax, 53
    ;mov ebx, 8
    ;int 40h
    mcall 53, 8, ecx
 
    mov ecx, eax

    ;pop ebx
    pop eax
  ret

;
; ecx <-- хендл сокета
;
;

  srv_disconnect:
    pushf
    push eax
    push ebx
    mov [flap.bId], FLAP_ID
    mov [flap.bCh], 4      ;Disconnect
    xor eax, eax
    mov ax, [seq]
    mov [flap.wSn], ax
    mov [flap.wDs], 0
    mov eax, flap
    mov ebx, buff
    call sendflap


    pop ebx
    pop eax
    popf
  ret

;
; <-- eax [bos_address]
; --> eax = IP ADDRESS
; --> ebx = port number
;
par_buff db 9 dup 0

  ip_parser:
    pushf
    push ecx
    push edx
    push esi
    push edi

    xor ecx, ecx
    ;xor eax, eax
    mov ebx, eax
    xor edx, edx
    xor esi, esi
    xor edi, edi 
   
    ip_loop:
    xor eax, eax
    ;xor edx, edx
    mov al, [ebx+ecx]
    cmp al, '.'
    jz ip_dot
    
    cmp al, 0
    jz ip_end_str
    
    cmp al, ':'
    jz ip_colon
    
    ;sub al, 30h
    ;cmp al, 9
    ;ja ip_err        ; Не цифра
    
    mov [par_buff+edx], al
    inc ecx
    inc edx
    jmp ip_loop 

    ip_dot:
    ;xor eax, eax
    mov [par_buff+edx], 0 ; Конец строки
    mov eax, par_buff
    call ascitoint

    ;data_debug 'Debug eax: ', eax

    cmp ecx, 0       ; Не может начинаться с точки
    jz ip_err
    shl esi, 8       ; Сдвигаем предыдущий байт
    add esi, eax
    inc ecx
    xor edx, edx     ; Счетчик буфера = 0
    jmp ip_loop


    ip_colon:         ; : В строке адреса
    inc edi           ; Было :
    jmp ip_dot
    
    ip_end_str:
    cmp edi, 1
    jz @f
                          ; : Не было
    mov [par_buff+edx], 0 ; Конец строки
    mov eax, par_buff
    call ascitoint
    shl esi, 8       ; Сдвигаем предыдущий байт
    add esi, eax
    ;mov eax, esi     ; IP в 16 ричной форме
    xor ebx, ebx    ; Номера порта нет
    jmp ip_end

    @@:                            ; Было :
    mov [par_buff+edx], 0          
    mov eax, par_buff
    call ascitoint
    mov ebx, eax
    jmp ip_end

    ip_err:
    xor esi, esi

    ip_end:
    mov eax, esi

    pop edi
    pop esi
    pop edx
    pop ecx
    popf
  ret

;
; <-- eax указатель на asci
; --> eax int
; 
  ascitoint:
    pushf
    push ebx
    push ecx
    push edx
    push esi
    push edi

    xor ebx, ebx  
    xor ecx, ecx
    xor edx, edx
    ;xor esi, esi
    xor edi, edi
   
    ati_loop:
    mov bl, [eax+ecx]
    cmp bl, 0         ; Конец строки
    jz ati_str_end
    cmp bl, 39h
    ja ati_err        ; Не цифра
    cmp bl, 30h
    jb ati_err

    inc ecx
    jmp ati_loop

    ati_str_end:      ; В ecx длина строки 
    ;dec ecx           ; Установим на последний символ
    add eax, ecx      ; Указатель на строку + Длина строки
    dec eax
    
    ati_loop2:
    cmp edx, ecx
    jz ati_all
    push eax
    sub eax, edx              ; Вычесть счетчик 
    movzx ebx, byte [eax]     ; В bl символ
    ;pop eax
    sub bl, 30h       ; Вычисляем 10тичную цифру

    ;push eax
    mov eax, ebx     ; В eax - цифра
    mov ebx, 10      ; Множитель

    xor esi, esi

    ati_mul: 

    cmp esi, edx     ; Умножаем на 10 n раз
    jz ati_mul_end
    ;push eax
    ;mov eax, ebx
    imul eax, ebx
    ;mov ebx, eax
    ;pop eax
    inc esi
    jmp ati_mul


    ati_mul_end:
    mov ebx, eax    ; В ebx вычисленное число
    pop eax

    add edi, ebx
    inc edx
    jmp ati_loop2

    ati_all:
    mov eax, edi
    jmp ati_end

    ati_err:

    ;ati_str_end:
    xor eax, eax

    ati_end:
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
    popf
  ret

;
;
; <-- ecx хендл сокета
; <-- eax указатель на структуру SNAC_head
; <-- ebx указатель на данные
; <-- edx размер данных
; --> eax результат записи в сокет
;

snac_buff db 1024 dup 0

  sendsnac:
    pushf
    push esi
    push edi
    push ebx
    push edx
    ;xor ebx, ebx
    mov esi, ecx            ; хендл сокета
    mov edi, ebx            ; Указатель на данные

    xor ebx, ebx
    mov bl, [eax]           ;
    mov [snac_buff+1], bl   ; Family ID
    mov bl, [eax+1]         ; Конвертируется в BigEndian
    mov [snac_buff], bl     ;

    mov bl, [eax+2]         ;
    mov [snac_buff+3], bl   ; Subtype ID
    mov bl, [eax+3]         ;
    mov [snac_buff+2], bl   ;
    
    mov bl, [eax+4]         ;
    mov [snac_buff+5], bl   ;
    mov bl, [eax+5]         ; Flags
    mov [snac_buff+4], bl   ;

    mov bl, [eax+6]         ;
    mov [snac_buff+9], bl   ;
    mov bl, [eax+7]         ;
    mov [snac_buff+8], bl   ;
    mov bl, [eax+8]         ; Reqest ID
    mov [snac_buff+7], bl   ;
    mov bl, [eax+9]         ;
    mov [snac_buff+6], bl   ;

    lea ebx, [snac_buff+10]                              
  
    mov eax, edi            ; Указатель на данные
    ;add ebx, 10             ; + размер заголовка SNAC
    mov ecx, edx            ; размер данных
    call strcpy


    mov ecx, esi            ; Хендл сокета
    mov [flap.bId], FLAP_ID
    mov [flap.bCh], 2       ; Канал для посылки SNAC
    xor ebx, ebx
    inc [seq]               ; seq Увеличивается на 1 при каждой посылке
    mov bx, [seq]
    mov [flap.wSn], bx
    add edx, 10             ; размер данных + размер заголовка SNAC
    mov [flap.wDs], dx
    mov eax, flap
    mov ebx, snac_buff
    call sendflap

    pop edx
    pop ebx
    pop edi
    pop esi
    popf
  ret



; Обработка всех пактов, приходящих от сервера
; ECX <-- Хендл сокета
;
;
;
;
;        
  main_loop:
    pushf
    ;push eax
    ;push ebx
    ;push edx
    pushad

    mov ecx, [socket]
    ;
    ;  ждем пакет
    ;
  ;m_loop:
    ;mov eax, 53
    ;mov ebx, 2
    ;int 40h
    ;cmp eax, 6       ; размер заголоака FLAP
    ;jnc recived      ; >=
    ;
    ; Уходим
    ;
    ;jmp m_fin
    ;mov eax, 5
    ;mov ebx, 5
    ;int 40h
    ;jmp m_loop
    ;
    ;  есть пакет
    ;
  ;recived:
    ;mov eax, rflap
    ;mov ebx, rbuff
    ;call recvflap
    ;
    ; Определяем тип принятого FLAP
    ;
    xor ebx, ebx
    mov bl, [rflap.bCh]
    cmp bl, 1                ; Установка соединения
    jz  m_login
    cmp bl, 2
    jz m_snac                ; Получен SNAC
    cmp bl, 3
    jz m_flap_err            ; FLAP-level error
    cmp bl, 4
    jz m_close_conn          ; Закрытие соединения
    cmp bl, 5
    jz m_keep_alive          ;
    ;
    ; Обработка рассоединения
    ;
  m_close_conn:
    write_debug 'Another Computer Use YOUR UIN!'
    call srv_disconnect
    call closesocket
    jmp m_fin
    ;
    ; обработка соединения
    ;
  m_login:
    ;
    ; проверяем версию протокола
    ;
    xor eax, eax
    mov al, [mbuff+3]
    cmp eax, 1
    jnz m_login_other    ; Не подходит


    ;
    ; генерируем случайный seq
    ; Для этого берем время, прошедшее с момента запуска системы
    ;
    ;mov eax, 26
    ;mov ebx, 9
    ;int 40h
    mcall 26, 9
    mov [seq], ax
    ;
    ; Отдаем серверу cookie
    ;
    mov [flap.bCh], 1
    mov [flap.wSn], ax
    xor eax, eax
    mov ax, [cookie_len]
    add eax, 8            ; TLV len + protocol version len
    mov [flap.wDs], ax
    mov dword [buff], 01000000h  ; 00 00 00 01 Номер протокола
    mov word [buff+4], 0600h     ; 00 06   TLV.Type

    mov ax, [cookie_len]
    mov [buff+6], ah             ;
    mov [buff+7], al             ; TLV.Length

    mov edx, ecx                 ; edx <-- socket handle

    mov ecx, eax                 ; ecx <-- cookie len
    mov eax, srv_cookie          ; Src
    lea ebx, [buff+8]
    call strcpy
    
    mov ecx, edx                 ; ecx <-- socket handle
    mov eax, flap
    mov ebx, buff
    call sendflap
    jmp m_fin

    m_login_other:
    jmp m_fin

    ;
    ; Как обработать ошибку, я не знаю
    ;
  m_flap_err:
  jmp m_fin

    ;
    ; Пока не обрабатывается
    ;
  m_keep_alive:
  jmp m_fin


    ;
    ; Получен SNAC
    ; Распознаем его тип
    ;
  m_snac:
    mov eax, rsnac
    mov ebx, mbuff
    call snacpar
    xor ebx, ebx
    xor edx, edx
    mov bx, [rsnac.wFid]
    mov dx, [rsnac.wSid]

    cmp bx, 1
    jz m_snac_1              ;Generic service controls
    cmp bx, 2
    jz m_snac_2              ;Location services
    cmp bx, 3
    jz m_snac_3              ;Buddy List management service
    cmp bx, 4
    jz m_snac_4              ;ICBM (messages) service
    cmp bx, 9
    jz m_snac_9              ;Privacy management service
    cmp bx, 015h
    jz m_snac_15             ;ICQ specific extensions service
    cmp bx, 013h
    jz m_snac_13             ;Server Side Information (SSI) service
    
    jmp m_other_snac
    ;
    ;   FAMILY 1
    ;
  m_snac_1:
    cmp dx, 7
    jz m_snac_1_7
    cmp dx, 3
    jz m_snac_1_3
    cmp dx, 018h
    jz m_snac_1_18
    cmp dx, 01Fh
    jz m_snac_1_f
    cmp dx, 13h
    jz m_snac_13
    cmp dx, 1
    jz m_snac_1_1
    jmp m_snac_1_other
    ;
    ; Rate limits information response
    ;
  m_snac_1_7:              ; Отвечаем
    mov [ssnac.wFid], 1    ; Family
    mov [ssnac.wSid], 8    ; Subtype
    mov [ssnac.dRi], 8
    mov word [buff], 0100h   ; 0001
    mov word [buff+2], 0200h ; 0002
    mov word [buff+4], 0300h ; 0003
    mov word [buff+6], 0400h ; 0004
    mov word [buff+8], 0500h ; 0005
    mov eax, ssnac
    mov ebx, buff
    mov edx, 10              ; Размер данных
    call sendsnac
    ;
    ; Client ask server location service limitations
    ;
    mov [ssnac.wFid], 2    ; Family
    mov [ssnac.wSid], 2    ; Subtype
    mov [ssnac.dRi], 2
    mov eax, ssnac
    mov ebx, buff
    xor edx, edx
    call sendsnac

    jmp m_fin

    ;
    ;  Server supported snac families list
    ;
  m_snac_1_3:
    ;
    ;  Server sends supported services list
    ;

    ;
    ;   SNAC(01,17)	  
    ;   Client ask for services version numbers
    ;
    mov [ssnac.wFid], 1    ; Family
    mov [ssnac.wSid], 17h    ; Subtype
    mov [ssnac.dRi], 17h
    ;
    ;   Список сервисов, которые нам нужны
    ;
    ;    xx xx	 	word	 	family number #1
    ;    xx xx	 	word	 	family version
    ;      ...	 	 ...	 	 ...
    ;

    ;
    ; Поправил из дампа &RQ
    ;
    mov word [buff], 0100h   ; 0001
    mov word [buff+2], 0300h ; 0003

    mov word [buff+4], 1300h ; 0013
    mov word [buff+6], 0200h ; 0002

    mov word [buff+8], 0200h ; 0002
    mov word [buff+10], 0100h ; 0001

    mov word [buff+12], 0300h ; 0002
    mov word [buff+14], 0100h ; 0001

    mov word [buff+16], 1500h ; 0015
    mov word [buff+18], 0100h ; 0001

    mov word [buff+20], 0400h ; 0004
    mov word [buff+22], 0100h ; 0001

    mov word [buff+24], 0600h ; 0006
    mov word [buff+26], 0100h ; 0001

    mov word [buff+28], 0900h ; 0009
    mov word [buff+30], 0100h ; 0001

    mov word [buff+32], 1300h ; 0013
    mov word [buff+34], 0400h ; 0004

    mov word [buff+36], 1500h ; 0015
    mov word [buff+38], 0400h ; 0004

    mov word [buff+40], 1000h ; 0010
    mov word [buff+42], 0100h ; 0001



    mov eax, ssnac
    mov ebx, buff
    mov edx, 44
    call sendsnac

    jmp m_fin


    ;
    ; Server services versions
    ;
  m_snac_1_18:
    ;
    ; Обработки пока нет
    ;

    ;
    ; Client ask server for rate limits info
    ; SNAC(01,06)
    ;
    mov [ssnac.wFid], 1    ; Family
    mov [ssnac.wSid], 6    ; Subtype
    mov [ssnac.dRi], 6
    mov eax, ssnac
    mov ebx, buff
    xor edx, edx
    call sendsnac



    jmp m_fin

    ;
    ; Requested online info response
    ;
  m_snac_1_f:
    ;
    ;Тут должна быть наша информация, пока обработки нет
    ;


    jmp m_fin

    ;
    ; Message of the day (MOTD)
    ;
  m_snac_1_13:
    ;
    ; Нечего обрабатывать :-))
    ;
    jmp m_fin

    ;
    ; Сообщение об ошибке
    ;

  m_snac_1_1:
    xor eax, eax
    mov ax, word [mbuff+10]
    call ntohs
    data_debug 'SERVER SEND ERROR #', eax


    jmp m_fin


  m_snac_1_other:
     data_debug 'Unknown SNAC Family 1 recived, type ', edx
     jmp m_fin



    ;
    ; Family 2
    ;
  m_snac_2:
    cmp dx, 3
    jz m_snac_2_3
    jmp m_snac_2_other
    ;
    ; Server replies via location service limitations
    ;
  m_snac_2_3:
    ;
    ;  Обработки пока нет
    ;

    ;
    ;  посылаем capabilities / profile
    ;
    mov [ssnac.wFid], 2    ; Family
    mov [ssnac.wSid], 4    ; Subtype
    mov [ssnac.dRi], 4

    ;mov eax, CAPABILITIES
    ;mov ebx, buff
    ;push ecx
    ;mov ecx, 5             ; TLV.Type(0x05) - CLSID values
    ;mov edx, C_LEN
    ;call tlvstr
    ;pop ecx
    mov word [buff], 0500h  ; 00 05
    mov eax, C_LEN
    call htons
    mov word [buff+2], ax



    push ecx

    mov eax, CAPABILITIES
    lea ebx, [buff+4]
    mov ecx, C_LEN
    call strcpy

    pop ecx


    mov eax, ssnac
    mov ebx, buff
    mov edx, C_LEN+4            ; Длина данных+размер заголовка TLV
    call sendsnac

    ;
    ; запрашиваем server BLM service limitations
    ;
    mov [ssnac.wFid], 3    ; Family
    mov [ssnac.wSid], 2    ; Subtype
    mov [ssnac.dRi], 2
    mov eax, ssnac
    mov ebx, buff
    xor edx, edx
    call sendsnac


    jmp m_fin

  m_snac_2_other:
    write_debug 'Unknown SNAC Family 2 Recived'
    jmp m_fin



    ;
    ;  FAMILY 3
    ;
  m_snac_3:
    cmp dx, 3
    jz m_snac_3_3
    cmp dx, 0Bh
    jz m_snac_3_b
    cmp dx, 0Ch
    jz m_snac_3_c
    jmp m_snac_3_other

    ;
    ; Server replies via BLM service limitations
    ;
  m_snac_3_3:
    ;
    ; Обработки пока нет
    ;

    ;
    ; Client ask server for ICBM service parameters
    ;
    mov [ssnac.wFid], 4    ; Family
    mov [ssnac.wSid], 4    ; Subtype
    mov [ssnac.dRi], 4     ; request-id
    mov eax, ssnac
    mov ebx, buff
    xor edx, edx
    call sendsnac



    jmp m_fin

    ;
    ;  User online notification
    ;
  m_snac_3_b:
    ;
    ; Из всей информации пока нужен только статус
    ;
    xor edx, edx           ; Счетчик - номер UIN в массиве
    xor ecx, ecx
    xor eax, eax
    cld             ; В направлении увеличения адресов

    dec edx
  m_snac_3_b_loop:
    inc edx
    cmp edx, UINS
    jnc m_snac_3_b_end     ;>=

    mov cl, [mbuff+10]     ; Длина УИН
    mov eax, ecx
    mov edi, UIN_LEN
    imul edi, edx
    lea edi, [uins+edi]    
    lea esi, [mbuff+11]
    repe cmpsb
    
    jnz m_snac_3_b_loop
    ;
    ; UIN Определен
    ;

    ;
    ; Найти TLV со статусом
    ; сохранять edx

    xor esi, esi    ; Счетчик TLV

    xor ecx, ecx                      
    mov ch, byte [mbuff + eax + 13]   ; Кол-во TLV в цепочке
    mov cl, byte [mbuff + eax + 14]   ;

    lea eax, [eax + 10 + 5]    ; eax указатель на цепочку TLV
    lea eax, [mbuff + eax]     ;


  m_snac_3_b_next_tlv:
    cmp esi, ecx
    jz m_snac_3_b_endchain


    xor ebx, ebx
    mov bh, [eax]              ;
    mov bl, [eax + 1]          ; TLV.Type

    data_debug 'TLV type', ebx

    cmp ebx, 0x06              ;TLV.Type(0x06) - user status
    jz m_snac_3_b_status

    ;
    ; Разбираем цепочку дальше
    ;

    get_next_tlv
    inc esi
    jmp m_snac_3_b_next_tlv



    ; FIXME Неоптимально - код будет удален
    ;
    ;lea ecx, [eax+10+11]           ; +sizeof SNAC_head + offset #2 TLV
    ;mov ax, word [mbuff+ecx]            ;#2 TLV.Type
    ;cmp ax, 0C00h                  ;dc info (optional)
    ;jz m_snac_3_b_dc
    ;cmp ax, 0A00h                  ;external ip address
    ;jz m_snac_3_b_extip
    ;jmp m_snac_3_b_bad_tlv
    

  ;m_snac_3_b_dc:
    ;
    ; Пропускаем этот TLV
    ;
    ;lea ecx, [ecx+41]
  ;m_snac_3_b_extip:
    ;
    ; И этот :-)
    ;lea ecx, [ecx+8]
    ;mov ax, word [mbuff+ecx]
    ;cmp ax, 0600h                 ;TLV.Type(0x0A) - external ip address
    ;jz m_snac_3_b_status
    ;jmp m_snac_3_b_bad_tlv
    ;
    ;


  m_snac_3_b_status:
    ;
    ; статус
    ;
    mov ecx, eax
    mov eax, dword [ecx + 4]
    ;mov eax, dword [mbuff+ecx+4]
    call ntohl
    ;mov ebx, 4
    ;imul ebx, edx
    ;mov [stats+ebx], eax
    mov ecx, eax
    mov ebx, NAME_LEN
    imul ebx, edx
    lea ebx, [names+ebx]
    mov eax, edx
    call loadbb
    jmp m_fin


  m_snac_3_b_bad_tlv:
    write_debug 'TLV Type Mismatch in SNAC(3,b)'
    jmp m_fin

  m_snac_3_b_end:
    write_debug 'UIN not in local Contact List'
    jmp m_fin

  m_snac_3_b_endchain:
    jmp m_fin



  m_snac_3_c:
    ;
    ; User offline notification
    ;
  xor edx, edx
  xor ecx, ecx

    dec edx
    m_snac_3_c_loop:
    inc edx
    cmp edx, UINS
    jnc m_snac_3_b_end     ;>=

    mov cl, [mbuff+10]     ; Длина УИН
    mov edi, UIN_LEN
    imul edi ,edx
    lea edi, [uins+edi]    
    lea esi, [mbuff+11]
    repe cmpsb
    jnz m_snac_3_c_loop
    ;
    ; UIN Определен
    ;
    ;mov eax, -1
    ;mov ebx, 4
    ;imul ebx, edx
    ;mov [stats+ebx], eax
    mov ecx, -1
    mov ebx, NAME_LEN
    imul ebx, edx
    lea ebx, [names+ebx]
    mov eax, edx
    call loadbb
    jmp m_fin






  m_snac_3_other:
    write_debug 'Unknown SNAC Family 3 Recived'
    jmp m_fin


    ;
    ;  FAMILY 4
    ;
  m_snac_4:
    cmp dx, 5
    jz m_snac_4_5
    cmp dx, 7
    jz m_snac_4_7
    jmp m_snac_4_other

    ;
    ;  Server sends ICBM service parameters to client
    ;
  m_snac_4_5:
    ;
    ;  Обработки пока нет
    ;

    ;
    ;  Client change default ICBM parameters command
    ;
    mov [ssnac.wFid], 4    ; Family
    mov [ssnac.wSid], 2    ; Subtype
    mov [ssnac.dRi], 2     ; request-id

    mov eax, ICBM_PARAMS
    mov ebx, buff
    push ecx
    mov ecx, ICBMP_LEN
    call strcpy
    pop ecx

    mov eax, ssnac
    mov ebx, buff
    mov edx, ICBMP_LEN
    call sendsnac

    ;
    ;   Client ask server PRM service limitations
    ;
    mov [ssnac.wFid], 9    ; Family
    mov [ssnac.wSid], 2    ; Subtype
    mov [ssnac.dRi], 2     ; request-id
    mov eax, ssnac
    mov ebx, buff
    xor edx, edx
    call sendsnac


    jmp m_fin

    ;
    ;  Message for client from server
    ;
  m_snac_4_7:
    ;
    ;  Определяем тип сообщения по полю message channel
    ;
    xor eax, eax
    mov ax, word [mbuff+10+8]           ; +10 - размер SNAC
                                        ; +8 смещение до message channel
    cmp ax, 0100h                       ; 00 01 
    jz m_snac_ch1
    cmp ax, 0200h
    jz m_snac_ch2
    cmp ax, 0400h
    jz m_snac_ch4
    jmp m_ch_other
    ;
    ;  channel 1 plain text
    ;
  m_snac_ch1:
    ;
    ; Т.к в очередной раз описание протокола не совпадает с реальностью
    ; разбираем все TLV по порядку

    mov eax, dword [mbuff+10]          ; cookie
    mov [msg_cookie1], eax
    mov eax, dword [mbuff+10+4]
    mov [msg_cookie2], eax             ; Используются для потверждения приема сообщений

    mov al, [mbuff+10+10]              ; Sender UIN length
    mov [ui.bUinLength], al

    push ecx
    movzx ecx, al

    lea eax, [mbuff+10+11]             ; UIN string 
    lea ebx, [ui.bUin]                 ; Dest
    call strcpy

    lea ecx, [ecx+10+15]               ; первый TLV
    

 m_snac_ch1_loop:

    movzx eax, word [mbuff+ecx]
    cmp eax, 0100h                     ;TLV.Type(0x01) - user class
    jz m_snac_ch1_1
    cmp eax, 0600h                     ;TLV.Type(0x06) - user status
    jz m_snac_ch1_6
    cmp eax, 0800h                     ; Unknown type
    jz m_snac_ch1_8
    cmp eax, 0500h                     ; Unknown type
    jz m_snac_ch1_5
    cmp eax, 0F00h                     ; TLV.Type(0x0f) - user idle time
    jz m_snac_ch1_f
    cmp eax, 0300h                     ; TLV.Type(0x03) - account creation time
    jz m_snac_ch1_3
    cmp eax, 0400h                     ; TLV.Type(0x04) - automated response flag
    jz m_snac_ch1_4
    cmp eax, 0200h                     ; TLV.Type(0x02) - message data
    jz m_snac_ch1_mess
    jmp m_snac_msg_tlv_err

    ;
    ; Возможно, дополнительная ииформация будет обрабатываться
    ; но пока нет 

  m_snac_ch1_1:
    movzx eax, word [mbuff+ecx+2]       ; TLV.Length
    call ntohs
    lea ecx, [eax+ecx+4]
    jmp m_snac_ch1_loop

  m_snac_ch1_6:

    mov eax, dword [mbuff+ecx+4]            ; User status
    call ntohl
    mov [ui.dUserStatus], eax


    movzx eax, word [mbuff+ecx+2]       ; TLV.Length
    call ntohs
    lea ecx, [eax+ecx+4]
    ;
    ; 
   
    
    jmp m_snac_ch1_loop

  m_snac_ch1_8:
    movzx eax, word [mbuff+ecx+2]       ; TLV.Length
    call ntohs
    lea ecx, [eax+ecx+4]
    jmp m_snac_ch1_loop

  m_snac_ch1_5:
    movzx eax, word [mbuff+ecx+2]       ; TLV.Length
    call ntohs
    lea ecx, [eax+ecx+4]
    jmp m_snac_ch1_loop

  m_snac_ch1_f:
    movzx eax, word [mbuff+ecx+2]       ; TLV.Length
    call ntohs
    lea ecx, [eax+ecx+4]
    jmp m_snac_ch1_loop

  m_snac_ch1_3:
    movzx eax, word [mbuff+ecx+2]       ; TLV.Length
    call ntohs
    lea ecx, [eax+ecx+4]
    jmp m_snac_ch1_loop


  m_snac_ch1_4:
    ;movzx eax, word [buff+ecx+2]       ; TLV.Length
    lea ecx, [ecx+4]
    jmp m_snac_ch1_loop



  m_snac_ch1_mess:
    ;
    ;
    movzx eax, word [mbuff+ecx+4]    ;
    cmp eax, 0105h                   ; 05 fragment identifier (array of required capabilities)
    jnz m_snac_ch1_fr_err            ; 01 fragment version

    movzx eax, word [mbuff+ecx+6]    ; Length
    call ntohs

    lea ecx, [ecx+eax+8]             ; Пропускаем  byte array of required capabilities (1 - text)

    movzx eax, word [mbuff+ecx]      ; 	fragment identifier (message text)
    cmp eax, 0101h                   ;  fragment version
    jnz m_snac_ch1_fr_err

    movzx eax, word [mbuff+ecx+2]    ; TLV Length
    call ntohs
    xchg eax, ecx

    lea eax, [eax+8]                 ; Начало текстового сообщения
    lea ecx, [ecx-4]                 ; - sizeof	Message charset number, Message charset subset

    push eax
    push ecx

    ;
    ;  Выводим Message From UIN
    ;

    mov eax, MESS
    call strlen
    mov ecx, eax

    mov eax, MESS
    mov ebx, buff
    call strcpy

    lea ebx, [ebx + ecx]

    ;
    ; Неплохо было бы вывести не UIN а Nickname, если он есть в контакт листе
    ;
    xor esi, esi    ; Счетчик

   m_snac_ch1_next_uin:

    cmp esi, UINS
    jz m_snac_ch1_notfound

    mov edx, UIN_LEN
    imul edx, esi

    lea edx, [uins + edx]
    movzx ecx, byte [ui.bUinLength] 
    strcmp edx, ui.bUin, ecx
    jz m_snac_ch1_uin

    inc esi
    jmp m_snac_ch1_next_uin


    ;
    ; Подходящий UIN есть в контакт-листе
    ;
   m_snac_ch1_uin:

    mov edx, NAME_LEN
    imul edx, esi

    lea edx, [names + edx]
    mov eax, edx


    call strlen
    mov ecx, eax

    mov eax, edx
    call strcpy

    jmp m_snac_ch1_msgfrom


    ;
    ; Если подходящего UIN нет в контакт-листе
    ;
   m_snac_ch1_notfound:
    
    lea eax, [ui.bUin]
    movzx ecx, byte [ui.bUinLength]
    call strcpy

    ;
    ; Вывод сообщения "от кого"
    ;
   m_snac_ch1_msgfrom:

    mov [ebx + ecx], byte 0

    mov eax, buff
    xor ebx, ebx

    call writemsg
    ;
    ;  Само сообщение
    ;

    pop ecx
    pop eax
    lea eax, [mbuff+eax]

    mov ebx, buff
    call strcpy
    mov [ebx+ecx], byte 0
    
    mov eax, buff
    call win2dos
    mov ebx, 00FF0000h
    call writemsg

    ;
    ;  Подтверждаем прием
    ;

    pop ecx
    ;
    ; Пока не реализовано, т.к. не могу найти клиент, который это использует :-)
    ;

    jmp m_fin

  m_snac_msg_tlv_err:
    write_debug 'TLV TYPE MISMATCH'
    pop ecx
    jmp m_fin

  m_snac_ch1_fr_err:
    write_debug 'UNKNOWN FRAGMENT IDENTIFIER OR FRAGMENT VERSION'

  ;m_snac_ch1_end:
    pop ecx

    jmp m_fin

    ;
    ;   Channel 2 message format (rtf messages, rendezvous)
    ;
  m_snac_ch2:
    ;
    ;  отправим сообщение, что канал не поддерживается
    ;  нужны куки и уин
    mov eax, dword [mbuff+10]
    mov [msg_cookie1], eax
    mov eax, dword [mbuff+10+4]
    mov [msg_cookie2], eax

    mov al, [mbuff+10+10]              ; Sender UIN length
    mov [ui.bUinLength], al

    push ecx
    movzx ecx, al

    lea eax, [mbuff+10+11]             ; UIN string 
    lea ebx, [ui.bUin]                 ; Dest
    call strcpy


    mov [ssnac.wFid], 4      ; Family
    mov [ssnac.wSid], 0Bh    ; Subtype
    mov [ssnac.dRi], 0Bh

    mov eax, [msg_cookie1]
    mov dword [buff], eax
    mov eax, [msg_cookie2]
    mov dword [buff+4], eax
    mov word [buff+8], 0200h      ; Channel 2

    mov al, [ui.bUinLength]
    mov [buff+10], al
    lea eax, [ui.bUin]
    lea ebx, [buff+11]
    call strcpy
    lea ecx, [ecx+11]

    mov word [buff+ecx], 0100h    ; reason code (1 - unsupported channel, 2 - busted payload, 3 - channel specific)
    mov edx, ecx

    pop ecx
    mov eax, ssnac
    mov ebx, buff
    call sendsnac


    jmp m_fin

    ;
    ;  Channel 4 message format (typed old-style messages)
    ;
  m_snac_ch4:



  m_ch_other:
    write_debug 'Unknown message channel'

    jmp m_fin


  m_snac_4_other:
    write_debug 'Unknown SNAC Family 4 recived'
    jmp m_fin



    ;
    ; FAMILY 9
    ;
  m_snac_9:
    cmp dx, 3
    jz m_snac_9_3
    jmp m_snac_9_other

    ;
    ; Server sends PRM service limitations to client
    ;
  m_snac_9_3:
    ;
    ; Обработки пока нет
    ;
    if USE_SSI <> 0

    ;
    ; Запрос КЛ с сервера
    ;

    ;
    ; Request contact list (first time)
    ;
    mov [ssnac.wFid], 13h    ; Family
    mov [ssnac.wSid], 04h    ; Subtype
    mov [ssnac.dRi], 04h     ; request-id

    mov eax, ssnac
    mov ebx, buff
    xor edx, edx            
    call sendsnac


    else


    ; Отключено, тк не поддерживается SIQ
    ;

    ;
    ;  Client ask server for SSI service limitations
    ;
    ;mov [ssnac.wFid], 13h  ; Family
    ;mov [ssnac.wSid], 2    ; Subtype
    ;mov [ssnac.dRi], 2     ; request-id
    ;mov eax, ssnac
    ;mov ebx, buff
    ;xor edx, edx
    ;call sendsnac

    ;
    ; последняя стадия соединения
    ;

    ;
    ; Запрашиваем свою информацию
    ;
    mov [ssnac.wFid], 1  ; Family
    mov [ssnac.wSid], 0Eh    ; Subtype
    mov [ssnac.dRi], 0Eh     ; request-id

    mov eax, ssnac
    mov ebx, buff
    xor edx, edx             ; TLV head len
    call sendsnac


    ;
    ; Client sends its DC info and status to server
    ;
    mov [ssnac.wFid], 1  ; Family
    mov [ssnac.wSid], 1Eh    ; Subtype
    mov [ssnac.dRi], 1Eh     ; request-id

    mov [buff], 0           ;  TLV type 06
    mov [buff+1], 6h        ;
    mov [buff+2], 0         ;  TLV data length
    mov [buff+3], 4         ;
    ;
    ;
    mov ax, STATUS_DCDISABLED  ; DC disabled
    call htons
    mov word [buff+4], ax
    mov ax, STATUS_ONLINE
    mov [status], ax
    mov word [buff+6], ax

    mov eax, ssnac
    mov ebx, buff
    mov edx, 8           ; TLV head len+ data len
    call sendsnac


    ;
    ;  Выгружаем на сервер КЛ
    ;
    call uploadkl

    ;
    ; Выгружаем инвизибл лист, пока пустой
    ;
    mov [ssnac.wFid], 9  ; Family
    mov [ssnac.wSid], 7    ; Subtype
    mov [ssnac.dRi], 7

    mov eax, ssnac
    mov ebx, buff
    xor edx, edx
    call sendsnac

    ;
    ; В &RQ Есть пакет установки разрешений. я использую его без изменения
    ;  т.к. не знаю, что он содержит
    ; - возможно, буду использовать позднее

    ;mov [ssnac.wFid], 15  ; Family
    ;mov [ssnac.wSid], 2    ; Subtype
    ;mov [ssnac.dRi], 2

    ;mov word [buff], 0100h   ; 00 01 encapsulated META_DATA
    ;mov word [buff+2], 1000h ; 00 10     Len
    ;mov word [buff+4], 000Eh ;  LE Len
    ;mov word [buff+10], 07D0h ; META_DATA_REQ


    ;mov eax, UIN
    ;call ascitoint
    ;mov dword [buff+6], eax

    ;mov word [buff+12], 0102h   ; request sequence number (incrementing)
    ;mov word [buff+14], 0424h   ; META_SET_PERMS_USERINFO
    ;mov [buff+16], 1            ; authorization (1-required, 0-not required)
    ;mov [buff+17], byte 0       ; webaware (0-no, 1-yes)
    ;mov [buff+18], 1             ; dc_perms (0-any, 1-contact, 2-authorization)
    ;mov [buff+19], 0            ;unknown

    ;mov eax, ssnac
    ;mov ebx, buff
    ;mov edx, 20


    ;
    ; Client READY command
    ;
    mov [ssnac.wFid], 1  ; Family
    mov [ssnac.wSid], 2    ; Subtype
    mov [ssnac.dRi], 2     ; request-id

    mov eax, FAMILY_ARR
    mov ebx, buff
    push ecx
    mov ecx, FA_LEN
    call strcpy
    pop ecx

    mov eax, ssnac
    mov ebx, buff
    mov edx, FA_LEN
    call sendsnac


    ;
    ; Запрашиваем offline сообщения
    ;
    mov [ssnac.wFid], 15h  ; Family
    mov [ssnac.wSid], 2    ; Subtype
    mov [ssnac.dRi], 2     ; request-id

    mov word [buff], 0100h      ;  TLV type 01
    mov word [buff+2], 0A00h    ;  00 0a Длина
    mov word [buff+4], 0008h    ;  08 00
    lea eax, [vtable + vartable.uin]
    call ascitoint
    mov dword [buff+6], eax

    mov word [buff+10],  003Ch ; 3C 00 - Запрос на оффлайновые сообщения
    mov word [buff+12],  0002  ; 02 00 - request sequence number
   
    mov edx, 14        ; Общий размер данных в буфере

    mov eax, ssnac
    mov ebx, buff
    call sendsnac

    

    ;
    ; Запрашиваем информацию всех UIN
    ;
    call getinfo
    ;
    ; завершено соединение
    ;
    mov [login], 2


    end if   ; USE_SSI = 0

    jmp m_fin

  m_snac_9_other:
    write_debug 'Unknown SNAC Family 9 Recived'
    jmp m_fin


    ;
    ; FAMILY 13
    ;
  m_snac_13:
    cmp dx, 3
    jz m_snac_13_3
    cmp dx, 6
    jz m_snac_13_6
    cmp dx, 0fh
    jz m_snac_13_F

    jmp m_snac_13_other

    ;
    ; Server sends SSI service limitations to client
    ;
  m_snac_13_3:
    ;
    ; Обработки пока нет
    ;

    ;
    ;  SNAC(13,05)	  Client check if its local SSI copy is up-to-date
    ;
    mov [ssnac.wFid], 13h  ; Family
    mov [ssnac.wSid], 5    ; Subtype
    mov [ssnac.dRi], 5     ; request-id
    mov eax, ssnac
    ;
    ; 
    ;
    mov [buff], 03Dh      ;
    mov [buff+1], 0E7h    ;   	modification date/time of client local SSI copy
    mov [buff+2], 48h     ;
    mov [buff+3], 17h     ;
    ;
    ;
    mov [buff+4], 00      ;
    mov [buff+5], 00h     ;  	number of items in client local SSI copy
    
    mov ebx, buff
    mov edx, 5
    call sendsnac

    jmp m_fin


    ;
    ; Server contact list reply
    ;
  m_snac_13_6:

    lea eax, [mbuff+10]     ; В eax загружаем адрес приемного буфера+ размер заголовка snac

    call ssi_process_data   ; Обработка пакета с КЛ

    ;
    ; Активируем SSI
    ;

    mov [ssnac.wFid], 13h  ; Family
    mov [ssnac.wSid], 7    ; Subtype
    mov [ssnac.dRi], 7     ; request-id
    mov eax, ssnac
    mov ebx, buff
    xor edx, edx
    call sendsnac


    ;
    ; последняя стадия соединения
    ;

    ;
    ; Запрашиваем свою информацию
    ;
    mov [ssnac.wFid], 1  ; Family
    mov [ssnac.wSid], 0Eh    ; Subtype
    mov [ssnac.dRi], 0Eh     ; request-id

    mov eax, ssnac
    mov ebx, buff
    xor edx, edx             ; TLV head len
    call sendsnac


    ;
    ; Client sends its DC info and status to server
    ;
    mov [ssnac.wFid], 1  ; Family
    mov [ssnac.wSid], 1Eh    ; Subtype
    mov [ssnac.dRi], 1Eh     ; request-id

    mov [buff], 0           ;  TLV type 06
    mov [buff+1], 6h        ;
    mov [buff+2], 0         ;  TLV data length
    mov [buff+3], 4         ;
    ;
    ;
    mov ax, STATUS_DCDISABLED  ; DC disabled
    call htons
    mov word [buff+4], ax
    mov ax, STATUS_ONLINE
    mov [status], ax
    mov word [buff+6], ax

    mov eax, ssnac
    mov ebx, buff
    mov edx, 8           ; TLV head len+ data len
    call sendsnac


    ;
    ;  Выгружаем на сервер КЛ
    ;   FIXME Возможно, здесь не нужна эта функция
    ;call uploadkl

    ;
    ; Выгружаем инвизибл лист, пока пустой
    ;
    ;mov [ssnac.wFid], 9  ; Family
    ;mov [ssnac.wSid], 7    ; Subtype
    ;mov [ssnac.dRi], 7

    ;mov eax, ssnac
    ;mov ebx, buff
    ;xor edx, edx
    ;call sendsnac

    ;
    ; В &RQ Есть пакет установки разрешений. я использую его без изменения
    ;  т.к. не знаю, что он содержит
    ; - возможно, буду использовать позднее

    ;mov [ssnac.wFid], 15  ; Family
    ;mov [ssnac.wSid], 2    ; Subtype
    ;mov [ssnac.dRi], 2

    ;mov word [buff], 0100h   ; 00 01 encapsulated META_DATA
    ;mov word [buff+2], 1000h ; 00 10     Len
    ;mov word [buff+4], 000Eh ;  LE Len
    ;mov word [buff+10], 07D0h ; META_DATA_REQ


    ;mov eax, UIN
    ;call ascitoint
    ;mov dword [buff+6], eax

    ;mov word [buff+12], 0102h   ; request sequence number (incrementing)
    ;mov word [buff+14], 0424h   ; META_SET_PERMS_USERINFO
    ;mov [buff+16], 1            ; authorization (1-required, 0-not required)
    ;mov [buff+17], byte 0       ; webaware (0-no, 1-yes)
    ;mov [buff+18], 1             ; dc_perms (0-any, 1-contact, 2-authorization)
    ;mov [buff+19], 0            ;unknown

    ;mov eax, ssnac
    ;mov ebx, buff
    ;mov edx, 20


    ;
    ; Client READY command
    ;
    mov [ssnac.wFid], 1  ; Family
    mov [ssnac.wSid], 2    ; Subtype
    mov [ssnac.dRi], 2     ; request-id

    mov eax, FAMILY_ARR
    mov ebx, buff
    push ecx
    mov ecx, FA_LEN
    call strcpy
    pop ecx

    mov eax, ssnac
    mov ebx, buff
    mov edx, FA_LEN
    call sendsnac


    ;
    ; Запрашиваем offline сообщения
    ;
    mov [ssnac.wFid], 15h  ; Family
    mov [ssnac.wSid], 2    ; Subtype
    mov [ssnac.dRi], 2     ; request-id

    mov word [buff], 0100h      ;  TLV type 01
    mov word [buff+2], 0A00h    ;  00 0a Длина
    mov word [buff+4], 0008h    ;  08 00
    lea eax, [vtable + vartable.uin]
    call ascitoint
    mov dword [buff+6], eax

    mov word [buff+10],  003Ch ; 3C 00 - Запрос на оффлайновые сообщения
    mov word [buff+12],  0002  ; 02 00 - request sequence number
   
    mov edx, 14        ; Общий размер данных в буфере

    mov eax, ssnac
    mov ebx, buff
    call sendsnac

    

    ;
    ; Запрашиваем информацию всех UIN
    ; FIXME Возможно, здесь не нужна эта функция
    ;call getinfo
    ;
    ; завершено соединение
    ;
    mov [login], 2


    jmp m_fin




    ;
    ;  Server tell client its local copy up-to-date
    ;
  m_snac_13_F:
    ;
    ;  Обработки нет 
    ;

    ;
    ;  Client activates server SSI data
    ;
    mov [ssnac.wFid], 13h  ; Family
    mov [ssnac.wSid], 7    ; Subtype
    mov [ssnac.dRi], 7     ; request-id
    mov eax, ssnac
    mov ebx, buff
    xor edx, edx
    call sendsnac

    ;
    ; последняя стадия соединения
    ;

    ;
    ; Client sends its DC info and status to server
    ;
    mov [ssnac.wFid], 1  ; Family
    mov [ssnac.wSid], 1Eh    ; Subtype
    mov [ssnac.dRi], 1Eh     ; request-id

    mov [buff], 0           ;  TLV type 06
    mov [buff+1], 6h        ;
    mov [buff+2], 0         ;  TLV data length
    mov [buff+3], 4         ;
    ;
    ;
    mov ax, STATUS_DCDISABLED  ; DC disabled
    call htons
    mov word [buff+4], ax
    ;
    ; 
    mov ax, [status]
    mov word [buff+6], ax

    mov eax, ssnac
    mov ebx, buff
    mov edx, 8           ; TLV head len+ data len
    call sendsnac

    ;
    ; Client READY command
    ;
    mov [ssnac.wFid], 1  ; Family
    mov [ssnac.wSid], 2    ; Subtype
    mov [ssnac.dRi], 2     ; request-id

    mov eax, FAMILY_ARR
    mov ebx, buff
    push ecx
    mov ecx, FA_LEN
    call strcpy
    pop ecx

    mov eax, ssnac
    mov ebx, buff
    mov edx, FA_LEN
    call sendsnac


    ;
    ; Запрашиваем offline сообщения
    ;
    mov [ssnac.wFid], 15h  ; Family
    mov [ssnac.wSid], 2    ; Subtype
    mov [ssnac.dRi], 2     ; request-id

    mov word [buff], 0100h      ;  TLV type 01
    mov word [buff+2], 0A00h    ;  00 0a Длина
    mov word [buff+4], 0008h    ;  08 00
    lea eax, [vtable + vartable.uin]
    call ascitoint
    mov dword [buff+6], eax

    mov [buff+10],  003Ch ; 3C 00 - Запрос на оффлайновые сообщения
    mov [buff+12],  0002  ; 02 00 - request sequence number
   
    mov edx, 14        ; Общий размер данных в буфере

    mov eax, ssnac
    mov ebx, buff
    call sendsnac



    jmp m_fin

  m_snac_13_other:
    write_debug 'Unknown SNAC Family 13 Recived'
    jmp m_fin




    ;
    ;  Family 15
    ;

  m_snac_15:
    
    cmp dx, 3
    jz m_snac_15_3
    
    jmp m_snac_15_other


    ;
    ; Server sends message #N
    ;
  m_snac_15_3:
    ;
    ; Определяем подтип принятого пакета
    ;

    ;write_debug 'SNAC 15, 3'

    xor eax, eax
    mov ax, word [mbuff+10]  ; + SNAC.head size
    cmp ax, 0100h            ; 00 01 TLV type
    jnz m_snac_tlv_err

    mov ax, word [mbuff+10+10]
    cmp ax, 0041h             ; Offline Message
    jz m_snac_offline_mes
    cmp ax, 0042h             ; End messages
    jz m_snac_offline_end
    cmp ax, 07DAh
    jz m_snac_meta_data


    write_debug 'Unknown Subtype SNAC (15,3)'
    jmp m_fin

  m_snac_offline_mes:
    mov eax, MESS                      ;
    call strlen                        ;  Выводим строку с сообщением о отправителе и времени отправки
    push ecx                           ;
    mov ecx, eax                   ;
    mov eax, MESS
    mov ebx, buff
    call strcpy

    mov eax, dword [mbuff+14+10]          ; Sender UIN
    lea ebx, [buff+ecx]                ; После строчки о сообщении
    call int2strd

    lea ebx, [ebx+eax]
    mov [ebx], byte ' '
    inc ebx

                                       ; + Длина UIN
    movzx eax, byte [mbuff+21+10]         ; Day
    call int2strd

    lea ebx, [ebx+eax]
    mov [ebx], byte '.'
    inc ebx

    
    movzx eax, byte [mbuff+20+10]         ;Mounth
    call int2strd

    lea ebx, [ebx+eax]
    mov [ebx], byte ' '
    inc ebx

    movzx eax, [mbuff+22+10]              ; Hour
    call int2strd

    lea ebx, [ebx+eax]
    mov [ebx], byte ':'
    inc ebx

    movzx eax, [mbuff+23+10]              ; Minute
    call int2strd

    lea ebx, [ebx+eax]
    ;mov [ebx], byte ' '
    ;inc ebx

    mov [ebx], byte 0                      ; Str end
    mov eax, buff
    xor ebx, ebx

    call writemsg

    movzx ecx, word [mbuff+26+10]             ; Длина соообщения
    lea eax, [mbuff+28+10]
    mov ebx, buff
    call strcpy

    mov [ebx+ecx], byte 0

    mov eax, buff
    call win2dos                              ;перекодируем

    mov ebx, 00FF0000h                        ;Цвет

    call writemsg


    pop ecx

    jmp m_fin


  m_snac_offline_end:
    ;
    ; Удаляем сообщения на сервере
    ;
    mov [ssnac.wFid], 15h  ; Family
    mov [ssnac.wSid], 2    ; Subtype
    mov [ssnac.dRi], 0602h     ; request-id

    mov word [buff], 0100h     ;  00 01	TLV.Type(1) - encapsulated META_DATA1
    mov word [buff+2], 0A00h   ;  00 0A	TLV.Length
    mov word [buff+4], 0008h   ;  08 00	data chunk size (TLV.Length-2)
    lea eax, [vtable + vartable.uin]
    call ascitoint
    mov dword [buff+6], eax    ; xx xx xx xx (LE) client uin
    mov word [buff+10], 003Eh  ; 3E 00	(LE) data type: delete offline msgs request cmd
    mov word [buff+12], 0007h  ;  xx xx	(LE) request sequence number

    mov edx, 14                ; Размер данных
    mov eax, ssnac
    mov ebx, buff
    call sendsnac



    jmp m_fin

    ;
    ; Ответ на запрос о пользователях
    ;
  m_snac_meta_data:
    ;
    ; Определяем очередной подтип :-)
    ;
    mov ax, word [mbuff+10+14]
    cmp ax, 0104h                   ;data subtype: META_SHORT_USERINFO
    jz m_snac_short_userinfo
    cmp ax, 00C8h
    jz m_snac_basic_userinfo        ;data subtype: META_BASIC_USERINFO
    write_debug 'Unknown META DATA subtype'
    jmp m_fin



  m_snac_short_userinfo:
    ;
    ; Из всей информации пока нужен только ник
    ;
    mov al, [mbuff+10+16]
     cmp al, 0Ah                    ;success byte
     jnz m_fin

     movzx eax, word [mbuff+10+12]  ;request sequence number
     ;
     ; В запросе я использовал порядковый номер юзера в КЛ
     lea ebx, [mbuff+10+19]         ;nickname string
     ; Длина строки не нужна, т.к. строка Null-Terminated
     ;Определяем статус
     mov ecx, 4
     imul ecx, eax
     mov ecx, [stats+ecx]

     call loadbb



    jmp m_fin

  ;
  ; тк SIQ на запрос короткой инфы отвечает 
  ; пакетом базовой информации, реализую пока только его
  ;
  m_snac_basic_userinfo:
     mov al, [mbuff+10+16]
     cmp al, 0Ah                    ;success byte
     jnz m_fin

     movzx eax, word [mbuff+10+12]  ;request sequence number
     ;
     ; В запросе я использовал порядковый номер юзера в КЛ
     lea ebx, [mbuff+10+19]         ;nickname string
     ; Длина строки не нужна, т.к. строка Null-Terminated
     ;Определяем статус
     mov ecx, 4
     imul ecx, eax
     mov ecx, [stats+ecx]

     call loadbb

       

    jmp m_fin

  m_snac_tlv_err:
    write_debug 'TLV TYPE MISMATCH'

    jmp m_fin
    

  m_snac_15_other:

    write_debug 'Unknown SNAC Family 15 Recived'

    jmp m_fin


  m_other_snac:
    write_debug 'Unknown SNAC recived'
    jmp m_fin



  m_fin:
    ;pop edx
    ;pop ebx
    ;pop eax
    popad
    popf
  ret

; Для перевода DWORD из Little Endian в Big Endian
; и наоборот :-)
; <--EAX DWORD
; -->EAX
;
  ntohl:
  htonl:
    ;pushf
    push ebx
    ;push ecx

    xor ebx, ebx

    mov bl, ah
    mov bh, al
    shl ebx, 16
    
    shr eax, 16
    mov bl, ah
    mov bh, al
    
    mov eax, ebx

    ;pop ecx
    pop ebx
    ;popf
  ret


; Для перевода WORD из Little Endian в Big Endian
; <--AX WORD                                             
; -->AX WORD
;

  ntohs:
  htons:
    ;pushf
    push ebx

    xor ebx, ebx
    mov bl, ah
    mov bh, al
    mov eax, ebx

    pop ebx
    ;popf
  ret

;
; парсит SNAC
; <--EAX указатель на SNAC_head
; <--EBX указатель на буффер
; -->EAX указатель начало данных = buffer+sizeof SNAC_head
; 
;
  snacpar:
    pushf
    push ecx
    ;push edx

    mov cl, [ebx+1]     ; Family (service) id number младший байт
    mov ch, [ebx]       ; старший
    mov word [eax], cx

    mov cl, [ebx+3]     ; Family subtype id number
    mov ch, [ebx+2]     ;
    mov word [eax+2], cx

    mov cl, [ebx+5]      ; SNAC flags
    mov ch, [ebx+4]      ;
    mov word [eax+4], cx ;
                           
    mov cl, [ebx+7]         ;
    mov ch, [ebx+6]         ;
    mov word [eax+8], cx    ; SNAC request id
    mov cl, [ebx+8]         ;
    mov ch, [ebx+7]         ;
    mov word [eax+6], cx    ;
    
    add ebx, 10             ;Размер заголовка
    mov eax, ebx


    ;pop edx
    pop ecx 
    popf
  ret

;
; парсит userinfo block
;  FIXIT
;

;  userinfopar:
;    pushf
;
;
;
;
;
;
;    popf
;  ret

;
;  посылка сообщения
;  [eax] <-- текстовый буфер  \
;  [ebx] <-- UIN              /  Null-terminated

  sendmsg:
    pushf
    pushad
    push eax
    push ebx

    mov [ssnac.wFid], 4h  ; Family
    mov [ssnac.wSid], 6    ; Subtype
    mov [ssnac.dRi], 106h     ; request-id
    ;
    ;  Получаем время с запуска системы, для cookie
    ;
    ;mov eax, 26
    ;mov ebx, 9
    ;int 40h
    mcall 26, 9

    mov dword [buff], eax      ; Cookie 1
    mov dword [buff+4], eax    ; Cookie 2

    mov word [buff+8], 0100h   ; Message channel 00 01


    pop ebx
    mov eax, ebx
    call strlen

    mov [buff+10], al
    mov ecx, eax
    mov eax, ebx
    lea ebx, [buff+11]
    call strcpy
    lea ecx, [ecx+11]

    mov word [buff+ecx], 0200h    ;  	TLV.Type(0x02) - message data
    
    ;push ecx                      ;
                                   ; TLV.Length

    mov word [buff+ecx+4], 0105h         ; 05 01   01 - fragment version, 05 - fragment identifier
    
    mov word [buff+ecx+6], 0100h         ; data length 
    
    mov [buff+ecx+8], 01                 ; byte array of required capabilities (1 - text)

    mov [buff+ecx+9], 01                 ; fragment identifier (text message)
    mov [buff+ecx+10], 01                 ; fragment version

    pop ebx
    mov eax, ebx
    call strlen
    mov edx, eax
    lea eax, [eax+4]                     ; Длина сообщения + Message charset number+ Message language number
    
    call htons
    mov word [buff+ecx+11], ax

    mov eax, edx
    lea eax, [eax+13]                     ; + длина служебных данных
    call htons
    mov word [buff+ecx+2], ax


    mov word [buff+ecx+13], 0700h        ; Message charset number
    mov word [buff+ecx+15], 0300h        ; Message language number

    mov eax, ecx
    mov ecx, edx                         ; Len
    lea edx, [eax+17]

    mov eax, ebx                         ;Source
    
    lea ebx, [buff+edx]                  ;Dest
    
    call strcpy
    lea ecx, [ecx+edx]                   ; +String length
    
    mov [buff+ecx], byte 0
    mov eax, ebx
    call dos2win


    mov word [buff+ecx], 0600h               ; TLV.Type(0x06) - store message if recipient offline
    mov word [buff+ecx+2], 0                 ; TLV.Length

    lea edx, [ecx+4]                         ; +TLV_head length
    mov eax, ssnac
    mov ebx, buff
    mov ecx, [socket]
    call sendsnac


 
    popad
    popf
  ret

;
;   Запрос информации UINов
;
  getinfo:
    pushad
    pushf
    ;
    ; SNAC (15,2) - Meta information request 
    ;

    mov [ssnac.wFid], 15h  ; Family
    mov [ssnac.wSid], 2    ; Subtype
    mov [ssnac.dRi], 702h     ; request-id

    mov word [buff], 0100h         ;TLV.Type(1) - encapsulated META_DATA
    mov word [buff+2], 1000h       ; 00 10  TLV.Length
    mov word [buff+4], 000Eh       ; (LE)	 	data chunk size (TLV.Length-2)
    lea eax, [vtable + vartable.uin]
    call ascitoint
    mov dword [buff+6], eax        ;(LE)	 	request owner uin
    mov word [buff+10], 07D0h      ;data type: META_DATA_REQ
    ;mov word [buff+12], 0008h      ; request sequence number <<<-- Может меняться FIXIT
    mov word [buff+14], 04BAh      ; data subtype: META_SHORTINFO_REQUEST

    mov ecx, [socket]
    mov edx, 20

    xor esi, esi        ; Счетчик
    xor eax, eax

  gi_loop:
    mov ebx, esi
    mov word [buff+12], bx      ; request sequence number
    mov ebx, UIN_LEN
    imul ebx, esi
    mov al,  [uins+ebx]
    cmp al, 0
    jz  gi_end

    lea eax, [uins+ebx]
    call ascitoint
    mov dword [buff+16], eax

    mov eax, ssnac
    mov ebx, buff
    
    call sendsnac
    inc esi
    cmp esi, UINS
    jnc gi_end
    jmp gi_loop






  gi_end:
    popf
    popad
  ret

;
; Загружаем локальный КЛ на сервер для получения статуса юзеров
;
  uploadkl:
    pushf
    pushad
    ;
    ;  Add buddy(s) to contact list
    ;
    mov [ssnac.wFid], 3   ; Family
    mov [ssnac.wSid], 4   ; Subtype
    mov [ssnac.dRi], 4    ; request-id

    xor esi, esi          ; Счетчик
    xor edx, edx          ; Заполнено байт

  ukk_loop:
    mov ebx, UIN_LEN
    imul  ebx, esi
    mov al, [uins+ebx]
    cmp al, 0
    jz ukk_end
    lea eax, [uins+ebx]
    
    call strlen
    mov [buff+edx], al
    inc edx

    mov ecx, eax
    lea eax, [uins+ebx]    ; Source
    lea ebx, [buff+edx]
    call strcpy
    add edx, ecx
    inc esi
    cmp esi, UINS
    jz ukk_end
    jmp ukk_loop

    



  ukk_end:
    mov eax, ssnac
    mov ebx, buff
    mov ecx, [socket]
    call sendsnac

    popad
    popf
  ret

;
;
;
  sendkeep:
    pushf
    pushad
    cmp [login], 2
    jnz @f
    mov ax, [timer]
    cmp ax, 300           ;60 c
    jb @f
    mov [timer], 0
    mov [flap.bId], FLAP_ID
    mov [flap.bCh], 5         ;Keep alive
    mov [flap.wDs], 0
    inc [seq]
    mov ax, [seq]
    mov [flap.wSn], ax
    mov eax, flap
    mov ebx, buff
    mov ecx, [socket]
    call sendflap


  @@:
    popad
    popf
  ret

;
; Функция для установки статуса
; статус в переменной status
 setstatus:
  push eax
  push ebx
  push edx
    ;
    ; Client sends its DC info and status to server
    ;
    mov [ssnac.wFid], 1  ; Family
    mov [ssnac.wSid], 1Eh    ; Subtype
    mov [ssnac.dRi], 1Eh     ; request-id

    mov [buff], 0           ;  TLV type 06
    mov [buff+1], 6h        ;
    mov [buff+2], 0         ;  TLV data length
    mov [buff+3], 4         ;
    ;
    ;
    mov ax, STATUS_DCDISABLED  ; DC disabled
    call htons
    mov word [buff+4], ax
    ;
    ;
    mov ax, [status]
    mov word [buff+6], ax

    mov eax, ssnac
    mov ebx, buff
    mov edx, 8           ; TLV head len+ data len
    call sendsnac

  pop edx
  pop ebx
  pop eax
 ret


;
; Макрос пропускает все пробелы в строке до 
; 1 го значащего символа
; eax - указатель на null-terminated строку

macro skip_spaces {
  local ..sp_end, ..sp_loop

  push ebx
  push ecx

  xor ebx, ebx
  xor ecx, ecx

 ..sp_loop:
 

  mov bl, [eax + ecx]
  cmp bl, 0x20
  jnz ..sp_end


  inc ecx
  jmp ..sp_loop



 ..sp_end:
  lea eax, [eax + ecx]

  pop ecx
  pop ebx
}





;
; Обработка команд
; В еax передается указатель на строку. Команда и аргументы разделены пробелом
; команда начинается с /
; в eax - результат выполнения команды, -1 команда не существует, 0 ок, другие зависят от команды

 cmd:
   push ebx
   push ecx
   push edi
   push esi

   ;
   ; Проверить первый символ 
   ;
   xor ebx, ebx
   mov bl, [eax]
   cmp bl, '/'
   jnz cmd_end

   ;
   ; Разделение по 1й букве команды
   ;
   mov bl, [eax + 1]

   cmp bl, 'c'
   jz cmd_c

   cmp bl, 'e'
   jz cmd_e

   cmp bl, 's'
   jz cmd_s

   jmp cmd_no

  cmd_c:

  cmd_e:

   lea ebx, [eax + 1]
   strcmp ebx, str_exit, str_exit.len
   jz cmd_exit

   jmp cmd_no




  cmd_s:

   lea ebx, [eax + 1]
   strcmp ebx, str_status, str_status.len
   jz cmd_status
  
   jmp cmd_no



  cmd_exit:


  cmd_status:
   ;
   ; установить статус и послать пакет смены статуса
   ;
   lea eax, [eax + 1 + str_status.len]
   skip_spaces

   strcmp eax, str_online, str_online.len
   jz cmd_st_online

   strcmp eax, str_away, str_away.len
   jz cmd_st_away

   strcmp eax, str_na, str_na.len
   jz cmd_st_na

   strcmp eax, str_dnd, str_dnd.len
   jz cmd_st_dnd

   strcmp eax, str_bisy, str_bisy.len
   jz cmd_st_bisy

   strcmp eax, str_free4chat, str_free4chat.len
   jz cmd_st_free4chat

   ;
   ; Статус не определен.
   ; Вывести сообщение  о доступных статусах
   ;
   mov eax, str_status_message
   xor ebx, ebx
   call writemsg

   jmp cmd_end


  cmd_st_online:

  cmd_st_away:

  cmd_st_na:

  cmd_st_dnd:

  cmd_st_bisy:

  cmd_st_free4chat:


  cmd_no:

  cmd_end:
   pop esi
   pop edi
   pop ecx
   pop ebx

 ret




; <--- initialised data --->
DATA
include "parser_data.inc"
include "ssi_data.inc"
include "comp_data.inc"


head db 'KI',0


;
MESS db 'Message from ', 0
CUSER db 'Current user: ', 0 

;
; Список IP серверов ICQ

;205.188.153.121
;icq_ip db '64.12.200.089',0
;icq_ip db '64.12.161.185',0 
;icq_ip db '205.188.179.233',0

 
;
flap FLAP_head
rflap FLAP_head
;
ssnac SNAC_head        ; для передачи SNAC
rsnac SNAC_head        ; для принятого SNAC
;
ui UI_head             ; User info
;
procinfo process_information
;
;UIN db '362820484',0
;PASS db 'test',0
ID_STRING db 'ICQ Inc. - Product of ICQ (TM).2000b.4.65.1.3281.85',0
;ID_STRING db 'ICQ Inc. - Product of ICQ (TM).2001b.5.17.1.3642.85',0


;CAPABILITIES db 0x09, 0x46, 0x13, 0x49, 0x4C, 0x7F, 0x11, 0xD1, 0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00,\ 
                ;0x97, 0xB1, 0x27, 0x51, 0x24, 0x3C, 0x43, 0x34, 0xAD, 0x22, 0xD6, 0xAB, 0xF7, 0x3F, 0x14, 0x92,\ 
CAPABILITIES db 0x2E, 0x7A, 0x64, 0x75, 0xFA, 0xDF, 0x4D, 0xC8, 0x88, 0x6F, 0xEA, 0x35, 0x95, 0xFD, 0xB6, 0xDF,\
                'KOLIBRI KI(cq)',0,0
                ;0x09, 0x46, 0x13, 0x44, 0x4C, 0x7F, 0x11, 0xD1, 0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 

; 1 строка
; {09461349-4C7F-11D1-8222-444553540000} 
; Client supports channel 2 extended, TLV(0x2711) based messages. Currently used only by ICQ clients. 
;ICQ clients and clones use this GUID as message format sign. Trillian client use another GUID 
; in channel 2 messages to implement its own message format (trillian doesn't use TLV(x2711) in SecureIM channel 2 messages!).
;
; 2 строка
; {97B12751-243C-4334-AD22-D6ABF73F1492}  
; Client supports RTF messages. This capability currently used by ICQ service and ICQ clients.
;
; 4 строка
; {0946134E-4C7F-11D1-8222-444553540000} 
; Client supports UTF-8 messages. This capability currently used by AIM service and AIM clients
;



                                     
;
; From &RQ
;

;CAPABILITIES db 0x09, 0x46, 0x13, 0x49, 0x4C, 0x7F, 0x11, 0xD1, 0x82, 0x22, 0x44, 0x45,\    ;...P.F.IL.TВ"DE
;                0x53, 0x54, 0x00, 0x00, 0x09, 0x46, 0x13, 0x44, 0x4C, 0x7F, 0x11, 0xD1, 0x82, 0x22, 0x44, 0x45,\        ;ST...F.DL.TВ"DE
;                0x53, 0x54, 0x00, 0x00, 0x09, 0x46, 0x13, 0x4E, 0x4C, 0x7F, 0x11, 0xD1, 0x82, 0x22, 0x44, 0x45,\        ;ST...F.NL.TВ"DE
;                0x53, 0x54, 0x00, 0x00, 0x09, 0x46, 0x00, 0x00, 0x4C, 0x7F, 0x11, 0xD1, 0x82, 0x22, 0x44, 0x45,\        ;ST...F..L.TВ"DE
;                0x53, 0x54, 0x00, 0x00, 0x26, 0x52, 0x51, 0x69, 0x6E, 0x73, 0x69, 0x64, 0x65, 0x02, 0x07, 0x09,\        ;ST..&RQinside...
;                0x00, 0x00, 0x00, 0x00                                         


C_LEN = 32
;C_LEN = 80
ICBM_PARAMS db 0, 0, 0, 0, 0, 0Bh, 01Fh, 040h, 3, 0E7h, 3, 0E7h, 0, 0, 0, 0
ICBMP_LEN = 16           ;    ^^^ from &RQ


;
; from &rq
;
FAMILY_ARR db  0x00, 0x01, 0x00, 0x03, 0x01, 0x10, 0x04, 0x7B,  0x00, 0x13, 0x00, 0x02, 0x01, 0x10, 0x04, 0x7B,\    
               0x00, 0x02, 0x00, 0x01, 0x01, 0x01, 0x04, 0x7B,  0x00, 0x03, 0x00, 0x01, 0x01, 0x10, 0x04, 0x7B,\    
               0x00, 0x15, 0x00, 0x01, 0x01, 0x10, 0x04, 0x7B,  0x00, 0x04, 0x00, 0x01, 0x01, 0x10, 0x04, 0x7B,\    
               0x00, 0x06, 0x00, 0x01, 0x01, 0x10, 0x04, 0x7B,  0x00, 0x09, 0x00, 0x01, 0x01, 0x10, 0x04, 0x7B,\    
               0x00, 0x0A, 0x00, 0x01, 0x01, 0x10, 0x04, 0x7B,  0x00, 0x10, 0x00, 0x01, 0x00, 0x10, 0x06, 0x6A    

;
;
; 

FA_LEN = 50h
;
;
;
ID_NUM = 010Ah
MAJOR = 05h
;MAJOR = 04h
;MINOR = 041h
MINOR = 011h
LESSER = 01h
;BUILD = 0CD1h
BUILD = 0E3Ah
DISTR = 055h
;
;

TCB_ESTABLISHED = 4
TCB_CLOSED = 11
;
CL_LANG db 'en',0
CL_COUNTRY db 'us',0

 
sbuff db 1024 dup 0     ; Буфер для передачи используется внутри sendflap

;recived db 0              ; Принято данных из тела пакета

;rbuff db 1024 dup 0     ; Приемный буфер
tbuff db 512 dup 0      ; Для TLV
srv_cookie db 512 dup 0 ; Куки для авторизации
bos_address db 128 dup 0 ; Адрес BOS сервера
cookie_len dw 0          ; Длина куки
seq dw 0                 ; Sequence number
bos_ip dd 0
bos_port dd 0
status dw 0             ; status

mbuff db 2048 dup 0     ; Для приема
MBUFF_SIZE              = 2048

hrf db 0                ; Флаг приема заголовка

mouse_flag dd 0
socket dd 0
login db 0

msg_cookie1 dd 0        ;   Используются для потверждения приема сообщений
msg_cookie2 dd 0        ;

curruser    db 0        ;  текущий пользователь, которому будут отправляться сообщения
                        ; - Номер в КЛ по порядку


timer dw 0

;ltest db "ADMIN",0
buff db 1024 dup 0
;  lbuff db 8 dup 0 	 ; Для 1 пакета от сервера

;
; Строки команд для сравнения
;
str_status db 'status '
str_status.len = $ - str_status
str_exit db 'exit '
str_exit.len = $ - str_exit
;
; Строки статусов для сравнения
;
str_away db 'away'
str_away.len = $ - str_away

str_dnd db 'dnd'
str_dnd.len = $ - str_dnd

str_bisy db 'bisy'
str_bisy.len = $ - str_bisy

str_na db 'na'
str_na.len = $ - str_na

str_online db 'online'
str_online.len = $ - str_online

str_free4chat db 'free4chat'
str_free4chat.len = $ - str_free4chat

str_status_message db 'Доступные статусы: away, bisy, na, dnd, online, free4chat',0


;
;
;

cfg_message db 'Config:',0
                 

;
; EDITBOXES
;
inputbuff: 
       rb 512

inputbox edit_box 490,10,460,0xffffff,0x6a9480,0,0xAABBCC,0,511,inputbuff,ed_focus,0,0


; <--- uninitialised data --->
UDATA


MEOS_APP_END
; <--- end of MenuetOS application --->

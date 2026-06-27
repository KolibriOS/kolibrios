;==============================================================================
; Fill background with gradient
;==============================================================================

fill_background_gradient:
    ; Функция 15 - работа с фоновой графикой
    ; Подфункция 4 - установить режим отрисовки фона.
    ; Режим отрисовки - замостить (1), растянуть (2)
    mcall   SF_BACKGROUND_SET,SSF_MODE_BG,2

    ; Функция 15 - работа с фоновой графикой
    ; Подфункция 1 - установить размер фонового изображения.
    mcall   SF_BACKGROUND_SET,SSF_SIZE_BG,2,2

    mov     eax,[color]
    mov     [cm+0],al
    mov     [cm+9],al
    shr     eax,8
    mov     [cm+1],al
    mov     [cm+10],al
    shr     eax,8
    mov     [cm+2],al
    mov     [cm+11],al

    mov     eax,[color2]
    mov     [cm+3],al
    mov     [cm+6],al
    shr     eax,8
    mov     [cm+4],al
    mov     [cm+7],al
    shr     eax,8
    mov     [cm+5],al
    mov     [cm+8],al

    ; Функция 15 - работа с фоновой графикой
    ; Подфункция 5 - поместить блок пикселей на фон.
    ; - Указатель на данные в формате BBGGRRBBGGRR
    ; - Cмещение в данных фонового изображения
    ; - Размер данных в байтах = 3 * число пикселей
    mcall   SF_BACKGROUND_SET,SSF_IMAGE_BG,cm,0,3*4

    ; Функция 15 - работа с фоновой графикой
    ; Подфункция 3 - перерисовать фон.
    mcall   SF_BACKGROUND_SET,SSF_REDRAW_BG

    stdcall save_eskin_ini, 'H '

    ret
;end_set_background


;==============================================================================
; Fill background with noisy (random) pixes
;==============================================================================

fill_background_noisy:
    mcall   SF_SYS_MISC, SSF_MEM_ALLOC, 256 * 256 * 3
    mov     [bgimg_buf], eax
    test    eax, eax
    jnz     .ok
    ret
  .ok:

    mov     edx, eax
    mov     ecx, 256 * 256
  @@:
    stdcall random, 15 + 1
    sub     al, 15 / 2

    mov     bh, byte [color + 0]
    add     bh, al
    mov     [edx + 0], bh
    mov     bh, byte [color + 1]
    add     bh, al
    mov     [edx + 1], bh
    mov     bh, byte [color + 2]
    add     bh, al
    mov     [edx + 2], bh
    add     edx, 3
    loop    @b

    mcall   SF_BACKGROUND_SET, SSF_SIZE_BG, 256, 256
    mcall   SF_BACKGROUND_SET, SSF_MODE_BG, 1
    mcall   SF_BACKGROUND_SET, SSF_IMAGE_BG, [bgimg_buf], 0, 256 * 256 * 3
    mcall   SF_BACKGROUND_SET, SSF_REDRAW_BG

    mcall   SF_SYS_MISC, SSF_MEM_FREE, [bgimg_buf]
    stdcall save_eskin_ini, 'B '
ret

;-------------------------------

 proc random uses ebx ecx edx, max_value
    mov     ebx, 0
    mov     eax, ebx
    or      eax, eax
    jnz     @f
    rdtsc
    xor     eax, edx
    mov     ebx, eax

 @@:
    xor     edx, edx
    mov     ecx, 127773
    div     ecx
    mov     ecx, eax
    mov     eax, 16807
    mul     edx
    mov     edx, ecx
    mov     ecx, eax
    mov     eax, 2836
    mul     edx
    sub     ecx, eax
    xor     edx, edx
    mov     eax, ecx
    mov     ebx, ecx
    mov     ecx, 100000
    div     ecx
    mov     eax, edx

    xor     edx, edx
    mov     ebx, [max_value]
    div     ebx
    mov     eax, edx

    ret
 endp

;==============================================================================
; Fill background with checkers 8x8
;==============================================================================

checker_buf rb 16*16*3

fill_background_checkers:

    mov     edi, checker_buf
    xor     ebx, ebx              ; y

.yloop:
    xor     esi, esi              ; x

.xloop:

    mov     eax, esi
    shr     eax, 3                ; x / 8

    mov     edx, ebx
    shr     edx, 3                ; y / 8

    xor     eax, edx
    test    eax, 1
    jnz     .col2

.col1:
    mov     eax, [color]
    jmp     .store

.col2:
    mov     eax, [color2]

.store:
    mov     [edi+0], al
    shr     eax, 8
    mov     [edi+1], al
    shr     eax, 8
    mov     [edi+2], al

    add     edi, 3

    inc     esi
    cmp     esi, 16
    jb      .xloop

    inc     ebx
    cmp     ebx, 16
    jb      .yloop

    mcall   SF_BACKGROUND_SET,SSF_SIZE_BG,16,16
    mcall   SF_BACKGROUND_SET,SSF_MODE_BG,1
    mcall   SF_BACKGROUND_SET,SSF_IMAGE_BG,checker_buf,0,16*16*3
    mcall   SF_BACKGROUND_SET,SSF_REDRAW_BG

    stdcall save_eskin_ini, 'C '
    ret

;==============================================================================
; Fill background with silk
;==============================================================================

fill_background_silk:
; 1. Выделяем память под тайл 256х256 (3 байта на пиксель)
    mcall   SF_SYS_MISC, SSF_HEAP_INIT
    mcall   SF_SYS_MISC, SSF_MEM_ALLOC, 256 * 256 * 3
    mov     [bgimg_buf], eax
    test    eax, eax
    jnz     .ok
    ret
  .ok:

    mov     edi, eax            ; EDI - указатель на буфер для записи BGR
    xor     ecx, ecx            ; ECX = Y (0..255)

.loop_y:
    xor     ebx, ebx            ; EBX = X (0..255)

.loop_x:
    ; --- ГЕНЕРАЦИЯ БЕСШОВНОГО УЗОРА ---
    
    ; Компонента 1: Волна по диагонали (X + Y)
    mov     eax, ebx
    add     eax, ecx
    and     eax, 255            ; Зацикливание по размеру тайла
    test    eax, 128            ; Превращаем пилу в треугольное зеркало (0..127)
    jz      @f
    xor     eax, 127
  @@:
    mov     edx, eax            ; Сохраняем первую волну в EDX

    ; Компонента 2: Волна по обратной диагонали (X - Y)
    mov     eax, ebx
    sub     eax, ecx
    and     eax, 255            ; Зацикливание
    test    eax, 128            ; Зеркало (0..127)
    jz      @f
    xor     eax, 127
  @@:
    add     edx, eax            ; Складываем обе волны. Получаем EDX в диапазоне 0..254

    ; --- ЧЕСТНОЕ СМЕШИВАНИЕ ЦВЕТОВ (EDX = коэффициент от 0 до 254) ---

    ; Смешиваем Синий (B)
    movzx   eax, byte [color2 + 0]
    movzx   esi, byte [color + 0]
    sub     eax, esi            ; (B2 - B1)
    imul    eax, edx            ; (B2 - B1) * factor
    sar     eax, 8              ; /256
    add     eax, esi            ; Итоговый B
    mov     [edi], al           ; Пишем в буфер

    ; Смешиваем Зеленый (G)
    movzx   eax, byte [color2 + 1]
    movzx   esi, byte [color + 1]
    sub     eax, esi            ; (G2 - G1)
    imul    eax, edx
    sar     eax, 8
    add     eax, esi
    mov     [edi+1], al         ; Пишем в буфер

    ; Смешиваем Красный (R)
    movzx   eax, byte [color2 + 2]
    movzx   esi, byte [color + 2]
    sub     eax, esi            ; (R2 - R1)
    imul    eax, edx
    sar     eax, 8
    add     eax, esi
    mov     [edi+2], al         ; Пишем в буфер

    add     edi, 3              ; Переходим к следующему пикселю

    inc     ebx
    cmp     ebx, 256
    jl      .loop_x

    inc     ecx
    cmp     ecx, 256
    jl      .loop_y

    ; 2. Выводим результат на фон Колибри ОС в режиме ТАЙЛОВ
    mcall   SF_BACKGROUND_SET, SSF_SIZE_BG, 256, 256
    mcall   SF_BACKGROUND_SET, SSF_MODE_BG, 1             ; Режим 1: Замостить (Tile)
    mcall   SF_BACKGROUND_SET, SSF_IMAGE_BG, [bgimg_buf], 0, 256 * 256 * 3
    mcall   SF_BACKGROUND_SET, SSF_REDRAW_BG

    ; Освобождаем память
    mcall   SF_SYS_MISC, SSF_MEM_FREE, [bgimg_buf]
    
    stdcall save_eskin_ini, 'S '                          ; Флаг 'S' для конфига
ret
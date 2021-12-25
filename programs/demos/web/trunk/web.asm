;"Web" demo for KolibriOS, version 0.3
;Copyright Alexander Meshcheryakov (Self-Perfection), 2009
;Contact me: alexander.s.m@gmail.com
;distributed under BSD license

;Used assumptions:
;   1) Screen resolution does not change while app is running
;   2) Screen width bigger than height
;   3) Screen width and height are even (2*k)

use32
    org 0
    db  'MENUET01'
    dd  0x01,__start,__end,__memory,__stack,param,0

include '../../../macros.inc'

background_cl = 0x000000
foreground_cl = 0xFFFFFF

delay = 4

; debug = 1

;KOS_APP_START

CODE
	;Check if the app is running in screensaver mode or not
	cmp dword [param], '@ss'
	setz [screensaver]
    mov     ebx, EVM_REDRAW + EVM_KEY + EVM_BUTTON
    cmovz   ebx, EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE
    mcall   40
	
	;Make cursor transparent
    mov     edi, transparent_cursor
    xor     eax, eax
    mov     ecx, 32*32
    rep     stosd
    mcall   37, 4, transparent_cursor, 2
    mov     ecx, eax
    mcall   37, 5
	
    ;Preinit. Randomize start counter
    mcall 3
    mov     [initial_counter], eax          ;init with system time

    ;Query screen size
    mcall   14
    add     eax, 0x00010001
    mov     dword [y_max], eax      ;store x_max and y_max
    shr     eax, 1
    mov     dword [radius], eax     ;store radius and x_center
    
    ;Calc line_number
    mov     ax, [y_max]
    mov     dx, 0
    mov     bx, 5
    div     bx
    mov     word [half_line_number], ax
    movzx   edx, ax         ;edx = half_line_number
    imul    edx, 2 * line_coords_element_size      ;Space needed for line_coords_array

    ;Demand memory
    mcall   68, 11      ; Init heap
    test    eax, eax    ; Is heap successfully inited?
    jnz     @f
    jmp     exit          ;   Netu pamjati?! Nu i nahuj vas
@@:
    movzx   ecx, [y_max]
    inc     ecx
    movzx   eax, [x_max]
    imul    ecx, eax        ;[Remember to left here space for 1-2 emergency lines at bottom]
    add     ecx, edx        ;And add space for line_coords_array
    mcall   68, 12,
    test    eax, eax    ; Did we get something non zero?
    jnz     @f
    jmp     exit
@@:
    mov     [line_coords_array_pointer], eax
    add     eax, edx
    mov     [image_pointer], eax


    call    clear_offscreen_bitmap



;Calc fixed line ends coords
    fninit
    fldpi
    fidiv word [half_line_number]    ;Now st0 contains angle step of line start points
    
    mov eax, [line_coords_array_pointer]          ;cleanup: comment
    movzx   ecx, word [half_line_number]
    shl     ecx, 1
    fld     st      ;skip zero angle to avoid 1px fixed line at right side


calculate_next_line_start_point:
    fld st

    ;Calculate line start points coords
    fsincos
    fimul [radius]
    fiadd [x_center]
    fistp word [eax+start_x_offset]
;     fchs                              ;affects direction, uncomment with corresponding line below
    fimul [radius]
    fiadd [radius]
    fistp word [eax+start_y_offset]

    ;Calculate line start point pointer
    movzx   ebx, word [eax+start_y_offset]
    movzx   edx, word [x_max]
    imul    ebx, edx
    movzx   edx, word [eax+start_x_offset]
    add     ebx, edx
    add     ebx, [image_pointer]

    mov     [eax+line_start_pointer_offset], ebx


    fadd st0, st1
    add eax, line_coords_element_size    ;Move to next element in line_coords_array

    loop calculate_next_line_start_point

    fstp    st0          ;drop current angle
    fidiv   [divider]   ;change angle step


draw_window:

;Start line coords calculation

    fld     st      ;skip zero angle to avoid 1px fixed line at right side

    ;Use time since start to get current counter value
    mcall   26, 9
    add     eax, [initial_counter]
    mov     [current_counter], eax

    mov eax, [line_coords_array_pointer]          ;cleanup: comment
    movzx   ecx, word [half_line_number]
    shl     ecx, 1

calculate_next_line_end_point:
    fld st

    ;Calculate line end points
    fimul [current_counter]
    fsincos
    fimul [radius]
    fiadd [x_center]
    fistp word [eax+2]
;     fchs                              ;affects direction, uncomment with corresponding line above
    fimul [radius]
    fiadd [radius]
    fistp word [eax+6]

    fadd st0, st1

    add eax, line_coords_element_size    ;Move to next element in line_coords_array
    loop calculate_next_line_end_point

    fstp    st0     ;drop current angle

    inc dword [initial_counter]


;   *********************************************
;   *******Draw lines on offscreen bitmap********
;   *********************************************

    ;draw end points
    movzx   edi, [half_line_number]
    shl     edi, 1
    mov esi, [line_coords_array_pointer]

  draw_next_line:

    if defined debug    ;Draw red points next to line ends in debug mode
        movzx   ebx, word [esi+start_y_offset]
        movzx   eax, word [x_max]
        imul    eax, ebx
        movzx   ebx, word [esi+start_x_offset]
        add     eax, ebx
        add     eax, dword [image_pointer]
        inc     eax
        mov     [eax], byte red_cl_index
        
        movzx   ebx, word [esi+end_y_offset]
        movzx   eax, word [x_max]
        imul    eax, ebx
        movzx   ebx, word [esi+end_x_offset]
        add     eax, ebx
        add     eax, dword [image_pointer]
        inc     eax                             ;Move one right to make more visible
        mov     [eax], byte red_cl_index
    end if


;Drawing lines. Need to keep esi and edi values in process

    mov     eax, [esi+line_start_pointer_offset]

  check_horizontal_line:

    mov     bx, word [esi+start_y_offset]
    cmp     bx, word [esi+end_y_offset]
    jnz     general_draw_line      ;Jump to next test if dy!=0
    
    pusha
    
    movzx   ecx, word [esi+end_x_offset]
    sub     cx, word [esi+start_x_offset]

    jnc     @f
    neg     cx
    sub     eax, ecx
  @@:

    cld
    inc     cx

    mov     edi, eax
    mov     al, foreground_cl_index
    rep stos byte [edi]

    popa

    jmp     line_drawing_end



    ;General line draw algorithm. Based on Bresenham's algorithm (below) but heavily optimized
;  function line(x0, x1, y0, y1)
;      boolean steep := abs(y1 - y0) > abs(x1 - x0)
;      if steep then
;          swap(x0, y0)
;          swap(x1, y1)
;      if x0 > x1 then
;          swap(x0, x1)
;          swap(y0, y1)
;      int deltax := x1 - x0
;      int deltay := abs(y1 - y0)
;      int error := deltax / 2
;      int ystep
;      int y := y0
;      if y0 < y1 then ystep := 1 else ystep := -1
;      for x from x0 to x1
;          if steep then plot(y,x) else plot(x,y)
;          error := error - deltay
;          if error < 0 then
;              y := y + ystep
;              error := error + deltax



general_draw_line:
    pusha

    ;init step_base and step_secondary
    mov     edx, esi
    mov     esi, 1
    movzx   edi, word [x_max]

    ;calc initial delta_base & delta_secondary values
    movzx   ebx, word [edx+end_x_offset]
    sub     bx, [edx+start_x_offset]
    jnc     @f
    neg     bx
    neg     esi
  @@:
    movzx   ecx, word [edx+end_y_offset]
    sub     cx, [edx+start_y_offset]
    jnc     @f
    neg     cx
    neg     edi
  @@:
    
    ;compare abs(y1 - y0) and abs(x1 - x0)
    cmp     bx, cx
    jnc     @f
    xchg    ebx, ecx
    xchg    esi, edi
  @@:


    shl     ebx, 16
    mov     bx, cx
    rol     ebx, 16
    mov     cx, bx      ;init counter
    inc     cx
    mov     dx, bx      ;init error
    shr     dx, 1
    rol     ebx, 16


;At current point:
;eax = current point pointer
;ebx = (delta_base shl 16) + delta_secondary
;ecx = counter
;[e]dx = error
;esi = step_base
;edi = step_secondary


  brasenham_plot_point:
    mov     byte [eax], foreground_cl_index
    add     eax, esi

    sub     dx, bx

    jnc     end_loop

;              y := y + ystep
;              error := error + deltax
  change_y:
    add     eax, edi
    rol     ebx, 16
    add     dx, bx
    rol     ebx, 16

  end_loop:
    loopw   brasenham_plot_point

    end_bresenham:

    popa

line_drawing_end:

    add esi, line_coords_element_size   ;Move to next element in line_coords_array
    dec edi
    jnz draw_next_line


;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************
;     mcall 18, 14    ;Wait for scanning (it seems doesn't do any good now)

    ; start redraw  (without calling 12 proc our window overwrites window that above it)
    mcall   12, 1
    xor eax,eax
    movzx ebx, [x_max]
    movzx ecx, [y_max]
    mov edx, 0x01000000     ;Window style       ;Draw nothing
;     mov edx, 0x00000000     ;Window style 
;     mov esi, 0x00000000     ;Header color (prevent odd color line on top of window in random cases)
    mcall           ;Define window


    mov ebp, 0
    mov ecx, dword [y_max]
    mcall 65, [image_pointer], , <0,0>, 8, palette


    mcall   12, 2       ; end redraw


    call    clear_offscreen_bitmap

wait_event:
    mcall 23, delay
;     mcall 10
    test eax, 0xFFFF - 1    ;Test for 0 (delay passed) or 1 (redraw) event
    jz  draw_window     ; Delay passed or redraw event
    dec eax
    dec eax
    jnz  exit           ; If not key then Alt+F4
; key pressed, read it and ignore
    mcall   2
	cmp	ah, 27		    ; Test Esc key press in ASCII
	jne	wait_event

; button pressed; we have only one button, close
; also seems to handle Alt+F4
exit:
    cmp     [screensaver], 0
    jz      @f
    mcall   70, f70
@@:
    mcall   -1


clear_offscreen_bitmap:
    mov edi, [image_pointer]
    movzx   ecx, [y_max]
    movzx   eax, [x_max]
    imul    ecx, eax
    shr     ecx, 2      ;dword is 4 bytes
    mov eax, 0
    rep stos dword [edi]
    ret


DATA
divider             dw 5000

palette:
    background_cl_index = 0
    dd      background_cl
    foreground_cl_index = 1
    dd      foreground_cl

    if defined debug
        red_cl_index = 2
        dd      0x00FF0000          ;Cleanup this!
    end if

UDATA
image_pointer       dd ?

initial_counter     dd ?
current_counter     dd ?        ;counter + current time

half_line_number    dw ?

y_max            dw ?     ; screen size
x_max            dw ?

radius          dw ?
x_center        dw ?

line_coords_array_pointer       dd ?

; line_coords_array:
;     repeat 1000 ;line_number
;         dw      ?       ;start_x
;         dw      ?       ;end_x
;         dw      ?       ;start_y
;         dw      ?       ;end_y
;         dd      ?       ;line_start_pointer
;     end repeat

    start_x_offset = 0
    end_x_offset = 2
    start_y_offset = 4
    end_y_offset = 6
    line_start_pointer_offset = 8
    line_coords_element_size = 12

__params:
param rb 40

f70: ; run
    dd 7, 0, 0, 0, 0
    db '/sys/@SS',0

screensaver db ?
transparent_cursor rd 32*32

MEOS_APP_END
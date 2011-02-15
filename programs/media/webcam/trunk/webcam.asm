;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   64 bit Menuet Webcam
;
;   Compile with FASM 1.60 or above
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

data1  equ  image_end+0x100000*1   ; Timestamp data
data3  equ  image_end+0x100000*2   ; Image data
data4  equ  image_end+0x100000*10  ; Image data time order
data5  equ  image_end+0x100000*11  ; Length data time order
data6  equ  image_end+0x100000*12  ; Decoded image
data8  equ  0x100000*16            ; 7M*4 - Data return, 4k aligned
data9  equ  0x100000*52            ; 1M*4 - Data length return
data10 equ  0x100000*56            ; Save snap / avi - size 16M

scx    equ  3                      ; Scroll x pos
rby    equ  (220 shl 32+21)        ; Ydim
b3y    equ  0                      ; B3y add 
bys    equ  21                     ; Buttons y add
scl    equ  0                      ; Scroll length

use32

    org   0x0

    db    'MENUET64'          ; Header identifier
    dq    0x01                ; Version
    dq    START               ; Start of code
    dq    image_end           ; Size of image
    dq    data10+0x100000*16  ; Memory for app
    dq    image_end           ; Stack
    dq    0x00                ; Prm 
    dq    0x00                ; Icon


turn_camera_on:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   Reserves, configures and turns camera on
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    cmp   [camera_state],byte 1
    je    camera_on

    ; Camera present ?

    mov   rax , 128
    mov   rbx , 1
    int   0x60
    add   rax , rbx
    cmp   rax , 0
    je    camera_fail

    ; Configuration found ?

    cmp   [sconf],dword 0
    je    camera_fail

    ; Set configuration

    mov   rax , 128
    mov   rbx , 3
    mov   rcx , 1
    mov   rdx , [sconf]
    int   0x60

    ; Start camera

    mov   rax , 128
    mov   rbx , 4
    mov   rcx , 1
    mov   rdx , 1
    int   0x60

    mov   [camera_state],byte 1

  camera_fail:
  camera_on:

    ret


turn_camera_off:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   Turns camera off and clears the reservation
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    cmp   [camera_state],byte 0
    je    camera_off

    ; Camera present ?

    mov   rax , 128
    mov   rbx , 1
    int   0x60
    add   rax , rbx
    cmp   rax , 0
    je    camera_off

    ; Stop camera

    mov   rax , 128
    mov   rbx , 4
    mov   rcx , 1
    mov   rdx , 0
    int   0x60

    ; Set configuration 0

    mov   rax , 128
    mov   rbx , 3
    mov   rcx , 1
    mov   rdx , 0
    int   0x60

  camera_off:

    mov   [camera_state],byte 0

    ret



server:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   Server thread for webcam picture - port 8090
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    mov   [serveronoff],byte 1

    mov   [datasent],dword 0

  server_open:

    ; Open port

    mov   rax , 53
    mov   rbx , 5
    mov   rcx , 8090
    mov   rdx , 0
    mov   rsi , 0
    mov   rdi , 0 ; Passive mode
    int   0x60

    mov   [server_socket],eax

    mov   r15 , 100*30

  server_loop:

    ; Close server port and re-open every 30 seconds

    dec   r15
    jz    timeoutclose

    ;

    mov   rax , 5
    mov   rbx , 1
    int   0x60

    cmp   [serveronoff],byte 2
    je    serveroff

    ; Server status ?

    mov   rax , 53
    mov   rbx , 6
    mov   rcx , [server_socket]
    int   0x60

    cmp   rax , 4
    jne   server_loop

    ; Wait 0.50 seconds

    mov   rax , 5
    mov   rbx , 50
    int   0x60

    mov   [picsend],byte 0

    ; Read all data

  read_server_data:

    mov   rax , 53
    mov   rbx , 3
    mov   rcx , [server_socket]
    int   0x60

    shl   r8  , 8
    mov   r8b , bl

    mov   r9  , 'pmb.egam'
    cmp   r8  , r9
    jne   nopicsend2
    mov   [picsend],byte 1
  nopicsend2:

    cmp   rax , 0
    jne   read_server_data

    ; Wait 0.05 seconds

    mov   rax , 5
    mov   rbx , 5
    int   0x60

    ; Send index htm file

    cmp   [picsend],byte 0
    jne   noindexsend

    mov   rax , 53
    mov   rbx , 7
    mov   rcx , [server_socket]
    mov   rdx , h_i_len-html_header_index
    mov   rsi , html_header_index
    int   0x60

    jmp   nopicsend

  noindexsend:

    ; form picture to data10+

    mov   rax , [resx]
    mov   [sizex],rax
    mov   rax , [resy]
    mov   [sizey],rax
    call  form_picture

    ; send picture - html header

    mov   rax , '00230454' ; 320
    cmp   [resx],dword 160
    jne   nor160
    mov   rax , '00057654' ; 160
  nor160:
    cmp   [resx],dword 640
    jne   nor640
    mov   rax , '00921654' ; 640
  nor640:
    mov   [hsz],rax 

    mov   rax , 53
    mov   rbx , 7
    mov   rcx , [server_socket]
    mov   rdx , h_len-html_header
    mov   rsi , html_header
    int   0x60

    mov   rax , 105
    mov   rbx , 100
    int   0x60

    ; send picture - bmp header

    mov   rax , 53
    mov   rbx , 7
    mov   rcx , [server_socket]
    mov   rdx , 54
    mov   rsi , data10
    int   0x60

    ; send picture - bmp data

    mov   rsi , data10+54

    mov   r15 , 99999
    call  server_stats
    mov   r15 , 0

  newdatasend:

    mov   rax , 53
    mov   rbx , 6
    mov   rcx , [server_socket]
    int   0x60
    cmp   rax , 4
    jne   timeoutclose

    mov   rax , 53
    mov   rbx , 7
    mov   rcx , [server_socket]
    mov   rdx , 160*3
    int   0x60

    mov   rax , 105
    mov   rbx , 3
    int   0x60

    ; If camera is off, display the stats

    call  server_stats ; In:r15

    add   rsi , 160*3

    mov   [datasent],rsi
    sub   [datasent],dword data10+54

    mov   rbx , [resx_resy_3]
    add   rbx , data10+54 

    cmp   rsi , rbx
    jb    newdatasend

  timeoutclose:
  nopicsend:

    mov   rax , 5
    mov   rbx , 5
    int   0x60

    ; close port

    mov   rax , 53
    mov   rbx , 8
    mov   rcx , [server_socket]
    int   0x60

    mov   [datasent],dword 0

    mov   r15 , 99999
    call  server_stats

 ;   mov   rax , 5
 ;   mov   rbx , 50
 ;   int   0x60

    jmp   server_open

  serveroff:

    ; Close port

    mov   rax , 53
    mov   rbx , 8
    mov   rcx , [server_socket]
    int   0x60

    mov   [serveronoff],byte 3

    mov   rax , 512
    int   0x60


server_stats:
;
; In: r15 - counter
;
    cmp   [camera_state],byte 1
    je    nodfps
    inc   r15
    cmp   r15 , 75
    jb    nodfps
    mov   [show_fps],byte 1
    mov   r15 , 0
  nodfps:

    ret



START:

    mov   rax , 141
    mov   rbx , 1
    mov   rcx , 1
    mov   rdx , 5 shl 32 + 5
    mov   r8  , 9 shl 32 + 12
    int   0x60

    ; 320x240 as default

    mov   r12, 010b
    call  scan_for_configuration
    mov   [sconf],r12

    call  grey_default

    ; Draw window

    call  draw_window    

still:

    mov   rax , 123          ; Check for event
    mov   rbx , 1
    int   0x60

    test  rax , 0x1         ; Window redraw
    jnz   window_event
    test  rax , 0x2         ; Keyboard press
    jnz   key_event
    test  rax , 0x4         ; Button press
    jnz   button_event

    call  check_camera_state

    call  read_data

    cmp   [show_fps],byte 1
    jne   nosfps
    call  display_frames_per_second
    mov   [show_fps],byte 0
  nosfps:

    jmp   still


window_event:

    call  draw_window
    jmp   still

key_event:

    mov   rax , 0x2        ; Read the key and ignore
    int   0x60

    jmp   still

button_event:

    mov   rax , 0x11
    int   0x60

    ; rax = status
    ; rbx = button id

    cmp   rbx , 0x10000001
    jne   no_application_terminate_button
    call  turn_camera_off
    mov   rax , 0x200
    int   0x60
  no_application_terminate_button:

    cmp   rbx , 0x102
    jb    no_stats
    cmp   rbx , 0x103
    ja    no_stats
    sub   rbx , 0x102
    mov   [sta1+1],byte ' '
    mov   [sta2+1],byte ' '
    imul  rbx , 14
    mov   [sta1+1+rbx],byte '>'
    call  fps_background
    call  display_frames_per_second
    jmp   still
  no_stats:

    cmp   rbx , 0x109
    jne   no_application_terminate_menu
    call  turn_camera_off
    mov   rax , 0x200
    int   0x60
  no_application_terminate_menu:

    cmp   rbx , 0x105
    jb    no_effect
    cmp   rbx , 0x107
    ja    no_effect
    sub   rbx , 0x105
    mov   [eo+1],byte ' '
    mov   [eb+1],byte ' '
    mov   [en+1],byte ' '
    imul  ebx , 12
    mov   [eo+1+rbx],byte '>'
    jmp   still
  no_effect:

    cmp   rbx , 121
    jb    nomodechange
    cmp   rbx , 123
    ja    nomodechange

    ; Turn camera off

    push  rbx
    call  turn_camera_off
    pop   rbx

    ; Turn server off

    push  rax rbx
    cmp   [serveronoff],byte 1
    jne   noserveron2
    mov   [serveronoff],byte 2
    mov   rax , 5
    mov   rbx , 20
    int   0x60
    call  server_button
  noserveron2:
    pop   rbx rax

    ;

    cmp   rbx , 123 ; 640 & 800
    jne   screenresxfine
    call  get_data_boot_info
    cmp   [data_boot_info+4*8],dword 800
    jae   screenresxfine
    mov   rax , 4
    mov   rbx , string_screen_req
    mov   rcx , 15+5
    mov   rdx , 49+5
    mov   rsi , 0xffffff
    mov   r9  , 1
    int   0x60
    mov   rax , 5
    mov   rbx , 200
    int   0x60
    call  display_image_yuv
    jmp   still
  screenresxfine:

    mov   r11 , [xpos]

    sub   rbx , 121

    mov   r8 , [xr+rbx*8]
    mov   r9 , [yr+rbx*8]
    mov   r10, [xp+rbx*8]

    mov   r12, [cb+rbx*8]
    call  scan_for_configuration

    cmp   r8 , [resx] ; already selected
    je    still

    mov   [resx],r8
    mov   [resy],r9
    mov   [xpos],r10
    mov   [sconf],r12

    imul  r8 , r9
    mov   r9 , r8
    add   r8 , r9
    mov   [resx_resy_2],r8
    add   r8 , r9
    mov   [resx_resy_3],r8
    add   r8 , r9
    mov   [resx_resy_4],r8

    cmp   r11 , [xpos] ; redraw needed
    jne   redraw
    mov   rax , 13
    mov   rbx , 15 shl 32 + 320
    mov   rcx , 49 shl 32 + 240
    mov   rdx , 0xffffff
    int   0x60
    call  display_image_yuv
    mov   [camera_connection_status],dword 2
    call  check_camera_state
    jmp   still
  redraw:

    mov   rax , 67
    mov   rbx , -1
    mov   rcx , -1

    mov   rdx , [resx]
    cmp   rdx , 320
    jae   rdxfine
    mov   rdx , 320
  rdxfine:
    mov   r8  , [resy]
    cmp   r8  , 240
    jae   r8fine
    mov   r8  , 240
  r8fine:
    add   rdx , 160
    add   r8  , 70

    cmp   r11 , [xpos]
    jae   noupleft
    mov   rbx , 0
    mov   rcx , 0
    mov   r8  , 561 ; 640x480 picture
  noupleft:

    int   0x60

    jmp   still

  scan_for_configuration:

    mov   rdx , 1

  newconftest:

    mov   rax , 128
    mov   rbx , 2
    mov   rcx , 1
    int   0x60
    shr   rbx , 16
    and   rbx , 0xff
    cmp   rbx , r12
    je    cfound

    inc   rdx
    cmp   rdx , 3
    jbe   newconftest

    mov   r12 , 0 ; conf not found
    ret

  cfound:

    mov   r12 , rdx ; conf found
    ret

  nomodechange:

    cmp   rbx , 101
    jne   no_on
    call  turn_camera_on
    jmp   still
  no_on:

    cmp   rbx , 102
    jne   no_off
    mov   [record_state],byte 0
    cmp   [serveronoff],byte 1
    jne   noserveron
    mov   [serveronoff],byte 2
    mov   rax , 5
    mov   rbx , 20
    int   0x60
  noserveron:
    call  turn_camera_off
    call  display_image_yuv
    call  server_button
    jmp   still
  no_off:

    cmp   rbx , 103
    jne   no_snap
    call  save_snap
    jmp   still
  no_snap:

    cmp   rbx , 104
    jne   no_clip
    cmp   [camera_state], byte 1
    jne   still
    cmp   [resx],dword 320
    je    resolutionfine
    mov   rax , 4
    mov   rbx , string_resolution_320
    mov   rcx , 15+5
    mov   rdx , 49+5
    mov   rsi , 0xffffff
    mov   r9  , 1
    int   0x60
    mov   rax , 5
    mov   rbx , 100
    int   0x60
    call  display_image_yuv
    jmp   still
  resolutionfine:
    mov   [record_state],byte 1
    mov   [record_position], dword data10+1208
    mov   rax , data10+(avi_end-avi_start)
    mov   [record_position],rax
    jmp   still
  no_clip:

    cmp   rbx , 105
    jne   no_server
    cmp   [serveronoff],byte 3
    je    startserver
    mov   [serveronoff],byte 2
  waitmore:
    mov   rax , 5
    mov   rbx , 10
    int   0x60
    call  read_data
    cmp   [serveronoff],byte 2
    je    waitmore
    call  server_button
    jmp   still
  startserver:
    ; Start server thread
    mov   rax , 51
    mov   rbx , 1
    mov   rcx , server
    mov   rdx , server_stack
    int   0x60
    mov   rax , 5
    mov   rbx , 10
    int   0x60
    call  server_button
    jmp   still
  no_server:


    cmp   rbx , 10000
    jb    noscroll1
    cmp   rbx , 11000
    ja    noscroll1
    mov   [scroll_value_1],rbx
    call  draw_scroll_1
    call  draw_scroll_values
    jmp   still
  noscroll1:

    cmp   rbx , 20000
    jb    noscroll2
    cmp   rbx , 21000
    ja    noscroll2
    mov   [scroll_value_2],rbx
    call  draw_scroll_2
    call  draw_scroll_values
    jmp   still
  noscroll2:

    cmp   rbx , 30000
    jb    noscroll3
    cmp   rbx , 31000
    ja    noscroll3
    mov   [scroll_value_3],rbx
    call  draw_scroll_3
    call  draw_scroll_values
    jmp   still
  noscroll3:

    jmp   still



grey_default:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   Grey default picture
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    mov   rdi , data6
    mov   rcx , [resx]
    imul  rcx , [resy]
    imul  rcx , 4
    mov   rax , 0xe0
    cld
    rep   stosb

    ret


get_data_boot_info:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   Screen resolution
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    push  rax rbx rcx rdx
    mov   rax , 26
    mov   rbx , 3
    mov   rcx , data_boot_info
    mov   rdx , 8*20
    int   0x60
    pop   rdx rcx rbx rax

    ret



check_camera_state:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   Displays text message if camera is disconnected
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    mov   rax , 128
    mov   rbx , 1
    int   0x60
    add   rax , rbx

    cmp   rax , [camera_connection_status]
    je    no_camera_change

    mov   [camera_connection_status],rax

    push  rax
    call  grey_default
    call  display_image_yuv
    pop   rax

    cmp   rax , 0
    jne   camera_connected

    mov   rax , 4
    mov   rbx , string_camera_disconnected
    mov   rcx , 114
    mov   rdx , 162
    mov   rcx , [resx]
    shr   rcx , 1
    add   rcx , 15
    sub   rcx , 62
    mov   rdx , [resy]
    shr   rdx , 1
    add   rdx , 43
    mov   rsi , 0xb8b8b8
    mov   r9  , 1
    int   0x60

  camera_connected:

  no_camera_change:

    ret



fps_delay:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;  Waits for next time to read from device
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

  fps_wait:

    mov   rax , 11
    int   0x60
    cmp   rax , 0
    jne   fps_wait_event

    mov   rax , 105
    mov   rbx , 1
    int   0x60

    mov   rax , 26
    mov   rbx , 1
    mov   rcx , fps_data
    mov   rdx , 1024
    int   0x60

    call  check_framerate

    mov   rax , [fps_data+384]

    cmp   [thread_running],byte 0
    jne   yes_fps_wait
    mov   [fps_add],dword 34 ; assume 30fps
    jmp   no_fps_wait
  yes_fps_wait:

    cmp   rax , [fps_next]
    jb    fps_wait

  no_fps_wait:

    add   rax , [fps_add]
    mov   [fps_next],rax

  no_fps_add:

  fps_wait_event:

    ret



check_framerate:

    ; Display FPS and adjust display rate delay

    mov   rax , [fps_data+384]
    cmp   rax , [fps_n]
    jb    nofps
    add   rax , 1000
    mov   [fps_n],rax
    ;
    cmp   [frames_per_second],dword 5
    jae   nofrpsz
    mov   [frames_per_second],dword 5
  nofrpsz:
    cmp   [frames_per_second],dword 30
    jbe   nofrpst
    mov   [frames_per_second],dword 30
  nofrpst:
    call  display_frames_per_second
    mov   rax , 1000
    xor   rdx , rdx
    mov   rbx , [frames_per_second]
    div   rbx
    ;
    cmp   rax , [fps_add]
    jae   nofaster
    inc   dword [fastercount]
    cmp   [fastercount],dword 2
    jb    nofasterset
    mov   [frames_per_second],dword 30
    mov   rax , 1000/30
  nofaster:
    mov   [fastercount],dword 0
  nofasterset:
    ;
    mov   [fps_add],rax
    mov   [frames_per_second],dword 0
    mov   [frames_per_second_missed],dword 0
  nofps:

    ret



display_frames_per_second:

    cmp   [sta2+1],byte '>'
    jne   nodispstats

    fpsy  equ 24

    call  fps_background
    ;
    mov   rax , 47
    mov   rbx , 6 * 65536
    mov   rcx , [datasent]
    mov   rdx , [fpsx]
    add   rdx , 2+6*5
    shl   rdx , 32
    add   rdx , fpsy+4
    mov   rsi , 0xb0b0b0
    int   0x60
    ;
    mov   rax , 47
    mov   rbx , 2 * 65536
    mov   rcx , [frames_per_second]
    sub   rcx , [frames_per_second_missed]
    mov   rdx , [fpsx]
    add   rdx , 2+6*16
    shl   rdx , 32
    add   rdx , fpsy+4
    mov   rsi , 0xb0b0b0
    int   0x60
    mov   rax , 47
    mov   rbx , 2 * 65536
    mov   rcx , [frames_per_second]
    mov   rdx , (fpsx+2+19*6) shl 32 + fpsy+4
    mov   rdx , [fpsx]
    add   rdx , 2+19*6
    shl   rdx , 32
    add   rdx , fpsy+4
    mov   rsi , 0xb0b0b0
    int   0x60
    mov   rax , 0x4                      
    mov   rbx , string_fps
    mov   rcx , [fpsx]
    add   rcx , 2
    mov   rdx , fpsy+4
    mov   rsi , 0xb0b0b0
    mov   r9  , 1
    int   0x60

  nodispstats:

    ret

  fps_background:

    mov   rax , [xpos]
    sub   rax , 8
    mov   [fpsx],rax

    mov   rax , 13
    mov   rbx , [fpsx]
    sub   rbx , 3
    shl   rbx , 32
    add   rbx , 22*6+5
    mov   rcx , fpsy shl 32 + 14
    mov   rdx , 0xe8e8e8
    int   0x60

    ret




read_block:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   Starts the read thread
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    cmp   [thread_running],byte 0
    jne   nostartthread

    ; Start thread

    mov   [thread_running],byte 1

    mov   rax , 51
    mov   rbx , 1
    mov   rcx , read_thread
    mov   rdx , thread_stack
    int   0x60

    mov   rax , 5
    mov   rbx , 20
    int   0x60

  nostartthread:

    ret
 

read_thread:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   Read data block in a separate thread. 
;   Doesn't block main process.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

  newth:

    inc   dword [threadpos]
    and   dword [threadpos],dword 3

    mov   rax , 128
    mov   rbx , 5
    mov   rcx , 1

    mov   rdx , [threadpos]
    imul  rdx , 0x100000*7
    add   rdx , data8

    mov   r8 , [threadpos]
    imul  r8 , 0x100000
    add   r8 , data9

    mov   r9  , 256

    int   0x60  

    cmp   [camera_state],byte 1
    je    newth

    mov   [thread_running],byte 0

    mov   rax , 512
    int   0x60




read_data:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   Reads and analyzes data blocks from device 
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    ;
    ; Camera enabled ?
    ;

    cmp   [camera_state],byte 1
    je    continue_read
    ret
  continue_read:

    ;
    ; Start thread
    ;

    call  read_block

    mov   rcx , [threadposprev]
  waitforread:
    mov   rax , 105
    mov   rbx , 1
    int   0x60
    cmp   rcx , [threadpos]
    je    waitforread

    mov   rcx , [threadpos]
    mov   [threadposprev],rcx

    mov   rdx , [threadpos]
    dec   rdx
    and   rdx , 0x3
    imul  rdx , 0x100000*7
    add   rdx , data8

    mov   r8 , [threadpos]
    dec   r8
    and   r8 , 0x3

    imul  r8 , 0x100000
    add   r8 , data9

    mov   [pdata82],rdx  ; data   - 1024*3  4k aligned
    mov   [pdata92],r8   ; length - qword

    ;
    ; Analyze
    ;

    ;
    ; Arrange data to data1+ for faster cache access
    ;

    mov   rsi , [pdata82]
    mov   rdi , data1
  newarr:
    xor   rax , rax
    mov   al  , [rsi]
    xor   rbx , rbx
    mov   ebx , [rsi+rax-6]
    mov   [rdi],rbx
    add   rsi , 1024*3
    add   rdi , 8
    cmp   rdi , data1+8*256*8
    jbe   newarr

    ;
    ; Timestamp order
    ;

    mov   r12 , data4 ; data position
    mov   r10 , data5 ; length position

  newdataorder2:

    ; Search for lowest timestamp count

    mov   rax , 0xfffffffff  ; timestamp

    mov   r11 , data1
    mov   r15 , data1 + 8*256*8

  lowsearch2:

    ; Timestamp position

    cmp   [r11+7],byte 1 ; used ?
    je    nolowfound2
    cmp   eax , [r11]    ; smaller ?
    jb    nolowfound2

    mov   r14 , r11      ; save position
    mov   eax , [r11]    ; save packet num

  nolowfound2:

    add   r11 , 8

    cmp   r11 , r15
    jbe   lowsearch2

    mov   r13 , 0xfffffffff
    cmp   rax , r13
    jae   dataintimeorder2

    mov   rdx , r14
    sub   rdx , data1

    mov   rbx , rdx
    add   rbx , [pdata92]
    mov   [r10],rbx         ; save length position to line
    add   r10 , 8

    imul  rdx , 1024*3/8
    add   rdx , [pdata82]
    mov   [r12], rdx        ; save data position to line
    add   r12  , 8

    mov   [r14+7],byte 1

    jmp   newdataorder2

  dataintimeorder2:

  markdone:

    ; Mark end of list

    mov   [r12+00],dword 0xfffffff
    mov   [r12+08],dword 0xfffffff
    mov   [r12+16],dword 0xfffffff
    mov   [r12+24],dword 0xfffffff

    ;
    ; Display frames in sent data
    ;

    mov   r12 , data4
    call  display_frame_yuv
    call  record_picture

    mov   r15 , 0
  yesnewframe:
    push  r15
    mov   r12 , [framebegin]
    add   r12 , 8
    call  display_frame_yuv
    call  record_picture
    pop   r15
    inc   r15
    cmp   r15 , 50
    ja    nonewframes
    cmp   rsi , 0
    je    yesnewframe
  nonewframes:

    inc   dword [frames_per_second]
    inc   dword [frames_per_second_missed]

    ret



display_number:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   Displays scroll values 
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    push  rax rbx rcx rdx rsi

    push  rcx rdx
    mov   rax , 13
    mov   rbx , rdx
    sub   rbx , 2
    mov   rcx , rdx
    sub   rcx , 2
    mov   bx  , 6*3
    shl   rcx , 32
    add   rcx , 11
    mov   rdx , 0xffffff
    int   0x60
    pop   rdx rcx

    mov   rax , 47
    mov   rbx , 3 * 65536 + 0*256
    mov   rsi , 0x000000
    int   0x60

    pop   rsi rdx rcx rbx rax

    ret


get_frame_size:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   Counts the number of bytes in frame
; 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    mov   rax , 0

    mov   r12 , [framebegin]

    mov   rsi , [r12]

    cmp   rsi , 0xfffffff
    jae   cfrl2

    ; rsi = data area

    mov   rcx , [rsi+1]
    and   rcx , 1

    mov   r10 , r12
    add   r10 , data5
    sub   r10 , data4
    mov   r10 , [r10]

    ; r10 = data count area

    movzx rax , word [r10]
    and   rax , 0xfff

    movzx rbx , byte [rsi]
    and   rbx , 0x7f
    sub   rax , rbx

  cfrl1:

    add   r12 , 8

    mov   rsi , [r12]
    cmp   rsi , 0xfffffff
    jae   cfrl2

    ; rsi = data area

    mov   rdx , [rsi+1] 
    and   rdx , 1

    ; last block found -> exit

    cmp   rcx , rdx
    jne   cfrl2

    mov   r10 , r12
    add   r10 , data5
    sub   r10 , data4
    mov   r10 , [r10]

    ; r10 = data count area

    movzx rbx , word [r10]
    and   rbx , 0xfff

    movzx r8  , byte [rsi]
    and   r8  , 0x7f

    cmp   rbx , r8
    jbe   cfrl1

    add   rax , rbx
    sub   rax , r8

    jmp   cfrl1

  cfrl2:

    ret



separate_data:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   Separates picture data from received packets
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    mov   r14 , [framebegin]

    mov   rdi , data3

  testnewline:

    mov   r10 , [r14]       ; data content position

    mov   rsi , r14
    add   rsi , data5
    sub   rsi , data4
    mov   rsi , [rsi]       ; data length position

    cmp   [r14], dword 0xfffffff
    je    nothisdata
    cmp   r14 , data4 + 8*256*8
    jae   nothisdata

    mov   rcx , [rsi]
    and   rcx , 0xfff

    movzx rdx , byte [r10]
    and   rdx , 0x7f

    cmp   rcx , rdx
    jbe   nothisc

    mov   rsi , r10
    add   rsi , rdx

    sub   rcx , rdx
    and   rcx , 4096-1

    cld
    rep   movsb

    mov   rcx , [resx_resy_2]
    add   rcx , data3 

    cmp   rdi , rcx
    ja    nothisdata

  nothisc:

    add   r14 , 8

    jmp   testnewline

  nothisdata:

    ret



display_frame_yuv:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   Displays wanted frame at data from video device
; 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    ;
    ; Search for beginning of frame
    ;

    mov   [framebegin],dword data4

    jmp   frbl32

  newframesearch2:

    mov   rsi , [r12]

    cmp   rsi , 0xfffffff
    jae   frbl312

    mov   rcx , [rsi+1]
    and   rcx , 1

  frbl12:

    add   r12 , 8
    mov   rsi , [r12]

    cmp   rsi , 0xfffffff
    jae   frbl312

    mov   rdx , [rsi+1]
    and   rdx , 1

    cmp   rdx , rcx
    je    frbl12

  frbl32:

    mov   [framebegin],r12

    ;
    ; Check framesize
    ;

    call  get_frame_size
    cmp   rax , [resx_resy_2]
    je    framesizefine

    ; If framesize if not acceptable, search for next frame

    mov   r12 , [framebegin]
    add   r12 , 8
    jmp   newframesearch2

  framesizefine:

    jmp   displayframe

    ;

  frbl312:

    mov   rsi , 1 ; no more frames
    ret

  displayframe:

    mov   r12 , [framebegin]
    mov   [found_frame],r12

    ;

    call  separate_data

    ;
    ; Form picture YUV -> RGB
    ;

    mov   rsi , data3

    mov   rdi , data6
    mov   r15 , [resx_resy_4]
    add   r15 , data6

  npixell1:

    ; Effect: none

    cmp   [eo+1],byte '>'
    jne   noeffectoff
    movzx r8  , byte [rsi]
    movzx r9  , byte [rsi+1]
    movzx r10 , byte [rsi+3]
    call  calculate_yuv
    mov   [rdi+0],edx
    movzx r8  , byte [rsi+2]
    movzx r9  , byte [rsi+1]
    movzx r10 , byte [rsi+3]
    call  calculate_yuv
    mov   [rdi+4],edx
    jmp   pixelsdone
  noeffectoff:

    ; Effect: Blur

    cmp   [eb+1],byte '>'
    jne   noeffectblur
    movzx r8  , byte [rsi]
    movzx r9  , byte [rsi+1]
    movzx r10 , byte [rsi+3]
    call  calculate_yuv
    shr   edx , 1
    and   edx , 0x7f7f7f
    shr   dword [rdi],byte 1 
    and   dword [rdi],dword 0x7f7f7f
    add   [rdi],edx
    movzx r8  , byte [rsi+2]
    movzx r9  , byte [rsi+1]
    movzx r10 , byte [rsi+3]
    call  calculate_yuv
    shr   edx , 1
    and   edx , 0x7f7f7f
    shr   dword [rdi+4],byte 1 
    and   dword [rdi+4],dword 0x7f7f7f
    add   [rdi+4],edx
    jmp   pixelsdone
  noeffectblur:

    ; Effect: Negative

    cmp   [en+1],byte '>'
    jne   noeffectnegative
    movzx r8  , byte [rsi]
    movzx r9  , byte [rsi+1]
    movzx r10 , byte [rsi+3]
    call  calculate_yuv
    not   edx
    mov   [rdi+0],edx
    movzx r8  , byte [rsi+2]
    movzx r9  , byte [rsi+1]
    movzx r10 , byte [rsi+3]
    call  calculate_yuv
    not   edx
    mov   [rdi+4],edx
    jmp   pixelsdone
  noeffectnegative:

  pixelsdone:

    add   rdi , 8

    add   rsi , 4

    cmp   rdi , r15
    jbe   npixell1

    call  fps_delay

    call  display_image_yuv

  display_exit:

    inc   dword [frames_per_second]

    mov   rsi , 0
    ret




display_image_yuv:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   Displays image
; 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    mov   rax , 7
    mov   rbx , 15 shl 32 
    mov   rcx , (49) shl 32
    add   rbx , [resx]
    add   rcx , [resy]
    mov   rdx , data6
    mov   r8  , 0
    mov   r9  , 0x1000000
    mov   r10 , 4
    int   0x60

    ret


calculate_yuv:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   Calculates RGB from YUV
;
;   In:  r8,r9,r10 - YUV
;
;   Out: rdx - RGB
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    push  rbx rcx r11

    mov   rcx , r8
    sub   rcx , 16    ; ?!
    mov   rdx , r9
    sub   rdx , 128
    mov   r8  , r10
    sub   r8  , 128

    ; For all components

    imul  rcx , 298

    ;

    mov   r9  , 0     ; limit low
    mov   r10 , 255   ; limit high

    ; Red

    push  rcx r8

    imul  r8 , 409

    add   rcx , r8
    add   rcx , 128
    shr   rcx , 8

    imul  rcx , [scroll_value_1_multiply]
    shr   rcx , 7

    cmp    rcx , 0xfffffff
    cmovae rcx , r9
    cmp    rcx , r10
    cmova  rcx , r10

    mov   r11 , rcx

    pop   r8 rcx

    ; Green

    push  rcx rdx

    imul  rdx , 100
    imul  r8  , 208

    sub   rcx , rdx
    sub   rcx , r8
    add   rcx , 128
    shr   rcx , 8

    imul  rcx , [scroll_value_2_multiply]
    shr   rcx , 7

    cmp    rcx , 0xfffffff
    cmovae rcx , r9
    cmp    rcx , r10
    cmova  rcx , r10

    shl   r11 , 8
    mov   r11b , cl

    pop   rdx rcx

    ; Blue

    imul  rdx , 516

    add   rcx , rdx
    add   rcx , 128
    shr   rcx , 8

    imul  rcx , [scroll_value_3_multiply]
    shr   rcx , 7

    cmp    rcx , 0xfffffff
    cmovae rcx , r9
    cmp    rcx , r10
    cmova  rcx , r10

    shl   r11 , 8
    mov   r11b , cl

    mov   rdx , r11

    pop   r11 rcx rbx

    ret



acalculate_yuv:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   YUV to RGB
; 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    push  rax rbx rcx r8

    mov   rcx , r8
    sub   rcx , 16
    mov   rdx , r9
    sub   rdx , 128
    mov   r8  , r10
    sub   r8  , 128

    ; Red

    push  rcx rdx r8

    imul  rcx , 298
    imul  r8  , 409

    add   rcx , r8
    add   rcx , 128
    shr   rcx , 8

    imul  rcx , [scroll_value_1_multiply]
    shr   rcx , 7

    cmp   rcx , 0xfffffff
    jb    norcxzero1
    mov   rcx , 0
  norcxzero1:
    cmp   rcx , 255
    jbe   rcxfine
    mov   rcx , 255
  rcxfine:
    mov   [red],cl

    pop   r8 rdx rcx

    ; Green

    push  rcx rdx r8

    imul  rcx , 298
    imul  rdx , 100
    imul  r8  , 208

    sub   rcx , rdx
    sub   rcx , r8
    add   rcx , 128
    shr   rcx , 8

    imul  rcx , [scroll_value_2_multiply]
    shr   rcx , 7

    cmp   rcx , 0xfffffff
    jb    norcxzero2
    mov   rcx , 0
  norcxzero2:
    cmp   rcx , 255
    jbe   rcxfine2
    mov   rcx , 255
  rcxfine2:
    mov   [green],cl

    pop   r8 rdx rcx

    ; Blue

    push  rcx rdx r8

    imul  rcx , 298
    imul  rdx , 516
    add   rcx , rdx
    add   rcx , 128
    shr   rcx , 8

    imul  rcx , [scroll_value_3_multiply]
    shr   rcx , 7

    cmp   rcx , 0xfffffff
    jb    norcxzero3
    mov   rcx , 0
  norcxzero3:
    cmp   rcx , 255
    jbe   rcxfine3
    mov   rcx , 255
  rcxfine3:
    mov   [blue],cl

    pop   r8 rdx rcx

    ;

    mov   rdx , [red]
    shl   rdx , 8
    add   rdx , [green]
    shl   rdx , 8
    add   rdx , [blue]

    pop   r8 rcx rbx rax

    ret


form_picture:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   Forms BMP picture to data10+
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    ; Header

    mov   rsi , bmpheader
    mov   rdi , data10
    mov   rcx , 54
    cld
    rep   movsb

    mov   rax , [sizex]
    mov   [data10+0x12],eax

    mov   rbx , [sizey]
    mov   [data10+0x16],ebx

    ; Picture data

    mov   rdi , data10+54
    mov   rsi , [sizex]
    imul  rsi , [sizey]
    imul  rsi , 4
    add   rsi , data6
    mov   rcx , 0
  newpicdata:

    sub   rsi , [sizex]
    sub   rsi , [sizex]
    sub   rsi , [sizex]
    sub   rsi , [sizex]

    push  rcx rsi
    mov   rcx , [sizex]
  datamovel1:
    mov   eax , [rsi]
    mov   [rdi],eax
    add   rsi , 4
    add   rdi , 3
    loop  datamovel1
    pop   rsi rcx

    inc   rcx
    cmp   rcx , [sizey]
    jb    newpicdata

    ret


save_snap:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   Saves BMP snap
; 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    push  qword [camera_state]

    mov   rax , 5
    mov   rbx , 10
    int   0x60

    call  turn_camera_off

    ; Saving BMP text

    mov   rbx , textsaving2
    call  display_text

    mov   rax , [resx]
    mov   [sizex],rax
    mov   rax , [resy]
    mov   [sizey],rax

    ; Form picture to data10+

    call  form_picture

    ; Snap count

    mov   rax , [snapcount]
    xor   rdx , rdx
    mov   rbx , 10
    div   rbx
    mov   [filesave+12],dl
    mov   [filesave+11],al
    add   [filesave+11],word '00'

    ; Delete file if persent

    mov   rax , 58
    mov   rbx , 2
    mov   r9  , filesave
    int   0x60

    ; Save file

    mov   rax , 58
    mov   rbx , 1
    mov   rcx , 0
    mov   rdx , [sizex]
    imul  rdx , [sizey]
    imul  rdx , 3
    add   rdx , 54
    mov   r8  , data10
    mov   r9  , filesave
    int   0x60

    ; Increase snap counter

    inc   dword [snapcount]

    ; Draw button

    call  draw_snap_button

    ; Draw image

    call  display_image_yuv

    pop   rax
    cmp   rax , 1
    jne   noturnbackon
    mov   rax , 5
    mov   rbx , 10
    int   0x60
    call  turn_camera_on
  noturnbackon:

    ret


display_text:

    mov   rax , 0x4    
    mov   rcx , 20
    mov   rdx , 54
    mov   rsi , 0xffffff
    mov   r9  , 0x1
    int   0x60

    ret



record_picture:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   Records picture for AVI
; 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    cmp   [record_state],byte 1
    jne   recl2

    ; 10 fps

    mov   rbx , textrec
    call  display_text

    mov   rax , 26
    mov   rbx , 1
    mov   rcx , fps_data
    mov   rdx , 1024
    int   0x60
    mov   rax , [fps_data+384]
    cmp   rax , [record_next]
    jb    recl2
    add   rax , 100
    mov   [record_next],rax

    ; All recorded ?

    mov   rax , [resx_resy_4]
    add   rax , 4*2
    imul  rax , 50
    add   rax , data10+(avi_end-avi_start)

    cmp   [record_position], rax
    jb    recfine
    mov   [record_state],byte 0

    call  display_image_yuv

    ; Saving text

    mov   rbx , textsaving
    call  display_text

    call  save_clip

    call  display_image_yuv

    ret
  recfine:

    call  display_image_yuv

    mov   rax , [record_position]
    sub   rax , data10
    xor   rdx , rdx
    mov   rbx , [resx_resy_4]
    imul  rbx , 10
    div   rbx
    add   rax , 48+1
    mov   [textrec+5],al

    mov   rbx , textrec
    call  display_text

    mov   rdi  , [record_position]
    mov   [rdi], dword '00db'

    mov   rsi, [resx_resy_4]

    mov   [rdi+4], esi

    add   rdi , 8
    mov   rsi , data6
    add   rsi , [resx_resy_4]
    sub   rsi , [resx]
    sub   rsi , [resx]
    sub   rsi , [resx]
    sub   rsi , [resx]
  recl1:

    push  rsi rdi
    mov   rcx , [resx]
    shl   rcx , 2
    cld
    rep   movsb
    pop   rdi rsi

    add   rdi , [resx]
    add   rdi , [resx]
    add   rdi , [resx]
    add   rdi , [resx]

    sub   rsi , [resx]
    sub   rsi , [resx]
    sub   rsi , [resx]
    sub   rsi , [resx]

    cmp   rsi , data6
    jae   recl1

    mov   rsi , [resx_resy_4]
    add   rsi , 8

    add   [record_position],esi

  recl2:

    ret


save_clip:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   Saves AVI clip
; 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    push  qword [camera_state]

    mov   rax , 5
    mov   rbx , 10
    int   0x60

    call  turn_camera_off

    ; Header

    mov   rsi , avi_start
    mov   rdi , data10
    mov   rcx , avi_end-avi_start
    cld
    rep   movsb

    ; idx1

    mov   rsi , idx1_start
    mov   rdi , data10 + (avi_end-avi_start) + ((4*2+320*240*4)*50)
    mov   rcx , idx1_end-idx1_start
    cld
    rep   movsb

    ; Filecount

    mov   rax , [clipcount]
    xor   rdx , rdx
    mov   rbx , 10
    div   rbx
    mov   [filesaveavi+12],dl
    mov   [filesaveavi+11],al
    add   [filesaveavi+11],word '00'

    ; Delete file if persent

    mov   rax , 58
    mov   rbx , 2
    mov   r9  , filesaveavi
    int   0x60

    ; Save file

    mov   rax , 58
    mov   rbx , 1
    mov   rcx , 0
    mov   rdx , (avi_end-avi_start)+(4*2+320*240*4)*50+(idx1_end-idx1_start)
    mov   r8  , data10
    mov   r9  , filesaveavi
    int   0x60

    ; Increase snap counter

    inc   dword [clipcount]

    ; Draw button

    call  draw_clip_button

    pop   rax
    cmp   rax , 1
    jne   noturnbackonavi
    mov   rax , 5
    mov   rbx , 10
    int   0x60
    call  turn_camera_on
  noturnbackonavi:

    ret



draw_window:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   Draws window
; 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    mov   rax , 0xC     ; Beginning of window draw
    mov   rbx , 0x1
    int   0x60

    ; Draw window

    mov   rax , 0x0                         
    mov   rbx , 150 shl 32 + 480
    mov   rcx , 80 shl 32 + 310
    mov   rdx , 0x0000000000FFFFFF            
    mov   r8  , 0x0000000000000001          
    mov   r9  , window_label                
    mov   r10 , menu_struct                 
    int   0x60

    ; Start camera button

    mov   rax , 8
    mov   rbx , [xpos]
    shl   rbx , 32
    add   rbx , 116
    mov   rcx , (247+21-21) shl 32 + 21
    mov   rdx , 101
    mov   r8  , 0
    mov   r9  , string_start_camera
    int   0x60

    ; Stop camera button

    mov   rax , 8
    mov   rbx , [xpos]
    shl   rbx , 32
    add   rbx , 116
    mov   rcx , (247+21+21-21) shl 32 + 21
    mov   rdx , 102
    mov   r8  , 0
    mov   r9  , string_stop_camera
    int   0x60

    ; Server button

    call  server_button

    ; Snap picture button

    call  draw_snap_button

    ; Clip button

    call  draw_clip_button

    ; Resolution buttons

    call  resolution_buttons

    ; Display image

    call  display_image_yuv

    ; Display text

    mov   rax , 0x4                      
    mov   rbx , text
    mov   rcx , [xpos]
    add   rcx , 4+scx-6
    mov   rdx , 132
    mov   rsi , 0x0
    mov   r9  , 0x1
  newline:
    int   0x60
    add   rbx , 0x1F
    add   rdx , 13-2
    cmp   [rbx],byte 'x'
    jne   newline

    ; Scrolls

    call  draw_scroll_1
    call  draw_scroll_2
    call  draw_scroll_3
    call  draw_scroll_values

    ; Camera status

    mov   [camera_connection_status],dword 2
    call  check_camera_state

    ; FPS

    call  display_frames_per_second

    mov   rax , 0xC                      
    mov   rbx , 0x2
    int   0x60

    ret


server_button:

    ; Server button

    mov   rax , 8
    mov   rbx , [xpos]
    shl   rbx , 32
    add   rbx , 116
    mov   rcx , (173-20+4+b3y) shl 32 + bys
    mov   rdx , 105
    mov   r8  , 0
    mov   r9  , string_server_off
    cmp   [serveronoff],byte 3
    je    serverl1
    mov   r9  , string_server_on
  serverl1:
    int   0x60

    ret


resolution_buttons:

    ; Resolution buttons

    mov   rax , 8
    mov   rbx , [xpos] ; shl 32 + 116/3+1
    shl   rbx , 32
    add   rbx , 116/3+1
    mov   rcx , rby ; 236-20-2) shl 32 + bys
    mov   rdx , 121
    mov   r8  , 0
    mov   r9  , string_res_160
    int   0x60

    mov   rax , 8
    mov   rbx , [xpos]
    add   rbx , 116/3+1
    shl   rbx , 32
    add   rbx , 116/3
    mov   rcx , rby ; (236-20-2) shl 32 + bys
    mov   rdx , 122
    mov   r8  , 0
    mov   r9  , string_res_320
    int   0x60

    mov   rax , 8
    mov   rbx , [xpos]
    add   rbx , 116/3*2+1
    shl   rbx , 32
    add   rbx , 116/3+1
    mov   rcx , rby ; (236-20-2) shl 32 + bys
    mov   rdx , 123
    mov   r8  , 0
    mov   r9  , string_res_640
    int   0x60

    ret


draw_snap_button:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   Draws snap button
; 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    ; Snap count

    mov   rax , [snapcount]
    xor   rdx , rdx
    mov   rbx , 10
    div   rbx
    mov   [string_snap_save+12],dl
    mov   [string_snap_save+11],al
    add   [string_snap_save+11],word '00'

    ; Save snap button

    mov   rax , 8
    mov   rbx , [xpos]
    shl   rbx , 32
    add   rbx , 116
    mov   rcx , (194-18+2+b3y) shl 32 + bys
    mov   rdx , 103
    mov   r8  , 0
    mov   r9  , string_snap_save
    int   0x60

    ret


draw_clip_button:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   Draws video clip button
; 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    ; Clip count

    mov   rax , [clipcount]
    xor   rdx , rdx
    mov   rbx , 10
    div   rbx
    mov   [string_clip_save+12],dl
    mov   [string_clip_save+11],al
    add   [string_clip_save+11],word '00'

    ; Save clip button

    mov   rax , 8
    mov   rbx , [xpos]
    shl   rbx , 32
    add   rbx , 116
    mov   rcx , (215-16+b3y) shl 32 + bys
    mov   rdx , 104
    mov   r8  , 0
    mov   r9  , string_clip_save
    int   0x60

    ret


draw_scroll_values:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   Draws scroll RGB values
; 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    mov   rbx , [scroll_value_1]
    sub   rbx , 10000
    mov   rcx , 200
    sub   rcx , rbx
    call  get_multiplier
    mov   [scroll_value_1_multiply], r8
    mov   rdx , [xpos]
    add   rdx , 4+scx
    shl   rdx , 32
    add   rdx , 140-14+6+scl
    call  display_number

    mov   rbx , [scroll_value_2]
    sub   rbx , 20000
    mov   rcx , 200
    sub   rcx , rbx
    call  get_multiplier
    mov   [scroll_value_2_multiply], r8
    mov   rdx , [xpos]
    add   rdx , 4+7*6+scx
    shl   rdx , 32
    add   rdx , 140-14+6+scl
    call  display_number

    mov   rbx , [scroll_value_3]
    sub   rbx , 30000
    mov   rcx , 200
    sub   rcx , rbx
    call  get_multiplier
    mov   [scroll_value_3_multiply], r8
    mov   rdx , [xpos]
    add   rdx , 4+14*6+scx
    shl   rdx , 32
    add   rdx , 140-14+6+scl
    call  display_number

    ret


get_multiplier:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   Multiplier for colour values
; 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    push  rax rbx rcx rdx

    shl   rcx , 7
    mov   rax , rcx
    xor   rdx , rdx
    mov   rbx , 100
    div   rbx
    mov   r8  , rax

    pop   rdx rcx rbx rax

    ret


draw_scroll_1:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   Scroll 1
; 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    mov   rax , 113
    mov   rbx , 1
    mov   rcx , 10000
    mov   rdx , 201
    mov   r8  ,[scroll_value_1]
    mov   r9  , [xpos]
    add   r9  , 7+scx
    mov   r10 , 50
    mov   r11 , 80-12+6+scl
    int   0x60

    ret


draw_scroll_2:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   Scroll 2
; 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    mov   rax , 113
    mov   rbx , 1
    mov   rcx , 20000
    mov   rdx , 201
    mov   r8  ,[scroll_value_2]
    mov   r9  , [xpos]
    add   r9  , 7+7*6+scx
    mov   r10 , 50
    mov   r11 , 80-12+6+scl
    int   0x60

    ret


draw_scroll_3:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   Scroll 3
; 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    mov   rax , 113
    mov   rbx , 1
    mov   rcx , 30000
    mov   rdx , 201
    mov   r8  ,[scroll_value_3]
    mov   r9  , [xpos]
    add   r9  , 7+14*6+scx
    mov   r10 , 50
    mov   r11 , 80-12+6+scl
    int   0x60

    ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   Data area
; 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

scroll_value_1: dq 10100
scroll_value_2: dq 20100
scroll_value_3: dq 30100

scroll_value_1_multiply: dq 10100
scroll_value_2_multiply: dq 20100
scroll_value_3_multiply: dq 30100

text:

    db    ' xxx%   xxx%   xxx%           ',0
    db    ' Red   Green   Blue           ',0
    db    'x'

string_res_160: db '160',0
string_res_320: db '320',0
string_res_640: db '640',0

picsend:      dq 0x0
show_fps:     dq 0x0
fastercount:  dq 0x0

string_start_camera:  db  'START',0
string_stop_camera:   db  'STOP',0
string_snap_save:     db  '/USB/1/SNAPXX.BMP',0
string_clip_save:     db  '/USB/1/CLIPXX.AVI',0

string_server_off:     db  'SERVER: OFF',0
string_server_on:      db  'PORT 8090 OPEN',0
string_resolution_320: db  '320x240 required.',0
string_fps:            db  'Sent:       FPS:  /  ',0
string_screen_req:     db  '800x600 screen resolution required.',0


string_camera_disconnected:

    db    'Camera disconnected.',0

read_wait: dq 0x0

pdata82:   dq   data8
pdata92:   dq   data9

scanstart:  dq  0x0
threadpos:  dq  0x0
fps_n:      dq  0x0

threadposprev:      dq 0x0
frames_per_second:  dq 0x0

fpsx:        dq  0x0
record_next: dq  0x0

camera_connection_status:  dq  0x2
frames_per_second_missed:  dq  0x0

xr: dq 160,320,640
yr: dq 120,240,480
xp: dq 346,346,666
cb: dq 001b,010b,100b

serveronoff: dq 3 ; 1=on-2=closing-3=closed

server_socket: dq 0x0

resx: dq 320
resy: dq 240
xpos: dq 346

resx_resy_2: dq 320*240*2
resx_resy_3: dq 320*240*3
resx_resy_4: dq 320*240*4

sconf: dq 0x0
sizex: dq 0x0  
sizey: dq 0x0 

filesaveavi:  db  '/usb/1/clipxx.avi',0
record_state: dq  0x0
clipcount:    dq  0x0
filesave:     db  '/usb/1/snapxx.bmp',0
snapcount:    dq  0x0

textrec:      db 'REC (1/5s)',0
textsaving:   db 'Saving AVI-file, which might take a minute...',0
textsaving2:  db 'Saving BMP-file...',0

record_position: dq data10 + 1208

red:    dq 0x0
green:  dq 0x0
blue:   dq 0x0

framebegin:   dq 0x0
found_frame:  dq 0x0

framestart:      dq  0x0
framenumber:     dq  0x0
timedifference:  dq  0x0
headerstart:     dq  0x0
next_update:     dq  0x0
thread_running:  dq  0x0
read_position:   dq  0x0

pre_read_block:          dq  0x1
analyze_block:           dq  0x0
analyze_block_position:  dq  0x0

readon:        dq 0x1
camera_state:  dq 0x0

tsc_base: dq  0x0
fps_next: dq  0x0
fps_add:  dq  34 ; 30fps
tsc_next: dq  0x0

color:      dq   0x0
ypos:       dq   0x0
maxcount:   dq   0x0
framec:     dq   0x0
datapos:    dq   0x0 
savecount:  dq   0x0

framecount:       dq  0x0
currentframe:     dq  0x0
bytesperframe:    dq  0x0
jpgheaders:       dq  0x0
jpgheadersmem:    dq  0x0
bytesperframejpg: dq  0x0
bytespersecond:   dq  0x0
framesinsample:   dq  0x0

data_amount:       dq  0x0
data_valid:        dq  0x0
data_pre:          dq  0x0
data_unavailable:  dq  0x0
datasent:          dq  0x0

window_label:

    db    'WEBCAM',0       ; Window label

menu_struct:                  ; Menu Struct

       dq   0                 ; Version

       dq   0x100             ; Start value of ID to return ( ID + Line )
                              ; Returned when menu closes and
                              ; user made no selections.

       db   0,'SETUP',0              

sta1:  db   1,'> Stats Off ',0
sta2:  db   1,'  Stats On  ',0

       db   1,'-',0

eo:    db   1,'> Normal  ',0
eb:    db   1,'  Blur    ',0
en:    db   1,'  Negative',0

       db   1,'-',0

       db   1,'Quit',0        

       db   255               ; End of Menu Struct


html_header_index:

     db  'HTTP/1.1 200 OK',13,10
     db  'Server: Menuet',13,10
     db  'Connection: close',13,10
     db  'Content-Length: 47',13,10
     db  'Content-Type: text/html',13,10
     db  13,10
     db  '<html><body><img src=image.bmp></body></html>',13,10

h_i_len:



html_header:

     db  'HTTP/1.1 200 OK',13,10
     db  'Server: Menuet',13,10
     db  'Connection: close',13,10
     db  'Content-Length: '
hsz: db  '00230454'
     db  13,10
     db  'Content-Type: image/bmp',13,10,13,10

h_len:


bmpheader:

    db    66   
    db    77
    db    54
    db    12
    db    0
    db    0
    db    0
    db    0
    db    0
    db    0    
    db    54
    db    0
    db    0
    db    0
    db    40
    db    0
    db    0
    db    0
    db    32 ; x
    db    0    
    db    0        
    db    0
    db    32 ; y
    db    0
    db    0
    db    0
    db    1
    db    0
    db    24
    db    0     
    db    0         
    db    0
    db    0
    db    0
    db    0
    db    12
    db    0
    db    0
    db    0
    db    0    
    db    0
    db    0
    db    0
    db    0
    db    0
    db    0
    db    0
    db    0
    db    0
    db    0    
    db    0
    db    0
    db    0
    db    0    

numframes equ 50

avi_start:

    db    'RIFF'
    dd    15361432-8

    db    'AVI '

    db    'LIST'
    dd    l4_end-l4
    l4:
    db    'hdrl'
    dd    'avih'
    dd    0x38     
    dd    100000    ; microseconds between frames
    dd    0x320000  ; maxbytespersec
    dd    0x200     ; paddinggranularity
    dd    0x810     ; flags
    dd    numframes ; total frames
    dd    0x00      ; zero for non-interleaved
    dd    0x01      ; number of streams
    dd    330*240*4 ; suggested buffer size
    dd    320       ; image size x
    dd    240       ; image size y
    dd    0,0,0,0

    db    'LIST'
    dd    l6_end-l6
    l6:
    db    'strl'
    db    'strh'
    dd    0x38
    db    'vids'
    db    'DIB '     ; handler
    dd    0x0        ; flags
    dd    0x0        ; priority and language
    dd    0x0        ; initial frames
    dd    01000000   ; scale 
    dd    10000000   ; rate  
    dd    0x0        ; start
    dd    numframes  ; number of frames
    dd    320*240*4  ; 0x4b008  ; suggested buffer size
    dd    0x0        ; quality
    dd    4          ; sample size
    dd    0x0        ; rect struct
    dd    0x0

    db    'strf' 
    dd    0x28      ; length
    dd    0x28      ; length II
    dd    320       ; width
    dd    240       ; height
    dd    0x200001  ; planes and bitcount
    dd    0x0       ; compression
    dd    320*240*4 ; image size
    dd    0x0       ; xpelspermeter
    dd    0x0       ; ypelspermeter
    dd    0x0       ; clrused
    dd    0x0       ; clrimportant

    l4_end:
    l6_end:

    db    'LIST'
    dd    4+(2*4+320*240*4)*50
    l5:
    db    'movi'

avi_end:

idx1_start:

    db    'idx1'
    dd    indx12-indx11
  indx11:
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*0
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*1
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*2
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*3
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*4
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*5
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*6
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*7
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*8
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*9
    dd    320*240*4

    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*10
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*11
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*12
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*13
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*14
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*15
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*16
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*17
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*18
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*19
    dd    320*240*4

    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*20
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*21
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*22
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*23
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*24
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*25
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*26
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*27
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*28
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*29
    dd    320*240*4

    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*30
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*31
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*32
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*33
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*34
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*35
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*36
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*37
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*38
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*39
    dd    320*240*4

    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*40
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*41
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*42
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*43
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*44
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*45
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*46
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*47
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*48
    dd    320*240*4
    db    '00db'
    dd    16
    dd    0x4+(320*240*4+8)*49
    dd    320*240*4

  indx12:

idx1_end:


data_boot_info: times 8*30 db ?


fps_data:        

    times 2048 db ?

    ;

    times 1024 db ? ; Stack server

server_stack:

    times 1024 db ? ; Stack read

thread_stack:

    times 1024 db ? ; Stack main

image_end:









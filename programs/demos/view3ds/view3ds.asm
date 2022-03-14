
; application : View3ds ver. 0.077 - tiny .3ds and .asc files viewer
;               with a few graphics effects demonstration.
; compiler    : FASM
; system      : KolibriOS
; author      : Macgub aka Maciej Guba
; email       : macgub3@wp.pl
; web         : http://macgub.co.pl
; Fell free to use this intro in your own distribution of KolibriOS.
; Special greetings to KolibriOS team .
; I hope because my demos Christian Belive will be near to each of You.


; Some adjustments made by Madis Kalme
; madis.kalme@mail.ee
; I tried optimizing it a bit, but don't know if it was successful. The objects
; can be:
; 1) Read from a file (*.3DS standard)
; 2) Written in manually (at the end of the code) ; now not exist


SIZE_X equ 512
SIZE_Y equ 512                               ;      /////     I want definitely
TIMEOUT equ 10                               ;     ------     say:
ROUND equ 10                                 ;     \ @ @/     keep smiling every
TEX_X equ 512    ; texture width             ;      \ ./    / day.
TEX_Y equ 512    ;         height            ;       \/    /
TEX_SHIFT equ 9  ; texture width shifting    ;     __||__ /
TEXTURE_SIZE EQU (TEX_X * TEX_Y)-1           ;   /|      |
TEX equ SHIFTING ; TEX={SHIFTING | FLUENTLY} ;  / \      /
FLUENTLY = 0                                 ; /   |    |
SHIFTING = 1                                 ;     ------
CATMULL_SHIFT equ 8                          ;      |  |
LIGHT_SIZE equ 22                            ;      |  |
NON   =   0                                  ;     -/  \-
MMX   =   1
SSE   =   2
SSE2  =   3
SSE3  =   4
Ext   =   SSE3          ;Ext={ NON | MMX | SSE | SSE2 | SSE3 }

; For now correct only SSE2 and SSE3 versions. if you have older CPU
; use older versions of app. Probably ver 005 will be OK but it need
; re-edit to support new Kolibri features.

use32
        org    0x0
        db     'MENUET01'       ; 8 byte id
        dd     0x01             ; header version
        dd     START            ; start of code
        dd     I_END            ; size of image
        dd     MEM_END          ; memory for app
        dd     MEM_END          ; esp
        dd     I_Param          ; I_Param
        dd     0x0              ; I_Icon

START:    ; start of execution
        rdtsc
        mov    [rand_seed],ax
        cld
        push   dword (SIZE_Y shr 3) * 3
        fninit
        fild   dword[esp]
        fstp   [rsscale]
        pop    ebx

        call    alloc_buffer_mem
        call    read_param
        call    read_from_disk    ; read, if all is ok eax = 0
        btr     eax,31            ; mark 1
        cmp     eax,0
        jne     .gen
        bts     eax,31            ; mark 2
        mov     esi,[fptr]
        cmp     [esi],word 4D4Dh
        jne     .asc_gen
        call    read_tp_variables ; init points and triangles count variables
        cmp     eax,0
        jne    .malloc
        xor    eax,eax            ; if failed read -> generate
    .gen:
    .asc_gen:   ; read asc file or generate
        push    eax
     ; if no house.3ds on rd - generate
        xor      bl,bl ; allocate memory
        mov      [triangles_count_var],20000
        mov      [points_count_var],20000
        call     alloc_mem_for_tp
        pop      eax
        bt       eax,31
        jc       .asc
        mov      bl,[generator_flag]
        call     generate_object
        mov      ax,1  ;mark

        jmp      .opt
    .asc:
   ;     xor    bl,bl
   ;     mov    [triangles_count_var],20000  ; to do: read asc header
   ;     mov    [points_count_var],20000
   ;     call   alloc_mem_for_tp
        call   read_asc
        xor    ax,ax
        jmp    .opt
    .malloc:
        call   alloc_mem_for_tp
        call   read_from_file
    .opt:
      if     Ext >= SSE2
        push   ax
      end if
        call   optimize_object1     ;  proc in file b_procs.asm
                                    ;  set point(0,0,0) in center and  calc all coords
                                    ;  to be in <-1.0,1.0>
        call   normalize_all_light_vectors
        call   copy_lights ; to aligned float
 ;       call   init_triangles_normals2


     if Ext >= SSE2
              ; if first byte of ax set -> old style normal vectors finding
        call   detect_chunks
        mov    [chunks_number],ecx
        mov    [chunks_ptr],ebx
        push   esi
        push   edi
        call   init_triangles_normals2
     ;  esi -   tri_ch
     ;  edi -   t_ptr - every vertice index  - pointer to to all triangles
     ;          that have this index
        pop    edi
        pop    esi
        pop    ax

     end if

        call   init_point_normals

        call   init_envmap2
        call   init_envmap_cub
        call   generate_texture2
        call   init_sincos_tab
        call   do_color_buffer   ; intit color_map
     if Ext >= SSE3
        call   init_point_lights
     end if
        mov    edi,bumpmap
        call   calc_bumpmap
        call   calc_bumpmap_coords   ; bump and texture mapping
        call   do_edges_list
        call   draw_window
     if Ext > SSE2
        mov     eax,1
        cpuid
        bt      ecx,0  ; is sse3 on board?
        jc      @f
        mov     byte[max_dr_flg],12
        mov     [isSSE3],0
       @@:
     end if


 still:
        cmp    [edit_flag],1
        jne    @f
        mov    eax,40         ; set events mask
        mov    ebx,1100000000000000000000000100111b
        jmp    .int
      @@:
        mov    eax,40         ; set events mask
        mov    ebx,111b
      .int:
        int    0x40
      if Ext > SSE2
        cmp    [ray_shd_flag],1
        jne    @f
        cmp    [isSSE3],1
        jne    @f
        mov    eax,10
        jmp    .intt
      end if

       @@:
        mov     eax,23
        mov     ebx,TIMEOUT
        cmp     [speed_flag],0
        je      .skip
        mov     eax,11
     .skip:
        cmp     [edit_flag],1
        jne     @f
        mov     eax,10

      @@:
      .intt:
        int     0x40

        cmp     eax,1           ; redraw event ?
        je      red
        cmp     eax,2           ; key event ?
        je      key
        cmp     eax,3           ; button event ?
        je      button

        mov     esi,eax
        mov     eax,37
        mov     ebx,7           ; get mouse scroll
        int     0x40
        and     eax, 0xFFFF     ; check only vertial
        cmp     eax, 65535
        je      button.zoom_in
        cmp     eax, 1
        je      button.zoom_out
        mov     eax,esi

        cmp     eax,6           ; mouse event ?
        jne     @f
        cmp     [edit_flag],1   ; handle mouse only when edit is active
        jne     @f
        mov     eax,37
        mov     ebx,3   ;read mouse state
        int     0x40
        mov     [mouse_state],eax
        call    edit
       @@:
        jmp     noclose

    red:   ; redraw
     ;   xor     edx,edx
     ; @@:
     ;   push    edx
        mov     eax,9  ; get process info
        mov     ebx,procinfo
        or      ecx,-1
        int     0x40
     ;   pop     edx
     ;   inc     edx
     ;   cmp     dword[procinfo+26],50000000  ; ~ 10 Mbytes
     ;   jb      @f
     ;   cmp     edx,1
     ;   je      @b


    ; @@:
        mov     eax,[procinfo+42]    ; read params of window
        sub     eax,225
        mov     [size_x_var],ax
        shr     ax,1
        mov     [vect_x],ax
;
        mov     eax,[procinfo+46]
        sub     eax,30
        mov     [size_y_var],ax
        shr     ax,1
        mov     [vect_y],ax

        mov     eax,[procinfo+34]
        mov     [x_start],ax
        mov     eax,[procinfo+38]
        mov     [y_start],ax
        call    alloc_buffer_mem ;realloc mem for scr & z buffs
        call    draw_window

        jmp     noclose

    key:                        ; key
        mov     eax,2           ; just read it and ignore
        int     0x40
        shr     eax,16          ; use scancodes (al)

        cmp     al, 013 ;+
        je      button.zoom_in
        cmp     al, 012 ;-
        je      button.zoom_out
        cmp     al, 075 ;left
        je      add_vec_buttons.x_minus
        cmp     al, 077 ;right
        je      add_vec_buttons.x_plus
        cmp     al, 072 ;up
        je      add_vec_buttons.y_minus
        cmp     al, 080 ;down
        je      add_vec_buttons.y_plus
        cmp     al, 073 ;page up
        je      .rot_inc_y
        cmp     al, 081 ;page down
        je      .rot_dec_y
        cmp     al, 051 ;<
        je      .rot_dec_x
        cmp     al, 052 ;>
        je      .rot_inc_x
        cmp     al, 057 ;space
        je      .rot_z

        jmp     noclose

        .rot_inc_x:
        inc     [angle_x]
        and     [angle_x],0xff
        jmp     noclose.end_rot
        .rot_dec_x:
        dec     [angle_x]
        and     [angle_x],0xff
        jmp     noclose.end_rot
        .rot_inc_y:
        inc     [angle_y]
        and     [angle_y],0xff
        jmp     noclose.end_rot
        .rot_dec_y:
        dec     [angle_y]
        and     [angle_y],0xff
        jmp     noclose.end_rot
        .rot_z:
        inc     [angle_z]
        and     [angle_z],0xff
        jmp     noclose.end_rot

    button:                     ; button
        mov     eax,17          ; get id
        int     0x40

        cmp     ah,1            ; button id=1 ?
        jne     @f

        mov     eax,-1          ; close this program
        int     0x40
    @@:
        cmp     ah,30
        jge     add_vec_buttons
        call    update_flags          ; update flags and write labels of flags

                                      ; do other operations according to flag
;        cmp     ah,3                 ; ah = 3 -> shading model
;        jne     .next_m6
;        cmp     [dr_flag],2
;        jne     @f
   ;     call    init_envmap2    ;   <----! this don't works in env mode
                                 ;          and more than ~18 kb objects
 ;       call    init_envmap_cub2
;     @@:
        cmp     [dr_flag],4
        jne     @f
        call    generate_texture2

     @@:
     .next_m6:
                                      ; ah = 5 -> scale-
        cmp     ah,5
        jne     @f
        .zoom_out:
        mov     dword[scale],0.7
        fninit
        fld     [rsscale]
        fmul    [scale]
        fstp    [rsscale]
       @@:
        cmp     ah,6                 ; ah = 6 ->  scale+
        jne     @f
      .zoom_in:
        mov     dword[scale],1.3
        fninit
        fld     [rsscale]
        fmul    [scale]
        fstp    [rsscale]
       @@:
        cmp     ah,9    ; lights random                 ;    'flat'  0
        jne     .next_m5                                ;    'grd '  1
        call    make_random_lights                      ;    'env '  2
        call    normalize_all_light_vectors             ;    'bump'  3
        call    copy_lights
      if Ext >= SSE3
        call   init_point_lights  ; for ex. ray casting
      end if
        call    do_color_buffer   ; intit color_map     ;    'tex '  4

        call    init_envmap2    ; update env map if shading model = environment or bump
    .next_m5:
        cmp      ah,11
        je       @f
        cmp      ah,12
        je       @f
        cmp      ah,13
        jne      .next_m4
      @@:
        call     mirror
     .next_m4:
        cmp      ah,14
        jne      @f
                .xchg:
        call     exchange
     @@:
        cmp      ah,15
        jne      @f
        cmp      [emboss_flag],1
     ;   call     init_envmap2
        call     do_emboss
     @@:
 ;       cmp      ah,17
 ;       jne      .next_m
 ;       cmp      [move_flag],2
 ;       jne      @f
 ;       call     draw_window             ; redraw other labels to navigation buttons
 ;     @@:
 ;       cmp      [move_flag],0
 ;       jne      .next_m
 ;       call     draw_window             ; redraw other labels to navigation buttons
     .next_m:
        cmp      ah,18
        jne      .next_m2

        mov      bl,1       ; reallocate memory
        mov      [triangles_count_var],20000
        mov      [points_count_var],20000
        call     alloc_mem_for_tp

        mov      bl,[generator_flag]
     ;   or       bl,bl
     ;   jz       .next_m2
        cmp      bl,1
        jne      @f
        call     generate_object
        jmp      .calc_norm
      @@:
        cmp      bl,4
        jg       @f
        movzx    eax,bl                ; eax < - object number
        call     generate_object2
        jmp     .calc_norm
      @@:
        call    generate_object3
      .calc_norm:
        call    optimize_object1
        call    init_triangles_normals2
     if Ext >= SSE2
        call   detect_chunks
        mov    [chunks_number],ecx
        mov    [chunks_ptr],ebx
        mov    ax,1  ; - old style detecting normal vectors
     ;   esi -   tri_ch
     ;   edi -   t_ptr - every vertice index  - pointer to to all triangles
     ;           that have this index
     end if

        call    init_point_normals
        call    calc_bumpmap_coords   ; bump and texture mapping
        call    do_edges_list
        call    write_info

     .next_m2:
        cmp      ah,19
        je       @f
        cmp      ah,20
        jne      .next_m3
     @@:
        mov      edi,bumpmap
        call     calc_bumpmap
     .next_m3:
        cmp     ah,21            ; re map bumps, texture coordinates
        jne     @f
        call    calc_bumpmap_coords
      @@:
        jmp     noclose


                               ; there are 6 navigation buttons each
   add_vec_buttons:            ; can move: object, camera,.. list is open
                               ;
        cmp     ah,30
        jne     .next
        cmp     [move_flag],0
        jne     @f
;        cmp     [move_flag],2
;        je      .set_light1
        .y_minus:
        sub     [vect_y],10
        jmp     .next
      @@:
        cmp     [move_flag],1
        jne     @f
        sub     [yobs],10   ;  observator = camera position
        jmp     .next
      @@:
        sub     [sin_amplitude],10
;--------------------------------------------------
;      .set_light1:          ;  r -
;        movzx   ebx,[light_no_flag]  ; * 22
;        mov     ecx,ebx
;        shl     ebx,4
;        shl     ecx,1
;        add     ebx,ecx
;        shl     ecx,1
;        add     ebx,ecx
;        add     ebx,lights+6    ; 6 -> light vector size
;
;        movzx   ecx,[light_comp_flag]
;        lea     ecx,[ecx*3}
;        add     ebx,ecx         ; ebx ->  color to set

;---------------------------------------------------
      .next:
        cmp     ah,31
        jne     .next1
        cmp     [move_flag],1
        je      @f
        add     [vect_z],10
        jmp     .next1
      @@:
        add     [zobs],10         ;  observator = camera position
     .next1:
        cmp     ah,33
        jne     .next2
        cmp     [move_flag],0
        jne     @f
        .x_minus:
        sub     word[vect_x],10
        jmp     .next2
      @@:
        cmp     [move_flag],1
        jne     @f
        sub     [xobs],10         ;  observator = camera position
        jmp     .next2
      @@:
        fninit
        fld     [sin_frq]
        fsub    [sin_delta]
        fstp    [sin_frq]
      .next2:
        cmp     ah,32
        jne     .next3
        cmp     [move_flag],0
        jne     @f
        .x_plus:
        add     word[vect_x],10
        jmp     .next3
      @@:
        cmp     [move_flag],1
        jne     @f
        add     [xobs],10         ;  observator = camera position
        jmp     .next3
      @@:
        fninit
        fld     [sin_frq]      ; change wave effect frequency
        fadd    [sin_delta]
        fstp    [sin_frq]
      .next3:
        cmp     ah,34
        jne     .next4
        cmp     [move_flag],1
        je      @f

        sub     [vect_z],10
        jmp     .next4
      @@:
        sub     [zobs],10         ;  observator = camera position
      .next4:
        cmp     ah,35
        jne     .next5
        cmp     [move_flag],0
        jne      @f
      ;  call    add_vector
            .y_plus:
        add     [vect_y],10
        jmp     .next5
      @@:
        cmp     [move_flag],1
        jne     @f
        add     [yobs],10         ;  observator = camera position
        jmp     .next5
      @@:
        add     [sin_amplitude],10
      .next5:



    noclose:

        cmp     [edit_flag],1
        jz      .end_rot
        cmp     [r_flag],2
        jne     .no_x
        inc     [angle_x]
        and     [angle_x],0xff
        ;mov     [angle_z],0
        jmp     .end_rot

      .no_x:
        cmp     [r_flag],0
        jne     .no_y
        inc     [angle_y]
        and     [angle_y],0xff
        ;mov     [angle_z],0
        jmp     .end_rot

      .no_y:
        cmp     [r_flag],1
        jne     .end_rot
        mov     cx,[angle_x]
        inc     cx
        and     cx,0xff
        ;mov     [angle_z],0
        mov     [angle_y],cx
        mov     [angle_x],cx
     .end_rot:

        mov     esi,angle_x
        mov     edi,matrix
        call    make_rotation_matrix
    RDTSC
    push eax
        mov     esi,[points_normals_ptr]
        mov     edi,[points_normals_rot_ptr]
        mov     ebx,matrix
        mov     ecx,[points_count_var]
        call    rotary

        mov     esi,matrix
        call    add_scale_to_matrix

        mov     esi,[points_ptr]
        mov     edi,[points_rotated_ptr]
        mov     ebx,matrix
        mov     ecx,[points_count_var]
        call    rotary


        mov     esi,[points_rotated_ptr]
        mov     edi,[points_translated_ptr]
        mov     ecx,[points_count_var]
        call    translate_points

        cmp     [fire_flag],0
        jne     @f
        call    clrscr          ; clear the screen
     @@:
    ;    cmp     [catmull_flag],1  ;non sort if Catmull = on
    ;    je      .no_sort
    ; 64 indexes    call    sort_triangles
      .no_sort:
        cmp     [dr_flag],7       ; fill if 2tex and texgrd
        jge     @f
        cmp     [dr_flag],6       ; non fill if dots
        je      .non_f
      @@:
        call    fill_Z_buffer     ; make background
     .non_f:
        cmp     [dr_flag],6
        jne     @f
        call     draw_dots
        jmp      .blurrr
      @@:
      if Ext > SSE2
        cmp     [ray_shd_flag],1  ;non fill if Catmull = off
        jne     @f
        cmp     [isSSE3],1
        jne     @f
        mov     ax,100
        jmp     .dr
       @@:
      end if

        movzx   ax,[dr_flag]
      .dr:
        call    draw_triangles  ; draw all triangles from the list
        cmp    [edit_flag],0
        jz     .no_edit
        call   clear_vertices_index
        movzx eax,[dr_flag]
        movzx ebx,[ray_shd_flag]
        shl   ebx,10
        or    eax,ebx
         call   draw_handlers
    ;    call   edit



    .no_edit:

      .blurrr:
        movzx eax,[dr_flag]
        movzx ebx,[ray_shd_flag]
        shl   ebx,10
        or    eax,ebx
        cmp   [sinus_flag],0
        je    .no_sin
        movzx eax,[dr_flag]
        movzx ebx,[ray_shd_flag]
        shl   ebx,10
        or    eax,ebx
        call  do_sinus
      ;  jmp   .finito
      .no_sin:
      @@:
        movzx   ecx,[fire_flag]
        cmp     [fire_flag],1
        je      @f
        cmp     [blur_flag],0
        je      .no_blur  ; no blur, no fire
        movzx   ecx,[blur_flag]
      @@:
        movzx   eax,[dr_flag]
        movzx   ebx,[ray_shd_flag]
        shl     ebx,10
        or      eax,ebx
        call    blur_screen    ; blur and fire
     ;   jmp     .finito

    .no_blur:                  ; no blur, no fire
        cmp     [emboss_flag],0
        je      @f
        movzx eax,[dr_flag]
        movzx ebx,[ray_shd_flag]
        shl   ebx,10
        or    eax,ebx
        call    do_emboss
    .finito:
     @@:


    cmp     [inc_bright_flag],0           ; increase brightness
    je      .no_inc_bright
    movzx   ebx,[inc_bright_flag]
    shl     ebx,4
    mov     esi,[screen_ptr]
    movzx   ecx,word[size_y_var]
    movzx   eax,word[size_x_var]
    mul     ecx
    lea     ecx,[eax*4]

if (Ext = MMX)|(Ext = SSE)
    emms
    mov      bh,bl
    push     bx
    shl      ebx,16
    pop      bx
    push     ebx
    push     ebx
    movq     mm0,[esp]
    add      esp,8
else if Ext >= SSE2
    mov      bh,bl
    push     bx
    shl      ebx,16
    pop      bx
    movd     xmm0,ebx
    shufps   xmm0,xmm0,0
end if
  .oop:
if Ext=NON
    lodsb
    add     al,bl
    jnc     @f
    mov     byte[esi-1],255
    loop    .oop
   @@:
    mov     [esi-1],al
    loop    .oop
else if (Ext=MMX)|(Ext=SSE)
    movq    mm1,[esi]
    movq    mm2,[esi+8]
    paddusb mm1,mm0
    paddusb mm2,mm0
    movq    [esi],mm1
    movq    [esi+8],mm2
    add     esi,16
    sub     ecx,16
    jnz     .oop
else
    movaps  xmm1,[esi]
    paddusb xmm1,xmm0
    movaps  [esi],xmm1
    add     esi,16
    sub     ecx,16
    jnc     .oop
end if

.no_inc_bright:


    cmp     [dec_bright_flag],0
    je      .no_dec_bright
    movzx   ebx,[dec_bright_flag]
    shl     ebx,4
    mov     esi,[screen_ptr]
    movzx   eax,word[size_x_var]
    movzx   ecx,word[size_y_var]
    mul     ecx
    lea     ecx,[eax*4]
 if (Ext = MMX)|(Ext = SSE)
    mov      bh,bl
    push     bx
    shl      ebx,16
    pop      bx
    push     ebx
    push     ebx
    movq     mm0,[esp]
    add      esp,8
else if Ext >=SSE2
    mov      bh,bl
    push     bx
    shl      ebx,16
    pop      bx
    movd     xmm0,ebx
    shufps   xmm0,xmm0,0
end if
 .oop1:
if Ext=NON
    lodsb
    sub     al,bl
    jb      @f
    mov     [esi-1],al
    loop    .oop1
   @@:
    mov     byte[esi-1],0
    loop    .oop1
else if (Ext = MMX)|(Ext=SSE)
    movq    mm1,[esi]
    psubusb mm1,mm0
    movq    [esi],mm1
    add     esi,8
    sub     ecx,8
    jnz     .oop1
else
    movaps  xmm1,[esi]
    psubusb xmm1,xmm0
    movaps  [esi],xmm1
    add     esi,16
    sub     ecx,16
    jnc     .oop1
end if
  .no_dec_bright:


    RDTSC
    sub eax,[esp]
    sub eax,41
;    pop     eax

    mov     ecx,10
  .dc:
    xor     edx,edx
    mov     edi,10
    div     edi
    add     dl,30h
    mov     [STRdata+ecx-1],dl
    loop    .dc
    pop eax


    mov     eax,7           ; put image
    mov     ebx,[screen_ptr]
    mov     ecx,[size_y_var]
    mov     edx,[offset_y]
    cmp     [ray_shd_flag],1
    jge     .ff
    cmp     [dr_flag],11
    jge     .ff
    int     0x40
    jmp     .f
  .ff:
    mov     eax,65
    mov     esi,32
    xor     ebp,ebp
    int     0x40
   .f:
    mov  eax,13
    mov  bx,[size_x_var]
    add  ebx,18
    shl  ebx,16
    mov  bx,60
    mov  cx,[size_y_var]
    sub  cx,2
    shl  ecx,16
    mov  cx,9
    xor  edx,edx
    int  40h

    mov  eax,4                     ; function 4 : write text to window
    mov  bx,[size_x_var]
    add  ebx,18
    shl  ebx,16
    mov  bx,[size_y_var]
    sub  bx,2         ; [x start] *65536 + [y start]
    mov  ecx,0x00888888
    mov  edx,STRdata               ; pointer to text beginning
    mov  esi,10                    ; text length
    int  40h



   jmp     still


;--------------------------------------------------------------------------------
;-------------------------PROCEDURES---------------------------------------------
;--------------------------------------------------------------------------------
include "flat_cat.inc"
include "tex_cat.inc"
include "bump_cat.inc"
include "3dmath.inc"
include "grd_line.inc"
include "b_procs.inc"
include "a_procs.inc"
include "chunks.inc"
include "grd_cat.inc"
include "bump_tex.inc"
include "grd_tex.inc"
include "two_tex.inc"
include "asc.inc"
if Ext >= SSE3
include "3r_phg.inc"
include '3stencil.inc'
include '3glass.inc'
include '3glass_tex.inc'
include '3ray_shd.inc'
end if
clear_vertices_index:
    mov   edi,[vertices_index_ptr]
    movzx eax,word[size_x_var]
    movzx ecx,word[size_y_var]
    imul  ecx,eax
    xor   eax,eax
 ;   shr   ecx,1
    rep   stosd
ret

edit:     ; mmx required, edit mesh by vertex
        push   ebp
        mov    ebp,esp
        sub    esp,128

        .y_coord equ ebp-2
        .x_coord equ ebp-4
        .points_translated equ ebp-10
        .points            equ ebp-26
        .points_rotated    equ ebp-26-16
        .mx                equ ebp-26-56

    macro check_bar
    {
        movzx  ebx,word[.x_coord]
        movzx  ecx,word[.y_coord]
        movzx  edx,word[size_x_var]
        imul   edx,ecx
        add    ebx,edx
        mov    ecx,ebx
        shl    ecx,2
        lea    ebx,[ebx*3]
        cmp    [dr_flag],10
        cmovg  ebx,ecx
        add    ebx,[screen_ptr]
        mov    ebx,[ebx]
        and    ebx,0x00ffffff
        cmp    ebx,0x00ff0000 ; is handle bar  ?
    }

        emms
        mov     eax,37  ; get mouse state
        mov     ebx,1   ; x = 5, y = 25 - offsets
        int     0x40

        mov     ebx,[offset_y] ;5 shl 16 + 25
        movd    mm0,ebx
        movd    mm1,eax
        movd    mm3,[size_y_var]
        pcmpgtw mm0,mm1
        pcmpgtw mm3,mm1
        pxor    mm3,mm0
        pmovmskb eax,mm3
        and     eax,1111b

        or      ax,ax
        jz      .no_edit


        movd    mm0,ebx
        psubw   mm1,mm0
        movd    eax,mm1

      ; store both x and y coordinates
        ror    eax,16
        mov    [.x_coord],eax
        test   word[mouse_state],100000000b
        jz     .not_press  ; check if left mouse button press

        ;  left button  pressed

        check_bar
        jne    .no_edit
        add    ecx,[vertices_index_ptr]
        mov    ecx,[ecx]
      ;  cmp    ecx,-1
      ;  je     .no_edit


        mov    [vertex_edit_no],ecx ;if vert_edit_no = -1, no vertex selected

        mov    eax,dword[.x_coord]
        mov    dword[edit_end_x],eax
        mov    dword[edit_start_x],eax
        jmp    .end
      .not_press:
        test   byte[mouse_state],1b       ; check if left button is held
        jz     .not_held
       ; check_bar
       ; jne    .no_edit
       ; add    ecx,[vertices_index_ptr]
       ; mov    cx,[ecx]
       ; inc    cx
        cmp    [vertex_edit_no],-1 ; cx  ; vertex number
        je     .end
        push   dword[.x_coord]
        pop    dword[edit_end_x]
        jmp    .end
      .not_held:
        shr    [mouse_state],16
        test   byte[mouse_state],1b  ; test if left button released
        jz     .end
        check_bar
        jne    .end

        movd        xmm0,[edit_end_x]
        punpcklwd   xmm0,[the_zero]
        movd        xmm1,[vect_x]
        punpcklwd   xmm1,[the_zero]
   ;     movd        xmm2,[offset_y]
   ;     punpcklwd   xmm2,[the_zero]
        psubd       xmm0,xmm1
   ;     psubd       xmm0,xmm2
        cvtdq2ps    xmm0,xmm0
        movups      [.points],xmm0


        mov     esi,matrix
        lea     edi,[.mx]
        call    reverse_mx_3x3

        lea     esi,[.points]
        lea     edi,[.points_rotated]
        lea     ebx,[.mx]
        mov     ecx,1
        call    rotary

   ;    inject into vertex list
        mov     edi,[vertex_edit_no]
      ;  dec     edi
        lea     edi,[edi*3]
        shl     edi,2
        add     edi,[points_ptr]
        lea     esi,[.points_rotated]
        cld
        movsd
        movsd
        movsd

        mov    dword[edit_start_x],0
        mov    dword[edit_end_x],0
        mov    [vertex_edit_no],-1

      .no_edit:
      .end:
      mov   esp,ebp
      pop   ebp
ret

alloc_buffer_mem:
    push    ebp
    mov     ebp,esp
    .temp   equ ebp-4
    push    dword 0

    mov     eax, 68
    mov     ebx, 11
    int     0x40    ;  -> create heap, to be sure


    movzx    ecx,word[size_x_var]
    movzx    eax,word[size_y_var]
    mul      ecx

    mov      [.temp],eax
    lea      ecx,[eax*4]    ; more mem for r_phg cause
    add      ecx,256
    mov      eax,68
    mov      ebx,20
    mov      edx,[screen_ptr]
    int      0x40
    mov      [screen_ptr],eax

    mov      ecx,[.temp]
    shl      ecx,2
    add      ecx,256
    mov      eax,68
    mov      ebx,20
    mov      edx,[Zbuffer_ptr]
    int      0x40
    mov      [Zbuffer_ptr],eax


    mov      ecx,[.temp]
    shl      ecx,2
    add      ecx,256
    mov      eax,68
    mov      ebx,20
    mov      edx,[vertices_index_ptr]
    int      0x40
    mov      [vertices_index_ptr],eax

    mov      esp,ebp
    pop      ebp
ret



update_flags:
; updates flags and writing flag description
; in    ah - button number
        push    ax
        mov     edi,menu
      .ch_another:
        cmp     ah,byte[edi]     ; ah = button id
        jne     @f
        mov     bl,byte[edi+11]  ; max_flag + 1
        cmp     bl,255
        je      .no_write
        inc     byte[edi+12]     ; flag
        cmp     byte[edi+12],bl
        jne     .write
        mov     byte[edi+12],0
        jmp     .write
      @@:
        add     edi,17
        cmp     byte[edi],-1
        jne     .ch_another
        jmp     .no_write
     .write:
;     clreol   {pascal never dies}
;          * eax = 13 - function number
;  * ebx = [coordinate on axis x]*65536 + [size on axis x]
;  * ecx = [coordinate on axis y]*65536 + [size on axis y]
;  * edx = color 0xRRGGBB or 0x80RRGGBB for gradient fill

        mov     eax,13                           ; function 13 write rectangle
        movzx   ecx,byte[edi]
        sub     cl,2
        lea     ecx,[ecx*3]
        lea     ecx,[ecx*5]
        add     ecx,28
        shl     ecx,16
        add     ecx,14   ;  ecx = [coord y]*65536 + [size y]
        mov     bx,[size_x_var]
        shl     ebx,16
        add     ebx,(12+70)*65536+25     ; [x start] *65536 + [size x]
        mov     edx,0x00000000                  ;  color  0x00RRGGBB
        int     0x40

        mov     eax,4                           ; function 4 : write text to window
        movzx   ebx,byte[edi]
        sub     bl,2
        lea     ebx,[ebx*3]
        lea     ebx,[ebx*5]
        mov     cx,[size_x_var]
        shl     ecx,16
        add     ebx,ecx
        add     ebx,(12+70)*65536+28     ; [x start] *65536 + [y start]
        mov     ecx,0x00ddeeff                  ; font 1 & color ( 0xF0RRGGBB )
        movzx   edx,byte[edi+12]                ; current flag
        shl     edx,2                           ; * 4 = text length
        add     edx,dword[edi+13]               ; pointer to text beginning
        mov     esi,4                           ; text length -
                                                ; flag description 4 characters
        int     0x40

     .no_write:
        pop     ax
ret
normalize_all_light_vectors:
        mov     edi,lights
     @@:
        call    normalize_vector           ;       3dmath.inc
        add     edi,LIGHT_SIZE
        cmp     edi,lightsend   ;ecx
        jl      @b
ret

calc_bumpmap_coords:      ; map texture, bump
;macro .comment222
;                                ; planar mapping
;        mov     esi,points
;        mov     edi,tex_points
;      @@:
;         add     esi,2
;         movsd
;         cmp     dword[esi],dword -1
;         jne     @b

;      .Pi2  equ dword[ebp-4]

;      mov   ebp,esp
;      sub   esp,4

      fninit
      fldpi
      fadd      st,st
      mov       esi,[points_ptr]
      mov       edi,[tex_points_ptr]
      mov       ecx,[points_count_var]
      inc       ecx
;      cmp       [map_tex_flag],1
;      jne       .cylindric
      ; spherical mapping around y axle

   @@:
      fld       dword[esi]     ; x coord
      fld       dword[esi+8]   ; z coord
      fpatan                   ; arctg(st1/st)
;      fdiv      .Pi2
      fdiv      st0,st1
      fimul     [tex_x_div2]
      fiadd     [tex_x_div2]
      fistp     word[edi]      ; x

      fld       dword[esi+4]   ; y coord
      fld       dword[esi]     ; x
      fmul      st,st0
      fld       dword[esi+4]   ; y
      fmul      st,st0
      fld       dword[esi+8]   ; z
      fmul      st,st0
      faddp
      faddp
      fsqrt
      fpatan
      fldpi
      fdivp
      fimul    [tex_y_div2]
      fiadd    [tex_y_div2]
      fistp    word[edi+2]     ; y

      add      esi,12
      add      edi,4
      loop     @b
      ffree    st0
;      jmp      .end_map
;  .cylindric:
;       fld     dword[esi]     ; around y axle
;       fld     dword[esi+8]
;       fpatan
;       fdiv    st0,st1
;       fimul   [tex_x_div2]
;       fiadd   [tex_x_div2]
;       fistp   word[edi]

;       fld     dword[esi+4]
;       fimul   [tex_y_div2]
;       fiadd   [tex_y_div2]
;       fistp   word[edi+2]

;       add     esi,12
;       add     edi,4
;       loop    .cylindric
;       ffree    st0
;;      mov      esp,ebp
;   .end_map:
ret


init_envmap2:         ; do env_map using many light sources
;env_map 512 x 512 x 3 bytes
.temp  equ word   [ebp-2]
.nEy   equ word  [ebp-4]
.nEx   equ word  [ebp-6]
.col_r equ    [ebp-8]
.col_g equ    [ebp-9]
.col_b equ    [ebp-10]

         push     ebp
         mov      ebp,esp
         sub      esp,20
         mov      edi,envmap
         fninit

         mov      dx,- TEX_Y / 2 ;256   ; dx - vertical coordinate = y
    .ie_ver:
         mov      cx,- TEX_X / 2 ;256   ; cx - horizontal coord = x
    .ie_hor:
         xor      ebx,ebx
         mov      dword .col_b, 0
     .light:
         lea      esi,[lights+ebx]
         fld      dword[esi]     ; light vector x cooficient
         fimul    [tex_x_div2] ;[i256]
         mov      .temp,cx
         fisubr   .temp
         fistp    .nEx
         fld      dword[esi+4]   ; light vector y cooficient
         fimul    [tex_y_div2] ;[i256]
         mov      .temp,dx
         fisubr   .temp
         fistp    .nEy

         cmp      .nEx,- TEX_X / 2 ;256
         jl       .update_counters
         cmp      .nEy,- TEX_Y / 2 ;256
         jl       .update_counters
         cmp      .nEx,TEX_X / 2 ;256
         jg       .update_counters
         cmp      .nEy,TEX_Y / 2 ;256
         jg       .update_counters

         fild     .nEx
         fmul     st,st0
         fild     .nEy
         fmul     st,st0
         faddp
         fsqrt
         fisubr   [i256]
         fmul     [env_const]
         fidiv    [i256]   ; st - 'virtual' dot product

         fcom     [dot_max]
         fstsw    ax
         sahf
         jb       @f
         ffree    st
         fld1     ;[dot_max]
      @@:
         fcom     [dot_min]
         fstsw    ax
         sahf
         ja       @f
         ffree    st
         fldz     ;[dot_min]
      @@:
         push     ebp
         movzx    ax,byte[esi+21]
         push     ax  ;- shines
         mov      al,byte[esi+14]   ; b    orginal color
         push     ax
         mov      al,byte[esi+13]   ; g
         push     ax
         mov      al,byte[esi+12]   ; r
         push     ax
         mov      al,byte[esi+20]   ; b     max color
         push     ax
         mov      al,byte[esi+19]   ; g
         push     ax
         mov      al,byte[esi+18]   ; r
         push     ax
         mov      al,byte[esi+17]   ; b    min col
         push     ax
         mov      al,byte[esi+16]   ; g
         push     ax
         mov      al,byte[esi+15]   ; r
         push     ax
         push     eax         ; earlier - dot pr
      ;  fstp     .dot_product
      ;  push     .dot_product
         call     calc_one_col
         pop      ebp
         ; eax-0x00rrggbb
         cmp      al,.col_b
         jbe      @f
         mov      .col_b,al
   @@:                        ;  eax - ggbb00rr
         shr      ax,8
         cmp      al,.col_g
         jbe      @f
         mov      .col_g,al
   @@:                        ;  eax - bb0000gg
         shr      eax,16
         cmp      al,.col_r
         jbe      @f
         mov      .col_r,al
   @@:
   .update_counters:                     ; update and jump when neccesery
         add      ebx,LIGHT_SIZE
         cmp      bx,[all_lights_size]
         jl       .light    ; next_light
         mov      eax,dword .col_b
         stosd
         dec      edi

         inc      cx
         cmp      cx,TEX_X / 2 ;256
         jne      .ie_hor

         inc      dx
         cmp      dx,TEX_Y / 2 ;256
         jne     .ie_ver

         mov     esp,ebp
         pop     ebp
ret



do_color_buffer:         ; do color buffer for Gouraud, flat shading
;env_map 512 x 512 x 3 bytes    ; many lights using
.temp  equ word   [ebp-2]
.nz    equ dword  [ebp-6]  ; dword
.ny    equ dword  [ebp-10]
.nx    equ dword  [ebp-14]
.col_r equ    [ebp-16]
.col_g equ    [ebp-17]
.col_b equ    [ebp-18]

         push     ebp
         mov      ebp,esp
         sub      esp,20
         mov      edi,color_map
         fninit

         mov      dx,- TEX_Y / 2 ;-256   ; dx - vertical coordinate = y
    .ie_ver:
         mov      cx,- TEX_X / 2 ;256   ; cx - horizontal coord = x
    .ie_hor:
         mov      .temp,cx
         fild     .temp
         fidiv    [i256]   ;st = Nx - vector normal x cooficient
         fst      .nx
         fmul     st,st0
         mov      .temp,dx
         fild     .temp
         fidiv    [i256]   ; st = Ny - vector normal y coeficient
         fst      .ny
         fmul     st,st0
         faddp
         fld1
         fchs
         faddp
         fabs
         fsqrt
         fchs
         fstp     .nz              ; st - Nz - vect normal z coeficient
         xor      ebx,ebx
         mov      dword .col_b, 0
     .light:
         push     edi   ;env_map
         lea      esi,[lights+ebx]
         lea      edi,.nx
         call     dot_product
         pop      edi
         fcom     [dot_min]
         fstsw    ax
         sahf
         ja       .env_ok1  ;compare with dot_max
         ffree    st

        jmp       .update_counters
      .env_ok1:
         fcom    [dot_max]
         fstsw   ax
         sahf
         jb      .env_ok2     ; calc col
         ffree   st
         jmp     .update_counters
      .env_ok2:            ;calc col
         push     ebp
         movzx    ax,byte[esi+21]
         push     ax  ;- shines
         mov      al,byte[esi+14]   ; b    orginal color
         push     ax
         mov      al,byte[esi+13]   ; g
         push     ax
         mov      al,byte[esi+12]   ; r
         push     ax
         mov      al,byte[esi+20]   ; b     max color
         push     ax
         mov      al,byte[esi+19]   ; g
         push     ax
         mov      al,byte[esi+18]   ; r
         push     ax
         mov      al,byte[esi+17]   ; b    min col
         push     ax
         mov      al,byte[esi+16]   ; g
         push     ax
         mov      al,byte[esi+15]   ; r
         push     ax
         push     eax         ; earlier - dot pr
      ;  fstp     .dot_product
      ;  push     .dot_product
         call     calc_one_col
         pop      ebp
         ; eax-0x00rrggbb
         cmp      al,.col_b
         jbe      @f
         mov      .col_b,al
   @@:
         shr      ax,8
         cmp      al,.col_g
         jbe      @f
         mov      .col_g,al
   @@:
         shr      eax,16
         cmp      al,.col_r
         jbe      @f
         mov      .col_r,al
  @@:
 .update_counters:                                  ; update and jump when neccesery
        add     ebx,LIGHT_SIZE
        cmp     bx,[all_lights_size]
        jl      .light    ; next_light
        mov     eax,dword .col_b
        stosd
        dec     edi

        inc     cx
        cmp     cx,TEX_X / 2 ;256
        jne     .ie_hor

        inc     dx
        cmp     dx,TEX_X / 2 ;256
        jne     .ie_ver

    .env_done:
         mov     esp,ebp
         pop     ebp
ret

if   Ext >= SSE2
init_point_normals:
;in:
;    esi - tri_ch
;    edi - t_ptr
;    ax =  1 -> old style finding normals
.z equ dword [ebp-8]
.y equ dword [ebp-12]
.x equ [ebp-16]
.point_number equ dword [ebp-28]
.hit_faces    equ dword [ebp-32]
.t_ptr        equ dword [ebp-36]
.tri_ch       equ dword [ebp-40]
.max_val      equ dword [ebp-44]
.mark         equ word  [ebp-45]

        push      ebp
        mov       ebp,esp
        sub       esp,64
        and       ebp,-16
        mov       .t_ptr,edi
        mov       .tri_ch,esi

;        mov       .mark,ax
        bt        ax,0
        jc        .old1


        mov       ecx,[triangles_count_var]
        shl       ecx,3
        lea       ecx,[ecx*3]
        add       ecx,.tri_ch
        mov       .max_val,ecx
        xor       edx,edx

     .lp1:
        mov       ebx,edx
        shl       ebx,2
        add       ebx,.t_ptr
        mov       esi,[ebx]
        or        esi,esi
        jz        .old

        xorps     xmm1,xmm1
        xor       ecx,ecx
     @@:
        mov       eax,[esi+4] ; eax - tri index
        mov       ebx,[esi]
        imul      eax,[i12]
        add       eax,[triangles_normals_ptr]
        movups    xmm0,[eax]
        inc       ecx
        addps     xmm1,xmm0
        add       esi,8
        cmp       esi,.max_val   ; some objects need this check
        ja        .old           ;old method
        cmp       ebx,[esi]
        je        @b

        cvtsi2ss xmm2,ecx
        rcpss    xmm2,xmm2
        shufps   xmm2,xmm2,0
        mulps    xmm1,xmm2
        mov      edi,edx
        imul     edi,[i12]
        add      edi,[points_normals_ptr]
        movlps   [edi],xmm1
        movhlps  xmm1,xmm1
        movss    [edi+8],xmm1
        call      normalize_vector

        inc      edx
        cmp      edx,[points_count_var]
        jnz      .lp1

        jmp      .end

      .old1:

        xor     edx,edx

      .old:

        mov       edi,[points_normals_ptr]
        mov       .point_number,edx
    .ipn_loop:
        movd      xmm0,.point_number
        pshufd    xmm0,xmm0,0
        mov       .hit_faces,0
        mov       .x,dword 0
        mov       .y,0
        mov       .z,0
        mov       esi,[triangles_ptr]
        xor       ecx,ecx              ; ecx - triangle number
    .ipn_check_face:
        movdqu    xmm1,[esi]
        pcmpeqd   xmm1,xmm0
        pmovmskb  eax,xmm1
        and       eax,0xfff
        or        eax,eax
        jz        .ipn_next_face
        push      esi
        mov       esi,ecx
        lea       esi,[esi*3]
        shl       esi,2
        add       esi,[triangles_normals_ptr]
        movups    xmm7,[esi]
        addps     xmm7,.x
        movaps    .x,xmm7
        pop       esi
        inc       .hit_faces

     .ipn_next_face:
        add       esi,12
        inc       ecx
        cmp       ecx,[triangles_count_var]
        jne       .ipn_check_face
        cvtsi2ss  xmm6,.hit_faces
        movaps    xmm7,.x

        rcpss     xmm6,xmm6
        shufps    xmm6,xmm6,11000000b
        mulps     xmm7,xmm6
        movlps    [edi],xmm7
        movhlps   xmm7,xmm7
        movss     [edi+8],xmm7
        call      normalize_vector
    ;    movaps    xmm6,xmm7
    ;    mulps     xmm6,xmm6
    ;    andps     xmm6,[zero_hgst_dd]
    ;    haddps    xmm6,xmm6
    ;    haddps    xmm6,xmm6
    ;    rsqrtps    xmm6,xmm6
    ;    mulps     xmm7,xmm6
    ;    movlps    [edi],xmm7
    ;    movhlps   xmm7,xmm7
    ;    movss     [edi+8],xmm7

        add       edi,12
        inc       .point_number
        mov       edx,.point_number
        cmp       edx,[points_count_var]
        jne       .ipn_loop
 ;       cmp       .mark,1
 ;       je        .end1
 ;        always free if Ext>=SSE2
     .end:

        mov     eax,68
        mov     ebx,13
        mov     ecx,.t_ptr
        int     0x40

        mov     eax,68
        mov     ebx,13
        mov     ecx,.tri_ch
        int     0x40

  ;   .end1:


        add       esp,64
        pop       ebp
ret
else
init_point_normals:
.x equ dword [ebp-4]
.y equ dword [ebp-8]
.z equ dword [ebp-12]
.point_number equ dword [ebp-28]
.hit_faces    equ dword [ebp-32]

        fninit
        mov       ebp,esp
        sub       esp,32
        mov       edi,[points_normals_ptr]
        mov       .point_number,0
    .ipn_loop:
        mov       .hit_faces,0
        mov       .x,0
        mov       .y,0
        mov       .z,0
        mov       esi,[triangles_ptr]
        xor       ecx,ecx              ; ecx - triangle number
    .ipn_check_face:
        xor       ebx,ebx              ; ebx - 'position' in one triangle
    .ipn_check_vertex:
        mov       eax,dword[esi+ebx]    ;  eax - point_number
        cmp       eax,.point_number
        jne       .ipn_next_vertex
        push      esi
        mov       esi,ecx
        lea       esi,[esi*3]
       ; lea       esi,[triangles_normals+esi*4]
        shl       esi,2
        add       esi,[triangles_normals_ptr]

        fld       .x
        fadd      dword[esi+vec_x]       ; vec_x this defined in 3dmath.asm - x cooficient
        fstp      .x                     ; of normal vactor
        fld       .y
        fadd      dword[esi+vec_y]
        fstp      .y
        fld       .z
        fadd      dword[esi+vec_z]
        fstp      .z
        pop       esi
        inc       .hit_faces
        jmp       .ipn_next_face
    .ipn_next_vertex:
        add       ebx,4
        cmp       ebx,12
        jne       .ipn_check_vertex
    .ipn_next_face:
        add       esi,12
        inc       ecx
        cmp       ecx,[triangles_count_var]
        jne       .ipn_check_face

        fld       .x
        fidiv     .hit_faces
        fstp      dword[edi+vec_x]
        fld       .y
        fidiv     .hit_faces
        fstp      dword[edi+vec_y]
        fld       .z
        fidiv     .hit_faces
        fstp      dword[edi+vec_z]
        call      normalize_vector
        add       edi,12  ;type vector 3d
        inc       .point_number
        mov       edx,.point_number
        cmp       edx,[points_count_var]
        jne       .ipn_loop

        mov       esp,ebp
ret
;===============================================================
end if
init_triangles_normals2:
        mov     ebx,[triangles_normals_ptr]
        mov     ebp,[triangles_ptr]
        mov     ecx,[triangles_count_var]
     @@:
        push    ecx
        push    ebx
        mov     ebx,vectors
        mov     esi,dword[ebp]          ; first point index
        lea     esi,[esi*3]
;        lea     esi,[points+esi*2]     ; esi - pointer to 1st 3d point
        shl     esi,2
        add     esi,[points_ptr]
        mov     edi,dword[ebp+4]          ; first point index
        lea     edi,[edi*3]
        shl     edi,2
        add     edi,[points_ptr]
;        movzx   edi,word[ebp+2]        ; second point index
;        lea     edi,[edi*3]
;        lea     edi,[points+edi*2]     ; edi - pointer to 2nd 3d point
        call    make_vector_r
        add     ebx,12
        mov     esi,edi
        mov     edi,dword[ebp+8]        ; third point index
        lea     edi,[edi*3]
        shl     edi,2
        add     edi,[points_ptr]
;        lea     edi,[points+edi*2]
        call    make_vector_r
        mov     edi,ebx                 ; edi - pointer to 2nd vector
        mov     esi,ebx
        sub     esi,12                  ; esi - pointer to 1st vector
        pop     ebx
        call    cross_product
        mov     edi,ebx
        call    normalize_vector
        add     ebp,12
        add     ebx,12
        pop     ecx
        sub     ecx,1
        jnz     @b
ret


copy_lights: ; after normalising !
        mov      esi,lights
        mov      edi,lights_aligned
        mov      ecx,3
       .again:
        push     ecx
        mov      ecx,3
        cld
        rep      movsd
        xor      eax,eax
        stosd
        mov      ecx,3
      .b:
        push     ecx
        mov      ecx,3
      @@:
        movzx    ebx,byte[esi]
        cvtsi2ss xmm0,ebx
        movss    [edi],xmm0
        inc      esi
        add      edi,4
        loop     @b
        stosd
        pop      ecx
        loop     .b
        inc      esi  ; skip  shiness
        pop      ecx
        loop     .again
ret


clrscr:
        mov     edi,[screen_ptr]
        movzx   ecx,word[size_x_var]
        movzx   eax,word[size_y_var]
        imul    ecx,eax
        cld
        xor     eax,eax
  ;    if Ext=NON
        rep     stosd
;      else if Ext = MMX
;        pxor    mm0,mm0
 ;     @@:
 ;       movq    [edi+00],mm0
;        movq    [edi+08],mm0
;        movq    [edi+16],mm0
 ;       movq    [edi+24],mm0
 ;       add     edi,32
 ;       sub     ecx,8
 ;       jnc     @b
 ;     else
 ;       push    ecx
 ;       mov     ecx,edi
 ;       and     ecx,0x0000000f
 ;       rep     stosb
 ;       pop     ecx
 ;       and     ecx,0xfffffff0
 ;       xorps   xmm0,xmm0
 ;     @@:
 ;       movaps  [edi],xmm0
 ;       movaps  [edi+16],xmm0
 ;       movaps  [edi+32],xmm0
 ;       movaps  [edi+48],xmm0
 ;       add     edi,64
 ;       sub     ecx,16
 ;       jnz     @b
 ;     end if

ret


draw_triangles:
;  in:  eax - render draw model
        .tri_no          equ dword[ebp-60]
        .point_index3    equ [ebp-8]
        .point_index2    equ [ebp-12]
        .point_index1    equ [ebp-16]
        .yy3             equ [ebp-18]
        .xx3             equ [ebp-20]
        .yy2             equ [ebp-22]
        .xx2             equ [ebp-24]
        .yy1             equ [ebp-26]
        .xx1             equ [ebp-28]

        .zz3             equ [ebp-30]
        .zz2             equ [ebp-32]
        .zz1             equ [ebp-34]
        .index3x12       equ [ebp-38]
        .index2x12       equ [ebp-42]
        .index1x12       equ [ebp-46]
        .temp1           equ dword[ebp-50]
        .temp2           equ dword[ebp-54]
        .dr_flag         equ word[ebp-56]


        push    ebp
        mov     ebp,esp
        sub     esp,64

 ;       movzx   ax,[dr_flag]
        mov     .dr_flag,ax


        emms
      ;  update translated list  MMX required
        cmp     [vertex_edit_no],-1
        je      @f
        mov     eax,[vertex_edit_no]
      ;  dec     eax
        movd    mm0,[edit_end_x]
        psubw   mm0,[edit_start_x]
        lea     eax,[eax*3]
        add     eax,eax
        add     eax,[points_translated_ptr]
        movd    mm1,dword[eax]
        paddw   mm1,mm0
        movd    dword[eax],mm1
     @@:
     if Ext >= SSE3

        cmp     .dr_flag,13
        jnge    .no_stencil
        mov     esi,[triangles_ptr]
        mov     ecx,[triangles_count_var]
     @@:
        push    esi
        push    ecx

        mov     eax,[esi]
        mov     ebx,[esi+4]
        mov     ecx,[esi+8]
        imul    eax,[i12]
        imul    ebx,[i12]
        imul    ecx,[i12]
        add     eax,[points_rotated_ptr]
        add     ebx,[points_rotated_ptr]
        add     ecx,[points_rotated_ptr]
        push    dword[ecx+8]
        push    dword[ebx+8]
        push    dword[eax+8]
        movups  xmm0,[esp]
        add     esp,12
        andps   xmm0,[zero_hgst_dd]


        mov     eax,[esi]
        mov     ebx,[esi+4]
        mov     ecx,[esi+8]
        shl     eax,1
        shl     ebx,1
        shl     ecx,1
        lea     eax,[eax*3]
        lea     ebx,[ebx*3]
        lea     ecx,[ecx*3]
        add     eax,[points_translated_ptr]
        add     ebx,[points_translated_ptr]
        add     ecx,[points_translated_ptr]
        mov     eax,[eax]
        mov     ebx,[ebx]
        mov     ecx,[ecx]
        ror     eax,16
        ror     ebx,16
        ror     ecx,16


        mov     esi,[Zbuffer_ptr]

        call    stencil_tri

        pop     ecx
        pop     esi
        add     esi,12
        dec     ecx
        jnz     @b

      .no_stencil:
      end if


        cmp     [dr_flag],11
        je      .draw_smooth_line

        mov esi,[triangles_ptr]
        xor ecx,ecx  ;mov ecx,[triangles_count_var]
    .again_dts:
       ; push    ebp
        push    esi
        push    ecx
        mov     .tri_no,ecx

        mov     eax,[esi]
        mov     ebx,[esi+4]
        mov     ecx,[esi+8]

        mov     .point_index1,eax
        mov     .point_index2,ebx
        mov     .point_index3,ecx
        imul    eax,[i12]
        imul    ebx,[i12]
        imul    ecx,[i12]
        mov     .index1x12,eax
        mov     .index2x12,ebx
        mov     .index3x12,ecx

        shr     eax,1
        shr     ebx,1
        shr     ecx,1
        add     eax,[points_translated_ptr]
        add     ebx,[points_translated_ptr]
        add     ecx,[points_translated_ptr]
        push    word[eax+4]
        push    word[ebx+4]
        push    word[ecx+4]
        pop     word .zz3
        pop     word .zz2
        pop     word .zz1

        mov     eax,[eax]
        mov     ebx,[ebx]
        mov     ecx,[ecx]
        ror     eax,16
        ror     ebx,16
        ror     ecx,16
        mov     .xx1,eax
        mov     .xx2,ebx
        mov     .xx3,ecx



      ;  push    esi
        fninit                            ; DO culling AT FIRST
        cmp     [culling_flag],1          ; (if culling_flag = 1)
        jne     .no_culling
        lea     esi,.point_index1          ; *********************************
        mov     ecx,3                     ;
      @@:
        mov     eax,dword[esi]
        lea     eax,[eax*3]
        shl     eax,2
        add     eax,[points_normals_rot_ptr]
        mov     eax,[eax+8]
        bt      eax,31
        jc      @f
                       ; *****************************
                       ; CHECKING OF Z COOFICIENT OF
                       ; NORMAL VECTOR
        add     esi,4
        loop    @b
        jmp     .end_draw   ; non visable
      @@:

      .no_culling:
        cmp     .dr_flag,0               ; draw type flag
        je      .flat_draw
        cmp     .dr_flag,2
        je      .env_mapping
        cmp     .dr_flag,3
        je      .bump_mapping
        cmp     .dr_flag,4
        je      .tex_mapping
        cmp     .dr_flag,5
        je      .rainbow
        cmp     .dr_flag,7
        je      .grd_tex
        cmp     .dr_flag,8
        je      .two_tex
        cmp     .dr_flag,9
        je      .bump_tex
        cmp     .dr_flag,10
        je      .cubic_env_mapping
        cmp     .dr_flag,11
        je      .draw_smooth_line
      if Ext >= SSE3
        cmp     .dr_flag,12
        je      .r_phg
        cmp     .dr_flag,13
        je      .glass
        cmp     .dr_flag,14
        je      .glass_tex
        cmp     .dr_flag,100
        je      .ray_shd

     end if

        push    ebp                    ; ****************
        lea     esi,.index3x12      ; do Gouraud shading
        lea     edi,.zz3
        mov     ecx,3
      .again_grd_draw:
        mov     eax,dword[esi]
        add     eax,[points_normals_rot_ptr]
        ; texture x=(rotated point normal -> x * 255)+255
        fld     dword[eax]       ; x cooficient of normal vector
        fimul   [correct_tex]
        fiadd   [correct_tex]
        fistp   .temp1
        ; texture y=(rotated point normal -> y * 255)+255
        fld     dword[eax+4]      ; y cooficient
        fimul   [correct_tex]
        fiadd   [correct_tex]
        fistp   .temp2

        mov      eax,.temp2
        mov      ebx,.temp1
        and      ebx,0xfffffff
        shl      eax,TEX_SHIFT
        add      eax,ebx
        lea      eax,[eax*3+color_map]
        mov      eax,dword[eax]
        push     word[edi]    ; zz1 ,2 ,3

        ror      eax,16               ; eax -0xxxrrggbb -> 0xggbbxxrr
        xor      ah,ah
        push     ax         ;r
        rol      eax,8                ; eax-0xggbb00rr -> 0xbb00rrgg
        xor      ah,ah
        push     ax         ;g
        shr      eax,24
        push     ax         ;b

        sub      esi,4
        sub      edi,2
        dec      cx
        jnz      .again_grd_draw
        jmp      .both_draw

   .rainbow:
        push     ebp
        push     word .zz3

        mov      eax, .xx3
        ror      eax,16
        mov      ebx,0x00ff00ff
        and      eax,ebx
        push     eax
        neg      al
        push     ax
        push     word .zz2

        mov      eax, .xx2
        ror      eax,16
        and      eax,ebx
        push     eax
        neg      al
        push     ax
        push     word .zz1

        mov      eax, .xx1
        ror      eax,16
        and      eax,ebx
        push     eax
        neg      al
        push     ax
    .both_draw:
        mov     eax, .xx1
        mov     ebx, .xx2
        mov     ecx, .xx3
        mov     edi,[screen_ptr]
        mov     esi,[Zbuffer_ptr]
        call    gouraud_triangle_z
        pop     ebp
        jmp     .end_draw

     .flat_draw:                     ;**************************
        fninit                             ; FLAT DRAWING
        mov     eax,.index1x12
        mov     ebx,.index2x12
        mov     ecx,.index3x12
        add     eax,[points_normals_rot_ptr]
        add     ebx,[points_normals_rot_ptr]
        add     ecx,[points_normals_rot_ptr]
        fld     dword[eax]      ; x cooficient of normal vector
        fadd    dword[ebx]
        fadd    dword[ecx]
        fidiv   [i3]
        fimul   [correct_tex]
        fiadd   [correct_tex]
        fistp   .temp1  ;dword[esp-4]    ; x temp variables
        fld     dword[eax+4]    ; y cooficient of normal vector
        fadd    dword[ebx+4]
        fadd    dword[ecx+4]
        fidiv   [i3]
        fimul   [correct_tex]
        fiadd   [correct_tex]
        fistp   .temp2  ;dword[esp-8]   ;  y
        mov     edx,.temp2 ;dword[esp-8]
        and     edx,0xfffffff
        and     .temp1,0xfffffff
        shl     edx,TEX_SHIFT
        add     edx,.temp1  ;dword[esp-4]

        lea     eax,[3*edx]
        add     eax,color_map
        mov     edx,dword[eax]

        and     edx,0x00ffffff    ; edx = 0x00rrggbb



     ;   mov     ax,[zz1]      ; z position depend draw
     ;   add     ax,[zz2]
     ;   add     ax,[zz3]
     ;   cwd
     ;   idiv    [i3] ;    = -((a+b+c)/3+130)
     ;   add     ax,130
     ;   neg     al
     ;   xor     edx,edx
     ;   mov     ah,al           ;set color according to z position
     ;   shl     eax,8
     ;   mov     edx,eax

        mov     eax,dword .xx1
        mov     ebx,dword .xx2
        mov     ecx,dword .xx3
        mov     edi,[screen_ptr]

        mov     esi,[Zbuffer_ptr]
        push    ebp
        push    word .zz3
        push    word .zz2
        push    word .zz1
        call    flat_triangle_z
        pop     ebp
        jmp     .end_draw

      .env_mapping:
        push    ebp
        push    word .zz3
        push    word .zz2
        push    word .zz1

        lea     esi, .index1x12
        sub     esp,12
        mov     edi,esp
        mov     ecx,3
      @@:
        mov     eax,dword[esi]
        add     eax,[points_normals_rot_ptr]       ;point_normals_rotated
        ; texture x=(rotated point normal -> x * 255)+255
        fld     dword[eax]
        fimul   [correct_tex]
        fiadd   [correct_tex]
        fistp   word[edi]
        ; texture y=(rotated point normal -> y * 255)+255
        fld     dword[eax+4]
        fimul   [correct_tex]
        fiadd   [correct_tex]
        fistp   word[edi+2]

        add     edi,4
        add     esi,4
        loop    @b

        mov     eax, .xx1
        mov     ebx,dword .xx2
        mov     ecx,dword .xx3
        mov     edi,[screen_ptr]
        mov     esi,envmap

        mov     edx,[Zbuffer_ptr]
        call    tex_triangle_z
        pop     ebp
        jmp     .end_draw
;+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     .cubic_env_mapping:
        push    ebp
        push    word .zz3
        push    word .zz2
        push    word .zz1

        lea     esi,.index1x12
        sub     esp,12
        mov     edi,esp
        mov     ecx,3
      @@:
        mov     eax,dword[esi]
        add     eax,[points_normals_rot_ptr]

        fld     dword[eax]
        fmul    dword[eax+4]
        fld1
        fld1
        faddp
        fmulp
        fimul   [correct_tex]
        fiadd   [correct_tex]
        fistp   word[edi]
        mov     word[edi+2],0
;        fistp   word[edi+2]
; # last change
;        ; texture x=(rotated point normal -> x * 255)+255
;        fld     dword[eax]
;        fimul   [correct_tex]
;        fiadd   [correct_tex]
;        fistp   word[edi]
;        ; texture y=(rotated point normal -> y * 255)+255
;        fld     dword[eax+4]
;        fimul   [correct_tex]
;        fiadd   [correct_tex]
;        fistp   word[edi+2]
; # end of last ch.
        add     edi,4
        add     esi,4
        loop    @b

        mov     eax, .xx1
        mov     ebx, .xx2
        mov     ecx, .xx3
        mov     edi,[screen_ptr]
        mov     esi,envmap_cub
        mov     edx,[Zbuffer_ptr]

        call    tex_triangle_z
        pop     ebp
        jmp     .end_draw

;+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

      .bump_mapping:
        push    ebp
        push    [Zbuffer_ptr]
        push    word .zz3
        push    word .zz2
        push    word .zz1

        lea     esi,.index1x12
        sub     esp,12
        mov     edi,esp
        mov     ecx,3
      @@:
        mov     eax,dword[esi]
        add     eax,[points_normals_rot_ptr]  ;point_normals_rotated
        ; texture x=(rotated point normal -> x * 255)+255
        fld     dword[eax]
        fimul   [correct_tex]
        fiadd   [correct_tex]
        fistp   word[edi]
        ; texture y=(rotated point normal -> y * 255)+255
        fld     dword[eax+4]
        fimul   [correct_tex]
        fiadd   [correct_tex]
        fistp   word[edi+2]

        add     edi,4
        add     esi,4
        loop    @b

        mov    esi, .point_index3      ; bump map coords
        shl    esi,2
        add    esi,[tex_points_ptr]
        push   dword[esi]
        mov    esi, .point_index2
        shl    esi,2
        add    esi,[tex_points_ptr]
        push   dword[esi]
        mov    esi, .point_index1
        shl    esi,2
        add    esi,[tex_points_ptr]
        push   dword[esi]

        mov     eax,dword .xx1
        mov     ebx,dword .xx2
        mov     ecx,dword .xx3
        mov     edi,[screen_ptr]
        mov     esi,envmap
        mov     edx,bumpmap            ;BUMP_MAPPING

        call    bump_triangle_z
        pop     ebp
        jmp     .end_draw

      .tex_mapping:
        push   ebp
        push   word .zz3
        push   word .zz2
        push   word .zz1
   ;   @@:
        mov    esi, .point_index3      ; tex map coords
        shl    esi,2
        add    esi,[tex_points_ptr]
        push   dword[esi]
        mov    esi, .point_index2
        shl    esi,2
        add    esi,[tex_points_ptr]
        push   dword[esi]
        mov    esi, .point_index1
        shl    esi,2
        add    esi,[tex_points_ptr]
        push   dword[esi]

        mov     eax,dword .xx1
        mov     ebx,dword .xx2
        mov     ecx,dword .xx3
        mov     edi,[screen_ptr]
        mov     esi,texmap
        mov     edx,[Zbuffer_ptr]

        call    tex_triangle_z
        pop     ebp
        jmp     .end_draw
;      .ray:
;        grd_triangle according to points index
;        cmp     [catmull_flag],0
;        je      @f
;        push    [zz3]                   ; spot light with attenuation
;     @@:
;        movzx   eax,[point_index3]      ; env_map - points color list
;        shl     eax,1                   ; each color as word, 0x00rr00gg00bb..
;        lea     eax,[3*eax+bumpmap]
;        push    word[eax]
;        push    word[eax+2]
;        push    word[eax+4]
;        cmp     [catmull_flag],0
;        je      @f
;        push    [zz2]
;    @@:
;        movzx   eax,[point_index2]      ; env_map - points color list
;        shl     eax,1                   ; each color as word, 0x00rr00gg00bb..
;        lea     eax,[eax*3+bumpmap]
;        push    word[eax]
;        push    word[eax+2]
;        push    word[eax+4]
;        cmp     [catmull_flag],0
;        je      @f
;        push    [zz1]
;     @@:
;        movzx   eax,[point_index1]      ; env_map - points color list
;        shl     eax,1                   ; each color as word, 0xrr00gg00bb00..
;        lea     eax,[eax*3+bumpmap]
;        push    word[eax]
;        push    word[eax+2]
;        push    word[eax+4]
;        jmp     .both_draw

     .grd_tex:            ; smooth shading + texture
         push   ebp

         mov    esi, .point_index3      ; tex map coords
         shl    esi,2
         add    esi,[tex_points_ptr]
         push   dword[esi]              ; texture coords as first
         mov    esi, .point_index2      ; group of parameters
         shl    esi,2
         add    esi,[tex_points_ptr]
         push   dword[esi]
         mov    esi, .point_index1
         shl    esi,2
         add    esi,[tex_points_ptr]
         push   dword[esi]

         lea     esi, .index3x12
         lea     edi, .zz3
         mov     ecx,3
       .aagain_grd_draw:

        push     word[edi]    ; zz1 ,2 ,3
        fninit
        mov     eax,dword[esi]
        add     eax,[points_normals_rot_ptr]
        ; texture x=(rotated point normal -> x * 255)+255
        fld     dword[eax]       ; x cooficient of normal vector
        fimul   [correct_tex]
        fiadd   [correct_tex]
        fistp   .temp1  ;word[ebp-2]
        ; texture y=(rotated point normal -> y * 255)+255
        fld     dword[eax+4]      ; y cooficient
        fimul   [correct_tex]
        fiadd   [correct_tex]
        fistp   .temp2  ;word[ebp-4]

        mov      eax,.temp2
        mov      ebx,.temp1
        and      ebx,0xfffffff ; some onjects need this 'and'
        shl      eax,TEX_SHIFT
        add      eax,ebx
        lea      eax,[eax*3]
        add      eax,color_map
        mov      eax,dword[eax]

        ror      eax,16               ; eax -0xxxrrggbb -> 0xggbbxxrr
        xor      ah,ah
        push     ax         ;r
        rol      eax,8                ; eax-0xggbb00rr -> 0xbb00rrgg
        xor      ah,ah
        push     ax         ;g
        shr      eax,24
        push     ax         ;b
        sub      edi,2
        sub      esi,4
        dec      cx
        jnz      .aagain_grd_draw

        mov     eax, .xx1
        mov     ebx, .xx2
        mov     ecx, .xx3
        mov     edi,[screen_ptr]
        mov     edx,texmap
        mov     esi,[Zbuffer_ptr]

        call    tex_plus_grd_triangle

        pop     ebp
        jmp     .end_draw

      .two_tex:
        push     ebp
        push    [Zbuffer_ptr]

        push    word .zz3
        push    word .zz2
        push    word .zz1

        fninit
        lea     esi, .point_index3     ; env coords
        mov     edi,esp
        sub     esp,24
        mov     ecx,3
      @@:
        mov     eax,dword[esi]
        shl     eax,2
        mov     ebx,eax
  ;      mov     ebx,eax
        add     ebx,[tex_points_ptr]
        mov     ebx,[ebx]
        mov     [edi-8],ebx
        lea     eax,[eax*3]
        add     eax,[points_normals_rot_ptr]
        ; texture x=(rotated point normal -> x * 255)+255
        fld     dword[eax]
        fimul   [correct_tex]
        fiadd   [correct_tex]
        fistp   word[edi-4]
        and     word[edi-4],0x7fff   ; some objects need it
        ; texture y=(rotated point normal -> y * 255)+255
        fld     dword[eax+4]
        fimul   [correct_tex]
        fiadd   [correct_tex]
        fistp   word[edi-2]
        and     word[edi-2],0x7fff  ; some objects need it

        sub     edi,8
        sub     esi,4
        loop    @b

        mov     eax, .xx1
        mov     ebx, .xx2
        mov     ecx, .xx3
        mov     edi,[screen_ptr]
        mov     esi,texmap
        mov     edx,envmap

        call    two_tex_triangle_z
        pop     ebp
        jmp     .end_draw

   .bump_tex:
        push   ebp
        fninit

        push  dword texmap

        push  [Zbuffer_ptr]

        push    word .zz3
        push    word .zz2
        push    word .zz1


        lea     ebx, .point_index1
        sub     esp,36
        mov     edi,esp
        mov     ecx,3
      @@:
        mov     eax,[ebx]
        shl     eax,2
        mov     esi,eax
        lea     esi,[esi*3]
        add     eax,[tex_points_ptr]
        mov     eax,[eax]
        ror     eax,16
        mov     [edi],eax
        mov     [edi+8],eax

        add     esi,[points_normals_rot_ptr]
        ; texture x=(rotated point normal -> x * 255)+255
        fld     dword[esi]
        fimul   [correct_tex]
        fiadd   [correct_tex]
        fistp   word[edi+6]             ; env coords
        ; texture y=(rotated point normal -> y * 255)+255
        fld     dword[esi+4]
        fimul   [correct_tex]
        fiadd   [correct_tex]
        fistp   word[edi+4]
        add     ebx,4
        add     edi,12
        loop    @b

        mov     eax,dword .xx1
        mov     ebx,dword .xx2
        mov     ecx,dword .xx3
        mov     edi,[screen_ptr]
        mov     esi,envmap
        mov     edx,bumpmap

        call bump_tex_triangle_z
        pop     ebp
        jmp     .end_draw


if Ext >= SSE3
     .r_phg:


        movd      xmm5,[size_y_var]
        punpcklwd xmm5,[the_zero]
        pshufd    xmm5,xmm5,01110011b


        mov     eax, .index1x12
        mov     ebx, .index2x12
        mov     ecx, .index3x12
        add     eax,[points_normals_rot_ptr]
        add     ebx,[points_normals_rot_ptr]
        add     ecx,[points_normals_rot_ptr]
        movups  xmm0,[eax]
        movups  xmm1,[ebx]
        movups  xmm2,[ecx]
        andps   xmm0,[zero_hgst_dd]
        andps   xmm1,[zero_hgst_dd]
        andps   xmm2,[zero_hgst_dd]
        xorps   xmm3,xmm3

        mov     eax, .index1x12
        mov     ebx, .index2x12
        mov     ecx, .index3x12
        add     eax,[points_rotated_ptr]
        add     ebx,[points_rotated_ptr]
        add     ecx,[points_rotated_ptr]
        push    dword[ecx+8]
        push    dword[ebx+8]
        push    dword[eax+8]
        movups  xmm4,[esp]
        add     esp,12
        andps   xmm4,[zero_hgst_dd]



        mov     eax,dword .xx1
        mov     ebx,dword .xx2
        mov     ecx,dword .xx3
        mov     edi,[screen_ptr]
        mov     esi,[Zbuffer_ptr]

        call    real_phong_tri_z

        jmp     .end_draw

     .glass:
        movd      xmm5,[size_y_var]
        punpcklwd xmm5,[the_zero]
        pshufd    xmm5,xmm5,01110011b


        mov     eax, .index1x12
        mov     ebx, .index2x12
        mov     ecx, .index3x12
        add     eax,[points_normals_rot_ptr]
        add     ebx,[points_normals_rot_ptr]
        add     ecx,[points_normals_rot_ptr]
        movups  xmm0,[eax]
        movups  xmm1,[ebx]
        movups  xmm2,[ecx]
        andps   xmm0,[zero_hgst_dd]
        andps   xmm1,[zero_hgst_dd]
        andps   xmm2,[zero_hgst_dd]
        xorps   xmm3,xmm3

        mov     eax, .index1x12
        mov     ebx, .index2x12
        mov     ecx, .index3x12
        add     eax,[points_rotated_ptr]
        add     ebx,[points_rotated_ptr]
        add     ecx,[points_rotated_ptr]
        push    dword[ecx+8]
        push    dword[ebx+8]
        push    dword[eax+8]
        movups  xmm4,[esp]
        add     esp,12
        andps   xmm4,[zero_hgst_dd]



        mov     eax, .xx1
        mov     ebx, .xx2
        mov     ecx, .xx3
        mov     edi,[screen_ptr]
        mov     edx,[Zbuffer_ptr]
        mov     esi,[Zbuffer_ptr]

        call    glass_tri
        jmp     .end_draw


   .glass_tex:
        movd      xmm5,[size_y_var]
        punpcklwd xmm5,[the_zero]
        pshufd    xmm5,xmm5,01110011b

        mov     eax, .index1x12
        mov     ebx, .index2x12
        mov     ecx, .index3x12
        add     eax,[points_normals_rot_ptr]
        add     ebx,[points_normals_rot_ptr]
        add     ecx,[points_normals_rot_ptr]
        movups  xmm0,[eax]
        movups  xmm1,[ebx]
        movups  xmm2,[ecx]
        andps   xmm0,[zero_hgst_dd]
        andps   xmm1,[zero_hgst_dd]
        andps   xmm2,[zero_hgst_dd]
        xorps   xmm3,xmm3

        mov     eax, .index1x12
        mov     ebx, .index2x12
        mov     ecx, .index3x12
        add     eax,[points_rotated_ptr]
        add     ebx,[points_rotated_ptr]
        add     ecx,[points_rotated_ptr]
        push    dword[ecx+8]
        push    dword[ebx+8]
        push    dword[eax+8]
        movups  xmm4,[esp]
        add     esp,12
        andps   xmm4,[zero_hgst_dd]

        mov    esi,.point_index3      ; tex map coords
        shl    esi,2
        add    esi,[tex_points_ptr]
        push   dword[esi]
        mov    esi,.point_index2
        shl    esi,2
        add    esi,[tex_points_ptr]
        push   dword[esi]
        mov    esi,.point_index1
        shl    esi,2
        add    esi,[tex_points_ptr]
        push   dword[esi]
        movups xmm6,[esp]
        add    esp,12
     ;   pshuflw xmm6,xmm6,10110001b
     ;   pshufhw xmm6,xmm6,10110001b

        movzx  eax,word[size_x_var]
        andps  xmm6,[zero_hgst_dd]
        movd   xmm7,eax
        pslldq xmm7,12
        por    xmm6,xmm7


        mov     eax,dword .xx1
        mov     ebx,dword .xx2
        mov     ecx,dword .xx3
        mov     edx,texmap
        mov     edi,[screen_ptr]
        mov     esi,[Zbuffer_ptr]

        call    glass_tex_tri
        jmp     .end_draw

   .ray_shd:
        emms
        movd      xmm5,[size_y_var]
        punpcklwd xmm5,[the_zero]
        pshufd    xmm5,xmm5,01110011b

        mov     eax, .index1x12
        mov     ebx, .index2x12
        mov     ecx, .index3x12
        add     eax,[points_normals_rot_ptr]
        add     ebx,[points_normals_rot_ptr]
        add     ecx,[points_normals_rot_ptr]
        movups  xmm0,[eax]
        movups  xmm1,[ebx]
        movups  xmm2,[ecx]
        andps   xmm0,[zero_hgst_dd]
        andps   xmm1,[zero_hgst_dd]
        andps   xmm2,[zero_hgst_dd]
        xorps   xmm3,xmm3

   ;     mov     ebx,.tri_no
   ;     cmp     ebx,0
   ;     je      @f
   ;     int3
   ;   @@:

        mov     eax, .index1x12
        mov     ebx, .index2x12
        mov     ecx, .index3x12
        add     eax,[points_rotated_ptr]
        add     ebx,[points_rotated_ptr]
        add     ecx,[points_rotated_ptr]
        push    dword[ecx+8]
        push    dword[ebx+8]
        push    dword[eax+8]
        movups  xmm4,[esp]
        add     esp,12
        andps   xmm4,[zero_hgst_dd]

        movd    mm7,.tri_no

   ;     mm7 - intialised


        mov     eax,dword .xx1
        mov     ebx,dword .xx2
        mov     ecx,dword .xx3
        mov     edx,texmap
        mov     edi,[screen_ptr]
        mov     esi,[Zbuffer_ptr]

        call    ray_shad



end if

      .end_draw:
    ;    pop     ebp
        pop     ecx
        pop     esi

        add     esi,12
        inc     ecx
        cmp     ecx,[triangles_count_var]
        jnz     .again_dts

        jmp     .eend



   .draw_smooth_line:
        mov     esi,[edges_ptr]
        xor     ecx,ecx
    .again_s_line:
        push    ecx
        push    esi

        mov     ecx,2
     .aga_n:
        mov     eax,[esi]
        shl     eax,2
        lea     eax,[eax*3]
        add     eax,[points_normals_rot_ptr]
        movups   xmm0,[eax]
        xorps    xmm1,xmm1
        mov      edx,lights_aligned
    .again_cc:
        movaps    xmm3,xmm0   ;.cnv
        mulps     xmm3,[edx]
        andps     xmm3,[zero_hgst_dd]
   ;     haddps    xmm3,xmm3
   ;     haddps    xmm3,xmm3   ; xmm3 - dot pr
        movhlps   xmm2,xmm3
        addps     xmm3,xmm2
        movdqa    xmm2,xmm3
        shufps    xmm2,xmm2,01010101b
        addps     xmm3,xmm2
        shufps    xmm3,xmm3,0
        movaps    xmm6,xmm3   ;xmm7
        mulps     xmm6,xmm6
        mulps     xmm6,xmm6
        mulps     xmm6,xmm6
        mulps     xmm6,xmm6
        mulps     xmm6,xmm6
        mulps     xmm6,[edx+48]
        movaps    xmm7,xmm3
        mulps     xmm7,[edx+16]
        addps     xmm7,xmm6
        addps     xmm7,[edx+32]
        minps     xmm7,[mask_255f]   ; global

        maxps     xmm1,xmm7
        add       edx,64     ; size of one light in aligned list
        cmp       edx,lights_aligned_end
        jl        .again_cc
        sub       esp,16
        movups    [esp],xmm1
        add       esi,4
        dec       ecx
        jnz      .aga_n

        movups    xmm0,[esp]
        movups    xmm1,[esp+16]
        add       esp,32
        sub       esi,8

        mov     ecx,[esi]
        mov     edx,[esi+4]
        imul    ecx,[i6]
        imul    edx,[i6]
        add     ecx,[points_translated_ptr]
        add     edx,[points_translated_ptr]

        movd    xmm7,[ecx]
        movhps  xmm7,[edx]
        pshufd  xmm7,xmm7,11101000b
        movdqa  xmm6,xmm7
        movdqa  xmm3,xmm7
        movdqa  xmm4,xmm7
        movd    xmm5,[size_y_var]
        pshuflw xmm5,xmm5,00010001b
        pcmpeqw xmm3,xmm5
        pcmpeqw xmm4,[the_zero]
        pcmpgtw xmm7,xmm5
        pcmpgtw xmm6,[the_zero]
        pxor    xmm7,xmm6
        pxor    xmm3,xmm4
        pxor    xmm7,xmm3
        pmovmskb eax,xmm7
        cmp     al,-1
        jnz      .skp




        mov     eax,[esi]
        mov     ebx,[esi+4]
        imul    eax,[i12]
        imul    ebx,[i12]
        add     eax,[points_rotated_ptr]
        add     ebx,[points_rotated_ptr]
        movss   xmm2,[eax+8]
        movss   xmm3,[ebx+8]


        movzx   eax,word[ecx]
        movzx   ebx,word[ecx+2]
        movzx   ecx,word[edx]
        movzx   edx,word[edx+2]

        emms
        movd    mm1,[screen_ptr]
        movd    mm0,[Zbuffer_ptr]
        movzx   esi,word[size_x_var]
        movd    mm2,esi

        push    ebp
        call    line_grd
        pop     ebp
      .skp:
        pop     esi
        pop     ecx
        add     esi,8
        inc     ecx
        cmp     ecx,[edges_count]
        jnz     .again_s_line






   .eend:
        add      esp,64
        pop      ebp

ret


draw_handlers:

       ;  in eax - render model
        push  ebp
        mov   ebp,esp
        emms
       .fac         equ  dword[ebp-16]
       .xplus_scr    equ ebp-8
       .xplus_index  equ ebp-12
       .dr_model equ dword[ebp-4]

       sub     esp,16
       mov     .dr_model,eax

       movzx   eax,word[size_x_var]
       cmp    .dr_model,10
       jg     @f
       lea    ebx,[eax*3]
       sub    ebx,3*6
       mov    [.xplus_scr],ebx   ; for scr 1st cause
       mov    .fac,3
       jmp    .in_r
    @@:
       lea    ebx,[eax*4]       ; for scr 2cond cause
       sub    ebx,4*6
       mov    [.xplus_scr],ebx
       mov    .fac,4
     .in_r:

       lea    ebx,[eax*4]
       sub    ebx,4*6
       mov    [.xplus_index],ebx  ; index

       xor    ecx,ecx
       mov    eax,4 shl 16 + 4
       movd   xmm0,[size_y_var]
       movd   xmm1,eax
       psubw  xmm0,xmm1
       pshuflw xmm0,xmm0,00000001b

     .l:
       push    ecx
       cmp     [culling_flag],1         ; (if culling_flag = 1)
       jne     .no_culling
       mov     edi,ecx           ; *********************************
       lea     edi,[edi*3]
       shl     edi,2
       add     edi,[points_normals_rot_ptr]
       bt      dword[edi+8],31
       jnc     .skip
     .no_culling:
       mov     esi,ecx
       lea     esi,[esi*3]
       add     esi,esi
       add     esi,[points_translated_ptr]
       movd    xmm2,[esi]
       movd    xmm3,[esi]
       pcmpgtw xmm2,xmm0
       pcmpgtw xmm3,xmm1
       pxor    xmm3,xmm2
       movd     eax,xmm3
       cmp      eax,-1
       jne      .skip

       movzx   eax,word[esi]
       movzx   ebx,word[esi+2]
       sub     eax,2
       sub     ebx,2
       movzx   edx, word[size_x_var]
       imul    ebx,edx
       add     ebx,eax
       mov     edi,ebx
       imul    ebx,.fac
       shl     edi,2
       add     ebx,[screen_ptr]
       add     edi,[vertices_index_ptr]
       mov     eax,ecx
       cld
       mov     ecx,6
    .l2:
       push    ecx
       mov     ecx,6       ; draw bar
    .l1:
       mov     word[ebx],0
       mov     byte[ebx+2],0xff
       stosd
       add     ebx,.fac
       loop    .l1
       add     ebx,[.xplus_scr]
       add     edi,[.xplus_index]
       pop     ecx
       loop    .l2
     .skip:
       pop      ecx
       inc      ecx
       cmp      ecx,[points_count_var]
       jna      .l

       mov      esp,ebp
       pop      ebp
ret



fill_Z_buffer:
        mov     eax,0x70000000
        cmp     [dr_flag],11
        jl      @f
        mov     eax,60000.1
    @@:
        mov     edi,[Zbuffer_ptr]
        movzx   ecx,word[size_x_var]
        movzx   ebx,word[size_y_var]
        imul    ecx,ebx
      if     Ext>=SSE2
        movd    xmm0,eax
        shufps  xmm0,xmm0,0
        push    ecx
        mov     ecx,edi
        and     edi,0xffffff00
        and     ecx,0x000000ff
        mov     edx,ecx
        rep     stosd
        pop     ecx
        sub     ecx,edx
      @@:
        movaps  [edi],xmm0
        movaps  [edi+16],xmm0
        movaps  [edi+32],xmm0
        movaps  [edi+48],xmm0
        add     edi,64
        sub     ecx,16
        jnc     @b
      else
        rep     stosd
      end if
ret

read_tp_variables:            ; read [triangles_count_var] and  [points_count_var]
                              ; and  allocate memory
        xor     ebx,ebx
        xor     ebp,ebp
        mov     [points_count_var],ebx
        mov     [triangles_count_var],ebx
        mov     esi,[fptr]

        cmp     [esi],word 4D4Dh
        je      @f ;Must be legal .3DS file
        xor     eax,eax
        ret
    @@:
        mov     eax,dword[esi+2]
        cmp     eax,[fsize] ;This must tell the length
        je      @f
        xor     eax,eax
        ret
     @@:
        add     eax,esi
        mov     [EndFile],eax    ;

        add     esi,6
      @@:
        cmp     [esi],word 3D3Dh
        je      @f
        add     esi,[esi+2]
        jmp     @b
      @@:
        add     esi,6
      .find4k:
        cmp     [esi],word 4000h
        je      @f
        add     esi,[esi+2]
        cmp     esi,[EndFile]
        jc      .find4k
        jmp     .exit
      @@:
        add     esi,6
      @@:
        cmp     [esi],byte 0
        je      @f
        inc     esi
        jmp     @b
      @@:
        inc     esi
      @@:
        cmp     [esi],word 4100h
        je      @f
        add     esi,[esi+2]
        jmp     @b
      @@:
        add     esi,6
      @@:
        cmp     [esi],word 4110h
        je      @f
        add     esi,[esi+2]
        jmp     @b
      @@:
        movzx   ecx,word[esi+6]
        add     [points_count_var],ecx

        mov     edx,ecx
        add     esi,8
     @@:
        lea     ecx,[ecx*3]
        add     ecx,ecx
        add     ebx,ecx
        add     ecx,ecx
        add     esi,ecx
     ;   dec     ecx
      ;  loop     @b
      @@:

      @@:
        cmp     [esi],word 4120h
        je      @f
        add     esi,[esi+2]
        jmp     @b
      @@:
        movzx   ecx,word[esi+6]
        add     [triangles_count_var],ecx
        add     esi,8

      @@:
        shl     ecx,3
        add     esi,ecx
       ; dec     ecx
       ; jnz     @b
       ; loop    @b
;        xor     ecx,ecx
        add     ebp,edx
        jmp     .find4k
        mov     eax,-1 ;<---mark if OK
      .exit:
ret

read_from_file:
        fninit
        mov     edi,[triangles_ptr]
        xor     ebx,ebx
        xor     ebp,ebp
        mov     [points_count_var],0
        mov     [triangles_count_var],0
        mov     esi,[fptr]
        cmp     [esi],word 4D4Dh
        jne     .exit ;Must be legal .3DS file
;        cmp     dword[esi+2],EndFile-SourceFile
;        jne     .exit ;This must tell the length
        mov     eax,dword[esi+2]
  ;      cmp     eax,[fsize]
  ;      jne     .exit

        add     eax,esi
        mov     [EndFile],eax    ;

        add     esi,6
      @@:
        cmp     [esi],word 3D3Dh
        je      @f
        add     esi,[esi+2]
        jmp     @b
      @@:
        add     esi,6
      .find4k:
        cmp     [esi],word 4000h
        je      @f
        add     esi,[esi+2]
        cmp     esi,[EndFile]
        jc      .find4k
        jmp     .exit
      @@:
        add     esi,6
      @@:
        cmp     [esi],byte 0
        je      @f
        inc     esi
        jmp     @b
      @@:
        inc     esi
      @@:
        cmp     [esi],word 4100h
        je      @f
        add     esi,[esi+2]
        jmp     @b
      @@:
        add     esi,6
      @@:
        cmp     [esi],word 4110h
        je      @f
        add     esi,[esi+2]
        jmp     @b
      @@:
        movzx   ecx,word[esi+6]
        add     [points_count_var],ecx

        mov     edx,ecx
        add     esi,8
     @@:
        push    edi
        mov     edi,[points_ptr]
        push    dword[esi+0]
        pop     dword[edi+ebx*2+0]
        push    dword[esi+4]
        pop     dword[edi+ebx*2+4]
        push    dword[esi+8]
        pop     dword[edi+ebx*2+8]
        pop     edi

        add     ebx,6
        add     esi,12
       ; dec     ecx
       ; jnz     @b
        loop    @b
      @@:
  ;      mov     dword[points+ebx],-1
        push    edi
        mov     edi,[points_ptr]
        mov     dword[edi+ebx*2],-1        ; end mark (not always in use)
        pop     edi
      @@:
        cmp     [esi],word 4120h
        je      @f
        add     esi,[esi+2]
        jmp     @b
      @@:
        movzx   ecx,word[esi+6]
        add     [triangles_count_var],ecx
        add     esi,8
        ;mov     edi,triangles
      @@:
        movzx   eax,word[esi]
        stosd
        movzx   eax,word[esi+2]
        stosd
        movzx   eax,word[esi+4]
        stosd
        add     dword[edi-12],ebp
        add     dword[edi-8],ebp
        add     dword[edi-4],ebp
        add     esi,8
    ;    dec     ecx
    ;    jnz     @b
        loop    @b
        add     ebp,edx
        jmp     .find4k
        mov     eax,-1 ;<---mark if OK
      .exit:
        mov     dword[edi],-1
ret


alloc_mem_for_tp:
;in:        bl = 1 - realloc
        mov     eax, 68
        cmp     bl,1
        jz      @f
        mov     ebx, 12
        jmp     .alloc
    @@:
        mov     ebx,20
    .alloc:

        mov     ecx,[triangles_count_var]
        add     ecx,20
        lea     ecx, [ecx*3]
        shl     ecx,2
        mov     edx,[triangles_ptr]
        int     0x40                   ;  -> allocate memory to triangles
        mov     [triangles_ptr], eax   ;  -> eax = pointer to allocated mem

;        mov     eax, 68
;        mov     ecx,[triangles_count_var]
;        imul    ecx,[i36]
;        mov     edx,[edges_ptr]
;        int     0x40                   ;  -> allocate memory to triangles
;        mov     [edges_ptr], eax   ;  -> eax = pointer to allocated mem


                                            ; ststic  memory

        mov     eax, 68
        mov     ecx, [triangles_count_var]
        lea     ecx, [6+ecx*3]
        shl     ecx, 2
        mov     edx,[triangles_normals_ptr]
        int     0x40                           ;  -> allocate memory for triangles normals
        mov     [triangles_normals_ptr], eax   ;  -> eax = pointer to allocated mem


        mov     eax, 68
        mov     ecx, [points_count_var]
        lea     ecx,[6+ecx*3]
        shl     ecx, 2
        mov     edx,[points_normals_ptr]
        int     0x40
        mov     [points_normals_ptr], eax


        mov     eax, 68
    ;    mov     ebx, 12
        mov     ecx, [points_count_var]
        lea     ecx,[10+ecx*3]
        shl     ecx, 2
        mov     edx,[points_normals_rot_ptr]
        int     0x40
        mov     [points_normals_rot_ptr], eax

        mov     eax, 68

        mov     edx,[points_ptr]
        int     0x40
        mov     [points_ptr], eax

        mov     eax, 68
        mov     edx,[points_rotated_ptr]
        int     0x40
        mov     [points_rotated_ptr], eax

        mov     eax, 68
 ;       mov     ebx, 12
        mov     ecx, [points_count_var]
        shl     ecx,2
        add     ecx,32
        mov     edx,[tex_points_ptr]
        int     0x40
        mov     [tex_points_ptr], eax

        mov     eax, 68
        mov     ecx, [points_count_var]
        add     ecx,10
        shl     ecx, 3
        mov     edx,[points_translated_ptr]
        int     0x40
        mov     [points_translated_ptr], eax
ret


read_from_disk:
    mov     eax, 68
    mov     ebx, 11
    int     0x40                   ;  -> create heap

    ;load kpacked files by Leency
       mov     eax,68
       mov     ebx,27
       mov     ecx,file_name
       int     0x40

       mov     [fsize],edx
       mov     [file_info+16],eax

       test    eax,eax
       jnz     .open_opened_well
       mov     eax,6 ;otherwise => failed
       jmp     @f
       .open_opened_well:
       xor     eax,eax
    @@:
  ;  eax = 0   -> ok file loaded
ret
read_param:
    cld
    mov        esi,I_Param
    cmp        dword[esi],0
    je         .end
    cmp        byte[esi],'/'
    je         .copy
    mov        edi,esi
    mov        ecx,25   ; 25 - would be enought
    repe       scasb
    jne        .end
    dec        edi
    mov        esi,edi
 .copy:
    mov         edi,file_name
    mov         ecx,50
    rep         movsd
 .end:
ret
buttons:                                      ; draw some buttons (all but navigation and close )
        mov     edi,menu
      .again:
        mov     eax,8             ; function 8 : define and draw button
        movzx   ebx,word[size_x_var]
        shl     ebx,16
        add     ebx,(10)*65536+62      ; [x start] *65536 + [x size]
        movzx   ecx,byte[edi]                 ; button id = position+2
        sub     ecx,2
        lea     ecx,[ecx*5]
        lea     ecx,[ecx*3]
        add     ecx,25
        shl     ecx,16
        add     ecx,12
        movzx   edx,byte[edi]                   ; button id
        mov     esi,0x6688dd                    ; button color RRGGBB
        int     0x40
         ; BUTTON  LABEL
        mov     eax,4                           ; function 4 : write text to window
        movzx   ebx,byte[edi]
        sub     ebx,2                            ; button id, according to position
        lea     ebx,[ebx*3]
        lea     ebx,[ebx*5]
        movzx   ecx,word[size_x_var]
        shl     ecx,16
        add     ebx,ecx
        add     ebx,(12)*65536+28        ; [x start] *65536 + [y start]
        mov     ecx,0x00ddeeff                  ; font 1 & color ( 0xF0RRGGBB )
        lea     edx,[edi+1]                     ; pointer to text beginning
        mov     esi,10                          ; text length
        int     0x40
        cmp     byte[edi+11],255                ; if max_flag=255
        je      @f                              ; skip
        ; flag description
;       mov     eax,4                           ; function 4 : write text to window
;       movzx   ebx,byte[edi]
;       sub     bl,2
;       lea     ebx,[ebx*3]
;       lea     ebx,[ebx*5]
;       add     ebx,(SIZE_X+12+70)*65536+28     ; [x start] *65536 + [y start]
        add     ebx,70*65536
;       mov     ecx,0x00ddeeff                  ; font 1 & color ( 0xF0RRGGBB )
        movzx   edx,byte[edi+12]                ; current flag
        shl     edx,2                           ; * 4 = text length
        add     edx,dword[edi+13]               ; pointer to text beginning
        mov     esi,4                           ; text length
        int     0x40

    @@:
        add     edi,17
        cmp     byte[edi],-1
        jnz     .again
ret
write_info:
        mov     eax,13
        mov     bx,[size_x_var]
        shl     ebx,16
        add     ebx,120*65536+70   ; [x start] *65536 + [y start]
        mov     ecx,30 shl 16 + 150
        xor     edx,edx
        int     0x40


        xor     esi,esi
        emms
        movd    mm7,esi
     .nxxx:
        push    esi
        movd    mm7,esi

        mov     eax,4           ; function 4 : write text to window
        movzx   ebx,word[size_x_var]
        shl     ebx,16
        add     ebx,120*65536   ; [x start] *65536 + [y start]
        mov     bx,si
        shl     bx,3
        add     ebx,30
        mov     ecx,0x00ddeeff  ; font 1 & color ( 0xF0RRGGBB )
        movd    edx,mm7
        shl     edx,2
        add     edx,lab_vert      ; pointer to text beginning
        mov     esi,lab_vert_end-lab_vert     ; text length
        int     0x40

        movd    esi,mm7
        mov     eax,[points_count_var+esi]
        mov     ecx,10
   .dc:
        xor     edx,edx
        mov     edi,10
        div     edi
        add     dl,30h
        mov     [STRdata+ecx-1],dl
        loop    .dc

        mov  eax,4                     ; function 4 : write text to window
        mov  bx,[size_x_var]
        add  ebx,120
        shl  ebx,16
        mov  bx,si                     ; [x start] *65536 + [y start]
        shl  bx,3
        add  bx,45
        mov  ecx,0x00ddeeff
        mov  edx,STRdata               ; pointer to text beginning
        mov  esi,10                    ; text length
        int  40h
        pop  esi
        add  esi,4
        cmp  esi,16
        jnz  .nxxx
ret
;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************
    draw_window:
        mov     eax,12          ; function 12:tell os about windowdraw
        mov     ebx,1           ; 1, start of draw
        int     0x40

        ; DRAW WINDOW
        mov     eax,0  ; function 0 : define and draw window
        mov     bx,[x_start]
        shl     ebx,16
        mov     cx,[y_start]
        shl     ecx,16
;ebx -  [x start] *65536 + [x size]
;ecx -  [y start] *65536 + [y size]
        mov     bx,[size_x_var]
        add     bx,225
        mov     cx,[size_y_var]
        add     cx,30
    ; @@:
        mov     edx,0x13000000  ; color of work area RRGGBB,8->color gl
        mov     edi,labelt      ; WINDOW LABEL
        int     0x40

        call    buttons         ; more buttons

        .Y_ADD equ 2   ;-> offset of 'add vector' buttons

     ;   mov     eax,47
     ;   mov     ebx,0000000000111000000000000000b
     ;   mov     ecx,[points_count_var]
     ;   movzx   edx,word[size_x_var]
     ;   shl     edx,16
     ;   add     edx,130*65536+60  ; [x start] *65536 + [y start]
     ;   mov     esi,0x00ddeeff  ; font 1 & color ( 0xF0RRGGBB )
     ;   int     0x40

       call  write_info

        ; ADD VECTOR LABEL      ; add vector buttons - 30 ++
        mov     eax,4           ; function 4 : write text to window
        movzx   ebx,word[size_x_var]
        shl     ebx,16
        add     ebx,(12)*65536+(168+15*(13+.Y_ADD))   ; [x start] *65536 + [y start]
        mov     ecx,0x00ddeeff  ; font 1 & color ( 0xF0RRGGBB )
        mov     edx,labelvector      ; pointer to text beginning
        mov     esi,labelvectorend-labelvector     ; text length
    ;    cmp     [move_flag],2
    ;    jne     @f
    ;    add     edx,navigation_size
    ;  @@:
        int     0x40
         ; VECTOR Y- BUTTON
        mov     eax,8           ; function 8 : define and draw button
        movzx   ebx,word[size_x_var]
        shl     ebx,16
        add     ebx,30*65536+20     ; [x start] *65536 + [x size]
        mov     ecx,(165+15*(14+.Y_ADD))*65536+12  ; [y start] *65536 + [y size]
        mov     edx,30           ; button id
        mov     esi,0x6688dd    ; button color RRGGBB
        int     0x40
        ;VECTOR Y- LABEL
        mov     eax,4           ; function 4 : write text to window
        movzx   ebx,word[size_x_var]
        shl     ebx,16
        add     ebx,(32)*65536+(168+15*(14+.Y_ADD))   ; [x start] *65536 + [y start]
        mov     ecx,0x00ddeeff  ; font 1 & color ( 0xF0RRGGBB )
        mov     edx,labelyminus      ; pointer to text beginning
        mov     esi,labelyminusend-labelyminus     ; text length
        cmp     [move_flag],2
   ;     jne     @f
   ;     add     edx,navigation_size
   ;   @@:
        int     0x40
        ; VECTOR Z+ BUTTON
        mov     eax,8           ; function 8 : define and draw button
        movzx   ebx,word[size_x_var]
        shl     ebx,16
        add     ebx,(51)*65536+21     ; [x start] *65536 + [x size]
        mov     ecx,(165+15*(14+.Y_ADD))*65536+12  ; [y start] *65536 + [y size]
        mov     edx,31           ; button id
        mov     esi,0x6688dd    ; button color RRGGBB
        int     0x40
        ;VECTOR Z+ LABEL
        mov     eax,4           ; function 4 : write text to window
        movzx   ebx,word[size_x_var]
        shl     ebx,16
        add     ebx,(53)*65536+(168+15*(14+.Y_ADD))   ; [x start] *65536 + [y start]
        mov     ecx,0x00ddeeff  ; font 1 & color ( 0xF0RRGGBB )
        mov     edx,labelzplus      ; pointer to text beginning
        mov     esi,labelzplusend-labelzplus     ; text length
   ;     cmp     [move_flag],2
   ;     jne     @f
   ;     add     edx,navigation_size
   ;   @@:

        int     0x40
        ; VECTOR x- BUTTON
        mov     eax,8           ; function 8 : define and draw button
        movzx   ebx,word[size_x_var]
        shl     ebx,16
        add     ebx,(10)*65536+21     ; [x start] *65536 + [x size]
        mov     ecx,(165+15*(15+.Y_ADD))*65536+12  ; [y start] *65536 + [y size]
        mov     edx,32           ; button id
        mov     esi,0x6688dd    ; button color RRGGBB
        int     0x40
        ;VECTOR x- LABEL
        mov     eax,4           ; function 4 : write text to window
        movzx   ebx,word[size_x_var]
        shl     ebx,16
        add     ebx,(12)*65536+(168+15*(15+.Y_ADD))   ; [x start] *65536 + [y start]
        mov     ecx,0x00ddeeff  ; font 1 & color ( 0xF0RRGGBB )
        mov     edx,labelxminus      ; pointer to text beginning
        mov     esi,labelxminusend-labelxminus     ; text length
   ;     cmp     [move_flag],2
   ;     jne     @f
   ;     add     edx,navigation_size
   ;   @@:
        int     0x40
        ; VECTOR x+ BUTTON
        mov     eax,8           ; function 8 : define and draw button
        movzx   ebx,word[size_x_var]
        shl     ebx,16
        add     ebx,(51)*65536+21     ; [x start] *65536 + [x size]
        mov     ecx,(165+15*(15+.Y_ADD))*65536+12  ; [y start] *65536 + [y size]
        mov     edx,33           ; button id
        mov     esi,0x6688dd    ; button color RRGGBB
        int     0x40
        ;VECTOR x+ LABEL
        mov     eax,4           ; function 4 : write text to window
        movzx   ebx,word[size_x_var]
        shl     ebx,16
        add     ebx,(53)*65536+(168+15*(15+.Y_ADD))   ; [x start] *65536 + [y start]
        mov     ecx,0x00ddeeff  ; font 1 & color ( 0xF0RRGGBB )
        mov     edx,labelxplus      ; pointer to text beginning
        mov     esi,labelxplusend-labelxplus     ; text length
   ;     cmp     [move_flag],2
   ;     jne     @f
   ;     add     edx,navigation_size
   ;   @@:
        int     0x40
        ; VECTOR z- BUTTON
        mov     eax,8           ; function 8 : define and draw button
        movzx   ebx,word[size_x_var]
        shl     ebx,16
        add     ebx,(10)*65536+62-41     ; [x start] *65536 + [x size]
        mov     ecx,(25+140+15*(16+.Y_ADD))*65536+12  ; [y start] *65536 + [y size]
        mov     edx,34           ; button id
        mov     esi,0x6688dd    ; button color RRGGBB
        int     0x40
        ;VECTOR z- LABEL
        mov     eax,4           ; function 4 : write text to window
        movzx   ebx,word[size_x_var]
        shl     ebx,16
        add     ebx,(12)*65536+(168+15*(16+.Y_ADD))   ; [x start] *65536 + [y start]
        mov     ecx,0x00ddeeff  ; font 1 & color ( 0xF0RRGGBB )
        mov     edx,labelzminus      ; pointer to text beginning
        mov     esi,labelzminusend-labelzminus     ; text length
   ;     cmp     [move_flag],2
   ;     jne     @f
   ;     add     edx,navigation_size
   ;   @@:
        int     0x40
       ;VECTOR Y+ BUTTON
        mov     eax,8           ; function 8 : define and draw button
        movzx   ebx,word[size_x_var]
        shl     ebx,16
        add     ebx,(10+20)*65536+20     ; [x start] *65536 + [x size]
        mov     ecx,(165+15*(16+.Y_ADD))*65536+12  ; [y start] *65536 + [y size]
        mov     edx,35           ; button id
        mov     esi,0x6688dd    ; button color RRGGBB
        int     0x40
        ;VECTOR Y+ LABEL
        mov     eax,4           ; function 4 : write text to window
        movzx   ebx,word[size_x_var]
        shl     ebx,16
        add     ebx,(32)*65536+(168+15*(16+.Y_ADD))   ; [x start] *65536 + [y start]
        mov     ecx,0x00ddeeff  ; font 1 & color ( 0xF0RRGGBB )
        mov     edx,labelyplus      ; pointer to text beginning
        mov     esi,labelyplusend-labelyplus     ; text length
   ;     cmp     [move_flag],2
   ;     jne     @f
   ;     add     edx,navigation_size
   ;   @@:
        int     0x40

        mov     eax,12          ; function 12:tell os about windowdraw
        mov     ebx,2           ; 2, end of draw
        int     0x40
      ;  pop     eax
      ;  mov     [fire_flag],al
        ret


   ; DATA AREA  ************************************

   include 'data.inc'
   align 16
MEM_END:

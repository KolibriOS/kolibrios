;
; application : Cruce - app shows three models shading
; compiler    : FASM  1.65.13
; system      : KolibriOS/MenuetOS
; author      : macgub
; email       : macgub3@wp.pl
; web         : www.menuet.xt.pl
; Fell free to use this intro in your own distribution of KolibriOS/MenuetOS.
; Special greetings to all MenuetOS maniax in the world.
; I hope because my intros Christian Belive will be near to each of You.


; Some adjustments made by Madis Kalme
; madis.kalme@mail.ee
; I tried optimizing it a bit, but don't know if it was successful. The objects
; can be:
; 1) Read from a file (*.3DS standard)
; 2) Written in manually (at the end of the code)

; Now the program uses window style Y=4 - 19.10.2007, by Ataualpa

SIZE_X equ 250
SIZE_Y equ 250
TIMEOUT equ 4
ROUND equ 10
TEX_X equ 512     ; texture width
TEX_Y equ 512     ;         height
TEX_SHIFT equ 9  ; texture widith shifting
TEX equ  SHIFTING ;  TEX={SHIFTING | FLUENTLY}
FLUENTLY = 0
SHIFTING = 1
;CATMULL_SHIFT equ 8
NON   =   0
MMX   =   1

Ext   =   MMX                   ;Ext={ NON | MMX}

use32
        org    0x0
        db     'MENUET01'       ; 8 byte id
        dd     0x01             ; header version
        dd     START            ; start of code
        dd     I_END            ; size of image
        dd     I_END            ; memory for app
        dd     I_END            ; esp
        dd     0x0 , 0x0        ; I_Param , I_Icon

START:    ; start of execution
        cld
 ;       call alloc_buffer_mem
        call read_from_file
        call init_triangles_normals
        call init_point_normals
        call init_envmap
        call draw_window


still:
        mov     eax,23          ; wait here for event with timeout
        mov     ebx,TIMEOUT
        cmp     [speed_flag],1
        jne     .skip
        mov     eax,11
    .skip:
        int     0x40

        cmp     eax,1           ; redraw request ?
        je      red
        cmp     eax,2           ; key in buffer ?
        je      key
        cmp     eax,3           ; button in buffer ?
        je      button

        jmp     noclose

    red:                        ; redraw
        call    draw_window
        jmp     noclose

    key:                        ; key
        mov     eax,2           ; just read it and ignore
        int     0x40
        jmp     noclose

    button:                     ; button
        mov     eax,17          ; get id
        int     0x40

        cmp     ah,1            ; button id=1 ?
        jne     .ch_another

        mov     eax,-1          ; close this program
        int     0x40
      .ch_another:
        cmp     ah,2
        jne     .ch_another1
        inc     [r_flag]
        cmp     [r_flag],3
        jne     noclose
        mov     [r_flag],0
      .ch_another1:
        cmp     ah,3
        jne     .ch_another2
        inc     [dr_flag]
        cmp     [dr_flag],3
        jne     noclose
        mov     [dr_flag],0
      .ch_another2:
        cmp     ah,4                     ; toggle speed
        jne     @f
        inc     [speed_flag]
        cmp     [speed_flag],2
        jne     noclose
        mov     [speed_flag],0
      @@:
        cmp     ah,5
        jne     @f                       ;scale-
        mov     [scale],0.7
        fninit
        fld     [sscale]
        fmul    [scale]
        fstp    [sscale]
        call    read_from_file
        mov     ax,[vect_x]  ;-- last change
        mov     bx,[vect_y]
        mov     cx,[vect_z]
        call    add_vector
;        call    do_scale
      @@:
        cmp     ah,6
        jne     @f                      ; scale+
        mov     [scale],1.3
        fninit
        fld     [sscale]
        fmul    [scale]
        fstp    [sscale]
        call    read_from_file
        mov     ax,[vect_x]
        mov     bx,[vect_y]
        mov     cx,[vect_z]
        call    add_vector
        call    init_triangles_normals
        call    init_point_normals
      @@:
        cmp     ah,7
        jne     @f
        xor     ax,ax            ;add vector to object and rotary point
        mov     bx,-10
        xor     cx,cx
        call    add_vector
        sub     [vect_y],10
        sub     [yo],10
      @@:
        cmp     ah,8
        jne     @f
        xor     ax,ax
        xor     bx,bx
        mov     cx,10
        call    add_vector
        add     [vect_z],10
        add     [zo],10
      @@:
        cmp     ah,9
        jne     @f
        mov     ax,-10
        xor     bx,bx
        xor     cx,cx
        call    add_vector
        sub     [vect_x],10
        sub     [xo],10
      @@:
        cmp     ah,10
        jne     @f
        mov     ax,10
        xor     bx,bx
        xor     cx,cx
        call    add_vector
        add     [vect_x],10
        add     [xo],10
      @@:
        cmp     ah,11
        jne     @f
        xor     ax,ax
        xor     bx,bx
        mov     cx,-10
        call    add_vector
        sub     [vect_z],10
        sub     [zo],10
      @@:
        cmp     ah,12
        jne     @f
        xor     ax,ax
        mov     bx,10
        xor     cx,cx
        call    add_vector
        add     [vect_y],10
        add     [yo],10
      @@:
        cmp     ah,13                    ; change main color -
        jne     @f                       ; - lead color setting
        cmp     [max_color_r],245
        jge     @f
        add     [max_color_r],10
        call    init_envmap
      @@:
        cmp     ah,14
        jne     @f
        cmp     [max_color_g],245
        jge     @f
        add     [max_color_g],10
        call    init_envmap
      @@:
        cmp     ah,15
        jne     @f
        cmp     [max_color_b],245
        jge     @f
        add     [max_color_b],10
        call    init_envmap
      @@:
        cmp     ah,16                    ; change main color
        jne     @f
        cmp     [max_color_r],10
        jle     @f
        sub     [max_color_r],10
        call    init_envmap
      @@:
        cmp     ah,17
        jne     @f
        cmp     [max_color_g],10
        jle     @f
        sub     [max_color_g],10
        call    init_envmap
      @@:
        cmp     ah,18
        jne     @f
        cmp     [max_color_b],10
        jle     @f
        sub     [max_color_b],10
        call    init_envmap
      @@:
        cmp     ah,19
        jne     @f
        inc     [catmull_flag]
        cmp     [catmull_flag],2
        jne     @f
        mov     [catmull_flag],0
      @@:
    noclose:

        call    calculate_angle ; calculates sinus and cosinus
        call    copy_point_normals
   ; copy normals and rotate the copy using sin/cosbeta - best way
        call    rotate_point_normals
        call    calculate_colors
        call    copy_points
        call    rotate_points
        call    translate_perspective_points; translate from 3d to 2d
        call    clrscr          ; clear the screen
        cmp     [dr_flag],2
        je      @f
        cmp     [catmull_flag],1  ;if env_mapping sort faces
        je      @f
      @@:
        call    sort_triangles
      @@:
        call    fill_Z_buffer

    RDTSC
    push eax
        call    draw_triangles  ; draw all triangles from the list

    RDTSC
    sub eax,[esp]
    sub eax,41
    ;    lea     esi,[debug_points]
    ;    lea     edi,[debug_points+6]
    ;    lea     ebx,[debug_vector1]
    ;    call    make_vector
    ;    fninit
    ;    fld     [sinbeta_one]
    ;    fimul   [debug_dwd]
    ;    fistp   [debug_dd]
    ;    movzx    eax,[debug_dd]


    mov     ecx,10
  .dc:
    xor     edx,edx
    mov     edi,10
    div     edi
    add     dl,30h
    mov     [STRdata+ecx-1],dl
    loop    .dc
    pop eax
macro show
{
        mov     eax,7           ; put image
        mov     ebx,screen
        mov     ecx,SIZE_X shl 16 + SIZE_Y
        mov     edx,5 shl 16 + 20
        int     0x40
}
    show
    mov  eax,4                     ; function 4 : write text to window
    mov  ebx,5*65536+23            ; [x start] *65536 + [y start]
    mov  ecx,-1
    mov  edx,STRdata               ; pointer to text beginning
    mov  esi,10                    ; text length
    int  40h



        jmp     still

;--------------------------------------------------------------------------------
;-------------------------PROCEDURES---------------------------------------------
;--------------------------------------------------------------------------------
include "tex3.asm"
include "flat_cat.asm"
include "grd_cat.asm"
include "3dmath.asm"
include "grd3.asm"
include "flat3.asm"


;alloc_buffer_mem:
;        mov     eax,68
;        mov     ebx,5
;        mov     ecx,SIZE_X*SIZE_Y*3
;        int     0x40
;        mov     [screen],eax
;ret
init_envmap:

.temp equ word [ebp-2]
         push     ebp
         mov      ebp,esp
         sub      esp,2
         mov      edi,envmap
         fninit

         mov      dx,-256
    .ie_ver:
         mov      cx,-256
    .ie_hor:
         mov      .temp,cx
         fild     .temp
         fmul     st,st0
         mov      .temp,dx
         fild     .temp
         fmul     st,st0
         faddp
         fsqrt
         mov      .temp,254
         fisubr   .temp
         fmul     [env_const]
         fistp    .temp
         mov      ax,.temp

         or      ax,ax
         jge     .ie_ok1
         xor     ax,ax
         jmp     .ie_ok2
  .ie_ok1:
         cmp     ax,254
         jle     .ie_ok2
         mov     ax,254
  .ie_ok2:
         push    dx
         mov     bx,ax
         mul     [max_color_b]
         shr     ax,8
         stosb
         mov     ax,bx
         mul     [max_color_g]
         shr     ax,8
         stosb
         mov     ax,bx
         mul     [max_color_r]
         shr     ax,8
         stosb
         pop     dx

         inc     cx
         cmp     cx,256
         jne     .ie_hor

         inc     dx
         cmp     dx,256
         jne     .ie_ver

         mov     esp,ebp
         pop     ebp
macro debug
{
         mov     edi,envmap
         mov     ecx,512*512*3/4
         mov     eax,0xffffffff
         rep     stosd
}
ret
calculate_colors:
        fninit
        xor     ebx,ebx
        movzx   ecx,[points_count_var]
        lea     ecx,[ecx*3]
        add     ecx,ecx
      .cc_again:
        mov     esi,light_vector
        lea     edi,[point_normals_rotated+ebx*2]
        call    dot_product
        fcom    [dot_min]
        fstsw   ax
        sahf
        ja      .cc_ok1
        ffree   st
        mov     dword[points_color+ebx],0
        mov     word[points_color+ebx+4],0
        add     ebx,6
        cmp     ebx,ecx
        jne     .cc_again
        jmp     .cc_done
      .cc_ok1:
        fcom    [dot_max]
        fstsw   ax
        sahf
        jb      .cc_ok2
        ffree   st
        mov     dword[points_color+ebx],0  ; clear r,g,b
        mov     word[points_color+ebx+4],0
        add     ebx,6
        cmp     ebx,ecx
        jne     .cc_again
        jmp     .cc_done
      .cc_ok2:
        fld     st
        fld     st
        fimul   [max_color_r]
        fistp   word[points_color+ebx]        ;each color as word
        fimul   [max_color_g]
        fistp   word[points_color+ebx+2]
        fimul   [max_color_b]
        fistp   word[points_color+ebx+4]
        add     ebx,6
        cmp     ebx,ecx
        jne     .cc_again
     .cc_done:
ret
copy_point_normals:
        movzx   ecx,[points_count_var]
        shl     ecx,2
        inc     ecx
        mov     esi,point_normals
        mov     edi,point_normals_rotated
        rep     movsd
ret
rotate_point_normals:
        movzx   ecx,[points_count_var]
        mov     ebx,point_normals_rotated
        fninit                     ; for now only rotate around Z axle
     .again_r:
        cmp     [r_flag],1
        je      .z_rot
        cmp     [r_flag],2
        je      .x_rot

      .y_rot:
        fld     dword[ebx]         ; x
        fld     [sinbeta]
        fmul    dword[ebx+8]       ; z * sinbeta
        fchs
        fld     [cosbeta]
        fmul    dword[ebx]         ; x * cosbeta
        faddp
        fstp    dword[ebx]         ; new x
        fmul    [sinbeta]          ; old x * sinbeta
        fld     [cosbeta]
        fmul    dword[ebx+8]       ; z * cosbeta
        faddp
        fstp    dword[ebx+8]       ; new z
        add     ebx,12
        loop    .y_rot
        jmp     .end_rot
      .z_rot:
        fld     dword[ebx]      ;x
        fld     [sinbeta]
        fmul    dword[ebx+4]    ;y
        fld     [cosbeta]
        fmul    dword[ebx]      ;x
        faddp
        fstp    dword[ebx]      ;new x
        fmul    [sinbeta]       ; sinbeta * old x
        fchs
        fld     [cosbeta]
        fmul    dword[ebx+4]         ; cosbeta * y
        faddp
        fstp    dword[ebx+4]    ; new y
        add     ebx,12
        loop    .z_rot
        jmp     .end_rot
       .x_rot:
        fld     dword[ebx+4]    ;y
        fld     [sinbeta]
        fmul    dword[ebx+8]    ;z
        fld     [cosbeta]
        fmul    dword[ebx+4]    ;y
        faddp
        fstp    dword[ebx+4]    ; new y
        fmul    [sinbeta]       ; sinbeta * old y
        fchs
        fld     [cosbeta]
        fmul    dword[ebx+8]
        faddp
        fstp    dword[ebx+8]
        add     ebx,12
        loop    .x_rot
       .end_rot:
ret
init_triangles_normals:
        mov     ebx,triangles_normals
        mov     ebp,triangles
     @@:
        push    ebx
        mov     ebx,vectors
        movzx   esi,word[ebp]          ; first point index
        lea     esi,[esi*3]
        lea     esi,[points+esi*2]     ; esi - pointer to 1st 3d point
        movzx   edi,word[ebp+2]        ; second point index
        lea     edi,[edi*3]
        lea     edi,[points+edi*2]     ; edi - pointer to 2nd 3d point
        call    make_vector
        add     ebx,12
        mov     esi,edi
        movzx   edi,word[ebp+4]        ; third point index
        lea     edi,[edi*3]
        lea     edi,[points+edi*2]
        call    make_vector
        mov     edi,ebx                 ; edi - pointer to 2nd vector
        mov     esi,ebx
        sub     esi,12                  ; esi - pointer to 1st vector
        pop     ebx
        call    cross_product
        mov     edi,ebx
        call    normalize_vector
        add     ebp,6
        add     ebx,12
        cmp     dword[ebp],-1
        jne     @b
ret

init_point_normals:
.x equ dword [ebp-4]
.y equ dword [ebp-8]
.z equ dword [ebp-12]
.point_number equ word [ebp-26]
.hit_faces    equ word [ebp-28]

        fninit
        mov       ebp,esp
        sub       esp,28
        mov       edi,point_normals
        mov       .point_number,0
    .ipn_loop:
        mov       .hit_faces,0
        mov       .x,0
        mov       .y,0
        mov       .z,0
        mov       esi,triangles
        xor       ecx,ecx              ; ecx - triangle number
    .ipn_check_face:
        xor       ebx,ebx              ; ebx - 'position' in one triangle
    .ipn_check_vertex:
        movzx     eax,word[esi+ebx]    ;  eax - point_number
        cmp       ax,.point_number
        jne       .ipn_next_vertex
        push      esi
        mov       esi,ecx
        lea       esi,[esi*3]
        lea       esi,[triangles_normals+esi*4]
       ; shl       esi,2
       ; add       esi,triangles_normals

        fld       .x
        fadd      dword[esi+vec_x]
        fstp      .x
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
        add       ebx,2
        cmp       ebx,6
        jne       .ipn_check_vertex
    .ipn_next_face:
        add       esi,6
        inc       ecx
        cmp       cx,[triangles_count_var]
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
        mov       dx,.point_number
        cmp       dx,[points_count_var]
        jne       .ipn_loop

        mov       esp,ebp
ret

add_vector:
        mov ebp,points
       @@:
        add word[ebp],ax
        add word[ebp+2],bx
        add word[ebp+4],cx
        add ebp,6
        cmp dword[ebp],-1
        jne @b
ret
;do_scale:
;        fninit
;        mov ebp,points
;      .next_sc:
;        fld1
;        fsub [scale]
;        fld st
;        fimul [xo]
;        fld [scale]
;        fimul word[ebp] ;x
;        faddp
;        fistp word[ebp]
;        fld st
;        fimul [yo]
;        fld [scale]
;        fimul word[ebp+2]
;        faddp
;        fistp word[ebp+2]
;        fimul [zo]
;        fld [scale]
;        fimul word[ebp+4]
;        faddp
;        fistp word[ebp+4]
;        add ebp,6
;        cmp dword[ebp],-1
;        jne .next_sc
;ret
sort_triangles:
        mov     esi,triangles
        mov     edi,triangles_with_z
        mov     ebp,points_rotated

    make_triangle_with_z:       ;makes list with triangles and z position
        movzx   eax,word[esi]
        lea     eax,[eax*3]
        movzx   ecx,word[ebp+eax*2+4]

        movzx   eax,word[esi+2]
        lea     eax,[eax*3]
        add     cx,word[ebp+eax*2+4]

        movzx   eax,word[esi+4]
        lea     eax,[eax*3]
        add     cx,word[ebp+eax*2+4]

        mov     ax,cx
       ; cwd
       ; idiv    word[i3]
        movsd                   ; store vertex coordinates
        movsw
        stosw                   ; middle vertex coordinate  'z' in triangles_with_z list
        cmp     dword[esi],-1
        jne     make_triangle_with_z
        movsd                   ; copy end mark
        mov     eax,4
        lea     edx,[edi-8-trizdd]
        mov     [high],edx
        call    quicksort
        mov     eax,4
        mov     edx,[high]
        call    insertsort
        jmp     end_sort

    quicksort:
        mov     ecx,edx
        sub     ecx,eax
        cmp     ecx,32
        jc      .exit
        lea     ecx,[eax+edx]
        shr     ecx,4
        lea     ecx,[ecx*8-4]; i
        mov     ebx,[trizdd+eax]; trizdd[l]
        mov     esi,[trizdd+ecx]; trizdd[i]
        mov     edi,[trizdd+edx]; trizdd[h]
        cmp     ebx,esi
        jg      @f              ; direction NB! you need to negate these to invert the order
      if Ext=NON
        mov     [trizdd+eax],esi
        mov     [trizdd+ecx],ebx
        mov     ebx,[trizdd+eax-4]
        mov     esi,[trizdd+ecx-4]
        mov     [trizdd+eax-4],esi
        mov     [trizdd+ecx-4],ebx
        mov     ebx,[trizdd+eax]
        mov     esi,[trizdd+ecx]
      else
        movq    mm0,[trizdq+eax-4]
        movq    mm1,[trizdq+ecx-4]
        movq    [trizdq+ecx-4],mm0
        movq    [trizdq+eax-4],mm1
        xchg    ebx,esi
      end if
      @@:
        cmp     ebx,edi
        jg      @f              ; direction
      if Ext=NON
        mov     [trizdd+eax],edi
        mov     [trizdd+edx],ebx
        mov     ebx,[trizdd+eax-4]
        mov     edi,[trizdd+edx-4]
        mov     [trizdd+eax-4],edi
        mov     [trizdd+edx-4],ebx
        mov     ebx,[trizdd+eax]
        mov     edi,[trizdd+edx]
      else
        movq    mm0,[trizdq+eax-4]
        movq    mm1,[trizdq+edx-4]
        movq    [trizdq+edx-4],mm0
        movq    [trizdq+eax-4],mm1
        xchg    ebx,edi
      end if
      @@:
        cmp     esi,edi
        jg      @f              ; direction
      if Ext=NON
        mov     [trizdd+ecx],edi
        mov     [trizdd+edx],esi
        mov     esi,[trizdd+ecx-4]
        mov     edi,[trizdd+edx-4]
        mov     [trizdd+ecx-4],edi
        mov     [trizdd+edx-4],esi
      else
        movq    mm0,[trizdq+ecx-4]
        movq    mm1,[trizdq+edx-4]
        movq    [trizdq+edx-4],mm0
        movq    [trizdq+ecx-4],mm1
;        xchg    ebx,esi
      end if
      @@:
        mov     ebp,eax         ; direction
        add     ebp,8      ;   j
      if Ext=NON
        mov     esi,[trizdd+ebp]
        mov     edi,[trizdd+ecx]
        mov     [trizdd+ebp],edi
        mov     [trizdd+ecx],esi
        mov     esi,[trizdd+ebp-4]
        mov     edi,[trizdd+ecx-4]
        mov     [trizdd+ecx-4],esi
        mov     [trizdd+ebp-4],edi
      else
        movq    mm0,[trizdq+ebp-4]
        movq    mm1,[trizdq+ecx-4]
        movq    [trizdq+ecx-4],mm0
        movq    [trizdq+ebp-4],mm1
      end if
        mov     ecx,edx    ;   i; direction
        mov     ebx,[trizdd+ebp]; trizdd[j]
      .loop:
        sub     ecx,8           ; direction
        cmp     [trizdd+ecx],ebx
        jl      .loop           ; direction
      @@:
        add     ebp,8           ; direction
        cmp     [trizdd+ebp],ebx
        jg      @b              ; direction
        cmp     ebp,ecx
        jge     @f              ; direction
      if Ext=NON
        mov     esi,[trizdd+ecx]
        mov     edi,[trizdd+ebp]
        mov     [trizdd+ebp],esi
        mov     [trizdd+ecx],edi
        mov     edi,[trizdd+ecx-4]
        mov     esi,[trizdd+ebp-4]
        mov     [trizdd+ebp-4],edi
        mov     [trizdd+ecx-4],esi
      else
        movq    mm0,[trizdq+ecx-4]
        movq    mm1,[trizdq+ebp-4]
        movq    [trizdq+ebp-4],mm0
        movq    [trizdq+ecx-4],mm1
      end if
        jmp     .loop
      @@:
      if Ext=NON
        mov     esi,[trizdd+ecx]
        mov     edi,[trizdd+eax+8]
        mov     [trizdd+eax+8],esi
        mov     [trizdd+ecx],edi
        mov     edi,[trizdd+ecx-4]
        mov     esi,[trizdd+eax+4]
        mov     [trizdd+eax+4],edi
        mov     [trizdd+ecx-4],esi
      else
        movq    mm0,[trizdq+ecx-4]
        movq    mm1,[trizdq+eax+4]; dir
        movq    [trizdq+eax+4],mm0; dir
        movq    [trizdq+ecx-4],mm1
      end if
        add     ecx,8
        push    ecx edx
        mov     edx,ebp
        call    quicksort
        pop     edx eax
        call    quicksort
      .exit:
    ret
    insertsort:
        mov     esi,eax
      .start:
        add     esi,8
        cmp     esi,edx
        ja      .exit
        mov     ebx,[trizdd+esi]
      if Ext=NON
        mov     ecx,[trizdd+esi-4]
      else
        movq    mm1,[trizdq+esi-4]
      end if
        mov     edi,esi
      @@:
        cmp     edi,eax
        jna     @f
        cmp     [trizdd+edi-8],ebx
        jg      @f                 ; direction
      if Ext=NON
        mov     ebp,[trizdd+edi-8]
        mov     [trizdd+edi],ebp
        mov     ebp,[trizdd+edi-12]
        mov     [trizdd+edi-4],ebp
      else
        movq    mm0,[trizdq+edi-12]
        movq    [trizdq+edi-4],mm0
      end if
        sub     edi,8
        jmp     @b
      @@:
      if Ext=NON
        mov     [trizdd+edi],ebx
        mov     [trizdd+edi-4],ecx
      else
        movq    [trizdq+edi-4],mm1
      end if
        jmp     .start
      .exit:
    ret
   end_sort:
    ; translate triangles_with_z to sorted_triangles
        mov     esi,triangles_with_z
      ;  mov     edi,sorted_triangles
        mov      edi,triangles
    again_copy:
      if Ext=NON
        movsd
        movsw
        add     esi,2
      else
        movq    mm0,[esi]
        movq    [edi],mm0
        add     esi,8
        add     edi,6
      end if
        cmp     dword[esi],-1
        jne     again_copy
;      if Ext=MMX
;        emms
;      end if
        movsd  ; copy end mark too
ret

clrscr:
        mov     edi,screen
        mov     ecx,SIZE_X*SIZE_Y*3/4
        xor     eax,eax
      if Ext=NON
        rep     stosd
      else
        pxor    mm0,mm0
      @@:
        movq    [edi+00],mm0
        movq    [edi+08],mm0
        movq    [edi+16],mm0
        movq    [edi+24],mm0
        add     edi,32
        sub     ecx,8
        jnc     @b
      end if
ret

calculate_angle:
        fninit
;        fldpi
;        fidiv   [i180]
        fld     [piD180]
        fimul   [angle_counter]
        fsincos
        fstp    [sinbeta]
        fstp    [cosbeta]
        inc     [angle_counter]
        cmp     [angle_counter],360
        jne     end_calc_angle
        mov     [angle_counter],0
    end_calc_angle:
ret

rotate_points:
        fninit
        mov     ebx,points_rotated
    again_r:
        cmp     [r_flag],1
        je      .z_rot
        cmp     [r_flag],2
        je      .x_rot
    .y_rot:
        mov     eax,[ebx+2]     ;z
        mov     ax,word[ebx]    ;x
        sub     eax,dword[xo]
        mov     dword[xsub],eax
        fld     [sinbeta]
        fimul   [zsub]
        fchs
        fld     [cosbeta]
        fimul   [xsub]
        faddp
        fiadd   [xo]
        fistp   word[ebx]  ;x
        fld     [sinbeta]
        fimul   [xsub]
        fld     [cosbeta]
        fimul   [zsub]
        faddp
        fiadd   [zo]
        fistp   word[ebx+4] ;z
        jmp     .end_rot
   .z_rot:
        mov     ax,word[ebx]
        sub     ax,word[xo]       ;need optimization
        mov     [xsub],ax
        mov     ax,word[ebx+2]
        sub     ax,word[yo]
        mov     [ysub],ax
        fld     [sinbeta]
        fimul   [ysub]
        fld     [cosbeta]
        fimul   [xsub]
        faddp
        fiadd   [xo]
        fistp   word[ebx]
        fld     [cosbeta]
        fimul   [ysub]
        fld     [sinbeta]
        fimul   [xsub]
        fchs
        faddp
        fiadd   [yo]
        fistp   word[ebx+2]
        jmp     .end_rot
   .x_rot:
        mov     ax,word[ebx+2]
        sub     ax,[yo]
        mov     [ysub],ax
        mov     ax,word[ebx+4]
        sub     ax,word[zo]
        mov     [zsub],ax
        fld     [sinbeta]
        fimul   [zsub]
        fld     [cosbeta]
        fimul   [ysub]
        faddp
        fiadd   [yo]
        fistp   word[ebx+2];y
        fld     [cosbeta]
        fimul   [zsub]
        fld     [sinbeta]
        fimul   [ysub]
        fchs
        faddp
        fiadd   [zo]
        fistp   word[ebx+4]
     .end_rot:
        add     ebx,6
        cmp     dword[ebx],-1
        jne     again_r
ret

draw_triangles:
        mov esi,triangles
    .again_dts:
        mov ebp,points_rotated
      if Ext=NON
        movzx   eax,word[esi]
        mov     [point_index1],ax
        lea     eax,[eax*3]
        add     eax,eax
        push    ebp
        add     ebp,eax
        mov     eax,[ebp]
        mov     dword[xx1],eax
        mov     eax,[ebp+4]
        mov     [zz1],ax
        pop     ebp


        movzx   eax,word[esi+2]
        mov     [point_index2],ax
        lea     eax,[eax*3]
        add     eax,eax
        push    ebp
        add     ebp,eax
        mov     eax,[ebp]
        mov     dword[xx2],eax
        mov     eax,[ebp+4]
        mov     [zz2],ax
        pop     ebp


        movzx   eax,word[esi+4]        ; xyz3 = [ebp+[esi+4]*6]
        mov     [point_index3],ax
        lea     eax,[eax*3]
        add     eax,eax
    ;    push    ebp
        add     ebp,eax
        mov     eax,[ebp]
        mov     dword[xx3],eax
        mov     eax,[ebp+4]
        mov     [zz3],ax
      else
        mov     eax,dword[esi]           ; don't know MMX
        mov     dword[point_index1],eax
       ; shr     eax,16
       ; mov     [point_index2],ax
        mov     ax,word[esi+4]
        mov     [point_index3],ax
        movq    mm0,[esi]
        pmullw  mm0,qword[const6]
        movd    eax,mm0
        psrlq   mm0,16
        movd    ebx,mm0
        psrlq   mm0,16
        movd    ecx,mm0
        and     eax,0FFFFh
        and     ebx,0FFFFh
        and     ecx,0FFFFh
        movq    mm0,[ebp+eax]
        movq    mm1,[ebp+ebx]
        movq    mm2,[ebp+ecx]
        movq    qword[xx1],mm0
        movq    qword[xx2],mm1
        movq    qword[xx3],mm2
;        emms
      end if
        push esi
        ; culling
        fninit
        mov     esi,point_index1
        mov     ecx,3
      @@:
        movzx   eax,word[esi]
        lea     eax,[eax*3]
        shl     eax,2
        lea     eax,[eax+point_normals_rotated]
        fld     dword[eax+8]
        ftst
        fstsw   ax
        sahf
        jb     @f
        ffree   st
        loop    @b
        jmp     .end_draw
      @@:
        ffree   st  ;is visable

        cmp     [dr_flag],0               ; draw type flag
        je      .flat_draw
        cmp     [dr_flag],2
        je      .env_mapping

        movzx   edi,[point_index3]
        lea     edi,[edi*3]
        lea     edi,[points_color+edi*2]
        cmp     [catmull_flag],0
        je      @f
        push    [zz3]
      @@:
        push    word[edi]
        push    word[edi+2]
        push    word[edi+4]
        movzx   edi,[point_index2]
        lea     edi,[edi*3]
        lea     edi,[points_color+edi*2]
        cmp     [catmull_flag],0
        je      @f
        push    [zz2]
     @@:
        push    word[edi]
        push    word[edi+2]
        push    word[edi+4]
        movzx   edi,[point_index1]
        lea     edi,[edi*3]
        lea     edi,[points_color+edi*2]
        cmp     [catmull_flag],0
        je      @f
        push    [zz1]
      @@:
        push    word[edi]
        push    word[edi+2]
        push    word[edi+4]

   ;     movzx   edi,[point_index3]   ;gouraud shading according to light vector
   ;     lea     edi,[edi*3]
   ;     lea     edi,[4*edi+point_normals_rotated] ; edi - normal
   ;     mov     esi,light_vector
   ;     call    dot_product
   ;     fabs
   ;     fimul   [max_color_r]
   ;     fistp   [temp_col]
   ;     and     [temp_col],0x00ff
   ;     push    [temp_col]
   ;     push    [temp_col]
   ;     push    [temp_col]

   ;     movzx   edi,[point_index2]
   ;     lea     edi,[edi*3]
   ;     lea     edi,[4*edi+point_normals_rotated] ; edi - normal
   ;     mov     esi,light_vector
   ;     call    dot_product
   ;     fabs
   ;     fimul   [max_color_r]
   ;     fistp    [temp_col]
   ;     and     [temp_col],0x00ff
   ;     push    [temp_col]
   ;     push    [temp_col]
   ;     push    [temp_col]

   ;     movzx   edi,[point_index1]
   ;     lea     edi,[edi*3]
   ;     lea     edi,[4*edi+point_normals_rotated] ; edi - normal
   ;     mov     esi,light_vector
   ;     call    dot_product
   ;     fabs
   ;     fimul   [max_color_r]
   ;     fistp   [temp_col]
   ;     and     [temp_col],0x00ff
   ;     push    [temp_col]
   ;     push    [temp_col]
   ;     push    [temp_col]

;        xor     edx,edx            ; draw acording to position
;        mov     ax,[zz3]
;        add     ax,128
;        neg     al
;        and     ax,0x00ff
;        push    ax
;        neg     al
;        push    ax
;        mov     dx,[yy3]
;        and     dx,0x00ff
;        push    dx
;        mov     ax,[zz2]
;        add     ax,128
;        neg     al
;        and     ax,0x00ff
;        push    ax
;        neg     al
;        push    ax
;        mov     dx,[yy2]
;        and     dx,0x00ff
;        push    dx
;        mov     ax,[zz1]
;        add     ax,128
;        neg     al
;        and     ax,0x00ff
;        push    ax
;        neg     al
;        push    ax
;        mov     dx,[yy1]
;        and     dx,0x00ff
;        push    dx
        mov     eax,dword[xx1]
        ror     eax,16
        mov     ebx,dword[xx2]
        ror     ebx,16
        mov     ecx,dword[xx3]
        ror     ecx,16
        lea     edi,[screen]
        cmp     [catmull_flag],0
        je      @f
        lea     esi,[Z_buffer]
        call    gouraud_triangle_z
        jmp     .end_draw
       @@:
        call    gouraud_triangle
        jmp     .end_draw
     .flat_draw:

        movzx   edi,[point_index3]
        lea     edi,[edi*3]
        lea     edi,[points_color+edi*2]
        movzx   eax,word[edi]
        movzx   ebx,word[edi+2]
        movzx   ecx,word[edi+4]
        movzx   edi,[point_index2]
        lea     edi,[edi*3]
        lea     edi,[points_color+edi*2]
        add     ax,word[edi]
        add     bx,word[edi+2]
        add     cx,word[edi+4]
        movzx   edi,[point_index1]
        lea     edi,[edi*3]
        lea     edi,[points_color+edi*2]
        add     ax,word[edi]
        add     bx,word[edi+2]
        add     cx,word[edi+4]
        cwd
        idiv    [i3]
        mov     di,ax
        shl     edi,16
        mov     ax,bx
        cwd
        idiv    [i3]
        mov     di,ax
        shl     di,8
        mov     ax,cx
        cwd
        idiv    [i3]
        mov     edx,edi
        mov     dl,al
        and     edx,0x00ffffff


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

        mov     eax,dword[xx1]
        ror     eax,16
        mov     ebx,dword[xx2]
        ror     ebx,16
        mov     ecx,dword[xx3]
        ror     ecx,16
       ; mov     edi,screen
        lea     edi,[screen]
        cmp     [catmull_flag],0
        je      @f
        lea     esi,[Z_buffer]
        push    word[zz3]
        push    word[zz2]
        push    word[zz1]
        call    flat_triangle_z
        jmp     .end_draw
      @@:
        call    draw_triangle
        jmp     .end_draw
      .env_mapping:
       ; fninit
        mov     esi,point_index1
        sub     esp,12
        mov     edi,esp
        mov     ecx,3
      @@:
        movzx   eax,word[esi]
        lea     eax,[eax*3]
        shl     eax,2
        add     eax,point_normals_rotated
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
        add     esi,2
        loop    @b

        mov     eax,dword[xx1]
        ror     eax,16
        mov     ebx,dword[xx2]
        ror     ebx,16
        mov     ecx,dword[xx3]
        ror     ecx,16
        mov     edi,screen
        mov     esi,envmap
        call    tex_triangle

      .end_draw:
        pop     esi
        add     esi,6
        cmp     dword[esi],-1
        jne     .again_dts
ret
translate_points:
;        fninit
;        mov     ebx,points_rotated
;    again_trans:
;        fild    word[ebx+4] ;z1
;        fmul    [sq]
;        fld     st
;        fiadd   word[ebx]  ;x1
;        fistp   word[ebx]
;        fchs
;        fiadd   word[ebx+2] ;y1
;        fistp   word[ebx+2] ;y1

;        add     ebx,6
;        cmp     dword[ebx],-1
;        jne     again_trans
;ret
translate_perspective_points: ;translate points from 3d to 2d using
        fninit                ;perspective equations
        mov ebx,points_rotated
      .again_trans:
        fild word[ebx]
        fisub [xobs]
        fimul [zobs]
        fild word[ebx+4]
        fisub [zobs]
        fdivp
        fiadd [xobs]
        fistp word[ebx]
        fild word[ebx+2]
        fisub [yobs]
        fimul [zobs]
        fild word[ebx+4]
        fisub [zobs]
        fdivp
        fchs
        fiadd [yobs]
        fistp word[ebx+2]
        add ebx,6
        cmp dword[ebx],-1
        jne .again_trans
ret


copy_points:
        mov     esi,points
        mov     edi,points_rotated
        mov     ecx,points_count*3+2
        rep     movsw
ret



read_from_file:
        mov     edi,triangles
        xor     ebx,ebx
        xor     ebp,ebp
        mov     esi,SourceFile
        cmp     [esi],word 4D4Dh
        jne     .exit ;Must be legal .3DS file
        cmp     dword[esi+2],EndFile-SourceFile
        jne     .exit ;This must tell the length
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
        cmp     esi,EndFile
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
        mov     [points_count_var],cx
        mov     edx,ecx
        add     esi,8
      @@:
        fld     dword[esi+4]
        fmul    [sscale]
        fadd    [xoffset]
        fld     dword[esi+8]
        fchs
        fmul    [sscale]
        fadd    [yoffset]
        fld     dword[esi+0]
        fchs
        fmul    [sscale]
        fistp   word[points+ebx+4]
        fistp   word[points+ebx+2]
        fistp   word[points+ebx+0]
        add     ebx,6
        add     esi,12
        dec     ecx
        jnz     @b
      @@:
        mov     dword[points+ebx],-1
      @@:
        cmp     [esi],word 4120h
        je      @f
        add     esi,[esi+2]
        jmp     @b
      @@:
        movzx   ecx,word[esi+6]
        mov     [triangles_count_var],cx
        add     esi,8
        ;mov     edi,triangles
      @@:
        movsd
        movsw
        add     word[edi-6],bp
        add     word[edi-4],bp
        add     word[edi-2],bp
        add     esi,2
        dec     ecx
        jnz     @b
        add     ebp,edx
        jmp     .find4k

      .exit:
        mov     dword[edi],-1
ret


;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************
    draw_window:
        mov     eax,12          ; function 12:tell os about windowdraw
        mov     ebx,1           ; 1, start of draw
        int     0x40

        ; DRAW WINDOW
        mov     eax,0           ; function 0 : define and draw window
        mov     ebx,100*65536+SIZE_X+80 ; [x start] *65536 + [x size]
        mov     ecx,100*65536+SIZE_Y+30 ; [y start] *65536 + [y size]
        mov     edx,0x04000000  ; color of work area RRGGBB,8->color gl
        mov     esi,0x805080d0  ; color of grab bar  RRGGBB,8->color gl
        mov     edi,0x005080d0  ; color of frames    RRGGBB
        int     0x40

        ; WINDOW LABEL
        mov     eax,4           ; function 4 : write text to window
        mov     ebx,8*65536+8   ; [x start] *65536 + [y start]
        mov     ecx,0x20ddeeff  ; font 1 & color ( 0xF0RRGGBB )
        mov     edx,labelt      ; pointer to text beginning
        mov     esi,labellen-labelt     ; text length
        int     0x40

         ; ROTARY BUTTON
        mov     eax,8           ; function 8 : define and draw button
        mov     ebx,(SIZE_X+10)*65536+62     ; [x start] *65536 + [x size]
        mov     ecx,25*65536+12  ; [y start] *65536 + [y size]
        mov     edx,2           ; button id
        mov     esi,0x6688dd    ; button color RRGGBB
        int     0x40

         ; ROTARY  LABEL
        mov     eax,4           ; function 4 : write text to window
        mov     ebx,(SIZE_X+12)*65536+28   ; [x start] *65536 + [y start]
        mov     ecx,0x20ddeeff  ; font 1 & color ( 0xF0RRGGBB )
        mov     edx,labelrot      ; pointer to text beginning
        mov     esi,labelrotend-labelrot     ; text length
        int     0x40

         ; DRAW MODE BUTTON
        mov     eax,8           ; function 8 : define and draw button
        mov     ebx,(SIZE_X+10)*65536+62     ; [x start] *65536 + [x size]
        mov     ecx,(25+15)*65536+12  ; [y start] *65536 + [y size]
        mov     edx,3           ; button id
        mov     esi,0x6688dd    ; button color RRGGBB
        int     0x40

         ; DRAW MODE LABEL
        mov     eax,4           ; function 4 : write text to window
        mov     ebx,(SIZE_X+12)*65536+28+15   ; [x start] *65536 + [y start]
        mov     ecx,0x20ddeeff  ; font 1 & color ( 0xF0RRGGBB )
        mov     edx,labeldrmod      ; pointer to text beginning
        mov     esi,labeldrmodend-labeldrmod     ; text length
        int     0x40

         ; SPEED BUTTON
        mov     eax,8           ; function 8 : define and draw button
        mov     ebx,(SIZE_X+10)*65536+62     ; [x start] *65536 + [x size]
        mov     ecx,(25+15*2)*65536+12  ; [y start] *65536 + [y size]
        mov     edx,4           ; button id
        mov     esi,0x6688dd    ; button color RRGGBB
        int     0x40

         ; SPEED MODE LABEL
        mov     eax,4           ; function 4 : write text to window
        mov     ebx,(SIZE_X+12)*65536+(28+15*2)   ; [x start] *65536 + [y start]
        mov     ecx,0x20ddeeff  ; font 1 & color ( 0xF0RRGGBB )
        mov     edx,labelspeedmod      ; pointer to text beginning
        mov     esi,labelspeedmodend-labelspeedmod     ; text length
        int     0x40

         ; SCALE- BUTTON
        mov     eax,8           ; function 8 : define and draw button
        mov     ebx,(SIZE_X+10)*65536+62     ; [x start] *65536 + [x size]
        mov     ecx,(25+15*3)*65536+12  ; [y start] *65536 + [y size]
        mov     edx,5           ; button id
        mov     esi,0x6688dd    ; button color RRGGBB
        int     0x40

         ; SCALE- MODE LABEL
        mov     eax,4           ; function 4 : write text to window
        mov     ebx,(SIZE_X+12)*65536+(28+15*3)   ; [x start] *65536 + [y start]
        mov     ecx,0x20ddeeff  ; font 1 & color ( 0xF0RRGGBB )
        mov     edx,labelzoomout      ; pointer to text beginning
        mov     esi,labelzoomoutend-labelzoomout     ; text length
        int     0x40

         ; SCALE+ BUTTON
        mov     eax,8           ; function 8 : define and draw button
        mov     ebx,(SIZE_X+10)*65536+62     ; [x start] *65536 + [x size]
        mov     ecx,(25+15*4)*65536+12  ; [y start] *65536 + [y size]
        mov     edx,6           ; button id
        mov     esi,0x6688dd    ; button color RRGGBB
        int     0x40

         ; SCALE+ MODE LABEL
        mov     eax,4           ; function 4 : write text to window
        mov     ebx,(SIZE_X+12)*65536+(28+15*4)   ; [x start] *65536 + [y start]
        mov     ecx,0x20ddeeff  ; font 1 & color ( 0xF0RRGGBB )
        mov     edx,labelzoomin      ; pointer to text beginning
        mov     esi,labelzoominend-labelzoomin     ; text length
        int     0x40

        ; ADD VECTOR LABEL
        mov     eax,4           ; function 4 : write text to window
        mov     ebx,(SIZE_X+12)*65536+(28+15*5)   ; [x start] *65536 + [y start]
        mov     ecx,0x20ddeeff  ; font 1 & color ( 0xF0RRGGBB )
        mov     edx,labelvector      ; pointer to text beginning
        mov     esi,labelvectorend-labelvector     ; text length
        int     0x40

         ; VECTOR Y- BUTTON
        mov     eax,8           ; function 8 : define and draw button
        mov     ebx,(SIZE_X+10+20)*65536+62-42     ; [x start] *65536 + [x size]
        mov     ecx,(25+15*6)*65536+12  ; [y start] *65536 + [y size]
        mov     edx,7           ; button id
        mov     esi,0x6688dd    ; button color RRGGBB
        int     0x40

         ;VECTOR Y- LABEL
        mov     eax,4           ; function 4 : write text to window
        mov     ebx,(SIZE_X+12+20)*65536+(28+15*6)   ; [x start] *65536 + [y start]
        mov     ecx,0x20ddeeff  ; font 1 & color ( 0xF0RRGGBB )
        mov     edx,labelyminus      ; pointer to text beginning
        mov     esi,labelyminusend-labelyminus     ; text length
        int     0x40

         ; VECTOR Z+ BUTTON
        mov     eax,8           ; function 8 : define and draw button
        mov     ebx,(SIZE_X+10+41)*65536+62-41     ; [x start] *65536 + [x size]
        mov     ecx,(25+15*6)*65536+12  ; [y start] *65536 + [y size]
        mov     edx,8           ; button id
        mov     esi,0x6688dd    ; button color RRGGBB
        int     0x40

         ;VECTOR Z+ LABEL
        mov     eax,4           ; function 4 : write text to window
        mov     ebx,(SIZE_X+12+41)*65536+(28+15*6)   ; [x start] *65536 + [y start]
        mov     ecx,0x20ddeeff  ; font 1 & color ( 0xF0RRGGBB )
        mov     edx,labelzplus      ; pointer to text beginning
        mov     esi,labelzplusend-labelzplus     ; text length
        int     0x40

         ; VECTOR x- BUTTON
        mov     eax,8           ; function 8 : define and draw button
        mov     ebx,(SIZE_X+10)*65536+62-41     ; [x start] *65536 + [x size]
        mov     ecx,(25+15*7)*65536+12  ; [y start] *65536 + [y size]
        mov     edx,9           ; button id
        mov     esi,0x6688dd    ; button color RRGGBB
        int     0x40

         ;VECTOR x- LABEL
        mov     eax,4           ; function 4 : write text to window
        mov     ebx,(SIZE_X+12)*65536+(28+15*7)   ; [x start] *65536 + [y start]
        mov     ecx,0x20ddeeff  ; font 1 & color ( 0xF0RRGGBB )
        mov     edx,labelxminus      ; pointer to text beginning
        mov     esi,labelxminusend-labelxminus     ; text length
        int     0x40
        ; VECTOR x+ BUTTON
        mov     eax,8           ; function 8 : define and draw button
        mov     ebx,(SIZE_X+10+41)*65536+62-41     ; [x start] *65536 + [x size]
        mov     ecx,(25+15*7)*65536+12  ; [y start] *65536 + [y size]
        mov     edx,10           ; button id
        mov     esi,0x6688dd    ; button color RRGGBB
        int     0x40
        ;VECTOR x+ LABEL
        mov     eax,4           ; function 4 : write text to window
        mov     ebx,(SIZE_X+12+41)*65536+(28+15*7)   ; [x start] *65536 + [y start]
        mov     ecx,0x20ddeeff  ; font 1 & color ( 0xF0RRGGBB )
        mov     edx,labelxplus      ; pointer to text beginning
        mov     esi,labelxplusend-labelxplus     ; text length
        int     0x40
        ; VECTOR z- BUTTON
        mov     eax,8           ; function 8 : define and draw button
        mov     ebx,(SIZE_X+10)*65536+62-41     ; [x start] *65536 + [x size]
        mov     ecx,(25+15*8)*65536+12  ; [y start] *65536 + [y size]
        mov     edx,11           ; button id
        mov     esi,0x6688dd    ; button color RRGGBB
        int     0x40
        ;VECTOR z- LABEL
        mov     eax,4           ; function 4 : write text to window
        mov     ebx,(SIZE_X+12)*65536+(28+15*8)   ; [x start] *65536 + [y start]
        mov     ecx,0x20ddeeff  ; font 1 & color ( 0xF0RRGGBB )
        mov     edx,labelzminus      ; pointer to text beginning
        mov     esi,labelzminusend-labelzminus     ; text length
        int     0x40
       ;VECTOR Y+ BUTTON
        mov     eax,8           ; function 8 : define and draw button
        mov     ebx,(SIZE_X+10+20)*65536+62-42     ; [x start] *65536 + [x size]
        mov     ecx,(25+15*8)*65536+12  ; [y start] *65536 + [y size]
        mov     edx,12           ; button id
        mov     esi,0x6688dd    ; button color RRGGBB
        int     0x40
        ;VECTOR Y+ LABEL
        mov     eax,4           ; function 4 : write text to window
        mov     ebx,(SIZE_X+12+20)*65536+(28+15*8)   ; [x start] *65536 + [y start]
        mov     ecx,0x20ddeeff  ; font 1 & color ( 0xF0RRGGBB )
        mov     edx,labelyplus      ; pointer to text beginning
        mov     esi,labelyplusend-labelyplus     ; text length
        int     0x40
        ; LEAD COLOR LABEL
        mov     eax,4           ; function 4 : write text to window
        mov     ebx,(SIZE_X+12)*65536+(28+15*9)   ; [x start] *65536 + [y start]
        mov     ecx,0x20ddeeff  ; font 1 & color ( 0xF0RRGGBB )
        mov     edx,labelmaincolor      ; pointer to text beginning
        mov     esi,labelmaincolorend-labelmaincolor     ; text length
        int     0x40

        ;RED+ BUTTON
        mov     eax,8           ; function 8 : define and draw button
        mov     ebx,(SIZE_X+10)*65536+62-41     ; [x start] *65536 + [x size]
        mov     ecx,(25+15*10)*65536+12  ; [y start] *65536 + [y size]
        mov     edx,13           ; button id
        mov     esi,0x6688dd    ; button color RRGGBB
        int     0x40
        ;RED+  LABEL
        mov     eax,4           ; function 4 : write text to window
        mov     ebx,(SIZE_X+12)*65536+(28+15*10)   ; [x start] *65536 + [y start]
        mov     ecx,0x20ddeeff  ; font 1 & color ( 0xF0RRGGBB )
        mov     edx,labelredplus      ; pointer to text beginning
        mov     esi,labelredplusend-labelredplus     ; text length
        int     0x40
        ;GREEN+ BUTTON
        mov     eax,8           ; function 8 : define and draw button
        mov     ebx,(SIZE_X+10+20)*65536+62-42     ; [x start] *65536 + [x size]
        mov     ecx,(25+15*10)*65536+12  ; [y start] *65536 + [y size]
        mov     edx,14           ; button id
        mov     esi,0x6688dd    ; button color RRGGBB
        int     0x40
        ;GREEN+ LABEL
        mov     eax,4           ; function 4 : write text to window
        mov     ebx,(SIZE_X+12+20)*65536+(28+15*10)   ; [x start] *65536 + [y start]
        mov     ecx,0x20ddeeff  ; font 1 & color ( 0xF0RRGGBB )
        mov     edx,labelgreenplus      ; pointer to text beginning
        mov     esi,labelgreenplusend-labelgreenplus     ; text length
        int     0x40
        ;BLUE+ BUTTON
        mov     eax,8           ; function 8 : define and draw button
        mov     ebx,(SIZE_X+10+41)*65536+62-41     ; [x start] *65536 + [x size]
        mov     ecx,(25+15*10)*65536+12  ; [y start] *65536 + [y size]
        mov     edx,15           ; button id
        mov     esi,0x6688dd    ; button color RRGGBB
        int     0x40
        ;BLUE+ LABEL
        mov     eax,4           ; function 4 : write text to window
        mov     ebx,(SIZE_X+12+41)*65536+(28+15*10)   ; [x start] *65536 + [y start]
        mov     ecx,0x20ddeeff  ; font 1 & color ( 0xF0RRGGBB )
        mov     edx,labelblueplus      ; pointer to text beginning
        mov     esi,labelblueplusend-labelblueplus     ; text length
        int     0x40
        ;RED- BUTTON
        mov     eax,8           ; function 8 : define and draw button
        mov     ebx,(SIZE_X+10)*65536+62-41     ; [x start] *65536 + [x size]
        mov     ecx,(25+15*11)*65536+12  ; [y start] *65536 + [y size]
        mov     edx,16           ; button id
        mov     esi,0x6688dd    ; button color RRGGBB
        int     0x40
        ;RED-  LABEL
        mov     eax,4           ; function 4 : write text to window
        mov     ebx,(SIZE_X+12)*65536+(28+15*11)   ; [x start] *65536 + [y start]
        mov     ecx,0x20ddeeff  ; font 1 & color ( 0xF0RRGGBB )
        mov     edx,labelredminus      ; pointer to text beginning
        mov     esi,labelredminusend-labelredminus     ; text length
        int     0x40
        ;GREEN- BUTTON
        mov     eax,8           ; function 8 : define and draw button
        mov     ebx,(SIZE_X+10+20)*65536+62-42     ; [x start] *65536 + [x size]
        mov     ecx,(25+15*11)*65536+12  ; [y start] *65536 + [y size]
        mov     edx,17           ; button id
        mov     esi,0x6688dd    ; button color RRGGBB
        int     0x40
        ;GREEN- LABEL
        mov     eax,4           ; function 4 : write text to window
        mov     ebx,(SIZE_X+12+20)*65536+(28+15*11)   ; [x start] *65536 + [y start]
        mov     ecx,0x20ddeeff  ; font 1 & color ( 0xF0RRGGBB )
        mov     edx,labelgreenminus      ; pointer to text beginning
        mov     esi,labelgreenminusend-labelgreenminus     ; text length
        int     0x40
        ;BLUE- BUTTON
        mov     eax,8           ; function 8 : define and draw button
        mov     ebx,(SIZE_X+10+41)*65536+62-41     ; [x start] *65536 + [x size]
        mov     ecx,(25+15*11)*65536+12  ; [y start] *65536 + [y size]
        mov     edx,18           ; button id
        mov     esi,0x6688dd    ; button color RRGGBB
        int     0x40
        ;BLUE- LABEL
        mov     eax,4           ; function 4 : write text to window
        mov     ebx,(SIZE_X+12+41)*65536+(28+15*11)   ; [x start] *65536 + [y start]
        mov     ecx,0x20ddeeff  ; font 1 & color ( 0xF0RRGGBB )
        mov     edx,labelblueminus      ; pointer to text beginning
        mov     esi,labelblueminusend-labelblueminus     ; text length
        int     0x40
        ; Catmull  LABEL
        mov     eax,4           ; function 4 : write text to window
        mov     ebx,(SIZE_X+12)*65536+(28+15*12)   ; [x start] *65536 + [y start]
        mov     ecx,0x20ddeeff  ; font 1 & color ( 0xF0RRGGBB )
        mov     edx,labelcatmullmod      ; pointer to text beginning
        mov     esi,labelcatmullmodend-labelcatmullmod     ; text length
        int     0x40
        ; on/off BUTTON
        mov     eax,8           ; function 8 : define and draw button
        mov     ebx,(SIZE_X+10)*65536+62     ; [x start] *65536 + [x size]
        mov     ecx,(25+15*13)*65536+12  ; [y start] *65536 + [y size]
        mov     edx,19           ; button id
        mov     esi,0x6688dd    ; button color RRGGBB
        int     0x40
         ; on/off LABEL
        mov     eax,4           ; function 4 : write text to window
        mov     ebx,(SIZE_X+12)*65536+(28+15*13)   ; [x start] *65536 + [y start]
        mov     ecx,0x20ddeeff  ; font 1 & color ( 0xF0RRGGBB )
        mov     edx,labelonoff      ; pointer to text beginning
        mov     esi,labelonoffend-labelonoff     ; text length
        int     0x40


        mov     eax,12          ; function 12:tell os about windowdraw
        mov     ebx,2           ; 2, end of draw
        int     0x40

        ret


        ; DATA AREA
        i3              dw      3
        light_vector    dd      0.0,0.0,-1.0
     ;  debug_vector    dd      0.0,2.0,2.0   ;--debug
     ;  debug_vector1   dd      0.0,0.0,0.0
     ;  debug_points    dw      1,1,1,3,4,20
     ;  debug_dd        dw      ?
     ;  debug_dwd       dd      65535
     ;  debug_counter   dw      0
        dot_max         dd      1.0
        dot_min         dd      0.0
        env_const       dd      1.2
        correct_tex     dw      255
        max_color_r     dw      33
        max_color_g     dw      255
        max_color_b     dw      110
        xobs            dw      SIZE_X / 2 ;200 ;observer
        yobs            dw      SIZE_Y / 2 ;200 ;coordinates
        zobs            dw      -500


        angle_counter dw 0
        piD180        dd 0.017453292519943295769236907684886
      ;  sq            dd 0.70710678118654752440084436210485
        const6        dw 6,6,6,6
        xo            dw 130;87
        zo            dw 0
        yo            dw 100
        xoffset       dd 130.0
        yoffset       dd 150.0
        sscale        dd 2.0             ; real scale
        vect_x        dw 0
        vect_y        dw 0
        vect_z        dw 0


        r_flag        db 0
        dr_flag       db 0
        speed_flag    db 0
        catmull_flag  db 0
    SourceFile file 'teapot.3ds'
    EndFile:
    labelrot:
        db   'rot. mode'
    labelrotend:
    labeldrmod:
        db   'shd. mode'
    labeldrmodend:
    labelspeedmod:
        db   'spd. mode'
    labelspeedmodend:
    labelzoomout:
        db   'zoom out'
    labelzoomoutend:
    labelzoomin:
        db   'zoom in'
    labelzoominend:
    labelvector:
        db   'add vector'
    labelvectorend:
    labelyminus:
        db   'y -'
    labelyminusend:
    labelzplus:
        db   'z +'
    labelzplusend:
    labelxminus:
        db   'x -'
    labelxminusend:
    labelxplus:
        db   'x +'
    labelxplusend:
    labelzminus:
        db   'z -'
    labelzminusend:
    labelyplus:
        db   'y +'
    labelyplusend:
    labelmaincolor:
        db   'main color'
    labelmaincolorend:
    labelredplus:
        db   'r +'
    labelredplusend:
    labelgreenplus:
        db   'g +'
    labelgreenplusend:
    labelblueplus:
        db   'b +'
    labelblueplusend:
    labelredminus:
        db   'r -'
    labelredminusend:
    labelgreenminus:
        db   'g -'
    labelgreenminusend:
    labelblueminus:
        db   'b -'
    labelblueminusend:
    labelcatmullmod:
        db 'catmull'
    labelcatmullmodend:
    labelonoff:
        db 'on/off'
    labelonoffend:
    labelt:
        db   'AVE CRUCE SALUS MEA'
      if Ext=MMX
        db   ' (MMX)'
      end if
    labellen:
        STRdata db '-1        '
align 8
        @col    dd      ?
        @y1     dw      ?
        @x1     dw      ?;+8
        @y2     dw      ?
        @x2     dw      ?
        @y3     dw      ?
        @x3     dw      ?;+16

        @dx12   dd      ?
        @dx13   dd      ?;+24
        @dx23   dd      ?

        sinbeta dd      ?;+32
        cosbeta dd      ?

        xsub    dw      ?
        zsub    dw      ?;+40
        ysub    dw      ?

        xx1     dw      ?
        yy1     dw      ?
        zz1     dw      ?;+48
        xx2     dw      ?
        yy2     dw      ?
        zz2     dw      ?
        xx3     dw      ?;+56
        yy3     dw      ?
        zz3     dw      ?
        scale   dd      ?                 ; help scale variable
        ;screen  dd      ?                ;pointer to screen buffer
        triangles_count_var dw ?
        points_count_var    dw ?


        point_index1        dw ?   ;-\
        point_index2        dw ?   ;  }  don't change order
        point_index3        dw ?   ;-/
        temp_col            dw ?
        high    dd      ?
align 8
        buffer  dq      ?

        err     dd      ?
        drr     dd      ?
        xx      dd      ?
        yy      dd      ?
        xst     dd      ?
        yst     dd      ?
align 16
    points:
        rw (EndFile-SourceFile)/12*3
        points_count = ($-points)/6
    triangles:
        rw (EndFile-SourceFile)/12*3
        triangles_count = ($-triangles)/6

align 16
        points_rotated rw points_count*3 + 2
align 16
        label trizdd dword
        label trizdq qword
        triangles_with_z rw triangles_count*4 + 2 ; triangles triple dw + z position
align 16
        triangles_normals rb triangles_count * 12 ;
        point_normals rb points_count * 12  ;one 3dvector - triple float dword x,y,z
align 16
        point_normals_rotated rb points_count * 12
align 16
        vectors rb 24
        points_color rb 6*points_count    ; each color as word
;        sorted_triangles rw triangles_count*3 + 2
;align 16
        screen rb SIZE_X * SIZE_Y * 3   ; screen buffer
        Z_buffer rb SIZE_X * SIZE_Y * 4
        envmap   rb 512*512*3
        memStack rb 1000 ;memory area for stack
    I_END:
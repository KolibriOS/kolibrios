                   ; DATA AREA  ************************************
    ;  if Ext > SSE2
        isSSE3          db      1
    ;  end if
        i3              dw      3
        i6              dd      6
        i12             dd      12
        i36             dd      36
        i256            dw      256
        i255d           dd      255
        f1:
        dot_max         dd      1.0      ; dot product max and min
        dot_min         dd      0.0
        env_const       dd      1.05
        correct_tex     dw      255
        tex_x_div2      dw      TEX_X  / 2
        tex_y_div2      dw      TEX_Y  / 2
        xobs            dw      0 ;SIZE_X / 2 ;200 ;observer  = camera
        yobs            dw      0 ;SIZE_Y / 2 ;200 ;coordinates
        zobs            dw      -1000


        angle_counter dw 0
        piD180        dd 0.017453292519943295769236907684886
        piD128        dd 0.024544
        const6        dw 6,6,6,6
        x_offset      dw SIZE_X / 2
        y_offset      dw SIZE_Y / 2
        z_offset      dw 0
        rsscale       dd 175.0           ; next real scale

;        vect_x:        dw SIZE_X / 2
;        vect_y        dw SIZE_Y / 2
;        vect_z        dw 0
;                size_y_var:
;        yres_var                dw SIZE_Y
;
;        size_x_var:
;        xres_var                dw SIZE_X

        angle_x       dw 0
        angle_y       dw 0
        angle_z       dw 0
        sin_amplitude dd 50
        sin_frq       dd 0.7
        sin_delta     dd 0.07    ; wave frequency granularity
        convert_muler:
                      dd 1, 10, 100, 1000, 10000
        XYZpartices:
                      db 'X','Y','Z'
        i10           dw 10
        offset_y:
        i25           dw 25  ; screen buff offset
        offset_x:
        i5            dw 5
        triangles_ptr           dd 0
        triangles_w_z_ptr       dd 0
        triangles_normals_ptr   dd 0
        points_normals_ptr      dd 0
        points_normals_rot_ptr  dd 0
        points_ptr              dd 0
        points_rotated_ptr      dd 0
        points_translated_ptr   dd 0
        edges_ptr               dd 0
        screen_ptr              dd 0
        Zbuffer_ptr             dd 0
        vertices_index_ptr      dd 0
        vertex_edit_no          dd -1
        edit_start_x:
                          dw 0
        edit_start_y      dw 0
        edit_end_x:
                          dw 0
        edit_end_y        dw 0
        x_start:          dw 20
        y_start:          dw 20

        mouse_state           dd 0


        menu:
               db 2                ; button number  = index
               db 'rotary    '     ; label
               db 4                ; max flag  + 1 , if = 255, no flag
 r_flag        db 1                ; flag
               dd axl_f            ; offset to flags description

               db 3
               db 'shd. model'
 max_dr_flg:
            if Ext >= SSE3
                db 15
            else
                db 12
            end if
 dr_flag       db 0  ;  6 - dots
               dd shd_f

               db 4
               db 'speed     '
               db 2
 speed_flag    db 0
               dd spd_f

               db 5
               db 'zoom out  '
               db 255
               db ?
               dd ?

               db 6
               db 'zoom in   '
               db 255
               db ?
               dd ?

               db 7
               db 'ray shadow'
               db 2
ray_shd_flag   db 0
               dd onoff_f

               db 8
               db 'culling   '
               db 2
culling_flag   db 0
               dd onoff_f

               db 9
               db 'rand.light'
               db 255
               db ?
               dd ?

               db 10
               db 'blur      '
               db 6
blur_flag      db 0
               dd blur_f

               db 11
               db 'mirror x  '
               db 2
mirr_x_flag    db 0
               dd onoff_f

               db 12
               db 'mirror y  '
               db 2
mirr_y_flag    db 0
               dd onoff_f

               db 13
               db 'mirror z  '
               db 2
mirr_z_flag    db 0
               dd onoff_f

               db 14
               db 'xchg      '
               db 4
xchg_flag      db 0
               dd xchg_f

               db 15
               db 'emboss    '
               db 2
emboss_flag    db 0
               dd onoff_f

               db 16
               db 'fire      '
               db 2
fire_flag      db 0
               dd blur_f

               db 17
               db 'move      '
               db 3
move_flag      db 0
               dd move_f

                  db 18
                  db 'generate  '
                  db 6
generator_flag    db 0
                  dd blur_f

                  db 19
                  db 'bumps     '
                  db 2
bumps_flag        db 0
                  dd bumps_f

                  db 20
                  db 'bumps deep'
                  db 4
bumps_deep_flag   db 3
                  dd bumps_d_f

                  db 21
                  db 're-map tex'
                  db 255
map_tex_flag      db ? ;1
                  dd ?  ;bumps_d_f

                  db 22
                  db 'bright +  '
                  db 6
inc_bright_flag   db 0 ;1
                  dd blur_f

                  db 23
                  db 'bright -  '
                  db 6
dec_bright_flag   db 0 ;1
                  dd blur_f

                  db 24
                  db 'wav effect'
                  db 2
sinus_flag        db 0
                  dd onoff_f


                  db 25
                  db 'editor    '
                  db 2
 edit_flag        db 0
                  dd onoff_f
;                  db 24
;                  db 'max       '
;                  db 2
;max_flag          db 0 ;1
;                  dd blur_f
;
;                  db 25
;                  db 'min       '
;                  db 2
;min_flag          db 0 ;1
;                  dd blur_f



;                  db 21
;                  db 'light No. '
;                  db 3
;light_no_flag     db 0
;                  dd bumps_d_f

;                db 22
;                db 'light comp'
;                db 3
;light_comp_flag db 0
;                dd light_component_f

;;                db 23
;;                db 'col. comp'
;;                db 3
;;color_comp_flag db 0
;;                dd color_component_f





               db -1       ; end mark


flags:             ; flags description
 shd_f:
    db 'flat'
    db 'grd '
    db 'env '
    db 'bump'
    db 'tex '
    db 'pos '
    db 'dots'
    db 'txgr'
    db '2tex'
    db 'btex'
    db 'cenv'
    db 'grdl'
    db 'rphg'
    db 'glas'
    db 'ptex'
 spd_f:
    db 'idle'
    db 'full'
 axl_f:
    db ' y  '
    db 'x+y '
    db ' x  '
    db 'keys'

 onoff_f:
    db 'off '
    db 'on  '

          blur_f:              ; blur, fire
            db 'off '
bumps_d_f:  db ' 1  '
            db ' 2  '
            db ' 3  '
            db ' 4  '
            db ' 5  '

 xchg_f:
    db 'no  '
    db 'x<>y'
    db 'z<>x'
    db 'y<>z'
 move_f:
    db 'obj '
    db 'camr'
    db 'wave'
;    db 'lght'
 bumps_f:
    db 'rand'
    db 'tex '
;    db 'cscl'
base_vector:
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


    labelt:
        db   'DEUS CARITAS EST'
      if Ext=MMX
        db   ' (MMX)'
      end if
      if Ext=SSE
        db   ' (SSE)'
      end if
      if Ext=SSE2
        db   ' (SSE2)'
      end if
      if Ext=SSE3
        db   ' (SSE3)'
      end if
        db   ' 0.077',0
    labellen:
        STRdata db '-1        '
     lab_vert:
        db 'Vertices count: '
     lab_vert_end:
     lab_tr:
        db 'Faces count:    '
     lab_tr_end:
     lab_ed:
        db 'Edges count:    '
     lab_ed_end:

        db 'Chunks detected:'



    all_lights_size dw lightsend-lights


;        file_info:
;                dd      0
;                dd      0
;                dd      0
;         fsize  dd      0 ;180000 ; sizeof(workarea)
;         fptr   dd      0 ;workarea
;        file_name:
;                db      '/sys/3d/house.3ds',0
;         ;      db      '/tmp0/1/ant.3ds',0
;
;      rb 256

;=============================================
lights:
     .light_vector        dd      0.0,0.0,-1.0   ; x,y,z  Z cooficient of vector must be negative
     .orginal_color_r     db      1              ;  +12
     .orginal_color_g     db      255              ;
     .orginal_color_b     db      1              ;  +14
     .min_color_r         db      1              ;
     .min_color_g         db      1              ;  +16
     .min_color_b         db      1              ;
     .max_color_r         db      255            ;
     .max_color_g         db      255            ;
     .max_color_b         db      255            ;
     .shine               db      24             ;  +21
 ;    LIGHT_SIZE equ ($-lights)

        dd      -0.5,-0.5,-1.0   ; x,y,z ;  .light_vector
        db      5                ; .orginal_color_r
        db      1                ; .orginal_color_g
        db      135              ; .orginal_color_b
        db      19               ; .min_color_r
        db      19               ; .min_color_g
        db      19               ; .min_color_b
        db      255              ; .max_color_r
        db      255              ; .max_color_g
        db      255              ; .max_color_b
        db      16               ; .shine

        dd      0.5,0.5,-1.0    ; x,y,z ;  .light_vector
        db      135             ; .orginal_color_r
        db      1               ; .orginal_color_g
        db      1               ; .orginal_color_b
        db      19              ; .min_color_r
        db      19              ; .min_color_g
        db      19              ; .min_color_b
        db      255             ; .max_color_r
        db      255             ; .max_color_g
        db      20              ; .max_color_b
        db      16              ; .shine
;        ALL_LIGHTS_SIZE equ ($ - lights)
;#all_lights_size dw ($ - lights) ;ALL_LIGHTS_SIZE
;===============================================

lightsend:



;if Ext >= SSE3
align 16
   point_light_coords:
              dd 50.0
              dd 50.0
              dd -215.0
              dd 0.0


              dd 815.0
              dd 815.0
              dd -215.0
              dd 0.0

              dd 1500.0
              dd 1500.0
              dd -215.0
              dd 0.0
if 0
    aabb1:
        .0              dd 1.0,1.0,1.0,0
        .1              dd -1.0,1.0,1.0,0
        .2              dd 1.0,-1.0,1.0,0
        .3              dd -1.0,-1.0,1.0,0
        .4              dd 1.0,1.0,-1.0,0
        .5              dd -1.0,1.0,-1.0,0
        .6              dd 1.0,-1.0,-1.0,0
        .7              dd -1.0,-1.0,-1.0,0

end if

    sign_mask:
              times 4 dd 0x80000000
    f05xz:    dd 0, 0,   - 1.0  ,0

    sign_z:
              dd -1,-1,0x7fffffff,0
    abs_mask:
           dd 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff
    emboss_bias:
           dw 128, 128, 128, 128, 128, 128, 128, 128
    zero_hgst_dd:
           dd -1, -1, -1, 0
    mask_255f:
    correct_texf:
           times 4 dd 255.0
    the_zero:
           times 4 dd 0.0
    tex_m2:
           times 4 dd 510.0
    the_one:
           times 4 dd 1.0

     eps:  times 4 dd 0.000001

        vect_x:        dw SIZE_X / 2
        vect_y        dw SIZE_Y / 2
        vect_z        dw 0
                size_y_var:
        yres_var                dw SIZE_Y

        size_x_var:
        xres_var                dw SIZE_X


     epsone      dd 1.00001
     aprox       dd 0.0001
     epsminus    dd 0.00001


        file_info:
                dd      0
                dd      0
                dd      0
         fsize  dd      0 ;180000 ; sizeof(workarea)
         fptr   dd      0 ;workarea
        file_name:
               db      '/sys/3d/house.3ds',0
             ;  db      '/tmp0/1/bmwm3.3ds',0

      rb 1024


I_END:
SourceFile:
workarea   rb 180
EndFile   dd ?
align 8
        sinbeta dd      ?;
        cosbeta dd      ?

        xsub    dw      ?
        zsub    dw      ?
        ysub    dw      ?

        col1    dd      ?
        col2    dd      ?
        col3    dd      ?
        scale   dd      ? ; help scale variable

        points_count_var        dd ?  ;
        triangles_count_var     dd ?  ; dont change order
        edges_count             dd ?  ;
        chunks_number           dd ?
        tex_points_ptr          dd ?
        chunks_ptr              dd ?

        temp_col            dw ?
        high                dd ?
        rand_seed           dw ?
align 8
        buffer  dq      ?
        errr    dd      ?
        drr     dd      ?
        xx      dd      ?
        yy      dd      ?
        xst     dd      ?
        yst     dd      ?
;        screen_ptr      dd     ?
;        Zbuffer_ptr     dd     ?

        matrix  rb 36
        cos_tab rd 360
        sin_tab rd 360
align 16
       lights_aligned:
       lights_aligned_end = $ + 16 * 12
                        rb 16 * 12

align 16
        vectors rb 24

align 16
        bumpmap         rb      TEXTURE_SIZE + 1
align 16
        bumpmap2        rb      TEXTURE_SIZE + 1
align 16
        envmap          rb      (TEXTURE_SIZE +1) * 3
align 16
        envmap_cub      rb      TEX_X * 3
align 16
        texmap          rb      (TEXTURE_SIZE +1) * 3
align 16
        color_map       rb      (TEXTURE_SIZE +100) * 3
align 16
 ;       tex_points      rb      points_count * 4  ; bump_map  and texture coords
 ;                               ; each point word x, word y
;align 16
;       lights_aligned:
;       lights_aligned_end = $ + 16 * 12
;                        rb 16 * 12


if Ext >= SSE2
        sse_repository  rb      1024
end if
        procinfo:
                        rb     2048 ; process info
        I_Param         rb      256
        memStack:
        rb 2000
align  16
screen:

;    Copyright (C) 2014 Anton_K
;
;    This program is free software: you can redistribute it and/or modify
;    it under the terms of the GNU General Public License as published by
;    the Free Software Foundation, either version 2 of the License, or
;    (at your option) any later version.
;
;    This program is distributed in the hope that it will be useful,
;    but WITHOUT ANY WARRANTY; without even the implied warranty of
;    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
;    GNU General Public License for more details.
;
;    You should have received a copy of the GNU General Public License
;    along with this program. If not, see <http://www.gnu.org/licenses/>.

include '../../../develop/libraries/libs-dev/libio/libio.inc'
include '../../../develop/libraries/libs-dev/libimg/libimg.inc'

include 'datadef.inc'

; ============================================================================ ;
; < eax = 0 - fail                                                             ;
; ============================================================================ ;
proc akode.init uses ebx ecx edx, plane_width, plane_height, fov, block_base_size, block_height
        bsr     ecx, [block_base_size]
        xor     eax, eax
        inc     eax
        shl     eax, cl
        mov     [akode_data.BlockWidthPowerOf2], ecx
        mov     [akode_data.BlockSize.Width], eax

        mov     eax, [block_height]
        mov     [akode_data.BlockSize.Height], eax

        shr     eax, 1
        mov     [akode_data.Camera.Position.Z], eax

        xor     eax, eax
        mov     [akode_data.Camera.Position.X], eax
        mov     [akode_data.Camera.Position.Y], eax
        mov     [akode_data.Camera.Direction], eax
        mov     [akode_data.OptimizedGetImage], eax
        dec     eax
        mov     [akode_data.MovementDirection], eax
        mov     [akode_data.TurningDirection], eax
        inc     eax
        stdcall akode.set_shading, eax, eax
        test    eax, eax
        jz      .exit

        mov     edx, [plane_width]

        mov     ecx, edx
        shl     ecx, 2
        mcall   68, 12                                          ; alloc width * 4 bytes

        test    eax, eax
        jz      .exit

        mov     [akode_data.DepthBufferPtr], eax

        mov     eax, [plane_height]
        mov     ecx, eax
        shl     ecx, 2
        imul    ecx, edx                                        ; ecx = height * 4 * width

        mov     [akode_data.ProjectionPlane.Size.Height], eax
        mov     [akode_data.ProjectionPlane.Size.Width], edx
        mov     [akode_data.ImageBufferSize], ecx

        mov     ebx, eax
        or      ebx, edx
        test    ebx, 0Fh
        jnz     @f
        inc     [akode_data.OptimizedGetImage]
@@:

        shr     eax, 1
        mov     [akode_data.ProjectionPlane.MidY], eax          ; MidY = height / 2

        mcall   68, 12                                          ; alloc ecx bytes

        test    eax, eax
        jz      .exit

        mov     [akode_data.ImageBufferPtr], eax
        DEBUGF  DEBUG_FINE, 'akode_data.ImageBufferPtr: %x\n', eax:8
        DEBUGF  DEBUG_FINE, 'akode_data.OptimizedGetImage: %u\n', [akode_data.OptimizedGetImage]

        mov     ecx, [fov]
        mov     ebx, edx
        imul    eax, edx, 360
        xor     edx, edx
        div     ecx                                             ; eax = width * 360 / fov

        mov     [akode_data.Camera.FieldOfView], ecx

        mov     [akode_data.Angle360], eax

        stdcall akode._.calc_tables

        test    eax, eax
        jz      .exit

        shr     ebx, 1                                          ; ebx = width / 2
        push    ebx
        shl     ebx, 5                                          ; ebx * 32
        mov     eax, [akode_data.TrigonometricTablePtr]

        fild    dword [esp]                                     ; width / 2
        fdiv    qword [eax + ebx + 16]                          ; tan (fov / 2)
        fistp   dword [esp]

        pop     eax
        mov     [akode_data.CameraToPlaneDistance], eax
        mov     ebx, eax

        mul     [block_height]
        mov     [akode_data.WallHeightDividend], eax

        mov     eax, [block_height]
        sub     eax, [akode_data.Camera.Position.Z]
        mul     ebx
        mov     [akode_data.CeilingDistanceDividend], eax

        imul    ebx, [akode_data.Camera.Position.Z]
        mov     [akode_data.FloorDistanceDividend], ebx

        DEBUGF  DEBUG_FINE, 'akode_data.CameraToPlaneDistance: %u\n', [akode_data.CameraToPlaneDistance]

        stdcall akode.set_movement_speed, [akode_data.BlockSize.Width], 90

.exit:
        ret
endp
; ============================================================================ ;

; ============================================================================ ;
proc akode.cleanup uses eax ebx ecx edx
        stdcall akode.unload_current_level

        xor     edx, edx
        mov     ebx, 13

        mov     ecx, [akode_data.ImageBufferPtr]
        test    ecx, ecx
        jz      @f
        mcall   68                                              ; free
        mov     [akode_data.ImageBufferPtr], edx
@@:
        mov     ecx, [akode_data.DepthBufferPtr]
        test    ecx, ecx
        jz      @f
        mcall   68
        mov     [akode_data.DepthBufferPtr], edx
@@:
        mov     ecx, [akode_data.TrigonometricTablePtr]
        test    ecx, ecx
        jz      @f
        mcall   68
        mov     [akode_data.TrigonometricTablePtr], edx
@@:
        mov     ecx, [akode_data.BlockWidthTanTablePtr]
        test    ecx, ecx
        jz      @f
        mcall   68
        mov     [akode_data.BlockWidthTanTablePtr], edx
@@:
        mov     ecx, [akode_data.ShadingTablePtr]
        test    ecx, ecx
        jz      @f
        mcall   68
        mov     [akode_data.ShadingTablePtr], edx
@@:

        ret
endp
; ============================================================================ ;

; ============================================================================ ;
; > level_load_callback = proc callback AKODE_LEVEL_LOAD, result               ;
; > action_callback     = proc callback AKODE_ACTION, cell x, cell y, result   ;
; ============================================================================ ;
proc akode.set_callbacks level_load_callback, action_callback
        m2m     [akode_data.LevelLoadCallback], [level_load_callback]
        m2m     [akode_data.ActionCallback], [action_callback]
        ret
endp
; ============================================================================ ;

; ============================================================================ ;
; < eax = 0 - failed to load some textures or create shading table             ;
; ============================================================================ ;
proc akode.load_level uses ebx ecx edx esi edi, level_ptr
        locals
                load_texture_results    dd 1
        endl

        stdcall akode.unload_current_level

        mov     ebx, akode_data.CurrentLevel

        cld
        mov     esi, [level_ptr]
        mov     edi, ebx
        mov     ecx, sizeof.akode.LevelHeader
        rep     movsb

        mov     [akode_data.CurrentLevelGridPtr], esi

        mov     eax, [ebx + akode.LevelHeader.InitCallback]
        test    eax, eax
        jz      @f
        stdcall eax

@@:
        mov     edx, [akode_data.LevelLoadCallback]
        test    edx, edx
        jz      @f
        stdcall edx, AKODE_LEVEL_LOAD.START, eax

@@:
        mov     ecx, [ebx + akode.LevelHeader.Size.Width]
        imul    ecx, [ebx + akode.LevelHeader.Size.Height]

.load_textures_loop:
        mov     edi, [esi]
        add     esi, 4

        test    edi, edi
        jz      .skip

        push    ecx

        mov     ecx, 6 * 2                                      ; 6 combined textures per cell

.load_cell_textures_loop:
        mov     eax, [edi]
        add     edi, 4

        test    eax, eax
        jz      @f

        stdcall akode.load_texture, eax

        test    eax, eax
        jnz     @f
        mov     [load_texture_results], eax

@@:
        sub     ecx, 1
        jnz     .load_cell_textures_loop

        pop     ecx

.skip:
        sub     ecx, 1
        jnz     .load_textures_loop

        mov     ecx, [ebx + akode.LevelHeader.ObjectCount]
        test    ecx, ecx
        jz      @f

        mov     [akode_data.CurrentLevelObjectsPtr], esi

.load_object_textures_loop:
        mov     eax, [esi]
        add     esi, sizeof.akode.Object

        test    eax, eax
        jz      .skip_object

        stdcall akode.load_texture, eax

        test    eax, eax
        jnz     .skip_object
        mov     [load_texture_results], eax

.skip_object:
        sub     ecx, 1
        jnz     .load_object_textures_loop

@@:
        mov     ecx, [ebx + akode.LevelHeader.TextureCount]
        test    ecx, ecx
        jz      @f

        mov     [akode_data.CurrentLevelAddTexturesPtr], esi

.load_additional_textures_loop:
        mov     eax, [esi]
        add     esi, 4

        test    eax, eax
        jz      .skip_additional

        stdcall akode.load_texture, eax

        test    eax, eax
        jnz     .skip_additional
        mov     [load_texture_results], eax

.skip_additional:
        sub     ecx, 1
        jnz     .load_additional_textures_loop

@@:
        mov     edi, [akode_data.BlockSize.Width]
        shr     edi, 1
        mov     ecx, [akode_data.BlockWidthPowerOf2]

        mov     eax, [ebx + akode.LevelHeader.StartPosition.X]
        shl     eax, cl
        add     eax, edi
        mov     [akode_data.Camera.Position.X], eax

        mov     eax, [ebx + akode.LevelHeader.StartPosition.Y]
        shl     eax, cl
        add     eax, edi
        mov     [akode_data.Camera.Position.Y], eax

        mov     eax, [ebx + akode.LevelHeader.StartDirection]
        imul    eax, [akode_data.Angle90]
        mov     [akode_data.Camera.Direction], eax

        xor     eax, eax
        dec     eax
        mov     [akode_data.MovementDirection], eax
        mov     [akode_data.TurningDirection], eax

        stdcall akode.set_shading, [ebx + akode.LevelHeader.ShadingColor], [ebx + akode.LevelHeader.ShadingDistance]
        test    eax, eax
        jz      @f
        mov     eax, [load_texture_results]
@@:

        test    edx, edx
        jz      @f
        stdcall edx, AKODE_LEVEL_LOAD.END, eax

@@:
        ret
endp
; ============================================================================ ;

; ============================================================================ ;
proc akode.unload_current_level uses eax ecx edx esi edi
        mov     esi, [akode_data.CurrentLevelGridPtr]
        test    esi, esi
        jz      .exit

        mov     ecx, [akode_data.CurrentLevel.Size.Width]
        imul    ecx, [akode_data.CurrentLevel.Size.Height]

.unload_textures_loop:
        mov     edi, [esi]
        add     esi, 4

        test    edi, edi
        jz      .skip

        mov     edx, 6 * 2                                      ; 6 combined textures per cell

.unload_cell_textures_loop:
        mov     eax, [edi]
        add     edi, 4

        test    eax, eax
        jz      @f

        stdcall akode.unload_texture, eax

@@:
        sub     edx, 1
        jnz     .unload_cell_textures_loop

.skip:
        sub     ecx, 1
        jnz     .unload_textures_loop

        mov     [akode_data.CurrentLevelGridPtr], ecx

        mov     ecx, [akode_data.CurrentLevel.ObjectCount]
        test    ecx, ecx
        jz      @f

.unload_object_textures_loop:
        mov     eax, [esi]
        add     esi, sizeof.akode.Object

        test    eax, eax
        jz      .skip_object

        stdcall akode.unload_texture, eax

.skip_object:
        sub     ecx, 1
        jnz     .unload_object_textures_loop

@@:
        mov     [akode_data.CurrentLevelObjectsPtr], ecx

        mov     ecx, [akode_data.CurrentLevel.TextureCount]
        test    ecx, ecx
        jz      @f

.unload_additional_textures_loop:
        mov     eax, [esi]
        add     esi, 4

        test    eax, eax
        jz      .skip_additional

        stdcall akode.unload_texture, eax

.skip_additional:
        sub     ecx, 1
        jnz     .unload_additional_textures_loop

@@:
        mov     [akode_data.CurrentLevelAddTexturesPtr], ecx

        mov     eax, [akode_data.CurrentLevel.DestroyCallback]
        test    eax, eax
        jz      @f
        stdcall eax

@@:
        mov     edx, [akode_data.LevelLoadCallback]
        test    edx, edx
        jz      @f
        stdcall edx, AKODE_LEVEL_LOAD.UNLOADED, eax

@@:

.exit:
        ret
endp
; ============================================================================ ;

; ============================================================================ ;
; < eax = 0 - fail                                                             ;
; ============================================================================ ;
proc akode.load_texture uses ebx ecx edx, texture_desc_ptr
        mov     ebx, [texture_desc_ptr]

        mov     eax, [ebx + akode.TextureDesc.Type]
        or      eax, [ebx + akode.TextureDesc.ImageDataPtr]
        jnz     .exit

        mov     ecx, [akode_data.BlockSize.Width]
        mov     edx, ecx

        mov     al, [ebx + akode.TextureDesc.Usage]
        test    al, al
        jz      @f
        mov     edx, [akode_data.BlockSize.Height]

@@:
        stdcall akode.load_and_scale_image, [ebx + akode.TextureDesc.ImagePathPtr], ecx, edx, 1

        mov     [ebx + akode.TextureDesc.ImageDataPtr], eax

.exit:
        ret
endp
; ============================================================================ ;

; ============================================================================ ;
proc akode.unload_texture uses eax ebx ecx, texture_desc_ptr
        mov     ebx, [texture_desc_ptr]

        mov     eax, [ebx + akode.TextureDesc.Type]
        test    eax, eax
        jnz     .exit

        mov     ecx, [ebx + akode.TextureDesc.ImageDataPtr]
        test    ecx, ecx
        jz      .exit

        push    ebx
        mcall   68, 13
        pop     ebx

        xor     eax, eax
        mov     [ebx + akode.TextureDesc.ImageDataPtr], eax

.exit:
        ret
endp
; ============================================================================ ;

; ============================================================================ ;
; < eax = pointer to image data / 0 - fail                                     ;
; ============================================================================ ;
proc akode.load_and_scale_image uses ebx ecx edx esi edi, image_path_ptr, width, height, internal_format
        locals
                decode_options  ImageDecodeOptions      sizeof.ImageDecodeOptions, 0FF00FFh
        endl

        stdcall akode.load_file, [image_path_ptr]
        test    eax, eax
        jz      .exit

        mov     esi, eax                                        ; esi = file data

        lea     eax, [decode_options]
        push    esi
        invoke  img_decode, esi, ebx, eax
        pop     esi
        test    eax, eax
        jz      .exit_and_free_file_fail

        mov     edi, eax                                        ; edi = image handle

        xor     eax, eax
        push    esi edi
        invoke  img_convert, edi, eax, Image.bpp24, eax, eax
        pop     edi esi
        push    eax

        push    esi
        invoke  img_destroy, edi                                ; destroy handle from img_decode
        pop     ecx
        mcall   68, 13

        pop     edi                                             ; edi = image handle from img_convert
        test    edi, edi
        jz      .exit_fail

        mov     ebx, [width]
        mov     ecx, [height]

        cmp     [edi + Image.Width], ebx
        jne     .scale
        cmp     [edi + Image.Height], ecx
        jne     .scale
        jmp     @f

.scale:
        xor     eax, eax
        push    edi
        invoke  img_scale, edi, eax, eax, [edi + Image.Width], [edi + Image.Height], eax, LIBIMG_SCALE_STRETCH, LIBIMG_INTER_BILINEAR, ebx, ecx
        pop     edi
        push    eax

        invoke  img_destroy, edi                                ; destroy handle from img_convert

        pop     edi                                             ; edi = image handle from img_scale
        test    edi, edi
        jz      .exit_fail

@@:
        mov     eax, [internal_format]
        test    eax, eax
        jz      @f

        invoke  img_flip, edi, FLIP_HORIZONTAL
        invoke  img_rotate, edi, ROTATE_90_CCW

@@:
        stdcall akode._.convert_image_to_32bpp, edi
        push    eax

        invoke  img_destroy, edi

        pop     eax
        jmp     .exit

.exit_and_free_file_fail:
        mcall   68, 13, esi
.exit_fail:
        xor     eax, eax
.exit:
        ret
endp
; ============================================================================ ;

; ============================================================================ ;
; < eax = pointer to image data / 0 - fail                                     ;
; ============================================================================ ;
proc akode._.convert_image_to_32bpp uses ebx ecx edx esi edi, image
        mov     edi, [image]

        invoke  img_to_rgb, edi
        test    eax, eax
        jz      .exit

        mov     esi, eax

        mov     ecx, [edi + Image.Width]
        imul    ecx, [edi + Image.Height]
        shl     ecx, 2

        mcall   68, 12
        test    eax, eax
        jz      .exit_and_free_fail

        push    eax
        mov     edi, eax
        mov     edx, esi
        add     esi, 8
        shr     ecx, 2

@@:
        mov     ax, [esi]
        movzx   bx, byte [esi + 2]

        mov     [edi], ax
        mov     [edi + 2], bx

        add     esi, 3
        add     edi, 4

        sub     ecx, 1
        jnz     @b

        mcall   68, 13, edx
        pop     eax
        jmp     .exit

.exit_and_free_fail:
        mcall   68, 13, esi
        xor     eax, eax
.exit:
        ret
endp
; ============================================================================ ;

; ============================================================================ ;
; < eax = pointer to data / 0 - fail                                           ;
; < ebx = file size                                                            ;
; ============================================================================ ;
proc akode.load_file uses ecx edx esi edi, file_path_ptr
        invoke  file_size, [file_path_ptr]
        test    eax, eax
        jnz     .exit_fail

        mov     edx, ebx                                        ; edx = file size

        mcall   68, 12, edx
        test    eax, eax
        jz      .exit

        mov     edi, eax                                        ; edi = pointer to data

        invoke  file_open, [file_path_ptr], O_READ
        test    eax, eax
        jz      .exit_and_free_fail

        mov     esi, eax                                        ; esi = file handle
        mov     ebx, edx

        invoke  file_read, esi, edi, edx
        sub     edx, eax

        invoke  file_close, esi

        test    edx, edx
        jnz     .exit_and_free_fail

        mov     eax, edi
        jmp     .exit

.exit_and_free_fail:
        mcall   68, 13, edi
.exit_fail:
        xor     eax, eax
.exit:
        ret
endp
; ============================================================================ ;

; ============================================================================ ;
; < eax = 0 - fail                                                             ;
; ============================================================================ ;
proc akode._.calc_tables uses ebx ecx edx esi edi
        mov     eax, [akode_data.Angle360]
        mov     ecx, eax
        mov     edi, eax

        shr     eax, 1
        mov     [akode_data.Angle180], eax
        mov     ebx, eax

        shr     eax, 1
        mov     [akode_data.Angle90], eax

        add     ebx, eax
        mov     [akode_data.Angle270], ebx

        DEBUGF  DEBUG_FINE, 'akode_data.ProjectionPlane.Size.Width: %u\n', [akode_data.ProjectionPlane.Size.Width]
        DEBUGF  DEBUG_FINE, 'akode_data.Camera.FieldOfView: %u\n', [akode_data.Camera.FieldOfView]
        DEBUGF  DEBUG_FINE, 'akode_data.Angle[90, 180, 270, 360]: %u, %u, %u, %u\n', [akode_data.Angle90], [akode_data.Angle180], \
                                                                                     [akode_data.Angle270], [akode_data.Angle360]

        shl     ecx, 5                                          ; ecx = Angle360 * (8 bytes * 4 columns)
        mcall   68, 20, , [akode_data.TrigonometricTablePtr]    ; realloc ecx bytes

        test    eax, eax
        jz      .exit

        mov     [akode_data.TrigonometricTablePtr], eax

        fninit
        xor     ecx, ecx
        fldpi
        fldpi
        faddp
        fidiv   [akode_data.Angle360]                           ; 2 * pi / Angle360
@@:                                                             ; calculate sin, cos, tan, 1 / cos
        fld     st0
        mov     [eax], ecx

        fimul   dword [eax]                                     ; 2 * pi / Angle360 * ecx = ecx in radians

        fsincos
        fst     qword [eax + 8]                                 ; cos

        fld     st0
        fld1
        fdivrp
        fstp    qword [eax + 24]                                ; 1 / cos

        fxch
        fst     qword [eax]                                     ; sin
        fdivrp
        fstp    qword [eax + 16]                                ; tan

        add     ecx, 1
        add     eax, 32
        cmp     ecx, edi
        jne     @b

        ;mov     ecx, edi
        shl     ecx, 3                                          ; ecx = Angle360 * (4 bytes * 2 columns)
        mcall   68, 20, , [akode_data.BlockWidthTanTablePtr]    ; realloc ecx bytes

        test    eax, eax
        jz      .pop_fpu_exit

        mov     [akode_data.BlockWidthTanTablePtr], eax

        mov     edx, [akode_data.Angle180]
        mov     ebx, [akode_data.Angle90]
        mov     esi, [akode_data.Angle270]
        xor     ecx, ecx
@@:                                                             ; calculate BlockSize.Width * tan and BlockSize.Width / tan
        fld     st0
        mov     [eax], ecx

        fimul   dword [eax]                                     ; 2 * pi / Angle360 * ecx = ecx in radians

        fptan
        fld     st1
        fimul   [akode_data.BlockSize.Width]
        fistp   dword [eax]                                     ; BlockSize.Width * tan

        cmp     ecx, ebx
        jb      .reverse_sign
        cmp     ecx, esi
        ja      .reverse_sign
        jmp     .dont_reverse_sign
.reverse_sign:
        neg     dword [eax]                                     ; reverse sign for angles < 90 and > 270
.dont_reverse_sign:

        fdivrp
        fimul   [akode_data.BlockSize.Width]
        fistp   dword [eax + 4]                                 ; BlockSize.Width * 1 / tan

        cmp     ecx, edx
        jna     .dont_reverse_sign2
        neg     dword [eax + 4]                                 ; reverse sign for angles > 180
.dont_reverse_sign2:

        add     ecx, 1
        add     eax, 8
        cmp     ecx, edi
        jne     @b

.pop_fpu_exit:
        fstp    st0

.exit:
        ret
endp
; ============================================================================ ;

; ============================================================================ ;
proc akode.set_field_of_view fov
        ; not supported

        ret
endp
; ============================================================================ ;

; ============================================================================ ;
; < eax = 0 - fail                                                             ;
; ============================================================================ ;
proc akode.set_shading uses ebx ecx edx, color, distance
        m2m     [akode_data.ShadingColor], [color]

        mov     ecx, [distance]
        mov     [akode_data.ShadingDistance], ecx

        inc     ecx
        shl     ecx, 2

        mcall   68, 20, , [akode_data.ShadingTablePtr]          ; realloc ecx bytes
        test    eax, eax
        jz      .exit

        mov     [akode_data.ShadingTablePtr], eax

        fninit
        xor     ecx, ecx
        cmp     [distance], ecx
        jne     @f
        mov     dword [eax], 255
        jmp     .exit

@@:
        imul    ebx, ecx, 255
        mov     [eax], ebx
        fild    dword [eax]
        fidiv   [distance]
        fistp   dword [eax]

        mov     ebx, 255
        sub     ebx, [eax]
        mov     [eax], ebx

        add     ecx, 1
        add     eax, 4
        cmp     [distance], ecx
        jae     @b

.exit:
        ret
endp
; ============================================================================ ;

; ============================================================================ ;
; > movement_speed      = speed in px per second                               ;
; > turning_speed       = speed in degrees per second                          ;
; ============================================================================ ;
proc akode.set_movement_speed uses eax ebx edx, movement_speed, turning_speed
        mov     eax, [turning_speed]
        mul     [akode_data.Angle360]
        mov     ebx, 360
        div     ebx
        mov     [akode_data.TurningSpeed], eax

        m2m     [akode_data.MovementSpeed], [movement_speed]

        ret
endp
; ============================================================================ ;

; ============================================================================ ;
; > movement_direction  = AKODE_DIRECTION.NORTH - forward,                     ;
;                         AKODE_DIRECTION.SOUTH - backward                     ;
; ============================================================================ ;
proc akode.start_moving uses eax ebx, movement_direction
        m2m     [akode_data.MovementDirection], [movement_direction]
        mcall   26, 9
        mov     [akode_data.LastMoveTimestamp], eax
        ret
endp
; ============================================================================ ;

; ============================================================================ ;
proc akode.stop_moving
        mov     [akode_data.MovementDirection], -1
        ret
endp
; ============================================================================ ;

; ============================================================================ ;
; > turning_direction   = AKODE_DIRECTION.WEST - left,                         ;
;                         AKODE_DIRECTION.EAST - right                         ;
; ============================================================================ ;
proc akode.start_turning uses eax ebx, turning_direction
        m2m     [akode_data.TurningDirection], [turning_direction]
        mcall   26, 9
        mov     [akode_data.LastTurnTimestamp], eax
        ret
endp
; ============================================================================ ;

; ============================================================================ ;
proc akode.stop_turning
        mov     [akode_data.TurningDirection], -1
        ret
endp
; ============================================================================ ;

; ============================================================================ ;
proc akode.process uses eax ebx ecx edx esi edi
        stdcall akode._.process_moving

        ret
endp
; ============================================================================ ;

; ============================================================================ ;
proc akode._.process_moving
        locals
                timestamp               dd ?
                collision_distance      dd ?
                current_cell_x          dd ?
                current_cell_y          dd ?
        endl

        mov     esi, [akode_data.MovementDirection]
        mov     edi, [akode_data.TurningDirection]
        mov     eax, esi
        and     eax, edi
        js      .exit

        mcall   26, 9                                           ; get timestamp
        mov     [timestamp], eax

        test    edi, edi
        js      .not_turning

        mov     ebx, eax
        sub     ebx, [akode_data.LastTurnTimestamp]
        cmp     ebx, 4
        jb      .not_turning

        mov     [akode_data.LastTurnTimestamp], eax

        mov     eax, ebx
        mul     [akode_data.TurningSpeed]
        mov     ecx, 100
        div     ecx                                             ; eax = turn angle

        test    eax, eax
        jnz     @f
        inc     eax                                             ; clamp
@@:

        cmp     [akode_data.Angle90], eax
        jae     @f
        mov     eax, [akode_data.Angle90]                       ; clamp
@@:

        mov     edx, [akode_data.Camera.Direction]
        mov     ecx, [akode_data.Angle360]

        test    edi, edi
        jz      .turn_right

        ; turn left
        add     edx, eax
        cmp     ecx, edx
        ja      @f
        sub     edx, ecx
@@:
        jmp     @f

.turn_right:
        sub     edx, eax
        jns     @f
        add     edx, ecx
@@:
        mov     [akode_data.Camera.Direction], edx

        mov     eax, [timestamp]

.not_turning:
        test    esi, esi
        js      .exit

        mov     ebx, eax
        sub     ebx, [akode_data.LastMoveTimestamp]
        cmp     ebx, 4
        jb      .exit

        mov     [akode_data.LastMoveTimestamp], eax

        mov     eax, ebx
        mul     [akode_data.MovementSpeed]
        mov     ecx, 100
        div     ecx                                             ; eax = move distance

        cmp     eax, 3
        jae     @f
        mov     eax, 3                                          ; clamp
@@:

        mov     edx, [akode_data.BlockSize.Width]
        shr     edx, 2
        mov     ebx, edx
        sub     edx, 2
        cmp     edx, eax
        jae     @f
        mov     eax, edx                                        ; clamp
@@:

        mov     [collision_distance], ebx

        fninit

        mov     ebx, [akode_data.Camera.Direction]
        shl     ebx, 5
        add     ebx, [akode_data.TrigonometricTablePtr]

        push    eax
        fild    dword [esp]
        fld     st0
        fmul    qword [ebx]
        fistp   dword [esp]
        mov     ecx, [esp]                                      ; ecx = distance * sin (Camera.Direction) = y displacement

        fmul    qword [ebx + 8]
        fistp   dword [esp]
        pop     edx                                             ; edx = distance * cos (Camera.Direction) = x displacement

        cmp     esi, AKODE_DIRECTION.NORTH
        mov     esi, [akode_data.Camera.Position.X]
        mov     edi, [akode_data.Camera.Position.Y]
        mov     eax, esi
        mov     ebx, edi
        jne     .move_backward

        ; move forward
        add     esi, edx
        sub     edi, ecx
        jmp     @f

.move_backward:
        sub     esi, edx
        add     edi, ecx
@@:

        ; collision detection
        locals
                top_border              dd 0
                left_border             dd 0
                right_border            dd 0
                bottom_border           dd 0
                new_x                   dd ?
                new_y                   dd ?
        endl

        mov     ecx, [akode_data.BlockWidthPowerOf2]
        mov     edx, [akode_data.CurrentLevelGridPtr]
        shr     eax, cl
        shr     ebx, cl
        mov     [current_cell_x], eax
        mov     [current_cell_y], ebx

        ; top border
        mov     eax, esi
        mov     ebx, edi
        sub     ebx, [collision_distance]

        shr     eax, cl
        shr     ebx, cl

        imul    ebx, [akode_data.CurrentLevel.Size.Width]
        add     ebx, eax
        mov     eax, [edx + ebx * 4]                            ; eax = pointer to akode.GridCell
        test    eax, eax
        jz      @f
        mov     ebx, [eax + akode.GridCell.Passable]
        test    ebx, ebx
        jnz     @f
        shr     edi, cl
        shl     edi, cl
        add     edi, [collision_distance]
        mov     [top_border], 1
@@:

        ; left border
        mov     eax, esi
        mov     ebx, edi
        sub     eax, [collision_distance]

        shr     eax, cl
        shr     ebx, cl

        imul    ebx, [akode_data.CurrentLevel.Size.Width]
        add     ebx, eax
        mov     eax, [edx + ebx * 4]                            ; eax = pointer to akode.GridCell
        test    eax, eax
        jz      @f
        mov     ebx, [eax + akode.GridCell.Passable]
        test    ebx, ebx
        jnz     @f
        shr     esi, cl
        shl     esi, cl
        add     esi, [collision_distance]
        mov     [left_border], 1
@@:

        ; right border
        mov     eax, esi
        mov     ebx, edi
        add     eax, [collision_distance]

        shr     eax, cl
        shr     ebx, cl

        imul    ebx, [akode_data.CurrentLevel.Size.Width]
        add     ebx, eax
        mov     eax, [edx + ebx * 4]                            ; eax = pointer to akode.GridCell
        test    eax, eax
        jz      @f
        mov     ebx, [eax + akode.GridCell.Passable]
        test    ebx, ebx
        jnz     @f
        shr     esi, cl
        shl     esi, cl
        add     esi, [akode_data.BlockSize.Width]
        sub     esi, [collision_distance]
        sub     esi, 1
        mov     [right_border], 1
@@:

        ; bottom border
        mov     eax, esi
        mov     ebx, edi
        add     ebx, [collision_distance]

        shr     eax, cl
        shr     ebx, cl

        imul    ebx, [akode_data.CurrentLevel.Size.Width]
        add     ebx, eax
        mov     eax, [edx + ebx * 4]                            ; eax = pointer to akode.GridCell
        test    eax, eax
        jz      @f
        mov     ebx, [eax + akode.GridCell.Passable]
        test    ebx, ebx
        jnz     @f
        shr     edi, cl
        shl     edi, cl
        add     edi, [akode_data.BlockSize.Width]
        sub     edi, [collision_distance]
        sub     edi, 1
        mov     [bottom_border], 1
@@:

        ; top left border
        mov     eax, [top_border]
        or      eax, [left_border]
        jnz     @f

        mov     eax, esi
        mov     ebx, edi
        sub     eax, [collision_distance]
        sub     ebx, [collision_distance]

        shr     eax, cl
        shr     ebx, cl

        imul    ebx, [akode_data.CurrentLevel.Size.Width]
        add     ebx, eax
        mov     eax, [edx + ebx * 4]                            ; eax = pointer to akode.GridCell
        test    eax, eax
        jz      @f
        mov     ebx, [eax + akode.GridCell.Passable]
        test    ebx, ebx
        jnz     @f
        mov     eax, esi
        mov     ebx, edi

        shr     eax, cl
        shl     eax, cl
        add     eax, [collision_distance]
        mov     [new_x], eax
        shr     ebx, cl
        shl     ebx, cl
        add     ebx, [collision_distance]
        mov     [new_y], ebx

        sub     eax, esi
        sub     ebx, edi
        cmp     eax, ebx
        jb      .l1
        mov     edi, [new_y]
        jmp     @f
.l1:    mov     esi, [new_x]
@@:

        ; top right border
        mov     eax, [top_border]
        or      eax, [right_border]
        jnz     @f

        mov     eax, esi
        mov     ebx, edi
        add     eax, [collision_distance]
        sub     ebx, [collision_distance]

        shr     eax, cl
        shr     ebx, cl

        imul    ebx, [akode_data.CurrentLevel.Size.Width]
        add     ebx, eax
        mov     eax, [edx + ebx * 4]                            ; eax = pointer to akode.GridCell
        test    eax, eax
        jz      @f
        mov     ebx, [eax + akode.GridCell.Passable]
        test    ebx, ebx
        jnz     @f
        mov     eax, esi
        mov     ebx, edi

        shr     eax, cl
        shl     eax, cl
        add     eax, [akode_data.BlockSize.Width]
        sub     eax, [collision_distance]
        sub     eax, 1
        mov     [new_x], eax
        shr     ebx, cl
        shl     ebx, cl
        add     ebx, [collision_distance]
        mov     [new_y], ebx

        neg     eax
        add     eax, esi
        sub     ebx, edi
        cmp     eax, ebx
        jb      .l2
        mov     edi, [new_y]
        jmp     @f
.l2:    mov     esi, [new_x]
@@:

        ; bottom left border
        mov     eax, [bottom_border]
        or      eax, [left_border]
        jnz     @f

        mov     eax, esi
        mov     ebx, edi
        sub     eax, [collision_distance]
        add     ebx, [collision_distance]

        shr     eax, cl
        shr     ebx, cl

        imul    ebx, [akode_data.CurrentLevel.Size.Width]
        add     ebx, eax
        mov     eax, [edx + ebx * 4]                            ; eax = pointer to akode.GridCell
        test    eax, eax
        jz      @f
        mov     ebx, [eax + akode.GridCell.Passable]
        test    ebx, ebx
        jnz     @f
        mov     eax, esi
        mov     ebx, edi

        shr     eax, cl
        shl     eax, cl
        add     eax, [collision_distance]
        mov     [new_x], eax
        shr     ebx, cl
        shl     ebx, cl
        add     ebx, [akode_data.BlockSize.Width]
        sub     ebx, [collision_distance]
        sub     ebx, 1
        mov     [new_y], ebx

        sub     eax, esi
        neg     ebx
        add     ebx, edi
        cmp     eax, ebx
        jb      .l3
        mov     edi, [new_y]
        jmp     @f
.l3:    mov     esi, [new_x]
@@:

        ; bottom right border
        mov     eax, [bottom_border]
        or      eax, [right_border]
        jnz     @f

        mov     eax, esi
        mov     ebx, edi
        add     eax, [collision_distance]
        add     ebx, [collision_distance]

        shr     eax, cl
        shr     ebx, cl

        imul    ebx, [akode_data.CurrentLevel.Size.Width]
        add     ebx, eax
        mov     eax, [edx + ebx * 4]                            ; eax = pointer to akode.GridCell
        test    eax, eax
        jz      @f
        mov     ebx, [eax + akode.GridCell.Passable]
        test    ebx, ebx
        jnz     @f
        mov     eax, esi
        mov     ebx, edi

        shr     eax, cl
        shl     eax, cl
        add     eax, [akode_data.BlockSize.Width]
        sub     eax, [collision_distance]
        sub     eax, 1
        mov     [new_x], eax
        shr     ebx, cl
        shl     ebx, cl
        add     ebx, [akode_data.BlockSize.Width]
        sub     ebx, [collision_distance]
        sub     ebx, 1
        mov     [new_y], ebx

        neg     eax
        add     eax, esi
        neg     ebx
        add     ebx, edi
        cmp     eax, ebx
        jb      .l4
        mov     edi, [new_y]
        jmp     @f
.l4:    mov     esi, [new_x]
@@:

        mov     eax, esi
        mov     ebx, edi
        shr     eax, cl
        shr     ebx, cl

        cmp     [current_cell_x], eax
        jne     .new_cell
        cmp     [current_cell_y], ebx
        jne     .new_cell

        mov     [akode_data.Camera.Position.X], esi
        mov     [akode_data.Camera.Position.Y], edi
        jmp     .exit

.new_cell:
        stdcall akode.action, AKODE_ACTION.CELL_LEAVE

        mov     [akode_data.Camera.Position.X], esi
        mov     [akode_data.Camera.Position.Y], edi

        stdcall akode.action, AKODE_ACTION.CELL_ENTER

.exit:
        ret
endp
; ============================================================================ ;

; ============================================================================ ;
; > action      = AKODE_ACTION                                                 ;
; ---------------------------------------------------------------------------- ;
; < eax         = action result                                                ;
; ============================================================================ ;
proc akode.action uses ebx ecx edx esi edi, action
        mov     edi, [action]
        mov     ecx, [akode_data.BlockWidthPowerOf2]
        mov     ebx, [akode_data.Camera.Position.X]
        mov     edx, [akode_data.Camera.Position.Y]
        shr     ebx, cl
        shr     edx, cl

        mov     eax, edx
        imul    eax, [akode_data.CurrentLevel.Size.Width]
        add     eax, ebx
        mov     esi, [akode_data.CurrentLevelGridPtr]
        mov     esi, [esi + eax * 4]
        test    esi, esi
        jz      .default_action

        mov     eax, [esi + akode.GridCell.ActionCallback]
        test    eax, eax
        jz      .default_action
        stdcall eax, edi, ebx, edx
        cmp     eax, -1
        jne     @f

.default_action:
        mov     eax, [akode_data.CurrentLevel.ActionCallback]
        test    eax, eax
        jz      @f
        stdcall eax, edi, ebx, edx

@@:
        mov     esi, [akode_data.ActionCallback]
        test    esi, esi
        jz      @f
        stdcall esi, edi, ebx, edx, eax

@@:
        ret
endp
; ============================================================================ ;

; ============================================================================ ;
proc akode.fill_with_color uses eax ecx edi, color
        cld
        mov     eax, [color]
        mov     edi, [akode_data.ImageBufferPtr]
        mov     ecx, [akode_data.ImageBufferSize]
        shr     ecx, 2
        rep     stosd

        ret
endp
; ============================================================================ ;

; ============================================================================ ;
; > eax = color 1                                                              ;
; > ebx = color 2                                                              ;
; > edx = alpha, 255 -> color 1, 0 -> color 2                                  ;
; ---------------------------------------------------------------------------- ;
; < eax = result color                                                         ;
; ============================================================================ ;
macro akode._.blend_colors
{
        push    ebx ecx edx esi edi

        mov     ecx, 256
        sub     ecx, edx
        add     edx, 1

        mov     edi, eax
        and     edi, 0FF00FFh
        imul    edi, edx

        mov     esi, ebx
        and     esi, 0FF00FFh
        imul    esi, ecx

        add     edi, esi
        and     edi, 0FF00FF00h

        and     eax, 00FF00h
        imul    eax, edx

        and     ebx, 00FF00h
        imul    ebx, ecx

        add     eax, ebx
        and     eax, 0FF0000h
        or      eax, edi
        shr     eax, 8

        pop     edi esi edx ecx ebx
}
; ============================================================================ ;

; ============================================================================ ;
; > eax = color 1                                                              ;
; > ebx = color 2                                                              ;
; > edx = alpha, 255 -> color 1, 0 -> color 2                                  ;
; ---------------------------------------------------------------------------- ;
; < eax = result color                                                         ;
; ============================================================================ ;
macro akode._.blend_colors_mmx
{
        movd    mm0, eax
        movd    mm1, ebx
        pxor    mm2, mm2
        punpcklbw mm0, mm2
        punpcklbw mm1, mm2

        mov     eax, 256
        sub     eax, edx
        movd    mm2, eax
        pshufw  mm3, mm2, 0

        mov     eax, edx
        add     eax, 1
        movd    mm4, eax
        pshufw  mm2, mm4, 0

        pmullw  mm1, mm3
        pmullw  mm0, mm2
        paddw   mm0, mm1
        psrlw   mm0, 8
        packuswb mm0, mm0
        movd    eax, mm0

        ;emms
}
; ============================================================================ ;

; ============================================================================ ;
; < eax = pointer to image / 0 - fail                                          ;
; ============================================================================ ;
proc akode.render uses ebx ecx edx esi edi
        mov     eax, [akode_data.CurrentLevelGridPtr]
        test    eax, eax
        jz      .exit

        mov     eax, [akode_data.CurrentLevel.BackgroundColor]
        cmp     eax, 0FF00FFh
        je      @f
        stdcall akode.fill_with_color, eax

@@:
        mov     eax, [akode_data.ProjectionPlane.Size.Width]
        mov     ebx, eax
        sub     ebx, 1
        shr     eax, 1
        add     eax, [akode_data.Camera.Direction]
        mov     edx, [akode_data.Angle360]
        cmp     edx, eax
        ja      @f
        sub     eax, edx

@@:
        push    eax                                             ; start_angle for akode._.draw_objects
        stdcall akode._.render_slices, 0, ebx, eax
        stdcall akode._.draw_objects

        mov     eax, [akode_data.ImageBufferPtr]

.exit:
        ret
endp
; ============================================================================ ;

; ============================================================================ ;
macro akode._.draw_textured_wall_slice
{
        label   .int_part       dword at x
        label   .fract_part     dword at y
        label   .e              dword at x_delta
        label   .target_height  dword at wall_slice_height

        mov     eax, [esi + akode.TextureDesc.ImageDataPtr]
        mov     [texture_desc_ptr1], eax

        mov     esi, [texture_desc_ptr2]
        test    esi, esi
        jz      .draw_one_texture

        mov     eax, [esi + akode.TextureDesc.Type]
        test    eax, eax
        jnz     .draw_one_texture

        movzx   eax, [esi + akode.TextureDesc.HasMagicPink]
        test    eax, eax
        jnz     @f
        mov     eax, [esi + akode.TextureDesc.ImageDataPtr]
        mov     [texture_desc_ptr1], eax
        jmp     .draw_one_texture

@@:
        ; draw 2 textures

        push    edx

        mov     esi, [esi + akode.TextureDesc.ImageDataPtr]
        mov     edx, [texture_x_coordinate]
        mov     eax, [akode_data.BlockSize.Width]
        imul    edx, eax
        shl     edx, 2
        add     esi, edx
        add     [texture_desc_ptr1], edx

        xor     edx, edx
        div     [.target_height]
        shl     eax, 2
        mov     [.int_part], eax
        mov     [.fract_part], edx

        pop     edx

        xor     eax, eax
        mov     [.e], eax

        mov     eax, [wall_slice_y]
        test    eax, eax
        js      .clipping2

.draw_double_textured_wall_slice_loop:
        mov     eax, [esi]
        cmp     eax, 0FF00FFh
        jne     @f
        mov     eax, [texture_desc_ptr1]
        mov     eax, [eax]

@@:

if ~(defined DISABLE_SHADING & DISABLE_SHADING)
        akode._.blend_colors_mmx
end if

        mov     [edi], eax

        add     edi, 4
        mov     eax, [.int_part]
        add     esi, eax
        add     [texture_desc_ptr1], eax
        mov     eax, [.e]
        add     eax, [.fract_part]
        mov     [.e], eax

        sub     eax, [.target_height]
        jl      @f
        mov     [.e], eax
        add     esi, 4
        add     [texture_desc_ptr1], 4

@@:
        sub     ecx, 1
        jnz     .draw_double_textured_wall_slice_loop

if ~(defined DISABLE_SHADING & DISABLE_SHADING)
        emms
end if

        jmp     .draw_floor_slice

.clipping2:
        sub     ecx, [wall_slice_y]

.draw_double_textured_wall_slice_loop2:
        mov     eax, [wall_slice_y]
        test    eax, eax
        js      .skip_pixel

        mov     eax, [esi]
        cmp     eax, 0FF00FFh
        jne     @f
        mov     eax, [texture_desc_ptr1]
        mov     eax, [eax]

@@:

if ~(defined DISABLE_SHADING & DISABLE_SHADING)
        akode._.blend_colors_mmx
end if

        mov     [edi], eax
        add     edi, 4
.skip_pixel:
        add     [wall_slice_y], 1
        mov     eax, [.int_part]
        add     esi, eax
        add     [texture_desc_ptr1], eax
        mov     eax, [.e]
        add     eax, [.fract_part]
        mov     [.e], eax

        sub     eax, [.target_height]
        jl      @f
        mov     [.e], eax
        add     esi, 4
        add     [texture_desc_ptr1], 4

@@:
        sub     ecx, 1
        jnz     .draw_double_textured_wall_slice_loop2

if ~(defined DISABLE_SHADING & DISABLE_SHADING)
        emms
end if

        jmp     .draw_floor_slice

.draw_one_texture:
        push    edx

        mov     esi, [texture_desc_ptr1]
        mov     edx, [texture_x_coordinate]
        mov     eax, [akode_data.BlockSize.Width]
        imul    edx, eax
        lea     esi, [esi + edx * 4]

        xor     edx, edx
        div     [.target_height]
        shl     eax, 2
        mov     [.int_part], eax
        mov     [.fract_part], edx

        pop     edx

        xor     eax, eax
        mov     [.e], eax

        mov     eax, [wall_slice_y]
        test    eax, eax
        js      .clipping

.draw_textured_wall_slice_loop:
        mov     eax, [esi]

if ~(defined DISABLE_SHADING & DISABLE_SHADING)
        akode._.blend_colors_mmx
end if

        mov     [edi], eax

        add     edi, 4
        add     esi, [.int_part]
        mov     eax, [.e]
        add     eax, [.fract_part]
        mov     [.e], eax

        sub     eax, [.target_height]
        jl      @f
        mov     [.e], eax
        add     esi, 4

@@:
        sub     ecx, 1
        jnz     .draw_textured_wall_slice_loop

if ~(defined DISABLE_SHADING & DISABLE_SHADING)
        emms
end if

        jmp     .draw_floor_slice

.clipping:
        sub     ecx, [wall_slice_y]

.draw_textured_wall_slice_loop2:
        mov     eax, [wall_slice_y]
        test    eax, eax
        js      @f
        mov     eax, [esi]

if ~(defined DISABLE_SHADING & DISABLE_SHADING)
        akode._.blend_colors_mmx
end if

        mov     [edi], eax
        add     edi, 4
@@:
        add     [wall_slice_y], 1
        add     esi, [.int_part]
        mov     eax, [.e]
        add     eax, [.fract_part]
        mov     [.e], eax

        sub     eax, [.target_height]
        jl      @f
        mov     [.e], eax
        add     esi, 4

@@:
        sub     ecx, 1
        jnz     .draw_textured_wall_slice_loop2

if ~(defined DISABLE_SHADING & DISABLE_SHADING)
        emms
end if
}

;void ScaleLine(PIXEL *Target, PIXEL *Source, int SrcWidth, int TgtWidth)
;{
;  int NumPixels = TgtWidth;
;  int IntPart = SrcWidth / TgtWidth;
;  int FractPart = SrcWidth % TgtWidth;
;  int E = 0;
;
;  while (NumPixels-- > 0) {
;    *Target++ = *Source;
;    Source += IntPart;
;    E += FractPart;
;    if (E >= TgtWidth) {
;      E -= TgtWidth;
;      Source++;
;    } /* if */
;  } /* while */
;}

proc akode._.render_slices first_slice, last_slice, start_angle
        locals
                distance_to_floor               dd ?
                distance_to_wall                dd ?
                horizontal_distance_to_wall     dq ?
                vertical_distance_to_wall       dq ?

                texture_x_coordinate            dd ?
                texture_y_coordinate            dd ?

                wall_slice_height               dd ?
                wall_slice_y                    dd ?
                wall_slice_top                  dd ?
                wall_slice_bottom               dd ?
                wall_slice_top_ptr              dd ?

                horizontal_texture_desc_ptr1    dd ?
                horizontal_texture_desc_ptr2    dd ?

                vertical_texture_desc_ptr1      dd ?
                vertical_texture_desc_ptr2      dd ?

                texture_desc_ptr1               dd ?
                texture_desc_ptr2               dd ?

                x                               dd ?
                y                               dd ?
                x_delta                         dd ?
                y_delta                         dd ?

                wall_side                       dd ?
        endl

        fninit

.slice_loop:
        ;DEBUGF  DEBUG_FINE, 'first_slice: %u, last_slice: %u, start_angle: %u\n', [first_slice], [last_slice], [start_angle]

; ------------------------------- render walls ------------------------------- ;

        ; horizontal intersections
        mov     ecx, [akode_data.BlockWidthPowerOf2]
        mov     eax, [akode_data.Camera.Position.Y]
        mov     edi, eax
        shr     eax, cl
        shl     eax, cl
        mov     edx, [akode_data.BlockSize.Width]

        mov     ebx, [start_angle]
        test    ebx, ebx
        jz      .ray_is_horizontal
        cmp     [akode_data.Angle180], ebx
        je      .ray_is_horizontal
        jb      .ray_is_facing_down

        ; ray is facing up

        sub     eax, 1
        neg     edx
        mov     [wall_side], AKODE_DIRECTION.SOUTH
        jmp     @f

        ; ray is facing down

.ray_is_facing_down:
        add     eax, edx                                        ; edx = BlockSize.Width
        mov     [wall_side], AKODE_DIRECTION.NORTH

@@:
        mov     [y], eax
        mov     [y_delta], edx

        sub     edi, eax                                        ; edi = Camera.Position.Y - y
        push    edi
        mov     esi, ebx
        shl     esi, 5                                          ; esi = angle * 32
        add     esi, [akode_data.TrigonometricTablePtr]

        fild    dword [esp]
        fdiv    qword [esi + 16]
        fistp   dword [esp]
        pop     edi                                             ; edi = (Camera.Position.Y - y) / tan (angle)

        add     edi, [akode_data.Camera.Position.X]
        mov     [x], edi

        mov     esi, [akode_data.BlockWidthTanTablePtr]
        mov     eax, [esi + ebx * 8 + 4]                        ; eax = BlockSize.Width / tan (angle)
        mov     [x_delta], eax
        ;DEBUGF  DEBUG_FINE, 'x_delta: %d\n', eax

.horizontal_intersections_loop:
        mov     eax, [x]
        mov     ebx, [y]

        sar     eax, cl
        js      .ray_is_horizontal
        cmp     [akode_data.CurrentLevel.Size.Width], eax
        jna     .ray_is_horizontal

        sar     ebx, cl
        js      .ray_is_horizontal
        cmp     [akode_data.CurrentLevel.Size.Height], ebx
        jna     .ray_is_horizontal

        imul    ebx, [akode_data.CurrentLevel.Size.Width]
        add     ebx, eax
        mov     esi, [akode_data.CurrentLevelGridPtr]
        mov     esi, [esi + ebx * 4]                            ; esi = pointer to akode.GridCell

        test    esi, esi
        jz      .next_horizontal_intersection

        mov     eax, [wall_side]
        lea     edi, [esi + eax * 8]

        mov     ebx, [edi]                                      ; first texture desc ptr
        test    ebx, ebx
        jz      .next_horizontal_intersection

        mov     edx, [edi + 4]                                  ; second texture desc ptr

        mov     [horizontal_texture_desc_ptr1], ebx
        mov     [horizontal_texture_desc_ptr2], edx

        mov     edx, -1
        shl     edx, cl
        not     edx

        mov     esi, [x]
        and     esi, edx
        cmp     eax, AKODE_DIRECTION.NORTH
        jne     @f
        mov     eax, [akode_data.BlockSize.Width]
        add     esi, 1
        xchg    eax, esi
        sub     esi, eax
@@:
        mov     [texture_x_coordinate], esi

        mov     ebx, [start_angle]
        mov     esi, [akode_data.TrigonometricTablePtr]

        mov     eax, [akode_data.Camera.Position.Y]
        sub     eax, [y]

        mov     edx, ebx
        shl     edx, 5

        push    eax
        fild    dword [esp]
        add     esp, 4
        fdiv    qword [esi + edx]                               ; st0 = (Camera.Position.Y - y) / sin (angle)

        ; correct fisheye
        mov     eax, [akode_data.Camera.Direction]
        sub     eax, ebx
        jns     @f
        neg     eax

@@:
        shl     eax, 5
        fmul    qword [esi + eax + 8]                           ; st0 * cos (Camera.Direction - angle)
        fstp    [horizontal_distance_to_wall]

        jmp     .vertical_intersections

.next_horizontal_intersection:
        mov     eax, [x_delta]
        mov     ebx, [y_delta]
        add     [x], eax
        add     [y], ebx
        jmp     .horizontal_intersections_loop

.ray_is_horizontal:
        ; horizontal_distance_to_wall = max double
        mov     dword [horizontal_distance_to_wall], 0FFFFFFFFh
        mov     dword [horizontal_distance_to_wall + 4], 7FEFFFFFh

        ; vertical intersections
.vertical_intersections:
        mov     ecx, [akode_data.BlockWidthPowerOf2]
        mov     eax, [akode_data.Camera.Position.X]
        mov     edi, eax
        shr     eax, cl
        shl     eax, cl
        mov     edx, [akode_data.BlockSize.Width]

        mov     ebx, [start_angle]
        cmp     [akode_data.Angle90], ebx
        je      .ray_is_vertical
        ja      .ray_is_facing_right
        cmp     [akode_data.Angle270], ebx
        je      .ray_is_vertical
        jb      .ray_is_facing_right

        ; ray is facing left

        sub     eax, 1
        neg     edx
        mov     [wall_side], AKODE_DIRECTION.EAST
        jmp     @f

        ; ray is facing right

.ray_is_facing_right:
        add     eax, edx                                        ; edx = BlockSize.Width
        mov     [wall_side], AKODE_DIRECTION.WEST

@@:
        mov     [x], eax
        mov     [x_delta], edx

        sub     edi, eax                                        ; edi = Camera.Position.X - x
        push    edi
        mov     esi, ebx
        shl     esi, 5                                          ; esi = angle * 32
        add     esi, [akode_data.TrigonometricTablePtr]

        fild    dword [esp]
        fmul    qword [esi + 16]
        fistp   dword [esp]
        pop     edi                                             ; edi = (Camera.Position.X - x) * tan (angle)

        add     edi, [akode_data.Camera.Position.Y]
        mov     [y], edi

        mov     esi, [akode_data.BlockWidthTanTablePtr]
        mov     eax, [esi + ebx * 8]                            ; eax = BlockSize.Width * tan (angle)
        mov     [y_delta], eax
        ;DEBUGF  DEBUG_FINE, 'y_delta: %d\n', eax

.vertical_intersections_loop:
        mov     eax, [x]
        mov     ebx, [y]

        sar     eax, cl
        js      .ray_is_vertical
        cmp     [akode_data.CurrentLevel.Size.Width], eax
        jna     .ray_is_vertical

        sar     ebx, cl
        js      .ray_is_vertical
        cmp     [akode_data.CurrentLevel.Size.Height], ebx
        jna     .ray_is_vertical

        imul    ebx, [akode_data.CurrentLevel.Size.Width]
        add     ebx, eax
        mov     esi, [akode_data.CurrentLevelGridPtr]
        mov     esi, [esi + ebx * 4]                            ; esi = pointer to akode.GridCell

        test    esi, esi
        jz      .next_vertical_intersection

        mov     eax, [wall_side]
        lea     edi, [esi + eax * 8]

        mov     ebx, [edi]                                      ; first texture desc ptr
        test    ebx, ebx
        jz      .next_vertical_intersection

        mov     edx, [edi + 4]                                  ; second texture desc ptr

        mov     [vertical_texture_desc_ptr1], ebx
        mov     [vertical_texture_desc_ptr2], edx

        mov     edx, -1
        shl     edx, cl
        not     edx

        mov     esi, [y]
        and     esi, edx
        test    eax, eax                                        ; cmp eax, AKODE_DIRECTION.EAST
        jnz     @f
        mov     eax, [akode_data.BlockSize.Width]
        add     esi, 1
        xchg    eax, esi
        sub     esi, eax
@@:
        mov     [texture_y_coordinate], esi

        mov     ebx, [start_angle]
        mov     esi, [akode_data.TrigonometricTablePtr]

        mov     eax, [x]
        sub     eax, [akode_data.Camera.Position.X]

        mov     edx, ebx
        shl     edx, 5

        push    eax
        fild    dword [esp]
        add     esp, 4
        fmul    qword [esi + edx + 24]                          ; st0 = (x - Camera.Position.X) / cos (angle)

        ; correct fisheye
        mov     eax, [akode_data.Camera.Direction]
        sub     eax, ebx
        jns     @f
        neg     eax

@@:
        shl     eax, 5
        fmul    qword [esi + eax + 8]                           ; st0 * cos (Camera.Direction - angle)
        fstp    [vertical_distance_to_wall]

        jmp     .draw_wall_slice

.next_vertical_intersection:
        mov     eax, [x_delta]
        mov     ebx, [y_delta]
        add     [x], eax
        add     [y], ebx
        jmp     .vertical_intersections_loop

.ray_is_vertical:
        ; vertical_distance_to_wall = max double
        mov     dword [vertical_distance_to_wall], 0FFFFFFFFh
        mov     dword [vertical_distance_to_wall + 4], 7FEFFFFFh


; ----------------------------- draw wall slice ------------------------------ ;

.draw_wall_slice:
        fld     [horizontal_distance_to_wall]
        fld     [vertical_distance_to_wall]
        fcomi   st1
        fcmovnb st1
        fist    [distance_to_wall]
        fidivr  [akode_data.WallHeightDividend]
        fistp   [wall_slice_height]
        fstp    st0

        mov     ebx, [akode_data.DepthBufferPtr]
        mov     ecx, [first_slice]
        lea     ebx, [ebx + ecx * 4]
        mov     eax, [distance_to_wall]
        mov     [ebx], eax

        jnb     .horizontal_textures
        mov     eax, [vertical_texture_desc_ptr1]
        mov     ebx, [vertical_texture_desc_ptr2]

        mov     ecx, [texture_y_coordinate]
        mov     [texture_x_coordinate], ecx
        jmp     @f

.horizontal_textures:
        mov     eax, [horizontal_texture_desc_ptr1]
        mov     ebx, [horizontal_texture_desc_ptr2]

@@:
        mov     ecx, [wall_slice_height]
        test    ecx, ecx
        jz      .skip_draw_wall_slice

        mov     [texture_desc_ptr1], eax
        mov     [texture_desc_ptr2], ebx

        mov     edx, [akode_data.ProjectionPlane.MidY]
        mov     eax, ecx
        shr     eax, 1
        sub     edx, eax
        mov     [wall_slice_y], edx
        jns     @f
        xor     edx, edx

@@:
        mov     ebx, [akode_data.ProjectionPlane.Size.Height]
        mov     eax, ecx
        add     eax, edx
        cmp     ebx, eax
        jae     @f
        mov     ecx, ebx
        sub     ecx, edx

@@:
        mov     eax, edx
        sub     eax, 1
        mov     [wall_slice_top], eax
        mov     eax, edx
        add     eax, ecx
        mov     [wall_slice_bottom], eax

        mov     eax, [akode_data.ProjectionPlane.Size.Height]
        imul    eax, [first_slice]
        add     eax, edx
        mov     edi, [akode_data.ImageBufferPtr]
        lea     edi, [edi + eax * 4]
        mov     eax, edi
        sub     eax, 4
        mov     [wall_slice_top_ptr], eax

if ~(defined DISABLE_SHADING & DISABLE_SHADING)
        mov     esi, [akode_data.ShadingTablePtr]
        mov     ebx, [akode_data.ShadingDistance]
        mov     eax, [distance_to_wall]
        cmp     ebx, eax
        jae     @f
        mov     eax, ebx
@@:
        mov     edx, [esi + eax * 4]
        mov     ebx, [akode_data.ShadingColor]
end if

        mov     esi, [texture_desc_ptr1]
        mov     eax, [esi + akode.TextureDesc.Type]
        test    eax, eax
        jz      .texture_type_image

        ; texture type is color

        add     esi, akode.TextureDesc.Color

if ~(defined DISABLE_SHADING & DISABLE_SHADING)
.draw_color_wall_slice_loop:
        mov     eax, [esi]

        akode._.blend_colors_mmx

        mov     [edi], eax
        add     edi, 4

        sub     ecx, 1
        jnz     .draw_color_wall_slice_loop

        emms
else
        mov     eax, [esi]

.draw_color_wall_slice_loop:
        mov     [edi], eax
        add     edi, 4

        sub     ecx, 1
        jnz     .draw_color_wall_slice_loop
end if

        jmp     .draw_floor_slice

.texture_type_image:
        akode._.draw_textured_wall_slice
        jmp     .draw_floor_slice

.skip_draw_wall_slice:
        mov     eax, [akode_data.ProjectionPlane.MidY]
        mov     [wall_slice_top], eax
        add     eax, 1
        mov     [wall_slice_bottom], eax

        mov     eax, [akode_data.ProjectionPlane.Size.Height]
        imul    eax, [first_slice]
        add     eax, [wall_slice_bottom]
        mov     edi, [akode_data.ImageBufferPtr]
        lea     edi, [edi + eax * 4]
        mov     eax, edi
        sub     eax, 4
        mov     [wall_slice_top_ptr], eax

; ----------------------------- draw floor slice ----------------------------- ;

.draw_floor_slice:
        mov     esi, [akode_data.TrigonometricTablePtr]
        mov     ebx, [start_angle]
        mov     eax, ebx
        shl     eax, 5
        fld     qword [esi + eax]                               ; sin (angle)
        fld     qword [esi + eax + 8]                           ; cos (angle)

        mov     eax, [akode_data.Camera.Direction]
        sub     eax, ebx
        jns     @f
        neg     eax

@@:
        shl     eax, 5
        fld     qword [esi + eax + 24]                          ; 1 / cos (Camera.Direction - angle)

        mov     ecx, [wall_slice_bottom]
        cmp     [akode_data.ProjectionPlane.Size.Height], ecx
        jbe     .skip_draw_floor_slice

        add     ecx, 1
        fild    [akode_data.FloorDistanceDividend]
        fmul    st0, st1                                        ; st0 = FloorDistanceDividend / cos (Camera.Direction - angle)

.draw_floor_slice_loop:
        mov     eax, ecx
        sub     eax, [akode_data.ProjectionPlane.MidY]
        push    eax

        fld     st0
        fidiv   dword [esp]
        fist    [distance_to_floor]                             ; FloorDistanceDividend / cos (Camera.Direction - angle) / (ecx - ProjectionPlane.MidY)

        fld     st0
        fmul    st0, st4                                        ; distance * cos (angle)
        fistp   dword [esp]
        mov     ebx, [esp]                                      ; ebx = x delta

        fmul    st0, st4                                        ; distance * sin (angle)
        fistp   dword [esp]
        pop     edx                                             ; edx = y delta

        mov     eax, [akode_data.Camera.Position.X]
        add     eax, ebx
        mov     ebx, [akode_data.Camera.Position.Y]
        sub     ebx, edx

        push    ecx
        mov     ecx, [akode_data.BlockWidthPowerOf2]

        mov     edx, -1
        shl     edx, cl
        not     edx

        mov     esi, eax
        and     esi, edx
        mov     [texture_x_coordinate], esi

        mov     esi, ebx
        and     esi, edx
        mov     [texture_y_coordinate], esi

        sar     eax, cl
        js      .next_floor_intersection
        cmp     [akode_data.CurrentLevel.Size.Width], eax
        jna     .next_floor_intersection

        sar     ebx, cl
        js      .next_floor_intersection
        cmp     [akode_data.CurrentLevel.Size.Height], ebx
        jna     .next_floor_intersection

        imul    ebx, [akode_data.CurrentLevel.Size.Width]
        add     ebx, eax
        mov     eax, [akode_data.CurrentLevelGridPtr]
        mov     eax, [eax + ebx * 4]                            ; eax = pointer to akode.GridCell

        test    eax, eax
        jz      .next_floor_intersection

        mov     esi, [eax + akode.GridCell.FloorTexture]
        test    esi, esi
        jz      .next_floor_intersection

if ~(defined DISABLE_SHADING & DISABLE_SHADING)
        mov     edx, [akode_data.ShadingTablePtr]
        mov     ebx, [akode_data.ShadingDistance]
        mov     ecx, [distance_to_floor]
        cmp     ebx, ecx
        jae     @f
        mov     ecx, ebx
@@:
        mov     edx, [edx + ecx * 4]
        mov     ebx, [akode_data.ShadingColor]
end if

        mov     eax, [esi + akode.TextureDesc.Type]
        test    eax, eax
        jz      .texture_type_image2

        ; texture type is color
        mov     eax, [esi + akode.TextureDesc.Color]

if ~(defined DISABLE_SHADING & DISABLE_SHADING)
        akode._.blend_colors
end if

        mov     [edi], eax

        jmp     .next_floor_intersection

.texture_type_image2:
        mov     eax, [texture_x_coordinate]
        imul    eax, [akode_data.BlockSize.Width]
        add     eax, [texture_y_coordinate]
        mov     esi, [esi + akode.TextureDesc.ImageDataPtr]
        mov     eax, [esi + eax * 4]

if ~(defined DISABLE_SHADING & DISABLE_SHADING)
        akode._.blend_colors
end if

        mov     [edi], eax

.next_floor_intersection:
        pop     ecx

        add     edi, 4
        add     ecx, 1
        cmp     [akode_data.ProjectionPlane.Size.Height], ecx
        jae     .draw_floor_slice_loop

        fstp    st0

.skip_draw_floor_slice:

; ---------------------------- draw ceiling slice ---------------------------- ;

        mov     ecx, [wall_slice_top]
        test    ecx, ecx
        js      .skip_draw_ceiling_slice

        sub     ecx, 1
        fild    [akode_data.CeilingDistanceDividend]
        fmul    st0, st1                                        ; st0 = CeilingDistanceDividend / cos (Camera.Direction - angle)
        mov     edi, [wall_slice_top_ptr]

.draw_ceiling_slice_loop:
        mov     eax, [akode_data.ProjectionPlane.MidY]
        sub     eax, ecx
        push    eax

        fld     st0
        fidiv   dword [esp]
        fist    [distance_to_floor]                             ; CeilingDistanceDividend / cos (Camera.Direction - angle) / (ProjectionPlane.MidY - ecx)

        fld     st0
        fmul    st0, st4                                        ; distance * cos (angle)
        fistp   dword [esp]
        mov     ebx, [esp]                                      ; ebx = x delta

        fmul    st0, st4                                        ; distance * sin (angle)
        fistp   dword [esp]
        pop     edx                                             ; edx = y delta

        mov     eax, [akode_data.Camera.Position.X]
        add     eax, ebx
        mov     ebx, [akode_data.Camera.Position.Y]
        sub     ebx, edx

        push    ecx
        mov     ecx, [akode_data.BlockWidthPowerOf2]

        mov     edx, -1
        shl     edx, cl
        not     edx

        mov     esi, eax
        and     esi, edx
        mov     [texture_x_coordinate], esi

        mov     esi, ebx
        and     esi, edx
        mov     [texture_y_coordinate], esi

        sar     eax, cl
        js      .next_ceiling_intersection
        cmp     [akode_data.CurrentLevel.Size.Width], eax
        jna     .next_ceiling_intersection

        sar     ebx, cl
        js      .next_ceiling_intersection
        cmp     [akode_data.CurrentLevel.Size.Height], ebx
        jna     .next_ceiling_intersection

        imul    ebx, [akode_data.CurrentLevel.Size.Width]
        add     ebx, eax
        mov     eax, [akode_data.CurrentLevelGridPtr]
        mov     eax, [eax + ebx * 4]                            ; eax = pointer to akode.GridCell

        test    eax, eax
        jz      .next_ceiling_intersection

        mov     esi, [eax + akode.GridCell.CeilingTexture]
        test    esi, esi
        jz      .next_ceiling_intersection

if ~(defined DISABLE_SHADING & DISABLE_SHADING)
        mov     edx, [akode_data.ShadingTablePtr]
        mov     ebx, [akode_data.ShadingDistance]
        mov     ecx, [distance_to_floor]
        cmp     ebx, ecx
        jae     @f
        mov     ecx, ebx
@@:
        mov     edx, [edx + ecx * 4]
        mov     ebx, [akode_data.ShadingColor]
end if

        mov     eax, [esi + akode.TextureDesc.Type]
        test    eax, eax
        jz      .texture_type_image3

        ; texture type is color
        mov     eax, [esi + akode.TextureDesc.Color]

if ~(defined DISABLE_SHADING & DISABLE_SHADING)
        akode._.blend_colors
end if

        mov     [edi], eax

        jmp     .next_ceiling_intersection

.texture_type_image3:
        mov     eax, [texture_x_coordinate]
        imul    eax, [akode_data.BlockSize.Width]
        add     eax, [texture_y_coordinate]
        mov     esi, [esi + akode.TextureDesc.ImageDataPtr]
        mov     eax, [esi + eax * 4]

if ~(defined DISABLE_SHADING & DISABLE_SHADING)
        akode._.blend_colors
end if

        mov     [edi], eax

.next_ceiling_intersection:
        pop     ecx

        sub     edi, 4
        sub     ecx, 1
        cmp     ecx, -1
        jge     .draw_ceiling_slice_loop

        fstp    st0

.skip_draw_ceiling_slice:

        fstp    st0
        fstp    st0
        fstp    st0

        sub     [start_angle], 1
        jns     @f
        mov     edx, [akode_data.Angle360]
        sub     edx, 1
        mov     [start_angle], edx

@@:
        mov     eax, [first_slice]
        add     eax, 1
        mov     [first_slice], eax
        cmp     [last_slice], eax
        jae     .slice_loop

        ret
endp
; ============================================================================ ;

; ============================================================================ ;
macro akode._.draw_object_slice
{
;        mov     eax, [akode_data.BlockSize.Height]
;        xor     edx, edx
;        div     [object_screen_height]
;        shl     eax, 2
;        mov     [slice.int_part], eax
;        mov     [slice.fract_part], edx

        xor     eax, eax
        mov     [slice.e], eax
        mov     ecx, [slice.pixel_count]

.draw_object_slice:
        mov     eax, [slice.y]
        test    eax, eax
        js      .skip_pixel

        mov     eax, [esi]
        cmp     eax, 0FF00FFh
        je      @f
        mov     [edi], eax
@@:     add     edi, 4

.skip_pixel:
        add     [slice.y], 1
        add     esi, [slice.int_part]
        mov     eax, [slice.e]
        add     eax, [slice.fract_part]
        mov     [slice.e], eax

        sub     eax, [object_screen_height]
        jl      @f
        mov     [slice.e], eax
        add     esi, 4

@@:
        sub     ecx, 1
        jnz     .draw_object_slice
}

macro akode._.draw_object_slice_with_shading
{
        xor     eax, eax
        mov     [slice.e], eax
        mov     ecx, [slice.pixel_count]

.draw_object_slice2:
        mov     eax, [slice.y]
        test    eax, eax
        js      .skip_pixel2

        mov     eax, [esi]
        cmp     eax, 0FF00FFh
        je      @f
        akode._.blend_colors_mmx
        mov     [edi], eax
@@:     add     edi, 4

.skip_pixel2:
        add     [slice.y], 1
        add     esi, [slice.int_part]
        mov     eax, [slice.e]
        add     eax, [slice.fract_part]
        mov     [slice.e], eax

        sub     eax, [object_screen_height]
        jl      @f
        mov     [slice.e], eax
        add     esi, 4

@@:
        sub     ecx, 1
        jnz     .draw_object_slice2
}

macro akode._.draw_object
{
        locals
                slice.int_part          dd ?
                slice.fract_part        dd ?
                slice.e                 dd ?
                slice.y                 dd ?
                slice.pixel_count       dd ?

                int_part                dd ?
                fract_part              dd ?
                e                       dd ?
                slice_count             dd ?
                object_distance         dd ?
        endl

        mov     eax, [object_screen_height]
        mov     ebx, [object_screen_width]

        cmp     eax, 10
        jb      .draw_object_exit
        cmp     ebx, 10
        jb      .draw_object_exit

        mov     ecx, [object_screen_x]
        mov     edx, [object_screen_y]

        cmp     ecx, [akode_data.ProjectionPlane.Size.Width]
        jge     .draw_object_exit
        cmp     edx, [akode_data.ProjectionPlane.Size.Height]
        jge     .draw_object_exit

        add     ecx, ebx
        add     edx, eax

        xor     edi, edi

        cmp     ecx, edi
        jle     .draw_object_exit
        cmp     edx, edi
        jle     .draw_object_exit

        sub     ecx, [akode_data.ProjectionPlane.Size.Width]
        jbe     @f
        sub     ebx, ecx
@@:
        mov     [slice_count], ebx

        sub     edx, [akode_data.ProjectionPlane.Size.Height]
        jbe     @f
        sub     eax, edx
@@:
        mov     [slice.pixel_count], eax

        mov     eax, [akode_data.BlockSize.Height]
        xor     edx, edx
        div     [object_screen_height]
        shl     eax, 2
        mov     [slice.int_part], eax
        mov     [slice.fract_part], edx

        mov     eax, [akode_data.BlockSize.Width]
        xor     edx, edx
        div     [object_screen_width]
        mov     [fract_part], edx
        mul     [akode_data.BlockSize.Height]
        shl     eax, 2
        mov     [int_part], eax

        xor     eax, eax
        mov     [e], eax

        mov     eax, [object_screen_x]
        cmp     eax, edi
        jge     @f
        xor     eax, eax

@@:
        mov     edi, [akode_data.ImageBufferPtr]
        mul     [akode_data.ProjectionPlane.Size.Height]
        lea     edi, [edi + eax * 4]

        mov     eax, [esi + akode.Object.Distance]
        mov     [object_distance], eax
        mov     ecx, [esi + akode.Object.ShadingDistance]

        mov     eax, [esi + akode.Object.DisableShading]
        mov     esi, [esi + akode.Object.TextureDescPtr]
        mov     esi, [esi + akode.TextureDesc.ImageDataPtr]

if ~(defined DISABLE_SHADING & DISABLE_SHADING)
        test    eax, eax
        jnz     .draw_object_without_shading

        ; draw object with shading
        mov     edx, [akode_data.ShadingTablePtr]
        mov     ebx, [akode_data.ShadingDistance]
        cmp     ebx, ecx
        jae     @f
        mov     ecx, ebx
@@:
        mov     edx, [edx + ecx * 4]
        mov     ebx, [akode_data.ShadingColor]

.draw_object_slices_loop2:
        mov     eax, [object_screen_x]
        test    eax, eax
        js      .skip_slice2

        mov     ecx, [akode_data.DepthBufferPtr]
        mov     eax, [ecx + eax * 4]
        cmp     [object_distance], eax
        jae     .slice_hidden_by_wall2

        push    esi
        push    edi

        mov     eax, [object_screen_y]
        mov     [slice.y], eax

        test    eax, eax
        jns     @f
        xor     eax, eax
@@:     lea     edi, [edi + eax * 4]

        akode._.draw_object_slice_with_shading

        pop     edi
        pop     esi

.slice_hidden_by_wall2:
        mov     eax, [akode_data.ProjectionPlane.Size.Height]
        lea     edi, [edi + eax * 4]

.skip_slice2:
        add     [object_screen_x], 1
        add     esi, [int_part]
        mov     eax, [e]
        add     eax, [fract_part]
        mov     [e], eax

        sub     eax, [object_screen_width]
        jl      @f
        mov     [e], eax
        mov     eax, [akode_data.BlockSize.Height]
        lea     esi, [esi + eax * 4]

@@:
        sub     [slice_count], 1
        jnz     .draw_object_slices_loop2

        emms

        jmp     .draw_object_exit
end if

.draw_object_without_shading:

.draw_object_slices_loop:
        mov     eax, [object_screen_x]
        test    eax, eax
        js      .skip_slice

        mov     ecx, [akode_data.DepthBufferPtr]
        mov     eax, [ecx + eax * 4]
        cmp     [object_distance], eax
        jae     .slice_hidden_by_wall

        push    esi
        push    edi

        mov     eax, [object_screen_y]
        mov     [slice.y], eax

        test    eax, eax
        jns     @f
        xor     eax, eax
@@:     lea     edi, [edi + eax * 4]

        akode._.draw_object_slice

        pop     edi
        pop     esi

.slice_hidden_by_wall:
        mov     eax, [akode_data.ProjectionPlane.Size.Height]
        lea     edi, [edi + eax * 4]

.skip_slice:
        add     [object_screen_x], 1
        add     esi, [int_part]
        mov     eax, [e]
        add     eax, [fract_part]
        mov     [e], eax

        sub     eax, [object_screen_width]
        jl      @f
        mov     [e], eax
        mov     eax, [akode_data.BlockSize.Height]
        lea     esi, [esi + eax * 4]

@@:
        sub     [slice_count], 1
        jnz     .draw_object_slices_loop

        .draw_object_exit:
}

proc akode._.draw_objects start_angle
        locals
                camera_cell_x           dd ?
                camera_cell_y           dd ?

                quadrants               dd ?
                x_quadrants             dd ?
                y_quadrants             dd ?

                object_count            dd ?

                object_screen_x_delta   dd ?
                object_screen_x         dd ?
                object_screen_y         dd ?
                object_screen_width     dd ?
                object_screen_height    dd ?
        endl

        mov     eax, [akode_data.CurrentLevel.ObjectCount]
        test    eax, eax
        jz      .exit

        fninit

        mov     eax, [start_angle]

        cmp     [akode_data.Angle0], eax
        jne     @f
        mov     ebx, 1001b
        jmp     .first_quadrant_found

@@:     cmp     [akode_data.Angle90], eax
        jne     @f
        mov     ebx, 0011b
        jmp     .first_quadrant_found

@@:     jb      @f
        mov     ebx, 0001b
        jmp     .first_quadrant_found

@@:     cmp     [akode_data.Angle180], eax
        jne     @f
        mov     ebx, 0110b
        jmp     .first_quadrant_found

@@:     jb      @f
        mov     ebx, 0010b
        jmp     .first_quadrant_found

@@:     cmp     [akode_data.Angle270], eax
        jne     @f
        mov     ebx, 1100b
        jmp     .first_quadrant_found

@@:     jb      @f
        mov     ebx, 0100b
        jmp     .first_quadrant_found

@@:     mov     ebx, 1000b

.first_quadrant_found:
        sub     eax, [akode_data.ProjectionPlane.Size.Width]
        add     eax, 1
        jns     @f
        add     eax, [akode_data.Angle360]

@@:
        cmp     [akode_data.Angle0], eax
        jne     @f
        mov     edx, 1001b
        jmp     .second_quadrant_found

@@:     cmp     [akode_data.Angle90], eax
        jne     @f
        mov     edx, 0011b
        jmp     .second_quadrant_found

@@:     jb      @f
        mov     edx, 0001b
        jmp     .second_quadrant_found

@@:     cmp     [akode_data.Angle180], eax
        jne     @f
        mov     edx, 0110b
        jmp     .second_quadrant_found

@@:     jb      @f
        mov     edx, 0010b
        jmp     .second_quadrant_found

@@:     cmp     [akode_data.Angle270], eax
        jne     @f
        mov     edx, 1100b
        jmp     .second_quadrant_found

@@:     jb      @f
        mov     edx, 0100b
        jmp     .second_quadrant_found

@@:     mov     edx, 1000b

.second_quadrant_found:
        mov     eax, ebx
        and     eax, edx
        jnz     @f
        mov     eax, ebx
        or      eax, edx

@@:
        mov     [quadrants], eax

        mov     ecx, [akode_data.BlockWidthPowerOf2]

        mov     eax, [akode_data.Camera.Position.X]
        shr     eax, cl
        mov     [camera_cell_x], eax

        mov     eax, [akode_data.Camera.Position.Y]
        shr     eax, cl
        mov     [camera_cell_y], eax

        mov     esi, [akode_data.CurrentLevelObjectsPtr]
        xor     eax, eax
        mov     [object_count], eax
        mov     ecx, [akode_data.CurrentLevel.ObjectCount]

.calc_distance_to_objects_loop:
        mov     eax, [esi + akode.Object.Visible]
        test    eax, eax
        jz      .next_object

        mov     ebx, [esi + akode.Object.Position.X]
        mov     edx, [esi + akode.Object.Position.Y]

        cmp     ebx, [camera_cell_x]
        jne     @f
        mov     [x_quadrants], 1111b
        jmp     .x_quadrants_found

@@:     jb      @f
        mov     [x_quadrants], 1001b
        jmp     .x_quadrants_found

@@:     mov     [x_quadrants], 0110b

.x_quadrants_found:
        cmp     edx, [camera_cell_y]
        jne     @f
        mov     [y_quadrants], 1111b
        jmp     .y_quadrants_found

@@:     jb      @f
        mov     [y_quadrants], 1100b
        jmp     .y_quadrants_found

@@:     mov     [y_quadrants], 0011b

.y_quadrants_found:
        mov     eax, [x_quadrants]
        and     eax, [y_quadrants]
        cmp     eax, 0Fh
        je      .next_object

        test    eax, [quadrants]
        jz      .next_object

        push    ecx
        mov     ecx, [akode_data.BlockWidthPowerOf2]
        shl     ebx, cl
        shl     edx, cl
        pop     ecx
        mov     eax, [akode_data.BlockSize.Width]
        shr     eax, 1
        add     ebx, eax
        add     edx, eax

        sub     ebx, [akode_data.Camera.Position.X]
        neg     edx
        add     edx, [akode_data.Camera.Position.Y]

        push    ebx
        push    edx
        fild    dword [esp]
        fild    dword [esp + 4]
        fpatan
        add     esp, 8

        imul    ebx, ebx
        imul    edx, edx
        add     ebx, edx

        push    ebx
        fild    dword [esp]
        fsqrt
        add     esp, 4
        fistp   [esi + akode.Object.Distance]

        mov     eax, [esi + akode.Object.Distance]
        mov     [esi + akode.Object.ShadingDistance], eax

        fldpi
        fldpi
        faddp
        fidivr  [akode_data.Angle360]                           ; Angle360 / 2 * pi
        fmulp
        fistp   [esi + akode.Object.Angle]

        mov     eax, [esi + akode.Object.Angle]
        test    eax, eax
        jns     @f
        add     eax, [akode_data.Angle360]
        mov     [esi + akode.Object.Angle], eax

@@:
        push    esi
        add     [object_count], 1

.next_object:
        add     esi, sizeof.akode.Object
        sub     ecx, 1
        jnz     .calc_distance_to_objects_loop

        mov     eax, [object_count]
        test    eax, eax
        jz      .exit

        mov     ebx, esp
        stdcall akode._.sort_objects, ebx, eax

        mov     ecx, [object_count]

.draw_objects_loop:
        pop     esi

        mov     eax, [esi + akode.Object.Angle]
        sub     eax, [akode_data.Camera.Direction]
        mov     ebx, eax
        jns     @f
        add     eax, [akode_data.Angle360]
        neg     ebx

@@:
        cmp     [akode_data.Angle90], eax
        je      .draw_next_object
        cmp     [akode_data.Angle270], eax
        je      .draw_next_object

        mov     edi, [akode_data.TrigonometricTablePtr]

        shl     eax, 5
        fld     qword [edi + eax + 16]
        fimul   [akode_data.CameraToPlaneDistance]
        fistp   [object_screen_x_delta]

        shl     ebx, 5
        fld     qword [edi + ebx + 8]
        fabs
        fimul   [esi + akode.Object.Distance]
        fist    [esi + akode.Object.Distance]
        fidivr  [akode_data.WallHeightDividend]
        fistp   [object_screen_height]
        ;fimul   [akode_data.BlockSize.Width]
        ;fidiv   [akode_data.BlockSize.Height]
        ;fistp   [object_screen_width]

        mov     ebx, [akode_data.ProjectionPlane.MidY]
        mov     eax, [object_screen_height]
        mov     edx, eax
        shr     edx, 1
        sub     ebx, edx
        mov     [object_screen_y], ebx

        mul     [akode_data.BlockSize.Width]
        div     [akode_data.BlockSize.Height]
        mov     [object_screen_width], eax

        mov     ebx, [akode_data.ProjectionPlane.Size.Width]
        shr     ebx, 1
        sub     ebx, [object_screen_x_delta]
        shr     eax, 1
        sub     ebx, eax
        mov     [object_screen_x], ebx

        push    ecx
        akode._.draw_object
        pop     ecx

.draw_next_object:
        sub     ecx, 1
        jnz     .draw_objects_loop

.exit:
        ret
endp
; ============================================================================ ;

; ============================================================================ ;
proc akode._.sort_objects objects_ptr, object_count
        mov     edx, [object_count]
        mov     ecx, 1

        cmp     edx, ecx
        jbe     .exit

        mov     esi, [objects_ptr]

.sort_loop:
        mov     ebx, ecx
        mov     edi, [esi + ebx * 4]

@@:
        test    ebx, ebx
        jz      .insert

        mov     eax, [esi + ebx * 4 - 4]
        mov     eax, [eax + akode.Object.Distance]
        cmp     [edi + akode.Object.Distance], eax
        jna     .insert

        mov     eax, [esi + ebx * 4 - 4]
        mov     [esi + ebx * 4], eax
        sub     ebx, 1
        jmp     @b

.insert:
        mov     [esi + ebx * 4], edi

        add     ecx, 1
        cmp     edx, ecx
        jne     .sort_loop

.exit:
        ret
endp

;void insertionsort()
;    {
;        int i, j, t;
;        for (i=1; i<n; i++)
;        {
;            j=i;
;            t=a[j];
;            while (j>0 && a[j-1]>t)
;            {
;                a[j]=a[j-1];
;                j--;
;            }
;            a[j]=t;
;        }
;    }
; ============================================================================ ;

; ============================================================================ ;
; > ebx = width of image (in px)                                               ;
; > edx = height of image (in px)                                              ;
; > esi = pointer to source block                                              ;
; > edi = pointer to destination block                                         ;
; ============================================================================ ;
macro akode._.transpose16x16
{
        local   .loop1, .loop2

        push    ebx
        push    edx

        mov     ebx, 15

.loop1:
        mov     ecx, 15

.loop2:
        mov     eax, ecx
        imul    eax, [esp]
        add     eax, ebx
        mov     eax, [esi + eax * 4]

        mov     edx, ebx
        imul    edx, [esp + 4]
        add     edx, ecx
        mov     [edi + edx * 4], eax

        sub     ecx, 1
        jns     .loop2

        sub     ebx, 1
        jns     .loop1

        pop     edx
        pop     ebx
}

; ============================================================================ ;
; > ebx = width of image / 2 (in px)                                           ;
; > edx = height of image (in px)                                              ;
; > esi = pointer to source block                                              ;
; > edi = pointer to destination block                                         ;
; ============================================================================ ;
macro akode._.transpose16x16to8x8
{
        local   .loop1, .loop2

        push    ebx

        mov     ebx, 7

.loop1:
        mov     ecx, 7

.loop2:
        mov     eax, ecx
        imul    eax, edx
        add     eax, ebx
        lea     eax, [esi + eax * 8]

        movq    mm0, [eax]
        pavgb   mm0, [eax + edx * 4]
        movq    mm1, mm0
        psrlq   mm0, 32
        pavgb   mm1, mm0

        mov     eax, ebx
        imul    eax, [esp]
        add     eax, ecx
        movd    [edi + eax * 4], mm1

        sub     ecx, 1
        jns     .loop2

        sub     ebx, 1
        jns     .loop1

        pop     ebx
}

; ============================================================================ ;
proc akode.get_image uses eax ebx ecx edx esi edi, buffer_ptr, downscale_factor_pow2
        mov     esi, [akode_data.ImageBufferPtr]
        mov     edi, [buffer_ptr]
        mov     ebx, [akode_data.ProjectionPlane.Size.Width]
        mov     edx, [akode_data.ProjectionPlane.Size.Height]
        mov     eax, [downscale_factor_pow2]

        mov     ecx, [akode_data.OptimizedGetImage]
        ;xor     ecx, ecx
        test    ecx, ecx
        jz      .no_optimization

; optimized
        test    edi, 0Fh
        jz      @f
        DEBUGF  DEBUG_INFO, 'akode.get_image: buffer_ptr is not aligned by 16\n'
@@:

        test    eax, eax
        jnz     .downscale2_optimized

; no downscale optimized
        xor     eax, eax

.loop1:
        xor     ecx, ecx

.loop2:
;        mov     esi, ecx
;        imul    esi, edx
;        add     esi, eax
;        shl     esi, 2
;        add     esi, [akode_data.ImageBufferPtr]
;
;        mov     edi, eax
;        imul    edi, ebx
;        add     edi, ecx
;        shl     edi, 2
;        add     edi, [buffer_ptr]

        mov     esi, ecx
        mov     edi, eax

        imul    esi, edx
        imul    edi, ebx

        add     esi, eax
        add     edi, ecx

        shl     esi, 2
        shl     edi, 2

        add     esi, [akode_data.ImageBufferPtr]
        add     edi, [buffer_ptr]

        push    eax
        push    ecx

        akode._.transpose16x16

        pop     ecx
        pop     eax

        add     ecx, 16
        cmp     ecx, ebx
        jne     .loop2

        add     eax, 16
        cmp     eax, edx
        jne     .loop1

        jmp     .exit

.downscale2_optimized:
        shr     ebx, 1

        xor     eax, eax

.loop3:
        xor     ecx, ecx

.loop4:
;        mov     esi, ecx
;        imul    esi, edx
;        add     esi, eax
;        shl     esi, 2
;        add     esi, [akode_data.ImageBufferPtr]
;
;        mov     edi, eax
;        imul    edi, ebx
;        add     edi, ecx
;        shl     edi, 1
;        add     edi, [buffer_ptr]

        mov     esi, ecx
        mov     edi, eax

        imul    esi, edx
        imul    edi, ebx

        add     esi, eax
        add     edi, ecx

        shl     esi, 2
        shl     edi, 1

        add     esi, [akode_data.ImageBufferPtr]
        add     edi, [buffer_ptr]

        push    eax
        push    ecx

        akode._.transpose16x16to8x8

        pop     ecx
        pop     eax

        add     ecx, 16
        cmp     [akode_data.ProjectionPlane.Size.Width], ecx
        jne     .loop4

        add     eax, 16
        cmp     eax, edx
        jne     .loop3

        emms

        jmp     .exit

.no_optimization:
        test    eax, eax
        jnz     .downscale2_no_optimization

; no downscale
        mov     ecx, ebx
        shl     ebx, 2
@@:
        mov     eax, [esi]
        mov     [edi], eax
        add     esi, 4
        add     edi, ebx

        sub     edx, 1
        jnz     @b

        mov     edx, [akode_data.ProjectionPlane.Size.Height]
        mov     edi, [buffer_ptr]
        add     edi, 4
        mov     [buffer_ptr], edi

        sub     ecx, 1
        jnz     @b

        jmp     .exit

.downscale2_no_optimization:
        mov     eax, edx
        shr     edx, 1
        shl     eax, 2

        shr     ebx, 1
        mov     ecx, ebx
        shl     ecx, 2
@@:
        movq    mm0, [esi]
        pavgb   mm0, [esi + eax]
        movq    mm1, mm0
        psrlq   mm0, 32
        pavgb   mm1, mm0
        movd    [edi], mm1

        add     esi, 8
        add     edi, ecx

        sub     edx, 1
        jnz     @b

        mov     edx, [akode_data.ProjectionPlane.Size.Height]
        shr     edx, 1
        add     esi, eax
        mov     edi, [buffer_ptr]
        add     edi, 4
        mov     [buffer_ptr], edi

        sub     ebx, 1
        jnz     @b

        emms

.exit:
        ret
endp
; ============================================================================ ;
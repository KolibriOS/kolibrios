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

format binary as 'kex'

__DEBUG__       = 1             ; 0 - disable debug output / 1 - enable debug output
__DEBUG_LEVEL__ = DEBUG_FINE    ; DEBUG_FINE - all debug messages / DEBUG_INFO - info and errors / DEBUG_ERR - only errors

DEBUG_FINE      = 0
DEBUG_INFO      = 1
DEBUG_ERR       = 2

include '../../macros.inc'
purge   mov, add, sub

; ================================= Header =================================== ;
MEOS_APP_START
store dword StartupPath at $ - 4

; ================================ Includes ================================== ;
include '../../debug-fdo.inc'
include '../../proc32.inc'
include '../../dll.inc'

include 'AKODE/AKODE.inc'
include 'datadef.inc'

; =============================== Entry point ================================ ;
CODE
        DEBUGF  DEBUG_INFO, 'Started\n'

        mcall   68, 11                          ; initialize heap
        test    eax, eax
        jz      .exit_fail_heap_init

        stdcall dll.Load, @IMPORT               ; load libraries
        test    eax, eax
        jnz     .exit_fail_load_libs

        mcall   40, MAIN_EVENT_MASK             ; used events
        mcall   66, 1, 1                        ; use scancodes

        stdcall draw_window

        mcall   9, ThreadInfoBuffer, -1         ; get real window size
        mov     eax, [ebx + process_information.client_box.width]
        inc     eax
        mov     [MainWindowWidth], eax
        mov     edx, [ebx + process_information.client_box.height]
        inc     edx
        mov     [MainWindowHeight], edx
        DEBUGF  DEBUG_FINE, 'Window width: %u\nWindow height: %u\n', eax, edx

        mov     ecx, eax
        shl     ecx, 2
        imul    ecx, edx                        ; ecx = width * 4 * height

        sub     edx, HUD_PANEL_HEIGHT
        mov     [WorldViewHeight], edx

if FSAA
        shl     eax, FSAA
        shl     edx, FSAA
end if

        stdcall akode.init, eax, edx, FIELD_OF_VIEW, BLOCK_BASE_SIZE, BLOCK_HEIGHT
        test    eax, eax
        jz      .exit_fail_akode_init

        stdcall akode.set_movement_speed, MOVEMENT_SPEED, 90

        mcall   68, 12                          ; alloc ecx bytes for image buffer
        test    eax, eax
        jz      .exit_fail_alloc

        mov     [ImageBufferPtr], eax
        push    eax

        stdcall set_startup_path

        stdcall load_hud_images
        test    eax, eax
        jnz     @f
        DEBUGF  DEBUG_ERR, 'Failed to load HUD images\n'
        jmp     .exit_fail_load_hud

@@:
        cld
        xor     eax, eax
        mov     edi, PressedKeys
        mov     ecx, 128 * 2
        rep     stosd

        mov     edi, Inventory
        mov     ecx, INVENTORY_SIZE * 2
        rep     stosd

        stdcall akode.set_callbacks, level_load_callback, action_callback

        stdcall akode.load_level, levels.level1

if FULLSCREEN
        stdcall hide_cursor
        push    eax
end if

        stdcall main_loop

if FULLSCREEN
        pop     ecx
        test    ecx, ecx
        jz      @f
        mcall   37, 6                           ; delete cursor
@@:
end if

.exit_fail_load_hud:
        stdcall free_hud_images

        pop     ecx
        mcall   68, 13                          ; free image buffer

        jmp     .exit

.exit_fail_heap_init:
        DEBUGF  DEBUG_ERR, 'Heap initialization failed\n'
        jmp     .exit

.exit_fail_load_libs:
        DEBUGF  DEBUG_ERR, 'Failed to load libraries\n'
        jmp     .exit

.exit_fail_akode_init:
        DEBUGF  DEBUG_ERR, 'AKODE initialization failed\n'
        jmp     .exit

.exit_fail_alloc:
        DEBUGF  DEBUG_ERR, 'Memory allocation for image buffer failed\n'
        ;jmp     .exit

.exit:
        stdcall akode.cleanup
        DEBUGF  DEBUG_INFO, 'Exiting\n'

        xor     eax, eax
        dec     eax
        mcall                                   ; kill this thread

; ============================================================================ ;
proc set_startup_path
        cld
        mov     esi, StartupPath
        mov     ecx, esi

@@:
        lodsb
        test    al, al
        jnz     @b

        sub     esi, 2
        std

@@:
        lodsb
        cmp     al, '/'
        jne     @b

        mov     [esi + 1], byte 0

        mcall   30, 1                           ; set current directory

        ret
endp
; ============================================================================ ;

; ============================================================================ ;
; < eax = 0 - fail                                                             ;
; ============================================================================ ;
proc load_hud_images
        mov     ebx, [MainWindowWidth]
        mov     ecx, [MainWindowHeight]
        xor     edx, edx

        stdcall akode.load_and_scale_image, LevelLoadingImageFile, ebx, ecx, edx
        test    eax, eax
        jz      .exit

        mov     [LevelLoadingImagePtr], eax

        stdcall akode.load_and_scale_image, DeathImageFile, ebx, ecx, edx
        test    eax, eax
        jz      .exit

        mov     [DeathImagePtr], eax

        stdcall akode.load_and_scale_image, EndImageFile, ebx, ecx, edx
        test    eax, eax
        jz      .exit

        mov     [EndImagePtr], eax

        stdcall akode.load_and_scale_image, HudPanelImageFile, ebx, HUD_PANEL_HEIGHT, edx
        test    eax, eax
        jz      .exit

        mov     [HudPanelImagePtr], eax

.exit:
        ret
endp
; ============================================================================ ;

; ============================================================================ ;
proc free_hud_images
        xor     edx, edx
        mov     ebx, 13

        mov     ecx, [LevelLoadingImagePtr]
        test    ecx, ecx
        jz      @f
        mcall   68                              ; free
        mov     [LevelLoadingImagePtr], edx
@@:
        mov     ecx, [HudPanelImagePtr]
        test    ecx, ecx
        jz      @f
        mcall   68
        mov     [HudPanelImagePtr], edx
@@:
        mov     ecx, [DeathImagePtr]
        test    ecx, ecx
        jz      @f
        mcall   68
        mov     [DeathImagePtr], edx
@@:
        mov     ecx, [EndImagePtr]
        test    ecx, ecx
        jz      @f
        mcall   68
        mov     [EndImagePtr], edx
@@:

        ret
endp
; ============================================================================ ;

; ============================================================================ ;
; < eax = cursor handle / 0 - fail                                             ;
; ============================================================================ ;
proc hide_cursor
        mcall   68, 12, 32 * 32 * 4
        test    eax, eax
        jz      .exit

        mov     edi, eax
        xor     ebx, ebx
        shr     ecx, 2
@@:     mov     [eax], ebx
        add     eax, 4
        loop    @b

        mcall   37, 4, edi, 2                   ; load cursor
        mov     ecx, eax
        inc     ebx
        mcall   37                              ; set cursor

        xchg    edi, ecx

        mcall   68, 13

        mov     eax, edi

.exit:
        ret
endp
; ============================================================================ ;

; ============================================================================ ;
proc draw_image uses eax ebx ecx edx esi edi, image_ptr, x, y, width, height
        mov     ebx, [image_ptr]
        mpack   ecx, [width], [height]
        mpack   edx, [x], [y]
        mov     esi, 32
        xor     edi, edi
        xchg    edi, ebp
        mcall   65
        mov     ebp, edi

        ret
endp
; ============================================================================ ;

; ============================================================================ ;
proc draw_image_to_buffer uses eax ebx ecx edx esi edi, image_ptr, x, y, width, height
        cld
        mov     esi, [image_ptr]
        mov     edi, [ImageBufferPtr]
        mov     ebx, [MainWindowWidth]
        mov     eax, [y]
        mul     ebx
        add     eax, [x]
        lea     edi, [edi + eax * 4]

        sub     ebx, [width]
        shl     ebx, 2
        mov     edx, [height]

@@:
        mov     ecx, [width]
        rep     movsd
        add     edi, ebx

        sub     edx, 1
        jnz     @b

        ret
endp
; ============================================================================ ;

; ============================================================================ ;
proc draw_image_with_transparency_to_buffer uses eax ebx ecx edx esi edi, image_ptr, x, y, width, height
        mov     esi, [image_ptr]
        mov     edi, [ImageBufferPtr]
        mov     ebx, [MainWindowWidth]
        mov     eax, [y]
        mul     ebx
        add     eax, [x]
        lea     edi, [edi + eax * 4]

        sub     ebx, [width]
        shl     ebx, 2
        mov     edx, [height]

.y_draw_loop:
        mov     ecx, [width]

.x_draw_loop:
        mov     eax, [esi]
        cmp     eax, 0FF00FFh
        je      @f
        mov     [edi], eax
@@:
        add     esi, 4
        add     edi, 4

        sub     ecx, 1
        jnz     .x_draw_loop

        add     edi, ebx

        sub     edx, 1
        jnz     .y_draw_loop

        ret
endp
; ============================================================================ ;

; ============================================================================ ;
proc draw_window
        mcall   12, 1                           ; start drawing

if FULLSCREEN
        mov     ebx, 0FFFFh
        mov     ecx, 0FFFFh
else
        mcall   48, 4                           ; eax - skin height

        mpack   ebx, MAIN_WINDOW_X, MAIN_WINDOW_WIDTH + 9
        mpack   ecx, MAIN_WINDOW_Y, MAIN_WINDOW_HEIGHT + 4
        add     ecx, eax
end if
        mov     edx, MAIN_WINDOW_STYLE
        mov     esi, MAIN_WINDOW_STYLE2
        mov     edi, MAIN_WINDOW_TITLE
        xor     eax, eax
        mcall                                   ; draw window

        mcall   12, 2                           ; end drawing

        ret
endp
; ============================================================================ ;

; ============================================================================ ;
proc draw_world
        mov     ebx, [ImageBufferPtr]
        ;test    ebx, ebx
        ;jz      @f

        stdcall akode.render
        stdcall akode.get_image, ebx, FSAA

        mpack   ecx, [MainWindowWidth], [WorldViewHeight]
        xor     edx, edx
        mov     esi, 32
        xor     edi, edi
        xchg    edi, ebp
        mcall   65
        mov     ebp, edi

;@@:
        ret
endp
; ============================================================================ ;

; ============================================================================ ;
proc draw_hud
        mov     eax, [HudPanelNeedsRedraw]
        test    eax, eax
        jz      .exit

        xor     eax, eax
        mov     [HudPanelNeedsRedraw], eax

        stdcall draw_image_to_buffer, [HudPanelImagePtr], eax, [WorldViewHeight], [MainWindowWidth], HUD_PANEL_HEIGHT

        mov     esi, Inventory + 4
        mov     ebx, INVENTORY_Y
        add     ebx, [WorldViewHeight]
        mov     edx, 2

.y_inventory_loop:
        mov     eax, INVENTORY_X
        mov     ecx, INVENTORY_SIZE / 2

.x_inventory_loop:
        mov     edi, [esi]
        add     esi, 8
        test    edi, edi
        jz      @f
        stdcall draw_image_with_transparency_to_buffer, edi, eax, ebx, OBJECT_IMAGE_WIDTH, OBJECT_IMAGE_HEIGHT

@@:
        add     eax, OBJECT_IMAGE_WIDTH + INVENTORY_PADDING_X

        sub     ecx, 1
        jnz     .x_inventory_loop

        add     ebx, OBJECT_IMAGE_HEIGHT + INVENTORY_PADDING_Y

        sub     edx, 1
        jnz     .y_inventory_loop

        mpack   ecx, [MainWindowWidth], HUD_PANEL_HEIGHT
        mov     edx, [WorldViewHeight]
        mov     ebx, [ImageBufferPtr]
        mov     eax, [MainWindowWidth]
        imul    eax, edx
        lea     ebx, [ebx + eax * 4]
        mov     esi, 32
        xor     edi, edi
        xchg    edi, ebp
        mcall   65
        mov     ebp, edi

        jmp     draw_game_message

.exit:
        ret
endp
; ============================================================================ ;

; ============================================================================ ;
proc draw_game_message
        mov     esi, [GameMessage]
        test    esi, esi
        jz      .exit

        mpack   ebx, GAME_MESSAGE_X, GAME_MESSAGE_Y
        add     ebx, [WorldViewHeight]
        mov     ecx, GAME_MESSAGE_COLOR or (80h shl 24)

.draw_strings_loop:
        mov     edx, esi

@@:
        mov     al, [esi]
        add     esi, 1

        test    al, al
        jz      .draw_last_string
        cmp     al, 0Ah
        jne     @b

        mov     [esi - 1], byte 0

        mcall   4

        mov     [esi - 1], byte 0Ah
        add     ebx, 10
        jmp     .draw_strings_loop

.draw_last_string:
        mcall   4

.exit:
        ret
endp
; ============================================================================ ;

; ============================================================================ ;
proc main_loop
        locals
                frame_count     dd 0
                last_timestamp  dd 0
        endl

.main_loop:
        mcall   26, 9                           ; get timestamp
        mov     ebx, eax
        sub     ebx, [last_timestamp]
        cmp     ebx, 100
        jb      @f

        mov     [last_timestamp], eax
        imul    eax, [frame_count], 100
        xor     edx, edx
        mov     [frame_count], edx
        div     ebx                             ; eax - fps

        DEBUGF  DEBUG_FINE, 'FPS: %u\n', eax

@@:
        mcall   11                              ; check events

        test    eax, eax
        jz      .idle

        dec     eax
        jz      .redraw

        dec     eax
        jz      .key

        dec     eax
        jz      .button

        sub     eax, 3
        jz      .mouse

        jmp     .idle

.redraw:
        stdcall draw_window
        mov     [HudPanelNeedsRedraw], 1

.idle:
        mov     eax, [GameStatus]
        test    eax, eax
        jnz     @f

        stdcall akode.process

        mov     eax, [GameStatus]
        test    eax, eax
        jnz     @f

        stdcall draw_hud
        stdcall draw_world

        add     [frame_count], 1
        jmp     .main_loop

@@:     cmp     eax, GAME_STATUS.LEVEL_LOAD_FAILED
        jne     @f
        jmp     .exit

@@:     cmp     eax, GAME_STATUS.DEAD
        jne     @f
        stdcall draw_image, [DeathImagePtr], 0, 0, [MainWindowWidth], [MainWindowHeight]
        jmp     .main_loop

@@:     cmp     eax, GAME_STATUS.END
        jne     @f
        stdcall draw_image, [EndImagePtr], 0, 0, [MainWindowWidth], [MainWindowHeight]
@@:
        jmp     .main_loop

.key:
        stdcall get_key
        test    eax, eax
        jz      .idle

        cmp     eax, 001h                       ; Esc
        je      .exit

        cmp     eax, 039h                       ; Space
        je      .space_pressed
        cmp     eax, 002h                       ; 1
        jb      @f
        cmp     eax, 00Bh                       ; 0
        ja      @f

        ; 0..9 pressed
        sub     eax, 002h
        mov     eax, dword [Inventory + eax * 8]
        test    eax, eax
        jz      @f
        shl     eax, 16
        push    eax

        stdcall check_key, 02Ah                 ; left Shift
        test    eax, eax
        jnz     .shift_pressed

        stdcall check_key, 036h                 ; right Shift
        test    eax, eax
        jnz     .shift_pressed

        pop     eax
        or      eax, ACTION.USE_OBJECT
        stdcall akode.action, eax
        jmp     @f

.shift_pressed:
        pop     eax
        or      eax, ACTION.LOOK_AT_OBJECT
        stdcall akode.action, eax
        jmp     @f

.space_pressed:
        stdcall check_key, 02Ah                 ; left Shift
        test    eax, eax
        jnz     .shift_pressed2

        stdcall check_key, 036h                 ; right Shift
        test    eax, eax
        jnz     .shift_pressed2

        stdcall akode.action, ACTION.DO_SOMETHING
        jmp     @f

.shift_pressed2:
        stdcall akode.action, ACTION.LOOK_AROUND

@@:
        xor     esi, esi
        dec     esi

        stdcall check_key, 0E048h               ; ^
        test    eax, eax
        jz      @f
        mov     esi, AKODE_DIRECTION.NORTH
        jmp     .set_moving_direction

@@:     stdcall check_key, 0E050h               ; v
        test    eax, eax
        jz      @f
        mov     esi, AKODE_DIRECTION.SOUTH
        jmp     .set_moving_direction

@@:     stdcall check_key, 011h                 ; W
        test    eax, eax
        jz      @f
        mov     esi, AKODE_DIRECTION.NORTH
        jmp     .set_moving_direction

@@:     stdcall check_key, 01Fh                 ; S
        test    eax, eax
        jz      @f
        mov     esi, AKODE_DIRECTION.SOUTH
        ;jmp     .set_moving_direction

@@:

.set_moving_direction:
        test    esi, esi
        js      @f
        stdcall akode.start_moving, esi
        jmp     .turn

@@:
        stdcall akode.stop_moving

.turn:
        xor     esi, esi
        dec     esi

        stdcall check_key, 0E04Bh               ; <-
        test    eax, eax
        jz      @f
        mov     esi, AKODE_DIRECTION.WEST
        jmp     .set_turning_direction

@@:     stdcall check_key, 0E04Dh               ; ->
        test    eax, eax
        jz      @f
        mov     esi, AKODE_DIRECTION.EAST
        jmp     .set_turning_direction

@@:     stdcall check_key, 01Eh                 ; A
        test    eax, eax
        jz      @f
        mov     esi, AKODE_DIRECTION.WEST
        jmp     .set_turning_direction

@@:     stdcall check_key, 020h                 ; D
        test    eax, eax
        jz      @f
        mov     esi, AKODE_DIRECTION.EAST
        ;jmp     .set_turning_direction

@@:

.set_turning_direction:
        test    esi, esi
        js      @f
        stdcall akode.start_turning, esi
        jmp     .key

@@:
        stdcall akode.stop_turning
        jmp     .key

.mouse:

        jmp     .idle

.button:
.exit:
        ret
endp
; ============================================================================ ;

; ============================================================================ ;
; < eax = key scancode (1 byte)                                                ;
; ============================================================================ ;
macro wait_for_scancode
{
        local   .wait

.wait:
        mcall   2
        test    al, al
        jnz     .wait

        shr     eax, 8
}

; ============================================================================ ;
; < eax = key scancode / 0 - no keys                                           ;
; ============================================================================ ;
proc get_key
        mcall   2                               ; get key scancode

        test    al, al
        jz      @f
        xor     eax, eax
        jmp     .exit

@@:
        shr     eax, 8

        cmp     eax, 0E1h
        jne     @f

        wait_for_scancode
        wait_for_scancode

        xor     eax, eax
        jmp     .exit

@@:
        xor     ebx, ebx
        mov     ecx, eax

        cmp     eax, 0E0h
        jne     @f

        wait_for_scancode

        mov     ecx, eax
        or      eax, 0E000h
        mov     ebx, 128

@@:
        test    ecx, 80h
        jnz     .key_up

        ; key down
        add     ebx, ecx
        lea     ebx, [PressedKeys + ebx * 4]

        mov     edx, [ebx]
        test    edx, edx
        jz      @f

        xor     eax, eax
        jmp     .exit

@@:
        inc     edx
        mov     [ebx], edx
        jmp     .exit

.key_up:
        and     ecx, 7Fh
        add     ebx, ecx
        xor     edx, edx
        mov     [PressedKeys + ebx * 4], edx

.exit:
        ;DEBUGF  DEBUG_FINE, 'get_key: %x\n', eax:4
        ret
endp
; ============================================================================ ;

; ============================================================================ ;
; < eax = 1 - key pressed / 0 - not pressed                                    ;
; ============================================================================ ;
proc check_key scancode
        mov     eax, [scancode]
        mov     ecx, eax
        shr     eax, 8
        and     ecx, 7Fh
        xor     ebx, ebx

        cmp     eax, 0E0h
        jne     @f
        mov     ebx, 128

@@:
        add     ebx, ecx
        mov     eax, [PressedKeys + ebx * 4]

        ret
endp
; ============================================================================ ;

; ============================================================================ ;
proc level_load_callback uses eax ebx ecx, load_action, action_result
        mov     eax, [load_action]
        mov     ebx, [action_result]
        xor     ecx, ecx

        cmp     eax, AKODE_LEVEL_LOAD.START
        jne     @f
        stdcall draw_image, [LevelLoadingImagePtr], ecx, ecx, [MainWindowWidth], [MainWindowHeight]

        cmp     ebx, -1
        je      .level_load_failed

        mov     [GameMessage], ebx
        inc     ecx
        mov     [HudPanelNeedsRedraw], ecx
        jmp     .exit

@@:     cmp     eax, AKODE_LEVEL_LOAD.END
        jne     @f
        DEBUGF  DEBUG_INFO, 'Level load result: %u\n', ebx

        test    ebx, ebx
        jnz     .exit

.level_load_failed:
        DEBUGF  DEBUG_ERR, 'Failed to load level\n'
        mov     [GameStatus], GAME_STATUS.LEVEL_LOAD_FAILED
        ;jmp     .exit

@@:

.exit:
        ret
endp
; ============================================================================ ;

; ============================================================================ ;
proc action_callback action, cell_x, cell_y, action_result
        m2m     [GameMessage], [action_result]
        mov     [HudPanelNeedsRedraw], 1
        ret
endp
; ============================================================================ ;

; ============================================================================ ;
proc add_object_to_inventory uses eax ecx edi, object_id, object_image_ptr
        mov     edi, Inventory
        mov     ecx, INVENTORY_SIZE

.inventory_loop:
        mov     eax, [edi]
        test    eax, eax
        jnz     @f

        mov     eax, [object_id]
        mov     [edi], eax
        mov     eax, [object_image_ptr]
        mov     [edi + 4], eax
        mov     [HudPanelNeedsRedraw], ecx
        jmp     .exit

@@:
        add     edi, 8

        sub     ecx, 1
        jnz     .inventory_loop

.exit:
        ret
endp
; ============================================================================ ;

; ============================================================================ ;
proc remove_object_from_inventory uses eax ecx esi, object_id
        mov     eax, [object_id]
        mov     esi, Inventory
        mov     ecx, INVENTORY_SIZE

.inventory_loop:
        cmp     [esi], eax
        jne     @f

        xor     eax, eax
        mov     [esi], eax
        mov     [esi + 4], eax
        mov     [HudPanelNeedsRedraw], ecx
        jmp     .exit

@@:
        add     esi, 8

        sub     ecx, 1
        jnz     .inventory_loop

.exit:
        ret
endp
; ============================================================================ ;

; ============================================================================ ;
; < eax = pointer to image data / 0 - fail                                     ;
; ============================================================================ ;
proc load_object_image image_path_ptr
        stdcall akode.load_and_scale_image, [image_path_ptr], OBJECT_IMAGE_WIDTH, OBJECT_IMAGE_HEIGHT, 0

        ret
endp
; ============================================================================ ;

; ============================================================================ ;
proc free_object_image uses eax ebx ecx, image_ptr
        mov     ecx, [image_ptr]
        test    ecx, ecx
        jz      .exit

        mcall   68, 13

.exit:
        ret
endp
; ============================================================================ ;

; ============================================================================ ;
proc player_death
        mov     [GameStatus], GAME_STATUS.DEAD
        ret
endp
; ============================================================================ ;

; ============================================================================ ;
proc game_over
        mov     [GameStatus], GAME_STATUS.END
        ret
endp
; ============================================================================ ;

; ============================ Initialized data ============================== ;
DATA
        include 'data.inc'

        ; for debug-fdo
        include_debug_strings

        align 4
        @IMPORT:
        include 'import.inc'

; =========================== Uninitialized data ============================= ;
UDATA
        include 'udata.inc'

; ================================= The End ================================== ;
MEOS_APP_END
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2024. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;; Synhronization for MenuetOS.                                 ;;
;; Author: Halyavin Andrey, halyavin@land.ru                    ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


struct FRB
        list            LHEAD
        magic           rd 1
        handle          rd 1
        destroy         rd 1

        width           rd 1
        height          rd 1
        pitch           rd 1
        format          rd 1
        private         rd 1
        pde             rd 8
ends

align 4
create_framebuffer:
        mov     ecx, sizeof.FRB
        call    create_object
        test    eax, eax
        jz      .fail

        mov     [eax + FRB.magic], 'FRMB'
        mov     [eax + FRB.destroy], 0
.fail:
        ret


align 4
init_video:
        mov     ebp, bios_fb

        movzx   eax, byte [BOOT.bpp]      ; bpp
        mov     [_display.bits_per_pixel], eax
        mov     [_display.vrefresh], 60

        movzx   eax, word [BOOT.x_res]; X max
        cmp     eax, MAX_SCREEN_WIDTH
        ja      $
        mov     [_display.width], eax
        mov     [ebp + FRB.width], eax
        mov     [display_width_standard], eax
        dec     eax
        mov     [screen_workarea.right], eax

        movzx   eax, word [BOOT.y_res]; Y max
        cmp     eax, MAX_SCREEN_HEIGHT
        ja      $
        mov     [_display.height], eax
        mov     [ebp + FRB.height], eax
        mov     [display_height_standard], eax
        dec     eax
        mov     [screen_workarea.bottom], eax

        movzx   eax, word [BOOT.vesa_mode]    ; screen mode
        mov     dword [SCR_MODE], eax
        mov     eax, 640 *4                             ; Bytes PerScanLine
        cmp     [SCR_MODE], word 0x13                   ; 320x200
        je      @f
        cmp     [SCR_MODE], word 0x12                   ; VGA 640x480
        je      @f
        movzx   eax, word[BOOT.pitch]         ; for other modes
@@:
        mov     [_display.lfb_pitch], eax
        mov     [ebp + FRB.pitch], eax

        mov     eax, [BOOT.lfb]
        mov     [LFBAddress], eax

        mov     eax, [_display.width]
        mul     [_display.height]
        mov     [_display.win_map_size], eax

        cmp     word [SCR_MODE], 0x0012                 ; VGA (640x480 16 colors)
        je      .vga
        cmp     word [SCR_MODE], 0x0013                 ; MCGA (320*200 256 colors)
        je      .32bpp
        cmp     byte [_display.bits_per_pixel], 32
        je      .32bpp
        cmp     byte [_display.bits_per_pixel], 24
        je      .24bpp
        cmp     byte [_display.bits_per_pixel], 16
        je      .16bpp

.vga:
        mov     [PUTPIXEL], VGA_putpixel
        mov     [GETPIXEL], Vesa20_getpixel32           ; Conversion buffer is 32 bpp
        mov     [_display.bytes_per_pixel], 4           ; Conversion buffer is 32 bpp
        jmp     .finish

.16bpp:
        mov     [PUTPIXEL], Vesa20_putpixel16_new
        mov     [GETPIXEL], Vesa20_getpixel16
        mov     [_display.bytes_per_pixel], 2
        jmp     .finish

.24bpp:
        mov     [PUTPIXEL], Vesa20_putpixel24_new
        mov     [GETPIXEL], Vesa20_getpixel24
        mov     [_display.bytes_per_pixel], 3
        jmp     .finish

.32bpp:
        mov     [PUTPIXEL], Vesa20_putpixel32_new
        mov     [GETPIXEL], Vesa20_getpixel32
        mov     [_display.bytes_per_pixel], 4

.finish:
        mov     [_display.check_mouse], check_mouse_area_for_putpixel_new
        mov     [_display.check_m_pixel], check_mouse_area_for_getpixel_new

        mov     ax, word [SCR_MODE]
        cmp     ax, 0x0012
        je      .fake
        cmp     ax, 0x0013
        je      .fake

        mov     esi, [LFBAddress]
        bt      [cpu_caps], CAPS_PSE
        jnc     .create_page_tables

        mov     edx, 0x00400000
        or      esi, PG_GLOBAL + PAT_WC + PG_UWR
        and     esi, [pte_valid_mask]
        or      esi, PDE_LARGE
        mov     [ebp + FRB.pde], esi
        add     esi, edx
        mov     [ebp + FRB.pde + 4], esi
        add     esi, edx
        mov     [ebp + FRB.pde + 8], esi
        add     esi, edx
        mov     [ebp + FRB.pde + 12], esi
        add     esi, edx
.ok:
        call    calculate_fast_getting_offset_for_WinMapAddress
; for Qemu or non standard video cards
; Unfortunately [BytesPerScanLine] does not always
; equal to [_display.width] * [ScreenBPP] / 8
        call    calculate_fast_getting_offset_for_LFB
        ret

.create_page_tables:

        add     ebp, FRB.pde
        or      esi, PG_GLOBAL + PAT_WC + PG_UWR
        and     esi, [pte_valid_mask]

        stdcall alloc_kernel_space, PAGE_SIZE
        mov     edi, eax
        mov     ebx, 4

.new_pd:
        call    alloc_page
        lea     edx, [eax + PG_UWR]
        mov     [ebp], edx

        stdcall map_page, edi, eax, PG_SWR

        mov     eax, esi
        mov     ecx, PAGE_SIZE/4
@@:
        stosd
        add     eax, PAGE_SIZE
        loop    @B

        add     esi, 0x400000
        add     ebp, 4
        sub     edi, PAGE_SIZE
        dec     ebx
        jnz     .new_pd
        stdcall free_kernel_space, edi
        jmp     .ok

.fake:
        mov     [BOOT.mtrr], byte 2

        stdcall alloc_kernel_space, PAGE_SIZE
        push    eax                            ;store in stack for subsequent
        mov     edi, eax                       ;free_kernel_space call

        call    alloc_page
        lea     edx, [eax + PG_UWR]
        mov     [ebp + FRB.pde], edx

        stdcall map_page, edi, eax, PG_SWR

; max VGA=640*480*4=1228800 bytes
; + 32*640*4=81920 bytes for mouse pointer
        stdcall alloc_pages, ((1228800+81920)/PAGE_SIZE)
        or      eax, PG_GLOBAL+PG_UWR
        and     eax, [pte_valid_mask]
        mov     ecx, (1228800+81920)/PAGE_SIZE
@@:
        stosd
        add     eax, PAGE_SIZE
        loop    @B

        call    free_kernel_space
        jmp     .ok

align 4
set_framebuffer:
        push    esi
        push    edi
        lea     esi, [ecx + FRB.pde]
        mov     eax, sys_proc

        cld
        pushfd
        cli
        mov     [_display.current_lfb], ecx
.patch_pde:
        lea     edi, [eax + PROC.pdt_0 + (LFB_BASE shr 20)] ;last 8 pd entries up to 32Mb framebuffer
        mov     ecx, 4
        rep movsd                               ;patch pde
        sub     esi, 16
        mov     eax, [eax + PROC.list.next]       ;next process/address space
        cmp     eax, sys_proc
        jne     .patch_pde

        bt      [cpu_caps], CAPS_PGE
        jnc     .cr3_flush

        mov     eax, cr4
        btr     eax, 7                          ;clear cr4.PGE
        mov     cr4, eax                        ;flush TLB
        bts     eax, 7
        mov     cr4, eax                        ;flush TLB
.exit:
        popfd
        pop     edi
        pop     esi
        ret

.cr3_flush:
        mov     eax, cr3
        mov     cr3, eax                        ;flush TLB
        jmp     .exit


align 4
; void __stdcall set_lfb_mode(dword width, dword height, dword bpp,
;                             dword pitch, dword lfb_phys)
; Switches the kernel video pipeline from the VGA software-conversion buffer to a
; real hardware linear framebuffer. Selects Vesa20 draw routines by bpp, leaves the
; VGA planar-conversion path (SCR_MODE != 0x12/0x13), remaps LFB_BASE onto the
; card's VRAM via large pages and re-runs set_screen to redraw.
; Intended for cards whose VBE exposes no LFB modes (e.g. S3 ViRGE under VESA 1.2),
; where a native driver programs the linear mode at runtime and then calls this.
; NOTE: requires CAPS_PSE (4 MB pages); all current KolibriOS targets provide it.
set_lfb_mode:
; [ebp+8]=width [ebp+12]=height [ebp+16]=bpp [ebp+20]=pitch [ebp+24]=lfb_phys
        push    ebp
        mov     ebp, esp
        pushad

        ; 1. Select drawing routines according to requested bpp.
        mov     eax, [ebp+16]
        cmp     eax, 16
        je      .bpp16
        cmp     eax, 24
        je      .bpp24
        mov     [PUTPIXEL], Vesa20_putpixel32_new
        mov     [GETPIXEL], Vesa20_getpixel32
        mov     [_display.bytes_per_pixel], 4
        mov     [_display.move_cursor], move_cursor_32
        mov     [_display.restore_cursor], restore_32
        jmp     .bpp_done
.bpp16:
        mov     [PUTPIXEL], Vesa20_putpixel16_new
        mov     [GETPIXEL], Vesa20_getpixel16
        mov     [_display.bytes_per_pixel], 2
        mov     [_display.move_cursor], move_cursor_16
        mov     [_display.restore_cursor], restore_16
        jmp     .bpp_done
.bpp24:
        mov     [PUTPIXEL], Vesa20_putpixel24_new
        mov     [GETPIXEL], Vesa20_getpixel24
        mov     [_display.bytes_per_pixel], 3
        mov     [_display.move_cursor], move_cursor_24
        mov     [_display.restore_cursor], restore_24
.bpp_done:
        mov     eax, [ebp+16]
        mov     [_display.bits_per_pixel], eax

        ; The system boots in VGA planar mode (0x12), so init_display left the
        ; cursor save/restore routines pointing at the VGA_* planar variants.
        ; After switching to a linear framebuffer those address VRAM with the
        ; wrong stride, leaving a trailing/ghost cursor. Re-arm the linear
        ; select_cursor path (move/restore set per-bpp just above).
        mov     [_display.select_cursor], select_cursor

        ; 2. Leave the VGA conversion path: any SCR_MODE other than 0x12/0x13
        ;    makes the Vesa20 blitters skip the planar-conversion tail.
        mov     dword [SCR_MODE], 0x118

        ; 3. Store new geometry.
        mov     eax, [ebp+8]
        mov     [_display.width], eax
        mov     eax, [ebp+12]
        mov     [_display.height], eax
        mov     eax, [ebp+20]
        mov     [_display.lfb_pitch], eax

        ; 3a. Runtime resolution change safety: the mouse coordinates and the
        ;     saved cursor background were computed for the OLD geometry. After
        ;     shrinking the screen a stale mouse position indexes the freshly
        ;     (re)allocated, smaller win_map out of bounds (-> #PF), and the
        ;     cursor restore writes old pixels at the old stride (-> colour box).
        ;     Clamp the mouse to the new bounds and drop the saved cursor area
        ;     (re-armed by the next move_cursor).
        mov     dword [cur.h], 0
        movzx   eax, word [MOUSE_X]
        mov     ebx, [_display.width]
        dec     ebx
        cmp     eax, ebx
        jbe     @f
        mov     [MOUSE_X], bx
@@:
        movzx   eax, word [MOUSE_Y]
        mov     ebx, [_display.height]
        dec     ebx
        cmp     eax, ebx
        jbe     @f
        mov     [MOUSE_Y], bx
@@:

        ; 4. Build large-page PDEs for the card's VRAM and remap LFB_BASE onto it.
        mov     eax, [ebp+24]
        mov     [LFBAddress], eax
        mov     esi, eax
        or      esi, PG_GLOBAL + PAT_WC + PG_UWR
        and     esi, [pte_valid_mask]
        or      esi, PDE_LARGE
        mov     edi, bios_fb + FRB.pde
        mov     edx, 0x00400000
        mov     ecx, 8                  ; 8 * 4 MB = 32 MB aperture
.fill_pde:
        mov     [edi], esi
        add     edi, 4
        add     esi, edx
        loop    .fill_pde

        mov     ecx, bios_fb
        call    set_framebuffer

        ; 5. Recompute offset tables / win_map.
        mov     eax, [_display.width]
        dec     eax
        mov     edx, [_display.height]
        dec     edx
        mov     ecx, [_display.lfb_pitch]
        call    set_screen

        ; 5a. Runtime resolution change: windows positioned/sized for the OLD
        ;     (larger) screen would draw past the new, smaller win_map/LFB and
        ;     crash. Clamp every used window to the new screen using the kernel's
        ;     own routines (no-op when growing). Iterate the window Z-stack the
        ;     same way the GUI does (WIN_POS -> slot -> window_data).
        push    esi
        mov     esi, 1
.clamp_next:
        cmp     esi, [thread_count]
        ja      .clamp_done
        movzx   edi, word [WIN_POS + esi*2]
        shl     edi, BSF sizeof.WDATA
        add     edi, window_data
        test    byte [edi + WDATA.fl_wstate], WSTATE_USED
        jz      .clamp_skip
        call    window._.check_window_position   ; clamp box to new screen
        call    window._.set_window_clientbox    ; recompute client area
.clamp_skip:
        inc     esi
        jmp     .clamp_next
.clamp_done:
        pop     esi

        ; 6. Force a full repaint of the background and all windows for the new mode.
        call    force_redraw_background
        xor     eax, eax
        call    redrawscreen

        popad
        pop     ebp
        ret     20

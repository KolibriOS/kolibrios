;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2016. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;; Synhronization for MenuetOS.                                 ;;
;; Author: Halyavin Andrey, halyavin@land.ru                    ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 6240 $

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

        mov     [eax+FRB.magic], 'FRMB'
        mov     [eax+FRB.destroy], 0
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
        mov     [ebp+FRB.width], eax
        mov     [display_width_standard], eax
        dec     eax
        mov     [screen_workarea.right], eax

        movzx   eax, word [BOOT.y_res]; Y max
        cmp     eax, MAX_SCREEN_HEIGHT
        ja      $
        mov     [_display.height], eax
        mov     [ebp+FRB.height], eax
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
        mov     [ebp+FRB.pitch], eax

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
        mov     [PUTPIXEL], Vesa20_putpixel16
        mov     [GETPIXEL], Vesa20_getpixel16
        mov     [_display.bytes_per_pixel], 2
        jmp     .finish

.24bpp:
        mov     [PUTPIXEL], Vesa20_putpixel24
        mov     [GETPIXEL], Vesa20_getpixel24
        mov     [_display.bytes_per_pixel], 3
        jmp     .finish

.32bpp:
        mov     [PUTPIXEL], Vesa20_putpixel32
        mov     [GETPIXEL], Vesa20_getpixel32
        mov     [_display.bytes_per_pixel], 4

.finish:
        mov     [MOUSE_PICTURE], mousepointer
        mov     [_display.check_mouse], check_mouse_area_for_putpixel
        mov     [_display.check_m_pixel], check_mouse_area_for_getpixel

        mov     ax, word [SCR_MODE]
        cmp     ax, 0x0012
        je      .fake
        cmp     ax, 0x0013
        je      .fake

        mov     esi, [LFBAddress]
        bt      [cpu_caps], CAPS_PSE
        jnc     .create_page_tables

        mov     edx, 0x00400000
        or      esi, PG_GLOBAL+PAT_WC+PG_UWR
        and     esi, [pte_valid_mask]
        or      esi, PDE_LARGE
        mov     [ebp+FRB.pde], esi
        add     esi, edx
        mov     [ebp+FRB.pde+4], esi
        add     esi, edx
        mov     [ebp+FRB.pde+8], esi
        add     esi, edx
        mov     [ebp+FRB.pde+12], esi
        add     esi, edx
.ok:
        call    calculate_fast_getting_offset_for_WinMapAddress
; for Qemu or non standart video cards
; Unfortunately [BytesPerScanLine] does not always
; equal to [_display.width] * [ScreenBPP] / 8
        call    calculate_fast_getting_offset_for_LFB
        ret

.create_page_tables:

        add     ebp, FRB.pde
        or      esi, PG_GLOBAL+PAT_WC+PG_UWR
        and     esi, [pte_valid_mask]

        stdcall alloc_kernel_space, 0x1000
        mov     edi, eax
        mov     ebx, 4

.new_pd:
        call    alloc_page
        lea     edx, [eax+PG_UWR]
        mov     [ebp], edx

        stdcall map_page, edi, eax, PG_SWR

        mov     eax, esi
        mov     ecx, 1024
@@:
        stosd
        add     eax, 0x1000
        loop    @B

        add     esi, 0x400000
        add     ebp, 4
        sub     edi, 4096
        dec     ebx
        jnz     .new_pd
        stdcall free_kernel_space, edi
        jmp     .ok

.fake:
        mov     [BOOT.mtrr], byte 2

        stdcall alloc_kernel_space, 0x1000
        push    eax                            ;store in stack for subsequent
        mov     edi, eax                       ;free_kernel_space call

        call    alloc_page
        lea     edx, [eax+PG_UWR]
        mov     [ebp+FRB.pde], edx

        stdcall map_page, edi, eax, PG_SWR

; max VGA=640*480*4=1228800 bytes
; + 32*640*4=81920 bytes for mouse pointer
        stdcall alloc_pages, ((1228800+81920)/4096)
        or      eax, PG_GLOBAL+PG_UWR
        and     eax, [pte_valid_mask]
        mov     ecx, (1228800+81920)/4096
@@:
        stosd
        add     eax, 0x1000
        loop    @B

        call    free_kernel_space
        jmp     .ok

align 4
set_framebuffer:
        push    esi
        push    edi
        lea     esi, [ecx+FRB.pde]
        mov     eax, sys_proc

        cld
        pushfd
        cli
        mov     [_display.current_lfb], ecx
.patch_pde:
        lea     edi, [eax+PROC.pdt_0+4096-32]   ;last 8 pd entries up to 32Mb framebuffer
        mov     ecx, 4
        rep movsd                               ;patch pde
        sub     esi, 16
        mov     eax, [eax+PROC.list.next]       ;next process/address space
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


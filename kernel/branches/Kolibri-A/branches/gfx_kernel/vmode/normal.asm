;-----------------------------------------------------------------------------
;///// PART OF ATi RADEON 9000 DRIVER ////////////////////////////////////////
;-----------------------------------------------------------------------------
; Copyright (c) 2004, mike.dld
; Using BeOS driver - Copyright (c) 2002, Thomas Kurschel
;-----------------------------------------------------------------------------
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
; FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
; DEALINGS IN THE SOFTWARE.
;-----------------------------------------------------------------------------

DRIVER_CODE_START:

;-----------------------------------------------------------------------------

include '../const.inc' ; kernel defines

include 'normal.inc'
include 'clipping.asm'

;-----------------------------------------------------------------------------

struct graph_funcs
 draw_line        dd ?
 disable_mouse    dd ?
 draw_pointer     dd ?
 draw_mouse_under dd ?
 drawbar          dd ?
 putpixel         dd ?
 getpixel         dd ?
 sys_putimage     dd ?
 drawbackground   dd ?
 calculatescreen  dd ?
 setscreen        dd ?
ends

SF graph_funcs

call_cnt dd 0
viewport RECT

set_bank dd set_bank1
setbnk   dd set_bank0,set_bank1,set_s3_bank,set_bank3

virtual at esi
  rr RECT
end virtual

virtual at 0x00010008
  GF graph_funcs
end virtual

;-----------------------------------------------------------------------------

CLIP_RECTS = 0x00720000 ; 0x00780000

;label bg_BytesPerScanLine dword at 0x600000-16;0x460000-16
;label bg_type   dword at 0x600000-12;0x460000-12
;label bg_width  dword at 0x600000-8;0x460000-8
;label bg_height dword at 0x600000-4;0x460000-4

label bg_BytesPerScanLine dword at 0x460000-16
label bg_type   dword at 0x460000-12
label bg_width  dword at 0x460000-8
label bg_height dword at 0x460000-4

BGT_TILE    = 1
BGT_STRETCH = 2

;-----------------------------------------------------------------------------

CRTC_INDX       equ     3D4h

func vm_mike_init
begin
;       jif     dword[mdrvm],e,0,.exit
        mov     eax,[0x00010004]
        jif     eax,e,'ENUE',.exit
        jif     [call_cnt],a,0,.exit.2
        add     eax, 0x00010000
        mov     [systlb],eax

;       SHFLOW  'System funcs table offset: 0x%x',eax

;--------------------------------------

        mov     esi,0x00010008
        mov     edi,SF
        mov     ecx,sizeof.graph_funcs/4
        cld
        rep     movsd

;       push    dword[CURRENT_TASK]
;       mov     dword[CURRENT_TASK],1
;       call    [SF.disable_mouse]
;       pop     dword[CURRENT_TASK]

        mov     [viewport.left],0
        mov     [viewport.top],0
        m2m     [viewport.right],[ScreenWidth]
        m2m     [viewport.bottom],[ScreenHeight]

        movzx   eax,byte[GFX_CARD_VENDOR]
        cmp     byte[VESA_VER_MAJOR],'2'
        jb      @f
        mov     al,0
    @@: mov     eax,[setbnk+eax*4]
        mov     [set_bank],eax

        mov     al,[ScreenBPP]
        cmp     al,32
        jne     @f

;       SHFLOW  'Driver initialized in 32-bit mode'

        mov     [GF.draw_line],vm_mike_draw_line.32
;;      mov     [GF.disable_mouse],vm_mike_disable_mouse.32
;;      mov     [GF.draw_pointer],vm_mike_draw_pointer.32
        mov     [GF.drawbar],vm_mike_draw_rect.32
        mov     [GF.putpixel],vm_mike_put_pixel.32
        mov     [GF.getpixel],vm_mike_get_pixel.32
        mov     [GF.sys_putimage],vm_mike_put_image.32
        mov     [GF.drawbackground],vm_mike_draw_bg.32

        jmp     .exit.2

    @@: cmp     al,24
        jne     @f

;       SHFLOW  'Driver initialized in 24-bit mode'

        mov     [GF.draw_line],vm_mike_draw_line.24
;       mov     [GF.disable_mouse],vm_mike_disable_mouse.24
;       mov     [GF.draw_pointer],vm_mike_draw_pointer.24
        mov     [GF.drawbar],vm_mike_draw_rect.24
        mov     [GF.putpixel],vm_mike_put_pixel.24
        mov     [GF.getpixel],vm_mike_get_pixel.24
        mov     [GF.sys_putimage],vm_mike_put_image.24
        mov     [GF.drawbackground],vm_mike_draw_bg.24

        jmp     .exit.2

    @@: cmp     al,16
        jne     @f

;       SHFLOW  'Driver initialized in 16-bit mode'

        mov     [GF.draw_line],vm_mike_draw_line.16
;       mov     [GF.disable_mouse],vm_mike_disable_mouse.16
;       mov     [GF.draw_pointer],vm_mike_draw_pointer.16
        mov     [GF.drawbar],vm_mike_draw_rect.16
        mov     [GF.putpixel],vm_mike_put_pixel.16
        mov     [GF.getpixel],vm_mike_get_pixel.16
        mov     [GF.sys_putimage],vm_mike_put_image.16
        mov     [GF.drawbackground],vm_mike_draw_bg.16

        jmp     .exit.2

    @@: cmp     al,15
        jne     @f

;       SHFLOW  'Driver initialized in 15-bit mode'

        mov     [GF.draw_line],vm_mike_draw_line.15
;       mov     [GF.disable_mouse],vm_mike_disable_mouse.15
;       mov     [GF.draw_pointer],vm_mike_draw_pointer.15
        mov     [GF.drawbar],vm_mike_draw_rect.15
        mov     [GF.putpixel],vm_mike_put_pixel.15
        mov     [GF.getpixel],vm_mike_get_pixel.15
        mov     [GF.sys_putimage],vm_mike_put_image.15
        mov     [GF.drawbackground],vm_mike_draw_bg.15

        jmp     .exit.2

    @@: cmp     al,8
        jne     @f

;       SHFLOW  'Driver initialized in 8-bit mode'

        call    setup_palette_8

        mov     [GF.draw_line],vm_mike_draw_line.08
;       mov     [GF.disable_mouse],vm_mike_disable_mouse.08
;       mov     [GF.draw_pointer],vm_mike_draw_pointer.08
        mov     [GF.drawbar],vm_mike_draw_rect.08
        mov     [GF.putpixel],vm_mike_put_pixel.08
        mov     [GF.getpixel],vm_mike_get_pixel.08
        mov     [GF.sys_putimage],vm_mike_put_image.08
        mov     [GF.drawbackground],vm_mike_draw_bg.08

        jmp     .exit.2

    @@: cmp     al,4
        jne     .exit

;       SHFLOW  'Driver initialized in 4-bit mode'

        mov     [GF.draw_line],vm_mike_draw_line.04
;       mov     [GF.disable_mouse],vm_mike_disable_mouse.04
;       mov     [GF.draw_pointer],vm_mike_draw_pointer.04
        mov     [GF.drawbar],vm_mike_draw_rect.04
        mov     [GF.putpixel],vm_mike_put_pixel.04
        mov     [GF.getpixel],vm_mike_get_pixel.04
        mov     [GF.sys_putimage],vm_mike_put_image.04
        mov     [GF.drawbackground],vm_mike_draw_bg.04

;--------------------------------------

  .exit.2:

        mov     [GF.calculatescreen],vm_mike_calculatescreen
        mov     [GF.setscreen],vm_mike_setscreen

;!      mov     [GF.disable_mouse],vm_dummy
;!      mov     [GF.draw_pointer],vm_dummy

        inc     [call_cnt]
        xor     eax,eax
  .exit.3:
        retn
  .exit:
        xor     eax,eax
        dec     eax
        retn
endf

func vm_dummy
begin
	ret
endf

func vm_mike_calculatescreen
begin
;       call    [SF.calculatescreen]
  .direct:
        pushad
        cli
        movzx   ecx,word[TASK_COUNT]    ; number of processes
        lea     edi,[CLIP_RECTS+ecx*4+4]
        push    dword[CURRENT_TASK]
        xor     eax,eax
  .next_window:
        inc     eax
        push    ecx ebx eax edi
        mov     [CURRENT_TASK],ax
        call    calc_clipping_rects
        pop     edi eax ebx
        mov     [CLIP_RECTS+eax*4],edi
        mov     [edi],ecx
        add     edi,4
        shl     ecx,2
        rep     movsd
        pop     ecx
        loop    .next_window
        pop     dword[CURRENT_TASK]
        sti
        popad
        ret
endf

func vm_mike_setscreen
begin
;       call    [SF.setscreen]
        call    vm_mike_calculatescreen.direct
        ret
endf

func vm_mike_uninit
begin
;       jif     dword[mdrvm],e,0,.exit
        jif     [call_cnt],nz,,.exit.2,dec
        mov     esi,SF
        mov     edi,GF
        mov     ecx,sizeof.graph_funcs/4
        rep     movsd
  .exit.2:
        xor     eax,eax
        retn
  .exit:
        xor     eax,eax
        dec     eax
        retn
endf

;-----------------------------------------------------------------------------

include 'norm_04.inc'
include 'norm_08.inc'
include 'norm_15.inc'
include 'norm_16.inc'
include 'norm_24.inc'
include 'norm_32.inc'

;-----------------------------------------------------------------------------

func is_intersect_rc
begin
        jif     ecx,l,[tr.left],.exit
        jif     edx,l,[tr.top],.exit
        jif     ebx,ge,[tr.bottom],.exit
        jif     eax,ge,[tr.right],.exit
        clc
        ret
    .exit:
        stc
        ret
endf

func is_intersect_hln
begin
        jif     edx,l,[tr.left],.exit
        jif     eax,l,[tr.top],.exit
        jif     eax,ge,[tr.bottom],.exit
        jif     ebx,ge,[tr.right],.exit
        clc
        ret
    .exit:
        stc
        ret
endf

func is_intersect_vln
begin
        jif     eax,l,[tr.left],.exit
        jif     edx,l,[tr.top],.exit
        jif     ebx,ge,[tr.bottom],.exit
        jif     eax,ge,[tr.right],.exit
        clc
        ret
    .exit:
        stc
        ret
endf

func is_intersect_pt
begin
        jif     eax,l,[tr.left],.exit
        jif     ebx,l,[tr.top],.exit
        jif     ebx,ge,[tr.bottom],.exit
        jif     eax,ge,[tr.right],.exit
        clc
        ret
    .exit:
        stc
        ret
endf

func get_cursor_rect
begin
        push    eax
        movsx   eax,word[MOUSE_X]
        mov     [tr.left],eax
        add     eax,31
        mov     [tr.right],eax
        movsx   eax,word[MOUSE_Y]
        mov     [tr.top],eax
        add     eax,31
        mov     [tr.bottom],eax
        pop     eax
        ret
endf

;-----------------------------------------------------------------------------

gamma_4_0 = 0x00
gamma_4_1 = 0x1F
gamma_4_2 = 0x2F
gamma_4_3 = 0x3F

align 16
palette_8_64  rb 256*3
palette_8_256 rb 256*3

rept 4 red:0
{
\rept 4 green:0
\{
\\rept 4 blue:0
\\{
   index = ((red shl 4) or (green shl 2) or blue)*3
   store gamma_4_  #red   at palette_8_64+index+0
   store gamma_4_ \#green at palette_8_64+index+1
   store gamma_4_\\#blue  at palette_8_64+index+2
   store gamma_4_  #red  *4+red   at palette_8_256+index+0
   store gamma_4_ \#green*4+green at palette_8_256+index+1
   store gamma_4_\\#blue *4+blue  at palette_8_256+index+2
\\}
\}
}

;rept 64 clr1:0
;{
; index = clr1*3+64*3
; clr2 = clr*4+(clr shr 4)
; store clr1 at palette_8_64 +index
; store clr2 at palette_8_256+index
; index = index + 64*3 + 1
; store clr1 at palette_8_64 +index
; store clr2 at palette_8_256+index
; index = index + 64*3 + 1
; store clr1 at palette_8_64 +index
; store clr2 at palette_8_256+index
;}

func setup_palette_8
begin
        mov     edx,0x03C8
        xor     al,al
        out     dx,al
        mov     ecx,256*3
        mov     edx,0x03C9
        mov     esi,palette_8_64
        cld
        rep     outsb
        ret
endf

;-----------------------------------------------------------------------------

func set_bank0
begin
        mov     ebp,[esp+4*1]
        ret
endf


; i810/i815
; by Protopopius
func set_bank1
begin
        cli
        push    eax edx
        mov     eax,[esp+4*3]
        mov     ebp,eax
        shr     eax,16
        sub     al,0x0A
        cmp     al,[BANK_RW]
        je      .exit
        mov     [BANK_RW],al

        mov     dx,3CEh
        mov     ah,al           ; Save value for later use
        mov     al,10h          ; Index GR10 (Address Mapping)
        out     dx,al           ; Select GR10
        inc     dl
        mov     al,3            ; Set bits 0 and 1 (Enable linear page mapping)
        out     dx,al           ; Write value
        dec     dl
        mov     al,11h          ; Index GR11 (Page Selector)
        out     dx,al           ; Select GR11
        inc     dl

        mov     al,ah           ; Write address
        out     dx,al           ; Write the value

  .exit:
        and     ebp,0x0000FFFF
        add     ebp,VGABasePtr
        pop     edx eax
        sti
        ret
endf


; S3
; by kmeaw
func set_bank2
begin
        cli
        push    eax edx ecx
        mov     eax,[esp+4*4]
        mov     ebp,eax
        shr     eax,16
        sub     al,0x0A
        cmp     al,[BANK_RW]
        je      .exit
        mov     [BANK_RW],al

        mov     cl,al
        mov     dx,0x3D4
        mov     al,0x38
        out     dx,al
        inc     dx
        mov     al,0x48
        out     dx,al
        dec     dx
        mov     al,0x31
        out     dx,al
        inc     dx
        in      al,dx
        dec     dx
        mov     ah,al
        mov     al,0x31
        out     dx,ax
        mov     al,ah
        or      al,9
        inc     dx
        out     dx,al
        dec     dx
        mov     al,0x35
        out     dx,al
        inc     dx
        in      al,dx
        dec     dx
        and     al,0xF0
        mov     ch,cl
        and     ch,0x0F
        or      ch,al
        mov     al,0x35
        out     dx,al
        inc     dx
        mov     al,ch
        out     dx,ax
        dec     dx
        mov     al,0x51
        out     dx,al
        inc     dx
        in      al,dx
        dec     dx
        and     al,0xF3
        shr     cl,2
        and     cl,0x0C
        or      cl,al
        mov     al,0x51
        out     dx,al
        inc     dx
        mov     al,cl
        out     dx,al
        dec     dx
        mov     al,0x38
        out     dx,al
        inc     dx
        xor     al,al
        out     dx,al

  .exit:
        and     ebp,0x0000FFFF
        add     ebp,VGABasePtr
        pop     ecx edx eax
        sti
        ret
endf


; from http://my.execpc.com/CE/AC/geezer/os/slfb.asm
func set_s3_bank
begin
        cli
        push    eax edx
        mov     eax,[esp+4*3]
        mov     ebp,eax
        shr     eax,16
        sub     al,0x0A
        cmp     al,[BANK_RW]
        je      .exit
        mov     [BANK_RW],al

        mov ah,al
; grrrr...mode-set locked the S3 registers, so unlock them again
; xxx - do this after mode-set
        mov dx,CRTC_INDX
        mov al,38h
        out dx,al
        inc edx
        mov al,48h
        out dx,al
        dec edx
        mov al,39h
        out dx,al
        inc edx
        mov al,0A5h
        out dx,al
; now: do the bank-switch
        mov dx,CRTC_INDX
        mov al,35h
        out dx,al
        inc edx
        in al,dx
        and al,0F0h
        or al,ah
        out dx,al

  .exit:
        and     ebp,0x0000FFFF
        add     ebp,VGABasePtr
        pop     edx eax
        sti
        ret
endf


func set_bank3
begin
        cli
        push    eax edx
        mov     eax,[esp+4*3]
        mov     ebp,eax
        shr     eax,16
        sub     al,0x0A
        cmp     al,[BANK_RW]
        je      .exit
        mov     [BANK_RW],al

        mov     ah,al
        mov     dx,0x03D4
        mov     al,0x39
        out     dx,al
        inc     dl
        mov     al,0xA5
        out     dx,al
        dec     dl
        mov     al,6Ah
        out     dx,al
        inc     dl
        mov     al,ah
        out     dx,al
        dec     dl
        mov     al,0x39
        out     dx,al
        inc     dl
        mov     al,0x5A
        out     dx,al
        dec     dl

  .exit:
        and     ebp,0x0000FFFF
        add     ebp,VGABasePtr
        pop     edx eax
        sti
        ret
endf

;-----------------------------------------------------------------------------

DRIVER_CODE_END:

diff10 'driver code size',DRIVER_CODE_START,DRIVER_CODE_END

;-----------------------------------------------------------------------------
;///// END ///////////////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
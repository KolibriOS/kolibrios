
struc VBE_VGAInfo {
  .VESASignature          dd ?    ; char
  .VESAVersion            dw ?    ; short
  .OemStringPtr           dd ?    ; char *
  .Capabilities           dd ?    ; ulong
  .VideoModePtr           dd ?    ; ulong
  .TotalMemory            dw ?    ; short
  ; VBE 2.0+
  .OemSoftwareRev         db ?    ; short
  .OemVendorNamePtr       dw ?    ; char *
  .OemProductNamePtr      dw ?    ; char *
  .OemProductRevPtr       dw ?    ; char *
  .reserved               rb 222  ; char
  .OemData                rb 256  ; char
}

struc VBE_ModeInfo {
  .ModeAttributes         dw ?    ; short
  .WinAAttributes         db ?    ; char
  .WinBAttributes         db ?    ; char
  .WinGranularity         dw ?    ; short
  .WinSize                dw ?    ; short
  .WinASegment            dw ?    ; ushort
  .WinBSegment            dw ?    ; ushort
  .WinFuncPtr             dd ?    ; void *
  .BytesPerScanLine       dw ?    ; short
  .XRes                   dw ?    ; short
  .YRes                   dw ?    ; short
  .XCharSize              db ?    ; char
  .YCharSize              db ?    ; char
  .NumberOfPlanes         db ?    ; char
  .BitsPerPixel           db ?    ; char
  .NumberOfBanks          db ?    ; char
  .MemoryModel            db ?    ; char
  .BankSize               db ?    ; char
  .NumberOfImagePages     db ?    ; char
  .res1                   db ?    ; char
  .RedMaskSize            db ?    ; char
  .RedFieldPosition       db ?    ; char
  .GreenMaskSize          db ?    ; char
  .GreenFieldPosition     db ?    ; char
  .BlueMaskSize           db ?    ; char
  .BlueFieldPosition      db ?    ; char
  .RsvedMaskSize          db ?    ; char
  .RsvedFieldPosition     db ?    ; char
  .DirectColorModeInfo    db ?    ; char ; MISSED IN THIS TUTORIAL!! SEE ABOVE
  ; VBE 2.0+
  .PhysBasePtr            dd ?    ; ulong
  .OffScreenMemOffset     dd ?    ; ulong
  .OffScreenMemSize       dw ?    ; short
  ; VBE 3.0+
  .LinbytesPerScanLine    dw ?    ; short
  .BankNumberOfImagePages db ?    ; char
  .LinNumberOfImagePages  db ?    ; char
  .LinRedMaskSize         db ?    ; char
  .LinRedFieldPosition    db ?    ; char
  .LingreenMaskSize       db ?    ; char
  .LinGreenFieldPosition  db ?    ; char
  .LinBlueMaskSize        db ?    ; char
  .LinBlueFieldPosition   db ?    ; char
  .LinRsvdMaskSize        db ?    ; char
  .LinRsvdFieldPosition   db ?    ; char
  .MaxPixelClock          dd ?    ; ulong
  .res2                   rb 190  ; char
}

virtual at $A000
  vi VBE_VGAInfo
  mi VBE_ModeInfo
  modes_table:
end virtual

ln_top db 1
ln_num db 6
ln_cnt db ?
db ?

ln_top2 db ?
ln_num2 db ?
ln_cnt2 db ?
db ?

vm_row db 2
vm_col db 2

print_vesa_info:
        push    0
        pop     es

        _setcursor 3,2

        mov     [es:vi.VESASignature],'VBE2'
        mov     ax,$4F00
        mov     di,vi
        int     $10
        or      ah,ah
        jz      @f
        mov     [es:vi.VESASignature],'VESA'
        mov     ax,$4F00
        mov     di,vi
        int     $10
        or      ah,ah
        jnz     .exit
  @@:
        cmp     [es:vi.VESASignature],'VESA'
        jne     .exit
        cmp     [es:vi.VESAVersion],$0100
        jb      .exit
        jmp     .vesaok2

  .exit:
        ;pushad
        push    0
        pop     es
        mov     si,novesa-0x10000
        call    printplain
        ret
        ;pushw   'bb'
        ;jmp     enum_modes.modes_ok

  .vesaok2:
        mov     ecx,0x00200000
        mov     ax,[es:vi.TotalMemory]
        shr     ax,4
        mov     bl,10
        div     bl
        mov     ch,ah
        and     ax,0x00FF
        div     bl
        shl     ecx,8
        mov     cx,ax
        add     ecx,'000'

        mov     ax,[es:vi.VESAVersion]
        add     ax,'00'

        push    0x1000
        pop     es
        mov     [es:s_vesa.ver-0x10000], ah
        mov     [es:s_vesa.ver+2-0x10000], al
        mov     dword[es:s_vesa.mem-0x10000],ecx
        mov     si,s_vesa-0x10000
        call    printplain

if 0 ; mm
        _setcursor 5,2
        mov     si,_oem-0x10000
        call    printplain
end if ; mm

        push    0
        pop     es
        mov     si,word[es:vi.OemStringPtr]
        mov     di,si
        push    word[es:vi.OemStringPtr+2]
        pop     es
if 0 ; mm
;       mov     al,0x0A
        pop     es
        mov     ax,es
        mov     ds,ax
        call    printplain
        pop     es
end if ; mm
        push    0x1000
        pop     ds
        mov     si,s_ven_intel-0x10000
        call    substr
        mov     al,1
        je      .vendor_ok
        mov     si,s_ven_s3-0x10000
        call    substr
        mov     al,2
        je      .vendor_ok
        mov     al,3
  .vendor_ok:
        push    0 0x1000
        pop     ds es
        mov     [es:0x9035],al
        ret

;-----------------------------------------------------------------------------

calc_vmodes_table:
        push    0 0x1000
        pop     ds es
        ;movzx   eax,byte [es:preboot_graph-0x10000]
        ;cmp     eax,0
        ;jne     pre_graph
        ;_setcursor 9,2
        ;mov     si,gr_mode-0x10000
        ;call    printplain
  .enum_modes:
        pushad
        push    0x0000
        pop     es
        ;mov     si,s_mode1-0x10000
        ;call    printplain
        mov     si,word[es:vi.VideoModePtr]
        push    word[es:vi.VideoModePtr+2]
        pop     fs
        ;pushw   1

        mov     bx,modes_table;-0x10000
  .next_mode:
        mov     cx,[fs:si] ; mode number
        cmp     cx,-1
        je      .modes_ok.2

        mov     ax,$4F01
        mov     di,mi
        int     $10

        or      ah,ah
        jnz     .modes_ok.2;vesa_info.exit
        test    [es:mi.ModeAttributes],00000001b
        jz      @f
        test    [es:mi.ModeAttributes],00010000b
        jz      @f

        cmp     [es:mi.BitsPerPixel],16
        jne     .l0
        cmp     [es:mi.GreenMaskSize],5
        jne     .l0
        mov     [es:mi.BitsPerPixel],15
  .l0:

        cmp     [es:mi.XRes],640
        jb      @f
        cmp     [es:mi.YRes],480
        jb      @f
        cmp     [es:mi.BitsPerPixel],15
        jb      @f

        mov     ax,[es:mi.XRes]
        mov     [es:bx+0],ax               ; +0[2] : resolution X
        mov     ax,[es:mi.YRes]
        mov     [es:bx+2],ax               ; +2[2] : resolution Y
        mov     ax,[es:mi.ModeAttributes]
        mov     [es:bx+4],ax               ; +4[2] : attributes
        cmp     [s_vesa.ver-0x10000],'2'
        jb      .lp1
        or      cx,0x4000  ; use LFB
  .lp1: mov     [es:bx+6],cx               ; +6[2] : mode number
        mov     al,[es:mi.BitsPerPixel]
        mov     [es:bx+8],al               ; +8[1] : bits per pixel

        add     bx,10

    @@:
        add     si,2
        jmp     .next_mode

  .modes_ok.2:

        mov     si,modes_table;-0x10000;m_info

    @@: add     si,10
        cmp     si,bx
        jae     @f
        mov     ax,[es:si+0-10]
        cmp     ax,[es:si+0]
        ja      .xch
        jb      @b
        mov     ax,[es:si+2-10]
        cmp     ax,[es:si+2]
        ja      .xch
        jb      @b
        mov     al,[es:si+8-10]
        cmp     al,[es:si+8]
        ja      .xch
        jb      @b
        pushad
  .del: push    dword[es:si+0] dword[es:si+4]  word[es:si+8] dword[es:si+0+10] dword[es:si+4+10]  word[es:si+8+10]
        pop      word[es:si+8] dword[es:si+4] dword[es:si+0]  word[es:si+8+10] dword[es:si+4+10] dword[es:si+0+10]
        add     si,10
        cmp     si,bx
        jb      .del
        popad
        mov     si,modes_table
        sub     bx,10
        jmp     @b
  .xch: push    dword[es:si+0] dword[es:si+4]  word[es:si+8] dword[es:si+0-10] dword[es:si+4-10]  word[es:si+8-10]
        pop      word[es:si+8] dword[es:si+4] dword[es:si+0]  word[es:si+8-10] dword[es:si+4-10] dword[es:si+0-10]
        mov     si,modes_table;-0x10000;m_info
        jmp     @b

    @@:

        push    0
        pop     es
        mov     word[es:bx],-1
        mov     si,modes_table
        mov     di,0x0AC00
        pushw   [es:si+2] [es:si+0]
        mov     bp,sp
        mov     [ds:ln_cnt],0
        mov     word[es:si-10],0
        jmp     .wr.2

    @@:
        cmp     word[es:si-10],-1
        je      @f

        mov     ax,[es:si+0]
        cmp     ax,[ss:bp+0]
        jne     .wr
        mov     ax,[es:si+2]
        cmp     ax,[ss:bp+2]
        jne     .wr

  .ww:  mov     al,[es:si+8]
        mov     bx,4
        cmp     al,4
        je      .l1
        add     bx,4
        cmp     al,8
        je      .l1
        add     bx,4
        cmp     al,15
        je      .l1
        add     bx,4
        cmp     al,16
        je      .l1
        add     bx,4
        cmp     al,24
        je      .l1
        add     bx,4
  .l1:  mov     eax,[es:si+4]      ; mode attributes & number
        mov     [es:di+bx],eax

        add     si,10
        jmp     @b

  .wr:
        pushw   [ss:bp+0] [ss:bp+2]
        popw    [es:di+2] [es:di+0]
        add     di,4+6*4
        inc     [ds:ln_cnt]
        pushw   [es:si+0] [es:si+2]
        popw    [ss:bp+2] [ss:bp+0]
  .wr.2:
        mov     cx,(4+6*4)/2
        xor     ax,ax
        push    di
        cld
        rep     stosw
        pop     di
        jmp     .ww

    @@: add     sp,4
        popad
        ret

;-----------------------------------------------------------------------------

vm_bpp db 4,8,15,16,24,32

draw_current_vmode:
        push    es ds 0 0x1000
        pop     es ds
        movzx   ax,[es:vm_row]
        mov     dx,4+6*4
        mul     dx
        mov     si,ax
        add     si,0x0AC00
        mov     di,0x0A000
        movzx   eax,word[si+0]
        mov     ecx,10
        call    int2strnz
        mov     byte[ds:di],'x'
        inc     di
        movzx   eax,word[si+2]
        call    int2strnz
        mov     byte[ds:di],'x'
        inc     di
        movzx   bx,[es:vm_col]
        movzx   eax,[es:bx+vm_bpp]
        call    int2strnz
        mov     byte[ds:di],0
        mov     si,0x0A000
        call    printplain
        pop     ds es
        ret

;-----------------------------------------------------------------------------

draw_vmodes_table:
        _setcursor 9,2
        mov     si,gr_mode-0x10000
        call    printplain
        ;mov    ah,9
        ;mov    dx,_tl
        ;int    $21
        mov     si,_tl-0x10000
        call    printplain
        push    0
        pop     es
        movzx   ax,[ds:ln_top]
        mov     dx,4+6*4
        mul     dx
        mov     si,ax
        add     si,0x0AC00

        mov     eax,dword[ds:ln_top]
        mov     dword[ds:ln_top2],eax ; top+num+cnt

        movzx   cx,[ds:ln_num2]
    @@: push    cx

        movzx   eax,word[es:si+0]
        mov     di,_rs-0x10000+2+14
        mov     ecx,10
        mov     bl,4
        call    int2str
        movzx   eax,word[es:si+2]
        inc     di
        mov     bl,4
        call    int2str

        add     si,4
        add     di,4
        mov     cx,6
  .lp0: mov     al,' '
        cmp     word[es:si],0
        je      .lp1
        mov     al,0xFB ; check
  .lp1: mov     [ds:di],al
        add     si,4
        add     di,6
        loop    .lp0

        push    si
        mov     si,_rs-0x10000
        call    printplain
        pop     si

        pop     cx
        loop    @b

        mov     si,_bt-0x10000
        call    printplain
        ret

;-----------------------------------------------------------------------------

draw_vmodes_table_cursor: ; 9+5
        mov     al,[vm_row]
        sub     al,[ln_top]
        add     al,13
        mov     dh,al
        mov     al,[vm_col]
        mov     dl,6
        mul     dl
        add     al,28
        mov     dl,al
        call    setcursor
        mov     si,_sel1-0x10000
        mov     [si],ch
        call    printplain
        add     dl,2
        call    setcursor
        mov     si,_sel2-0x10000
        mov     [si],cl
        call    printplain
        dec     dl
        call    setcursor
        ret

;-----------------------------------------------------------------------------

macro qwe
{
  pushad
  pushfd
  push es ds 0 0x1000
  pop ds es
  mov si,badsect-2-0x10000
  call printplain
  pop ds es
  popfd
  popad
}

set_vmode:
        push    0 0x1000
        pop     ds es
        movzx   ax,[ds:vm_row]
        mov     dx,4+6*4
        mul     dx
        add     ax,0x0AC00
        mov     si,ax
        mov     cx,[es:si+0]            ; resolution X
        mov     dx,[es:si+2]            ; resolution Y
        movzx   bx,[ds:vm_col]
        shl     bx,2
        mov     bx,[es:si+4+bx+2]         ; mode number

;        mov     bx,0x4118
;        mov     cx,1024
;        mov     dx,768
;        mov     byte[es:0x9000],24

        cmp     bx,0x12
        je      .gml00
        cmp     bx,0x13
        je      .sgml1

        movzx   bp,[ds:vm_col]
        mov     al,[ds:bp+vm_bpp]
        mov     [es:0x9000],al
        jmp     .gml10

       .sgml1:
        mov     [es:0x9000],byte 32
        jmp     .gml10

       .gml00:
        mov     [es:0x9000],byte 4

    .gml10:
        mov     [es:0x9008],bx
        mov     [es:0x900A],cx
        mov     [es:0x900C],dx
        mov     al,[s_vesa.ver-0x10000]
        mov     [es:0x9034],al
;       push    0x1000
;       pop     es
        mov     [s_bpp.bpp-0x10000],'8 '
        cmp     bx,0x13
        je      .bppl;nov
        mov     [s_bpp.bpp-0x10000],'4 '
        cmp     bx,0x12
        je      .bppl;nov

; bx - mode : cx - x size : dx - y size

; FIND VESA 2.0 LFB & BPP

        mov     ax,0x4f01
        mov     cx,bx
        and     cx,0xfff
        push    0x0000
        pop     es
        mov     di,mi;0xa000
        int     0x10
        ; LFB
        mov     eax,[es:mi.PhysBasePtr];di+0x28]
        mov     [es:0x9018],eax
        ; BPP
;        cmp     [es:mi.BitsPerPixel],16
;        jne     .l0
;        cmp     [es:mi.GreenMaskSize],5
;        jne     .l0
;        mov     [es:mi.BitsPerPixel],15
;  .l0:  mov     al,[es:mi.BitsPerPixel];di+0x19]
;        mov     [es:0x9000],al
        ; ---- vbe voodoo
        BytesPerScanLine equ 0x10
        push ax
        mov ax, [es:mi.BytesPerScanLine];di+BytesPerScanLine]
        mov [es:0x9001],ax
        pop ax
        ; -----
    .nov:
        mov     al,[es:0x9000]

        cmp     al,15
        jnz     .nbpp15
        mov     [s_bpp.bpp-0x10000],'15'
        jmp     .bppl
    .nbpp15:
        cmp     al,16
        jnz     .nbpp16
        mov     [s_bpp.bpp-0x10000],'16'
        jmp     .bppl
    .nbpp16:
        cmp     al,24
        jnz     .nbpp24
        mov     [s_bpp.bpp-0x10000],'24'
        jmp     .bppl
    .nbpp24:
        cmp     al,32
        jnz     .nbpp32
        mov     [s_bpp.bpp-0x10000],'32'
        jmp     .bppl
    .nbpp32:
        mov     [s_bpp.bpp-0x10000],'??'
        mov     si,s_bpp-0x10000
        call    printplain
        jmp     $
    .bppl:
        mov     si,s_bpp-0x10000
        call    printplain


; FIND VESA 1.2 PM BANK SWITCH ADDRESS

        cmp     [s_vesa.ver-0x10000],'2'
        jb      @f

        mov     ax,0x4f0A
        mov     bx,0x0
        int     0x10
        xor     eax,eax
        xor     ebx,ebx
        mov     ax,es
        shl     eax,4
        mov     bx,di
        add     eax,ebx
        movzx   ebx,word[es:di]
        add     eax,ebx
        push    0x0000
        pop     es
        mov     [es:0x9014],eax
        jmp     .lp1
    @@:

        mov     dword[es:0x9018],0x000A0000
  .lp1:

        ret

;=============================================================================
;=============================================================================
;=============================================================================

; DISPLAY VESA INFORMATION
vesa_info:
macro sdfawgsgwaew {
        push    0
        pop     es

        mov     [es:vi.VESASignature],'VBE2'
        mov     ax,$4F00
        mov     di,vi
        int     $10
        or      ah,ah
        jz      @f
        mov     [es:vi.VESASignature],'VESA'
        mov     ax,$4F00
        mov     di,vi
        int     $10
        or      ah,ah
        jnz     .exit
  @@:
        cmp     [es:vi.VESASignature],'VESA'
        jne     .exit
        cmp     [es:vi.VESAVersion],$0100
        jb      .exit
        jmp     vesaok2

  .exit:
        pushad
        push    0
        pop     es
        mov     si,s_mode1-0x10000
        call    printplain
;       mov     si,word[es:vi.VideoModePtr]
;       push    word[es:vi.VideoModePtr+2]
;       pop     fs
        pushw   'bb'
        jmp     enum_modes.modes_ok
;       push    0x1000
;       pop     es
;       mov     si,novesa-0x10000
;       call    print
;       mov     ax,16
;       jmp     $;novesafound

  vesaok2:
        mov     ecx,0x00200000
        mov     ax,[es:vi.TotalMemory]
        shr     ax,4
        mov     bl,10
        div     bl
        mov     ch,ah
        and     ax,0x00FF
        div     bl
        shl     ecx,8
        mov     cx,ax
        add     ecx,'000'

        mov     ax,[es:vi.VESAVersion]
        add     ax,'00'

        push    0x1000
        pop     es
        mov     [es:s_vesa.ver-0x10000], ah
        mov     [es:s_vesa.ver+2-0x10000], al
        mov     dword[es:s_vesa.mem-0x10000],ecx
        mov     si,s_vesa-0x10000
        call    print

;// mike.dld, 2005-08-20 [
        mov     si,_oem-0x10000
        call    print

        push    0
        pop     es
        mov     si,word[es:vi.OemStringPtr]
        mov     di,si
        push    word[es:vi.OemStringPtr+2]
        pop     es
        mov     al,0x0A
        push    es
        call    draw_text
        pop     es

        push    0x1000
        pop     ds
        mov     si,s_ven_intel-0x10000
        call    substr
        mov     al,1
        je      vendor_ok
        mov     si,s_ven_s3-0x10000
        call    substr
        mov     al,2
        je      vendor_ok
        mov     al,3
  vendor_ok:
        push    0 0x1000
        pop     ds es
        mov     [es:0x9035],al
;// mike.dld, 2005-08-20 ]

      novesafound:
;!!     call    setbase1000

; ASK GRAPHICS MODE

        movzx   eax,byte [es:preboot_graph-0x10000]
        cmp     eax,0
        jne     pre_graph
        mov     si,gr_mode-0x10000
        call    printplain
  enum_modes:
        pushad
        push    0x0000
        pop     es
        mov     si,s_mode1-0x10000
        call    printplain
        mov     si,word[es:vi.VideoModePtr]
        push    word[es:vi.VideoModePtr+2]
        pop     fs
        pushw   1

        mov     bx,modes_table;-0x10000
  .next_mode:
        mov     cx,[fs:si] ; mode number
        cmp     cx,-1
        je      .modes_ok.2

        mov     ax,$4F01
        mov     di,mi
        int     $10

        or      ah,ah
        jnz     .modes_ok.2;vesa_info.exit
        test    [es:mi.ModeAttributes],00000001b
        jz      @f
        test    [es:mi.ModeAttributes],00010000b
        jz      @f

        cmp     [es:mi.BitsPerPixel],16
        jne     .l0
        cmp     [es:mi.GreenMaskSize],5
        jne     .l0
        mov     [es:mi.BitsPerPixel],15
  .l0:

        cmp     [es:mi.XRes],640
        jb      @f
        cmp     [es:mi.YRes],480
        jb      @f
        cmp     [es:mi.BitsPerPixel],15
        jb      @f

        mov     ax,[es:mi.XRes]
        mov     [es:bx+0],ax
        mov     ax,[es:mi.YRes]
        mov     [es:bx+2],ax
        mov     ax,[es:mi.ModeAttributes]
        mov     [es:bx+4],ax
        cmp     [s_vesa.ver-0x10000],'2'
        jb      .lp1
        or      cx,0x4000  ; use LFB
  .lp1: mov     [es:bx+6],cx
        mov     al,[es:mi.BitsPerPixel]
        mov     [es:bx+8],al

        add     bx,10

    @@:
        add     si,2
        jmp     .next_mode

  .modes_ok.2:

        mov     si,modes_table;-0x10000;m_info

    @@: add     si,10
        cmp     si,bx
        jae     @f
        mov     ax,[es:si+0-10]
        cmp     ax,[es:si+0]
        ja      .xch
        jb      @b
        mov     ax,[es:si+2-10]
        cmp     ax,[es:si+2]
        ja      .xch
        jb      @b
        mov     al,[es:si+8-10]
        cmp     al,[es:si+8]
        ja      .xch
        jb      @b
        pushad
  .del: push    dword[es:si+0] dword[es:si+4]  word[es:si+8] dword[es:si+0+10] dword[es:si+4+10]  word[es:si+8+10]
        pop      word[es:si+8] dword[es:si+4] dword[es:si+0]  word[es:si+8+10] dword[es:si+4+10] dword[es:si+0+10]
        add     si,10
        cmp     si,bx
        jb      .del
        popad
        mov     si,modes_table
        sub     bx,10
        jmp     @b
  .xch: push    dword[es:si+0] dword[es:si+4]  word[es:si+8] dword[es:si+0-10] dword[es:si+4-10]  word[es:si+8-10]
        pop      word[es:si+8] dword[es:si+4] dword[es:si+0]  word[es:si+8-10] dword[es:si+4-10] dword[es:si+0-10]
        mov     si,modes_table;-0x10000;m_info
        jmp     @b

    @@: ;mov    ah,9
        ;mov    dx,_tl
        ;int    $21
        mov     si,_tl-0x10000
        push    bx
        call    printplain
        pop     bx
        push    0
        pop     es
        mov     word[es:bx],-1
        mov     si,modes_table;-0x10000;m_info
        pushw   [es:si+0] [es:si+2] 'c'
        mov     bp,sp

    @@:
        cmp     word[es:si-10],-1
        je      @f
;       cmp     si,bx
;       jae     @f
        mov     ax,[es:si+0]
        cmp     ax,[ss:bp+4]
        jne     .wr
        mov     ax,[es:si+2]
        cmp     ax,[ss:bp+2]
        jne     .wr

  .ww:  push    bx
        movzx   eax,word[es:si+0]
        mov     di,_rs-0x10000+2
        mov     ecx,10
        mov     bl,4
        call    int2str
        movzx   eax,word[es:si+2]
        inc     di
        mov     bl,4
        call    int2str

        mov     al,[es:si+8]
        mov     di,_rs-0x10000+16
        cmp     al,4
        je      .l1
        add     di,5
        cmp     al,8
        je      .l1
        add     di,5
        cmp     al,15
        je      .l1
        add     di,5
        cmp     al,16
        je      .l1
        add     di,5
        cmp     al,24
        je      .l1
        add     di,5
  .l1:  mov     al,[ss:bp]
        cmp     byte[ds:di],'-'
        jne     .lp0
        mov     [ds:di],al
        inc     byte[ss:bp]
        jmp     .lp2
  .lp0:
  .lp2:
        pop     bx
        add     si,10
        jmp     @b;.sk

  .wr:  ;mov    ah,9
        ;mov    dx,_rs
        ;int    $21
        push    bx si
        mov     si,_rs-0x10000
        call    printplain
        pop     si bx

        mov     byte[ds:_rs-0x10000+16+5*0],'-'
        mov     byte[ds:_rs-0x10000+16+5*1],'-'
        mov     byte[ds:_rs-0x10000+16+5*2],'-'
        mov     byte[ds:_rs-0x10000+16+5*3],'-'
        mov     byte[ds:_rs-0x10000+16+5*4],'-'
        mov     byte[ds:_rs-0x10000+16+5*5],'-'

        pushw   [es:si+0] [es:si+2]
        popw    [ss:bp+2] [ss:bp+4]

  .sk:
        jmp     .ww;@b

    @@: ;add    sp,6

        pop     bx
        add     sp,6
        sub     bl,2
        shl     bx,8
        push    bx
;       mov     ah,9
;       mov     dx,_bt
;       int     $21
        mov     si,_bt-0x10000
        call    printplain

  .modes_ok:
        pop     bx
        mov     bp,sp
        mov     [ss:bp+4*4],bx
        popad

       gml0:
        mov     bl,'a'
        call    getkey ; !! getchar
       pre_graph:
        or      al,al
        jz      gml00
        cmp     al,1
        je      sgml1
        ;shl    ax,4
        sub     ax,2
        mov     si,10
        mul     si
        add     ax,modes_table;-0x10000
        mov     si,ax
        mov     bx,[es:si+6]
        mov     cx,[es:si+0]
        mov     dx,[es:si+2]
        jmp     gml10
       sgml1:
;        cmp     al,9
;        jnz     gml00
        mov     bx,0x13
        mov     cx,640
        mov     dx,480
        push    0x0
        pop     es
        mov     [es:0x9000],byte 32
;        push    0x1000
;        pop     es
        jmp     gml10
       gml00:
;        cmp     al,0xa
;        jnz     gml02
        mov     bx,0x12
        mov     cx,640
        mov     dx,480
        push    0x0
        pop     es
        mov     [es:0x9000],byte 4;32

    gml10:
        push    0x0000
        pop     es
        mov     [es:0x9008],bx
        mov     [es:0x900A],cx
        mov     [es:0x900C],dx
        mov     al,[s_vesa.ver-0x10000]
        mov     [es:0x9034],al
        push    0x1000
        pop     es
        mov     [s_bpp.bpp-0x10000],'8 '
        cmp     bx,0x13
        je      .bppl;nov
        mov     [s_bpp.bpp-0x10000],'4 '
        cmp     bx,0x12
        je      .bppl;nov

; bx - mode : cx - x size : dx - y size

; FIND VESA 2.0 LFB & BPP

        mov     ax,0x4f01
        mov     cx,bx
        and     cx,0xfff
        push    0x0000
        pop     es
        mov     di,mi;0xa000
        int     0x10
        ; LFB
        mov     eax,[es:mi.PhysBasePtr];di+0x28]
        mov     [es:0x9018],eax
        ; BPP
        cmp     [es:mi.BitsPerPixel],16
        jne     .l0
        cmp     [es:mi.GreenMaskSize],5
        jne     .l0
        mov     [es:mi.BitsPerPixel],15
  .l0:  mov     al,[es:mi.BitsPerPixel];di+0x19]
        mov     [es:0x9000],al
        ; ---- vbe voodoo
        BytesPerScanLine equ 0x10
        push ax
        mov ax, [es:mi.BytesPerScanLine];di+BytesPerScanLine]
        mov [es:0x9001],ax
        pop ax
        ; -----
    .nov:
        cmp     al,15
        jnz     .nbpp15
        mov     [s_bpp.bpp-0x10000],'15'
        jmp     .bppl
    .nbpp15:
        cmp     al,16
        jnz     .nbpp16
        mov     [s_bpp.bpp-0x10000],'16'
        jmp     .bppl
    .nbpp16:
        cmp     al,24
        jnz     .nbpp24
        mov     [s_bpp.bpp-0x10000],'24'
        jmp     .bppl
    .nbpp24:
        cmp     al,32
        jnz     .nbpp32
        mov     [s_bpp.bpp-0x10000],'32'
        jmp     .bppl
    .nbpp32:
        mov     [s_bpp.bpp-0x10000],'??'
        call    print
        jmp     $
    .bppl:
        mov     si,s_bpp-0x10000
        call    printplain


; FIND VESA 1.2 PM BANK SWITCH ADDRESS

        cmp     [s_vesa.ver-0x10000],'2'
        jb      @f

        mov     ax,0x4f0A
        mov     bx,0x0
        int     0x10
        xor     eax,eax
        xor     ebx,ebx
        mov     ax,es
        shl     eax,4
        mov     bx,di
        add     eax,ebx
        movzx   ebx,word[es:di]
        add     eax,ebx
        push    0x0000
        pop     es
        mov     [es:0x9014],eax
        jmp     .lp1
    @@:

        mov     dword[es:0x9018],0x000A0000
  .lp1:
        push    0x1000
        pop     es
}

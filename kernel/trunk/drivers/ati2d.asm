;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format MS COFF

DEBUG           equ 1

include 'proc32.inc'
include 'imports.inc'

R500_HW2D       equ 0

API_VERSION     equ 0x01000100

STRIDE          equ 8

VID_ATI         equ 0x1002

LOAD_FROM_FILE  equ 0
LOAD_FROM_MEM   equ 1
LOAD_INDIRECT   equ 2
LOAD_SYSTEM     equ 3

SRV_GETVERSION  equ 0

struc BITMAPINFOHEADER {
  .biSize          dd ? ; DWORD
  .biWidth         dd ? ; LONG
  .biHeight        dd ? ; LONG
  .biPlanes        dw ? ; WORD
  .biBitCount      dw ? ; WORD
  .biCompression   dd ? ; DWORD
  .biSizeImage     dd ? ; DWORD
  .biXPelsPerMeter dd ? ; LONG
  .biYPelsPerMeter dd ? ; LONG
  .biClrUsed       dd ? ; DWORD
  .biClrImportant  dd ? ; DWORD
}

virtual at 0
  BI BITMAPINFOHEADER
end virtual

struc CURSOR
{;common object header
   .magic       dd ?   ;'CURS'
   .destroy     dd ?   ;internal destructor
   .fd          dd ?   ;next object in list
   .bk          dd ?   ;prev object in list
   .pid         dd ?   ;owner id

 ;cursor data
   .base        dd ?   ;allocated memory
   .hot_x       dd ?   ;hotspot coords
   .hot_y       dd ?
}
virtual at 0
  CURSOR CURSOR
end virtual

CURSOR_SIZE      equ 32

OS_BASE          equ 0x80000000
SLOT_BASE        equ (OS_BASE+0x0080000)
LFB_BASE         equ 0xFE000000

PG_SW            equ 0x003
PG_NOCACHE       equ 0x018

PCI_MEMORY_MASK  equ 0xfffffff0

struc IOCTL
{  .handle           dd ?
   .io_code          dd ?
   .input            dd ?
   .inp_size         dd ?
   .output           dd ?
   .out_size         dd ?
}

virtual at 0
  IOCTL IOCTL
end virtual

;MMIO                   equ 0F9000000h
RD_RB3D_CNTL               equ 1c3ch

RD_MEM_CNTL                equ 0140h
RD_CRTC_GEN_CNTL           equ 0050h
RD_CRTC_CUR_EN             equ 10000h
RD_DISPLAY_BASE_ADDR       equ 023ch
RD_DEFAULT_OFFSET          equ 16e0h

CUR_HORZ_VERT_OFF          equ 0268h
CUR_HORZ_VERT_POSN         equ 0264h
CUR_OFFSET                 equ 0260h


RD_RB3D_CNTL               equ 1c3ch
RD_RBBM_STATUS             equ 0e40h
RD_RBBM_FIFOCNT_MASK       equ 007fh
RD_RBBM_ACTIVE             equ 80000000h
RD_TIMEOUT                 equ 2000000

RD_DP_GUI_MASTER_CNTL      equ 0146ch
RD_DP_BRUSH_BKGD_CLR       equ 01478h
RD_DP_BRUSH_FRGD_CLR       equ 0147ch
RD_DP_SRC_BKGD_CLR         equ 015dch
RD_DP_SRC_FRGD_CLR         equ 015d8h
RD_DP_CNTL                 equ 016c0h
RD_DP_DATATYPE             equ 016c4h
RD_DP_WRITE_MASK           equ 016cch
RD_DP_SRC_SOURCE_MEMORY    equ (2 shl 24)
RD_DP_SRC_SOURCE_HOST_DATA equ (3 shl 24)
RD_DEFAULT_SC_BOTTOM_RIGHT equ 16e8h
RD_GMC_BRUSH_SOLID_COLOR   equ (13 shl 4)
RD_DEFAULT_SC_RIGHT_MAX    equ 1fffh
RD_DEFAULT_SC_BOTTOM_MAX   equ 1fff0000h
RD_GMC_DST_DATATYPE_SHIFT  equ 8

RD_ROP3_S                  equ 00cc0000h
RD_ROP3_P                  equ 00f00000h

RD_RB2D_DSTCACHE_MODE      equ 03428h
RD_RB2D_DSTCACHE_CTLSTAT   equ 0342ch
RD_RB2D_DC_FLUSH_ALL       equ 000fh
RD_RB2D_DC_BUSY            equ 80000000h

RD_GMC_BRUSH_SOLID_COLOR   equ 000000D0h
RD_GMC_SRC_DATATYPE_COLOR  equ (3 shl 12)
RD_GMC_CLR_CMP_CNTL_DIS    equ (1 shl 28)
RD_GMC_WR_MSK_DIS          equ (1 shl 30)

cmdSolidFill               equ 73f036d0h

RD_DST_PITCH_OFFSET        equ 142ch
RD_SRC_PITCH_OFFSET        equ 1428h

RD_DST_X_LEFT_TO_RIGHT     equ 1
RD_DST_Y_TOP_TO_BOTTOM     equ 2
RD_DST_Y_X                 equ 1438h
RD_DST_WIDTH_HEIGHT        equ 1598h
RD_DST_LINE_START          equ 1600h
RD_DST_LINE_END            equ 1604h
R300_MEM_NUM_CHANNELS_MASK equ 0003h

macro rdr op1, op2
{
     mov edi, [ati_io]
     mov op1, [edi+op2]
}

macro BEGIN_RING
{
      mov edi, [rhd.ring_base]
      mov edx, [rhd.ring_wp]
}

macro COMMIT_RING
{
        and edx, 0x1FFF
        mov [rhd.ring_wp], edx

        lock add [esp], dword 0            ; Flush writes to ring

        wrr RADEON_CP_RB_WPTR, edx
        rdr eax, RADEON_CP_RB_RPTR
}

macro OUT_PACKET0 reg, count
{
     mov eax, (RADEON_CP_PACKET0 + (count shl 16) + (reg shr 2))
     mov [edi+edx*4], eax
     inc edx
}

macro OUT_PACKET3 pkt, count                                              \
{
     mov eax, (RADEON_CP_PACKET3 or pkt or (count shl 16))
     mov [edi+edx*4], eax
     inc edx
}

macro OUT_RING  val
{
     mov eax, val
     mov [edi+edx*4], eax
     inc edx
}

macro RADEON_WAIT_UNTIL_IDLE
{
     OUT_PACKET0 RADEON_WAIT_UNTIL, 0
     OUT_RING RADEON_WAIT_2D_IDLECLEAN + \
              RADEON_WAIT_3D_IDLECLEAN + \
              RADEON_WAIT_HOST_IDLECLEAN
}

macro RADEON_PURGE_CACHE
{
     OUT_PACKET0 R5XX_RB3D_DSTCACHE_CTLSTAT, 0
     OUT_RING  R5XX_RB3D_DC_FLUSH_ALL
}

macro RADEON_PURGE_ZCACHE
{
     OUT_PACKET0 RADEON_RB3D_ZCACHE_CTLSTAT, 0
     OUT_RING RADEON_RB3D_ZC_FLUSH_ALL
}

macro wrr dest, src
{
     mov edi, [ati_io]
     mov dword [edi+dest], src
}

macro rmask dest, val, mask
{
     mov edi, [ati_io]
     mov eax, [edi+dest]
     and eax, not mask
     or eax, (val and mask)
     mov [edi+dest], eax
}

public START
public service_proc
public version

CURSOR_IMAGE_OFFSET  equ 0x00500000

DRV_ENTRY equ 1
DRV_EXIT  equ -1

section '.flat' code readable align 16

proc START stdcall, state:dword

           cmp [state], 1
           jne .restore

     if DEBUG
           mov esi, msgInit
           call SysMsgBoardStr
     end if

           call detect_ati
           test eax, eax
           jz .fail

           mov ebx, [SelectHwCursor]
           mov ecx, [SetHwCursor]
           mov edx, [HwCursorRestore]
           mov esi, [HwCursorCreate]

           mov [oldSelect], ebx
           mov [oldSet], ecx
           mov [oldRestore], edx
           mov [oldCreate], esi

           call eax

           or eax, -1
           mov [cursor_map], eax
           mov [cursor_map+4], eax
           mov edx, cursor_map
           mov [cursor_start], edx
           add edx, 8
           mov [cursor_end], edx

           stdcall RegService, sz_ati_srv, service_proc
           test eax, eax
           jz .restore

if R500_HW2D
           stdcall RegService, sz_HDraw_srv, r500_HDraw

           mov ebx, START
           and ebx, -4096
           mov [eax+0x20], ebx
           mov [eax+0x24], dword 0                       ;hack
end if
           mov ebx, [fnSelect]
           mov ecx, [fnSet]

           mov [SelectHwCursor], ebx
           mov [SetHwCursor], ecx
           mov dword [HwCursorRestore], drv_restore
           mov dword [HwCursorCreate], ati_cursor

           ret
.restore:
           mov eax, [oldSelect]
           mov ebx, [oldSet]
           mov ecx, [oldRestore]
           mov edx, [oldCreate]

           mov [SelectHwCursor], eax
           mov [SetHwCursor], ebx
           mov [HwCursorRestore], ecx
           mov [HwCursorCreate], edx

           xor eax, eax
           ret

.fail:
     if DEBUG
           mov esi, msgFail
           call SysMsgBoardStr
     end if

           xor eax, eax
           ret
endp

handle     equ  IOCTL.handle
io_code    equ  IOCTL.io_code
input      equ  IOCTL.input
inp_size   equ  IOCTL.inp_size
output     equ  IOCTL.output
out_size   equ  IOCTL.out_size

align 4
proc service_proc stdcall, ioctl:dword

           mov ebx, [ioctl]
           cmp [ebx+io_code], SRV_GETVERSION
           jne .fail

           mov eax, [ebx+output]
           cmp [ebx+out_size], 4
           jne .fail
           mov [eax], dword API_VERSION
           xor eax, eax
           ret
.fail:
           or eax, -1
           ret
endp

restore   handle
restore   io_code
restore   input
restore   inp_size
restore   output
restore   out_size

align 4
proc detect_ati
           locals
             last_bus dd ?
           endl

           xor eax, eax
           mov [bus], eax
           inc eax
           call PciApi
           cmp eax, -1
           je .err

           mov [last_bus], eax

.next_bus:
           and [devfn], 0
.next_dev:
           stdcall PciRead32, [bus], [devfn], dword 0
           test eax, eax
           jz .next
           cmp eax, -1
           je .next

           mov edi, devices
@@:
           mov ebx, [edi]
           test ebx, ebx
           jz .next

           cmp eax, ebx
           je .found
           add edi, STRIDE
           jmp @B
.next:
           inc [devfn]
           cmp [devfn], 256
           jb  .next_dev
           mov eax, [bus]
           inc eax
           mov [bus], eax
           cmp eax, [last_bus]
           jna .next_bus
           xor eax, eax
           ret
.found:
           mov eax, [edi+4]
           ret
.err:
           xor eax, eax
           ret
endp

align 4
proc init_r200
           stdcall PciRead32, [bus], [devfn], dword 0x18
           and eax, PCI_MEMORY_MASK
           stdcall MapIoMem,eax,0x10000,(PG_SW+PG_NOCACHE)
           test eax, eax
           jz .fail

           mov [ati_io], eax
           mov edi, eax

           mov dword [edi+RD_RB3D_CNTL], 0
           call engRestore

           mov edi, [ati_io]
           mov eax, [edi+0x50]
           mov ebx,3
           shl ebx,20
           not ebx
           and eax,ebx
           mov ebx, 2
           shl ebx,20
           or eax, ebx
           mov [edi+0x50], eax

           call r200_ShowCursor

           mov [fnSelect], r200_SelectCursor
           mov [fnSet], r200_SetCursor

           xor eax, eax
           inc eax
.fail:
           ret
endp

if R500_HW2D
  include 'r500hw.inc'
end if

align 4
proc init_r500

           stdcall PciRead32, [bus], [devfn], dword 0x18
           and eax, PCI_MEMORY_MASK
           stdcall MapIoMem,eax,0x10000,(PG_SW+PG_NOCACHE)
           test eax, eax
           jz .fail

           mov [ati_io], eax

           mov [fnSelect], r500_SelectCursor
           mov [fnSet], r500_SetCursor

           rdr eax, 0x6110
           mov [r500_LFB], eax

if R500_HW2D
           call R5xx2DInit
end if
           wrr 0x6410, 0x001F001F
           wrr 0x6400, dword (3 shl 8)

           xor eax, eax
           inc eax
.fail:
           ret
endp


align 4
drv_restore:
           ret 8


align 4
proc r500_SelectCursor stdcall,hcursor:dword

           mov esi, [hcursor]

           mov edx, [esi+CURSOR.base]
           sub edx, LFB_BASE
           add edx, [r500_LFB]
           wrr 0x6408, edx

           mov eax, [esi+CURSOR.hot_x]
           shl eax, 16
           mov ax, word [esi+CURSOR.hot_y]
           wrr 0x6418, eax
           ret
endp

align 4
proc r500_SetCursor stdcall, hcursor:dword, x:dword, y:dword
           pushfd
           cli

           mov esi, [hcursor]
           mov edi, [ati_io]

           mov eax, [x]
           shl eax, 16
           mov ax, word [y]

           mov [edi+0x6414], eax
           or dword [edi+0x6400], 1

           popfd
           ret
endp

align 4
r500_ShowCursor:

           mov edi, [ati_io]
           or dword [edi+0x6400], 1
           ret

align 4
r200_ShowCursor:
           mov edi, [ati_io]

           mov eax, [edi+RD_CRTC_GEN_CNTL]
           bts eax,16
           mov [edi+RD_CRTC_GEN_CNTL], eax
           ret


align 4
proc r200_SelectCursor stdcall,hcursor:dword

           ret
endp

align 4
proc r200_SetCursor stdcall, hcursor:dword, x:dword, y:dword
           pushfd
           cli

           xor eax, eax
           xor edx, edx
           mov esi, [hcursor]
           mov ebx, [x]
           mov ecx, [y]

           sub ebx, [esi+CURSOR.hot_x]
           jnc @F
           neg ebx
           mov eax, ebx
           shl eax, 16
           xor ebx, ebx
@@:
           sub ecx, [esi+CURSOR.hot_y]
           jnc @F
           neg ecx
           mov ax, cx
           mov edx, ecx
           xor ecx, ecx
@@:
           or eax, 0x80000000
           wrr CUR_HORZ_VERT_OFF, eax

           shl ebx, 16
           mov bx, cx
           or ebx, 0x80000000
           wrr CUR_HORZ_VERT_POSN, ebx

           shl edx, 8
           add edx, [esi+CURSOR.base]
           sub edx, LFBAddress
           wrr CUR_OFFSET, edx
           popfd
           ret
endp

align 4
proc video_alloc

           pushfd
           cli
           mov ebx, [cursor_start]
           mov ecx, [cursor_end]
.l1:
           bsf eax,[ebx];
           jnz .found
           add ebx,4
           cmp ebx, ecx
           jb .l1
           popfd
           xor eax,eax
           ret
.found:
           btr [ebx], eax
           popfd

           mov [cursor_start],ebx
           sub ebx, cursor_map
           lea eax,[eax+ebx*8]

           shl eax,14
           add eax, LFBAddress+CURSOR_IMAGE_OFFSET
           ret
endp

align 4
video_free:
           pushfd
           cli
           sub eax, LFBAddress+CURSOR_IMAGE_OFFSET
           shr eax, 14
           mov ebx, cursor_map
           bts [ebx], eax
           shr eax, 3
           and eax, not 3
           add eax, ebx
           cmp [cursor_start], eax
           ja @f
           popfd
           ret
@@:
           mov [cursor_start], eax
           popfd
           ret

; param
;  eax= pid
;  ebx= src
;  ecx= flags

align 4
ati_cursor:
.src     equ esp
.flags   equ esp+4
.hcursor equ esp+8

           sub esp, 4          ;space for .hcursor
           push ecx
           push ebx

           mov ebx, eax
           mov eax, CURSOR_SIZE
           call CreateObject
           test eax, eax
           jz .fail

           mov [.hcursor],eax

           xor ebx, ebx
           mov [eax+CURSOR.magic], 'CURS'
           mov [eax+CURSOR.destroy], destroy_cursor
           mov [eax+CURSOR.hot_x], ebx
           mov [eax+CURSOR.hot_y], ebx

           call video_alloc
           mov edi, [.hcursor]
           mov [edi+CURSOR.base], eax

           mov esi, [.src]
           mov ebx, [.flags]
           cmp bx, LOAD_INDIRECT
           je .indirect

           movzx ecx, word [esi+10]
           movzx edx, word [esi+12]
           mov [edi+CURSOR.hot_x], ecx
           mov [edi+CURSOR.hot_y], edx

           stdcall ati_init_cursor, eax, esi
           mov eax, [.hcursor]
.fail:
           add esp, 12
           ret
.indirect:
           shr ebx, 16
           movzx ecx, bh
           movzx edx, bl
           mov [edi+CURSOR.hot_x], ecx
           mov [edi+CURSOR.hot_y], edx

           mov edi, eax
           mov ebx, eax
           mov ecx, 64*64
           xor eax,eax
           cld
           rep stosd
           mov edi, ebx

           mov esi, [.src]
           mov ebx, 32
           cld
@@:
           mov ecx, 32
           rep movsd
           add edi, 128
           dec ebx
           jnz @B
           mov eax, [.hcursor]
           add esp, 12
           ret

align 4
destroy_cursor:

           push eax
           mov eax, [eax+CURSOR.base]
           call video_free
           pop eax

           call DestroyObject
           ret

align 4
proc ati_init_cursor stdcall, dst:dword, src:dword
           locals
             rBase    dd ?
             pQuad    dd ?
             pBits    dd ?
             pAnd     dd ?
             width    dd ?
             height   dd ?
             counter  dd ?
           endl

           mov esi, [src]
           add esi,[esi+18]
           mov eax,esi

           cmp [esi+BI.biBitCount], 24
           je .img_24
           cmp [esi+BI.biBitCount], 8
           je .img_8
           cmp [esi+BI.biBitCount], 4
           je .img_4

.img_2:
           add eax, [esi]
           mov [pQuad],eax
           add eax,8
           mov [pBits],eax
           add eax, 128
           mov [pAnd],eax
           mov eax,[esi+4]
           mov [width],eax
           mov ebx,[esi+8]
           shr ebx,1
           mov [height],ebx

           mov edi, pCursor
           add edi, 32*31*4
           mov [rBase],edi

           mov esi,[pQuad]
.l21:
           mov ebx, [pBits]
           mov ebx, [ebx]
           bswap ebx
           mov eax, [pAnd]
           mov eax, [eax]
           bswap eax
           mov [counter], 32
@@:
           xor edx, edx
           shl eax,1
           setc dl
           dec edx

           xor ecx, ecx
           shl ebx,1
           setc cl
           mov ecx, [esi+ecx*4]
           and ecx, edx
           and edx, 0xFF000000
           or edx, ecx
           mov [edi], edx

           add edi, 4
           dec [counter]
           jnz @B

           add [pBits], 4
           add [pAnd], 4
           mov edi,[rBase]
           sub edi,128
           mov [rBase],edi
           sub [height],1
           jnz .l21
           jmp .copy
.img_4:
           add eax, [esi]
           mov [pQuad],eax
           add eax,64
           mov [pBits],eax
           add eax, 0x200
           mov [pAnd],eax
           mov eax,[esi+4]
           mov [width],eax
           mov ebx,[esi+8]
           shr ebx,1
           mov [height],ebx

           mov edi, pCursor
           add edi, 32*31*4
           mov [rBase],edi

           mov esi,[pQuad]
           mov ebx, [pBits]
.l4:
           mov eax, [pAnd]
           mov eax, [eax]
           bswap eax
           mov [counter], 16
@@:
           xor edx, edx
           shl eax,1
           setc dl
           dec edx

           movzx ecx, byte [ebx]
           and cl, 0xF0
           shr ecx, 2
           mov ecx, [esi+ecx]
           and ecx, edx
           and edx, 0xFF000000
           or edx, ecx
           mov [edi], edx

           xor edx, edx
           shl eax,1
           setc dl
           dec edx

           movzx ecx, byte [ebx]
           and cl, 0x0F
           mov ecx, [esi+ecx*4]
           and ecx, edx
           and edx, 0xFF000000
           or edx, ecx
           mov [edi+4], edx

           inc ebx
           add edi, 8
           dec [counter]
           jnz @B

           add [pAnd], 4
           mov edi,[rBase]
           sub edi,128
           mov [rBase],edi
           sub [height],1
           jnz .l4
           jmp .copy
.img_8:
           add eax, [esi]
           mov [pQuad],eax
           add eax,1024
           mov [pBits],eax
           add eax, 1024
           mov [pAnd],eax
           mov eax,[esi+4]
           mov [width],eax
           mov ebx,[esi+8]
           shr ebx,1
           mov [height],ebx

           mov edi, pCursor
           add edi, 32*31*4
           mov [rBase],edi

           mov esi,[pQuad]
           mov ebx, [pBits]
.l81:
           mov eax, [pAnd]
           mov eax, [eax]
           bswap eax
           mov [counter], 32
@@:
           xor edx, edx
           shl eax,1
           setc dl
           dec edx

           movzx ecx,  byte [ebx]
           mov ecx, [esi+ecx*4]
           and ecx, edx
           and edx, 0xFF000000
           or edx, ecx
           mov [edi], edx

           inc ebx
           add edi, 4
           dec [counter]
           jnz @B

           add [pAnd], 4
           mov edi,[rBase]
           sub edi,128
           mov [rBase],edi
           sub [height],1
           jnz .l81
           jmp .copy
.img_24:
           add eax, [esi]
           mov [pQuad],eax
           add eax, 0xC00
           mov [pAnd],eax
           mov eax,[esi+BI.biWidth]
           mov [width],eax
           mov ebx,[esi+BI.biHeight]
           shr ebx,1
           mov [height],ebx

           mov edi, pCursor
           add edi, 32*31*4
           mov [rBase],edi

           mov esi,[pAnd]
           mov ebx, [pQuad]
.row_24:
           mov eax, [esi]
           bswap eax
           mov [counter], 32
@@:
           xor edx, edx
           shl eax,1
           setc dl
           dec edx

           mov ecx, [ebx]
           and ecx, 0x00FFFFFF
           and ecx, edx
           and edx, 0xFF000000
           or edx, ecx
           mov [edi], edx
           add ebx, 3
           add edi, 4
           dec [counter]
           jnz @B

           add esi, 4
           mov edi,[rBase]
           sub edi,128
           mov [rBase],edi
           sub [height],1
           jnz .row_24
.copy:
           mov edi, [dst]
           mov ecx, 64*64
           xor eax,eax
           rep stosd

           mov esi, pCursor
           mov edi, [dst]
           mov ebx, 32
           cld
@@:
           mov ecx, 32
           rep movsd
           add edi, 128
           dec ebx
           jnz @B
           ret
endp

align 4
proc engFlush

           mov edi, [ati_io]

           mov eax, [edi+RD_RB2D_DSTCACHE_CTLSTAT]
           or eax,RD_RB2D_DC_FLUSH_ALL
           mov [edi+RD_RB2D_DSTCACHE_CTLSTAT],eax

           mov ecx, RD_TIMEOUT
@@:
           mov eax,[edi+RD_RB2D_DSTCACHE_CTLSTAT]
           and eax, RD_RB2D_DC_BUSY
           jz .exit

           sub ecx,1
           jnz @B
.exit:
           ret
endp


align 4
engWaitForFifo:
cnt equ bp+8
           push ebp
           mov ebp, esp

           mov edi, [ati_io]

           mov ecx, RD_TIMEOUT
@@:
           mov eax, [edi+RD_RBBM_STATUS]
           and eax, RD_RBBM_FIFOCNT_MASK
           cmp eax, [ebp+8]
           jae .exit

           sub ecx,1
           jmp @B

.exit:
           leave
           ret 4

align 4
proc engWaitForIdle

           push dword 64
           call engWaitForFifo

           mov edi, [ati_io]
           mov ecx ,RD_TIMEOUT
@@:
           mov eax, [edi+RD_RBBM_STATUS]
           and eax,RD_RBBM_ACTIVE
           jz .exit

           sub ecx,1
           jnz @B
.exit:
           call engFlush
           ret
endp


align 4
proc engRestore

;             push dword 1
;             call engWaitForFifo

;             mov dword  [MMIO+RD_RB2D_DSTCACHE_MODE], 0

           push dword 3
           call engWaitForFifo

           mov edi, [ati_io]

           mov eax, [edi+RD_DISPLAY_BASE_ADDR]
           shr eax, 10d
           or eax,(64d shl 22d)
           mov [edi+RD_DEFAULT_OFFSET],eax
           mov [edi+RD_SRC_PITCH_OFFSET],eax
           mov [edi+RD_DST_PITCH_OFFSET],eax

           push dword 1
           call engWaitForFifo

           mov edi, [ati_io]
           mov eax, [edi+RD_DP_DATATYPE]
           btr eax, 29d
           mov [edi+RD_DP_DATATYPE],eax

           push dword 1
           call engWaitForFifo

           mov edi, [ati_io]
           mov dword [edi+RD_DEFAULT_SC_BOTTOM_RIGHT],\
                     (RD_DEFAULT_SC_RIGHT_MAX or RD_DEFAULT_SC_BOTTOM_MAX)

           push dword 1
           call engWaitForFifo

           mov edi, [ati_io]
           mov dword [edi+RD_DP_GUI_MASTER_CNTL],\
                     (RD_GMC_BRUSH_SOLID_COLOR or \
                      RD_GMC_SRC_DATATYPE_COLOR or \
                     (6 shl RD_GMC_DST_DATATYPE_SHIFT) or \
                      RD_GMC_CLR_CMP_CNTL_DIS or \
                      RD_ROP3_P or \
                      RD_GMC_WR_MSK_DIS)


           push dword 7
           call engWaitForFifo

           mov edi, [ati_io]

           mov dword [edi+RD_DST_LINE_START],0
           mov dword [edi+RD_DST_LINE_END], 0
           mov dword [edi+RD_DP_BRUSH_FRGD_CLR], 808000ffh
           mov dword [edi+RD_DP_BRUSH_BKGD_CLR], 002020ffh
           mov dword [edi+RD_DP_SRC_FRGD_CLR],   808000ffh
           mov dword [edi+RD_DP_SRC_BKGD_CLR],   004000ffh
           mov dword [edi+RD_DP_WRITE_MASK],0ffffffffh

           call engWaitForIdle

           ret
endp



align 4
dword2str:
      mov  esi, hex_buff
      mov ecx, -8
@@:
      rol eax, 4
      mov ebx, eax
      and ebx, 0x0F
      mov bl, [ebx+hexletters]
      mov [8+esi+ecx], bl
      inc ecx
      jnz @B
      ret

hexletters   db '0123456789ABCDEF'
hex_buff     db 8 dup(0),13,10,0

R200M       equ 0x5a62  ;R300
R7000       equ 0x5159  ;R200
R750M       equ 0x4c57  ;M7 mobile rv200
R8500       equ 0x514C  ;R200
R9000       equ 0x4966  ;RV250
R9200       equ 0x5961  ;RV280
R9200SE     equ 0x5964  ;RV280
R9500       equ 0x4144  ;R300
R9500P      equ 0x4E45  ;R300
R9550       equ 0x4153  ;RV350
R9600       equ 0x4150  ;RV350
R9600XT     equ 0x4152  ;RV360
R9700P      equ 0x4E44  ;R300
R9800       equ 0x4E49  ;R350
R9800P      equ 0x4E48  ;R350
R9800XT     equ 0x4E4A  ;R360


align 4

devices:
        dd (R200M   shl 16)+VID_ATI, init_r200   ;R300
        dd (R7000   shl 16)+VID_ATI, init_r200
        dd (R750M   shl 16)+VID_ATI, init_r200   ;M7
        dd (R8500   shl 16)+VID_ATI, init_r200
        dd (R9000   shl 16)+VID_ATI, init_r200
        dd (0x514D  shl 16)+VID_ATI, init_r200   ;R200     9100

        dd (R9200   shl 16)+VID_ATI, init_r200
        dd (R9200SE shl 16)+VID_ATI, init_r200

        dd (0x5960  shl 16)+VID_ATI, init_r200   ;RV280    9250

        dd (R9500   shl 16)+VID_ATI, init_r200
        dd (R9500P  shl 16)+VID_ATI, init_r200
        dd (R9550   shl 16)+VID_ATI, init_r200

        dd (R9600   shl 16)+VID_ATI, init_r200
        dd (R9600XT shl 16)+VID_ATI, init_r200
        dd (0x4155  shl 16)+VID_ATI, init_r200   ;RV350    9600
        dd (0x4151  shl 16)+VID_ATI, init_r200   ;RV350    9600
        dd (0x4E51  shl 16)+VID_ATI, init_r200   ;RV350    9600

        dd (R9700P  shl 16)+VID_ATI, init_r200

        dd (0x4148  shl 16)+VID_ATI, init_r200   ;R350    9800
        dd (R9800   shl 16)+VID_ATI, init_r200
        dd (R9800P  shl 16)+VID_ATI, init_r200
        dd (R9800XT shl 16)+VID_ATI, init_r200

        dd (0x5B60  shl 16)+VID_ATI, init_r200   ;RV370    X300/X550
        dd (0x5B63  shl 16)+VID_ATI, init_r200   ;RV370    X550
        dd (0x5B62  shl 16)+VID_ATI, init_r200   ;RV380x   X600
        dd (0x3E50  shl 16)+VID_ATI, init_r200   ;RV380    X600/X550

        dd (0x5B4F  shl 16)+VID_ATI, init_r200   ;RV410    X700
        dd (0x5B4D  shl 16)+VID_ATI, init_r200   ;RV410    X700
        dd (0x5B4B  shl 16)+VID_ATI, init_r200   ;RV410    X700
        dd (0x5B4C  shl 16)+VID_ATI, init_r200   ;RV410    X700

        dd (0x4a49  shl 16)+VID_ATI, init_r200   ;R420     X800 PRO/GTO
        dd (0x4a4B  shl 16)+VID_ATI, init_r200   ;R420     X800
        dd (0x5549  shl 16)+VID_ATI, init_r200   ;R423     X800
        dd (0x4a4A  shl 16)+VID_ATI, init_r200   ;R420     X800
        dd (0x554F  shl 16)+VID_ATI, init_r200   ;R430     X800
        dd (0x554D  shl 16)+VID_ATI, init_r200   ;R430     X800
        dd (0x554E  shl 16)+VID_ATI, init_r200   ;R430     X800
        dd (0x5D57  shl 16)+VID_ATI, init_r200   ;R423     X800 XT
        dd (0x4A50  shl 16)+VID_ATI, init_r200   ;R420     X800 XT
        dd (0x554A  shl 16)+VID_ATI, init_r200   ;R423     X800 XT
        dd (0x5D4F  shl 16)+VID_ATI, init_r200   ;R423     X800/X850
        dd (0x554B  shl 16)+VID_ATI, init_r200   ;R423     X800 GT

        dd (0x4B4B  shl 16)+VID_ATI, init_r200   ;R481     X850
        dd (0x4B49  shl 16)+VID_ATI, init_r200   ;R481     X850
        dd (0x4B4C  shl 16)+VID_ATI, init_r200   ;R481     X850

        dd (0x5D4D  shl 16)+VID_ATI, init_r200   ;R480     X850
        dd (0x5D52  shl 16)+VID_ATI, init_r200   ;R480     X850

        dd (0x791E  shl 16)+VID_ATI, init_r500   ;RS690   X1200

        dd (0x7140  shl 16)+VID_ATI, init_r500   ;RV515   X1300
        dd (0x7142  shl 16)+VID_ATI, init_r500   ;RV515   X1300
        dd (0x7146  shl 16)+VID_ATI, init_r500   ;RV515   X1300
        dd (0x714D  shl 16)+VID_ATI, init_r500   ;RV515   X1300
        dd (0x714E  shl 16)+VID_ATI, init_r500   ;RV515   X1300

        dd (0x7183  shl 16)+VID_ATI, init_r500   ;RV515   X1300
        dd (0x7187  shl 16)+VID_ATI, init_r500   ;RV515   X1300
        dd (0x718F  shl 16)+VID_ATI, init_r500   ;RV515   X1300

        dd (0x7143  shl 16)+VID_ATI, init_r500   ;RV515   X1550
        dd (0x7147  shl 16)+VID_ATI, init_r500   ;RV515   X1550
        dd (0x715F  shl 16)+VID_ATI, init_r500   ;RV515   X1550
        dd (0x7193  shl 16)+VID_ATI, init_r500   ;RV515   X1550
        dd (0x719F  shl 16)+VID_ATI, init_r500   ;RV515   X1550

        dd (0x71C0  shl 16)+VID_ATI, init_r500   ;RV530   X1600
        dd (0x71C1  shl 16)+VID_ATI, init_r500   ;RV535   X1650
        dd (0x71C2  shl 16)+VID_ATI, init_r500   ;RV530   X1600
        dd (0x71C3  shl 16)+VID_ATI, init_r500   ;RV535   X1600
        dd (0x71C6  shl 16)+VID_ATI, init_r500   ;RV530   X1600
        dd (0x71C7  shl 16)+VID_ATI, init_r500   ;RV534   X1650

        dd (0x7181  shl 16)+VID_ATI, init_r500   ;RV515   X1600
        dd (0x71CD  shl 16)+VID_ATI, init_r500   ;RV530   X1600

        dd (0x7291  shl 16)+VID_ATI, init_r500   ;R580    X1650
        dd (0x7293  shl 16)+VID_ATI, init_r500   ;R580    X1650

        dd (0x7100  shl 16)+VID_ATI, init_r500   ;RV520   X1800
        dd (0x7109  shl 16)+VID_ATI, init_r500   ;RV520   X1800
        dd (0x710A  shl 16)+VID_ATI, init_r500   ;RV520   X1800 GTO

        dd (0x7249  shl 16)+VID_ATI, init_r500   ;RV580   X1900
        dd (0x724B  shl 16)+VID_ATI, init_r500   ;RV580   X1900 GT

        dd (0x7240  shl 16)+VID_ATI, init_r500   ;RV580   X1950
        dd (0x7244  shl 16)+VID_ATI, init_r500   ;RV580   X1950
        dd (0x7248  shl 16)+VID_ATI, init_r500   ;RV580   X1950

        dd (0x7288  shl 16)+VID_ATI, init_r500   ;R580    X1950 GT
        dd (0x7280  shl 16)+VID_ATI, init_r500   ;R580    X1950 PRO

        dd (0x94C3  shl 16)+VID_ATI, init_r500   ;RV610   HD 2400 PRO
        dd (0x94C1  shl 16)+VID_ATI, init_r500   ;RV610   HD 2400 XT

        dd (0x9589  shl 16)+VID_ATI, init_r500   ;RV630   HD 2600 PRO
        dd (0x958A  shl 16)+VID_ATI, init_r500   ;RV630   HD 2600 X2
        dd (0x9588  shl 16)+VID_ATI, init_r500   ;RV630   HD 2600 XT

        dd (0x9403  shl 16)+VID_ATI, init_r500   ;R600    HD 2900 PRO
        dd (0x9409  shl 16)+VID_ATI, init_r500   ;R600    HD 2900 XT


        dd 0    ;terminator

version      dd (5 shl 16) or (API_VERSION and 0xFFFF)

if R500_HW2D

align 16
R5xxRops  dd R5XX_ROP3_ZERO, R5XX_ROP3_ZERO  ;GXclear
          dd R5XX_ROP3_DSa,  R5XX_ROP3_DPa   ;Gxand
          dd R5XX_ROP3_SDna, R5XX_ROP3_PDna  ;GXandReverse
          dd R5XX_ROP3_S,    R5XX_ROP3_P     ;GXcopy
          dd R5XX_ROP3_DSna, R5XX_ROP3_DPna  ;GXandInverted
          dd R5XX_ROP3_D,    R5XX_ROP3_D     ;GXnoop
          dd R5XX_ROP3_DSx,  R5XX_ROP3_DPx   ;GXxor
          dd R5XX_ROP3_DSo,  R5XX_ROP3_DPo   ;GXor
          dd R5XX_ROP3_DSon, R5XX_ROP3_DPon  ;GXnor
          dd R5XX_ROP3_DSxn, R5XX_ROP3_PDxn  ;GXequiv
          dd R5XX_ROP3_Dn,   R5XX_ROP3_Dn    ;GXinvert
          dd R5XX_ROP3_SDno, R5XX_ROP3_PDno  ;GXorReverse
          dd R5XX_ROP3_Sn,   R5XX_ROP3_Pn    ;GXcopyInverted
          dd R5XX_ROP3_DSno, R5XX_ROP3_DPno  ;GXorInverted
          dd R5XX_ROP3_DSan, R5XX_ROP3_DPan  ;GXnand
          dd R5XX_ROP3_ONE,  R5XX_ROP3_ONE   ;GXset
end if


sz_ati_srv   db 'HWCURSOR',0

msgInit      db 'detect hardware...',13,10,0
msgPCI       db 'PCI accsess not supported',13,10,0
msgFail      db 'device not found',13,10,0
msg_neg      db 'neg ecx',13,10,0

if R500_HW2D

sz_HDraw_srv db 'HDRAW',0

msgR5xx2DFlushtimeout \
             db 'R5xx2DFlush timeout error',13,10,0
msgR5xxFIFOWaitLocaltimeout \
             db 'R5xxFIFOWaitLocal timeout error', 13, 10,0
msgR5xx2DIdleLocaltimeout \
             db 'R5xx2DIdleLocal timeout error', 13,10,0

align 4
R520_cp_microcode:
dd     0x4200e000, 0000000000
dd     0x4000e000, 0000000000
dd     0x00000099, 0x00000008
dd     0x0000009d, 0x00000008
dd     0x4a554b4a, 0000000000
dd     0x4a4a4467, 0000000000
dd     0x55526f75, 0000000000
dd     0x4a7e7d65, 0000000000
dd     0xe0dae6f6, 0000000000
dd     0x4ac54a4a, 0000000000
dd     0xc8828282, 0000000000
dd     0xbf4acfc1, 0000000000
dd     0x87b04ad5, 0000000000
dd     0xb5838383, 0000000000
dd     0x4a0f85ba, 0000000000
dd     0x000ca000, 0x00000004
dd     0x000d0012, 0x00000038
dd     0x0000e8b4, 0x00000004
dd     0x000d0014, 0x00000038
dd     0x0000e8b6, 0x00000004
dd     0x000d0016, 0x00000038
dd     0x0000e854, 0x00000004
dd     0x000d0018, 0x00000038
dd     0x0000e855, 0x00000004
dd     0x000d001a, 0x00000038
dd     0x0000e856, 0x00000004
dd     0x000d001c, 0x00000038
dd     0x0000e857, 0x00000004
dd     0x000d001e, 0x00000038
dd     0x0000e824, 0x00000004
dd     0x000d0020, 0x00000038
dd     0x0000e825, 0x00000004
dd     0x000d0022, 0x00000038
dd     0x0000e830, 0x00000004
dd     0x000d0024, 0x00000038
dd     0x0000f0c0, 0x00000004
dd     0x000d0026, 0x00000038
dd     0x0000f0c1, 0x00000004
dd     0x000d0028, 0x00000038
dd     0x0000e000, 0x00000004
dd     0x000d002a, 0x00000038
dd     0x0000e000, 0x00000004
dd     0x000d002c, 0x00000038
dd     0x0000e000, 0x00000004
dd     0x000d002e, 0x00000038
dd     0x0000e000, 0x00000004
dd     0x000d0030, 0x00000038
dd     0x0000e000, 0x00000004
dd     0x000d0032, 0x00000038
dd     0x0000f180, 0x00000004
dd     0x000d0034, 0x00000038
dd     0x0000f393, 0x00000004
dd     0x000d0036, 0x00000038
dd     0x0000f38a, 0x00000004
dd     0x000d0038, 0x00000038
dd     0x0000f38e, 0x00000004
dd     0x0000e821, 0x00000004
dd     0x0140a000, 0x00000004
dd     0x00000043, 0x00000018
dd     0x00cce800, 0x00000004
dd     0x001b0001, 0x00000004
dd     0x08004800, 0x00000004
dd     0x001b0001, 0x00000004
dd     0x08004800, 0x00000004
dd     0x001b0001, 0x00000004
dd     0x08004800, 0x00000004
dd     0x0000003a, 0x00000008
dd     0x0000a000, 0000000000
dd     0x2000451d, 0x00000004
dd     0x0000e580, 0x00000004
dd     0x000ce581, 0x00000004
dd     0x08004580, 0x00000004
dd     0x000ce581, 0x00000004
dd     0x00000047, 0x00000008
dd     0x0000a000, 0000000000
dd     0x000c2000, 0x00000004
dd     0x0000e50e, 0x00000004
dd     0x00032000, 0x00000004
dd     0x00022051, 0x00000028
dd     0x00000051, 0x00000024
dd     0x0800450f, 0x00000004
dd     0x0000a04b, 0x00000008
dd     0x0000e565, 0x00000004
dd     0x0000e566, 0x00000004
dd     0x00000052, 0x00000008
dd     0x03cca5b4, 0x00000004
dd     0x05432000, 0x00000004
dd     0x00022000, 0x00000004
dd     0x4ccce05e, 0x00000030
dd     0x08274565, 0x00000004
dd     0x0000005e, 0x00000030
dd     0x08004564, 0x00000004
dd     0x0000e566, 0x00000004
dd     0x00000055, 0x00000008
dd     0x00802061, 0x00000010
dd     0x00202000, 0x00000004
dd     0x001b00ff, 0x00000004
dd     0x01000064, 0x00000010
dd     0x001f2000, 0x00000004
dd     0x001c00ff, 0x00000004
dd     0000000000, 0x0000000c
dd     0x00000072, 0x00000030
dd     0x00000055, 0x00000008
dd     0x0000e576, 0x00000004
dd     0x0000e577, 0x00000004
dd     0x0000e50e, 0x00000004
dd     0x0000e50f, 0x00000004
dd     0x0140a000, 0x00000004
dd     0x00000069, 0x00000018
dd     0x00c0e5f9, 0x000000c2
dd     0x00000069, 0x00000008
dd     0x0014e50e, 0x00000004
dd     0x0040e50f, 0x00000004
dd     0x00c0006c, 0x00000008
dd     0x0000e570, 0x00000004
dd     0x0000e571, 0x00000004
dd     0x0000e572, 0x0000000c
dd     0x0000a000, 0x00000004
dd     0x0140a000, 0x00000004
dd     0x0000e568, 0x00000004
dd     0x000c2000, 0x00000004
dd     0x00000076, 0x00000018
dd     0x000b0000, 0x00000004
dd     0x18c0e562, 0x00000004
dd     0x00000078, 0x00000008
dd     0x00c00077, 0x00000008
dd     0x000700c7, 0x00000004
dd     0x00000080, 0x00000038
dd     0x0000e5bb, 0x00000004
dd     0x0000e5bc, 0000000000
dd     0x0000a000, 0x00000004
dd     0x0000e821, 0x00000004
dd     0x0000e800, 0000000000
dd     0x0000e821, 0x00000004
dd     0x0000e82e, 0000000000
dd     0x02cca000, 0x00000004
dd     0x00140000, 0x00000004
dd     0x000ce1cc, 0x00000004
dd     0x050de1cd, 0x00000004
dd     0x00400000, 0x00000004
dd     0x0000008f, 0x00000018
dd     0x00c0a000, 0x00000004
dd     0x0000008c, 0x00000008
dd     0x00000091, 0x00000020
dd     0x4200e000, 0000000000
dd     0x00000098, 0x00000038
dd     0x000ca000, 0x00000004
dd     0x00140000, 0x00000004
dd     0x000c2000, 0x00000004
dd     0x00160000, 0x00000004
dd     0x700ce000, 0x00000004
dd     0x00140094, 0x00000008
dd     0x4000e000, 0000000000
dd     0x02400000, 0x00000004
dd     0x400ee000, 0x00000004
dd     0x02400000, 0x00000004
dd     0x4000e000, 0000000000
dd     0x000c2000, 0x00000004
dd     0x0240e51b, 0x00000004
dd     0x0080e50a, 0x00000005
dd     0x0080e50b, 0x00000005
dd     0x00220000, 0x00000004
dd     0x000700c7, 0x00000004
dd     0x000000a4, 0x00000038
dd     0x0080e5bd, 0x00000005
dd     0x0000e5bb, 0x00000005
dd     0x0080e5bc, 0x00000005
dd     0x00210000, 0x00000004
dd     0x02800000, 0x00000004
dd     0x00c000ab, 0x00000018
dd     0x4180e000, 0x00000040
dd     0x000000ad, 0x00000024
dd     0x01000000, 0x0000000c
dd     0x0100e51d, 0x0000000c
dd     0x000045bb, 0x00000004
dd     0x000080a7, 0x00000008
dd     0x0000f3ce, 0x00000004
dd     0x0140a000, 0x00000004
dd     0x00cc2000, 0x00000004
dd     0x08c053cf, 0x00000040
dd     0x00008000, 0000000000
dd     0x0000f3d2, 0x00000004
dd     0x0140a000, 0x00000004
dd     0x00cc2000, 0x00000004
dd     0x08c053d3, 0x00000040
dd     0x00008000, 0000000000
dd     0x0000f39d, 0x00000004
dd     0x0140a000, 0x00000004
dd     0x00cc2000, 0x00000004
dd     0x08c0539e, 0x00000040
dd     0x00008000, 0000000000
dd     0x03c00830, 0x00000004
dd     0x4200e000, 0000000000
dd     0x0000a000, 0x00000004
dd     0x200045e0, 0x00000004
dd     0x0000e5e1, 0000000000
dd     0x00000001, 0000000000
dd     0x000700c4, 0x00000004
dd     0x0800e394, 0000000000
dd     0000000000, 0000000000
dd     0x0000e8c4, 0x00000004
dd     0x0000e8c5, 0x00000004
dd     0x0000e8c6, 0x00000004
dd     0x0000e928, 0x00000004
dd     0x0000e929, 0x00000004
dd     0x0000e92a, 0x00000004
dd     0x000000c8, 0x00000008
dd     0x0000e928, 0x00000004
dd     0x0000e929, 0x00000004
dd     0x0000e92a, 0x00000004
dd     0x000000cf, 0x00000008
dd     0xdeadbeef, 0000000000
dd     0x00000116, 0000000000
dd     0x000700d3, 0x00000004
dd     0x080050e7, 0x00000004
dd     0x000700d4, 0x00000004
dd     0x0800401c, 0x00000004
dd     0x0000e01d, 0000000000
dd     0x02c02000, 0x00000004
dd     0x00060000, 0x00000004
dd     0x000000de, 0x00000034
dd     0x000000db, 0x00000008
dd     0x00008000, 0x00000004
dd     0xc000e000, 0000000000
dd     0x0000e1cc, 0x00000004
dd     0x0500e1cd, 0x00000004
dd     0x000ca000, 0x00000004
dd     0x000000e5, 0x00000034
dd     0x000000e1, 0x00000008
dd     0x0000a000, 0000000000
dd     0x0019e1cc, 0x00000004
dd     0x001b0001, 0x00000004
dd     0x0500a000, 0x00000004
dd     0x080041cd, 0x00000004
dd     0x000ca000, 0x00000004
dd     0x000000fb, 0x00000034
dd     0x0000004a, 0x00000008
dd     0000000000, 0000000000
dd     0000000000, 0000000000
dd     0000000000, 0000000000
dd     0000000000, 0000000000
dd     0000000000, 0000000000
dd     0000000000, 0000000000
dd     0000000000, 0000000000
dd     0000000000, 0000000000
dd     0000000000, 0000000000
dd     0x000c2000, 0x00000004
dd     0x001d0018, 0x00000004
dd     0x001a0001, 0x00000004
dd     0x000000fb, 0x00000034
dd     0x0000004a, 0x00000008
dd     0x0500a04a, 0x00000008
dd     0000000000, 0000000000
dd     0000000000, 0000000000
dd     0000000000, 0000000000
dd     0000000000, 0000000000


end if

if 0
msg6100      db '6100:  ',0
msg6104      db '6104:  ',0
msg6108      db '6108:  ',0
msg6110      db '6110:  ',0
msg6120      db '6120:  ',0
msg6124      db '6124:  ',0
msg6128      db '6128:  ',0
msg612C      db '612C:  ',0
msg6130      db '6130:  ',0
msg6134      db '6134:  ',0
msg6138      db '6138:  ',0
end if

buff         db 8 dup(0)
             db 13,10, 0

section '.data' data readable writable align 16

pCursor  db 4096 dup(?)

cursor_map     rd 2
cursor_start   rd 1
cursor_end     rd 1

fnSelect       rd 1
fnSet          rd 1
oldSelect      rd 1
oldSet         rd 1
oldRestore     rd 1
oldCreate      rd 1

r500_LFB       rd 1

bus            dd ?
devfn          dd ?
ati_io         dd ?

if R500_HW2D

__xmin         rd 1
__xmax         rd 1
__ymin         rd 1
__ymax         rd 1

rhd            RHD

end if

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format MS COFF

include 'proc32.inc'
include 'imports.inc'

API_VERSION     equ 0x01000100

DEBUG           equ 1

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

CURSOR_SIZE     equ 32

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

OS_BASE         equ 0x80000000
SLOT_BASE       equ (OS_BASE+0x0080000)

PG_SW        equ 0x003
PG_NOCACHE   equ 0x018

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
RD_RB3D_CNTL            equ 1c3ch

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

macro wrr dest, src
{
     mov edi, [ati_io]
     mov dword [edi+dest], src
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
           jne .exit

     if DEBUG
           mov esi, msgInit
           call SysMsgBoardStr
     end if

           call detect_ati
           test eax, eax
           jz .fail

           call init_ati
           test eax, eax
           jz .fail

           or eax, -1
           mov [cursor_map], eax
           mov [cursor_map+4], eax
           mov edx, cursor_map
           mov [cursor_start], edx
           add edx, 8
           mov [cursor_end], edx

           stdcall RegService, sz_ati_srv, service_proc
           test eax, eax
           jz .fail
           mov dword [SetHwCursor], drvCursorPos ;enable hardware cursor
           mov dword [HwCursorRestore], drv_restore
           mov dword [HwCursorCreate], ati_cursor
           ret
.fail:
     if DEBUG
           mov esi, msgFail
           call SysMsgBoardStr
     end if

.exit:
           xor eax, eax
;           mov ebx, SetHwCursor
;           mov dword [ebx], eax    ;force disable hardware cursor
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
           add edi, 4
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
           xor eax, eax
           inc eax
           ret
.err:
           xor eax, eax
           ret
endp

align 4
proc init_ati

           stdcall AllocKernelSpace, dword 0x10000
           test eax, eax
           jz .fail

           mov [ati_io], eax

           stdcall PciRead32, [bus], [devfn], dword 0x18
           and eax, 0xFFFF0000
           mov esi, eax

           mov edi, [ati_io]
           mov edx, 16
@@:
           stdcall MapPage,edi,esi,PG_SW+PG_NOCACHE
           add edi, 0x1000
           add esi, 0x1000
           dec edx
           jnz @B

           mov edi, [ati_io]
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

           call drvShowCursor
           xor eax, eax
           inc eax
.fail:
           ret
endp

align 4
drv_restore:
           ret 8

align 4
drvShowCursor:
           mov edi, [ati_io]

           mov eax, [edi+RD_CRTC_GEN_CNTL]
           bts eax,16
           mov [edi+RD_CRTC_GEN_CNTL], eax
           ret

align 4
proc drvCursorPos stdcall, hcursor:dword, x:dword, y:dword
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
engSetupSolidFill:
           push ebp
           mov ebp, esp

           push dword 3
           call engWaitForFifo

           wrr RD_DP_GUI_MASTER_CNTL, cmdSolidFill

           mov eax, [ebp+8]
           wrr RD_DP_BRUSH_FRGD_CLR,eax

           mov edi, [ati_io]
           mov dword [edi+RD_DP_CNTL],(RD_DST_X_LEFT_TO_RIGHT or RD_DST_Y_TOP_TO_BOTTOM)
           leave
           ret 4


align 4
drvSolidFill:
;x:word,y:word,w:word,h:word,color:dword
            push ebp
            mov ebp, esp
x equ ebp+8
y equ ebp+12
w equ ebp+16
h equ ebp+20
color equ ebp+24

           push dword [ebp+24]
           call engSetupSolidFill

           push dword 2
           call engWaitForFifo

           mov edi, [ati_io]

           mov eax, [y]
           mov ebx, [x]
           shl eax,16
           or eax, ebx

           mov ecx,  [w]
           mov edx,  [h]
           shl ecx,16
           or ecx, edx
           mov [edi+RD_DST_Y_X], eax
           mov [edi+RD_DST_WIDTH_HEIGHT], ecx
           call engFlush
           leave
           ret 20

align 4
devices dd (R8500   shl 16)+VID_ATI
        dd (R9000   shl 16)+VID_ATI
        dd (R9200   shl 16)+VID_ATI
        dd (R9200SE shl 16)+VID_ATI
        dd (R9500   shl 16)+VID_ATI
        dd (R9500P  shl 16)+VID_ATI
        dd (R9550   shl 16)+VID_ATI
        dd (R9600   shl 16)+VID_ATI
        dd (R9600XT shl 16)+VID_ATI
        dd (R9700P  shl 16)+VID_ATI
        dd (R9800   shl 16)+VID_ATI
        dd (R9800P  shl 16)+VID_ATI
        dd (R9800XT shl 16)+VID_ATI
        dd 0    ;terminator

version      dd (5 shl 16) or (API_VERSION and 0xFFFF)

sz_ati_srv   db 'HWCURSOR',0

msgInit      db 'detect hardware...',13,10,0
msgPCI       db 'PCI accsess not supported',13,10,0
msgFail      db 'device not found',13,10,0
msg_neg      db 'neg ecx',13,10,0
buff         db 8 dup(0)
             db 13,10, 0

section '.data' data readable writable align 16

pCursor  db 4096 dup(?)

cursor_map     rd 2
cursor_start   rd 1
cursor_end     rd 1

bus            dd ?
devfn          dd ?
ati_io         dd ?




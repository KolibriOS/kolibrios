;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2013. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format MS COFF

DEBUG           = 1

include 'proc32.inc'
include '../struct.inc'
include 'imports.inc'

VID_INTEL         = 0x8086
VID_NVIDIA        = 0x10DE
VID_VIA           = 0x1106
VID_SIS           = 0x1039
VID_FM801         = 0x1319
VID_CREATIVE      = 0x1102
VID_ATI           = 0x1002
VID_AMD           = 0x1022
VID_ULI           = 0x10B9
VID_TERA          = 0x6549
VID_RDC           = 0x17F3
VID_VMWARE        = 0x15AD

CTRL_ICH          = 0x2415
CTRL_ICH0         = 0x2425
CTRL_ICH2         = 0x2435
CTRL_ICH3         = 0x2445
CTRL_ICH4         = 0x24C5
CTRL_ICH5         = 0x24D5
CTRL_ICH6         = 0x266E
CTRL_ICH7         = 0x27DE

CTRL_NFORCE       = 0x01B1
CTRL_NFORCE2      = 0x006A
CTRL_NFORCE3      = 0x00DA
CTRL_MCP04        = 0x003A
CTRL_CK804        = 0x0059
CTRL_CK8          = 0x008A
CTRL_CK8S         = 0x00EA
CTRL_MCP51        = 0x026B

CTRL_VT82C686     = 0x3058
CTRL_VT8233_5     = 0x3059

CTRL_SIS          = 0x7012

CTRL_FM801        = 0x0801

CTRL_CT0200       = 0x0006  ; Dell OEM version (EMU10K1X)

CTRL_INTEL_SCH2          =  0x080a
CTRL_INTEL_HPT           =  0x0c0c
CTRL_INTEL_CPT           =  0x1c20
CTRL_INTEL_PGB           =  0x1d20
CTRL_INTEL_PPT1          =  0x1e20
CTRL_INTEL_82801F        =  0x2668
CTRL_INTEL_63XXESB       =  0x269a
CTRL_INTEL_82801G        =  0x27d8
CTRL_INTEL_82801H        =  0x284b
CTRL_INTEL_82801_UNK1    =  0x2911
CTRL_INTEL_82801I        =  0x293e
CTRL_INTEL_82801_UNK2    =  0x293f
CTRL_INTEL_82801JI       =  0x3a3e
CTRL_INTEL_82801JD       =  0x3a6e
CTRL_INTEL_PCH           =  0x3b56
CTRL_INTEL_PCH2          =  0x3b57
CTRL_INTEL_SCH           =  0x811b
CTRL_INTEL_LPT           =  0x8c20

CTRL_NVIDIA_MCP51        =  0x026c
CTRL_NVIDIA_MCP55        =  0x0371
CTRL_NVIDIA_MCP61_1      =  0x03e4
CTRL_NVIDIA_MCP61_2      =  0x03f0
CTRL_NVIDIA_MCP65_1      =  0x044a
CTRL_NVIDIA_MCP65_2      =  0x044b
CTRL_NVIDIA_MCP67_1      =  0x055c
CTRL_NVIDIA_MCP67_2      =  0x055d
CTRL_NVIDIA_MCP78_1      =  0x0774
CTRL_NVIDIA_MCP78_2      =  0x0775
CTRL_NVIDIA_MCP78_3      =  0x0776
CTRL_NVIDIA_MCP78_4      =  0x0777
CTRL_NVIDIA_MCP73_1      =  0x07fc
CTRL_NVIDIA_MCP73_2      =  0x07fd
CTRL_NVIDIA_MCP79_1      =  0x0ac0
CTRL_NVIDIA_MCP79_2      =  0x0ac1
CTRL_NVIDIA_MCP79_3      =  0x0ac2
CTRL_NVIDIA_MCP79_4      =  0x0ac3
CTRL_NVIDIA_0BE2         =  0x0be2
CTRL_NVIDIA_0BE3         =  0x0be3
CTRL_NVIDIA_0BE4         =  0x0be4
CTRL_NVIDIA_GT100        =  0x0be5
CTRL_NVIDIA_GT106        =  0x0be9
CTRL_NVIDIA_GT108        =  0x0bea
CTRL_NVIDIA_GT104        =  0x0beb
CTRL_NVIDIA_GT116        =  0x0bee
CTRL_NVIDIA_MCP89_1      =  0x0d94
CTRL_NVIDIA_MCP89_2      =  0x0d95
CTRL_NVIDIA_MCP89_3      =  0x0d96
CTRL_NVIDIA_MCP89_4      =  0x0d97
CTRL_NVIDIA_GF119        =  0x0e08
CTRL_NVIDIA_GF110_1      =  0x0e09
CTRL_NVIDIA_GF110_2      =  0x0e0c

CTRL_ATI_SB450           =  0x437b
CTRL_ATI_SB600           =  0x4383

CTRL_ATI_RS600           =  0x793b
CTRL_ATI_RS690           =  0x7919
CTRL_ATI_RS780           =  0x960f
CTRL_ATI_RS_UNK1         =  0x970f
CTRL_ATI_R600            =  0xaa00
CTRL_ATI_RV630           =  0xaa08
CTRL_ATI_RV610           =  0xaa10
CTRL_ATI_RV670           =  0xaa18
CTRL_ATI_RV635           =  0xaa20
CTRL_ATI_RV620           =  0xaa28
CTRL_ATI_RV770           =  0xaa30
CTRL_ATI_RV730           =  0xaa38
CTRL_ATI_RV710           =  0xaa40
CTRL_ATI_RV740           =  0xaa48

CTRL_AMD_HUDSON          =  0x780d

CTRL_VIA_VT82XX          =  0x3288
CTRL_VIA_VT61XX          =  0x9140
CTRL_VIA_VT71XX          =  0x9170

CTRL_SIS_966             =  0x7502

CTRL_ULI_M5461           =  0x5461

CTRL_CREATIVE_CA0110_IBG     =  0x0009
CTRL_CREATIVE_SOUND_CORE3D_1 =  0x0010
CTRL_CREATIVE_SOUND_CORE3D_2 =  0x0012

CTRL_TERA_UNK1           =  0x1200

CTRL_RDC_R3010           =  0x3010

CTRL_VMWARE_UNK1         =  0x1977

API_VERSION             = 0x01000100

public START
public service_proc
public version

struct  SRV
        srv_name        rb 16    ;ASCIIZ string
        magic           dd ?     ;+0x10 ;'SRV '
        size            dd ?     ;+0x14 ;size of structure SRV
        fd              dd ?     ;+0x18 ;next SRV descriptor
        bk              dd ?     ;+0x1C ;prev SRV descriptor
        base            dd ?     ;+0x20 ;service base address
        entry           dd ?     ;+0x24 ;service START function
        srv_proc        dd ?     ;+0x28 ;user mode service handler
        srv_proc_ex     dd ?     ;+0x2C ;kernel mode service handler
ends


section '.flat' code readable align 16

proc START stdcall, state:dword

        mov     eax, [srv_entry]
        test    eax, eax
        jnz     .done

        cmp     [state], 1
        jne     .stop

     if DEBUG
        mov     esi, msgInit
        call    SysMsgBoardStr
     end if

        call    detect_controller
        ret
.stop:
        jmp     eax
.done:
        xor     eax, eax
        ret
endp

align 4
proc service_proc stdcall, ioctl:dword

        or      eax, -1
        ret
endp

align 4
proc detect_controller

           locals
             last_bus dd ?
             bus      dd ?
             devfn    dd ?
           endl

        xor     eax, eax
        mov     [bus], eax
        inc     eax
        call    PciApi
        cmp     eax, -1
        je      .err

        mov     [last_bus], eax

  .next_bus:
        and     [devfn], 0
  .next_dev:
        stdcall PciRead32, [bus], [devfn], dword 0
        test    eax, eax
        jz      .next
        cmp     eax, -1
        je      .next

        mov     edi, devices
  @@:
        mov     ebx, [edi]
        test    ebx, ebx
        jz      .next

        cmp     eax, ebx
        je      .found
        add     edi, 8
        jmp     @B
  .next:
        inc     [devfn]
        cmp     [devfn], 256
        jb      .next_dev
        mov     eax, [bus]
        inc     eax
        mov     [bus], eax
        cmp     eax, [last_bus]
        jna     .next_bus
        xor     eax, eax
        ret
  .found:

     if DEBUG
        mov     esi, msgLoading
        call    SysMsgBoardStr

        mov     esi, dword[edi+4]
        call    SysMsgBoardStr

        mov     esi, msgNewline
        call    SysMsgBoardStr
     end if

        stdcall GetService, dword[edi+4]
        test    eax, eax
        jz      .err

        mov     edx, [eax+SRV.entry]
        mov     [srv_entry], edx
        ret

  .err:
     if DEBUG
        mov     esi, msgFail
        call    SysMsgBoardStr
     end if

        xor     eax, eax
        ret

endp

align 4
devices         dd (CTRL_ICH  shl 16)+VID_INTEL, intelac97
                dd (CTRL_ICH0 shl 16)+VID_INTEL, intelac97
                dd (CTRL_ICH2 shl 16)+VID_INTEL, intelac97
                dd (CTRL_ICH3 shl 16)+VID_INTEL, intelac97
                dd (CTRL_ICH4 shl 16)+VID_INTEL, intelac97
                dd (CTRL_ICH5 shl 16)+VID_INTEL, intelac97
                dd (CTRL_ICH6 shl 16)+VID_INTEL, intelac97
                dd (CTRL_ICH7 shl 16)+VID_INTEL, intelac97

                dd (CTRL_NFORCE  shl 16)+VID_NVIDIA, intelac97
                dd (CTRL_NFORCE2 shl 16)+VID_NVIDIA, intelac97
                dd (CTRL_NFORCE3 shl 16)+VID_NVIDIA, intelac97
                dd (CTRL_MCP04   shl 16)+VID_NVIDIA, intelac97
                dd (CTRL_CK804   shl 16)+VID_NVIDIA, intelac97
                dd (CTRL_CK8     shl 16)+VID_NVIDIA, intelac97
                dd (CTRL_CK8S    shl 16)+VID_NVIDIA, intelac97
                dd (CTRL_MCP51   shl 16)+VID_NVIDIA, intelac97

                dd (CTRL_VT82C686  shl 16)+VID_VIA, vt823x
                dd (CTRL_VT8233_5  shl 16)+VID_VIA, vt823x

                dd (CTRL_SIS  shl 16)+VID_SIS, sis

                dd (CTRL_FM801 shl 16)+VID_FM801, fm801

                dd (0x5000 shl 16)+0x1274, ensoniq
                dd (0x5880 shl 16)+0x1274, ensoniq

                dd (CTRL_CT0200 shl 16)+VID_CREATIVE, emu10k1x
; Intel
                dd (CTRL_INTEL_SCH2    shl 16)+VID_INTEL, intelhda
                dd (CTRL_INTEL_HPT     shl 16)+VID_INTEL, intelhda
                dd (CTRL_INTEL_CPT     shl 16)+VID_INTEL, intelhda
                dd (CTRL_INTEL_PGB     shl 16)+VID_INTEL, intelhda
                dd (CTRL_INTEL_PPT1    shl 16)+VID_INTEL, intelhda
                dd (CTRL_INTEL_82801F  shl 16)+VID_INTEL, intelhda
                dd (CTRL_INTEL_63XXESB shl 16)+VID_INTEL, intelhda
                dd (CTRL_INTEL_82801G  shl 16)+VID_INTEL, intelhda
                dd (CTRL_INTEL_82801H  shl 16)+VID_INTEL, intelhda
                dd (CTRL_INTEL_82801_UNK1  shl 16)+VID_INTEL, intelhda
                dd (CTRL_INTEL_82801I  shl 16)+VID_INTEL, intelhda
                dd (CTRL_INTEL_82801_UNK2  shl 16)+VID_INTEL, intelhda
                dd (CTRL_INTEL_82801JI shl 16)+VID_INTEL, intelhda
                dd (CTRL_INTEL_82801JD shl 16)+VID_INTEL, intelhda
                dd (CTRL_INTEL_PCH     shl 16)+VID_INTEL, intelhda
                dd (CTRL_INTEL_PCH2    shl 16)+VID_INTEL, intelhda
                dd (CTRL_INTEL_SCH     shl 16)+VID_INTEL, intelhda
                dd (CTRL_INTEL_LPT     shl 16)+VID_INTEL, intelhda
; Nvidia
                dd (CTRL_NVIDIA_MCP51    shl 16)+VID_NVIDIA, intelhda
                dd (CTRL_NVIDIA_MCP55    shl 16)+VID_NVIDIA, intelhda
                dd (CTRL_NVIDIA_MCP61_1  shl 16)+VID_NVIDIA, intelhda
                dd (CTRL_NVIDIA_MCP61_2  shl 16)+VID_NVIDIA, intelhda
                dd (CTRL_NVIDIA_MCP65_1  shl 16)+VID_NVIDIA, intelhda
                dd (CTRL_NVIDIA_MCP65_2  shl 16)+VID_NVIDIA, intelhda
                dd (CTRL_NVIDIA_MCP67_1  shl 16)+VID_NVIDIA, intelhda
                dd (CTRL_NVIDIA_MCP67_2  shl 16)+VID_NVIDIA, intelhda
                dd (CTRL_NVIDIA_MCP73_1  shl 16)+VID_NVIDIA, intelhda
                dd (CTRL_NVIDIA_MCP73_2  shl 16)+VID_NVIDIA, intelhda
                dd (CTRL_NVIDIA_MCP78_1  shl 16)+VID_NVIDIA, intelhda
                dd (CTRL_NVIDIA_MCP78_2  shl 16)+VID_NVIDIA, intelhda
                dd (CTRL_NVIDIA_MCP78_3  shl 16)+VID_NVIDIA, intelhda
                dd (CTRL_NVIDIA_MCP78_4  shl 16)+VID_NVIDIA, intelhda
                dd (CTRL_NVIDIA_MCP79_1  shl 16)+VID_NVIDIA, intelhda
                dd (CTRL_NVIDIA_MCP79_2  shl 16)+VID_NVIDIA, intelhda
                dd (CTRL_NVIDIA_MCP79_3  shl 16)+VID_NVIDIA, intelhda
                dd (CTRL_NVIDIA_MCP79_4  shl 16)+VID_NVIDIA, intelhda
                dd (CTRL_NVIDIA_0BE2     shl 16)+VID_NVIDIA, intelhda
                dd (CTRL_NVIDIA_0BE3     shl 16)+VID_NVIDIA, intelhda
                dd (CTRL_NVIDIA_0BE4     shl 16)+VID_NVIDIA, intelhda
                dd (CTRL_NVIDIA_GT100    shl 16)+VID_NVIDIA, intelhda
                dd (CTRL_NVIDIA_GT106    shl 16)+VID_NVIDIA, intelhda
                dd (CTRL_NVIDIA_GT108    shl 16)+VID_NVIDIA, intelhda
                dd (CTRL_NVIDIA_GT104    shl 16)+VID_NVIDIA, intelhda
                dd (CTRL_NVIDIA_GT116    shl 16)+VID_NVIDIA, intelhda
                dd (CTRL_NVIDIA_MCP89_1  shl 16)+VID_NVIDIA, intelhda
                dd (CTRL_NVIDIA_MCP89_2  shl 16)+VID_NVIDIA, intelhda
                dd (CTRL_NVIDIA_MCP89_3  shl 16)+VID_NVIDIA, intelhda
                dd (CTRL_NVIDIA_MCP89_4  shl 16)+VID_NVIDIA, intelhda
                dd (CTRL_NVIDIA_GF119    shl 16)+VID_NVIDIA, intelhda
                dd (CTRL_NVIDIA_GF110_1  shl 16)+VID_NVIDIA, intelhda
                dd (CTRL_NVIDIA_GF110_2  shl 16)+VID_NVIDIA, intelhda
; ATI
                dd (CTRL_ATI_SB450   shl 16)+VID_ATI, intelhda
                dd (CTRL_ATI_SB600   shl 16)+VID_ATI, intelhda
                dd (CTRL_ATI_RS600   shl 16)+VID_ATI, intelhda
                dd (CTRL_ATI_RS690   shl 16)+VID_ATI, intelhda
                dd (CTRL_ATI_RS780   shl 16)+VID_ATI, intelhda
                dd (CTRL_ATI_RS_UNK1 shl 16)+VID_ATI, intelhda
                dd (CTRL_ATI_R600    shl 16)+VID_ATI, intelhda
                dd (CTRL_ATI_RV610   shl 16)+VID_ATI, intelhda
                dd (CTRL_ATI_RV620   shl 16)+VID_ATI, intelhda
                dd (CTRL_ATI_RV630   shl 16)+VID_ATI, intelhda
                dd (CTRL_ATI_RV635   shl 16)+VID_ATI, intelhda
                dd (CTRL_ATI_RV670   shl 16)+VID_ATI, intelhda
                dd (CTRL_ATI_RV710   shl 16)+VID_ATI, intelhda
                dd (CTRL_ATI_RV730   shl 16)+VID_ATI, intelhda
                dd (CTRL_ATI_RV740   shl 16)+VID_ATI, intelhda
                dd (CTRL_ATI_RV770   shl 16)+VID_ATI, intelhda
; AMD
                dd (CTRL_AMD_HUDSON shl 16)+VID_AMD, intelhda
; VIA
                dd (CTRL_VIA_VT82XX shl 16)+VID_VIA, intelhda
                dd (CTRL_VIA_VT61XX shl 16)+VID_VIA, intelhda
                dd (CTRL_VIA_VT71XX shl 16)+VID_VIA, intelhda
; SiS
                dd (CTRL_SIS_966    shl 16)+VID_SIS, intelhda
; ULI
                dd (CTRL_ULI_M5461  shl 16)+VID_ULI, intelhda
; Teradici
                dd (CTRL_TERA_UNK1  shl 16)+VID_ULI, intelhda
; Creative
                dd (CTRL_CREATIVE_CA0110_IBG     shl 16)+VID_CREATIVE, intelhda
                dd (CTRL_CREATIVE_SOUND_CORE3D_1 shl 16)+VID_CREATIVE, intelhda
                dd (CTRL_CREATIVE_SOUND_CORE3D_2 shl 16)+VID_CREATIVE, intelhda
; RDC Semiconductor
                dd (CTRL_RDC_R3010  shl 16)+VID_RDC, intelhda
; VMware
                dd (CTRL_VMWARE_UNK1  shl 16)+VID_VMWARE, intelhda

                dd 0    ;terminator


version         dd (5 shl 16) or (API_VERSION and 0xFFFF)

srv_entry       dd 0

intelac97       db 'INTELAC97', 0
vt823x          db 'VT823X', 0
sis             db 'SIS', 0
fm801           db 'FM801', 0
ensoniq         db 'ENSONIQ', 0
emu10k1x        db 'EMU10K1X', 0
intelhda        db 'INTEL_HDA', 0

msgInit         db 'Detecting hardware...',13,10,0
msgFail         db 'No compatible soundcard found!',13,10,0
msgLoading      db 'Loading ',0
msgNewline      db 13,10,0

section '.data' data readable writable align 16

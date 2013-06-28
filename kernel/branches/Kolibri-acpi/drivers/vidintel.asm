; Stub of videodriver for Intel videocards.
; (c) CleverMouse

; When the start procedure gots control,
; it tries to detect preferred resolution,
; sets the detected resolution assuming 32-bpp VESA mode and exits
; (without registering a service).
; Detection can be overloaded with compile-time settings
; use_predefined_mode/predefined_width/predefined_height.

; set predefined resolution here
use_predefined_mode = 0;1
predefined_width = 0;1366
predefined_height = 0;768

; standard driver stuff
format MS COFF

DEBUG = 1

include 'proc32.inc'
include 'imports.inc'

public START
public version

section '.flat' code readable align 16
; the start procedure (see the description above)
START:
; 1. Detect device. Abort if not found.
        push    esi
        call    DetectDevice
        test    esi, esi
        jz      .return0
; 2. Detect optimal mode unless the mode is given explicitly. Abort if failed.
if use_predefined_mode = 0
        call    DetectMode
end if
        cmp     [width], 0
        jz      .return0_cleanup
; 3. Set the detected mode.
        call    SetMode
; 4. Cleanup and return.
.return0_cleanup:
        stdcall FreeKernelSpace, esi
.return0:
        pop     esi
        xor     eax, eax
        ret     4

; check that there is Intel videocard
; if so, map MMIO registers and set internal variables
; esi points to MMIO block; NULL means no device
DetectDevice:
; 1. Sanity check: check that we are dealing with Intel videocard.
; Integrated video device for Intel is always at PCI:0:2:0.
        xor     esi, esi        ; initialize return value to NULL
; 1a. Get PCI VendorID and DeviceID.
        push    esi
        push    10h
        push    esi
        call    PciRead32
; 1b. loword(eax) = ax = VendorID, hiword(eax) = DeviceID.
; Test whether we have Intel chipset.
        cmp     ax, 8086h
        jnz     .return
; 1c. Say hi including DeviceID.
        shr     eax, 10h
        push    edi
        pusha
        mov     edi, pciid_text
        call    WriteWord
        mov     esi, hellomsg
        call    SysMsgBoardStr
        popa
; 1d. Test whether we know this DeviceID.
; If this is the case, remember the position of the device in line of Intel cards;
; this knowledge will be useful later.
; Tested on devices with id: 8086:0046, partially 8086:2A02.
        mov     ecx, pciids_num
        mov     edi, pciids
        repnz scasw
        pop     edi
        jnz     .return_unknown_pciid
        sub     ecx, pciids_num - 1
        neg     ecx
        mov     [deviceType], ecx
; 1e. Continue saying hi with positive intonation.
        pusha
        mov     esi, knownmsg
        call    SysMsgBoardStr
        popa
; 2. Prepare MMIO region to control the card.
; 2a. Read MMIO physical address from PCI config space.
        push    10h
        cmp     ecx, i9xx_start
        jae     @f
        mov     byte [esp], 14h
@@:
        push    10h
        push    esi
        call    PciRead32
; 2b. Mask out PCI region type, lower 4 bits.
        and     al, not 0xF
; 2c. Create virtual mapping of the physical memory.
        push    1Bh
        push    100000h
        push    eax
        call    MapIoMem
; 3. Return.
        xchg    esi, eax
.return:
        ret
; 1f. If we do not know DeviceID, continue saying hi with negative intonation.
.return_unknown_pciid:
        pusha
        mov     esi, unknownmsg
        call    SysMsgBoardStr
        popa
        ret

; Convert word in ax to hexadecimal text in edi, advance edi.
WriteWord:
; 1. Convert high byte.
        push    eax
        mov     al, ah
        call    WriteByte
        pop     eax
; 2. Convert low byte.
; Fall through to WriteByte; ret from WriteByte is ret from WriteWord too.

; Convert byte in al to hexadecimal text in edi, advance edi.
WriteByte:
; 1. Convert high nibble.
        push    eax
        shr     al, 4
        call    WriteNibble
        pop     eax
; 2. Convert low nibble.
        and     al, 0xF
; Fall through to WriteNibble; ret from WriteNibble is ret from WriteByte too.

; Convert nibble in al to hexadecimal text in edi, advance edi.
WriteNibble:
; Obvious, isn't it?
        cmp     al, 10
        sbb     al, 69h
        das
        stosb
        ret

if use_predefined_mode = 0
; detect resolution of the flat panel
DetectMode:
        push    esi edi
; 1. Get the location of block of GMBUS* registers.
; Starting with Ironlake, GMBUS* registers were moved.
        add     esi, 5100h
        cmp     [deviceType], ironlake_start
        jb      @f
        add     esi, 0xC0000
@@:
; 2. Initialize GMBUS engine.
        mov     edi, edid
        mov     ecx, 0x10000
@@:
        test    byte [esi+8+1], 80h
        loopnz  @b
        jnz     .fail
        mov     dword [esi], 3
        test    byte [esi+8+1], 4
        jz      .noreset
        call    ResetGMBus
        jnz     .fail
.noreset:
; 3. Send read command.
        and     dword [esi+20h], 0
        mov     dword [esi+4], 4E8000A1h
; 4. Wait for data, writing to the buffer as data arrive.
.getdata:
        mov     ecx, 0x10000
@@:
        test    byte [esi+8+1], 8
        loopz   @b
        test    byte [esi+8+1], 4
        jz      .dataok
        call    ResetGMBus
        jmp     .fail
.dataok:
        mov     eax, [esi+0Ch]
        stosd
        cmp     edi, edid+80h
        jb      .getdata
; 5. Wait for bus idle.
        mov     ecx, 0x10000
@@:
        test    byte [esi+8+1], 2
        loopnz  @b
; 6. We got EDID; dump it if DEBUG.
if DEBUG
        pusha
        xor     ecx, ecx
        mov     esi, edid
        mov     edi, edid_text
.dumploop:
        lodsb
        call    WriteByte
        mov     al, ' '
        stosb
        inc     cl
        test    cl, 15
        jnz     @f
        mov     byte [edi-1], 13
        mov     al, 10
        stosb
@@:
        test    cl, cl
        jns     .dumploop
        mov     esi, edidmsg
        call    SysMsgBoardStr
        popa
end if
; 7. Test whether EDID is good.
; 7a. Signature: 00 FF FF FF FF FF FF 00.
        mov     esi, edid
        cmp     dword [esi], 0xFFFFFF00
        jnz     .fail
        cmp     dword [esi+4], 0x00FFFFFF
        jnz     .fail
; 7b. Checksum must be zero.
        xor     edx, edx
        mov     ecx, 80h
@@:
        lodsb
        add     dl, al
        loop    @b
        jnz     .fail
; 8. Get width and height from EDID.
        xor     eax, eax
        mov     ah, [esi-80h+3Ah]
        shr     ah, 4
        mov     al, [esi-80h+38h]
        mov     [width], eax
        mov     ah, [esi-80h+3Dh]
        shr     ah, 4
        mov     al, [esi-80h+3Bh]
        mov     [height], eax
; 9. Return.
.fail:
        pop     edi esi
        ret

; reset bus, clear all errors
ResetGMBus:
; look into the PRM
        mov     dword [esi+4], 80000000h
        mov     dword [esi+4], 0
        mov     ecx, 0x10000
@@:
        test    byte [esi+8+1], 2
        loopnz  @b
        ret
end if

; set resolution [width]*[height]
SetMode:
; 1. Program the registers of videocard.
; look into the PRM
        cli
;       or      byte [esi+7000Ah], 0Ch  ; PIPEACONF: disable Display+Cursor Planes
;       or      byte [esi+7100Ah], 0Ch  ; PIPEBCONF: disable Display+Cursor Planes
        xor     eax, eax
        xor     edx, edx
        cmp     [deviceType], i965_start
        jb      @f
        mov     dl, 9Ch - 84h
@@:
;       or      byte [esi+71403h], 80h  ; VGACNTRL: VGA Display Disable
        and     byte [esi+70080h], not 27h      ; CURACNTR: disable cursor A
        mov     dword [esi+70084h], eax ; CURABASE: force write to CURA* regs
        and     byte [esi+700C0h], not 27h      ; CURBCNTR: disable cursor B
        mov     dword [esi+700C4h], eax ; CURBBASE: force write to CURB* regs
        and     byte [esi+70183h], not 80h      ; DSPACNTR: disable Primary A Plane
        mov     dword [esi+edx+70184h], eax     ; DSPALINOFF/DSPASURF: force write to DSPA* regs
        and     byte [esi+71183h], not 80h      ; DSPBCNTR: disable Primary B Plane
        mov     dword [esi+edx+71184h], eax     ; DSPBLINOFF/DSPBSURF: force write to DSPB* regs
if 1
        cmp     [deviceType], ironlake_start
        jae     .disable_pipes
        mov     edx, 10000h
        or      byte [esi+70024h], 2    ; PIPEASTAT: clear VBLANK status
        or      byte [esi+71024h], 2    ; PIPEBSTAT: clear VBLANK status
.wait_vblank_preironlake1:
        mov     ecx, 1000h
        loop    $
        test    byte [esi+7000Bh], 80h          ; PIPEACONF: pipe A active?
        jz      @f
        test    byte [esi+70024h], 2            ; PIPEASTAT: got VBLANK?
        jz      .wait_vblank_preironlake2
@@:
        test    byte [esi+7100Bh], 80h          ; PIPEBCONF: pipe B active?
        jz      .disable_pipes
        test    byte [esi+71024h], 2            ; PIPEBSTAT: got VBLANK?
        jnz     .disable_pipes
.wait_vblank_preironlake2:
        dec     edx
        jnz     .wait_vblank_preironlake1
        jmp     .not_disabled
.disable_pipes:
end if
        and     byte [esi+7000Bh], not 80h      ; PIPEACONF: disable pipe
        and     byte [esi+7100Bh], not 80h      ; PIPEBCONF: disable pipe
        cmp     [deviceType], gen4_start
        jb      .wait_watching_scanline
; g45 and later: use special flag from PIPE*CONF
        mov     edx, 10000h
@@:
        mov     ecx, 1000h
        loop    $
        test    byte [esi+7000Bh], 40h  ; PIPEACONF: wait until pipe disabled
        jz      @f
        dec     edx
        jnz     @b
        jmp     .not_disabled
@@:
        test    byte [esi+7100Bh], 40h  ; PIPEBCONF: wait until pipe disabled
        jz      .disabled
        mov     ecx, 1000h
        loop    $
        dec     edx
        jnz     @b
        jmp     .not_disabled
; pineview and before: wait while scanline still changes
.wait_watching_scanline:
        mov     edx, 1000h
.dis1:
        push    dword [esi+71000h]
        push    dword [esi+70000h]
        mov     ecx, 10000h
        loop    $
        pop     eax
        xor     eax, [esi+70000h]
        and     eax, 1FFFh
        pop     eax
        jnz     .notdis1
        xor     eax, [esi+71000h]
        and     eax, 1FFFh
        jz      .disabled
.notdis1:
        dec     edx
        jnz     .dis1
.not_disabled:
        sti
        jmp     .return
.disabled:
        lea     eax, [esi+61183h]
        cmp     [deviceType], ironlake_start
        jb      @f
        add     eax, 0xE0000 - 0x60000
@@:
        lea     edx, [esi+60000h]
        test    byte [eax], 40h
        jz      @f
        add     edx, 1000h
@@:
        mov     eax, [width]
        dec     eax
        shl     eax, 16
        mov     ax, word [height]
        dec     eax
        mov     dword [edx+1Ch], eax    ; PIPEASRC: set source image size
        ror     eax, 16
        mov     dword [edx+10190h], eax ; for old cards
        mov     ecx, [width]
        add     ecx, 15
        and     ecx, not 15
        shl     ecx, 2
        mov     dword [edx+10188h], ecx ; DSPASTRIDE: set scanline length
        mov     dword [edx+10184h], 0   ; DSPALINOFF: force write to DSPA* registers
        and     byte [esi+61233h], not 80h      ; PFIT_CONTROL: disable panel fitting 
        or      byte [edx+1000Bh], 80h          ; PIPEACONF: enable pipe
;       and     byte [edx+1000Ah], not 0Ch      ; PIPEACONF: enable Display+Cursor Planes
        or      byte [edx+10183h], 80h          ; DSPACNTR: enable Display Plane A
        sti
; 2. Notify the kernel that resolution has changed.
        call    GetDisplay
        mov     edx, [width]
        mov     dword [eax+8], edx
        mov     edx, [height]
        mov     dword [eax+0Ch], edx
        mov     [eax+18h], ecx
        mov     eax, [width]
        dec     eax
        dec     edx
        call    SetScreen
.return:
        ret

align 4
hellomsg        db      'Intel videocard detected, PciId=8086:'
pciid_text      db      '0000'
                db      ', which is ', 0
knownmsg        db      'known',13,10,0
unknownmsg      db      'unknown',13,10,0

if DEBUG
edidmsg         db      'EDID successfully read:',13,10
edid_text       rb      8*(16*3+1)
                db      0
end if

version:
        dd      0x50005

width   dd      predefined_width
height  dd      predefined_height

pciids:
        dw      0x3577          ; i830m
        dw      0x2562          ; 845g
        dw      0x3582          ; i855gm
i865_start = ($ - pciids) / 2
        dw      0x2572          ; i865g
i9xx_start = ($ - pciids) / 2
        dw      0x2582  ; i915g
        dw      0x258a  ; e7221g (i915g)
        dw      0x2592  ; i915gm
        dw      0x2772  ; i945g
        dw      0x27a2  ; i945gm
        dw      0x27ae  ; i945gme
i965_start = ($ - pciids) / 2
        dw      0x2972  ; i946qz (i965g)
        dw      0x2982  ; g35g (i965g)
        dw      0x2992  ; i965q (i965g)
        dw      0x29a2  ; i965g
        dw      0x29b2  ; q35g
        dw      0x29c2  ; g33g
        dw      0x29d2  ; q33g
        dw      0xa001  ; pineview
        dw      0xa011  ; pineview
gen4_start = ($ - pciids) / 2
        dw      0x2a02  ; i965gm
        dw      0x2a12  ; i965gm
        dw      0x2a42  ; gm45
        dw      0x2e02  ; g45
        dw      0x2e12  ; g45
        dw      0x2e22  ; g45
        dw      0x2e32  ; g45
        dw      0x2e42  ; g45
        dw      0x2e92  ; g45
ironlake_start = ($ - pciids) / 2
        dw      0x0042  ; ironlake_d
        dw      0x0046  ; ironlake_m
        dw      0x0102  ; sandybridge_d
        dw      0x0112  ; sandybridge_d
        dw      0x0122  ; sandybridge_d
        dw      0x0106  ; sandybridge_m
        dw      0x0116  ; sandybridge_m
        dw      0x0126  ; sandybridge_m
        dw      0x010A  ; sandybridge_d
pciids_num = ($ - pciids) / 2

align 4
deviceType      dd      ?
edid    rb      0x80

; SPDX-License-Identifier: NOASSERTION
;
; KBD - Kolibri Bus Disconnect
;
; Authors: Ilya Mikhailov (Ghost)

; ====================================================================

use32
org 0

; ====================================================================

db      "MENUET01"              ; 8-byte signature
dd      1                       ; header version
dd      START                   ; entry point
dd      I_END                   ; image size
dd      MEM                     ; total memory
dd      STACKTOP                ; esp
dd      PARAMS                  ; I_Param: cmdline buffer
dd      0                       ; I_Path: unused

; ====================================================================

include "../../macros.inc"
include "../../KOSfuncs.inc"
include "../../encoding.inc"

; ====================================================================

START:
        call    find_north_bridge

        ; "BOOT" cmdline -> enable disconnect on boot, then exit
        cmp     dword [PARAMS], "BOOT"
        jne     .gui
        test    dword [bd_id], -1       ; nothing to enable if not found
        jz      controller_not_found
        call    set_bd_stat
        mcall   SF_TERMINATE_PROCESS

        ; GUI mode: always show the window, even if no controller was found
        .gui:
        mcall   SF_SET_EVENTS_MASK, EVM_REDRAW + EVM_BUTTON
        jmp     draw_window

; ====================================================================

controller_not_found:
        ; notify the user, then terminate
        mcall   SF_FILE, notify_info
        mcall   SF_TERMINATE_PROCESS

; ====================================================================

still:
        mcall   SF_WAIT_EVENT

        cmp     al, EV_REDRAW
        je      draw_window
        cmp     al, EV_BUTTON
        je      button

        jmp     still

; ====================================================================

draw_window:
        mcall   SF_STYLE_SETTINGS, SSF_GET_COLORS, sc, sizeof.system_colors
        mcall   SF_REDRAW, SSF_BEGIN_DRAW

        mov     edx, [sc.work]
        or      edx, 0x13000000
        mcall   SF_CREATE_WINDOW, <100, 300>, <100, 90>, , , title

        ; labels (reuse eax = SF_DRAW_TEXT, ecx = text color + null-term flag)
        mov     ecx, [sc.work_text]
        or      ecx, 0x80000000         ; null-terminated strings
        mcall   SF_DRAW_TEXT, <17, 30>, , msg_nb

        mov     edx, [nb_name]
        mcall   , <105, 30>

        mcall   , <17, 40>, , msg_stat
        mcall   , <102, 50>, , msg_divs
        mcall   , <17, 62>, , msg_hdd
        mcall   , <17, 72>, , msg_sgd

        ; HDD and SGD divisor values
        call    get_divs
        mov     ecx, [val_hdd]
        mov     edx, 8
        shl     edx, cl
        mov     ecx, edx                ; ecx = HDD divisor
        mcall   SF_DRAW_NUMBER, 0x30000, , <80, 62>, [sc.work_text]

        mov     ecx, [val_sgd]
        mov     edx, 8
        shl     edx, cl
        mov     ecx, edx                ; ecx = SGD divisor
        mcall   , , , <80, 72>

        ; status text (Enabled / Disabled / Not found)
        call    get_bd_stat
        mov     edx, msg_nf
        mov     al, [bd_stat]
        test    al, al
        jz      @f
        mov     edx, msg_dis
        dec     al
        jz      @f
        mov     edx, msg_en
        
        @@:
        mov     ecx, [sc.work_text]
        or      ecx, 0x80000000
        mcall   SF_DRAW_TEXT, <105, 40>

        ; status toggle button
        mcall   SF_DEFINE_BUTTON, <220, 70>, <27, 20>, 2, [sc.work_button]

        ; 14 divisor buttons (two rows of 7)
        mov     edi, 7
        mpack   ecx, 60, 10
        mov     eax, SF_DEFINE_BUTTON
        mpack   ebx, 105, 25
        mov     edx, 3
        
        @@:
        mcall
        inc     edx
        add     ebx, 27 shl 16
        dec     edi
        jnz     @b

        add     ecx, 12 shl 16
        mpack   ebx, 105, 25
        mov     edi, 7
        
        @@:
        mcall
        inc     edx
        add     ebx, 27 shl 16
        dec     edi
        jnz     @b

        mcall   SF_REDRAW, SSF_END_DRAW
        jmp     still

; ====================================================================

button:
        mcall   SF_GET_BUTTON
        cmp     ah, 1
        je      .exit
        cmp     dword [bd_id], 0
        jz      draw_window             ; no controller -> ignore controls
        cmp     ah, 2
        jne     .divisor

        ; bus disconnect toggle
        mov     dl, [bd_stat]
        test    dl, dl
        jz      draw_window             ; unsupported chipset -> just redraw
        xor     eax, eax
        dec     dl
        jnz     @f
        inc     eax
        
        @@:
        call    set_bd_stat
        jmp     draw_window

        ; HDD divisor (buttons 3..9)
        .divisor:
        cmp     ah, 9
        jg      .sgd
        sub     ah, 3
        movzx   esi, ah
        mov     edi, [val_sgd]
        call    set_divs
        jmp     draw_window

        .sgd:
        ; SGD divisor (buttons 10..16)
        sub     ah, 10
        movzx   edi, ah
        mov     esi, [val_hdd]
        call    set_divs
        jmp     draw_window

        .exit:
        mcall   SF_TERMINATE_PROCESS

; ====================================================================

find_north_bridge:
        mov     bl, 6                   ; SF_PCI: read configuration dword
        xor     cl, cl                  ; register 0 = vendor / device id

        .next_bus:
        mov     bh, [bus_num]
        mov     byte [devfn], 0         ; restart sweep at device 0, function 0

        .next_devfn:
        mov     ch, [devfn]
        mcall   SF_PCI
        cmp     eax, 0xFFFFFFFF         ; nothing at this address
        je      .skip

        ; look the id up in bd_table
        mov     esi, bd_table_end - bd_table - 8

        .scan:
        cmp     eax, [bd_table + esi]
        je      .found
        test    esi, esi
        jz      .skip
        sub     esi, 8
        jmp     .scan

        .skip:
        inc     byte [devfn]
        jnz     .next_devfn            ; wraps 255->0 once all 256 slots swept
        dec     byte [bus_num]
        jns     .next_bus
        ret

        .found:
        mov     [bd_id], eax
        movzx   eax, word [bd_table + esi + 4]  ; name string
        mov     [nb_name], eax
        movzx   eax, word [bd_table + esi + 6]  ; mask recipe
        mov     [bd_msk], eax

        ret

; ====================================================================

get_bd_stat:
        mov     byte [bd_stat], 1       ; assume disconnect disabled
        cld
        mov     esi, [bd_msk]
        lodsw                           ; al = register, ah = bit mask
        test    al, al
        jnz     .check
        mov     byte [bd_stat], 0       ; empty recipe -> unsupported
        ret

        .check:
        push    eax
        mov     bh, [bus_num]
        mov     bl, 4                   ; SF_PCI: read configuration byte
        mov     ch, [devfn]
        mov     cl, al                  ; register offset
        mcall   SF_PCI
        pop     edx                     ; dl = register, dh = bit mask
        and     al, dh
        jnz     .enabled                ; masked bit set -> enabled

        lodsw                           ; next register / mask pair
        test    al, al
        jnz     .check
        ret                             ; all pairs clear -> stays disabled

        .enabled:
        mov     byte [bd_stat], 2
        ret

; ====================================================================

; IN: eax = 0 -> disable, non-zero -> enable
set_bd_stat:
        cmp     dword [bd_id], 0x01E010DE       ; nForce2 uses a different register
        je      set_stat_nforce2

        mov     edi, eax                        ; edi = enable flag
        cld
        mov     esi, [bd_msk]

        .next:
        lodsw                                   ; al = register, ah = bit mask
        test    al, al
        jz      .done                           ; end of recipe

        mov     dl, ah                          ; dl = bit mask
        mov     bh, [bus_num]
        mov     bl, 4                           ; SF_PCI: read configuration byte
        mov     ch, [devfn]
        mov     cl, al                          ; register offset
        mcall   SF_PCI

        mov     bl, 8                           ; SF_PCI: write configuration byte
        test    edi, edi
        jz      .clear

        or      al, dl                          ; enable: set the mask bits
        mov     dl, al
        mcall   SF_PCI
        jmp     .next

        .clear:
        not     dl                              ; disable: clear the mask bits
        and     al, dl
        mov     dl, al
        mcall   SF_PCI
        jmp     .next

        .done:
        ret

; ====================================================================

; IN: eax = 0 -> disable, non-zero -> enable
set_stat_nforce2:
        mov     edi, eax                        ; enable flag (survives mcall)

        mov     bh, [bus_num]
        mov     bl, 4                           ; SF_PCI: read configuration byte
        mov     ch, [devfn]
        mov     cl, 0x6F
        mcall   SF_PCI

        mov     dl, al
        test    edi, edi
        jz      .disable
        or      dl, 0x10                        ; enable disconnect
        jmp     .write

        .disable:
        and     dl, 0xEF                        ; disable disconnect

        .write:
        mov     bl, 8                           ; SF_PCI: write configuration byte
        mcall   SF_PCI

        test    edi, edi                        ; C1-HALT fixup applies on enable only
        jz      .done
        mov     bh, [bus_num]
        mov     bl, 8
        mov     ch, [devfn]
        mov     cl, 0x6E
        mov     dl, 1
        mcall   SF_PCI

        .done:
        ret

; ====================================================================

; OUT: val_hdd / val_sgd = current divisor indices (0 = x8, 1 = x16, ...)
get_divs:
        cmp     dword [bd_id], 0
        jnz     .read
        mov     dword [val_hdd], 0      ; no controller -> default x8, skip MSR
        mov     dword [val_sgd], 0
        ret

        .read:
        ; al = HDD divisor code, ah = SGD divisor code, bit 18 = SGD upper range
        mcall   SF_SYS_MISC, SSF_READ_MSR, , 0xC001001b

        ; reverse-lookup HDD code -> index
        mov     ecx, 7
        .find_hdd:
        cmp     [div_hdd + ecx - 1], al
        je      .got_hdd
        loop    .find_hdd

        .got_hdd:
        dec     ecx
        mov     [val_hdd], ecx
        
        ; reverse-lookup SGD code -> index
        mov     ecx, 4
        .find_sgd:
        cmp     [div_sgd + ecx - 1], ah
        je      .got_sgd
        loop    .find_sgd
        
        .got_sgd:
        dec     ecx
        ; bit 18 set -> upper SGD range (x128..x512)
        test    eax, 0x40000
        jz      .store
        add     ecx, 4

        .store:
        mov     [val_sgd], ecx
        ret

; ====================================================================

; IN: ESI = HDD index, EDI = SGD index (0 = x8, 1 = x16, ...)
set_divs:
        ; eax = MSR low dword, ebx = MSR high dword
        mcall   SF_SYS_MISC, SSF_READ_MSR, , 0xC001001b

        mov     al, [div_hdd + esi]     ; byte 0 = HDD divisor code
        mov     ah, [div_sgd + edi]     ; byte 1 = SGD divisor code
        and     eax, 0xFFFBFFFF         ; clear bit 18 (SGD range select)
        cmp     edi, 3
        jle     .write
        or      eax, 0x40000            ; SGD index > 3 -> upper range (bit 18)

        .write:
        mov     edi, eax                ; edi = patched low dword
        mov     esi, ebx                ; esi = high dword (unchanged)
        mcall   SF_SYS_MISC, SSF_WRITE_MSR  ; edx still holds the MSR index
        ret

; ====================================================================

bus_num:        db      2       ; scan start bus (walks 2 -> 0)
devfn:          db      0       ; PCI dev/func scan cursor (reset per bus)
bd_id:          dd      0       ; Device ID
bd_stat:        db      0       ; 0 - not found, 1 - disabled, 2 - enabled

nb_name         dd      msg_nf
bd_msk          dd      msk_none

; ====================================================================

msk_none        db      0       ; empty mask = unsupported / "not found"

; --- AMD ---
msk_amd751      db      0x62, 0x06, 0              ; 751, 761
nb_amd751       db      "AMD 751", 0
nb_amd761       db      "AMD 761", 0
msk_amd762      db      0x62, 0x02, 0x6A, 0x02, 0  ; 762 (two host bridges)
nb_amd762       db      "AMD 762", 0

; --- nVIDIA ---
msk_nforce      db      0xE7, 0x06, 0
nb_nforce       db      "nForce", 0
msk_nforce2     db      0x6F, 0x10, 0             ; written via set_stat_nforce2
nb_nforce2      db      "nForce 2", 0

; --- SiS ---
msk_sis730      db      0x6B, 0x01, 0             ; 730, 733
nb_sis730       db      "SiS 730", 0
nb_sis733       db      "SiS 733", 0
msk_sis735      db      0x6A, 0x01, 0             ; 735, 740, 745, 755
nb_sis735       db      "SiS 735", 0
nb_sis740       db      "SiS 740", 0
nb_sis745       db      "SiS 745", 0
nb_sis755       db      "SiS 755", 0
msk_sis746      db      0x6C, 0x01, 0             ; 741, 746, 748
nb_sis741       db      "SiS 741/741GX/M741", 0
nb_sis746       db      "SiS 746/746DX/746FX", 0
nb_sis748       db      "SiS 748", 0

; --- VIA ---
msk_viakt133    db      0x52, 0x80, 0             ; KT133, KX133, KLE133
nb_viakt133     db      "VIA KT133(A)/KM133/KL133/KN133", 0
nb_viakx133     db      "VIA KX133", 0
nb_viakle133    db      "VIA KLE133", 0
msk_viakt266    db      0x92, 0x80, 0x95, 0x02, 0 ; KT266, KM266, KN266
nb_viakt266     db      "VIA KT266(A)/KT333", 0
nb_viakm266     db      "VIA KM266(A)/KL266/KM333", 0
nb_viakn266     db      "VIA KN266", 0
msk_viakt400    db      0xD2, 0x80, 0xD5, 0x02, 0 ; KT400, KM400
nb_viakt400     db      "VIA KT400(A)/KT600", 0
nb_viakm400     db      "VIA KM400(A)", 0
msk_viakt880    db      0x82, 0x80, 0x85, 0x02, 0 ; KT880
nb_viakt880     db      "VIA KT880", 0

; entry: dd pci_id, dw ->name, dw ->mask  (8 bytes)
bd_table:
        ; AMD
        dd      0x70061022              ; AMD 751
        dw      nb_amd751
        dw      msk_amd751

        dd      0x700E1022              ; AMD 761
        dw      nb_amd761
        dw      msk_amd751

        dd      0x700C1022              ; AMD 762
        dw      nb_amd762
        dw      msk_amd762

        ; nVIDIA
        dd      0x01A410DE              ; nForce
        dw      nb_nforce
        dw      msk_nforce

        dd      0x01E010DE              ; nForce 2
        dw      nb_nforce2
        dw      msk_nforce2

        ; SiS
        dd      0x07301039              ; SiS 730
        dw      nb_sis730
        dw      msk_sis730

        dd      0x07331039              ; SiS 733
        dw      nb_sis733
        dw      msk_sis730

        dd      0x07351039              ; SiS 735
        dw      nb_sis735
        dw      msk_sis735

        dd      0x07401039              ; SiS 740
        dw      nb_sis740
        dw      msk_sis735

        dd      0x07411039              ; SiS 741/741GX/M741
        dw      nb_sis741
        dw      msk_sis746

        dd      0x07451039              ; SiS 745
        dw      nb_sis745
        dw      msk_sis735

        dd      0x07461039              ; SiS 746/746DX/746FX
        dw      nb_sis746
        dw      msk_sis746

        dd      0x07481039              ; SiS 748
        dw      nb_sis748
        dw      msk_sis746

        dd      0x07551039              ; SiS 755
        dw      nb_sis755
        dw      msk_sis735

        ; VIA
        dd      0x03051106              ; VIA KT133(A)/KM133/KL133/KN133
        dw      nb_viakt133
        dw      msk_viakt133

        dd      0x03911106              ; VIA KX133
        dw      nb_viakx133
        dw      msk_viakt133

        dd      0x06911106              ; VIA KX133
        dw      nb_viakx133
        dw      msk_viakt133

        dd      0x31121106              ; VIA KLE133
        dw      nb_viakle133
        dw      msk_viakt133

        dd      0x30991106              ; VIA KT266(A)/KT333
        dw      nb_viakt266
        dw      msk_viakt266

        dd      0x31161106              ; VIA KM266(A)/KL266/KM333
        dw      nb_viakm266
        dw      msk_viakt266

        dd      0x31561106              ; VIA KN266
        dw      nb_viakn266
        dw      msk_viakt266

        dd      0x31891106              ; VIA KT400(A)/KT600
        dw      nb_viakt400
        dw      msk_viakt400

        dd      0x32051106              ; VIA KM400(A)
        dw      nb_viakm400
        dw      msk_viakt400

        dd      0x22691106              ; VIA KT880
        dw      nb_viakt880
        dw      msk_viakt880
bd_table_end:

; ====================================================================

;                         x8   x16   x32   x64  x128  x256  x512
div_hdd:        db      0x23, 0x27, 0x2B, 0x2F, 0x63, 0x67, 0x6B ; Halt Disconnect Divisor
div_sgd:        db      0x12, 0x52, 0x92, 0xD2, 0x12, 0x52, 0x92 ; Stop Grand Divisor
; low word of 0xC001001B MSR
; HDD\SGD     8     16     32     64       128    256    512
; 8        0x1223 0x5223 0x9223 0xD223 | 0x1223 0x5223 0x9223
; 16       0x1227                      |
; 32       0x122B                      |
; 64       0x122F                      |      0x522F
; 128      0x1263                      |
; 256      0x1267   & bit 18 is clear  |  & bit 18 is set
; 512      0x126B                      |
           ; ^^^^
           ; ||||_HDD
           ; ||_SGD

; ====================================================================

notify_info:
        dd      SSF_START_APP
        dd      0
        dd      err_msg     ; @notify command line
        dd      0
        dd      0
        db      "/sys/@notify", 0

; ====================================================================

msg_divs        db ' x8  x16  x32 x64 x128 x256 x512', 0
msg_hdd         db 'Hatl Disc.', 0
msg_sgd         db 'Stop Grand', 0
msg_nb          db 'North bridge :', 0
msg_stat        db 'Status :', 0

if lang eq it_IT
        title   db "Disconnessione bus 2.0", 0

        err_msg db "'KBD\nNorthbridge supportato non trovato' -tdE", 0
        
        msg_en  db "Abilitato", 0
        msg_dis db "Disabilitato", 0
        msg_nf  db "Non trovato", 0

else if lang eq ru_RU
        title   cp866 "Отключение шины 2.0", 0
        
        err_msg cp866 "'KBD\nПоддерживаемый северный мост не найден' -tdE", 0
        
        msg_en  cp866 "Включено", 0
        msg_dis cp866 "Выключено", 0
        msg_nf  cp866 "Не найдено", 0

else if lang eq es_ES
        title   db "Desconexion del bus 2.0", 0
        
        err_msg db "'KBD\nPuente norte compatible no encontrado' -tdE", 0
        
        msg_en  db "Activado", 0
        msg_dis db "Desactivado", 0
        msg_nf  db "No encontrado", 0

else ;  lang eq en_US
        title   db "Bus Disconnect 2.0", 0
        
        err_msg db "'KBD\nSupported north bridge not found' -tdE", 0
        
        msg_en  db "Enabled", 0
        msg_dis db "Disabled", 0
        msg_nf  db "Not found", 0

end if

; ====================================================================

I_END:

sc      system_colors
val_hdd dd ?
val_sgd dd ?

PARAMS  rb 1024     ; command line buffer (I_Param)
        rb 2048     ; stack
align 16
STACKTOP:
MEM:

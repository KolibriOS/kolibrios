;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2022. All rights reserved. ;;
;; Copyright (C) MenuetOS 2000-2004 Ville Mikael Turjanmaa      ;;
;; Distributed under terms of the GNU General Public License    ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


VKEY_LSHIFT   = 00000000_00000001b
VKEY_RSHIFT   = 00000000_00000010b
VKEY_LCONTROL = 00000000_00000100b
VKEY_RCONTROL = 00000000_00001000b
VKEY_LALT     = 00000000_00010000b
VKEY_RALT     = 00000000_00100000b
VKEY_CAPSLOCK = 00000000_01000000b
VKEY_NUMLOCK  = 00000000_10000000b
VKEY_SCRLOCK  = 00000001_00000000b
VKEY_LWIN     = 00000010_00000000b
VKEY_RWIN     = 00000100_00000000b

VKEY_SHIFT    = VKEY_LSHIFT + VKEY_RSHIFT
VKEY_CONTROL  = VKEY_LCONTROL + VKEY_RCONTROL
VKEY_ALT      = VKEY_LALT + VKEY_RALT

uglobal
  align 4
  kb_state      dd 0
  ext_code      db 0

  keyboard_mode db 0
  keyboard_data db 0

  altmouseb     db 0
  ctrl_alt_del  db 0

  kb_lights     db 0
  old_kb_lights db 0

align 4
        hotkey_scancodes        rd      256     ; we have 256 scancodes
        hotkey_list             rd      256*4   ; max 256 defined hotkeys
        hotkey_buffer           rd      120*2   ; buffer for 120 hotkeys
endg

iglobal
hotkey_tests    dd      hotkey_test0
                dd      hotkey_test1
                dd      hotkey_test2
                dd      hotkey_test3
                dd      hotkey_test4
hotkey_tests_num = 5
endg
;---------------------------------------------------------------------
hotkey_test0:
        test    al, al
        setz    al
        ret
;---------------------------------------------------------------------
hotkey_test1:
        test    al, al
        setnp   al
        ret
;---------------------------------------------------------------------
hotkey_test2:
        cmp     al, 3
        setz    al
        ret
;---------------------------------------------------------------------
hotkey_test3:
        cmp     al, 1
        setz    al
        ret
;---------------------------------------------------------------------
hotkey_test4:
        cmp     al, 2
        setz    al
        ret
;---------------------------------------------------------------------
hotkey_do_test:
        push    eax
        mov     edx, [kb_state]
        shr     edx, cl
        add     cl, cl
        mov     eax, [eax+4]
        shr     eax, cl
        and     eax, 15
        cmp     al, hotkey_tests_num
        jae     .fail
        
        xchg    eax, edx
        and     al, 3
        call    [hotkey_tests + edx*4]
        cmp     al, 1
        pop     eax
        ret
;--------------------------------------
.fail:
        stc
        pop     eax
        ret
;---------------------------------------------------------------------
align 4
; @brief Export function - Add new scancode in buffer
; @param ecx - scancode
; @return  not return
set_keyboard_data:
        movzx   eax, word[thread_count]; top window process
        movzx   eax, word[WIN_POS + eax*2]
        shl     eax, BSF sizeof.APPDATA
        mov     al, [SLOT_BASE + eax + APPDATA.keyboard_mode]
        mov     [keyboard_mode], al
        
        mov     eax, ecx
        
        push    ebx esi edi ebp
        call    send_scancode
        pop     ebp edi esi ebx
        ret
;---------------------------------------------------------------------
struct  KEYBOARD
        next           dd      ?
        prev           dd      ?
        functions      dd      ?
        userdata       dd      ?
ends

struct  KBDFUNC
        strucsize      dd      ?
        close          dd      ?
        setlights      dd      ?
ends

iglobal
keyboards:
        dd      keyboards
        dd      keyboards
endg
uglobal
keyboard_list_mutex     MUTEX
endg

; @brief Export function - Registration new keyboard
; @param [esp + 4] - pointer on KBDFUNC this keyboard
; @param [esp + 8] - userdata for callback function
; @return  eax = pointer on KEYBOARD structure or 0 on error
register_keyboard:
        push    ebx
        movi    eax, sizeof.KEYBOARD
        call    malloc
        test    eax, eax
        jz      .nothing
        mov     ecx, [esp+4+4]
        mov     [eax + KEYBOARD.functions], ecx
        mov     ecx, [esp+8+4]
        mov     [eax + KEYBOARD.userdata], ecx
        xchg    eax, ebx
        mov     ecx, keyboard_list_mutex
        call    mutex_lock
        mov     ecx, keyboards
        mov     edx, [ecx + KEYBOARD.prev]
        mov     [ebx + KEYBOARD.next], ecx
        mov     [ebx + KEYBOARD.prev], edx
        mov     [edx + KEYBOARD.next], ebx
        mov     [ecx + KEYBOARD.prev], ebx
        mov     ecx, [ebx + KEYBOARD.functions]
        cmp     [ecx + KBDFUNC.strucsize], KBDFUNC.setlights
        jbe     .unlock
        mov     ecx, [ecx + KBDFUNC.setlights]
        test    ecx, ecx
        jz      .unlock
        stdcall ecx, [ebx + KEYBOARD.userdata], dword [kb_lights]
.unlock:
        mov     ecx, keyboard_list_mutex
        call    mutex_unlock
        xchg    eax, ebx
.nothing:
        pop     ebx
        ret     8

; @brief Export function - Delete keyboard
; @param [esp + 4] -  pointer on KEYBOARD structure
; @return  not return
delete_keyboard:
        push    ebx
        mov     ebx, [esp+4+4]
        mov     ecx, keyboard_list_mutex
        call    mutex_lock
        mov     eax, [ebx + KEYBOARD.next]
        mov     edx, [ebx + KEYBOARD.prev]
        mov     [eax + KEYBOARD.prev], edx
        mov     [edx + KEYBOARD.next], eax
        call    mutex_unlock
        mov     ecx, [ebx + KEYBOARD.functions]
        cmp     [ecx + KBDFUNC.strucsize], KBDFUNC.close
        jbe     .nothing
        mov     ecx, [ecx + KBDFUNC.close]
        test    ecx, ecx
        jz      .nothing
        stdcall ecx, [ebx + KEYBOARD.userdata]
.nothing:
        pop     ebx
        ret     4
;---------------------------------------------------------------------
align 4
irq1:
        movzx   eax, word[thread_count]; top window process
        movzx   eax, word[WIN_POS + eax*2]
        shl     eax, BSF sizeof.APPDATA
        mov     al, [SLOT_BASE + eax + APPDATA.keyboard_mode]
        mov     [keyboard_mode], al
        
        in      al, 0x60
;--------------------------------------
send_scancode:
        ;DEBUGF  1, "K : Scan code: %x \n", al
        mov     [keyboard_data], al
; ch = scancode
; cl = ext_code
; bh = 0 - normal key
; bh = 1 - modifier (Shift/Ctrl/Alt)
; bh = 2 - extended code
        mov     ch, al
        cmp     al, 0xE0
        je      @f
        
        cmp     al, 0xE1
        jne     .normal_code
@@:
        mov     bh, 2
        mov     [ext_code], al
        jmp     .writekey
;--------------------------------------
.normal_code:
        mov     cl, 0
        xchg    cl, [ext_code]
        and     al, 0x7F
        mov     bh, 1
;--------------------------------------
@@:
        cmp     al, 0x5B
        jne     @f
        
        cmp     cl, 0xE0
        jne     @f
        
        mov     eax, VKEY_LWIN
        mov     bh, 0
        jmp     .modifier
;--------------------------------------
@@:
        cmp     al, 0x5C
        jne     @f
        
        cmp     cl, 0xE0
        jne     @f
        
        mov     eax, VKEY_RWIN
        mov     bh, 0
        jmp     .modifier
;--------------------------------------
@@:
        cmp     al, 0x2A
        jne     @f
        
        cmp     cl, 0xE0
        je      .writekey
        
        mov     eax, VKEY_LSHIFT
        jmp     .modifier
;--------------------------------------
@@:
        cmp     al, 0x36
        jne     @f
        
        cmp     cl, 0xE0
        je      .writekey
        
        mov     eax, VKEY_RSHIFT
        jmp     .modifier
;--------------------------------------
@@:
        cmp     al, 0x38
        jne     @f
        
        mov     eax, VKEY_LALT
        test    cl, cl
        jz      .modifier
        
        mov     al, VKEY_RALT
        jmp     .modifier
;--------------------------------------
@@:
        cmp     al, 0x1D
        jne     @f
        
        mov     eax, VKEY_LCONTROL
        test    cl, cl
        jz      .modifier
        
        mov     al, VKEY_RCONTROL
        cmp     cl, 0xE0
        jz      .modifier
        
        mov     [ext_code], cl
        jmp     .writekey
;--------------------------------------
@@:
        cmp     al, 0x3A
        jne     @f
        
        mov     bl, 4
        mov     eax, VKEY_CAPSLOCK
        jmp     .no_key.xor
;--------------------------------------
@@:
        cmp     al, 0x45
        jne     @f
        test    cl, cl
        jnz     .writekey
        
        mov     bl, 2
        mov     eax, VKEY_NUMLOCK
        jmp     .no_key.xor
;--------------------------------------
@@:
        cmp     al, 0x46
        jne     @f
        
        mov     bl, 1
        mov     eax, VKEY_SCRLOCK
        jmp     .no_key.xor
;--------------------------------------
@@:
        xor     ebx, ebx
        test    ch, ch
        js      .writekey
        
        movzx   eax, ch          ; plain key
        mov     bl, [keymap + eax]
        mov     edx, [kb_state]
        test    dl, VKEY_CONTROL ; ctrl alt del
        jz      .noctrlaltdel
        
        test    dl, VKEY_ALT
        jz      .noctrlaltdel
        
        cmp     ch, 53h
        jne     .noctrlaltdel
        
        mov     [ctrl_alt_del], 1
        call    wakeup_osloop
.noctrlaltdel:
        test    dl, VKEY_CONTROL ; ctrl on ?
        jz      @f
        
        sub     bl, 0x60
@@:
        test    dl, VKEY_CAPSLOCK        ; caps lock on ?
        jz      .no_caps_lock
        
        test    dl, VKEY_SHIFT   ; shift on ?
        jz      .keymap_shif
        
        jmp     @f
;--------------------------------------
.no_caps_lock:
        test    dl, VKEY_SHIFT   ; shift on ?
        jz      @f
.keymap_shif:   
        mov     bl, [keymap_shift + eax]
@@:
        test    dl, VKEY_ALT     ; alt on ?
        jz      @f
        
        mov     bl, [keymap_alt + eax]
@@:
        jmp     .writekey
;--------------------------------------
.modifier:
        test    ch, ch
        js      .modifier.up
        or      [kb_state], eax
        jmp     .writekey
;--------------------------------------
.modifier.up:
        not     eax
        and     [kb_state], eax
        jmp     .writekey
;--------------------------------------
.no_key.xor:
        mov     bh, 0
        test    ch, ch
        js      .writekey
        
        xor     [kb_state], eax
        xor     [kb_lights], bl
.writekey:
        pushad
; test for system hotkeys
        movzx   eax, ch
        cmp     bh, 1
        ja      .nohotkey
        jb      @f
        
        xor     eax, eax
@@:
        mov     eax, [hotkey_scancodes + eax*4]
.hotkey_loop:
        test    eax, eax
        jz      .nohotkey
        
        mov     cl, 0
        call    hotkey_do_test
        jc      .hotkey_cont
        
        mov     cl, 2
        call    hotkey_do_test
        jc      .hotkey_cont
        
        mov     cl, 4
        call    hotkey_do_test
        jnc     .hotkey_found
.hotkey_cont:
        mov     eax, [eax]
        jmp     .hotkey_loop
;--------------------------------------
.hotkey_found:
        mov     eax, [eax+8]
; put key in buffer for process in slot eax
        mov     edi, hotkey_buffer
@@:
        cmp     dword [edi], 0
        jz      .found_free
        
        add     edi, 8
        cmp     edi, hotkey_buffer+120*8
        jb      @b
; no free space - replace first entry
        mov     edi, hotkey_buffer
.found_free:
        mov     [edi], eax
        movzx   eax, ch
        cmp     bh, 1
        jnz     @f
        
        xor     eax, eax
@@:
        mov     [edi+4], ax
        mov     eax, [kb_state]
        mov     [edi+6], ax

        cmp     [PID_lock_input], dword 0
        je      .nohotkey

        popad
        jmp     .exit.irq1
;--------------------------------------
.nohotkey:
        popad

        cmp     [keyboard_mode], 0; return from keymap
        jne     .scancode
        
        test    bh, bh
        jnz     .exit.irq1
        
        test    bl, bl
        jz      .exit.irq1

        cmp     cl, 0xE0        ; extended keycode
        jne     @f

        cmp     ch, 53
        jne     .dowrite
        
        mov     bl, '/'
        jmp     .dowrite
@@:

        cmp     ch, 55
        jne     @f
        
        mov     bl, '*'
        jmp     .dowrite
@@:

        cmp     ch, 74
        jne     @f
        
        mov     bl, '-'
        jmp     .dowrite
@@:

        cmp     ch, 78
        jne     @f
        
        mov     bl, '+'
        jmp     .dowrite
@@:

        test    [kb_state], VKEY_NUMLOCK
        jz      .dowrite

        cmp     ch, 71
        jb      .dowrite
        
        cmp     ch, 83
        ja      .dowrite
        
        movzx   eax, ch
        mov     bl, [numlock_map + eax - 71]
        jmp     .dowrite
;--------------------------------------
.scancode:
        mov     bl, ch
.dowrite:
        movzx   eax, byte[KEY_COUNT]
        cmp     al, 120
        jae     .exit.irq1
        inc     eax
        mov     [KEY_COUNT], al
; store ascii or scancode
        mov     [KEY_BUFF + eax -1], bl
; store original scancode
        add     eax, 120+2
        push    ecx
        cmp     [keyboard_mode], 0; return from keymap
        je      @f

        xor     ch, ch
@@:
        mov     [KEY_BUFF + eax -1], ch
        pop     ecx
        sub     eax, 120+2
.exit.irq1:
        ret
;---------------------------------------------------------------------
set_lights:
        push    ebx esi
        mov     ecx, keyboard_list_mutex
        call    mutex_lock
        mov     esi, keyboards
.loop:
        mov     esi, [esi + KEYBOARD.next]
        cmp     esi, keyboards
        jz      .done
        mov     eax, [esi + KEYBOARD.functions]
        cmp     dword [eax], KBDFUNC.setlights
        jbe     .loop
        mov     eax, [eax + KBDFUNC.setlights]
        test    eax, eax
        jz      .loop
        stdcall eax, [esi + KEYBOARD.userdata], dword [kb_lights]
        jmp     .loop
.done:
        mov     ecx, keyboard_list_mutex
        call    mutex_unlock
        pop     esi ebx
        ret

ps2_set_lights:
        stdcall disable_irq, 1
        mov     al, 0xED
        call    kb_write_wait_ack
        mov     al, [esp+8]
        call    kb_write_wait_ack
        stdcall enable_irq, 1
        ret     8

;// mike.dld ]
proc check_lights_state_has_work?
        mov     al, [kb_lights]
        cmp     al, [old_kb_lights]
        ret
endp

check_lights_state:
        call    check_lights_state_has_work?
        jz      .nothing
        mov     [old_kb_lights], al
        call    set_lights
.nothing:
        ret
;---------------------------------------------------------------------
iglobal
numlock_map db '789-456+1230.'
endg
;---------------------------------------------------------------------
align 4
kb_write_wait_ack:

        push    ecx edx

        mov     dl, al
        mov     ecx, 0x1ffff; last 0xffff, new value in view of fast CPU's
.wait_output_ready:
        in      al, 0x64
        test    al, 2
        jz      @f
        loop    .wait_output_ready
        mov     ah, 1
        jmp     .nothing
@@:
        mov     al, dl
        out     0x60, al
        mov     ecx, 0xfffff; last 0xffff, new value in view of fast CPU's
.wait_ack:
        in      al, 0x64
        test    al, 1
        jnz     @f
        loop    .wait_ack
        mov     ah, 1
        jmp     .nothing
@@:
        in      al, 0x60
        xor     ah, ah

.nothing:
        pop     edx ecx
        ret
;-----------------------------------------------------------------------------

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; 66 sys function.                                                ;;
;; in eax=66,ebx in [0..5],ecx,edx                                 ;;
;; out eax                                                         ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
iglobal
align 4
f66call:
           dd sys_process_def.1   ; 1 = set keyboard mode
           dd sys_process_def.2   ; 2 = get keyboard mode
           dd sys_process_def.3   ; 3 = get keyboard ctrl, alt, shift
           dd sys_process_def.4   ; 4 = set system-wide hotkey
           dd sys_process_def.5   ; 5 = delete installed hotkey
           dd sys_process_def.6   ; 6 = disable input, work only hotkeys
           dd sys_process_def.7   ; 7 = enable input, opposition to f.66.6
endg
;-----------------------------------------------------------------------------
align 4
sys_process_def:
        dec     ebx
        cmp     ebx, 7
        jae     .not_support    ;if >=8 then or eax,-1

        mov     edi, [current_slot]
        jmp     dword [f66call + ebx*4]

.not_support:
        or      [esp + SYSCALL_STACK.eax], -1
        ret
;-----------------------------------------------------------------------------
align 4
.1:
        mov     [edi + APPDATA.keyboard_mode], cl
        ret
;-----------------------------------------------------------------------------
align 4
.2:                             ; 2 = get keyboard mode
        movzx   eax, byte [edi + APPDATA.keyboard_mode]
        mov     [esp + SYSCALL_STACK.eax], eax
        ret
;-----------------------------------------------------------------------------
align 4
.3:                             ;3 = get keyboard ctrl, alt, shift
        mov     eax, [kb_state]
        mov     [esp + SYSCALL_STACK.eax], eax
        ret
;-----------------------------------------------------------------------------
align 4
.4:
        mov     edi, [current_slot_idx]
        mov     eax, hotkey_list
@@:
        cmp     dword [eax + 8], 0
        jz      .found_free
        add     eax, 16
        cmp     eax, hotkey_list+16*256
        jb      @b
        mov     dword [esp + SYSCALL_STACK.eax], 1
        ret
.found_free:
        mov     [eax + 8], edi
        mov     [eax + 4], edx
        movzx   ecx, cl
        lea     ecx, [hotkey_scancodes+ecx*4]
        mov     edx, [ecx]
        mov     [eax], edx
        mov     [ecx], eax
        mov     [eax + 12], ecx
        test    edx, edx
        jz      @f
        mov     [edx + 12], eax
@@:
        and     dword [esp + SYSCALL_STACK.eax], 0
        ret
;-----------------------------------------------------------------------------
align 4
.5:
        mov     edi, [current_slot_idx]
        movzx   ebx, cl
        lea     ebx, [hotkey_scancodes+ebx*4]
        mov     eax, [ebx]
.scan:
        test    eax, eax
        jz      .notfound
        cmp     [eax + 8], edi
        jnz     .next
        cmp     [eax + 4], edx
        jz      .found
.next:
        mov     eax, [eax]
        jmp     .scan
.notfound:
        mov     dword [esp + SYSCALL_STACK.eax], 1
        ret
.found:
        mov     ecx, [eax]
        jecxz   @f
        mov     edx, [eax + 12]
        mov     [ecx + 12], edx
@@:
        mov     ecx, [eax + 12]
        mov     edx, [eax]
        mov     [ecx], edx
        xor     edx, edx
        mov     [eax + 4], edx
        mov     [eax + 8], edx
        mov     [eax + 12], edx
        mov     [eax], edx
        mov     [esp + SYSCALL_STACK.eax], edx
        ret
;-----------------------------------------------------------------------------
align 4
.6:
        pushfd
        cli
        mov     eax, [PID_lock_input]
        test    eax, eax
        jnz     @f
; get current PID
        mov     eax, [current_slot]
        mov     eax, [eax + APPDATA.tid]
; set current PID for lock input
        mov     [PID_lock_input], eax
@@:
        popfd
        ret
;-----------------------------------------------------------------------------
align 4
.7:
        mov     eax, [PID_lock_input]
        test    eax, eax
        jz      @f
; get current PID
        mov     ebx, [current_slot]
        mov     ebx, [ebx + APPDATA.tid]
; compare current lock input with current PID
        cmp     ebx, eax
        jne     @f

        xor     eax, eax
        mov     [PID_lock_input], eax
@@:
        ret
;-----------------------------------------------------------------------------
uglobal
  PID_lock_input dd 0x0
endg
;-----------------------------------------------------------------------------
align 4
; @brief System function 2 - Get pressed key
; @param eax = 2- number function
; @return  eax = 1 - buffer empty, else
;          al = 0, ah = code pressed key,
;                  16-23 bits - scancode pressed key(in ASCII mode)
;          if al=2 ah=scancode pressed key, 16-31 bits - state control keys
sys_getkey:
        mov     [esp + SYSCALL_STACK.eax], dword 1
        ; test main buffer
        mov     ebx, [current_slot_idx]                          ; TOP OF WINDOW STACK
        movzx   ecx, word [WIN_STACK + ebx * 2]
        mov     edx, [thread_count]
        cmp     ecx, edx
        jne     .finish
        cmp     [KEY_COUNT], byte 0
        je      .finish
        movzx   ax, byte [KEY_BUFF + 120 + 2]
        shl     eax, 8
        mov     al, byte [KEY_BUFF]
        shl     eax, 8
        push    eax
        dec     byte [KEY_COUNT]
        and     byte [KEY_COUNT], 127
        movzx   ecx, byte [KEY_COUNT]
        add     ecx, 2
        mov     eax, KEY_BUFF + 1
        mov     ebx, KEY_BUFF
        call    memmove
        add     eax, 120 + 2
        add     ebx, 120 + 2
        call    memmove
        pop     eax
;--------------------------------------
align 4
.ret_eax:
        mov     [esp + SYSCALL_STACK.eax], eax
        ret
;--------------------------------------
align 4
.finish:
; test hotkeys buffer
        mov     ecx, hotkey_buffer
;--------------------------------------
align 4
@@:
        cmp     [ecx], ebx
        jz      .found
        add     ecx, 8
        cmp     ecx, hotkey_buffer + 120 * 8
        jb      @b
        ret
;--------------------------------------
align 4
.found:
        mov     ax, [ecx + 6]
        shl     eax, 16
        mov     ah, [ecx + 4]
        mov     al, 2
        and     dword [ecx + 4], 0
        and     dword [ecx], 0
        jmp     .ret_eax
;------------------------------------------------------------------------------
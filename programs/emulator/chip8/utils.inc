; note: proc that defines without stdcall, call using "call"
; note: by stdcall convention callee is responsible for cleaning up the stack,
; arguments are pushed onto the stack in right-to-left order
align 4
proc _memset stdcall, dest:dword, val:byte, cnt:dword ; doesnt clobber any registers
        ;DEBUGF  DBG_INFO, "memset(%x, %u, %u)\n", [dest], [val], [cnt]
        push    eax ecx edi
        mov     edi, dword [dest]
        mov     al,  byte [val]
        mov     ecx, dword [cnt]
        rep     stosb   
        pop     edi ecx eax
        ret 
endp

align 4
proc _srand stdcall, seed:dword
        push    eax
        ;DEBUGF  DBG_INFO, "_srand: next_rand = %u\n", [next_rand]
        mov     eax, dword [seed]
        mov     dword [next_rand], eax
        pop     eax
        ret
endp

align 4
proc _rand stdcall
        push    ebx edx
        ;DEBUGF  DBG_INFO, "_rand: next_rand = %u\n", [next_rand]
        mov     eax, dword [next_rand]
        mov     ebx, 214013
        mul     ebx
        add     eax, 2531011
        mov     dword [next_rand], eax
        shr     eax, 16
        and     eax, 0x7fff
        pop     edx ebx
        ret
endp

align 4
proc _getseed stdcall
        push    edx
        rdtsc
        xor     eax, edx
        pop     edx
        ret
endp

; calculate (num % num_mod) / num_div
align 4
proc mod_div stdcall, num: dword, num_mod: dword, num_div:dword ; TODO check
; in:  num - number
;      num_mod - first divisor
;      num_div - second divisor
; out: eax - the result
; destroys: mb flags
        push    ecx edx
        xor     edx, edx
        mov     eax, dword [num]
        mov     ecx, dword [num_mod]
        div     ecx
        
        mov     eax, edx
        xor     edx, edx
        mov     ecx, dword [num_div]
        div     ecx
        pop     edx ecx
        ret
endp

; update key map; TODO impl;
align 4
proc keyboard_update stdcall, _code: dword
        push    eax ebx ecx edx
        mov     eax, dword [_code]

        mov     edx, eax
        and     edx, 0xF000

        mov     ebx, eax
        and     ebx, 0x0F00
        shr     ebx, 8

        xor     ecx, ecx
        cmp     edx, 0x6000
        jne     .if1_end
        mov     ecx, 0x9
.if1_end:
        add     ebx, ecx
        ; DEBUGF  DBG_INFO, "keynum %x\n", ebx
        cmp     ebx, KEY_SIZE
        jae     .ret
        DEBUGF  DBG_INFO, "keynum %x\n", ebx
        mov     byte [key + ebx], 1
        
.ret:
        pop     edx ecx ebx eax
        ret
endp

; ; get seconds count
; align 4
; proc get_clock stdcall
; ; out: eax - time from system start in 10^-2 secs
;         push    ebx
;         mov     eax, 26
;         mov     ebx, 9
;         int     0x40
;         pop     ebx
;         ret
; endp

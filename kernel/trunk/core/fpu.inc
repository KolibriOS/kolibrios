;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2022. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


init_fpu:
        clts
        fninit

        bt      [cpu_caps+(CAPS_XSAVE/32)*4], CAPS_XSAVE mod 32
        jnc     .no_xsave

        mov     ecx, cr4
        or      ecx, CR4_OSXSAVE
        mov     cr4, ecx
        ; don't call cpuid again
        bts     [cpu_caps+(CAPS_OSXSAVE/32)*4], CAPS_OSXSAVE mod 32

        ; zero xsave header
        mov     ecx, 64/4
        xor     eax, eax
        mov     edi, fpu_data + 512     ; skip legacy region
        rep stosd

        mov     eax, 0x0d       ; extended state enumeration main leaf
        xor     ecx, ecx
        cpuid
        and     eax, XCR0_FPU_MMX + XCR0_SSE + XCR0_AVX + XCR0_AVX512
        xor     edx, edx
        mov     [xsave_eax], eax
        mov     [xsave_edx], edx
        xor     ecx, ecx
        xsetbv

        mov     eax, 0x0d
        xor     ecx, ecx
        cpuid
        add     ebx, 63
        and     ebx, NOT 63
        mov     [xsave_area_size], ebx
        cmp     ebx, fpu_data_size
        ja      $

        test    eax, XCR0_AVX512
        jz      @f
        call    init_avx512
        mov     eax, [xsave_eax]
        mov     edx, [xsave_edx]
        xsave   [fpu_data]
        ret
@@:
        test    eax, XCR0_AVX
        jz      @f
        call    init_avx
        mov     eax, [xsave_eax]
        mov     edx, [xsave_edx]
        xsave   [fpu_data]
        ret
@@:
        test    eax, XCR0_SSE
        jz      $
        call    init_sse
        mov     eax, [xsave_eax]
        mov     edx, [xsave_edx]
        xsave   [fpu_data]
        ret
.no_xsave:
        mov     [xsave_area_size], 512  ; enough for FPU/MMX and SSE
        bt      [cpu_caps], CAPS_SSE
        jnc     .fpu_mmx
.sse:
        call    init_sse
        fxsave  [fpu_data]
        ret
.fpu_mmx:
        call    init_fpu_mmx
        fnsave  [fpu_data]
        ret

init_fpu_mmx:
        mov     ecx, cr0
        and     ecx, not CR0_EM
        or      ecx, CR0_MP + CR0_NE
        mov     cr0, ecx
        ret

init_sse:
        mov     ebx, cr4
        mov     ecx, cr0
        or      ebx, CR4_OSFXSR + CR4_OSXMMEXPT
        mov     cr4, ebx

        and     ecx, not (CR0_EM + CR0_MP)
        or      ecx, CR0_NE
        mov     cr0, ecx

        mov     dword [esp-4], MXCSR_INIT
        ldmxcsr [esp-4]

        xorps   xmm0, xmm0
        xorps   xmm1, xmm1
        xorps   xmm2, xmm2
        xorps   xmm3, xmm3
        xorps   xmm4, xmm4
        xorps   xmm5, xmm5
        xorps   xmm6, xmm6
        xorps   xmm7, xmm7
        ret

init_avx:
        mov     ebx, cr4
        or      ebx, CR4_OSFXSR + CR4_OSXMMEXPT
        mov     cr4, ebx

        mov     ecx, cr0
        and     ecx, not (CR0_EM + CR0_MP)
        or      ecx, CR0_NE
        mov     cr0, ecx

        mov     dword [esp-4], MXCSR_INIT
        vldmxcsr [esp-4]

        vzeroall
        ret

init_avx512:
        mov     ebx, cr4
        or      ebx, CR4_OSFXSR + CR4_OSXMMEXPT
        mov     cr4, ebx

        mov     ecx, cr0
        and     ecx, not (CR0_EM + CR0_MP)
        or      ecx, CR0_NE
        mov     cr0, ecx

        mov     dword [esp-4], MXCSR_INIT
        vldmxcsr [esp-4]

        vpxorq  zmm0, zmm0, zmm0
        vpxorq  zmm1, zmm1, zmm1
        vpxorq  zmm2, zmm2, zmm2
        vpxorq  zmm3, zmm3, zmm3
        vpxorq  zmm4, zmm4, zmm4
        vpxorq  zmm5, zmm5, zmm5
        vpxorq  zmm6, zmm6, zmm6
        vpxorq  zmm7, zmm7, zmm7

        ret

; param
;  eax= 512 bytes memory area aligned on a 16-byte boundary

align 4
fpu_save:
        push    ecx
        push    esi
        push    edi

        pushfd
        cli

        clts
        mov     edi, eax

        mov     ecx, [fpu_owner]
        mov     esi, [current_slot_idx]
        cmp     ecx, esi
        jne     .save

        call    save_fpu_context
        jmp     .exit
.save:
        mov     [fpu_owner], esi

        shl     ecx, BSF sizeof.APPDATA
        mov     eax, [SLOT_BASE + ecx + APPDATA.fpu_state]

        call    save_context

; first 512 bytes of XSAVE area have the same format as FXSAVE
        shl     esi, BSF sizeof.APPDATA
        mov     esi, [SLOT_BASE + esi + APPDATA.fpu_state]
        mov     ecx, 512/4
        cld
        rep movsd
        fninit
.exit:
        popfd
        pop     edi
        pop     esi
        pop     ecx
        ret

avx_save_size:
        mov     eax, [xsave_area_size]
        ret

; param
;  eax= avx_save_size() bytes memory area aligned on a 64-byte boundary

align 4
avx_save:
        push    ecx
        push    esi
        push    edi

        pushfd
        cli

        clts
        mov     edi, eax

        mov     ecx, [fpu_owner]
        mov     esi, [current_slot_idx]
        cmp     ecx, esi
        jne     .save

        call    save_context
        jmp     .exit
.save:
        mov     [fpu_owner], esi

        shl     ecx, BSF sizeof.APPDATA
        mov     eax, [SLOT_BASE + ecx + APPDATA.fpu_state]

        call    save_context

        shl     esi, BSF sizeof.APPDATA
        mov     esi, [SLOT_BASE + esi + APPDATA.fpu_state]
        mov     ecx, [xsave_area_size]
        add     ecx, 3
        shr     ecx, 2
        rep movsd
        fninit
.exit:
        popfd
        pop     edi
        pop     esi
        pop     ecx
        ret

align 4
save_context:
        bt      [cpu_caps + (CAPS_OSXSAVE/32)*4], CAPS_OSXSAVE mod 32
        jnc     save_fpu_context
        push    eax edx
        mov     ecx, eax
        mov     eax, [xsave_eax]
        mov     edx, [xsave_edx]
        xsave   [ecx]
        pop     edx eax
        ret
save_fpu_context:
        bt      [cpu_caps], CAPS_SSE
        jnc     .no_SSE
        fxsave  [eax]
        ret
.no_SSE:
        fnsave  [eax]
        ret


align 4
fpu_restore:
        push    ecx
        push    esi

        mov     esi, eax

        pushfd
        cli

        mov     ecx, [fpu_owner]
        mov     eax, [current_slot_idx]
        cmp     ecx, eax
        jne     .copy

        clts
        bt      [cpu_caps], CAPS_SSE
        jnc     .no_SSE

        fxrstor [esi]
        popfd
        pop     esi
        pop     ecx
        ret
.no_SSE:
        fnclex                  ;fix possible problems
        frstor  [esi]
        popfd
        pop     esi
        pop     ecx
        ret
.copy:
        shl     eax, BSF sizeof.APPDATA
        mov     edi, [SLOT_BASE + eax + APPDATA.fpu_state]
        mov     ecx, 512/4
        cld
        rep movsd
        popfd
        pop     esi
        pop     ecx
        ret

align 4
avx_restore:
        push    ecx
        push    esi

        mov     esi, eax

        pushfd
        cli

        mov     ecx, [fpu_owner]
        mov     eax, [current_slot_idx]
        cmp     ecx, eax
        jne     .copy

        clts
        bt      [cpu_caps + (CAPS_OSXSAVE/32)*4], CAPS_OSXSAVE mod 32
        jnc     .no_xsave
        push    edx
        mov     eax, [xsave_eax]
        mov     edx, [xsave_edx]
        xrstor  [esi]
        pop     edx
        popfd
        pop     esi
        pop     ecx
        ret
.no_xsave:
        bt      [cpu_caps], CAPS_SSE
        jnc     .no_SSE

        fxrstor [esi]
        popfd
        pop     esi
        pop     ecx
        ret
.no_SSE:
        fnclex                  ;fix possible problems
        frstor  [esi]
        popfd
        pop     esi
        pop     ecx
        ret
.copy:
        shl     eax, BSF sizeof.APPDATA
        mov     edi, [SLOT_BASE + eax + APPDATA.fpu_state]
        mov     ecx, [xsave_area_size]
        add     ecx, 3
        shr     ecx, 2
        cld
        rep movsd
        popfd
        pop     esi
        pop     ecx
        ret

align 4
except_7:                  ;#NM exception handler
        save_ring3_context
        clts
        mov     ax, app_data;
        mov     ds, ax
        mov     es, ax

        mov     ebx, [fpu_owner]
        cmp     ebx, [current_slot_idx]
        je      .exit

        shl     ebx, BSF sizeof.APPDATA
        mov     eax, [SLOT_BASE + ebx + APPDATA.fpu_state]
        bt      [cpu_caps + (CAPS_OSXSAVE/32)*4], CAPS_OSXSAVE mod 32
        jnc     .no_xsave
        mov     ecx, eax
        mov     eax, [xsave_eax]
        mov     edx, [xsave_edx]
        xsave   [ecx]
        mov     ebx, [current_slot_idx]
        mov     [fpu_owner], ebx
        shl     ebx, BSF sizeof.APPDATA
        mov     ecx, [SLOT_BASE + ebx + APPDATA.fpu_state]
        xrstor  [ecx]
.exit:
        restore_ring3_context
        iret
.no_xsave:
        bt      [cpu_caps], CAPS_SSE
        jnc     .no_SSE

        fxsave  [eax]
        mov     ebx, [current_slot_idx]
        mov     [fpu_owner], ebx
        shl     ebx, BSF sizeof.APPDATA
        mov     eax, [SLOT_BASE + ebx + APPDATA.fpu_state]
        fxrstor [eax]
        restore_ring3_context
        iret

.no_SSE:
        fnsave  [eax]
        mov     ebx, [current_slot_idx]
        mov     [fpu_owner], ebx
        shl     ebx, BSF sizeof.APPDATA
        mov     eax, [SLOT_BASE + ebx + APPDATA.fpu_state]
        frstor  [eax]
        restore_ring3_context
        iret

iglobal
  fpu_owner dd 2
endg

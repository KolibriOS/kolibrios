struct  RING_BUF
        start_ptr       dd ?   ; Pointer to start of buffer
        end_ptr         dd ?   ; Pointer to end of buffer
        read_ptr        dd ?   ; Read pointer
        write_ptr       dd ?   ; Write pointer
        size            dd ?   ; Size of buffer
ends

;bool __fastcall ring_buf_create(struct RING_BUF *buf, u32 size)
align 4
proc ring_buf_create
        push    ecx
        push    edx
        invoke  CreateRingBuffer, edx, PG_SW
        pop     edx
        pop     ecx
        test    eax, eax
        jz      .exit

        mov     [ecx + RING_BUF.start_ptr], eax
        mov     [ecx + RING_BUF.write_ptr], eax
        mov     [ecx + RING_BUF.read_ptr], eax
        add     eax, edx
        mov     [ecx + RING_BUF.end_ptr], eax
        mov     [ecx + RING_BUF.size], edx
        or      eax, 1
.exit:
        ret
endp

;void __fastcall ring_buf_destroy(struct RING_BUF *buf)
align 4
proc ring_buf_destroy
        xor     eax, eax
        mov     [ecx + RING_BUF.write_ptr], eax
        mov     [ecx + RING_BUF.read_ptr], eax
        mov     [ecx + RING_BUF.end_ptr], eax
        mov     [ecx + RING_BUF.size], eax
        xchg    eax, [ecx + RING_BUF.start_ptr]
        invoke  KernelFree, eax
        ret
endp

;u32 __fastcall ring_buf_count(struct RING_BUF *buf)
align 4
proc ring_buf_count
        mov     eax, [ecx + RING_BUF.write_ptr]
        mov     edx, [ecx + RING_BUF.read_ptr]
        cmp     eax, edx
        jb      @f
        sub     eax, edx
        ret

  @@:
        sub     eax, edx
        add     eax, [ecx + RING_BUF.size]
        ret
endp

;u32 __fastcall ring_buf_free(struct RING_BUF *buf)
align 4
proc ring_buf_free
        mov     eax, [ecx + RING_BUF.read_ptr]
        mov     edx, [ecx + RING_BUF.write_ptr]
        cmp     eax, edx
        jae     @f
        sub     eax, edx
        dec     eax
        ret

  @@:
        sub     eax, edx
        dec     eax
        add     eax, [ecx + RING_BUF.size]
        ret
endp

;u32 __stdcall ring_buf_write(struct RING_BUF *buf, const u32 *src, u32 size)
align 4
proc ring_buf_write stdcall uses esi edi, buf, src, size
        mov     ecx, [buf]
        call    ring_buf_free
        test    eax, eax
        jz      .exit

        mov     esi, [src]
        mov     edi, [ecx + RING_BUF.write_ptr]
        mov     ecx, [size]
        cmp     ecx, eax
        jbe     .copy
        mov     ecx, eax
  .copy:
        mov     eax, ecx
        cld
        shr     ecx, 1
        jnc     .nb
        movsb
  .nb:
        shr     ecx, 1
        jnc     .nw
        movsw
  .nw:
        test    ecx, ecx
        jz      .nd
        rep movsd
  .nd:
        mov     ecx, [buf]
        cmp     edi, [ecx + RING_BUF.end_ptr]
        jb      @f
        sub     edi, [ecx + RING_BUF.size]
  @@:
        mov     [ecx + RING_BUF.write_ptr], edi
  .exit:
        ret
endp

;u32 __stdcall ring_buf_read(struct RING_BUF *buf, u32 *dst, u32 size)
align 4
proc ring_buf_read stdcall uses ebx esi edi, buf, dst, size
        mov     ecx, [buf]
        call    ring_buf_count
        test    eax, eax
        jz      .exit

        mov     esi, [ecx + RING_BUF.read_ptr]
        mov     edi, [dst]
        mov     ecx, [size]
        cmp     ecx, eax
        jbe     .copy
        mov     ecx, eax
  .copy:
        mov     eax, ecx
        shr     ecx, 1
        jnc     .nb
        cld
        movsb
  .nb:
        shr     ecx, 1
        jnc     .nw
        movsw
  .nw:
        test    ecx, ecx
        jz      .nd
        rep movsd
  .nd:
        mov     ecx, [buf]
        cmp     esi, [ecx + RING_BUF.end_ptr]
        jb      .save_ptr
        sub     esi, [ecx + RING_BUF.size]
  .save_ptr:
        mov     [ecx + RING_BUF.read_ptr], esi
  .exit:
        ret
endp

;u32 __fastcall ring_buf_discard(struct RING_BUF *buf, u32 size)
align 4
proc ring_buf_discard
        push    ecx
        push    edx
        call    ring_buf_count
        pop     edx
        pop     ecx

        cmp     eax, edx
        jae     .discard
        mov     edx, eax
  .discard:
        push    edx
        add     edx, [ecx + RING_BUF.read_ptr]
        cmp     edx, [ecx + RING_BUF.end_ptr]
        jb      .save_ptr
        sub     edx, [ecx + RING_BUF.size]
  .save_ptr:
        mov     [ecx + RING_BUF.read_ptr], edx
        pop     eax
        ret
endp

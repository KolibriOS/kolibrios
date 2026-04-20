; Password handling in 7-Zip: "7zAES" filter (SHA256 + AES256).
; Ported from C++ sources of 7-Zip (c) Igor Pavlov.
aes7z_decoder:
virtual at 0
.outStream      rb      streamInfo.size
.inStream       dd      ?
.inLen          dd      ?
.inPtr          dd      ?
.bufRest        dd      ?
; key data
.NumCyclesPower dd      ?
.SaltSize       dd      ?
.Salt           rb      16
; AES data
.iv             rb      16
.Key            rb      32
.nr             dd      ?
.KeyExpanded    rb      32*15
.size = $
end virtual

.fillBuf:
        mov     esi, [eax+.inPtr]
        mov     ebp, eax
        add     edi, [eax+.bufRest]
        sub     ecx, [eax+.bufRest]
        js      .rest1
        and     [eax+.bufRest], 0
.mainloop:
        test    ecx, ecx
        jz      .done
        sub     [ebp+.inLen], 16
        js      .refill
.refilled:
        push    esi edi ecx
        mov     ebx, edi
        lea     edi, [ebp+.nr]
        call    aes_decode
        pop     ecx edi esi
        mov     eax, dword [ebp+.iv]
        xor     [edi], eax
        lodsd
        mov     dword [ebp+.iv], eax
        mov     eax, dword [ebp+.iv+4]
        xor     [edi+4], eax
        lodsd
        mov     dword [ebp+.iv+4], eax
        mov     eax, dword [ebp+.iv+8]
        xor     [edi+8], eax
        lodsd
        mov     dword [ebp+.iv+8], eax
        mov     eax, dword [ebp+.iv+12]
        xor     [edi+12], eax
        lodsd
        mov     dword [ebp+.iv+12], eax
        add     edi, 16
        sub     ecx, 16
        jns     .mainloop
.rest1:
        neg     ecx
        mov     [ebp+.bufRest], ecx
.done:
        mov     [ebp+.inPtr], esi
        popad
        ret

.refill:
        mov     edx, [ebp+.inLen]
        add     edx, 16
        jnz     .rest
        js      return.err
        mov     eax, [ebp+.inStream]
        call    fillBuf
        mov     edx, [eax+streamInfo.bufDataLen]
        test    edx, edx
        jz      return.err
        mov     esi, [eax+streamInfo.bufPtr]
        mov     [ebp+.inLen], edx
        sub     [ebp+.inLen], 16
        jns     .refilled
.rest:
; ASSERT([eax+streamInfo.fullSize] == 0);
        sub     edx, ecx
        jb      return.err
        add     ecx, edx
        rep     movsb
        mov     [ebp+.bufRest], edx
        jmp     .done

aes7z_get_buf_size:
        mov     eax, aes7z_decoder.size
        mov     edx, 0x4000
        ret

aes7z_init_decoder:
; zero all
        xor     eax, eax
        mov     [ebp+aes7z_decoder.inLen], eax
        mov     [ebp+aes7z_decoder.bufRest], eax
        mov     [ebp+aes7z_decoder.NumCyclesPower], eax
        mov     [ebp+aes7z_decoder.SaltSize], eax
        lea     edi, [ebp+aes7z_decoder.Salt]
        push    8
        pop     ecx
        rep     stosd   ; zero .Salt and .iv
        mov     [ebp+streamInfo.fillBuf], aes7z_decoder.fillBuf
; parse parameters
        cmp     dword [esi-4], eax
        jz      .parok  ; no parameters - OK
        lodsb
        mov     cl, al
        and     al, 0x3F
        mov     byte [ebp+aes7z_decoder.NumCyclesPower], al
        test    cl, 0xC0
        jz      .parok
        test    cl, 0x80
        setnz   byte [ebp+aes7z_decoder.SaltSize]
        shr     cl, 6
        and     ecx, 1
        cmp     dword [esi-1-4], 2
        jb      return.err
        lodsb
        mov     dl, al
        shr     al, 4
        add     byte [ebp+aes7z_decoder.SaltSize], al
        and     edx, 0xF
        add     ecx, edx
        lea     edx, [ecx+2]
        push    ecx
        mov     ecx, [ebp+aes7z_decoder.SaltSize]
        add     edx, ecx
        cmp     dword [esi-2-4], edx
        jb      return.err
        lea     edi, [ebp+aes7z_decoder.Salt]
        rep     movsb
        pop     ecx
        lea     edi, [ebp+aes7z_decoder.iv]
        rep     movsb
.parok:
        test    bl, bl
        jnz     .ret    ; if reinitializing - all calculations have been already done
        call    query_password
        jz      return.clear
;.CalculateDigest:
        mov     cl, byte [ebp+aes7z_decoder.NumCyclesPower]
        cmp     cl, 0x3F
        jnz     .sha
        lea     edi, [ebp+aes7z_decoder.Key]
        mov     ecx, [ebp+aes7z_decoder.SaltSize]
        push    32
        pop     edx
        sub     edx, ecx
        lea     esi, [ebp+aes7z_decoder.Salt]
        rep     movsb
        mov     ecx, [password_size]
        add     ecx, ecx
        cmp     ecx, edx
        jbe     @f
        mov     ecx, edx
@@:
        sub     edx, ecx
        mov     esi, password_unicode
        rep     movsb
        mov     ecx, edx
        xor     eax, eax
        rep     stosb
        jmp     .setkey
.sha:
        cmp     cl, 32
        jb      .normal
        push    1
        shl     dword [esp], cl
        push    0
        jmp     @f
.normal:
        push    0
        push    1
        shl     dword [esp], cl
@@:
        push    0
        push    0
        call    sha256_init
.loop:
        lea     esi, [ebp+aes7z_decoder.Salt]
        mov     edx, [ebp+aes7z_decoder.SaltSize]
        call    sha256_update
        mov     esi, password_unicode
        mov     edx, [password_size]
        add     edx, edx
        call    sha256_update
        mov     esi, esp
        push    8
        pop     edx
        call    sha256_update
        mov     esi, esp
        dec     esi
@@:
        inc     esi
        inc     byte [esi]
        jz      @b
        sub     dword [esp+8], 1
        sbb     dword [esp+12], 0
        mov     eax, [esp+8]
        or      eax, [esp+12]
        jnz     .loop
        lea     edi, [ebp+aes7z_decoder.Key]
        call    sha256_final
        add     esp, 16
.setkey:
        lea     esi, [ebp+aes7z_decoder.Key]
        push    8
        pop     edx     ; 7z uses 256-bit keys
        lea     edi, [ebp+aes7z_decoder.nr]
        call    aes_setkey
.ret:
        ret

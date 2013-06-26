; Sort array of unsigned dwords in non-decreasing order.
; ecx = array size, edx = array pointer.
; Destroys eax, ecx, esi, edi.
sort:
	test	ecx, ecx
	jz	.done
        mov     eax, ecx
@@:
        push    eax
        call    .restore
        pop     eax
        dec     eax
        jnz     @b
@@:
        cmp     ecx, 1
        jz      .done
        mov     esi, 1
        mov     edi, ecx
        call    .exchange
        dec     ecx
        mov     eax, 1
        call    .restore
        jmp     @b
.done:
        ret

.exchange:
        push    eax ecx
        mov     eax, [edx+esi*4-4]
        mov     ecx, [edx+edi*4-4]
        mov     [edx+esi*4-4], ecx
        mov     [edx+edi*4-4], eax
        pop     ecx eax
        ret

.restore:
        lea     esi, [eax+eax]
        cmp     esi, ecx
        ja      .doner
        mov     edi, [edx+eax*4-4]
        cmp	[edx+esi*4-4], edi
        ja      .need_xchg
        cmp     esi, ecx
        jae     .doner
        mov     edi, [edx+eax*4-4]
        cmp	[edx+esi*4], edi
        jbe     .doner
.need_xchg:
        cmp     esi, ecx
        jz      .do_xchg
        mov     edi, [edx+esi*4-4]
        cmp	[edx+esi*4], edi
        sbb     esi, -1
.do_xchg:
        mov     edi, eax
        call    .exchange
        mov     eax, esi
        jmp     .restore
.doner:
        ret

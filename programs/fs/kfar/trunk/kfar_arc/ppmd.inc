; PPMD decoder, ported from C++ sources of 7-Zip (c) Igor Pavlov
; C++ code is based on Dmitry Shkarin's PPMdH code
uglobal
ppmd_decoder.NS2Indx    rb      256
ppmd_decoder.NS2BSIndx  rb      256
ppmd_decoder.HB2Flag    rb      256
ppmd_decoder.Indx2Units rb      ppmd_decoder.N_INDEXES
ppmd_decoder.Units2Indx rb      128
endg

iglobal
label ppmd_decoder.InitBinEsc word
        dw      0x3CDD, 0x1F3F, 0x59BF, 0x48F3, 0x64A1, 0x5ABC, 0x6632, 0x6051
ppmd_decoder.ExpEscape db 25, 14, 9, 7, 5, 5, 4, 4, 4, 3, 3, 3, 2, 2, 2, 2
endg

init_ppmd:
; NS2Indx table
        mov     edi, ppmd_decoder.NS2Indx
        xor     eax, eax
        stosb
        inc     eax
        stosb
        inc     eax
        stosb
        mov     edx, 3-256
@@:
        lea     ecx, [eax-1]
        inc     eax
        add     edx, ecx
        jc      @f
        rep     stosb
        jmp     @b
@@:
        sub     ecx, edx
        rep     stosb
; NS2BSIndx table
        xor     eax, eax
        stosb
        add     al, 2
        stosb
        add     al, 2
        mov     cl, 9
        rep     stosb
        add     al, 2
        mov     cl, 256-11
        rep     stosb
; HB2Flag table
        mov     cl, 0x40/4
        xor     eax, eax
        rep     stosd
        mov     al, 8
        mov     cl, 0x100-0x40
        rep     stosb
; Indx2Units table
        mov     eax, 0x04030201
        stosd
        mov     eax, 0x0C0A0806
        stosd
        mov     eax, 0x1815120F
        stosd
        mov     al, 0x1C
@@:
        stosb
        add     al, 4
        cmp     al, 0x80
        jbe     @b
; Units2Indx table
        xor     eax, eax
        xor     edx, edx
        inc     edx
        xor     ecx, ecx
@@:
        cmp     [ppmd_decoder.Indx2Units+eax], dl
        adc     al, 0
        stosb
        inc     edx
        cmp     dl, 0x80
        jbe     @b
        ret

ppmd_decoder:
virtual at 0
; base is standard structure
.outStream      rb      streamInfo.size
.inStream       dd      ?

; RangeDecoder data
.inLen          dd      ?
.inPtr          dd      ?
.code           dd      ?
.range          dd      ?

.outSize        dd      ?       ; number of bytes rest for output

; PPMD data
.order          db      ?
.GlueCount      db      ?
.bInited        db      ?
                rb      1
.usedMemorySize dd      ?

; CSubAllocator constants
.N1 = 4
.N2 = 4
.N3 = 4
.N4 = (128+3-1*.N1-2*.N2-3*.N3)/4
.UNIT_SIZE = 12
.N_INDEXES = .N1+.N2+.N3+.N4
.kExtraSize = .UNIT_SIZE*3
.kMaxMemBlockSize = 0xFFFFFFFF - .kExtraSize

; CSubAllocator data
.HeapStart      dd      ?
.LoUnit         dd      ?
.HiUnit         dd      ?
.pText          dd      ?
.UnitsStart     dd      ?
.FreeList       rd      .N_INDEXES

; Context constants
.INT_BITS = 7
.PERIOD_BITS = 7
.TOT_BITS = .INT_BITS + .PERIOD_BITS
.INTERVAL = 1 shl .INT_BITS
.BIN_SCALE = 1 shl .TOT_BITS
.MAX_FREQ = 124

.kMaxOrderCompress = 32
.MAX_O = 255

; CDecodeInfo (inherits from CInfo) data
; SEE2_CONTEXT is 4 bytes long
.SEE2Cont       rd      25*16
.DummySEE2Cont  dd      ?
.MinContext     dd      ?
.MaxContext     dd      ?
.FoundState     dd      ?       ; found next state transition
.NumMasked      dd      ?
.InitEsc        dd      ?
.OrderFall      dd      ?
.RunLength      dd      ?
.InitRL         dd      ?
.CharMask       rb      256
.EscCount       db      ?
.PrintCount     db      ?
.PrevSuccess    db      ?
.HiBitsFlag     db      ?
.BinSumm        rw      128*64

.basesize = $
.Base:
;               rb      .kExtraSize + [.usedMemorySize]
end virtual

.init:
        mov     eax, [eax+.inStream]
        call    fillBuf
        mov     esi, [eax+streamInfo.bufPtr]
        mov     eax, [eax+streamInfo.bufDataLen]
        sub     eax, 5
        jb      return.err
        mov     [ebp+.inLen], eax
        inc     esi
        lodsd
        mov     [ebp+.inPtr], esi
        bswap   eax
        mov     [ebp+.code], eax
        or      [ebp+.range], -1
        mov     [ebp+.bInited], 1
        call    .StartModelRare
        mov     eax, ebp
        jmp     .mainloop

.fillBuf:
        mov     ebp, eax
        mov     [eax+.outSize], ecx
        cmp     [eax+.bInited], 0
        jz      .init
.mainloop:
        sub     [ebp+.outSize], 1
        js      .mainloopdone
;        cmp     edi, 0xde070+0x18
;        jnz     @f
;        int3
;@@:
        call    .DecodeSymbol
        jmp     .mainloop
.mainloopdone:
        popad
        ret

.GetBinSumm:
; CInfo::GetBinSumm(ebx=rs, ecx=numStates)
        movzx   eax, [ebp+.PrevSuccess]
        movzx   edx, [.NS2BSIndx+ecx-1]
        add     eax, edx
        mov     edx, [ebp+.FoundState]
        movzx   edx, byte [edx]
        movzx   edx, [.HB2Flag+edx]
        mov     [ebp+.HiBitsFlag], dl
        add     eax, edx
        movzx   edx, byte [ebx]
        movzx   edx, [.HB2Flag+edx]
        lea     eax, [eax+edx*2]
        mov     edx, [ebp+.RunLength]
        shr     edx, 26
        and     edx, 0x20
        add     eax, edx
        movzx   edx, byte [ebx+1]
        shl     edx, 6
        add     eax, edx
        lea     ecx, [ebp+.BinSumm+eax*2-2*64]
        ret

.StartModelRare:
; CInfo::StartModelRare(.order)
        mov     [ebp+.EscCount], 1
        mov     [ebp+.PrintCount], 1
; N.B.
; 1. Original code has some handling of [.order]<2, but this handling is incorrect
;    and causes exception (access violation).
; 2. 7-Zip never generates archives with [.order]<2 due to input preprocessing
;    (for PPMd method in switch -mo=<n> archiver checks that 2 <= n <= 32).
; 3. If manually created archive says [.order]<2, the exception will be generated
;    in StartModelRare, but it will be handled in Code() resulting in "data error".
        cmp     [ebp+.order], 2
        jb      return.err
        mov     byte [ebp+.DummySEE2Cont+2], .PERIOD_BITS

.RestartModelRare:
; CInfo::RestartModelRare(void)
        push    edi
        lea     edi, [ebp+.CharMask]
        xor     eax, eax
        push    0x40
        pop     ecx
        rep     stosd
; CSubAllocator::InitSubAllocator start
        mov     [ebp+.GlueCount], al
        lea     edi, [ebp+.FreeList]
        mov     cl, .N_INDEXES
        rep     stosd
        mov     ebx, [ebp+.HeapStart]
        mov     [ebp+.pText], ebx
        add     ebx, [ebp+.usedMemorySize]
        mov     [ebp+.HiUnit], ebx
        mov     eax, [ebp+.usedMemorySize]
        xor     edx, edx
        mov     cl, 8*.UNIT_SIZE
        div     ecx
        imul    eax, 7*.UNIT_SIZE
        sub     ebx, eax
        mov     [ebp+.LoUnit], ebx
        mov     [ebp+.UnitsStart], ebx
; CSubAllocator::InitSubAllocator end
        pop     edi
        movzx   eax, [ebp+.order]
        cmp     al, 12
        jb      @f
        mov     al, 12
@@:
        neg     eax
        dec     eax
        mov     [ebp+.InitRL], eax
        mov     [ebp+.RunLength], eax
        call    .AllocContext
        mov     [ebp+.MinContext], eax
        mov     [ebp+.MaxContext], eax
        and     dword [eax+8], 0
        mov     esi, eax
        movzx   edx, [ebp+.order]
        mov     [ebp+.OrderFall], edx
        mov     dword [eax], 257*10000h+256
        mov     ecx, 256/2
        call    .AllocUnits
        mov     [ebp+.FoundState], eax
        mov     [esi+4], eax
        push    edi
        mov     edi, eax
        xor     eax, eax
        mov     [ebp+.PrevSuccess], al
@@:
        stosb
        mov     byte [edi], 1
        and     dword [edi+1], 0
        add     edi, 5
        inc     al
        jnz     @b
        lea     edi, [ebp+.BinSumm]
        push    2
        pop     ecx
.rmr1:
        mov     esi, .InitBinEsc
@@:
        lodsw
        xor     edx, edx
        div     ecx
        sub     eax, .BIN_SCALE
        neg     eax
        mov     [edi+2*8], ax
        mov     [edi+2*16], ax
        mov     [edi+2*24], ax
        mov     [edi+2*32], ax
        mov     [edi+2*40], ax
        mov     [edi+2*48], ax
        mov     [edi+2*56], ax
        stosw
        cmp     esi, .InitBinEsc+2*8
        jb      @b
        add     edi, 128-16
        inc     ecx
        cmp     ecx, 128+2
        jb      .rmr1
        lea     edi, [ebp+.SEE2Cont]
        mov     eax, (10 shl (.PERIOD_BITS-4)) + ((.PERIOD_BITS-4) shl 16) + (4 shl 24)
        push    25
        pop     edx
@@:
        push    16
        pop     ecx
        rep     stosd
        add     ax, 5 shl (.PERIOD_BITS-4)
        dec     edx
        jnz     @b
        pop     edi
        ret

.CreateSuccessors:
; CInfo::CreateSuccessors(bool al=skip,STATE* esi=p1)
        push    ebx edi
        mov     ebx, [ebp+.MinContext]  ; ebx=pc
        mov     ecx, [ebp+.FoundState]
        mov     ecx, [ecx+2]            ; ecx=UpBranch
        sub     esp, .MAX_O*4           ; esp=ps
        mov     edi, esp                ; edi=pps
        test    al, al
        jnz     @f
        mov     eax, [ebp+.FoundState]
        stosd
        cmp     dword [ebx+8], 0
        jz      .csnoloop
@@:
        test    esi, esi
        jz      .csloopstart
        mov     edx, esi                ; edx=p
        mov     ebx, [ebx+8]
        jmp     .csloopentry
.csloopstart:
        mov     ebx, [ebx+8]
        lea     edx, [ebx+2]
        cmp     word [ebx], 1
        jz      .csloopentry
        mov     edx, [ebx+4]
        mov     eax, [ebp+.FoundState]
        sub     edx, 6
        mov     al, [eax]
@@:
        add     edx, 6
        cmp     al, [edx]
        jnz     @b
.csloopentry:
        cmp     ecx, [edx+2]
        jz      @f
        mov     ebx, [edx+2]
        jmp     .csnoloop
@@:
        mov     [edi], edx
        add     edi, 4
        cmp     dword [ebx+8], 0
        jnz     .csloopstart
.csnoloop:
        cmp     edi, esp
        jz      .csr
        push    eax
        push    eax
        mov     al, [ecx]
        mov     [esp], al
        add     ecx, 1
        mov     [esp+2], ecx
        mov     ah, [ebx+3]
        cmp     word [ebx], 1
        jz      .cs2
        mov     edx, [ebx+4]
        sub     edx, 6
@@:
        add     edx, 6
        cmp     [edx], al
        jnz     @b
        movzx   edx, byte [edx+1]
        sub     edx, 1          ; edx=cf
        movzx   ecx, word [ebx+2]
        movzx   eax, word [ebx]
        sub     ecx, eax
        sub     ecx, edx        ; ecx=s0
        lea     eax, [edx+edx]
        cmp     eax, ecx
        ja      .cs0
        lea     eax, [edx*5]
        cmp     eax, ecx
        seta    ah
        jmp     .cs1
.cs0:
        lea     eax, [eax+ecx*2]
        lea     eax, [eax+ecx-1]
        add     ecx, ecx
        xor     edx, edx
        div     ecx
        mov     ah, al
.cs1:
        add     ah, 1
.cs2:
        mov     [esp+1], ah
        sub     edi, 8
.cs3:
; PPM_CONTEXT::createChild(this=ebx,pStats=[edi+4],FirstState=esp) begin
        call    .AllocContext
        test    eax, eax
        jz      .csr0
        mov     word [eax], 1
        mov     dx, [esp]
        mov     [eax+2], dx
        mov     edx, [esp+2]
        mov     [eax+4], edx
        mov     [eax+8], ebx
        mov     edx, [edi+4]
        mov     [edx+2], eax
; PPM_CONTEXT::createChild end
        mov     ebx, eax
        sub     edi, 4
        cmp     edi, esp
        jnz     .cs3
        pop     eax eax
.csr:
        mov     eax, ebx
@@:
        add     esp, .MAX_O*4
        pop     edi ebx
        ret
.csr0:
        pop     eax eax
        xor     eax, eax
        jmp     @b

; CInfo::UpdateModel(void)
.UpdateModel:
        mov     ebx, [ebp+.FoundState]
        xor     esi, esi                ; esi=p
        movzx   eax, word [ebx]
        mov     ebx, [ebx+2]            ; ebx=fs.Successor
        push    eax
        cmp     ah, .MAX_FREQ/4
        jae     .um2
        mov     eax, [ebp+.MinContext]
        mov     eax, [eax+8]
        test    eax, eax
        jz      .um2
        cmp     word [eax], 1
        jz      .um1
        push    eax
        mov     esi, [eax+4]
        mov     al, [esp+4]
        cmp     al, [esi]
        jz      .um0
@@:
        add     esi, 6
        cmp     al, [esi]
        jnz     @b
        mov     al, [esi+1]
        cmp     al, [esi-6+1]
        jb      @f
        mov     eax, [esi]
        xchg    [esi-6], eax
        mov     [esi], eax
        mov     ax, [esi+4]
        xchg    [esi-6+4], ax
        mov     [esi+4], ax
        sub     esi, 6
@@:
.um0:
        pop     eax
        cmp     byte [esi+1], .MAX_FREQ-9
        jae     @f
        add     byte [esi+1], 2
        add     word [eax+2], 2
@@:
        jmp     .um2
.um1:
        lea     esi, [eax+2]
        cmp     byte [esi+1], 32
        adc     byte [esi+1], 0
.um2:
        cmp     [ebp+.OrderFall], 0
        jnz     .um3
        pop     eax
        mov     al, 1
        call    .CreateSuccessors
        mov     [ebp+.MinContext], eax
        mov     [ebp+.MaxContext], eax
        mov     edx, [ebp+.FoundState]
        mov     [edx+2], eax
        test    eax, eax
        jz      .RestartModel
        ret
.um3:
        mov     edx, [ebp+.pText]
        mov     al, [esp]
        mov     [edx], al
        add     edx, 1          ; edx=Successor
        mov     [ebp+.pText], edx
        cmp     edx, [ebp+.UnitsStart]
        jae     .RestartModelPop
        test    ebx, ebx
        jz      .um4
        cmp     ebx, [ebp+.pText]
        ja      @f
        push    edx
        xor     eax, eax
        call    .CreateSuccessors
        pop     edx
        mov     ebx, eax
        test    eax, eax
        jz      .RestartModelPop
@@:
        sub     [ebp+.OrderFall], 1
        jnz     @f
        mov     edx, ebx
        xor     ecx, ecx
        mov     eax, [ebp+.MinContext]
        cmp     eax, [ebp+.MaxContext]
        setnz   cl
        sub     [ebp+.pText], ecx
@@:
        jmp     .um5
.um4:
        mov     eax, [ebp+.FoundState]
        mov     [eax+2], edx
        mov     ebx, [ebp+.MinContext]
.um5:
        mov     eax, [ebp+.MinContext]
        movzx   ecx, word [eax] ; ecx=ns
        movzx   eax, word [eax+2]
        sub     eax, ecx
        push    eax
        movzx   eax, byte [esp+5]
        sub     eax, 1
        sub     [esp], eax      ; [esp]=s0
        mov     esi, [ebp+.MaxContext]  ; ebx=pc
        cmp     esi, [ebp+.MinContext]
        jz      .um12
.um6:
        movzx   eax, word [esi]
        cmp     eax, 1
        jz      .um8
        push    eax
        shr     eax, 1
        jc      .um7
        push    esi
        mov     esi, [esi+4]
        call    .ExpandUnits
        pop     esi
        mov     [esi+4], eax
        test    eax, eax
        jz      .RestartModelPop3
.um7:
        pop     eax
        add     eax, eax
        cmp     eax, ecx
        adc     word [esi+2], 0
        add     eax, eax
        cmp     eax, ecx
        ja      @f
        lea     eax, [eax+eax+1]
        cmp     word [esi+2], ax
        ja      @f
        add     word [esi+2], 2
@@:
        push    edx
        jmp     .um9
.um8:
        push    edx ecx
        mov     ecx, 1
        call    .AllocUnits
        pop     ecx
        test    eax, eax
        jz      .RestartModelPop3
        mov     dx, [esi+2]
        mov     [eax], dx
        mov     edx, [esi+4]
        mov     [eax+2], edx
        mov     [esi+4], eax
        movzx   edx, byte [eax+1]
        add     edx, edx
        cmp     edx, (.MAX_FREQ/4-1)*2
        jb      @f
        mov     edx, .MAX_FREQ-4
@@:
        mov     [eax+1], dl
        add     edx, [ebp+.InitEsc]
        cmp     ecx, 4
        sbb     edx, -1
        mov     [esi+2], dx
.um9:
        movzx   edx, word [esi+2]
        mov     eax, [esp+4]
        push    ecx
        lea     ecx, [eax+edx]  ; ecx=sf
        add     edx, 6
        movzx   eax, byte [esp+13]
        add     eax, eax
        imul    eax, edx        ; eax=cf
        lea     edx, [ecx*3]
        add     edx, edx
        cmp     eax, edx
        jae     .um10
        mov     edx, 1
        cmp     ecx, eax
        adc     edx, 0
        shl     ecx, 2
        add     eax, 1
        cmp     ecx, eax
        adc     edx, 0
        add     word [esi+2], 3
        jmp     .um11
.um10:
        lea     ecx, [ecx*3]
        lea     edx, [ecx*3]
        add     eax, 1
        push    4
        cmp     edx, eax
        adc     dword [esp], 0
        add     edx, ecx
        cmp     edx, eax
        adc     dword [esp], 0
        add     edx, ecx
        cmp     edx, eax
        adc     dword [esp], 0
        pop     edx
        add     [esi+2], dx
.um11:
        movzx   eax, word [esi]
        lea     eax, [eax*3]
        add     eax, eax
        add     eax, [esi+4]
        mov     ecx, [esp+4]
        mov     [eax+2], ecx
        mov     cl, [esp+12]
        mov     [eax], cl
        mov     [eax+1], dl
        add     word [esi], 1
        pop     ecx edx
        mov     esi, [esi+8]
        cmp     esi, [ebp+.MinContext]
        jnz     .um6
.um12:
        pop     eax
        pop     eax
        mov     [ebp+.MinContext], ebx
        mov     [ebp+.MaxContext], ebx
        ret
.RestartModelPop3:
        pop     eax
        pop     eax
.RestartModelPop:
        pop     eax
.RestartModel:
        call    .RestartModelRare
        mov     [ebp+.EscCount], 0
        mov     [ebp+.PrintCount], 0xFF
        ret

.rescale:
        mov     esi, [ebp+.MinContext]
        movzx   ecx, word [esi]
        push    ecx     ; [esp]=OldNS
        sub     ecx, 1
        mov     ebx, [ebp+.FoundState]
        cmp     ebx, [esi+4]
        jz      .r1
.r0:
        mov     ax, [ebx]
        xchg    [ebx-6], ax
        mov     [ebx], ax
        mov     eax, [ebx+2]
        xchg    [ebx-6+2], eax
        mov     [ebx+2], eax
        sub     ebx, 6
        cmp     ebx, [esi+4]
        jnz     .r0
.r1:
        add     byte [ebx+1], 4
        add     word [esi+2], 4
        movzx   eax, byte [ebx+1]
        movzx   edx, word [esi+2]
        sub     edx, eax        ; edx=EscFreq
        cmp     [ebp+.OrderFall], 1
        sbb     eax, -1
        shr     eax, 1
        mov     [ebx+1], al
        mov     [esi+2], ax
.r2:
        add     ebx, 6
        movzx   eax, byte [ebx+1]
        sub     edx, eax
        cmp     [ebp+.OrderFall], 1
        sbb     eax, -1
        shr     eax, 1
        mov     [ebx+1], al
        add     [esi+2], ax
        cmp     al, [ebx-6+1]
        jbe     .r3
        push    ecx
        push    ebx
        push    dword [ebx]
        push    word [ebx+4]
@@:
        mov     ecx, [ebx-6]
        mov     [ebx], ecx
        mov     cx, [ebx-6+4]
        mov     [ebx+4], cx
        sub     ebx, 6
        cmp     ebx, [esi+4]
        jz      @f
        cmp     al, [ebx-6+1]
        ja      @b
@@:
        pop     word [ebx+4]
        pop     dword [ebx]
        pop     ebx
        pop     ecx
.r3:
        sub     ecx, 1
        jnz     .r2
        cmp     byte [ebx+1], 0
        jnz     .r4
@@:
        add     ecx, 1
        sub     ebx, 6
        cmp     byte [ebx+1], 0
        jz      @b
        add     edx, ecx
        sub     word [esi], cx
        cmp     word [esi], 1
        jnz     .r4
        pop     ebx
        mov     eax, [esi+4]
        movzx   ecx, word [eax+4]
        push    ecx
        push    dword [eax]
        movzx   eax, byte [eax+1]
@@:
        add     eax, 1
        shr     eax, 1
        shr     edx, 1
        cmp     edx, 1
        ja      @b
        mov     [esp+1], al
        add     ebx, 1
        shr     ebx, 1
        mov     eax, [esi+4]
        call    .FreeUnits
        lea     ebx, [esi+2]
        mov     [ebp+.FoundState], ebx
        pop     dword [ebx]
        pop     eax
        mov     [ebx+4], ax
        ret
.r4:
        add     edx, 1
        shr     edx, 1
        add     [esi+2], dx
        pop     ebx
        add     ebx, 1
        shr     ebx, 1
        movzx   ecx, word [esi]
        add     ecx, 1
        shr     ecx, 1
        cmp     ebx, ecx
        jz      @f
        mov     eax, [esi+4]
        call    .ShrinkUnits
        mov     [esi+4], eax
@@:
        mov     eax, [esi+4]
        mov     [ebp+.FoundState], eax
        ret

.DecodeSymbol:
; CDecodeInfo::DecodeSymbol
        mov     esi, [ebp+.MinContext]
        cmp     word [esi], 1
        jz      .binsymbol
; CDecodeInfo::DecodeSymbol1 start
        mov     ebx, [esi+4]    ; state
        movzx   ecx, word [esi+2]
        mov     eax, [ebp+.range]
        xor     edx, edx
        div     ecx
        mov     [ebp+.range], eax
        mov     ecx, eax
        mov     eax, [ebp+.code]
        xor     edx, edx
        div     ecx
        movzx   edx, byte [ebx+1]
        cmp     eax, edx
        jae     .ds0
        push    edx
        add     edx, edx
        cmp     dx, [esi+2]
        pop     edx
        seta    [ebp+.PrevSuccess]
        movzx   eax, [ebp+.PrevSuccess]
        add     [ebp+.RunLength], eax
        xor     eax, eax
        call    .RangeDecoder.Decode
        mov     [ebp+.FoundState], ebx
        add     edx, 4
        mov     [ebx+1], dl
        add     word [esi+2], 4
        cmp     edx, .MAX_FREQ
        jbe     @f
        call    .rescale
@@:
        jmp     .dscmn
.ds0:
        mov     [ebp+.PrevSuccess], 0
        movzx   ecx, word [esi]
        sub     ecx, 1
        push    eax
.ds1:
        add     ebx, 6
        movzx   eax, byte [ebx+1]
        add     edx, eax
        cmp     edx, [esp]
        ja      .ds2
        sub     ecx, 1
        jnz     .ds1
        pop     eax
        mov     eax, [ebp+.FoundState]
        movzx   eax, byte [eax]
        mov     al, [.HB2Flag+eax]
        mov     [ebp+.HiBitsFlag], al
        mov     eax, edx
        movzx   edx, word [esi+2]
        sub     edx, eax
        call    .RangeDecoder.Decode
        mov     al, [ebp+.EscCount]
        movzx   edx, byte [ebx]
        mov     [ebp+.CharMask+edx], al
        movzx   ecx, word [esi]
        mov     [ebp+.NumMasked], ecx
        sub     ecx, 1
@@:
        sub     ebx, 6
        movzx   edx, byte [ebx]
        mov     [ebp+.CharMask+edx], al
        sub     ecx, 1
        jnz     @b
        mov     [ebp+.FoundState], ecx
        jmp     .dscmn
.ds2:
        pop     eax
        mov     eax, edx
        movzx   edx, byte [ebx+1]
        sub     eax, edx
        call    .RangeDecoder.Decode
.update1:
        mov     [ebp+.FoundState], ebx
        add     byte [ebx+1], 4
        add     word [esi+2], 4
        mov     al, [ebx+1]
        cmp     al, [ebx-6+1]
        jbe     @f
        mov     eax, [ebx]
        xchg    eax, [ebx-6]
        mov     [ebx], eax
        mov     ax, [ebx+4]
        xchg    ax, [ebx-6+4]
        mov     [ebx+4], ax
        sub     ebx, 6
        mov     [ebp+.FoundState], ebx
        cmp     byte [ebx+1], .MAX_FREQ
        jbe     @f
        call    .rescale
@@:
        jmp     .dscmn
; CDecodeInfo::DecodeSymbol1 end
.binsymbol:
; CDecodeInfo::DecodeBinSymbol start
        lea     ebx, [esi+2]
        mov     ecx, [esi+8]
        movzx   ecx, word [ecx]
        call    .GetBinSumm
        movzx   eax, word [ecx]
        call    .RangeDecoder.DecodeBit
        jc      .ds3
        mov     [ebp+.FoundState], ebx
        cmp     byte [ebx+1], 128
        adc     byte [ebx+1], 0
        movzx   eax, word [ecx]
        add     eax, 1 shl (.PERIOD_BITS-2)
        shr     eax, .PERIOD_BITS
        sub     eax, .INTERVAL
        sub     [ecx], ax
        mov     [ebp+.PrevSuccess], 1
        add     [ebp+.RunLength], 1
        jmp     .dscmn
.ds3:
        movzx   eax, word [ecx]
        add     eax, 1 shl (.PERIOD_BITS-2)
        shr     eax, .PERIOD_BITS
        sub     [ecx], ax
        movzx   eax, word [ecx]
        shr     eax, 10
        movzx   eax, [.ExpEscape+eax]
        mov     [ebp+.InitEsc], eax
        mov     [ebp+.NumMasked], 1
        mov     al, [ebp+.EscCount]
        movzx   edx, byte [ebx]
        mov     [ebp+.CharMask+edx], al
        mov     [ebp+.PrevSuccess], 0
        and     [ebp+.FoundState], 0
; CDecodeInfo::DecodeBinSymbol end
.dscmn:
        cmp     [ebp+.FoundState], 0
        jnz     .dsfnd
.ds4:
        add     [ebp+.OrderFall], 1
        mov     eax, [ebp+.MinContext]
        mov     eax, [eax+8]
        test    eax, eax
        jz      return.err      ; no end-of-stream mark
        mov     [ebp+.MinContext], eax
        movzx   ecx, word [eax]
        sub     ecx, [ebp+.NumMasked]
        jz      .ds4
; CDecodeInfo::DecodeSymbol2 start
        call    .makeEscFreq2
        push    eax
        mov     ebx, [esi+4]
        sub     ebx, 6
        sub     esp, 256*4
        mov     esi, esp
        xor     eax, eax
        push    eax
@@:
        add     ebx, 6
        mov     al, [ebx]
        mov     al, [ebp+.CharMask+eax]
        cmp     al, [ebp+.EscCount]
        jz      @b
        mov     al, [ebx+1]
        add     [esp], eax
        mov     [esi], ebx
        add     esi, 4
        sub     ecx, 1
        jnz     @b
        add     edx, [esp]
        mov     ecx, edx
        mov     eax, [ebp+.range]
        xor     edx, edx
        div     ecx
        mov     [ebp+.range], eax
        mov     eax, [ebp+.code]
        xor     edx, edx
        div     [ebp+.range]
        cmp     eax, [esp]
        jae     .ds5
        pop     ecx
        mov     esi, esp
        xor     ecx, ecx
@@:
        mov     ebx, [esi]
        add     esi, 4
        movzx   edx, byte [ebx+1]
        add     ecx, edx
        cmp     eax, ecx
        jae     @b
        mov     eax, ecx
        movzx   edx, byte [ebx+1]
        sub     eax, edx
        call    .RangeDecoder.Decode
        add     esp, 256*4
        pop     eax
        mov     cl, [eax+2]
        cmp     cl, .PERIOD_BITS
        jae     @f
        sub     byte [eax+3], 1
        jnz     @f
        shl     word [eax], 1
        mov     dl, 3
        shl     dl, cl
        mov     [eax+3], dl
        add     byte [eax+2], 1
@@:
.update2:
        mov     [ebp+.FoundState], ebx
        add     byte [ebx+1], 4
        mov     esi, [ebp+.MinContext]
        add     word [esi+2], 4
        cmp     byte [ebx+1], .MAX_FREQ
        jbe     @f
        call    .rescale
@@:
        add     [ebp+.EscCount], 1
        mov     eax, [ebp+.InitRL]
        mov     [ebp+.RunLength], eax
        jmp     .dsfnd
.ds5:
        pop     eax
        mov     edx, ecx
        sub     edx, eax
        call    .RangeDecoder.Decode
        mov     eax, [ebp+.MinContext]
        movzx   eax, word [eax]
        mov     ebx, eax
        sub     ebx, [ebp+.NumMasked]
        mov     [ebp+.NumMasked], eax
        mov     esi, esp
        mov     al, [ebp+.EscCount]
@@:
        mov     edx, [esi]
        add     esi, 4
        movzx   edx, byte [edx]
        mov     [ebp+.CharMask+edx], al
        sub     ebx, 1
        jnz     @b
        add     esp, 256*4
        pop     eax
        add     word [eax], cx
; CDecodeInfo::DecodeSymbol2 end
        cmp     [ebp+.FoundState], 0
        jz      .ds4
.dsfnd:
        mov     eax, [ebp+.FoundState]
        mov     al, [eax]
        stosb

.NextContext:
; CInfo::NextContext(void)
        mov     ebx, [ebp+.FoundState]
        mov     ebx, [ebx+2]
        cmp     [ebp+.OrderFall], 0
        jnz     .nc0
        cmp     ebx, [ebp+.pText]
        jbe     .nc0
        mov     [ebp+.MinContext], ebx
        mov     [ebp+.MaxContext], ebx
        ret
.nc0:
        call    .UpdateModel
        cmp     [ebp+.EscCount], 0
        jz      @f
        ret
@@:
        mov     [ebp+.EscCount], 1
        push    edi
        lea     edi, [ebp+.CharMask]
        mov     ecx, 256/4
        xor     eax, eax
        rep     stosd
        pop     edi
        ret

.makeEscFreq2:
; CInfo::makeEscFreq2(ecx=Diff)->{eax->SEE2_CONTEXT,edx=scale}
        mov     esi, [ebp+.MinContext]
        cmp     word [esi], 256
        jz      .mef0
        movzx   edx, [.NS2Indx+ecx-1]
        shl     edx, 4
        mov     eax, [esi+8]
        movzx   eax, word [eax]
        sub     ax, [esi]
        cmp     ecx, eax
        adc     edx, 0
        movzx   eax, word [esi]
        push    edx
        lea     edx, [eax*9]
        lea     edx, [edx+eax*2]
        movzx   eax, word [esi+2]
        cmp     eax, edx
        pop     edx
        setc    al
        movzx   eax, al
        lea     edx, [edx+eax*2]
        cmp     ecx, [ebp+.NumMasked]
        setc    al
        lea     edx, [edx+eax*4]
        add     dl, [ebp+.HiBitsFlag]
        lea     eax, [ebp+edx*4+.SEE2Cont]
        movzx   edx, word [eax]
        push    ecx
        mov     cl, [eax+2]
        shr     edx, cl
        sub     [eax], dx
        pop     ecx
        cmp     edx, 1
        adc     edx, 0
        ret
.mef0:
        lea     eax, [ebp+.DummySEE2Cont]
        mov     edx, 1
        ret

.RangeDecoder.DecodeBit:
; CRangeDecoder::DecodeBit(eax=size0,numTotalBits=.TOT_BITS)
        mov     edx, [ebp+.range]
        shr     edx, .TOT_BITS
        imul    eax, edx
        cmp     [ebp+.code], eax
        jae     .rddb
        mov     [ebp+.range], eax
        call    .RangeDecoder.Normalize
        clc
        ret
.rddb:
        sub     [ebp+.code], eax
        sub     [ebp+.range], eax
        call    .RangeDecoder.Normalize
        stc
        ret

.RangeDecoder.Decode:
        imul    eax, [ebp+.range]
        sub     [ebp+.code], eax
        mov     eax, [ebp+.range]
        imul    eax, edx
        mov     [ebp+.range], eax
.RangeDecoder.Normalize:
        cmp     byte [ebp+.range+3], 0
        jz      @f
        ret
@@:
        sub     [ebp+.inLen], 1
        js      .refill
.filled:
        shl     [ebp+.range], 8
        shl     [ebp+.code], 8
        mov     eax, [ebp+.inPtr]
        add     [ebp+.inPtr], 1
        mov     al, [eax]
        mov     byte [ebp+.code], al
        jmp     .RangeDecoder.Normalize
.refill:
        mov     eax, [ebp+.inStream]
        call    fillBuf
        push    [eax+streamInfo.bufPtr]
        pop     [ebp+.inPtr]
        mov     eax, [eax+streamInfo.bufDataLen]
        sub     eax, 1
        js      return.err
        mov     [ebp+.inLen], eax
        jmp     .filled

.GlueFreeBlocks:
; CSubAllocator::GlueFreeBlocks, called from AllocUnitsRare
        push    eax
        mov     [ebp+.GlueCount], 255
        mov     edx, [ebp+.HeapStart]
        add     edx, [ebp+.usedMemorySize]
        ; we need add extra MEM_BLK with Stamp=0
        and     word [edx], 0
        add     edx, .UNIT_SIZE
        mov     eax, [ebp+.LoUnit]
        cmp     eax, [ebp+.HiUnit]
        jz      @f
        mov     byte [eax], 0
@@:
        mov     [edx+4], edx
        mov     [edx+8], edx
        push    ecx
        xor     ecx, ecx
.gfb1:
        mov     eax, [ebp+ecx*4+.FreeList]
        test    eax, eax
        jz      .gfb2
        push    dword [eax]
        pop     dword [ebp+ecx*4+.FreeList]
        mov     [eax+8], edx
        push    edx
        mov     edx, [edx+4]
        mov     [eax+4], edx
        mov     [edx+8], eax
        or      word [eax], 0xFFFF
        movzx   edx, [.Indx2Units+ecx]
        mov     [eax+2], dx
        pop     edx
        mov     [edx+4], eax
        jmp     .gfb1
.gfb2:
        inc     ecx
        cmp     ecx, .N_INDEXES
        jb      .gfb1
        mov     ecx, edx
.gfb3:
        mov     ecx, [ecx+4]
        cmp     ecx, edx
        jz      .gfb5
.gfb4:
        movzx   eax, word [ecx+2]
        lea     eax, [eax*3]
        lea     eax, [ecx+eax*4]
        cmp     word [eax], 0xFFFF
        jnz     .gfb3
        push    eax
        mov     ax, [eax+2]
        add     ax, [ecx+2]
        pop     eax
        jc      .gfb3
        push    edx
        mov     edx, [eax+4]
        push    dword [eax+8]
        pop     dword [edx+8]
        mov     edx, [eax+8]
        push    dword [eax+4]
        pop     dword [edx+4]
        pop     edx
        mov     ax, [eax+2]
        add     [ecx+2], ax
        jmp     .gfb4
.gfb5:
        mov     ecx, [edx+4]
        cmp     ecx, edx
        jz      .gfb8
        mov     eax, [ecx+4]
        mov     [eax+8], edx
        mov     [edx+4], eax
        movzx   eax, word [ecx+2]
        push    edx
.gfb6:
        sub     eax, 128
        jbe     .gfb7
        mov     edx, ecx
        xchg    edx, [ebp+.FreeList+(.N_INDEXES-1)*4]
        mov     [ecx], edx
        add     ecx, 128*.UNIT_SIZE
        jmp     .gfb6
.gfb7:
        add     eax, 128
        movzx   edx, [.Units2Indx+eax-1]
        cmp     [.Indx2Units+edx], al
        jz      @f
        dec     edx
        push    edx
        movzx   edx, [.Indx2Units+edx]
        sub     eax, edx
        lea     eax, [ebp+.FreeList+(eax-1)*4]
        lea     edx, [edx*3]
        lea     edx, [ecx+edx*4]
        push    dword [eax]
        pop     dword [edx]
        mov     [eax], edx
        pop     edx
@@:
        mov     eax, ecx
        xchg    eax, [ebp+.FreeList+edx*4]
        mov     [ecx], eax
        pop     edx
        jmp     .gfb5
.gfb8:
        pop     ecx
        pop     eax
        mov     edx, [ebp+.FreeList+eax*4]
        test    edx, edx
        jz      .aur.cont
        push    edx
        mov     edx, [edx]
        mov     [ebp+.FreeList+eax*4], edx
        pop     eax
        ret

.AllocContext:
; CSubAllocator::AllocContext
        mov     eax, [ebp+.HiUnit]
        cmp     eax, [ebp+.LoUnit]
        jz      @f
        sub     eax, .UNIT_SIZE
        mov     [ebp+.HiUnit], eax
        ret
@@:
        mov     eax, [ebp+.FreeList]
        test    eax, eax
        jz      @f
        mov     edx, [eax]
        mov     [ebp+.FreeList], edx
        ret
@@:
        xor     eax, eax
        jmp     .AllocUnitsRare

.AllocUnits:
; CSubAllocator::AllocUnits(ecx)
        movzx   ecx, [.Units2Indx+ecx-1]
        mov     eax, [ebp+.FreeList+ecx*4]
        test    eax, eax
        jz      @f
        mov     edx, [eax]
        mov     [ebp+.FreeList+ecx*4], edx
        ret
@@:
        mov     eax, [ebp+.LoUnit]
        movzx   edx, [.Indx2Units+ecx]
        lea     edx, [edx*3]
        lea     eax, [eax+edx*4]
        cmp     eax, [ebp+.HiUnit]
        ja      @f
        xchg    eax, [ebp+.LoUnit]
        ret
@@:
        mov     eax, ecx

.AllocUnitsRare:
; CSubAllocator::AllocUnitsRare(eax)
        cmp     [ebp+.GlueCount], 0
        jz      .GlueFreeBlocks
.aur.cont:
        push    eax
.aur1:
        inc     eax
        cmp     eax, .N_INDEXES
        jz      .aur3
        mov     edx, [ebp+.FreeList+eax*4]
        test    edx, edx
        jz      .aur1
        push    edx
        mov     edx, [edx]
        mov     [ebp+.FreeList+eax*4], edx
        call    .SplitBlock
        pop     eax
        pop     edx
        ret
.aur3:
        dec     [ebp+.GlueCount]
        pop     eax
        movzx   eax, [.Indx2Units+eax]
        lea     edx, [eax*3]
        shl     edx, 2
        mov     eax, [ebp+.UnitsStart]
        sub     eax, [ebp+.pText]
        cmp     eax, edx
        jbe     .aur4
        mov     eax, [ebp+.UnitsStart]
        sub     eax, edx
        mov     [ebp+.UnitsStart], eax
        ret
.aur4:
        xor     eax, eax
        ret

.SplitBlock:
; CSubAllocator::SplitBlock(pv=[esp+4],oldIndx=eax,newIndx=[esp+8])
        push    eax
        mov     edx, [esp+12]
        movzx   eax, [.Indx2Units+eax]
        movzx   edx, [.Indx2Units+edx]
        sub     eax, edx
        lea     edx, [edx*3]
        push    ecx
        mov     ecx, [esp+12]
        lea     ecx, [ecx+edx*4]
        movzx   edx, [.Units2Indx+eax-1]
        cmp     [.Indx2Units+edx], al
        jz      .aur2
        push    dword [ebp+.FreeList+(edx-1)*4]
        pop     dword [ecx]
        mov     [ebp+.FreeList+(edx-1)*4], ecx
        movzx   edx, [.Indx2Units+edx-1]
        sub     eax, edx
        lea     edx, [edx*3]
        lea     ecx, [ecx+edx*4]
.aur2:
        movzx   eax, [.Units2Indx+eax-1]
        push    dword [ebp+.FreeList+eax*4]
        pop     dword [ecx]
        mov     [ebp+.FreeList+eax*4], ecx
        pop     ecx
        pop     eax
        ret

.ExpandUnits:
; CSubAllocator::ExpandUnits(void* oldPtr=esi, int oldNU=eax)
        push    edx
        movzx   edx, [.Units2Indx + eax - 1]
        cmp     dl, [.Units2Indx + eax]
        jnz     @f
        pop     edx
        mov     eax, esi
        ret
@@:
        push    eax ecx edx
        lea     ecx, [eax+1]
        call    .AllocUnits
        pop     edx
        test    eax, eax
        jz      @f
        push    esi edi
        mov     edi, eax
        mov     ecx, [esp+8+4]
        lea     ecx, [ecx*3]
        rep     movsd
        pop     edi esi
        mov     ecx, [ebp+.FreeList+edx*4]
        mov     [esi], ecx
        mov     [ebp+.FreeList+edx*4], esi
@@:
        pop     ecx
        add     esp, 4
        pop     edx
        ret

.ShrinkUnits:
; CSubAllocator::ShrinkUnits(void* oldPtr=eax, int oldNU=ebx, int newNU=ecx)
        push    ecx
        movzx   ebx, [.Units2Indx+ebx-1]
        movzx   ecx, [.Units2Indx+ecx-1]
        cmp     ebx, ecx
        jnz     @f
        pop     ecx
        ret
@@:
        mov     edx, [ebp+.FreeList+ecx*4]
        test    edx, edx
        jz      @f
        push    dword [edx]
        pop     [ebp+.FreeList+ecx*4]
        pop     ecx
        push    esi edi
        mov     esi, eax
        mov     edi, edx
        lea     ecx, [ecx*3]
        rep     movsd
        pop     edi esi
        mov     ecx, [ebp+.FreeList+ebx*4]
        mov     [eax], ecx
        mov     [ebp+.FreeList+ebx*4], eax
        mov     eax, edx
        ret
@@:
        push    ecx
        push    eax
        mov     eax, ebx
        call    .SplitBlock
        pop     eax
        pop     ecx
        pop     ecx
        ret

.FreeUnits:
; CSubAllocator::FreeUnits(void* ptr=eax, int oldNU=ebx)
        movzx   ebx, [.Units2Indx+ebx-1]
        push    [ebp+.FreeList+ebx*4]
        pop     dword [eax]
        mov     [ebp+.FreeList+ebx*4], eax
        ret

ppmd_get_buf_size:
        cmp     dword [esi-4], 5
        jb      return.err
        lodsb
        lodsd
        cmp     eax, ppmd_decoder.kMaxMemBlockSize
        ja      return.err
        add     eax, ppmd_decoder.basesize + ppmd_decoder.kExtraSize
        mov     edx, 0x4000
        ret

ppmd_init_decoder:
        mov     [ebp+ppmd_decoder.bInited], 0
; CDecoder::SetDecoderProperties2
        lodsb
        mov     [ebp+ppmd_decoder.order], al
        lodsd
        mov     [ebp+ppmd_decoder.usedMemorySize], eax
; CSubAllocator::CSubAllocator
        xor     eax, eax
        mov     [ebp+ppmd_decoder.GlueCount], al
        lea     edi, [ebp+ppmd_decoder.LoUnit]
        mov     ecx, (ppmd_decoder.SEE2Cont - ppmd_decoder.LoUnit)/4
        rep     stosd
; CSubAllocator::StartSubAllocator
        lea     eax, [ebp+ppmd_decoder.Base+ppmd_decoder.UNIT_SIZE]
        mov     [ebp+ppmd_decoder.HeapStart], eax
        mov     [ebp+streamInfo.fillBuf], ppmd_decoder.fillBuf
        ret

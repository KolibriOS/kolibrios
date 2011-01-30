
_DEBUG = 1

; --- Parameters inside WndProc

hWnd equ [ebp + 8]
uiMsg equ [ebp + 12]
WParam equ [ebp + 16]
LParam equ [ebp + 20]

; --- Splitting WndProc parameters

macro mcLPtoXY LParam {
    ; OUT: EDX = Y coordinate or HEIGHT
    ; OUT: EAX = X coordinate or WIDTH
    mov     eax, LParam
    mov     edx, LParam
    and     eax, 0000FFFFh
    shr     edx, 16
}

; --- String operation (WCHAR)

macro mcStrCopy {
    xor     eax, eax
@@:
    lodsw
    stosw
    test    eax, eax
    jnz     @r
}

; --- Indirectly call STDCALL procedure

macro invoke api_proc, [arg]
{
    common
    if ~ arg eq
    reverse
    pushd arg
    common
    end if
    call [api_proc]
}

macro rinvoke api_proc_reg, [arg]
{
    common
    if ~ arg eq
    reverse
    pushd arg
    common
    end if
    call api_proc_reg
}

; --- FPU

macro mcFpuPushR32 r32
{
    mov     [glb_FPU_Int32], r32
    fild    [glb_FPU_Int32]
}

macro mcFpuPop
{
    fstp    [glb_dbl_FpuPopper]
}

; --- Heap allocator

macro mcMemAlloc hHeap, nBytes {
    invoke  HeapAlloc, hHeap, HEAP_NO_SERIALIZE, nBytes
}

macro mcMemFree hHeap, pBuf {
    invoke  HeapFree, hHeap, HEAP_NO_SERIALIZE, pBuf
}

macro mcGlobalMalloc nBytes {
    mcMemAlloc [glb_HAllocator], nBytes
}

macro mcGlobalFree pBuf {
    mcMemFree [glb_HAllocator], pBuf
}

; --- Page allocator

macro mcAllocatePages nBytes {
    invoke  VirtualAlloc, 0, nBytes, \
            MEM_RESERVE or MEM_COMMIT, \
            PAGE_READWRITE
}

macro mcReleasePages pData {
    invoke  VirtualFree, pData, 0, MEM_RELEASE
}

; --- Local variables

macro mcLoadLocalRef AnyReg, instance {
    lea     AnyReg, [esp + instance]
}

macro mcLoadLocal AnyReg, instance {
    mov     AnyReg, [esp + instance]
}

macro mcStoreThis {
    mov     [esp], ebx
}

macro mcLoadThis {
    mov     ebx, [esp]
}

macro mcStoreLocal instance, AnyReg {
    mov     [esp + instance], AnyReg
}

macro mcBeginLocals LocalSize {
    sub     esp, LocalSize
}

macro mcEndLocals LocalSize {
    add     esp, LocalSize
}

; --- Virtual call on the object instance

macro mcVirtCallOnEBX OfsMethod {
    mov     eax, [ebx]
    call    dword [eax + OfsMethod]
}

macro mcVirtCall Instance, OfsMethod {
    lea     ebx, [Instance]
    mov     eax, [ebx]
    call    dword [eax + OfsMethod]
}

; --- CALLBACK

macro mcProlog {
    push    ebp
    mov     ebp, esp
    push    ebx
    push    esi
    push    edi
}

macro mcEpilog {
    pop     edi
    pop     esi
    pop     ebx
    leave
}

; --- Misc...

macro mc_CMP_ST0_ST1
{
    fcomip  st, st1
    fstp    [glb_dbl_FpuPopper]
}

macro mcLoadQuadStatePtr r32
{
    mov     r32, [glb_pGameState]
    add     r32, TGameState.game_QuadState
}

macro mcLoadNeg1 r32
{
    mcZeroBits r32
    dec     r32
}

macro mcLoad8bitsToReg32 r32, _8bit_value
{
    push    _8bit_value
    pop     r32
}

macro mcLoad1 r32
{
    mcZeroBits r32
    inc     r32
}

; ESI = source
; EDI = destination
macro mcMemCopy nBytesAligned4
{
    mov     ecx, nBytesAligned4
    shr     ecx, 2
    rep     movsd
}

macro mcMul10 r32 {
    push    esi
    shl     r32, 1
    push    r32
    shl     r32, 2
    pop     esi
    add     r32, esi
    pop     esi
}

macro mcZeroBits any_reg {
    xor     any_reg, any_reg
}

macro mcLoadRGB reg32, r8, g8, b8 {
    mov     reg32, (b8 shl 16) or (g8 shl 8) or r8
}

; --- Rectangle

macro mcRectCopy {
    ; ESI = source rectangle
    ; EDI = target rectangle
    movsd
    movsd
    movsd
    movsd
}

macro mcRectCopySafe {
    ; ESI = source rectangle
    ; EDI = target rectangle
    push    esi
    movsd
    movsd
    movsd
    movsd
    pop     esi
}

macro mcRectWidth r32, pRect {
    mov     r32, [pRect + RECT.rc_Right]
    sub     r32, [pRect + RECT.rc_Left]
}

macro mcRectHeight r32, pRect {
    mov     r32, [pRect + RECT.rc_Bottom]
    sub     r32, [pRect + RECT.rc_Top]
}

; --- Branching

macro mcOnRegEqu AnyReg, AnyValue, AnyLabel {
    cmp     AnyReg, AnyValue
    je      AnyLabel
}

macro mcOnRegNotEqu AnyReg, AnyValue, AnyLabel {
    cmp     AnyReg, AnyValue
    jne     AnyLabel
}

macro mcOnRegZero AnyReg, AnyLabel {
    test    AnyReg, AnyReg
    jz      AnyLabel
}

macro mcOnRegNotZero AnyReg, AnyLabel {
    test    AnyReg, AnyReg
    jnz     AnyLabel
}

; --- OOP

macro mcLoadMemberRef AnyReg, instance {
    lea     AnyReg, [ebx + instance]
}

macro mcLoadMember AnyReg, instance {
    mov     AnyReg, [ebx + instance]
}

macro mcStoreMember instance, AnyReg {
    mov     [ebx + instance], AnyReg
}

macro mcAttachVTable addrvtbl {
    mov     eax, addrvtbl
    mov     [ebx], eax
}

; --- EOF ---

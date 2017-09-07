; to compile: nasm -f bin fillscr.asm -o fillscr ;
; to generate  random  colors use "fillscr rnd"  ;
; otherwise use "filscr r,g,b, r,g,b, r,g,b,..." ;
ORG 0
BITS 32
; ---------------------------- ;
points         equ END
POINTS_SIZE    equ 108
PARAMS_SIZE    equ 256
; ---------------------------- ;
MENUET01       db 'MENUET01'
version        dd 1
program.start  dd START
program.end    dd END
program.memory dd END + POINTS_SIZE + PARAMS_SIZE
program.stack  dd 0
program.params dd END + POINTS_SIZE
program.path   dd 0
; ---------------------------- ;
points_count   dd 0
rnd            dd 0
; ---------------------------- ;
START:
        mov    edi, points
        mov    esi, [program.params]
        cmp    [esi], dword "rnd"
        jne    .not_rnd
        mov    [points_count], dword POINTS_SIZE / 3
        mov    ecx, POINTS_SIZE / 2
.next_rnd:
        rdtsc
        xor    eax, edx
        xor    [rnd], eax
        imul   eax, [rnd], 134775813
        add    eax, 2531011
        mov    [rnd], eax
        shr    eax, 16
        mov    [edi], ax
        inc    edi
        inc    edi
        loop   .next_rnd
        jmp    .exit
.not_rnd:
        xor    ebp, ebp
.next:
.skip_spaces:
        cmp    [esi], byte " "
        jne    .spaces_skipped
        inc    esi
        jmp    .skip_spaces
.spaces_skipped:
;         cmp [esi], byte 0
;         je .exit
        mov    eax, esi
.find_end_or_comma:
        cmp    [esi], byte ","
        je     .end_or_comma_found
        cmp    [esi], byte 0
        je     .end_or_comma_found
        inc    esi
        jmp    .find_end_or_comma
.end_or_comma_found:
; in eax start   of number
; in esi pointer to comma or end after number
        mov    ecx, esi
        sub    ecx, eax
        xor    ebx, ebx
        cmp    cl, 1
        jne    .cmp2
        xor    cl, cl
        jmp    .1
.cmp2:
        cmp    cl, 2
        jne    .cmp3
        xor    cl, cl
        jmp    .2
.cmp3:
        cmp    cl, 3
        jne    .exit
        xor    cl, cl
.3:
        movzx  edx, byte[eax + ecx]
        sub    dl, 48
        shl    dl, 2
        lea    edx, [edx * 4 + edx]
        lea    edx, [edx * 4 + edx]
        add    bl, dl
        inc    ecx
.2:
        movzx  edx, byte[eax + ecx]
        sub    dl, 48
        shl    dl,  1
        lea    edx, [edx * 4 + edx]
        add    bl, dl
        inc    ecx
.1:
        movzx  edx, byte[eax + ecx]
        sub    dl, 48
        add    bl, dl
        test   ebp, ebp
        jnz    .cmp_next
        mov    [edi + 2], bl
        jmp    .putted
.cmp_next:
        cmp    ebp, 2
        jne    .cmp_next1
        mov    [edi - 2], bl
        jmp    .putted
.cmp_next1:
        mov    [edi], bl
.putted:
        inc    ebp
        cmp    ebp, 3
        jne    .not_3
        xor    ebp, ebp
        inc    dword[points_count]
.not_3:
        inc    edi
        inc    esi
        jmp    .next
.exit:
; width = height = sqrt(points_count)
        mov    eax, [points_count]
        or     edx, -1
@@:
        add    edx, 2
        sub    eax, edx
        jnle   @@
        sbb    edx, -1
        shr    edx, 1
; set width, height
        mov    eax, 15
        mov    ebx, 1
        mov    ecx, edx
        int    64
; set "stretch"
;         mov eax, 15
        mov    ebx, 4
        mov    ecx, 2
        int    64
; put pixels
;         mov eax, 15
        mov    ebx,  5
        mov    ecx, points       ; BBGGRRBBGGRR...
        xor    edx, edx
        mov    esi, [points_count] ; size of data = count * 3
        lea    esi, [esi * 2 + esi]
        int    64
; refresh screen
;         mov eax, 15
        mov    ebx, 3
        int    64
; thread terminate
        mov    eax, -1
        int    64
END:
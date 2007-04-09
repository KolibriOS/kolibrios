;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                      ;;
;;                  SWITCH TO 32 BIT PROTECTED MODE                     ;;
;;                                                                      ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

os_data        =  os_data_l-gdts    ; GDTs
os_code        =  os_code_l-gdts
int_code       equ  int_code_l-gdts
int_data       equ  int_data_l-gdts
tss0sys        equ  tss0sys_l-gdts
graph_data     equ  3+graph_data_l-gdts
tss0           equ  tss0_l-gdts
app_code       equ  3+app_code_l-gdts
app_data       equ  3+app_data_l-gdts



; CR0 Flags - Protected mode and Paging

        mov ecx, CR0_PE

; Enabling 32 bit protected mode

        sidt    [cs:old_ints_h-0x10000]

        cli                             ; disable all irqs
        cld
        mov     al,255                  ; mask all irqs
        out     0xa1,al
        out     0x21,al
   l.5: in      al, 0x64                ; Enable A20
        test    al, 2
        jnz     l.5
        mov     al, 0xD1
        out     0x64, al
   l.6: in      al, 0x64
        test    al, 2
        jnz     l.6
        mov     al, 0xDF
        out     0x60, al
   l.7: in      al, 0x64
        test    al, 2
        jnz     l.7
        mov     al, 0xFF
        out     0x64, al
        lgdt    [cs:gdts-0x10000]       ; Load GDT
        mov     eax, cr0                ; Turn on paging // protected mode
        or      eax, ecx
        and     eax, 10011111b *65536*256 + 0xffffff ; caching enabled
        mov     cr0, eax
        jmp     $+2
org $+0x10000
        mov     ax,os_data              ; Selector for os
        mov     ds,ax
        mov     es,ax
        mov     fs,ax
        mov     gs,ax
        mov     ss,ax
        mov     esp,0x3ec00             ; Set stack
        jmp     pword os_code:B32       ; jmp to enable 32 bit mode

if gdte >= $
error 'GDT overlaps with used code!'
end if

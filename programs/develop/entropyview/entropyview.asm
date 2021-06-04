; EntropyView - file entropy visualisation
; rgimad 2021
 
; header:
use32              
        org     0
        db      'MENUET01'   ; magic
        dd      1            ; header version
        dd      _start       ; entry point
        dd      _i_end       ; program size
        dd      _mem         ; memory size
        dd      _stacktop    ; stack top addr
        dd      cmdline      ; buf for args
        dd      0            ; reversed


__DEBUG__       = 1             ; 0 - disable debug output / 1 - enable debug output
__DEBUG_LEVEL__ = DBG_ERR       ; set the debug level
 
DBG_ALL       = 0  ; all messages
DBG_INFO      = 1  ; info and errors
DBG_ERR       = 2  ; only errors

WND_START_X     = 200
WND_START_Y     = 200
WND_WIDTH       = 540
WND_HEIGHT      = 275

COL_KOEF        = 7
COL_WIDTH       = 2
HIST_Y          = 230
HIST_X          = 10

TMP_BUF_SIZE    = 4096
 
include '../../macros.inc'
purge   mov, add, sub

include '../../debug-fdo.inc'
include '../../proc32.inc'
 
_start:
        cmp     byte [cmdline], 0 ; if no argument then print usage and exit 
        jne     @f     

        mov     dword [notify_struct.msg], msg_print_usage
        mcall   70, notify_struct
        mcall   -1
@@:
        stdcall calculate_entropy

        ; print the byte table (for debug purposes)
        ; xor     ecx, ecx
        ; .table_loop:
        ;     cmp     ecx, 256
        ;     jae     .end_table_loop
        ;     mov     ebx, dword [byte_table + ecx*4]
        
        ;     DEBUGF  DBG_INFO, "number of bytes %x = %u\n", ecx, ebx

        ;     inc     ecx
        ;     jmp     .table_loop
        ; .end_table_loop:
 

; event loop:
event_loop:
        mcall   10            ; wait for event
 
        cmp     eax, 1        ; redraw event
        je      on_redraw
 
        cmp     eax,3         ; button event
        je      on_button        
 
        jmp     event_loop
 
 
on_button:
        mcall   17            ; 17 - get key code
        cmp     ah, 1         ; if key with code 1 is not pressed then continue
        jne     event_loop
        mcall   -1 ; else exit
 
; define and draw window
align 4
on_redraw:
        mcall   12, 1       ; begin redraw
        mcall   48, 3, sc,sizeof.system_colors
 
        mov     edx, [sc.work] ; background color
        or      edx, 0x34000000        ; window type
        mcall   0, <WND_START_X, WND_WIDTH>, <WND_START_Y, WND_HEIGHT>, , , wnd_title

        ; draw bottom line
        mov     eax, 38
        mov     ebx, HIST_X
        shl     ebx, 16
        add     ebx, HIST_X + COL_WIDTH*255
        mov     ecx, HIST_Y
        shl     ecx, 16
        add     ecx, HIST_Y
        mov     edx, 0x00FF0000
        int     0x40

        ; visualize table:
        xor     ecx, ecx
.table_loop:
        cmp     ecx, 256
        jae     .end_table_loop
        mov     ebx, dword [byte_table + ecx*4] ; ebx = frequency of ecx value
        
        mov     esi, ecx
        imul    esi, COL_WIDTH ; esi = x of column

        push    COL_KOEF
        fild    dword [esp]
        add     esp, 4

        push    ebx
        fild    dword [esp]
        add     esp, 4

        fyl2x   ; fpu stack top = COL_KOEF*log_2(ebx)

        sub     esp, 4
        fistp   dword [esp]
        pop     eax

        mov     ebp, eax ; ebp = height of column
        mov     edi, HIST_Y
        sub     edi, eax ; edi = y of left upper corner of column 

        push    ecx
        ; DEBUGF  DBG_INFO, "drawing rect x = %u y = %u, height = %u\n", esi, edi, ebp
        mov     eax, 13
        mov     ebx, esi
        add     ebx, HIST_X
        shl     ebx, 16
        add     ebx, COL_WIDTH - 1
        mov     ecx, edi
        shl     ecx, 16
        add     ecx, ebp
        mov     edx, 0x80000000
        int     0x40
        pop     ecx


        inc     ecx
        jmp     .table_loop
.end_table_loop:
        ; DEBUGF  DBG_INFO, "esi = %u\n", esi
 
        mcall   12, 2                  ; end draw
        jmp     event_loop

; calculate entropy of file
align 4
proc calculate_entropy stdcall
        stdcall _memset, byte_table, 0, 256*4
        mov     dword [file_size], 0
        DEBUGF  DBG_INFO, "starting reading blocks...\n"
.read_block:
        mov     eax, 70
        mov     ebx, fread_struct
        int     0x40

        add     dword [fread_struct.offset_low], ebx ; add how many was read
        ; mov     ebp, dword [fread_struct.offset_low]
        ; DEBUGF  DBG_INFO, "file pos = %u\n", ebp

        cmp     eax, 6 ; if EOF its normal so skip next check
        je      @f

        test    eax, eax ; if error occured
        jnz     .fail

@@:
        mov     edx, TMP_BUF_SIZE
        cmp     eax, 6
        jne     @f
        mov     edx, ebx ; if eof then use how many read, not max block size
@@:
        xor     ecx, ecx
        .buf_loop:
            cmp     ecx, edx
            jae     .end_buf_loop

            movzx   ebx, byte [tmp_buf + ecx]
            shl     ebx, 2     ; ebx *= 4;
            add     ebx, byte_table
            inc     dword [ebx]
            inc     ecx
            jmp     .buf_loop
        .end_buf_loop:

        cmp     eax, 6 ; if EOF
        je      .end_read

        jmp     .read_block

.end_read:
        mov     eax, dword [fread_struct.offset_low]
        mov     dword [file_size], eax
        DEBUGF  DBG_INFO, "calculate_entropy end...\n"
        ret

.fail:
        DEBUGF DBG_ERR, "error reading file, code = %u\n", eax
        mov     dword [notify_struct.msg], msg_file_not_found
        mcall   70, notify_struct
        mcall   -1
        ret
endp


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


; data:
include_debug_strings ; for debug-fdo

align 4
fread_struct:
    .subfunction    dd 0               ; + 0
    .offset_low     dd 0               ; + 4
    .offset_high    dd 0               ; + 8
    .size           dd TMP_BUF_SIZE    ; + 12
    .buffer         dd tmp_buf         ; + 16
                    db 0               ; + 20
    .filename:      dd cmdline         ; + 24

file_size           dd 0

sc              system_colors
wnd_title       db 'EntropyView 0.0.1', 0
msg_file_not_found  db '"File not found" -tE', 0    
msg_print_usage  db '"Use from shell like:\nentropyview somefile.txt" -tI', 0 

notify_struct:
        dd 7            ; run application
        dd 0
  .msg  dd ?
        dd 0
        dd 0
        db '/sys/@notify', 0
 
; reverved data:

align 16
_i_end:
    cmdline     rb 1024  ; reserve for command line arguments
    tmp_buf     rb TMP_BUF_SIZE ; temporary buffer for reading file
    byte_table  rd 256   ; table which stores how many times each byte value(0-255) occured in the file

    rb          4096     ; for stack
 
align 16
_stacktop:               ; stack top label, stack grows downwards
                       
_mem:                    ; end
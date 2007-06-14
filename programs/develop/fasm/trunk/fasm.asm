;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                 ;;
;;  flat assembler source                          ;;
;;  Copyright (c) 1999-2006, Tomasz Grysztar       ;;
;;  All rights reserved.                           ;;
;;                                                 ;;
;;  Menuet port by VT                              ;;
;;                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

NORMAL_MODE    = 8
CONSOLE_MODE   = 32

MAGIC1         = 6*(text.line_size-1)+14
MAX_PATH       = 100

APP_MEMORY     = 0x00800000

;; Menuet header

appname equ "flat assembler "

use32

  org 0x0
  db 'MENUET01'  ; 8 byte id
  dd 0x01        ; header version
  dd START       ; program start
  dd program_end ; program image size
  dd APP_MEMORY  ; required amount of memory
  dd 0xDFFF0     ; stack
  dd params,0x0  ; parameters,icon

include 'lang.inc'
include '..\..\..\macros.inc'
purge add,sub    ; macros.inc does incorrect substitution
include 'fasm.inc'

center fix true

START:	    ; Start of execution
        mov     edi, fileinfos
        mov     ecx, (fileinfos_end-fileinfos)/4
        or      eax, -1
        rep     stosd

   cmp    [params],0
   jz	    red

   mov    ecx,10
   mov    eax,'    '
   mov    edi,infile
   push   ecx
   cld
   rep    stosd
   mov    ecx,[esp]
   mov    edi,outfile
   rep    stosd
   pop    ecx
   mov    edi,path
   rep    stosd

   mov     esi,params
;  DEBUGF  "params: %s\n",esi
   mov     edi,infile
   call    mov_param_str
;  mov     edi,infile
;  DEBUGF  " input: %s\n",edi
   inc     esi
   mov     edi,outfile
   call    mov_param_str
;  mov     edi,outfile
;  DEBUGF  "output: %s\n",edi
   inc     esi
   mov     edi,path
   call    mov_param_str
;  mov     edi,path
;  DEBUGF  "  path: %s\n",edi

   cmp     [esi], dword ',run'
   jne     @f
   mov     [_run_outfile],1
  @@:

   mov     [_mode],CONSOLE_MODE
   jmp     start


red:	; Redraw
    call draw_window

still:  
    push 10          ; Wait here for event
    pop eax 
    mcall 
    dec eax 
    je  red          ; Redraw request
    dec eax 
    jne button       ; Button in buffer

key:                 ; Key
    mov  al,2        ; Read it and ignore
    mcall
    jmp  still

button:    ; Button in Window

    mov  al,17
    mcall

    cmp     ah,1
    jne     noclose
    or      eax,-1
    mcall

noclose:    
    cmp  ah,2         ; Start compiling
    je   start
    cmp  ah,3         ; Start compiled file
    jnz  norunout

    mov  edx,outfile
    call make_fullpaths
    mcall  70,file_info_start
;   xor   ecx,ecx
    jmp  still
   norunout:
    cmp  ah,4
    jnz  norundebug

    mov  edx,outfile
    call make_fullpaths
    mcall 70,file_info_debug
    jmp  still

   norundebug:
    mov  ecx,5
    mov  [ya],ecx

    cmp  ah,11	   ; Infile
    je   f1
    cmp  ah,12	   ; Outfile
    je   f2
    cmp  ah,13	   ; Path
    je   f3
    cmp  ah,14
    je   f4

    jmp  still

f4:
        xor     [bGenerateDebugInfo], 1
        mcall   8,,,0x8000000E
        call    draw_checkbox
        jmp     still

draw_window:

    pusha

    mcall  12,1 ; Start of draw

    get_sys_colors 1,0

    xor  eax,eax                     
    mov  ebx,50*65536+280
    mov  ecx,50*65536+260
    mov  edx,[sc.work]
    or   edx,0x33000000
    mov  edi,title             ; Draw Window Label Text
    mcall

    mcall 9,PROCESSINFO,-1	    

    mpack ecx,1,1
    mov   ebx,[pinfo.box.width]
    sub   ebx,10

    push  ecx
    madd  ecx, 14*3+16+2, 14*3+16+2
    mcall 38,,,[sc.work_graph]
    pop   ecx

    sub   ebx,MAGIC1+3
    mcall
    madd  ecx, 14, 14
    mcall
    madd  ecx, 14, 14
    mcall
    madd  ecx, 14, 14
    mcall
    push  ebx
    mpack ebx,MAGIC1,MAGIC1
    sub   ecx, 14*3
    mcall
    mov   ebx,[esp-2]
    pop   bx
    mcall
    add   esp,2

    mpack ebx,0,MAGIC1-1
    mpack ecx,1+1, 14-2
    mcall 8,,,0x4000000B       ; Button: Enter Infile
    madd  ecx, 14,0
    mcall  ,,,0x4000000C       ; Button: Enter Outfile
    madd  ecx, 14,0
    mcall  ,,,0x4000000D       ; Button: Enter Path

    mpack ebx,[pinfo.box.width],MAGIC1
    msub  ebx,MAGIC1+10+1,0
    mpack ecx,0, (14*3+16)/3-1
    madd  ecx,1,0
    mcall  ,,,0x00000002,[sc.work_button]
    madd  ecx, (14*3+16)/3+1,0
    mcall  ,,,0x00000003
    madd  ecx, (14*3+16)/3+1,0
    mcall ,,,4

    mpack ebx,6,0    ; Draw Window Text
    add  ebx,1+ 14/2-3
    mov  ecx,[sc.work_text]
    mov  edx,text
    mov  esi,text.line_size
    mov  eax,4
   newline:
    mcall
    add  ebx, 14
    add  edx,text.line_size
    cmp  byte[edx],'x'
    jne  newline

    mov   ebx,[pinfo.box.width]
    sub   ebx,MAGIC1+10+1-9
    shl   ebx,16
    add   ebx,1+( (14*3+16)/3-1)/2-3
    mcall  ,,[sc.work_button_text],s_compile,7
    add   ebx,(14*3+16)/3+1
    mcall ,,,s_run
    add   ebx,(14*3+16)/3+1
    mcall ,,,s_debug

    mpack ebx,MAGIC1+6,0
    add   ebx,1+ 14/2-3+ 14*0
    mov   esi,[pinfo.box.width]
    sub   esi,MAGIC1*2+5*2+6+3
    mov   eax,esi
    mov   cl,6
    div   cl
    cmp   al,MAX_PATH
    jbe   @f
    mov   al,MAX_PATH
@@: movzx esi,al
    mcall 4,,[sc.work_text],infile
    add   ebx,14
    mcall ,,,outfile
    add   ebx,14
    mcall ,,,path

        call    draw_checkbox

    call  draw_messages

    mcall  12,2 ; End of Draw

    popa
    ret

bottom_right dd ?

draw_checkbox:
        mcall   8,<5,10>,<14*3+5,10>,14,[sc.work_button]
        cmp     [bGenerateDebugInfo], 0
        jz      @f
        mov     edx, [sc.work_button_text]
        mcall   38,<7,13>,<14*3+7,14*3+13>
        mcall   38,,<14*3+13,14*3+7>
@@:
        mov     ecx, [sc.work_text]
        or      ecx, 0x80000000
        mcall   4,<20,14*3+7>,,s_dbgdescr
        ret

draw_messages:
    mov    eax,13      ; clear work area
    mpack  ebx,7-2,[pinfo.box.width]
    sub    ebx,5*2+7*2-1-2*2
    mpack  ecx,0,[pinfo.box.height]
    madd   ecx, 14*3+16+1+7+1,-( 14*3+16+1+7*2+25)
    mov    word[bottom_right+2],bx
    mov    word[bottom_right],cx
    msub   [bottom_right],7,11
    add    [bottom_right],7 shl 16 + 53
    mov    edx,[sc.work]
    mcall
_cy = 0
_sy = 2
_cx = 4
_sx = 6
    push   ebx ecx
    mpack  ebx,4,5
    add    bx,[esp+_cx]
    mov    ecx,[esp+_sy-2]
    mov    cx,[esp+_sy]
    msub   ecx,1,1
    mcall  38,,,[sc.work_graph]
    mov    si,[esp+_cy]
    add    cx,si
    shl    esi,16
    add    ecx,esi
    madd   ecx,1,1
    mcall
    mpack  ebx,4,4
    mov    esi,[esp+_sy-2]
    mov    si,cx
    mov    ecx,esi
    mcall
    mov    si,[esp+_cx]
    add    bx,si
    shl    esi,16
    add    ebx,esi
    madd   ebx,1,1
    mcall
    pop    ecx ebx
    ret

; read string

f1: mov  [addr],infile
    add  [ya], 14*0
    jmp  rk
f2: mov  [addr],outfile
    add  [ya], 14*1
    jmp  rk
f3: mov  [addr],path
    add  [ya], 14*2
rk:

    mov  edi,[addr]
    mov  al,0
    mov  ecx,MAX_PATH
    add  edi,ecx
    dec  edi
    std
    repe scasb
    sub  ecx,MAX_PATH
    neg  ecx
    mov  al,$1C ; ''
    add  edi,2
    push  edi
    cld
    rep  stosb
    call print_text
    pop edi
f11:mcall  10
    cmp    eax,2
    jne read_done
    mcall; 2
    shr    eax,8
    cmp    al,13
    je	  read_done
    cmp    al,8
    jne nobs
    cmp    edi,[addr]
    je	  f11
    sub    edi,1
    mov    byte[edi],$1C ; '_'
    call   print_text
    jmp    f11
   nobs:
    movzx  ebx,al
    sub    ebx,$20
    jle    f11
    sub    al,[sub_table+ebx]
   keyok:
    mov    ecx,[addr]
    add    ecx,MAX_PATH
    cmp    edi,ecx
    jae    f11
    mov    [edi],al

    call   print_text
    inc    edi
    jmp f11

  read_done:

    mov    ecx,[addr]
    add    ecx,MAX_PATH
    sub    ecx,edi
    mov    al,0;' '
    cld
    rep    stosb
    call   print_text

    jmp    still

print_text:

    mpack ebx,MAGIC1+6,[pinfo.box.width]
    sub   ebx,MAGIC1*2+19
    movzx esi,bx
    mov   ecx,[ya-2]
    mov   cx,8
    mcall 13,,,[sc.work]

    mpack ebx,MAGIC1+6,[ya]
    mov   eax,esi
    mov   cl,6
    div   cl
    cmp   al,MAX_PATH
    jbe   @f
    mov   al,MAX_PATH
@@: movzx esi,al
    mcall 4,,[sc.work_text],[addr]

    ret


; DATA

text:
  db ' INFILE:'
.line_size = $-text
  db 'OUTFILE:'
  db '   PATH:'
  db 'x'

s_compile db 'COMPILE'
s_run     db '  RUN  '
s_debug   db ' DEBUG '

s_dbgdescr      db      'Generate debug information',0

infile	  db 'example.asm'
  times MAX_PATH+$-infile  db 0
outfile db 'example'
  times MAX_PATH+$-outfile db 0
path	db '/rd/1/'
  times MAX_PATH+$-path    db 0

lf db 13,10,0

addr dd 0x0
ya   dd 0x0
zero db 0x0

mov_param_str:
  @@:
    mov    al,[esi]
    cmp    al,','
    je	     @f
    cmp    al,0
    je	     @f
    mov    [edi],al
    inc    esi
    inc    edi
    jmp    @b
  @@:
    mov    al,0
    stosb
ret

start:
    cmp    [_mode],NORMAL_MODE
    jne    @f
    call   draw_messages
    mov    [textxy],7 shl 16 + 70
@@:
    mov    esi,_logo
    call   display_string

 ;
 ;   Fasm native code
 ;

    mov    [input_file],infile
    mov    [output_file],outfile

    call   init_memory

    call   make_timestamp
    mov    [start_time],eax

    call   preprocessor
    call   parser
    call   assembler
    cmp    [bGenerateDebugInfo], 0
    jz     @f
    call   symbol_dump
@@:
    call   formatter

    call   display_user_messages
    movzx  eax,[current_pass]
    inc    eax
    call   display_number
    mov    esi,_passes_suffix
    call   display_string
    call   make_timestamp
    sub    eax,[start_time]
    xor    edx,edx
    mov    ebx,100
    div    ebx
    or	     eax,eax
    jz	     display_bytes_count
    xor    edx,edx
    mov    ebx,10
    div    ebx
    push   edx
    call   display_number
    mov    dl,'.'
    call   display_character
    pop    eax
    call   display_number
    mov    esi,_seconds_suffix
    call   display_string
  display_bytes_count:
    mov    eax,[written_size]
    call   display_number
    mov    esi,_bytes_suffix
    call   display_string
    xor    al,al

    cmp    [_run_outfile],0
    je     @f
    mov    edx,outfile
    call   make_fullpaths
    mov    eax,70
    mov    ebx,file_info_start
    xor    ecx,ecx
    mcall
@@:
    jmp    exit_program


include 'system.inc'
include 'version.inc'
include 'errors.inc'
include 'expressi.inc'
include 'preproce.inc'
include 'parser.inc'
include 'assemble.inc'
include 'formats.inc'
include 'x86_64.inc'
include 'tables.inc'
include 'symbdump.inc'

title db appname,VERSION_STRING,0

_logo db 'flat assembler  version ',VERSION_STRING,13,10,0

_passes_suffix db ' passes, ',0
_seconds_suffix db ' seconds, ',0
_bytes_suffix db ' bytes.',13,10,0

_include db 'INCLUDE',0

_counter db 4,'0000'

_mode	       dd NORMAL_MODE
_run_outfile  dd 0
bGenerateDebugInfo db 0

sub_table:
times $41 db $00
times $1A db $20
times $25 db $00
times $10 db $20
times $30 db $00
times $10 db $50
times $04 db $00,$01
times $08 db $00

;include_debug_strings

params db 0 ; 'TINYPAD.ASM,TINYPAD,/HD/1/TPAD4/',
program_end:
rb 1000h

align 4

include 'variable.inc'

program_base dd ?
buffer_address dd ?
memory_setting dd ?
start_time dd ?

dbgfilename     rb      MAX_PATH+4

sc    system_colors
max_handles = 8
fileinfos rb (4+20+MAX_PATH)*max_handles
fileinfos_end:
pinfo process_information

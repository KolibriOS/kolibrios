; Include file for dumping user apps' memory through new debug BOARD

; Max amount of bytes to be dumped
IPC_BUF equ 160

; Dump macro parameters:
;     ptr - pointer to memory dumped
;     len - dump length
;     workarea - any work area for sysfunc 9
;     run_new  - if not empty, run BOARD unless it is running already

macro Dump ptr, len, workarea,run_new
{
  local .exist,.lt
  pusha
  mov  ebx,workarea
  call Board_seek
if ~ run_new eq
  test edx,edx
  jne .exist
  mcall 70,Board_seek.board_fn,0
  mov  edx,eax
  mcall 5,20
end if
 .exist:
  mov  esi,len
  cmp  esi,IPC_BUF
  jbe  .lt
  mov  esi,IPC_BUF
 .lt:
  mcall 60,2,edx,ptr
  popa
}

if used Board_seek
Board_seek:
; ebx - prcinfo
  xor  edx,edx
  mcall 9,,-1
  mov  ecx,eax
  mov  esi,dword[.board_fn]
.lp:
  mcall 9
  cmp  dword[ebx+10],esi
;  jne  .no
;  cmp  dword[ebx+42],399
  je   .ok
.no:
  loop .lp
  ret
.ok:
  mov  edx,[ebx+30]
  ret
.board_fn:
        dd      7
        dd      0
        dd      0
        dd      0
        dd      0
        db      '/RD/1/BOARD',0
end if

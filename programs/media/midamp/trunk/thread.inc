new_thread:
    mov  ecx,ipcarea2
    call init_ipc
    mcall 40,1000000b
  tstill:
    mov  dword[ipcarea2],0
    mov  dword[ipcarea2+4],8
    mcall 10
  .prc:
    cmp  eax,7
    jne  tstill
    and  [flag],not FL_PLAY
    mov  eax,dword[ipcarea2+8]
    cmp  eax,[parentPID]
    jne  tstill
    cmp  byte[ipcarea2+16],IPC_PLAY
    je   .play_next
    cmp  byte[ipcarea2+16],IPC_TRIG
    jne  tstill
  .play_next:
    or   [flag],FL_PLAY
    mov  edi,[cur_ptr]
    movzx eax,word[edi]
    mov  [play_area],ax
    add  [cur_ptr],2
  if SOUND eq ON
    test [flag],FL_MUTE
    jnz   .nosound
    mov  esi,play_area
    mov  eax,55
    mov  ebx,eax
    mcall
  .nosound:
  end if
    movzx ebx,byte[play_area]
    and  ebx,0x7f
    add  [cur_tick],ebx
    mov  eax,23
    mcall
    cmp  word[edi],0
    jne  .checkevt
    mov  eax,IPC_NEXT
    call ipc_send
    jmp  tstill
  .checkevt:
    test eax,eax
    jz   .upd
    cmp  eax,7
    jne  tstill
    cmp  byte[ipcarea2+16],IPC_PLAY
    jne  tstill
  .upd:
    mov  eax,IPC_UPDT
    call ipc_send
    jmp  .play_next

respawn:
    mcall 9,prcinfo,-1
    mov  ecx,eax
    xor  edx,edx
    xor  esi,esi
  .nxt:
    mcall 9
    cmp  dx,[ebx+4]
    jae  .less
    mov  dx,[ebx+4]
    mov  esi,[ebx+30]
  .less:
    loop .nxt
    push esi
    mcall 51,1,new_thread,APP_MEM
    pop  edx
    cmp  edx,[childPID]
    jne  .nochild
    mov  edx,[parentPID]
  .nochild:
    mov  [childPID],eax
    call prc_find
    mcall 18,3
    ret

kill:
    mov  edx,[childPID]
    call prc_find
    mcall 18,2
  .nothread:
    ret

prc_find: ;in: edx-PID, out: ecx-number
    mcall 9,prcinfo,-1
    mov  ecx,eax
  .nxt:
    mcall 9
    cmp  edx,[ebx+30]
    je   .found
    loop .nxt
  .found:
    ret

init_ipc:
    mcall 60,1,,20
    ret

ipc_send: ;eax-msg
   test [flag],FL_LOCK
   jnz  .noipc
   pusha
   mov  [ipcmsg],al
   mov  ebx,ipcarea2
   mov  ecx,childPID
   cmp  eax,0xb0
   jb   .noparent
   add  ecx,4
   sub  ebx,20
 .noparent:
   mov  dword[ebx],0
   mov  dword[ebx+4],8
   mcall 60,2,[ecx],ipcmsg,1
   popa
  .noipc:
   ret

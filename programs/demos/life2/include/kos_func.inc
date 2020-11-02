;const int
MENUET_BORDER_SIZE = 4;
;const int
MENUET_HEADER_SIZE = 20;

;const int
MENUET_THREAD_DATA_USER     = 0; // Thread data begin from the user dword
;const int
MENUET_THREAD_DATA_ST_BEGIN = 1; // Stack beginning follows after the user dword
;const int
MENUET_THREAD_DATA_NEXT     = 2;
;const int
MENUET_THREAD_DATA_PID	    = 3;
;const int
MENUET_THREAD_DATA_FLAG     = 4;
;const int
MENUET_THREAD_DATA_X	    = 5;
;const int
MENUET_THREAD_DATA_Y	    = 6;
;const int
MENUET_THREAD_DATA_C_WINDOW = 7;
;const int
MENUET_THREAD_DATA_C_HEADER = 8;
;const int
MENUET_THREAD_DATA_C_BORDER = 9;
;const int
MENUET_THREAD_DATA_C_TITLE  = 10;
;const int
MENUET_THREAD_DATA_TITLE    = 11;
;const int
MENUET_THREAD_DATA_PICTURE  = 12;
;const int
MENUET_THREAD_DATA_SZ_PICT  = 13;
;const int
MENUET_THREAD_DATA_LAST_SX  = 14;
;const int
MENUET_THREAD_DATA_LAST_SY  = 15;
;const int
MENUET_THREAD_DATA_LEN	    = 16;

;const int
MENUET_MUTEX_MAX_TIME_WAIT  = 20;

;const int
MENUET_FILE_BLOCK_SIZE	    = 512;

;const int
MENUET_FILE_MEMORY_OS_NEED  = 4096;

;/***

macro segment name
{
  segment name
  if name eq _init_ | name eq _INIT_
Menuet_SegmentInit:
  else if name eq _exit_ | name eq _EXIT_
Menuet_SegmentExit:
  end if
}

macro endseg  name
{
  if name eq _init_ | name eq _INIT_
Menuet_SegmentInitEnd:
  else if name eq _exit_ | name eq _EXIT_
Menuet_SegmentExitEnd:
  end if
  endseg  name
}

macro Menuet_Put_MovEaxVal_Ret	address,val
{
  mov  byte [address],0xB8
  mov  dword [address+4],0xC089C300
  mov  dword [address+1],val
}

proc @Kolibri@Main$qv
  and  esp,not 3
  sub  esp,1024
  mov  eax,9
  mov  ebx,esp
  mov  ecx,-1
  int  0x40
  mov  ebx,[esp+26]
  mov  edx,[esp+30]
  lea  eax,[ebx-0x20]
  add  esp,1024
  cmp  esp,eax
  cmova esp,eax
  and  esp,not 3
if defined @Kolibri@CommandLine
	mov	byte [@Kolibri@CommandLine+256], 0
end if
  xor  eax,eax
  cld
  mov  edi,@Kolibri@_ThreadTable
  mov  ecx,256
  rep stos dword [edi]
  mov  esi,@Kolibri@GetPid$qv
  mov  edi,@Kolibri@_ThreadSavedBegProc
  movs dword [edi],[esi]
  movs dword [edi],[esi]
  mov  esi,@Kolibri@GetThreadData$qv
  movs dword [edi],[esi]
  movs dword [edi],[esi]
  Menuet_Put_MovEaxVal_Ret  @Kolibri@GetPid$qv,edx
if defined MenuetHeapInit
  mov  ecx,esp
  push ebx
  push ecx
  push U_END
  call MenuetHeapInit	; Initialize a dynamic heap and create new memory in its begin.
  pop  ecx		; Parameters: begin of a new heap, end of data to create in
  mov  [esp+4],eax	; the begin of a heap. Return address of the created data.
  mov  dword [esp],0
else
  xor  eax,eax
  push eax
  push eax
end if
  call @Kolibri@ThreadMain$qpvt1
.ThreadFinish:
  add  esp,8
if defined MenuetHeapFreeAndThreadFinish
  test eax,eax
  jz   .ThreadFinish_end
  push dword @Kolibri@_ExitProcessNow
  push eax
  call MenuetHeapFreeAndThreadFinish   ; Free the given memory and finish the thread,
end if				       ; should exit process if second argument points to not zero.
.ThreadFinish_end:
  or   eax,-1
  int  0x40
endp

proc @Kolibri@ThreadMain$qpvt1
  xchg ebx,[esp+4]
  xchg ebp,[esp+8]
  push esi edi
  sub  esp,MENUET_THREAD_DATA_LEN*4
  mov  [esp],ebx
  mov  [esp+4],ebp
  mov  eax,40
  mov  ebx,0x27
  int  0x40
  mov  ebx,esp
  cmp  byte [@Kolibri@_ThreadSavedBegProc],0x90
  jz   .main_else_first_check
  Menuet_Put_MovEaxVal_Ret  @Kolibri@GetThreadData$qv,esp
if defined Menuet_SegmentInit & defined Menuet_SegmentInitEnd
  push Menuet_SegmentInitEnd
  push Menuet_SegmentInit
  jmp  .main_after_first_check
end if
.main_else_first_check:
  xor  eax,eax
  push eax eax
.main_after_first_check:
  push ebx
  call @@Kolibri@_CallStart$qppvpvt2
  add  esp,12
  test al,al
  jnz  .main_test_close_first
  jmp  .main_end
.main_close_first:
  btr  dword [esp+MENUET_THREAD_DATA_FLAG*4],31
  push esp
  call @@MenuetOnClose$qppv
  pop  ecx
  test al,al
  jnz  .main_end
.main_test_close_first:
  cmp  dword [esp+MENUET_THREAD_DATA_FLAG*4],0
  jl   .main_close_first
  push esp
  push dword 1
  call @Kolibri@Redraw$qippv
  add  esp,8
.main_paint_msg:
  or   dword [esp+MENUET_THREAD_DATA_FLAG*4],3
  sub  esp,1024
  mov  eax,9
  mov  ebx,esp
  mov  ecx,-1
  int  0x40
  mov  eax,[esp+34]
  mov  ebx,[esp+38]
  mov  ecx,[esp+42]
  mov  edx,[esp+46]
  add  esp,1024
  cmp  ecx,[esp+MENUET_THREAD_DATA_LAST_SX*4]
  jnz  .main_size
  cmp  edx,[esp+MENUET_THREAD_DATA_LAST_SY*4]
  jz   .main_paint
.main_size:
  mov  [esp+MENUET_THREAD_DATA_LAST_SX*4],ecx
  mov  [esp+MENUET_THREAD_DATA_LAST_SY*4],edx
  push edx
  push ecx
  push ebx
  push eax
  lea  ecx,[esp+16]
  mov  edx,esp
  push ecx
  push edx
  call @@MenuetOnSize$qpippv
  add  esp,24
  test dword [esp+MENUET_THREAD_DATA_FLAG*4],3
  jz   .main_cycle
.main_paint:
  cmp  dword [esp+MENUET_THREAD_DATA_FLAG*4],0
  jl   .main_close
  push esp
  push dword 0
  call @Kolibri@Redraw$qippv
  add  esp,8
.main_cycle:
  mov  eax,11
.main_message:
  cmp  dword [esp+MENUET_THREAD_DATA_FLAG*4],0
  jl   .main_close
  int  0x40
  test eax,eax
  jnz  .main_on_message
  cmp  dword [esp+MENUET_THREAD_DATA_FLAG*4],0
  jne  .main_paint
  push esp
  call @@MenuetOnIdle$qppv
  pop  ecx
  test eax,eax
  jz   .main_cycle
  jl   .main_wait_message
  mov  ebx,eax
  mov  eax,23
  jmp  .main_message
.main_wait_message:
  mov  eax,10
  jmp  .main_message
.main_key_press:
  push esp
  call @@MenuetOnKeyPress$qppv
  pop  ecx
  jmp  .main_cycle
.main_mouse:
  push esp
  call @@MenuetOnMouse$qppv
  pop  ecx
  jmp  .main_cycle
.main_on_message:
  dec  eax
  jz   .main_paint_msg
  dec  eax
  jz   .main_key_press
  cmp  eax,4
  jz   .main_mouse
  dec  eax
  jnz  .main_cycle
.main_button:
  mov  eax,17
  int  0x40
  test al,al
  jnz  .main_cycle
.main_close:
  btr  dword [esp+MENUET_THREAD_DATA_FLAG*4],31
  push esp
  call @@MenuetOnClose$qppv
  pop  ecx
  test al,al
  jz   .main_button
.main_end:
  mov  ebx,esp
  lock dec dword [@Kolibri@_ThreadNumber]
if defined Menuet_SegmentExit & defined Menuet_SegmentExitEnd
  jnz  .main_else_last_check
  push Menuet_SegmentExitEnd
  push Menuet_SegmentExit
  jmp  .main_after_last_check
end if
.main_else_last_check:
  xor  eax,eax
  push eax
  push eax
.main_after_last_check:
  push ebx
  call @@Kolibri@_RemoveThreadData$qppvpvt2
  add  esp,12
  lock inc dword [@Kolibri@_ThreadScanCount+4]
  mov  ebx,1
  jmp  .main_end_wait
.main_end_wait_loop:
  mov  eax,5
  int  0x40
  shl  ebx,1
  cmp  ebx,MENUET_MUTEX_MAX_TIME_WAIT
  jna  .main_end_wait
  mov  ebx,MENUET_MUTEX_MAX_TIME_WAIT
.main_end_wait:
  cmp  dword [@Kolibri@_ExitProcessNow],0
  jnz  @Kolibri@ExitProcess$qv
  cmp  dword [@Kolibri@_ThreadScanCount],0
  jnz  .main_end_wait_loop
  lock dec dword [@Kolibri@_ThreadScanCount+4]
  mov  ebp,[esp+4]
  mov  ebx,[esp]
  add  esp,MENUET_THREAD_DATA_LEN*4
  mov  eax,ebp
  pop  edi esi
  xchg ebp,[esp+8]
  xchg ebx,[esp+4]
  ret
endp

macro call func
{
  if func eq __chkstk
	sub  esp,eax
  else
	call func
  end if
}

proc @Kolibri@Redraw$qippv
  push ebp
  mov  ebp,[esp+12]
  mov  edx,[ebp+MENUET_THREAD_DATA_FLAG*4]
  cmp  dword [esp+8],0
  jl   .redraw_only_inv
  jz   .redraw_no_frame
  or   dl,2
.redraw_no_frame:
  bt   edx,30
  jnc  .redraw_begin
  or   dl,1
  mov  [ebp+MENUET_THREAD_DATA_FLAG*4],edx
  jmp  .redraw_end
.redraw_only_inv:
  test dl,3
  jnz  .redraw_no_frame
.redraw_end:
  pop  ebp
  ret
.redraw_begin:
  push ebx esi edi
  and  dword [ebp+MENUET_THREAD_DATA_FLAG*4],0xFFFFFFFC
  test dl,2
  jz   .redraw_picture
  mov  eax,12
  mov  ebx,1
  int  0x40
  xor  eax,eax
  mov  ebx,[ebp+MENUET_THREAD_DATA_X*4]
  mov  ecx,[ebp+MENUET_THREAD_DATA_Y*4]
  mov  edx,[ebp+MENUET_THREAD_DATA_C_WINDOW*4]
  mov  esi,[ebp+MENUET_THREAD_DATA_C_HEADER*4]
  mov  edi,[ebp+MENUET_THREAD_DATA_C_BORDER*4]
  int  0x40
  mov  edx,[ebp+MENUET_THREAD_DATA_TITLE*4]
  test edx,edx
  jz   .window_defined
  mov  edi,edx
  mov  ecx,0xFFFFFFFF
  xor  al,al
  cld
  repnz scas byte [edi]
  not  ecx
  mov  esi,ecx
  dec  esi
  jz   .window_defined
  mov  eax,4
  mov  ebx,0x00070007
  mov  ecx,[ebp+MENUET_THREAD_DATA_C_TITLE*4]
  int  0x40
.window_defined:
  mov  eax,12
  mov  ebx,2
  int  0x40
.redraw_picture:
  mov  eax,12
  mov  ebx,2
  int  0x40
  mov  esi,[ebp+MENUET_THREAD_DATA_PICTURE*4]
  test esi,esi
  jz   .redraw_end_draw
  mov  ecx,[ebp+MENUET_THREAD_DATA_SZ_PICT*4]
  jecxz .redraw_end_draw
  mov  al,byte [ebp+MENUET_THREAD_DATA_C_WINDOW*4+3]
  and  al,15
  mov  edx,MENUET_BORDER_SIZE*65536+MENUET_HEADER_SIZE
  cmp  al,3
  jnz  .redraw_no_skin
  mov  eax,48
  mov  ebx,4
  int  0x40
  mov  dx,ax
.redraw_no_skin:
  mov  eax,7
  mov  ebx,esi
  int  0x40
.redraw_end_draw:
  pop  edi esi ebx ebp
  ret
endp

proc @Kolibri@MoveWindow$qxpxi uses ebx esi
  mov  eax,[esp+12]
  mov  ebx,[eax]
  mov  ecx,[eax+4]
  mov  edx,[eax+8]
  mov  esi,[eax+12]
  mov  eax,67
  int  0x40
  ret
endp

;proc @Kolibri@Abort$qv
;  push dword [@Kolibri@DebugPrefix]
;  call @Kolibri@DebugPutString$qpxc
;  mov	dword [esp],Menuet_abort_string
;  call @Kolibri@DebugPutString$qpxc
;  pop	ecx
proc @Kolibri@ExitProcess$qv
  lock bts dword [@Kolibri@_ExitProcessNow],0
  jc   .exit_process_wait
  sub  esp,1024
  mov  eax,9
  mov  ebx,esp
  mov  ecx,-1
  int  0x40
  mov  esi,eax
  mov  edi,[esp+30]
.exit_process_loop:
  mov  eax,9
  mov  ebx,esp
  mov  ecx,esi
  int  0x40
  mov  eax,[esp+30]
  cmp  eax,edi
  jz   .exit_process_continue
  mov  ebx,eax
  or   bl,15
  inc  ebx
  jz   .exit_process_continue
  mov  ebx,eax
  call Menuet_HashInt
  movzx eax,al
  mov  eax,dword [@Kolibri@_ThreadTable+eax*4]
  jmp  .exit_process_test
.exit_process_next:
  mov  eax,dword [eax+MENUET_THREAD_DATA_NEXT*4]
.exit_process_test:
  test eax,eax
  jz   .exit_process_continue
  cmp  ebx,[eax+MENUET_THREAD_DATA_PID*4]
  jnz  .exit_process_next
  mov  eax,18
  mov  ebx,2
  mov  ecx,esi
  int  0x40
.exit_process_continue:
  dec  esi
  jnl  .exit_process_loop
  add  esp,1024
  mov  dword [@Kolibri@_ExitProcessNow],-1
if defined EMULATOR
  int3
  call 0x76543210
end if
.exit_process_end:
  mov  dword [@Kolibri@_ThreadMutex],0
  or   eax,-1
  int  0x40
.exit_process_wait:
  mov  eax,5
  mov  ebx,1
.exit_process_wait_loop:
  cmp  dword [@Kolibri@_ExitProcessNow],0
  jl   .exit_process_end
  int  0x40
  shl  ebx,1
  cmp  ebx,MENUET_MUTEX_MAX_TIME_WAIT
  jna  .exit_process_wait_loop
  mov  ebx,MENUET_MUTEX_MAX_TIME_WAIT
  jmp  .exit_process_wait_loop
endp

proc @Kolibri@ExitThread$qppv,@Kolibri@ThreadMain$qpvt1
  mov  esp,[esp+4]
  jmp  Menuet_main_end
endp

proc @Kolibri@ReturnMessageLoop$qppv,@Kolibri@ThreadMain$qpvt1
  mov  esp,[esp+4]
  bt   dword [esp+MENUET_THREAD_DATA_FLAG*4],30
  jc   Menuet_main_end
  jmp  Menuet_main_cycle
endp

proc @Kolibri@Delay$qui uses ebx
  mov  eax,5
  mov  ebx,[esp+8]
  int  0x40
  ret
endp

proc @Kolibri@Clock$qv uses ebx
  mov  eax,26
  mov  ebx,9
  int  0x40
  ret
endp

proc @Kolibri@GetPackedTime$qv
  mov  eax,3
  int  0x40
  ret
endp

proc @Kolibri@GetTime$qpi
  mov  eax,3
  int  0x40
  mov  edx,[esp+4]
  movzx ecx,al
  shr  ecx,4
  and  al,0x0F
  imul ecx,10
  add  cl,al
  mov  dword [edx+8],ecx
  mov  cl,ah
  shr  ecx,4
  and  ah,0x0F
  imul ecx,10
  add  cl,ah
  mov  dword [edx+4],ecx
  bswap eax
  mov  cl,ah
  shr  ecx,4
  and  ah,0x0F
  imul ecx,10
  add  cl,ah
  mov  dword [edx],ecx
  ret
endp

proc @Kolibri@GetPackedDate$qv
  mov  eax,29
  int  0x40
  ret
endp

proc @Kolibri@GetDate$qpi
  mov  eax,29
  int  0x40
  mov  edx,[esp+4]
  movzx ecx,al
  shr  ecx,4
  and  al,0x0F
  imul ecx,10
  add  cl,al
  mov  dword [edx+4],ecx
  mov  cl,ah
  shr  ecx,4
  and  ah,0x0F
  imul ecx,10
  add  cl,ah
  mov  dword [edx],ecx
  bswap eax
  mov  cl,ah
  shr  ecx,4
  and  ah,0x0F
  imul ecx,10
  add  cl,ah
  mov  dword [edx+8],ecx
  ret
endp

proc @Kolibri@ReadCommonColors$qpui uses ebx
  mov  eax,48
  mov  ebx,3
  mov  ecx,[esp+8]
  mov  edx,40
  int  0x40
  ret
endp

proc @Kolibri@GetProcessInfo$qpuipct1t1piui uses ebx esi edi
  sub  esp,1024
  mov  eax,9
  mov  ebx,esp
  mov  ecx,[1024+12+24+esp]
  int  0x40
  xor  edi,edi
  or   edi,[1024+12+4+esp]
  jz   .get_proc_info_no_usecpu
  mov  ecx,[esp]
  mov  [edi],ecx
  xor  edi,edi
.get_proc_info_no_usecpu:
  or   edi,[1024+12+8+esp]
  jz   .get_proc_info_no_name
  lea  esi,[esp+10]
  cld
  movs dword [edi],[esi]
  movs dword [edi],[esi]
  movs dword [edi],[esi]
  mov  byte [edi],0
  xor  edi,edi
.get_proc_info_no_name:
  or   edi,[1024+12+12+esp]
  jz   .get_proc_info_no_mem
  mov  ecx,[esp+26]
  mov  [edi],ecx
  xor  edi,edi
.get_proc_info_no_mem:
  or   edi,[1024+12+16+esp]
  jz   .get_proc_info_no_pid
  mov  ecx,[esp+30]
  mov  [edi],ecx
  xor  edi,edi
.get_proc_info_no_pid:
  or   edi,[1024+12+20+esp]
  jz   .get_proc_info_no_rect
  lea  esi,[esp+34]
  cld
  movs dword [edi],[esi]
  movs dword [edi],[esi]
  movs dword [edi],[esi]
  movs dword [edi],[esi]
  xor  edi,edi
.get_proc_info_no_rect:
  add  esp,1024
  ret
endp

proc @Kolibri@GetPid$qv uses ebx
  sub  esp,1024
  mov  eax,9
  mov  ebx,esp
  mov  ecx,-1
  int  0x40
  mov  eax,[esp+30]
  add  esp,1024
  ret
endp

proc @Kolibri@GetPid$qppv
  mov  ecx,[esp+4]
  mov  eax,[ecx+MENUET_THREAD_DATA_PID*4]
  ret
endp

proc @Kolibri@_HashByte$qui
@Kolibri@_HashWord$qui:
@Kolibri@_HashDword$qui:
  mov  eax,[esp+4]
Menuet_HashInt:
  mul  dword [Menuet_hash_int_val0]
  xor  eax,edx
  bswap eax
  mul  dword [Menuet_hash_int_val1]
  shrd eax,edx,14
  bswap eax
  lea  eax,[eax+4*eax]
  ror  eax,9
  ret
endp

Menuet_hash_int_val0:
  dd   0xA82F94C5
Menuet_hash_int_val1:
  dd   0x9193780B

proc @Kolibri@GetThreadData$qv
  call @Kolibri@GetPid$qv
  push eax
  call @Kolibri@GetThreadData$qui
  pop  ecx
  ret
endp

proc @Kolibri@GetThreadData$qui
  mov  eax,[esp+4]
  call Menuet_HashInt
  movzx eax,al
  cmp  dword [@Kolibri@_ThreadScanCount+4],0
  jnz  .get_thread_data_wait
.get_thread_data_nowait:
  lock inc dword [@Kolibri@_ThreadScanCount]
  mov  eax,dword [@Kolibri@_ThreadTable+eax*4]
  mov  ecx,[esp+4]
  jmp  .get_thread_data_test
.get_thread_data_loop:
  mov  eax,dword [eax+MENUET_THREAD_DATA_NEXT*4]
.get_thread_data_test:
  test eax,eax
  jz   .get_thread_data_end
  cmp  ecx,[eax+MENUET_THREAD_DATA_PID*4]
  jnz  .get_thread_data_loop
.get_thread_data_end:
  lock dec dword [@Kolibri@_ThreadScanCount]
  ret
.get_thread_data_wait:
  push eax ebx
  mov  eax,5
  mov  ebx,1
.get_thread_data_wait_loop:
  int  0x40
  cmp  dword [@Kolibri@_ThreadScanCount+4],0
  jz   .get_thread_data_wait_end
  shl  ebx,1
  cmp  ebx,MENUET_MUTEX_MAX_TIME_WAIT
  jna  .get_thread_data_wait_loop
  mov  ebx,MENUET_MUTEX_MAX_TIME_WAIT
  jmp  .get_thread_data_wait_loop
.get_thread_data_wait_end:
  pop  ebx eax
  jmp .get_thread_data_nowait
endp

proc @Kolibri@_GetSkinHeader$qv uses ebx
  mov  eax,48
  mov  ebx,4
  int  0x40
  ret
endp

proc @Kolibri@GetScreenSize$qrust1
  mov  eax,14
  int  0x40
  mov  ecx,[esp+8]
  mov  word [ecx],ax
  mov  ecx,[esp+4]
  shr  eax,16
  mov  word [ecx],ax
  ret
endp

proc Menuet_MutexLockNoWait
  pop  eax
  xor  al,al
  ret
endp

proc Menuet_MutexLockWait uses ebx
  mov  eax,5
  xor  ebx,ebx
.lock_wait_cycle:
  int  0x40
  shl  byte [ecx],1
  jz   .lock_wait_cycle
  mov  al,1
  ret
endp

proc Menuet_MutexLockWaitTime
  cmp  dword [esp+12],0
  jng  .MutexLockWait
  push ebx edx
  mov  edx,[esp+20]
  mov  eax,26
  mov  ebx,9
  int  0x40
  add  edx,eax
.lock_wait_time_cycle:
  mov  eax,5
  xor  ebx,ebx
  int  0x40
  shl  byte [ecx],1
  jnz  .lock_wait_time_ret_true
  mov  eax,26
  mov  ebx,9
  int  0x40
  cmp  eax,edx
  js   .lock_wait_time_cycle
  pop  edx ebx eax
  xor  al,al
  ret
.lock_wait_time_ret_true:
  pop  edx ebx
  mov  al,1
  ret
endp

proc Menuet_MutexLock
  shl  byte [ecx],1
  jnz  .lock_first
  call eax
.lock_first:
  mov  al,1
  ret
endp

proc @Kolibri@TryLock$qp14Kolibri@TMutex
  mov  eax,Menuet_MutexLockNoWait
  mov  ecx,[esp+4]
  jmp  Menuet_MutexLock
endp

proc @Kolibri@Lock$qp14Kolibri@TMutex
  mov  eax,Menuet_MutexLockWait
  mov  ecx,[esp+4]
  jmp  Menuet_MutexLock
endp

proc @Kolibri@LockTime$qp14Kolibri@TMutexi
  mov  eax,Menuet_MutexLockWaitTime
  mov  ecx,[esp+4]
  jmp  Menuet_MutexLock
endp

proc @Kolibri@UnLock$qp14Kolibri@TMutex
  mov  ecx,[esp+4]
  shr  byte [ecx],1
  jz   .unlock_pause
  ret
.unlock_pause:
  mov  byte [ecx],0x40
  push ebx
  mov  eax,5
  xor  ebx,ebx
  int  0x40
  pop  ebx
  ret
endp

proc Menuet_MutexLockRec
  shl  byte [ecx],1
  jng  .lock_first
  cmp  dword [ecx+4],edx
  jz   .lock_rec_self
  call eax
.lock_rec_first:
  mov  al,1
  mov  dword [ecx+4],edx
  ret
.lock_rec_self:
  mov  al,1
  add  dword [ecx],0x100
  jc   .lock_rec_overflow
  ret
.lock_rec_overflow:
  push dword [@Kolibri@DebugPrefix]
  call @Kolibri@DebugPutString$qpxc
  mov  dword [esp],Menuet_try_lock_rec_overflow_string
  call @Kolibri@DebugPutString$qpxc
  pop  ecx
  jmp  @Kolibri@Abort$qv
endp

proc @Kolibri@TryLock$qp16Kolibri@TRecMutexui
  mov  eax,Menuet_MutexLockNoWait
  mov  ecx,[esp+4]
  mov  edx,[esp+8]
  jmp  Menuet_MutexLockRec
endp

proc @Kolibri@Lock$qp16Kolibri@TRecMutexui
  mov  eax,Menuet_MutexLockWait
  mov  ecx,[esp+4]
  mov  edx,[esp+8]
  jmp  Menuet_MutexLockRec
endp

proc @Kolibri@LockTime$qp16Kolibri@TRecMutexiui
  mov  eax,Menuet_MutexLockWaitTime
  mov  ecx,[esp+4]
  mov  edx,[esp+12]
  jmp  Menuet_MutexLockRec
endp

proc @Kolibri@UnLock$qp16Kolibri@TRecMutexui
  mov  ecx,[esp+4]
  mov  edx,[esp+8]
  cmp  dword [ecx+4],edx
  jnz  .unlock_rec_notlocked
  sub  dword [ecx],0x100
  jnc  .unlock_rec_end
  add dword [ecx],0x100
  shl byte [ecx],1
  shr byte [ecx],2
  jng  .unlock_rec_pause
.unlock_rec_end:
  ret
.unlock_rec_pause:
  mov  byte [ecx],0x20
  push ebx
  mov  eax,5
  xor  ebx,ebx
  int  0x40
  pop  ebx
  ret
.unlock_rec_notlocked:
  push dword [@Kolibri@DebugPrefix]
  call @Kolibri@DebugPutString$qpxc
  mov  dword [esp],Menuet_unlock_rec_notlocked_string
  call @Kolibri@DebugPutString$qpxc
  pop  ecx
  jmp  @Kolibri@Abort$qv
endp

proc @Kolibri@DebugPutChar$qc
  mov  cl,byte [esp+4]
  cmp  cl,13
  jz   .debug_put_char_ret
  push ebx
  cmp  cl,10
  jz   .debug_put_char_enter
.debug_put_char_after_cmp:
  mov  eax,63
  mov  ebx,1
  int  0x40
  pop  ebx
.debug_put_char_ret:
  ret
.debug_put_char_enter:
  mov  cl,13
  mov  eax,63
  mov  ebx,1
  int  0x40
  mov  cl,10
  jmp  .debug_put_char_after_cmp
endp

proc @Kolibri@DebugPutString$qpxc
  push esi
  push dword 0
  mov  esi,dword [esp+12]
  jmp  .debug_put_string_test
.debug_put_string_loop:
  mov  dword [esp],eax
  call @Kolibri@DebugPutChar$qc
  inc  esi
.debug_put_string_test:
  xor  eax,eax
  or   al,[esi]
  test al,al
  jnz  .debug_put_string_loop
  pop  ecx esi
  ret
endp

proc @Kolibri@GetKey$qv
  mov  eax,2
  int  0x40
  test al,al
  jnz  .get_key_eof
  movzx eax,ah
  ret
.get_key_eof:
  mov  eax,-1
  ret
endp

proc @Kolibri@GetMouseButton$qv uses ebx
  mov  eax,37
  mov  ebx,2
  int  0x40
  ret
endp

proc @Kolibri@GetMousePosition$qrst1o uses ebx
  mov  eax,37
  xor  ebx,ebx
  cmp  byte [esp+16],0
  jnz  .get_mouse_pos_absolute
  inc  ebx
.get_mouse_pos_absolute:
  int  0x40
  mov  ecx,[esp+12]
  mov  word [ecx],ax
  mov  ecx,[esp+8]
  shr  eax,16
  mov  word [ecx],ax
  ret
endp

proc @Kolibri@WasThreadCreated$qv
  cmp  byte [@Kolibri@_ThreadSavedBegProc],0x90
  setz al
  ret
endp

proc @Kolibri@CreateThread$qpvuit1
  push ebx
  mov  edx,[esp+16]
  mov  ebx,[esp+12]
  test edx,edx
  jnz  .create_thread_after_new
if defined MenuetHeapAlloc
  cmp  ebx,4096
  jnb  .create_thread_alloc
  mov  ebx,STACKSIZE
.create_thread_alloc:
  push ebx
  call MenuetHeapAlloc	 ; Create new dynamic memory of the given size
  pop  ecx
  test eax,eax
  jnz  .create_thread_mem_created
end if
  or   eax,-1
  jmp  .create_thread_end
.create_thread_mem_created:
  lea  edx,[eax+ebx]
.create_thread_after_new:
  neg  ebx
  jz   .create_thread_test_first
  add  ebx,edx
.create_thread_test_first:
  cmp  byte [@Kolibri@_ThreadSavedBegProc],0x90
  jnz  .create_thread_init
.create_thread_fill_stack:
  lock inc dword [@Kolibri@_ThreadNumber]
  and  edx,not 3
  sub  edx,12
  mov  ecx,[esp+8]
  mov  dword [edx+8],ebx
  mov  dword [edx+4],ecx
  mov  dword [edx],Menuet_ThreadFinish
  mov  eax,51
  mov  ebx,1
  mov  ecx,@Kolibri@ThreadMain$qpvt1
  int  0x40
  mov  ebx,eax
  or   bl,15
  inc  ebx
  jnz  .create_thread_end
  lock dec dword [@Kolibri@_ThreadNumber]
if defined MenuetHeapFree
  or   ebx,[edx+8]
  jz   .create_thread_end
  push ebx
  call MenuetHeapFree	; Delete the given dynamic memory
  pop  ecx
end if
.create_thread_end:
  pop  ebx
  ret
.create_thread_init:
  push esi edi
  cld
  mov  esi,@Kolibri@_ThreadSavedBegProc
  mov  edi,@Kolibri@GetPid$qv
  movs dword [edi],[esi]
  movs dword [edi],[esi]
  mov  edi,@Kolibri@GetThreadData$qv
  movs dword [edi],[esi]
  movs dword [edi],[esi]
  mov  eax,0x90909090
  mov  edi,@Kolibri@_ThreadSavedBegProc
  stos dword [edi]
  stos dword [edi]
  stos dword [edi]
  stos dword [edi]
  pop  edi esi
  jmp  .create_thread_fill_stack
endp

proc @Kolibri@_FileAccess$qpv uses ebx
  mov  eax,70
  mov  ebx,[esp+8]
  int  0x40
  mov  ecx,[esp+8]
  mov  [ecx],ebx
  ret
endp

Menuet_abort_string:
  db 'Abnormal program termination.',10,0
Menuet_try_lock_rec_overflow_string:
  db 'Recursive mutex lock count overflow.',10,0
Menuet_unlock_rec_notlocked_string:
  db 'Recursive mutex unlock error.',10,0

include "kos_lib.inc"

;/**/


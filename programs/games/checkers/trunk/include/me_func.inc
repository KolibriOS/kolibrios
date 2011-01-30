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
MENUET_THREAD_DATA_PID      = 3;
;const int
MENUET_THREAD_DATA_FLAG     = 4;
;const int
MENUET_THREAD_DATA_X        = 5;
;const int
MENUET_THREAD_DATA_Y        = 6;
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
MENUET_THREAD_DATA_LEN      = 16;

;const int
MENUET_MUTEX_MAX_TIME_WAIT  = 20;

;const int
MENUET_FILE_BLOCK_SIZE      = 512;

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

macro Menuet_Put_MovEaxVal_Ret  address,val
{
  mov  byte [address],0xB8
  mov  dword [address+4],0xC089C300
  mov  dword [address+1],val
}

define @Menuet@Main$qv
nextdef MenuetEntryPoint
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
  xor  eax,eax
  cld
  mov  edi,@Menuet@_ThreadTable
  mov  ecx,256
  rep stos dword [edi]
  mov  esi,@Menuet@GetPid$qv
  mov  edi,@Menuet@_ThreadSavedBegProc
  movs dword [edi],[esi]
  movs dword [edi],[esi]
  mov  esi,@Menuet@GetThreadData$qv
  movs dword [edi],[esi]
  movs dword [edi],[esi]
  Menuet_Put_MovEaxVal_Ret  @Menuet@GetPid$qv,edx
if defined MenuetHeapInit
  mov  ecx,esp
  push ebx
  push ecx
  push U_END
  call MenuetHeapInit   ; Initialize a dynamic heap and create new memory in its begin.
  pop  ecx              ; Parameters: begin of a new heap, end of data to create in
  mov  [esp+4],eax      ; the begin of a heap. Return address of the created data.
  mov  dword [esp],0
else
  xor  eax,eax
  push eax
  push eax
end if
  call @Menuet@ThreadMain$qpvt1
nextdef Menuet_ThreadFinish
  add  esp,8
if defined MenuetHeapFreeAndThreadFinish
  test eax,eax
  jz   Menuet_ThreadFinish_end
  push dword @Menuet@_ExitProcessNow
  push eax
  call MenuetHeapFreeAndThreadFinish   ; Free the given memory and finish the thread,
end if                                 ; should exit process if second argument points to not zero.
Menuet_ThreadFinish_end:
  or   eax,-1
  int  0x40
enddef

define @Menuet@ThreadMain$qpvt1
  xchg ebx,[esp+4]
  xchg ebp,[esp+8]
  push esi
  push edi
  sub  esp,MENUET_THREAD_DATA_LEN*4
  mov  [esp],ebx
  mov  [esp+4],ebp
  mov  eax,40
  mov  ebx,0x27
  int  0x40
  mov  ebx,esp
  cmp  byte [@Menuet@_ThreadSavedBegProc],0x90
  jz   Menuet_main_else_first_check
  Menuet_Put_MovEaxVal_Ret  @Menuet@GetThreadData$qv,esp
if defined Menuet_SegmentInit & defined Menuet_SegmentInitEnd
  push Menuet_SegmentInitEnd
  push Menuet_SegmentInit
  jmp  Menuet_main_after_first_check
end if
Menuet_main_else_first_check:
  xor  eax,eax
  push eax
  push eax
Menuet_main_after_first_check:
  push ebx
  call @@Menuet@_CallStart$qppvpvt2
  add  esp,12
  test al,al
  jnz  Menuet_main_test_close_first
  jmp  Menuet_main_end
Menuet_main_close_first:
  btr  dword [esp+MENUET_THREAD_DATA_FLAG*4],31
  push esp
  call @@MenuetOnClose$qppv
  pop  ecx
  test al,al
  jnz  Menuet_main_end
Menuet_main_test_close_first:
  cmp  dword [esp+MENUET_THREAD_DATA_FLAG*4],0
  jl   Menuet_main_close_first
;  push esp
;  push dword 1
;  call @Menuet@Redraw$qippv
;  add  esp,8
Menuet_main_paint_msg:
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
  jnz  Menuet_main_size
  cmp  edx,[esp+MENUET_THREAD_DATA_LAST_SY*4]
  jz   Menuet_main_paint
Menuet_main_size:
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
  jz   Menuet_main_cycle
Menuet_main_paint:
  cmp  dword [esp+MENUET_THREAD_DATA_FLAG*4],0
  jl   Menuet_main_close
  push esp
  push dword 0
  call @Menuet@Redraw$qippv
  add  esp,8
Menuet_main_cycle:
  mov  eax,11
Menuet_main_message:
  cmp  dword [esp+MENUET_THREAD_DATA_FLAG*4],0
  jl   Menuet_main_close
  int  0x40
  test eax,eax
  jnz  Menuet_main_on_message
  cmp  dword [esp+MENUET_THREAD_DATA_FLAG*4],0
  jne  Menuet_main_paint
  push esp
  call @@MenuetOnIdle$qppv
  pop  ecx
  test eax,eax
  jz   Menuet_main_cycle
  jl   Menuet_main_wait_message
  mov  ebx,eax
  mov  eax,23
  jmp  Menuet_main_message
Menuet_main_wait_message:
  mov  eax,10
  jmp  Menuet_main_message
Menuet_main_key_press:
  push esp
  call @@MenuetOnKeyPress$qppv
  pop  ecx
  jmp  Menuet_main_cycle
Menuet_main_mouse:
  push esp
  call @@MenuetOnMouse$qppv
  pop  ecx
  jmp  Menuet_main_cycle
Menuet_main_on_message:
  dec  eax
  jz   Menuet_main_paint_msg
  dec  eax
  jz   Menuet_main_key_press
  cmp  eax,4
  jz   Menuet_main_mouse
  dec  eax
  jnz  Menuet_main_cycle
Menuet_main_button:
  mov  eax,17
  int  0x40
  test al,al
  jnz  Menuet_main_cycle
Menuet_main_close:
  btr  dword [esp+MENUET_THREAD_DATA_FLAG*4],31
  push esp
  call @@MenuetOnClose$qppv
  pop  ecx
  test al,al
  jz   Menuet_main_button
Menuet_main_end:
  mov  ebx,esp
  lock dec dword [@Menuet@_ThreadNumber]
if defined Menuet_SegmentExit & defined Menuet_SegmentExitEnd
  jnz  Menuet_main_else_last_check
  push Menuet_SegmentExitEnd
  push Menuet_SegmentExit
  jmp  Menuet_main_after_last_check
end if
Menuet_main_else_last_check:
  xor  eax,eax
  push eax
  push eax
Menuet_main_after_last_check:
  push ebx
  call @@Menuet@_RemoveThreadData$qppvpvt2
  add  esp,12
  lock inc dword [@Menuet@_ThreadScanCount+4]
  mov  ebx,1
  jmp  Menuet_main_end_wait
Menuet_main_end_wait_loop:
  mov  eax,5
  int  0x40
  shl  ebx,1
  cmp  ebx,MENUET_MUTEX_MAX_TIME_WAIT
  jna  Menuet_main_end_wait
  mov  ebx,MENUET_MUTEX_MAX_TIME_WAIT
Menuet_main_end_wait:
  cmp  dword [@Menuet@_ExitProcessNow],0
  jnz  @Menuet@ExitProcess$qv
  cmp  dword [@Menuet@_ThreadScanCount],0
  jnz  Menuet_main_end_wait_loop
  lock dec dword [@Menuet@_ThreadScanCount+4]
  mov  ebp,[esp+4]
  mov  ebx,[esp]
  add  esp,MENUET_THREAD_DATA_LEN*4
  mov  eax,ebp
  pop  edi
  pop  esi
  xchg ebp,[esp+8]
  xchg ebx,[esp+4]
  ret
enddef

macro call func
{
  if func eq __chkstk
	sub  esp,eax
  else
	call func
  end if
}

define @Menuet@Redraw$qippv
  push ebp
  mov  ebp,[esp+12]
  mov  edx,[ebp+MENUET_THREAD_DATA_FLAG*4]
  cmp  dword [esp+8],0
  jl   Menuet_redraw_only_inv
  jz   Menuet_redraw_no_frame
  or   dl,2
Menuet_redraw_no_frame:
  bt   edx,30
  jnc  Menuet_redraw_begin
  or   dl,1
  mov  [ebp+MENUET_THREAD_DATA_FLAG*4],edx
  jmp  Menuet_redraw_end
Menuet_redraw_only_inv:
  test dl,3
  jnz  Menuet_redraw_no_frame
Menuet_redraw_end:
  pop  ebp
  ret
Menuet_redraw_begin:
  push ebx
  push esi
  push edi
  and  dword [ebp+MENUET_THREAD_DATA_FLAG*4],0xFFFFFFFC
  mov  eax,12
  mov  ebx,1
  int  0x40
  test dl,2
  jz   Menuet_redraw_picture
  push 48
  pop  eax
  push 4
  pop  ebx
  int  0x40
  mov  ebx,[ebp+MENUET_THREAD_DATA_X*4]
  add  ebx,2*5-1
  mov  ecx,[ebp+MENUET_THREAD_DATA_Y*4]
  add  cx,ax
  add  ecx,5-1
  mov  edx,[ebp+MENUET_THREAD_DATA_C_WINDOW*4]
;  mov  esi,[ebp+MENUET_THREAD_DATA_C_HEADER*4]
;  mov  edi,[ebp+MENUET_THREAD_DATA_C_BORDER*4]
  mov  edi,[ebp+MENUET_THREAD_DATA_TITLE*4]
  xor  eax,eax
  int  0x40
;  mov  edx,[ebp+MENUET_THREAD_DATA_TITLE*4]
;  test edx,edx
;  jz   Menuet_redraw_picture
;  mov  edi,edx
;  mov  ecx,0xFFFFFFFF
;  xor  al,al
;  cld
;  repnz scas byte [edi]
;  not  ecx
;  mov  esi,ecx
;  dec  esi
;  jz   Menuet_redraw_picture
;  mov  eax,4
;  mov  ebx,0x00070007
;  mov  ecx,[ebp+MENUET_THREAD_DATA_C_TITLE*4]
;  int  0x40
Menuet_redraw_picture:
;  mov  esi,[ebp+MENUET_THREAD_DATA_PICTURE*4]
;  test esi,esi
;  jz   Menuet_redraw_end_draw
;  mov  ecx,[ebp+MENUET_THREAD_DATA_SZ_PICT*4]
;  jecxz Menuet_redraw_end_draw
;  mov  al,byte [ebp+MENUET_THREAD_DATA_C_WINDOW*4+3]
;  and  al,15
;  mov  edx,MENUET_BORDER_SIZE*65536+MENUET_HEADER_SIZE
;  cmp  al,3
;  jnz  Menuet_redraw_no_skin
;  mov  eax,48
;  mov  ebx,4
;  int  0x40
;  mov  dx,ax
;Menuet_redraw_no_skin:
;  mov  eax,7
;  mov  ebx,esi
;  int  0x40
	call	@@MenuetOnDraw$qv
Menuet_redraw_end_draw:
  mov  eax,12
  mov  ebx,2
  int  0x40
  pop  edi
  pop  esi
  pop  ebx
  pop  ebp
  ret
enddef

define @Menuet@MoveWindow$qxpxi
  push ebx
  push esi
  mov  eax,[esp+12]
  mov  ebx,[eax]
  mov  ecx,[eax+4]
  mov  edx,[eax+8]
  mov  esi,[eax+12]
  mov  eax,67
  int  0x40
  pop  esi
  pop  ebx
  ret
enddef

define @Menuet@Abort$qv
  push dword [@Menuet@DebugPrefix]
  call @Menuet@DebugPutString$qpxc
  mov  dword [esp],Menuet_abort_string
  call @Menuet@DebugPutString$qpxc
  pop  ecx
nextdef @Menuet@ExitProcess$qv
  lock bts dword [@Menuet@_ExitProcessNow],0
  jc   Menuet_exit_process_wait
  sub  esp,1024
  mov  eax,9
  mov  ebx,esp
  mov  ecx,-1
  int  0x40
  mov  esi,eax
  mov  edi,[esp+30]
Menuet_exit_process_loop:
  mov  eax,9
  mov  ebx,esp
  mov  ecx,esi
  int  0x40
  mov  eax,[esp+30]
  cmp  eax,edi
  jz   Menuet_exit_process_continue
  mov  ebx,eax
  or   bl,15
  inc  ebx
  jz   Menuet_exit_process_continue
  mov  ebx,eax
  call Menuet_HashInt
  movzx eax,al
  mov  eax,dword [@Menuet@_ThreadTable+eax*4]
  jmp  Menuet_exit_process_test
Menuet_exit_process_next:
  mov  eax,dword [eax+MENUET_THREAD_DATA_NEXT*4]
Menuet_exit_process_test:
  test eax,eax
  jz   Menuet_exit_process_continue
  cmp  ebx,[eax+MENUET_THREAD_DATA_PID*4]
  jnz  Menuet_exit_process_next
  mov  eax,18
  mov  ebx,2
  mov  ecx,esi
  int  0x40
Menuet_exit_process_continue:
  dec  esi
  jnl  Menuet_exit_process_loop
  add  esp,1024
  mov  dword [@Menuet@_ExitProcessNow],-1
if defined EMULATOR
  int3
  call 0x76543210
end if
Menuet_exit_process_end:
  mov  dword [@Menuet@_ThreadMutex],0
  or   eax,-1
  int  0x40
Menuet_exit_process_wait:
  mov  eax,5
  mov  ebx,1
Menuet_exit_process_wait_loop:
  cmp  dword [@Menuet@_ExitProcessNow],0
  jl   Menuet_exit_process_end
  int  0x40
  shl  ebx,1
  cmp  ebx,MENUET_MUTEX_MAX_TIME_WAIT
  jna  Menuet_exit_process_wait_loop
  mov  ebx,MENUET_MUTEX_MAX_TIME_WAIT
  jmp  Menuet_exit_process_wait_loop
enddef

define @Menuet@ExitThread$qppv,@Menuet@ThreadMain$qpvt1
  mov  esp,[esp+4]
  jmp  Menuet_main_end
enddef

define @Menuet@ReturnMessageLoop$qppv,@Menuet@ThreadMain$qpvt1
  mov  esp,[esp+4]
  bt   dword [esp+MENUET_THREAD_DATA_FLAG*4],30
  jc   Menuet_main_end
  jmp  Menuet_main_cycle
enddef

define @Menuet@Delay$qui
  push ebx
  mov  eax,5
  mov  ebx,[esp+8]
  int  0x40
  pop  ebx
  ret
enddef

define @Menuet@Clock$qv
  push ebx
  mov  eax,26
  mov  ebx,9
  int  0x40
  pop  ebx
  ret
enddef

define @Menuet@GetPackedTime$qv
  mov  eax,3
  int  0x40
  ret
enddef

define @Menuet@GetTime$qpi
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
enddef

define @Menuet@GetPackedDate$qv
  mov  eax,29
  int  0x40
  ret
enddef

define @Menuet@GetDate$qpi
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
enddef

define @Menuet@ReadCommonColors$qpui
  push ebx
  mov  eax,48
  mov  ebx,3
  mov  ecx,[esp+8]
  mov  edx,40
  int  0x40
  pop  ebx
  ret
enddef

define @Menuet@GetProcessInfo$qpuipct1t1piui
  push ebx
  push esi
  push edi
  sub  esp,1024
  mov  eax,9
  mov  ebx,esp
  mov  ecx,[1024+12+24+esp]
  int  0x40
  xor  edi,edi
  or   edi,[1024+12+4+esp]
  jz   Menuet_get_proc_info_no_usecpu
  mov  ecx,[esp]
  mov  [edi],ecx
  xor  edi,edi
Menuet_get_proc_info_no_usecpu:
  or   edi,[1024+12+8+esp]
  jz   Menuet_get_proc_info_no_name
  lea  esi,[esp+10]
  cld
  movs dword [edi],[esi]
  movs dword [edi],[esi]
  movs dword [edi],[esi]
  mov  byte [edi],0
  xor  edi,edi
Menuet_get_proc_info_no_name:
  or   edi,[1024+12+12+esp]
  jz   Menuet_get_proc_info_no_mem
  mov  ecx,[esp+26]
  mov  [edi],ecx
  xor  edi,edi
Menuet_get_proc_info_no_mem:
  or   edi,[1024+12+16+esp]
  jz   Menuet_get_proc_info_no_pid
  mov  ecx,[esp+30]
  mov  [edi],ecx
  xor  edi,edi
Menuet_get_proc_info_no_pid:
  or   edi,[1024+12+20+esp]
  jz   Menuet_get_proc_info_no_rect
  lea  esi,[esp+34]
  cld
  movs dword [edi],[esi]
  movs dword [edi],[esi]
  movs dword [edi],[esi]
  movs dword [edi],[esi]
  xor  edi,edi
Menuet_get_proc_info_no_rect:
  add  esp,1024
  pop  edi
  pop  esi
  pop  ebx
  ret
enddef

define @Menuet@GetPid$qv
  push ebx
  sub  esp,1024
  mov  eax,9
  mov  ebx,esp
  mov  ecx,-1
  int  0x40
  mov  eax,[esp+30]
  add  esp,1024
  pop  ebx
  ret
enddef

define @Menuet@GetPid$qppv
  mov  ecx,[esp+4]
  mov  eax,[ecx+MENUET_THREAD_DATA_PID*4]
  ret
enddef

define @Menuet@_HashByte$qui
nextdef @Menuet@_HashWord$qui
nextdef @Menuet@_HashDword$qui
  mov  eax,[esp+4]
nextdef Menuet_HashInt
  mul  dword [Menuet_hash_int_val0]
  xor  eax,edx
  bswap eax
  mul  dword [Menuet_hash_int_val1]
  shrd eax,edx,14
  bswap eax
  lea  eax,[eax+4*eax]
  ror  eax,9
  ret
Menuet_hash_int_val0:
  dd   0xA82F94C5
Menuet_hash_int_val1:
  dd   0x9193780B
enddef

define @Menuet@GetThreadData$qv
  call @Menuet@GetPid$qv
  push eax
  call @Menuet@GetThreadData$qui
  pop  ecx
  ret
enddef

define @Menuet@GetThreadData$qui
  mov  eax,[esp+4]
  call Menuet_HashInt
  movzx eax,al
  cmp  dword [@Menuet@_ThreadScanCount+4],0
  jnz  Menuet_get_thread_data_wait
Menuet_get_thread_data_nowait:
  lock inc dword [@Menuet@_ThreadScanCount]
  mov  eax,dword [@Menuet@_ThreadTable+eax*4]
  mov  ecx,[esp+4]
  jmp  Menuet_get_thread_data_test
Menuet_get_thread_data_loop:
  mov  eax,dword [eax+MENUET_THREAD_DATA_NEXT*4]
Menuet_get_thread_data_test:
  test eax,eax
  jz   Menuet_get_thread_data_end
  cmp  ecx,[eax+MENUET_THREAD_DATA_PID*4]
  jnz  Menuet_get_thread_data_loop
Menuet_get_thread_data_end:
  lock dec dword [@Menuet@_ThreadScanCount]
  ret
Menuet_get_thread_data_wait:
  push eax
  push ebx
  mov  eax,5
  mov  ebx,1
Menuet_get_thread_data_wait_loop:
  int  0x40
  cmp  dword [@Menuet@_ThreadScanCount+4],0
  jz   Menuet_get_thread_data_wait_end
  shl  ebx,1
  cmp  ebx,MENUET_MUTEX_MAX_TIME_WAIT
  jna  Menuet_get_thread_data_wait_loop
  mov  ebx,MENUET_MUTEX_MAX_TIME_WAIT
  jmp  Menuet_get_thread_data_wait_loop
Menuet_get_thread_data_wait_end:
  pop  ebx
  pop  eax
  jmp Menuet_get_thread_data_nowait
enddef

define @Menuet@_GetSkinHeader$qv
  push ebx
  mov  eax,48
  mov  ebx,4
  int  0x40
  pop  ebx
  ret
enddef

define @Menuet@GetScreenSize$qrust1
  mov  eax,14
  int  0x40
  mov  ecx,[esp+8]
  mov  word [ecx],ax
  mov  ecx,[esp+4]
  shr  eax,16
  mov  word [ecx],ax
  ret
enddef

define Menuet_MutexLockNoWait
  pop  eax
  xor  al,al
  ret
enddef

define Menuet_MutexLockWait
  push ebx
  mov  eax,5
  xor  ebx,ebx
Menuet_lock_wait_cycle:
  int  0x40
  shl  byte [ecx],1
  jz   Menuet_lock_wait_cycle
  pop  ebx
  mov  al,1
  ret
enddef

define Menuet_MutexLockWaitTime
  cmp  dword [esp+12],0
  jng  Menuet_MutexLockWait
  push ebx
  push edx
  mov  edx,[esp+20]
  mov  eax,26
  mov  ebx,9
  int  0x40
  add  edx,eax
Menuet_lock_wait_time_cycle:
  mov  eax,5
  xor  ebx,ebx
  int  0x40
  shl  byte [ecx],1
  jnz  Menuet_lock_wait_time_ret_true
  mov  eax,26
  mov  ebx,9
  int  0x40
  cmp  eax,edx
  js   Menuet_lock_wait_time_cycle
  pop  edx
  pop  ebx
  pop  eax
  xor  al,al
  ret
Menuet_lock_wait_time_ret_true:
  pop  edx
  pop  ebx
  mov  al,1
  ret
enddef

define Menuet_MutexLock
  shl  byte [ecx],1
  jnz  Menuet_lock_first
  call eax
Menuet_lock_first:
  mov  al,1
  ret
enddef

define @Menuet@TryLock$qp13Menuet@TMutex
  mov  eax,Menuet_MutexLockNoWait
  mov  ecx,[esp+4]
  jmp  Menuet_MutexLock
enddef

define @Menuet@Lock$qp13Menuet@TMutex
  mov  eax,Menuet_MutexLockWait
  mov  ecx,[esp+4]
  jmp  Menuet_MutexLock
enddef

define @Menuet@LockTime$qp13Menuet@TMutexi
  mov  eax,Menuet_MutexLockWaitTime
  mov  ecx,[esp+4]
  jmp  Menuet_MutexLock
enddef

define @Menuet@UnLock$qp13Menuet@TMutex
  mov  ecx,[esp+4]
  shr  byte [ecx],1
  jz   Menuet_unlock_pause
  ret
Menuet_unlock_pause:
  mov  byte [ecx],0x40
  push ebx
  mov  eax,5
  xor  ebx,ebx
  int  0x40
  pop  ebx
  ret
enddef

define Menuet_MutexLockRec
  shl  byte [ecx],1
  jng  Menuet_lock_first
  cmp  dword [ecx+4],edx
  jz   Menuet_lock_rec_self
  call eax
Menuet_lock_rec_first:
  mov  al,1
  mov  dword [ecx+4],edx
  ret
Menuet_lock_rec_self:
  mov  al,1
  add  dword [ecx],0x100
  jc   Menuet_lock_rec_overflow
  ret
Menuet_lock_rec_overflow:
  push dword [@Menuet@DebugPrefix]
  call @Menuet@DebugPutString$qpxc
  mov  dword [esp],Menuet_try_lock_rec_overflow_string
  call @Menuet@DebugPutString$qpxc
  pop  ecx
  jmp  @Menuet@Abort$qv
enddef

define @Menuet@TryLock$qp16Menuet@TRecMutexui
  mov  eax,Menuet_MutexLockNoWait
  mov  ecx,[esp+4]
  mov  edx,[esp+8]
  jmp  Menuet_MutexLockRec
enddef

define @Menuet@Lock$qp16Menuet@TRecMutexui
  mov  eax,Menuet_MutexLockWait
  mov  ecx,[esp+4]
  mov  edx,[esp+8]
  jmp  Menuet_MutexLockRec
enddef

define @Menuet@LockTime$qp16Menuet@TRecMutexiui
  mov  eax,Menuet_MutexLockWaitTime
  mov  ecx,[esp+4]
  mov  edx,[esp+12]
  jmp  Menuet_MutexLockRec
enddef

define @Menuet@UnLock$qp16Menuet@TRecMutexui
  mov  ecx,[esp+4]
  mov  edx,[esp+8]
  cmp  dword [ecx+4],edx
  jnz  Menuet_unlock_rec_notlocked
  sub  dword [ecx],0x100
  jnc  Menuet_unlock_rec_end
  add dword [ecx],0x100
  shl byte [ecx],1
  shr byte [ecx],2
  jng  Menuet_unlock_rec_pause
Menuet_unlock_rec_end:
  ret
Menuet_unlock_rec_pause:
  mov  byte [ecx],0x20
  push ebx
  mov  eax,5
  xor  ebx,ebx
  int  0x40
  pop  ebx
  ret
Menuet_unlock_rec_notlocked:
  push dword [@Menuet@DebugPrefix]
  call @Menuet@DebugPutString$qpxc
  mov  dword [esp],Menuet_unlock_rec_notlocked_string
  call @Menuet@DebugPutString$qpxc
  pop  ecx
  jmp  @Menuet@Abort$qv
enddef

define @Menuet@DebugPutChar$qc
  mov  cl,byte [esp+4]
  cmp  cl,13
  jz   Menuet_debug_put_char_ret
  push ebx
  cmp  cl,10
  jz   Menuet_debug_put_char_enter
Menuet_debug_put_char_after_cmp:
  mov  eax,63
  mov  ebx,1
  int  0x40
  pop  ebx
Menuet_debug_put_char_ret:
  ret
Menuet_debug_put_char_enter:
  mov  cl,13
  mov  eax,63
  mov  ebx,1
  int  0x40
  mov  cl,10
  jmp  Menuet_debug_put_char_after_cmp
enddef

define @Menuet@DebugPutString$qpxc
  push esi
  push dword 0
  mov  esi,dword [esp+12]
  jmp  Menuet_debug_put_string_test
Menuet_debug_put_string_loop:
  mov  dword [esp],eax
  call @Menuet@DebugPutChar$qc
  inc  esi
Menuet_debug_put_string_test:
  xor  eax,eax
  or   al,[esi]
  test al,al
  jnz  Menuet_debug_put_string_loop
  pop  ecx
  pop  esi
  ret
enddef

define @Menuet@GetKey$qv
  mov  eax,2
  int  0x40
  test al,al
  jnz  Menuet_get_key_eof
  movzx eax,ah
  ret
Menuet_get_key_eof:
  mov  eax,-1
  ret
enddef

define @Menuet@GetMouseButton$qv
  push ebx
  mov  eax,37
  mov  ebx,2
  int  0x40
  pop  ebx
  ret
enddef

define @Menuet@GetMousePosition$qrst1o
  push ebx
  mov  eax,37
  xor  ebx,ebx
  cmp  byte [esp+16],0
  jnz  Menuet_get_mouse_pos_absolute
  inc  ebx
Menuet_get_mouse_pos_absolute:
  int  0x40
  mov  ecx,[esp+12]
  mov  word [ecx],ax
  mov  ecx,[esp+8]
  shr  eax,16
  mov  word [ecx],ax
  pop  ebx
  ret
enddef

define @Menuet@WasThreadCreated$qv
  cmp  byte [@Menuet@_ThreadSavedBegProc],0x90
  setz al
  ret
enddef

define @Menuet@CreateThread$qpvuit1
  push ebx
  mov  edx,[esp+16]
  mov  ebx,[esp+12]
  test edx,edx
  jnz  Menuet_create_thread_after_new
if defined MenuetHeapAlloc
  cmp  ebx,4096
  jnb  Menuet_create_thread_alloc
  mov  ebx,STACKSIZE
Menuet_create_thread_alloc:
  push ebx
  call MenuetHeapAlloc   ; Create new dynamic memory of the given size
  pop  ecx
  test eax,eax
  jnz  Menuet_create_thread_mem_created
end if
  or   eax,-1
  jmp  Menuet_create_thread_end
Menuet_create_thread_mem_created:
  lea  edx,[eax+ebx]
Menuet_create_thread_after_new:
  neg  ebx
  jz   Menuet_create_thread_test_first
  add  ebx,edx
Menuet_create_thread_test_first:
  cmp  byte [@Menuet@_ThreadSavedBegProc],0x90
  jnz  Menuet_create_thread_init
Menuet_create_thread_fill_stack:
  lock inc dword [@Menuet@_ThreadNumber]
  and  edx,not 3
  sub  edx,12
  mov  ecx,[esp+8]
  mov  dword [edx+8],ebx
  mov  dword [edx+4],ecx
  mov  dword [edx],Menuet_ThreadFinish
  mov  eax,51
  mov  ebx,1
  mov  ecx,@Menuet@ThreadMain$qpvt1
  int  0x40
  mov  ebx,eax
  or   bl,15
  inc  ebx
  jnz  Menuet_create_thread_end
  lock dec dword [@Menuet@_ThreadNumber]
if defined MenuetHeapFree
  or   ebx,[edx+8]
  jz   Menuet_create_thread_end
  push ebx
  call MenuetHeapFree   ; Delete the given dynamic memory
  pop  ecx
end if
Menuet_create_thread_end:
  pop  ebx
  ret
Menuet_create_thread_init:
  push esi
  push edi
  cld
  mov  esi,@Menuet@_ThreadSavedBegProc
  mov  edi,@Menuet@GetPid$qv
  movs dword [edi],[esi]
  movs dword [edi],[esi]
  mov  edi,@Menuet@GetThreadData$qv
  movs dword [edi],[esi]
  movs dword [edi],[esi]
  mov  eax,0x90909090
  mov  edi,@Menuet@_ThreadSavedBegProc
  stos dword [edi]
  stos dword [edi]
  stos dword [edi]
  stos dword [edi]
  pop  edi
  pop  esi
  jmp  Menuet_create_thread_fill_stack
enddef

define @Menuet@_FileAccess$qpv
  push ebx
  mov  eax,58
  mov  ebx,[esp+8]
  int  0x40
  mov  ecx,[esp+8]
  mov  [ecx],ebx
  pop  ebx
  cmp  eax,5
  jz   Menuet_file_access_ret_zero
  ret
Menuet_file_access_ret_zero:
  xor  eax,eax
  ret
enddef

__pure_error_:
	jmp	$

define Menuet_abort_string
  db 'Abnormal program termination.',10,0
newdef Menuet_try_lock_rec_overflow_string
  db 'Recursive mutex lock count overflow.',10,0
newdef Menuet_unlock_rec_notlocked_string
  db 'Recursive mutex unlock error.',10,0
enddef

include "me_lib.inc"

;/**/


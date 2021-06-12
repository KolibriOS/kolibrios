;const int
KOLIBRI_BORDER_SIZE = 4;
;const int
KOLIBRI_HEADER_SIZE = 20;

;const int
KOLIBRI_THREAD_DATA_USER     = 0; // Thread data begin from the user dword
;const int
KOLIBRI_THREAD_DATA_ST_BEGIN = 1; // Stack beginning follows after the user dword
;const int
KOLIBRI_THREAD_DATA_NEXT     = 2;
;const int
KOLIBRI_THREAD_DATA_PID	    = 3;
;const int
KOLIBRI_THREAD_DATA_FLAG     = 4;
;const int
KOLIBRI_THREAD_DATA_X	    = 5;
;const int
KOLIBRI_THREAD_DATA_Y	    = 6;
;const int
KOLIBRI_THREAD_DATA_C_WINDOW = 7;
;const int
KOLIBRI_THREAD_DATA_C_HEADER = 8;
;const int
KOLIBRI_THREAD_DATA_C_BORDER = 9;
;const int
KOLIBRI_THREAD_DATA_C_TITLE  = 10;
;const int
KOLIBRI_THREAD_DATA_TITLE    = 11;
;const int
KOLIBRI_THREAD_DATA_PICTURE  = 12;
;const int
KOLIBRI_THREAD_DATA_SZ_PICT  = 13;
;const int
KOLIBRI_THREAD_DATA_LAST_SX  = 14;
;const int
KOLIBRI_THREAD_DATA_LAST_SY  = 15;
;const int
KOLIBRI_THREAD_DATA_LEN	    = 16;

;const int
KOLIBRI_MUTEX_MAX_TIME_WAIT  = 20;

;/***

macro segment name
{
  segment name
  if name eq _init_ | name eq _INIT_
Kolibri_SegmentInit:
  else if name eq _exit_ | name eq _EXIT_
Kolibri_SegmentExit:
  end if
}

macro endseg  name
{
  if name eq _init_ | name eq _INIT_
Kolibri_SegmentInitEnd:
  else if name eq _exit_ | name eq _EXIT_
Kolibri_SegmentExitEnd:
  end if
  endseg  name
}

macro Kolibri_Put_MovEaxVal_Ret	address,val
{
  mov  byte [address],0xB8
  mov  dword [address+4],0xC089C300
  mov  dword [address+1],val
}

align 4
proc @Kolibri@Main$qv
  and  esp,not 3
  sub  esp,1024
  mov  eax,SF_THREAD_INFO
  mov  ebx,esp
  mov  ecx,-1
  int  0x40
  mov  ebx,[esp+process_information.used_memory]
  mov  edx,[esp+process_information.PID]
  lea  eax,[ebx-0x20]
  add  esp,1024
  cmp  esp,eax
  cmova esp,eax
  and  esp,not 3
  xor  eax,eax
  cld
  mov  edi,@Kolibri@_ThreadTable
  mov  ecx,256
  rep stosd
  mov  esi,@Kolibri@GetPid$qv
  mov  edi,@Kolibri@_ThreadSavedBegProc
  movsd
  movsd
  mov  esi,@Kolibri@GetThreadData$qv
  movsd
  movsd
  Kolibri_Put_MovEaxVal_Ret  @Kolibri@GetPid$qv,edx
if defined KolibriHeapInit
  call KolibriHeapInit	; Initialize a dynamic heap
end if
  xor  eax,eax
  push eax
  push eax
  call @Kolibri@ThreadMain$qpvt1
.ThreadFinish:
  add  esp,8
if defined KolibriHeapFreeAndThreadFinish
  test eax,eax
  jz   .ThreadFinish_end
  push dword @Kolibri@_ExitProcessNow
  push eax
  call KolibriHeapFreeAndThreadFinish   ; Free the given memory and finish the thread,
end if				       ; should exit process if second argument points to not zero.
.ThreadFinish_end:
  or   eax,-1
  int  0x40
endp

align 4
proc @Kolibri@ThreadMain$qpvt1
  xchg ebx,[esp+4]
  xchg ebp,[esp+8]
  push esi edi
  sub  esp,KOLIBRI_THREAD_DATA_LEN*4
  mov  [esp],ebx
  mov  [esp+4],ebp
  mov  eax,SF_SET_EVENTS_MASK
  mov  ebx,0x27
  int  0x40
  mov  ebx,esp
  cmp  byte [@Kolibri@_ThreadSavedBegProc],0x90
  jz   .main_else_first_check
  Kolibri_Put_MovEaxVal_Ret  @Kolibri@GetThreadData$qv,esp
if defined Kolibri_SegmentInit & defined Kolibri_SegmentInitEnd
  push Kolibri_SegmentInitEnd
  push Kolibri_SegmentInit
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
  btr  dword [esp+KOLIBRI_THREAD_DATA_FLAG*4],31
if defined @@KolibriOnClose$qppv
  push esp
  call @@KolibriOnClose$qppv
  pop  ecx
  test al,al
  jnz  .main_end
end if
.main_test_close_first:
  cmp  dword [esp+KOLIBRI_THREAD_DATA_FLAG*4],0
  jl   .main_close_first
;  push esp
;  push dword 1
;  call @Kolibri@Redraw$qippv
;  add	esp,8
.main_paint_msg:
  or   dword [esp+KOLIBRI_THREAD_DATA_FLAG*4],3
  sub  esp,1024
  mov  eax,SF_THREAD_INFO
  mov  ebx,esp
  mov  ecx,-1
  int  0x40
  mov  eax,[esp+process_information.box.left]
  mov  ebx,[esp+process_information.box.top]
  mov  ecx,[esp+process_information.box.width]
  mov  edx,[esp+process_information.box.height]
  add  esp,1024
  cmp  ecx,[esp+KOLIBRI_THREAD_DATA_LAST_SX*4]
  jnz  .main_size
  cmp  edx,[esp+KOLIBRI_THREAD_DATA_LAST_SY*4]
  jz   .main_paint
.main_size:
  mov  [esp+KOLIBRI_THREAD_DATA_LAST_SX*4],ecx
  mov  [esp+KOLIBRI_THREAD_DATA_LAST_SY*4],edx
if defined @@KolibriOnSize$qpippv
  push edx
  push ecx
  push ebx
  push eax
  lea  ecx,[esp+16]
  mov  edx,esp
  push ecx
  push edx
  call @@KolibriOnSize$qpippv
  add  esp,24
end if
  test dword [esp+KOLIBRI_THREAD_DATA_FLAG*4],3
  jz   .main_cycle
.main_paint:
  cmp  dword [esp+KOLIBRI_THREAD_DATA_FLAG*4],0
  jl   .main_close
  push esp
  push dword 0
  call @Kolibri@Redraw$qippv
  add  esp,8
.main_cycle:
  mov  eax,SF_CHECK_EVENT
.main_message:
  cmp  dword [esp+KOLIBRI_THREAD_DATA_FLAG*4],0
  jl   .main_close
  int  0x40
  test eax,eax
  jnz  .main_on_message
  cmp  dword [esp+KOLIBRI_THREAD_DATA_FLAG*4],0
  jne  .main_paint
if defined @@KolibriOnIdle$qppv
  push esp
  call @@KolibriOnIdle$qppv
  pop  ecx
else
  or eax,-1
end if
  test eax,eax
  jz   .main_cycle
  jl   .main_wait_message
  mov  ebx,eax
  mov  eax,SF_WAIT_EVENT_TIMEOUT
  jmp  .main_message
.main_wait_message:
  mov  eax,SF_WAIT_EVENT
  jmp  .main_message
if defined @@KolibriOnKeyPress$qppv
.main_key_press:
  push esp
  call @@KolibriOnKeyPress$qppv
  pop  ecx
  jmp  .main_cycle
end if
if defined @@KolibriOnMouse$qppv
.main_mouse:
  push esp
  call @@KolibriOnMouse$qppv
  pop  ecx
  jmp  .main_cycle
end if

align 4
.main_on_message:
  dec  eax
  jz   .main_paint_msg
  dec  eax
if defined @@KolibriOnKeyPress$qppv
  jz   .main_key_press
else
  jz   .main_cycle
end if
  cmp  eax,4
if defined @@KolibriOnMouse$qppv
  jz   .main_mouse
else
  jz   .main_cycle
end if
  dec  eax
  jnz  .main_cycle

align 4
.main_button:
  mov  eax,SF_GET_BUTTON
  int  0x40
  shr  eax,8
  cmp  eax,1
  je   .main_close
if defined @@KolibriOnButton$qlppv
  push esp
  push eax
  call @@KolibriOnButton$qlppv
  add  esp,8
end if
  jmp  .main_cycle
.main_close:
  btr  dword [esp+KOLIBRI_THREAD_DATA_FLAG*4],31
if defined @@KolibriOnClose$qppv
  push esp
  call @@KolibriOnClose$qppv
  pop  ecx
  test al,al
  jz   .main_button
end if
.main_end:
  mov  ebx,esp
  lock dec dword [@Kolibri@_ThreadNumber]
if defined Kolibri_SegmentExit & defined Kolibri_SegmentExitEnd
  jnz  .main_else_last_check
  push Kolibri_SegmentExitEnd
  push Kolibri_SegmentExit
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
  mov  eax,SF_SLEEP
  int  0x40
  shl  ebx,1
  cmp  ebx,KOLIBRI_MUTEX_MAX_TIME_WAIT
  jna  .main_end_wait
  mov  ebx,KOLIBRI_MUTEX_MAX_TIME_WAIT
.main_end_wait:
  cmp  dword [@Kolibri@_ExitProcessNow],0
  jnz  @Kolibri@ExitProcess$qv
  cmp  dword [@Kolibri@_ThreadScanCount],0
  jnz  .main_end_wait_loop
  lock dec dword [@Kolibri@_ThreadScanCount+4]
  mov  ebp,[esp+4]
  mov  ebx,[esp]
  add  esp,KOLIBRI_THREAD_DATA_LEN*4
  mov  eax,ebp
  pop  edi esi
  xchg ebp,[esp+8]
  xchg ebx,[esp+4]
  ret
endp

align 4
proc @Kolibri@Redraw$qippv
  push ebp
  mov  ebp,[esp+12]
  mov  edx,[ebp+KOLIBRI_THREAD_DATA_FLAG*4]
  cmp  dword [esp+8],0
  jl   .redraw_only_inv
  jz   .redraw_no_frame
  or   dl,2
.redraw_no_frame:
  bt   edx,30
  jnc  .redraw_begin
  or   dl,1
  mov  [ebp+KOLIBRI_THREAD_DATA_FLAG*4],edx
  jmp  .redraw_end
.redraw_only_inv:
  test dl,3
  jnz  .redraw_no_frame
.redraw_end:
  pop  ebp
  ret
.redraw_begin:
  push ebx esi edi
  and  dword [ebp+KOLIBRI_THREAD_DATA_FLAG*4],0xFFFFFFFC
  mov  eax,SF_REDRAW
  mov  ebx,SSF_BEGIN_DRAW
  int  0x40
  test dl,2
  jz   .redraw_picture
  mov  eax,SF_STYLE_SETTINGS
  mov  ebx,SSF_GET_SKIN_HEIGHT
  int  0x40
  mov  ebx,[ebp+KOLIBRI_THREAD_DATA_X*4]
  add  ebx,2*5-1
  mov  ecx,[ebp+KOLIBRI_THREAD_DATA_Y*4]
  add  cx,ax
  add  ecx,5-1
  mov  edx,[ebp+KOLIBRI_THREAD_DATA_C_WINDOW*4]
  mov  edi,[ebp+KOLIBRI_THREAD_DATA_TITLE*4]
  xor  eax,eax ;SF_CREATE_WINDOW
  int  0x40
.redraw_picture:
  call	@@KolibriOnPaint$qv
.redraw_end_draw:
  mov  eax,SF_REDRAW
  mov  ebx,SSF_END_DRAW
  int  0x40
  pop  edi esi ebx ebp
  ret
endp

align 4
proc @Kolibri@MoveWindow$qxpxi uses ebx esi
  mov  eax,[esp+12]
  mov  ebx,[eax]
  mov  ecx,[eax+4]
  mov  edx,[eax+8]
  mov  esi,[eax+12]
  mov  eax,SF_CHANGE_WINDOW
  int  0x40
  ret
endp

align 4
proc @Kolibri@ExitDebug$qv
  push dword [@Kolibri@DebugPrefix]
  call @Kolibri@DebugPutString$qpxc
  mov	dword [esp],Kolibri_debug_string
  call @Kolibri@DebugPutString$qpxc
  pop	ecx
  jmp  @Kolibri@ExitProcess$qv
endp

align 4
proc @Kolibri@ExitProcess$qv
  lock bts dword [@Kolibri@_ExitProcessNow],0
  jc   .exit_process_wait
  sub  esp,1024
  mov  eax,SF_THREAD_INFO
  mov  ebx,esp
  mov  ecx,-1
  int  0x40
  mov  esi,eax
  mov  edi,[esp+process_information.PID]
.exit_process_loop:
  mov  eax,SF_THREAD_INFO
  mov  ebx,esp
  mov  ecx,esi
  int  0x40
  mov  eax,[esp+process_information.PID]
  cmp  eax,edi
  jz   .exit_process_continue
  mov  ebx,eax
  or   bl,15
  inc  ebx
  jz   .exit_process_continue
  mov  ebx,eax
  call Kolibri_HashInt
  movzx eax,al
  mov  eax,dword [@Kolibri@_ThreadTable+eax*4]
  jmp  .exit_process_test
.exit_process_next:
  mov  eax,dword [eax+KOLIBRI_THREAD_DATA_NEXT*4]
.exit_process_test:
  test eax,eax
  jz   .exit_process_continue
  cmp  ebx,[eax+KOLIBRI_THREAD_DATA_PID*4]
  jnz  .exit_process_next
  mov  eax,SF_SYSTEM
  mov  ebx,SSF_TERMINATE_THREAD
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
  mov  eax,SF_SLEEP
  mov  ebx,1
.exit_process_wait_loop:
  cmp  dword [@Kolibri@_ExitProcessNow],0
  jl   .exit_process_end
  int  0x40
  shl  ebx,1
  cmp  ebx,KOLIBRI_MUTEX_MAX_TIME_WAIT
  jna  .exit_process_wait_loop
  mov  ebx,KOLIBRI_MUTEX_MAX_TIME_WAIT
  jmp  .exit_process_wait_loop
endp

align 4
proc @Kolibri@ExitThread$qppv
  mov  esp,[esp+4]
  jmp  @Kolibri@ThreadMain$qpvt1.main_end
endp

align 4
proc @Kolibri@ReturnMessageLoop$qppv
  mov  esp,[esp+4]
  bt   dword [esp+KOLIBRI_THREAD_DATA_FLAG*4],30
  jc   @Kolibri@ThreadMain$qpvt1.main_end
  jmp  @Kolibri@ThreadMain$qpvt1.main_cycle
endp

align 4
proc @Kolibri@Delay$qui uses ebx
  mov  eax,SF_SLEEP
  mov  ebx,[esp+8]
  int  0x40
  ret
endp

align 4
proc @Kolibri@Clock$qv uses ebx
  mov  eax,SF_SYSTEM_GET
  mov  ebx,SSF_TIME_COUNT
  int  0x40
  ret
endp

align 4
proc @Kolibri@DrawButton$qllllll uses ebx esi
  mov  eax,SF_DEFINE_BUTTON
  mov  ebx,[esp+12-2+8]
  mov  bx,[esp+20+8]
  mov  ecx,[esp+16-2+8]
  mov  cx,[esp+24+8]
  mov  edx,[esp+4+8]
  mov  esi,[esp+8+8]
  int  0x40
  ret 
endp

align 4
proc @Kolibri@GetPackedTime$qv
  mov  eax,SF_GET_SYS_TIME
  int  0x40
  ret
endp

align 4
proc @Kolibri@GetTime$qpi
  mov  eax,SF_GET_SYS_TIME
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

align 4
proc @Kolibri@GetPackedDate$qv
  mov  eax,SF_GET_SYS_DATE
  int  0x40
  ret
endp

align 4
proc @Kolibri@GetDate$qpi
  mov  eax,SF_GET_SYS_DATE
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

align 4
proc @Kolibri@ReadCommonColors$qpui uses ebx
  mov  eax,SF_STYLE_SETTINGS
  mov  ebx,SSF_GET_COLORS
  mov  ecx,[esp+8]
  mov  edx,40
  int  0x40
  ret
endp

align 4
proc @Kolibri@DrawText$qssipxc uses ebx
  mov  eax,SF_DRAW_TEXT
  mov  ebx,[esp+8-2]
  mov  bx,[esp+12]
  mov  ecx,[esp+16]
  or   ecx,0x80000000
  mov  edx,[esp+20]
  int  0x40
  ret
endp

align 4
proc @Kolibri@DrawRect$qlllll uses ebx
  mov  eax,SF_DRAW_RECT
  mov  ebx,[esp+8-2]
  mov  bx,[esp+16]
  mov  ecx,[esp+12-2]
  mov  cx,[esp+20]
  mov  edx,[esp+24]
  int  0x40
  ret
endp

align 4
proc @Kolibri@PutImage$qpucllll uses ebx
  mov  eax,SF_PUT_IMAGE
  mov  ebx,[esp+8]
  mov  ecx,[esp+20-2]
  mov  cx,[esp+24]
  mov  edx,[esp+12-2]
  mov  dx,[esp+16]
  int  0x40
  ret
endp

align 4
proc @Kolibri@SetWindowCaption$qpxc uses ebx
  mov  eax,SF_SET_CAPTION
  mov  ebx,2
  mov  ecx,[esp+8]
  int  0x40
  ret
endp

align 4
proc @Kolibri@GetProcessInfo$qpuipct1t1piui uses ebx esi edi
  sub  esp,1024
  mov  eax,SF_THREAD_INFO
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
  lea  esi,[esp+process_information.process_name]
  cld
  movsd
  movsd
  movsd
  mov  byte [edi],0
  xor  edi,edi
.get_proc_info_no_name:
  or   edi,[1024+12+12+esp]
  jz   .get_proc_info_no_mem
  mov  ecx,[esp+process_information.used_memory]
  mov  [edi],ecx
  xor  edi,edi
.get_proc_info_no_mem:
  or   edi,[1024+12+16+esp]
  jz   .get_proc_info_no_pid
  mov  ecx,[esp+process_information.PID]
  mov  [edi],ecx
  xor  edi,edi
.get_proc_info_no_pid:
  or   edi,[1024+12+20+esp]
  jz   .get_proc_info_no_rect
  lea  esi,[esp+process_information.box]
  cld
  movsd
  movsd
  movsd
  movsd
  xor  edi,edi
.get_proc_info_no_rect:
  add  esp,1024
  ret
endp

align 4
proc @Kolibri@GetPid$qv uses ebx
  sub  esp,1024
  mov  eax,SF_THREAD_INFO
  mov  ebx,esp
  mov  ecx,-1
  int  0x40
  mov  eax,[esp+process_information.PID]
  add  esp,1024
  ret
endp

align 4
proc @Kolibri@GetPid$qppv
  mov  ecx,[esp+4]
  mov  eax,[ecx+KOLIBRI_THREAD_DATA_PID*4]
  ret
endp

align 4
proc @Kolibri@_HashByte$qui
@Kolibri@_HashWord$qui:
@Kolibri@_HashDword$qui:
  mov  eax,[esp+4]
Kolibri_HashInt:
  mul  dword [Kolibri_hash_int_val0]
  xor  eax,edx
  bswap eax
  mul  dword [Kolibri_hash_int_val1]
  shrd eax,edx,14
  bswap eax
  lea  eax,[eax+4*eax]
  ror  eax,9
  ret
endp

if defined @Kolibri@_HashByte$qui | defined @Kolibri@_HashWord$qui | defined @Kolibri@_HashDword$qui
Kolibri_hash_int_val0:
  dd   0xA82F94C5
Kolibri_hash_int_val1:
  dd   0x9193780B
end if

align 4
proc @Kolibri@GetThreadData$qv
  call @Kolibri@GetPid$qv
  push eax
  call @Kolibri@GetThreadData$qui
  pop  ecx
  ret
endp

align 4
proc @Kolibri@GetThreadData$qui
  mov  eax,[esp+4]
  call Kolibri_HashInt
  movzx eax,al
  cmp  dword [@Kolibri@_ThreadScanCount+4],0
  jnz  .get_thread_data_wait
.get_thread_data_nowait:
  lock inc dword [@Kolibri@_ThreadScanCount]
  mov  eax,dword [@Kolibri@_ThreadTable+eax*4]
  mov  ecx,[esp+4]
  jmp  .get_thread_data_test
.get_thread_data_loop:
  mov  eax,dword [eax+KOLIBRI_THREAD_DATA_NEXT*4]
.get_thread_data_test:
  test eax,eax
  jz   .get_thread_data_end
  cmp  ecx,[eax+KOLIBRI_THREAD_DATA_PID*4]
  jnz  .get_thread_data_loop
.get_thread_data_end:
  lock dec dword [@Kolibri@_ThreadScanCount]
  ret
.get_thread_data_wait:
  push eax ebx
  mov  eax,SF_SLEEP
  mov  ebx,1
.get_thread_data_wait_loop:
  int  0x40
  cmp  dword [@Kolibri@_ThreadScanCount+4],0
  jz   .get_thread_data_wait_end
  shl  ebx,1
  cmp  ebx,KOLIBRI_MUTEX_MAX_TIME_WAIT
  jna  .get_thread_data_wait_loop
  mov  ebx,KOLIBRI_MUTEX_MAX_TIME_WAIT
  jmp  .get_thread_data_wait_loop
.get_thread_data_wait_end:
  pop  ebx eax
  jmp .get_thread_data_nowait
endp

align 4
proc @Kolibri@_GetSkinHeader$qv uses ebx
  mov  eax,SF_STYLE_SETTINGS
  mov  ebx,SSF_GET_SKIN_HEIGHT
  int  0x40
  ret
endp

align 4
proc @Kolibri@GetScreenSize$qrust1
  mov  eax,SF_GET_SCREEN_SIZE
  int  0x40
  mov  ecx,[esp+8]
  mov  word [ecx],ax
  mov  ecx,[esp+4]
  shr  eax,16
  mov  word [ecx],ax
  ret
endp

align 4
proc Kolibri_MutexLockNoWait
  pop  eax
  xor  al,al
  ret
endp

align 4
proc Kolibri_MutexLockWait uses ebx
  mov  eax,SF_SLEEP
  xor  ebx,ebx
.lock_wait_cycle:
  int  0x40
  shl  byte [ecx],1
  jz   .lock_wait_cycle
  mov  al,1
  ret
endp

align 4
proc Kolibri_MutexLockWaitTime
  cmp  dword [esp+12],0
  jng  .MutexLockWait
  push ebx edx
  mov  edx,[esp+20]
  mov  eax,SF_SYSTEM_GET
  mov  ebx,SSF_TIME_COUNT
  int  0x40
  add  edx,eax
.lock_wait_time_cycle:
  mov  eax,SF_SLEEP
  xor  ebx,ebx
  int  0x40
  shl  byte [ecx],1
  jnz  .lock_wait_time_ret_true
  mov  eax,SF_SYSTEM_GET
  mov  ebx,SSF_TIME_COUNT
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

align 4
proc Kolibri_MutexLock
  shl  byte [ecx],1
  jnz  .lock_first
  call eax
.lock_first:
  mov  al,1
  ret
endp

align 4
proc @Kolibri@TryLock$qp14Kolibri@TMutex
  mov  eax,Kolibri_MutexLockNoWait
  mov  ecx,[esp+4]
  jmp  Kolibri_MutexLock
endp

align 4
proc @Kolibri@Lock$qp14Kolibri@TMutex
  mov  eax,Kolibri_MutexLockWait
  mov  ecx,[esp+4]
  jmp  Kolibri_MutexLock
endp

align 4
proc @Kolibri@LockTime$qp14Kolibri@TMutexi
  mov  eax,Kolibri_MutexLockWaitTime
  mov  ecx,[esp+4]
  jmp  Kolibri_MutexLock
endp

align 4
proc @Kolibri@UnLock$qp14Kolibri@TMutex
  mov  ecx,[esp+4]
  shr  byte [ecx],1
  jz   .unlock_pause
  ret
.unlock_pause:
  mov  byte [ecx],0x40
  push ebx
  mov  eax,SF_SLEEP
  xor  ebx,ebx
  int  0x40
  pop  ebx
  ret
endp

align 4
proc Kolibri_MutexLockRec
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
  mov  dword [esp],Kolibri_try_lock_rec_overflow_string
  call @Kolibri@DebugPutString$qpxc
  pop  ecx
  jmp  @Kolibri@ExitDebug$qv
endp

align 4
proc @Kolibri@TryLock$qp16Kolibri@TRecMutexui
  mov  eax,Kolibri_MutexLockNoWait
  mov  ecx,[esp+4]
  mov  edx,[esp+8]
  jmp  Kolibri_MutexLockRec
endp

align 4
proc @Kolibri@Lock$qp16Kolibri@TRecMutexui
  mov  eax,Kolibri_MutexLockWait
  mov  ecx,[esp+4]
  mov  edx,[esp+8]
  jmp  Kolibri_MutexLockRec
endp

align 4
proc @Kolibri@LockTime$qp16Kolibri@TRecMutexiui
  mov  eax,Kolibri_MutexLockWaitTime
  mov  ecx,[esp+4]
  mov  edx,[esp+12]
  jmp  Kolibri_MutexLockRec
endp

align 4
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
  mov  eax,SF_SLEEP
  xor  ebx,ebx
  int  0x40
  pop  ebx
  ret
.unlock_rec_notlocked:
  push dword [@Kolibri@DebugPrefix]
  call @Kolibri@DebugPutString$qpxc
  mov  dword [esp],Kolibri_unlock_rec_notlocked_string
  call @Kolibri@DebugPutString$qpxc
  pop  ecx
  jmp  @Kolibri@ExitDebug$qv
endp

align 4
proc @Kolibri@DebugPutChar$qc
  mov  cl,byte [esp+4]
  cmp  cl,13
  jz   .debug_put_char_ret
  push ebx
  cmp  cl,10
  jz   .debug_put_char_enter
.debug_put_char_after_cmp:
  mov  eax,SF_BOARD
  mov  ebx,SSF_DEBUG_WRITE
  int  0x40
  pop  ebx
.debug_put_char_ret:
  ret
.debug_put_char_enter:
  mov  cl,13
  mov  eax,SF_BOARD
  mov  ebx,SSF_DEBUG_WRITE
  int  0x40
  mov  cl,10
  jmp  .debug_put_char_after_cmp
endp

align 4
proc @Kolibri@DebugPutString$qpxc uses esi
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
  pop  ecx
  ret
endp

align 4
proc @Kolibri@GetKey$qv
  mov  eax,SF_GET_KEY
  int  0x40
  test al,al
  jnz  .get_key_eof
  movzx eax,ah
  ret
.get_key_eof:
  mov  eax,SF_TERMINATE_PROCESS
  ret
endp

align 4
proc @Kolibri@GetMousePosition$qrst1o uses ebx
  mov  eax,SF_MOUSE_GET
  xor  ebx,ebx ;SSF_SCREEN_POSITION
  cmp  byte [esp+16],0
  jnz  @f
  inc  ebx ;SSF_WINDOW_POSITION
@@:
  int  0x40
  mov  ecx,[esp+12]
  mov  word [ecx],ax
  mov  ecx,[esp+8]
  shr  eax,16
  mov  word [ecx],ax
  ret
endp

align 4
proc @Kolibri@GetMouseButton$qv uses ebx
  mov  eax,SF_MOUSE_GET
  mov  ebx,SSF_BUTTON_EXT
  int  0x40
  ret
endp

align 4
proc @Kolibri@GetMouseScrollData$qrst1 uses ebx
  mov  eax,SF_MOUSE_GET
  mov  ebx,SSF_SCROLL_DATA
  int  0x40
  mov  ecx,[esp+12]
  mov  word [ecx],ax
  mov  ecx,[esp+8]
  shr  eax,16
  mov  word [ecx],ax
  ret
endp

align 4
proc @Kolibri@WasThreadCreated$qv
  cmp  byte [@Kolibri@_ThreadSavedBegProc],0x90
  setz al
  ret
endp

align 4
proc @Kolibri@CreateThread$qpvuit1
  push ebx
  mov  edx,[esp+16]
  mov  ebx,[esp+12]
  test edx,edx
  jnz  .create_thread_after_new
if defined KolibriHeapAlloc
  cmp  ebx,4096
  jnb  .create_thread_alloc
  mov  ebx,STACKSIZE
.create_thread_alloc:
  push ebx
  call KolibriHeapAlloc	 ; Create new dynamic memory of the given size
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
  mov  dword [edx],Kolibri_ThreadFinish
  mov  eax,SF_CREATE_THREAD
  mov  ebx,1
  mov  ecx,@Kolibri@ThreadMain$qpvt1
  int  0x40
  mov  ebx,eax
  or   bl,15
  inc  ebx
  jnz  .create_thread_end
  lock dec dword [@Kolibri@_ThreadNumber]
if defined KolibriHeapFree
  or   ebx,[edx+8]
  jz   .create_thread_end
  push ebx
  call KolibriHeapFree	; Delete the given dynamic memory
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
  movsd
  movsd
  mov  edi,@Kolibri@GetThreadData$qv
  movsd
  movsd
  mov  eax,0x90909090
  mov  edi,@Kolibri@_ThreadSavedBegProc
  stosd
  stosd
  stosd
  stosd
  pop  edi esi
  jmp  .create_thread_fill_stack
endp

align 4
proc @Kolibri@_FileAccess$qp21Kolibri@FileInfoBlock uses ebx
  mov  eax,SF_FILE
  mov  ebx,[esp+8]
  int  0x40
  mov  ecx,[esp+8]
  mov  [ecx],ebx
  ret
endp

if defined Kolibri_debug_string
Kolibri_debug_string:
  db 'Abnormal program termination.',10,0
end if

if defined Kolibri_MutexLockRec
Kolibri_try_lock_rec_overflow_string:
  db 'Recursive mutex lock count overflow.',10,0
end if

if defined @Kolibri@UnLock$qp16Kolibri@TRecMutexui
Kolibri_unlock_rec_notlocked_string:
  db 'Recursive mutex unlock error.',10,0
end if

include "kos_lib.inc"

;/**/


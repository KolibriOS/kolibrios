define  @Menuet@OpenFileDialog$qr22Menuet@TOpenFileStruct
  push ebx
  push esi
  push edi
  push ebp
  call @Menuet@GetThreadData$qv
  mov  ebp,eax
  mov  ebx,[esp+20]
  cmp  dword [ebx+4],0
  jnz  Menuet_open_file_dlg_str
  mov  esi,7
  jmp  Menuet_open_file_dlg_alloc
Menuet_open_file_dlg_str:
  cld
  mov  edi,[ebx+4]
  mov  ecx,-1
  xor  al,al
  repnz scas byte [edi]
  not  ecx
  mov  esi,ecx
Menuet_open_file_dlg_alloc:
  push esi
  call @@Menuet@Alloc$qui
  pop  ecx
  test eax,eax
  jz   Menuet_open_file_dlg_ret
  mov  ecx,esi
  dec  ecx
  push esi
  push ecx
  push eax
  mov  dword [ebx],-1
  cmp  dword [ebx+4],0
  jnz  Menuet_open_file_dlg_copy
  mov  dword [eax],0x2F64722F
  mov  word [eax+4],0x2F31
  jmp  Menuet_open_file_dlg_redraw
Menuet_open_file_dlg_copy:
  cld
  mov  ecx,esi
  mov  esi,[ebx+4]
  mov  edi,eax
  rep movs byte [edi],[esi]
Menuet_open_file_dlg_redraw:
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
  mov  eax,4
  mov  ebx,0x00070007
  mov  ecx,[ebp+MENUET_THREAD_DATA_C_TITLE*4]
  mov  edx,Menuet_open_file_dlg_name
  mov  esi,Menuet_open_file_dlg_name_end - Menuet_open_file_dlg_name
  int  0x40
  mov  eax,4
  mov  ebx,0x000C001E
  xor  ecx,ecx
  mov  edx,[esp]
  mov  esi,[esp+4]
  int  0x40
  mov  eax,12
  mov  ebx,2
  int  0x40
Menuet_open_file_dlg_loop:
  mov  eax,10
  int  0x40
  dec  eax
  jz   Menuet_open_file_dlg_redraw
  dec  eax
  jz   Menuet_open_file_dlg_key
  dec  eax
  jz   Menuet_open_file_dlg_end
  jmp  Menuet_open_file_dlg_loop
Menuet_open_file_dlg_key:
  xor  edi,edi
Menuet_open_file_dlg_key_loop:
  mov  eax,2
  int  0x40
  test al,al
  jnz  Menuet_open_file_dlg_key_end
  cmp  ah,27
  jz   Menuet_open_file_dlg_end
  cmp  ah,13
  jz   Menuet_open_file_dlg_apply
  cmp  ah,8
  jz   Menuet_open_file_dlg_key_bsp
  cmp  ah,32
  jna  Menuet_open_file_dlg_key_loop
  mov  ebx,[esp+4]
  cmp  ebx,[esp+8]
  jb   Menuet_open_file_dlg_key_any
  lea  esi,[ebx+2*ebx]
  shr  esi,1
  inc  esi
  push eax
  push esi
  push dword [esp+8]
  call @@Menuet@ReAlloc$qpvui
  add  esp,8
  mov  ecx,eax
  pop  eax
  test ecx,ecx
  jz   Menuet_open_file_dlg_key
  mov  [esp+8],esi
  mov  [esp],ecx
Menuet_open_file_dlg_key_any:
  mov  esi,ebx
  add  esi,[esp]
  mov  byte [esi],ah
  inc  ebx
  mov  edi,1
  mov  [esp+4],ebx
  jmp  Menuet_open_file_dlg_key_loop
Menuet_open_file_dlg_key_bsp:
  mov  eax,[esp+4]
  test eax,eax
  jz   Menuet_open_file_dlg_key_loop
  dec  eax
  mov  edi,1
  mov  [esp+4],eax
  jmp  Menuet_open_file_dlg_key_loop
Menuet_open_file_dlg_key_end:
  test edi,edi
  jnz  Menuet_open_file_dlg_redraw
  jmp  Menuet_open_file_dlg_loop
Menuet_open_file_dlg_apply:
  mov  eax,[esp+4]
  inc  eax
  mov  [esp+8],eax
  push eax
  push dword [esp+4]
  call @@Menuet@ReAlloc$qpvui
  add  esp,8
  test eax,eax
  jz   Menuet_open_file_dlg_end
  mov  eax,[esp]
  mov  esi,eax
  add  eax,[esp+4]
  mov  byte [eax],0
  add  esp,12
  mov  ebx,[esp+20]
  mov  dword [ebx],2
  push dword [ebx+4]
  call @@Menuet@Free$qpv
  pop  ecx
  mov  [ebx+4],esi
  jmp  Menuet_open_file_dlg_invalidate
Menuet_open_file_dlg_end:
  call @@Menuet@Free$qpv
  add  esp,12
  mov  ebx,[esp+20]
  mov  dword [ebx],1
Menuet_open_file_dlg_invalidate:
  push ebp
  push dword 1
  call @@Menuet@Invalidate$qippv
  add  esp,8
Menuet_open_file_dlg_ret:
  pop  ebp
  pop  edi
  pop  esi
  pop  ebx
  ret
Menuet_open_file_dlg_name:
  db   'Open file'
Menuet_open_file_dlg_name_end:
enddef

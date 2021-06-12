
KolibriHeapInit = @Kolibri@HeapInit$qv
KolibriHeapAlloc = @Kolibri@Alloc$qul
KolibriHeapReAlloc = @Kolibri@ReAlloc$qpvul
KolibriHeapFree = @Kolibri@Free$qpv

@$bnwa$qui equ @Kolibri@Alloc$qul ;new variable
@$bnew$qui equ @Kolibri@Alloc$qul ;new struct or class
@$bdele$qpv equ @Kolibri@Free$qpv ;delete

align 4
proc @Kolibri@HeapInit$qv uses ebx
  mov  eax,SF_SYS_MISC
  mov  ebx,SSF_HEAP_INIT
  int  0x40
  ret
endp

align 4
proc @Kolibri@Alloc$qul uses ebx
  mov  eax,SF_SYS_MISC
  mov  ebx,SSF_MEM_ALLOC
  mov  ecx,[esp+8]
  int  0x40
  ret
endp

align 4
proc @Kolibri@ReAlloc$qpvul uses ebx
  mov  eax,SF_SYS_MISC
  mov  ebx,SSF_MEM_REALLOC
  mov  ecx,[esp+12]
  mov  edx,[esp+8]
  int  0x40
  ret
endp

align 4
proc @Kolibri@Free$qpv uses ebx
  mov  eax,SF_SYS_MISC
  mov  ebx,SSF_MEM_FREE
  mov  ecx,[esp+8]
  int  0x40
  ret
endp

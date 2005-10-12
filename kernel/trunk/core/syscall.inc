;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                            ;;
;;                   SYSTEM CALL ENTRY                        ;;
;;                                                            ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
uglobal
  task_tss rd 1
endg
align 32
i40:
      pushad

      push  ds
      mov   ax,word os_data
      mov   ds,ax
      mov   es,ax
      mov   [task_tss],esp
      add   dword [task_tss],4
      cli

      mov    edi,[0x3000]
      mov    eax,edi
      shl    edi, 3
;clear busy flag in application's TSS      
      mov    [edi+gdts+ tss0 +5], word 01010000b *256 +11101001b


      shl    edi,5
      mov    [edi+0x80000+0xB0],eax  ; used i40 handler

      ; for syscall trace function
      call  save_registers

;      mov   esi, [0x3000]
;      imul  esi, tss_step
;      add   esi, tss_data
      mov   esi,[task_tss]
;      sub   esi,0x28
      ; esi holds address of TSS of interupted program
      ; load first 3 registers
      mov   eax,[esi+28];+l.eax-tss_sceleton]
      mov   ebx,[esi+16];l.ebx-tss_sceleton]
      mov   ecx,[esi+24];l.ecx-tss_sceleton]

      ; save current registers
      ; stack may be modified by a system function to return some value to caller!
      pushad

      ; load all registers from TSS of the application, in crossed order (why?)
      mov   edi,[esi+28];l.eax-tss_sceleton]
      mov   eax,[esi+16];l.ebx-tss_sceleton]
      mov   ebx,[esi+24];l.ecx-tss_sceleton]
      mov   ecx,[esi+20];l.edx-tss_sceleton]
      mov   edx,[esi+4];l.esi-tss_sceleton]
      mov   esi,[esi+0];l.edi-tss_sceleton]

      ; enable interupts  -  a task switch or an IRQ _CAN_ interrupt i40 handler
      sti
      ; eax doesn't need to be saved, but...
      push   eax
      and    edi,0xff
      call   dword [servetable+edi*4]
      pop    eax
      cli
       
      ; return saved and probably even changed regs
      popad

 ; <Ivan 05.03.2005> esi already loaded  -  look above "pusha"
      ;mov   esi,[0x3000]
      ;imul  esi,tss_step
      ;add   esi,tss_data
 ; </Ivan 05.03.2005>

      ; modify 3 program's registers (in its TSS)
      mov   [esi+28],eax;[esi+l.eax-tss_sceleton], eax    
      mov   [esi+16],ebx;[esi+l.ebx-tss_sceleton], ebx
      mov   [esi+24],ecx;[esi+l.ecx-tss_sceleton], ecx

      xor    eax, eax
      mov    edi, [0x3000]          ; no syscall interrupt in use anymore
      shl    edi, 8
      mov    [edi+0x80000+0xB0],eax

      pop     eax
      mov     ds,ax
      mov     es,ax
      
      popad
      iretd

      jmp    i40

label reg1    dword at 0x6000
label reg2    dword at 0x6400
label usedi40 byte  at 0x6800

uglobal
  schd dd 0x0
endg

align 4
save_registers:

      mov   esi,[0x3000]
      imul  esi,tss_step
      add   esi,tss_data

      mov   eax,[esi+l.eax-tss_sceleton]
      mov   ebx,[esi+l.ebx-tss_sceleton]
      mov   ecx,[esi+l.ecx-tss_sceleton]
      mov   edx,[esi+l.edx-tss_sceleton]
      mov   edi,[esi+l.edi-tss_sceleton]
      mov   ebp,[esi+l.ebp-tss_sceleton]

      mov   esi,[esi+l.esi-tss_sceleton]

      push  eax ecx esi edi
      mov   esi,[0x3010]
      mov   eax,[esi+0x4]
      mov   esi,esp
      inc   [save_syscall_count]
      mov   edi,[save_syscall_count]
      and   edi,0xF
      shl   edi,6
      add   edi,save_syscall_data+32
      mov   [edi-32],eax
      mov   ecx,32 / 4
      cld
      rep   movsd
      pop   edi esi ecx eax
      ret

uglobal
  save_syscall_count  dd 0x0
endg

label save_syscall_data dword at 0x5000


iglobal
  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;
  ;; SYSTEM FUNCTIONS TABLE ;;
  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;

  align 4
  servetable:

      dd sys_drawwindow          ; 0-DrawWindow
      dd syscall_setpixel        ; 1-SetPixel
      dd sys_getkey              ; 2-GetKey
      dd sys_clock               ; 3-GetTime
      dd syscall_writetext       ; 4-WriteText
      dd delay_hs                ; 5-DelayHs
      dd syscall_openramdiskfile ; 6-OpenRamdiskFile
      dd syscall_putimage        ; 7-PutImage
      dd sys_button              ; 8-DefineButton
      dd sys_cpuusage            ; 9-GetProcessInfo
      dd sys_waitforevent        ; 10-WaitForEvent
      dd sys_getevent            ; 11-CheckForEvent
      dd sys_redrawstat          ; 12-BeginDraw and EndDraw
      dd syscall_drawrect        ; 13-DrawRect
      dd syscall_getscreensize   ; 14-GetScreenSize
      dd sys_background          ; 15-bgr
      dd sys_cachetodiskette     ; 16-FlushFloppyCache
      dd sys_getbutton           ; 17-GetButton
      dd syscall_system          ; 18-Shutdown,KillApp,WindowActivate
      dd syscall_startapp        ; 19-StartApp
      dd sys_midi                ; 20-ResetMidi and OutputMidi
      dd sys_setup               ; 21-SetMidiBase,SetKeymap,SetShiftKeymap,.
      dd sys_settime             ; 22-setting date,time,clock and alarm-clock
      dd sys_wait_event_timeout  ; 23-TimeOutWaitForEvent
      dd syscall_cdaudio         ; 24-PlayCdTrack,StopCd and GetCdPlaylist
      dd sys_sb16                ; 25-SetSb16
      dd sys_getsetup            ; 26-GetMidiBase,GetKeymap,GetShiftKeymap,.
      dd sys_wss                 ; 27-SetWssMainVol and SetWssCdVol
      dd sys_sb16II              ; 28-SetSb16
      dd sys_date                ; 29-GetDate
      dd syscall_readhd          ; 30-ReadHd
      dd syscall_starthdapp      ; 31-StartHdApp
      dd syscall_delramdiskfile  ; 32-DelRamdiskFile
      dd syscall_writeramdiskfile; 33-WriteRamdiskFile
      dd read_floppy_file        ; 34-ReadFloppyDrive
      dd syscall_getpixel        ; 35-GetPixel
      dd syscall_readstring      ; 36-ReadString (not yet ready)
      dd readmousepos            ; 37-GetMousePosition_ScreenRelative,.
      dd syscall_drawline        ; 38-DrawLine
      dd sys_getbackground       ; 39-GetBackgroundSize,ReadBgrData,.
      dd set_app_param           ; 40-WantEvents
      dd syscall_getirqowner     ; 41-GetIrqOwner
      dd get_irq_data            ; 42-ReadIrqData
      dd sys_outport             ; 43-SendDeviceData
      dd sys_programirq          ; 44-ProgramIrqs
      dd reserve_free_irq        ; 45-ReserveIrq and FreeIrq
      dd syscall_reserveportarea ; 46-ReservePortArea and FreePortArea
      dd display_number          ; 47-WriteNum
      dd display_settings        ; 48-SetRedrawType and SetButtonType
      dd syscall_appints         ; 49-AppInts
      dd random_shaped_window    ; 50-Window shape & scale
      dd syscall_threads         ; 51-Threads
      dd stack_driver_stat       ; 52-Stack driver status
      dd socket                  ; 53-Socket interface
      dd user_events             ; 54-User events
      dd sound_interface         ; 55-Sound interface
      dd write_to_hd             ; 56-Write a file to hd
      dd delete_from_hd          ; 57-Delete a file from hd
      dd file_system             ; 58-Common file system interface
      dd sys_trace               ; 59-System call trace
      dd new_sys_ipc             ; 60-Inter Process Communication
      dd sys_gs                  ; 61-Direct graphics access
      dd sys_pci                 ; 62-PCI functions
      dd sys_msg_board           ; 63-System message board
      dd sys_resize_app_memory   ; 64-Resize application memory usage
      dd undefined_syscall       ; 65-UTF
      dd sys_process_def         ; 66-Process definitions - keyboard
      dd sys_window_move         ; 67-Window move or resize
      dd sys_internal_services   ; 68-Some internal services

  times 255 - ( ($-servetable) /4 )  dd undefined_syscall

      dd sys_end                 ; -1-end application
endg


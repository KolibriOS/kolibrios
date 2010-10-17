	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

; Old style system call converter
align 16
cross_order:
	; load all registers in crossed order
	mov	eax, ebx
	mov	ebx, ecx
	mov	ecx, edx
	mov	edx, esi
	mov	esi, edi
	movzx	edi, byte[esp+28 + 4]
	  sub	    edi, 53		; all zeroes before
	call	dword [servetable+edi*4]
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                            ;;
;;                   SYSTEM CALL ENTRY                        ;;
;;                                                            ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 16
i40:
	pushad
	cld
	and   eax, 0x07F
	call	dword [servetable2 + eax * 4]
	popad
	iretd

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                            ;;
;;               SYSCALL ENTRY   --    NEW  !!!               ;;
;;                                                            ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 32
syscall_entry:
;        sti
	push ecx
	and   eax, 3
	call	dword [servetable3 + eax * 4]
	pop	ecx
	sysret

iglobal
  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;
  ;; SYSTEM FUNCTIONS TABLE ;;
  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;

  align 4
  servetable:
      dd socket 		 ; 53-Socket interface
      dd 0
      dd 0
      dd 0
      dd 0
      dd file_system		 ; 58-Common file system interface
      dd 0
      dd 0
      dd 0
      dd sys_pci		 ; 62-PCI functions
      dd sys_msg_board		 ; 63-System message board

  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
  ;; NEW SYSTEM FUNCTIONS TABLE ;;
  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
  align 4
  servetable2:

      dd syscall_draw_window	 ; 0-DrawWindow
      dd syscall_setpixel	 ; 1-SetPixel
      dd sys_getkey		 ; 2-GetKey
      dd sys_clock		 ; 3-GetTime
      dd syscall_writetext	 ; 4-WriteText
      dd delay_hs		 ; 5-DelayHs
      dd syscall_openramdiskfile ; 6-OpenRamdiskFile
      dd syscall_putimage	 ; 7-PutImage
      dd syscall_button 	 ; 8-DefineButton
      dd sys_cpuusage		 ; 9-GetProcessInfo
      dd sys_waitforevent	 ; 10-WaitForEvent
      dd sys_getevent		 ; 11-CheckForEvent
      dd sys_redrawstat 	 ; 12-BeginDraw and EndDraw
      dd syscall_drawrect	 ; 13-DrawRect
      dd syscall_getscreensize	 ; 14-GetScreenSize
      dd sys_background 	 ; 15-bgr
      dd sys_cachetodiskette	 ; 16-FlushFloppyCache
      dd sys_getbutton		 ; 17-GetButton
      dd sys_system		 ; 18-System Services
      dd paleholder		 ; 19-reserved
      dd sys_midi		 ; 20-ResetMidi and OutputMidi
      dd sys_setup		 ; 21-SetMidiBase,SetKeymap,SetShiftKeymap,.
      dd sys_settime		 ; 22-setting date,time,clock and alarm-clock
      dd sys_wait_event_timeout  ; 23-TimeOutWaitForEvent
      dd undefined_syscall	; syscall_cdaudio	 ; 24-PlayCdTrack,StopCd and GetCdPlaylist
      dd undefined_syscall	 ; 25-reserved
      dd sys_getsetup		 ; 26-GetMidiBase,GetKeymap,GetShiftKeymap,.
      dd undefined_syscall	 ; 27-reserved
      dd undefined_syscall	 ; 28-reserved
      dd sys_date		 ; 29-GetDate
      dd sys_current_directory	 ; 30-Get/SetCurrentDirectory
      dd undefined_syscall	 ; 31-reserved
      dd undefined_syscall	 ; 32-reserved
      dd undefined_syscall	 ; 33-reserved
      dd undefined_syscall	 ; 34-reserved
      dd syscall_getpixel	 ; 35-GetPixel
      dd syscall_getarea	 ; 36-GetArea
      dd readmousepos		 ; 37-GetMousePosition_ScreenRelative,.
      dd syscall_drawline	 ; 38-DrawLine
      dd sys_getbackground	 ; 39-GetBackgroundSize,ReadBgrData,.
      dd set_app_param		 ; 40-WantEvents
      dd syscall_getirqowner	 ; 41-GetIrqOwner
      dd get_irq_data		 ; 42-ReadIrqData
      dd sys_outport		 ; 43-SendDeviceData
      dd sys_programirq 	 ; 44-ProgramIrqs
      dd reserve_free_irq	 ; 45-ReserveIrq and FreeIrq
      dd syscall_reserveportarea ; 46-ReservePortArea and FreePortArea
      dd display_number 	 ; 47-WriteNum
      dd syscall_display_settings ; 48-SetRedrawType and SetButtonType
      dd sys_apm		 ; 49-Advanced Power Management (APM)
      dd syscall_set_window_shape ; 50-Window shape & scale
      dd syscall_threads	 ; 51-Threads
      dd stack_driver_stat	 ; 52-Stack driver status
      dd cross_order		 ; 53-Socket interface
      dd undefined_syscall	 ; 54-reserved
      dd sound_interface	 ; 55-Sound interface
      dd undefined_syscall	 ; 56-reserved
      dd sys_pcibios		 ; 57-PCI BIOS32
      dd cross_order		 ; 58-Common file system interface
      dd undefined_syscall	 ; 59-reserved
      dd sys_IPC		 ; 60-Inter Process Communication
      dd sys_gs 		 ; 61-Direct graphics access
      dd cross_order		 ; 62-PCI functions
      dd cross_order		 ; 63-System message board
      dd sys_resize_app_memory	 ; 64-Resize application memory usage
      dd sys_putimage_palette	 ; 65-PutImagePalette
      dd sys_process_def	 ; 66-Process definitions - keyboard
      dd syscall_move_window	 ; 67-Window move or resize
      dd f68			 ; 68-Some internal services
      dd sys_debug_services	 ; 69-Debug
      dd file_system_lfn	 ; 70-Common file system interface, version 2
      dd syscall_window_settings ; 71-Window settings
      dd sys_sendwindowmsg	 ; 72-Send window message
	times 127 - ( ($-servetable2) /4 )  dd undefined_syscall
      dd sys_end		 ; -1-end application

align 4
servetable3:

	dd	paleholder	 ; 0
	dd	paleholder	 ; 1
	dd	paleholder	 ; 2
	dd	sys_end ; last

endg

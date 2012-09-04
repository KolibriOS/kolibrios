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
        movzx   edi, byte[esp+28 + 4]
	call	dword [servetable+edi*4]
	ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                            ;;
;;                     SYSENTER ENTRY                         ;;
;;                                                            ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 32
sysenter_entry:
	; Настраиваем стек
	mov	esp, [ss:tss._esp0]
	sti
	push	ebp			; save app esp + 4
	mov	ebp, [ebp]		; ebp - original ebp
	;------------------
	pushad
	cld

	movzx	eax, al
	call	dword [servetable2 + eax * 4]

	popad
	;------------------
	xchg	ecx, [ss:esp]		; в вершин стека - app ecx, ecx - app esp + 4
	sub	ecx, 4
	xchg	edx, [ecx]		; edx - return point, & save original edx
	push	edx
	mov	edx, [ss:esp + 4]
	mov	[ecx + 4], edx		; save original ecx
	pop	edx
	sysexit

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                            ;;
;;                   SYSTEM CALL ENTRY                        ;;
;;                                                            ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 16
i40:
        pushad
        cld
        movzx   eax, al
        call    dword [servetable2 + eax * 4]
        popad
        iretd

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                            ;;
;;                     SYSCALL ENTRY                          ;;
;;                                                            ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
align 32
syscall_entry:
  ;     cli                 syscall clear IF
	xchg	esp, [ss:tss._esp0]
	push	ecx
	lea	ecx, [esp+4]
	xchg	ecx, [ss:tss._esp0]
	sti
	push	ecx
	mov	ecx, [ecx]
	;------------------
	pushad
	cld

	movzx	eax, al
	call	dword [servetable2 + eax * 4]

	popad
	;------------------
	mov	ecx, [ss:esp+4]
	pop	esp
	sysret
iglobal
  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;
  ;; SYSTEM FUNCTIONS TABLE ;;
  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;

  align 4
  servetable:

      dd 0
      dd 0
      dd 0
      dd 0
      dd 0
      dd 0
      dd 0
      dd 0
      dd 0
      dd 0
      dd 0
      dd 0
      dd 0
      dd 0
      dd 0
      dd 0
      dd 0
      dd 0
      dd 0
      dd 0
      dd sys_midi		 ; 20-ResetMidi and OutputMidi
      dd sys_setup		 ; 21-SetMidiBase,SetKeymap,SetShiftKeymap,.
      dd sys_settime		 ; 22-setting date,time,clock and alarm-clock
      dd 0
      dd syscall_cdaudio	 ; 24-PlayCdTrack,StopCd and GetCdPlaylist
      dd 0                       ;
      dd sys_getsetup		 ; 26-GetMidiBase,GetKeymap,GetShiftKeymap,.
      dd 0
      dd 0                       ;
      dd 0
      dd sys_current_directory	 ; 30-Get/SetCurrentDirectory
      dd 0
      dd 0
      dd 0
      dd 0
      dd 0
      dd 0
      dd readmousepos		 ; 37-GetMousePosition_ScreenRelative,.
      dd 0			 ; 38-DrawLine
      dd sys_getbackground	 ; 39-GetBackgroundSize,ReadBgrData,.
      dd 0
      dd 0
      dd 0
      dd sys_outport		 ; 43-SendDeviceData
      dd 0
      dd 0
      dd syscall_reserveportarea ; 46-ReservePortArea and FreePortArea
      dd display_number 	 ; 47-WriteNum
      dd display_settings	 ; 48-SetRedrawType and SetButtonType
      dd sys_apm		 ; 49-Advanced Power Management (APM)
      dd random_shaped_window	 ; 50-Window shape & scale
      dd syscall_threads	 ; 51-Threads
      dd stack_driver_stat	 ; 52-Stack driver status
      dd socket 		 ; 53-Socket interface
      dd 0
      dd sound_interface	 ; 55-Sound interface
      dd 0
      dd sys_pcibios		 ; 57-PCI BIOS32
      dd file_system		 ; 58-Common file system interface
      dd 0
      dd sys_IPC		 ; 60-Inter Process Communication
      dd sys_gs 		 ; 61-Direct graphics access
      dd sys_pci		 ; 62-PCI functions
      dd sys_msg_board		 ; 63-System message board
      dd sys_resize_app_memory	 ; 64-Resize application memory usage
      dd syscall_putimage_palette; 65-PutImagePalette
      dd sys_process_def	 ; 66-Process definitions - keyboard
      dd sys_window_move	 ; 67-Window move or resize
      dd new_services		 ; 68-Some internal services
      dd sys_debug_services	 ; 69-Debug
      dd file_system_lfn	 ; 70-Common file system interface, version 2
      dd syscall_windowsettings  ; 71-Window settings
      dd sys_sendwindowmsg	 ; 72-Send window message

  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
  ;; NEW SYSTEM FUNCTIONS TABLE ;;
  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
  align 4
  servetable2:

      dd sys_drawwindow              ; 0-DrawWindow
      dd syscall_setpixel            ; 1-SetPixel
      dd sys_getkey                  ; 2-GetKey
      dd sys_clock                   ; 3-GetTime
      dd syscall_writetext           ; 4-WriteText
      dd delay_hs                    ; 5-DelayHs
      dd syscall_openramdiskfile     ; 6-OpenRamdiskFile
      dd syscall_putimage            ; 7-PutImage
      dd sys_button                  ; 8-DefineButton
      dd sys_cpuusage                ; 9-GetProcessInfo
      dd sys_waitforevent            ; 10-WaitForEvent
      dd sys_getevent                ; 11-CheckForEvent
      dd sys_redrawstat              ; 12-BeginDraw and EndDraw
      dd syscall_drawrect            ; 13-DrawRect
      dd syscall_getscreensize       ; 14-GetScreenSize
      dd sys_background              ; 15-bgr
      dd sys_cachetodiskette         ; 16-FlushFloppyCache
      dd sys_getbutton               ; 17-GetButton
      dd sys_system                  ; 18-System Services
      dd paleholder                  ; 19-reserved
      dd cross_order                 ; 20-ResetMidi and OutputMidi
      dd cross_order                 ; 21-SetMidiBase,SetKeymap,SetShiftKeymap,.
      dd cross_order                 ; 22-setting date,time,clock and alarm-clock
      dd sys_wait_event_timeout      ; 23-TimeOutWaitForEvent
      dd cross_order                 ; 24-PlayCdTrack,StopCd and GetCdPlaylist
      dd undefined_syscall           ; 25-reserved
      dd cross_order                 ; 26-GetMidiBase,GetKeymap,GetShiftKeymap,.
      dd undefined_syscall           ; 27-reserved
      dd undefined_syscall           ; 28-reserved
      dd sys_date                    ; 29-GetDate
      dd cross_order                 ; 30-Get/SetCurrentDirectory
      dd undefined_syscall           ; 31-reserved
      dd undefined_syscall           ; 32-reserved
      dd undefined_syscall           ; 33-reserved
      dd undefined_syscall           ; 34-reserved
      dd syscall_getpixel            ; 35-GetPixel
      dd undefined_syscall           ; 36-reserved
      dd cross_order                 ; 37-GetMousePosition_ScreenRelative,.
      dd syscall_drawline            ; 38-DrawLine
      dd cross_order                 ; 39-GetBackgroundSize,ReadBgrData,.
      dd set_app_param               ; 40-WantEvents
      dd syscall_getirqowner         ; 41-GetIrqOwner
      dd get_irq_data                ; 42-ReadIrqData
      dd cross_order                 ; 43-SendDeviceData
      dd sys_programirq              ; 44-ProgramIrqs
      dd reserve_free_irq            ; 45-ReserveIrq and FreeIrq
      dd cross_order                 ; 46-ReservePortArea and FreePortArea
      dd cross_order                 ; 47-WriteNum
      dd cross_order                 ; 48-SetRedrawType and SetButtonType
      dd cross_order                 ; 49-Advanced Power Management (APM)
      dd cross_order                 ; 50-Window shape & scale
      dd cross_order                 ; 51-Threads
      dd cross_order                 ; 52-Stack driver status
      dd cross_order                 ; 53-Socket interface
      dd undefined_syscall           ; 54-reserved
      dd cross_order                 ; 55-Sound interface
      dd undefined_syscall           ; 56-reserved
      dd cross_order                 ; 57-PCI BIOS32
      dd cross_order                 ; 58-Common file system interface
      dd undefined_syscall           ; 59-reserved
      dd cross_order                 ; 60-Inter Process Communication
      dd cross_order                 ; 61-Direct graphics access
      dd cross_order                 ; 62-PCI functions
      dd cross_order                 ; 63-System message board
      dd cross_order                 ; 64-Resize application memory usage
      dd cross_order                 ; 65-PutImagePalette
      dd cross_order                 ; 66-Process definitions - keyboard
      dd cross_order                 ; 67-Window move or resize
      dd cross_order                 ; 68-Some internal services
      dd cross_order                 ; 69-Debug
      dd cross_order                 ; 70-Common file system interface, version 2
      dd cross_order                 ; 71-Window settings
      dd cross_order                 ; 72-Send window message
      dd stb_create_window           ; 73-create window
      dd stb_show_window             ; 74-show window
      dd stb_get_event               ; 75-get event
      dd stb_def_window_proc
	times 255 - ( ($-servetable2) /4 )  dd undefined_syscall
      dd sys_end		 ; -1-end application

endg

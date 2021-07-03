;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2015. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                            ;;
;;                     SYSENTER ENTRY                         ;;
;;                                                            ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 32
sysenter_entry:
        ; Setting up the stack
        mov     esp, [ss:tss._esp0]
        sti
        push    ebp                     ; save app esp + 4
        mov     ebp, [ebp]              ; ebp - original ebp
        ;------------------
        pushad
        cld

        call    protect_from_terminate

        movzx   eax, byte [esp+28]
        mov     edx, dword [esp+20]
        call    dword [servetable2 + eax * 4]

        call    unprotect_from_terminate
        popad
        ;------------------
        xchg    ecx, [ss:esp]           ; in the stack top - app ecx, ecx - app esp + 4
        sub     ecx, 4
        xchg    edx, [ecx]              ; edx - return point, & save original edx
        push    edx
        mov     edx, [ss:esp + 4]
        mov     [ecx + 4], edx          ; save original ecx
        pop     edx
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
        call    protect_from_terminate
        movzx   eax, byte [esp+28]
        mov     edx, dword [esp+20]
        call    dword [servetable2 + eax * 4]
        call    unprotect_from_terminate
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
        xchg    esp, [ss:tss._esp0]
        push    ecx
        lea     ecx, [esp+4]
        xchg    ecx, [ss:tss._esp0]
        sti
        push    ecx
        mov     ecx, [ecx]
        ;------------------
        pushad
        cld
        call    protect_from_terminate

        movzx   eax, byte [esp+28]
        mov     edx, dword [esp+20]
        call    dword [servetable2 + eax * 4]

        call    unprotect_from_terminate
        popad
        ;------------------
        mov     ecx, [ss:esp+4]
        pop     esp
        sysret

iglobal
  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;
  ;; SYSTEM FUNCTIONS TABLE ;;
  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;
  align 4
  servetable2:

      dd syscall_draw_window            ;  0 - define and draw the window;
      dd syscall_setpixel               ;  1 - put pixel in the window;
      dd syscall_getkey                 ;  2 - get the code of the pressed key;
      dd syscall_clock                  ;  3 - get system time;
      dd syscall_writetext              ;  4 - draw text string;
      dd syscall_delay_hs_unprotected   ;  5 - delay;
      dd syscall_undefined              ;  6 - system call undefined;
      dd syscall_putimage               ;  7 - draw image in the window;
      dd syscall_button                 ;  8 - define/delete the button;
      dd syscall_thread_info            ;  9 - information on execution thread;
      dd syscall_waitforevent           ; 10 - wait for event;
      dd syscall_checkforevent          ; 11 - check for event, no wait;
      dd syscall_redrawstat             ; 12 - begin/end window redraw;
      dd syscall_drawrect               ; 13 - draw a rectangle in the window;
      dd syscall_getscreensize          ; 14 - get screen size;
      dd syscall_background             ; 15 - working with the background;
      dd syscall_cachetodiskette        ; 16 - save ramdisk on a floppy;
      dd syscall_getbutton              ; 17 - get the identifier of the pressed button;
      dd syscall_system                 ; 18 - system services;
      dd syscall_undefined              ; 19 - system call undefined;
      dd syscall_midi                   ; 20 - reset MIDI and output MIDI;
      dd syscall_setup                  ; 21 - setting system parameters;
      dd syscall_settime                ; 22 - setting date, time, clock and alarm-clock;
      dd syscall_wait_event_timeout     ; 23 - wait for event with timeout;
      dd syscall_cdaudio                ; 24 - work with a CD player;
      dd syscall_putarea_backgr         ; 25 - put area to background;
      dd syscall_getsetup               ; 26 - get system parameters;
      dd syscall_undefined              ; 27 - system call undefined;
      dd syscall_undefined              ; 28 - system call undefined;
      dd syscall_date                   ; 29 - get date;
      dd syscall_current_directory      ; 30 - get/set current directory;
      dd syscall_undefined              ; 31 - system call undefined;
      dd syscall_undefined              ; 32 - system call undefined;
      dd syscall_undefined              ; 33 - system call undefined;
      dd syscall_getpixel_WinMap        ; 34 - find out who the screen point belongs to;
      dd syscall_getpixel               ; 35 - read the color of the pixel on the screen;
      dd syscall_getarea                ; 36 - read the screen area;
      dd syscall_get_mouse_pos          ; 37 - work with the mouse;
      dd syscall_drawline               ; 38 - draw a line;
      dd syscall_getbackground          ; 39 - get background info;
      dd syscall_set_eventmask          ; 40 - set event mask;
      dd syscall_undefined              ; 41 - system call undefined;
      dd syscall_undefined              ; 42 - system call undefined;
      dd syscall_outport                ; 43 - input / output to the port;
      dd syscall_undefined              ; 44 - system call undefined;
      dd syscall_undefined              ; 45 - system call undefined;
      dd syscall_reserveportarea        ; 46 - reserve port area and free port area;
      dd syscall_putnumber              ; 47 - print a number to the window;
      dd syscall_window_styles          ; 48 - window display styles;
      dd syscall_apm                    ; 49 - Advanced Power Management (APM);
      dd syscall_set_window_shape       ; 50 - window shape & scale;
      dd syscall_threads                ; 51 - threads;
      dd syscall_undefined              ; 52 - system call undefined;
      dd syscall_undefined              ; 53 - system call undefined;
      dd syscall_clipboard              ; 54 - working with the clipboard;
      dd syscall_sound_interface        ; 55 - sound interface;
      dd syscall_undefined              ; 56 - system call undefined;
      dd syscall_pcibios                ; 57 - PCI BIOS32;
      dd syscall_undefined              ; 58 - system call undefined; 
      dd syscall_undefined              ; 59 - system call undefined;
      dd syscall_IPC                    ; 60 - Inter Process Communication(IPC);
      dd syscall_dga                    ; 61 - direct graphics access;
      dd syscall_pci_api                ; 62 - PCI functions;
      dd syscall_msg_board              ; 63 - system message board;
      dd syscall_resize_app_memory      ; 64 - resize application memory usage;
      dd syscall_putimage_palette       ; 65 - display an image with a palette in the window;
      dd syscall_process_def            ; 66 - process definitions - keyboard;
      dd syscall_move_window            ; 67 - window move or resize;
      dd syscall_some_intrenal_services ; 68 - some internal services;
      dd syscall_debug_services         ; 69 - debug services;
      dd syscall_file_system_lfn        ; 70 - file system interface;
      dd syscall_window_settings        ; 71 - window settings;
      dd syscall_send_window_msg        ; 72 - send window message;
      dd syscall_blit_32                ; 73 - graphic blitter;
      dd syscall_network                ; 74 - work with network devices;
      dd syscall_socket                 ; 75 - work with network sockets;
      dd syscall_protocols              ; 76 - network options and statistics;
      dd syscall_posix_subsystem        ; 77 - POSIX subsystem;
      dd syscall_undefined              ; 78 - system call undefined;
      dd syscall_undefined              ; 79 - system call undefined;
      dd syscall_fs_diff_encodings      ; 80 - file system interface for different encodings;

        times 255 - ( ($-servetable2) /4 )  dd syscall_undefined
      dd syscall_end                    ; -1-end application

endg

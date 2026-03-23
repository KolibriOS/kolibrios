;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2013-2024. All rights reserved. ;;
;;  Distributed under terms of the GNU General Public License.  ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; wraps command line arguments in quotes and launches a target.
        include 'KOSfuncs.inc'
        include 'macros.inc'

        org     0x0
        use32

        db      'MENUET01'
        dd      1
        dd      start
        dd      i_end
        dd      m_end
        dd      m_end
        dd      path
        dd      target_path

start:
        mov     edi, target_path
        or      ecx, -1
        xor     al, al
        repne scasb
        
        sub     edi, 6
        mov     byte [edi], 0

        cmp     byte [path], 0
        je      .launch

        mov     byte [q_path], '"'

        mov     edi, path
        or      ecx, -1
        xor     al, al
        repne scasb
        
        mov     word [edi-1], '"'

        mov     dword [fi + 8], q_path

.launch:
        mcall   SF_FILE, fi
        mcall   SF_TERMINATE_PROCESS

fi:
        dd      SSF_START_APP, 0, 0, 0, 0
        db      0
        dd      target_path

i_end:

q_path rb 1
path   rb 255

target_path rb 1024

m_end:
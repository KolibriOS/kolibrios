NOTIFY_RUN:

 ;; Make param
    stdcall string.copy, sz_quote, params
	stdcall string.copy, filelist_first, current_filename
    call    make_text
    stdcall string.concatenate, sz_quote, params
    stdcall string.concatenate, sz_flags, params
;; RUN NOTIFY	
    mcall   70, fi_launch

 ;; CONVERT PID TO STR
    mov     ebx, 10
    mov     ecx, 0
  @@:
    mov     edx, 0
    div     ebx
    push    edx
    inc     ecx
    cmpne   eax, 0, @b

    mov     ebx, ctrl.name
  @@:
    pop     eax
    add     al, "0"
    mov     [ebx], al
    inc     ebx
    loop    @b

 ;; ADD POSTFIX TO STR
    mov     dword [ebx + 0], "-NOT"
    mov     dword [ebx + 4], "IFY"

 ;; OPEN CONTROLLER (0x08 + 0x01 -- CREATE AND READ/WRITE)
    mcall   68, 22, ctrl.name, 2048, 0x09
    mov     [ctrl.addr], eax

 ;; WAIT UNTIL CONTROLLER BECOMES READY TO USE
    add     eax, NTCTRL_READY
  @@:
	push    eax
    mcall   5, 1
	pop     eax
    cmpe    byte [eax], 0, @b

 ;; CONFIG PBAR
    mov     eax, [ctrl.addr]
    add     eax, NTCTRL_PBAR_MAX
    mov     dword [eax], 55

    mov     eax, [ctrl.addr]
    add     eax, NTCTRL_PBAR_CUR
    mov     dword [eax], 0

    mov     eax, [ctrl.addr]
    add     eax, NTCTRL_APPLY_PBAR
    mov     byte [eax], 1
	
	ret






	
	
NOTIFY_CHANGE:

 ;; CHANGE TIMER
    inc     dword [timer]
	mov     ebx,  dword [timer]
	
 ;; SEND TIMER TO PBAR
    mov     eax, [ctrl.addr]
    add     eax, NTCTRL_PBAR_CUR
    mov     dword [eax], ebx

 ;; APPLY PBAR
    mov     eax, [ctrl.addr]
    add     eax, NTCTRL_APPLY_PBAR
    mov     byte [eax], 1

  ;; CNANGE TEXT
    mov     byte [params], 0
    call    make_text

  ;; SEND TEXT TO NOTIFY
    mov     eax, [ctrl.addr]
    add     eax, NTCTRL_TEXT
    stdcall string.copy, params, eax

  ;; APPLY NEW TEXT
    mov     eax, [ctrl.addr]
    add     eax, NTCTRL_APPLY_TEXT
    mov     byte [eax], 1
	
	ret
	




	

EXIT:
 ;; CHANGE ICON
    mov     eax, [ctrl.addr]
    add     eax, NTCTRL_ICON
    mov     byte [eax], 4
	
 ;; APPLY NEW ICON
    mov     eax, [ctrl.addr]
    add     eax, NTCTRL_APPLY_ICON
    mov     byte [eax], 1
	
  ;; CNANGE TEXT
    mov     byte [params], 0
    stdcall string.concatenate, sz_final_text, params

	;; SEND TEXT TO NOTIFY
    mov     eax, [ctrl.addr]
    add     eax, NTCTRL_TEXT
	stdcall string.copy, params, eax

  ;; APPLY NEW TEXT
    mov     eax, [ctrl.addr]
    add     eax, NTCTRL_APPLY_TEXT
    mov     byte [eax], 1
	
	mcall 5, 300
	
	mcall   70, fileopen

   ;; CLOSE NOTIFY
    mov     eax, [ctrl.addr]
    add     eax, NTCTRL_CLOSE
    mov     byte [eax], 1
  
    mcall   -1

;-------------------------------------------------------------------------------
 make_text:
    stdcall string.concatenate, sz_text, params
    stdcall string.concatenate, sz_sec_line_start, params
    stdcall string.concatenate, current_filename, params

    ret
;-------------------------------------------------------------------------------
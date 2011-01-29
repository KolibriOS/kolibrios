exit_apl:
    mov  [confirmation_type],exit_type
    call confirmation_action
    cmp  [work_confirmation_yes],1
    jne  red
    mcall -1
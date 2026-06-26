; Splits command line to argv array.
; Uses standard Windows rules:
; * in normal mode, arguments are separated with spaces and tabs,
;   duplicate spaces and tabs are ignored
;   (two sequential spaces are the same as one);
; * unescaped quote " in normal mode starts quoted mode, 
;   it does not end the current argument, it is not included in the argument;
; * spaces and tabs in quoted mode are included in the argument as is;
; * unescaped quote " in quoted mode returns to normal mode,
;   it does not end the current argument, it is not included in the argument;
; * quotes can be escaped with backslashes \ in both modes
;   (the recommended way), \" means copying " to the argument
;   without switching modes;
; * backslashes not before a quote are just regular characters,
;   backslashes before a quote should be escaped by another backslash:
;   " means unescaped quote
;   \" means character "
;   \\" means character \ plus unescaped quote
;   \\\" means characters \"
;   and so on;
; * quotes in quoted mode can also be escaped by doubling them, ""
;   (the confusing way); note that in normal mode "" means empty argument.
; For example, the command line
; begin"quoted mode"end\ \"escaped" "quotes" "1\" "" """escaped quotes 2"""
; has 4 arguments:
; 1) beginquoted modeend\
; 2) "escaped quotes 1"
; 3)
; 4) "escaped quotes 2"
; The recommended way to create a command line with the same arguments:
; "beginquoted modeend"\ "\"escaped quotes 1\"" "\"escaped quotes 2\"".
;
; in: esi -> command line
; in: edi -> data for arguments, maybe null
; in: edx -> pointers to arguments, maybe null
; out: ebx = argument count
;
; There are too many branches and labels here,
; isolate some of them into macro.
macro start_arg
; Increments argument count;
; if arguments are tracked, stores the current address.
{
local .label
        test    edx, edx
        jz      .label
        mov     [edx], edi
        add     edx, 4
.label:
        inc     ebx
}
; In typical cases decoded arguments and input line have large chunks in common.
; When going through the input string, we do not copy arguments immediately,
; but track size of last chunk that should be copied instead.
; This macros copies the last chunk of data if arguments are tracked.
; If arguments are tracked, ecx is reset to zero;
; otherwise, we do not care about ecx.
macro copy_arg_data
{
local .label
        test    edi, edi
        jz      .label
        dec     esi
        sub     esi, ecx
        rep movsb
        inc     esi
.label:
}
; Process backslash.
macro process_slash
{
; 1. Count number of backslashes.
local .label1, .label2
        xor     ecx, ecx
.label1:
        inc     ecx
        mov     al, byte [esi]
        inc     esi
        cmp     al, '\'
        jz      .label1
; 2. If the next character is not ", backslash is a regular character;
; copy all of them.
        cmp     al, '"'
        jnz     .label2
; 3. If the next character is ", then only half of backslashes
; should be copied, other are escaping characters.
; If number of backslashes is odd, include " to copied chunk
; and advance to the next character.
        shr     ecx, 1
        jnc     .label2
        mov     al, byte [esi]
        inc     esi
        inc     ecx
.label2:
        copy_arg_data
}

; Parser procedure.
proc parse_cmdline
; Registers:
; ebx = argc = argument count
; ecx = size of last chunk if edi is nonzero, garbage otherwise
; al = current input character = [esi-1]
; esi = pointer to input past the current character
; edi = zero or pointer to the next output data
; edx = zero or pointer to the next output pointer
        xor     ebx, ebx
        xor     ecx, ecx
; There are two large blocks of code for normal and quoted modes.
; We start in normal mode.
; 1. Processing in normal mode.
; 1a. Skip initial spaces and tabs.
.skip_spaces:
        mov     al, byte [esi]
        inc     esi
        cmp     al, ' '
        jz      .skip_spaces
        cmp     al, 9
        jz      .skip_spaces
; 1b. If the command line has ended, exit.
        test    al, al
        jz      .done
; 1c. Any character in this state starts a new argument.
        start_arg
; 1d. Loop over the input string, watching for one of:
; (space), (tab), (terminator), ", \
; All other characters should be copied as is.
; The first character here cannot be (space), (tab) or (terminator),
; but " and \ are possible. For these, skip 1e, because we have nothing
; to copy yet, and go directly where 1f would direct us.
        cmp     al, '"'
        jz      .enter_quoted_mode
        cmp     al, '\'
        jz      .slash_normal
.normal_mode:
        inc     ecx
.enter_normal_mode:
        mov     al, byte [esi]
        inc     esi
.reenter_normal_mode:
        cmp     al, ' '
        jz      .copydata
        cmp     al, 9
        jz      .copydata
        test    al, al
        jz      .copydata
        cmp     al, '\'
        jz      .copydata
        cmp     al, '"'
        jnz     .normal_mode
.copydata:
; 1e. Copy the found chunk.
        copy_arg_data
; 1f. One of (space), (tab), (terminator), ", \ is found.
; For terminator, end the current argument and exit.
; For \, go to 1h.
; For ", switch to quoted mode.
        test    al, al
        jz      .done_termarg
        cmp     al, '\'
        jz      .slash_normal
        cmp     al, '"'
        jz      .enter_quoted_mode
; 1g. If we are here, (space) or (tab) has occured in 1d.
; End the current argument and restart processing from 1a.
        test    edi, edi
        jz      .skip_spaces
        mov     byte [edi], 0
        inc     edi
        jmp     .skip_spaces
.done_termarg:
        test    edi, edi
        jz      .done
        mov     byte [edi], 0
        inc     edi
.done:
        ret
.slash_normal:
; 1h. Process chunk of slashes with possible ending " if escaped
; as described in process_slash macros.
; After that, return to loop in 1d; note that the next character can be space.
        process_slash
        jmp     .reenter_normal_mode
; 2. Processing in quoted mode.
; This block is simpler because the current argument never ends in quoted mode,
; except when the input ends.
; 2a. Loop over the input string, watching for one of:
; (terminator), ", \.
.quoted_mode:
        inc     ecx
.enter_quoted_mode:
        mov     al, byte [esi]
        inc     esi
.reenter_quoted_mode:
        test    al, al
        jz      .copydata2
        cmp     al, '\'
        jz      .copydata2
        cmp     al, '"'
        jnz     .quoted_mode
.copydata2:
; 2b. Copy the found chunk.
        copy_arg_data
; 2c. One of (terminator), ", \ is found.
; For terminator, end the current argument and exit.
; For \, go to 2d.
        test    al, al
        jz      .done_termarg
        cmp     al, '\'
        jz      .slash_quoted
; For ", check whether the next character is also ":
; for a single quote, switch to the normal mode 1d,
; for a double quote, skip the first quote
; and start a new chunk from the second one.
        cmp     byte [esi], '"'
        jnz     .enter_normal_mode
.double_quote:
        inc     esi
        jmp     .quoted_mode
.slash_quoted:
; 2d. Process chunk of slashes with possible ending " if escaped
; as described in process_slash macros.
; After that, return to loop in 2a.
        process_slash
        jmp     .reenter_quoted_mode
endp
purge start_arg
purge copy_arg_data
purge process_slash

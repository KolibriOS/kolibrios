;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;    FTPS
;    FTP Server
;
;    Compile with FASM for Menuet
;

; note: telnet == 23, ftp cmd == 21, data on 20

use32

    org     0x0

    db      'MENUET01'              ; 8 byte id
    dd      1                       ; header version
    dd      START                   ; program start
    dd      I_END                   ; program image size
    dd      0x170000                ; required amount of memory
    dd      0x7FFF0                 ; esp = 0x7FFF0
    dd      0, 0                    ; no params, no path

include 'macros.inc'
; Various states of client connection
USER_NONE       equ 0   ; Awaiting a connection
USER_CONNECTED  equ 1   ; User just connected, prompt given
USER_USERNAME   equ 2   ; User given username
USER_LOGGED_IN  equ 3   ; User given password





START:                          ; start of execution
    ; Clear the screen memory
    mov     eax, '    '
    mov     edi,text
    mov     ecx,80*30 /4
    cld
    rep     stosd

    call    draw_window

    ; init the receive buffer pointer
    mov     [buffptr], buff

    ; Init FTP server state machine
    mov     [state], USER_NONE

    ; Open the listening socket
    call    connect

still:
    ; check connection status
    mov     eax,53
    mov     ebx,6               ; Get socket status
    mov     ecx,[CmdSocket]
    mcall

    mov     ebx, [CmdSocketStatus]
    mov     [CmdSocketStatus], eax

    cmp     eax, ebx
    je      waitev

    ; If the socket closed by remote host, open it again.
    cmp     eax, 7
    je      con

    ; If socket closed by Reset, open it again
    cmp     eax, 11
    je      con

    ; If a user has just connected, start by outputting welcome msg
    cmp     eax, 4
    jne     noc

    mov     esi, loginStr0
    mov     edx, loginStr0_end - loginStr0
    call    outputStr

    mov     [state], USER_CONNECTED
    jmp     noc


con:
    ; Need to call disconnect, since a remote close does not fully
    ; close the socket
    call    disconnect
    mov     eax,5
    mov     ebx,10          ; Delay for 100ms
    mcall
    call    connect
    jmp     noc

noc:
    ; Display the changed connected status
    call    draw_window

waitev:
    mov     eax,23                 ; wait here for event
    mov     ebx,1                 ; Delay for up to 1s
    mcall

    cmp     eax,1                  ; redraw request ?
    je      red
    cmp     eax,2                  ; key in buffer ?
    je      key
    cmp     eax,3                  ; button in buffer ?
    je      button

    ; any data from the socket?

    mov     eax, 53
    mov     ebx, 2                  ; Get # of bytes in input queue
    mov     ecx, [CmdSocket]
    mcall
    test    eax, eax
    jz      still

read_input:
    mov     eax, 53
    mov     ebx, 3                  ; Get a byte from socket in bl
    mov     ecx, [CmdSocket]
    mcall

    call    ftpRxCmdData            ; process incoming ftp command

    ; Keep processing data until there is no more to process
    mov     eax, 53
    mov     ebx, 2                  ; Get # of bytes in input queue
    mov     ecx, [CmdSocket]
    mcall
    cmp     eax, 0
    jne     read_input

    ; Now redraw the text text field.
    ; Probably not required, since ftp requires no
    ; console i/o.
    ; Leave in for now, for debugging.
; (fall through to "red:")
;    call    draw_text
;    jmp     still

red:                          ; REDRAW WINDOW
    call    draw_window
    jmp     still

key:                          ; KEY
    mov     eax,2                  ; get but ignore
    mcall
    jmp     still

button:
    mov     eax,17
    mcall
    cmp     ah,1
    jne     still

    ; Exit button pressed, so close socket and quit
    mov     eax,53
    mov     ebx,8
    mov     ecx,[CmdSocket]
    mcall

    ; ... terminate program
    or     eax,-1
    mcall
    jmp     still



;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************
draw_window:
    pusha

    mov  eax,12
    mov  ebx,1
    mcall

    xor  eax,eax                   ; DRAW WINDOW
    mov  ebx,100*65536+491 + 8 +15
    mov  ecx,100*65536+270 + 20     ; 20 for status bar
    mov  edx,0x14000000
    mov  edi,labelt
    mcall

    ; draw status bar
    mov     eax, 13
    mov     ebx, 4*65536+484 + 8 +15
    mov     ecx, 270*65536 + 3
    mov     edx, 0x00557799
    mcall


    mov  esi,contlen-contt          ; display connected status
    mov     edx, contt
    cmp     [CmdSocketStatus], 4    ; 4 is connected
    je      pcon
    mov     esi,discontlen-discontt
    mov     edx, discontt
pcon:

    mov  eax,4                      ; status text
    mov  ebx,380*65536+276
    mov  ecx,0x00ffffff
    mcall

    ; Draw the text on the screen, clearing it first
    ; This can go when we loose debuggin info.
    xor  eax,eax
    mov  edi,text+80*30
    mov  ecx,80*30 /4
    cld
    rep  stosd

    call draw_text

    mov  eax,12
    mov  ebx,2
    mcall

    popa

    ret


;***************************************************************************
;   Function
;      draw_text
;
;   Description
;       Updates the text on the screen. This is part of the debugging code
;
;   Inputs
;       Character to add in bl
;
;***************************************************************************
draw_text:

    pusha

    mov  esi,text
    mov  eax,0
    mov  ebx,0
  newletter:
    mov  cl,[esi]
    cmp  cl,[esi+30*80]
    jz   noletter
  yesletter:
    mov  [esi+30*80],cl

    ; erase character

    pusha
    mov     edx, 0                  ; bg colour
    mov     ecx, ebx
    add     ecx, 26
    shl     ecx, 16
    mov     cx, 9
    mov     ebx, eax
    add     ebx, 6
    shl     ebx, 16
    mov     bx, 6
    mov     eax, 13
    mcall
    popa

    ; draw character

    pusha
    mov     ecx, 0x00ffffff
    push bx
    mov  ebx,eax
    add  ebx,6
    shl  ebx,16
    pop  bx
    add  bx,26
    mov  eax,4
    mov  edx,esi
    mov  esi,1
    mcall
    popa

  noletter:

    add  esi,1
    add  eax,6
    cmp  eax,80*6
    jb   newletter
    mov  eax,0
    add  ebx,10
    cmp  ebx,24*10
    jb   newletter

    popa
    ret



;***************************************************************************
;   Function
;      ftpRxCmdData
;
;   Description
;       Prcoesses incoming command data, calling a handler for each command.
;       Commands are built up in buff before being processed.
;
;   Inputs
;       Character to add in bl
;
;***************************************************************************
ftpRxCmdData:
    ; Quit if we are not connected
    ;( This case shouldn't be necessary, but be safe )
    mov     al, [state]
    cmp     al, USER_NONE
    je      frcd_exit

    ; Store the incoming character
    mov     esi, [buffptr]
    mov     [esi], bl
    inc     esi
    mov     [buffptr], esi

    ; For debugging, show the data coming in
    pusha
    call    printChar
    popa

    ; Do we have an end of line? (LF)
    ; if not, just exit
    cmp     bl, 0x0a
    jne     frcd_exit

    ; OK we have a complete command.
    ; Process, and send response

    ; There are a number of states involved in ftp,
    ; to do with logging in.

    mov     al, [state]
    cmp     al, USER_CONNECTED
    jne     fs001

    ; This should be the username

    ; TODO validate username

    ; OK, username accepted - ask for password
    mov     esi, loginStr1
    mov     edx, loginStr1_end - loginStr1
    call    outputStr

    mov     [state], USER_USERNAME

    ; init the receive buffer pointer
    mov     [buffptr], buff

    jmp     frcd_exit

fs001:
    cmp     al, USER_USERNAME
    jne     fs002

    ; This should be the password

    ; TODO validate password

    ; OK, password accepted - show they are logged in
    mov     esi, loginStr2
    mov     edx, loginStr2_end - loginStr2
    call    outputStr

    mov     [state], USER_LOGGED_IN

    ; init the receive buffer pointer
    mov     [buffptr], buff

    jmp     frcd_exit

fs002:
    cmp     al, USER_LOGGED_IN
    jne     fs003

    ; This should be a cmd
    call    findCmd
    mov     eax, [cmdPtr]
    cmp     eax, 0

    je      fs002b

    call    [cmdPtr]

fs002a:
    ; init the receive buffer pointer
    mov     [buffptr], buff

    jmp     frcd_exit

fs002b:
    ; an unsupported command was entered.
    ; Tell user that the command is not supported

    mov     esi, unsupStr
    mov     edx, unsupStr_end - unsupStr
    call    outputStr

    jmp     fs002a

fs003:
frcd_exit:
    ret



;***************************************************************************
;   Function
;      outputStr
;
;   Description
;       Sends a string over the 'Command' socket
;
;   Inputs
;       String in esi
;       Length in edx
;
;***************************************************************************
outputStr:
    push    esi
    push    edx
    mov     eax,53
    mov     ebx,7
    mov     ecx,[CmdSocket]
    mcall
    pop     edx
    pop     esi

    cmp     eax, 0
    je      os_exit

    ; The TCP/IP transmit queue is full; Wait a bit, then retry
    pusha
    mov     eax,5
    mov     ebx,1                 ; Delay for up 100ms
    mcall
    popa
    jmp     outputStr
os_exit:
    ret



;***************************************************************************
;   Function
;      outputDataStr
;
;   Description
;       Sends a string over the 'Data' socket
;
;   Inputs
;       String in esi
;       Length in edx
;
;***************************************************************************
outputDataStr:
    push    esi
    push    edx
    mov     eax,53
    mov     ebx,7
    mov     ecx,[DataSocket]
    mcall
    pop     edx
    pop     esi

    cmp     eax, 0
    je      ods_exit

    ; The TCP/IP transmit queue is full; Wait a bit, then retry
    pusha
    mov     eax,5
    mov     ebx,20           ; Delay for upto 200ms
    mcall
    popa
    jmp     outputDataStr
ods_exit:
    ret



;***************************************************************************
;   Function
;      printChar
;
;   Description
;       Writes a character to the screen; Used to display the data coming
;       in from the user. Really only useful for debugging.
;
;   Inputs
;       Character in bl
;
;***************************************************************************
printChar:
    cmp     bl,13                          ; BEGINNING OF LINE
    jne     nobol
    mov     ecx,[pos]
    add     ecx,1
boll1:
    sub     ecx,1
    mov     eax,ecx
    xor     edx,edx
    mov     ebx,80
    div     ebx
    cmp     edx,0
    jne     boll1
    mov     [pos],ecx
    jmp     newdata
nobol:

    cmp     bl,10                            ; LINE DOWN
    jne     nolf
addx1:
    add     [pos],dword 1
    mov     eax,[pos]
    xor     edx,edx
    mov     ecx,80
    div     ecx
    cmp     edx,0
    jnz     addx1
    mov     eax,[pos]
    jmp     cm1
nolf:

    cmp     bl,8                            ; BACKSPACE
    jne     nobasp
    mov     eax,[pos]
    dec     eax
    mov     [pos],eax
    mov     [eax+text],byte 32
    mov     [eax+text+60*80],byte 0
    jmp     newdata
nobasp:

    cmp     bl,15                           ; CHARACTER
    jbe     newdata
putcha:
    mov     eax,[pos]
    mov     [eax+text],bl
    mov     eax,[pos]
    add     eax,1
cm1:
    mov     ebx,[scroll+4]
    imul    ebx,80
    cmp     eax,ebx
    jb      noeaxz
    mov     esi,text+80
    mov     edi,text
    mov     ecx,ebx
    cld
    rep     movsb
    mov     eax,ebx
    sub     eax,80
noeaxz:
    mov     [pos],eax
newdata:
    ret


;***************************************************************************
;   Function
;      disconnect
;
;   Description
;       Closes the command socket
;
;   Inputs
;       None
;
;***************************************************************************
disconnect:
    mov     eax, 53         ; Stack Interface
    mov     ebx,8           ; Close TCP socket
    mov     ecx,[CmdSocket]
    mcall
    ret



;***************************************************************************
;   Function
;      disconnectData
;
;   Description
;       Closes the data socket
;
;   Inputs
;       None
;
;***************************************************************************
disconnectData:
    ; This delay would be better done by allowing the socket code
    ; to wait for all data to pass through the stack before closing
    pusha
    mov     eax,5
    mov     ebx,10          ; Delay for 100ms
    mcall
    popa

    mov     eax, 53         ; Stack Interface
    mov     ebx,8           ; Close TCP socket
    mov     ecx,[DataSocket]
    mcall
    ret




;***************************************************************************
;   Function
;      connect
;
;   Description
;       Opens the command socket
;
;   Inputs
;       None
;
;***************************************************************************
connect:
    pusha

    mov     eax, 53     ; Stack Interface
    mov     ebx, 5      ; Open TCP socket
    mov     esi, 0      ; No remote IP address
    mov     edx, 0      ; No remote port
    mov     ecx, 21     ; ftp command port id
    mov     edi, 0      ; passive open
    mcall
    mov     [CmdSocket], eax

    popa

    ret



;***************************************************************************
;   Function
;      connectData
;
;   Description
;       Opens the data socket
;
;   Inputs
;       None
;
;***************************************************************************
connectData:
    pusha

    mov     eax, 53     ; Stack Interface
    mov     ebx, 5      ; Open TCP socket
    mov     esi, [DataIP]      ; remote IP address
    mov     edx, [DataPort]    ; remote port
    mov     ecx, 0      ; ftp data port id
    mov     edi, 1      ; active open
    mcall
    mov     [DataSocket], eax

    popa

    ret




;***************************************************************************
;   Function
;      findCmd
;
;   Description
;       Scans the command string for a valid command. The command string
;       is in the global variable buff.
;
;       Returns result in cmdPtr. This will be zero if none found
;
;   Inputs
;       None
;
;***************************************************************************
findCmd:
    ; Setup to return 'none' in cmdPtr, if we find no cmd
    mov     eax, 0
    mov     [cmdPtr], eax
    cld
    mov     esi, buff
    mov     edi, CMDList

fc000:
    cmp     [edi], byte 0   ; Are we at the end of the CMDList?
    je      fc_exit

fc000a:
    cmpsb

    je      fc_nextbyte

    ; Command is different - move to the next entry in cmd table
    mov     esi, buff

fc001:
    ; skip to the next command in the list
    cmp     [edi], byte 0
    je      fc002
    inc     edi
    jmp     fc001
fc002:
    add     edi, 5
    jmp     fc000

fc_nextbyte:
    ; Have we reached the end of the CMD text?
    cmp     [edi], byte 0
    je      fc_got      ; Yes - so we have a match
    jmp     fc000a      ; No - loop back

fc_got:
    ; Copy the function pointer for the selected command
    inc     edi
    mov     eax, [edi]
    mov     [cmdPtr], eax

fc_exit:
    ret



;***************************************************************************
;   Function
;      decStr2Byte
;
;   Description
;       Converts the decimal string pointed to by esi to a byte
;
;   Inputs
;       string ptr in esi
;
;   Outputs
;       esi points to next character not in string
;       eax holds result ( 0..255)
;
;***************************************************************************
decStr2Byte:
    xor     eax, eax
    xor     ebx, ebx
    mov     ecx, 3

dsb001:
    mov     bl, [esi]

    cmp     bl, '0'
    jb      dsb_exit
    cmp     bl, '9'
    ja      dsb_exit

    imul    eax, 10
    add     eax, ebx
    sub     eax, '0'
    inc     esi
    loop    dsb001

dsb_exit:
    ret



;***************************************************************************
;   Function
;      parsePortStr
;
;   Description
;       Converts the parameters of the PORT command, and stores them in the
;       appropriate variables.
;
;   Inputs
;       None ( string in global variable buff )
;
;   Outputs
;       None
;
;***************************************************************************
parsePortStr:
    ; skip past the PORT text to get the the parameters. The command format
    ; is
    ; PORT i,i,i,i,p,p,0x0d,0x0a
    ; where i and p are decimal byte values, high byte first.
    xor     eax, eax
    mov     [DataIP], eax
    mov     [DataPort], eax
    mov     esi, buff + 4       ; Start at first space character

pps001:
    cmp     [esi], byte ' '     ; Look for first non space character
    jne     pps002
    inc     esi
    jmp     pps001

pps002:
    call    decStr2Byte
    add     [DataIP], eax
    ror     dword [DataIP], 8
    inc     esi
    call    decStr2Byte
    add     [DataIP], eax
    ror     dword [DataIP], 8
    inc     esi
    call    decStr2Byte
    add     [DataIP], eax
    ror     dword [DataIP], 8
    inc     esi
    call    decStr2Byte
    add     [DataIP], eax
    ror     dword [DataIP], 8
    inc     esi
    call    decStr2Byte
    add     [DataPort], eax
    shl     [DataPort], 8
    inc     esi
    call    decStr2Byte
    add     [DataPort], eax

    ret



;***************************************************************************
;   Function
;      sendDir
;
;   Description
;       Transmits the directory listing over the data connection.
;       The data connection is already open.
;
;   Inputs
;       None
;
;   Outputs
;       None
;
;***************************************************************************
sendDir:
        mov     ebx, dirinfoblock
        and     dword [ebx+4], 0        ; start from zero block
sd001:
; Read the next DirBlocksPerCall (=16) blocks
        mov     eax, 70
        mcall
; Did we read anything?
        test    eax, eax
        jz      @f
        cmp     eax, 6
        jnz     sd_exit
@@:
        test    ebx, ebx
        jz      sd_exit
; Parse these blocks. There could be up to 16 files specified
        mov     esi, text + 0x1300 + 0x20
sd002:
        dec     ebx
        js      sd004
        push    ebx
; OK, lets parse the entry. Ignore volume entries
        test    byte [esi], 8
        jnz     sd003
; Valid file or directory. Start to compose the string we will send
        mov     edi, dirStr
; If we have been called as a result of an NLST command, we only display
; the filename
        cmp     [buff], byte 'N'
        jz      sd006

        mov     [edi], byte '-'
        test    byte [esi], 10h
        jz      @f
        mov     [edi], byte 'd'
@@:
; Ok, now copy across the directory listing text that will be constant
; ( I dont bother looking at the read only or archive bits )

        mov     ebx, tmplStr
@@:
        inc     edi
        mov     al, [ebx]
        test    al, al
        jz      @f
        mov     [edi], al
        inc     ebx
        jmp     @b
@@:
; point to the last character of the string;
; We will write the file size here, backwards
        push    edi             ; Remember where the string ends

        dec     edi
; eax holds the number
        mov     eax, [esi+32]
        mov     ebx, 10
@@:
        xor     edx, edx
        div     ebx
        add     dl, '0'
        mov     [edi], dl
        dec     edi
        test    eax, eax
        jnz     @b

        pop     edi
; now create the time & date fields
;timeStr:            db ' Jan 1    2000 ',0
        mov     al, ' '
        stosb
        movzx   eax, byte [esi+28+1]
        mov     eax, dword [months + (eax-1)*4]
        stosd
        mov     al, byte [esi+28]
        aam
        test    ah, ah
        jz      sd005a
        xchg    al, ah
        add     ax, '00'
        jmp     sd005b
sd005a:
        add     al, '0'
        mov     ah, ' '
sd005b:
        stosw
        mov     al, ' '
        mov     ecx, 6
        rep     stosb
        push    edi
        movzx   eax, word [esi+28+2]
@@:
        xor     edx, edx
        div     ebx
        add     dl, '0'
        mov     [edi], dl
        dec     edi
        test    eax, eax
        jnz     @b
        pop     edi
        inc     edi
        mov     al, ' '
        stosb
sd006:
; ** End of copying

; now copy the filename across
        lea     ebx, [esi+40]
@@:
        mov     al, [ebx]
        inc     ebx
        test    al, al
        jz      @f
        stosb
        jmp     @b
@@:
terminate:
; Now terminate the line by putting CRLF sequence in
        mov     al, 0x0D
        stosb
        mov     al, 0x0A
        stosb
; Send the completed line to the user over data socket
        push    esi
        mov     esi, dirStr
        mov     edx, edi
        sub     edx, esi
        call    outputDataStr
        pop     esi

sd003:                  ; Move to next entry in the block
        pop     ebx
        add     esi, 304
        jmp     sd002
sd004:
        mov     ebx, dirinfoblock
        add     dword [ebx+4], DirBlocksPerCall
        jmp     sd001

sd_exit:
        ret





;***************************************************************************
;   Function
;      setupFilePath
;
;   Description
;       Copies the file name from the input request string into the
;       file descriptor
;
;   Inputs
;       None
;
;   Outputs
;       None
;
;***************************************************************************
setupFilePath:
    mov     esi, buff + 4       ; Point to (1 before) first character of file

    ; Skip any trailing spaces or / character
sfp001:
    inc     esi
    cmp     [esi], byte ' '
    je      sfp001
    cmp     [esi], byte '/'
    je      sfp001

    ; esi points to start of filename.


    ; Copy across the directory path '/'
    ; into the fileinfoblock
    mov     edi, filename
    mov     dword [edi], '/RD/'
    mov     word [edi+4], '1/'
    add     edi, 6

    ; Copy across the filename
sfp002:
    cld
    movsb
    cmp     [esi], byte 0x0d
    jne     sfp002
    mov     [edi], byte 0
    ret




;***************************************************************************
;   Function
;      sendFile
;
;   Description
;       Transmits the requested file over the open data socket
;       The file to send is named in the buff string
;
;   Inputs
;       None
;
;   Outputs
;       None
;
;***************************************************************************
sendFile:
    call    setupFilePath

    ; init fileblock descriptor, for file read
    mov     ebx, fileinfoblock
    and     dword [ebx], 0 ; read cmd
    and     dword [ebx+4], 0 ; first block

sf002a:
    ; now read the file..
    mov     eax,70
    mcall
    test    eax, eax
    jz      @f
    cmp     eax, 6
    jnz     sf_exit
@@:
    push    eax
    mov     esi, text + 0x1300
    mov     edx, ebx
    call    outputDataStr
    pop     eax
    test    eax, eax
    jnz     sf_exit
; wait a bit
    mov     eax, 5
    mov     ebx, 10
    mcall
    mov     ebx, fileinfoblock
    add     dword [ebx+4], edx
    jmp     sf002a

sf_exit:
    ret


;***************************************************************************
;   Function
;      getFile
;
;   Description
;       Receives the specified file over the open data socket
;       The file to receive is named in the buff string
;
;   Inputs
;       None
;
;   Outputs
;       None
;
;***************************************************************************
getFile:
    call    setupFilePath

    ; init fileblock descriptor, for file write
    xor     eax, eax
    mov     [fsize], eax            ; Start filelength at 0
    mov     [fileinfoblock+4], eax    ; set to 0
    inc     eax
    inc     eax
    mov     [fileinfoblock], eax    ; write cmd

    ; Read data from the socket until the socket closes
    ; loop
    ;   loop
    ;     read byte from socket
    ;     write byte to file buffer
    ;   until no more bytes in socket
    ;   sleep 100ms
    ; until socket no longer connected
    ; write file to ram

gf000:
    mov     eax, 53
    mov     ebx, 2                  ; Get # of bytes in input queue
    mov     ecx, [DataSocket]
    mcall
    test    eax, eax
    je      gf_sleep

    mov     eax, 53
    mov     ebx, 3                  ; Get a byte from socket in bl
    mov     ecx, [DataSocket]
    mcall                    ; returned data in bl

    mov     esi, text + 0x1300
    add     esi, dword [fsize]
    mov     [esi], bl
    inc     dword [fsize]

    ; dummy, write to screen
    ;call    printChar

    jmp     gf000

gf_sleep:
    ; Check to see if socket closed...
    mov     eax,53
    mov     ebx,6               ; Get socket status
    mov     ecx,[DataSocket]
    mcall

    cmp     eax, 7
    jne     gf001               ; still open, so just sleep a bit

    ; Finished, so write the file
    mov     eax, [fsize]
    mov     [fileinfoblock+12], eax
    mov     eax,70
    mov     ebx,fileinfoblock
    mcall

    ret                         ; Finished

gf001:
    ; wait a bit
    mov     eax,5
    mov     ebx,10              ; Delay for up 100ms
    mcall
    jmp     gf000               ; try for more data





;***************************************************************************
;   COMMAND HANDLERS FOLLOW
;
;   These handlers implement the functionality of each supported FTP Command
;
;***************************************************************************

cmdPWD:
    ; OK, show the directory name text
    mov     esi, ramdir
    mov     edx, ramdir_end - ramdir
    call    outputStr

    ; TODO give real directory

    ret


cmdCWD:
    ; Only / is valid for the ramdisk
    cmp     [buff+5], byte 0x0d
    jne     ccwd_000

    ; OK, show the directory name text
    mov     esi, chdir
    mov     edx, chdir_end - chdir
    jmp     ccwd_001

ccwd_000:
    ; Tell user there is no such directory
    mov     esi, noFileStr
    mov     edx, noFileStr_end - noFileStr

ccwd_001:
    call    outputStr

    ret


cmdQUIT:
    ; The remote end will do the close; We just
    ; say goodbye.
    mov     esi, byeStr
    mov     edx, byeStr_end - byeStr
    call    outputStr
    ret


cmdABOR:

    ; Close port
    call    disconnectData

    mov     esi, abortStr
    mov     edx, abortStr_end - abortStr
    call    outputStr

    ret

cmdPORT:
    ; TODO
    ; Copy the IP and port values to DataIP and DataPort

    call    parsePortStr

    ; Indicate the command was accepted
    mov     esi, cmdOKStr
    mov     edx, cmdOKStr_end - cmdOKStr
    call    outputStr
    ret

cmdnoop:
    ; Indicate the command was accepted
    mov     esi, cmdOKStr
    mov     edx, cmdOKStr_end - cmdOKStr
    call    outputStr
    ret


cmdTYPE:
    ; TODO
    ; Note the type field selected - reject if needed.

    ; Indicate the command was accepted
    mov     esi, cmdOKStr
    mov     edx, cmdOKStr_end - cmdOKStr
    call    outputStr
    ret

cmdsyst:
    ; Indicate the system type
    mov     esi, systStr
    mov     edx, systStr_end - systStr
    call    outputStr
    ret


cmdDELE:
    call    setupFilePath

    mov     ebx, fileinfoblock
    mov     dword [ebx], 8
    and     dword [ebx+4], 0
    push    dword [ebx+12]
    push    dword [ebx+16]
    and     dword [ebx+12], 0
    and     dword [ebx+16], 0
    mov     eax, 70
    mcall
    pop     dword [ebx+16]
    pop     dword [ebx+12]

    test    eax, eax
    jne     cmdDele_err

    mov     esi, delokStr
    mov     edx, delokStr_end - delokStr
    call    outputStr

    jmp     cmdDele_exit

cmdDele_err:
    mov     esi, noFileStr
    mov     edx, noFileStr_end - noFileStr
    call    outputStr


cmdDele_exit:
    ret


cmdNLST:
cmdLIST:
    ; Indicate the command was accepted
    mov     esi, startStr
    mov     edx, startStr_end - startStr
    call    outputStr

    call    connectData

    ; Wait for socket to establish

cl001:
    ; wait a bit
    mov     eax,5
    mov     ebx,10                ; Delay for up 100ms
    mcall

    ; check connection status
    mov     eax,53
    mov     ebx,6               ; Get socket status
    mov     ecx,[DataSocket]
    mcall

    cmp     eax, 4
    jne     cl001

    ; send directory listing
    call    sendDir

    ; Close port
    call    disconnectData

    mov     esi, endStr
    mov     edx, endStr_end - endStr
    call    outputStr
    ret

cmdRETR:
    ; Indicate the command was accepted
    mov     esi, startStr
    mov     edx, startStr_end - startStr
    call    outputStr

    call    connectData

    ; Wait for socket to establish

cr001:
    ; wait a bit
    mov     eax,5
    mov     ebx,10                ; Delay for up 100ms
    mcall

    ; check connection status
    mov     eax,53
    mov     ebx,6               ; Get socket status
    mov     ecx,[DataSocket]
    mcall

    cmp     eax, 4
    jne     cr001

    ; send data to remote user
    call    sendFile

    ; Close port
    call    disconnectData

    mov     esi, endStr
    mov     edx, endStr_end - endStr
    call    outputStr


    ret


cmdSTOR:
    ; Indicate the command was accepted
    mov     esi, storStr
    mov     edx, storStr_end - storStr
    call    outputStr

    call    connectData

    ; Wait for socket to establish

cs001:
    ; wait a bit
    mov     eax,5
    mov     ebx,10                ; Delay for up 100ms
    mcall

    ; check connection status
    mov     eax,53
    mov     ebx,6               ; Get socket status
    mov     ecx,[DataSocket]
    mcall

    cmp     eax, 4
    je      @f
    cmp     eax, 7
    jne     cs001
@@:

    ; get data file from remote user
    call    getFile

    mov     esi, endStr
    mov     edx, endStr_end - endStr
    call    outputStr

    ; Close port
    call    disconnectData

    ret



; DATA AREA

; This is the list of supported commands, and the function to call
; The list end with a NULL.
CMDList:
                    db  'pwd',0
                    dd  cmdPWD

                    db  'PWD',0
                    dd  cmdPWD

                    db  'XPWD',0
                    dd  cmdPWD

                    db  'xpwd',0
                    dd  cmdPWD

                    db  'QUIT',0
                    dd  cmdQUIT

                    db  'quit',0
                    dd  cmdQUIT

                    db  'PORT',0
                    dd  cmdPORT

                    db  'port',0
                    dd  cmdPORT

                    db  'LIST',0
                    dd  cmdLIST

                    db  'list',0
                    dd  cmdLIST

                    db  'NLST',0
                    dd  cmdNLST

                    db  'nlst',0
                    dd  cmdNLST

                    db  'TYPE',0
                    dd  cmdTYPE

                    db  'type',0
                    dd  cmdTYPE

                    db  'syst',0
                    dd  cmdsyst

                    db  'noop',0
                    dd  cmdnoop

                    db  'CWD',0
                    dd  cmdCWD

                    db  'cwd',0
                    dd  cmdCWD

                    db  'RETR',0
                    dd  cmdRETR

                    db  'retr',0
                    dd  cmdRETR

                    db  'DELE',0
                    dd  cmdDELE

                    db  'dele',0
                    dd  cmdDELE

                    db  'stor',0
                    dd  cmdSTOR

                    db  'STOR',0
                    dd  cmdSTOR

                    db  'ABOR',0
                    dd  cmdABOR

                    db  'abor',0
                    dd  cmdABOR

                    db  0xff,0xf4,0xff,0xf2,'ABOR',0
                    dd  cmdABOR

                    db  0


cmdPtr              dd  0
CmdSocket           dd  0x0
CmdSocketStatus     dd  0x0
DataSocket          dd  0x0
DataSocketStatus    dd  0x0
DataPort            dd  0x00
DataIP              dd  0x00
pos                 dd  80 * 1
scroll              dd  1
                    dd  24

labelt              db  'FTP Server v0.1',0
contt               db  'Connected'
contlen:
discontt            db  'Disconnected'
discontlen:

cmdOKStr:           db  '200 Command OK',0x0d,0x0a
cmdOKStr_end:

loginStr0:          db  '220-  Menuet FTP Server v0.1',0x0d,0x0a
                    db  '220 Username and Password required',0x0d,0x0a
loginStr0_end:

loginStr1:          db  '331 Password now required',0x0d,0x0a
loginStr1_end:

loginStr2:          db  '230 You are now logged in.',0x0d,0x0a
loginStr2_end:

byeStr:             db  '221 Bye bye!',0x0d,0x0a
byeStr_end:

systStr:            db  '215 UNIX system type',0x0d,0x0a
systStr_end:

ramdir:             db  '257 "/"',0x0d,0x0a
ramdir_end:

chdir:              db  '200 directory changed to /',0x0d,0x0a
chdir_end:

unsupStr:           db  '500 Unsupported command',0x0d,0x0a
unsupStr_end:

noFileStr:          db  '550 No such file',0x0d,0x0a
noFileStr_end:

delokStr:           db  '250 DELE command successful',0x0d,0x0a
delokStr_end:

startStr:           db  '150 Here it comes...',0x0d,0x0a
startStr_end:

storStr:            db  '150 Connecting for STOR',0x0d,0x0a
storStr_end:

endStr:             db  '226 Transfer OK, Closing connection',0x0d,0x0a
endStr_end:

abortStr:           db  '225 Abort successful',0x0d,0x0a
abortStr_end:

 ; This is the buffer used for building up a directory listing line
dirStr:             times 128 db 0

; This is template string used in building up a directory listing line
tmplStr:            db 'rw-rw-rw-    1 0        0                ',0

months:
        dd     'Jan ','Feb ','Mar ','Apr ','May ','Jun '
        dd     'Jul ','Aug ','Sep ','Oct ','Nov ','Dec '

fileinfoblock:
                    dd      0x00
                    dd      0x00
                    dd      0x00
                    dd      0x200         ; bytes to read
                    dd      text + 0x1300 ; data area
filename:           times 256 db 0

; The following lines define data for reading a directory block
DirBlocksPerCall = 16
dirinfoblock:
                    dd      1
                    dd      0x00
                    dd      0x00
                    dd      DirBlocksPerCall
                    dd      text + 0x1300   ; data area
; The 'filename' for a directory listing
dirpath             db      '/sys',0

fsize:              dd      0

state               db  0
buffptr             dd  0
buff:               times 256 db 0  ; Could put this after iend

; Ram use at the end of the application:
; text                  : 2400 bytes for screen memory
; text + 0x1300          : file data area
text:
I_END:

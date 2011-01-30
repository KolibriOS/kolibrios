use32
org	0
db	'MENUET01'
dd	1
dd	_start
dd	_end
dd	_memory
dd	_stack
dd	_param
dd	0

_start:

	call lib_console_init
	push    caption
	push    -1
	push    -1
	push    -1
	push    -1
	call    [con_init]

 ;----------------------------------------------------;
 ; Program code starts here                           ;
 ;----------------------------------------------------;
        mov   dl,1                                    ; init dl to 1, reply paging
        call  [Clstext]                               ; Clear screen
        mov   esi,line1                               ; Load intro screen line adress
        call  [PrintString]                           ; Print it
        call  [WaitForKeyPress]                       ; Key press ends intro
        call  [Clstext]                               ; Clear screen
        mov   ax,0111h                                ; Write location in Dex title block
        mov   esi,Program_name                        ; What to write
        call  [SetCursorPos]                          ; Put cursor there
        call  [PrintString]                           ; Write title
        mov   ax,0400h                                ; Write location to start chat
        call  [SetCursorPos]                          ; Set it
        mov   al,2                                    ; Eliza text color
        call  [TextColor]                             ; Set it
        mov   esi,Message                             ; Intro Message
        call  [PrintString]                           ; Print it
userinput:
        call  prtblk                                  ; Add a blank line
        mov   al,3                                    ; User text color
        call  [TextColor]                             ; Set it
        call  [GetUserInput]                          ; Get sentance from user
        call  prtblk                                  ; Add line after input
        call  [UpperCase]                             ; Make user input caps
        mov   al,2                                    ; Eliza text color
        call  [TextColor]                             ; Set it
phr:
        push  edi                                     ; Save user input pointer
        mov   ebx,keywords                            ; Set keyword pointer
phr1:
        mov   al,[edi]                                ; Get char from user input
        inc   edi                                     ; Set pointer to next char
        inc   ebx                                     ; Set keyword pointer to next char
        mov   ah,[ebx]                                ; Get letter to test from keywords
        and   ah,127                                  ; Strip out bit 7 marker
        cmp   al,ah                                   ; Do the letters match?
        je    phr2                                    ; Jump if they do
phr1a:
        cmp   byte[ebx],128                           ; Was that the last letter of keyword?
        jnb   phr1b                                   ; Jump if it was
        inc   ebx                                     ; increment key word pointer to next letter
        jmp   phr1a                                   ; Jump back to see if this is the last letter
phr1b:
        add   ebx,5                                   ; Add 5 to get to next word past execution address
        cmp   byte[ebx],128                           ; No more keywords?
        je    phr3                                    ; Jump to maybe inc user input
        dec   ebx                                     ; Correct for pre-increment
        pop   edi                                     ; Restore user input pointer
        push  edi                                     ; Save user input pointer
        jmp   phr1                                    ; Check next keyword for a match
phr2:
        cmp   byte[ebx],128                           ; Last letter of a keyword?
        jb    phr1                                    ; No check some more
        inc   ebx                                     ; ebx -> is the execution address
        pop   eax                                     ; Clear stored pointer from stack
        jmp   dword[ebx]                              ; Goto line stored with keyword
phr3:   pop   edi                                     ; Restore input pointer
        inc   edi                                     ; Move to next letter of user input
        cmp   byte[edi],0                             ; Nothing left to check?
        je    rep29                                   ; Go to sorry message
        jmp   phr                                     ; Try some more
rep1:
        mov   esi,rep1a                               ; Load reply
        cmp   dl,2                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep1b                               ; Load reply
        cmp   dl,4                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep1c                               ; Load reply
        cmp   dl,6                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep1b                               ; Load reply
        cmp   dl,8                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep1c                               ; Load reply
        jmp   printmore                               ; reply with user text
rep2:
        mov   esi,rep2a                               ; Load reply
        cmp   dl,2                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep2b                               ; Load reply
        cmp   dl,4                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep2a                               ; Load reply
        cmp   dl,6                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep2b                               ; Load reply
        cmp   dl,8                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep2a                               ; Load reply
        cmp   dl,9                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep2b                               ; Load reply
        jmp   printmore                               ; reply with user text
rep3:
        mov   esi,rep3a                               ; Load reply
        cmp   dl,2                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep3b                               ; Load reply
        cmp   dl,4                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep3c                               ; Load reply
        cmp   dl,6                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep3d                               ; Load reply
        cmp   dl,8                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep3b                               ; Load reply
        cmp   dl,9                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep3c                               ; Load reply
        jmp   printmore                               ; reply with user text
rep4:
        mov   esi,rep4a                               ; Load reply
        cmp   dl,2                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep4b                               ; Load reply
        cmp   dl,4                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep4c                               ; Load reply
        cmp   dl,6                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep4d                               ; Load reply
        cmp   dl,8                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep4b                               ; Load reply
        cmp   dl,9                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep4c                               ; Load reply
        jmp   printmore                               ; reply with user text
rep5:
        mov   esi,rep5a                               ; Load reply
        cmp   dl,2                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep5b                               ; Load reply
        cmp   dl,4                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep5c                               ; Load reply
        cmp   dl,6                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep5b                               ; Load reply
        cmp   dl,8                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep5c                               ; Load reply
        jmp   printmore                               ; reply with user text
rep6:
        mov   esi,rep6a                               ; Load reply
        cmp   dl,2                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep6b                               ; Load reply
        cmp   dl,4                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep6c                               ; Load reply
        cmp   dl,6                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep6b                               ; Load reply
        cmp   dl,8                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep6c                               ; Load reply
        jmp   printmore                               ; reply with user text
rep7:
        mov   esi,rep7a                               ; Load reply
        cmp   dl,2                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep7b                               ; Load reply
        cmp   dl,4                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep7a                               ; Load reply
        cmp   dl,6                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep7b                               ; Load reply
        cmp   dl,8                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep7a                               ; Load reply
        cmp   dl,9                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep7b                               ; Load reply
        jmp   printmore                               ; reply with user text
rep8:
        mov   esi,rep8a                               ; Load reply
        cmp   dl,2                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep8b                               ; Load reply
        cmp   dl,4                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep8c                               ; Load reply
        cmp   dl,6                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep8b                               ; Load reply
        cmp   dl,8                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep8c                               ; Load reply
        jmp   printmore                               ; reply with user text
rep9:
        mov   esi,rep9a                               ; Load reply
        cmp   dl,2                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep9b                               ; Load reply
        cmp   dl,4                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep9c                               ; Load reply
        cmp   dl,6                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep9b                               ; Load reply
        cmp   dl,8                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep9c                               ; Load reply
        jmp   printmore                               ; reply with user text
rep10:
        mov   esi,rep10a                              ; Load reply
        cmp   dl,2                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep10b                              ; Load reply
        cmp   dl,4                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep10c                              ; Load reply
        cmp   dl,6                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep10d                              ; Load reply
        cmp   dl,8                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep10c                              ; Load reply
        jmp   printmore                               ; reply with user text
rep11:
        mov   esi,rep11a                              ; Load reply
        cmp   dl,2                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep11b                              ; Load reply
        cmp   dl,4                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep11c                              ; Load reply
        cmp   dl,6                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep11b                              ; Load reply
        cmp   dl,8                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep11c                              ; Load reply
        jmp   printless                               ; reply with user text
rep12:
        mov   esi,rep12a                              ; Load reply
        cmp   dl,2                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep12b                              ; Load reply
        cmp   dl,4                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep12c                              ; Load reply
        cmp   dl,6                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep12d                              ; Load reply
        cmp   dl,8                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep12e                              ; Load reply
        jmp   printmore                               ; reply with user text
rep13:
        mov   esi,rep13a                              ; Load reply
        cmp   dl,1                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep13b                              ; Load reply
        cmp   dl,2                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep13c                              ; Load reply
        cmp   dl,3                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep13d                              ; Load reply
        cmp   dl,4                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep13e                              ; Load reply
        cmp   dl,5                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep13f                              ; Load reply
        cmp   dl,6                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep13g                              ; Load reply
        cmp   dl,7                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep13h                              ; Load reply
        cmp   dl,8                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep13i                              ; Load reply
        jmp   printless                               ; reply with user text
rep14:
        mov   esi,rep14a                              ; Load reply
        cmp   dl,2                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep14b                              ; Load reply
        cmp   dl,4                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep14a                              ; Load reply
        cmp   dl,6                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep14b                              ; Load reply
        cmp   dl,8                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep14a                              ; Load reply
        cmp   dl,9                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep14b                              ; Load reply
        jmp   printless                               ; reply with user text
rep15:
        mov   esi,rep15a                              ; Load reply
        cmp   dl,2                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep15b                              ; Load reply
        cmp   dl,4                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep15c                              ; Load reply
        cmp   dl,6                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep15d                              ; Load reply
        cmp   dl,8                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep15c                              ; Load reply
        jmp   printless                               ; reply with user text
rep16:
        mov   esi,rep16a                              ; Load reply
        cmp   dl,2                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep16b                              ; Load reply
        cmp   dl,4                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep16c                              ; Load reply
        cmp   dl,6                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep16d                              ; Load reply
        cmp   dl,8                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep16c                              ; Load reply
        jmp   printless                               ; reply with user text
rep17:
        mov   esi,rep17a                              ; Load reply
        cmp   dl,2                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep17b                              ; Load reply
        cmp   dl,4                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep17c                              ; Load reply
        cmp   dl,6                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep17d                              ; Load reply
        cmp   dl,8                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep17c                              ; Load reply
        jmp   printless                               ; reply with user text
rep18:
        mov   esi,rep18a                              ; Load reply
        jmp   printless                               ; print reply
rep19:
        mov   esi,rep19a                              ; Load reply
        cmp   dl,2                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep19b                              ; Load reply
        cmp   dl,4                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep19c                              ; Load reply
        cmp   dl,6                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep19d                              ; Load reply
        cmp   dl,8                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep19e                              ; Load reply
        jmp   printless                               ; reply with user text
rep20:
        mov   esi,rep20a                              ; Load reply
        cmp   dl,2                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep20b                              ; Load reply
        cmp   dl,4                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep20c                              ; Load reply
        cmp   dl,6                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep20d                              ; Load reply
        cmp   dl,8                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep20e                              ; Load reply
        jmp   printless                               ; reply with user text
rep21:
        mov   esi,rep21a                              ; Load reply
        cmp   dl,2                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep21b                              ; Load reply
        cmp   dl,4                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep21a                              ; Load reply
        cmp   dl,6                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep21b                              ; Load reply
        cmp   dl,8                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep21a                              ; Load reply
        cmp   dl,9                                    ; are we repeating our self
        jna   printmore                               ; reply with user text
        mov   esi,rep21b                              ; Load reply
        jmp   printmore                               ; reply with user text
rep22:
        mov   esi,rep22a                              ; Load reply
        cmp   dl,2                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep22b                              ; Load reply
        cmp   dl,4                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep22c                              ; Load reply
        cmp   dl,6                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep22d                              ; Load reply
        cmp   dl,8                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep22c                              ; Load reply
        jmp   printless                               ; reply with user text
rep23:
        mov   esi,rep23a                              ; Load reply
        cmp   dl,2                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep23b                              ; Load reply
        cmp   dl,4                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep23c                              ; Load reply
        cmp   dl,6                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep23b                              ; Load reply
        cmp   dl,8                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep23c                              ; Load reply
        jmp   printless                               ; reply with user text
rep24:
        mov   esi,rep24a                              ; Load reply
        cmp   dl,1                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep24b                              ; Load reply
        cmp   dl,2                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep24c                              ; Load reply
        cmp   dl,3                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep24d                              ; Load reply
        cmp   dl,4                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep24e                              ; Load reply
        cmp   dl,5                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep24f                              ; Load reply
        cmp   dl,6                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep24g                              ; Load reply
        cmp   dl,7                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep24c                              ; Load reply
        cmp   dl,8                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep24f                              ; Load reply
        jmp   printless                               ; reply with user text
rep25:
        mov   esi,rep25a                              ; Load reply
        cmp   dl,2                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep25b                              ; Load reply
        cmp   dl,4                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep25c                              ; Load reply
        cmp   dl,6                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep25b                              ; Load reply
        cmp   dl,8                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep25c                              ; Load reply
        jmp   printless                               ; reply with user text
rep26:
        mov   esi,rep26a                              ; Load reply
        cmp   dl,1                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep26b                              ; Load reply
        cmp   dl,2                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep26c                              ; Load reply
        cmp   dl,3                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep26d                              ; Load reply
        cmp   dl,4                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep26e                              ; Load reply
        cmp   dl,5                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep26f                              ; Load reply
        cmp   dl,6                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep26e                              ; Load reply
        cmp   dl,7                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep26c                              ; Load reply
        cmp   dl,8                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep26f                              ; Load reply
        jmp   printless                               ; reply with user text
rep27:
        mov   esi,rep27a                              ; Load reply
        cmp   dl,1                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep27b                              ; Load reply
        cmp   dl,2                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep27c                              ; Load reply
        cmp   dl,3                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep27d                              ; Load reply
        cmp   dl,4                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep27e                              ; Load reply
        cmp   dl,5                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep27f                              ; Load reply
        cmp   dl,6                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep27g                              ; Load reply
        cmp   dl,7                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep27c                              ; Load reply
        cmp   dl,8                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep27f                              ; Load reply
        jmp   printless                               ; reply with user text
rep28:
        mov   esi,rep28a                              ; Load reply
        cmp   dl,2                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep28b                              ; Load reply
        cmp   dl,4                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep28c                              ; Load reply
        cmp   dl,6                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep28d                              ; Load reply
        cmp   dl,8                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep28e                              ; Load reply
        jmp   printless                               ; reply with user tex

rep29:
        mov   esi,rep29a                              ; Load reply
        cmp   dl,1                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep29b                              ; Load reply
        cmp   dl,2                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep29c                              ; Load reply
        cmp   dl,3                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep29d                              ; Load reply
        cmp   dl,4                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep29e                              ; Load reply
        cmp   dl,5                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep29f                              ; Load reply
        cmp   dl,6                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep29g                              ; Load reply
        cmp   dl,7                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep29c                              ; Load reply
        cmp   dl,8                                    ; are we repeating our self
        jna   printless                               ; reply with user text
        mov   esi,rep29f                              ; Load reply
        jmp   printless                               ; reply with user text
rep30:
        mov   al,"A"
        call  [PrintChar]
        mov   al,"M"
        call  [PrintChar]
        jmp   printmore2
rep31:
        mov   al,"W"
        call  [PrintChar]
        mov   al,"A"
        call  [PrintChar]
        mov   al,"S"
        call  [PrintChar]
        jmp   printmore2
rep32:
        mov   al,"I"
        call  [PrintChar]
        jmp   printmore2
rep33:
        mov   al,"M"
        call  [PrintChar]
        mov   al,"Y"
        call  [PrintChar]
        jmp   printmore2
rep34:
        mov   al,"Y"
        call  [PrintChar]
        mov   al,"O"
        call  [PrintChar]
        mov   al,"U"
        call  [PrintChar]
        mov   al,"'"
        call  [PrintChar]
        mov   al,"V"
        call  [PrintChar]
        mov   al,"E"
        call  [PrintChar]
        jmp   printmore2
rep35:
        mov   al,"Y"
        call  [PrintChar]
        mov   al,"O"
        call  [PrintChar]
        mov   al,"U"
        call  [PrintChar]
        mov   al,"'"
        call  [PrintChar]
        mov   al,"R"
        call  [PrintChar]
        mov   al,"E"
        call  [PrintChar]
        jmp   printmore2
rep36:
        mov   al,"M"
        call  [PrintChar]
        mov   al,"E"
        call  [PrintChar]
        jmp   printmore2
rep37:
        jmp   printmore2
progmend:
        mov   esi,message2                            ; Load good bye message
        call  [PrintString]                           ; Print it
        mov   al,7
        call  [TextColor]                             ; Restore text color

	
	push	0
	call	[con_exit]
	
	mov	eax, -1
	int	0x40
        ret                                           ; Exit.
printmore:
        call [PrintString]                            ; Print rep phrase with rephrased user input
printmore2:
        push  edi                                     ; Save user input pointer
        mov   ebx,keyword2                            ; Set keyword pointer
rphr1:
        mov   al,[edi]                                ; Get char from user input
        inc   edi                                     ; Set pointer to next char
        inc   ebx                                     ; Set keyword pointer to next char
        mov   ah,[ebx]                                ; Get letter to test from keywords
        and   ah,127                                  ; Strip out bit 7 marker
        cmp   al,ah                                   ; Do the letters match?
        je    rphr2                                   ; Jump if they do
rphr1a:
        cmp   byte[ebx],128                           ; Was that the last letter of keyword?
        jnb   rphr1b                                  ; Jump if it was
        inc   ebx                                     ; increment key word pointer to next letter
        jmp   rphr1a                                  ; Jump back to see if this is the last letter
rphr1b:
        add   ebx,5                                   ; Add 5 to get to next word past execution address
        cmp   byte[ebx],128                           ; No more keywords?
        je    rphr3                                   ; Jump to maybe inc user input
        dec   ebx                                     ; Correct for pre-increment
        pop   edi                                     ; Restore user input pointer
        push  edi                                     ; Save user input pointer
        jmp   rphr1                                   ; Check next keyword for a match
rphr2:
        cmp   byte[ebx],128                           ; Last letter of a keyword?
        jb    rphr1                                   ; No check some more
        inc   ebx                                     ; ebx -> is the execution address
        pop   eax                                     ; Clear stored pointer from stack
        jmp   dword[ebx]                              ; Goto line stored with keyword
rphr3:
        pop   edi                                     ; Restore input pointer
        mov   al,[edi]                                ; load non key word letter
        call  [PrintCharCursor]                       ; Print it
        inc   edi                                     ; Move to next letter of user input
        cmp   byte[edi],0                             ; Nothing left to check?
        je    userinput                               ; Go to get another user input
        jmp   printmore2                              ; Try some more
printless:
        call  [PrintString]                           ; Print it
        call  prtblk                                  ; Add a blank line
        inc   dl                                      ; inc reply
        cmp   dl,10                                   ; is it to large
        jna   printless2                              ; jump if it's ok
        mov   dl,1                                    ; if not reset it to 1
printless2:
        jmp   userinput                               ; go back and get more user input
keywords:
        db   " "
        db   "CAN YO",213
        dd   rep1
        db   "CAN ",201
        dd   rep2
        db   "YOU AR",197
        dd   rep3
        db   "YOU'R",197
        dd   rep3
        db   "I DON'",212
        dd   rep4
        db   "I FEE",204
        dd   rep5
        db   "WHY DON'T YO",213
        dd   rep6
        db   "WHY CAN'T ",201
        dd   rep7
        db   "ARE YO",213
        dd   rep8
        db   "I CAN'",212
        dd   rep9
        db   "I A",205
        dd   rep10
        db   "I'M",160
        dd   rep10
        db   "YOU",160
        dd   rep11
        db   "I WAN",212
        dd   rep12
        db   "WHA",212
        dd   rep13
        db   "HO",215
        dd   rep13
        db   "WH",207
        dd   rep13
        db   "WHER",197
        dd   rep13
        db   "WHE",206
        dd   rep13
        db   "WH",217
        dd   rep13
        db   "NAM",197
        dd   rep14
        db   "CAUS",197
        dd   rep15
        db   "SORR",217
        dd   rep16
        db   "DREA",205
        dd   rep17
        db   "HELL",207
        dd   rep18
        db   "HI",160
        dd   rep18
        db   "MAYB",197
        dd   rep19
        db   " N",207
        dd   rep20
        db   "YOU",210
        dd   rep21
        db   "ALWAY",211
        dd   rep22
        db   "THIN",203
        dd   rep23
        db   "ALIK",197
        dd   rep24
        db   "YE",211
        dd   rep25
        db   "FRIEN",196
        dd   rep26
        db   "COMPUTE",210
        dd   rep27
        db   "CA",210
        dd   rep28
        db   "BY",197
        dd   progmend
        db   128
        dd   rep29
keyword2:
        db   " "
        db   "AR",197
        dd   rep30
        db   "WER",197
        dd   rep31
        db   "YOU",210
        dd   rep33
        db   "YO",213
        dd   rep32
        db   "I'V",197
        dd   rep34
        db   "I'",205
        dd   rep35
        db   "YO",213
        dd   rep36
        db   128
        dd   rep37
prtblk:
        mov  esi,blkprt
        call [PrintString]
        ret
 ;----------------------------------------------------;
 ; calltable include goes here.                       ;
 ;----------------------------------------------------;
blkprt:
        db   13,13
userline:
        dw   0
line1:
	db   13,10
        db   "                           *** ELIZA ***",13,10, 13,10
        db   "                    First writen by: Joseph Weizenbaum ",13, 10, 13, 10
        db   "     MODIFIED FROM CYBER 175 AT UNIVERSITY OF ILLINOIS AT CHAMPAGNE",13, 10
        db   "                       BY JOHN SCHUGG JANUARY 1985",13, 10, 13, 10
        db   "                 Converted to asm for DexOS by Roboman 2007",13, 10
        db   "                Converted to asm for KolibriOS by Albom 2008",13, 10, 13, 10
        db   "                          HAVE ANY PROBLEMS ?",13, 10, 13, 10
        db   "                          LET ELIZA HELP YOU!",13, 10, 13, 10
        db   "                        TO STOP ELIZA TYPE 'bye'",13, 10, 13, 10
        db   "            (THIS VERSION WILL NOT RECORD YOUR CONVERSATIONS)",13, 10, 13, 10
        db   "                    < Press any key to continue...>",13, 10, 0
Program_name:
        db   "--** Dr. Eliza **--",0
Message:
        db   "HI!  I'M ELIZA.  WHAT'S YOUR PROBLEM?",0
message2:
        db  13,13,"Thanks for talking things over with Eliza, bye",13,13,0
rep1a:  db  "DON'T YOU BELIEVE THAT I CAN ",0
rep1b:  db  "PERHAPS YOU WOULD LIKE TO BE ABLE TO ",0
rep1c:  db  "YOU WANT ME TO BE ABLE TO ",0
rep2a:  db  "PERHAPS YOU DON'T WANT TO ",0
rep2b:  db  "DO YOU WANT TO BE ABLE TO ",0
rep3a:  db  "WHAT MAKES YOU THINK I AM ",0
rep3b:  db  "DOES IT PLEASE YOU TO BELIEVE I AM ",0
rep3c:  db  "PERHAPS YOU WOULD LIKE TO BE ",0
rep3d:  db  "DO YOU SOMETIMES WISH YOU WERE ",0
rep4a:  db  "DON'T YOU REALLY ",0
rep4b:  db  "WHY DON'T YOU ",0
rep4c:  db  "DO YOU WISH TO BE ABLE TO ",0
rep4d:  db  "DOES THAT TROUBLE YOU?",0
rep5a:  db  "TELL ME MORE ABOUT SUCH FEELINGS.",0
rep5b:  db  "DO YOU OFTEN FEEL ",0
rep5c:  db  "DO YOU ENJOY FEELING ",0
rep6a:  db  "DO YOU REALLY BELIEVE I DON'T ",0
rep6b:  db  "PERHAPS IN GOOD TIME I WILL ",0
rep6c:  db  "DO YOU WANT ME TO ",0
rep7a:  db  "DO YOU THINK YOU SHOULD BE ABLE TO ",0
rep7b:  db  "WHY CAN'T YOU ",0
rep8a:  db  "WHY ARE YOU INTERESTED IN WHETHER OR NOT I AM ",0
rep8b:  db  "WOULD YOU PREFER IF I WERE NOT ",0
rep8c:  db  "PERHAPS IN YOUR FANTASIES I AM ",0
rep9a:  db  "HOW DO YOU KNOW YOU CAN'T ",0
rep9b:  db  "HAVE YOU TRIED?",0
rep9c:  db  "PERHAPS YOU CAN NOW ",0
rep10a: db  "DID YOU COME TO ME BECAUSE YOU ARE ",0
rep10b: db  "HOW LONG HAVE YOU BEEN ",0
rep10c: db  "DO YOU BELIEVE IT IS NORMAL TO BE ",0
rep10d: db  "DO YOU ENJOY BEING ",0
rep11a: db  "WE WERE DISCUSSING YOU-- NOT ME.",0
rep11b: db  "OH, I ",0
rep11c: db  "YOU'RE NOT REALLY TALKING ABOUT ME, ARE YOU?",0
rep12a: db  "WHAT WOULD IT MEAN TO YOU IF YOU GOT ",0
rep12b: db  "WHY DO YOU WANT ",0
rep12c: db  "SUPPOSE YOU SOON GOT ",0
rep12d: db  "WHAT IF YOU NEVER GOT ",0
rep12e: db  "I SOMETIMES ALSO WANT ",0
rep13a: db  "WHY DO YOU ASK?",0
rep13b: db  "DOES THAT QUESTION INTEREST YOU?",0
rep13c: db  "WHAT ANSWER WOULD PLEASE YOU THE MOST?",0
rep13d: db  "WHAT DO YOU THINK?",0
rep13e: db  "ARE SUCH QUESTIONS ON YOUR MIND OFTEN?",0
rep13f: db  "WHAT IS IT THAT YOU REALLY WANT TO KNOW?",0
rep13g: db  "HAVE YOU ASKED ANYONE ELSE?",0
rep13h: db  "HAVE YOU ASKED SUCH QUESTIONS BEFORE?",0
rep13i: db  "WHAT ELSE COMES TO MIND WHEN YOU ASK THAT?",0
rep14a: db  "NAMES DON'T INTEREST ME.",0
rep14b: db  "I DON'T CARE ABOUT NAMES-- PLEASE GO ON.",0
rep15a: db  "IS THAT THE REAL REASON?",0
rep15b: db  "DON'T ANY OTHER REASONS COME TO MIND?",0
rep15c: db  "DOES THAT REASON EXPLAIN ANY THING ELSE?",0
rep15d: db  "WHAT OTHER REASONS MIGHT THERE BE?",0
rep16a: db  "PLEASE DON'T APOLOGIZE.",0
rep16b: db  "APOLOGIES ARE NOT NECESSARY.",0
rep16c: db  "WHAT FEELINGS DO YOU HAVE WHEN YOU APOLOGIZE?",0
rep16d: db  "DON'T BE SO DEFENSIVE!",0
rep17a: db  "WHAT DOES THAT DREAM SUGGEST TO YOU?",0
rep17b: db  "DO YOU DREAM OFTEN?",0
rep17c: db  "WHAT PERSONS APPEAR IN YOUR DREAMS?",0
rep17d: db  "ARE YOU DISTURBED BY YOUR DREAMS?",0
rep18a: db  "HOW DO YOU DO--PLEASE STATE YOUR PROBLEM.",0
rep19a: db  "YOU DON'T SEEM QUITE CERTAIN.",0
rep19b: db  "WHY THE UNCERTAIN TONE?",0
rep19c: db  "CAN'T YOU BE MORE POSITIVE?",0
rep19d: db  "YOU AREN'T SURE?",0
rep19e: db  "DON'T YOU KNOW?",0
rep20a: db  "ARE YOU SAYING NO JUST TO BE NEGATIVE?",0
rep20b: db  "YOU ARE BEING A BIT NEGATIVE.",0
rep20c: db  "WHY NOT?",0
rep20d: db  "ARE YOU SURE?",0
rep20e: db  "WHY NO?",0
rep21a: db  "WHY ARE YOU CONCERNED ABOUT MY ",0
rep21b: db  "WHAT ABOUT YOUR OWN ",0
rep22a: db  "CAN YOU THINK OF A SPECIFIC EXAMPLE?",0
rep22b: db  "WHEN?",0
rep22c: db  "WHAT ARE YOU THINKING OF?",0
rep22d: db  "REALLY, ALWAYS?",0
rep23a: db  "DO YOU REALLY THINK SO?",0
rep23b: db  "BUT YOU ARE NOT SURE YOU ",0
rep23c: db  "DO YOU DOUBT YOU ",0
rep24a: db  "IN WHAT WAY?",0
rep24b: db  "WHAT RESEMBLANCE DO YOU SEE?",0
rep24c: db  "WHAT DOES THE SIMILARITY SUGGEST TO YOU?",0
rep24d: db  "WHAT OTHER CONNECTIONS DO YOU SEE?",0
rep24e: db  "COULD THERE REALLY BE SOME CONNECTION?",0
rep24f: db  "HOW?",0
rep24g: db  "YOU SEEM QUITE POSITIVE.",0
rep25a: db  "ARE YOU SURE?",0
rep25b: db  "I SEE.",0
rep25c: db  "I UNDERSTAND.",0
rep26a: db  "WHY DO YOU BRING UP THE TOPIC OF FRIENDS?",0
rep26b: db  "DO YOUR FRIENDS WORRY YOU?",0
rep26c: db  "DO YOUR FRIENDS PICK ON YOU?",0
rep26d: db  "ARE YOU SURE YOU HAVE ANY FRIENDS?",0
rep26e: db  "DO YOU IMPOSE ON YOUR FRIENDS?",0
rep26f: db  "PERHAPS YOUR LOVE FOR FRIENDS WORRIES YOU?",0
rep27a: db  "DO COMPUTERS WORRY YOU?",0
rep27b: db  "ARE YOU TALKING ABOUT ME IN PARTICULAR?",0
rep27c: db  "ARE YOU FRIGHTENED BY MACHINES?",0
rep27d: db  "WHY DO YOU MENTION COMPUTERS?",0
rep27e: db  "WHAT DO YOU THINK MACHINES HAVE TO DO WITH YOUR PROBLEM?",0
rep27f: db  "DON'T YOU THINK COMPUTERS CAN HELP PEOPLE?",0
rep27g: db  "WHAT IS IT ABOUT MACHINES THAT WORRIES YOU?",0
rep28a: db  "OH, DO YOU LIKE CARS?",0
rep28b: db  "MY FAVORITE CAR IS A LAMBORGINI COUNTACH. WHAT IS YOUR FAVORITE CAR?",0
rep28c: db  "MY FAVORITE CAR COMPANY IS FERRARI.  WHAT IS YOURS?",0
rep28d: db  "DO YOU LIKE PORSCHES?",0
rep28e: db  "DO YOU LIKE PORSCHE TURBO CARRERAS?",0
rep29a: db  "SAY, DO YOU HAVE ANY PSYCHOLOGICAL PROBLEMS?",0
rep29b: db  "WHAT DOES THAT SUGGEST TO YOU?",0
rep29c: db  "I SEE.",0
rep29d: db  "I'M NOT SURE I UNDERSTAND YOU FULLY.",0
rep29e: db  "COME COME ELUCIDATE YOUR THOUGHTS.",0
rep29f: db  "CAN YOU ELABORATE ON THAT?",0
rep29g: db  "THAT IS QUITE INTERESTING.",0

include 'myConsole.inc'
include 'myDex.inc'

_param:
rb 256

_end:

align 32
rb 2048
_stack:
_memory:
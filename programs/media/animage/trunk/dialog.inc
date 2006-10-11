macro opendialog redproc,openoff,erroff,path
{
local new_d, get_loops, dlg_pid_get, DLGPID, num_of_proc
local run_fileinfo, param
local getmesloop, loox, mred, mkey, mbutton, mgetmes
local dlg_is_work, ready, procinfo
;
; STEP 1 Run SYSXTREE with parametrs MYPID 4 bytes in dec,
; 1 byte space, 1 byte type of dialog (O - Open ,S - Save)
;

    cld
;;    mov esi,path
    mov edi,path
    mov eax,0
    mov ecx,200
    rep stosb

;mov [get_loops],0
mov [dlg_pid_get],0

; Get my PID in dec format 4 bytes
    mov eax,9
    mov ebx,procinfo
    mov ecx,-1
    int 0x40

; convert eax bin to param dec
    mov eax,dword [procinfo+30]  ;offset of myPID
    mov edi,param+4-1		 ;offset to 4 bytes
    mov ecx,4
    mov ebx,10
    cld
new_d:
    xor edx,edx
    div ebx
    add dl,'0'
    mov [edi],dl
    dec edi
    loop new_d

; wirite 1 byte space to param
    mov [param+4],byte 32    ;Space for next parametr
; and 1 byte type of dialog to param
    mov [param+5],byte 'O'   ;Get Open dialog (Use 'S' for Save dialog)

;
; STEP2 prepare IPC area for get messages
;

; prepare IPC area
    mov [path],dword 0
    mov [path+4],dword 8

; define IPC memory
    mov eax,60
    mov ebx,1	     ; define IPC
    mov ecx,path     ; offset of area
    mov edx,150      ; size 150 bytes
    int 0x40

; change wanted events list 7-bit IPC event
    mov eax,40
    mov ebx,01000111b
    int 0x40

;
; STEP 3 run SYSTEM XTREE with parameters
;

    mov eax,58
    mov ebx,run_fileinfo
    int 0x40

    call redproc

    mov [get_loops],0
getmesloop:
    mov eax,23
    mov ebx,50	   ;0.5 sec
    int 0x40

    cmp eax,1
    je	mred
    cmp eax,2
    je	mkey
    cmp eax,3
    je	mbutton
    cmp eax,7
    je	mgetmes

; Get number of procces
    mov ebx,procinfo
    mov ecx,-1
    mov eax,9
    int 0x40
    mov ebp,eax

loox:
    mov eax,9
    mov ebx,procinfo
    mov ecx,ebp
    int 0x40
    mov eax,[DLGPID]
    cmp [procinfo+30],eax    ;IF Dialog find
    je	dlg_is_work	     ;jmp to dlg_is_work
    dec ebp
    jnz loox

    jmp erroff

dlg_is_work:
    cmp [procinfo+50],word 9 ;If slot state 9 - dialog is terminated
    je	erroff		       ;TESTODP2 terminated too

    cmp [dlg_pid_get],dword 1
    je	getmesloop
    inc [get_loops]
    cmp [get_loops],4  ;2 sec if DLG_PID not get, TESTOP2  terminated
    jae erroff
    jmp getmesloop

mred:
    call redproc
    jmp  getmesloop
mkey:
    mov  eax,2
    int  0x40			; read (eax=2)
    jmp  getmesloop
mbutton:
    mov  eax,17 		; get id
    int  0x40
    cmp  ah,1			; button id=1 ?
    jne  getmesloop
    mov  eax,-1 		; close this program
    int  0x40
mgetmes:

; If dlg_pid_get then second message get jmp to still
    cmp  [dlg_pid_get],dword 1
    je	 ready

; First message is number of PID SYSXTREE dialog

; convert PID dec to PID bin
    movzx eax,byte [path+16]
    sub eax,48
    imul eax,10
    movzx ebx,byte [path+16+1]
    add eax,ebx
    sub eax,48
    imul eax,10
    movzx ebx,byte [path+16+2]
    add eax,ebx
    sub eax,48
    imul eax,10
    movzx ebx,byte [path+16+3]
    add eax,ebx
    sub eax,48
    mov [DLGPID],eax

; Claear and prepare IPC area for next message
    mov [path],dword 0
    mov [path+4],dword 8
    mov [path+8],dword 0
    mov [path+12],dword 0
    mov [path+16],dword 0

; Set dlg_pid_get for get next message
    mov [dlg_pid_get],dword 1
    call redproc   ;show DLG_PID
    jmp  getmesloop

ready:
;
; The second message get
; Second message is 100 bytes path to SAVE/OPEN file
; shl path string on 16 bytes
;
    cld
    mov esi,path+16
    mov edi,path
    mov ecx,200
    rep movsb
    mov [edi],byte 0

    jmp openoff


; DATA AREA
get_loops   dd 0
dlg_pid_get dd 0
DLGPID	    dd 0

param:
   dd 0    ; My dec PID
   dd 0,0  ; Type of dialog

run_fileinfo:
 dd 16
 dd 0
 dd param
 dd 0
 dd procinfo ; 0x10000
;run_filepath
 db '/RD/1/SYSXTREE',0

procinfo:
times 1024 db 0
}


macro savedialog redproc,openoff,erroff,path
{
local new_d, get_loops, dlg_pid_get, DLGPID, num_of_proc
local run_fileinfo, run_filepath, param
local getmesloop, loox, mred, mkey, mbutton, mgetmes
local dlg_is_work, ready, procinfo
;
; STEP 1 Run SYSXTREE with parametrs MYPID 4 bytes in dec,
; 1 byte space, 1 byte type of dialog (O - Open ,S - Save)
;

    cld
;;    mov esi,path
    mov edi,path
    mov eax,0
    mov ecx,200
    rep stosb

;mov [get_loops],0
mov [dlg_pid_get],0

; Get my PID in dec format 4 bytes
    mov eax,9
    mov ebx,procinfo
    mov ecx,-1
    int 0x40

; convert eax bin to param dec
    mov eax,dword [procinfo+30]  ;offset of myPID
    mov edi,param+4-1		 ;offset to 4 bytes
    mov ecx,4
    mov ebx,10
    cld
new_d:
    xor edx,edx
    div ebx
    add dl,'0'
    mov [edi],dl
    dec edi
    loop new_d

; wirite 1 byte space to param
    mov [param+4],byte 32    ;Space for next parametr
; and 1 byte type of dialog to param
    mov [param+5],byte 'S'   ;Get Open dialog (Use 'S' for Save dialog)

;
; STEP2 prepare IPC area for get messages
;

; prepare IPC area
    mov [path],dword 0
    mov [path+4],dword 8

; define IPC memory
    mov eax,60
    mov ebx,1	     ; define IPC
    mov ecx,path ; offset of area
    mov edx,150      ; size 150 bytes
    int 0x40

; change wanted events list 7-bit IPC event
    mov eax,40
    mov ebx,01000111b
    int 0x40

;
; STEP 3 run SYSTEM XTREE with parameters
;

    mov eax,58
    mov ebx,run_fileinfo
    int 0x40

    call redproc

    mov [get_loops],0
getmesloop:
    mov eax,23
    mov ebx,50	   ;0.5 sec
    int 0x40

    cmp eax,1
    je	mred
    cmp eax,2
    je	mkey
    cmp eax,3
    je	mbutton
    cmp eax,7
    je	mgetmes

; Get number of procces
    mov ebx,procinfo
    mov ecx,-1
    mov eax,9
    int 0x40
    mov ebp,eax

loox:
    mov eax,9
    mov ebx,procinfo
    mov ecx,ebp
    int 0x40
    mov eax,[DLGPID]
    cmp [procinfo+30],eax    ;IF Dialog find
    je	dlg_is_work	     ;jmp to dlg_is_work
    dec ebp
    jnz loox

    jmp erroff

dlg_is_work:
    cmp [procinfo+50],word 9 ;If slot state 9 - dialog is terminated
    je	erroff		       ;TESTODP2 terminated too

    cmp [dlg_pid_get],dword 1
    je	getmesloop
    inc [get_loops]
    cmp [get_loops],4  ;2 sec if DLG_PID not get, TESTOP2  terminated
    jae erroff
    jmp getmesloop

mred:
    call redproc
    jmp  getmesloop
mkey:
    mov  eax,2
    int  0x40			; read (eax=2)
    jmp  getmesloop
mbutton:
    mov  eax,17 		; get id
    int  0x40
    cmp  ah,1			; button id=1 ?
    jne  getmesloop
    mov  eax,-1 		; close this program
    int  0x40
mgetmes:

; If dlg_pid_get then second message get jmp to still
    cmp  [dlg_pid_get],dword 1
    je	 ready

; First message is number of PID SYSXTREE dialog

; convert PID dec to PID bin
    movzx eax,byte [path+16]
    sub eax,48
    imul eax,10
    movzx ebx,byte [path+16+1]
    add eax,ebx
    sub eax,48
    imul eax,10
    movzx ebx,byte [path+16+2]
    add eax,ebx
    sub eax,48
    imul eax,10
    movzx ebx,byte [path+16+3]
    add eax,ebx
    sub eax,48
    mov [DLGPID],eax

; Claear and prepare IPC area for next message
    mov [path],dword 0
    mov [path+4],dword 8
    mov [path+8],dword 0
    mov [path+12],dword 0
    mov [path+16],dword 0

; Set dlg_pid_get for get next message
    mov [dlg_pid_get],dword 1
    call redproc   ;show DLG_PID
    jmp  getmesloop

ready:
;
; The second message get
; Second message is 100 bytes path to SAVE/OPEN file
; shl path string on 16 bytes
;
    cld
    mov esi,path+16
    mov edi,path
    mov ecx,200
    rep movsb
    mov [edi],byte 0

    jmp openoff


; DATA AREA
get_loops   dd 0
dlg_pid_get dd 0
DLGPID	    dd 0

param:
   dd 0  ; My dec PID
   dd 0,0  ; Type of dialog

run_fileinfo:
 dd 16
 dd 0
 dd param
 dd 0
 dd procinfo
;run_filepath:
 db '/RD/1/SYSXTREE',0

procinfo:
times 1024 db 0
}
;-----------------------------------------------------------

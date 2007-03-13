;
;   MenuetOS Driver (vmode.mdr)
;   Target: Vertical Refresh Rate programming and videomode changing
;
;   Author: Trans <<<<<13>>>>>
;   Date:   20.07.2003
;
;   Version: 1.0
;   OS: MenuetOS
;   Compiler: FASM
;

OS_BASE equ 0x80000000

use32

macro align value { rb (value-1) - ($ + value-1) mod value }

        org    0x80760000

headerstart=$

mdid    db     'MDAZ'              ; 4 byte id
mdhver  dd     0x00                ; header version
mdcode  dd     MDSTART             ; start of code
mdver   dd     0x00000001          ; driver version (subversion*65536+version)
mdname  db     'Trans VideoDriver' ; 32 bytes of full driver name
    times (32-($-mdname)) db ' '   ;

headerlen=$-headerstart
    times (256-headerlen) db 0     ; reserved area for future

MDSTART:    ; start of driver code ( base_adr+256 bytes)
; ebx(=ecx in program):
;   1 - Get DriverInfo and Driver Initial Set
;   2 - Get Current Video Mode With Vertical Refresh Rate
;   3 - Change Video Mode
;   4 - Return at Start System Video Mode
;   5 - Change vertical and horizontal size of visible screen area
;   6 - Change Vert/Hor position visible area on screen (not complete yet)
;
;   MAXF - ...
MAXF=5

;-------Main Manager-------------
    pushad
    cmp ebx,1
    jb mdvm_00
    cmp ebx,MAXF
    ja mdvm_00
    shl ebx,2
    add ebx,mdvm_func_table
    call dword [ebx]
    mov [esp+28],eax
    mov [esp+24],ecx
    mov [esp+20],edx
    mov [esp+16],ebx
    popad
    retn
mdvm_00:
    popad
    xor eax,eax
    dec eax
    retn

; ------Drivers Functions----------

align 4

; EBX=1 (in applications ECX=1)- Get DriverInfo and Driver Initial Set
;
; IN: ecx (in app. edx) - pointer to 512-bytes info area in application
; OUT:
;
vm_info_init:
      push ecx
      cmp [mdrvm],dword 0
      jnz .vmii_00
      call vm_safe_reg
      call vm_get_initial_videomode
      mov eax,[initvm]
      mov [currvm],eax
      call vm_search_sys_func_table
      call vm_get_cur_vert_rate
      mov [initrr],eax
      call vm_calc_pixelclock
      call vm_calc_refrate
      inc [mdrvm]
.vmii_00:
      pop ecx
      call vm_transfer_drv_info
      mov ebx,dword [refrate]
      mov eax,dword [mdid]      ;dword [systlb]
      retn


align 4

; EBX=2 (in applications ECX=2)- Get Current Video Mode
;
; OUT: eax = X_screen*65536+Y_screen
;      ebx = current vertical rate
;      ecx = current video mode (number)
vm_get_cur_mode:
      cmp [mdrvm],dword 0
      jz .vmgcm_00
      call vm_get_cur_vert_rate
      mov eax,[OS_BASE+0FE00h]
      mov ebx,[OS_BASE+0FE04h]
      shl eax,16
      add eax,ebx
      add eax,00010001h
      mov ebx,[refrate]
      mov ecx,[currvm]
      retn
.vmgcm_00:
      xor eax,eax
      dec eax
      retn


align 4

; EBX=3 (in applications ECX=3)- Change Video Mode
;
; IN:  ecx = VertRate*65536+VideoMode
; OUT: eax = 0 if no error
;
vm_set_video_mode:
      cmp [mdrvm],dword 0
      jz .vmsvm_00
      call vm_set_selected_mode
;      xor eax,eax
      retn
.vmsvm_00:
      xor eax,eax
      dec eax
      retn


align 4

; EBX=4 (in applications ECX=4)- Return at Start System Video Mode
;
; IN:
; OUT: eax = = 0 if no error
;
vm_restore_init_video_mode:
      cmp [mdrvm],dword 0
      jz .vmrivm_00
      call vm_restore_reg
      xor eax,eax
      retn
.vmrivm_00:
      xor eax,eax
      dec eax
      retn


align 4

; EBX=5 (in applications ECX=5)- Change vertical and horizontal size
;                                 of visible screen area
; IN:  ecx (in app. edx) = 0/1 - -/+ horizontal size on 1 position
;                        = 2/3 - -/+ vertical size on 1 position (8 pixels)
;                          ^-^----- not complete yet
; OUT: eax = = 0 if no error
;
vm_change_screen_size:
      cmp [mdrvm],dword 0
      jz .vmcss_00
      cmp cl,1
      ja .vmcss_01
      mov eax,ecx
      call vm_inc_dec_width
      xor eax,eax
      retn
.vmcss_01:
      and ecx,01h
      mov eax,ecx
;     call vm_inc_dec_high   ; not complete yet
      xor eax,eax
      retn
.vmcss_00:
      xor eax,eax
      dec eax
      retn


align 4

; EBX=6 (in applications ECX=6)- Change Vert/Hor position visible area on screen
;
; IN:  ecx (in app. edx) = 0/1 - -/+ horizontal position on 1 point
;                        = 2/3 - -/+ vertical position on 1 pixel
;                          ^-^----- not complete yet
; OUT: eax = 0 if no error
;
vm_change_position_screen:
      cmp [mdrvm],dword 0
      jz .vmcps_00
  ; ...
      xor eax,eax
      retn
.vmcps_00:
      xor eax,eax
      dec eax
      retn


;-----Drivers Subfunctions---------

;
; Searching i40 system functions pointer table in kernel area location
;
vm_search_sys_func_table:
      push eax     ; eax - current value
      push ecx     ; ecx - will be counter of equevalent value
      push edx     ; edx - last value
      push esi     ; esi - current address
      xor ecx,ecx
      mov esi,OS_BASE+010000h            ; Start address of kernel location
      lodsd
      mov edx,eax
      cld
.vmssft_00:
      cmp esi,OS_BASE+30000h
      ja .vmssft_03
      inc ecx
      lodsd
      cmp edx,eax
      mov edx,eax
      je .vmssft_00
      cmp ecx,128
      ja .vmssft_02
.vmssft_01:
      xor ecx,ecx
      jmp .vmssft_00
.vmssft_02:
      cmp edx,0
      je .vmssft_01
      sub esi,256*4-1
      mov [systlb],esi
      xor ecx,ecx
.vmssft_03_0:
      inc ecx
      lodsd
      cmp edx,eax
      mov edx,eax
      jne .vmssft_03_0
      mov esi,dword [systlb]
      cmp cx,60
      jae .vmssft_03
      add esi,256*4-4
      lodsb
      mov edx,eax
      jmp .vmssft_01
.vmssft_03:
      mov [systlb],esi
      pop esi
      pop edx
      pop ecx
      pop eax
      retn

; IN:
; OUT: eax= vertical rate in Hz
vm_get_cur_vert_rate:
      push edx
      push ebx
      xor eax,eax
      mov edx,eax
      mov ebx,eax
      mov dx,03DAh
.vmgcvt_00:
      in al,dx
      test al,8
      jz .vmgcvt_00
.vmgcvt_01:
      in al,dx
      test al,8
      jnz .vmgcvt_01
      mov ebx,edx
      rdtsc
      mov edx,ebx
      mov ebx,eax
.vmgcvt_02:
      in al,dx
      test al,8
      jz .vmgcvt_02
.vmgcvt_03:
      in al,dx
      test al,8
      jnz .vmgcvt_03
      rdtsc
      sub eax,ebx
      mov ebx,eax
      mov eax,[OS_BASE+0F600h]
      xor edx,edx
      div ebx
      inc eax
      mov [refrate],eax
      pop ebx
      pop edx
      retn

vm_calc_pixelclock:
        push ebx
        push edx
        xor eax,eax
        mov al,[_00]
        add ax,5
        shl eax,3
        xor ebx,ebx
        mov bl,[_06]
        mov bh,[_07]
        and bh,00100001b
        btr bx,13
        jnc .vmcpc_00
        or bh,2
.vmcpc_00:
        xor edx,edx
        mul ebx
        xor edx,edx
        mul [initrr]
        mov [pclock],eax
        pop edx
        pop ebx
        retn

;
; Safe of initial CRTC state
;
vm_safe_reg:
    push edx
    push ebx
    push ecx
    push edi
    cli
    mov dx,3d4h  ; CRTC
    mov al,11h
    out dx,al
    inc dx
    in al,dx
    and al,7fh
    out dx,al    ; Clear protection bit
    dec dx
    xor ecx,ecx
    mov cl,19h
    xor bl,bl
    mov edi,CRTCreg
.vmsr_00:
    mov al,bl
    out dx,al
    inc dx
    in al,dx
    dec dx
    stosb
    inc bl
    loop .vmsr_00
    sti
    pop edi
    pop ecx
    pop ebx
    pop edx
    retn

;
; Restore of initial CRTC state
;
vm_restore_reg:
    push eax
    push ebx
    push edx
    push esi
    mov eax,[oldX]
    mov [OS_BASE+0FE00h],eax
    mov eax,[oldY]
    mov [OS_BASE+0FE04h],eax
    mov dx,03dah
.vmrr_00:
    in al,dx
    test al,8
    jnz .vmrr_00
.vmrr_01:
    in al,dx
    test al,8
    jnz .vmrr_01
    cli
    mov dx,03c4h
    mov ax,0101h
    out dx,ax
    mov dx,3d4h  ; CRTC
    mov al,11h
    out dx,al
    inc dx
    in al,dx
    and al,7fh  ; Clear Protection bit
    out dx,al
    dec dx
    xor ecx,ecx
    mov cl,19h
    mov esi,CRTCreg
    xor bl,bl
.vmrr_02:
    lodsb
    mov ah,al
    mov al,bl
    out dx,ax
    inc bl
    loop .vmrr_02
    sti
;    call ref_screen
    pop esi
    pop edx
    pop ecx
    pop eax
    retn

; Calculate of possible vertical refrash rate
;  (light version of function)
vm_calc_refrate:
        push ebx
        push ecx
        push edx
        push edi
        push esi
        mov eax,[pclock]
        xor edx,edx
        mov edi,_m1
        mov ebx,eax
        mov ecx,(1696*1065)
        div ecx
        xor edx,edx
        stosw
        add edi,8
        mov eax,ebx
        mov ecx,(1344*804)
        div ecx
        xor edx,edx
        stosw
        add edi,8
        mov eax,ebx
        mov ecx,(1056*636)
        div ecx
        xor edx,edx
        stosw
        add edi,8
        mov eax,ebx
        mov ecx,(800*524)
        div ecx
        xor edx,edx
        stosw
        mov edi,_m1
        mov esi,edi
        mov ecx,5*4
.vmcrr_00:
        lodsw
        cmp ax,55
        jb .vmcrr_01
        stosw
        loop .vmcrr_00
        pop esi
        pop edi
        pop edx
        pop ecx
        pop ebx
        retn
.vmcrr_01:
        xor ax,ax
        stosw
        loop .vmcrr_00
        pop esi
        pop edi
        pop edx
        pop ecx
        pop ebx
        retn

vm_get_initial_videomode:
    push eax
    mov eax,dword [OS_BASE+0FE00h]
    mov [oldX],eax
    mov eax,dword [OS_BASE+0FE04h]
    mov [oldY],eax
    mov eax,dword [OS_BASE+0FE0Ch] ; initial video mode
    and ax,01FFh
    mov dword [initvm],eax
    pop eax
    retn


; IN: eax = 0/1  -  -/+ 1 position of width
vm_inc_dec_width:
    push ebx
    push edx
    mov ebx,eax
    mov dx,3d4h  ; CRTC
    mov al,11h
    out dx,al
    inc dx
    in al,dx
    and al,7fh  ; Clear Protection bit
    out dx,al
    dec dx
    xor al,al
    out dx,al
    inc dx
    in al,dx
    dec al
    cmp bl,0
    jnz .vmidr_00
    inc al
    inc al
.vmidr_00:
    out dx,al
    pop edx
    pop ebx
    retn

;
; Copy driver info to application area
;
; IN: ecx (in app. edx) - pointer to 512-bytes info area in application
; OUT:
vm_transfer_drv_info:
        push ecx
        push edi
        push esi
        mov eax,ecx
        xor ecx,ecx
        mov cl,32/4
        mov esi,mdname
        mov edi,drvname
        rep movsd
        mov ecx,eax
        mov eax,[mdver]
        mov [drvver],eax
        mov edi,[OS_BASE+3010h]
        mov edi,[edi+10h]
        add edi,ecx
        mov esi,drvinfo
        xor ecx,ecx
        mov cx,512
        rep movsb
        pop esi
        pop edi
        pop ecx
        retn


;
; Set selected video mode
; (light version)
;
; IN: ecx = VertRate*65536+VideoMode
;
vm_set_selected_mode:
    push edx
    push ecx
    push esi
    ror ecx,16
    cmp cx,00h
    je .vmssm_03
    rol ecx,16
    mov eax,ecx
    shl eax,16
    shr eax,16
    mov [currvm],eax
    cmp cx,112h
    jne .vmssm_00
    mov esi,mode0
    mov ecx,639
    mov edx,479
    jmp .vmssm_st00
.vmssm_00:
    cmp cx,115h
    jne .vmssm_01
    mov esi,mode1
    mov ecx,799
    mov edx,599
    jmp .vmssm_st00
.vmssm_01:
    cmp cx,118h
    jne .vmssm_02
    mov esi,mode2
    mov ecx,1023
    mov edx,767
    jmp .vmssm_st00
.vmssm_02:
    cmp cx,11Bh
    jne .vmssm_03
    mov esi,mode2
    mov ecx,1279
    mov edx,1023
    jmp .vmssm_st00
.vmssm_03:
    xor eax,eax
    dec eax
    pop esi
    pop ecx
    pop edx
    retn
.vmssm_st00:
    mov [OS_BASE+0FE00h],ecx
    mov [OS_BASE+0FE04h],edx
    cli
    mov dx,03c4h
    lodsw
    out dx,ax
    mov dx,03d4h
    mov al,11h
    out dx,al
    inc dx
    in al,dx
    and al,7fh
    out dx,al
    dec dx
    mov ecx,13
.vmssm_st01:
    lodsw
    out dx,ax
    loop .vmssm_st01
    sti
    xor eax,eax
    pop esi
    pop ecx
    pop edx
    retn


;------------DATA AREA---------------
align 4

mdvm_func_table:
        dd MDSTART
        dd vm_info_init,          vm_get_cur_mode
        dd vm_set_video_mode,     vm_restore_init_video_mode
        dd vm_change_screen_size, vm_change_position_screen


CRTCreg:
_00  db ?
_01  db ?
_02  db ?
_03  db ?
_04  db ?
_05  db ?
_06  db ?
_07  db ?
_08  db ?
_09  db ?
_0a  db ?
_0b  db ?
_0c  db ?
_0d  db ?
_0e  db ?
_0f  db ?
_10  db ?
_11  db ?
_12  db ?
_13  db ?
_14  db ?
_15  db ?
_16  db ?
_17  db ?
_18  db ?
_19  db ?

align 4

oldX       dd ?
oldY       dd ?
initvm     dd ?
currvm     dd 0
refrate    dd 0
initrr     dd 0
systlb     dd 0
pclock     dd ?
mdrvm      dd 0 ; 0 - not drv init yet, 1 - already drv init


drvinfo:
drvname:   times 32 db ' '
drvver     dd 0
           times (32-($-drvver))/4 dd 0
drvmode    dw 011Bh,0118h,0115h,0112h
           times (64-($-drvmode))/2 dw 00h
_m1        dw 0,0,0,0,0
_m2        dw 0,0,0,0,0
_m3        dw 0,0,0,0,0
_m4        dw 0,0,0,0,0
_m5        dw 0,0,0,0,0
           times (512-($-drvinfo)) db 0
drvinfoend:


;1280x1024 - 11Bh
mode3:
      dw 0101h
      dw 0d000h,9f01h,9f02h,9303h,0a904h,1905h,2806h,5a07h
      dw 0110h,8411h,0ff12h,0ff15h,2916h

;1024x768 - 118h
mode2:
      dw 0101h
      dw 0a400h,7f01h,7f02h,8703h,8404h,9505h,2406h,0f507h
      dw 0310h,8911h,0ff12h,0ff15h,2516h

;800x600  - 115h
mode1:
      dw 0101h
      dw 8000h,6301h,6302h,8303h,6a04h,1a05h,7206h,0f007h
      dw 5910h,8d11h,5712h,5715h,7316h

;640x480 - 112h, 12h
mode0:
      dw 0101h
      dw 6000h,4f01h,4f02h,8303h,5304h,9f05h,00b06h,3e07h
      dw 0ea10h,8c11h,0df12h,0df15h,0c16h

; 640x400
;mymode0:
; dw 0101h
;_0_7 dw 5f00h,4f01h,4f02h,8303h,5304h,9f05h,0BF06h,1f07h
; dw 9c10h,8e11h,8f12h,9615h,0B916h ;,4013h

; 640x800
;mymode1:
; dw 0101h
; dw 5f00h,4f01h,4f02h,8003h,5004h,9f05h,06006h,0FF07h
; dw 2d10h,8f11h,2012h,2615h,05716h ;,4013h


DRVM_END:


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                        ;;
;;  BOOTCODE.INC                                          ;;
;;                                                        ;;
;;  16 bit bootcode for MenuetOS                          ;;
;;                                                        ;;
;;  Copyright 2002 Ville Turjanmaa                        ;;
;;                                                        ;;
;;  See file COPYING for details                          ;;
;;                                                        ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



;==========================================================================
;
;                           16 BIT FUNCTIONS
;
;==========================================================================

print:       push  si
             mov   si,leftpr-0x10000
             call  printplain
             pop   si

printplain:  pusha
             mov   dx,0x1000
             mov   es,dx
             cmp   byte [es:display_atboot-0x10000],2
             je    printplain_exit
             mov   ds,dx
             cld
             lodsb
           prpl1:
             mov   ah,0xe
             xor   bh,bh
             int   0x10
             lodsb
             cmp   al,0
             jne   prpl1
           printplain_exit:
             popa
             ret

setbase1000: push ax
             mov  ax,0x1000
             mov  es,ax
             mov  ds,ax
             pop  ax
             ret

getkey:      push  ecx
             push  edx
             add   ebx,0x0101
             xor   eax,eax

           gk1:
             in    al,0x60
             mov   cl,al
           gk0:
             in    al,0x60
             cmp   al,cl
             je    gk0
             cmp   ax,11
             jg    gk0
             gk0_1:
             mov   cl,al

;             add   al,47
;             mov   [ds:keyinbs-0x10000],al
;             mov   si,keyinbs-0x10000
;             call  printplain

           gk12:
             in    al,0x60
             cmp   al,cl
             je    gk12
             cmp   ax,240
             jne   gk13
             mov   al,cl
             jmp   gk14
           gk13:
             add   cl,128
             cmp   al,cl
             jne   gk1
             sub   al,128
           gk14:

             movzx edx,bl
             cmp   eax,edx
             jb    gk1
             movzx edx,bh
             cmp   eax,edx
             jg    gk1
             test  ebx,0x010000
             jnz   gk3
             mov   cx,0x1000
             mov   dx,cx
             add   eax,47
             mov   cx,ax
             cmp   cx,58
             jb    gk_nozero
             sub   cx,10
           gk_nozero:
             mov   [ds:keyin-0x10000],cl
             mov   si,keyin-0x10000
             call  printplain
           gk3:
             sub   eax,48
             pop   edx
             pop   ecx
             ret


;=========================================================================
;
;                           16 BIT CODE
;
;=========================================================================


start_of_code:

; RESET 16 BIT SELECTORS/REGISTERS/STACK

        mov  ax,0x1000
        mov  es,ax
        mov  ds,ax

        mov  ax,0x2000
        mov  ss,ax
        mov  sp,0xffff

        xor  ax,ax
        xor  bx,bx
        xor  cx,cx
        xor  dx,dx
        xor  si,si
        xor  di,di
        xor  bp,bp



; DRAW FRAMES

        call setbase1000

        cmp  byte [es:display_atboot-0x10000],2
        je   no_mode_atboot

        mov  ax,0x0003
        mov  bx,0x0000
        mov  dx,0x0000
        int  0x10

   no_mode_atboot:

 ; Load & set russian VGA font (RU.INC)
        mov  bp,RU_FNT1-10000h   ; RU_FNT1 - First part
          mov  bx,1000h            ; 768 bytes
          mov  cx,30h              ; 48 symbols
          mov  dx,80h              ; 128 - position of first symbol
        mov  ax,1100h
          push cs
          pop  es
          int  10h

        mov  bp,RU_FNT2-10000h   ; RU_FNT2 -Second part
        mov  bx,1000h            ; 512 bytes
        mov  cx,20h              ; 32 symbols
        mov  dx,0E0h             ; 224 - position of first symbol
        mov  ax,1100h
          push cs
          pop  es
          int  10h
 ; End set VGA russian font

        call setbase1000
        mov  ax,0xb800
        mov  es,ax
        mov  di,0
        mov  si,d80x25-0x10000
        mov  cx,80*25
        mov  ah,1*16+15
       dfl1:
        cld
        lodsb
        stosw
        loop dfl1

        call setbase1000


; SAY HI TO USER

        mov     si,linef2-0x10000
        call    printplain
        mov     si,version-0x10000
        call    print


; TEST FOR 386+

        pushf
        pop     ax
        mov     dx,ax
        xor     ax,0x4000
        push    ax
        popf
        pushf
        pop     ax
        and     ax,0x4000
        and     dx,0x4000
        cmp     ax,dx
        jnz     cpugood
        mov     si,not386-0x10000
        call    print
        jmp     $
     cpugood:

; RESET 32 BIT SELECTORS/REGISTERS/SELECTORS

        mov  ax,0x1000
        mov  es,ax
        mov  ds,ax

        mov  ax,0x2000
        mov  ss,ax
        mov  esp,0xffff

        xor  eax,eax
        xor  ebx,ebx
        xor  ecx,ecx
        xor  edx,edx
        xor  esi,esi
        xor  edi,edi
        xor  ebp,ebp



; FLUSH 8042 KEYBOARD CONTROLLER

;// mike.dld [
 ;       mov     al,0xED
 ;       out     0x60,al
 ;       or      cx,-1
 ;     @@:
 ;       in      al,0x64
 ;       test    al,2
 ;       jz      @f
 ;       loop    @b
 ;     @@:
 ;       mov     al,0
 ;       out     0x60,al
 ;       or      cx,-1
 ;     @@:
 ;       in      al,0x64
 ;       test    al,2
 ;       jz      @f
 ;       loop    @b
 ;     @@:
;// mike.dld ]

;       mov     ecx,10000
;      fl1:
;       in      al,0x64
;       loop    fl1
;       test    al,1
;       jz      fl2
;       in      al,0x60
;       jmp     fl1
;      fl2:

;****************************************************************
; The function is modified Mario79
;*****************************************************************
; wait_kbd:        ; variant 1
;       mov      cx,2500h  ;задержка порядка 10 мсек
; test_kbd:
;       in       al,64h    ;читаем состояние клавиатуры
;       test     al,2      ;проверка бита готовности
;       loopnz   test_kbd

    mov   al,0xf6         ; Сброс клавиатуры, разрешить сканирование
    out   0x60,al
        mov     cx,0
wait_loop:       ; variant 2
; reading state of port of 8042 controller
        in      al,64h 
        and     al,00000010b  ; ready flag
; wait until 8042 controller is ready 
        loopnz  wait_loop

; DISPLAY VESA INFORMATION

        mov     ax,0x0
        mov     es,ax
        mov     ax,0x4f00
        mov     di,0xa000
        int     0x10
        cmp     ax,0x004f
        je      vesaok2
        mov     dx,0x1000
        mov     es,dx
        mov     si,novesa-0x10000
        call    print
        mov     ax,16
        jmp     novesafound
       vesaok2:
        mov     ax,[es:di+4]
        mov     dx,ax
        add     ax,'0'*256+'0'
        push    word 0x1000
        pop     es
        mov     [es:vervesa+vervesa_off-0x10000], ah
        mov     [es:vervesa+vervesa_off+2-0x10000], al
        ; ivan 24/11/2004 begin
        ;push    ax
        ; ivan 24/11/2004 end
        mov     si,vervesa-0x10000
        call    print
      novesafound:
        call    setbase1000
        ; ivan 24/11/2004 begin
        ;pop     bx
        ; ivan 24/11/2004 end


; ASK GRAPHICS MODE

        movzx   eax,byte [es:preboot_graph-0x10000]
        cmp     eax,0
        jne     pre_graph
        mov     si,gr_mode-0x10000
        call    printplain
       gml0:
        mov     ebx,0x0A01
        call    getkey
       pre_graph:
        cmp     eax,1
        jl      sgml1
        cmp     eax,8
        jg      sgml1
        mov     si,ax
        sub     si,1
        shl     si,4
        add     si,gr_table-0x10000
        mov     bx,[es:si+0]
        mov     cx,[es:si+4]
        mov     dx,[es:si+8]
        jmp     gml10
       sgml1:
        cmp     al,9
        jnz     gml00
        mov     bx,0x13
        mov     cx,640
        mov     dx,480
        push    word 0x0
        pop     es
        mov     [es:0x9000],byte 32
        mov     dword [es:0x9018],0x800000
        push    word 0x1000
        pop     es
        jmp     gml10
       gml00:
        cmp     al,0xa
        jnz     gml02
        mov     bx,0x12
        mov     cx,640
        mov     dx,480
        push    word 0x0
        pop     es
        mov     [es:0x9000],byte 32
        mov     dword [es:0x9018],0x800000
        push    word 0x1000
        pop     es
        jmp     gml10
       gml02:
        jmp     gml0
       gr_table:
        dd      0x112+0100000000000000b ,  640 ,  480 , 0
        dd      0x115+0100000000000000b ,  800 ,  600 , 0
        dd      0x118+0100000000000000b , 1024 ,  768 , 0
        dd      0x11B+0100000000000000b , 1280 , 1024 , 0
        dd      0x112 ,  640 , 480 , 0
        dd      0x115 ,  800 , 600 , 0
        dd      0x118 , 1024 , 768 , 0
        dd      0x11B , 1280 ,1024 , 0
       gml10:
        push    word 0x0000
        pop     es
        mov     [es:0x9008],bx
        mov     [es:0x900A],cx
        mov     [es:0x900C],dx
        push    word 0x1000
        pop     es
        mov     ax,32
        cmp     bx,0x13
        je      nov
        cmp     bx,0x12
        je      nov


; USE DEFAULTS OR PROBE

; bx - mode : cx - x size : dx - y size

     ;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        movzx   ax,[es:preboot_gprobe-0x10000]
        test    ax,ax
        jne     pre_probe
     ;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

        test    bx,0100000000000000b
        jz      noprobe

        mov     si,probetext-0x10000
        call    printplain
        push    bx
        mov     ebx,0x0201
        call    getkey
        pop     bx

     pre_probe:
        cmp     ax,1
        je      noprobe

        push    cx dx

        mov     bx,0x100

     newprobe:

        inc     bx
        cmp     bx,0x17f
        jne     probemore

        mov     si,prnotfnd-0x10000
        call    printplain

        jmp     $

     probemore:

        mov     ax,0x4f01
        mov     cx,bx
        and     cx,0xfff
        push    word 0x0000
        pop     es
        mov     di,0xa000
        int     0x10

        mov     eax,[es:di]       ; lfb ?
        test    eax,10000000b
        jz      newprobe

        mov     eax,[es:di+0x12]  ; x size ?
        cmp     ax,word [esp+2]
        jne     newprobe

        mov     eax,[es:di+0x14]  ; y size ?
        cmp     ax,dx
        jne     newprobe

        movzx   eax,byte [es:di+0x19]
        cmp     eax,32 ;24
        jb      newprobe

        push    word 0x0000        ; save probed mode
        pop     es
        add     bx,0100000000000000b
        mov     [es:0x9008],bx
        push    word 0x1000
        pop     es

        push    bx

        mov     si,prid-0x10000
        call    printplain

        pop     bx dx cx

     noprobe:


; FIND VESA 2.0 LFB & BPP

        mov     ax,0x4f01
        mov     cx,bx
        and     cx,0xfff
        push    word 0x0000
        pop     es
        mov     di,0xa000
        int     0x10
        ; LFB
        mov     ecx,[es:di+0x28]
        mov     [es:0x9018],ecx
        ; BPP
        movzx   ax,byte [es:di+0x19]
        mov     [es:0x9000],ax
        ; ---- vbe voodoo
        BytesPerScanLine equ 0x10
        push ax
        mov ax, [es:di+BytesPerScanLine]
        mov [es:0x9001],ax
        pop ax
        ; -----
       nov:
        cmp     ax,24
        jnz     nbpp24
        mov     si,bt24-0x10000
        jmp     bppl
       nbpp24:
        cmp     ax,32
        jnz     nbpp32
        mov     si,bt32-0x10000
        jmp     bppl
       nbpp32:
        mov     si,btns-0x10000
        call    print
        jmp     $
       bppl:
        call    printplain


; FIND VESA 1.2 PM BANK SWITCH ADDRESS

        mov     ax,0x4f0A
        mov     bx,0x0
        int     0x10
        xor     eax,eax
        xor     ebx,ebx
        mov     ax,es
        shl     eax,4
        mov     bx,di
        add     eax,ebx
        xor     ebx,ebx
        mov     bx,[es:di]
        add     eax,ebx
        push    word 0x0
        pop     es
        mov     [es:0x9014],eax
        push    word 0x1000
        pop     es



; GRAPHICS ACCELERATION

        mov     al, [es:preboot_mtrr-0x10000]
        test    al,al
        jne     pre_mtrr
        mov     si,gr_acc-0x10000
        call    printplain
        mov     ebx,0x0201
        call    getkey
      pre_mtrr:
        push    word 0x0000
        pop     es
        mov     [es:0x901C],al
        push    word 0x1000
        pop     es
        mov     si,linef-0x10000
        call    printplain


; VRR_M USE

        mov     al,[es:preboot_vrrm-0x10000]
        test    al,al
        jne     pre_vrrm
        mov     si,vrrmprint-0x10000
        call    print
        mov     ebx,0x0301
        call    getkey
      pre_vrrm:
        push    word 0x0000
        pop     es
        mov     [es:0x9030],al
        push    word 0x1000
        pop     es
        mov     si,linef2-0x10000
        call    printplain


; MEMORY MODEL

;        movzx   eax,byte [es:preboot_memory-0x10000]
;        cmp     eax,0
;        jne     pre_mem
;;;;;;;;;;;;;;;;;;;;;;;;;
; mario79 - memory size ;
;;;;;;;;;;;;;;;;;;;;;;;;;
;           mov ax,0E801h
;;;           xor bx,bx    ; thanks to Alexei for bugfix [18.07.2004]
;           xor cx, cx
;           xor dx, dx
;           int 0x15
;           movzx ebx, dx ;bx
;           movzx eax, cx ;ax
;           shl   ebx,6   ; перевод в килобайты (x64)
;           add   eax,ebx
;           add eax, 1000h ;440h
;           cmp eax,40000h ; 256?
;           jge mem_256_z
;           cmp eax,20000h ; 128?
;           jge mem_128_z
;           cmp eax,10000h ; 64?
;           jge mem_64_z
;           cmp eax,8000h ; 32?
;           jge mem_32_z
;           jmp mem_16_z
;
;mem_256_z: mov     si,memokz256-0x10000
;           call    printplain
;           mov eax,5
;           jmp pre_mem
;mem_128_z: mov     si,memokz128-0x10000
;           call    printplain
;           mov eax,4
;           jmp pre_mem
;mem_64_z:  mov     si,memokz64-0x10000
;           call    printplain
;           mov eax,3
;           jmp pre_mem
;mem_32_z:  mov     si,memokz32-0x10000
;           call    printplain
;           mov eax,2
;           jmp pre_mem
;mem_16_z:  mov     si,memokz16-0x10000
;           call    printplain
;           mov eax,1
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;      pre_mem:
;        push    word 0x0000
;        pop     es
;        mov     [es:0x9030],al
;        push    word 0x1000
;        pop     es
;        mov     si,linef-0x10000
;        call    printplain




; DIRECT WRITE TO LFB, PAGING DISABLED

;        movzx   eax,byte [es:preboot_lfb-0x10000]
;        mov     eax,1                             ; paging disabled
;        cmp     eax,0
;        jne     pre_lfb
;        mov     si,gr_direct-0x10000
;        call    printplain
;        mov     ebx,0x0201
;        call    getkey
;      pre_lfb:
;        push    word 0x0000
;        pop     es
;        mov     [es:0x901E],al
;        mov     ax,0x1000
;        mov     es,ax
;        mov     si,linef-0x10000
;        call    printplain
        push    0
        pop     es
        mov     [es:0x901E],byte 1
        push    0x1000
        pop     es



; BOOT DEVICE

        movzx   eax,byte [es:preboot_device-0x10000]
        cmp     eax,0
        jne     pre_device
        mov     si,bdev-0x10000
        call    printplain
        mov     ebx,0x0301
        call    getkey
      pre_device:
        dec     al
        mov     [es:boot_dev-0x10000],al
        mov     si,linef-0x10000
        call    printplain



; READ DISKETTE TO MEMORY

        cmp     [boot_dev-0x10000],0
        jne     no_sys_on_floppy
        mov     si,diskload-0x10000
        call    print
        mov     ax,0x0000               ; reset drive
        mov     dx,0x0000
        int     0x13
        mov     cx,0x0001               ; startcyl,startsector
        mov     dx,0x0000               ; starthead,drive
        push    word 80*2               ; read no of sect
       reads:
        pusha
        xor     si,si
       newread:
        push    word 0x0
        pop     es
        mov     bx,0xa000               ; es:bx -> data area
        mov     ax,0x0200+18            ; read, no of sectors to read
        int     0x13
        cmp     ah,0
        jz      goodread
        add     si,1
        cmp     si,10
        jnz     newread
        mov     si,badsect-0x10000
        call    printplain
        jmp     $
       goodread:
        ; move -> 1mb
        mov     si,movedesc-0x10000
        push    word 0x1000
        pop     es
        mov     cx,256*18
        mov     ah,0x87
        int     0x15

        cmp     ah,0                  ; was the move successfull ?
        je      goodmove
        mov     dx,0x3f2              ; floppy motor off
        mov     al,0
        out     dx,al
        mov     si,memmovefailed-0x10000
        call    print
        jmp     $
      goodmove:

        mov     eax,[es:movedesc-0x10000+0x18+2]
        add     eax,512*18
        mov     [es:movedesc-0x10000+0x18+2],eax
        popa
        inc     dh
        cmp     dh,2
        jnz     bb2
        mov     dh,0
        inc     ch
        pusha                        ; print prosentage
        push    word 0x1000
        pop     es
        xor     eax,eax  ; 5
        mov     al,ch
        shr     eax,2
        and     eax,1
        mov     ebx,5
        mul     bx
        add     al,48
        mov     [es:pros+1-0x10000],al
        xor     eax,eax  ; 10
        mov     al,ch
        shr     eax,3
        add     al,48
        mov     [es:pros-0x10000],al
        mov     si,pros-0x10000
        call    printplain
        popa
       bb2:
        pop     ax
        dec     ax
        push    ax
        cmp     ax,0
        jnz     rs
        jmp     readdone
       rs:
        jmp     reads
       movedesc:
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0

        db      0xff,0xff,0x0,0xa0,0x00,0x93,0x0,0x0
        db      0xff,0xff,0x0,0x00,0x10,0x93,0x0,0x0

        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
       readdone:
        pop     ax
        mov     dx,0x3f2             ; floppy motor off
        mov     al,0
        out     dx,al
        mov     si,backspace-0x10000
        call    printplain
        call    printplain
        mov     si,okt-0x10000
        call    printplain
       no_sys_on_floppy:
        mov     ax,0x0000               ; reset drive
        mov     dx,0x0000
        int     0x13
       mov dx,0x3f2 ; floppy motor off
       mov al,0
       out dx,al       


; PAGE TABLE

        push    word 0x0000
        pop     es
        mov     ecx,[es:0x9018]
        push    ecx

        map_mem equ 64                ; amount of memory to map

        mov     bx,0x6000
        mov     es,bx                   ; [es:di] = 6000:0
        xor     edi,edi
        mov     ecx,256*map_mem         ; Map (mapmem) M
        mov     eax,7
        cld
       pt2:
        cmp     ecx,256*(map_mem-8)     ; 8 M map to LFB
        jnz     pt3
        pop     eax
        add     eax,7
       pt3:
        cmp     ecx,256*(map_mem-12)    ; 12 M back to linear = physical
        jnz     pt4
        mov     eax,12*0x100000 + 7
       pt4:
        stosd
        add     eax,4096
        loop    pt2
        
        mov     bx,0x7100
        mov     es,bx
        xor     edi,edi
        mov     eax,8*0x100000+7
        mov     ecx,256*4
      pt5:
        stosd
        add     eax,0x1000
        loop    pt5
        
; 4 KB PAGE DIRECTORY

        mov     bx , 0x7F00
        mov     es , bx                 ; [es:di] = 7000:0
        xor     edi, edi
        mov     ecx, 64 / 4
        mov     eax, 0x60007            ; for 0 M
        cld
      pd4k:
        stosd
        add     eax, 0x1000
        loop    pd4k
        mov     dword [es:0x800],0x71007   ;map region 0x80000000-0x803FFFFF to 0x800000-0xCFFFFF
        xor     esi,esi
        mov     edi,second_base_address shr 20
        mov     ecx,64/4
        mov     bx,0x7F00
        mov     ds,bx
        rep     movsd
        mov     bx,0x1000
        mov     ds,bx
       
        mov     eax, 0x7F000 +8+16      ; Page directory and enable caches
        mov     cr3, eax

; SET GRAPHICS

        mov     dx,0x0000
        mov     es,dx
        mov     bx,[es:0x9008]
        mov     ax,bx                ; vga & 320x200
        cmp     ax,0x13
        je      setgr
        cmp     ax,0x12
        je      setgr
        mov     ax,0x4f02            ; Vesa
       setgr:
        int     0x10
        cmp     ah,0
        jz      gmok
        mov     si,fatalsel-0x10000
        call    print
        jmp     $
        
       gmok:
        mov     dx,0x1000
        mov     es,dx

; set mode 0x12 graphics registers:

        cmp     bx,0x12
        jne     gmok2

        mov     al,0x05
        mov     dx,0x03ce
        out     dx,al      ; select GDC mode register
        mov     al,0x02
        mov     dx,0x03cf
        out     dx,al      ; set write mode 2

        mov     al,0x02
        mov     dx,0x03c4
        out     dx,al      ; select VGA sequencer map mask register
        mov     al,0x0f
        mov     dx,0x03c5
        out     dx,al      ; set mask for all planes 0-3

        mov     al,0x08
        mov     dx,0x03ce
        out     dx,al      ; select GDC bit mask register
                           ; for writes to 0x03cf

       gmok2:
        mov     dx,0x1000
        mov     es,dx

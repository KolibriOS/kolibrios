;#**************************
;#*    X-0-X    =  MAIN()
;#************************** 21/10/2007
cB     equ cBLACK
cW     equ cWHITE

macro main_loop
{

; ???????? ?⠭??????? 梥?? ????
;========================================
mov eax, 48
mov ebx, 3
mov ecx, colors
mov edx, 40
int 0x40



; ?㭪??? ????ᮢ?? ????
;========================================
draw_window:
        mov     eax,12                ; ?㭪??? 12:     ᮮ????? ???⥬?
        mov     ebx,1                 ; ????㭪??? 1,  ??砫? ??????ᮢ??
        int     0x40
                                      ; =====  ???? !!!
        mov     eax,0                 ; ?㭪??? 0: ??।????? ? ?뢥??? ????
        mov     ebx,100*65536+250     ; [x ??砫????] *65536 + [x ࠧ???]
        mov     ecx,ebx  ;,100*65536+250     ; [y ??砫????] *65536 + [y ࠧ???]
        mov     edx, 0x34FFFFFF

        mov     esi, 0x808899AA       ; 梥? ????????? RRGGBB
        mov     edi, caption          ; ?????????
        int     0x40

;-----------------------------------#-----------------------------------------
; SCENE OF GAME - PLAY YARD - BOARD - DESK - PLAYING FIELD
;---------------
 playing_field                        ;macro in macs.inc
;-------------------------------------------------------------------------------
;                              ----??????----
;-------------------------------------------------------------------------------
;buttons:
        mov     eax,8                 ;
        mov     ebx,200*65536+35      ;
        mov     ecx,15*65536+20       ;
        mov     edx,200               ;== NEW GAME ==
        mov     esi, [colors+0x18]
        int     0x40
;--------------------------------------------
        mov     eax,4                 ;  ???????? ⥪?? ?? ?????? NEW
        mov     ebx,210*65536+20
        mov     ecx,[colors+0x1C]
        mov     edx,textend
        mov     esi,3
        int     0x40
 endbut:
        mov     eax,12
        mov     ebx,2
        int     0x40
;============================= draw_window: END
 event_wait:
        mov     eax,23        ;==???????==
        mov     ebx,100       ;????? ᥪ㭤?
        int     0x40
;-------------------------------------------------------------------------------
; ⨯ ᮡ???? ????????? ? eax, ????? ?஢??塞, ????? ᮡ?⨥ ?ந??諮
;-------------------------------------------------------------------------------
        cmp     eax,1        ; ?????? ?? ??????ᮢ???
        je      redraw
        cmp     eax,2        ; ?????? ??????? ???????????
        je      key
        cmp     eax,3        ; ?????? ?????? ? ???? ?ணࠬ???
        je      button
        cmp     eax,6        ; mouse - click or move?
        je      MOUSE
        jmp     event_wait   ; ??????頥??? ? ??砫? 横?? ???????? ᮡ?⨩
                             ; ?᫨ ᮡ?⨩  ???
;-------------------------------------------------------------------------------
;  ??ࠡ?⪠ ᮡ?⨩
;-------------------------------------------------------------------------------
redraw:
        call    draw_window  ; ???뢠?? ?㭪??? draw_window ?
        jmp     event_wait   ; ??????頥??? ????? ? 横?? ????????
key:
        mov     eax,2        ; ????뢠?? ??? ????⮩ ???????. ??????饭 ? ah.
        int     0x40         ; ??????? ?????? ???? ?????⠭? ??? ????⪨
        jmp     event_wait   ; ??????? ? event_wait
button:
        mov     eax,17       ; ????뢠?? ??????䨪???? ????⮩ ??????
        int     0x40         ; ??????饭 ? ah.
;-------------------------------------------------------------------------------
; ᬮ?ਬ, ????? ?????? ?뫠 ?????? ? ᮮ⢥??????騬 ??ࠧ?? ॠ???㥬.
;-------------------------------------------------------------------------------
        cmp     ah, 1          ; ?????? ? id=1 ("???????")?
        jne     noclose        ;
  clos0:                       ;
        mov     eax,-1         ; ?㭪??? -1: ????????? ?ணࠬ??
        int     0x40           ;
  noclose:                     ; ?????? id=200( "New" )
        mov dword[pox],0
        mov dword[pox+4],0     ;clear memory
        mov dword[pox_],0      ;rest and other stuff
        mov word[poxl],0       ; bits of pole X
        mov word[pool],0       ; bits of pole O
        ;mov byte[fill],0       ; ?????塞 ????稪 室??
        ;mov byte[xwin],0       ; flag - x winner
        ;mov byte[owin],0       ; flag - o winner
        jmp     redraw         ; ???⮥ ??஢?? ???? ==>>
;-------------------------------------------------------------------------------
; ᬮ?ਬ, ??? ?뫠 ?????? ????
;-------------------------------------------------------------------------------
 MOUSE:
        mov eax,37      ; ?????..
        mov ebx,2
        int 0x40
        cmp eax,0       ; ..??祣? ?? ?????? - ????? ᮡ?⨩
        je  event_wait
                        ;?????? ?????? ????
        mov eax,37      ; cursor coords
        mov ebx,1
        int 0x40        ;eax=x*65536+y
        mov [px],eax    ;[px][py]  -  ???न???? ⪭?⮣? ?????
	
	cmp [px],word 40
        jb  event_wait	
	
	cmp [px],word 200
        ja  event_wait	
	
	cmp [py],word 40
        jb  event_wait	
	
	cmp [py],word 200
        ja  event_wait	

          xor eax,eax
     g11:                      ;1st cell======
          cmp word[px],92
          ja g12               ;>92
          cmp word[py],92
          ja g12               ;>92

          ;mov dword[idx],0     ;N=(0..8)
          jmp puts

     g12:                      ;2nd cell========
          inc al
          cmp word[py],145
          ja g13               ;>142
          cmp word[px],92
          ja g13               ;>92
          ;mov dword[idx],1     ;N=(0..8)
          jmp puts
     g13:                      ;3=========
          inc al
          cmp word[px],92
          ja g21               ;>92
          ;mov dword[idx],2     ;N=(0..8)
          jmp puts
     g21:                      ;4========
          inc al
          cmp word[px],145
          ja g22
          cmp word[py],92
          ja g22
          ;mov dword[idx],3     ;N=(0..8)
          jmp puts
     g22:                      ;5=======
          inc al
          cmp word[px],145
          ja g23
          cmp word[py],145
          ja g23
          ;mov dword[idx],4     ;N=(0..8)
          jmp puts
     g23:                      ;6=======
          inc al
          cmp word[px],142
          ja g31               ;>142
          ;mov dword[idx],5     ;N=(0..8)
          jmp puts
     g31:                      ;7=======
          inc al
          cmp word[py],92
          ja g32
          ;mov dword[idx],6     ;N=(0..8)
          jmp puts
     g32:                      ;8======
          inc al
          cmp word[py],145
          ja g33
          ;mov dword[idx],7     ;N=(0..8)
          jmp puts
     g33:                      ;9======
          inc al
          ;mov dword[idx],8     ;N=(0..8)

   puts:                            ;㦥 ??।????? ??????
           push eax
           pop dword[idx]
          mov esi,dword[idx]        ;
          cmp byte[pox+esi],0       ;???? ᢮?????? (0-᢮?????? ????)
          jne event_wait            ;??????頥??? ????? ? 横?? ????????

          shl esi,2                 ;*4
          mov eax,dword[pole+esi]
          mov dword[px],eax           ;x coord from tabl ->px
          ;mov ax,word[pole+esi+2]
          ;mov word[py],ax           ;y coord from tabl ->py

          call x_put                ;"X"

          mov esi,dword[idx]        ;esi - cell number  =0..8
          mov byte[pox+esi],'x'
          inc byte[fill]            ;turn+1

          xor eax,eax               ;eax=0
          inc eax                   ;bit=1       eax=1
          mov ecx,dword[idx]        ;  esi=n
          shl  ax,cl                ; <<n-bit   bit[n]=1
          or word[poxl],ax          ;set bit in [poxl]

          call x_win                ;?஢?ઠ ?먣???? ?
          cmp byte[xwin],1          ;䫠? ?????? ??⠭??????
          jne del5                  ;??? - ????᪮? ?? delay

        mov     eax,4                 ; ?㭪 4: ???????? ⥪?? 'WINNER!'
        mov     ebx,18*65536+215      ; [x ???] *65536 + [y ???]
        mov     ecx, [colors+0x20]    ; 梥? ⥪??? RRGGBB
        mov     edx,texts             ; 㪠??⥫? ?? ??砫? ⥪???
        mov     esi,textend-texts     ; ????? ⥪??? ? ??????
        int     0x40                  ;
        jmp   event_wait              ; ??????頥??? ????? ? 横?? ????????

;===========================================================
 del5:    mov eax,5                 ;=== delay 0,5 sec ===
          mov ebx,50
          int 0x40

          cmp byte[fill],8    ;??? ???? ?????? - 室??? ???㤠
          jnb event_wait      ;??????頥??? ????? ? 横?? ???????? - ?????.

   rnew:                      ;
          call rndnext              ; ???砩??? ??᫮ ? eax
          and eax, 0111b            ; eax = 0..7
          mov esi,eax               ; eax->esi
          call rndnext              ; ???砩??? ??᫮ ? eax
          and eax, 01b              ; eax= 0..1
          add esi,eax               ; esi+=eax
                                    ;esi=0..8 - index
          cmp byte[pox+esi],0       ;???砩??? ???? ᢮??????
          jne rnew                  ;??? - ????? ???砩??? ??᫮

          cmp byte[pox+4],0         ;業?? ᢮??????
          jne @F                    ;??? - ???砩??? 室 (㦥 ? esi)
          mov esi,4                 ;??  - 室?? ? 業?? (4) ?????? ????.
     @@:
          mov dword[idx],esi        ;???࠭?? ? idx
                                    ;esi=0..8 - index
          call o_com                ;?஢?ઠ 24? ???????権 XX0
          call x_com                ;?஢?ઠ 24? ???????権 00*

          mov esi,dword[idx]        ;idx-?????? ???⪨(0..8) ?㤠 室
          shl esi,2                 ; esi*4 - ???? ???⮨? ?? ????ᥩ ?? dword
          mov eax,dword[pole+esi]     ;
          mov dword[px],eax           ;x coord from tabl ->px
        ;  mov ax,word[pole+esi+2]   ;
        ;  mov word[py],ax           ;y coord from tabl ->py
          call o_put                ; ???㥬 ????? 'O'

          mov esi,dword[idx]
          mov byte[pox+esi],'o'
          inc byte[fill]            ;turn++

          xor eax,eax               ;eax=0
          inc eax                   ;bit=1       eax=1
          mov ecx,dword[idx]        ;  ecx=n
          shl  ax,cl                ; <<n-bit   bit[n]=1
          or word[pool],ax          ;set bit in [pool]

          call o_win
          cmp byte[owin],1
          jne event_wait

        mov     eax,4                 ; ?㭪 4: ???????? ⥪?? 'LOOSER!'
        mov     ebx,48*65536+215      ; [x ???] *65536 + [y ???]
        mov     ecx, [colors+0x20]    ; 梥? ⥪??? RRGGBB
        mov     edx,lootxt            ; 㪠??⥫? ?? ??砫? ⥪???
        mov     esi,text-lootxt       ; ????? ⥪??? ? ??????
        int     0x40                  ;
        jmp   event_wait              ; ??????頥??? ????? ? 横?? ????????
;===========================================--
;
}



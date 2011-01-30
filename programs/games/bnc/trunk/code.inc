macro CODE_SECTION

{
;+-----------------------------------+     Uses:eax,ebx,edx
;|-------Random Number Generator-----|---------------------------|
;+-----------------------------------+
 rndseed:                       ;
        mov     eax,26          ; func 26 subfunc 9
        mov     ebx,9           ;
        int     0x40            ;
        mov dword[seed],eax     ; system time from boot
 random9:                       ; RND=[0..9]
        mov  eax,3              ;
        int  0x40               ; get current time
        shr  eax,16             ; seconds in al
        add  eax,dword[seed]    ;
        imul eax,0x15a4e35      ; random formula rv=rv*0x15a4e35+1
        inc  eax                ; return (rv&0x7FFFFFFF)%N
        mov  dword[seed],eax    ;
        and  eax,0x07FFFFFFF    ;
        mov  ebx,10             ; N=10
        xor  edx,edx            ;
        div  bx                 ; eax/ebx= int[result]+rest
        add dl,'0'              ; DL='0'..'9' - rest
        ret                     ;
;=================================================
   rnew:                        ; New Random Secret Number
          xor   esi,esi         ;----------------------------
      LoopBegin  lb1,0          ;
      rn: call random9          ; DL=RND('0'..'9')
          xor  edi,edi          ;
     lb2: cmp byte[nA+edi],dl   ;
          je  rn                ;
          inc edi               ;
          cmp  edi,4            ; 4 digit
          jne  lb2              ;
          mov byte[nA+esi],dl   ;nA - secret number without double digits
          inc esi               ;nA - 4 bytes of non-repeated digits '0'..'9'
      LoopEnd lb1,4             ;
          ret                   ;
;-------------------------------------
 numStr:  inc  dword[turn]      ; turn++
          mov  ebx,dword[turn]  ;
          push ebx              ;
          pop  ecx              ; ebx=ecx=turn
          add  cl,'0'           ;
          add  ch,':'           ;
          dec  ebx              ; -1
          shl  ebx,4            ; *16
          mov  word[tStr+ebx],cx; begin string by turn number +':'
          ret
;-------------------------------------
 paint_list:
      LoopBegin  lbList,0
          mov  ebx,ecx
          shl  ebx,4            ; *16
          mov  edx,tStr
          add  edx,ebx
          add  ebx,25*65536     ;x
          add  ebx,45           ;y
          mov  eax,4       ;func 4
          mov  ecx,cTxt    ;color
          mov  esi,16      ;line length
          int  0x40
      LoopEnd    lbList,9  ;
          ret
;-INPUT------------------------------------ KEYBOARD INPUT -----------
  input:                     ;ah - key code
          mov byte[char],ah  ;char<-ah
          cmp  ah,13         ;13- enter
          jne   @f
          jmp _enter
     @@:
          cmp  ah,8         ;8 - del backSpace <=|
          jne   @f
          jmp _back
     @@:

          cmp  ah,'0'        ; '0'..'9'
          jae  @f
          ret
     @@:  cmp  ah,'9'
          jbe  @f
          ret                ; if not - return subroutine
     @@:
;------------------------pressed:0123456789-----------
          cmp byte[posn],4
          je @f                 ;full line - do nothing
          call ptrStr           ;esi - pointer at tStr
          xor eax,eax
          add al,byte[posn]
          add esi,eax           ;+posn
          mov al,byte[char]
          mov byte[esi+3],al    ;esi+3 = place 4 digits
          inc byte[posn]        ;next digit
      @@: ret
;------------------------
  _enter:
          cmp byte[posn],4     ; Enter Turn
          je  @f
          ret
     @@:
          call check           ; check input for difference
          cmp eax,0
          je @f
          ret
     @@:
          call bcalculate       ; calc b+c
          call    numStr        ;  'N:'   next string number
          mov byte[posn],0      ; new line
          ret
;------------------------
  _back:  cmp byte[posn],0      ; empty input - do nothing
          je  @f
          dec byte[posn]
          call ptrStr           ;esi = pointer at tStr
          xor eax,eax
          add al,byte[posn]     ; byte of esi  +posn
          add esi,eax           ;+posn
          mov byte[esi+3],' '
     @@:  ret
;===========================================--end - input -
  ptrStr:                      ; esi=tStr+turn*16
          mov esi,dword[turn]
          shl esi,4
          add esi,tStr
          ret
;------------------------
 clears:  mov ecx,160
   clr1:  mov byte[tStr+ecx],0;' '
          loop clr1
          mov byte[tStr],0;' '
          mov  byte[posn],0
          mov  dword[turn],0
          ret
;-------------------------CHECK---Input--[1234]-------
  check:                        ; check for similar digits (equal)
          call ptrStr           ; esi ...pointer
          ;mov esi,dword[turn]
          ;shl esi,4
          ;add esi,tStr
          xor  eax,eax
          xor  edi,edi          ; edi - counter 0..3
          xor  ecx,ecx
   lb4ch: mov dl,byte[esi+ecx+3]
          cmp byte[esi+edi+3],dl
          jne @f
          inc eax               ; =equivalent +1
       @@:
          inc  edi              ;
          cmp  edi,4            ; cycle ,4 digits
          jne  lb4ch            ;
          xor  edi,edi          ; edi - counter 0..3

        inc   ecx
        cmp   ecx,4
        jne   lb4ch

          sub eax,4              ; must be 4, if more - doubles present
          ret

 bcalculate: ;------------------; calc b:+c:
          call ptrStr           ; esi ...
          add esi,3
          ;inc esi
          ;inc esi               ;+3
          ;inc esi               ; ...- pointer  to Line
    LoopBegin  lbBu1,0             ;bull count cycle  ecx=0..3
          mov  bl,byte[nA+ecx]
          mov  dl,byte[esi+ecx]
          cmp  bl,dl
          jne  @f
          inc  byte[nBul]
     @@:
    LoopEnd    lbBu1,4

          mov ax,'B:'             ; output Bulls
          mov word[esi+6],ax
          mov al,byte[nBul]
          add al,'0'
          mov byte[esi+8],al

        LoopBegin  lbCo2,0       ;Cows count cycle ecx=0..3
          mov  dl,byte[esi+ecx]
          xor  edi,edi          ; cycle edi=0..3
    lb3:  mov bl,byte[nA+edi]
          cmp  bl,dl
          jne  @f
          inc  byte[nCow]       ;Cows+Bulls
     @@:  inc edi               ;
          cmp  edi,4            ;
          jne  lb3              ;
        LoopEnd    lbCo2,4      ;endcycle ecx

          mov ax,'C:'         ; output Cows
          mov word[esi+10],ax
          mov al,byte[nCow]
          sub al,byte[nBul]
          add al,'0'
          mov byte[esi+12],al
          mov dword[nBul],0   ;nCow=posn=char=0
          ret
;--------------------------------------
}


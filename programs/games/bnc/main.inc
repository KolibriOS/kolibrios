;#***************
;#*  BNC.MAIN()
;#***************
cTxt   equ  0x10000000
cTxtW  equ  0x10FFFFFF

macro main_loop
{
draw_window:
        redraw_begin
;------------------------------
        ;--- MainWindow
        mcall 0,100*65536+250,100*65536+250,0x02FFFFFF ,0x808899AA ,0xFFAAcc
        ;--- MainWindow TitleBar
        mcall 4,8*65536+8,0x00224422,header,20
        ;--- Button - Close MainWindow
        mcall 8,230*65536+12, 5*65536+12, 100 ,0xFF6611
        ;--- Button - "New" ...
        mcall 8,190*65536+50,25*65536+15, 200 ,0xAA66CC
        mcall 4,205*65536+30,cTxtW,new,3
        ;--- Button - "Turn" ...
        ;mcall 8,190*65536+50,50*65536+15, 150 ,0x116611
;-----------------------------1234-secret-number---show
  ;     mcall 4, 50*65536+30,cTxt ,nA,4
;-----------------------------9-lines-
        call    paint_list
        redraw_end
;=============================
 event_wait:
        mcall 23,100
;-------------------------------------------------------------------------------
;    event in eax   -  and  what will happen
;-------------------------------------------------------------------------------
     EventIs EV_REDRAW,draw_window    ; redraw window
     EventIs EV_KEY,key               ; pressed key
     EventIs EV_BUTTON,button         ; one clicks button
        jmp     event_wait   ; returns at point of waiting for
;-------------------------------------------------------------------------------
;     Events handler
;-------------------------------------------------------------------------------
key:                         ;
        mov     eax,2        ; key code in ah.
        int     0x40         ;
        cmp     ah,27        ; ESC?
        je      close        ; exit on ESC

        call    input        ; input 0-9,Enter,<_BackSpace

        jmp    draw_window ; event_wait   ;
button:                      ;
        mov     eax,17       ; button ID in ah
        int     0x40         ;
;-------------------------------------------------------------------------------
;     Buttons handler
;-------------------------------------------------------------------------------
        cmp     ah,100        ; id=100("Close window")?
        jne     noclose       ;
  close:                      ;
        mov     eax,-1        ; CLOSE all
        int     0x40          ;
  noclose:                    ;
        cmp     ah,150        ; id=150 "Turn"
        je      @F
        call    rnew          ; id=200 "New"      ->nA

        call    clears
        jmp     draw_window    ;event_wait

  @@:
        call    paint_list
        jmp     event_wait
;
}


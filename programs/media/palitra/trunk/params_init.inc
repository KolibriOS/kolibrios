    mov  edi,params	    ; 㪠??⥫? ?? ??ப?
    call str_len	    ; ????砥? ????? ??ப?
    cmp  eax,0		    ; ??ࠢ?????? ????? ? ?????
    je	 red		    ; ?᫨ ??ࠬ??஢ ??? ?? ?室??
    call    _read_params    ; ????? ??⠥? ??ࠬ???? 梥?? (?祭? ?ਢ?)
    xor  eax,eax	    ; ?????塞 ॣ????
    mov  al,byte [params]   ; ??⠥? ??ࠬ??? ????
    cmp  al,'N' 	    ; ?ࠢ?????? ? normal mode
    jne _no_normalmode	    ; ?᫨ ??? ?? ?室?? ?? ?஢???? ??????
    mov  [runmode],1	    ; ?᫨ ?? ?? ??⠭???????? ???
    jmp  red		    ; ???? ?? ????ᮢ??
  _no_normalmode:	    ; ?᫨ ?? normal mode ??
    cmp  al,'H' 	    ; ???????? hidden mode
    jne _no_hiddenmode	    ; ?᫨ ?? ?? ?? ?室?? ??????
    mov  [runmode],2	    ; ?᫨ ?? ?? ??⠭???????? ??? ??? ?? (????)
    call set_background     ; ???塞 䮭
    jmp  bexit		    ; ? ????뢠?? ?ண?
  _no_hiddenmode:	    ; ?᫨ ?? hidden mode
    cmp  al,'B' 	    ; ???????? noise mode
    jne _no_noisemode	    ; ?᫨ ?? ?? ?? ?室?? ??????
    mov  [runmode],2	    ; ?᫨ ?? ?? ??⠭???????? ??? ??? ?? (????)
    call set_background2    ; ???塞 䮭
    jmp  bexit		    ; ? ????뢠?? ?ண?
  _no_noisemode:	    ; ?᫨ ?? noise mode
    cmp  al,'D' 	    ; ???????? ??? color dialog mode
    jne _no_dialogmode	    ; ?᫨ ??? ⠪? ?? ?? ?? ?室??
    mov  [runmode],3	    ; ????? 㪠?뢠?? ??? ??? ⠪? ??
    jmp  red		    ; ? ?室?? ?? ?믮??????
  _no_dialogmode:	    ; ?᫨ ??? ?? ???? ?? ?????? ?????
    mov  [runmode],1	    ; ?? ?????뢠?? ?? ??䮫?
    jmp  red		    ; ? ???㥬
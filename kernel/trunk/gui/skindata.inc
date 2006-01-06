;
;   WINDOW SKIN for MenuetOS
;   ivan-yar@bk.ru
;

iglobal
  _skinh          dd      22

  _skinleftw      dd      10

  _skinbase       dd      10
  _skinbasew      dd      8

  _skinoper       dd      -39 ;-21
  _skinoperw      dd      39   ;21

  _buttonCx       dd      5            ; close
  _buttonCy       dd      2
  _buttonCw       dd      15
  _buttonCh       dd      17

  _buttonMx       dd      23            ; minimize
  _buttonMy       dd      2
  _buttonMw       dd      15
  _buttonMh       dd      17 
  

  _colorframe     dd      0x3a6cb6  ;0x586E93
  _colorborder    dd      0x00081D
  _coloroutborder dd      0x00081D

  _fileleft       db      'LEFT.BMP     '
  _filebase       db      'BASE.BMP     '
  _fileoper       db      'OPER.BMP     '
  _fileleft_1     db      'LEFT_1.BMP   '
  _filebase_1     db      'BASE_1.BMP   '
  _fileoper_1     db      'OPER_1.BMP   '
endg

uglobal
  _refoper        dd      0
  _refbase        dd      0
  _refleft        dd      0
  _skinleft       dd      0
;  _skinwinw       dd      0
  _refoper_1        dd      0
  _refbase_1        dd      0
  _refleft_1        dd      0
  _skinleft_1       dd      0
endg


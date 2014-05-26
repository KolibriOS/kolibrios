alt_general:

;     db   ' ',27
;     db   ' @ $  {[]}\ ',8,9
;     db   '            ',13
;     db   '             ',0,'           ',0,'4',0,' '
;     db   '             ',180,178,184,'6',176,'7'
;     db   179,'8',181,177,183,185,182
;     db   'ABCD',255,'FGHIJKLMNOPQRSTUVWXYZ'
;     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
;     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
;     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'

en_keymap:

     db   '6',27
     db   '1234567890-=',8,9
     db   'qwertyuiop[]',13
     db   '~asdfghjkl;',39,96,0,'\zxcvbnm,./',0,'45 '
     db   '@234567890123',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'AB<D',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'

en_keymap_shift:

     db   '6',27
     db   '!@#$%^&*()_+',8,9
     db   'QWERTYUIOP{}',13
     db   '~ASDFGHJKL:"~',0,'|ZXCVBNM<>?',0,'45 '
     db   '@234567890123',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'AB>D',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'





fr_keymap:

     db   '6',27
     db   '&é"',39,'(-ç_ìê)=',8,9
     db   'azertyuiop',136,'$',13
     db   '~qsdfghjklmí',0,0,'*wxcvbn,;:!',0,'45 '
     db   '@234567890123',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'AB<D',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'

fr_keymap_shift:

     db   '6',27
     db   '1234567890+',8,9
     db   'AZERTYUIOPïî',13
     db   '~QSDFGHJKLM%',0,'ñWXCVBN?./',0,'45 '
     db   '@234567890123',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'AB>D',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'

fr_keymap_alt_gr:

     db   '6',27
     db   28,'~#{[|ò\^@]}',8,9
     db   'azertyuiop^$',13
     db   '~qsdfghjklmí',0,0,'*wxcvbn,;:!',0,'45 '
     db   '@234567890123',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'AB<D',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'




fi_keymap:

     db   '6',27
     db   '1234567890+[',8,9
     db   'qwertyuiop',192,'~',13
     db   '~asdfghjkl',194,193,'1',0,39,'zxcvbnm,.-',0,'45 '
     db   '@234567890123',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'AB<D',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'

fi_keymap_shift:

     db   '6',27
     db   '!"#è%&/()=?]',8,9
     db   'QWERTYUIOP',200,'~',13
     db   '~ASDFGHJKL',202,201,'1',0,'*ZXCVBNM;:_',0,'45 '
     db   '@234567890123',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'AB>D',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'





ge_keymap:

     db   '6',27
     db   '1234567890?[',8,9
     db   'qwertzuiop',203,'~',13
     db   '~asdfghjkl',194,193,'1',0,39,'yxcvbnm,.-',0,'45 '
     db   '@234567890123',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'AB<D',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'

ge_keymap_shift:

     db   '6',27
     db   '!"#$%&/()=',197,']',8,9
     db   'QWERTZUIOP',195,'~',13
     db   '~ASDFGHJKL',202,201,'1',0,'*YXCVBNM;:_',0,'45 '
     db   '@234567890123',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'AB>D',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'




ru_keymap:

     db   '6',27
     db   '1234567890-=',8,9
     db   '©Ê„™•≠£ËÈßÂÍ',13
     db   0,"‰Î¢†Ø‡Æ´§¶Ì"
     db   0xf1, '-/'
     db   "ÔÁ·¨®‚Ï°Ó",'.-','45 '
     db   '@234567890123',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'AB<D',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'

ru_keymap_shift:

     db   '6',27
     db   '!"N;%:?*()_+',8,0
     db   "âñìäÖçÉòôáïö",13
     db   0,"îõÇÄèêéãÑÜù"
     db   0xf0, '-\'
     db   "üóëåàíúÅû",',-','45 '
     db   '@234567890123',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'AB>D',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'




et_keymap:

     db   '6',27
     db   '1234567890+¥',8,9
     db   'qwertyuiop¸ı',13
     db   '~asdfghjklˆ‰','1',0,39,'zxcvbnm,.-',0,'45 '
     db   '@234567890123',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'AB<D',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'

et_keymap_shift:

     db   '6',27
     db   '!"#§%&/()=?`',8,9
     db   'QWERTYUIOP‹’',13
     db   '~ASDFGHJKL÷ƒ','1',0,'*ZXCVBNM;:_',0,'45 '
     db   '@234567890123',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'AB>D',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'



be_keymap:

     db   '6',27
     db   '&È"',39,'(ßË!Á‡)-',8,9
     db   'azertyuiop',136,'$',13
     db   '~qsdfghjklm˘≤',0,'µwxcvbn,;:=',0,'45 '
     db   '@2345-7+90123',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'AB<D',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'

be_keymap_shift:

     db   '6',27
     db   '1234567890∞_',8,9
     db   'AZERTYUIOP®*',13
     db   '~QSDFGHJKLM%≥',0,'£WXCVBN?./+',0,'45 '
     db   '@234567890123',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'AB>D',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'

be_keymap_alt_gr:

     db   '6',27
     db   '|@#',0,0,'^',0,0,'{}',0,0,8,9
     db   'azÄrtyuiop[]',13
     db   '~qsdfghjklm¥',0,0,'`wxcvbn',0,0,0,'~',0,'45 '
     db   '@234567890123',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'AB\D',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     
it_keymap:
  
     db   '6',27
     db   '1234567890',39,0,8,9
     db   'qwertyuiop',0,'+',13
     db   '~asdfghjkl',0,0,0,0,'<zxcvbnm,.-',0,'45 '
     db   '@234567890123',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'AB<D',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     
it_keymap_shift:

     db   '6',27
     db   '!"',0,'$%&/()=?^',8,9
     db   'QWERTYUIOP',0,'*',13
     db   '~ASDFGHJKL',0,0,0,0,'>ZXCVBNM;:_',0,'45 '
     db   '@234567890123',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'AB>D',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     
it_keymap_alt_gr:

     db   '6',27
     db   172,185,178,179,188,189,172,'{[]}',96,'~',8,9
     db   'qwertyuiop[]',13
     db   '~asdfghjkl@#',0,'<zxcvbnm;:_',0,'45 '
     db   '@234567890123',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'AB<D',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     
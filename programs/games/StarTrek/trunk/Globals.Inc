; --------------------------------------------------------------------------
; Global variables
; --------------------------------------------------------------------------

glb_GameThreadBusy BOOL 1
glb_Instance HINSTANCE 0
glb_Allocator HANDLE 0
glb_MainWnd HWND 0
glb_Font HFONT 0
glb_FPU_Int32 INT32 0
glb_Score INT32 0
glb_CharSize rb SIZE.size
glb_pConsole PTConsole 0
glb_pCmdBuf PTCmdBuf 0
glb_pRawMsgBuffer PCHAR 0
glb_pMsgItems PPCHAR 0
glb_NumMsgItems COUNT 0
glb_pGameData PTGameData 0
glb_pCommon PTCommon 0
glb_pMersenne PTRandom 0
glb_pLog PVOID 0
glb_MOVED BYTE 0
glb_LogEnabled BYTE 0
glb_bEmExit BYTE 0
glb_AttrCondition BYTE ATTR_COND_GREEN, ATTR_COND_RED, ATTR_COND_YELLOW, ATTR_COND_GREEN

glb_ObjectMap BYTE CHAR_ROMULAN, CHAR_KLINGON, CHAR_COMMANDER, \
    CHAR_SCOM, CHAR_ASTERISK, CHAR_PLANET, CHAR_STARBASE, \
    CHAR_BLACK_HOLE, CHAR_THOLIAN

glb_Plaque BYTE PLAQUE_END_OF_LINE,PLAQUE_END_OF_LINE,\
    PLAQUE_END_OF_LINE,PLAQUE_END_OF_LINE,\
    64,' ',50,'E',PLAQUE_END_OF_LINE,\
    38,' ',3,'E',22,' ',PLAQUE_STRING,6,'E  : :',41,' ',1,':',2,' ',1,'E',PLAQUE_END_OF_LINE,\
    36,' ',2,'E',3,' ',3,'E',19,' ',PLAQUE_STRING,6,'E  : :',19,' ',\
      PLAQUE_STRING,8,'NCC-1701',14,' ',PLAQUE_STRING,4,':  E',PLAQUE_END_OF_LINE,\
    20,' ',16,'E',8,' ',15,'E',4,' ',PLAQUE_STRING,6,'E  : :',41,' ',PLAQUE_STRING,3,': E',\
      PLAQUE_END_OF_LINE,\
    21,' ',1,'E',37,' ',1,'E',4,' ',48,'E',PLAQUE_END_OF_LINE,\
    22,' ',9,'E',15,' ',13,'E',17,' ',PLAQUE_STRING,4,'E  E',PLAQUE_END_OF_LINE,\
    31,' ',7,'E',3,' ',5,'E',4,' ',1,'E',10,' ',1,'E',14,' ',PLAQUE_STRING,4,'E  E',PLAQUE_END_OF_LINE,\
    38,' ',3,'E',11,' ',1,'E',10,' ',1,'E',12,' ',PLAQUE_STRING,4,'E  E',PLAQUE_END_OF_LINE,\
    55,' ',1,'E',9,' ',1,'E',10,' ',PLAQUE_STRING,4,'E  E',PLAQUE_END_OF_LINE,\
    57,' ',13,'E',6,' ',PLAQUE_STRING,4,'E  E',PLAQUE_END_OF_LINE,\
    54,' ',3,'E',1,' ',1,':',11,' ',7,'E',2,' ',8,'E',PLAQUE_END_OF_LINE,\
    52,' ',1,':',1,'E',4,' ',1,':',17,' ',4,'E',7,' ',1,'E',PLAQUE_END_OF_LINE,\
    51,' ',PLAQUE_STRING,8,')-E   -:',5,'-',23,' ',1,'E',PLAQUE_END_OF_LINE,\
    52,' ',1,':',1,'E',4,' ',1,':',28,' ',1,'E',PLAQUE_END_OF_LINE,\
    54,' ',2,'E',2,' ',1,':',20,' ',8,'E',PLAQUE_END_OF_LINE,\
    56,' ',23,'E',PLAQUE_END_OF_LINE,\
    PLAQUE_END_OF_LINE,PLAQUE_END_OF_LINE,PLAQUE_END_OF_LINE,\
    55,' ',PLAQUE_STRING,19,'U. S. S. ENTERPRISE',PLAQUE_END_OF_LINE,\
    PLAQUE_END_OF_LINE,PLAQUE_END_OF_LINE,PLAQUE_END_OF_LINE,PLAQUE_END_OF_LINE,\
    34,' ',PLAQUE_STRING,43,'FOR DEMONSTRATING OUTSTANDING ABILITY AS A ',\
      PLAQUE_STRING,16,'STARSHIP CAPTAIN',PLAQUE_END_OF_LINE,PLAQUE_END_OF_LINE,\
    48,' ',PLAQUE_STRING,32,'STARFLEET COMMAND BESTOWS TO YOU',PLAQUE_END_OF_LINE,\
    PLAQUE_END_OF_LINE,PLAQUE_INSERT_NAME,PLAQUE_END_OF_LINE,\
    59,' ',PLAQUE_STRING,11,'THE RANK OF',PLAQUE_END_OF_LINE,PLAQUE_END_OF_LINE,\
    55,' ',PLAQUE_STRING,20,'"COMMODORE EMERITUS"',PLAQUE_END_OF_LINE,PLAQUE_END_OF_LINE,\
    58,' ',PLAQUE_INSERT_LEVEL,PLAQUE_STRING,6,' LEVEL',PLAQUE_END_OF_LINE,PLAQUE_END_OF_LINE,\
    54,' ',PLAQUE_STRING,12,'THIS DAY OF ',PLAQUE_INSERT_DATE,PLAQUE_END_OF_LINE,\
    PLAQUE_END_OF_LINE,56,' ',PLAQUE_STRING,13,'YOUR SCORE:  ',PLAQUE_INSERT_SCORE,\
    PLAQUE_END_OF_LINE,PLAQUE_END_OF_LINE,\
    52,' ',PLAQUE_STRING,24,'KLINGONS PER STARDATE:  ',PLAQUE_INSERT_KLRATE,\
    PLAQUE_END_OF_LINE,PLAQUE_END_OF_ALL

align 2
str_MainWndClass du 'SST32-FASM',0
str_MainWndTitle du 'STAR TREK',0
str_FontFace du 'Courier',0
str_CharSizeText du 'Mj',0
str_SetDirFailed du 'Unable to Set Current Directory. Please check permissions.',0
str_MsgFileName du 'MsgTable.txt',0
str_LogFileName du 'STARTREK.LOG',0
str_FileMask du '*.TRK',0
str_THOLIANX db 'THOLIANX',0
str_TRK db '.TRK',0
str_CmdListGames db '?.TRK',0
str_EmSaveGame db 'EMSAVE.TRK',0
str_PlaqueFile db 'PLAQUE.TXT',0

align 16
glb_dbl_2e31 DOUBLE 2147483648.0
glb_dbl_2e32 DOUBLE 4294967296.0
glb_dbl_Ten DOUBLE 10.0
glb_dbl_FpuPopper DOUBLE 0.0
glb_dbl_0dot0625 DOUBLE 0.0625
glb_dbl_0dot08 DOUBLE 0.08
glb_dbl_0dot095 DOUBLE 0.095
glb_dbl_0dot1 DOUBLE 0.1
glb_dbl_0dot01 DOUBLE 0.01
glb_dbl_0dot001 DOUBLE 0.001
glb_dbl_0dot2 DOUBLE 0.2
glb_dbl_0dot02 DOUBLE 0.02
glb_dbl_0dot00001 DOUBLE 0.00001
glb_dbl_0dot2777 DOUBLE 0.2777
glb_dbl_0dot1777777777 DOUBLE 0.1777777777
glb_dbl_0dot15 DOUBLE 0.15
glb_dbl_0dot015 DOUBLE 0.015
glb_dbl_0dot5 DOUBLE 0.5
glb_dbl_0dot5235998 DOUBLE 0.5235998
glb_dbl_0dot5235988 DOUBLE 0.5235988
glb_dbl_0dot3 DOUBLE 0.3
glb_dbl_0dot03 DOUBLE 0.03
glb_dbl_0dot333 DOUBLE 0.3333333333
glb_dbl_1dot5 DOUBLE 1.5
glb_dbl_10dot5 DOUBLE 10.5
glb_dbl_2dot5 DOUBLE 2.5
glb_dbl_1dot2 DOUBLE 1.2
glb_dbl_1dot23 DOUBLE 1.23
glb_dbl_1dot4142 DOUBLE 1.4142
glb_dbl_1dot57079633 DOUBLE 1.57079633
glb_dbl_1dot01 DOUBLE 1.01
glb_dbl_1dot90985932 DOUBLE 1.90985932
glb_dbl_0dot04 DOUBLE 0.04
glb_dbl_0dot4 DOUBLE 0.4
glb_dbl_0dot45 DOUBLE 0.45
glb_dbl_0dot49 DOUBLE 0.49
glb_dbl_0dot05 DOUBLE 0.05
glb_dbl_0dot005 DOUBLE 0.005
glb_dbl_0dot00038 DOUBLE 0.00038
glb_dbl_0dot006 DOUBLE 0.006
glb_dbl_0dot75 DOUBLE 0.75
glb_dbl_0dot77 DOUBLE 0.77
glb_dbl_0dot65 DOUBLE 0.65
glb_dbl_0dot6 DOUBLE 0.6
glb_dbl_0dot89 DOUBLE 0.89
glb_dbl_0dot8 DOUBLE 0.8
glb_dbl_0dot9 DOUBLE 0.9
glb_dbl_0dot99 DOUBLE 0.99
glb_dbl_9dot99 DOUBLE 9.99
glb_dbl_0dot98 DOUBLE 0.98
glb_dbl_0dot995 DOUBLE 0.995
glb_dbl_DOCKFAC DOUBLE 0.25
glb_dbl_100 DOUBLE 100.0
glb_dbl_200 DOUBLE 200.0
glb_dbl_270 DOUBLE 270.0
glb_dbl_66dot666 DOUBLE 66.666666666
glb_dbl_2 DOUBLE 2.0
glb_dbl_20 DOUBLE 20.0
glb_dbl_3 DOUBLE 3.0
glb_dbl_31 DOUBLE 31.0
glb_dbl_39dot95 DOUBLE 39.95
glb_dbl_4 DOUBLE 4.0
glb_dbl_4dot5 DOUBLE 4.5
glb_dbl_5 DOUBLE 5.0
glb_dbl_6 DOUBLE 6.0
glb_dbl_8 DOUBLE 8.0
glb_dbl_25 DOUBLE 25.0
glb_dbl_125 DOUBLE 125.0
glb_dbl_50 DOUBLE 50.0
glb_dbl_150 DOUBLE 150.0
glb_dbl_950 DOUBLE 950.0
glb_dbl_7 DOUBLE 7.0
glb_dbl_7dot5 DOUBLE 7.5
glb_dbl_9 DOUBLE 9.0
glb_dbl_7dot853981634 DOUBLE 7.853981634
glb_dbl_15 DOUBLE 15.0
glb_dbl_12 DOUBLE 12.0
glb_dbl_1175 DOUBLE 1175.0
glb_dbl_75 DOUBLE 75.0
glb_dbl_300 DOUBLE 300.0
glb_dbl_30 DOUBLE 30.0
glb_dbl_3000 DOUBLE 3000.0
glb_dbl_400 DOUBLE 400.0
glb_dbl_500 DOUBLE 500.0
glb_dbl_600 DOUBLE 600.0
glb_dbl_700 DOUBLE 700.0
glb_dbl_800 DOUBLE 800.0
glb_dbl_7200 DOUBLE 7200.0
glb_dbl_450 DOUBLE 450.0
glb_dbl_1000 DOUBLE 1000.0
glb_dbl_5000 DOUBLE 5000.0
glb_dbl_2500 DOUBLE 2500.0
glb_dbl_1400 DOUBLE 1400.0
glb_dbl_1500 DOUBLE 1500.0
glb_dbl_1250 DOUBLE 1250.0
glb_dbl_1E38 DOUBLE 1e38
glb_dbl_3E_neg_5 DOUBLE 3e-5
glb_dbl_KlingonsPerDate DOUBLE 0.0

; --- EOF ---

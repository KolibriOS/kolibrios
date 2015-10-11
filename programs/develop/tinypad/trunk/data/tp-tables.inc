
align 4

key0 db \
  0x00,0x00,0x02,0x03,0x04,0x05,0x06,0x07,\
  0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x00,0x00,\
  0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,\
  0x18,0x19,0x1A,0x1B,0x00,0x00,0x1E,0x1F,\
  0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,\
  0x28,0x29,0x00,0x2B,0x2C,0x2D,0x2E,0x2F,\
  0x30,0x31,0x32,0x33,0x34,0x35,0x00,0x00,\
  0x00,0x39,0x00,0x00,0x00,0x00,0x00,0x00
  times 12*16 db 0x00

align 4

numpad_table_off dd \
  0x00000135,0x06000035, \ ; /
  0x00000037,0x07010009, \ ; *
  0x0000004A,0x0600000C, \ ; -
  0x00000047,0x07000147, \ ; KP Home
  0x00000048,0x07000148, \ ; KP Up
  0x00000049,0x07000149, \ ; KP PageUp
  0x0000004B,0x0700014B, \ ; KP Left
  0x0000004C,0x0700014C, \ ; KP Begin
  0x0000004D,0x0700014D, \ ; KP Right
  0x0000004E,0x0701000D, \ ; +
  0x0000004F,0x0700014F, \ ; KP End
  0x00000050,0x07000150, \ ; KP Down
  0x00000051,0x07000151, \ ; KP PageDown
  0x00000052,0x07000152, \ ; KP Insert
  0x00000053,0x07000153, \ ; KP Delete
  0x0000011C,0x0700001C, \ ; KP Enter
  0

align 4

numpad_table_on dd \
  0x00000135,0x06000035, \ ; /
  0x00000037,0x07010009, \ ; *
  0x0000004A,0x0600000C, \ ; -
  0x00000047,0x06000008, \ ; 7
  0x00000048,0x06000009, \ ; 8
  0x00000049,0x0600000A, \ ; 9
  0x0000004B,0x06000005, \ ; 4
  0x0000004C,0x06000006, \ ; 5
  0x0000004D,0x06000007, \ ; 6
  0x0000004E,0x0701000D, \ ; +
  0x0000004F,0x06000002, \ ; 1
  0x00000050,0x06000003, \ ; 2
  0x00000051,0x06000004, \ ; 3
  0x00000052,0x0600000B, \ ; 0
  0x00000053,0x06000034, \ ; .
  0x0000011C,0x0700001C, \ ; Enter
  0

align 4

accel_table_main dd		    \
  0x0000000E,key.bkspace	   ,\ ; BackSpace
  0x0000000F,key.tab		   ,\ ; Tab
  0x0000001C,key.return 	   ,\ ; Return
  0x0000003D,key.f3		   ,\ ; F3
  0x00000043,key.f9		   ,\ ; F9
  0x00000044,key.f10		   ,\ ; F10
  0x00000147,key.home		   ,\ ; Home
  0x00000148,key.up		   ,\ ; Up
  0x00000149,key.pgup		   ,\ ; PageUp
  0x0000014B,key.left		   ,\ ; Left
  0x0000014D,key.right		   ,\ ; Right
  0x0000014F,key.end		   ,\ ; End
  0x00000150,key.down		   ,\ ; Down
  0x00000151,key.pgdn		   ,\ ; PageDown
  0x00000152,key.ins		   ,\ ; Insert
  0x00000153,key.del		   ,\ ; Delete
  0x00010043,key.shift_f9	   ,\ ; Shift+F9
  0x00010147,key.shift_home	   ,\ ; Shift+Home
  0x00010148,key.shift_up	   ,\ ; Shift+Up
  0x00010149,key.shift_pgup	   ,\ ; Shift+PageUp
  0x0001014B,key.shift_left	   ,\ ; Shift+Left
  0x0001014D,key.shift_right	   ,\ ; Shift+Right
  0x0001014F,key.shift_end	   ,\ ; Shift+End
  0x00010150,key.shift_down	   ,\ ; Shift+Down
  0x00010151,key.shift_pgdn	   ,\ ; Shift+PageDown
  0x00010153,key.del		   ,\ ; Shift+Delete
  0x0002000C,key.ctrl_minus	   ,\ ; Ctrl+Minus
  0x0002000D,key.ctrl_plus	   ,\ ; Ctrl+Plus
  0x0002000F,key.ctrl_tab	   ,\ ; Ctrl+Tab
  0x00020015,key.ctrl_y 	   ,\ ; Ctrl+Y
  0x00020018,key.ctrl_o 	   ,\ ; Ctrl+O
  0x0002001E,key.ctrl_a 	   ,\ ; Ctrl+A
  0x0002001F,key.ctrl_s 	   ,\ ; Ctrl+S
  0x00020020,key.ctrl_d 	   ,\ ; Ctrl+D
  0x00020021,key.ctrl_f 	   ,\ ; Ctrl+F
  0x00020022,key.ctrl_g 	   ,\ ; Ctrl+G
  0x00020023,key.ctrl_h 	   ,\ ; Ctrl+H
\;0x00020026,key.ctrl_l            ,\ ; Ctrl+L
  0x0002002D,key.ctrl_x 	   ,\ ; Ctrl+X
  0x0002002E,key.ctrl_c 	   ,\ ; Ctrl+C
  0x0002002F,key.ctrl_v 	   ,\ ; Ctrl+V
  0x00020031,key.ctrl_n 	   ,\ ; Ctrl+N
  0x0002003E,key.ctrl_f4	   ,\ ; Ctrl+F4
  0x00020043,key.ctrl_f9	   ,\ ; Ctrl+F9
  0x00020147,key.ctrl_home	   ,\ ; Ctrl+Home
\;0x00020148,key.ctrl_up           ,\ ; Ctrl+Up
  0x00020149,key.ctrl_pgup	   ,\ ; Ctrl+PageUp
  0x0002014B,key.ctrl_left	   ,\ ; Ctrl+Left
  0x0002014D,key.ctrl_right	   ,\ ; Ctrl+Right
  0x0002014F,key.ctrl_end	   ,\ ; Ctrl+End
\;0x00020150,key.ctrl_down         ,\ ; Ctrl+Down
  0x00020151,key.ctrl_pgdn	   ,\ ; Ctrl+PageDown
  0x00020153,key.del		   ,\ ; Ctrl+Del
  0x0003000D,key.ctrl_plus	   ,\ ; Ctrl+Plus
  0x0003000F,key.shift_ctrl_tab    ,\ ; Shift+Ctrl+Tab
  0x0003001F,key.shift_ctrl_s	   ,\ ; Shift+Ctrl+S
  0x00030147,key.shift_ctrl_home   ,\ ; Shift+Ctrl+Home
\;0x00030148,key.shift_ctrl_up     ,\ ; Shift+Ctrl+Up
  0x00030149,key.shift_ctrl_pgup   ,\ ; Shift+Ctrl+PageUp
  0x0003014B,key.shift_ctrl_left   ,\ ; Shift+Ctrl+Left
  0x0003014D,key.shift_ctrl_right  ,\ ; Shift+Ctrl+Right
  0x0003014F,key.shift_ctrl_end    ,\ ; Shift+Ctrl+End
\;0x00030150,key.shift_ctrl_down   ,\ ; Shift+Ctrl+Down
  0x00030151,key.shift_ctrl_pgdn   ,\ ; Shift+Ctrl+PageDown
  0x0004002D,key.alt_x		   ,\ ; Alt+X
  0

align 4

accel_table_textbox dd		    \
\;0x00000001,key.tb.escape         ,\ ; Escape
  0x0000000E,key.tb.bkspace	   ,\ ; BackSpace
\;0x0000000F,key.tb.tab            ,\ ; Tab
\;0x0000001C,key.tb.return         ,\ ; Return
  0x00000147,key.tb.home	   ,\ ; Home
  0x0000014B,key.tb.left	   ,\ ; Left
  0x0000014D,key.tb.right	   ,\ ; Right
  0x0000014F,key.tb.end 	   ,\ ; End
  0x00000153,key.tb.del 	   ,\ ; Delete
  0x00010147,key.tb.shift_home	   ,\ ; Shift+Home
  0x0001014B,key.tb.shift_left	   ,\ ; Shift+Left
  0x0001014D,key.tb.shift_right    ,\ ; Shift+Right
  0x0001014F,key.tb.shift_end	   ,\ ; Shift+End
  0x00010153,key.tb.del 	   ,\ ; Shift+Del
  0

align 4

accel_table2 dd 	  \
  1    ,key.alt_x	 ,\
  2    ,key.ctrl_f4	 ,\
  'VSL',btn.vscroll_up	 ,\
  'VSG',btn.vscroll_down ,\
  'HSL',btn.hscroll_up	 ,\
  'HSG',btn.hscroll_down ,\
  'TBL',btn.tabctl_right ,\
  'TBG',btn.tabctl_left  ,\
  0

align 4

accel_table2_botdlg dd	   \
  1    ,key.alt_x	  ,\
  20001,btn.bot.cancel	  ,\
  20002,btn.bot.opensave  ,\
  20003,btn.bot.find	  ,\
  20004,btn.bot.appearance,\
  20007,btn.bot.no	  ,\
  21001,btn.bot.tabpos	  ,\
  0

align 4

add_table:
  times $1A db -$20
  times $25 db -$00
  times $10 db -$20
  times $30 db -$00
  times $10 db -$50
  times $04 db -$00,-$01
  times $08 db -$00

align 4

table.866.1251 db \
  $C0,$C1,$C2,$C3,$C4,$C5,$C6,$C7 , $C8,$C9,$CA,$CB,$CC,$CD,$CE,$CF,\
  $D0,$D1,$D2,$D3,$D4,$D5,$D6,$D7 , $D8,$D9,$DA,$DB,$DC,$DD,$DE,$DF,\
  $E0,$E1,$E2,$E3,$E4,$E5,$E6,$E7 , $E8,$E9,$EA,$EB,$EC,$ED,$EE,$EF,\
  $00,$00,$00,$00,$00,$00,$00,$00 , $00,$00,$00,$00,$00,$00,$00,$00,\
  $00,$00,$00,$00,$00,$00,$00,$00 , $00,$00,$00,$00,$00,$00,$00,$00,\
  $00,$00,$00,$00,$00,$00,$00,$00 , $00,$00,$00,$00,$00,$00,$00,$00,\
  $F0,$F1,$F2,$F3,$F4,$F5,$F6,$F7 , $F8,$F9,$FA,$FB,$FC,$FD,$FE,$FF,\
  $A8,$B8,$AA,$BA,$AF,$BF,$A1,$A2 , $B0,$95,$B7,$00,$B9,$A4,$00,$00
table.1251.866 db \
  $00,$00,$00,$00,$00,$00,$00,$00 , $00,$00,$00,$00,$00,$00,$00,$00,\
  $00,$00,$00,$00,$00,$F9,$00,$00 , $00,$00,$00,$00,$00,$00,$00,$00,\
  $00,$F6,$F7,$00,$FD,$00,$00,$00 , $F0,$00,$F2,$00,$00,$00,$00,$F4,\
  $F8,$00,$00,$00,$00,$00,$00,$FA , $F1,$FC,$F3,$00,$00,$00,$00,$F5,\
  $80,$81,$82,$83,$84,$85,$86,$87 , $88,$89,$8A,$8B,$8C,$8D,$8E,$8F,\
  $90,$91,$92,$93,$94,$95,$96,$97 , $98,$99,$9A,$9B,$9C,$9D,$9E,$9F,\
  $A0,$A1,$A2,$A3,$A4,$A5,$A6,$A7 , $A8,$A9,$AA,$AB,$AC,$AD,$AE,$AF,\
  $E0,$E1,$E2,$E3,$E4,$E5,$E6,$E7 , $E8,$E9,$EA,$EB,$EC,$ED,$EE,$EF
table.866.koi db \
  $E1,$E2,$F7,$E7,$E4,$E5,$F6,$FA , $E9,$EA,$EB,$EC,$ED,$EE,$EF,$F0,\
  $F2,$F3,$F4,$F5,$E6,$E8,$E3,$FE , $FB,$FD,$FF,$F9,$F8,$FC,$E0,$F1,\
  $C1,$C2,$D7,$C7,$C4,$C5,$D6,$DA , $C9,$CA,$CB,$CC,$CD,$CE,$CF,$D0,\
  $90,$00,$00,$81,$87,$00,$00,$00 , $00,$00,$00,$00,$00,$00,$00,$83,\
  $84,$89,$88,$86,$80,$8A,$00,$00 , $00,$00,$00,$00,$00,$00,$00,$00,\
  $00,$00,$00,$00,$00,$00,$00,$00 , $00,$85,$82,$8D,$8C,$8E,$8F,$8B,\
  $D2,$D3,$D4,$D5,$C6,$C8,$C3,$DE , $DB,$DD,$DF,$D9,$D8,$DC,$C0,$D1,\
  $B3,$A3,$B4,$A4,$B7,$A7,$BE,$AE , $00,$95,$9E,$00,$B0,$9F,$00,$A0
table.koi.866 db \
  $C4,$B3,$DA,$BF,$C0,$D9,$C3,$B4 , $C2,$C1,$C5,$DF,$DC,$DB,$DD,$DE,\
  $B0,$00,$00,$00,$00,$F9,$00,$00 , $00,$00,$00,$00,$00,$00,$FA,$FD,\
  $FF,$00,$00,$F1,$F3,$00,$00,$F5 , $00,$00,$00,$00,$00,$00,$F7,$00,\
  $FC,$00,$00,$F0,$F2,$00,$00,$F4 , $00,$00,$00,$00,$00,$00,$F6,$00,\
  $EE,$A0,$A1,$E6,$A4,$A5,$E4,$A3 , $E5,$A8,$A9,$AA,$AB,$AC,$AD,$AE,\
  $AF,$EF,$E0,$E1,$E2,$E3,$A6,$A2 , $EC,$EB,$A7,$E8,$ED,$E9,$E7,$EA,\
  $9E,$80,$81,$96,$84,$85,$94,$83 , $95,$88,$89,$8A,$8B,$8C,$8D,$8E,\
  $8F,$9F,$90,$91,$92,$93,$86,$82 , $9C,$9B,$87,$98,$9D,$99,$97,$9A
table.1251.koi db \
  $B1,$B2,$00,$A2,$00,$00,$00,$00 , $00,$00,$B9,$00,$BA,$BC,$BB,$BF,\
  $A1,$91,$92,$93,$94,$95,$96,$97 , $00,$99,$A9,$00,$AA,$AC,$AB,$AF,\
  $A0,$BE,$AE,$B8,$9F,$BD,$00,$00 , $B3,$98,$B4,$9D,$00,$00,$9C,$B7,\
  $00,$00,$B6,$A6,$AD,$00,$00,$9E , $A3,$B0,$A4,$9B,$A8,$B5,$A5,$A7,\
  $E1,$E2,$F7,$E7,$E4,$E5,$F6,$FA , $E9,$EA,$EB,$EC,$ED,$EE,$EF,$F0,\
  $F2,$F3,$F4,$F5,$E6,$E8,$E3,$FE , $FB,$FD,$FF,$F9,$F8,$FC,$E0,$F1,\
  $C1,$C2,$D7,$C7,$C4,$C5,$D6,$DA , $C9,$CA,$CB,$CC,$CD,$CE,$CF,$D0,\
  $D2,$D3,$D4,$D5,$C6,$C8,$C3,$DE , $DB,$DD,$DF,$D9,$D8,$DC,$C0,$D1
table.koi.1251 db \
  $00,$00,$00,$00,$00,$00,$00,$00 , $00,$00,$00,$00,$00,$00,$00,$00,\
  $00,$91,$92,$93,$94,$95,$96,$97 , $00,$99,$00,$BB,$AE,$AB,$B7,$A4,\
  $A0,$90,$83,$B8,$BA,$BE,$B3,$BF , $BC,$9A,$9C,$9E,$9D,$B4,$A2,$9F,\
  $B9,$80,$81,$A8,$AA,$BD,$B2,$AF , $A3,$8A,$8C,$8E,$8D,$A5,$A1,$8F,\
  $FE,$E0,$E1,$F6,$E4,$E5,$F4,$E3 , $F5,$E8,$E9,$EA,$EB,$EC,$ED,$EE,\
  $EF,$FF,$F0,$F1,$F2,$F3,$E6,$E2 , $FC,$FB,$E7,$F8,$FD,$F9,$F7,$FA,\
  $DE,$C0,$C1,$D6,$C4,$C5,$D4,$C3 , $D5,$C8,$C9,$CA,$CB,$CC,$CD,$CE,\
  $CF,$DF,$D0,$D1,$D2,$D3,$C6,$C2 , $DC,$DB,$C7,$D8,$DD,$D9,$D7,$DA

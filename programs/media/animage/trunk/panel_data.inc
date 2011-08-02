;**********************************************************
;------------------TEXT DATA-------------------------------
;**********************************************************
name_of_program      db 'ANIMAGE graphics editor v1.3',0
mouse_pos_x	     db 'X='
mouse_pos_y	     db 'Y='
new_text1	     db 'PICTURE SIZE X'
new_text2	     db 'PICTURE SIZE Y'
ok_text 	     db 'OK'

panel_text:	     dd 15,24,4
		     db 'File'
		     dd 65,24,4
		     db 'Edit'
		     dd 115,24,11
		     db 'Instruments'
		     dd 235,24,7
		     db 'Palette'
		     dd 315,24,7
		     db 'Filters'
		     dd 395,24,9
		     db 'Animation'
		     dd 495,24,5
		     db 'Help '
menu_text_en:
		     dd menu_text7
		     dd menu_text6
		     dd menu_text5
		     dd menu_text4
		     dd menu_text3
		     dd menu_text2
		     dd menu_text1
		     dd 0,0,0

menu_rectangles:     dd 485,36,120,100
		     dd 385,36,120,100
		     dd 295,36,120,100
		     dd 225,36,120,100
		     dd 105,36,120,100
		     dd 55,36,120,100;20
		     dd 5,36,120,100


menu_text1:
		     dd 15,43,3
		     db 'New'
		     dd 15,63,4
		     db 'Open'
		     dd 15,83,4
		     db 'Save'
		     dd 15,103,7
		     db 'Save as'
		     dd 15,123,4
		     db 'Exit'

menu_text2:
		     dd 65,43,4
		     db 'Undo'
		     dd 65,63,4
		     db 'Copy'
		     dd 65,83,5
		     db 'Paste'
		     dd 65,103,3
		     db 'Cut'
		     dd 65,123,13
		     db 'Clear screen'
		     ;dd 95,143,12
		     ;db 'Allocate all'
menu_text3:
		     dd 115,43,3
		     db 0,0,0;'Pencil'
		     dd 115,63,3
		     db 0,0,0;'Brushes'
		     dd 115,83,3
		     db 0,0,0;'Colors'
		     dd 115,103,3
		     db 0,0,0;'Pipetka'
menu_text4:
		     dd 235,43,1
		     db 0,0,0
menu_text5:
		     dd 315,43,1
		     db 0,0,0
menu_text6:
		     dd 395,43,1
		     db 0,0,0
menu_text7:
		     dd 495,43,1
		     db 0,0,0,0


menu_counters:
		     dd 1,1,1,1,1,5,5

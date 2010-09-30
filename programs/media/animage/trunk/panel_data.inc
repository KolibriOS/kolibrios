;**********************************************************
;------------------TEXT DATA-------------------------------
;**********************************************************
name_of_program      db 'Graphics editor <<ANIMAGE>> V 1.2',0
mouse_pos_x	     db 'X='
mouse_pos_y	     db 'Y='
new_text1	     db 'PICTURE SIZE X'
new_text2	     db 'PICTURE SIZE Y'
ok_text 	     db 'OK'

panel_text:	     dd 15,24,4
		     db 'FILE'
		     dd 65,24,4
		     db 'EDIT'
		     dd 115,24,11
		     db 'INSTRUMENTS'
		     dd 235,24,7
		     db 'PALETTE'
		     dd 315,24,7
		     db 'FILTERS'
		     dd 395,24,9
		     db 'ANIMATION'
		     dd 495,24,4
		     db 'HELP'
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
		     dd 15,45,3
		     db 'NEW'
		     dd 15,65,4
		     db 'OPEN'
		     dd 15,85,4
		     db 'SAVE'
		     dd 15,105,7
		     db 'SAVE AS'
		     dd 15,125,4
		     db 'EXIT'

menu_text2:
		     dd 65,45,4
		     db 'UNDO'
		     dd 65,65,4
		     db 'COPY'
		     dd 65,85,5
		     db 'PASTE'
		     dd 65,105,3
		     db 'CUT'
		     dd 65,125,13
		     db 'CLEARE SCREEN'
		     ;dd 95,145,12
		     ;db 'ALLOCATE ALL'
menu_text3:
		     dd 115,45,3
		     db 0,0,0;'PENCIL'
		     dd 115,65,3
		     db 0,0,0;'BRUSHES'
		     dd 115,85,3
		     db 0,0,0;'COLORS'
		     dd 115,105,3
		     db 0,0,0;'PIPETKA'
menu_text4:
		     dd 235,45,1
		     db 0,0,0
menu_text5:
		     dd 315,45,1
		     db 0,0,0
menu_text6:
		     dd 395,45,1
		     db 0,0,0
menu_text7:
		     dd 495,45,1
		     db 0,0,0,0


menu_counters:
		     dd 1,1,1,1,1,5,5

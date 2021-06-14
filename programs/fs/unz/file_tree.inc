;как устроено дерево:
;массив, на который указывает PTree содержит записи _tree
;
;0 0,szRoot
;1  1,dir1
;2  1,dir2
;3   2,dir21
;4   2,dir22
;5    3,dir221
;6   2,dir23
;7  1,dir3
;8  1,dir4
;9   2,dir41



TR_LINE_H equ 18

NUM_READ_FILES = 26




macro p2p r1,r2
{
	push	dword r2
	pop	dword r1
}





uglobal
;-------------- file_tree.inc

openDir rb 1024 	;здесь путь для чтения папки
tmpStr rb 1024

treeDirBuf rb 32+304*NUM_READ_FILES
PTree	rd 1	;указатель на массив структур _tree. Последняя - _tree.name = 0
TreeSize rd 1

endg

iglobal
;------------- file_tree.inc
szRoot db 'hd0',0

tree_info:
  .x		dd 13
  .y		dd 110
  .w		dd 284
  .h		dd 300
  .numLines	dd 200/TR_LINE_H
  .firstLine	dd 0

fiTree		dd 1
  .numBlock	dd 0
  .flags	dd 0
  .numRead	dd NUM_READ_FILES
  .buff 	dd treeDirBuf;buf
		db 0
  .path 	dd openDir



readFolder dd rdFoldforTree	;функция, которой читается папка


endg

DDt dd 0


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




RAZD equ 1		;Знак разделителя строк

F_ONLY_READ	equ 1b
F_HIDDEN	equ 10b
F_SYSTEM	equ 100b
F_LABEL 	equ 1000b
F_FOLDER	equ 10000b
F_NOT_BACKUP	equ 100000b

TR_LINE_H equ 18
TR_STEP_W equ 10
NUM_READ_FILES = 26
TR_BACKGROUND = 0EFF8FFh




macro p2p r1,r2
{
	push	dword r2
	pop	dword r1
}

macro CopyStr strOut,strIn
{
	mov	edi,strOut
	mov	esi,strIn
    @@: lodsb
	stosb
	test	al,al
	jnz	@b
}


struct _BDWK
	flags		rd 1	;bit0: only read , hidden , system , label , folder , not backup
	encod		rb 1	;0-ASCII, 1-UNICODE
	res		rb 3
	timeCreated	rd 1
	dataCreated	rd 1
	lastTimeAccess rd 1
	lastDataAccess rd 1
	lastTimeModify rd 1
	lastDataModify rd 1
	size		rq 1
	name		rb 263
ends

proc d_OutTree
	pushad
	pushfd

	mov	edi,[PTree]
	dps 'Дерево:'
	dnl
   @@:	cmp	dword[edi+4],0
	je	.exit
	dph [edi]
	dps ' '
	dph [edi+4]
	dps ' '
	dpsP [edi+4]
	dnl
	add	edi,8
	jmp	@b
  .exit:

	popfd
	popad
	ret
endp



struct _tree
	open	rd 1	;0, либо количество подкаталогов(учитывая подкаталоги открытых подкаталогов)
	name	rd 1	;указатель на строку относительно PStrings
ends




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


imgFree:
 file 'free.raw'
imgOk:
 file 'ok.raw'
imgMinus:
 file 'minus.raw'
imgPlus:
 file 'plus.raw'
i_end:

readFolder dd rdFoldforTree	;функция, которой читается папка


endg


DDD123 = 0


proc file_tree_Init
	stdcall MM_AllocMem,4000
	mov	[TVROOT],eax
	mov	dword[eax],0
	mov	dword[eax+4],4000

	mcall	68,12,4096
	mov	[PTree],eax

	mov	[eax+_tree.open],dword 0
	mov	[eax+_tree.name],dword szRoot
	mov	[eax+8+_tree.open],dword 0
	mov	[eax+8+_tree.name],dword 0


DDD123 = 0
if DDD123
	mov	[1*8+eax+_tree.open],dword 1
       mov     [1*8+eax+_tree.name],dword sz_d1
	mov	[2*8+eax+_tree.open],dword 1
	mov	[2*8+eax+_tree.name],dword sz_d2
	mov	[3*8+eax+_tree.open],dword 2
	mov	[3*8+eax+_tree.name],dword sz_d21
	mov	[4*8+eax+_tree.open],dword 2
	mov	[4*8+eax+_tree.name],dword sz_d22
	mov	[5*8+eax+_tree.open],dword 3
	mov	[5*8+eax+_tree.name],dword sz_d221
	mov	[6*8+eax+_tree.open],dword 2
       mov     [6*8+eax+_tree.name],dword sz_d23
	mov	[7*8+eax+_tree.open],dword 1
	mov	[7*8+eax+_tree.name],dword sz_d3
	mov	[8*8+eax+_tree.open],dword 1
	mov	[8*8+eax+_tree.name],dword sz_d4
	mov	[9*8+eax+_tree.open],dword 2
	mov	[9*8+eax+_tree.name],dword sz_d41
	mov	[0Ah*8+eax+_tree.open],dword 0
	mov	[0Ah*8+eax+_tree.name],dword 0
end if
	mov	[TreeSize],1

;Fantomer      ;  mcall   68,12,4096
      ;  mov     [PWoSearch],eax
      ;  mov     dword[eax],searchPath
      ;  mov     dword[eax+4],0
	ret
endp
if DDD123
sz_d1	db 'dir1',0
sz_d2	db 'dir2',0
sz_d21	db 'dir21',0
sz_d22	db 'dir22',0
sz_d221 db 'dir221',0
sz_d23	db 'dir23',0
sz_d3	db 'dir3',0
sz_d4	db 'dir4',0
sz_d41	db 'dir41',0
end if

uglobal
  bufImg rb 384*300*3
endg

proc file_tree_Draw
locals
       tr_y	rd 1
       tr_numLine rd 1
endl

	mcall	13,<[tree_info.x],[tree_info.w]>,\
		   <[tree_info.y],[tree_info.h]>,TR_BACKGROUND

	mov	[tr_numLine],0
	p2p	[tr_y],[tree_info.y]
	mov	eax,[PTree]
	mov	eax,[eax+_tree.open]
	inc	eax


	mov	ecx,[TreeSize]
    .drawLine:
	push	ecx

	mov	edi,[tr_numLine]
	shl	edi,3
	add	edi,[PTree]

     ;   mov     eax,[edi+_tree.open]
     ;   mov     bx,TR_LINE_H
     ;   mul     bx
     ;   add     eax,[tree_info.x]
     ;   mpack   ebx,eax,[tr_y]

POPRDLJAKART = 4
	mov	eax,[edi+_tree.open]
	mov	bx,TR_STEP_W
	mul	bx
	add	eax,[tree_info.x]
	mpack	edx,eax,[tr_y]
	add	edx,POPRDLJAKART   ;поправка для картинок

	mov	eax,[edi+_tree.open]
	inc	eax
	cmp	eax,[edi+8+_tree.open]
	je     @f
	mov	ebx,imgPlus
	jmp	.draw1
      @@:
	mov	ebx,imgMinus
     .draw1:
	mcall	7,,<9,9>

	push	edx

	;mov     eax,edi
	;call    getPath
	;mov     ebx,eax
	;stdcall findVetka,eax
	;push    ecx
	;stdcall strCpy,ebx,
	;stdcall MM_DelMem,ebx
	mov	ebx,edi
	call	getFullPath
	stdcall findVetka,openDir

	test	ecx,ecx
	jnz	.l1
	mov	ebx,imgOk
	jmp	.l2
.l1:
	mov	ebx,imgFree
.l2:

	pop	edx
	add	edx,15 shl 16
	mov	ebx,imgOk
	mcall	7,,<9,9>
	sub	edx,POPRDLJAKART   ;убираем поправку для картинок

	lea	ebx,[edx+1+10*10000h]


	mov	ecx,90000000h
	or	ecx,[sc.work_text]

	mov	edx,[edi+_tree.name]
	mcall	4

	add	[tr_y],TR_LINE_H
	inc	[tr_numLine]

	mov	eax,[tree_info.y]
	add	eax,[tree_info.h]
	cmp	[tr_y],eax
	jae	.end

	pop	ecx
	dec	ecx
	jnz	.drawLine
.end:

	ret
endp



proc file_tree_Mouse
locals
   m_x rd 1
endl
	;mcall   37,0 абсолютные координаты мыши

	mcall	37,2 ;кнопки
	test	al,1
	jz	.exit


	mcall	37,1 ;координаты относительно окна
	mov	bx,ax	;y
	shr	eax,16	;x
	mov	[m_x],eax

	cmp	eax,[tree_info.x]	;отбрасываем клики вне дерева
	jb	.exit

	sub	eax,[tree_info.x]
	cmp	eax,[tree_info.w]
	ja	.exit

	cmp	ebx,[tree_info.y]
	jb	.exit
	sub	ebx,[tree_info.y]
	mov	edx,ebx
	cmp	edx,[tree_info.h]
	ja	.exit

	mov	eax,edx

	mov	bx,TR_LINE_H
	xor	edx,edx
	div	bx

	mov	ebx,[PTree]
	lea	ebx,[ebx+eax*8]
	      ;eax=номер строки, ebx - указатель на _tree,

				;пропускаем все клики ниже дерева

	mov	edx,[TreeSize]
	dec	edx
	cmp	eax,edx
	ja	.exit

	mov	eax,[ebx+_tree.open]
	mov	dx,TR_STEP_W
	mul	dx
	add	eax,[tree_info.x]
	cmp	eax,[m_x]
	ja	.mark
	add	eax,9
	cmp	eax,[m_x]
	jb	.mark

				;проверяем: закрыть или открыть папку
	mov	edx,[ebx+_tree.open]
	inc	edx
	cmp	[ebx+_tree.open+8],edx
	jne	@f
	call	tree_closeFolder
	call	file_tree_Draw
	ret
     @@:
	call	tree_openFolder

	call	file_tree_Draw
	jmp	.exit
 .mark:
	;клик по строке

 .exit:
	ret
endp


;-------------------------------------------------------------------------------
;---------------------- закрывает папку ----------------------------------------
;-------------------------------------------------------------------------------
; inp:  ebx - указатель на _tree,
;-------------------------------------------------------------------------------
proc tree_closeFolder

	mov	esi,[ebx+_tree.open]
	lea	edi,[ebx+8]
    @@:
	cmp	[edi+_tree.open],esi
	jbe	@f
	stdcall MM_DelMem,[edi+_tree.name]
	add	edi,8
	jmp	@b
    @@:


	mov	esi,edi
	lea	edi,[ebx+8]

	mov	eax,esi
	sub	eax,edi
	shr	eax,3
	sub	[TreeSize],eax

			   ;сдвигаем все структуры, которые ниже закрываемой, вверх
    @@: movsd
	lodsd
	stosd
	test	eax,eax
	jnz	@b

	ret
endp

;-------------------------------------------------------------------------------
;-------------------- открывает папку ------------------------------------------
;-------------------------------------------------------------------------------
; inp:  eax=номер строки, ebx - указатель на _tree,
;-------------------------------------------------------------------------------

proc tree_openFolder
locals
	p_treeOpen  rd 1
	currLine rd 1
	PTmpTree rd 1
	endTmpTree rd 1
	lEOF	 rd 1
	numFolds rd 1
	openLine rd 1

endl
	mov	[p_treeOpen],ebx
  ; int3
	call	getFullPath

;----- Читаем папку и заполняем tmpTree---------------------------------------------------------

	mcall	68,12,8192
	mov	[PTmpTree],eax	      ;память для списка каталогов

	mov	[lEOF],0
	mov	[endTmpTree],0
	mov	[numFolds],0

	mov	[fiTree.numBlock],0

 .stepLoadFold: 		      ;чтение папки
;   dpsP [fiTree+21]
;   dnl

	mcall	70,fiTree
;        mov     eax, fiTree
;        call    [readFolder]

	test	eax,eax
	je	@f
	cmp	eax,6
	jne	.err
     @@:

	cmp	ebx,NUM_READ_FILES
	je	@f
	mov	[lEOF],1		;ставим EOF
   @@:
	mov	ecx,ebx

	mov	edi,treeDirBuf+32

   .testRec:			    ;обработка записей в treeDirBuf
	push	ecx

	lea	eax,[edi+_BDWK.name]  ;отбрасываем не папки и папки . и ..
	cmp	word[eax],'.'
	je	.endtestRec

	cmp	word[eax],'..'
	jne	@f
	cmp	byte[eax+2],0
	je	.endtestRec
     @@:

       ; cmp     [edi+_BDWK.flags],F_FOLDER
       ; jne     .endtestRec

	inc	[numFolds]
	push	edi

	lea	esi,[edi+_BDWK.name]


     ;;;;;;;;;   ;добавление папки в список
	mov	edi,esi       ;получаем длину строки
	xor	al,al
	mov	ecx,260
	repne	scasb
	mov	eax,260
	sub	eax,ecx

	stdcall MM_AllocMem,eax
;dph eax
;dnl
	mov	edi,eax
	mov	ebx,[endTmpTree]
	add	ebx,[PTmpTree]
	mov	[ebx],eax
	add	[endTmpTree],4

	mov	ecx,ebx      ;если мало места под записи, то перераспределить
	add	ecx,4	     ;больше памяти
	test	ecx,0FFFh
	jz	@f
	and	ecx,0FFFFF000h
	add	ecx,1000h
    @@: sub	ecx,4
	cmp	ecx,ebx
	jne	@f
	mcall	68,20,,[PTmpTree]
	mov	[PTmpTree],eax

    @@: lodsb
	stosb
	test	al,al
	jnz	@b


	pop	edi

    .endtestRec:
	add	edi,304
	pop	ecx
	dec	ecx
	jnz	.testRec


	add	[fiTree.numBlock],NUM_READ_FILES

	cmp	[lEOF],1
	jne	.stepLoadFold

	cmp	[numFolds],0
	je	.exit
      ;;;;;;;;;;;; вписываем в основное дерево
			   ;смещаем записи вниз, чтоб вписать новую ветку дерева

	mov	esi,[PTree]
	mov	eax,[TreeSize]
	inc	eax
	shl	eax,3
	add	esi,eax

	mov	edi,esi
	mov	eax,[numFolds]
	shl	eax,3
	add	edi,eax

	mov	eax,esi
	sub	eax,[p_treeOpen]
	shr	eax,2
	mov	ecx,eax
	add	esi,4
	add	edi,4
	std
	rep	movsd
	cld

				;теперь записываем в [PTree]
	mov	ebx,[p_treeOpen]
	mov	eax,[ebx+_tree.open]
	inc	eax
	mov	esi,[PTmpTree]
	lea	edi,[ebx+8]
	mov	ecx,[numFolds]
    @@: mov	[edi+_tree.open],eax
	p2p	[edi+_tree.name],[esi]
	add	esi,4
	add	edi,8
	loop	@b

	mov	eax,[numFolds]
	add	[TreeSize],eax

.exit:
  .err:

	mcall	68,13,[PTmpTree]

   call d_OutTree
	ret
endp

;-------------------------------------------------------------------------------
;--- находит полный путь до элемента -------------------------------------------
;-------------------------------------------------------------------------------
;inp: ebx = Pointer to _tree.
;outp: openDir содержит полный путь (после последней папки имеется /)
;-------------------------------------------------------------------------------
proc getFullPath
;нужно пробежаться от последней папки в пути вверх, по родительским
;сначала записываем папки в tmpStr, разделяя их символом RAZD, затем
;скопиpуем их в обратном порядке в готовый путь в openDir
	push	ebx edi esi
	mov	eax,[ebx+_tree.open]
	mov	edi,tmpStr+1
	mov	byte[edi-1],RAZD

	cmp	[ebx+_tree.name],0
	je	.copyIn_openDir

	jmp	.addFoldToPath
   .testLine:
	cmp	[ebx+_tree.open],eax
	jb	.addFoldToPath

	sub	ebx,8
	cmp	ebx,[PTree]
	jne	.testLine
       ; jmp     .copyIn_openDir

   .addFoldToPath:
	mov	byte[edi-1],RAZD
	mov	esi,[ebx+_tree.name]
    @@: lodsb
	stosb
	test	al,al
	jnz	@b

	mov	eax,[ebx+_tree.open]
	cmp	ebx,[PTree]
	je	.copyIn_openDir

	cmp	ebx,[PTree]
	jne	.testLine
	sub	ebx,8
	cmp	ebx,[PTree]
	jne	.testLine

   .copyIn_openDir:
	sub	edi,2
	mov	edx,openDir+1
	mov	byte[edx-1],'/'
	mov	byte[edx],0
	cmp	edi,tmpStr-1
	je	.endConv
    @@:
	mov	al,RAZD
	mov	ecx,1024
	std
	repne scasb
	cld

	push	edi
	add	edi,2
	mov	eax,1024
	sub	eax,ecx
	mov	ecx,eax
	mov	esi,edi
	mov	edi,edx
	rep movsb
	mov	byte[edi-1],'/'
	mov	byte[edi],0
	mov	edx,edi
	pop	edi

	cmp	edi,tmpStr-1
	jne	@b
     .endConv:

	pop	esi edi ebx
	ret
endp


;inp eax = Pointer to _tree
;outp eax = Pointer to string - full path
;Память с именем освободить!!!
proc getPath
locals
	strTmp rb 256
	reslt rd 1
endl
	push	ebx edi esi
	mov	ebx,eax
	mov	eax,[eax+_tree.open]
	lea	edi,[strTmp+1]
	mov	byte[edi-1],RAZD

	cmp	ebx,[PTree]
	je	.copyIn_openDir
	jmp	.addFoldToPath
   .testLine:
	cmp	[ebx+_tree.open],eax
	jb	.addFoldToPath

	sub	ebx,8
	cmp	ebx,[PTree]
	jne	.testLine
	jmp	.copyIn_openDir

   .addFoldToPath:
	mov	byte[edi-1],RAZD
	mov	esi,[ebx+_tree.name]
    @@: lodsb
	stosb
	test	al,al
	jnz	@b

	mov	eax,[ebx+_tree.open]
	sub	ebx,8
	cmp	ebx,[PTree]
	jne	.testLine

   .copyIn_openDir:
	lea	eax,[strTmp]
	stdcall strLen,eax
	stdcall MM_AllocMem,eax
	mov	[reslt],eax
	mov	edx,eax
	inc	edx
	sub	edi,2
	mov	byte[edx-1],'/'
	mov	byte[edx],0
	lea	eax,[strTmp-1]
	cmp	edi,eax
	je	.endConv
    @@:
	mov	al,RAZD
	mov	ecx,1024
	std
	repne scasb
	cld

	push	edi
	add	edi,2
	mov	eax,1024
	sub	eax,ecx
	mov	ecx,eax
	mov	esi,edi
	mov	edi,edx
	rep movsb
	mov	byte[edi-1],'/'
	mov	byte[edi],0
	mov	edx,edi
	pop	edi

	lea	eax,[strTmp-1]
	cmp	edi,eax
	jne	@b
     .endConv:
	mov	eax,[reslt]
	pop	esi edi ebx
	ret
endp

;####################################################################################################
;####################################################################################################
;####################################################################################################
;####################################################################################################


;tree_ функции, хранящие отмеченные пути. построены в виде дерева папок\файлов
;в памяти храняться как список структур (usel)

;tree_Add(path)
;добавляет путь в дерево отмеченых файлов
;вывод
;еах = 0 - добавлено
;      1 - такой путь уже есть
;     -1 - неверный путь
;
;
;tree_Del(path)





;####################################################################################################
struct _vetka

	sizeused rd 1
	sizebuf rd 1
ends

struct _usel

	pName rd 1
	pNext rd 1
ends


;''''''''''''''''' ГОТОВА
;добавляет путь в дерево отмеченых файлов
;вывод
;еах = 0 - добавлено
;      1 - такой путь уже есть
;     -1 - неверный путь
proc tree_Add path:DWORD
locals
 numFolds rd 1
 vetB rd 1
 uselB rd 1
endl
	push	ebx edi esi
;int3
	stdcall getNumFolds, [path]
	cmp	eax,-1
	je	.error

	mov	ecx,eax
	inc	eax
	mov	[numFolds],eax
				;ищем каталог, с которого начинаем дописывание дерева
.searchBegin:
	push	ecx
	mov	eax,[numFolds]
	sub	eax,ecx
	stdcall getFirstTailPath, [path], eax
	push	eax
	stdcall findVetka,eax
	mov	ebx,ecx
	mov	[vetB],eax
	mov	[uselB],edx
	pop	eax
	stdcall MM_DelMem,eax
	cmp	ebx,0
	jne	.foundBegin
	pop	ecx
	loop	.searchBegin
	jmp	.exitNotAdded
.foundBegin:	 ;в стеке ещё есх от прошлого loop'a
;int3
	mov	eax,[uselB]
	mov	ebx,[eax+_usel.pNext]
	cmp	ebx,0
	jne	@f
	mov	ebx,eax
	stdcall MM_AllocMem,4000
	mov	[ebx+_usel.pNext],eax
	mov	dword[eax],0
	mov	dword[eax+4],4000
	mov	ebx,eax
@@:
	mov	eax,ebx
	add	[eax+_vetka.sizeused],8
	mov	edx,[eax+_vetka.sizebuf]
	sub	edx,8
	cmp	[eax+_vetka.sizeused],edx
	jb	.noOverflow

	add	edx,8
	push	eax		;увеличиваем размер буфера
	push	edx		;функции realloc нет - потому такое извращение - Fantom
	add	edx,4000
	mov	esi,eax
	stdcall MM_AllocMem, edx
	mov	ebx,eax
	mov	edi,eax
	pop	ecx
	shr	ecx,2
	rep movsd
	pop	eax
	stdcall MM_DelMem,eax
	mov	eax,ebx

.noOverflow:
	mov	ebx, [eax+_vetka.sizeused]
	lea	ebx, [ebx+eax]
				;ebx = PU_      - usel
				;eax = P_       - vetka
	mov	eax,[numFolds]
	sub	eax,[esp]		 ;- в стеке всё ещё есх от .searchBegin
	dec	eax
	stdcall getFoldByNum ,[path], eax
	mov	[ebx+_usel.pName],eax
	mov	[ebx+_usel.pNext],0

;call D_OutTree
;dps '--------------------------------'
;dnl
	pop	ecx
	dec	ecx
	jnz	.searchBegin
	mov	eax, 0
	jmp	.exit
.exitNotAdded:
	mov	eax,1
	jmp	.exit
.error:
	mov	eax,-1

.exit:
;dps '-- ИТОГ ------------------------'
;dnl
;call D_OutTree
	pop	esi edi ebx
	ret
endp

DDt dd 0

;-------------------------------------------------------------------------------
; ДЛЯ ОТЛАДКИ
proc D_OutTree
	mov	eax,[TVROOT]
	call	D_OutTree2
	ret
endp

proc D_OutTree2
	inc	[DDt]
	mov	ecx,[eax]
	shr	ecx,3
	add	eax,8
.loop:
	push	eax ecx
	mov	ecx,[DDt]
.space: dps ' '
	loop	.space

	dpsP	[eax]
      dps '    '
      dph  dword[eax+4]
	dnl
	cmp	dword[eax+4],0
	je	@f
	mov	eax,[eax+4]
	call	D_OutTree2
       @@:
	pop	ecx eax
	add	eax,8
	loop	.loop
	dec	[DDt]

	ret
endp
;-------------------------------------------------------------------------------


;tree_Del(path)
;           если в указанном узле есть указатель на ветку, то
;             treeDelIn(path)
;           удалить указанный узел
;           если он не последний в списке, то
;             если он не стоит последним, то
;                сдвинуть все последующие структуры узел вверх на 8
;last_elem:  иначе
;             удалить эту ветку
;             tree_Del(путь - 1 элемент)

;treeDelIn(path)
;        если это ветка, то
;          пока есть узлы {
;            если у узла есть ветка, то
;              treeDelIn(путь до текущего узла)
;            иначе
;              освободить память с именем
;          }
;ГОТОВ
;return:
;eax = 0 - success, -1 - error
proc tree_Del path:DWORD
locals
 PU rd 1
 P rd 1
 path2 rb 256
endl
;int3
	push	ebx edi esi
	stdcall findVetka,[path]
	test	ecx,ecx
	jne	.err
	mov	edi,edx
	mov	esi,eax
	cmp	[edi+_usel.pNext],dword 0
	je	@f
	mov	eax,edi
	push	edi esi
	call treeDelIn
	pop	esi edi
@@:
	stdcall MM_DelMem,[edi+_usel.pName]
	mov	eax,[esi+_vetka.sizeused]
	cmp	eax,8
	je	.last_elem

	add	eax,esi ; - last _usel
	cmp	edi,eax  ;if last _usel, then do not
	je	@f
	push	esi
	sub	eax,edi ;move all _usel up
	shr	eax,2
	mov	ecx,eax
	mov	esi,edi
	add	esi,8
	rep	movsd
	pop	esi

@@:	sub	dword [esi+_vetka.sizeused],8
	jmp	.exit

.last_elem:
	stdcall MM_DelMem,esi
	stdcall findUselMinusOne,[path]
	cmp	ecx,0
	jne	.exit
	mov	[edx+_usel.pNext],0
.exit:
	xor	eax,eax
	pop	esi edi ebx
	ret

.err:
	or	eax,-1
	pop	esi edi ebx
	ret

endp

;!!!!   не сохраняет регистры

;input: eax = pointer to _usel
proc treeDelIn
	;mov     eax,[eax+_usel.pNext]  ;опасно, но и так везде проверяется перед вызовом
	;cmp     eax,0
	;je      .exit

	lea	ebx,[eax+8]
	mov	ecx,[eax]
	shr	ecx,3	;количество узлов
.loop:
	push	ecx
	mov	eax,[ebx+_usel.pNext]
	test	eax,eax
	jz	@f
	push	ebx
	call	treeDelIn
	pop	ebx
    @@:
	stdcall MM_DelMem,[ebx+_usel.pName]
	stdcall MM_DelMem,[ebx+_usel.pNext]
	add	ebx,8
	pop	ecx
	dec	ecx
	jnz	.loop
.exit:

	ret
endp

; """""""""""""""" ГОТОВА
;path = /hd0/1/kol      -folder
;TV1 -> us'hd0' -> us'1' -> us'kol'
;path = /hd0/1/mtldr    -file
;path = /rd/1/kernel.mnt - file in ramdisk

;находит ветку в которой находится узел с нужной нам папкой
;вывод:
;eax - ветка
;edx - узел, который найден последним, т. е. для пути /rd/1/lib/qwe будет узел lib, так как файла qwe нет
;ecx - 0 - нужный ветка/узел успешно найден
;      1 - нужный ветка/узел не найден - на выходе последний найденый узел
;      2 - неверный параметр
proc findVetka path:DWORD
locals
	curFold rd 1
	num_f rd 1
	old_vetk rd 1
	old_usel rd 1
endl
;int3
	mov	eax, [TVROOT]
	cmp	dword[eax],0
	jne	@f
	mov	edx, 0
	mov	ecx,2
	xor	eax,eax
	ret
     @@:
	push	ebx edi esi
	mov	[num_f], 0
	xor	edx, edx
	stdcall getFoldByNum, [path], 0
	test	eax, eax
	jz	.error2
;dpsP eax
;dnl

	mov	[curFold], eax
	mov	esi, [TVROOT]
.goLoop:
	mov	ebx, 8
	mov	ecx, [esi]
	shr	ecx, 3		     ;столько в TVROOT структур usel
.loop:
	push	ecx
	stdcall strCmp, [curFold], [esi+ebx]
	test	al, al
	jz	.find
	add	ebx, 8
	pop	ecx
	loop	.loop
	jmp	.error1

.find:
	pop	eax	; выровняли
	inc	[num_f]
	stdcall MM_DelMem, [curFold]
	stdcall getFoldByNum, [path], [num_f]
	test	eax, eax
	jz	.end
	mov	[curFold], eax


	cmp	dword[esi+ebx+4], 0
	jz	.error
	lea	eax, [esi+ebx]
	mov	[old_vetk],esi
	mov	[old_usel],eax
	mov	esi, [esi+ebx+4]
	jmp	.goLoop
.end:

	mov	eax, esi
	lea	edx, [esi+ebx]
	xor	ecx,ecx
	jmp	.exit

.error:
	stdcall MM_DelMem, [curFold]
	mov	eax, esi
	lea	edx, [esi+ebx]
	mov	ecx,1
	jmp	.exit
.error1:
	stdcall MM_DelMem, [curFold]
	mov	eax, [old_vetk]
	mov	edx, [old_usel]
	mov	ecx,1
	jmp	.exit
.error2:
	stdcall MM_DelMem, [curFold]
	mov	eax, 0
	mov	edx, 0
	mov	ecx,2

.exit:
	pop	esi edi ebx
	ret
endp

	  ;ГОТОВА
;находит узел, который ссылается на заданный узел
;вернёт в edx usel, для предпоследней папки в пути
;eax - ветка, в которой этот узел
;есх - статус(0, 1, 2, -1)
proc findUselMinusOne path:DWORD
locals
  path2 rb 256
endl
	push	ebx edi esi

	stdcall strLen,[path]
	cmp	eax,255
	ja	.err
	lea	eax,[path2]
	stdcall strCpy,[path],eax

	lea	edi,[path2]
	xor	al,al
	mov	ecx,256
	repne scasb		;           V
	test	ecx,ecx 	;/hd0/1/kol
	jz	.err   ;только одна папка
	sub	ecx,255
	xor	ecx,-1
	inc	ecx
	sub	edi,2
	cmp	byte [edi], '/'
	jne	@f
	dec	edi
	dec	ecx
	@@:
	mov	al,'/'
	std
	repne	scasb
	cld
	test	ecx,ecx
	jz	.err   ; только одна папка

	inc	edi
	mov	byte[edi],0

	lea	eax,[path2]
	stdcall findVetka,eax
	jmp	@f
.err:
	xor	eax,eax
	xor	edx,edx
	or     ecx,-1
@@:
	pop	esi edi ebx
	ret
endp

;--------------Готова
;находит узел для заданного каталога в пути
;eax = Pointer to _vetka
;edx = Pointer to _usel
;ecx = status (0 - success,             1 - found path not full,
;              2 - path not corrected, -1 - only one folder in path)

proc findUselByNum path:DWORD,num:DWORD
locals
  path2 rb 256
endl
	push	ebx edi esi

	stdcall getNumFolds,[path]
	cmp	eax,[num]
	jl	.err

	xor	ebx,ebx
	lea	edi,[path2]
	mov	esi,[path]
	cmp	byte[esi],'/'
	jne	.l2
	dec	ebx
.l2:	lodsb
	stosb
	cmp	al,'/'
	jne	@f
	inc	ebx
	cmp	ebx,[num]
	ja	.go
      @@:
	test	al,al
	jnz	.l2

.go:
	mov	byte[esi-1],0
	lea	eax,[path2]
	stdcall findVetka,eax
	jmp	@f
.err:
	xor	eax,eax
	xor	edx,edx
	or	ecx,-1
@@:
	pop	esi edi ebx
	ret
endp

;''''''''''''''' ГОТОВА
;Возвращает имя файла/каталога из всего пути
; --- !!! Память нужно будет освободить
proc getName path:DWORD
	push	esi edi

	stdcall strLen,[path]
	mov	edi,[path]
	add	edi,eax
	sub	edi,2
	cmp	byte[edi],'/'
	jne	@f
	dec	edi
@@:
	mov	ecx,eax
	mov	al,'/'
	std
	repne scasb
	cld
	add	edi,2
	stdcall strLen,edi
	mov	esi,eax
	stdcall MM_AllocMem, ecx
	push	eax
	add	esi,eax
	stdcall strCpy,edi, eax
	cmp	byte[esi-2],'/'
	jne	@f
	mov	byte[esi-2],0
@@:
	pop	eax
	pop	edi esi
	ret
endp



; """""""""""""""" ГОТОВА
proc strCpy src:DWORD,dest:DWORD
	push	esi edi
	mov	edi,[dest]
	mov	esi,[src]
@@:	lodsb
	stosb
	test	al,al
	jnz	@b
	pop	edi esi
	ret
endp

; """""""""""""""" ГОТОВА
; с учётом завершающего 0
proc strLen strz:DWORD
	push	edi
	mov	edi,[strz]
	xor	al,al
	mov	ecx,1024
	repnz	scasb
	mov	eax,1024
	sub	eax,ecx
	pop	edi
	ret
endp

; """""""""""""""" ГОТОВА
proc strCmp src1:DWORD, src2:DWORD
	push	esi edi
	mov	edi, [src1]
	mov	esi, [src2]
	mov	eax, 1
@@:	test	al, al
	jz	.end
	lodsb
	scasb
	jz	@b

	ja	.low
	mov	eax, 1
	jmp	@f
.low:
	mov	eax, -1
@@:
	pop	edi esi
	ret
.end:
	xor	eax,eax
	jmp	@b
endp


; """""""""""""""" ГОТОВА
;возращает имя папки из всего пути по его номеру.
;! Счёт начинается с 0
; --- !!! Память нужно будет освободить
proc getFoldByNum strz:DWORD, num:DWORD
	push	ebx edi esi

	mov	esi, [strz]
	mov	ecx, 1024
	stdcall MM_AllocMem, 256
	mov	ebx, eax
	cmp	byte[esi], '/'
	jne	@f
	inc	esi
     @@:

.find_begin:
	cmp	[num], 0
	jz	.copy

    @@: lodsb
	cmp	al, '/'
	je	@f
	cmp	al, 0
	je	.error
	loop	@b
    @@: dec	[num]
	jmp	.find_begin

.copy:
	;dec     esi
	mov	edi, ebx
	mov	byte[edi], 1 ;это метка того, что ни один байт не записан
    @@:
	lodsb
	cmp	al, '/'
	je	@f
	cmp	al, 0
	je	@f
	stosb
	loop	@b
    @@:
	cmp	byte[edi], 1
	jne	@f
.error:
	stdcall MM_DelMem, ebx
	xor	ebx, ebx
	jmp	.end
      @@:
	xor	al, al
	stosb
.end:
	mov	eax, ebx
	pop	esi edi ebx
	ret
endp


; """""""""""""""" ГОТОВА
;возращает количество элементов
;ret: eax = Num or -1
proc getNumFolds path:DWORD
	push	ebx edi esi

	stdcall strLen,[path]
	cmp	eax,255
	ja	.err

	xor	ebx,ebx
	mov	esi,[path]
	cmp	byte[esi],'/'
	jne	.l1
	dec	ebx
.l1:	lodsb
	cmp	al,'/'
	jne	@f
	inc	ebx
      @@:
	test	al,al
	jnz	.l1

	cmp	[esi-2],byte '/'
	je	@f
	inc	ebx
@@:

	mov	eax,ebx
	jmp	.exit
.err:
	or	eax,-1
.exit:
	pop	esi edi ebx
	ret
endp


;''''''''''''''''ГОТОВА
;возвращает часть пути: первые num каталогов
;ret: Указатель на строку.
; --- !!! Память нужно будет освободить
proc getFirstTailPath path:DWORD, num:DWORD
	push	ebx edi esi

	cmp	[num],0
	je	.err


	stdcall strLen,[path]
	cmp	eax,255
	ja	.err

	stdcall MM_AllocMem, eax
	mov	edi,eax
	push	eax


	xor	ebx,ebx
	mov	esi,[path]
	cmp	byte[esi],'/'
	jne	.l1
.l1:	lodsb
	stosb
	cmp	al,'/'
	jne	@f
	inc	ebx
	cmp	ebx,[num]
	ja	.endloop
      @@:
	test	al,al
	jnz	.l1
.endloop:

	cmp	[esi-2],byte '/'
	je	@f
	mov	byte[edi-1],0
	jmp	.l2
@@:
	mov	byte[edi-2],0
.l2:

	pop	eax
	jmp	.exit
.err:

	or	eax,-1

.exit:

	pop	esi edi ebx
	ret
endp


proc TESTINIT
;---- чтобы струтуры менеджера памяти были в норме нужно именно им выделить память под это дерево

;;TEST                            getFoldByNum
;tstStr db '/hd0/1/dqw',0
;        stdcall getFoldByNum,tstStr,2
;        dpsP eax
;        dps '|'
;        dnl
;        ret
	stdcall MM_AllocMem,4000
	mov	[TVROOT],eax
	stdcall MM_AllocMem,4000
	mov	[Tus1+4],eax
	stdcall MM_AllocMem,4000
	mov	[Tus2+4],eax

	stdcall MM_AllocMem,4
	mov	[Tus1],eax
	stdcall MM_AllocMem,4
	mov	[Tus2],eax
	stdcall MM_AllocMem,4
	mov	[Tus3],eax

	stdcall strCpy,sname1,[Tus1]
	stdcall strCpy,sname2,[Tus2]
	stdcall strCpy,sname3,[Tus3]

	mov	esi,TvetkaHD0
	mov	edi,[TVROOT]
	mov	ecx,4
	rep	movsd

	mov	esi,Tvetka1
	mov	edi,[Tus1+4]
	mov	ecx,4
	rep	movsd

	mov	esi,TvetkaKOL
	mov	edi,[Tus2+4]
	mov	ecx,4
	rep	movsd


;int3
;        stdcall tree_Add,strTets1
       ; stdcall tree_Add,strTets2
       ; stdcall tree_Add,strTets3
;dps '----------'
;dnl
;call D_OutTree

;        stdcall tree_Del,strTets2
;dps '----------'
;dnl
;call D_OutTree
;dps '-------------------------'
;dnl
    ;    stdcall getFoldByNum,strTets,0
;mov     edi,eax
;dpsP edi
;dnl
     ;int3
	mov	eax,[PTree]
	lea	eax, [5*8+eax+_tree.open]
	call GetPath
      dpsP eax
      dnl
	ret
endp


;;TEST
strTets1 db '/hd0/kol/asd1',0
strTets2 db '/hd0/kol/asd2',0
strTets3 db '/hd0/kol/asd3',0

TvetkaHD0:
	dd 2*4
	dd 4000

 Tus1:
	dd sname1
	dd Tvetka1
;---------------------



Tvetka1:
	dd 2*4
	dd 4000

 Tus2:
	dd sname2
	dd TvetkaKOL
;---------------------


TvetkaKOL:
	dd 2*4
	dd 4000

 Tus3:
	dd sname3
	dd 0
;---------------------

sname1 db 'hd0',0
sname2 db '1',0
sname3 db 'kol',0

_TBUFSIZE dd 4000 - $


TVROOT rd 1

;будет буфер отмеченных файлов/папок
;открыть папку
; если она отмечена вся, то ничего не делать
; если в ней убрать галку, то добавить в список отмеченных все остальные,
;            а папку убрать из списка
; если поставить галку на последнем не выделенном файле, то ничего. просто добавить его в список,манипуляции с папками незачем
;

;vetka:
; sizeused
; sizebuf(def - 4000)
; array of usel
;
; --- реализация ---
;дерево выделенных объектов:
; узел (usel):
;   pName - двслово - указатель на строку имя узла
;   pNext - двслово - указатель на vetka или 0


;vetka {2*8,4000-2*8,
;usel [Program_Files,0],
;usel [Doc,v2]
;}
;v2:
;vetka {1*8,4000-8,
;usel [fold1,0]
;}

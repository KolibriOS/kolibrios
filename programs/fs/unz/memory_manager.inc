;MM_ = MemoryManager
;Этот модуль позволяет выделять память маленькими кусочками, оптимально используя
;страницы памяти
;Блоки - это одна или несколько страниц, которые имеют запись
;в MM_BlockInfo и которые в конце имеют стекообразную структуру(в смысле
;растёт к меньшим адресам), заканчивающейся dword 0. В начале блока
;находятся данные. В структуре находятся пары dword'ов: начало
;участков, их конец(посл. байт+1). Эти пары всегда сортируются по
;расположению описываемых
;участков в обратном порядке, т.е. в самом конце блока будет пара данных
;на самый первый участок. Для выделения памяти нужно найти блок, в котором
;достаточно места (причём необходимое место = запрашиваемый объём + место
;под пару с данными в конце блока) и вставить пару с нужными данными.
;Для удаления участка нужно только убрать из пар пару с нужным участком
;и поправить расположение остальных пар.
;begData1.....endData1,begData2...endData2,.......0,beg2,end2,beg1,end1


;выделяет память
;return eax = указатель на выделеный блок
;proc MM_AllocMem stdcall,Size:DWORD

;БАГ в выделении крупных кусков: всегда выделяет новую страницу



align 4
MM_AllocMem:
	Size equ ebp+8
	begFree equ ebp-8 ;начало
	endFree equ ebp-4 ;и конец свободного места от конца инфы до
	endArea equ ebp-12
	push	ebp
	mov	ebp,esp
	add	esp,-4*3
	push	ebx edi esi
;dps 'MEM: '
;dph [Size]
;dps ' '
				;начала пар записей
	mov	edx,[MM_NBlocks]
	cmp	edx,0
	jne	.BegTestBlocks	       ;если блоков нет, то добавить новый
	mov	ecx,[Size]
	call	AddBlock
	jmp	.return
align 4
 .BegTestBlocks:


	xor	ebx,ebx
	mov	ecx,edx
align 4
  .TestBlock:	 ;проверка блока
		   ;проверка: есть ли место для ещё одной пары
	mov	edi,[MM_BlocksInfo+ebx]
	add	edi,[MM_BlocksInfo+ebx+4]
	mov	[endArea], edi
	sub	edi,4

	cmp	dword[edi],0	 ;если в блоке нет ни одной записи
	jne	.NoEmptyBlock
	mov	eax,[MM_BlocksInfo+ebx]
	mov	dword[edi-4],eax
	mov	dword[edi],eax
	mov	edx,[Size]
	add	dword[edi],edx
	mov	dword[edi-8],0
	jmp	.return
align 4
 .NoEmptyBlock:
	xor	eax,eax
	push	ecx
	or	ecx,-1
	std
	repne	scasd
	cld
	pop	ecx
	mov	eax,[edi+12] ;конец посл участка
	add	eax,4
	cmp	eax,edi
	jb	@f

	add	ebx,8
	dec	ecx
	jnz	.TestBlock

	mov	ecx,[Size]
	call	AddBlock
	jmp	.return
     @@:


	mov	[begFree],eax  ;eax = конец посл. участка + 4
	mov	[endFree],edi  ;edi = указатель на конец посл участка - 12
	sub	dword[begFree],4
	add	dword[endFree],12

   ;проверка перед всеми участками
	mov	edi,[MM_BlocksInfo+ebx]
	mov	eax,[endArea]
	mov	eax,[eax-8]
	sub	eax,[MM_BlocksInfo+ebx]
	cmp	eax,[Size]
	ja	.AddInBegBlock

   ;проверка между участками
	mov	eax,[endArea]
	cmp	dword[eax-12],0
	je	.EndTest ;если в блоке только 1 участок

	sub	eax,4
     @@:
	mov	edi,[eax-12]
	sub	edi,[eax]
	cmp	edi,[Size]
	jae	.AddInMiddle
	sub	eax,8
	cmp	dword[eax-8],0
	jne	@b

   .EndTest:

   ;проверка после всех блоков
	mov	eax,[begFree]
	mov	edi,[endFree]
	lea	esi,[edi-8]    ;8 - место под запись
	sub	esi,eax

	cmp	esi,[Size]
	ja	.AddInEnd

	add	ebx,8
	dec	ecx
	jnz	.TestBlock

	mov	ecx,[Size]
	call	AddBlock

	jmp	.return
align 4
  .AddInBegBlock: ;Добавить в начало. В edi начало блока
	;pop     eax

	mov	eax,edi
	add	eax,[MM_BlocksInfo+ebx+4]
	sub	eax,4
	push	eax
	call	MoveRecordsLeft
	pop	eax
	mov	[eax-4],edi
	push	edi
	add	edi,[Size]
	mov	[eax],edi
	pop	eax

	jmp	.return
align 4
  .AddInMiddle:  ;Добавить между участками, еах=конец участка перед свободным местом

	;pop     ecx   ;add     esp,4
	push	eax
	sub	eax,8
	call	MoveRecordsLeft
	pop	eax

	mov	edx,[eax]
	mov	[eax-12],edx
	add	edx,[Size]
	mov	[eax-8],edx
	mov	eax,[eax]
	jmp	.return
align 4
  .AddInEnd:  ;Добавить после участков. еdi=указатель на 2ой элем пары с инфой о посл участке

	;add     esp,4
	mov	eax,[edi]
	mov	[edi-12],eax
	push	eax
	add	eax,[Size]
	mov	[edi-8],eax
	pop	eax

.return:

	pop	esi edi ebx
	leave
	ret 4

restore Xren
restore Size
restore begFree
restore endFree



;eax - первый сдвигаемый dword
;сдвигает пары dword'ов на 8B назад включая dword 0
align 4
proc MoveRecordsLeft
local	var1:DWORD,\
	var2:DWORD

	p2p	[var1],[eax]
	p2p	[var2],[eax-4]

     @@:
	sub	eax,8
	cmp	dword[var1],0
	je	@f

	push	dword[eax]
	p2p	[eax],[var1]
	pop	dword[var1]

	push	dword[eax-4]
	p2p	[eax-4],[var2]
	pop	dword[var2]
	jmp	@b
      @@:

	mov	dword[eax],0
	ret
endp

;ecx = размер требуемого участка
;добавляет блок и создаёт в нём участок размером ecx
align 4
proc AddBlock
	mov	edx,[MM_NBlocks]
	inc	edx
	cmp	edx,MM_MAX_BLOCKS
	ja	.ErrAlloc

	push	ecx
	add	ecx,12

	test	ecx,0FFFh	   ;округляем до большей границы страницы
	jz	@f
	add	ecx,1000h
	and	ecx,0FFFFF000h
      @@:

	mcall	68,12,ecx
	mov	[MM_NBlocks],edx     ;заполнение данных о блоке
	mov	[edx*4*2-4*2+MM_BlocksInfo],eax  ;begin
	mov	[edx*4*2-4+MM_BlocksInfo],ecx	 ;size
;dps 'Block '
;dph eax
;dps ' '
;dph ecx
;dnl
	mov	edx,eax
	add	edx,ecx
	mov	[edx-8],eax
	pop	dword[edx-4]
	add	[edx-4],eax
	mov	dword[edx-12],0
	ret

 .ErrAlloc:
	pop	ecx
	xor	eax,eax
	ret
endp
;-------------------------------------------------------------------------------


;удаляет  память
;proc MM_DelMem Pointer:DWORD
align 4
MM_DelMem:
	Pointer equ ebp+8
	push	ebp
	mov	ebp,esp
; int3
	push	ebx
	mov	ecx,[MM_NBlocks]
	test	ecx,ecx
	jnz	@f
	xor	eax,eax
	pop	ebx
	leave
	ret 4
      @@:

	mov	eax,[Pointer]
	xor	ebx,ebx ;ebx - (номер блока)*8
   .TestBlocks:
	mov	edx,[MM_BlocksInfo+ebx]
	add	edx,[MM_BlocksInfo+ebx+4]
	sub	edx,8	      ;edx - указатель на 1ую пару

     .TestMems:
	cmp	[edx],eax
	je	.FoundMem

	sub	edx,8
	cmp	dword[edx+4],0
	jne	.TestMems

	add	ebx,4
	loop	.TestBlocks

	xor	eax,eax
	pop	ebx
	leave
	ret 4

   .FoundMem:
	cmp	dword[edx-4],0
	jz	.EndDelMem

   .NextMoveMem:
	p2p    [edx+4],[edx-4]
	p2p    [edx],[edx-8]
	sub	edx,8
	cmp	dword[edx-4],0
	jnz	.NextMoveMem

    .EndDelMem:
	mov	dword[edx+4],0
	mov	dword[edx],0

	mov	eax,1
	pop	ebx
	leave
	ret 4

restore Pointer

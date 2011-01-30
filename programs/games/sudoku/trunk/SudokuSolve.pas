;unit SudokuSolve;

;interface

;type
;   TSudokuBoard=array[0..8,0..8] of byte;//тип поле для игры

;   function CheckSudoku(var Map: TSudokuBoard): boolean;
;   //возвращает true если поле составлено правильно

;   function Solve(var Map: TSudokuBoard): integer;
;   //решает Судоку
;   //Result = 1; - решение найдено
;   //Result = -1; - тупиклвая комбинация

;   procedure CopyArray(var Dst: TSudokuBoard;Src: TSudokuBoard);

;implementation

;type
;   TSudokuPt=record //тип точка с минимальным числом кандидатов
;       mini,minj,variances: byte;
;   end;

;var
;  TempMap: TSudokuBoard;//поля
;  Pt: TSudokuPt;            //точка
;  i,j: byte;                //счетчики



align 4
CheckSudoku:
pushad
;function CheckSudoku(var Map: TSudokuBoard): boolean;		;карта в esi!
;var
;    i,j,x,y: byte;
;    Zap: set of byte;
;begin

;//проверяем столбцы
; for i:=0 to 8 do
; begin
; Zap:=[];
; for j:=0 to 8 do
;   if Map[i,j]<>0 then
;     if Map[i,j] in Zap then
;     begin
;       Result:=false;
;       Exit;
;       end else
;       Zap:=Zap+[Map[i,j]];
; end;
	xor	ecx,ecx
	xor	eax,eax
	xor	edx,edx
	mov	bx,0x0909
.1:	mov	al, byte [esi+ecx]
	test	al,al
	jz	@f
		btc	dx,ax
		jc	.ret_false
	@@:
	add	cl,9
	dec	bl
	jnz	.1
;	test	[flags],1 shl 15
;	jz	@f
;	cmp	edx,1022 ;1111111110b
;	jne	.ret_false
;@@:
	bt	[flags],15
	jnc	@f
	cmp	dx,1022 ;1111111110
	jne	.ret_false
@@:
	sub	cl,9*9-1
	xor	edx,edx
	mov	bl,9
	dec	bh
	jnz	.1


;//проверяем строки
; for j:=0 to 8 do
; begin
; Zap:=[];
; for i:=0 to 8 do
;    if Map[i,j]<>0 then
;       if Map[i,j] in Zap then
;       begin
;       Result:=false;
;       Exit;
;       end else
;       Zap:=Zap+[Map[i,j]]
; end;

	xor	ecx,ecx
	xor	eax,eax
	xor	edx,edx
	mov	bx,0x0909
.2:	mov	al, byte [esi+ecx]
	test	al,al
	jz	@f
		btc	dx,ax
		jc	.ret_false
	@@:
	inc	ecx
	dec	bl
	jnz	.2
	bt	[flags],15
	jnc	@f
	cmp	dx,1022 ;1111111110
	jne	.ret_false
@@:
	xor	edx,edx
	mov	bl,9
	dec	bh
	jnz	.2

;//проверяем сектора
;for i:=0 to 2 do
;  for j:=0 to 2 do
;  begin
;  zap:=[];
;  for x:=0 to 2 do
;     for y:=0 to 2 do
;     if map[i*3+y,j*3+x]<>0 then
;     if map[i*3+y,j*3+x] in Zap then
;     begin
;     Result:=false;
;     exit;
;     end else
;     Zap:=zap+[map[i*3+y,j*3+x]];
;  end;
;Result:=true;
;end;
	mov	ecx,0x0303 ;ij
	xor	eax,eax
	xor	edx,edx
	mov	ebx,0x0303 ;xy

.3:	movzx	eax,ch
	dec	al
	lea	eax,[eax*2+eax]
	add	al,bl		;i*3+y
	dec al
	mov	edi,eax
	movzx	eax,cl
	dec	al
	lea	eax,[eax*2+eax]
	add	al,bh		;j*3+x
	dec al
	xchg	eax,edi
	mov	ah,9
	mul	ah
	add	eax,edi		;i*3+y,j*3+x
	mov	al,[esi+eax]
	test	al,al
	jz	@f
		btc	dx,ax
		jc	.ret_false
	@@:
	dec	bl
	jnz	.3
	mov	bl,3
	dec	bh
	jnz	.3
	bt	[flags],15
	jnc	@f
	cmp	dx,1022 ;1111111110
	jne	.ret_false
@@:
	mov	bx,0x0303
	xor	edx,edx
	dec	cl
	jnz	.3
	mov	cl,3
	dec	ch
	jnz	.3
popad
clc
ret
.ret_false:
popad
stc
ret

_iRet	dw ?

Pt:
.mini db ?
.minj db ?
.variances db ?

bFree db ?
nVariances db ?
nMinVariances db ?

align 4
FindMinPoint:
	pushad
	mov	[bFree],0
	mov	[nMinVariances],10
	mov	cx,0x0909 ;ij
.1:	movzx	eax,ch
	mov	ah,9
	dec	al
	mul	ah
	add	al,cl
	dec	al
	cmp	byte [esi+eax],0
	je	@f
.11:	dec	cl
	jnz	.1
	mov	cl,9
	dec	ch
	jnz	.1
	jmp	.3

@@:	mov	[nVariances],0
	mov	[bFree],1
	mov	ebx,1
.2:	mov	[esi+eax],bl
	call	CheckSudoku
	jc	@f
	inc	[nVariances]
@@:	inc	ebx
	cmp	ebx,9
	jbe	.2

	mov	byte [esi+eax],0
	mov	dl,[nVariances]
	cmp	dl,0
	je	.11
	cmp	dl,[nMinVariances]
	jnb	.11
	mov	[Pt.mini],ch
	mov	[Pt.minj],cl
	mov	[Pt.variances],dl
	mov	[nMinVariances],dl
	jmp	.11

.3:	cmp	[bFree],1
	jne	@f
	cmp	[nMinVariances],10
	jge	@f
	mov	[_iRet],1
	popad
	ret
@@:	cmp	[bFree],1
	jne	@f
	mov	[_iRet],-1
	popad
	ret
@@:	mov	[_iRet],0
	popad
ret


;   //ищет точку с минимальным числом кандидатов и записивает результат в MinPt
;   //Result = -1; - нашли, куда можно поставить цифру
;   //Result = 1; - свободное место есть на поле, но поставить никуда нельзя
;   //Result = 0; - нет свободного места
;function FindMinPoint(var MinPt: TSudokuPt;var Map: TSudokuBoard): integer;
;var
;nVariances,nMinVariances,variance: byte;
;bFree: boolean;
;begin
;bFree:= false;
;nMinVariances:=10;
;for i:=0 to 8 do
;  for j:=0 to 8 do
;  begin
;  if Map[i,j]<>0 then continue;
;  nVariances := 0;
;  bFree := true;
;  for variance:=1 to 9 do
;   begin
;     map[i,j]:=variance;
;     if CheckSudoku(Map) then inc(nVariances);
;   end;
;  Map[i,j]:=0;
;  if nVariances=0 then continue;
;  if nVariances < nMinVariances then
;    begin
;    MinPt.mini:=i;
;    MInPt.minj:=j;
;    MinPt.variances:=nVariances;
;    nMinVariances := nVariances;
;    end;
;  end;
;if (bFree) and (nMinVariances<10) then // нашли, куда можно поставить цифру
;  begin
;  Result:=1;
;  Exit;
;  end;
;if bFree then  // свободное место есть на поле, но поставить никуда нельзя
;   begin
;   Result:=-1;
;   exit;
;   end;
;result:=0; // нет свободного места
;end;



;procedure CopyArray(var Dst: TSudokuBoard;Src: TSudokuBoard);
;begin
;for i:=0 to 8 do
;  for j:=0 to 8 do
;   Dst[i,j]:=Src[i,j];
;end;

CopyArray:
	push	ecx eax
	xor	ecx,ecx
@@:	mov	al,[esi+ecx]
	mov	[edi+ecx],al
	inc	ecx
	cmp	ecx,9*9
	jb	@b
	pop	eax ecx
ret



align 4
Solve:
	pushad
if DEBUG
dbg_dec esp
dbg_dec ecx
mcall 5,1
endf
	call	FindMinPoint
	cmp	[_iRet],0
	jne	@f
		mov	[_iRet],1
		popad
		ret
	@@:
	cmp	[_iRet],-1
	jne	@f
		popad
		ret
@@:
	push	esi edi
	mov	edi,TempMap
	call	CopyArray
	pop	edi esi
	mov	ecx,1
	movzx	eax, byte [Pt.mini]
	dec	al
	mov	ah,9
	mul	ah
	add	al,byte [Pt.minj]
	dec	al
.1:	mov	byte [esi+eax],cl
	call	CheckSudoku
	jnc	@f
.2:	inc	ecx
	cmp	ecx,9
	jbe	.1
;	jmp	.1
@@:	call	Solve
	cmp	[_iRet],-1
	jne	@f
	push	esi edi
	mov	edi,TempMap
	xchg	esi,edi
	call	CopyArray
	pop	edi esi
	popad
	ret
@@:	cmp	[_iRet],1
	jne	.3
	popad
	ret
.3:	mov	[_iRet],0
	popad
ret

;function Solve(var Map: TSudokuBoard): integer;
;var
;  variance: byte;
;  iRet: integer;
;begin
;// ищем клетку с наименьшим числом кандидатов:
;iRet:=FindMinPoint(Pt,Map);
;if (iRet = 0) then
;   begin
;   result:=1;// решение найдено
;   exit;
;   end else
;if (iRet = -1) then // тупиковая комбинация
;     begin
;     result:=-1;
;     exit;
;     end;

;CopyArray(TempMap,Map); // сохраняем то, что есть
;for variance:=1 to 9 do
;begin
; Map[pt.mini,pt.minj]:=variance; // пытаемся ставить на найденную клетку цифры от 1 до 9
; if not CheckSudoku(Map) then continue; // такую цифру поставить нельзя. Берём следующую.
; iRet:=Solve(Map); // запускаем функцию рекурсивно
; if iRet=-1 then // тупиковая комбинация
;  begin
;  CopyArray(Map,tempmap); // восстанавливаем массив и берём следующую цифру на то же место
;  continue;
;  end;
;if iRet=1 then // решение найдено
;  begin
;  Result:=1;
;  exit;
;  end;
;end;
;Result:=0;
;end;

;end.


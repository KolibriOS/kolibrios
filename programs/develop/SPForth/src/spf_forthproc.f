( Основные низкоуровневые слова "форт-процессора"
  Copyright [C] 1992-1999 A.Cherezov ac@forth.org
  Преобразование из 16-разрядного в 32-разрядный код - 1995-96гг
  Ревизия - сентябрь 1999
)

( Реализация для подпрограммного шитого кода.
  ESP - указатель стека возвратов
  EBP - указатель стека данных
  EDI - сохраняемый регистр [указатель данных потока в SPF]
)

HEX

\ ================================================================
\ Стековые манипуляции

?HS

Code DUP ;( x -- x x ) \ 94
; Продублировать x.
     LEA EBP, [EBP-4]
     mov [ebp],eax
     RET
EndCode


\ ' DUP TO 'DUP_V

Code ?DUP ;( x -- 0 | x x ) \ 94
; Продублировать x, если не ноль.
     OR  EAX, EAX
     JNZ { ' DUP }
     RET
EndCode

Code 2DUP ;( x1 x2 -- x1 x2 x1 x2 ) \ 94
; Продублировать пару ячеек x1 x2.
     MOV EDX, [EBP]
     MOV [EBP-4], EAX
     MOV [EBP-8], EDX
     LEA EBP, [EBP-8]
     RET
EndCode

Code DROP ;( x -- ) \ 94
; Убрать x со стека.
     mov eax,[ebp]
     LEA EBP, [EBP+4]
     RET
EndCode

\ ' DROP TO 'DROP_V

Code MAX ;( n1 n2 -- n3 ) \ 94
; n3 - большее из n1 и n2.
     CMP EAX, [EBP]
     JL  { ' DROP }
     LEA EBP, [EBP+4]
     RET
EndCode

Code MIN ;( n1 n2 -- n3 ) \ 94
 ; n3 - меньшее из n1 и n2.
     CMP EAX, [EBP]
     JG  { ' DROP }
     LEA EBP, [EBP+4]
     RET
EndCode

Code UMAX       ;( u1 u2 -- n3 ) \ RETurn the lesser of unsigned u1 and
                                ; unsigned u2
     CMP  EAX, [EBP]
     JB { ' DROP }
     LEA EBP, [EBP+4]
     RET
EndCode

Code UMIN       ;( u1 u2 -- n3 ) \ RETurn the lesser of unsigned u1 and
                                ; unsigned u2
     CMP EAX, [EBP]
     JA { ' DROP }
     LEA EBP, [EBP+4]
     RET
EndCode

Code 2DROP ;( x1 x2 -- ) \ 94
; Убрать со стека пару ячеек x1 x2.
     MOV     EAX , [EBP+4]
     ADD     EBP , 8
     RET
EndCode

Code SWAP ;( x1 x2 -- x2 x1 ) \ 94
; поменять местами два верхних элемента стека
;     XCHG EAX  { EBP }
     MOV   EDX, [EBP]
     MOV   [EBP],  EAX
     MOV   EAX, EDX
     RET
EndCode

Code 2SWAP ;( x1 x2 x3 x4 -- x3 x4 x1 x2 ) \ 94
; Поменять местами две верхние пары ячеек.
     MOV ECX, [EBP]
     MOV EBX, [EBP+4]
     MOV EDX, [EBP+8]
     MOV [EBP+8], ECX
     MOV [EBP+4], EAX
     MOV [EBP], EDX
     MOV EAX, EBX
     RET
EndCode

Code OVER ;( x1 x2 -- x1 x2 x1 ) \ 94
; Положить копию x1 на вершину стека.
     LEA EBP, [EBP-4]
     MOV  [EBP],  EAX
     MOV  EAX, [EBP+4]
     RET
EndCode

Code 2OVER ;( x1 x2 x3 x4 -- x1 x2 x3 x4 x1 x2 ) \ 94
; Копировать пару ячеек x1 x2 на вершину стека.
     MOV EDX, [EBP+8]
     MOV [EBP-4], EAX
     MOV [EBP-8], EDX
     MOV EAX, [EBP+4]
     LEA EBP, [EBP-8]
     RET
EndCode

Code NIP ;( x1 x2 -- x2 ) \ 94 CORE EXT
; Убрать первый элемент под вершиной стека.
     ADD EBP,  4
     RET
EndCode

Code ROT ;( x1 x2 x3 -- x2 x3 x1 ) \ 94
; Прокрутить три верхних элемента стека.
;     XCHG EAX     [EBP]
;     XCHG EAX   4  [EBP]
     MOV  EDX, [EBP]
     MOV  [EBP], EAX
     MOV  EAX, [EBP+4]
     MOV  [EBP+4], EDX
     RET
EndCode


Code -ROT ;( x1 x2 x3 -- x3 x1 x2 ) ; !!!!!
; Обратное ROT
     MOV  EDX, [EBP+4]
     MOV  [EBP+4], EAX
     MOV  EAX, [EBP]
     MOV  [EBP], EDX
     RET
EndCode

Code PICK      ;( ... +n -- ... w ) \ Copy the nth stack item to tos.
      MOV    EAX, [EBP + EAX*4 ]
      RET
EndCode

Code ROLL ;( xu xu-1 ... x0 u -- xu-1 ... x0 xu ) \ 94 CORE EXT
; Убрать u. Повернуть u+1 элемент на вершине стека.
; Неопределенная ситуация возникает, если перед выполнением ROLL
; на стеке меньше чем u+2 элементов.
     OR EAX, EAX
     JZ SHORT LL1
     MOV ECX, EAX
     LEA EAX, [EAX*4]
     MOV EDX, EBP
     ADD EDX, EAX
     MOV EBX, [EDX]
LL2: LEA EDX, [EDX-4]
     MOV EAX, [EDX]
     MOV [EDX+4], EAX
     DEC ECX
     JNZ SHORT LL2
     MOV EAX, EBX
     JMP SHORT LL3
LL1: MOV EAX, [EBP]
LL3: LEA EBP, [EBP+4]
     RET
EndCode

Code TUCK ;( x1 x2 -- x2 x1 x2 ) \ 94
     LEA EBP, [EBP-4]
     MOV  EDX, [EBP+4]
     MOV  [EBP],  EDX
     MOV  [EBP+4],  EAX
     RET
EndCode

\ ================================================================
\ Стек возвратов

Code 2>R   ; 94 CORE EXT
; Интерпретация: семантика неопределена.
; Выполнение: ;( x1 x2 -- ) ;( R: -- x1 x2 )
; Перенести пару ячеек x1 x2 на стек возвратов. Семантически
; эквивалентно SWAP >R >R.
     POP   EBX
     PUSH  DWORD PTR [EBP]
     PUSH  EAX
     LEA   EBP, [EBP+8]
     MOV   EAX, [EBP-4]
     JMP   EBX
EndCode

Code 2R>  ; 94 CORE EXT
; Интерпретация: семантика неопределена.
; Выполнение: ;( -- x1 x2 ) ;( R: x1 x2 -- )
; Перенести пару ячеек x1 x2 со стека возвратов. Семантически
; эквивалентно R> R> SWAP.  \ !!!!
     LEA EBP, [EBP-8]
     POP EBX
     MOV [EBP+4], EAX
     POP EAX
     POP DWORD PTR [EBP]
     PUSH EBX
     RET
EndCode

Code R@    ; 94
; Исполнение: ;( -- x ) ;( R: x -- x )
; Интерпретация: семантика в режиме интерпретации неопределена.
     LEA EBP, [EBP-4]
     MOV [EBP], EAX
     MOV EAX, [ESP + 4 ]
     RET
EndCode

Code 2R@  ; 94 CORE EXT
; Интерпретация: семантика неопределена.
; Выполнение: ;( -- x1 x2 ) ;( R: x1 x2 -- x1 x2 )
; Копировать пару ячеек x1 x2 со стека возвратов. Семантически
; эквивалентно R> R> 2DUP >R >R SWAP.
     LEA EBP, [EBP-8]
     MOV [EBP+4], EAX
     MOV EAX, [ESP + { 2 CELLS } ]
     MOV DWORD PTR [EBP],  EAX
     MOV  EAX, [ESP + 4 ]
     RET
EndCode

\ ================================================================
\ Операции с памятью

Code @ ;( a-addr -- x ) \ 94
; x - значение по адресу a-addr.
     MOV EAX, [EAX ]
     RET
EndCode

Code ! ;( x a-addr -- ) \ 94
; Записать x по адресу a-addr.
     MOV EDX, [EBP]
     MOV DWORD PTR [EAX ],  EDX
     MOV EAX , [EBP+4]
     ADD EBP , 8
     RET
EndCode

Code C@ ;( c-addr -- char ) \ 94
; Получить символ по адресу c-addr. Незначащие старшие биты ячейки нулевые.
     MOVZX EAX, BYTE PTR [EAX ]
     RET
EndCode

Code C! ;( char c-addr -- ) \ 94
; Записать char по адресу a-addr.
     MOV EDX, [EBP]
     MOV BYTE PTR [EAX ],  DL
     MOV EAX , [EBP+4]
     ADD EBP , 8
     RET
EndCode

Code W@ ;( c-addr -- word )
; Получить word по адресу c-addr. Незначащие старшие биты ячейки нулевые.
     MOVZX   EAX, WORD PTR [EAX ]
     RET
EndCode

Code W! ;( word c-addr -- )
; Записать word по адресу a-addr.
     MOV EDX, [EBP]
     MOV WORD PTR [EAX ],  DX
     MOV   EAX , [EBP+4]
     ADD   EBP , 8
     RET
EndCode

Code 2@ ;( a-addr -- x1 x2 ) \ 94
; Получить пару ячеек x1 x2, записанную по адресу a-addr.
; x2 по адресу a-addr, x1 в следующей ячейке.
; Равносильно DUP CELL+ @ SWAP @
     MOV EDX, [EAX + 4 ]
     LEA EBP, [EBP-4]
     MOV DWORD PTR [EBP],  EDX
     MOV EAX,    DWORD PTR [EAX ]
     RET
EndCode

Code 2! ;( x1 x2 a-addr -- ) \ 94
; Записать пару ячеек x1 x2 по адресу a-addr,
; x2 по адресу a-addr, x1 в следующую ячейку.
; Равносильно SWAP OVER ! CELL+ !
     MOV EDX, [EBP]
     MOV [EAX], EDX
     MOV EDX, [EBP+4]
     MOV [EAX+4], EDX
     LEA EBP, [EBP+0CH]
     MOV EAX, [EBP-4]
     RET
EndCode

Code D@ ;( a-addr -- x1 x2 )
; 2@ SWAP
     MOV EDX, [EAX]
     LEA EBP, [EBP-4]
     MOV DWORD PTR [EBP],  EDX
     MOV EAX,    DWORD PTR [EAX+4]
     RET
EndCode

Code D! ;( x1 x2 a-addr -- )
; >R SWAP R> 2!
	MOV EDX, [EBP]
	MOV [EAX+4], EDX
	MOV EDX, [EBP+4]
	MOV [EAX], EDX
	LEA EBP, [EBP+0CH]
	MOV EAX, [EBP-4]
	RET
EndCode


Code EBX@ ;( -- EBX )
	LEA EBP, [EBP-4]
	mov [ebp],eax
	MOV EAX,EBX
	RET
EndCode

\ ================================================================
\ Вычисления

Code 1+ ;( n1|u1 -- n2|u2 ) \ 94
; Прибавить 1 к n1|u1 и получить сумму u2|n2.
     LEA EAX, [EAX+1]
     RET
EndCode

Code 1- ;( n1|u1 -- n2|u2 ) \ 94
; Вычесть 1 из n1|u1 и получить разность n2|u2.
     LEA EAX, [EAX-1]
     RET
EndCode

Code 2+ ;( W -> W+2 )
     LEA EAX, [EAX+2]
     RET
EndCode

Code 2- ;( W -> W-2 )
     LEA EAX, [EAX-2]
     RET
EndCode

Code 2* ;( x1 -- x2 ) \ 94
; x2 - результат сдвига x1 на один бит влево, с заполнением
; наименее значимого бита нулем.
;  SHL EAX
;  LEA EAX, [EAX+EAX]
  LEA EAX, [EAX*2]
  RET 
EndCode

Code CELL+ ;( a-addr1 -- a-addr2 ) \ 94
; Вычесть размер ячейки к a-addr1 и получить a-addr2.
     LEA EAX, [EAX+4]
     RET
EndCode

Code CELL- ;( a-addr1 -- a-addr2 ) \ 94
; Вычесть размер ячейки к a-addr1 и получить a-addr2.
     LEA EAX, [EAX-4]
     RET
EndCode

Code CELLS ;( n1 -- n2 ) \ 94
; n2 - размер n1 ячеек.
  LEA     EAX,  DWORD PTR [EAX *4 ]
  RET 
EndCode


Code + ;( n1|u1 n2|u2 -- n3|u3 ) \ 94
; Сложить n1|u1 и n2|u2 и получить сумму n3|u3.
     ADD EAX,  DWORD PTR [EBP]
     LEA EBP, [EBP+4]
     RET
EndCode

Code D+ ;( d1|ud1 d2|ud2 -- d3|ud3 ) \ 94 DOUBLE
; Сложить d1|ud1 и d2|ud2 и дать сумму d3|ud3.
       MOV EDX,    DWORD PTR [EBP]
       ADD DWORD PTR [EBP + { 2 CELLS } ],  EDX
       ADC EAX, DWORD PTR [EBP +4 ]
       LEA EBP, [EBP+8]
       RET
EndCode

Code D- ;( d1 d2 -- d3 ) \ 94 DOUBLE
; perform a double subtract (64bit)
       MOV EDX,   DWORD PTR [EBP]
       SUB DWORD PTR [EBP + { 2 CELLS } ],  EDX
       SBB [EBP+4], EAX
       MOV EAX, DWORD PTR [EBP +4 ]
       LEA EBP, [EBP+8]
       RET
EndCode
               
Code - ;( n1|u1 n2|u2 -- n3|u3 ) \ 94
; Вычесть n2|u2 из n1|u1 и получить разность n3|u3.
     NEG EAX
     ADD EAX, [EBP]
     LEA EBP, [EBP+4]
     RET
EndCode

Code 1+! ;( A -> )
     INC   DWORD PTR [EAX ]
     MOV   EAX, [EBP]
     LEA EBP, [EBP+4]
     RET
EndCode

Code 0! ;( A -> )
     MOV   DWORD PTR [EAX ], 0 
     MOV   EAX, [EBP]
     LEA EBP, [EBP+4]
     RET
EndCode

Code COUNT ;( c-addr1 -- c-addr2 u ) \ 94
; Получить строку символов из строки со счетчиком c-addr1.
; c-addr2 - адрес первого символа за c-addr1.
; u - содержимое байта c-addr1, являющееся длиной строки символов,
; начинающейся с адреса c-addr2.
     LEA EBP, [EBP-4]
     LEA  EDX,   DWORD PTR [EAX +1 ]
     MOV  DWORD PTR [EBP],    EDX
     MOVZX EAX,  BYTE PTR [EAX ]
     RET
EndCode

Code * ;( n1|u1 n2|u2 -- n3|u3 ) \ 94
; Перемножить n1|u1 и n2|u2 и получить произведение n3|u3.
     IMUL DWORD PTR [EBP]
     LEA EBP, [EBP+4]
     RET
EndCode

Code AND ;( x1 x2 -- x3 ) \ 94
; x3 - побитовое "И" x1 и x2.
     AND EAX, [EBP]
     LEA EBP, [EBP+4]
     RET
EndCode

Code OR ;( x1 x2 -- x3 ) \ 94
; x3 - побитовое "ИЛИ" x1 и x2.
     OR EAX, [EBP]
     LEA EBP, [EBP+4]
     RET
EndCode

Code XOR ;( x1 x2 -- x3 ) \ 94
; x3 - побитовое "исключающее ИЛИ" x1 и x2.
     XOR EAX, [EBP]
     LEA EBP, [EBP+4]
     RET
EndCode

Code INVERT ;( x1 -- x2 ) \ 94
; Инвертировать все биты x1 и получить логическую инверсию x2.
     NOT EAX
     RET
EndCode

Code NEGATE ;( n1 -- n2 ) \ 94
; n2 - арифметическая инверсия n1.
     NEG EAX
     RET
EndCode

Code ABS ;( n -- u ) \ 94
; u - абсолютная величина n.
     TEST  EAX, EAX
     JS  { ' NEGATE }
     RET
EndCode

Code DNEGATE ;( d1 -- d2 ) \ 94 DOUBLE
; d2 результат вычитания d1 из нуля.
     MOV  EDX, [EBP]
     NEG  EAX
     NEG  EDX
     SBB  EAX,   0
     MOV  DWORD PTR [EBP],  EDX
     RET
EndCode

Code NOOP ;( -> )
     RET
EndCode

Code S>D ;( n -- d ) \ 94
; Преобразовать число n в двойное число d с тем же числовым значением.
     LEA EBP, [EBP-4]
     MOV  [EBP], EAX
     CDQ
     MOV  EAX, EDX
     RET
EndCode

Code D>S ;( d -- n ) \ 94 DOUBLE
; n - эквивалент d.
; Исключительная ситуация возникает, если d находится вне диапазона
; знаковых одинарных чисел.
     MOV     EAX, [EBP]
     ADD     EBP, 4
     RET
EndCode

Code U>D ;( U -> D ) \ расширить число до двойной точности нулем
     LEA EBP, [EBP-4]
     MOV  [EBP], EAX
     XOR  EAX, EAX
     RET
EndCode

Code C>S ;( c -- n )  \ расширить CHAR
     MOVSX  EAX, AL
     RET
EndCode

Code UM* ;( u1 u2 -- ud ) \ 94
; ud - произведение u1 и u2. Все значения и арифметика беззнаковые.
       MUL  DWORD PTR [EBP] 
       MOV  [EBP], EAX
       MOV  EAX, EDX
       RET
EndCode

Code / ;( n1 n2 -- n3 ) \ 94
; Делить n1 на n2, получить частное n3.
; Исключительная ситуация возникает, если n2 равен нулю.
; Если n1 и n2 различаются по знаку - возвращаемый результат зависит от
; реализации.
     MOV  ECX,  EAX
     MOV  EAX, [EBP]
     CDQ
     IDIV ECX
     LEA EBP, [EBP+4]
     RET
EndCode

Code U/ ;( W1, W2 -> W3 ) \ беззнаковое деление W1 на W2
     MOV ECX, EAX
     MOV EAX, [EBP]
     XOR EDX, EDX
     LEA EBP, [EBP+4]
     DIV ECX
     RET
EndCode

Code +! ;( n|u a-addr -- ) \ 94     \ !!!!!
; Прибавить n|u к одинарному числу по адресу a-addr.
     MOV EDX, [EBP]
     ADD DWORD PTR [EAX ],  EDX
     MOV EAX, [EBP+4]
     LEA EBP, [EBP+8]
     RET
EndCode

Code MOD ;( n1 n2 -- n3 ) \ 94
; Делить n1 на n2, получить остаток n3.
; Исключительная ситуация возникает, если n2 равен нулю.
; Если n1 и n2 различаются по знаку - возвращаемый результат зависит от
; реализации.
     MOV  ECX,  EAX
     MOV  EAX, [EBP]
     LEA EBP, [EBP+4]
     CDQ
     IDIV ECX
     MOV  EAX, EDX
     RET
EndCode

Code /MOD ;( n1 n2 -- n3 n4 ) \ 94
; Делить n1 на n2, дать остаток n3 и частное n4.
; Неоднозначная ситуация возникает, если n2 нуль.
       MOV ECX, EAX
       MOV EAX, [EBP]
       CDQ
       IDIV ECX
       MOV [EBP], EDX
       RET
EndCode

Code UMOD ;( W1, W2 -> W3 ) \ остаток от деления W1 на W2
     MOV ECX,  EAX
     XOR EDX, EDX
     MOV EAX, [EBP]
     LEA EBP, [EBP+4]
     DIV ECX
     MOV EAX, EDX
     RET
EndCode

Code UM/MOD ;( ud u1 -- u2 u3 ) \ 94
; Делить ud на u1, получить частное u3 и остаток u2.
; Все значения и арифметика беззнаковые.
; Исключительная ситуация возникает, если u1 ноль или частное
; находится вне диапазона одинарных беззнаковых чисел.
       MOV ECX, EAX
       MOV EDX, [EBP]
       MOV EAX, [EBP+4]
       DIV ECX
       LEA EBP, [EBP+4]
       MOV [EBP], EDX

       RET
EndCode

Code 2/ ;( x1 -- x2 ) \ 94
; x2 - результат сдвига x1 на один бит вправо без изменения старшего бита.
     SAR   EAX,1
     RET
EndCode

Code */MOD ;( n1 n2 n3 -- n4 n5 ) \ 94
; Умножить n1 на n2, получить промежуточный двойной результат d.
; Разделить d на n3, получить остаток n4 и частное n5.
     MOV     EBX, EAX
     MOV     EAX, [EBP]
     MOV     ECX, [EBP+4]
     IMUL    ECX
     IDIV    EBX
     MOV  [EBP+4], EDX
     LEA EBP, [EBP+4]
     RET
EndCode

Code M* ;( n1 n2 -- d ) \ 94
; d - знаковый результат умножения n1 на n2.
     IMUL DWORD PTR [EBP]
     MOV  [EBP], EAX
     MOV  EAX,  EDX
     RET
EndCode

Code LSHIFT ;( x1 u -- x2 ) ; 94
; Сдвинуть x1 на u бит влево. Поместить нули в наименее значимые биты,
; освобождаемые при сдвиге.
; Неоднозначная ситуация возникает, если u больше или равно
; числу бит в ячейке.
     MOV ECX, EAX
     MOV EAX, [EBP]
     SHL EAX, CL
     LEA EBP, [EBP+4]
     RET
EndCode

Code RSHIFT ;( x1 u -- x2 ) \ 94
; Сдвинуть x1 на u бит вправо. Поместить нули в наиболее значимые биты,
; освобождаемые при сдвиге.
; Неоднозначная ситуация возникает, если u больше или равно
; числу бит в ячейке.
     MOV ECX, EAX
     MOV EAX, [EBP]
     SHR EAX, CL
     LEA EBP, [EBP+4]
     RET
EndCode

Code SM/REM ;( d1 n1 -- n2 n3 ) \ 94
; Разделить d1 на n1, получить симметричное частное n3 и остаток n2.
; Входные и выходные аргументы знаковые.
; Неоднозначная ситуация возникает, если n1 ноль, или частное вне
; диапазона одинарных знаковых чисел.
     MOV EBX, EAX
     MOV EDX, [EBP]
     MOV EAX, [EBP+4]
     IDIV EBX
     LEA EBP, [EBP+4]
     MOV [EBP], EDX
     RET
EndCode

Code FM/MOD ;( d1 n1 -- n2 n3 ) \ 94
; ╨рчфхышЄ№ d1 эр n1, яюыєўшЄ№ ўрёЄэюх n3 ш юёЄрЄюъ n2.
; ┬їюфэ√х ш т√їюфэ√х рЁуєьхэЄ√ чэръют√х.
; ═хюфэючэрўэр  ёшЄєрЎш  тючэшърхЄ, хёыш n1 эюы№, шыш ўрёЄэюх тэх
; фшрярчюэр юфшэрЁэ√ї чэръют√ї ўшёхы.
        MOV ECX, EAX
        MOV EDX, [EBP]
        MOV EBX, EDX
        MOV EAX, [EBP+4]
        IDIV ECX
        TEST EDX, EDX            ; ╬ёЄрЄюъ-Єю хёЄ№?
        JZ  SHORT @@1
        XOR EBX, ECX             ; └ рЁуєьхэЄ√ Ёрчэюую чэрър?
        JNS SHORT @@1
        DEC EAX
        ADD EDX, ECX
@@1:    LEA EBP, [EBP+4]
        MOV [EBP], EDX
        RET
EndCode

\ ================================================================
\ Сравнения

Code = ;( x1 x2 -- flag ) \ 94
; flag "истина" тогда и только тогда, когда x1 побитно равен x2.
     XOR  EAX, [EBP]
     SUB  EAX, 1
     SBB  EAX, EAX
     LEA EBP, [EBP+4]
     RET
EndCode

Code <> ;( x1 x2 -- flag ) \ 94 CORE EXT
; flag "истина" тогда и только тогда, когда x1 не равен x2.
     XOR  EAX, [EBP]
     NEG  EAX
     SBB  EAX,  EAX
     LEA EBP, [EBP+4]
     RET
EndCode

Code < ;( n1 n2 -- flag ) \ 94
; flag "истина" тогда и только тогда, когда n1 меньше n2.
       CMP  [EBP], EAX
       SETGE AL
       AND  EAX, 01
       DEC  EAX
       LEA EBP, [EBP+4]
       RET
EndCode

Code > ;( n1 n2 -- flag ) \ 94
; flag "истина" тогда и только тогда, когда n1 больше n2.
       CMP  EAX, [EBP]
       SETGE AL
       AND  EAX, 01
       DEC  EAX
       LEA EBP, [EBP+4]
       RET
EndCode

Code WITHIN     ;( n1 low high -- f1 ) \ f1=true if ((n1 >= low) & (n1 < high))
      MOV  EBX, [EBP+4]
      SUB  EAX, [EBP]
      SUB  EBX, [EBP]
      SUB  EBX, EAX
      SBB  EAX, EAX
      ADD  EBP, 8
      RET
EndCode

Code D< ;( d1 d2 -- flag ) \ DOUBLE
; flag "истина" тогда и только тогда, когда d1 меньше d2.
     MOV EBX, [EBP]
     CMP DWORD PTR [EBP +8 ], EBX
     SBB DWORD PTR [EBP +4 ], EAX
     MOV EAX,  0
     SIF <
       DEC EAX
     STHEN
     ADD EBP,   0CH
     RET
EndCode

Code D> ;( d1 d2 -- flag ) \ DOUBLE
; flag "истина" тогда и только тогда, когда d1 больше d2.
    MOV EBX, [EBP]
    CMP EBX, [EBP+8]
    SBB EAX, [EBP+4]
    SAR EAX,   1FH
    ADD EBP,   0CH
    RET
EndCode

Code U< ;( u1 u2 -- flag ) \ 94
; flag "истина" тогда и только тогда, когда u1 меньше u2.
    CMP  [EBP], EAX
    SBB  EAX, EAX
    ADD  EBP,  04
    RET
EndCode

Code U> ;( u1 u2 -- flag ) \ 94
; flag "истина" тогда и только тогда, когда u1 больше u2.
    CMP  EAX, [EBP]
    SBB  EAX, EAX
    ADD  EBP,   04
    RET
EndCode

Code 0< ;( n -- flag ) \ 94
; flag "истина" тогда и только тогда, когда n меньше нуля.
    SAR EAX,   1F
    RET
EndCode

Code 0= ;( x -- flag ) \ 94
; flag "истина" тогда и только тогда, когда x равно нулю.
     SUB  EAX, 1
     SBB  EAX, EAX 
     RET
EndCode

Code 0<> ;( x -- flag ) \ 94 CORE EXT
; flag "истина" тогда и только тогда, когда x не равно нулю.
     NEG  EAX
     SBB  EAX, EAX
     RET
EndCode

Code D0= ;( xd -- flag ) \ 94 DOUBLE
; flag "истина" тогда и только тогда, когда xd равен нулю.
     OR   EAX, [EBP]
     SUB  EAX, 1
     SBB  EAX, EAX 
     LEA EBP, [EBP+4]
     RET
EndCode

Code D= ;( xd1 xd2 -- flag ) \ 94 DOUBLE
; flag is true if and only if xd1 is bit-for-bit the same as xd2
     MOV  EDX,[EBP]
     XOR  EAX,[EBP+4]
     XOR  EDX,[EBP+8]
      OR  EAX,EDX
     SUB  EAX,1
     SBB  EAX,EAX
     LEA  EBP,[EBP+0CH]
     RET
EndCode

Code D2* ;( xd1 -- xd2 ) \ 94 DOUBLE
; xd2 is the result of shifting xd1 one bit toward the most-significant
; bit, filling the vacated least-significant bit with zero     
     SHL DWORD PTR [EBP],  1
     RCL  EAX,   1
     RET
EndCode

Code D2/ ;( xd1 -- xd2 ) \ 94 DOUBLE
; xd2 is the result of shifting xd1 one bit toward the least-significant bit,
; leaving the most-significant bit unchanged
     SAR  EAX,  1
     RCR DWORD PTR [EBP], 1
     RET
EndCode

\ ================================================================
\ Строки

Code -TRAILING ;( c-addr u1 -- c-addr u2 ) \ 94 STRING
; Если u1 больше нуля, u2 равно u1, уменьшенному на количество пробелов в конце
; символьной строки, заданной c-addr и u1. Если u1 ноль или вся строка состоит
; из пробелов, u2 ноль.
     PUSH EDI
     MOV  ECX, EAX
     SIF C0<>
       MOV EDI, DWORD PTR [EBP]
       ADD EDI, ECX
       DEC EDI 
       MOV AL,  20H
       STD
       REPZ SCASB
       SIF 0<>
          INC ECX 
       STHEN
       CLD
       MOV  EAX, ECX 
     STHEN
     POP EDI
     RET
EndCode

Code COMPARE ;( c-addr1 u1 c-addr2 u2 -- n ) \ 94 STRING !!!!!
; Сравнить строку, заданную c-addr1 u1, со строкой, заданной c-addr2 u2.
; Строки сравниваются, начиная с заданных адресов, символ за символом, до длины
; наиболее короткой из строк или до нахождения различий. Если две строки
; идентичны, n ноль. Если две строки идентичны до длины наиболее короткой из
; строк, то n минус единица (-1), если u1 меньше u2, иначе единица (1).
; Если две строки не идентичны до длины наиболее короткой из строк, то n минус
; единица (-1), если первый несовпадающий символ строки, заданной c-addr1 u1
; имеет меньшее числовое значение, чем соответствующий символ в строке,
; заданной c-addr2 u2, и единица в противном случае.
   PUSH EDI
   MOV  ECX, EAX
   SUB  EAX, EAX
   CMP  ECX, [EBP +4 ]
   SIF 0<>
      SIF U<
         INC   EAX
      SELSE
         DEC EAX
         MOV ECX, [EBP +4 ]
      STHEN
   STHEN
   MOV ESI, [EBP + { 2 CELLS } ]
   MOV EDI, [EBP]
   REPE CMPSB
   SIF 0<>
      SIF U>=
         MOV EAX,  1
      SELSE
         MOV EAX, -1
      STHEN
   STHEN
   LEA  EBP, [EBP + { 3 CELLS } ]
   POP EDI          
   RET
EndCode

Code SEARCH ;( c-addr1 u1 c-addr2 u2 -- c-addr3 u3 flag ) \ 94 STRING
; Произвести поиск в строке, заданной c-addr1 u1, строки, заданной c-addr2 u2.
; Если флаг "истина", совпадение найдено по адресу c-addr3 с оставшимися u3
; символами. Если флаг "ложь", совпадения не найдено, и c-addr3 есть c-addr1,
; и u3 есть u1.        \ !!!!!
      LEA EBP, [EBP-4]
      MOV  [EBP], EAX
      PUSH EDI
      CLD
      MOV EBX, DWORD PTR [EBP]
      OR  EBX, EBX
      SIF 0<> 
        MOV EDX, DWORD PTR [EBP + { 2 CELLS } ]
        MOV EDI, DWORD PTR [EBP + { 3 CELLS } ]
        ADD EDX, EDI
        SBEGIN
           MOV ESI, DWORD PTR [EBP +4 ]
           LODSB
           MOV ECX, EDX
           SUB ECX, EDI
           JECXZ LLD
           REPNZ
           SCASB
           JNE SHORT LLD   ; во всей строке нет первого символа искомой строки
           CMP EBX,   1
           JZ SHORT LLC   ; искомая строка имела длину 1 и найдена
           MOV ECX, EBX
           DEC ECX
           MOV EAX, EDX
           SUB EAX, EDI
           CMP EAX, ECX
           JC SHORT LLD   ; остаток строки короче искомой строки
           PUSH EDI
           REPZ CMPSB
           POP EDI
        SUNTIL 0=
LLC:    DEC EDI       ; нашли полное совпадение
        SUB EDX, EDI
        MOV  DWORD PTR [EBP + { 3 CELLS } ], EDI
        MOV  DWORD PTR [EBP + { 2 CELLS } ], EDX
      STHEN
      MOV EAX,   -1 
      JMP SHORT LLA
LLD:  XOR EAX, EAX
LLA:  LEA EBP, [EBP+4]
      MOV [EBP], EAX
      POP EDI
      MOV EAX, [EBP]
      LEA EBP, [EBP+4]
      RET
EndCode

Code CMOVE ;( c-addr1 c-addr2 u -- ) \ 94 STRING
; Если u больше нуля, копировать u последовательных символов из пространства
; данных начиная с адреса c-addr1 в c-addr2, символ за символом, начиная с
; младших адресов к старшим.
     MOV  EDX, EDI
     MOV  ECX, EAX
     MOV  EDI, DWORD PTR [EBP]
     MOV  ESI, DWORD PTR [EBP +4 ]
     CLD
     REPZ   MOVSB
     LEA EBP, [EBP+0CH]
     MOV EAX, [EBP-4]
     MOV EDI, EDX
     RET
EndCode

: QCMOVE CMOVE ;

Code CMOVE> ;( c-addr1 c-addr2 u -- ) \ 94 STRING
; Если u больше нуля, копировать u последовательных символов из пространства
; данных начиная с адреса c-addr1 в c-addr2, символ за символом, начиная со
; старших адресов к младшим.

       MOV EDX, EDI
       MOV ECX, EAX
       MOV EDI, [EBP]
       MOV ESI, [EBP+4]
       STD
       ADD EDI, ECX
       DEC EDI
       ADD ESI, ECX
       DEC ESI
       REP MOVSB
       MOV EDI, EDX
       LEA EBP, [EBP+0CH]
       MOV EAX, [EBP-4]
     RET
EndCode

Code FILL ;( c-addr u char -- ) \ 94  \ !!!!!
; Если u больше нуля, заслать char в u байтов по адресу c-addr.
   MOV EDX, EDI
   MOV ECX, [EBP]
   MOV EDI, [EBP+4]
   CLD
   REP STOSB
   MOV EDI, EDX
   LEA EBP, [EBP+0CH]
   MOV EAX, [EBP-4]
   RET
EndCode

Code ZCOUNT ;( c-addr -- c-addr u )
     LEA EBP, [EBP-4]
     MOV  [EBP], EAX
     XOR  EBX, EBX
     SBEGIN
       MOV  BL, BYTE PTR [EAX ]
       INC  EAX 
       OR   BL,  BL
     SUNTIL 0=
     DEC  EAX
     SUB  EAX, [EBP]
     RET
EndCode

\ ================================================================
\ Указатели стеков

Code SP! ;( A -> )
     LEA EBP, [EAX+4]
     MOV EAX, [EBP-4]
     RET
EndCode

Code RP! ;( A -> )
     POP EBX
     MOV ESP, EAX
     MOV EAX, [EBP]
     LEA EBP, [EBP+4]
     JMP EBX
EndCode

Code SP@ ;( -> A )
     LEA EBP, [EBP-4]
     MOV [EBP], EAX
     MOV EAX, EBP
     RET
EndCode

Code RP@ ;( -- RP )
     LEA EBP, [EBP-4]
     MOV  [EBP], EAX
     LEA  EAX, [ESP + 4 ]
     RET
EndCode


\ ================================================================
\ Регистр потока (задачи внутри форта)

Code TlsIndex! ;( x -- ) \ указатель локального пула потока
     MOV  EDI, EAX
     MOV  EAX, [EBP]
     LEA EBP, [EBP+4]
     RET
EndCode

Code TlsIndex@ ;( -- x )
     LEA EBP, [EBP-4]
     MOV [EBP], EAX
     MOV  EAX, EDI
     RET
EndCode

\ ================================================================
\ Циклы

Code C-J
	LEA EBP, [EBP-4]
	MOV  [EBP], EAX
	MOV EAX, DWORD PTR [ESP + { 3 CELLS } ]
	SUB EAX, DWORD PTR [ESP + { 4 CELLS } ]
	RET
EndCode

( inline'ы для компиляции циклов )

Code C-DO
      LEA  EBP, [EBP+8]
      MOV  EDX, 80000000H
      SUB  EDX, [EBP-8]
      LEA  EBX, [EAX+EDX]
      MOV  EAX, [EBP-4]
      MOV  EDX, EDX  ; FOR OPT
;      PUSH EDX
;      PUSH EBX
      RET
EndCode

Code C-?DO
      CMP  EAX, [EBP-8]
      SIF  0=
        MOV  EAX, [EBP-4]
        JMP  EBX
      STHEN
      PUSH EBX
      MOV     EBX , 80000000
      SUB  EBX, [EBP-8]
      PUSH EBX  ; 80000000h-to
      ADD  EBX, EAX
      PUSH EBX  ; 80000000H-to+from
      MOV  EAX, [EBP-4]
      RET
EndCode

Code ADD[ESP],EAX 
  ADD [ESP] , EAX 
 RET
EndCode

Code C-I
   LEA EBP, [EBP-4]
   MOV  [EBP], EAX
   MOV  EAX, DWORD PTR [ESP]
   SUB  EAX, [ESP+4]
   RET
EndCode

Code C->R
     PUSH EAX
     MOV  EAX, [EBP]
     LEA  EBP, [EBP+4]
     RET
EndCode

Code C-R>
     LEA  EBP, [EBP-4]
     MOV  [EBP], EAX
     POP EAX
     RET
EndCode

Code C-RDROP
     ADD  ESP, 4 
     RET
EndCode

Code C-2RDROP
     ADD  ESP, 8
     RET
EndCode

Code C-3RDROP
     ADD  ESP, 0CH
     RET
EndCode

TRUE [IF]
Code C-EXECUTE ;( i*x xt -- j*x ) \ 94
; Убрать xt со стека и выполнить заданную им семантику.
; Другие изменения на стеке определяются словом, которое выполняется.
     MOV  EDX, EAX
     MOV  EAX, [EBP]
     LEA  EBP, [EBP+4]
     CALL EDX
     RET
EndCode
[THEN]

Code EXECUTE ;( i*x xt -- j*x ) \ 94
; Убрать xt со стека и выполнить заданную им семантику.
; Другие изменения на стеке определяются словом, которое выполняется.
     MOV EBX, EAX
     MOV EAX, [EBP]
     LEA EBP, [EBP+4]
     JMP EBX
EndCode

Code @EXECUTE ;( i*x xt -- j*x )
     MOV EBX, EAX
     MOV EAX, [EBP]
     LEA EBP, [EBP+4]
     JMP [EBX]
EndCode

\ ================================================================
\ Поддержка LOCALS

Code DRMOVE ;( x1 ... xn n*4 -- )
; перенести n чисел со стека данных на стек возвратов
     POP  EDX ; адрес возврата
     MOV  ESI, EAX
LL1: 
     PUSH DWORD PTR [EBP+ESI-4]
     SUB  ESI, 4
     JNZ  SHORT LL1
     ADD  EBP, EAX
     MOV  EAX, [EBP]
     LEA  EBP, [EBP+4]
     JMP  EDX
EndCode

Code NR> ;( R: x1 ... xn n -- D: x1 ... xn n )
; Перенести n чисел со стека возвратов на стек данных
; Если n=0 возвратить 0
     POP  EDX ; адрес возврата
     LEA  EBP, [EBP-4]
     MOV  [EBP], EAX
     POP  EAX
     OR   EAX, EAX
     JNZ  @@2
     JMP  EDX

@@2: LEA  EAX, [EAX*4]
     MOV  ESI, EAX
@@1: 
     MOV  EBX, EBP
     SUB  EBX, ESI
     POP  DWORD PTR [EBX]
     SUB  ESI,  4
     JNZ  SHORT @@1
     SUB  EBP, EAX
     SAR  EAX,  2
     JMP  EDX
EndCode

Code N>R ;( D: x1 ... xn n -- R: x1 ... xn n )
; перенести n чисел со стека данных на стек возвратов
     LEA  EBP, [EBP-4]
     MOV  [EBP], EAX
     LEA EAX, [EAX*4+4]

     POP  EDX ; адрес возврата
     MOV  ESI, EAX
@@1: 
     PUSH DWORD PTR [EBP+ESI-4]
     SUB  ESI,  4
     JNZ  SHORT @@1
     ADD  EBP, EAX
     MOV  EAX, [EBP]
     LEA  EBP, [EBP+4]
     JMP  EDX
EndCode

Code NRCOPY ;( D: i*x i -- D: i*x i R: i*x i )
; скопировать n чисел со стека данных на стек возвратов
     MOV  ECX, EAX
     LEA  ECX, [ECX*4]

     POP  EDX ; адрес возврата
     JECXZ @@2
     MOV  ESI, ECX
@@1: 
     PUSH DWORD PTR [ESI+EBP-4]
     SUB  ESI,  4
     JNZ  SHORT @@1
@@2:
     PUSH EAX
     JMP  EDX
EndCode

Code RP+@ ;( offs -- x )
; взять число со смещением offs байт от вершины стека возвратов (0 RP+@ == RP@)
     MOV EAX, [EAX+ESP+4]
     RET
EndCode
     
Code RP+ ;( offs -- addr )
; взять адрес со смещением offs байт от вершины стека возвратов
	LEA EAX, [EAX+ESP+4]
	RET
EndCode

Code RP+! ;( x offs -- )
; записать число x по смещению offs байт от вершины стека возвратов
	MOV  EBX, [EBP]
	MOV  [EAX+ESP+4], EBX
	LEA  EBP, [EBP+8]
	MOV  EAX, [EBP-4]
	RET
EndCode

Code RALLOT ;( n -- addr )
; зарезервировать n ячеек на стеке возвратов,
; сделаем с инициализацией (а то если больше 8К выделим, exception может)
     POP  EDX
     MOV  ECX, EAX
     XOR  EAX, EAX
@@1: PUSH EAX
     DEC  ECX
     JNZ  SHORT @@1
     MOV  EAX, ESP
     JMP  EDX
EndCode

Code (RALLOT) ;( n -- )
; зарезервировать n ячеек на стеке возвратов
     POP  EDX
     MOV  ECX, EAX
     XOR  EAX, EAX
@@1: PUSH EAX
     DEC  ECX
     JNZ  SHORT @@1
     MOV  EAX, [EBP]
     LEA  EBP, [EBP+4]
     JMP  EDX
EndCode

Code RFREE ;( n -- )
; вернуть n ячеек стека возвратов
     POP  EDX
     LEA  ESP, [ESP+EAX*4]
     MOV EAX, [EBP]
     LEA EBP, [EBP+4]
     JMP  EDX
EndCode

Code (LocalsExit) ;( -- )
; вернуть память в стек вовратов, число байт лежит на стеке
     POP  EBX
     ADD  ESP, EBX
     RET
EndCode

Code TIMER@ ;( -- tlo thi ) \ Только для Intel Pentium и выше!!!
; Возвратить значение таймера процессора как ud
   MOV [EBP-4], EAX
   RDTSC
   MOV [EBP-8], EDX
   LEA EBP,  [EBP-8]
   XCHG EAX, [EBP]
   RET
EndCode

\ Для остальных процессоров раскомментируйте:
\ : TIMER@ 0 GetTickCount ;

Code TRAP-CODE ;( D: j*x u R: i*x i -- i*x u )
; Вспомогательное слово для восстановления значений, сохраненных
; перед CATCH на стеке возвратов
     POP  EDX
     POP  ESI
     OR   ESI, ESI
     JZ   @@2
     LEA  ESI, [ESI*4]
     MOV  ECX, ESI
@@1: MOV  EBX, [ESI+ESP-4]
     MOV  [ESI+EBP-4], EBX
     SUB  ESI, 4
     JNZ  SHORT @@1
     ADD  ESP, ECX
@@2: JMP  EDX
EndCode

DECIMAL

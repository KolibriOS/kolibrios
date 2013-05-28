;
; Данные для parser.inc
;
;





;
; имя файла, который нужно парсить
;
fname db '/sys/ki.cfg',0

;
;  таблицы значений 
;
;  +----+-------------+-----------------+
;  | in | Variable    | Variable        |
;  | de | name        | string          |
;  | x  |             |                 |
;  |    |             |                 |
;  +----+-------------+-----------------+
;  |    |             |                 |
;  |    |             |                 |
;  | 1  | UIN         | 'XXXXX..XX'     |
;  |    |             |                 |
;  +----+-------------+-----------------+
;  |                  |                 |
;
;  загрузка int пока не реализована
;
;  +----+-------------+-----------------+
;  | in | Variable    | Variable        |
;  | de | name        | int             |
;  | x  |             |                 |
;  |    |             |                 |
;  +----+-------------+-----------------+
;  |    |             |                 |
;  |    |             |                 |
;  | 1  |    BUFFSIZE |  XXXXXXXX       |
;  |    |             |                 |
;  +----+-------------+-----------------+
;  |                  |                 |
;
;



;nvtable db (TABLE_SIZE * NAME_LEN)   dup 0
vtable  db (TABLE_SIZE * VAR_LEN)    dup 0

finfo sinfo

;
; Заполнить таблицу имен переменных
;
nvtable db 'UIN',(VNAME_LEN - 3) dup 0
	db 'PASS',(VNAME_LEN - 4) dup 0
	db 'ICQIP',(VNAME_LEN - 5) dup 0
        db ((TABLE_SIZE - 3) * VNAME_LEN) dup 0

;
; для инициализации таблиц
;
;
;
;name1 db 'UIN',(VNAME_LEN - 3) dup 0
;name2 db 'PASS',(VNAME_LEN - 4) dup 0
;name3 db 'ICQIP',(VNAME_LEN - 5) dup 0


;
; Буфер для ввода/вывода
;
iobuff db IOBUFF_SIZE dup 0
;
; Смещение в файле
;
shift  dd 0

;
; Переменные для хранения номеров 
; символов начала имени переменной
; конца, равно, конца значения
stnpos dd 0
ednpos dd 0
eqpos  dd 0
edvpos dd 0
;
;Номер строки для определения ошибочных
;
strnum dd 0

;
; Буфер для вывода загруженного конфига
;
cfgbuff db (VAR_LEN + VNAME_LEN + 8) dup 0
cfgbuff.len = $ - cfgbuff



include 'lang.inc'

if lang eq ru

text		db ' ВхФайл:','ВыхФайл:','   Путь:'
	.line_size = ($-text)/3

s_compile	db 'Компил.'
s_run		db ' Пуск  '
s_debug 	db 'Отладка'
s_dbgdescr	db 'Создавать отладочную информацию',0

sz_EPnotFound	db 'Not found entry point to ',0
sizeof.sz_EPnotFound = $-sz_EPnotFound

sz_cantLL	db 'Can not load library ',0
sizeof.sz_cantLL = $-sz_cantLL

sz_system_error db 'System error: ',0
sizeof.sz_system_error = $-sz_system_error

else

text	db    ' InFile:','OutFile:','   Path:'
	.line_size = ($-text)/3

s_compile	db    'COMPILE'
s_run		db    '  RUN  '
s_debug 	db    ' DEBUG '
s_dbgdescr	db    'Generate debug information',0

sz_EPnotFound	db 'Not found entry point to ',0
sizeof.sz_EPnotFound = $-sz_EPnotFound

sz_cantLL	db 'Can not load library ',0
sizeof.sz_cantLL = $-sz_cantLL

sz_system_error db 'System error: ',0
sizeof.sz_system_error = $-sz_system_error

end if


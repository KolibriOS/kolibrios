;
; Ghost Monitor - утилита (стресс) тестирования и мониторинга системы
; Copyright (C) 2005, 2006, 2007 Mihailov Ilia (ghost.nsk@mail.ru)
; All Right Reserved

; Language support for locales: it_IT, en_US.

	mov	edx, msg_about
	mov	ebx, 17 * 65536 + 285
	call	show_text
	jmp	redraw

if lang eq it_IT
	msg_about mls \
		'               Ghost Monitor',\
		'',\
		'strumento per testare il proprio hardware',\
		'',\
		'Supported SuperIO : W83627HF,W83697HF',\
		'  W83627THF,W83627THF-A,W83627EHF-A,',\
		'  W83637THF,IT8705F,IT8712F,SiS950',\
		'  ABIT uGuru',\
		'',\
		'Test basati su R.J.Redelmeier CPUBurn',\
		'',\
		'Mailto : ghost.nsk@mail.ru',\
		'',\
		'      *** Usare a prioprio rischio ***'
else ; Default to en_US
	msg_about mls \	; должна быть хотябы одна строка
		'               Ghost Monitor',\
		'',\
		' tool for testing and monitoring hardware',\
		'',\
		'Supported SuperIO : W83627HF,W83697HF',\
		'  W83627THF,W83627THF-A,W83627EHF-A,',\
		'  W83637THF,IT8705F,IT8712F,SiS950',\
		'  ABIT uGuru',\
		'',\
		'Tests based on R.J.Redelmeier CPUBurn',\
		'',\
		'Mailto : ghost.nsk@mail.ru',\
		'',\
		'       *** Use at your own risk ***'
end if

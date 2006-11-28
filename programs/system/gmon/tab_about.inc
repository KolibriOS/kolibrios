
	mov	edx, msg_about
	mov	ebx, 17 * 65536 + 285
	call	show_text
	jmp	end_drow_tab

msg_about mls \	; должна быть хот€бы одна строка
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
	'       *** Use at YOUR own RISK ***'

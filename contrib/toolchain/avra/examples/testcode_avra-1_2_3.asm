	.device ATmega8
	.org	0  	.db	1,2
	.message "The previous line is ignored with avra-1.2.2 because .org 0 is terminated with CR only. "
	.message "This is line 5 but avra-1.2.2 shows line 4"
	.db 	"X%MINUTE%YEAR%"	; Take a look at this percent chars too : % % % %
	.db 	"%YEAR%HELLO%"		; Strange replacement, if one percent char is missing
	.db	"%HOUR%:%MINUTE%%"
	.db	"øC"			; Look at the special char. (Error in listing only. HEX-file was ok)
		; Additional warning : Don't use linux editors with UTF charset ! A single special char
		; (Code > 127 in codepage 850 e.g. german umlauts) could be an unvisible TWO bytes sequence
		; in UTF coding. To be on the save side never use chars with code > 127. 
		; It's better to replace them by the code e.g.  .db "M",129,"nchen" (german town 'Munich')
	ldi R16, ';'			; This is wrong with avra-1.2.2.
	ldi R16, 0x3b			; Should generate same code like above

; TODO :
;	ldi	R16,high (11111)	; "high(" is OK, "high (" isn't. Same with other functions...


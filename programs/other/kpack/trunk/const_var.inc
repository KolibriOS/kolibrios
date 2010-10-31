;---------------------------------------------------------------------
aQuestion	db '?'
caption_str	db 'KPack',0
buttons1names	db ' InFile:'
		db 'OutFile:'
		db '   Path:'
aCompress	db 'COMPRESS',0
aDecompress	db 'DECOMPRESS',0
definoutname	db 0
defpath		db '/RD/1/'
curedit		dd 0

info_str	db 'KPack - Kolibri Packer, version 0.13',10
		db 'Uses LZMA v4.32 compression library',10,10
info_len	= $ - info_str
usage_str	db 'Written by diamond in 2006, 2007, 2009 specially for KolibriOS',10
		db 'LZMA  is copyright (c) 1999-2005 by Igor Pavlov',10
		db 10
		db 'Command-line usage:',10
		db ' kpack infile [outfile]',10
		db 'If no output file is specified,',10
		db '    packed data will be written back to input file',10
		db 10
		db 'Window usage:',10
		db " enter input file name, output file name and press needed button",10
usage_len	= $ - usage_str
errload_str	db 'Cannot load input file',10
errload_len	= $ - errload_str
outfileerr_str	db 'Cannot save output file',10
outfileerr_len	= $ - outfileerr_str
nomem_str	db 'No memory',10
nomem_len	= $ - nomem_str
too_big_str	db 'failed, output is greater than input.',10
too_big_len	= $ - too_big_str
compressing_str	db 'Compressing ... '
compressing_len = $ - compressing_str
lzma_memsmall_str db	'Warning: not enough memory for default LZMA settings,',10
		db '         will use less dictionary size',10
lzma_memsmall_len = $ - lzma_memsmall_str
notpacked_str	db 'Input file is not packed with KPack!',10
notpacked_len	= $ - notpacked_str
unpacked_ok	db 'Unpacked successful',10
unpacked_len	= $ - unpacked_ok

done_str	db 'OK! Compression ratio: '
ratio		dw	'00'
		db '%',10
done_len	= $ - done_str
;---------------------------------------------------------------------
align 4
LiteralNextStates:
db 0,0,0,0,1,2,3,4,5,6,4,5
MatchNextStates:
db 7,7,7,7,7,7,7,10,10,10,10,10
RepNextStates:
db 8,8,8,8,8,8,8,11,11,11,11,11
ShortRepNextStates:
db 9,9,9,9,9,9,9,11,11,11,11,11
;---------------------------------------------------------------------
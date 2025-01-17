format MS COFF

StackSize = 16384

; must be alphabetically first in the image
section '.1seg' data readable writable
extrn _crtStartUp	; real entry point
extrn _kosCmdLine
extrn _kosExePath
extrn _exeStack
public fakeEntry

kos_header:
	db	'MENUET01'	; header
	dd	1		; headerver
	dd	_crtStartUp	; entry
	dd	0		; i_end, filled by doexe2.asm
	dd	0		; memsize, filled by doexe2.asm
	dd	_exeStack + StackSize	; stack
	dd	_kosCmdLine	; params
	dd	_kosExePath	; icon
fakeEntry:	; only for linker, to force including this obj file
		; real entry is crtStartUp

; initializers
section '.CRT$XCA' data readable writable
public ___xc_a
___xc_a:
section '.CRT$XCZ' data readable writable
public ___xc_z
___xc_z:

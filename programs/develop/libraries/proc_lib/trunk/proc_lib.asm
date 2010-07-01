;*****************************************************************************
; Proc_Lib - library for common procedures
;
; Authors:
; Marat Zakiyanov aka Mario79, aka Mario
;*****************************************************************************

format MS COFF

public EXPORTS

section '.flat' code readable align 16
include '../../../../macros.inc'
;include '../../../../proc32.inc'
include 'opendial.mac'

;-----------------------------------------------------------------------------

;--------------------------------------------------
; Open Dialog
;--------------------------------------------------
align 16
use_OpenDialog

;--------------------------------------------------
align 16
lib_init:
ret

;--------------------------------------------------
align 16
EXPORTS:


dd	sz_init,			lib_init
dd	sz_version,			0x00000001

dd	sz_OpenDialog_init,		OpenDialog.init
dd	sz_OpenDialog_start,		OpenDialog.start
dd	szVersion_OpenDialog,		0x00010001

dd	0,0


sz_init 			db 'lib_init',0
sz_version			db 'version',0

sz_OpenDialog_init 		db 'OpenDialog_init',0
sz_OpenDialog_start		db 'OpenDialog_start',0
szVersion_OpenDialog	db 'Version_OpenDialog',0

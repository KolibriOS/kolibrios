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
include 'colrdial.mac'
;include 'debug.inc'
;-----------------------------------------------------------------------------

;--------------------------------------------------
; Open Dialog
;--------------------------------------------------
align 16
use_OpenDialog

;--------------------------------------------------
; Color Dialog
;--------------------------------------------------
align 16
use_ColorDialog

;--------------------------------------------------
align 16
lib_init:
ret

;--------------------------------------------------
align 16
EXPORTS:


dd	sz_init,			lib_init
dd	sz_version,			0x00000001

dd	sz_OpenDialog_init,	OpenDialog.init
dd	sz_OpenDialog_start,	OpenDialog.start
dd	szVersion_OpenDialog,	0x00010001

dd	sz_ColorDialog_init,	ColorDialog.init
dd	sz_ColorDialog_start,	ColorDialog.start
dd	szVersion_ColorDialog,	0x00010001

dd	0,0
;-----------------------------------------------------------------------------
sz_init 			db 'lib_init',0
sz_version			db 'version',0

sz_OpenDialog_init 		db 'OpenDialog_init',0
sz_OpenDialog_start		db 'OpenDialog_start',0
szVersion_OpenDialog		db 'Version_OpenDialog',0

sz_ColorDialog_init 		db 'ColorDialog_init',0
sz_ColorDialog_start		db 'ColorDialog_start',0
szVersion_ColorDialog		db 'Version_ColorDialog',0
;-----------------------------------------------------------------------------

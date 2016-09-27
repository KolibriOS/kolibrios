format coff
use32                                   ; Tell compiler to use 32 bit instructions

section '.init' code			; Keep this line before includes or GCC messes up call addresses

include '../../../programs/proc32.inc'
include '../../../programs/macros.inc'
purge section,mov,add,sub
	
include '../../../programs/dll.inc'
	
public init_kmenu as '_kolibri_kmenu_init'
;;; Returns 0 on success. -1 on failure.

proc init_kmenu
	pusha
	mcall 68,11
	stdcall dll.Load, @IMPORT
	popa
	ret
endp	
	
@IMPORT:
library lib_kmenu, 	'kmenu.obj'

import lib_kmenu, \
	kmainmenu_draw, 				 'kmainmenu_draw' , \
	kmainmenu_dispatch_cursorevent,  'kmainmenu_dispatch_cursorevent' , \
	kmainmenu_get_height, 			 'kmainmenu_get_height', \
	ksubmenu_new, 					 'ksubmenu_new' , \
	ksubmenu_delete,  				 'ksubmenu_delete' , \
	ksubmenu_draw, 					 'ksubmenu_draw' , \
	ksubmenu_add, 					 'ksubmenu_add' , \
	ksubmenu_set_items_margin,  	 'ksubmenu_set_items_margin' , \
	ksubmenu_set_items_padding, 	 'ksubmenu_set_items_padding' , \
	kmenuitem_new, 					 'kmenuitem_new' , \
	kmenuitem_delete, 				 'kmenuitem_delete' , \
	kmenuitem_draw, 				 'kmenuitem_draw' , \
	kmenuitem_get_preffered_width, 	 'kmenuitem_get_preffered_width' , \
	kmenuitem_get_preffered_height,  'kmenuitem_get_preffered_height' , \
	kmenu_set_font,					 'kmenu_set_font' , \
	kmenu_init, 					 'kmenu_init'
	
public	kmainmenu_draw	 				 as '_kmainmenu_draw'
public	kmainmenu_dispatch_cursorevent   as '_kmainmenu_dispatch_cursorevent'
public	kmainmenu_get_height 			 as '_kmainmenu_get_height'
public	ksubmenu_new 					 as '_ksubmenu_new'
public	ksubmenu_delete  				 as '_ksubmenu_delete'
public	ksubmenu_draw 					 as '_ksubmenu_draw'
public	ksubmenu_add 					 as '_ksubmenu_add'
public	ksubmenu_set_items_margin 	 	 as '_ksubmenu_set_items_margin'
public	ksubmenu_set_items_padding  	 as '_ksubmenu_set_items_padding'
public	kmenuitem_new					 as '_kmenuitem_new'
public	kmenuitem_new					 as '_kmenuitem__submenu_new'
public	kmenuitem_delete 				 as '_kmenuitem_delete'
public	kmenuitem_draw  				 as '_kmenuitem_draw'
public	kmenuitem_get_preffered_width 	 as '_kmenuitem_get_preffered_width'
public	kmenuitem_get_preffered_height   as '_kmenuitem_get_preffered_height'
public	kmenu_set_font					 as '_kmenu_set_font'
public	kmenu_init 						 as '_kmenu_init'

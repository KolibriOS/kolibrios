;fasm version file 'kolibri_menu.h'


KMENUITEM_NORMAL equ 0
KMENUITEM_SUBMENU equ 1
KMENUITEM_SEPARATOR equ 2

KMENUITEM_MAINMENU equ 0x80000000

KMENUITEM_SEPARATOR_WIDTH equ 10 ;170
KMENUITEM_SEPARATOR_HEIGHT equ 2

KMENU_LBORDER_SIZE equ 2
KMENU_DBORDER_SIZE equ 1

struct t_font
	type  dd ? ;int
	color dd ? ;uint32_t
	union
		bgcolor dd ? ;uint32_t
		buffer  dd ? ;void*
	ends
ends

struct t_rect
	left   dw ? ;uint16_t
	top    dw ? ;uint16_t
	right  dw ? ;uint16_t
	bottom dw ? ;uint16_t
ends

struct kmenuitem_t
	type dd ? ;int
	text dd ? ;char*
	union
		submenu dd ? ;void*
		btnid   dd ? ;size_t
	ends
	paint       dd ? ;void *
	is_focused  dd ? ;int
	is_enabled  dd ? ;int
	is_visible  dd ? ;int
	style       dd ? ;size_t
	font_width  dd ? ;int
	font_height dd ? ;int
	font        t_font
	left           dd ? ;size_t
	top            dd ? ;size_t
	width          dd ? ;size_t
	height         dd ? ;size_t
	pref_width     dd ? ;size_t
	pref_height    dd ? ;size_t
	margin_left    dd ? ;size_t
	margin_right   dd ? ;size_t
	margin_top     dd ? ;size_t
	margin_bottom  dd ? ;size_t
	padding_left   dd ? ;size_t
	padding_right  dd ? ;size_t
	padding_top    dd ? ;size_t
	padding_bottom dd ? ;size_t
ends

struct ksubmenu_t
	items      dd ? ;kmenuitem_t**
	count      dd ? ;int
	paint      dd ? ;void *
	is_opened  dd ? ;int
	focus_idx  dd ? ;int
	width      dd ? ;size_t
	height     dd ? ;size_t
	left       dd ? ;size_t
	top        dd ? ;size_t
	tid        dd ? ;int
	parent_wnd dd ? ;int
	parent_tid dd ? ;int
	submenu_tid dd ? ;int
	self_wnd     dd ? ;int
	level         dd ? ;int
	return_btnid   dd ? ;size_t
	set_new_padding dd ? ;int
	set_new_margin  dd ? ;int
	items_padding   t_rect
	items_margin    t_rect
	parent          dd ? ;ksubmenu_t*
ends

struct kmenu_t
	submenu dd ? ;kmenuitem_t**
	count   dd ? ;int
	width   dd ? ;size_t
	height   dd ? ;size_t
	left      dd ? ;size_t
	top        dd ? ;size_t
	line_height dd ? ;int
	focus_idx   dd ? ;int
	submenu_tid dd ? ;int
ends
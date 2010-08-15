
  use32
  org	 0x0

  db	 'MENUET01'
  dd	 0x01
  dd	 START
  dd	 I_END
  dd	 0x1000
  dd	 0x1000
  dd	 0x0
  dd	 0x0

FALSE = 0
TRUE  = 1

include '../../../../../proc32.inc'
include '../../../../../macros.inc'
include '../../libio/libio.inc'
include '../dll.inc'

yy dd 20

proc draw xx,color,key,val
	pushad

	mov	esi,[key]
	mov	edi,buf
	cld
	mov	al,"'"
	stosb
    @@: lodsb
	stosb
	or	al,al
	jnz	@b
	mov	word[edi-1],"'"

	mov	esi,[val]
	or	esi,esi
	je	.noval
	mov	eax," = '"
	stosd
    @@: lodsb
	stosb
	or	al,al
	jnz	@b
	mov	word[edi-1],"'"
  .noval:

	or	[color],0x80000000
	mcall	4,<[xx],[yy]>,[color],buf
	add	[yy],10
	popad
	ret
endp

proc callb_k f_name,sec_name,key_name,key_val
	stdcall draw,224,0x0000FF,[key_name],[key_val]
	mov	eax,TRUE
	ret
endp

proc callb f_name,sec_name
	stdcall draw,200,0xFF0000,[sec_name],0
	invoke	ini.enum_keys,[f_name],[sec_name],callb_k
	mov	eax,TRUE
	ret
endp

;---------------------------------------------------------------------
;---  Õ¿◊¿ÀŒ œ–Œ√–¿ÃÃ€  ----------------------------------------------
;---------------------------------------------------------------------

s_key1 db "LeftViewMode",0
s_key2 db "RightViewMode",0

macro wildcard_test_data label1, label2, label3, [str1, str2, res]
{
  common
    label label1
  forward
    local ..lbl
    dd ..lbl - $
    db str1,0
    ..lbl:
  common
    dd 0
    label label2
  forward
    local ..lbl
    dd ..lbl - $
    db str2,0
    ..lbl:
  common
    label label3
  forward
    if res = "t"
      dd 1
    else
      dd 0
    end if
}

wildcard_test_data _str1, _str2, _str3, \
  ""	       ,""	 ,"t",	  ""		,"a"	    ,"f", \
  "a"	       ,"a"	 ,"t",	  "a"		,"b"	    ,"f", \
  "?"	       ,"b"	 ,"t",	  "??"		,"bc"	    ,"t", \
  "?c"	       ,"bc"	 ,"t",	  "b?"		,"bc"	    ,"t", \
  "[a-z]"      ,"b"	 ,"t",	  "[A-Z]"	,"b"	    ,"f", \
  "*"	       ,"a"	 ,"t",	  "**"		,"a"	    ,"t", \
  "*"	       ,""	 ,"t",	  "*bc*hij"	,"abcdfghij","t", \
  "*b*a*"      ,"b"	 ,"f",	  "*bc*hik"	,"abcdfghij","f", \
  "abc*"       ,"abc"	 ,"t",	  "abc**"	,"abc"	    ,"t", \
  "[^]]"       ,"^"	 ,"t",	  "[^]]"	,"]"	    ,"f", \
  "[^abc]"     ,"d"	 ,"t",	  "[^abc]"	,"b"	    ,"f", \
  "*???"       ,"abc"	 ,"t",	  "*???"	,"ab"	    ,"f", \
  "*???"       ,"abcd"	 ,"t",	  "*?*" 	,"abcd"     ,"t", \
  "*bc"        ,"abc"	 ,"t",	  "*cc" 	,"abc"	    ,"f", \
  "[a-c]*"     ,"d"	 ,"f",	  "*[a-e]"	,"d"	    ,"t", \
  "*a*"        ,"de"	 ,"f",	  "*[a-c]"	,"d"	    ,"f", \
  "[a-c]"      ,"d"	 ,"f",	  "[b-d]"	,"a"	    ,"f", \
  "[]abc]"     ,"b"	 ,"t",	  "[]abc]"	,"d"	    ,"f", \
  "[z-a]"      ,"-"	 ,"t",	  "[z-a]"	,"b"	    ,"f", \
  "[A-]"       ,"-"	 ,"t",	  "[A-]"	,"]"	    ,"f", \
  "[-a]"       ,"-"	 ,"t",	  "[-[]"	,"-"	    ,"t", \
  "[-]"        ,"-"	 ,"t",	  "[^-b]"	,"a"	    ,"t", \
  "[^-b]"      ,"-"	 ,"f",	  "[-b]"	,"a"	    ,"f", \
  "[a-g]lorian","florian","t",	  "[a-g]*rorian","f"	    ,"f", \
  "*???*"      ,"123"	 ,"t"

START:
	mcall	68,11

	stdcall dll.Load,@IMPORT
	or	eax,eax
	jnz	exit

@^ ; commenting out, file.aux.match_wildcard is no longer exported
	mov	esi,_str1 + 4
	mov	edi,_str2 + 4
	mov	ecx,_str3
	xor	ebx,ebx
	inc	ebx
    @@: invoke	file.aux.match_wildcard,edi,esi,0
	cmp	eax,[ecx]
	je	.ok
	mcall	-1
  .ok:	inc	ebx
	add	esi,[esi-4]
	add	edi,[edi-4]
	add	ecx,4
	cmp	dword[esi-4],0
	jnz	@b
^@

	invoke	ini.set_int,s_ini,s_sec,s_key1,100
	invoke	ini.set_int,s_ini,s_sec,s_key2,101

	invoke	ini.get_int,s_ini,s_sec,s_key1,-1
	cmp	eax,100
	jne	exit
	invoke	ini.get_int,s_ini,s_sec,s_key2,-1
	cmp	eax,101
	jne	exit

	mov	dword[buf],'102'
	invoke	ini.set_str,s_ini,s_sec,s_key1,buf,3
	mov	dword[buf],'103'
	invoke	ini.set_str,s_ini,s_sec,s_key2,buf,3

	invoke	ini.get_str,s_ini,s_sec,s_key1,buf,1024,0
	cmp	dword[buf],'102'
	jne	exit
	invoke	ini.get_str,s_ini,s_sec,s_key2,buf,1024,0
	cmp	dword[buf],'103'
	jne	exit

	invoke	ini.enum_sections,s_ini,callb

;       invoke  file.open,s_ininame,O_WRITE+O_CREATE
;       or      eax,eax
;       jnz     @f
;       int3
;   @@: mov     [fh],eax
;       invoke  file.seek,[fh],SEEK_SET,8192
;       invoke  file.write,[fh],s_ininame,16
;       invoke  file.seteof,[fh]
;       invoke  file.close,[fh]

red:

	call	draw_window

;---------------------------------------------------------------------
;---  ÷» À Œ¡–¿¡Œ“ » —Œ¡€“»…  ----------------------------------------
;---------------------------------------------------------------------

still:
	mcall	10

	cmp	eax,1
	je	red
	cmp	eax,2
	je	key
	cmp	eax,3
	je	button

	jmp	still

;---------------------------------------------------------------------

  key:
	mcall	2
	jmp	still

;---------------------------------------------------------------------

  button:
	mcall	17

	cmp	ah,1
	jne	still

  exit:
	mcall	-1

;---------------------------------------------------------------------
;---  Œœ–≈ƒ≈À≈Õ»≈ » Œ“–»—Œ¬ ¿ Œ Õ¿  ----------------------------------
;---------------------------------------------------------------------

ctx dd ?

draw_window:
	invoke	gfx.open,TRUE
	mov	[ctx],eax
	mcall	0,<200,700>,<200,200>,0x33FFFFFF,,s_header
	invoke	gfx.pen.color,[ctx],0x00FF0000
	invoke	gfx.line,[ctx],0,0,50,50
	invoke	gfx.framerect,[ctx],10,10,100,70
	invoke	gfx.brush.color,[ctx],0x000000FF
	invoke	gfx.fillrect,[ctx],15,15,95,65
	invoke	gfx.pen.color,[ctx],0x00008800
	invoke	gfx.brush.color,[ctx],0x00CCCCFF
	invoke	gfx.rectangle,[ctx],20,20,90,60
	invoke	gfx.move.to,[ctx],13,5
	invoke	gfx.line.to,[ctx],105,5
	invoke	gfx.line.to,[ctx],105,75
	invoke	gfx.line.to,[ctx],5,75
	invoke	gfx.line.to,[ctx],5,13
	invoke	gfx.line.to,[ctx],13,5
	invoke	gfx.pen.color,[ctx],0x00888888
	invoke	gfx.polyline,[ctx],poly_points,11
	invoke	gfx.close,[ctx]

	mov	[yy],10
	invoke	ini.enum_sections,s_ini,callb

	invoke	file.find_first,_f_path,_f_mask,FA_ANY-FA_FOLDER
	cmp	eax,0
	jle	.finished
	mov	ebp,eax
	mov	[yy],10
    @@: lea	edx,[ebp+FileInfo.FileName]
	mcall	4,<450,[yy]>,0x80000000
	add	[yy],10
	invoke	file.find_next,ebp
	cmp	eax,0
	jg	@b
	invoke	file.find_close,ebp

  .finished:

	ret

_f_path db '/rd/1/lib',0
_f_mask db '*ini*',0

;-----------------------------------------------------------------------------
proc mem.Alloc size ;/////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	push	ebx ecx
	mov	eax,[size]
	lea	ecx,[eax+4+4095]
	and	ecx,not 4095
	mcall	68,12
	add	ecx,-4
	mov	[eax],ecx
	add	eax,4
	pop	ecx ebx
	ret
endp

;-----------------------------------------------------------------------------
proc mem.ReAlloc mptr,size;///////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	push	ebx ecx esi edi eax
	mov	eax,[mptr]
	mov	ebx,[size]
	or	eax,eax
	jz	@f
	lea	ecx,[ebx+4+4095]
	and	ecx,not 4095
	add	ecx,-4
	cmp	ecx,[eax-4]
	je	.exit
    @@: mov	eax,ebx
	call	mem.Alloc
	xchg	eax,[esp]
	or	eax,eax
	jz	.exit
	mov	esi,eax
	xchg	eax,[esp]
	mov	edi,eax
	mov	ecx,[esi-4]
	cmp	ecx,[edi-4]
	jbe	@f
	mov	ecx,[edi-4]
    @@: add	ecx,3
	shr	ecx,2
	cld
	rep	movsd
	xchg	eax,[esp]
	call	mem.Free
  .exit:
	pop	eax edi esi ecx ebx
	ret
endp

;-----------------------------------------------------------------------------
proc mem.Free mptr ;//////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	mov	eax,[mptr]
	or	eax,eax
	jz	@f
	push	ebx ecx
	lea	ecx,[eax-4]
	mcall	68,13
	pop	ecx ebx
    @@: ret
endp

;---------------------------------------------------------------------
;---  ƒ¿ÕÕ€≈ œ–Œ√–¿ÃÃ€  ----------------------------------------------
;---------------------------------------------------------------------

s_header db 'EXAMPLE APPLICATION',0

s_ini db '/rd/1/test001.ini',0
s_sec db 'Panels',0
s_key db 'param1',0

align 4
poly_points dd \
  140, 10, \
  150, 10, \
  150, 20, \
  160, 20, \
  160, 30, \
  170, 30, \
  170, 40, \
  180, 40, \
  180, 50, \
  140, 50, \
  140, 10

;---------------------------------------------------------------------

align 16
@IMPORT:

library \
	libini,'libini.obj',\
	libio ,'libio.obj',\
	libgfx,'libgfx.obj'

import	libini, \
	ini.get_str,'ini_get_str',\
	ini.set_str,'ini_set_str',\
	ini.get_int,'ini_get_int',\
	ini.set_int,'ini_set_int',\
	ini.enum_sections,'ini_enum_sections',\
	ini.enum_keys,'ini_enum_keys'

import	libio, \
\;      file.aux.match_wildcard,'file_aux_match_wildcard',\
	file.find_first,'file_find_first',\
	file.find_next,'file_find_next',\
	file.find_close,'file_find_close',\
	file.open,'file_open',\
	file.seek,'file_seek',\
	file.write,'file_write',\
	file.truncate,'file_truncate',\
	file.close,'file_close'

import	libgfx, \
	gfx.open	,'gfx_open',\
	gfx.close	,'gfx_close',\
	gfx.pen.color	,'gfx_pen_color',\
	gfx.brush.color ,'gfx_brush_color',\
	gfx.pixel	,'gfx_pixel',\
	gfx.move.to	,'gfx_move_to',\
	gfx.line.to	,'gfx_line_to',\
	gfx.line	,'gfx_line',\
	gfx.polyline	,'gfx_polyline',\
	gfx.polyline.to ,'gfx_polyline_to',\
	gfx.fillrect	,'gfx_fillrect',\
	gfx.fillrect.ex ,'gfx_fillrect_ex',\
	gfx.framerect	,'gfx_framerect',\
	gfx.framerect.ex,'gfx_framerect_ex',\
	gfx.rectangle	,'gfx_rectangle',\
	gfx.rectangle.ex,'gfx_rectangle_ex'

I_END:

fh dd ?

buf rb 1024

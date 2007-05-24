
__REV = 0

macro $Revision a {
  match =: Num =$,a \{
    if __REV < Num
      __REV = Num
    end if
  \}
}

$Revision$

; structure definition helper
macro struct name, [arg]
 {
  common
   name@struct equ name
   struc name arg {
 }

macro struct_helper name
 {
  match xname,name
  \{
   virtual at 0
   xname xname
   sizeof.#xname = $ - xname
   name equ sizeof.#xname
   end virtual
  \}
 }

ends fix } struct_helper name@struct

;// mike.dld, 2006-29-01 [

; macros definition
macro diff16 title,l1,l2
{
  local s,d
  s = l2-l1
  display title,': 0x'
  repeat 16
    d = 48 + s shr ((16-%) shl 2) and $0F
    if d > 57
      d = d + 65-57-1
    end if
    display d
  end repeat
  display 13,10
}
macro diff10 title,l1,l2
 {
  local s,d,z,m
  s = l2-l1
  z = 0
  m = 1000000000
  display title,': '
  repeat 10
   d = '0' + s / m
   s = s - (s/m)*m
   m = m / 10
   if d <> '0'
    z = 1
   end if
   if z <> 0
    display d
   end if
  end repeat
  display 13,10
 }

include 'kglobals.inc'

; \begin{diamond}[29.09.2006]
; may be useful for kernel debugging
; example 1:
;       dbgstr 'Hello, World!'
; example 2:
;       dbgstr 'Hello, World!', save_flags
macro dbgstr string*, f
{
local a
iglobal_nested
a db 'K : ',string,13,10,0
endg_nested
if ~ f eq
	pushfd
end if
	push	esi
	mov	esi, a
	call	sys_msg_board_str
	pop	esi
if ~ f eq
	popfd
end if
}
; \end{diamond}[29.09.2006]

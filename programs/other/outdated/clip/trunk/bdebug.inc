; debugging include file by barsuk
; if DEBUG=TRUE, adds debug.inc
; if DEBUG=FALSE, adds stubs for several macros from debug.inc

match =TRUE, DEBUG
{

include '..\..\..\..\debug.inc'

macro dph1 str
\{
  debug_print_hex str
  push eax
  mov al, ' '
  call debug_outchar
  pop eax
\}

macro outs str
\{
	pushad
	lea	edx, [str]
	call	debug_outstr
	popad
\}
}

match =FALSE, DEBUG
{

macro print message
\{
\}

macro pregs
\{
\}

macro dph1 str
\{
\}

macro outs str
\{
\}

}
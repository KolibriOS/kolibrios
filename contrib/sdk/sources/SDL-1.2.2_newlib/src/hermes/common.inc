; Some common macros for hermes nasm code

%macro SDL_FUNC 1
%ifdef HIDDEN_VISIBILITY
GLOBAL %1:function hidden
%else
GLOBAL %1
%endif
%endmacro

%ifdef __OS2__
; declare segments with proper attributes for OS/2 386 builds:
SEGMENT .data CLASS=DATA ALIGN=16 USE32 FLAT
SEGMENT .text CLASS=CODE ALIGN=16 USE32 FLAT
%endif

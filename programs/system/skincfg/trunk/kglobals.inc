;------------------------------------------------------------------
; use "iglobal" for inserting initialized global data definitions.
;------------------------------------------------------------------
macro iglobal {
  IGlobals equ IGlobals,
  macro __IGlobalBlock { }

macro iglobal_nested {
  IGlobals equ IGlobals,
  macro __IGlobalBlock \{ }

;-------------------------------------------------------------
; use 'uglobal' for inserting uninitialized global definitions.
; even when you define some data values, these variables
; will be stored as uninitialized data.
;-------------------------------------------------------------
macro uglobal {
  UGlobals equ UGlobals,
  macro __UGlobalBlock { }

macro uglobal_nested {
  UGlobals equ UGlobals,
  macro __UGlobalBlock \{ }

endg fix }      ; Use endg for ending iglobal and uglobal blocks.
endg_nested fix \}

macro IncludeIGlobals{
  macro IGlobals dummy,[n] \{ __IGlobalBlock
     purge __IGlobalBlock  \}
  match I, IGlobals \{ I \} }


macro IncludeUGlobals{
  macro UGlobals dummy,[n] \{
    \common
      \local begin, size
      begin = $
      virtual at $
    \forward
      __UGlobalBlock
      purge __UGlobalBlock
    \common
      size = $ - begin
    end virtual
    rb size
  \}
  match U, UGlobals \{ U \} }

macro IncludeAllGlobals {
  IncludeIGlobals
  IncludeUGlobals
}
 
iglobal
endg

uglobal
endg


macro forward_list_builder name {
        local c
        macro .#name a& \{
                \local m
                c equ m
                macro m a
        \}
        macro name#List \{
                irpv v,c \\{
                       ;\\forward
                        v
                \\}
        \}
}

macro reverse_list_builder name {
        local c
        macro .#name a& \{
                \local m
                c equ m
                macro m a
        \}
        macro name#List \{
                irpv v,c \\{
                        \\reverse
                        v
                \\}
        \}
}

forward_list_builder IGlobalBlock
forward_list_builder SGlobalBlock
forward_list_builder TGlobalBlock
forward_list_builder UGlobalBlock
forward_list_builder VGlobalBlock
forward_list_builder XGlobalBlock

;------------------------------------------------------------------
; use "tglobal" for inserting TLS definitions.
;------------------------------------------------------------------
macro tglobal {
  .TGlobalBlock { }

macro tglobal_nested {
  .TGlobalBlock \{ }

;------------------------------------------------------------------
; use "vglobal" for inserting virtual definitions.
;------------------------------------------------------------------
macro vglobal {
  .VGlobalBlock { }

macro vglobal_nested {
  .VGlobalBlock \{ }

;------------------------------------------------------------------
; use "iglobal" for inserting initialized global data definitions.
;------------------------------------------------------------------
macro iglobal {
  .IGlobalBlock { }

macro iglobal_nested {
  .IGlobalBlock \{ }

;------------------------------------------------------------------
; use "sglobal" for inserting strings definitions.
;------------------------------------------------------------------
macro sglobal {
  .SGlobalBlock { }

macro sglobal_nested {
  .SGlobalBlock \{ }

;-------------------------------------------------------------
; use 'uglobal' for inserting uninitialized global definitions.
; even when you define some data values, these variables
; will be stored as uninitialized data.
;-------------------------------------------------------------
macro uglobal {
  .UGlobalBlock { }

macro uglobal_nested {
  .UGlobalBlock \{ }

;-------------------------------------------------------------
; use 'xglobal' for inserting uninitialized global definitions.
; even when you define some data values, these variables
; will be stored as uninitialized data.
;-------------------------------------------------------------
macro xglobal {
  .XGlobalBlock { }

macro xglobal_nested {
  .XGlobalBlock \{ }

endg fix }              ; Use endg for ending iglobal and uglobal blocks.
endg_nested fix \}

macro IncludeGlobals MacroName
{
  local Begin,Size
  virtual at $
    Begin = $
    MacroName
    Size = $-Begin
  end virtual
  rb Size
}

macro IncludeAllGlobals {
  IGlobalBlockList
  SGlobalBlockList
UDataStr:
  IncludeGlobals UGlobalBlockList       ;will be zeroed
UDataEnd:
  IncludeGlobals XGlobalBlockList       ;will be undefined
}

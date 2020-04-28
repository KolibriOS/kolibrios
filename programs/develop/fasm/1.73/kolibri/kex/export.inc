
; Macroinstructions for making exports data structure

macro export [name,sname]
{
  forward
    dd __#name#_export_name__,name
  common
    dd 0
  forward
    align 4
    __#name#_export_name__ db sname,0
}
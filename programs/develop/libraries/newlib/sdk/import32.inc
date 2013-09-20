
; Macroinstructions for making import section

macro library [name,string]
 { common
    import.data:
   forward
    local _label
    if defined name#.redundant
     if ~ name#.redundant
      dd name#.lookup,0,0, _label, name#.address
     end if
    end if
    name#.referred = 1
   common
    dd 0,0,0,0,0
   forward
    if defined name#.redundant
     if ~ name#.redundant
      _label db string,0
       rb $ and 1
     end if
    end if }

macro import name,[label,string]
 { common
    rb (- $) and 3
    if defined name#.referred
     name#.lookup:
   forward
     if used label
      if string eqtype ''
       local _label
       dd _label
      else
       dd 80000000h + string
      end if
     end if
   common
     if $ > name#.lookup
      name#.redundant = 0
      dd 0
     else
      name#.redundant = 1
     end if
     name#.address:
   forward
     if used label
      if string eqtype ''
       label dd _label
      else
       label dd 80000000h + string
      end if
     end if
   common
     if ~ name#.redundant
      dd 0
     end if
   forward
     if used label & string eqtype ''
     _label dw 0
	    db string,0
      rb $ and 1
     end if
   common
    end if }

macro api [name] {}

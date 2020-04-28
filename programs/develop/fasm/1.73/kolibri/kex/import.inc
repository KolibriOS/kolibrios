
; Macroinstructions for making import section

macro library [name,string]
 { common
     import.data:
   forward
     local _label
     if defined name#.lookup
       if name#.lookup.size
	 dd name#.lookup,_label
       end if
     end if
     name#.referred = 1
   common
     dd 0
   forward
     if defined name#.lookup
       if name#.lookup.size
	 align 4
	 db '/sys/lib/'
	 _label db string,0
       end if
     end if }

macro import name,[label,string]
 { common
    align 4
    if defined name#.referred
     name#.lookup:
   forward
     if used label
      local _label
      label dd _label
     end if
   common
     if $ > name#.lookup
      dd 0
     end if
     name#.lookup.size=$ - name#.lookup
   forward
     if used label
      align 4
      _label db string,0
     end if
   common
    end if }

macro api [name] {}

; Macroinstructions for calling procedures (without defining)

macro stdcall proc,[arg]		; directly call STDCALL procedure
 { common
    if ~ arg eq
   reverse
    pushd arg
   common
    end if
    call proc }

macro invoke proc,[arg] 		; indirectly call STDCALL procedure
 { common
    if ~ arg eq
   reverse
     pushd arg
   common
    end if
    call [proc] }

macro ccall proc,[arg]			; directly call CDECL procedure
 { common
    size@ccall = 0
    if ~ arg eq
   reverse
    pushd arg
    size@ccall = size@ccall+4
   common
    end if
    call proc
    if size@ccall
    add esp,size@ccall
    end if }

macro cinvoke proc,[arg]		; indirectly call CDECL procedure
 { common
    size@ccall = 0
    if ~ arg eq
   reverse
    pushd arg
    size@ccall = size@ccall+4
   common
    end if
    call [proc]
    if size@ccall
    add esp,size@ccall
    end if }
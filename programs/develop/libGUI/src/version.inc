;****************************************************
;****************get version of library**************
;****************************************************
;IN
;not
;OUT
;version of library
;fersion format year:mouth:day
align 4

get_version:

        xor eax,eax
        mov eax,dword 080131  ;31.01.08

        ret
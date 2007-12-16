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
        mov eax,dword 071216  ;16.12.07

        ret
; LOAD DEFAULT SKIN

        mov     esi,_skin_file_default
        mov     edi,_skin_file
        movsd
        movsd
        movsd
        call    load_skin
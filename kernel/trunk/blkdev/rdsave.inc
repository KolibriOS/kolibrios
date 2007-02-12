iglobal
saverd_fileinfo:
        dd      2               ; subfunction: write
        dd      0               ; (reserved)
        dd      0               ; (reserved)
        dd      1440*1024       ; size 1440 Kb
        dd      0x100000 - std_application_base_address ; base address
        db      0
.name:
        dd      ?
endg
sysfn_saveramdisk:      ; 18.6 = SAVE FLOPPY IMAGE (HD version only)
        call    restorefatchain
        mov     eax, saverd_fileinfo - std_application_base_address
        mov     [saverd_fileinfo.name], ebx
        pushad
        push    eax
        call    file_system_lfn
        pop     eax
        popad
        mov     [esp+36], eax
        ret

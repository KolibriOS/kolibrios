:int load_dll3(dword dllname, import_table)
{
        EAX = 68;
        EBX = 19;
        ECX = dllname;
        $int     0x40
        IF(EAX==0) GOTO exit3;

        EDX = EAX;
        ESI = import_table;

@import_loop3:
        $lodsd
        IF(EAX==0) GOTO import_done3;
        $push    edx
@import_find3:
        EBX = DSDWORD[EDX];
        IF(EBX==0) GOTO exit3;
        $push    eax
@next3:
        CL = DSBYTE[EAX];
        $cmp     CL,DSBYTE[EBX];
        $jnz     import_find_next3
        IF (CL==0) GOTO import_found3;
        EAX++;
        EBX++;
        goto     next3;
@import_find_next3:
        $pop     eax
        EDX = EDX + 8;
        goto     import_find3;
@import_found3:
        $pop     ebx
        EAX = DSDWORD[EDX+4];
        DSDWORD[ESI-4] = EAX;
        $pop     edx

        goto     import_loop3;
@import_done3:
        return 0;
@exit3:
        return -1;
}

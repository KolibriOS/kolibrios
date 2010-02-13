
format MS COFF

;struct builtin_fw {
;        char *name;
;        void *data;
;        unsigned long size;
;};

public ___start_builtin_fw
public ___end_builtin_fw

section '.text' code readable executable align 16

align 16

___start_builtin_fw:

        dd FIRMWARE_R100
        dd R100CP_START
        dd (R100CP_END - R100CP_START)

        dd FIRMWARE_R200
        dd R200CP_START
        dd (R200CP_END - R200CP_START)

        dd FIRMWARE_R300
        dd R300CP_START
        dd (R300CP_END - R300CP_START)

        dd FIRMWARE_R420
        dd R420CP_START
        dd (R420CP_END - R420CP_START)

        dd FIRMWARE_RS690
        dd RS690CP_START
        dd (RS690CP_END - RS690CP_START)

        dd FIRMWARE_RS600
        dd RS600CP_START
        dd (RS600CP_END - RS600CP_START)

        dd FIRMWARE_R520
        dd R520CP_START
        dd (R520CP_END - R520CP_START)
___end_builtin_fw:


FIRMWARE_R100   db 'radeon/R100_cp.bin',0
FIRMWARE_R200   db 'radeon/R200_cp.bin',0
FIRMWARE_R300   db 'radeon/R300_cp.bin',0
FIRMWARE_R420   db 'radeon/R420_cp.bin',0
FIRMWARE_RS690  db 'radeon/RS690_cp.bin',0
FIRMWARE_RS600  db 'radeon/RS600_cp.bin',0
FIRMWARE_R520   db 'radeon/R520_cp.bin', 0

align 16

R100CP_START:
        file 'firmware/r100_cp.bin'
R100CP_END:

R200CP_START:
        file 'firmware/r200_cp.bin'
R200CP_END:

R300CP_START:
        file 'firmware/r300_cp.bin'
R300CP_END:

R420CP_START:
        file 'firmware/r420_cp.bin'
R420CP_END:

RS690CP_START:
        file 'firmware/rs690_cp.bin'
RS690CP_END:

RS600CP_START:
        file 'firmware/rs600_cp.bin'
RS600CP_END:

align 16
R520CP_START:
        file 'firmware/r520_cp.bin'
R520CP_END:

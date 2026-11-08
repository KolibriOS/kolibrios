; The target of RDsave and End, kept in app.ini; RDsave writes it before
; a save, so the saved image keeps it:
;   [RDSave]
;   target=1                            ; TARGET_FLOPPY
;   target=2                            ; TARGET_FILE, with
;   path=/hd0/1/kolibri.img
;
; The program defines target dd and fname_buf rb PATH_MAX and imports
; ini_get_int, ini_get_str of libini

PATH_MAX      = 4096

TARGET_NONE   = 0
TARGET_FLOPPY = 1
TARGET_FILE   = 2

FLOPPY_A      = 1                               ; SF 16: the first drive
ERR_DEVICE    = 11                              ; file system error 11, also for SF 16

; SF 18.11: the short table, its first byte the floppy drives, the first
; one in the high nibble
DISC_INFO_SHORT = 1
DISC_INFO_SIZE  = 16
FLOPPY_A_DRIVE  = 0xF0

; End runs "rdsave -then N" to ask for the target, N a digit,
; SSSF_SHUTDOWN_*: after the save, or without it, RDsave does the
; shutdown N
THEN_KEY      equ "-then "

; [target], fname_buf <- app.ini; only floppy A with a drive or a file
; with its path is a target, anything else TARGET_NONE
rd_read_target:
        invoke  ini_get_int, ini_file, ini_sec, key_target, TARGET_NONE
        mov     [target], eax
        invoke  ini_get_str, ini_file, ini_sec, key_path, fname_buf, PATH_MAX, ini_no_path

        cmp     [target], TARGET_FILE
        je      .file
        cmp     [target], TARGET_FLOPPY
        jne     .none

        call    rd_has_floppy
        test    eax, eax
        jz      .none
        ret

.file:
        cmp     byte [fname_buf], 0
        je      .none
        ret

.none:
        mov     [target], TARGET_NONE
        ret

; eax = 1 if there is a drive for floppy A, as the kernel makes /fd
; (blkdev/flp_drv.inc), else 0
rd_has_floppy:
        mcall   SF_SYSTEM, SSF_INFO_DISC_SYS, DISC_INFO_SHORT, rd_disc_info
        xor     eax, eax
        test    byte [rd_disc_info], FLOPPY_A_DRIVE
        jz      .done
        inc     eax

.done:
        ret

; [target] = TARGET_FLOPPY or TARGET_FILE: eax = 0 or an error code
rd_save:
        cmp     [target], TARGET_FLOPPY
        jne     .file

        mcall   SF_RD_TO_FLOPPY, FLOPPY_A
        test    eax, eax
        jz      .done
        mov     eax, ERR_DEVICE
        ret

.file:
        mcall   SF_SYSTEM, SSF_RD_TO_HDD, fname_buf

.done:
        ret

ini_file        db "/sys/settings/app.ini", 0
ini_sec         db "RDSave", 0
key_target      db "target", 0
key_path        db "path", 0
ini_no_path     db 0

rd_disc_info    rb DISC_INFO_SIZE


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

macro CP_code [arg]
{
        dd FIRMWARE_#arg#_CP
        dd arg#_CP_START
        dd (arg#_CP_END - arg#_CP_START)
}

macro CP_firmware [arg]
{
forward
FIRMWARE_#arg#_CP       db 'i915/',`arg,'.bin',0
forward

align 16
arg#_CP_START:
        file "firmware/"#`arg#".bin"
arg#_CP_END:
}

___start_builtin_fw:

CP_code skl_guc_ver4

___end_builtin_fw:

CP_firmware skl_guc_ver4



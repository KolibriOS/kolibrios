
format MS COFF

;struct builtin_fw {
;        char *name;
;        void *data;
;        unsigned long size;
;};

public ___start_builtin_fw
public ___end_builtin_fw

section '.rdata' data readable align 16

align 16

macro DMC_code [arg]
{
        dd FIRMWARE_#arg#_DMC
        dd arg#_DMC_START
        dd (arg#_DMC_END - arg#_DMC_START)
}

macro DMC_firmware [arg]
{
forward
FIRMWARE_#arg#_DMC       db 'i915/',`arg,'.bin',0
forward

align 16
arg#_DMC_START:
        file "firmware/"#`arg#".bin"
arg#_DMC_END:
}

macro GUC_code [arg]
{
        dd FIRMWARE_#arg#_GUC
        dd arg#_GUC_START
        dd (arg#_GUC_END - arg#_GUC_START)
}

macro GUC_firmware [arg]
{
forward
FIRMWARE_#arg#_GUC       db 'i915/',`arg,'.bin',0
forward

align 16
arg#_GUC_START:
        file "firmware/"#`arg#".bin"
arg#_GUC_END:
}

___start_builtin_fw:

DMC_code skl_dmc_ver1
DMC_code bxt_dmc_ver1
GUC_code skl_guc_ver4

___end_builtin_fw:

DMC_firmware skl_dmc_ver1
DMC_firmware bxt_dmc_ver1
GUC_firmware skl_guc_ver4



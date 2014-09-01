
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

macro CE_code [arg]
{
        dd FIRMWARE_#arg#_CE
        dd arg#_CE_START
        dd (arg#_CE_END - arg#_CE_START)
}

macro CE_firmware [arg]
{
forward
FIRMWARE_#arg#_CE       db 'radeon/',`arg,'_ce.bin',0
forward

align 16
arg#_CE_START:
        file "firmware/"#`arg#"_ce.bin"
arg#_CE_END:
}

macro CP_code [arg]
{
        dd FIRMWARE_#arg#_CP
        dd arg#_CP_START
        dd (arg#_CP_END - arg#_CP_START)
}

macro CP_firmware [arg]
{
forward
FIRMWARE_#arg#_CP       db 'radeon/',`arg,'_cp.bin',0
forward

align 16
arg#_CP_START:
        file "firmware/"#`arg#"_cp.bin"
arg#_CP_END:
}

macro PFP_code [arg]
{
        dd FIRMWARE_#arg#_PFP
        dd arg#_PFP_START
        dd (arg#_PFP_END - arg#_PFP_START)
}

macro PFP_firmware [arg]
{
forward
FIRMWARE_#arg#_PFP      db 'radeon/',`arg,'_pfp.bin',0
forward

align 16
arg#_PFP_START:
        file "firmware/"#`arg#"_pfp.bin"
arg#_PFP_END:
}

macro MC_code [arg]
{
        dd FIRMWARE_#arg#_MC
        dd arg#_MC_START
        dd (arg#_MC_END - arg#_MC_START)
}

macro MC_firmware [arg]
{

forward
FIRMWARE_#arg#_MC       db 'radeon/',`arg,'_mc.bin',0
forward

align 16
arg#_MC_START:
        file "firmware/"#`arg#"_mc.bin"
arg#_MC_END:
}

macro MC2_code [arg]
{
        dd FIRMWARE_#arg#_MC2
        dd arg#_MC2_START
        dd (arg#_MC2_END - arg#_MC2_START)
}

macro MC2_firmware [arg]
{

forward
FIRMWARE_#arg#_MC2       db 'radeon/',`arg,'_mc2.bin',0
forward

align 16
arg#_MC2_START:
        file "firmware/"#`arg#"_mc2.bin"
arg#_MC2_END:
}

macro ME_code [arg]
{
        dd FIRMWARE_#arg#_ME
        dd arg#_ME_START
        dd (arg#_ME_END - arg#_ME_START)
}

macro ME_firmware [arg]
{

forward
FIRMWARE_#arg#_ME       db 'radeon/',`arg,'_me.bin',0
forward

align 16
arg#_ME_START:
        file "firmware/"#`arg#"_me.bin"
arg#_ME_END:
}

macro MEC_code [arg]
{
        dd FIRMWARE_#arg#_MEC
        dd arg#_MEC_START
        dd (arg#_MEC_END - arg#_MEC_START)
}

macro MEC_firmware [arg]
{
forward
FIRMWARE_#arg#_MEC       db 'radeon/',`arg,'_mec.bin',0
forward

align 16
arg#_MEC_START:
        file "firmware/"#`arg#"_mec.bin"
arg#_MEC_END:
}

macro RLC_code [arg]
{
        dd FIRMWARE_#arg#_RLC
        dd arg#_RLC_START
        dd (arg#_RLC_END - arg#_RLC_START)
}

macro RLC_firmware [arg]
{
forward
FIRMWARE_#arg#_RLC       db 'radeon/',`arg,'_rlc.bin',0
forward

align 16
arg#_RLC_START:
        file "firmware/"#`arg#"_rlc.bin"
arg#_RLC_END:
}

macro SDMA_code [arg]
{
        dd FIRMWARE_#arg#_SDMA
        dd arg#_SDMA_START
        dd (arg#_SDMA_END - arg#_SDMA_START)
}

macro SDMA_firmware [arg]
{
forward
FIRMWARE_#arg#_SDMA       db 'radeon/',`arg,'_sdma.bin',0
forward

align 16
arg#_SDMA_START:
        file "firmware/"#`arg#"_sdma.bin"
arg#_SDMA_END:
}

macro SMC_code [arg]
{
        dd FIRMWARE_#arg#_SMC
        dd arg#_SMC_START
        dd (arg#_SMC_END - arg#_SMC_START)
}

macro SMC_firmware [arg]
{
forward
FIRMWARE_#arg#_SMC       db 'radeon/',`arg,'_smc.bin',0
forward

align 16
arg#_SMC_START:
        file "firmware/"#`arg#"_smc.bin"
arg#_SMC_END:
}

macro UVD_code [arg]
{
        dd FIRMWARE_#arg#_UVD
        dd arg#_UVD_START
        dd (arg#_UVD_END - arg#_UVD_START)
}

macro UVD_firmware [arg]
{
forward
FIRMWARE_#arg#_UVD       db 'radeon/',`arg,'_uvd.bin',0
forward

align 16
arg#_UVD_START:
        file "firmware/"#`arg#"_uvd.bin"
arg#_UVD_END:
}

macro VCE_code [arg]
{
        dd FIRMWARE_#arg#_VCE
        dd arg#_VCE_START
        dd (arg#_VCE_END - arg#_VCE_START)
}

macro VCE_firmware [arg]
{
forward
FIRMWARE_#arg#_VCE       db 'radeon/',`arg,'_vce.bin',0
forward

align 16
arg#_VCE_START:
        file "firmware/"#`arg#"_vce.bin"
arg#_VCE_END:
}

___start_builtin_fw:

CE_code BONAIRE, HAINAN, HAWAII, KABINI, KAVERI, MULLINS, OLAND,\
        PITCAIRN, TAHITI, VERDE

CP_code R100, R200, R300, R420, R520, RS600, RS690

MC_code BARTS, BONAIRE, CAICOS, CAYMAN, HAINAN,\
        HAWAII, OLAND, PITCAIRN,\
        TAHITI, TURKS, VERDE

MC2_code BONAIRE, HAINAN, HAWAII, OLAND, PITCAIRN,\
        TAHITI, VERDE

ME_code R600, RS780, RV610, RV620, RV630, RV635, RV670, RV710, RV730, RV770,\
        ARUBA, BARTS, BONAIRE, CAICOS, CAYMAN, CEDAR, CYPRESS, HAINAN,\
        HAWAII, JUNIPER, KABINI, KAVERI, MULLINS, OLAND, PALM, PITCAIRN,\
        REDWOOD, SUMO, SUMO2, TAHITI, TURKS, VERDE

MEC_code BONAIRE, HAWAII, KABINI, KAVERI, MULLINS

PFP_code R600, RS780, RV610, RV620, RV630, RV635, RV670, RV710, RV730, RV770,\
        ARUBA, BARTS, BONAIRE, CAICOS, CAYMAN, CEDAR, CYPRESS, HAINAN,\
        HAWAII, JUNIPER, KABINI, KAVERI, MULLINS, OLAND, PALM, PITCAIRN,\
        REDWOOD, SUMO, SUMO2, TAHITI, TURKS, VERDE

RLC_code R600, R700,\
        ARUBA, BONAIRE, BTC, CAYMAN, CEDAR, CYPRESS, HAINAN,\
        HAWAII, JUNIPER, KABINI, KAVERI, MULLINS, OLAND, PITCAIRN,\
        REDWOOD, SUMO, TAHITI, VERDE

SDMA_code BONAIRE, HAWAII, KABINI, KAVERI, MULLINS

SMC_code RV710, RV730, RV740, RV770,\
        BARTS, BONAIRE, CAICOS, CAYMAN, CEDAR, CYPRESS, HAINAN,\
        HAWAII, JUNIPER, OLAND, PITCAIRN,\
        REDWOOD, TAHITI, TURKS, VERDE

UVD_code RV710, BONAIRE, CYPRESS, SUMO, TAHITI

VCE_code BONAIRE

___end_builtin_fw:

CE_firmware BONAIRE, HAINAN, HAWAII, KABINI, KAVERI, MULLINS, OLAND,\
        PITCAIRN, TAHITI, VERDE

CP_firmware R100, R200, R300, R420, R520, RS600, RS690

MC_firmware BARTS, BONAIRE, CAICOS, CAYMAN, HAINAN,\
        HAWAII, OLAND, PITCAIRN,\
        TAHITI, TURKS, VERDE

MC2_firmware BONAIRE, HAINAN, HAWAII, OLAND, PITCAIRN,\
        TAHITI, VERDE

ME_firmware R600, RS780, RV610, RV620, RV630, RV635, RV670, RV710, RV730, RV770,\
        ARUBA, BARTS, BONAIRE, CAICOS, CAYMAN, CEDAR, CYPRESS, HAINAN,\
        HAWAII, JUNIPER, KABINI, KAVERI, MULLINS, OLAND, PALM, PITCAIRN,\
        REDWOOD, SUMO, SUMO2, TAHITI, TURKS, VERDE

MEC_firmware BONAIRE, HAWAII, KABINI, KAVERI, MULLINS

PFP_firmware R600, RS780, RV610, RV620, RV630, RV635, RV670, RV710, RV730, RV770,\
        ARUBA, BARTS, BONAIRE, CAICOS, CAYMAN, CEDAR, CYPRESS, HAINAN,\
        HAWAII, JUNIPER, KABINI, KAVERI, MULLINS, OLAND, PALM, PITCAIRN,\
        REDWOOD, SUMO, SUMO2, TAHITI, TURKS, VERDE

RLC_firmware R600, R700,\
        ARUBA, BONAIRE, BTC, CAYMAN, CEDAR, CYPRESS, HAINAN,\
        HAWAII, JUNIPER, KABINI, KAVERI, MULLINS, OLAND, PITCAIRN,\
        REDWOOD, SUMO, TAHITI, VERDE

SDMA_firmware BONAIRE, HAWAII, KABINI, KAVERI, MULLINS

SMC_firmware RV710, RV730, RV740, RV770,\
        BARTS, BONAIRE, CAICOS, CAYMAN, CEDAR, CYPRESS, HAINAN,\
        HAWAII, JUNIPER, OLAND, PITCAIRN,\
        REDWOOD, TAHITI, TURKS, VERDE

UVD_firmware RV710, BONAIRE, CYPRESS, SUMO, TAHITI

VCE_firmware BONAIRE


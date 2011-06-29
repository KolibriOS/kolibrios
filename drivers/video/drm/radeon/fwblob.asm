
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

        dd FIRMWARE_R100_CP
        dd R100CP_START
        dd (R100CP_END - R100CP_START)

        dd FIRMWARE_R200_CP
        dd R200CP_START
        dd (R200CP_END - R200CP_START)

        dd FIRMWARE_R300_CP
        dd R300CP_START
        dd (R300CP_END - R300CP_START)

        dd FIRMWARE_R420_CP
        dd R420CP_START
        dd (R420CP_END - R420CP_START)

        dd FIRMWARE_R520_CP
        dd R520CP_START
        dd (R520CP_END - R520CP_START)

        dd FIRMWARE_RS600_CP
        dd RS600CP_START
        dd (RS600CP_END - RS600CP_START)

        dd FIRMWARE_RS690_CP
        dd RS690CP_START
        dd (RS690CP_END - RS690CP_START)



        dd FIRMWARE_R600_ME
        dd R600ME_START
        dd (R600ME_END - R600ME_START)

        dd FIRMWARE_RS780_ME
        dd RS780ME_START
        dd (RS780ME_END - RS780ME_START)

        dd FIRMWARE_RS780_PFP
        dd RS780PFP_START
        dd (RS780PFP_END - RS780PFP_START)

        dd FIRMWARE_RV610_ME
        dd RV610ME_START
        dd (RV610ME_END - RV610ME_START)

        dd FIRMWARE_RV620_ME
        dd RV620ME_START
        dd (RV620ME_END - RV620ME_START)

        dd FIRMWARE_RV630_ME
        dd RV630ME_START
        dd (RV630ME_END - RV630ME_START)

        dd FIRMWARE_RV635_ME
        dd RV635ME_START
        dd (RV635ME_END - RV635ME_START)

        dd FIRMWARE_RV670_ME
        dd RV670ME_START
        dd (RV670ME_END - RV670ME_START)

        dd FIRMWARE_RV710_ME
        dd RV710ME_START
        dd (RV710ME_END - RV710ME_START)

        dd FIRMWARE_RV730_ME
        dd RV730ME_START
        dd (RV730ME_END - RV730ME_START)

        dd FIRMWARE_RV770_ME
        dd RV770ME_START
        dd (RV770ME_END - RV770ME_START)

        dd FIRMWARE_CYPRESS_ME
        dd CYPRESSME_START
        dd (CYPRESSME_END - CYPRESSME_START)

        dd FIRMWARE_REDWOOD_ME
        dd REDWOODME_START
        dd (REDWOODME_END - REDWOODME_START)

        dd FIRMWARE_CEDAR_ME
        dd CEDARME_START
        dd (CEDARME_END - CEDARME_START)

        dd FIRMWARE_JUNIPER_ME
        dd JUNIPERME_START
        dd (JUNIPERME_END - JUNIPERME_START)

        dd FIRMWARE_PALM_ME
        dd PALMME_START
        dd (PALMME_END - PALMME_START)


        dd FIRMWARE_RV610_PFP
        dd RV610PFP_START
        dd (RV610PFP_END - RV610PFP_START)

        dd FIRMWARE_RV620_PFP
        dd RV620PFP_START
        dd (RV620PFP_END - RV620PFP_START)

        dd FIRMWARE_RV630_PFP
        dd RV630PFP_START
        dd (RV630PFP_END - RV630PFP_START)

        dd FIRMWARE_RV635_PFP
        dd RV635PFP_START
        dd (RV635PFP_END - RV635PFP_START)

        dd FIRMWARE_RV670_PFP
        dd RV670PFP_START
        dd (RV670PFP_END - RV670PFP_START)


        dd FIRMWARE_RV710_PFP
        dd RV670PFP_START
        dd (RV710PFP_END - RV710PFP_START)

        dd FIRMWARE_RV730_PFP
        dd RV730PFP_START
        dd (RV730PFP_END - RV730PFP_START)

        dd FIRMWARE_RV770_PFP
        dd RV770PFP_START
        dd (RV770PFP_END - RV770PFP_START)

        dd FIRMWARE_CYPRESS_PFP
        dd CYPRESSPFP_START
        dd (CYPRESSPFP_END - CYPRESSPFP_START)

        dd FIRMWARE_REDWOOD_PFP
        dd REDWOODPFP_START
        dd (REDWOODPFP_END - REDWOODPFP_START)

        dd FIRMWARE_CEDAR_PFP
        dd CEDARPFP_START
        dd (CEDARPFP_END - CEDARPFP_START)

        dd FIRMWARE_JUNIPER_PFP
        dd JUNIPERPFP_START
        dd (JUNIPERPFP_END - JUNIPERPFP_START)

        dd FIRMWARE_PALM_PFP
        dd PALMPFP_START
        dd (PALMPFP_END - PALMPFP_START)



        dd FIRMWARE_R600_RLC
        dd R600RLC_START
        dd (R600RLC_END - R600RLC_START)

        dd FIRMWARE_R700_RLC
        dd R700RLC_START
        dd (R700RLC_END - R700RLC_START)

        dd FIRMWARE_CYPRESS_RLC
        dd CYPRESSRLC_START
        dd (CYPRESSRLC_END - CYPRESSRLC_START)

        dd FIRMWARE_REDWOOD_RLC
        dd REDWOODRLC_START
        dd (REDWOODRLC_END - REDWOODRLC_START)

        dd FIRMWARE_CEDAR_RLC
        dd CEDARRLC_START
        dd (CEDARRLC_END - CEDARRLC_START)

        dd FIRMWARE_JUNIPER_RLC
        dd JUNIPERRLC_START
        dd (JUNIPERRLC_END - JUNIPERRLC_START)


___end_builtin_fw:


FIRMWARE_R100_CP        db 'radeon/R100_cp.bin',0
FIRMWARE_R200_CP        db 'radeon/R200_cp.bin',0
FIRMWARE_R300_CP        db 'radeon/R300_cp.bin',0
FIRMWARE_R420_CP        db 'radeon/R420_cp.bin',0
FIRMWARE_R520_CP        db 'radeon/R520_cp.bin',0

FIRMWARE_RS600_CP       db 'radeon/RS600_cp.bin',0
FIRMWARE_RS690_CP       db 'radeon/RS690_cp.bin',0

FIRMWARE_RS780_ME       db 'radeon/RS780_me.bin',0
FIRMWARE_RS780_PFP      db 'radeon/RS780_pfp.bin',0

FIRMWARE_R600_ME        db 'radeon/RV600_me.bin',0
FIRMWARE_RV610_ME       db 'radeon/RV610_me.bin',0
FIRMWARE_RV620_ME       db 'radeon/RV620_me.bin',0
FIRMWARE_RV630_ME       db 'radeon/RV630_me.bin',0
FIRMWARE_RV635_ME       db 'radeon/RV635_me.bin',0
FIRMWARE_RV670_ME       db 'radeon/RV670_me.bin',0
FIRMWARE_RV710_ME       db 'radeon/RV710_me.bin',0
FIRMWARE_RV730_ME       db 'radeon/RV730_me.bin',0
FIRMWARE_RV770_ME       db 'radeon/RV770_me.bin',0
FIRMWARE_CYPRESS_ME     db 'radeon/CYPRESS_me.bin',0
FIRMWARE_REDWOOD_ME     db 'radeon/REDWOOD_me.bin',0
FIRMWARE_CEDAR_ME       db 'radeon/CEDAR_me.bin',0
FIRMWARE_JUNIPER_ME     db 'radeon/JUNIPER_me.bin',0
FIRMWARE_PALM_ME        db 'radeon/PALM_me.bin',0


FIRMWARE_R600_PFP       db 'radeon/R600_pfp.bin',0
FIRMWARE_RV610_PFP      db 'radeon/RV610_pfp.bin',0
FIRMWARE_RV620_PFP      db 'radeon/RV620_pfp.bin',0
FIRMWARE_RV630_PFP      db 'radeon/RV630_pfp.bin',0
FIRMWARE_RV635_PFP      db 'radeon/RV635_pfp.bin',0
FIRMWARE_RV670_PFP      db 'radeon/RV670_pfp.bin',0
FIRMWARE_RV710_PFP      db 'radeon/RV710_pfp.bin',0
FIRMWARE_RV730_PFP      db 'radeon/RV730_pfp.bin',0
FIRMWARE_RV770_PFP      db 'radeon/RV770_pfp.bin',0
FIRMWARE_CYPRESS_PFP    db 'radeon/CYPRESS_pfp.bin',0
FIRMWARE_REDWOOD_PFP    db 'radeon/REDWOOD_pfp.bin',0
FIRMWARE_CEDAR_PFP      db 'radeon/CEDAR_pfp.bin',0
FIRMWARE_JUNIPER_PFP    db 'radeon/JUNIPER_pfp.bin',0
FIRMWARE_PALM_PFP       db 'radeon/PALM_pfp.bin',0


FIRMWARE_R600_RLC       db 'radeon/R600_rlc.bin',0
FIRMWARE_R700_RLC       db 'radeon/R700_rlc.bin',0
FIRMWARE_CYPRESS_RLC    db 'radeon/CYPRESS_rlc.bin',0
FIRMWARE_REDWOOD_RLC    db 'radeon/REDWOOD_rlc.bin',0
FIRMWARE_CEDAR_RLC      db 'radeon/CEDAR_rlc.bin',0
FIRMWARE_JUNIPER_RLC    db 'radeon/JUNIPER_rlc.bin',0


align 16
R100CP_START:
        file 'firmware/R100_cp.bin'
R100CP_END:

align 16
R200CP_START:
        file 'firmware/R200_cp.bin'
R200CP_END:

align 16
R300CP_START:
        file 'firmware/R300_cp.bin'
R300CP_END:

align 16
R420CP_START:
        file 'firmware/R420_cp.bin'
R420CP_END:

align 16
R520CP_START:
        file 'firmware/R520_cp.bin'
R520CP_END:

align 16
RS600CP_START:
        file 'firmware/RS600_cp.bin'
RS600CP_END:

align 16
RS690CP_START:
        file 'firmware/RS690_cp.bin'
RS690CP_END:

align 16
RS780ME_START:
        file 'firmware/RS780_me.bin'
RS780ME_END:

align 16
RS780PFP_START:
        file 'firmware/RS780_pfp.bin'
RS780PFP_END:

align 16
R600ME_START:
        file 'firmware/R600_me.bin'
R600ME_END:

align 16
RV610ME_START:
        file 'firmware/RV610_me.bin'
RV610ME_END:

align 16
RV620ME_START:
        file 'firmware/RV620_me.bin'
RV620ME_END:

align 16
RV630ME_START:
        file 'firmware/RV630_me.bin'
RV630ME_END:

align 16
RV635ME_START:
        file 'firmware/RV635_me.bin'
RV635ME_END:

align 16
RV670ME_START:
        file 'firmware/RV670_me.bin'
RV670ME_END:


align 16
RV710ME_START:
        file 'firmware/RV710_me.bin'
RV710ME_END:

align 16
RV730ME_START:
        file 'firmware/RV730_me.bin'
RV730ME_END:

align 16
RV770ME_START:
        file 'firmware/RV770_me.bin'
RV770ME_END:

align 16
CYPRESSME_START:
        file 'firmware/CYPRESS_me.bin'
CYPRESSME_END:

align 16
REDWOODME_START:
        file 'firmware/REDWOOD_me.bin'
REDWOODME_END:

align 16
CEDARME_START:
        file 'firmware/CEDAR_me.bin'
CEDARME_END:

align 16
JUNIPERME_START:
        file 'firmware/JUNIPER_me.bin'
JUNIPERME_END:

align 16
PALMME_START:
        file 'firmware/PALM_me.bin'
PALMME_END:


align 16
RV610PFP_START:
        file 'firmware/RV610_pfp.bin'
RV610PFP_END:


align 16
RV620PFP_START:
        file 'firmware/RV620_pfp.bin'
RV620PFP_END:

align 16
RV630PFP_START:
        file 'firmware/RV630_pfp.bin'
RV630PFP_END:


align 16
RV635PFP_START:
        file 'firmware/RV635_pfp.bin'
RV635PFP_END:

align 16
RV670PFP_START:
        file 'firmware/RV670_pfp.bin'
RV670PFP_END:

align 16
RV710PFP_START:
        file 'firmware/RV710_pfp.bin'
RV710PFP_END:

align 16
RV730PFP_START:
        file 'firmware/RV730_pfp.bin'
RV730PFP_END:


align 16
RV770PFP_START:
        file 'firmware/RV770_pfp.bin'
RV770PFP_END:


align 16
CYPRESSPFP_START:
        file 'firmware/CYPRESS_pfp.bin'
CYPRESSPFP_END:

align 16
REDWOODPFP_START:
        file 'firmware/REDWOOD_pfp.bin'
REDWOODPFP_END:

align 16
CEDARPFP_START:
        file 'firmware/CEDAR_pfp.bin'
CEDARPFP_END:

align 16
JUNIPERPFP_START:
        file 'firmware/JUNIPER_pfp.bin'
JUNIPERPFP_END:

align 16
PALMPFP_START:
        file 'firmware/PALM_pfp.bin'
PALMPFP_END:


align 16
R600RLC_START:
        file 'firmware/R600_rlc.bin'
R600RLC_END:

align 16
R700RLC_START:
        file 'firmware/R700_rlc.bin'
R700RLC_END:

align 16
CYPRESSRLC_START:
        file 'firmware/CYPRESS_rlc.bin'
CYPRESSRLC_END:

align 16
REDWOODRLC_START:
        file 'firmware/REDWOOD_rlc.bin'
REDWOODRLC_END:

align 16
CEDARRLC_START:
        file 'firmware/CEDAR_rlc.bin'
CEDARRLC_END:

align 16
JUNIPERRLC_START:
        file 'firmware/JUNIPER_rlc.bin'
JUNIPERRLC_END:


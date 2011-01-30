; --------------------------------------------------------------------------
; FILE: TCommon.Inc
; DATE: October 4, 2008
; --------------------------------------------------------------------------

DEV_SR_SENSORS = 1
DEV_LR_SENSORS = 2
DEV_PHASERS = 3
DEV_PHOTON_TUBES = 4
DEV_LIFE_SUPPORT = 5
DEV_WARP_ENGINES = 6
DEV_IMPULSE_ENGINES = 7
DEV_SHIELDS = 8
DEV_SUBSPACE_RADIO = 9
DEV_SHUTTLE_CRAFT = 10
DEV_COMPUTER = 11
DEV_TRANSPORTER = 12
DEV_SHIELD_CONTROL = 13
DEV_DEATHRAY = 14

CONDITION_GREEN = 1
CONDITION_RED = 2
CONDITION_YELLOW = 3
CONDITION_DOCKED = 4

virtual at 0
TPlanet:
    .planet_X BYTE ?
    .planet_Y BYTE ?
    .planet_CLASS BYTE ?
    .planet_DILITHIUM BYTE ?
    .planet_SCANNED BYTE ?
    .size = $
end virtual

virtual at 0
TREKDATA:
    .CRYPROB DOUBLE ?
    .DATE DOUBLE ?
    .REMRES DOUBLE ?
    .REMTIME DOUBLE ?
    .KPOWER DOUBLES 20
    .KDIST DOUBLES 20
    .DAMAGE DOUBLES 14
    .INRESOR DOUBLE ?
    .INTIME DOUBLE ?
    .INENRG DOUBLE ?
    .INSHLD DOUBLE ?
    .INLSR DOUBLE ?
    .INDATE DOUBLE ?
    .ENERGY DOUBLE ?
    .SHLD DOUBLE ?
    .WARPFAC DOUBLE ?
    .WFACSQ DOUBLE ?
    .LSUPRES DOUBLE ?
    .DIST DOUBLE ?
    .DIREC DOUBLE ?
    .TIME DOUBLE ?
    .DOCKDATE DOUBLE ?
    .DAMFAC DOUBLE ?
    .FUTURE1 DOUBLE ?
    .FUTURE2 DOUBLE ?
    .FUTURE3 DOUBLE ?
    .FUTURE4 DOUBLE ?
    .FUTURE5 DOUBLE ?
    .FUTURE6 DOUBLE ?
    .FUTURE7 DOUBLE ?
    .HEIGHT DOUBLE ?
    ;
    ; Integer variables
    ;
align 4
    .GALAXY INTEGERS 64
    .STARCH INTEGERS 64
    .INSTAR INT32 ?
    .REMKL BYTE ?
    .REMCOM BYTE ?
    .REMBASE BYTE ?
    .STARKL BYTE ?
    .BASEKL BYTE ?
    .KILLK BYTE ?
    .KILLC BYTE ?
    .CX BYTES 10
    .CY BYTES 10
    .BASEQX BYTES 5
    .BASEQY BYTES 5
    .NEWSTUF BYTES 64
    .SCANNED BYTES 64
    .PLNETS BYTES (10 * TPlanet.size)
    .ISX BYTE ?
    .ISY BYTE ?
    .KSTUF4 BYTE ?
    .KSTUF5 BYTE ?
    .NSCREM BYTE ?
    .NROMKL BYTE ?
    .NROMREM BYTE ?
    .NSCKILL BYTE ?
    .ICRYSTL BYTE ?
    .NPLANKL BYTE ?
    .QUAD BYTES 100
    .KX BYTES 20
    .KY BYTES 20
    .INKLING BYTE ?
    .INBASE BYTE ?
    .INCOM BYTE ?
    .INTORPS BYTE ?
    .SHLDUP BYTE ?
    .CONDIT BYTE ?
    .TORPS BYTE ?
    .SHIP BYTE ?
    .QUADX BYTE ?
    .QUADY BYTE ?
    .SECTX BYTE ?
    .SECTY BYTE ?
    .ITHX BYTE ?
    .ITHY BYTE ?
    .ITHERE BYTE ?
    .LENGTH BYTE ?
    .SKILL BYTE ?
    .QTORPS BYTE ?
    .NPTUBES BYTE ?
    .PASSWD CHARS 12
    .BASEX BYTE ?
    .BASEY BYTE ?
    .KLHERE BYTE ?
    .COMHERE BYTE ?
    .CASUAL BYTE ?
    .NHELP BYTE ?
    .NKINKS BYTE ?
    .IDIDIT BYTE ?
    .KDIDIT BYTE ?
    .KSHOT BYTE ?
    .GAMEWON BYTE ?
    .ALIVE BYTE ?
    .JUSTIN BYTE ?
    .RESTING BYTE ?
    .ALLDONE BYTE ?
    .SHLDCHG BYTE ?
    .THINGX BYTE ?
    .THINGY BYTE ?
    .PLNETX BYTE ?
    .PLNETY BYTE ?
    .INORBIT BYTE ?
    .LANDED BYTE ?
    .IPLANET BYTE ?
    .IMINE BYTE ?
    .INPLAN BYTE ?
    .NENHERE BYTE ?
    .ISHERE BYTE ?
    .NEUTZ BYTE ?
    .IRHERE BYTE ?
    .ICRAFT BYTE ?
    .IENTESC BYTE ?
    .ISCRAFT BYTE ?
    .ISATB BYTE ?
    .ISCATE BYTE ?
    .IPHWHO BYTE ?
    .BATX BYTE ?
    .BATY BYTE ?
    .ICSOS BYTE ?
    .ISSOS BYTE ?
    .ISUBDAM BYTE ?
    .SHUTUP BYTE ?
    .size = $
end virtual

virtual at 0
TCommon:
    .SNAP BYTE ?
    .SNAPSHT BYTES TREKDATA.size

align 16
    .GAMEDB BYTES TREKDATA.size
    .size = $
end virtual

macro mcLoadGameDataPtr r32
{
    mov     r32, [glb_pGameData]
}

PTCommon equ dd
PTGameData equ dd

; --- EOF ---

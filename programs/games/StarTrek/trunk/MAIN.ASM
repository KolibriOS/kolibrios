; --------------------------------------------------------------------------
; Win32 PE File Template
; {BINEXT=EXE} (Do not edit or remove this line)
; --------------------------------------------------------------------------

; *********************************************************************
; *         PERMISSION IS HEREBY GRANTED FOR THE COPYING,             *
; *    DISTRIBUTION, MODIFICATION AND USE OF THIS PROGRAM AND         *
; *    ASSOCIATED DOCUMENTATION FOR RECREATIONAL PURPOSES,            *
; *    PROVIDED THAT ALL REFERENCES TO THE AUTHOR ARE RETAINED.       *
; *    HOWEVER, PERMISSION IS NOT AND WILL NOT BE GRANTED FOR         *
; *    THE SALE OR PROMOTIONAL USE OF THIS PROGRAM OR PROGRAM         *
; *    DOCUMENTATION, OR FOR USE IN ANY SITUATION IN WHICH            *
; *    PROFIT MAY BE CONSIDERED AN OBJECTIVE, SINCE IT IS THE         *
; *    DESIRE OF THE AUTHOR TO RESPECT THE COPYRIGHTS OF THE          *
; *    ORIGINATORS OF STAR TREK.                                      *
; *********************************************************************

    format  PE GUI 4.0
    entry   start
    stack   10000h, 10000h

; --------------------------------------------------------------------------
; Definitions
; --------------------------------------------------------------------------

    include 'Macros.Inc'
    include 'Win32.Inc'
    include 'TConsole.Inc'
    include 'TCmdBuf.Inc'
    include 'TCommon.Inc'
    include 'TRandom.Inc'
    include 'TLog.Inc'
    ;{INCLDEF} (Definitions will be inserted before this line)

    PROC_ALIGN equ 8

; --------------------------------------------------------------------------
; Data Section
; --------------------------------------------------------------------------
section '.data' data readable writeable

    include 'Globals.Inc'

; --------------------------------------------------------------------------
; Code Modules
; --------------------------------------------------------------------------
section '.code' code readable executable

    include 'TApp.Asm'
    include 'TConsole.Asm'
    include 'TString.Asm'
    include 'TCmdBuf.Asm'
    include 'TGame.Asm'
    include 'TMsgTable.Asm'
    include 'TCommon.Asm'
    include 'TRandom.Asm'
    include 'TArray.Asm'
    include 'TFormat.Asm'
    include 'TSrScan.Asm'
    include 'TLrScan.Asm'
    include 'TChart.Asm'
    include 'TMove.Asm'
    include 'TFinish.Asm'
    include 'TAttack.Asm'
    include 'TEvents.Asm'
    include 'TNova.Asm'
    include 'TPhasers.Asm'
    include 'TShields.Asm'
    include 'TDock.Asm'
    include 'TPhotons.Asm'
    include 'TPlanet.Asm'
    include 'TImpulse.Asm'
    include 'TLog.Asm'
    ;{INCLIMPL} (Code modules will be inserted before this line)

; --------------------------------------------------------------------------
; Entry Point
; --------------------------------------------------------------------------
align PROC_ALIGN
start:
    call    TApp_SetLocalDir
    jnc     .exit

    call    TApp_Init
    call    TApp_Run

.exit:
    invoke  ExitProcess, 0

; --------------------------------------------------------------------------
; Imports
; --------------------------------------------------------------------------
section '.idata' import data readable writeable

    include 'Imports.Inc'

; --- EOF ---

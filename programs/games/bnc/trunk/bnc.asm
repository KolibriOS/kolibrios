;************************************************
;*  Bulls-and-Cows       */ Kolibri OS /*
;*************************  25/10/2007  *********
include 'main.inc'   ; main.inc
include 'data.inc'   ; data.inc
include 'code.inc'   ; code.inc
include 'macs.inc'   ; macs.inc
include 'macros.inc' ; standard macro definitions - mcall & all-all-all

  BeginProgram
  call clears
  call rndseed
  call rnew

  main_loop

  CODE_SECTION

  DATA_SECTION

  EndProgram


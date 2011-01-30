; --------------------------------------------------------------------------
; FILE: TRandom.Inc
; DATE: October 4, 2008
; --------------------------------------------------------------------------

virtual at 0
TRandom:
    .rand_NumberSet INTEGERS 624
    .rand_NextIndex INT32 ?
    .size = $
end virtual

; --- EOF ---

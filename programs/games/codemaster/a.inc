; $$$$$$$$$$$$$$$$$$$ ABAKIS $$$$$$$$$$$$$$$$$$$$$
; ************** STAR^2 SOFTWARE *****************
; ????????????????????????????????????????????????

; true portability: windows + kolibri

; define cpu intel windows
 define cpu intel kolibri
; define cpu arm mobile

; current directory. optional '/usbhd0/1'
; prefix to assemble+run from SD in kolibri

 CD fix ''
; CD fix '/usbhd0/1/'

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

MAIN.ICON fix ''

SCREEN.W   = WINDOW.W ; 1024
SCREEN.H   = WINDOW.H ; 768-32
SCREEN.BPP = 32
SCREEN.PI  = SCREEN.W*(SCREEN.BPP/8)

match x =windows, cpu {
 format PE GUI 4.0
 entry !main
 stack 8*KB
 heap 16*KB
 include CD#'/include/a.inc'
 WINDOW.X=0
 WINDOW.Y=0
}

match x =kolibri, cpu {
 format binary as ""
 use32
 org 0
 db 'MENUET01'
 dd 1, !main, end.code,\
  4*MB, 32*KB, 0, 0
 postpone \{ end.code: \}
 include CD#'include/a.inc'
 WINDOW.X=(1024/2)-(WINDOW.W/2)
 WINDOW.Y=(768/2)-(WINDOW.H/2)
}
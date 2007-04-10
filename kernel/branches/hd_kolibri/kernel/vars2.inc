

; device irq owners
uglobal
irq_owner:       ; process id

     dd   0x0
     dd   0x0
     dd   0x0
     dd   0x0
     dd   0x0
     dd   0x0
     dd   0x0
     dd   0x0
     dd   0x0
     dd   0x0
     dd   0x0
     dd   0x0
     dd   0x0
     dd   0x0
     dd   0x0
     dd   0x0
endg


; on irq read ports
uglobal
  irq00read  dd  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  irq01read  dd  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  irq02read  dd  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  irq03read  dd  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  irq04read  dd  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  irq05read  dd  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  irq06read  dd  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  irq07read  dd  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  irq08read  dd  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  irq09read  dd  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  irq10read  dd  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  irq11read  dd  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  irq12read  dd  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  irq13read  dd  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  irq14read  dd  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  irq15read  dd  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
endg

; status
uglobal
  hd1_status                  dd 0x0  ; 0 - free : other - pid
  application_table_status    dd 0x0  ; 0 - free : other - pid
endg

; device addresses
uglobal
  mididp     dd 0x0
  midisp     dd 0x0

  cdbase     dd 0x0
  cdid       dd 0x0

  hdbase              dd   0x1f0  ; for boot 0x1f0
  hdid                dd   0x0
  hdpos               dd   0x1  ; for boot 0x1
  fat32part           dd   0x1  ; for boot 0x1

  ;part2_ld            dd   0x0

;* start code - Mario79
mouse_pause         dd   0
MouseTickCounter    dd   0
ps2_mouse_detected  db   0
com1_mouse_detected db   0
com2_mouse_detected db   0
;* end code - Mario79

wraw_bacground_select db 0
  lba_read_enabled    dd   0x1  ; 0 = disabled , 1 = enabled
  pci_access_enabled  dd   0x1  ; 0 = disabled , 1 = enabled

  sb16       dd 0x0

  buttontype         dd 0x0
  windowtypechanged  dd 0x0

align 4
  cpu_caps    dd 4 dup(0)
  pg_data  PG_DATA
  heap_test   dd ?
  hd_entries  rd 1     ;unused ? 0xfe10
endg

iglobal
  keyboard   dd 0x1
  sound_dma  dd 0x1
  syslang    dd 0x1
endg

if __DEBUG__ eq 1
  include_debug_strings
end if
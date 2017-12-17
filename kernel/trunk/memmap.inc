;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2015. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   MEMORY MAP
;
;   Boot:
;
;   BOOT_LO / boot_data structure
;   0:9000     byte   bits per pixel
;   0:9001     word   scanline length
;   0:9008     word   vesa video mode
;   0:900A     word   X res
;   0:900C     word   Y res
;   0:9014     dword  Vesa 1.2 pm bank switch
;   0:9018     dword  Vesa 2.0 LFB address
;   0:901C     byte   0 or 1 : enable MTRR graphics acceleration
;   0:901D     byte   (0 or 1) start the first app (right now it's LAUNCHER) after kernel is loaded?
;   0:901E     byte   If nonzero, duplicates debug output to the screen.
;   0:901F     byte   DMA write : 1=yes, 2=no
;   0:9020     8bytes pci data
;   0:9030     byte   shutdown type (see sysfn 18.9)
;   0:9040     dword  entry point of APM BIOS
;   0:9044     word   version (BCD)
;   0:9046     word   flags
;   0:9050     word   APM_CODE_32
;   0:9052     word   APM_CODE_16
;   0:9054     word   APM_DATA_16
;   0:907F     byte   number of BIOS hard disks
;   0:9080     Nbytes BIOS hard disks
;   0:9100     word   available physical memory map: number of blocks
;   0:9104            available physical memory map: blocks
;
;   Runtime:
;
; 0x00000000 -> 0x7FFFFFFF  application 2Gb

; 0x80000000 -> 0FFF  physical page zero - do not write
;                     (used by int 13h in some configurations)
;
; 0x80001000 -> 2FFF   window_data   - 256 entries
;
;         0000 dword  x start
;         0004 dword  y start
;         0008 dword  x size
;         000C dword  y size
;         0010 dword  color of work area
;         0014 dword  color of grab bar
;         0018 dword  color of frames
;         001C dword  window flags, +30 = window drawn, +31 redraw flag
;
;   3000  ->   4FFF   task list      - 256 entries
;
;         00   dword  process count
;         04   dword  no of processes
;         10   dword  base of running process at 0x3000+
;
;         20   dword  application event mask
;         24   dword  PID - process identification number
;         2a   byte   slot state: 0=running, 1,2=suspended
;                        3=zombie, 4=terminate,
;                        5=waiting for event, 9 = not used
;         2e   byte   window number on screen
;         30   dword  exact position in memory
;         34   dword  counter sum
;         38   dword  time stamp counter add
;         3c   dword  cpu usage in cpu timer tics
;
;
;   5000  ->   68FF   free (6k6)
;   6900  ->   6EFF   saved picture under mouse pointer (1k5)
;
;   6F00  ->   6FFF   free (256)
;
;   7000  ->   7FFF   used CD driver
;
;   8000  ->   A3FF   used FLOPPY driver
;
;   A400  ->   B0FF   free (3k3), unused ACTIVE_PROC_STACK

;   B100  ->   B307   IDT for int_0x00..int_0x40

;   B308  ->   BFFF   free (3k3)

;   C000  ->   C3FF   window stack C000 no of windows - all in words
;   C402  ->   C7FF   window position in stack
;   D000  ->   D1FF   FDC controller
;   D200  ->   D3FF   FDC controller for Fat12
;   D400  ->   DFFF   free (3k)
;   E000  byte        multitasking started
;   E020  dword       putpixel address
;   E024  dword       getpixel address
;   E030  dword       Vesa 1.2 pm bank switch address
;   E034  ->   F1FF   free (4k5)
;   F200  dword       mousepicture -pointer
;   F204  dword       mouse appearance counter
;   F208  ->   F2FF   free (248)
;   F300  dword       x & y temp for windowmove
;   F304  ->   F3FF   free (252)
;   F400  byte        no of keys in buffer
;   F401  byte        'buffer'
;   F402  ->   F4FF   reserved for keys
;   F500  byte        no of buttons in buffer
;   F501  dword       'buffer'
;   F502  ->   F5FF   reserved for buttons
;   F600  dword       tsc / second
;   F604  byte        (unused?) mouse port: 1 ps2, 2 com1, 3 com2
;   F605  ->   FAFF   free (1k2)
;   FB00  ->   FB0F   mouse memory 00 chunk count, that includes:
;   FB08 word       -- mouse H-scroll
;   FB0A word       -- mouse x 
;   FB0C word         -- mouse y
;   FB0E word       -- mouse V-scroll
;   FB10  ->   FB17   mouse color mem
;   FB21              x move
;   FB22              y move
;   FB28              high bits temp
;   FB30              color temp
;   FB40  byte        buttons down
;   FB44  byte        0 mouse down -> do not draw
;   FB4A  ->   FB4D   FB4A-B x-under - FB4C-D y-under
;   FBF1  byte        bits per pixel
;   FC00  ->   FCFE   com1/ps2 buffer
;   FCFF              com1/ps2 buffer count starting from FC00
;   FD00  ->   FDFF   free (256)
;   FE00  dword       screen x size             
;   FE04  dword       screen y size             
;   FE08  dword       screen y multiplier       
;   FE0C  dword       screen mode       
;   FE10  ->   FE7F   free (112)
;   FE80  dword       address of LFB in physical
;   FE84  dword       address of applications memory start in physical  ?
;   FE88  dword       address of button list
;   FE8C  dword       memory to use 
;   FE90  ->   FEFF   free (112)
;   FF00  byte        1 = system shutdown request
;   FF01  byte        task activation request?
;   FFF0  byte        >0 if redraw background request from app
;   FFF1  byte        free
;   FFF2              write and read bank in screen
;   FFF4  byte        0 if first mouse draw & do not return picture under
;   FFF5  byte        1 do not draw pointer
;   FFFF  byte        do not change task for 1/100 sec.
;
; 0x80010000 ->  6CBFF   kernel, 32-bit run-time code (up to 371 Kb)

; 0x8006CC00 ->  6DBFF   stack at boot time (4Kb)
;
; 0x8006DC00 ->  6E5FF   free (2560)
; 0x8006E600 ->  6Efff   free (2560)
; 0x8006F000 ->  6FFFF   main page directory

; 0x80070000 ->  7FFFF   data of retrieved disks and partitions (Mario79)
; 0x80080000 ->  8FFFF   additional app info, in 256 byte steps - 256 entries
;
;         00  11db  name of app running
;       0x10 dword  pointer to  fpu save area
;       0x14 dword  event count
;       0x18 dword  user fpu exceptoins handler
;       0x1c dword  user sse exceptions handler
;         20 dword  PL0 stack base
;         24 dword  user heap base
;         28 dword  user heap top
;         2c dword  window cursor handle
;         30 dword  first event in list
;         34 dword  last event in list
;         38 dword  first kernel object in list
;         3c dword  last kernel object in list
;         40 dword  thread esp
;         44 dword  io permission map page 0
;         48 dword  io permission map page 1
;         4c dword  debug state: 1= load debug registers
;         50 dword  current directory ptr
;         54 dword  wait timeout
;         58 dword  thread TSS._esp0 (= pl0 stack base + size except for V86)
;         5C-7F     unused
;
;         80 dword  address of random shaped window area
;         84 byte   shape area scale
;         88 dword  free
;         8C dword  application memory size
;         90 dword  window X position save
;         94 dword  window Y position save
;         98 dword  window X size save
;         9C dword  window Y size save
;         A0 dword  IPC memory start
;         A4 dword  IPC memory size
;         A8 dword  event bits: mouse, stack,..
;         AC dword  0 or debugger slot
;         B0 dword  free
;         B4  byte  keyboard mode: 0 = keymap, 1 = scancodes
;         B8 dword  physical address of directory table
;         BC dword  address of debug event memory
;         C0  5 dd  thread debug registers: DR0,DR1,DR2,DR3,DR7
;
; 0x80090000 ->  9FFFF   tmp (64k) - unused?
; 0x800A0000 ->  AFFFF   screen access area
; 0x800B0000 ->  FFFFF   bios rest in peace -area (320k)        ?
; 0x80100000 -> 27FFFF   diskette image (1m5)
; 0x80280000 -> 283FFF   free (16k)
;
; 0x80284000 -> 28BFFF   HDD DMA AREA   (32k)
; 0x8028C000 -> 297FFF   free (48k)
;
; 0x80298000 -> 29FFFF   auxiliary table for background smoothing code (32k)
;
; 0x802A0000 -> 2B00FF   wav device buffer (64k)
; 0x802A0000 -> 2B00FF   wav device status (256)
;
; 0x802B0100 -> 2B3FFD   free (15k7)
;
; 0x802B3FEE -> 2B3FEF   button info (64K+ 16 + 2 byte)
;     2B3FEE   0000 word    number of buttons
;     2B3FF0    first button entry
;      
;        button entry at 0x10
;       +0000 word   process number
;       +0002 word   button id number : bits 00-15
;       +0004 word   x start
;       +0006 word   x size
;       +0008 word   y start
;       +000A word   y size
;       +000C word   button id number : bits 16-31
;
; 0x802C4000 -> 2C9FFF   area for fast getting offset to LFB (24k)
;                        BPSLine_calc_area
; 0x802CA000 -> 2CFFFF   area for fast getting offset to _WinMapAddress (24k)
;                        d_width_calc_area
;
; 0x802D0000 -> 2DFFFF   reserved port area (64k)
;
;       0000 dword   no of port areas reserved
;       0010 dword   process id
;            dword   start port
;            dword   end port
;            dword   0
;
; 0x802E0000 -> 2EFFFF   irq data area  (64k)  ;BOOT_VAR
;
; 0x802F0000 -> 2F3FFF   tcp memory  stack_data_start eth_data_start (16k)
;
; 0x802F4000 -> 30ffff   stack_data | stack_data_end (112k)
;
; 0x80310000 -> 317fff   resendQ (32k)
;
; 0x80318000 -> 31ffff   skin_data (32k)
;
; 0x80320000 -> 323FF3   draw data     - 256 entries (4k)
;         00   dword  draw limit - x start
;         04   dword  draw limit - y start
;         08   dword  draw limit - x end
;         0C   dword  draw limit - y end
;
; 0x8032BFF4 -> 32BFFF   background info 
;         0x80323FF4    BgrDrawMode
;         0x80323FF8    BgrDataWidth
;         0x80323FFC    BgrDataHeight
;
; 0x80324000             page map     (length b = memsize shr 15)
; 0x80324000 + b         start of static pagetables

; 0x803FFFFF <- no direct address translation beyond this point
; ============================================================= 

; 0x805FF000 -> 5FFF80   TSS  
; 0x80600000 -> 601FFF   i/o maps

; 0x80800000 ->       kernel heap
; 0x80FFFFFF          heap min limit
; 0xFDBFFFFF          heap max limit

; 0xF0000000 -> 0xF1FFFFFF  PCI-express extended config space
; 0xFDC00000 -> 0xFDFFFFFF  page tables 4Mb
; 0xFE000000 -> 0xFFFFFFFF  LFB 32Mb
; 0xFE000000 -> 0xFE7FFFFF  application available LFB 8Mb
; 0xFE800000 -> 0xFFFFFFFF  kernel LFB part 24 Mb



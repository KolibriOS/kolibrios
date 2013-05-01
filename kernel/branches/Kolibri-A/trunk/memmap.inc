;
;   MEMORY MAP
;
;   Boot:
;
;  0:7C00->7CFF   boot_code
;
;   0:9000     byte   bits per pixel
;   0:9001     word   scanline length
;   0:9008     word   vesa video mode
;   0:900A     word   X res
;   0:900C     word   Y res
;   0:9010     byte   mouse port  - not used
;   0:9014     dword  Vesa 1.2 pm bank switch
;   0:9018     dword  Vesa 2.0 LFB address
;   0:901C     byte   0 or 1 : enable MTRR graphics acceleration
;   0:901D     byte   not used anymore (0 or 1 : enable system log display)
;   0:901E     byte   0 or 1 : enable direct lfb write, paging disabled
;   0:901F     byte   DMA write : 1=yes, 2=no
;   0:9020     8bytes pci data
;   0:9030     byte   VRR start enabled 1, 2-no
;   0:9031     word   IDEContrRegsBaseAddr
;    0x9040 - dword - entry point of APM BIOS
;    0x9044 -  word - version (BCD)
;    0x9046 -  word - flags
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
;   5000  ->   73FF   << FLOPPY buffer (9k)                             - fs/fat12.inc

;   7400  ->   78ff   << IDT for int_0x00..int_0x40 + reserv

;   7900  ->   7EFF   << saved picture under mouse pointer (1k5)
;
;   7F00  ->   7FFF   << SB16_Status (256)
;
; XX  8000  ->   BFFF   free (16k)  XX
; 0x80008000 -> 9FFF   ramdisk fat    (8k)
; 0x8000A000 -> BFFF   floppy  fat    (8k)

;   C000  ->   C3FF   window stack C000 no of windows - all in words
;   C402  ->   C7FF   window position in stack
;   D000  ->   D1FF   FDC controller
;   D200  ->   D3FF   FDC controller for Fat12
;                     - D201..D2A1 cleaned in fdc.init (fs/fdc.inc) 
;   D400  ->   DFFF   free (3k)

;   E000  ->   F9FF   free (6k5)

;   FA00  byte        no of keys in buffer
;   FA01  byte        'buffer'
;   FA02  ->   FAFF   reserved for keys

;   FB00  byte        no of buttons in buffer
;   FB01  dword       'buffer'
;   FB02  ->   FBFF   reserved for buttons


;   FC00  ->   FCFE   com1/ps2 buffer
;   FCFF              com1/ps2 buffer count starting from FC00

;   FD00  ->   FDFF   free (256)

;   FE00  ->   FE0F   mouse memory 00 chunk count, that includes:
;   FE08 word       -- mouse H-scroll
;   FE0A word       -- mouse x
;   FE0C word         -- mouse y
;   FE0E word       -- mouse V-scroll

;   FE10  ->   FB17   mouse color mem
;   FE21              x move
;   FE22              y move
;   FE28              high bits temp
;   FE30              color temp
;   FE40  byte        buttons down
;   FE44  byte        0 mouse down -> do not draw
;   FE4A  ->   FE4D   FE4A-B x-under - FE4C-D y-under

;   FEB8  dword       putpixel address
;   FEBC  dword       getpixel address
;   FEC1  byte        bits per pixel
;   FEC4  dword       mouse appearance counter
;   FEC8  dword       x & y temp for windowmove
;   FECC  dword       tsc / second

;   FED0  dword       screen x size
;   FED4  dword       screen y size
;   FED8  dword       screen y multiplier
;   FEDC  dword       screen mode

;   FEE4  dword       PCIe extended (memory-mappable) config space - physical address
;   FEE8  dword       PCIe extended config space limit
;   FEEC  word        number of 4M-pages needed to map PCIe config space
;   FEEE  word        PCIe bus range (power-ow-two Mbytes)

;   FEF0  dword       physical address of user-accessible static system buffer
;   FEF4  dword       physical address of LFB
;   FEF8  dword       address of button list
;   FEFC  dword       memory to use
;   FF00  byte        1 = system shutdown request
;   FF01  byte        task activation request?

;   FFE4  ->   FFEF   background info
;   FFF0  byte        >0 if redraw background request from app
;   FFF1  byte        >0 if background changed
;   FFF2              write and read bank in screen
;   FFF4  byte        0 if first mouse draw & do not return picture under
;   FFF5  byte        1 do not draw pointer
;   FFFF  byte        do not change task for 1/100 sec.
;
; 0x80010000 ->  6CBFF   the kernel (up to 371 Kb)
;       in the current version:
;            ->  00B3C   16-bit code end
;            ->  00C40   16-bit data end
;            ->  105F0   32-bit code start
;            ->  2C889   32-bit code end
;            ->  2FC28..end_of_kernel    zero-filled zone after preinit_mem
;            ->  33563   uninitialized globals start
;            ->  3B752   end_of_kernel
;            ->  3C000   not used (>200k)
; 0x80050000 -> 090000   zero-filled zone after preinit_mem
; 0x8006CC00 ->  6DBFF   stack at boot time (4Kb)
;
; 0x8006DC00 ->  6E5FF   basic text font II
; 0x8006E600 ->  6Efff   basic text font I
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
; 0x80090000 ->  9FFFF   wav device buffer (64k)        ; << SB16Buffer
; 0x800A0000 ->  AFFFF   screen access area               << VGA legacy buffer
; 0x800B0000 ->  FFFFF   bios rest in peace -area (320k), << purpose unknown
; 0x80100000 -> 283FFF   diskette image (1m5)

; 0x80284000 -> HEAP_BASE zero-filled zone after preinit_mem
;
; 0x80284000 -> 28BFFF   HDD DMA AREA   (32k)
; 0x8028C000 -> 297FFF   free (48k)
;
; 0x80298000 -> 29ffff   auxiliary table for background smoothing code (32k)
;
; 0x802A0000 -> 2Bffff   free (128k)
; 0x80328000 -> 32FFFF   !vrr driver      (32k)


; 0x80338000 -> 338FFF draw data     - 256 entries (4k)
;         00   dword  draw limit - x start
;         04   dword  draw limit - y start
;         08   dword  draw limit - x end
;         0C   dword  draw limit - y end
; 0x80339000 -> 3BFFF3   free (12k)

; 0x802C0000 -> 2C3fff   button info (8k)
;
;       0000 word    number of buttons
;        first button entry at 0x10
;       +0000 word   process number
;       +0002 word   button id number : bits 00-15
;       +0004 word   x start
;       +0006 word   x size
;       +0008 word   y start
;       +000A word   y size
;       +000C word   button id number : bits 16-31
;
; 0x802C4000 -> 2CFFFF   free (48k)
;
; 0x802D0000 -> 2DFFFF   reserved port area (64k)
;
;       0000 dword   no of port areas reserved
;       0010 dword   process id
;            dword   start port
;            dword   end port
;            dword   0
;
; 0x80370000 -> 37FFFF   irq data area   (64k)
; 0x80380000 -> 38FFFF   low memory save (64k)
;
; 0x80390000 -> 3AFFFF   tcp memory       (128k)
; 0x803B0000 -> 3B7FFF   tcp memory       (32k)

; 0x803B8000 -> 3BFFFF   skin data              (32k)

; 0x803C0000             page map     (length b = memsize shr 15)
; 0x803C0000 + b         start of static pagetables

; 0x803FFFFF <- no direct address translation beyond this point
; =============================================================

; 0x805FF000 -> 5FFF80   TSS
; 0x80600000 -> 601FFF   i/o maps

; 0x80800000 ->827FFFFF  kernel heap

; 0xF0000000 -> 0xF1FFFFFF  PCI-express extended config space
; 0xFD000000 -> 0xFD3FFFFF  static system buffer 4Mb
; 0xFDC00000 -> 0xFDFFFFFF  page tables 4Mb
; 0xFE000000 -> 0xFFFFFFFF  LFB 32Mb
; 0xFE000000 -> 0xFE7FFFFF  application available LFB 8Mb
; 0xFE800000 -> 0xFFFFFFFF  kernel LFB part 24 Mb



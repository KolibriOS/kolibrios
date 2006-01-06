;
;   MEMORY MAP
;
;   Boot:
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
;   0:9020     8bytes pci data
;   0:9030     byte   VRR start enabled 1, 2-no
;   0:9031     word   IDEContrRegsBaseAddr
;
;
;   Runtime:
;
;   0000  ->   1FFF   window_data   - 256 entries
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
;   2000  ->   2FFF   free
;
;   3000  ->   4FFF   task list      - 256 entries
;
;         00   dword  process count
;         04   dword  no of processes
;         10   dword  base of running process at 0x3000+
;
;         20   dword  application event mask
;         24   dword  PID - process identification number
;         2a   word   0, state 3=zombie, 4=terminate
;         2e   byte   window number on screen
;         30   dword  exact position in memory
;         34   dword  counter sum
;         38   dword  time stamp counter add
;         3c   dword  cpu usage in cpu timer tics
;
;
;   5000  ->   5FFF   save_syscall_data - syscall trace
;   6000  ->   68FF   free
;   6900  ->   6EFF   saved picture under mouse pointer
;
;   6F00  ->   6FFF   free
;
;   7000  ->   7FFF   used CD driver
;
;   8000  ->   A3FF   used FLOPPY driver
;
;   A400  ->   A7FF   used active process stack
;
;   A800  ->   B0FF   free

;   B100  ->   B2FF   IDT

;   B300  ->   BFFF   free

;   C000  ->   C3FF   window stack C000 no of windows - all in words
;   C402  ->   C7FF   window position in stack
;   D000  ->   D1FF   FDC controller
;   D200  ->   D3FF   FDC controller for Fat12
;   D400  ->   DFFF   free
;   E000  byte        multitasking started
;   E020  dword       putpixel address
;   E024  dword       getpixel address
;   E030  dword       Vesa 1.2 pm bank switch address
;   F200  dword       mousepicture -pointer
;   F204  dword       mouse appearance counter
;   F300  dword       x & y temp for windowmove
;   F400  byte        no of keys in buffer
;   F401  byte        'buffer'
;   F402  ->   F4FF   reserved for keys
;   F500  byte        no of buttons in buffer
;   F501  dword       'buffer'
;   F502  ->   F5FF   reserved for buttons
;   F600  dword       tsc / second
;   F604  byte        mouse port: 1 ps2, 2 com1, 3 com2
;   FB00  ->   FB0F   mouse memory 00 chunk count - FB0A-B x - FB0C-D y
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
;   FE00  dword       screen x size
;   FE04  dword       screen y size
;   FE08  dword       screen y multiplier
;   FE0C  dword       screen mode
;   FE10  dword       entries in hd cache
;   FE80  dword       address of LFB in physical
;   FE84  dword       address of applications memory start in physical
;   FE88  dword       address of button list
;   FE8C  dword       memory to use
;   FF00  byte        1 = system shutdown request
;   FF01  dword       if >1 then activate process
;   FFF0  byte        1 = redraw background request from app
;   FFF1  byte        1 = diskette int occur
;   FFF2              write and read bank in screen
;   FFF4  byte        0 if first mouse draw & do not return picture under
;   FFF5  byte        1 do not draw pointer
;   FFFF  byte        do not change task for 1/100 sec.
;
;  10000  ->  3DBFF   kernel, 32-bit run-time code (up to 183 Kb)
;  3DC00  ->  3EBFF   stack at boot time (4Kb)
;  3EC00  ->  3F5FF   basic text font II
;  3F600  ->  3FFFF   basic text font I
;  40000  ->  4FFFF   data of retrieved disks and partitions (Mario79)

;  50000  ->  5FFFF   free (64 Kb)

;  60000  ->  7FFFF   paging tables
;  80000  ->  8FFFF   additional app info, in 256 byte steps - 256 entries
;
;           00  11db  name of app running
;           10 108db  floating point unit save area
;           7f byte   0= no fpu saved ,  1= fpu saved to 0x10 -> restore
;           80 dword  address of random shaped window area
;           84 byte   shape area scale
;           88 dword  used GDT of TSS
;           8C dword  application memory size
;           90 dword  window X position save
;           94 dword  window Y position save
;           98 dword  window X size save
;           9C dword  window Y size save
;           A0 dword  IPC memory start
;           A4 dword  IPC memory size
;           A8 dword  event bits: mouse, stack,..
;
;           B0 dword  int40 handler in use ; 0 if not in use
;           B4  byte  keyboard mode: 0 = keymap, 1 = scancodes
;           B8 dword  physical address of directory table
;
;  90000  ->  9FFFF   tmp
;  A0000  ->  AFFFF   screen access area
;  B0000  ->  FFFFF   bios rest in peace -area
; 100000  -> 27FFFF   diskette image
; 280000  -> 281FFF   ramdisk fat
; 282000  -> 283FFF   floppy  fat
;
; 284000  -> 29FFFF   free (112 Kb)
;
; 2A0000  -> 2B00ff   wav device data
; 2C0000  -> 2C3fff   button info
;
;        0000 word    number of buttons
;        first button entry at 0x10
;        +0000 word   process number
;        +0002 word   button id number : bits 00-15
;        +0004 word   x start
;        +0006 word   x size
;        +0008 word   y start
;        +000A word   y size
;        +000C word   button id number : bits 16-31
;
; 2C4000  -> 2CFFFF   free (48Kb)
;
; 2D0000  -> 2DFFFF   reserved port area
;
;     0000   dword    no of port areas reserved
;     0010   dword    process id
;            dword    start port
;            dword    end port
;            dword    0
;
; 2E0000  -> 2EFFFF   irq data area
; 2F0000  -> 2FFFFF   low memory save
;
; 300000  -> 45FFFF   background image, max 1,375 M
;
; 460000  -> 5FFFFF   display info
;
; 600000  -> 6FFFFF   hd cache
;
; 700000  -> 71ffff   tcp memory       (128 kb)
; 720000  -> 75ffff   free (256 kb)
;
; 760000  -> 76ffff   !vrr driver
; 770000  -> 777fff   tcp memory       ( 32 kb)
;
; 778000  -> 77ffff   window skinning  ( 32 kb)
; 780000  -> 7fffff   free (512 Kb)
;
; 800000  -> BFFFFF   mapped to LFB
;
;
; C00000  -> C01FFF   draw_data   - 256 entries
;
;         00   dword  draw limit - x start
;         04   dword  draw limit - y start
;         08   dword  draw limit - x end
;         0C   dword  draw limit - y end
;
; C02000  -> C02fff   free (4 Kb)
;
; C03000  -> D02fff   sysint_stack_data
;                     - ring0 stacks for ring3 processes
;                     - used for interrupt handling
;                     - 256 entries * 4096 step
;
; D03000  -> D1ffff   free (116 Kb)
;
; D20000  -> F28000   TSS and IO map for (8192*8)=65536 ports
;                     (128+8192)*256 = 557956 = 0x88000
;
; 1000000 -> 3FFFFFF  for applications
;






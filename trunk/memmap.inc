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
;   0000  ->   2FFF   window_data   - 256 entries
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
;         2a   word   0, state 3=zombie, 4=terminate
;         2e   byte   window number on screen
;         30   dword  exact position in memory
;         34   dword  counter sum
;         38   dword  time stamp counter add
;         3c   dword  cpu usage in cpu timer tics
;
; -> delete ->   5000  ->   AFFF   free
;
;!!!
;   5000  ->   5FFF   save_syscall_data - syscall trace
;   6000  ->   63FF   reg1 array - for i40 (first parts of saved TSS descriptor)
;   6400  ->   67FF   reg2 array - same thing, second part
;   6800  ->   68FF   used i40 (1 if used, 0 if free), first element not used
;   6900  ->   6EFF   saved picture under mouse pointer
;
;   6F00  ->   7FFF   i38 stack (4,25Kb)
;
;   8000  ->   A3FF   used FLOPPY driver
;
;   A400  ->   AFFF   free
;
;   B000  ->   B005   jump address for irq0 (task switch)
;   B008  ->   B00B   count of ticks remaining to updating CPU usage info
;
;   B060  ->   B07F   fpu error handler's stack
;   B080  ->   B0FF   fpu_tss -> temporary TSS for FPU context switching
;   B100  ->   B2FF   IDT
;   B300  ->   BAFF   tasknum array - for i40 handler
;   BB00  ->   BEFF   free
;!!!

;   BF00  ->   BFFF   bytes, 1 if process running/memory in use
;   C000  ->   C3FF   window stack C000 no of windows - all in words
;   C402  ->   C7FF   window position in stack
;   D000  ->   D1FF   FDC controller
;   D200  ->   D3FF   FDC controller for Fat12
;   D400  ->   DFFF   free
;   E000  byte        multitasking started
;   E004  dword       save for boot mem 0x24
;   E010  dword       pm bank switch address
;   E014  dword       window multiplier
;   E020  dword       putpixel address
;   E024  dword       getpixel address
;   E030  dword       Vesa 1.2 pm bank switch address
;   F000  dword       hd partition 1 start
;   F004  dword       root start at partition 1
;   F008  dword       root start at hd
;   F100  byte        1=only os draw on screen
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
;   FBF0  byte        multiplied window
;   FBF1  byte        bits per pixel
;   FC00  ->   FCFE   com1/ps2 buffer
;   FCFF              com1/ps2 buffer count starting from FC00
;   FDF0  ->   FDF3   timer tick inc every 1/100 sec 4 bytes
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
;  10000  ->  1FFFF   32 bit os
;  20000  ->  2FFFF   32 bit stack
;  30000  ->  36FFF   basic text font II
;  37000  ->  3BFFF   basic text font I
;  40000  ->  4FFFF   data of retrieved disks and partitions (Mario79)
;  50000  ->  500FF   TSS of scheduler
;  51000  ->  54000   esp0,esp1,esp2
;  5F000  ->  5FFFF   schedulers stack
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
;
;  90000  ->  9FFFF   tmp
;  A0000  ->  AFFFF   screen access area
;  B0000  ->  FFFFF   bios rest in peace -area
; 100000  -> 27FFFF   diskette image
; 280000  -> 281FFF   ramdisk fat
; 282000  -> 283FFF   floppy  fat
;
; 284000  -> 28FFFF   free
;
; 290000  -> 297FFF   TSS's of interrupts
; 298000  -> 29FFFF   TSS's of SYSTEM interrupts - 256 entries
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
; 300000  -> 45FFFF   background image, max 1 M
;
; 460000  -> 5FFFFF   display info
;
; 600000  -> 6FFFFF   hd cache
;
; 700000  -> 71ffff   tcp memory       (128 kb)
; 720000  -> 75ffff   interrupt stacks (256 kpl * 1024)
;
; 760000  -> 76ffff   !vrr driver
; 770000  -> 777fff   tcp memory       ( 32 kb)
;
; 778000  -> 77ffff   window skinning  ( 32 kb)
; 780000  -> 7fffff   free
;
; 800000  -> BFFFFF   mapped to LFB
;
; C00000  -> DFFFFF * Application information
;
; C00000  -> C01FFF   draw_data   - 256 entries
;
;         00   dword  draw limit - x start
;         04   dword  draw limit - y start
;         08   dword  draw limit - x end
;         0C   dword  draw limit - y end
;         1C   dword  app int vector
;
; C03000  -> D11fff   sysint_stack_data
;                     - system interrupt stacks
;                     - 256 entries * 4096 step
;
; D20000  -> DA8000   TSS and IO map for (2048*8)=16384 ports
;                     (128+2048)*256 = 557956 = 0x88000
;
; 1000000 -> 3FFFFFF  for applications
;






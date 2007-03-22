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
;   0:901F     byte   DMA write : 1=yes, 2=no
;   0:9020     8bytes pci data
;   0:9030     byte   VRR start enabled 1, 2-no
;   0:9031     word   IDEContrRegsBaseAddr
;    0x9040 - dword - entry point of APM BIOS
;    0x9044 -  word - version (BCD)
;    0x9046 -  word - flags
;
;   Runtime:
;
; 0x00000000 -> 0x7FFFFFFF  application 2Gb

; 0x80000000 -> 1FFF   window_data   - 256 entries
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
;   5000  ->   68FF   free
;   6900  ->   6EFF   saved picture under mouse pointer
;
;   6F00  ->   6FFF   free
;
;   7000  ->   7FFF   used CD driver
;
;   8000  ->   A3FF   used FLOPPY driver
;
;   A400  ->   B0FF   free

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
;   FE80  dword       address of LFB in physical
;   FE84  dword       address of applications memory start in physical
;   FE88  dword       address of button list
;   FE8C  dword       memory to use
;   FF00  byte        1 = system shutdown request
;   FF01  dword       free
;   FFF0  byte        1 = redraw background request from app
;   FFF1  byte        1 = diskette int occur
;   FFF2              write and read bank in screen
;   FFF4  byte        0 if first mouse draw & do not return picture under
;   FFF5  byte        1 do not draw pointer
;   FFFF  byte        do not change task for 1/100 sec.
;
; 0x80010000 ->  3DBFF   kernel, 32-bit run-time code (up to 183 Kb)
; 0x8003DC00 ->  3EBFF   stack at boot time (4Kb)
; 0x8003EC00 ->  3F5FF   basic text font II
; 0x8003F600 ->  3FFFF   basic text font I
; 0x80040000 ->  4FFFF   data of retrieved disks and partitions (Mario79)

; 0x80050000 ->  50FFF   main page directory
; 0x80050200 ->  5FFFF   pages bitmap

; 0x80060000 ->  7FFFF   free (128 Kb)
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
;
;         40-7F     unused
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
; 0x80090000 ->  9FFFF   tmp
; 0x800A0000 ->  AFFFF   screen access area
; 0x800B0000 ->  FFFFF   bios rest in peace -area
; 0x80100000 -> 27FFFF   diskette image
; 0x80280000 -> 281FFF   ramdisk fat
; 0x80282000 -> 283FFF   floppy  fat
;
; 0x80284000 -> 29FFFF   free (112 Kb)
;
; 0x802A0000 -> 2B00ff   wav device data
; 0x802C0000 -> 2C3fff   button info
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
; 0x802C4000 -> 2CFFFF   free (48Kb)
;
; 0x802D0000 -> 2DFFFF   reserved port area
;
;       0000 dword   no of port areas reserved
;       0010 dword   process id
;            dword   start port
;            dword   end port
;            dword   0
;
; 0x802E0000 -> 2EFFFF   irq data area
; 0x802F0000 -> 2FFFFF   low memory save
;
; 0x80300000 -> 45FFFF   background image, max 1,375 M
;
; 0x80460000 -> 5FFFFF   display info
;
; 0x80600000 -> 6FFFFF   hd cache
;
; 0x80700000 -> 71ffff   tcp memory       (128 kb)
; 0x80720000 -> 75ffff   free (256 kb)
;
; 0x80760000 -> 76ffff   !vrr driver
; 0x80770000 -> 777fff   tcp memory       ( 32 kb)
;
; 0x80780000 -> 0x80987FFF TSS and IO map for (8192*8)=65536 ports
;                          (128+8192)*256 = 2129920 = 0x208000
;
; 0x80988000 -> 0x8098AFFF draw_data   - 256 entries
;
;         00   dword  draw limit - x start
;         04   dword  draw limit - y start
;         08   dword  draw limit - x end
;         0C   dword  draw limit - y end
;
;
; 0x8098B000 ->       kernel heap
; 0x81FFFFFF          heap min limit
; 0xFDBFFFFF          heap max limit

; 0xFDC00000 -> 0xFDFFFFFF  page tables 4Mb
; 0xFE000000 -> 0xFFFFFFFF  LFB 32Mb
; 0xFE000000 -> 0xFE7FFFFF  application available LFB 8Mb
; 0xFE800000 -> 0xFFFFFFFF  kernel LFB part 24 Mb

if 0

Эта бадяга с копирайтами уже задолбала. Приходится делать всё самому
Вот проект заголовков.

Для сравнения я брал исходники MeOS 079p1

Если файл попал в один из списков ошибочно, укажите.

Список разработчиков ядра KolibriOS взят из последнего дистрибутива.
Если кого-то забыли, укажите

Через две недели я проставлю заголовки на все перечисленные файлы.

kernel.asm

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Copyright (C) KolibriOS team 2004-2007.  All rights reserved.
;;   PROGRAMMING:
;;     Иван Поддубный
;;     Марат Закиянов (Mario79)
;;     VaStaNi
;;     Trans
;;     Михаил Семеняко (mike.dld)
;;     Сергей Кузьмин (Wildwest)
;;     Халявин Андрей (halyavin)
;;     Михаил Лисовин (Mihasik)
;;     Андрей Игнатьев (andrew_programmer)
;;     NoName
;;     Евгений Гречников (Diamond)
;;     Михайлов Илья  (Ghost)
;;     Сергей Семёнов (Serge)
;;     Johnny_B
;;
;; Data in this file was originally part of MenuetOS project which is
;; distributed under the terms of GNU GPL. It is modified and redistributed as
;; part of KolibriOS project under the terms of GNU GPL.
;;
;; Copyright (C) MenuetOS  2000-2004 Ville Mikael Turjanmaa
;;   PROGRAMMING:
;;
;;     Ville Mikael Turjanmaa, villemt@itu.jyu.fi
;;      - main os coding/design
;;     Jan-Michael Brummer, BUZZ2@gmx.de
;;     Felix Kaiser, info@felix-kaiser.de
;;     Paolo Minazzi, paolo.minazzi@inwind.it
;;     quickcode@mail.ru
;;     Alexey, kgaz@crosswinds.net
;;     Juan M. Caravaca, bitrider@wanadoo.es
;;     kristol@nic.fi
;;     Mike Hibbett, mikeh@oceanfree.net
;;     Lasse Kuusijarvi, kuusijar@lut.fi
;;     Jarek Pelczar, jarekp3@wp.pl
;;
;; KolibriOS is distributed in the hope that it will be useful, but WITHOUT ANY
;; WARRANTY. No author or distributor accepts responsibility to anyone for the
;; consequences of using it or for whether it serves any particular purpose or
;; works at all, unless he says so in writing. Refer to the GNU General Public
;; License (the "GPL") for full details.
;
;; Everyone is granted permission to copy, modify and redistribute KolibriOS,
;; but only under the conditions described in the GPL. A copy of this license
;; is supposed to have been given to you along with KolibriOS so you can know
;; your rights and responsibilities. It should be in a file named COPYING.
;; Among other things, the copyright notice and this notice must be preserved
;; on all copies.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



На этих файлах уже стоят различные копирайты
Будет добавлен копирайт KolibriOS Team

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;   Copyright (C) KolibriOS team 2004-2007.  All rights reserved.
;;
;;   Distributed under terms of the GNU General Public License
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

fdo.inc
kernel16.inc
kernel32.inc
vmodeld.inc
vmodeint.inc

blkdev/cd_drv.inc
blkdev/flp_drv.inc
blkdev/hd_drv.inc
blkdev/rd.inc

boot/shutdown.inc

bus/pci/pci16.inc
bus/pci/pci32.inc

core/sync.inc
core/sys32.inc

detect/commouse.inc
detect/dev_fd.inc
detect/dev_hdcd.inc
detect/sear_par.inc

fs/fat12.inc
fs/fat32.inc
fs/fs.inc
fs/fs_lfn.inc
fs/part_set.inc

network/icmp.inc
network/ip.inc
network/queue.inc
network/socket.inc
network/stack.inc
network/tcp.inc
network/udp.inc

network/eth_drv/arp.inc
network/eth_drv/ethernet.inc
network/eth_drv/drivers/3c59x.inc
network/eth_drv/drivers/i8255x.inc
network/eth_drv/drivers/pcnet32.inc
network/eth_drv/drivers/rtl8029.inc
network/eth_drv/drivers/rtl8139.inc
network/eth_drv/drivers/rtl8169.inc
network/eth_drv/drivers/sis900.inc

sound/playnote.inc

video/vesa12.inc
video/vesa20.inc
video/vga.inc


Файлы без копирайтов с кодом из МеОС
Будут добавлены копирайты KolibriOS Team и
Menuet OS Team

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;   Copyright (C) KolibriOS team 2004-2007.  All rights reserved.
;;   Copyright (C) MenuetOS  2000-2004 Ville Mikael Turjanmaa
;;
;;   Distributed under terms of the GNU General Public License
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


blkdev/cdrom.inc
blkdev/fdc.inc

boot/bootcode.inc

core/sched.inc

gui/button.inc
gui/event.inc
gui/mouse.inc
gui/window.inc

hid/keyboard.inc

sound/sb16.inc


Фалы с копирайтом только KolibriOS Team

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;   Copyright (C) KolibriOS team 2004-2007.  All rights reserved.
;;
;;   Distributed under terms of the GNU General Public License
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

kglobals.inc
unpacker.inc
const.inc

blkdev/rdsave.inc

boot/preboot.inc
boot/rdload.inc

core/debug.inc
core/dll.inc
core/exports.inc
core/fpu.inc
core/heap.inc
core/malloc.inc
core/memory.inc
core/syscall.inc
core/taskman.inc

detect/disks.inc
detect/ps2mouse.inc

docs/apm.txt
docs/sysfuncr.txt
docs/sysfuncs.txt

drivers/ati2d.asm
drivers/ensoniq.asm
drivers/infinity.asm
drivers/mixer.asm
drivers/sceletone.asm
drivers/sis.asm
drivers/sound.asm
drivers/codec.inc
drivers/imports.inc
drivers/main.inc
drivers/mix_mmx.inc
drivers/mix_sse2.inc

fs/iso9660.inc
fs/ntfs.inc

gui/font.inc
gui/skincode.inc
gui/skindata.inc

hid/m_com1.inc
hid/m_com2.inc
hid/m_ps2.inc
hid/mousedrv.inc
hid/set_dtc.inc

network/eth_drv/pci.inc

skin/default.asm
skin/me_skin.inc

video/cursors.inc


файлы которые можно оставить без копирайта

memmap.inc

boot/booteng.inc
boot/bootet.inc
boot/bootge.inc
boot/bootru.inc
boot/et.inc
boot/ru.inc

end if


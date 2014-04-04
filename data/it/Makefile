# General rule for naming: variables with CAPITALIZED names hold settings,
# you can - and are expected - to modify it; variables with lowercase names
# are intermediate variables and macroses not to be modified unless you
# know what you're doing.

# Define directories for destination, source repository, sources of kernel, sources of programs.
BUILD_DIR:=build
REPOSITORY:=../..
KERNEL:=$(REPOSITORY)/kernel/trunk
PROGS:=$(REPOSITORY)/programs

# The main goal: build kolibri.img and kolibri.iso
all: $(BUILD_DIR)/kolibri.img $(BUILD_DIR)/kolibri.iso

# Docpak requires some documents; we place them
# into 'docs' subdir and communicate with FASM
# through environment var DOCDIR.
DOCDIR:=docs/
export DOCDIR

# Because most programs are written in FASM and have
# the common structure of one main .asm file possibly including
# several .inc files, we handle this case separately.
# Namely, we organize one big list of such programs
# (FASM_PROGRAMS, see below) containing name of local binary file,
# name of file inside kolibri.img and name of the source file.
# This list is maintained by hand, and the rest is done with some macroses...
# well, slightly complicated macroses - however, you do not need to
# understand them in order to maintain the list.
# To add a FASM program with one .asm file, just
# add the corresponding item to the list and enjoy
# the power of GNU make.

# The list of all FASM programs with one main FASM file.
# Every item consists of three parts, separated by ':'.
# First part is the real name of binary file in $(BUILD_DIR)
# as it should be produced by make.
# Second part is the name of a file inside kolibri.img,
# usually uppercased version of first part - to save space
# for FAT filesystem.
# Third part is the name of the source file.
# Spaces separate items, so spaces in names should be
# represented as '|'.
FASM_PROGRAMS:=\
 @menu:@MENU:$(PROGS)/system/menu/trunk/menu.asm \
 @panel:@PANEL:$(PROGS)/system/panel/trunk/@PANEL.ASM \
 @docky:@DOCKY:$(PROGS)/system/docky/trunk/docky.asm \
 @ss:@SS:$(PROGS)/system/ss/trunk/@ss.asm\
 refrscrn:REFRSCRN:$(PROGS)/system/refrscrn/refrscrn.asm \
 asciivju:ASCIIVJU:$(PROGS)/develop/asciivju/trunk/asciivju.asm \
 calc:CALC:$(PROGS)/other/calc/trunk/calc.asm \
 calendar:CALENDAR:$(PROGS)/system/calendar/trunk/calendar.asm \
 colrdial:COLRDIAL:$(PROGS)/system/colrdial/color_dialog.asm \
 loaddrv:LOADDRV:$(PROGS)/system/loaddrv/loaddrv.asm \
 cpu:CPU:$(PROGS)/system/cpu/trunk/cpu.asm \
 cpuid:CPUID:$(PROGS)/system/cpuid/trunk/CPUID.ASM \
 desktop:DESKTOP:$(PROGS)/system/desktop/trunk/desktop.asm \
 disptest:DISPTEST:$(PROGS)/system/disptest/trunk/disptest.ASM \
 docpack:DOCPACK:$(PROGS)/system/docpack/trunk/docpack.asm \
 end:END:$(PROGS)/system/end/light/end.asm \
 gmon:GMON:$(PROGS)/system/gmon/gmon.asm \
 hdd_info:HDD_INFO:$(PROGS)/system/hdd_info/trunk/hdd_info.asm \
 @icon:@ICON:$(PROGS)/system/icon_new/@icon.asm \
 cropflat:CROPFLAT:$(PROGS)/system/cropflat/cropflat.asm \
 kbd:KBD:$(PROGS)/system/kbd/trunk/kbd.ASM \
 kpack:KPACK:$(PROGS)/other/kpack/trunk/kpack.asm \
 launcher:LAUNCHER:$(PROGS)/system/launcher/trunk/launcher.asm \
 magnify:MAGNIFY:$(PROGS)/demos/magnify/trunk/magnify.asm \
 mgb:MGB:$(PROGS)/system/mgb/trunk/mgb.asm \
 mousemul:MOUSEMUL:$(PROGS)/system/mousemul/trunk/mousemul.asm \
 madmouse:MADMOUSE:$(PROGS)/other/madmouse/madmouse.asm \
 mykey:MYKEY:$(PROGS)/system/MyKey/trunk/MyKey.asm \
 pcidev:PCIDEV:$(PROGS)/system/pcidev/trunk/PCIDEV.ASM \
 rdsave:RDSAVE:$(PROGS)/system/rdsave/trunk/rdsave.asm \
 rtfread:RTFREAD:$(PROGS)/other/rtfread/trunk/rtfread.asm \
 run:RUN:$(PROGS)/system/run/trunk/run.asm \
 searchap:SEARCHAP:$(PROGS)/system/searchap/searchap.asm \
 scrshoot:SCRSHOOT:$(PROGS)/media/scrshoot/scrshoot.asm \
 setup:SETUP:$(PROGS)/system/setup/trunk/setup.asm \
 test:TEST:$(PROGS)/system/test/trunk/test.asm \
 tinypad:TINYPAD:$(PROGS)/develop/tinypad/trunk/tinypad.asm \
 zkey:ZKEY:$(PROGS)/system/zkey/trunk/ZKEY.ASM \
 terminal:TERMINAL:$(PROGS)/system/terminal/terminal.asm \
 3d/3dsheart:3D/3DSHEART:$(PROGS)/demos/3dsheart/trunk/3dsheart.asm \
 3d/3dwav:3D/3DWAV:$(PROGS)/demos/3dwav/trunk/3dwav.asm \
 3d/crownscr:3D/CROWNSCR:$(PROGS)/demos/crownscr/trunk/crownscr.asm  \
 3d/free3d04:3D/FREE3D04:$(PROGS)/demos/free3d04/trunk/free3d04.asm \
 3d/view3ds:3D/VIEW3DS:$(PROGS)/demos/3DS/VIEW3DS.ASM \
 demos/bcdclk:DEMOS/BCDCLK:$(PROGS)/demos/bcdclk/trunk/bcdclk.asm \
 demos/circle:DEMOS/CIRCLE:$(PROGS)/develop/examples/circle/trunk/circle.asm \
 demos/colorref:DEMOS/COLORREF:$(PROGS)/demos/colorref/trunk/colorref.asm \
 demos/cslide:DEMOS/CSLIDE:$(PROGS)/demos/cslide/trunk/cslide.asm \
 demos/eyes:DEMOS/EYES:$(PROGS)/demos/eyes/trunk/eyes.asm \
 demos/firework:DEMOS/FIREWORK:$(PROGS)/demos/firework/trunk/firework.asm \
 demos/movback:DEMOS/MOVBACK:$(PROGS)/demos/movback/trunk/movback.asm \
 demos/plasma:DEMOS/PLASMA:$(PROGS)/demos/plasma/trunk/plasma.asm \
 demos/tinyfrac:DEMOS/TINYFRAC:$(PROGS)/demos/tinyfrac/trunk/tinyfrac.asm \
 demos/trantest:DEMOS/TRANTEST:$(PROGS)/demos/trantest/trunk/trantest.asm \
 demos/tube:DEMOS/TUBE:$(PROGS)/demos/tube/trunk/tube.asm \
 demos/unvwater:DEMOS/UNVWATER:$(PROGS)/demos/unvwater/trunk/unvwater.asm \
 demos/use_mb:DEMOS/USE_MB:$(PROGS)/demos/use_mb/use_mb.asm \
 demos/web:DEMOS/WEB:$(PROGS)/demos/web/trunk/web.asm \
 develop/board:DEVELOP/BOARD:$(PROGS)/system/board/trunk/board.asm \
 develop/cObj:DEVELOP/cObj:$(PROGS)/develop/cObj/trunk/cObj.asm \
 develop/fasm:DEVELOP/FASM:$(PROGS)/develop/fasm/trunk/fasm.asm \
 develop/h2d2b:DEVELOP/H2D2B:$(PROGS)/develop/h2d2b/trunk/h2d2b.asm \
 develop/heed:DEVELOP/HEED:$(PROGS)/develop/heed/trunk/heed.asm \
 develop/keyascii:DEVELOP/KEYASCII:$(PROGS)/develop/keyascii/trunk/keyascii.asm \
 develop/mtdbg:DEVELOP/MTDBG:$(PROGS)/develop/mtdbg/mtdbg.asm \
 develop/scancode:DEVELOP/SCANCODE:$(PROGS)/develop/scancode/trunk/scancode.asm \
 develop/t_edit:DEVELOP/T_EDIT:$(PROGS)/other/t_edit/t_edit.asm \
 develop/test_gets:DEVELOP/test_gets:$(PROGS)/develop/libraries/console/examples/test_gets.asm \
 develop/testcon2:DEVELOP/TESTCON2:$(PROGS)/develop/libraries/console/examples/testcon2_eng.asm \
 develop/thread:DEVELOP/THREAD:$(PROGS)/develop/examples/thread/trunk/thread.asm \
 develop/info/asm.syn:DEVELOP/INFO/ASM.SYN:$(PROGS)/other/t_edit/info/asm_syn.asm \
 develop/info/cpp_kol_cla.syn:DEVELOP/INFO/CPP_KOL_CLA.SYN:$(PROGS)/other/t_edit/info/cpp_kol_cla_syn.asm \
 develop/info/cpp_kol_dar.syn:DEVELOP/INFO/CPP_KOL_DAR.SYN:$(PROGS)/other/t_edit/info/cpp_kol_dar_syn.asm \
 develop/info/cpp_kol_def.syn:DEVELOP/INFO/CPP_KOL_DEF.SYN:$(PROGS)/other/t_edit/info/cpp_kol_def_syn.asm \
 develop/info/default.syn:DEVELOP/INFO/DEFAULT.SYN:$(PROGS)/other/t_edit/info/default_syn.asm \
 develop/info/html.syn:DEVELOP/INFO/HTML.SYN:$(PROGS)/other/t_edit/info/html_syn.asm \
 develop/info/ini_files.syn:DEVELOP/INFO/INI_FILES.SYN:$(PROGS)/other/t_edit/info/ini_files_syn.asm \
 develop/info/win_const.syn:DEVELOP/INFO/WIN_CONST.SYN:$(PROGS)/other/t_edit/info/win_const_syn.asm \
 drivers/com_mouse.obj:DRIVERS/COM_MOUSE.OBJ:$(KERNEL)/drivers/com_mouse.asm \
 drivers/emu10k1x.obj:DRIVERS/EMU10K1X.OBJ:$(KERNEL)/drivers/emu10k1x.asm \
 drivers/fm801.obj:DRIVERS/FM801.OBJ:$(KERNEL)/drivers/fm801.asm \
 drivers/infinity.obj:DRIVERS/INFINITY.OBJ:$(KERNEL)/drivers/infinity.asm \
 drivers/intel_hda.obj:DRIVERS/intel_hda.obj:$(REPOSITORY)/drivers/audio/intel_hda/intel_hda.asm \
 drivers/ps2mouse.obj:DRIVERS/PS2MOUSE.OBJ:$(REPOSITORY)/drivers/mouse/ps2mouse4d/trunk/ps2mouse.asm \
 drivers/sb16.obj:DRIVERS/SB16.OBJ:$(KERNEL)/drivers/sb16/sb16.asm \
 drivers/sound.obj:DRIVERS/SOUND.OBJ:$(KERNEL)/drivers/sound.asm \
 drivers/intelac97.obj:DRIVERS/INTELAC97.OBJ:$(KERNEL)/drivers/intelac97.asm \
 drivers/tmpdisk.obj:DRIVERS/TMPDISK.OBJ:$(KERNEL)/drivers/tmpdisk.asm \
 drivers/usbhid.obj:DRIVERS/USBHID.OBJ:$(KERNEL)/drivers/usbhid/usbhid.asm \
 drivers/usbstor.obj:DRIVERS/USBSTOR.OBJ:$(KERNEL)/drivers/usbstor.asm \
 drivers/vt823x.obj:DRIVERS/VT823X.OBJ:$(KERNEL)/drivers/vt823x.asm \
 drivers/vidintel.obj:DRIVERS/VIDINTEL.OBJ:$(KERNEL)/drivers/vidintel.asm \
 drivers/3c59x.obj:DRIVERS/3C59X.obj:$(REPOSITORY)/drivers/ethernet/3c59x.asm \
 drivers/dec21x4x.obj:DRIVERS/DEC21X4X.obj:$(REPOSITORY)/drivers/ethernet/dec21x4x.asm \
 drivers/forcedeth.obj:DRIVERS/FORCEDETH.obj:$(REPOSITORY)/drivers/ethernet/forcedeth.asm \
 drivers/i8254x.sys:DRIVERS/I8254X.SYS:$(REPOSITORY)/drivers/ethernet/i8254x.asm \
 drivers/i8255x.sys:DRIVERS/I8255X.SYS:$(REPOSITORY)/drivers/ethernet/i8255x.asm \
 drivers/mtd80x.sys:DRIVERS/MTD80X.SYS:$(REPOSITORY)/drivers/ethernet/mtd80x.asm \
 drivers/pcnet32.sys:DRIVERS/PCNET32.SYS:$(REPOSITORY)/drivers/ethernet/pcnet32.asm \
 drivers/r6040.sys:DRIVERS/R6040.SYS:$(REPOSITORY)/drivers/ethernet/R6040.asm \
 drivers/rhine.sys:DRIVERS/RHINE.SYS:$(REPOSITORY)/drivers/ethernet/rhine.asm \
 drivers/rtl8029.obj:DRIVERS/RTL8029.obj:$(REPOSITORY)/drivers/ethernet/RTL8029.asm \
 drivers/rtl8139.obj:DRIVERS/RTL8139.obj:$(REPOSITORY)/drivers/ethernet/RTL8139.asm \
 drivers/rtl8169.obj:DRIVERS/RTL8169.obj:$(REPOSITORY)/drivers/ethernet/RTL8169.asm \
 drivers/sis900.obj:DRIVERS/SIS900.obj:$(REPOSITORY)/drivers/ethernet/sis900.asm \
 drivers/uhci.sys:DRIVERS/UHCI.SYS:$(REPOSITORY)/drivers/usb/uhci.asm \
 drivers/ohci.sys:DRIVERS/OHCI.SYS:$(REPOSITORY)/drivers/usb/ohci.asm \
 drivers/ehci.sys:DRIVERS/EHCI.SYS:$(REPOSITORY)/drivers/usb/ehci.asm \
 drivers/rdc.sys:DRIVERS/RDC.SYS:$(REPOSITORY)/drivers/video/rdc.asm \
 File|Managers/kfar:File|Managers/KFAR:$(PROGS)/fs/kfar/trunk/kfar.asm \
 File|Managers/kfm:File|Managers/KFM:$(PROGS)/fs/kfm/trunk/kfm.asm \
 File|Managers/opendial:File|Managers/OPENDIAL:$(PROGS)/fs/opendial/opendial.asm \
 games/15:GAMES/15:$(PROGS)/games/15/trunk/15.ASM \
 games/arcanii:GAMES/ARCANII:$(PROGS)/games/arcanii/trunk/arcanii.asm \
 games/freecell:GAMES/FREECELL:$(PROGS)/games/freecell/freecell.asm \
 games/gomoku:GAMES/GOMOKU:$(PROGS)/games/gomoku/trunk/gomoku.asm \
 games/invaders:GAMES/INVADERS:$(PROGS)/games/invaders/invaders.asm \
 games/klavisha:GAMES/KLAVISHA:$(PROGS)/games/klavisha/trunk/klavisha.asm \
 games/lines:GAMES/LINES:$(PROGS)/games/lines/lines.asm \
 games/mblocks:GAMES/MBLOCKS:$(PROGS)/games/mblocks/trunk/mblocks.asm \
 games/msquare:GAMES/MSQUARE:$(PROGS)/games/MSquare/trunk/MSquare.asm \
 games/phenix:GAMES/PHENIX:$(PROGS)/games/phenix/trunk/phenix.asm \
 games/pipes:GAMES/PIPES:$(PROGS)/games/pipes/pipes.asm \
 games/pong:GAMES/PONG:$(PROGS)/games/pong/trunk/pong.asm \
 games/pong3:GAMES/PONG3:$(PROGS)/games/pong3/trunk/pong3.asm \
 games/rsquare:GAMES/RSQUARE:$(PROGS)/games/rsquare/trunk/rsquare.asm \
 games/soko:GAMES/SOKO:$(PROGS)/games/soko/trunk/SOKO.ASM \
 games/snake:GAMES/SNAKE:$(PROGS)/games/snake/trunk/snake.asm \
 games/sq_game:GAMES/SQ_GAME:$(PROGS)/games/sq_game/trunk/SQ_GAME.ASM \
 games/sudoku:GAMES/SUDOKU:$(PROGS)/games/sudoku/trunk/sudoku.asm \
 games/sw:GAMES/SW:$(PROGS)/games/sw/trunk/sw.asm \
 games/tanks:GAMES/TANKS:$(PROGS)/games/tanks/trunk/tanks.asm \
 games/tetris:GAMES/TETRIS:$(PROGS)/games/tetris/trunk/tetris.asm \
 lib/archiver.obj:LIB/ARCHIVER.OBJ:$(PROGS)/fs/kfar/trunk/kfar_arc/kfar_arc.asm \
 lib/box_lib.obj:LIB/BOX_LIB.OBJ:$(PROGS)/develop/libraries/box_lib/trunk/box_lib.asm \
 lib/buf2d.obj:LIB/BUF2D.OBJ:$(PROGS)/develop/libraries/buf2d/trunk/buf2d.asm \
 lib/console.obj:LIB/CONSOLE.OBJ:$(PROGS)/develop/libraries/console/console.asm \
 lib/libgfx.obj:LIB/LIBGFX.OBJ:$(PROGS)/develop/libraries/libs-dev/libgfx/libgfx.asm \
 lib/libimg.obj:LIB/LIBIMG.OBJ:$(PROGS)/develop/libraries/libs-dev/libimg/libimg.asm \
 lib/libini.obj:LIB/LIBINI.OBJ:$(PROGS)/develop/libraries/libs-dev/libini/libini.asm \
 lib/libio.obj:LIB/LIBIO.OBJ:$(PROGS)/develop/libraries/libs-dev/libio/libio.asm \
 lib/msgbox.obj:LIB/MSGBOX.OBJ:$(PROGS)/develop/libraries/msgbox/msgbox.asm \
 lib/network.obj:LIB/NETWORK.OBJ:$(PROGS)/develop/libraries/network/network.asm \
 lib/sort.obj:LIB/SORT.OBJ:$(PROGS)/develop/libraries/sorter/sort.asm \
 lib/http.obj:LIB/HTTP.OBJ:$(PROGS)/develop/libraries/http/http.asm \
 lib/proc_lib.obj:LIB/PROC_LIB.OBJ:$(PROGS)/develop/libraries/proc_lib/trunk/proc_lib.asm \
 lib/cnv_png.obj:LIB/CNV_PNG.OBJ:$(PROGS)/media/zsea/plugins/png/cnv_png.asm \
 media/animage:MEDIA/ANIMAGE:$(PROGS)/media/animage/trunk/animage.asm \
 media/kiv:MEDIA/KIV:$(PROGS)/media/kiv/trunk/kiv.asm \
 media/listplay:MEDIA/LISTPLAY:$(PROGS)/media/listplay/trunk/listplay.asm \
 media/midamp:MEDIA/MIDAMP:$(PROGS)/media/midamp/trunk/midamp.asm \
 media/palitra:MEDIA/PALITRA:$(PROGS)/media/palitra/trunk/palitra.asm \
 media/startmus:MEDIA/STARTMUS:$(PROGS)/media/startmus/trunk/STARTMUS.ASM \
 network/ping:NETWORK/PING:$(PROGS)/network/icmp/ping.asm \
 network/netcfg:NETWORK/NETCFG:$(PROGS)/network/netcfg/netcfg.asm \
 network/netstat:NETWORK/NETSTAT:$(PROGS)/network/netstat/netstat.asm \
 network/nslookup:NETWORK/NSLOOKUP:$(PROGS)/network/nslookup/nslookup.asm \
 network/synergyc:NETWORK/SYNERGYC:$(PROGS)/network/synergyc/synergyc.asm \
 network/tcpserv:NETWORK/TCPSERV:$(PROGS)/network/tcpserv/tcpserv.asm \
 network/telnet:NETWORK/TELNET:$(PROGS)/network/telnet/telnet.asm \
 network/zeroconf:NETWORK/ZEROCONF:$(PROGS)/network/zeroconf/zeroconf.asm \
 network/ftpc:NETWORK/FTPC:$(PROGS)/network/ftpc/ftpc.asm \
 network/ftpd:NETWORK/FTPD:$(PROGS)/network/ftpd/ftpd.asm \
 network/tftpc:NETWORK/TFTPC:$(PROGS)/network/tftpc/tftpc.asm \
 network/ircc:NETWORK/IRCC:$(PROGS)/network/ircc/ircc.asm \
 network/downloader:NETWORK/DOWNLOADER:$(PROGS)/network/downloader/downloader.asm \
# end of list
# The list of all FASM programs with one main FASM file for CD image.
# Format of an item is exactly the same as in the previous list.
FASM_PROGRAMS_CD:=\
 drivers/atikms:DRIVERS/ATIKMS:$(REPOSITORY)/drivers/video/drm/radeon/atikms.asm \
 distr_data/zsea:DISTR_DATA/ZSEA:$(PROGS)/media/zsea/zSea.asm \
 distr_data/cnv_bmp.obj:DISTR_DATA/CNV_BMP.OBJ:$(PROGS)/media/zsea/plugins/bmp/cnv_bmp.asm \
 distr_data/cnv_gif.obj:DISTR_DATA/CNV_GIF.OBJ:$(PROGS)/media/zsea/plugins/gif/cnv_gif.asm \
 distr_data/cnv_jpeg.obj:DISTR_DATA/CNV_JPEG.OBJ:$(PROGS)/media/zsea/plugins/jpeg/cnv_jpeg.asm \
 distr_data/convert.obj:DISTR_DATA/CONVERT.OBJ:$(PROGS)/media/zsea/plugins/convert/convert.asm \
 distr_data/rotate.obj:DISTR_DATA/ROTATE.OBJ:$(PROGS)/media/zsea/plugins/rotate/rotate.asm \
 distr_data/scaling.obj:DISTR_DATA/SCALING.OBJ:$(PROGS)/media/zsea/plugins/scaling/scaling.asm \
# end of list

# The list of all FASM programs which needs to be compiled without KPACKing.
FASM_NOKPACK_PROGRAMS:=\
 distr_data/9x2klbr.exe:-:$(PROGS)/hd_load/9x2klbr/9x2klbr.asm \
 distr_data/MeOSload.com:-:$(PROGS)/hd_load/meosload/MeOSload.asm \
 distr_data/mtldr:-:$(PROGS)/hd_load/mtldr/mtldr.asm \
 mtldr_for_installer:-:$(PROGS)/hd_load/mtldr_install/mtldr_code/mtldr.asm \
 distr_data/mtldr_install.exe:-:$(PROGS)/hd_load/mtldr_install/mtldr_install.asm \
 distr_data/MeOSload_for_usb_boot_old.com:-:$(PROGS)/hd_load/usb_boot_old/MeOSload.asm \
 distr_data/enable_for_usb_boot_old.exe:-:$(PROGS)/hd_load/usb_boot_old/enable.asm \
 distr_data/BOOT_F32.BIN:-:$(PROGS)/hd_load/usb_boot/BOOT_F32.ASM \
 distr_data/MTLD_F32:-:$(PROGS)/hd_load/usb_boot/mtldr.asm \
 distr_data/inst.exe:-:$(PROGS)/hd_load/usb_boot/inst.asm \
 distr_data/setmbr.exe:-:$(PROGS)/hd_load/usb_boot/setmbr.asm \
#end of list

# The list of all NASM programs with one main NASM file.
# Format of an item is exactly the same as in the previous list.
NASM_PROGRAMS:=\
 demos/aclock:DEMOS/ACLOCK:$(PROGS)/demos/aclock/trunk/aclock.asm \
 demos/timer:DEMOS/TIMER:$(PROGS)/other/Timer/Timer.asm \
 games/c4:GAMES/C4:$(PROGS)/games/c4/trunk/c4.asm \
# end of list

# The list of files which should be copied from somewhere.
# Format of an item is exactly the same as in the previous list.
COPY_FILES:=\
 macros.inc:MACROS.INC:$(PROGS)/macros.inc \
 config.inc:CONFIG.INC:$(PROGS)/config.inc \
 struct.inc:STRUCT.INC:$(PROGS)/struct.inc \
 develop/te_icon.png:DEVELOP/TE_ICON.PNG:$(PROGS)/other/t_edit/te_icon.png \
 develop/tl_nod_16.png:DEVELOP/TL_NOD_16.PNG:$(PROGS)/other/t_edit/tl_nod_16.png \
 develop/tl_sys_16.png:DEVELOP/TL_SYS_16.PNG:$(PROGS)/media/log_el/trunk/tl_sys_16.png \
 develop/t_edit.ini:DEVELOP/T_EDIT.INI:$(PROGS)/other/t_edit/t_edit.ini \
 File|Managers/z_icons.png:File|Managers/Z_ICONS.PNG:$(PROGS)/fs/opendial/z_icons.png \
 File|Managers/kfm_keys.txt:File|Managers/KFM_KEYS.TXT:$(PROGS)/fs/kfm/trunk/docs/english/kfm_keys.txt \
 File|Managers/buttons.bmp:File|Managers/BUTTONS.BMP:$(PROGS)/fs/kfm/trunk/buttons.bmp \
 File|Managers/icons.bmp:File|Managers/ICONS.BMP:$(PROGS)/fs/kfm/trunk/icons.bmp \
 fonts/litt.chr:FONTS/LITT.CHR:$(PROGS)/demos/bgitest/trunk/FONTS/LITT.CHR \
 games/snake.ini:GAMES/SNAKE.INI:$(PROGS)/games/snake/trunk/snake.ini \
 games/skin.raw:GAMES/SKIN.RAW:$(PROGS)/games/soko/trunk/SKIN.RAW \
 games/soko-4.lev:GAMES/SOKO-4.LEV:$(PROGS)/games/soko/trunk/SOKO-4.LEV \
 games/soko-5.lev:GAMES/SOKO-5.LEV:$(PROGS)/games/soko/trunk/SOKO-5.LEV \
 games/soko-6.lev:GAMES/SOKO-6.LEV:$(PROGS)/games/soko/trunk/SOKO-6.LEV \
 games/soko-7.lev:GAMES/SOKO-7.LEV:$(PROGS)/games/soko/trunk/SOKO-7.LEV \
 games/soko-8.lev:GAMES/SOKO-8.LEV:$(PROGS)/games/soko/trunk/SOKO-8.LEV \
 games/soko-9.lev:GAMES/SOKO-9.LEV:$(PROGS)/games/soko/trunk/SOKO-9.LEV \
 media/kiv.ini:MEDIA/KIV.INI:$(PROGS)/media/kiv/trunk/kiv.ini \
 settings/.shell:SETTINGS/.shell:$(PROGS)/system/shell/bin/eng/.shell \
 settings/mykey.ini:SETTINGS/MYKEY.INI:$(PROGS)/system/MyKey/trunk/mykey_it.ini \
 network/wv_skin.png:NETWORK/WV_SKIN.PNG:$(PROGS)/cmm/browser/wv_skin.png \
# end of list

# The list of all C-- programs with one main C-- file.
# Format of an item is exactly the same as in the previous list,
# except that there can be fourth field with parameters for a compiler.
CMM_PROGRAMS:=\
 #File|Managers/eolite:File|Managers/EOLITE:$(PROGS)/cmm/eolite/Eolite.c \
 games/clicks:GAMES/CLICKS:$(PROGS)/games/clicks/trunk/clicks.c \
 games/FindNumbers:GAMES/FindNumbers:$(PROGS)/games/FindNumbers/trunk/FindNumbers.c-- \
 games/flood-it:GAMES/flood-it:$(PROGS)/games/flood-it/trunk/flood-it.c \
 games/mine:GAMES/MINE:$(PROGS)/games/mine/trunk/mine.c--:/MEOS \
 network/WebView:network/WebView:$(PROGS)/cmm/browser/WebView.c \
 tmpdisk:tmpdisk:$(PROGS)/cmm/tmpdisk/tmpdisk.c \
 @notify:@notify:$(PROGS)/cmm/notify/notify.c \
#develop/c--:DEVELOP/C--:$(PROGS)/develop/c--/trunk/32.c-- \
# end of list

# List of other files to be included in the image file.
# Some of them are auto-built with special rules, some just exist before build.
# Each item is of the form <local name>:<name inside image>.
# Spaces should be represented as |.
OTHER_FILES:=settings/autorun.dat:SETTINGS/AUTORUN.DAT \
 default.skn:DEFAULT.SKN \
 #graph:GRAPH \
 settings/icon.ini:SETTINGS/ICON.INI \
 ../common/iconstrp.png:ICONSTRP.PNG index_htm:INDEX.HTM \
 kernel.mnt:KERNEL.MNT \
 ../common/kerpack:KERPACK \
 settings/keymap.key:SETTINGS/KEYMAP.KEY \
 settings/lang.ini:SETTINGS/LANG.INI \
 ../common/kuzkina.mid:KUZKINA.MID lang.inc:LANG.INC \
 settings/menu.dat:SETTINGS/MENU.DAT \
 settings/kolibri.lbl:SETTINGS/KOLIBRI.LBL \
 settings/panel.ini:SETTINGS/PANEL.INI \
 ../common/settings/docky.ini:SETTINGS/DOCKY.INI \
 ../common/settings/wallpaper.dat:SETTINGS/WALLPAPER.dat \
 settings/setup.dat:SETTINGS/SETUP.DAT \
 shell:SHELL table:TABLE \
 ../common/vmode:VMODE 3d/3dsheart:3D/3DSHEART \
 ../common/3d/cubeline:3D/CUBELINE \
 ../common/3d/gears:3D/GEARS \
 ../common/3d/house.3ds:3D/HOUSE.3DS \
 ../common/demos/ak47.lif:DEMOS/AK47.LIF \
 ../common/demos/life2:DEMOS/LIFE2 \
 ../common/demos/relay.lif:DEMOS/RELAY.LIF \
 ../common/demos/rpento.lif:DEMOS/RPENTO.LIF \
 ../common/File|Managers/eolite.ini:File|Managers/EOLITE.INI \
 ../common/File|Managers/icons.ini:File|Managers/ICONS.INI \
 ../common/File|Managers/kfar.ini:File|Managers/KFAR.INI \
 ../common/File|Managers/kfm.ini:File|Managers/KFM.INI \
 games/checkers:GAMES/CHECKERS \
 games/reversi:GAMES/REVERSI games/rforces:GAMES/RFORCES \
 games/xonix:GAMES/XONIX games/megamaze:GAMES/MEGAMAZE \
 games/kosilka:GAMES/KOSILKA \
 ../common/lib/pixlib.obj:LIB/PIXLIB.OBJ \
 ../common/lib/iconv.obj:LIB/ICONV.OBJ \
 ../common/lib/netcode.obj:LIB/NETCODE.OBJ \
 media/ac97snd:MEDIA/AC97SND \
 ../common/media/ImgF/ImgF:MEDIA/ImgF/ImgF \
 ../common/media/ImgF/cEdg.obj:MEDIA/ImgF/cEdg.obj \
 ../common/media/ImgF/dither.obj:MEDIA/ImgF/dither.obj \
 ../common/media/ImgF/invSol.obj:MEDIA/ImgF/invSol.obj \
 settings/network.ini:SETTINGS/NETWORK.INI \
 ../common/network/ftpd.ini:NETWORK/FTPD.INI \
 ../common/network/users.ini:NETWORK/USERS.INI \
#end of list

# Generate skins list understandable by gnu make
Makefile.skins: $(REPOSITORY)/skins/authors.txt $(REPOSITORY)/data/generate_makefile_skins.sh
	cut -f1 $< | $(SHELL) $(REPOSITORY)/data/generate_makefile_skins.sh > $@
include Makefile.skins

# Extra targets for LiveCD image in the syntax of mkisofs
MKISOFS_EXTRA:=\
 #emu/e80/e80=e80 \
 emu/dosbox/=$(PROGS)/emulator/DosBox/dosbox \
 emu/dosbox/=$(PROGS)/emulator/DosBox/dosbox.conf \
 emu/dosbox/=$(PROGS)/emulator/DosBox/readme.txt \
 /=distr_data/autorun.inf \
 /=distr_data/KolibriOS_icon.ico \
 /=distr_data/readme.txt \
 /=settings/kolibri.lbl \
 Skins/=$(REPOSITORY)/skins/authors.txt \
 Docs/config.txt=docs/CONFIG.TXT \
 Docs/copying.txt=docs/COPYING.TXT \
 Docs/hot_keys.txt=docs/HOT_KEYS.TXT \
 Docs/install.txt=docs/INSTALL.TXT \
 Docs/readme.txt=docs/README.TXT \
 Docs/stack.txt=docs/STACK.TXT \
 Docs/sysfuncs.txt=docs/SYSFUNCS.TXT \
 HD_Load/9x2klbr/=distr_data/9x2klbr.exe \
 HD_Load/9x2klbr/=../common/HD_load/9x2klbr/LDKLBR.VXD \
 HD_Load/9x2klbr/=$(PROGS)/hd_load/9x2klbr/readme.txt \
 HD_Load/MeOSLoad/=distr_data/MeOSload.com \
 HD_Load/MeOSLoad/=$(PROGS)/hd_load/meosload/AUTOEXEC.BAT \
 HD_Load/MeOSLoad/=$(PROGS)/hd_load/meosload/CONFIG.SYS \
 HD_Load/MeOSLoad/=$(PROGS)/hd_load/meosload/L_readme.txt \
 HD_Load/MeOSLoad/=$(PROGS)/hd_load/meosload/L_readme_Win.txt \
 HD_Load/mtldr/=distr_data/mtldr \
 HD_Load/mtldr/install.txt=$(PROGS)/hd_load/mtldr/install_eng.txt \
 HD_Load/mtldr/=$(PROGS)/hd_load/mtldr/vista_install.bat \
 HD_Load/mtldr/=$(PROGS)/hd_load/mtldr/vista_remove.bat \
 HD_Load/=distr_data/mtldr_install.exe \
 HD_Load/memdisk=../common/HD_load/memdisk \
 HD_Load/USB_Boot/=distr_data/BOOT_F32.BIN \
 HD_Load/USB_Boot/=distr_data/MTLD_F32 \
 HD_Load/USB_Boot/=distr_data/inst.exe \
 HD_Load/USB_Boot/=distr_data/setmbr.exe \
 HD_Load/USB_Boot/readme.txt=$(PROGS)/hd_load/usb_boot/readme_eng.txt \
 HD_Load/USB_boot_old/=$(PROGS)/hd_load/usb_boot_old/usb_boot.rtf \
 HD_Load/USB_boot_old/=$(PROGS)/hd_load/usb_boot_old/usb_boot_866.txt \
 HD_Load/USB_boot_old/=$(PROGS)/hd_load/usb_boot_old/usb_boot_1251.txt \
 HD_Load/USB_boot_old/MeOSload.com=distr_data/MeOSload_for_usb_boot_old.com \
 HD_Load/USB_boot_old/enable.exe=distr_data/enable_for_usb_boot_old.exe \
 games/=../common/games \
 games/fara=games/fara \
 games/soko/soko=games/soko \
 kolibrios/lib/avcodec-55.dll=../common/lib/avcodec-55.dll \
 kolibrios/lib/avdevice-55.dll=../common/lib/avdevice-55.dll \
 kolibrios/lib/avformat-55.dll=../common/lib/avformat-55.dll \
 kolibrios/lib/avutil-52.dll=../common/lib/avutil-52.dll \
 kolibrios/lib/freetype.dll=../common/lib/freetype.dll \
 kolibrios/lib/libc.dll=../common/lib/libc.dll \
 kolibrios/lib/pixlib.dll=../common/lib/pixlib.dll \
 kolibrios/lib/swresample-0.dll=../common/lib/swresample-0.dll \
 kolibrios/lib/swscale-2.dll=../common/lib/swscale-2.dll \
 kolibrios/lib/zlib.dll=../common/lib/zlib.dll \
 kolibrios/media/fplay-full=../common/media/fplay-full \
 kolibrios/media/updf=updf \
 kolibrios/media/zsea/zsea=distr_data/zsea \
 kolibrios/media/zsea/plugins/cnv_bmp.obj=distr_data/cnv_bmp.obj \
 kolibrios/media/zsea/plugins/cnv_gif.obj=distr_data/cnv_gif.obj \
 kolibrios/media/zsea/plugins/cnv_jpeg.obj=distr_data/cnv_jpeg.obj \
 kolibrios/media/zsea/plugins/convert.obj=distr_data/convert.obj \
 kolibrios/media/zsea/plugins/rotate.obj=distr_data/rotate.obj \
 kolibrios/media/zsea/plugins/scaling.obj=distr_data/scaling.obj \
 kolibrios/media/zsea/zsea.ini=$(REPOSITORY)/programs/media/zsea/zSea.ini \
 kolibrios/media/zsea/zsea_keys.txt=$(REPOSITORY)/programs/media/zsea/Docs/zSea_keys_eng.txt \
 kolibrios/media/zsea/buttons/buttons.png=$(REPOSITORY)/programs/media/zsea/buttons.png \
#end of list

# Some macro for convenient work.
# Macros for replacing '|' to escaped space '\ '.
space:=\ #plus space
respace=$(subst |,$(space),$(1))
# Macro for selecting different parts of ':'-separated items.
binarypart=$(word 1,$(subst :, ,$(1)))
imagepart=$(word 2,$(subst :, ,$(1)))
sourcepart=$(word 3,$(subst :, ,$(1)))
parampart=$(word 4,$(subst :, ,$(1)))
# Get file names, possibly with spaces inside, from an item.
# Here $(f) is an item - in fact, macro argument.
fbinary=$(call respace,$(call binarypart,$(f)))
fimage=$(call respace,$(call imagepart,$(f)))
fsource=$(call respace,$(call sourcepart,$(f)))
fparam=$(call respace,$(call parampart,$(f)))

# Define targets for image file.
# Join all the lists above.
targets_full:=$(COPY_FILES) $(FASM_PROGRAMS) $(NASM_PROGRAMS) $(OTHER_FILES) $(CMM_PROGRAMS)
# For each item in the united list call fbinary.
targets:=$(foreach f,$(targets_full),$(fbinary))

# Define a command for copying a file inside the image.
# mcopy_command is a macro with two parameters,
# local file name $(1) and image file name $(2).
# Note that spaces in these have to be escaped with backslashes.
mcopy_command=mcopy -moi $(BUILD_DIR)/kolibri.img $(1) ::$(2)
# Specialize a previous command for an item $(f) in one of lists.
mcopy_item_command=$(call mcopy_command,$(fbinary),$(fimage))

# Join all $(mcopy_item_command) for all items,
# append newline after each item.
# The first newline character after "define" line and
# the last newline character before "endef" line get away
# with define/endef, so we make three newline characters,
# that is two empty lines, in order to get one in $(newline).
define newline


endef
mcopy_all_items:=$(foreach f,$(targets_full),$(mcopy_item_command)$(newline))

# add skins to MKISOFS_EXTRA
MKISOFS_EXTRA:=$(MKISOFS_EXTRA) $(foreach f,$(SKIN_SOURCES),$(call imagepart,$(f))=$(call binarypart,$(f)))
# dependencies of MKISOFS_EXTRA; we iterate through $(MKISOFS_EXTRA),
# substitute "=" with space, get the 2nd word and join all results
mkisofs_extra_targets:=$(call respace,$(foreach f,$(MKISOFS_EXTRA),$(word 2,$(subst =, ,$(f)))))

# The first goal: floppy image.
$(BUILD_DIR)/kolibri.img: $(BUILD_DIR)/.dir \
 Makefile \
 $(BUILD_DIR)/boot_fat12.bin \
 $(targets)
	str=`date -u +"[auto-build %d %b %Y %R, r$(REV)]"`; \
	echo -n $$str|dd of=kernel.mnt bs=1 seek=`expr 279 - length "$$str"` conv=notrunc 2>/dev/null
	dd if=/dev/zero of=$(BUILD_DIR)/kolibri.img count=2880 bs=512 2>&1
	mformat -f 1440 -i $(BUILD_DIR)/kolibri.img ::
	dd if=$(BUILD_DIR)/boot_fat12.bin of=$(BUILD_DIR)/kolibri.img count=1 bs=512 conv=notrunc 2>&1
	mmd -i $(BUILD_DIR)/kolibri.img ::3D
	mmd -i $(BUILD_DIR)/kolibri.img ::DEMOS
	mmd -i $(BUILD_DIR)/kolibri.img ::DEVELOP
	mmd -i $(BUILD_DIR)/kolibri.img ::DEVELOP/INFO
	mmd -i $(BUILD_DIR)/kolibri.img ::DRIVERS
	mmd -i $(BUILD_DIR)/kolibri.img ::File\ Managers
	mmd -i $(BUILD_DIR)/kolibri.img ::FONTS
	mmd -i $(BUILD_DIR)/kolibri.img ::GAMES
	mmd -i $(BUILD_DIR)/kolibri.img ::LIB
	mmd -i $(BUILD_DIR)/kolibri.img ::MEDIA
	mmd -i $(BUILD_DIR)/kolibri.img ::MEDIA/ImgF
	mmd -i $(BUILD_DIR)/kolibri.img ::NETWORK
	mmd -i $(BUILD_DIR)/kolibri.img ::SETTINGS
	$(mcopy_all_items)

# The second goal: LiveCD image.
$(BUILD_DIR)/kolibri.iso: $(BUILD_DIR)/kolibri.img $(mkisofs_extra_targets)
	mkisofs -U -J -pad -b kolibri.img -c boot.catalog -hide-joliet boot.catalog -graft-points \
	-A "KolibriOS AutoBuilder" -p "CleverMouse" -publisher "KolibriOS Team" -V "KolibriOS r$(REV)" -sysid "KOLIBRI" \
	-iso-level 3 -o $(BUILD_DIR)/kolibri.iso $(BUILD_DIR)/kolibri.img $(call respace,$(MKISOFS_EXTRA)) 2>&1

# Special targets to modify behaviour of make.
.DELETE_ON_ERROR:
.SUFFIXES: # delete all predefined rules

# The floppy bootsector.
$(BUILD_DIR)/boot_fat12.bin: $(KERNEL)/bootloader/boot_fat12.asm $(KERNEL)/bootloader/floppy1440.inc
	fasm $(KERNEL)/bootloader/boot_fat12.asm $(BUILD_DIR)/boot_fat12.bin

$(BUILD_DIR)/.dir 3d/.dir demos/.dir develop/.dir develop/info/.dir drivers/.dir fonts/.dir \
 games/.dir lib/.dir media/.dir network/.dir settings/.dir allskins/.dir distr_data/.dir .deps/.dir:
	mkdir -p $(dir $@)
	touch $@
develop/info/.dir: develop/.dir
File\ Managers/.dir:
	mkdir -p "File Managers"
	touch "File Managers/.dir"

# extra dependency for mtldr_install.exe
distr_data/mtldr_install.exe: mtldr_for_installer

# FASM black magic goes to Makefile.fasm.
include Makefile.fasm

# Similar for NASM.
include Makefile.nasm

# Similar for copying files.
include Makefile.copy

# Special rules for copying sysfuncs.txt and fasm.txt - they aren't directly included in the image.
docpack: $(DOCDIR)SYSFUNCS.TXT $(DOCDIR)FASM.TXT
$(DOCDIR)SYSFUNCS.TXT: $(KERNEL)/docs/sysfuncs.txt
	cp $(KERNEL)/docs/sysfuncs.txt $(DOCDIR)SYSFUNCS.TXT
$(DOCDIR)FASM.TXT: $(PROGS)/develop/fasm/trunk/fasm.txt
	cp $(PROGS)/develop/fasm/trunk/fasm.txt $(DOCDIR)FASM.TXT

# Similar for C--.
include Makefile.cmm

# Sorry, even black magic seems to be insufficient for
# auto-handling all subtle effects. So we just define
# command lines for compiling and linking, and
# maintain the list of sources and objects by hand.
include Makefile.msvc

# Rules for table
table: .obj.table/table.exe
	$(msvc_final)
TABLE_OBJECTS:=.obj.table/calc.obj .obj.table/func.obj .obj.table/hello.obj \
 .obj.table/KosFile.obj .obj.table/kosSyst.obj .obj.table/math2.obj \
 .obj.table/mcsmemm.obj .obj.table/parser.obj
TABLE_H_FILES:=$(wildcard $(PROGS)/other/table/*.h)
.obj.table/table.exe: $(TABLE_OBJECTS)
	$(msvc_link)
$(TABLE_OBJECTS): .obj.table/%.obj: $(PROGS)/other/table/%.cpp $(TABLE_H_FILES) Makefile.msvc | .obj.table
	$(msvc_compile)
.obj.table:
	mkdir -p .obj.table

# Rules for graph
graph: .obj.graph/graph.exe
	$(msvc_final)
GRAPH_CPP_OBJECTS:=.obj.graph/func.obj .obj.graph/hello.obj .obj.graph/kolibri.obj \
 .obj.graph/KosFile.obj .obj.graph/kosSyst.obj .obj.graph/math2.obj \
 .obj.graph/mcsmemm.obj .obj.graph/parser.obj
GRAPH_C_OBJECTS:=.obj.graph/string.obj
GRAPH_H_FILES:=$(wildcard $(PROGS)/other/graph/*.h)
GRAPH_FASM_OBJECTS:=.obj.graph/memcpy.obj .obj.graph/memset.obj
.obj.graph/graph.exe: $(GRAPH_CPP_OBJECTS) $(GRAPH_C_OBJECTS) $(GRAPH_FASM_OBJECTS)
	$(msvc_link)
$(GRAPH_CPP_OBJECTS): .obj.graph/%.obj: $(PROGS)/other/graph/%.cpp $(GRAPH_H_FILES) Makefile.msvc | .obj.graph
	$(msvc_compile)
$(GRAPH_C_OBJECTS): .obj.graph/%.obj: $(PROGS)/other/graph/%.c $(GRAPH_H_FILES) Makefile.msvc | .obj.graph
	$(msvc_compile)
$(GRAPH_FASM_OBJECTS): .obj.graph/%.obj: $(PROGS)/other/graph/%.asm Makefile.msvc | .obj.graph
	fasm $< $@
.obj.graph:
	mkdir -p .obj.graph

# Rules for kosilka
games/kosilka: .obj.kosilka/kosilka.exe
	$(msvc_final)
KOSILKA_OBJECTS:=.obj.kosilka/kosilka.obj .obj.kosilka/KosFile.obj .obj.kosilka/kosSyst.obj .obj.kosilka/mcsmemm.obj
KOSILKA_H_FILES:=$(PROGS)/games/kosilka/*.h
.obj.kosilka/kosilka.exe: $(KOSILKA_OBJECTS)
	$(msvc_link)
$(KOSILKA_OBJECTS): .obj.kosilka/%.obj: $(PROGS)/games/kosilka/%.cpp $(KOSILKA_H_FILES) Makefile.msvc | .obj.kosilka
	$(msvc_compile)
.obj.kosilka:
	mkdir -p .obj.kosilka

include Makefile.gcc

# Rules for shell
shell: .obj.shell/start.o .obj.shell/shell.o .obj.shell/kolibri.o .obj.shell/stdlib.o .obj.shell/string.o .obj.shell/ctype.o \
 $(PROGS)/system/shell/kolibri.ld
	$(call gcc_link,$(PROGS)/system/shell/kolibri.ld)
.obj.shell/shell.o: $(PROGS)/system/shell/shell.c \
 $(PROGS)/system/shell/all.h \
 $(PROGS)/system/shell/system/*.h \
 $(PROGS)/system/shell/cmd/*.c \
 $(PROGS)/system/shell/modules/*.c \
 $(PROGS)/system/shell/locale/rus/globals.h \
 Makefile.gcc | .obj.shell
	$(gcc_compile)
.obj.shell/kolibri.o .obj.shell/stdlib.o .obj.shell/string.o .obj.shell/ctype.o: .obj.shell/%.o: \
 $(PROGS)/system/shell/system/%.c $(PROGS)/system/shell/system/*.h \
 Makefile.gcc | .obj.shell
	$(gcc_compile)
	win32-gcc -c -Os -o $@ $<
.obj.shell/start.o: $(PROGS)/system/shell/start.asm | .obj.shell
	fasm $< $@
.obj.shell:
	mkdir -p .obj.shell

# Rules for e80
E80DIR=$(PROGS)/emulator/e80/trunk
e80: .obj.e80/start.o .obj.e80/kolibri.o .obj.e80/stdlib.o .obj.e80/string.o .obj.e80/z80.o .obj.e80/e80.o
	$(call gcc_link,$(E80DIR)/kolibri.ld)
.obj.e80/e80.o: $(E80DIR)/e80.c $(E80DIR)/48.h \
 $(E80DIR)/system/*.h $(E80DIR)/system/msgbox.c \
 $(E80DIR)/z80/z80.h Makefile.gcc | .obj.e80
	$(gcc_compile)
.obj.e80/kolibri.o .obj.e80/stdlib.o .obj.e80/string.o: .obj.e80/%.o: \
 $(E80DIR)/system/%.c $(E80DIR)/system/*.h Makefile.gcc | .obj.e80
	$(gcc_compile)
.obj.e80/z80.o: $(E80DIR)/z80/z80.c $(E80DIR)/z80/*
	$(gcc_compile)
.obj.e80/start.o: $(E80DIR)/asm_code.asm | .obj.e80
	fasm $< $@
.obj.e80:
	mkdir -p .obj.e80


# Rules for sdk/sound, used by media/ac97snd
SOUNDDIR=$(PROGS)/develop/sdk/trunk/sound/src
SOUND_OBJECTS:=$(patsubst $(SOUNDDIR)/%.asm,.sdk/%.obj,$(wildcard $(SOUNDDIR)/*.asm))
SOUND_INC_FILES:=$(wildcard $(SOUNDDIR)/*.inc)
.sdk/sound.lib: $(SOUND_OBJECTS)
	win32-link /lib /out:$@ $^
$(SOUND_OBJECTS): .sdk/%.obj: $(SOUNDDIR)/%.asm $(SOUND_INC_FILES) | .sdk
	fasm $< $@
.sdk:
	mkdir -p .sdk
# Rules for media/ac97snd
AC97DIR=$(PROGS)/media/ac97snd
media/ac97snd: .obj.ac97snd/ac97snd.exe
	$(msvc_final)
.obj.ac97snd/ac97snd.exe: .obj.ac97snd/ac97wav.obj .obj.ac97snd/crt.obj .obj.ac97snd/k_lib.obj \
	.obj.ac97snd/mpg.lib .sdk/sound.lib .obj.ac97snd/ufmod.obj
	$(msvc_link)
.obj.ac97snd/ac97wav.obj: $(AC97DIR)/ac97snd/ac97wav.c \
	$(AC97DIR)/kolibri.h $(AC97DIR)/ac97snd/ac97wav.h $(AC97DIR)/mpg/mpg123.h \
	$(AC97DIR)/sound.h $(AC97DIR)/ufmod-codec.h Makefile.msvc | .obj.ac97snd
	$(msvc_compile)
.obj.ac97snd/crt.obj: $(AC97DIR)/ac97snd/crt.c $(AC97DIR)/ac97snd/crt.h Makefile.msvc | .obj.ac97snd
	$(msvc_compile)
.obj.ac97snd/k_lib.obj: $(AC97DIR)/ac97snd/k_lib.asm $(AC97DIR)/ac97snd/proc32.inc | .obj.ac97snd
	fasm $< $@
.obj.ac97snd/ufmod.obj: $(AC97DIR)/ufmod-config.asm | .obj.ac97snd
	fasm $< $@ -s .deps/ac97snd-ufmod.fas
	prepsrc .deps/ac97snd-ufmod.fas /dev/stdout | \
	perl -n -e 's|\\|/|g;s| |\\ |g;push @a,$$1 if/^;include\\ \x27(.*?)\x27/;' \
	-e 'END{$$a=join " \\\n ",@a;print "$@: $$a\n$$a:\n"}' > .deps/ac97snd-ufmod.Po
-include .deps/ac97snd-ufmod.Po
AC97SND_MPG_C_FILES:=$(wildcard $(AC97DIR)/mpg/*.c)
AC97SND_MPG_H_FILES:=$(wildcard $(AC97DIR)/mpg/*.h)
AC97SND_MPG_C_OBJECTS:=$(patsubst $(AC97DIR)/mpg/%.c,.obj.ac97snd/%.o,$(AC97SND_MPG_C_FILES))
.obj.ac97snd/mpg.lib: $(AC97SND_MPG_C_OBJECTS) .obj.ac97snd/pow.obj
	win32-link /lib /ltcg /out:$@ $^
$(AC97SND_MPG_C_OBJECTS): .obj.ac97snd/%.o: $(AC97DIR)/mpg/%.c $(AC97SND_MPG_H_FILES) Makefile.msvc | .obj.ac97snd
	$(msvc_compile)
.obj.ac97snd/pow.obj: $(AC97DIR)/mpg/pow.asm $(AC97DIR)/mpg/proc32.inc | .obj.ac97snd
	fasm $< $@
.obj.ac97snd:
	mkdir -p .obj.ac97snd

# Rules for atikms.dll
# Use Makefile from $(REPOSITORY)/drivers/ddk and $(REPOSITORY)/drivers/video/drm/radeon
# However, dependencies must be duplicated - I don't know how to avoid this
# without need to rebuild kolibri.img at every iteration...
# Note that we are going to write in the directory shared
# between all Makefiles, so we need locked operations.
drivers/atikms.dll: $(REPOSITORY)/drivers/video/drm/radeon/atikms.dll drivers/.dir
	kpack.o --nologo $< $@
$(REPOSITORY)/drivers/video/drm/radeon/atikms.dll: $(REPOSITORY)/drivers/video/drm/radeon/Makefile.lto
	flock $(REPOSITORY)/drivers/video/drm/radeon/.lock \
	$(MAKE) CC=win32-gcc45 AS=win32-as LD=win32-ld AR=win32-ar FASM=fasm -C $(REPOSITORY)/drivers/video/drm/radeon -f Makefile.lto
$(REPOSITORY)/drivers/ddk/libddk.a: $(REPOSITORY)/drivers/ddk/Makefile
	flock $(REPOSITORY)/drivers/ddk/.lock \
	$(MAKE) CC=win32-gcc45 AS=win32-as LD=win32-ld AR=win32-ar FASM=fasm -C $(REPOSITORY)/drivers/ddk libddk.a
$(REPOSITORY)/drivers/ddk/libcore.a: $(REPOSITORY)/drivers/ddk/Makefile
	flock $(REPOSITORY)/drivers/ddk/.lock \
	$(MAKE) CC=win32-gcc45 AS=win32-as LD=win32-ld AR=win32-ar FASM=fasm -C $(REPOSITORY)/drivers/ddk libcore.a
# dependencies
$(REPOSITORY)/drivers/video/drm/radeon/atikms.dll: \
 $(REPOSITORY)/drivers/video/drm/radeon/atikms.lds \
 $(REPOSITORY)/drivers/ddk/libddk.a \
 $(REPOSITORY)/drivers/ddk/libcore.a \
 $(REPOSITORY)/drivers/video/drm/radeon/*.[Sch] \
 $(REPOSITORY)/drivers/video/drm/radeon/*.asm \
 $(REPOSITORY)/drivers/video/drm/radeon/Makefile \
 $(REPOSITORY)/drivers/video/drm/radeon/firmware/*.bin \
 $(REPOSITORY)/drivers/include/*.h \
 $(REPOSITORY)/drivers/include/*/*.h \
 $(REPOSITORY)/drivers/include/*/*/*.h \
 $(REPOSITORY)/drivers/include/*/*/*/*.h
$(REPOSITORY)/drivers/ddk/libddk.a: \
 $(REPOSITORY)/drivers/ddk/*/* \
 $(REPOSITORY)/drivers/include/*.h \
 $(REPOSITORY)/drivers/include/*/*.h \
 $(REPOSITORY)/drivers/include/*/*/*.h \
 $(REPOSITORY)/drivers/include/*/*/*/*.h
$(REPOSITORY)/drivers/ddk/libcore.a: \
 $(REPOSITORY)/drivers/ddk/core.S



#Rules for updf
UPDFDIR=$(REPOSITORY)/contrib/media/updf
updf:  
	$(MAKE) -C $(REPOSITORY)/contrib/menuetlibc

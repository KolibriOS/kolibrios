# General rule for naming: variables with CAPITALIZED names hold settings,
# you can - and are expected - to modify it; variables with lowercase names
# are intermediate variables and macroses not to be modified unless you
# know what you're doing.

# Define directories for destination, source repository, sources of kernel, sources of programs.
BUILD_DIR:=build
REPOSITORY:=../..
KERNEL:=$(REPOSITORY)/kernel/trunk
PROGS:=$(REPOSITORY)/programs

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
 @clip:@CLIP:$(PROGS)/system/clip/trunk/@clip.ASM \
 @menu:@MENU:$(PROGS)/system/menu/trunk/menu.asm \
 @panel:@PANEL:$(PROGS)/system/panel/trunk/@PANEL.ASM \
 @rb:@RB:$(PROGS)/system/rb/trunk/@RB.ASM \
 @ss:@SS:$(PROGS)/system/ss/trunk/@ss.asm \
 calc:CALC:$(PROGS)/other/calc/trunk/calc.asm \
 calendar:CALENDAR:$(PROGS)/system/calendar/trunk/calendar.asm \
 commouse:COMMOUSE:$(PROGS)/system/commouse/trunk/commouse.asm \
 copyr:COPYR:$(PROGS)/fs/copyr/trunk/copyr.asm \
 cpu:CPU:$(PROGS)/system/cpu/trunk/cpu.asm \
 cpuid:CPUID:$(PROGS)/system/cpuid/trunk/CPUID.ASM \
 desktop:DESKTOP:$(PROGS)/system/desktop/trunk/desktop.asm \
 disptest:DISPTEST:$(PROGS)/system/disptest/trunk/disptest.ASM \
 docpak:DOCPAK:$(PROGS)/system/docpack/trunk/docpack.asm \
 downloader:downloader:$(PROGS)/network/downloader/trunk/downloader.asm \
 end:END:$(PROGS)/system/end/light/end.asm \
 gmon:GMON:$(PROGS)/system/gmon/gmon.asm \
 hdd_info:HDD_INFO:$(PROGS)/system/hdd_info/trunk/hdd_info.asm \
 icon:ICON:$(PROGS)/system/icon/trunk/icon.asm \
 kbd:KBD:$(PROGS)/system/kbd/trunk/kbd.ASM \
 launcher:LAUNCHER:$(PROGS)/system/launcher/trunk/launcher.asm \
 magnify:MAGNIFY:$(PROGS)/demos/magnify/trunk/magnify.asm \
 mgb:MGB:$(PROGS)/system/mgb/trunk/mgb.asm \
 mousemul:MOUSEMUL:$(PROGS)/system/mousemul/trunk/mousemul.asm \
 pcidev:PCIDEV:$(PROGS)/system/pcidev/trunk/PCIDEV.ASM \
 period:PERIOD:$(PROGS)/other/period/trunk/period.asm \
 pic4:PIC4:$(PROGS)/media/pic4/trunk/pic4.asm \
 rdsave:RDSAVE:$(PROGS)/system/rdsave/trunk/rdsave.asm \
 rtfread:RTFREAD:$(PROGS)/other/rtfread/trunk/rtfread.asm \
 run:RUN:$(PROGS)/system/run/trunk/run.asm \
 scrshoot:SCRSHOOT:$(PROGS)/media/scrshoot/scrshoot.asm \
 setup:SETUP:$(PROGS)/system/setup/trunk/setup.asm \
 tinypad:TINYPAD:$(PROGS)/develop/tinypad/trunk/tinypad.asm \
 vrr:VRR:$(PROGS)/system/vrr/trunk/vrr.asm \
 vrr_m:VRR_M:$(PROGS)/system/vrr_m/trunk/vrr_m.asm \
 zkey:ZKEY:$(PROGS)/system/zkey/trunk/ZKEY.ASM \
 3d/3dcube2:3D/3DCUBE2:$(PROGS)/demos/3dcube2/trunk/3DCUBE2.ASM \
 3d/3dtcub10:3D/3DTCUB10:$(PROGS)/demos/3dtcub10/trunk/3DTCUB10.ASM \
 3d/crownscr:3D/CROWNSCR:$(PROGS)/demos/crownscr/trunk/crownscr.asm  \
 3d/free3d04:3D/FREE3D04:$(PROGS)/demos/free3d04/trunk/free3d04.asm \
 3d/view3ds:3D/VIEW3DS:$(PROGS)/demos/3DS/VIEW3DS.ASM \
 demos/bcdclk:DEMOS/BCDCLK:$(PROGS)/demos/bcdclk/bcdclk/bcdclk.asm \
 demos/circle:DEMOS/CIRCLE:$(PROGS)/develop/examples/circle/trunk/circle.asm \
 demos/colorref:DEMOS/COLORREF:$(PROGS)/demos/colorref/trunk/colorref.asm \
 demos/cslide:DEMOS/CSLIDE:$(PROGS)/demos/cslide/trunk/cslide.asm \
 demos/eyes:DEMOS/EYES:$(PROGS)/demos/eyes/trunk/eyes.asm \
 demos/fire:DEMOS/FIRE:$(PROGS)/demos/fire/trunk/fire.asm \
 demos/firework:DEMOS/FIREWORK:$(PROGS)/demos/firework/trunk/firework.asm \
 demos/movback:DEMOS/MOVBACK:$(PROGS)/demos/movback/trunk/movback.asm \
 demos/plasma:DEMOS/PLASMA:$(PROGS)/demos/plasma/trunk/plasma.asm \
 demos/timer:DEMOS/TIMER:$(PROGS)/demos/timer/trunk/timer.asm \
 demos/tinyfrac:DEMOS/TINYFRAC:$(PROGS)/demos/tinyfrac/trunk/tinyfrac.asm \
 demos/trantest:DEMOS/TRANTEST:$(PROGS)/demos/trantest/trunk/trantest.asm \
 demos/tube:DEMOS/TUBE:$(PROGS)/demos/tube/trunk/tube.asm \
 demos/unvwater:DEMOS/UNVWATER:$(PROGS)/demos/unvwater/trunk/unvwater.asm \
 develop/board:DEVELOP/BOARD:$(PROGS)/system/board/trunk/board.asm \
 develop/cObj:DEVELOP/cObj:$(PROGS)/develop/cObj/trunk/cObj.asm \
 develop/fasm:DEVELOP/FASM:$(PROGS)/develop/fasm/trunk/fasm.asm \
 develop/h2d2b:DEVELOP/H2D2B:$(PROGS)/develop/h2d2b/trunk/h2d2b.asm \
 develop/heed:DEVELOP/HEED:$(PROGS)/develop/heed/trunk/heed.asm \
 develop/ipc:DEVELOP/IPC:$(PROGS)/network/ipc/trunk/ipc.asm \
 develop/keyascii:DEVELOP/KEYASCII:$(PROGS)/develop/keyascii/trunk/keyascii.asm \
 develop/mtdbg:DEVELOP/MTDBG:$(PROGS)/develop/mtdbg/mtdbg.asm \
 develop/scancode:DEVELOP/SCANCODE:$(PROGS)/develop/scancode/trunk/scancode.asm \
 develop/test_gets:DEVELOP/test_gets:$(PROGS)/develop/libraries/console/examples/test_gets.asm \
 develop/testcon2:DEVELOP/TESTCON2:$(PROGS)/develop/libraries/console/examples/testcon2.asm \
 develop/thread:DEVELOP/THREAD:$(PROGS)/develop/examples/thread/trunk/thread.asm \
 drivers/atikms:DRIVERS/ATIKMS:$(REPOSITORY)/drivers/video/drm/radeon/atikms.asm \
 drivers/com_mouse.obj:DRIVERS/COM_MOUSE.OBJ:$(KERNEL)/drivers/com_mouse.asm \
 drivers/emu10k1x.obj:DRIVERS/EMU10K1X.OBJ:$(KERNEL)/drivers/emu10k1x.asm \
 drivers/fm801.obj:DRIVERS/FM801.OBJ:$(KERNEL)/drivers/fm801.asm \
 drivers/infinity.obj:DRIVERS/INFINITY.OBJ:$(KERNEL)/drivers/infinity.asm \
 drivers/ps2mouse.obj:DRIVERS/PS2MOUSE.OBJ:$(REPOSITORY)/drivers/mouse/ps2mouse4d/trunk/ps2mouse.asm \
 drivers/sb16.obj:DRIVERS/SB16.OBJ:$(KERNEL)/drivers/sb16/sb16.asm \
 drivers/sound.obj:DRIVERS/SOUND.OBJ:$(KERNEL)/drivers/sound.asm \
 drivers/vmode.mdr:DRIVERS/VMODE.MDR:$(KERNEL)/drivers/vmode.asm \
 drivers/vt8235.obj:DRIVERS/VT8235.OBJ:$(KERNEL)/drivers/vt823x.asm \
 File|Managers/kfar:File|Managers/KFAR:$(PROGS)/fs/kfar/trunk/kfar.asm \
 File|Managers/OpenDial:File|Managers/OPENDIAL:$(PROGS)/develop/libraries/box_lib/asm/trunk/OpenDial.asm \
 fonts/bgitest:FONTS/BGITEST:$(PROGS)/demos/bgitest/trunk/bgitest.asm \
 games/15:GAMES/15:$(PROGS)/games/15/trunk/15.ASM \
 games/arcanii:GAMES/ARCANII:$(PROGS)/games/arcanii/trunk/arcanii.asm \
 games/freecell:GAMES/FREECELL:$(PROGS)/games/freecell/freecell.ASM \
 games/invaders:GAMES/INVADERS:$(PROGS)/games/invaders/invaders.asm \
 games/lines:GAMES/LINES:$(PROGS)/games/lines/lines.asm \
 games/mblocks:GAMES/MBLOCKS:$(PROGS)/games/mblocks/trunk/mblocks.asm \
 games/phenix:GAMES/PHENIX:$(PROGS)/games/phenix/trunk/phenix.asm \
 games/pipes:GAMES/PIPES:$(PROGS)/games/pipes/pipes.asm \
 games/pong:GAMES/PONG:$(PROGS)/games/pong/trunk/pong.asm \
 games/pong3:GAMES/PONG3:$(PROGS)/games/pong3/trunk/pong3.asm \
 games/rsquare:GAMES/RSQUARE:$(PROGS)/games/rsquare/trunk/rsquare.asm \
 games/snake:GAMES/SNAKE:$(PROGS)/games/snake/trunk/snake.asm \
 games/tetris:GAMES/TETRIS:$(PROGS)/games/tetris/trunk/tetris.asm \
 lib/archiver.obj:LIB/ARCHIVER.OBJ:$(PROGS)/fs/kfar/trunk/kfar_arc/kfar_arc.asm \
 lib/box_lib.obj:LIB/BOX_LIB.OBJ:$(PROGS)/develop/libraries/box_lib/trunk/box_lib.asm \
 lib/console.obj:LIB/CONSOLE.OBJ:$(PROGS)/develop/libraries/console/console.asm \
 lib/libgfx.obj:LIB/LIBGFX.OBJ:$(PROGS)/develop/libraries/libs-dev/libgfx/libgfx.asm \
 lib/libimg.obj:LIB/LIBIMG.OBJ:$(PROGS)/develop/libraries/libs-dev/libimg/libimg.asm \
 lib/libini.obj:LIB/LIBINI.OBJ:$(PROGS)/develop/libraries/libs-dev/libini/libini.asm \
 lib/libio.obj:LIB/LIBIO.OBJ:$(PROGS)/develop/libraries/libs-dev/libio/libio.asm \
 lib/network.obj:LIB/NETWORK.OBJ:$(PROGS)/develop/libraries/network/network.asm \
 lib/proc_lib.obj:LIB/PROC_LIB.OBJ:$(PROGS)/develop/libraries/proc_lib/trunk/proc_lib.asm \
 media/animage:MEDIA/ANIMAGE:$(PROGS)/media/animage/trunk/animage.asm \
 media/cdp:MEDIA/CDP:$(PROGS)/media/cdp/trunk/cdp.asm \
 media/kiv:MEDIA/KIV:$(PROGS)/media/kiv/trunk/kiv.asm \
 media/midamp:MEDIA/MIDAMP:$(PROGS)/media/midamp/trunk/midamp.asm \
 network/airc:NETWORK/AIRC:$(PROGS)/network/airc/trunk/airc.asm \
 network/arpstat:NETWORK/ARPSTAT:$(PROGS)/network/arpstat/trunk/arpstat.asm \
 network/chess:NETWORK/CHESS:$(PROGS)/network/chess/trunk/chess.asm \
 network/ethstat:NETWORK/ETHSTAT:$(PROGS)/network/ethstat/trunk/ethstat.asm \
 network/ftps:NETWORK/FTPS:$(PROGS)/network/ftps/trunk/FTPS.ASM \
 network/httpc:NETWORK/HTTPC:$(PROGS)/network/httpc/trunk/httpc.asm \
 network/https:NETWORK/HTTPS:$(PROGS)/network/https/trunk/https.asm \
 network/nntpc:NETWORK/NNTPC:$(PROGS)/network/nntpc/trunk/nntpc.asm \
 network/nslookup:NETWORK/NSLOOKUP:$(PROGS)/develop/libraries/network/examples/nslookup.asm \
 network/popc:NETWORK/POPC:$(PROGS)/network/popc/trunk/popc.asm \
 network/smtps:NETWORK/SMTPS:$(PROGS)/network/smtps/trunk/smtps.asm \
 network/stackcfg:NETWORK/STACKCFG:$(PROGS)/network/stackcfg/trunk/stackcfg.asm \
 network/telnet:NETWORK/TELNET:$(PROGS)/network/telnet/trunk/telnet.asm \
 network/tftpc:NETWORK/TFTPC:$(PROGS)/network/tftpc/trunk/tftpc.asm \
 network/VNCclient:NETWORK/VNCclient:$(PROGS)/network/VNCclient/VNCclient.asm \
 network/ym:NETWORK/YM:$(PROGS)/network/ym/trunk/ym.asm \
 network/zeroconf:NETWORK/ZEROCONF:$(PROGS)/network/zeroconf/trunk/zeroconf.asm \
# end of list

# The list of all NASM programs with one main NASM file.
# Format of an item is exactly the same as in the previous list.
NASM_PROGRAMS:=\
 demos/aclock:DEMOS/ACLOCK:$(PROGS)/demos/aclock/trunk/aclock.asm \
 games/c4:GAMES/C4:$(PROGS)/games/c4/trunk/c4.asm \
# end of list

# The list of files which should be copied from somewhere.
# Format of an item is exactly the same as in the previous list.
COPY_FILES:=\
 macros.inc:MACROS.INC:$(PROGS)/macros.inc \
 config.inc:CONFIG.INC:$(PROGS)/config.inc \
 struct.inc:STRUCT.INC:$(PROGS)/struct.inc \
 File|Managers/z_icons.png:File|Managers/Z_ICONS.PNG:$(PROGS)/develop/libraries/box_lib/asm/trunk/z_icons.png \
 fonts/euro.chr:FONTS/EURO.CHR:$(PROGS)/demos/bgitest/trunk/FONTS/EURO.CHR \
 fonts/goth.chr:FONTS/GOTH.CHR:$(PROGS)/demos/bgitest/trunk/FONTS/GOTH.CHR \
 fonts/lcom.chr:FONTS/LCOM.CHR:$(PROGS)/demos/bgitest/trunk/FONTS/LCOM.CHR \
 fonts/litt.chr:FONTS/LITT.CHR:$(PROGS)/demos/bgitest/trunk/FONTS/LITT.CHR \
 fonts/sans.chr:FONTS/SANS.CHR:$(PROGS)/demos/bgitest/trunk/FONTS/SANS.CHR \
 fonts/scri.chr:FONTS/SCRI.CHR:$(PROGS)/demos/bgitest/trunk/FONTS/SCRI.CHR \
 fonts/simp.chr:FONTS/SIMP.CHR:$(PROGS)/demos/bgitest/trunk/FONTS/SIMP.CHR \
 fonts/smal.chr:FONTS/SMAL.CHR:$(PROGS)/demos/bgitest/trunk/FONTS/SMAL.CHR \
 fonts/trip.chr:FONTS/TRIP.CHR:$(PROGS)/demos/bgitest/trunk/FONTS/TRIP.CHR \
 fonts/tscr.chr:FONTS/TSCR.CHR:$(PROGS)/demos/bgitest/trunk/FONTS/TSCR.CHR

# List of other files to be included in the image file.
# Some of them are auto-built with special rules, some just exist before build.
# Each item is of the form <local name>:<name inside image>.
# Spaces should be represented as |.
OTHER_FILES:=asciivju:ASCIIVJU autorun.dat:AUTORUN.DAT \
 background.gif:background.gif default.skn:DEFAULT.SKN \
 e80:E80 graph:GRAPH hdread:HDREAD htmlv:HTMLV \
 icons.dat:ICONS.DAT iconstrp.gif:ICONSTRP.GIF index_htm:INDEX.HTM \
 kernel.mnt:KERNEL.MNT kerpack:KERPACK keymap.key:KEYMAP.KEY kpack:KPACK \
 kuzkina.mid:KUZKINA.MID lang.inc:LANG.INC lang.ini:LANG.INI \
 menu.dat:MENU.DAT mykey:MYKEY @notify:@NOTIFY \
 panel.dat:PANEL.DAT pipetka:PIPETKA setup.dat:SETUP.DAT \
 shell:SHELL spanel:SPANEL table:TABLE test:TEST \
 vmode:VMODE \
 3d/3dsheart:3D/3DSHEART 3d/3dspiral:3D/3DSPIRAL 3d/cubeline:3D/CUBELINE \
 3d/flatwav:3D/FLATWAV 3d/gears:3D/GEARS 3d/house.3ds:3D/HOUSE.3DS \
 demos/ak47.lif:DEMOS/AK47.LIF \
 demos/barge.lif:DEMOS/BARGE.LIF demos/life2:DEMOS/LIFE2 \
 demos/relay.lif:DEMOS/RELAY.LIF demos/rpento.lif:DEMOS/RPENTO.LIF \
 demos/use_mb:DEMOS/USE_MB demos/web:DEMOS/WEB \
 drivers/atikms.dll:DRIVERS/ATIKMS.DLL \
 File|Managers/buttons.bmp:File|Managers/BUTTONS.BMP \
 File|Managers/eolite:File|Managers/EOLITE \
 File|Managers/eolite.ini:File|Managers/EOLITE.INI \
 File|Managers/icons.bmp:File|Managers/ICONS.BMP \
 File|Managers/icons.ini:File|Managers/ICONS.INI \
 File|Managers/kfar.ini:File|Managers/KFAR.INI \
 File|Managers/kfm:File|Managers/KFM \
 File|Managers/kfm.ini:File|Managers/KFM.INI \
 File|Managers/kfm_keys.txt:File|Managers/KFM_KEYS.TXT \
 fonts/bold.chr:FONTS/BOLD.CHR fonts/char2.mt:FONTS/CHAR2.MT \
 fonts/char.mt:FONTS/CHAR.MT \
 games/appdata.dat:GAMES/APPDATA.DAT games/ataka:GAMES/ATAKA \
 games/basekurs.kla:GAMES/BASEKURS.KLA games/bnc:GAMES/BNC \
 games/checkers:GAMES/CHECKERS games/clicks:GAMES/CLICKS \
 games/FindNumbers:GAMES/FindNumbers games/gomoku:GAMES/GOMOKU \
 games/klavisha:GAMES/KLAVISHA games/kosilka:GAMES/KOSILKA \
 games/kosskin.gfx:GAMES/KOSSKIN.GFX games/kox:GAMES/KOX \
 games/lights:GAMES/LIGHTS games/megamaze:GAMES/MEGAMAZE games/mine:GAMES/MINE \
 games/msquare:GAMES/MSQUARE games/padenie:GAMES/PADENIE \
 games/reversi:GAMES/REVERSI games/rforces:GAMES/RFORCES \
 games/snake.ini:GAMES/SNAKE.INI games/sq_game:GAMES/SQ_GAME \
 games/sudoku:GAMES/SUDOKU games/sw:GAMES/SW games/whowtbam:GAMES/WHOWTBAM \
 games/xonix:GAMES/XONIX \
 lib/cnv_png.obj:LIB/CNV_PNG.OBJ lib/msgbox.obj:LIB/MSGBOX.OBJ \
 lib/pixlib.obj:LIB/PIXLIB.OBJ lib/sort.obj:LIB/SORT.OBJ \
 media/ac97snd:MEDIA/AC97SND media/kiv.ini:MEDIA/KIV.INI \
 media/listplay:MEDIA/LISTPLAY media/startmus:MEDIA/STARTMUS \
 network/jmail:NETWORK/JMAIL network/zeroconf.ini:NETWORK/ZEROCONF.INI \
#end of list
#3d/logio.bmp:3D/LOGIO.BMP 3d/cubetext:3D/CUBETEXT

# Some macro for convenient work.
# Macros for replacing '|' to escaped space '\ '.
space:=\ #plus space
respace=$(subst |,$(space),$(1))
# Macro for selecting different parts of ':'-separated items.
binarypart=$(word 1,$(subst :, ,$(1)))
imagepart=$(word 2,$(subst :, ,$(1)))
sourcepart=$(word 3,$(subst :, ,$(1)))
# Get file names, possibly with spaces inside, from an item.
# Here $(f) is an item - in fact, macro argument.
fbinary=$(call respace,$(call binarypart,$(f)))
fimage=$(call respace,$(call imagepart,$(f)))
fsource=$(call respace,$(call sourcepart,$(f)))

# Define targets for image file.
# Join all the lists above.
targets_full:=$(COPY_FILES) $(FASM_PROGRAMS) $(NASM_PROGRAMS) $(OTHER_FILES)
# For each item in the united list call fbinary.
targets:=$(foreach f,$(targets_full),$(fbinary))

# Define a command for copying a file inside the image.
# mcopy_command is a macro with two parameters,
# local file name $(1) and image file name $(2).
# Note that spaces in these have to be escaped with backslashes.
mcopy_command=mcopy -mi $(BUILD_DIR)/kolibri.img $(1) ::$(2)
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

# The main goal: image.
$(BUILD_DIR)/kolibri.img: $(BUILD_DIR)/.dir \
 Makefile \
 $(BUILD_DIR)/boot_fat12.bin \
 $(targets)
# SYSXTREE
# 3d/CUBETEXT
# 3d/LOGIO.BMP
	dd if=/dev/zero of=$(BUILD_DIR)/kolibri.img count=2880 bs=512 2>&1
	mformat -f 1440 -i $(BUILD_DIR)/kolibri.img ::
	dd if=$(BUILD_DIR)/boot_fat12.bin of=$(BUILD_DIR)/kolibri.img count=1 bs=512 conv=notrunc 2>&1
	mmd -i $(BUILD_DIR)/kolibri.img ::3D
	mmd -i $(BUILD_DIR)/kolibri.img ::DEMOS
	mmd -i $(BUILD_DIR)/kolibri.img ::DEVELOP
	mmd -i $(BUILD_DIR)/kolibri.img ::DRIVERS
	mmd -i $(BUILD_DIR)/kolibri.img ::File\ Managers
	mmd -i $(BUILD_DIR)/kolibri.img ::FONTS
	mmd -i $(BUILD_DIR)/kolibri.img ::GAMES
	mmd -i $(BUILD_DIR)/kolibri.img ::LIB
	mmd -i $(BUILD_DIR)/kolibri.img ::MEDIA
	mmd -i $(BUILD_DIR)/kolibri.img ::NETWORK
	$(mcopy_all_items)

# Special target to modify behaviour of make.
.DELETE_ON_ERROR:

# The floppy bootsector.
$(BUILD_DIR)/boot_fat12.bin: $(KERNEL)/bootloader/boot_fat12.asm $(KERNEL)/bootloader/floppy1440.inc
	fasm $(KERNEL)/bootloader/boot_fat12.asm $(BUILD_DIR)/boot_fat12.bin

$(BUILD_DIR)/.dir 3d/.dir demos/.dir develop/.dir drivers/.dir fonts/.dir \
 games/.dir lib/.dir media/.dir network/.dir .deps/.dir:
	mkdir -p $(dir $@)
	touch $@
File\ Managers/.dir:
	mkdir -p "File Managers"
	touch "File Managers/.dir"

# FASM black magic goes to Makefile.fasm.
include Makefile.fasm

# Similar for NASM.
include Makefile.nasm

# Similar for copying files.
include Makefile.copy

# Special rules for copying sysfuncr.txt - it isn't directly included in the image.
docpak: $(DOCDIR)SYSFUNCR.TXT $(wildcard $(DOCDIR)*)
$(DOCDIR)SYSFUNCR.TXT: $(KERNEL)/docs/sysfuncr.txt
	cp $(KERNEL)/docs/sysfuncr.txt $(DOCDIR)SYSFUNCR.TXT

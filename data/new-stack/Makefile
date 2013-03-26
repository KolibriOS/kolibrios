# General rule for naming: variables with CAPITALIZED names hold settings,
# you can - and are expected - to modify it; variables with lowercase names
# are intermediate variables and macroses not to be modified unless you
# know what you're doing.

# Define directories for destination, source repository, sources of kernel, sources of programs.
BUILD_DIR:=build
REPOSITORY:=../..
KERNEL:=$(REPOSITORY)/kernel/branches/net
TRUNKKERNEL:=$(REPOSITORY)/kernel/trunk
PROGS:=$(REPOSITORY)/programs
NETPROGS:=$(REPOSITORY)/kernel/branches/net/applications

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
 @panel:@PANEL:$(PROGS)/system/panel/trunk/@PANEL.ASM \
 @clip:@CLIP:$(PROGS)/system/clip/trunk/@clip.ASM \
 @menu:@MENU:$(PROGS)/system/menu/trunk/menu.asm \
 @ss:@SS:$(PROGS)/system/ss/trunk/@ss.asm\
 refrscrn:REFRSCRN:$(PROGS)/system/refrscrn/refrscrn.asm \
 asciivju:ASCIIVJU:$(PROGS)/develop/asciivju/trunk/asciivju.asm \
 calc:CALC:$(PROGS)/other/calc/trunk/calc.asm \
 calendar:CALENDAR:$(PROGS)/system/calendar/trunk/calendar.asm \
 colrdial:COLRDIAL:$(PROGS)/system/colrdial/color_dialog.asm \
 commouse:COMMOUSE:$(PROGS)/system/commouse/trunk/commouse.asm \
 cpu:CPU:$(PROGS)/system/cpu/trunk/cpu.asm \
 cpuid:CPUID:$(PROGS)/system/cpuid/trunk/CPUID.ASM \
 desktop:DESKTOP:$(PROGS)/system/desktop/trunk/desktop.asm \
 disptest:DISPTEST:$(PROGS)/system/disptest/trunk/disptest.ASM \
 docpack:DOCPACK:$(PROGS)/system/docpack/trunk/docpack.asm \
 end:END:$(PROGS)/system/end/light/end.asm \
 gmon:GMON:$(PROGS)/system/gmon/gmon.asm \
 hdd_info:HDD_INFO:$(PROGS)/system/hdd_info/trunk/hdd_info.asm \
 icon:ICON:$(PROGS)/system/icon/trunk/icon.asm \
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
 scrshoot:SCRSHOOT:$(PROGS)/media/scrshoot/scrshoot.asm \
 setup:SETUP:$(PROGS)/system/setup/trunk/setup.asm \
 test:TEST:$(PROGS)/system/test/trunk/test.asm \
 tinypad:TINYPAD:$(PROGS)/develop/tinypad/trunk/tinypad.asm \
 zkey:ZKEY:$(PROGS)/system/zkey/trunk/ZKEY.ASM \
 develop/board:DEVELOP/BOARD:$(PROGS)/system/board/trunk/board.asm \
 develop/cObj:DEVELOP/cObj:$(PROGS)/develop/cObj/trunk/cObj.asm \
 develop/fasm:DEVELOP/FASM:$(PROGS)/develop/fasm/trunk/fasm.asm \
 develop/h2d2b:DEVELOP/H2D2B:$(PROGS)/develop/h2d2b/trunk/h2d2b.asm \
 develop/heed:DEVELOP/HEED:$(PROGS)/develop/heed/trunk/heed.asm \
 develop/ipc:DEVELOP/IPC:$(PROGS)/network/ipc/trunk/ipc.asm \
 develop/keyascii:DEVELOP/KEYASCII:$(PROGS)/develop/keyascii/trunk/keyascii.asm \
 develop/mtdbg:DEVELOP/MTDBG:$(PROGS)/develop/mtdbg/mtdbg.asm \
 develop/scancode:DEVELOP/SCANCODE:$(PROGS)/develop/scancode/trunk/scancode.asm \
 develop/t_edit:DEVELOP/T_EDIT:$(PROGS)/other/t_edit/t_edit.asm \
 develop/test_gets:DEVELOP/test_gets:$(PROGS)/develop/libraries/console/examples/test_gets.asm \
 develop/testcon2:DEVELOP/TESTCON2:$(PROGS)/develop/libraries/console/examples/testcon2.asm \
 develop/thread:DEVELOP/THREAD:$(PROGS)/develop/examples/thread/trunk/thread.asm \
 drivers/com_mouse.obj:DRIVERS/COM_MOUSE.OBJ:$(TRUNKKERNEL)/drivers/com_mouse.asm \
 drivers/emu10k1x.obj:DRIVERS/EMU10K1X.OBJ:$(TRUNKKERNEL)/drivers/emu10k1x.asm \
 drivers/fm801.obj:DRIVERS/FM801.OBJ:$(TRUNKKERNEL)/drivers/fm801.asm \
 drivers/infinity.obj:DRIVERS/INFINITY.OBJ:$(TRUNKKERNEL)/drivers/infinity.asm \
 drivers/ps2mouse.obj:DRIVERS/PS2MOUSE.OBJ:$(REPOSITORY)/drivers/mouse/ps2mouse4d/trunk/ps2mouse.asm \
 drivers/sb16.obj:DRIVERS/SB16.OBJ:$(TRUNKKERNEL)/drivers/sb16/sb16.asm \
 drivers/sound.obj:DRIVERS/SOUND.OBJ:$(TRUNKKERNEL)/drivers/sound.asm \
 drivers/intelac97.obj:DRIVERS/INTELAC97.OBJ:$(TRUNKKERNEL)/drivers/intelac97.asm \
 drivers/vt823x.obj:DRIVERS/VT823X.OBJ:$(TRUNKKERNEL)/drivers/vt823x.asm \
 drivers/tmpdisk.obj:DRIVERS/TMPDISK.OBJ:$(TRUNKKERNEL)/drivers/tmpdisk.asm \
 File|Managers/kfar:File|Managers/KFAR:$(PROGS)/fs/kfar/trunk/kfar.asm \
 File|Managers/kfm:File|Managers/KFM:$(PROGS)/fs/kfm/trunk/kfm.asm \
 File|Managers/opendial:File|Managers/OPENDIAL:$(PROGS)/fs/opendial/opendial.asm \
 develop/info/asm.syn:DEVELOP/INFO/ASM.SYN:$(PROGS)/other/t_edit/info/asm_syn.asm \
 develop/info/cpp_kol_cla.syn:DEVELOP/INFO/CPP_KOL_CLA.SYN:$(PROGS)/other/t_edit/info/cpp_kol_cla_syn.asm \
 develop/info/cpp_kol_dar.syn:DEVELOP/INFO/CPP_KOL_DAR.SYN:$(PROGS)/other/t_edit/info/cpp_kol_dar_syn.asm \
 develop/info/cpp_kol_def.syn:DEVELOP/INFO/CPP_KOL_DEF.SYN:$(PROGS)/other/t_edit/info/cpp_kol_def_syn.asm \
 develop/info/default.syn:DEVELOP/INFO/DEFAULT.SYN:$(PROGS)/other/t_edit/info/default_syn.asm \
 develop/info/html.syn:DEVELOP/INFO/HTML.SYN:$(PROGS)/other/t_edit/info/html_syn.asm \
 develop/info/ini_files.syn:DEVELOP/INFO/INI_FILES.SYN:$(PROGS)/other/t_edit/info/ini_files_syn.asm \
 develop/info/win_const.syn:DEVELOP/INFO/WIN_CONST.SYN:$(PROGS)/other/t_edit/info/win_const_syn.asm \
 lib/archiver.obj:LIB/ARCHIVER.OBJ:$(PROGS)/fs/kfar/trunk/kfar_arc/kfar_arc.asm \
 lib/box_lib.obj:LIB/BOX_LIB.OBJ:$(PROGS)/develop/libraries/box_lib/trunk/box_lib.asm \
 lib/buf2d.obj:LIB/BUF2D.OBJ:$(PROGS)/develop/libraries/buf2d/trunk/buf2d.asm \
 lib/console.obj:LIB/CONSOLE.OBJ:$(PROGS)/develop/libraries/console/console.asm \
 lib/libgfx.obj:LIB/LIBGFX.OBJ:$(PROGS)/develop/libraries/libs-dev/libgfx/libgfx.asm \
 lib/libimg.obj:LIB/LIBIMG.OBJ:$(PROGS)/develop/libraries/libs-dev/libimg/libimg.asm \
 lib/libini.obj:LIB/LIBINI.OBJ:$(PROGS)/develop/libraries/libs-dev/libini/libini.asm \
 lib/libio.obj:LIB/LIBIO.OBJ:$(PROGS)/develop/libraries/libs-dev/libio/libio.asm \
 lib/proc_lib.obj:LIB/PROC_LIB.OBJ:$(PROGS)/develop/libraries/proc_lib/trunk/proc_lib.asm \
 lib/cnv_png.obj:LIB/CNV_PNG.OBJ:$(PROGS)/media/zsea/plugins/png/cnv_png.asm \
 media/animage:MEDIA/ANIMAGE:$(PROGS)/media/animage/trunk/animage.asm \
 media/cdp:MEDIA/CDP:$(PROGS)/media/cdp/trunk/cdp.asm \
 media/kiv:MEDIA/KIV:$(PROGS)/media/kiv/trunk/kiv.asm \
 media/listplay:MEDIA/LISTPLAY:$(PROGS)/media/listplay/trunk/listplay.asm \
 media/midamp:MEDIA/MIDAMP:$(PROGS)/media/midamp/trunk/midamp.asm \
 media/palitra:MEDIA/PALITRA:$(PROGS)/media/palitra/trunk/palitra.asm \
 media/startmus:MEDIA/STARTMUS:$(PROGS)/media/startmus/trunk/STARTMUS.ASM \
 network/arpcfg:NETWORK/ARPCFG:$(NETPROGS)/arpcfg/arpcfg.asm \
 network/ping:NETWORK/PING:$(NETPROGS)/icmp/ping.asm \
 network/netcfg:NETWORK/NETCFG:$(NETPROGS)/netcfg/netcfg.asm \
 network/netstat:NETWORK/NETSTAT:$(NETPROGS)/netstat/netstat.asm \
 network/nslookup:NETWORK/NSLOOKUP:$(NETPROGS)/nslookup/nslookup.asm \
 network/synergyc:NETWORK/SYNERGYC:$(NETPROGS)/synergyc/synergyc.asm \
 network/tcpserv:NETWORK/TCPSERV:$(NETPROGS)/tcpserv/tcpserv.asm \
 network/telnet:NETWORK/TELNET:$(NETPROGS)/telnet/telnet.asm \
 network/zeroconf:NETWORK/ZEROCONF:$(NETPROGS)/zeroconf/zeroconf.asm \
 network/ftpd:NETWORK/FTPD:$(NETPROGS)/ftpd/ftpd.asm \
 network/tftpc:NETWORK/TFTPC:$(NETPROGS)/tftpc/tftpc.asm \
 lib/network.obj:LIB/NETWORK.OBJ:$(NETPROGS)/libraries/network/network.asm \
 drivers/3c59x.obj:DRIVERS/3C59X.OBJ:$(KERNEL)/drivers/3c59x.asm \
 drivers/dec21x4x.obj:DRIVERS/DEC21X4X.OBJ:$(KERNEL)/drivers/dec21x4x.asm \
 drivers/i8255x.obj:DRIVERS/I8255X.OBJ:$(KERNEL)/drivers/i8255x.asm \
 drivers/mtd80x.obj:DRIVERS/MTD80X.OBJ:$(KERNEL)/drivers/mtd80x.asm \
 drivers/pcnet32.obj:DRIVERS/PCNET32.OBJ:$(KERNEL)/drivers/pcnet32.asm \
 drivers/r6040.obj:DRIVERS/R6040.OBJ:$(KERNEL)/drivers/R6040.asm \
 drivers/rtl8029.obj:DRIVERS/RTL8029.OBJ:$(KERNEL)/drivers/RTL8029.asm \
 drivers/rtl8139.obj:DRIVERS/RTL8139.OBJ:$(KERNEL)/drivers/RTL8139.asm \
 drivers/rtl8169.obj:DRIVERS/RTL8169.OBJ:$(KERNEL)/drivers/RTL8169.asm \
 drivers/sis900.obj:DRIVERS/SIS900.OBJ:$(KERNEL)/drivers/sis900.asm \
 drivers/i8254x.obj:DRIVERS/I8254X.OBJ:$(KERNEL)/drivers/i8254x.asm \
 drivers/rhine.obj:DRIVERS/RHINE.OBJ:$(KERNEL)/drivers/rhine.asm \
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
 media/kiv.ini:MEDIA/KIV.INI:$(PROGS)/media/kiv/trunk/kiv.ini \
 .shell:.shell:$(PROGS)/system/shell/bin/eng/.shell \
 mykey.ini:MYKEY.INI:$(PROGS)/system/MyKey/trunk/mykey.ini \
# end of list

# The list of all C-- programs with one main C-- file.
# Format of an item is exactly the same as in the previous list,
# except that there can be fourth field with parameters for a compiler.
CMM_PROGRAMS:=\
 File|Managers/eolite:File|Managers/EOLITE:$(PROGS)/cmm/eolite/Eolite.c \
 HTMLv:HTMLv:$(PROGS)/cmm/browser/HTMLv.c \
 tmpdisk:tmpdisk:$(PROGS)/cmm/tmpdisk/tmpdisk.c \
 @notify:@notify:$(PROGS)/cmm/notify/notify.c \
 @rb:@rb:$(PROGS)/cmm/rb/rb.c \
#develop/c--:DEVELOP/C--:$(PROGS)/develop/c--/trunk/32.c-- \
# end of list

# List of other files to be included in the image file.
# Some of them are auto-built with special rules, some just exist before build.
# Each item is of the form <local name>:<name inside image>.
# Spaces should be represented as |.
OTHER_FILES:=autorun.dat:AUTORUN.DAT \
 background.png:background.png default.skn:DEFAULT.SKN \
 icons.dat:ICONS.DAT iconstrp.png:ICONSTRP.PNG index_htm:INDEX.HTM \
 kernel.mnt:KERNEL.MNT kerpack:KERPACK keymap.key:KEYMAP.KEY \
 lang.inc:LANG.INC lang.ini:LANG.INI \
 menu.dat:MENU.DAT \
 panel.ini:PANEL.INI setup.dat:SETUP.DAT \
 shell:SHELL \
 vmode:VMODE \
 File|Managers/eolite.ini:File|Managers/EOLITE.INI \
 File|Managers/icons.ini:File|Managers/ICONS.INI \
 File|Managers/kfar.ini:File|Managers/KFAR.INI \
 File|Managers/kfm.ini:File|Managers/KFM.INI \
 fonts/char2.mt:FONTS/CHAR2.MT \
 fonts/char.mt:FONTS/CHAR.MT \
 lib/msgbox.obj:LIB/MSGBOX.OBJ \
 lib/pixlib.obj:LIB/PIXLIB.OBJ lib/sort.obj:LIB/SORT.OBJ \
 media/ac97snd:MEDIA/AC97SND \
 network/zeroconf.ini:NETWORK/ZEROCONF.INI \
 network/ftpd.ini:NETWORK/FTPD.INI \
 network/users.ini:NETWORK/USERS.INI \
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

# dependencies of MKISOFS_EXTRA; we iterate through $(MKISOFS_EXTRA),
# substitute "=" with space, get the 2nd word and join all results
mkisofs_extra_targets:=$(foreach f,$(MKISOFS_EXTRA),$(word 2,$(subst =, ,$(f))))

# The main goal: build kolibri.img and kolibri.iso
all: $(BUILD_DIR)/kolibri.img

# The first goal: floppy image.
$(BUILD_DIR)/kolibri.img: $(BUILD_DIR)/.dir \
 Makefile \
 $(BUILD_DIR)/boot_fat12.bin \
 $(targets)
# SYSXTREE
# 3d/CUBETEXT
# 3d/LOGIO.BMP
	str=`date -u +"[auto-build %d %b %Y %R, r$(REV)]"`; \
	echo -n $$str|dd of=kernel.mnt bs=1 seek=`expr 279 - length "$$str"` conv=notrunc 2>/dev/null
	dd if=/dev/zero of=$(BUILD_DIR)/kolibri.img count=2880 bs=512 2>&1
	mformat -f 1440 -i $(BUILD_DIR)/kolibri.img ::
	dd if=$(BUILD_DIR)/boot_fat12.bin of=$(BUILD_DIR)/kolibri.img count=1 bs=512 conv=notrunc 2>&1
	mmd -i $(BUILD_DIR)/kolibri.img ::DEVELOP
	mmd -i $(BUILD_DIR)/kolibri.img ::DEVELOP/INFO
	mmd -i $(BUILD_DIR)/kolibri.img ::DRIVERS
	mmd -i $(BUILD_DIR)/kolibri.img ::File\ Managers
	mmd -i $(BUILD_DIR)/kolibri.img ::FONTS
	mmd -i $(BUILD_DIR)/kolibri.img ::LIB
	mmd -i $(BUILD_DIR)/kolibri.img ::MEDIA
	mmd -i $(BUILD_DIR)/kolibri.img ::NETWORK
	$(mcopy_all_items)

# Special targets to modify behaviour of make.
.DELETE_ON_ERROR:
.SUFFIXES: # delete all predefined rules

# The floppy bootsector.
$(BUILD_DIR)/boot_fat12.bin: $(TRUNKKERNEL)/bootloader/boot_fat12.asm $(TRUNKKERNEL)/bootloader/floppy1440.inc
	fasm $(TRUNKKERNEL)/bootloader/boot_fat12.asm $(BUILD_DIR)/boot_fat12.bin

$(BUILD_DIR)/.dir develop/.dir develop/info/.dir drivers/.dir fonts/.dir \
 lib/.dir media/.dir network/.dir .deps/.dir:
	mkdir -p $(dir $@)
	touch $@
develop/info/.dir: develop/.dir
File\ Managers/.dir:
	mkdir -p "File Managers"
	touch "File Managers/.dir"

# FASM black magic goes to Makefile.fasm.
include Makefile.fasm

# Similar for NASM.
include Makefile.nasm

# Similar for copying files.
include Makefile.copy

# Special rules for copying sysfuncs.txt - it isn't directly included in the image.
docpack: $(DOCDIR)SYSFUNCS.TXT
$(DOCDIR)SYSFUNCS.TXT: $(TRUNKKERNEL)/docs/sysfuncs.txt
	cp $(TRUNKKERNEL)/docs/sysfuncs.txt $(DOCDIR)SYSFUNCS.TXT

# Similar for C--.
include Makefile.cmm

# Sorry, even black magic seems to be insufficient for
# auto-handling all subtle effects. So we just define
# command lines for compiling and linking, and
# maintain the list of sources and objects by hand.
include Makefile.msvc
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

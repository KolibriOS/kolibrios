-- Do nothing unless explicitly requested in tup.config.
build_type = tup.getconfig('BUILD_TYPE')
if build_type == "" then
  return
end

--[================================[ DATA ]================================]--

PROGS = "../programs"

-- Static data that doesn't need to be compiled
-- Files to be included in kolibri.img.
-- The first subitem of every item is name inside kolibri.img, the second is name of local file.
img_files = {
 {"MACROS.INC", PROGS .. "/macros.inc"},
 {"CONFIG.INC", PROGS .. "/config.inc"},
 {"STRUCT.INC", PROGS .. "/struct.inc"},
 {"DEVELOP/TE_ICON.PNG", PROGS .. "/other/t_edit/te_icon.png"},
 {"DEVELOP/TL_NOD_16.PNG", PROGS .. "/other/t_edit/tl_nod_16.png"},
 {"DEVELOP/TL_SYS_16.PNG", PROGS .. "/media/log_el/trunk/tl_sys_16.png"},
 {"DEVELOP/T_EDIT.INI", PROGS .. "/other/t_edit/t_edit.ini"},
 {"File Managers/Z_ICONS.PNG", PROGS .. "/fs/opendial/z_icons.png"},
 {"File Managers/BUTTONS.BMP", PROGS .. "/fs/kfm/trunk/buttons.bmp"},
 {"File Managers/ICONS.BMP", PROGS .. "/fs/kfm/trunk/icons.bmp"},
 {"FONTS/LITT.CHR", PROGS .. "/demos/bgitest/trunk/FONTS/LITT.CHR"},
 {"GAMES/SNAKE.INI", PROGS .. "/games/snake/trunk/snake.ini"},
 {"MEDIA/KIV.INI", PROGS .. "/media/kiv/trunk/kiv.ini"},
 {"MEDIA/PIXIE/PIXIE.INI", PROGS .. "/cmm/pixie/pixie.ini"},
 {"MEDIA/PIXIE/S_DARK.PNG", PROGS .. "/cmm/pixie/s_dark.png"},
 {"MEDIA/PIXIE/S_LIGHT.PNG", PROGS .. "/cmm/pixie/s_light.png"},
 {"MEDIA/PIXIE/MINIMP3", PROGS .. "/cmm/pixie/minimp3"},
 {"NETWORK/WV_SKIN.PNG", PROGS .. "/cmm/browser/wv_skin.png"},
 {"SETTINGS/AUTORUN.DAT", build_type .. "/settings/autorun.dat"},
 {"DEFAULT.SKN", "common/default.skn"},
 {"SETTINGS/ICON.INI", build_type .. "/settings/icon.ini"},
 {"ICONS32.PNG", "common/icons32.png"},
 {"INDEX.HTM", build_type .. "/index_htm"},
 {"KERPACK", "common/kerpack"},
 {"SETTINGS/KEYMAP.KEY", build_type .. "/settings/keymap.key"},
 {"SETTINGS/LANG.INI", build_type .. "/settings/lang.ini"},
 {"KUZKINA.MID", "common/kuzkina.mid"},
 {"LANG.INC", build_type .. "/lang.inc"},
 {"SETTINGS/MENU.DAT", build_type .. "/settings/menu.dat"},
 {"SETTINGS/KOLIBRI.LBL", build_type .. "/settings/kolibri.lbl"},
 {"SETTINGS/TASKBAR.INI", build_type .. "/settings/taskbar.ini"},
 {"SETTINGS/DOCKY.INI", "common/settings/docky.ini"},
 {"SETTINGS/ASSOC.INI", "common/settings/assoc.ini"},
 {"SETTINGS/SYSTEM_PANEL.INI", "common/settings/system_panel.ini"},
 {"NOTIFY3.PNG", "common/notify3.png"},
 {"SETTINGS/SETUP.DAT", build_type .. "/settings/setup.dat"},
 {"VMODE", "common/vmode"},
 {"3D/HOUSE.3DS", "common/3d/house.3ds"},
 {"DEMOS/AK47.LIF", "common/demos/ak47.lif"},
 {"DEMOS/LIFE2", "common/demos/life2"},
 {"DEMOS/RELAY.LIF", "common/demos/relay.lif"},
 {"DEMOS/RPENTO.LIF", "common/demos/rpento.lif"},
 {"File Managers/EOLITE.INI", "common/File Managers/eolite.ini"},
 {"File Managers/ICONS.INI", "common/File Managers/icons.ini"},
 {"File Managers/KFM.INI", "common/File Managers/kfm.ini"},
 {"LIB/PIXLIB.OBJ", "common/lib/pixlib.obj"},
 {"LIB/ICONV.OBJ", "common/lib/iconv.obj"},
 {"LIB/NETCODE.OBJ", "common/lib/netcode.obj"},
 {"LIB/KMENU.OBJ", "common/lib/kmenu.obj"},
 {"MEDIA/IMGF/IMGF", "common/media/ImgF/ImgF"},
 {"MEDIA/IMGF/CEDG.OBJ", "common/media/ImgF/cEdg.obj"},
 {"MEDIA/IMGF/DITHER.OBJ", "common/media/ImgF/dither.obj"},
 {"MEDIA/IMGF/INVSOL.OBJ", "common/media/ImgF/invSol.obj"},
 {"SETTINGS/NETWORK.INI", build_type .. "/settings/network.ini"},
 {"NETWORK/FTPD.INI", "common/network/ftpd.ini"},
 {"NETWORK/USERS.INI", "common/network/users.ini"},
}
-- For russian build, add russian-only files.
if build_type == "rus" then tup.append_table(img_files, {
 {"File Managers/KFM_KEYS.TXT", PROGS .. "/fs/kfm/trunk/docs/russian/dos_kolibri/kfm_keys.txt"},
 {"SETTINGS/.shell", PROGS .. "/system/shell/bin/rus/.shell"},
 {"SETTINGS/GAME_CENTER.INI", "rus/settings/game_center.ini"},
 {"SETTINGS/MYKEY.INI", PROGS .. "/system/MyKey/trunk/mykey.ini"},
 {"EXAMPLE.ASM", PROGS .. "/develop/examples/example/trunk/rus/example.asm"},
 {"PIPETKA", build_type .. "/pipetka"},
 {"File Managers/KFAR.INI", build_type .. "/File Managers/kfar.ini"},
 {"GAMES/APPDATA.DAT", build_type .. "/games/appdata.dat"},
 {"GAMES/ATAKA", build_type .. "/games/ataka"},
 {"GAMES/BASEKURS.KLA", build_type .. "/games/basekurs.kla"},
 {"GAMES/PADENIE", build_type .. "/games/padenie"},
 {"GAMES/WHOWTBAM", build_type .. "/games/whowtbam"},
}) else tup.append_table(img_files, {
 {"File Managers/KFM_KEYS.TXT", PROGS .. "/fs/kfm/trunk/docs/english/kfm_keys.txt"},
 {"SETTINGS/GAME_CENTER.INI", "common/settings/game_center.ini"},
 {"GAMES/SKIN.RAW", PROGS .. "/games/soko/trunk/SKIN.RAW"},
 {"GAMES/SOKO-4.LEV", PROGS .. "/games/soko/trunk/SOKO-4.LEV"},
 {"GAMES/SOKO-5.LEV", PROGS .. "/games/soko/trunk/SOKO-5.LEV"},
 {"GAMES/SOKO-6.LEV", PROGS .. "/games/soko/trunk/SOKO-6.LEV"},
 {"GAMES/SOKO-7.LEV", PROGS .. "/games/soko/trunk/SOKO-7.LEV"},
 {"GAMES/SOKO-8.LEV", PROGS .. "/games/soko/trunk/SOKO-8.LEV"},
 {"GAMES/SOKO-9.LEV", PROGS .. "/games/soko/trunk/SOKO-9.LEV"},
 {"SETTINGS/.shell", PROGS .. "/system/shell/bin/eng/.shell"},
 {"EXAMPLE.ASM", PROGS .. "/develop/examples/example/trunk/example.asm"},
 {"File Managers/KFAR.INI", "common/File Managers/kfar.ini"},
}) end
if build_type == "it" then tup.append_table(img_files, {
 {"SETTINGS/MYKEY.INI", PROGS .. "/system/MyKey/trunk/mykey_it.ini"},
}) else tup.append_table(img_files, {
 {"SETTINGS/MYKEY.INI", PROGS .. "/system/MyKey/trunk/mykey.ini"},
}) end

--[[
Files to be included in kolibri.iso and distribution kit outside of kolibri.img.

The first subitem of every item is name relative to the root of ISO or distribution kit,
the second is name of local file.

If the first subitem ends in /, the last component of local file name is appended.
The last component of the second subitem may contain '*'; if so, it will be expanded
according to usual rules, but without matching directories.

Tup does not allow a direct dependency on a file that is generated in a directory
other than where Tupfile.lua is and its children. Most files are generated
in the directory with Tupfile.lua; for other files, the item should contain
a named subitem "group=path/<groupname>" and the file should be put in <groupname>.
--]]
extra_files = {
 {"/", build_type .. "/distr_data/autorun.inf"},
 {"/", build_type .. "/distr_data/KolibriOS_icon.ico"},
 {"/", build_type .. "/settings/kolibri.lbl"},
 {"Skins/", "../skins/authors.txt"},
 {"Docs/stack.txt", build_type .. "/docs/STACK.TXT"},
 {"HD_Load/9x2klbr/", "common/HD_load/9x2klbr/LDKLBR.VXD"},
 {"HD_Load/MeOSLoad/", PROGS .. "/hd_load/meosload/AUTOEXEC.BAT"},
 {"HD_Load/MeOSLoad/", PROGS .. "/hd_load/meosload/CONFIG.SYS"},
 {"HD_Load/MeOSLoad/", PROGS .. "/hd_load/meosload/L_readme.txt"},
 {"HD_Load/MeOSLoad/", PROGS .. "/hd_load/meosload/L_readme_Win.txt"},
 {"HD_Load/mtldr/", PROGS .. "/hd_load/mtldr/vista_install.bat"},
 {"HD_Load/mtldr/", PROGS .. "/hd_load/mtldr/vista_remove.bat"},
 {"HD_Load/", "common/HD_load/memdisk"},
 {"HD_Load/USB_boot_old/", PROGS .. "/hd_load/usb_boot_old/usb_boot.rtf"},
 {"HD_Load/USB_boot_old/", PROGS .. "/hd_load/usb_boot_old/usb_boot_866.txt"},
 {"HD_Load/USB_boot_old/", PROGS .. "/hd_load/usb_boot_old/usb_boot_1251.txt"},
 {"kolibrios/emul/dosbox/", "common/emul/DosBox/*"},
 {"kolibrios/emul/fceu/", "common/emul/fceu/*"},
 {"kolibrios/emul/", "common/emul/gameboy"},
 {"kolibrios/emul/", "common/emul/scummvm"},
 {"kolibrios/emul/", "common/emul/zsnes"},
 {"kolibrios/games/doom/", "common/games/doom/*"},
 {"kolibrios/games/pig/", "common/games/pig/*"},
 {"kolibrios/games/fara/fara.gfx", "common/games/fara.gfx"},
 {"kolibrios/games/jumpbump/", "common/games/jumpbump/*"},
 {"kolibrios/games/LaserTank/", "common/games/LaserTank/*"},
 {"kolibrios/games/lrl/", "common/games/lrl/*"},
 {"kolibrios/games/megamaze", build_type .. "/games/megamaze"},
 {"kolibrios/games/phenix", PROGS .. "/games/phenix/trunk/phenix"},
 {"kolibrios/games/soko/soko", build_type .. "/games/soko"},
 {"kolibrios/games/soko/", "common/games/soko/*"},
 {"kolibrios/drivers/", "common/drivers/*"},
 {"kolibrios/lib/avcodec-55.dll", "common/lib/avcodec-55.dll"},
 {"kolibrios/lib/avdevice-55.dll", "common/lib/avdevice-55.dll"},
 {"kolibrios/lib/avformat-55.dll", "common/lib/avformat-55.dll"},
 {"kolibrios/lib/avutil-52.dll", "common/lib/avutil-52.dll"},
 {"kolibrios/lib/freetype.dll", "common/lib/freetype.dll"},
 {"kolibrios/lib/i965-video.dll", "common/lib/i965-video.dll"},
 {"kolibrios/lib/libdrm.dll", "common/lib/libdrm.dll"},
 {"kolibrios/lib/libegl.dll", "common/lib/libegl.dll"},
 {"kolibrios/lib/libGL.dll", "common/lib/libGL.dll"},
 {"kolibrios/lib/libva.dll", "common/lib/libva.dll"},
 {"kolibrios/lib/libz.dll", "common/lib/libz.dll"},
 {"kolibrios/lib/libc.dll", "../contrib/sdk/bin/libc.dll", group = "../contrib/sdk/lib/<libc.dll.a>"},
 {"kolibrios/lib/pixlib-gl.dll", "common/lib/pixlib-gl.dll"},
 {"kolibrios/lib/swresample-0.dll", "common/lib/swresample-0.dll"},
 {"kolibrios/lib/i915_dri.drv", "common/lib/i915_dri.drv"},
 {"kolibrios/lib/i965_dri.drv", "common/lib/i965_dri.drv"},
 {"kolibrios/media/fplay", "common/media/fplay"},
 {"kolibrios/media/zsea/zsea.ini", PROGS .. "/media/zsea/zSea.ini"},
 {"kolibrios/media/zsea/buttons/buttons.png", PROGS .. "/media/zsea/buttons.png"},
}
if build_type == "rus" then tup.append_table(extra_files, {
 {"Docs/cp866/config.txt", build_type .. "/docs/CONFIG.TXT"},
 {"Docs/cp866/gnu.txt", build_type .. "/docs/GNU.TXT"},
 {"Docs/cp866/history.txt", build_type .. "/docs/HISTORY.TXT"},
 {"Docs/cp866/hot_keys.txt", build_type .. "/docs/HOT_KEYS.TXT"},
 {"Docs/cp866/install.txt", build_type .. "/docs/INSTALL.TXT"},
 {"Docs/cp866/readme.txt", build_type .. "/docs/README.TXT"},
 {"Docs/cp866/sysfuncr.txt", PROGS .. "/system/docpack/trunk/SYSFUNCR.TXT"},
 {"Docs/cp1251/config.txt", build_type .. "/docs/CONFIG.WIN.TXT", cp1251_from = build_type .. "/docs/CONFIG.TXT"},
 {"Docs/cp1251/gnu.txt", build_type .. "/docs/GNU.WIN.TXT", cp1251_from = build_type .. "/docs/GNU.TXT"},
 {"Docs/cp1251/history.txt", build_type .. "/docs/HISTORY.WIN.TXT", cp1251_from = build_type .. "/docs/HISTORY.TXT"},
 {"Docs/cp1251/hot_keys.txt", build_type .. "/docs/HOT_KEYS.WIN.TXT", cp1251_from = build_type .. "/docs/HOT_KEYS.TXT"},
 {"Docs/cp1251/install.txt", build_type .. "/docs/INSTALL.WIN.TXT", cp1251_from = build_type .. "/docs/INSTALL.TXT"},
 {"Docs/cp1251/readme.txt", build_type .. "/docs/README.WIN.TXT", cp1251_from = build_type .. "/docs/README.TXT"},
 {"Docs/cp1251/sysfuncr.txt", build_type .. "/docs/SYSFUNCR.WIN.TXT", cp1251_from = PROGS .. "/system/docpack/trunk/SYSFUNCR.TXT"},
 {"HD_Load/9x2klbr/", PROGS .. "/hd_load/9x2klbr/readme_dos.txt"},
 {"HD_Load/9x2klbr/", PROGS .. "/hd_load/9x2klbr/readme_win.txt"},
 {"HD_Load/mtldr/", PROGS .. "/hd_load/mtldr/install.txt"},
 {"HD_Load/USB_Boot/", PROGS .. "/hd_load/usb_boot/readme.txt"},
 {"kolibrios/media/zsea/zsea_keys.txt", PROGS .. "/media/zsea/Docs/zSea_keys_rus.txt"},
 {"kolibrios/games/Dungeons/Resources/Textures/Environment/", PROGS .. "/games/Dungeons/Resources/Textures/Environment/*"},
 {"kolibrios/games/Dungeons/Resources/Textures/Objects/", PROGS .. "/games/Dungeons/Resources/Textures/Objects/*"},
 {"kolibrios/games/Dungeons/Resources/Textures/HUD/", PROGS .. "/games/Dungeons/Resources/Textures/HUD/*"},
 {"kolibrios/games/Dungeons/Resources/Textures/", PROGS .. "/games/Dungeons/Resources/Textures/Licenses.txt"},
 {"kolibrios/games/Dungeons/", PROGS .. "/games/Dungeons/readme_ru.txt"},
}) else tup.append_table(extra_files, {
 {"Docs/config.txt", build_type .. "/docs/CONFIG.TXT"},
 {"Docs/copying.txt", build_type .. "/docs/COPYING.TXT"},
 {"Docs/hot_keys.txt", build_type .. "/docs/HOT_KEYS.TXT"},
 {"Docs/install.txt", build_type .. "/docs/INSTALL.TXT"},
 {"Docs/readme.txt", build_type .. "/docs/README.TXT"},
 {"Docs/sysfuncs.txt", PROGS .. "/system/docpack/trunk/SYSFUNCS.TXT"},
 {"HD_Load/9x2klbr/", PROGS .. "/hd_load/9x2klbr/readme.txt"},
 {"HD_Load/mtldr/install.txt", PROGS .. "/hd_load/mtldr/install_eng.txt"},
 {"HD_Load/USB_Boot/readme.txt", PROGS .. "/hd_load/usb_boot/readme_eng.txt"},
 {"kolibrios/media/zsea/zsea_keys.txt", PROGS .. "/media/zsea/Docs/zSea_keys_eng.txt"},
}) end
--[[
Files to be included in distribution kit outside of kolibri.img, but not kolibri.iso.
Same syntax as extra_files.
]]--
if build_type == "rus" then
distr_extra_files = {
 {"/readme_dos.txt", build_type .. "/distr_data/readme_dos_distr.txt"},
 {"/readme.txt", build_type .. "/distr_data/readme_distr.txt", cp1251_from = build_type .. "/distr_data/readme_dos_distr.txt"},
}
else
distr_extra_files = {
 {"/readme.txt", build_type .. "/distr_data/readme_distr.txt"},
}
end
--[[
Files to be included in kolibri.iso outside of kolibri.img, but not distribution kit.
Same syntax as extra_files.
]]--
if build_type == "rus" then
iso_extra_files = {
 {"/readme_dos.txt", build_type .. "/distr_data/readme_dos.txt"},
 {"/readme.txt", build_type .. "/distr_data/readme.txt", cp1251_from = build_type .. "/distr_data/readme_dos.txt"},
}
else
iso_extra_files = {
 {"/readme.txt", build_type .. "/distr_data/readme.txt"},
}
end

-- Programs that require FASM to compile.
if tup.getconfig('NO_FASM') ~= 'full' then
tup.append_table(img_files, {
 {"KERNEL.MNT", "../kernel/trunk/kernel.mnt"},
 {"@MENU", PROGS .. "/system/menu/trunk/menu"},
 {"@VOLUME", PROGS .. "/media/volume/volume"},
 {"@TASKBAR", PROGS .. "/system/taskbar/trunk/TASKBAR"},
 {"@DOCKY", PROGS .. "/system/docky/trunk/docky"},
 {"@OPEN", PROGS .. "/system/open/open"},
 {"@NOTIFY", PROGS .. "/system/notify3/notify"},
 {"@SS", PROGS .. "/system/ss/trunk/@ss"},
 {"REFRSCRN", PROGS .. "/system/refrscrn/refrscrn"},
 {"ASCIIVJU", PROGS .. "/develop/asciivju/trunk/asciivju"},
 {"CALC", PROGS .. "/other/calc/trunk/calc"},
 {"CALENDAR", PROGS .. "/system/calendar/trunk/calendar"},
 {"COLRDIAL", PROGS .. "/system/colrdial/color_dialog"},
 {"LOADDRV", PROGS .. "/system/loaddrv/loaddrv"},
 {"CPU", PROGS .. "/system/cpu/trunk/cpu"},
 {"CPUID", PROGS .. "/system/cpuid/trunk/CPUID"},
 {"DESKTOP", PROGS .. "/system/desktop/trunk/desktop"},
 {"DISPTEST", PROGS .. "/system/disptest/trunk/disptest"},
 {"DOCPACK", PROGS .. "/system/docpack/trunk/docpack"},
 {"END", PROGS .. "/system/end/light/end"},
 {"FSPEED", PROGS .. "/fs/fspeed/fspeed"},
 {"GMON", PROGS .. "/system/gmon/gmon"},
 {"HDD_INFO", PROGS .. "/system/hdd_info/trunk/hdd_info"},
 {"@ICON", PROGS .. "/system/icon_new/@icon"},
 {"CROPFLAT", PROGS .. "/system/cropflat/cropflat"},
 {"KBD", PROGS .. "/system/kbd/trunk/kbd"},
 {"KPACK", PROGS .. "/other/kpack/trunk/kpack"},
 {"LAUNCHER", PROGS .. "/system/launcher/trunk/launcher"},
 {"MAGNIFY", PROGS .. "/demos/magnify/trunk/magnify"},
 {"MGB", PROGS .. "/system/mgb/trunk/mgb"},
 {"MOUSEMUL", PROGS .. "/system/mousemul/trunk/mousemul"},
 {"MADMOUSE", PROGS .. "/other/madmouse/madmouse"},
 {"MYKEY", PROGS .. "/system/MyKey/trunk/MyKey"},
 {"PCIDEV", PROGS .. "/system/pcidev/trunk/PCIDEV"},
 {"RDSAVE", PROGS .. "/system/rdsave/trunk/rdsave"},
 {"RTFREAD", PROGS .. "/other/rtfread/trunk/rtfread"},
 {"RUN", PROGS .. "/system/run/trunk/run"},
 {"SEARCHAP", PROGS .. "/system/searchap/searchap"},
 {"SCRSHOOT", PROGS .. "/media/scrshoot/scrshoot"},
 {"SETUP", PROGS .. "/system/setup/trunk/setup"},
 {"TEST", PROGS .. "/system/test/trunk/test"},
 {"TINYPAD", PROGS .. "/develop/tinypad/trunk/tinypad"},
 {"ZKEY", PROGS .. "/system/zkey/trunk/ZKEY"},
 {"TERMINAL", PROGS .. "/system/terminal/terminal"},
 {"3D/3DSHEART", PROGS .. "/demos/3dsheart/trunk/3dsheart"},
 {"3D/3DWAV", PROGS .. "/demos/3dwav/trunk/3dwav"},
 {"3D/CROWNSCR", PROGS .. "/demos/crownscr/trunk/crownscr"},
 {"3D/FREE3D04", PROGS .. "/demos/free3d04/trunk/free3d04"},
 {"3D/VIEW3DS", PROGS .. "/demos/3DS/VIEW3DS"},
 {"DEMOS/BCDCLK", PROGS .. "/demos/bcdclk/trunk/bcdclk"},
 {"DEMOS/CIRCLE", PROGS .. "/develop/examples/circle/trunk/circle"},
 {"DEMOS/COLORREF", PROGS .. "/demos/colorref/trunk/colorref"},
 {"DEMOS/CSLIDE", PROGS .. "/demos/cslide/trunk/cslide"},
 {"DEMOS/EYES", PROGS .. "/demos/eyes/trunk/eyes"},
 {"DEMOS/FIREWORK", PROGS .. "/demos/firework/trunk/firework"},
 {"DEMOS/MOVBACK", PROGS .. "/demos/movback/trunk/movback"},
 {"DEMOS/PLASMA", PROGS .. "/demos/plasma/trunk/plasma"},
 {"DEMOS/TINYFRAC", PROGS .. "/demos/tinyfrac/trunk/tinyfrac"},
 {"DEMOS/TRANTEST", PROGS .. "/demos/trantest/trunk/trantest"},
 {"DEMOS/TUBE", PROGS .. "/demos/tube/trunk/tube"},
 {"DEMOS/UNVWATER", PROGS .. "/demos/unvwater/trunk/unvwater"},
 {"DEMOS/USE_MB", PROGS .. "/demos/use_mb/use_mb"},
 {"DEMOS/WEB", PROGS .. "/demos/web/trunk/web"},
 {"DEVELOP/BOARD", PROGS .. "/system/board/trunk/board"},
 {"DEVELOP/cObj", PROGS .. "/develop/cObj/trunk/cObj"},
 {"DEVELOP/FASM", PROGS .. "/develop/fasm/trunk/fasm"},
 {"DEVELOP/H2D2B", PROGS .. "/develop/h2d2b/trunk/h2d2b"},
 {"DEVELOP/HEED", PROGS .. "/develop/heed/trunk/heed"},
 {"DEVELOP/KEYASCII", PROGS .. "/develop/keyascii/trunk/keyascii"},
 {"DEVELOP/MTDBG", PROGS .. "/develop/mtdbg/mtdbg"},
 {"DEVELOP/SCANCODE", PROGS .. "/develop/scancode/trunk/scancode"},
 {"DEVELOP/T_EDIT", PROGS .. "/other/t_edit/t_edit"},
 {"DEVELOP/test_gets", PROGS .. "/develop/libraries/console/examples/test_gets"},
 {"DEVELOP/THREAD", PROGS .. "/develop/examples/thread/trunk/thread"},
 {"DEVELOP/INFO/ASM.SYN", PROGS .. "/other/t_edit/info/asm.syn"},
 {"DEVELOP/INFO/CPP_KOL_CLA.SYN", PROGS .. "/other/t_edit/info/cpp_kol_cla.syn"},
 {"DEVELOP/INFO/CPP_KOL_DAR.SYN", PROGS .. "/other/t_edit/info/cpp_kol_dar.syn"},
 {"DEVELOP/INFO/CPP_KOL_DEF.SYN", PROGS .. "/other/t_edit/info/cpp_kol_def.syn"},
 {"DEVELOP/INFO/DEFAULT.SYN", PROGS .. "/other/t_edit/info/default.syn"},
 {"DEVELOP/INFO/HTML.SYN", PROGS .. "/other/t_edit/info/html.syn"},
 {"DEVELOP/INFO/INI_FILES.SYN", PROGS .. "/other/t_edit/info/ini_files.syn"},
 {"DEVELOP/INFO/WIN_CONST.SYN", PROGS .. "/other/t_edit/info/win_const.syn"},
 {"File Managers/KFAR", PROGS .. "/fs/kfar/trunk/kfar"},
 {"File Managers/KFM", PROGS .. "/fs/kfm/trunk/kfm"},
 {"File Managers/OPENDIAL", PROGS .. "/fs/opendial/opendial"},
 {"GAMES/15", PROGS .. "/games/15/trunk/15"},
 {"GAMES/ARCANII", PROGS .. "/games/arcanii/trunk/arcanii"},
 {"GAMES/FREECELL", PROGS .. "/games/freecell/freecell"},
 {"GAMES/GOMOKU", PROGS .. "/games/gomoku/trunk/gomoku"},
 {"GAMES/KLAVISHA", PROGS .. "/games/klavisha/trunk/klavisha"},
 {"GAMES/LINES", PROGS .. "/games/lines/lines"},
 {"GAMES/MBLOCKS", PROGS .. "/games/mblocks/trunk/mblocks"},
 {"GAMES/MSQUARE", PROGS .. "/games/MSquare/trunk/MSquare"},
 {"GAMES/PIPES", PROGS .. "/games/pipes/pipes"},
 {"GAMES/PONG", PROGS .. "/games/pong/trunk/pong"},
 {"GAMES/PONG3", PROGS .. "/games/pong3/trunk/pong3"},
 {"GAMES/RSQUARE", PROGS .. "/games/rsquare/trunk/rsquare"},
 {"GAMES/SNAKE", PROGS .. "/games/snake/trunk/snake"},
 {"GAMES/SQ_GAME", PROGS .. "/games/sq_game/trunk/SQ_GAME"},
 {"GAMES/SUDOKU", PROGS .. "/games/sudoku/trunk/sudoku"},
 {"GAMES/SW", PROGS .. "/games/sw/trunk/sw"},
 {"GAMES/TANKS", PROGS .. "/games/tanks/trunk/tanks"},
 {"GAMES/TETRIS", PROGS .. "/games/tetris/trunk/tetris"},
 {"LIB/ARCHIVER.OBJ", PROGS .. "/fs/kfar/trunk/kfar_arc/kfar_arc.obj"},
 {"LIB/BOX_LIB.OBJ", PROGS .. "/develop/libraries/box_lib/trunk/box_lib.obj"},
 {"LIB/BUF2D.OBJ", PROGS .. "/develop/libraries/buf2d/trunk/buf2d.obj"},
 {"LIB/CONSOLE.OBJ", PROGS .. "/develop/libraries/console/console.obj"},
 {"LIB/LIBGFX.OBJ", PROGS .. "/develop/libraries/libs-dev/libgfx/libgfx.obj"},
 {"LIB/LIBIMG.OBJ", PROGS .. "/develop/libraries/libs-dev/libimg/libimg.obj"},
 {"LIB/LIBINI.OBJ", PROGS .. "/develop/libraries/libs-dev/libini/libini.obj"},
 {"LIB/LIBIO.OBJ", PROGS .. "/develop/libraries/libs-dev/libio/libio.obj"},
 {"LIB/MSGBOX.OBJ", PROGS .. "/develop/libraries/msgbox/msgbox.obj"},
 {"LIB/NETWORK.OBJ", PROGS .. "/develop/libraries/network/network.obj"},
 {"LIB/SORT.OBJ", PROGS .. "/develop/libraries/sorter/sort.obj"},
 {"LIB/HTTP.OBJ", PROGS .. "/develop/libraries/http/http.obj"},
 {"LIB/PROC_LIB.OBJ", PROGS .. "/develop/libraries/proc_lib/trunk/proc_lib.obj"},
 {"LIB/CNV_PNG.OBJ", PROGS .. "/media/zsea/plugins/png/cnv_png.obj"},
 {"MEDIA/ANIMAGE", PROGS .. "/media/animage/trunk/animage"},
 {"MEDIA/KIV", PROGS .. "/media/kiv/trunk/kiv"},
 {"MEDIA/LISTPLAY", PROGS .. "/media/listplay/trunk/listplay"},
 {"MEDIA/MIDAMP", PROGS .. "/media/midamp/trunk/midamp"},
 {"MEDIA/PALITRA", PROGS .. "/media/palitra/trunk/palitra"},
 {"MEDIA/STARTMUS", PROGS .. "/media/startmus/trunk/STARTMUS"},
 {"NETWORK/PING", PROGS .. "/network/icmp/ping"},
 {"NETWORK/NETCFG", PROGS .. "/network/netcfg/netcfg"},
 {"NETWORK/NETSTAT", PROGS .. "/network/netstat/netstat"},
 {"NETWORK/NSLOOKUP", PROGS .. "/network/nslookup/nslookup"},
 {"NETWORK/PASTA", PROGS .. "/network/pasta/pasta"},
 {"NETWORK/SYNERGYC", PROGS .. "/network/synergyc/synergyc"},
 {"NETWORK/TCPSERV", PROGS .. "/network/tcpserv/tcpserv"},
 {"NETWORK/TELNET", PROGS .. "/network/telnet/telnet"},
 {"NETWORK/@ZEROCONF", PROGS .. "/network/zeroconf/zeroconf"},
 {"NETWORK/FTPC", PROGS .. "/network/ftpc/ftpc"},
 {"NETWORK/FTPD", PROGS .. "/network/ftpd/ftpd"},
 {"NETWORK/TFTPC", PROGS .. "/network/tftpc/tftpc"},
 {"NETWORK/IRCC", PROGS .. "/network/ircc/ircc"},
 {"NETWORK/DOWNLOADER", PROGS .. "/network/downloader/downloader"},
 {"DRIVERS/VIDINTEL.SYS", "../drivers/video/vidintel.sys"},
 {"DRIVERS/3C59X.SYS", "../drivers/ethernet/3c59x.sys"},
 {"DRIVERS/DEC21X4X.SYS", "../drivers/ethernet/dec21x4x.sys"},
 {"DRIVERS/FORCEDETH.SYS", "../drivers/ethernet/forcedeth.sys"},
 {"DRIVERS/I8254X.SYS", "../drivers/ethernet/i8254x.sys"},
 {"DRIVERS/I8255X.SYS", "../drivers/ethernet/i8255x.sys"},
 {"DRIVERS/MTD80X.SYS", "../drivers/ethernet/mtd80x.sys"},
 {"DRIVERS/PCNET32.SYS", "../drivers/ethernet/pcnet32.sys"},
 {"DRIVERS/R6040.SYS", "../drivers/ethernet/R6040.sys"},
 {"DRIVERS/RHINE.SYS", "../drivers/ethernet/rhine.sys"},
 {"DRIVERS/RTL8029.SYS", "../drivers/ethernet/RTL8029.sys"},
 {"DRIVERS/RTL8139.SYS", "../drivers/ethernet/RTL8139.sys"},
 {"DRIVERS/RTL8169.SYS", "../drivers/ethernet/RTL8169.sys"},
 {"DRIVERS/SIS900.SYS", "../drivers/ethernet/sis900.sys"},
 {"DRIVERS/UHCI.SYS", "../drivers/usb/uhci.sys"},
 {"DRIVERS/OHCI.SYS", "../drivers/usb/ohci.sys"},
 {"DRIVERS/EHCI.SYS", "../drivers/usb/ehci.sys"},
 {"DRIVERS/USBHID.SYS", "../drivers/usb/usbhid/usbhid.sys"},
 {"DRIVERS/USBSTOR.SYS", "../drivers/usb/usbstor.sys"},
 {"DRIVERS/RDC.SYS", "../drivers/video/rdc.sys"},
 {"DRIVERS/COMMOUSE.SYS", "../drivers/mouse/commouse.sys"},
 {"DRIVERS/PS2MOUSE.SYS", "../drivers/mouse/ps2mouse4d/trunk/ps2mouse.sys"},
 {"DRIVERS/TMPDISK.SYS", "../drivers/disk/tmpdisk.sys"},
 {"DRIVERS/intel_hda.sys", "../drivers/audio/intel_hda/intel_hda.sys"},
 {"DRIVERS/SB16.SYS", "../drivers/audio/sb16/sb16.sys"},
 {"DRIVERS/SOUND.SYS", "../drivers/audio/sound.sys"},
 {"DRIVERS/INFINITY.SYS", "../drivers/audio/infinity/infinity.sys"},
 {"DRIVERS/INTELAC97.SYS", "../drivers/audio/intelac97.sys"},
 {"DRIVERS/EMU10K1X.SYS", "../drivers/audio/emu10k1x.sys"},
 {"DRIVERS/FM801.SYS", "../drivers/audio/fm801.sys"},
 {"DRIVERS/VT823X.SYS", "../drivers/audio/vt823x.sys"},
 {"DRIVERS/SIS.SYS", "../drivers/audio/sis.sys"},
})
tup.append_table(extra_files, {
 {"HD_Load/9x2klbr/", PROGS .. "/hd_load/9x2klbr/9x2klbr.exe"},
 {"HD_Load/MeOSLoad/", PROGS .. "/hd_load/meosload/MeOSload.com"},
 {"HD_Load/mtldr/", PROGS .. "/hd_load/mtldr/mtldr"},
 {"HD_Load/", PROGS .. "/hd_load/mtldr_install/mtldr_install.exe"},
 {"HD_Load/USB_Boot/", PROGS .. "/hd_load/usb_boot/BOOT_F32.BIN"},
 {"HD_Load/USB_Boot/", PROGS .. "/hd_load/usb_boot/MTLD_F32"},
 {"HD_Load/USB_Boot/", PROGS .. "/hd_load/usb_boot/inst.exe"},
 {"HD_Load/USB_Boot/", PROGS .. "/hd_load/usb_boot/setmbr.exe"},
 {"HD_Load/USB_boot_old/", PROGS .. "/hd_load/usb_boot_old/MeOSload.com"},
 {"HD_Load/USB_boot_old/", PROGS .. "/hd_load/usb_boot_old/enable.exe"},
 {"kolibrios/games/codemaster/binary_master", PROGS .. "/games/codemaster/binary_master"},
 {"kolibrios/games/codemaster/hang_programmer", PROGS .. "/games/codemaster/hang_programmer"},
 {"kolibrios/games/codemaster/kolibri_puzzle", PROGS .. "/games/codemaster/kolibri_puzzle"},
 {"kolibrios/games/invaders", PROGS .. "/games/invaders/invaders"},
 {"kolibrios/media/zsea/zsea", PROGS .. "/media/zsea/zSea"},
 {"kolibrios/media/zsea/plugins/cnv_bmp.obj", PROGS .. "/media/zsea/plugins/bmp/cnv_bmp.obj"},
 {"kolibrios/media/zsea/plugins/cnv_gif.obj", PROGS .. "/media/zsea/plugins/gif/cnv_gif.obj"},
 {"kolibrios/media/zsea/plugins/cnv_jpeg.obj", PROGS .. "/media/zsea/plugins/jpeg/cnv_jpeg.obj"},
 {"kolibrios/media/zsea/plugins/convert.obj", PROGS .. "/media/zsea/plugins/convert/convert.obj"},
 {"kolibrios/media/zsea/plugins/rotate.obj", PROGS .. "/media/zsea/plugins/rotate/rotate.obj"},
 {"kolibrios/media/zsea/plugins/scaling.obj", PROGS .. "/media/zsea/plugins/scaling/scaling.obj"},
})
-- For russian build, add russian-only programs.
if build_type == "rus" then tup.append_table(img_files, {
 {"PERIOD", PROGS .. "/other/period/trunk/period"},
 {"DEVELOP/TESTCON2", PROGS .. "/develop/libraries/console/examples/testcon2_rus"},
}) else tup.append_table(img_files, {
 {"DEVELOP/TESTCON2", PROGS .. "/develop/libraries/console/examples/testcon2_eng"},
 {"GAMES/SOKO", PROGS .. "/games/soko/trunk/SOKO"},
}) end

if build_type == "rus" then tup.append_table(extra_files, {
 {"kolibrios/games/Dungeons/Dungeons", PROGS .. "/games/Dungeons/Dungeons"},
}) end

end -- tup.getconfig('NO_FASM') ~= 'full'

-- Programs that require NASM to compile.
if tup.getconfig('NO_NASM') ~= 'full' then
tup.append_table(img_files, {
 {"DEMOS/ACLOCK", PROGS .. "/demos/aclock/trunk/aclock"},
 {"DEMOS/TIMER", PROGS .. "/other/Timer/timer"},
 {"GAMES/C4", PROGS .. "/games/c4/trunk/c4"},
 {"TINFO", PROGS .. "/system/tinfo/tinfo"},
 {"DEVELOP/MSTATE", PROGS .. "/develop/mstate/mstate"},
})
end -- tup.getconfig('NO_NASM') ~= 'full'

-- Programs that require C-- to compile.
if tup.getconfig('NO_CMM') ~= 'full' then
tup.append_table(img_files, {
 {"File Managers/EOLITE", PROGS .. "/cmm/eolite/Eolite.com"},
 {"GAMES/CLICKS", PROGS .. "/games/clicks/trunk/clicks.com"},
 {"GAMES/FindNumbers", PROGS .. "/games/FindNumbers/trunk/FindNumbers"},
 {"GAMES/flood-it", PROGS .. "/games/flood-it/trunk/flood-it.com"},
 {"GAMES/MINE", PROGS .. "/games/mine/trunk/mine"},
 {"MEDIA/PIXIE/PIXIE", PROGS .. "/cmm/pixie/pixie.com"},
 {"MOUSE_CFG", PROGS .. "/cmm/mouse_cfg/mouse_cfg.com"},
 {"NETWORK/WEBVIEW", PROGS .. "/cmm/browser/WebView.com"},
 {"PANELS_CFG", PROGS .. "/cmm/panels_cfg/panels_cfg.com"},
 {"TMPDISK", PROGS .. "/cmm/tmpdisk/tmpdisk.com"},
 {"GAME_CENTER", PROGS .. "/cmm/software_widget/software_widget.com"},
 {"SYSTEM_PANEL", PROGS .. "/cmm/software_widget/software_widget.com"},
})
end -- tup.getconfig('NO_CMM') ~= 'full'

-- Programs that require MSVC to compile.
if tup.getconfig('NO_MSVC') ~= 'full' then
tup.append_table(img_files, {
 {"GRAPH", PROGS .. "/other/graph/graph"},
 {"TABLE", PROGS .. "/other/table/table"},
 {"MEDIA/AC97SND", PROGS .. "/media/ac97snd/ac97snd.bin"},
 {"GAMES/KOSILKA", PROGS .. "/games/kosilka/kosilka"},
 {"GAMES/RFORCES", PROGS .. "/games/rforces/trunk/rforces"},
 {"GAMES/XONIX", PROGS .. "/games/xonix/trunk/xonix"},
})
tup.append_table(extra_files, {
 {"kolibrios/games/fara/fara", PROGS .. "/games/fara/trunk/fara"},
 {"kolibrios/games/LaserTank/LaserTank", PROGS .. "/games/LaserTank/trunk/LaserTank"},
})
end -- tup.getconfig('NO_MSVC') ~= 'full'

-- Programs that require GCC to compile.
if tup.getconfig('NO_GCC') ~= 'full' then
tup.append_table(img_files, {
 {"3D/CUBELINE", PROGS .. "/demos/cubeline/trunk/cubeline"},
 {"3D/GEARS", PROGS .. "/demos/gears/trunk/gears"},
 {"GAMES/CHECKERS", PROGS .. "/games/checkers/trunk/checkers"},
 {"GAMES/REVERSI", PROGS .. "/games/reversi/trunk/reversi"},
 {"SHELL", PROGS .. "/system/shell/shell"},
})
tup.append_table(extra_files, {
 {"kolibrios/emul/e80/e80", PROGS .. "/emulator/e80/trunk/e80"},
 {"kolibrios/games/heliothryx/", PROGS .. "/games/heliothryx/heliothryx"},
 {"kolibrios/games/2048/", PROGS .. "/games/2048/2048"},
 {"kolibrios/games/marblematch3/", PROGS .. "/games/marblematch3/marblematch3"},
 {"kolibrios/games/nsider/", PROGS .. "/games/nsider/nsider"},
 {"kolibrios/games/quake/", "common/games/quake/*"}, -- not really gcc, but no sense without sdlquake
 {"kolibrios/games/quake/", "../contrib/other/sdlquake-1.0.9/sdlquake"},
})
-- For russian build, add russian-only programs.
if build_type == "rus" then tup.append_table(extra_files, {
 {"kolibrios/games/21days/", PROGS .. "/games/21days/21days"},
}) end
end -- tup.getconfig('NO_GCC') ~= 'full'

-- Skins.
tup.include("../skins/skinlist.lua")

--[================================[ CODE ]================================]--
-- expand extra_files and similar
function expand_extra_files(files)
  local result = {}
  for i,v in ipairs(files) do
    if string.match(v[2], "%*")
    then
      local g = tup.glob(v[2])
      for j,x in ipairs(g) do
        table.insert(result, {v[1], x, group=v.group})
      end
    else
      if v.cp1251_from then
        tup.definerule{inputs = {v.cp1251_from}, command = 'iconv -f cp866 -t cp1251 "%f" > "%o"', outputs = {v[2]}}
      end
      table.insert(result, {v[1], v[2], group=v.group})
    end
  end
  return result
end

-- append skins to extra_files
for i,v in ipairs(skinlist) do
  table.insert(extra_files, {"Skins/", "../skins/" .. v})
end

-- prepare distr_extra_files and iso_extra_files: expand and append common part
extra_files = expand_extra_files(extra_files)
distr_extra_files = expand_extra_files(distr_extra_files)
iso_extra_files = expand_extra_files(iso_extra_files)
tup.append_table(distr_extra_files, extra_files)
tup.append_table(iso_extra_files, extra_files)

-- generate list of directories to be created inside kolibri.img
img_dirs = {}
input_deps = {}
for i,v in ipairs(img_files) do
  img_file = v[1]
  local_file = v[2]

  slash_pos = 0
  while true do
    slash_pos = string.find(img_file, '/', slash_pos + 1)
    if not slash_pos then break end
    table.insert(img_dirs, string.sub(img_file, 1, slash_pos - 1))
  end

  -- tup does not want to see hidden files as dependencies
  if not string.match(local_file, "/%.") then
    table.insert(input_deps, v.group or local_file)
  end
end

-- create empty 1.44M file
make_img_command = '^ MKIMG kolibri.img^ ' -- for tup: don't write full command to logs
make_img_command = make_img_command .. "dd if=/dev/zero of=kolibri.img count=2880 bs=512 2>&1"
-- format it as a standard 1.44M floppy
make_img_command = make_img_command .. " && mformat -f 1440 -i kolibri.img ::"
-- copy bootloader
if tup.getconfig("NO_FASM") ~= "full" then
bootloader = "../kernel/trunk/bootloader/boot_fat12.bin"
make_img_command = make_img_command .. " && dd if=" .. bootloader .. " of=kolibri.img count=1 bs=512 conv=notrunc 2>&1"
table.insert(input_deps, bootloader)
end
-- make folders
table.sort(img_dirs)
for i,v in ipairs(img_dirs) do
  if v ~= img_dirs[i-1] then
    make_img_command = make_img_command .. ' && mmd -i kolibri.img "::' .. v .. '"'
  end
end
-- copy files
for i,v in ipairs(img_files) do
  local_file = v[2]
  if v[1] == "KERNEL.MNT" and tup.getconfig("INSERT_REVISION_ID") ~= ""
  then
    -- for kernel.mnt, insert autobuild revision identifier
    -- from .revision to .kernel.mnt
    -- note that .revision and .kernel.mnt must begin with .
    -- to prevent tup from tracking them
    if build_type == "rus"
    then str='$(LANG=ru_RU.utf8 date -u +"[автосборка %d %b %Y %R, r$(cat .revision)]"|iconv -f utf8 -t cp866)'
    else str='$(date -u +"[auto-build %d %b %Y %R, r$(cat .revision)]")'
    end
    str = string.gsub(str, "%$", "\\$") -- escape $ as \$
    str = string.gsub(str, "%%", "%%%%") -- escape % as %%
    make_img_command = make_img_command .. " && cp " .. local_file .. " .kernel.mnt"
    make_img_command = make_img_command .. " && str=" .. str
    make_img_command = make_img_command .. ' && echo -n $str | dd of=.kernel.mnt bs=1 seek=`expr 279 - length "$str"` conv=notrunc 2>/dev/null'
    local_file = ".kernel.mnt"
  end
  make_img_command = make_img_command .. ' && mcopy -moi kolibri.img "' .. local_file .. '" "::' .. v[1] .. '"'
end

-- generate tup rule for kolibri.img
tup.definerule{inputs = input_deps, command = make_img_command, outputs = {"kolibri.img"}}

-- generate command and dependencies for mkisofs
input_deps = {"kolibri.img"}
iso_files_list = ""
for i,v in ipairs(iso_extra_files) do
  iso_files_list = iso_files_list .. ' "' .. v[1] .. '=' .. v[2] .. '"'
  table.insert(input_deps, v.group or v[2])
end

-- generate tup rule for kolibri.iso
if tup.getconfig("INSERT_REVISION_ID") ~= ""
then volume_id = "KolibriOS r`cat .revision`"
else volume_id = "KolibriOS"
end
tup.definerule{inputs = input_deps, command =
  '^ MKISOFS kolibri.iso^ ' .. -- for tup: don't write full command to logs
  'mkisofs -U -J -pad -b kolibri.img -c boot.catalog -hide-joliet boot.catalog -graft-points ' ..
  '-A "KolibriOS AutoBuilder" -p "CleverMouse" -publisher "KolibriOS Team" -V "' .. volume_id .. '" -sysid "KOLIBRI" ' ..
  '-iso-level 3 -o kolibri.iso kolibri.img' .. iso_files_list .. ' 2>&1',
  outputs = {"kolibri.iso"}}

-- generate command and dependencies for distribution kit
cp = 'cp "%f" "%o"'
tup.definerule{inputs = {"kolibri.img"}, command = cp, outputs = {"distribution_kit/kolibri.img"}}
for i,v in ipairs(distr_extra_files) do
  cmd = cp:gsub("%%f", v[2]) -- input can be a group, we can't rely on tup's expansion of %f in this case
  if string.sub(v[1], -1) == "/"
  then tup.definerule{inputs = {v.group or v[2]}, command = cmd, outputs = {"distribution_kit/" .. v[1] .. tup.file(v[2])}}
  else tup.definerule{inputs = {v.group or v[2]}, command = cmd, outputs = {"distribution_kit/" .. v[1]}}
  end
end

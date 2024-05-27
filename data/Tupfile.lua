-- Do nothing unless explicitly requested in tup.config.
build_type = tup.getconfig('BUILD_TYPE')
if build_type == "" then
  return
end

--[================================[ DATA ]================================]--

SRC = tup.getcwd() .. '/..'
SRC_PROGS = SRC .. "/programs"
SRC_KERNEL = SRC .. "/kernel/trunk"

VAR = tup.getvariantdir() .. '/..'
VAR_PROGS = VAR .. "/programs"
VAR_DRVS = VAR .. "/drivers"
VAR_SKINS = VAR .. "/skins"
VAR_KERNEL = VAR .. "/kernel/trunk"
VAR_CONTRIB = VAR .. "/contrib"
VAR_DATA = VAR .. "/data"

-- Static data that doesn't need to be compiled
-- Files to be included in kolibri.img.
-- The first subitem of every item is name inside kolibri.img, the second is name of local file.
img_files = {
 {"MACROS.INC", SRC_PROGS .. "/macros.inc"},
-- {"CONFIG.INC", SRC_PROGS .. "/config.inc"},
 {"STRUCT.INC", SRC_PROGS .. "/struct.inc"},
 {"FB2READ", "common/fb2read"},
 {"ALLGAMES", "common/allgames"},
 {"HOME.PNG", "common/wallpapers/T_Home.png"},
 {"ICONS32.PNG", "common/icons32.png"},
 {"ICONS16.PNG", "common/icons16.png"},
 {"INDEX.HTM", "common/index_htm"},
 {"KUZKINA.MID", "common/kuzkina.mid"},
 {"SINE.MP3", "common/sine.mp3"},
 {"LANG.INC", build_type .. "/lang.inc"},
 {"NOTIFY3.PNG", "common/notify3.png"},
 {"UNIMG", SRC_PROGS .. "/fs/unimg/unimg"},
 {"3D/HOUSE.3DS", "common/3d/house.3ds"},
 {"File Managers/ICONS.INI", "common/File Managers/icons.ini"},
 {"GAMES/FLPYBIRD", SRC_PROGS .. "/games/flappybird/Release/flappybird"},
 {"FONTS/TAHOMA.KF", "common/fonts/tahoma.kf"},
 -- {"LIB/ICONV.OBJ", "common/lib/iconv.obj"},
 {"LIB/KMENU.OBJ", "common/lib/kmenu.obj"},
 {"LIB/PIXLIB.OBJ", "common/lib/pixlib.obj"},
 {"MEDIA/IMGF/IMGF", "common/media/ImgF/ImgF"},
 {"MEDIA/IMGF/CEDG.OBJ", "common/media/ImgF/cEdg.obj"},
 {"MEDIA/IMGF/DITHER.OBJ", "common/media/ImgF/dither.obj"},
 {"MEDIA/IMGF/INVSOL.OBJ", "common/media/ImgF/invSol.obj"},
 {"MEDIA/PIXIESKN.PNG", SRC_PROGS .. "/cmm/pixie2/pixieskn.png"},
 {"NETWORK/FTPC.INI", SRC_PROGS .. "/network/ftpc/ftpc.ini"},
 {"NETWORK/FTPC_SYS.PNG", SRC_PROGS .. "/network/ftpc/ftpc_sys.png"},
 {"NETWORK/FTPC_NOD.PNG", SRC_PROGS .. "/network/ftpc/ftpc_nod.png"},
 {"NETWORK/FTPD.INI", "common/network/ftpd.ini"},
 {"NETWORK/KNMAP", "common/network/knmap"},
 {"NETWORK/USERS.INI", "common/network/users.ini"},
 {"SETTINGS/APP.INI", "common/settings/app.ini"},
 {"SETTINGS/APP_PLUS.INI", "common/settings/app_plus.ini"},
 {"SETTINGS/ASSOC.INI", "common/settings/assoc.ini"},
 {"SETTINGS/AUTORUN.DAT", "common/settings/AUTORUN.DAT"},
 {"SETTINGS/CEDIT.INI", SRC_PROGS .. "/develop/cedit/CEDIT.INI"},
 {"SETTINGS/DOCKY.INI", "common/settings/docky.ini"},
 {"SETTINGS/FB2READ.INI", "common/settings/fb2read.ini"},
 {"SETTINGS/HA.CFG", SRC_PROGS .. "/other/ha/SETTINGS/HA.CFG"},
 {"SETTINGS/ICON.INI", build_type .. "/settings/icon.ini"},
 {"SETTINGS/KEYMAP.KEY", SRC_PROGS .. "/system/taskbar/trunk/KEYMAP.KEY"},
 {"SETTINGS/KOLIBRI.LBL", build_type .. "/settings/kolibri.lbl"},
 {"SETTINGS/LANG.INI", build_type .. "/settings/lang.ini"},
 {"SETTINGS/MENU.DAT", build_type .. "/settings/menu.dat"},
 {"SETTINGS/NETWORK.INI", "common/settings/network.ini"},
 {"SETTINGS/SYSTEM.INI", "common/settings/system.ini"},
 {"SETTINGS/TASKBAR.INI", "common/settings/taskbar.ini"},
 {"SETTINGS/SYSTEM.ENV", "common/settings/system.env"},
}

-- For russian build, add russian-only files.
if build_type == "ru_RU" then tup.append_table(img_files, {
 {"EXAMPLE.ASM", SRC_PROGS .. "/develop/examples/example/trunk/rus/example.asm"},
 {"DEVELOP/BACKY", SRC_PROGS .. "/develop/backy/Backy_ru"},
 {"GAMES/BASEKURS.KLA", build_type .. "/games/basekurs.kla"},
 {"File Managers/KFAR.INI", build_type .. "/File Managers/kfar.ini"},
 {"GAMES/DESCENT", build_type .. "/games/descent"},
 {"SETTINGS/.shell", SRC_PROGS .. "/system/shell/bin/rus/.shell"},
 {"SETTINGS/GAMES.INI", "ru_RU/settings/games.ini"},
 {"SETTINGS/MYKEY.INI", SRC_PROGS .. "/system/MyKey/trunk/mykey.ini"},
 {"SETTINGS/SYSPANEL.INI", "ru_RU/settings/syspanel.ini"},
}) elseif build_type == "en_US" then tup.append_table(img_files, {
 {"EXAMPLE.ASM", SRC_PROGS .. "/develop/examples/example/trunk/example.asm"},
 {"DEVELOP/BACKY", SRC_PROGS .. "/develop/backy/Backy"},
 {"File Managers/KFAR.INI", "common/File Managers/kfar.ini"},
 {"GAMES/DESCENT", "common/games/descent"},
 {"SETTINGS/.shell", SRC_PROGS .. "/system/shell/bin/eng/.shell"},
 {"SETTINGS/GAMES.INI", "common/settings/games.ini"},
 {"SETTINGS/MYKEY.INI", SRC_PROGS .. "/system/MyKey/trunk/mykey.ini"},
 {"SETTINGS/SYSPANEL.INI", "common/settings/syspanel.ini"},
}) elseif build_type == "es_ES" then tup.append_table(img_files, {
 {"EXAMPLE.ASM", SRC_PROGS .. "/develop/examples/example/trunk/example.asm"},
 {"DEVELOP/BACKY", SRC_PROGS .. "/develop/backy/Backy"},
 {"File Managers/KFAR.INI", "common/File Managers/kfar.ini"},
 {"GAMES/DESCENT", "common/games/descent"},
 {"SETTINGS/.shell", SRC_PROGS .. "/system/shell/bin/eng/.shell"},
 {"SETTINGS/GAMES.INI", "common/settings/games.ini"},
 {"SETTINGS/MYKEY.INI", SRC_PROGS .. "/system/MyKey/trunk/mykey.ini"},
 {"SETTINGS/SYSPANEL.INI", "common/settings/syspanel.ini"},
}) elseif build_type == "it_IT" then tup.append_table(img_files, {
 {"EXAMPLE.ASM", SRC_PROGS .. "/develop/examples/example/trunk/example.asm"},
 {"DEVELOP/BACKY", SRC_PROGS .. "/develop/backy/Backy"},
 {"File Managers/KFAR.INI", "common/File Managers/kfar.ini"},
 {"GAMES/DESCENT", "common/games/descent"},
 {"SETTINGS/.shell", SRC_PROGS .. "/system/shell/bin/eng/.shell"},
 {"SETTINGS/MYKEY.INI", SRC_PROGS .. "/system/MyKey/trunk/mykey_it.ini"},
 {"SETTINGS/GAMES.INI", "common/settings/games.ini"},
 {"SETTINGS/SYSPANEL.INI", "common/settings/syspanel.ini"},
}) else tup.append_table(img_files, {
 {"EXAMPLE.ASM", SRC_PROGS .. "/develop/examples/example/trunk/example.asm"},
 {"DEVELOP/BACKY", SRC_PROGS .. "/develop/backy/Backy"},
 {"File Managers/KFAR.INI", "common/File Managers/kfar.ini"},
 {"GAMES/DESCENT", "common/games/descent"},
 {"SETTINGS/.shell", SRC_PROGS .. "/system/shell/bin/eng/.shell"},
 {"SETTINGS/GAMES.INI", "common/settings/games.ini"},
 {"SETTINGS/MYKEY.INI", SRC_PROGS .. "/system/MyKey/trunk/mykey.ini"},
 {"SETTINGS/SYSPANEL.INI", "common/settings/syspanel.ini"},
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
 {"/", "common/distr_data/autorun.inf"},
 {"/", "common/distr_data/KolibriOS_icon.ico"},
 {"Docs/stack.txt", "../kernel/trunk/docs/stack.txt"},
 {"HD_Load/9x2klbr/", "common/HD_load/9x2klbr/LDKLBR.VXD"},
 {"HD_Load/MeOSLoad/", SRC_PROGS .. "/hd_load/meosload/AUTOEXEC.BAT"},
 {"HD_Load/MeOSLoad/", SRC_PROGS .. "/hd_load/meosload/CONFIG.SYS"},
 {"HD_Load/MeOSLoad/", SRC_PROGS .. "/hd_load/meosload/L_readme.txt"},
 {"HD_Load/MeOSLoad/", SRC_PROGS .. "/hd_load/meosload/L_readme_Win.txt"},
 {"HD_Load/mtldr/", SRC_PROGS .. "/hd_load/mtldr/vista_install.bat"},
 {"HD_Load/mtldr/", SRC_PROGS .. "/hd_load/mtldr/vista_remove.bat"},
 {"HD_Load/", "common/HD_load/memdisk"},
 {"HD_Load/USB_boot_old/", SRC_PROGS .. "/hd_load/usb_boot_old/usb_boot.rtf"},
 {"HD_Load/USB_boot_old/", SRC_PROGS .. "/hd_load/usb_boot_old/usb_boot_866.txt"},
 {"HD_Load/USB_boot_old/", SRC_PROGS .. "/hd_load/usb_boot_old/usb_boot_1251.txt"},
 {"kolibrios/3D/info3ds/INFO3DS.INI", SRC_PROGS .. "/develop/info3ds/info3ds.ini"},
 {"kolibrios/3D/info3ds/OBJECTS.PNG", SRC_PROGS .. "/develop/info3ds/objects.png"},
 {"kolibrios/3D/info3ds/TOOLBAR.PNG", SRC_PROGS .. "/develop/info3ds/toolbar.png"},
 {"kolibrios/3D/info3ds/FONT8X9.BMP", SRC_PROGS .. "/fs/kfar/trunk/font8x9.bmp"},
 {"kolibrios/3D/blocks/", "../programs/bcc32/games/blocks/bin/*"},
 {"kolibrios/3D/blocks/models/", "../programs/bcc32/games/blocks/models/*"},
 {"kolibrios/3D/md2view/", "common/3d/md2view/*"},
 {"kolibrios/3D/md2view/md2_model/", "common/3d/md2view/md2_model/*"},
 {"kolibrios/3D/voxel_editor/VOX_EDITOR.INI", SRC_PROGS .. "/media/voxel_editor/trunk/vox_editor.ini"},
 {"kolibrios/3D/voxel_editor/HOUSE1.VOX", SRC_PROGS .. "/media/voxel_editor/trunk/house1.vox"},
 {"kolibrios/3D/voxel_editor/HOUSE2.VOX", SRC_PROGS .. "/media/voxel_editor/trunk/house2.vox"},
 {"kolibrios/3D/voxel_editor/SQUIRREL.VOX", SRC_PROGS .. "/media/voxel_editor/trunk/squirrel.vox"},
 {"kolibrios/3D/voxel_utilites/VOX_MOVER.INI" , SRC_PROGS .. "/media/voxel_editor/utilites/vox_mover.ini"},
 {"kolibrios/3D/FONT8X9.BMP", SRC_PROGS .. "/fs/kfar/trunk/font8x9.bmp"},
 {"kolibrios/3D/TOOLB_1.PNG", SRC_PROGS .. "/develop/libraries/TinyGL/asm_fork/examples/toolb_1.png"},
 {"kolibrios/3D/TEST_GLU1", VAR_PROGS .. "/develop/libraries/TinyGL/asm_fork/examples/test_glu1"},
 {"kolibrios/3D/TEST_GLU2", VAR_PROGS .. "/develop/libraries/TinyGL/asm_fork/examples/test_glu2"},
 {"kolibrios/3D/TEXT_2.PNG", SRC_PROGS .. "/develop/libraries/TinyGL/asm_fork/examples/text_2.png"},
 {"kolibrios/demos/ak47.lif", "common/demos/ak47.lif"},
 {"kolibrios/demos/life2", "common/demos/life2"},
 {"kolibrios/demos/relay.lif", "common/demos/relay.lif"},
 {"kolibrios/demos/rpento.lif", "common/demos/rpento.lif"},
 {"kolibrios/develop/c--/c--.elf", SRC_PROGS .. "/cmm/c--/c--.elf"},
 {"kolibrios/develop/c--/c--.exe", SRC_PROGS .. "/cmm/c--/c--.exe"},
 {"kolibrios/develop/c--/c--.ini", SRC_PROGS .. "/cmm/c--/c--.ini"},
 {"kolibrios/develop/c--/manual_c--.htm", SRC_PROGS .. "/cmm/c--/manual_c--.htm"},
 {"kolibrios/develop/fpc/", "common/develop/fpc/*"},
 {"kolibrios/develop/fpc/examples/", "../programs/develop/fp/examples/src/*"},
 {"kolibrios/develop/fpc/examples/build.sh", "common/develop/fpc/build.sh"},
 {"kolibrios/develop/oberon07/", "../programs/develop/oberon07/*"},
 {"kolibrios/develop/oberon07/doc/", "../programs/develop/oberon07/doc/*"},
 {"kolibrios/develop/oberon07/lib/KolibriOS/", "../programs/develop/oberon07/lib/KolibriOS/*"},
 {"kolibrios/develop/oberon07/samples/", SRC_PROGS .. "/develop/oberon07/samples/*"},
 {"kolibrios/develop/tcc/lib/", SRC_PROGS ..  "/develop/ktcc/trunk/bin/lib/*"},
 {"kolibrios/develop/tcc/include/", SRC_PROGS ..  "/develop/ktcc/trunk/libc.obj/include/*"},
 {"kolibrios/develop/tcc/include/clayer/", SRC_PROGS ..  "/develop/ktcc/trunk/libc.obj/include/clayer/*"},
 {"kolibrios/develop/tcc/include/cryptal/", SRC_PROGS .. "/develop/ktcc/trunk/libc.obj/include/cryptal/*"},
 {"kolibrios/develop/tcc/include/sys/", SRC_PROGS .. "/develop/ktcc/trunk/libc.obj/include/sys/*"},
 {"kolibrios/develop/tcc/include/SDL/", "../contrib/sdk/sources/SDL-1.2.2_newlib/include/*"},
 {"kolibrios/develop/tcc/samples/", SRC_PROGS ..  "/develop/ktcc/trunk/libc.obj/samples/*.c"},
 {"kolibrios/develop/tcc/samples/", SRC_PROGS ..  "/develop/ktcc/trunk/libc.obj/samples/*.sh"},
 {"kolibrios/develop/tcc/samples/clayer/", SRC_PROGS ..  "/develop/ktcc/trunk/libc.obj/samples/clayer/*"},
 {"kolibrios/develop/utils/SPEDump", SRC_PROGS .. "/develop/SPEDump/SPEDump.kex"},
 {"kolibrios/emul/", "common/emul/*"},
 {"kolibrios/emul/dosbox/", "common/emul/DosBox/*"},
 {"kolibrios/emul/e80/readme.txt", SRC_PROGS .. "/emulator/e80/trunk/readme.txt"},
 {"kolibrios/emul/e80/keyboard.png", SRC_PROGS .. "/emulator/e80/trunk/keyboard.png"},
 {"kolibrios/emul/fceu/fceu", SRC_PROGS .. "/emulator/fceu/fceu"},
 {"kolibrios/emul/fceu/FCEU ReadMe.txt", SRC_PROGS .. "/emulator/fceu/FCEU ReadMe.txt"},
 {"kolibrios/emul/chip8/chip8", VAR_PROGS .. "/emulator/chip8/chip8"},
 {"kolibrios/emul/chip8/readme.txt", SRC_PROGS .. "/emulator/chip8/readme.txt"},
 {"kolibrios/emul/chip8/roms/", SRC_PROGS .. "/emulator/chip8/roms/*"},
 {"kolibrios/emul/kwine/kwine", SRC_PROGS .. "/emulator/kwine/bin/kwine"},
 {"kolibrios/emul/kwine/lib/", SRC_PROGS .. "/emulator/kwine/bin/lib/*"},
 {"kolibrios/emul/uarm/", "common/emul/uarm/*"},
 {"kolibrios/emul/zsnes/", "common/emul/zsnes/*"},
 {"kolibrios/games/BabyPainter", "common/games/BabyPainter"},
 {"kolibrios/games/bomber/ackack.bmp", SRC_PROGS .. "/games/bomber/ackack.bmp"},
 {"kolibrios/games/bomber/bomb.bmp", SRC_PROGS .. "/games/bomber/bomb.bmp"},
 {"kolibrios/games/bomber/plane.bmp", SRC_PROGS .. "/games/bomber/plane.bmp"},
 {"kolibrios/games/bomber/tile.bmp", SRC_PROGS .. "/games/bomber/tile.bmp"},
 {"kolibrios/games/doom1/", "common/games/doom/*"},
 {"kolibrios/games/fara/fara.gfx", "common/games/fara.gfx"},
 {"kolibrios/games/jumpbump/", "common/games/jumpbump/*"},
 {"kolibrios/games/knight", "common/games/knight"},
 {"kolibrios/games/KosChess/", "common/games/KosChess/*"},
 {"kolibrios/games/KosChess/images/", "common/games/KosChess/images/*"},
 {"kolibrios/games/LaserTank/", "common/games/LaserTank/*"},
 {"kolibrios/games/lrl/", "common/games/lrl/*"},
 {"kolibrios/games/mun/data/", "common/games/mun/data/*"},
 {"kolibrios/games/mun/libc.dll", "common/games/mun/libc.dll"},
 {"kolibrios/games/mun/mun", "common/games/mun/mun"},
 {"kolibrios/games/pig/", "common/games/pig/*"},
 {"kolibrios/games/soko/", "common/games/soko/*"},
 {"kolibrios/games/fridge/", "common/games/fridge/*"},
 {"kolibrios/games/the_bus/menu.png", SRC_PROGS .. "/cmm/the_bus/menu.png"},
 {"kolibrios/games/the_bus/objects.png", SRC_PROGS .. "/cmm/the_bus/objects.png"},
 {"kolibrios/games/the_bus/road.png", SRC_PROGS .. "/cmm/the_bus/road.png"},
 {"kolibrios/grafx2/fonts/", "common/media/grafx2/fonts/*"},
 {"kolibrios/grafx2/scripts/", "common/media/grafx2/scripts/libs/*"},
 {"kolibrios/grafx2/scripts/libs/", "common/media/grafx2/scripts/*"},
 {"kolibrios/grafx2/skins/", "common/media/grafx2/skins/*"},
 {"kolibrios/grafx2/", "common/media/grafx2/*"},
 {"kolibrios/speech/", "common/media/speech/*"},
 {"kolibrios/drivers/drvinf.ini", "common/drivers/drvinf.ini"},
 {"kolibrios/drivers/ahci/", "common/drivers/ahci/*"},
 {"kolibrios/drivers/acpi/readme.txt", "common/drivers/acpi/readme.txt"},
 -- {"kolibrios/drivers/acpi/", "common/drivers/acpi/*"},
 {"kolibrios/drivers/atikms/", "common/drivers/atikms/*"},
 -- {"kolibrios/drivers/geode/", "common/drivers/geode/*"},
 {"kolibrios/drivers/i915/", "common/drivers/i915/*"},
 {"kolibrios/drivers/test/", "common/drivers/test/*"},
 {"kolibrios/drivers/vmware/", "common/drivers/vmware/*"},
 {"kolibrios/drivers/virtualbox/", "common/drivers/virtualbox/*"},
 {"kolibrios/KolibriNext/", "common/KolibriNext/*"},
 {"kolibrios/KolibriNext/settings/", "common/KolibriNext/settings/*"},
 {"kolibrios/lib/avcodec-56.dll", "common/lib/avcodec-56.dll"},
 {"kolibrios/lib/avdevice-56.dll", "common/lib/avdevice-56.dll"},
 {"kolibrios/lib/avformat-56.dll", "common/lib/avformat-56.dll"},
 {"kolibrios/lib/swscale-3.dll", "common/lib/swscale-3.dll"},
 {"kolibrios/lib/avutil-54.dll", "common/lib/avutil-54.dll"},
 {"kolibrios/lib/cairo2.dll", "common/lib/cairo2.dll"},
 {"kolibrios/lib/freetype.dll", "common/lib/freetype.dll"},
 {"kolibrios/lib/i965-video.dll", "common/lib/i965-video.dll"},
 {"kolibrios/lib/libdrm.dll", "common/lib/libdrm.dll"},
 {"kolibrios/lib/libegl.dll", "common/lib/libegl.dll"},
 {"kolibrios/lib/libeglut.dll", "common/lib/libeglut.dll"},
 {"kolibrios/lib/libGL.dll", "common/lib/libGL.dll"},
 {"kolibrios/lib/libjpeg.dll", "common/lib/libjpeg.dll"},
 {"kolibrios/lib/libpng16.dll", "common/lib/libpng16.dll"},
 {"kolibrios/lib/libva.dll", "common/lib/libva.dll"},
 {"kolibrios/lib/libz.dll", "common/lib/libz.dll"},
 {"kolibrios/lib/osmesa.dll", "common/lib/osmesa.dll"},
 {"kolibrios/lib/pixlib-gl.dll", "common/lib/pixlib-gl.dll"},
 {"kolibrios/lib/pixman-1.dll", "common/lib/pixman-1.dll"},
 {"kolibrios/lib/swresample-1.dll", "common/lib/swresample-1.dll"},
 {"kolibrios/lib/i915_dri.drv", "common/lib/i915_dri.drv"},
 {"kolibrios/media/fplay", "common/media/fplay"},
 {"kolibrios/media/fplay_run", "common/media/fplay_run"},
 {"kolibrios/media/minimp3", "common/media/minimp3"},
 {"kolibrios/media/updf", "common/media/updf"},
 {"kolibrios/media/vttf", "common/media/vttf"},
 {"kolibrios/media/beat/Beat", SRC_PROGS .. "/media/Beat/Beat"},
 {"kolibrios/media/beat/Beep1.raw", SRC_PROGS .. "/media/Beat/Beep1.raw"},
 {"kolibrios/media/beat/Beep2.raw", SRC_PROGS .. "/media/Beat/Beep2.raw"},
 {"kolibrios/media/beat/PlayNote", SRC_PROGS .. "/media/Beat/PlayNote/PlayNote"},
 {"kolibrios/media/beat/Readme-en.txt", SRC_PROGS .. "/media/Beat/Readme-en.txt"},
 {"kolibrios/media/beat/Readme-ru.txt", SRC_PROGS .. "/media/Beat/Readme-ru.txt"},
 {"kolibrios/media/zsea/zsea.ini", SRC_PROGS .. "/media/zsea/zSea.ini"},
 {"kolibrios/media/zsea/buttons/buttons.png", SRC_PROGS .. "/media/zsea/buttons.png"},
 {"kolibrios/netsurf/netsurf", "common/network/netsurf/netsurf"},
 {"kolibrios/netsurf/res/", "common/network/netsurf/res/*"},
 {"kolibrios/res/skins/", "../skins/authors.txt"},
 {"kolibrios/res/templates/", "common/templates/*"},
 {"kolibrios/res/templates/", SRC_PROGS .. "/emulator/e80/trunk/games/*"},
 {"kolibrios/res/templates/NES/", "common/templates/NES/*"},
 {"kolibrios/res/wallpapers/", "common/wallpapers/*"},
 {"kolibrios/res/system/", build_type .. "/settings/kolibri.lbl"},
 {"kolibrios/utils/vmode", "common/vmode"},
 {"kolibrios/utils/texture", "common/utils/texture"},
 {"kolibrios/utils/cnc_editor/cnc_editor", VAR_PROGS .. "/other/cnc_editor/cnc_editor"},
 {"kolibrios/utils/cnc_editor/kolibri.NC", SRC_PROGS .. "/other/cnc_editor/kolibri.NC"},
 {"kolibrios/utils/kfm/kfm.ini", "common/File Managers/kfm.ini"},
 {"kolibrios/utils/kfm/kfm_keys_eng.txt", SRC_PROGS .. "/fs/kfm/trunk/docs/english/kfm_keys.txt"},
 {"kolibrios/utils/kfm/kfm_keys_rus.txt", SRC_PROGS .. "/fs/kfm/trunk/docs/russian/dos_kolibri/kfm_keys.txt"},
 {"kolibrios/utils/fNav/", "common/File Managers/fNav/*"},
 {"kolibrios/utils/NDN/", "common/File Managers/ndn/*"},
 {"kolibrios/utils/NDN/COLORS/", "common/File Managers/ndn/COLORS/*"},
 {"kolibrios/utils/NDN/XLT/", "common/File Managers/ndn/XLT/*"},
 {"kolibrios/utils/tedit/t_edit.ini", SRC_PROGS .. "/other/t_edit/t_edit.ini"},
 {"kolibrios/utils/tedit/info/ASM.SYN", VAR_PROGS .. "/other/t_edit/info/asm.syn"},
 {"kolibrios/utils/tedit/info/CPP_CLA.SYN", VAR_PROGS .. "/other/t_edit/info/cpp_kol_cla.syn"},
 {"kolibrios/utils/tedit/info/CPP_DAR.SYN", VAR_PROGS .. "/other/t_edit/info/cpp_kol_dar.syn"},
 {"kolibrios/utils/tedit/info/CPP_DEF.SYN", VAR_PROGS .. "/other/t_edit/info/cpp_kol_def.syn"},
 {"kolibrios/utils/tedit/info/DEFAULT.SYN", VAR_PROGS .. "/other/t_edit/info/default.syn"},
 {"kolibrios/utils/tedit/info/HTML.SYN", VAR_PROGS .. "/other/t_edit/info/html.syn"},
 {"kolibrios/utils/tedit/info/INI.SYN", VAR_PROGS .. "/other/t_edit/info/ini_files.syn"},
 }
if build_type == "ru_RU" then tup.append_table(extra_files, {
 {"Docs/cp866/config.txt", build_type .. "/docs/CONFIG.TXT"},
 {"Docs/cp866/gnu.txt", build_type .. "/docs/GNU.TXT"},
 {"Docs/cp866/history.txt", build_type .. "/docs/HISTORY.TXT"},
 {"Docs/cp866/hot_keys.txt", build_type .. "/docs/HOT_KEYS.TXT"},
 {"Docs/cp866/install.txt", build_type .. "/docs/INSTALL.TXT"},
 {"Docs/cp866/credits.txt", build_type .. "/docs/CREDITS.TXT"},
 {"Docs/cp866/sysfuncr.txt", VAR_PROGS .. "/system/docpack/trunk/SYSFUNCR.TXT"},
 {"Docs/cp1251/config.txt", VAR_DATA .. "/" .. build_type .. "/docs/CONFIG.WIN.TXT", cp1251_from = build_type .. "/docs/CONFIG.TXT"},
 {"Docs/cp1251/gnu.txt", "$(VAR_DATA)/$(build_type)/docs/GNU.WIN.TXT", cp1251_from = build_type .. "/docs/GNU.TXT"},
 {"Docs/cp1251/history.txt", "$(VAR_DATA)/$(build_type)/docs/HISTORY.WIN.TXT", cp1251_from = build_type .. "/docs/HISTORY.TXT"},
 {"Docs/cp1251/hot_keys.txt", "$(VAR_DATA)/$(build_type)/docs/HOT_KEYS.WIN.TXT", cp1251_from = build_type .. "/docs/HOT_KEYS.TXT"},
 {"Docs/cp1251/install.txt", "$(VAR_DATA)/$(build_type)/docs/INSTALL.WIN.TXT", cp1251_from = build_type .. "/docs/INSTALL.TXT"},
 {"Docs/cp1251/credits.txt", "$(VAR_DATA)/$(build_type)/docs/CREDITS.WIN.TXT", cp1251_from = build_type .. "/docs/CREDITS.TXT"},
 {"Docs/cp1251/sysfuncr.txt", "$(VAR_DATA)/$(build_type)/docs/SYSFUNCR.WIN.TXT", cp1251_from = SRC_PROGS .. "/system/docpack/trunk/SYSFUNCR.TXT"},
 {"HD_Load/9x2klbr/", SRC_PROGS .. "/hd_load/9x2klbr/readme_dos.txt"},
 {"HD_Load/9x2klbr/", SRC_PROGS .. "/hd_load/9x2klbr/readme_win.txt"},
 {"HD_Load/mtldr/", SRC_PROGS .. "/hd_load/mtldr/install.txt"},
 {"HD_Load/USB_Boot/", SRC_PROGS .. "/hd_load/usb_boot/readme.txt"},
 {"kolibrios/games/ataka", "common/games/ataka/ataka_ru"},
 {"kolibrios/games/Dungeons/Resources/Textures/Environment/", SRC_PROGS .. "/games/Dungeons/Resources/Textures/Environment/*"},
 {"kolibrios/games/Dungeons/Resources/Textures/Objects/", SRC_PROGS .. "/games/Dungeons/Resources/Textures/Objects/*"},
 {"kolibrios/games/Dungeons/Resources/Textures/HUD/", SRC_PROGS .. "/games/Dungeons/Resources/Textures/HUD/*"},
 {"kolibrios/games/Dungeons/Resources/Textures/", SRC_PROGS .. "/games/Dungeons/Resources/Textures/Licenses.txt"},
 {"kolibrios/games/Dungeons/", SRC_PROGS .. "/games/Dungeons/readme_ru.txt"},
 {"kolibrios/games/sstartrek/SStarTrek", "common/games/sstartrek/SStarTrek_ru"},
 {"kolibrios/games/WHOWTBAM/", build_type .. "/games/whowtbam"},
 {"kolibrios/games/WHOWTBAM/", build_type .. "/games/appdata.dat"},
 {"kolibrios/media/zsea/zsea_keys.txt", SRC_PROGS .. "/media/zsea/Docs/zSea_keys_rus.txt"},
 {"kolibrios/res/guide/", build_type .. "/docs/guide/*"},
 {"kolibrios/develop/tcc/doc/", SRC_PROGS .. "/develop/ktcc/trunk/bin/doc/ru/*"},
}) else tup.append_table(extra_files, {
 {"Docs/config.txt", build_type .. "/docs/CONFIG.TXT"},
 {"Docs/copying.txt", build_type .. "/docs/COPYING.TXT"},
 {"Docs/hot_keys.txt", build_type .. "/docs/HOT_KEYS.TXT"},
 {"Docs/install.txt", build_type .. "/docs/INSTALL.TXT"},
 {"Docs/credits.txt", build_type .. "/docs/CREDITS.TXT"},
 {"Docs/sysfuncs.txt", VAR_PROGS .. "/system/docpack/trunk/SYSFUNCS.TXT"},
 {"HD_Load/9x2klbr/", SRC_PROGS .. "/hd_load/9x2klbr/readme.txt"},
 {"HD_Load/mtldr/install.txt", SRC_PROGS .. "/hd_load/mtldr/install_eng.txt"},
 {"HD_Load/USB_Boot/readme.txt", SRC_PROGS .. "/hd_load/usb_boot/readme_eng.txt"},
 {"kolibrios/games/ataka", "common/games/ataka/ataka_en"},
 {"kolibrios/games/sstartrek/SStarTrek", "common/games/sstartrek/SStarTrek_en"},
 {"kolibrios/media/zsea/zsea_keys.txt", SRC_PROGS .. "/media/zsea/Docs/zSea_keys_eng.txt"},
 {"kolibrios/develop/tcc/doc/", SRC_PROGS .. "/develop/ktcc/trunk/bin/doc/en/*"},
}) end
--[[
Files to be included in distribution kit outside of kolibri.img, but not kolibri.iso.
Same syntax as extra_files.
]]--
if build_type == "ru_RU" then
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
if build_type == "ru_RU" then
iso_extra_files = {
 {"/readme_dos.txt", build_type .. "/distr_data/readme_dos.txt"},
 {"/readme.txt", "$(VAR_DATA)/$(build_type)/distr_data/readme.txt", cp1251_from = build_type .. "/distr_data/readme_dos.txt"},
}
else
iso_extra_files = {
 {"/readme.txt", build_type .. "/distr_data/readme.txt"},
}
end

-- Programs that require FASM to compile.
if tup.getconfig('NO_FASM') ~= 'full' then
tup.append_table(img_files, {
 {"KERNEL.MNT", VAR_PROGS .. "/../kernel/trunk/kernel.mnt"},
 {"@DOCKY", VAR_PROGS .. "/system/docky/trunk/docky"},
 {"@HA", VAR_PROGS .. "/other/ha/HA"},
 {"@ICON", VAR_PROGS .. "/system/icon_new/icon"},
 {"@MENU", VAR_PROGS .. "/system/menu/trunk/menu"},
 {"@NOTIFY", VAR_PROGS .. "/system/notify3/notify"},
 {"@OPEN", VAR_PROGS .. "/system/open/open"},
 {"@TASKBAR", VAR_PROGS .. "/system/taskbar/trunk/TASKBAR"},
 {"@SS", VAR_PROGS .. "/system/scrsaver/scrsaver"},
 {"@VOLUME", VAR_PROGS .. "/media/volume/volume"},
 {"HACONFIG", VAR_PROGS .. "/other/ha/HACONFIG"},
 {"APM", VAR_PROGS .. "/system/apm/apm"},
 {"CALC", VAR_PROGS .. "/other/calc/trunk/calc"},
 {"CALENDAR", VAR_PROGS .. "/system/calendar/trunk/calendar"},
 {"COLRDIAL", VAR_PROGS .. "/system/colrdial/color_dialog"},
 {"CROPFLAT", VAR_PROGS .. "/system/cropflat/cropflat"},
 {"CPU", VAR_PROGS .. "/system/cpu/trunk/cpu"},
 {"CPUID", VAR_PROGS .. "/testing/cpuid/trunk/CPUID"},
 {"DOCPACK", VAR_PROGS .. "/system/docpack/trunk/docpack"},
 {"DEFAULT.SKN", VAR_SKINS .. "/../skins/Leency/Shkvorka/Shkvorka.skn"},
 {"DISPTEST", VAR_PROGS .. "/testing/disptest/trunk/disptest"},
 {"END", VAR_PROGS .. "/system/end/light/end"},
 {"ESKIN", VAR_PROGS .. "/system/eskin/trunk/eskin"},
 {"FSPEED", VAR_PROGS .. "/testing/fspeed/fspeed"},
 {"GMON", VAR_PROGS .. "/system/gmon/gmon"},
 {"HDD_INFO", VAR_PROGS .. "/system/hdd_info/trunk/hdd_info"},
 {"KBD", VAR_PROGS .. "/testing/kbd/trunk/kbd"},
 {"KPACK", VAR_PROGS .. "/other/kpack/trunk/kpack"},
 {"LAUNCHER", VAR_PROGS .. "/system/launcher/trunk/launcher"},
 {"LOADDRV", VAR_PROGS .. "/system/loaddrv/loaddrv"},
 {"MAGNIFY", VAR_PROGS .. "/demos/magnify/trunk/magnify"},
 {"MGB", VAR_PROGS .. "/testing/mgb/trunk/mgb"},
 {"MOUSEMUL", VAR_PROGS .. "/system/mousemul/trunk/mousemul"},
 {"MADMOUSE", VAR_PROGS .. "/other/madmouse/madmouse"},
 {"MYKEY", VAR_PROGS .. "/system/MyKey/trunk/MyKey"},
 {"PCIDEV", VAR_PROGS .. "/testing/pcidev/trunk/PCIDEV"},
 {"RDSAVE", VAR_PROGS .. "/system/rdsave/trunk/rdsave"},
 {"RTFREAD", VAR_PROGS .. "/other/rtfread/trunk/rtfread"},
 {"SEARCHAP", VAR_PROGS .. "/system/searchap/searchap"},
 {"SCRSHOOT", VAR_PROGS .. "/media/scrshoot/scrshoot"},
 {"SETUP", VAR_PROGS .. "/system/setup/trunk/setup"},
 {"SKINCFG", VAR_PROGS .. "/system/skincfg/trunk/skincfg"},
 {"TERMINAL", VAR_PROGS .. "/system/terminal/terminal"},
 {"TEST", VAR_PROGS .. "/testing/protection/trunk/test"},
 {"TINYPAD", VAR_PROGS .. "/develop/tinypad/trunk/tinypad"},
 {"UNZ", VAR_PROGS .. "/fs/unz/unz"},
 {"ZKEY", VAR_PROGS .. "/system/zkey/trunk/ZKEY"},
 {"3D/3DWAV", VAR_PROGS .. "/demos/3dwav/trunk/3dwav"},
 {"3D/CROWNSCR", VAR_PROGS .. "/demos/crownscr/trunk/crownscr"},
 {"3D/3DCUBE2", VAR_PROGS .. "/demos/3dcube2/trunk/3DCUBE2"},
 {"3D/FREE3D04", VAR_PROGS .. "/demos/free3d04/trunk/free3d04"},
 {"3D/GEARS", VAR_PROGS .. "/develop/libraries/TinyGL/asm_fork/examples/gears"},
 {"3D/RAY", VAR_PROGS .. "/demos/ray/ray"},
 {"3D/VIEW3DS", VAR_PROGS .. "/demos/view3ds/view3ds"},
 {"DEMOS/BCDCLK", VAR_PROGS .. "/demos/bcdclk/trunk/bcdclk"},
 {"DEMOS/BUDHBROT", VAR_PROGS .. "/demos/buddhabrot/trunk/buddhabrot"},
 {"DEMOS/EYES", VAR_PROGS .. "/demos/eyes/trunk/eyes"},
 {"DEMOS/FIREWORK", VAR_PROGS .. "/demos/firework/trunk/firework"},
 {"DEMOS/MOVBACK", VAR_PROGS .. "/demos/movback/trunk/movback"},
 {"DEMOS/PLASMA", VAR_PROGS .. "/demos/plasma/trunk/plasma"},
 {"DEMOS/SPIRAL", VAR_PROGS .. "/demos/spiral/spiral"},
 {"DEMOS/TINYFRAC", VAR_PROGS .. "/demos/tinyfrac/trunk/tinyfrac"},
 {"DEMOS/TRANTEST", VAR_PROGS .. "/demos/trantest/trunk/trantest"},
 {"DEMOS/TUBE", VAR_PROGS .. "/demos/tube/trunk/tube"},
 {"DEMOS/UNVWATER", VAR_PROGS .. "/demos/unvwater/trunk/unvwater"},
 {"DEMOS/WEB", VAR_PROGS .. "/demos/web/trunk/web"},
 {"DEMOS/ZEROLINE", VAR_PROGS .. "/demos/zeroline/trunk/zeroline"},
 {"DEVELOP/ASCIIVJU", VAR_PROGS .. "/develop/asciivju/trunk/asciivju"},
 {"DEVELOP/BOARD", VAR_PROGS .. "/system/board/trunk/board"},
 {"DEVELOP/CEDIT", SRC_PROGS .. "/develop/cedit/CEDIT"},
 {"DEVELOP/COBJ", VAR_PROGS .. "/develop/cObj/trunk/cObj"},
 {"DEVELOP/ENTROPYV", VAR_PROGS .. "/develop/entropyview/entropyview"},
 {"DEVELOP/FASM", VAR_PROGS .. "/develop/fasm/1.73/fasm"},
 {"DEVELOP/H2D2B", VAR_PROGS .. "/develop/h2d2b/trunk/h2d2b"},
 {"DEVELOP/HEED", VAR_PROGS .. "/develop/heed/trunk/heed"},
 {"DEVELOP/KEYASCII", VAR_PROGS .. "/develop/keyascii/trunk/keyascii"},
 {"DEVELOP/MTDBG", VAR_PROGS .. "/develop/mtdbg/mtdbg"},
 {"DEVELOP/SCANCODE", VAR_PROGS .. "/develop/scancode/trunk/scancode"},
 {"DEVELOP/EXAMPLES/CIRCLE", VAR_PROGS .. "/develop/examples/circle/trunk/circle"},
 {"DEVELOP/EXAMPLES/COLORREF", VAR_PROGS .. "/demos/colorref/trunk/colorref"},
 {"DEVELOP/EXAMPLES/CONGET", VAR_PROGS .. "/develop/libraries/console_coff/examples/test_gets"},
 {"DEVELOP/EXAMPLES/CSLIDE", VAR_PROGS .. "/demos/cslide/trunk/cslide"},
 {"DEVELOP/EXAMPLES/THREAD", VAR_PROGS .. "/develop/examples/thread/trunk/thread"},
 {"DEVELOP/EXAMPLES/USE_MB", VAR_PROGS .. "/demos/use_mb/use_mb"},
 {"File Managers/KFAR", VAR_PROGS .. "/fs/kfar/trunk/kfar"},
 {"File Managers/OPENDIAL", VAR_PROGS .. "/fs/opendial/opendial"},
 {"GAMES/15", VAR_PROGS .. "/games/15/trunk/15"},
 {"GAMES/FREECELL", VAR_PROGS .. "/games/freecell/freecell"},
 {"GAMES/GOMOKU", VAR_PROGS .. "/games/gomoku/trunk/gomoku"},
 {"GAMES/LIGHTS", VAR_PROGS .. "/games/sq_game/trunk/SQ_GAME"},
 {"GAMES/LINES", VAR_PROGS .. "/games/lines/lines"},
 {"GAMES/MSQUARE", VAR_PROGS .. "/games/MSquare/trunk/MSquare"},
 {"GAMES/PIPES", VAR_PROGS .. "/games/pipes/pipes"},
 {"GAMES/PONG", VAR_PROGS .. "/games/pong/trunk/pong"},
 {"GAMES/PONG3", VAR_PROGS .. "/games/pong3/trunk/pong3"},
 {"GAMES/RSQUARE", VAR_PROGS .. "/games/rsquare/trunk/rsquare"},
 {"GAMES/SNAKE", VAR_PROGS .. "/games/snake/trunk/snake"},
 {"GAMES/SUDOKU", VAR_PROGS .. "/games/sudoku/trunk/sudoku"},
 {"GAMES/SW", VAR_PROGS .. "/games/sw/trunk/sw"},
 {"GAMES/TANKS", VAR_PROGS .. "/games/tanks/trunk/tanks"},
 {"GAMES/TETRIS", VAR_PROGS .. "/games/tetris/trunk/tetris"},
 {"LIB/ARCHIVER.OBJ", VAR_PROGS .. "/fs/kfar/trunk/kfar_arc/kfar_arc.obj"},
 {"LIB/BOX_LIB.OBJ", VAR_PROGS .. "/develop/libraries/box_lib/trunk/box_lib.obj"},
 {"LIB/BUF2D.OBJ", VAR_PROGS .. "/develop/libraries/buf2d/trunk/buf2d.obj"},
 {"LIB/CONSOLE.OBJ", VAR_PROGS .. "/develop/libraries/console_coff/console.obj"},
 {"LIB/CNV_PNG.OBJ", VAR_PROGS .. "/media/zsea/plugins/png/cnv_png.obj"},
 {"LIB/DLL.OBJ", VAR_PROGS .. "/develop/libraries/dll/dll.obj"},
 {"LIB/HTTP.OBJ", VAR_PROGS .. "/develop/libraries/http/http.obj"},
 {"LIB/LIBCRASH.OBJ", VAR_PROGS .. "/develop/libraries/libcrash/libcrash.obj"},
 {"LIB/LIBGFX.OBJ", VAR_PROGS .. "/develop/libraries/libs-dev/libgfx/libgfx.obj"},
 {"LIB/LIBIMG.OBJ", VAR_PROGS .. "/develop/libraries/libs-dev/libimg/libimg.obj"},
 {"LIB/LIBINI.OBJ", VAR_PROGS .. "/develop/libraries/libs-dev/libini/libini.obj"},
 {"LIB/LIBIO.OBJ", VAR_PROGS .. "/develop/libraries/libs-dev/libio/libio.obj"},
 {"LIB/MSGBOX.OBJ", VAR_PROGS .. "/develop/libraries/msgbox/msgbox.obj"},
 {"LIB/NETWORK.OBJ", VAR_PROGS .. "/develop/libraries/network/network.obj"},
 {"LIB/PROC_LIB.OBJ", VAR_PROGS .. "/develop/libraries/proc_lib/trunk/proc_lib.obj"},
 {"LIB/RASTERWORKS.OBJ", VAR_PROGS .. "/develop/libraries/fontRasterWorks_unicode/RasterWorks.obj"},
 {"LIB/SORT.OBJ", VAR_PROGS .. "/develop/libraries/sorter/sort.obj"},
 {"LIB/TINYGL.OBJ", VAR_PROGS .. "/develop/libraries/TinyGL/asm_fork/tinygl.obj"},
 {"MEDIA/ANIMAGE", VAR_PROGS .. "/media/animage/trunk/animage"},
 {"MEDIA/KIV", VAR_PROGS .. "/media/kiv/trunk/kiv"},
 {"MEDIA/LISTPLAY", VAR_PROGS .. "/media/listplay/trunk/listplay"},
 {"MEDIA/MIDAMP", VAR_PROGS .. "/media/midamp/trunk/midamp"},
 {"MEDIA/MP3INFO", VAR_PROGS .. "/media/mp3info/mp3info"},
 {"MEDIA/PALITRA", VAR_PROGS .. "/media/palitra/trunk/palitra"},
 {"MEDIA/PIANO", VAR_PROGS .. "/media/piano/piano"},
 {"MEDIA/STARTMUS", VAR_PROGS .. "/media/startmus/trunk/STARTMUS"},
 {"NETWORK/PING", VAR_PROGS .. "/network/ping/ping"},
 {"NETWORK/NETCFG", VAR_PROGS .. "/network/netcfg/netcfg"},
 {"NETWORK/NETSTAT", VAR_PROGS .. "/network/netstat/netstat"},
 {"NETWORK/NSINST", VAR_PROGS .. "/network/netsurf/nsinstall"},
 {"NETWORK/NSLOOKUP", VAR_PROGS .. "/network/nslookup/nslookup"},
 {"NETWORK/PASTA", VAR_PROGS .. "/network/pasta/pasta"},
 {"NETWORK/SYNERGYC", VAR_PROGS .. "/network/synergyc/synergyc"},
 {"NETWORK/SNTP", VAR_PROGS .. "/network/sntp/sntp"},
 {"NETWORK/TELNET", VAR_PROGS .. "/network/telnet/telnet"},
 {"NETWORK/@ZEROCONF", VAR_PROGS .. "/network/zeroconf/zeroconf"},
 {"NETWORK/FTPC", VAR_PROGS .. "/network/ftpc/ftpc"},
 {"NETWORK/FTPD", VAR_PROGS .. "/network/ftpd/ftpd"},
 {"NETWORK/TFTPC", VAR_PROGS .. "/network/tftpc/tftpc"},
 {"NETWORK/IRCC", VAR_PROGS .. "/network/ircc/ircc"},
 {"NETWORK/DOWNLOADER", VAR_PROGS .. "/network/downloader/downloader"},
 {"NETWORK/VNCC", VAR_PROGS .. "/network/vncc/vncc"},
 {"DRIVERS/VIDINTEL.SYS", VAR_DRVS .. "/video/vidintel.sys"},
 {"DRIVERS/3C59X.SYS", VAR_DRVS .. "/ethernet/3c59x.sys"},
 {"DRIVERS/AR81XX.SYS", VAR_DRVS .. "/ethernet/ar81xx.sys"},
 {"DRIVERS/DEC21X4X.SYS", VAR_DRVS .. "/ethernet/dec21x4x.sys"},
 {"DRIVERS/FORCEDETH.SYS", VAR_DRVS .. "/ethernet/forcedeth.sys"},
 {"DRIVERS/I8254X.SYS", VAR_DRVS .. "/ethernet/i8254x.sys"},
 {"DRIVERS/I8255X.SYS", VAR_DRVS .. "/ethernet/i8255x.sys"},
 {"DRIVERS/MTD80X.SYS", VAR_DRVS .. "/ethernet/mtd80x.sys"},
 {"DRIVERS/PCNET32.SYS", VAR_DRVS .. "/ethernet/pcnet32.sys"},
 {"DRIVERS/R6040.SYS", VAR_DRVS .. "/ethernet/R6040.sys"},
 {"DRIVERS/RHINE.SYS", VAR_DRVS .. "/ethernet/rhine.sys"},
 {"DRIVERS/RTL8029.SYS", VAR_DRVS .. "/ethernet/RTL8029.sys"},
 {"DRIVERS/RTL8139.SYS", VAR_DRVS .. "/ethernet/RTL8139.sys"},
 {"DRIVERS/RTL8169.SYS", VAR_DRVS .. "/ethernet/RTL8169.sys"},
 {"DRIVERS/SIS900.SYS", VAR_DRVS .. "/ethernet/sis900.sys"},
 {"DRIVERS/UHCI.SYS", VAR_DRVS .. "/usb/uhci.sys"},
 {"DRIVERS/OHCI.SYS", VAR_DRVS .. "/usb/ohci.sys"},
 {"DRIVERS/EHCI.SYS", VAR_DRVS .. "/usb/ehci.sys"},
 {"DRIVERS/USBHID.SYS", VAR_DRVS .. "/usb/usbhid/usbhid.sys"},
 {"DRIVERS/USBSTOR.SYS", VAR_DRVS .. "/usb/usbstor.sys"},
 {"DRIVERS/RDC.SYS", VAR_DRVS .. "/video/rdc.sys"},
 {"DRIVERS/COMMOUSE.SYS", VAR_DRVS .. "/mouse/commouse.sys"},
 {"DRIVERS/PS2MOUSE.SYS", VAR_DRVS .. "/mouse/ps2mouse4d/trunk/ps2mouse.sys"},
 {"DRIVERS/TMPDISK.SYS", VAR_DRVS .. "/disk/tmpdisk.sys"},
 {"DRIVERS/HDAUDIO.SYS", VAR_DRVS .. "/audio/intel_hda/hdaudio.sys"},
 {"DRIVERS/SB16.SYS", VAR_DRVS .. "/audio/sb16/sb16.sys"},
 {"DRIVERS/SOUND.SYS", VAR_DRVS .. "/audio/sound.sys"},
 {"DRIVERS/INFINITY.SYS", VAR_DRVS .. "/audio/infinity/infinity.sys"},
 {"DRIVERS/AC97.SYS", VAR_DRVS .. "/audio/ac97.sys"},
 {"DRIVERS/EMU10K1X.SYS", VAR_DRVS .. "/audio/emu10k1x.sys"},
 {"DRIVERS/FM801.SYS", VAR_DRVS .. "/audio/fm801.sys"},
 {"DRIVERS/VT823X.SYS", VAR_DRVS .. "/audio/vt823x.sys"},
 {"DRIVERS/SIS.SYS", VAR_DRVS .. "/audio/sis.sys"},
 {"DRIVERS/SDHCI.SYS", "../drivers/sdhci/sdhci.sys"},
})
tup.append_table(extra_files, {
 {"HD_Load/9x2klbr/", VAR_PROGS .. "/hd_load/9x2klbr/9x2klbr.exe"},
 {"HD_Load/MeOSLoad/", VAR_PROGS .. "/hd_load/meosload/MeOSload.com"},
 {"HD_Load/mtldr/", VAR_PROGS .. "/hd_load/mtldr/mtldr"},
 {"HD_Load/", VAR_PROGS .. "/hd_load/mtldr_install/mtldr_install.exe"},
 {"HD_Load/USB_Boot/", VAR_PROGS .. "/hd_load/usb_boot/BOOT_F32.BIN"},
 {"HD_Load/USB_Boot/", VAR_PROGS .. "/hd_load/usb_boot/MTLD_F32"},
 {"HD_Load/USB_Boot/", VAR_PROGS .. "/hd_load/usb_boot/inst.exe"},
 {"HD_Load/USB_Boot/", VAR_PROGS .. "/hd_load/usb_boot/setmbr.exe"},
 {"HD_Load/USB_boot_old/", VAR_PROGS .. "/hd_load/usb_boot_old/MeOSload.com"},
 {"HD_Load/USB_boot_old/", VAR_PROGS .. "/hd_load/usb_boot_old/enable.exe"},
 {"kolibrios/3D/3dsheart", VAR_PROGS .. "/demos/3dsheart/trunk/3dsheart"},
 {"kolibrios/3D/flatwav", VAR_PROGS .. "/demos/flatwav/trunk/flatwav"},
 {"kolibrios/3D/mos3de", VAR_PROGS .. "/demos/mos3de/mos3de"},
 {"kolibrios/3D/info3ds/INFO3DS", VAR_PROGS .. "/develop/info3ds/info3ds"},
 {"kolibrios/3D/textures1", VAR_PROGS .. "/develop/libraries/TinyGL/asm_fork/examples/textures1"},
 {"kolibrios/3D/info3ds/INFO3DS_U", VAR_PROGS .. "/develop/info3ds/info3ds_u"},
 {"kolibrios/3D/voxel_editor/VOXEL_EDITOR", VAR_PROGS .. "/media/voxel_editor/trunk/voxel_editor"},
 {"kolibrios/3D/voxel_utilites/VOX_CREATOR" , VAR_PROGS .. "/media/voxel_editor/utilites/vox_creator"},
 {"kolibrios/3D/voxel_utilites/VOX_MOVER" , VAR_PROGS .. "/media/voxel_editor/utilites/vox_mover"},
 {"kolibrios/3D/voxel_utilites/VOX_TGL" , VAR_PROGS .. "/media/voxel_editor/utilites/vox_tgl"},
 {"kolibrios/demos/life3", VAR_PROGS .. "/games/life3/trunk/life3"},
 {"kolibrios/demos/qjulia", VAR_PROGS .. "/demos/qjulia/trunk/qjulia"},
 {"kolibrios/develop/utils/koldbg", VAR_PROGS .. "/develop/koldbg/koldbg"},
 {"kolibrios/games/Almaz", VAR_PROGS .. "/games/almaz/almaz"},
 {"kolibrios/games/arcanii", VAR_PROGS .. "/games/arcanii/trunk/arcanii"},
 {"kolibrios/games/bomber/bomber", VAR_PROGS .. "/games/bomber/bomber"},
 {"kolibrios/games/bomber/bomberdata.bin", VAR_PROGS .. "/games/bomber/sounds/bomberdata.bin"},
 {"kolibrios/games/codemaster/binary_master", VAR_PROGS .. "/games/codemaster/binary_master"},
 {"kolibrios/games/codemaster/hang_programmer", VAR_PROGS .. "/games/codemaster/hang_programmer"},
 {"kolibrios/games/codemaster/kolibri_puzzle", VAR_PROGS .. "/games/codemaster/kolibri_puzzle"},
 {"kolibrios/games/megamaze", VAR_PROGS .. "/games/megamaze/trunk/megamaze"},
 {"kolibrios/games/invaders", VAR_PROGS .. "/games/invaders/invaders"},
 {"kolibrios/games/phenix", VAR_PROGS .. "/games/phenix/trunk/phenix"},
 {"kolibrios/games/soko/soko", VAR_PROGS .. "/games/soko/trunk/SOKO"},
 {"kolibrios/media/img_transform", VAR_PROGS .. "/media/img_transform/img_transform"},
 {"kolibrios/media/zsea/zsea", VAR_PROGS .. "/media/zsea/zSea"},
 {"kolibrios/media/zsea/plugins/cnv_bmp.obj", VAR_PROGS .. "/media/zsea/plugins/bmp/cnv_bmp.obj"},
 {"kolibrios/media/zsea/plugins/cnv_gif.obj", VAR_PROGS .. "/media/zsea/plugins/gif/cnv_gif.obj"},
 {"kolibrios/media/zsea/plugins/cnv_jpeg.obj", VAR_PROGS .. "/media/zsea/plugins/jpeg/cnv_jpeg.obj"},
 {"kolibrios/media/zsea/plugins/convert.obj", VAR_PROGS .. "/media/zsea/plugins/convert/convert.obj"},
 {"kolibrios/media/zsea/plugins/rotate.obj", VAR_PROGS .. "/media/zsea/plugins/rotate/rotate.obj"},
 {"kolibrios/media/zsea/plugins/scaling.obj", VAR_PROGS .. "/media/zsea/plugins/scaling/scaling.obj"},
 {"kolibrios/utils/AMDtemp", VAR_PROGS .. "/system/amd_temp_view/AMDtemp"},
 {"kolibrios/utils/calcplus", VAR_PROGS .. "/other/calcplus/calcplus"},
 {"kolibrios/utils/kfm/kfm", VAR_PROGS .. "/fs/kfm/trunk/kfm"},
 {"kolibrios/utils/tedit/t_edit", VAR_PROGS .. "/other/t_edit/t_edit"},
})
-- For russian build, add russian-only programs.
if build_type == "ru_RU" then tup.append_table(img_files, {
 {"PERIOD", VAR_PROGS .. "/other/period/trunk/period"},
 {"GAMES/KLAVISHA", VAR_PROGS .. "/games/klavisha/trunk/klavisha"},
 {"DEVELOP/EXAMPLES/TESTCON2", VAR_PROGS .. "/develop/libraries/console_coff/examples/testcon2_rus"},
}) else tup.append_table(img_files, {
 {"DEVELOP/TESTCON2", VAR_PROGS .. "/develop/libraries/console_coff/examples/testcon2_eng"},
}) end

if build_type == "ru_RU" then tup.append_table(extra_files, {
 {"kolibrios/games/Dungeons/Dungeons", VAR_PROGS .. "/games/Dungeons/Dungeons"},
}) end

end -- tup.getconfig('NO_FASM') ~= 'full'

-- Programs that require NASM to compile.
if tup.getconfig('NO_NASM') ~= 'full' then
tup.append_table(img_files, {
 {"ACLOCK", VAR_PROGS .. "/demos/aclock/trunk/aclock"},
 {"LOD", VAR_PROGS .. "/fs/lod/lod"},
 {"TIMER", VAR_PROGS .. "/other/Timer/timer"},
 {"TINFO", VAR_PROGS .. "/system/tinfo/tinfo"},
 {"DEVELOP/MSTATE", VAR_PROGS .. "/develop/mstate/mstate"},
 {"DEVELOP/GENFILES", VAR_PROGS .. "/testing/genfiles/GenFiles"},
 {"GAMES/C4", VAR_PROGS .. "/games/c4/trunk/c4"},
 {"MEDIA/FILLSCR", VAR_PROGS .. "/media/FillScr/fillscr"},
})
tup.append_table(extra_files, {
})
end -- tup.getconfig('NO_NASM') ~= 'full'

-- Programs that require JWASM to compile.
if tup.getconfig('NO_JWASM') ~= 'full' then
tup.append_table(img_files, {
 {"LIB/INPUTBOX.OBJ", VAR_PROGS .. "/develop/libraries/InputBox/INPUTBOX.OBJ"},
})
  if tup.getconfig('NO_GCC') ~= 'full' then
  tup.append_table(img_files, {
   {"RUN", VAR_PROGS .. "/system/RunOD/1/RUN"},
  })
  end
end -- tup.getconfig('NO_JWASM') ~= 'full'

-- Programs that require C-- to compile.
if tup.getconfig('NO_CMM') ~= 'full' then
tup.append_table(img_files, {
 {"@RESHARE", VAR_PROGS .. "/cmm/misc/reshare.com"},
 {"APP_PLUS", VAR_PROGS .. "/cmm/app_plus/app_plus.com"},
 {"EASYSHOT", VAR_PROGS .. "/cmm/misc/easyshot.com"},
 {"MOUSECFG", VAR_PROGS .. "/cmm/mousecfg/mousecfg.com"},
 {"BARSCFG", VAR_PROGS .. "/cmm/barscfg/barscfg.com"},
 {"SYSPANEL", VAR_PROGS .. "/cmm/misc/software_widget.com"},
 {"SYSMON", VAR_PROGS .. "/cmm/sysmon/sysmon.com"},
 {"TMPDISK", VAR_PROGS .. "/cmm/tmpdisk/tmpdisk.com"},
 {"DEVELOP/CLIPVIEW", VAR_PROGS .. "/cmm/clipview/clipview.com"},
 {"DEVELOP/MENU", VAR_PROGS .. "/cmm/menu/menu.com"},
 {"DEVELOP/PIPET", VAR_PROGS .. "/cmm/misc/pipet.com"},
 {"File Managers/EOLITE", VAR_PROGS .. "/cmm/eolite/Eolite.com"},
 {"File Managers/KFM2", VAR_PROGS .. "/cmm/misc/kfm2.com"},
 {"KF_VIEW", VAR_PROGS .. "/cmm/kf_font_viewer/font_viewer.com"},
 {"DEVELOP/DIFF", VAR_PROGS .. "/cmm/diff/diff.com"},
 {"GAMES/CLICKS", VAR_PROGS .. "/games/clicks/trunk/clicks.com"},
 {"GAMES/MBLOCKS", VAR_PROGS .. "/cmm/misc/mblocks.com"},
 {"GAMES/FLOOD-IT", VAR_PROGS .. "/games/flood-it/trunk/flood-it.com"},
 {"GAMES/MINE", VAR_PROGS .. "/games/mine/trunk/mine"},
 {"GAMES/NUMBERS", VAR_PROGS .. "/games/FindNumbers/trunk/FindNumbers"},
 {"MEDIA/PIXIE", VAR_PROGS .. "/cmm/pixie2/pixie.com"},
 {"MEDIA/ICONEDIT", VAR_PROGS .. "/cmm/iconedit/iconedit.com"},
 {"NETWORK/DL", VAR_PROGS .. "/cmm/downloader/dl.com"},
 {"NETWORK/WEBVIEW", VAR_PROGS .. "/cmm/browser/WebView.com"},
})
tup.append_table(extra_files, {
 {"kolibrios/drivers/drvinst.kex", VAR_PROGS .. "/cmm/drvinst/drvinst.com"},
 {"kolibrios/drivers/acpi/install.kex", VAR_PROGS .. "/cmm/misc/acpi_install.com"},
 {"kolibrios/games/pig/pigex", VAR_PROGS .. "/cmm/examples/pigex.com"},
 {"kolibrios/games/the_bus/the_bus", VAR_PROGS .. "/cmm/the_bus/the_bus.com"},
 {"kolibrios/KolibriNext/install.kex", VAR_PROGS .. "/cmm/misc/install.com"},
 {"kolibrios/utils/appearance", VAR_PROGS .. "/cmm/appearance/appearance.com"},
 {"kolibrios/utils/dicty.kex", VAR_PROGS .. "/cmm/dicty/dicty.com"},
 {"kolibrios/utils/notes", VAR_PROGS .. "/cmm/notes/notes.com"},
 {"kolibrios/utils/osupdate", VAR_PROGS .. "/cmm/misc/osupdate.com"},
 {"kolibrios/utils/quark", VAR_PROGS .. "/cmm/quark/quark.com"},
})
end -- tup.getconfig('NO_CMM') ~= 'full'

-- Programs that require MSVC to compile.
if tup.getconfig('NO_MSVC') ~= 'full' then
tup.append_table(img_files, {
 {"GRAPH", VAR_PROGS .. "/other/graph/graph"},
 {"TABLE", VAR_PROGS .. "/other/table/table"},
 {"MEDIA/AC97SND", VAR_PROGS .. "/media/ac97snd/ac97snd.bin"},
 {"GAMES/KOSILKA", VAR_PROGS .. "/games/kosilka/kosilka"},
 {"GAMES/RFORCES", VAR_PROGS .. "/games/rforces/trunk/rforces"},
 {"GAMES/XONIX", VAR_PROGS .. "/games/xonix/trunk/xonix"},
})
tup.append_table(extra_files, {
 {"kolibrios/games/fara/fara", VAR_PROGS .. "/games/fara/trunk/fara"},
 {"kolibrios/games/LaserTank/LaserTank", VAR_PROGS .. "/games/LaserTank/trunk/LaserTank"},
})
end -- tup.getconfig('NO_MSVC') ~= 'full'

-- Programs that require TCC to compile.
if tup.getconfig('NO_TCC') ~= 'full' then
tup.append_table(img_files, {
 {"NETWORK/WHOIS", VAR_PROGS .. "/network/whois/whois"},
 {"SHELL", VAR_PROGS .. "/system/shell/shell"},
})
tup.append_table(extra_files, {
 {"kolibrios/utils/thashview", VAR_PROGS .. "/other/TinyHashView/thashview"},
 {"kolibrios/demos/kmatrix", VAR_PROGS .. "/demos/kmatrix/trunk/kmatrix"},
 {"kolibrios/utils/graph", VAR_PROGS .. "/other/graph/branches/tcc_current/graph"},
 {"kolibrios/develop/TinyBasic/TinyBasic", VAR_PROGS .. "/develop/tinybasic-1.0.4/tinybasic"},
 {"kolibrios/develop/TinyBasic/bas/", SRC_PROGS .. "/develop/tinybasic-1.0.4/bas/*"},
 {"kolibrios/develop/TinyBasic/TinyBasic.man", SRC_PROGS .. "/develop/tinybasic-1.0.4/doc/tinybasic.man"},
-- {"kolibrios/utils/teatool", VAR_PROGS .. "/other/TEAtool/teatool"},
 {"kolibrios/utils/passwordgen", VAR_PROGS .. "/other/PasswordGen/passwordgen"},
 {"kolibrios/utils/kruler", VAR_PROGS .. "/other/kruler/kruler"},
 {"kolibrios/media/qr_tool", SRC_PROGS .. "/media/qr_tool/qr_tool"},
 {"kolibrios/utils/weather", VAR_PROGS .. "/other/Weather/weather"},
 {"kolibrios/settings/weather.json", SRC_PROGS .. "/other/Weather/weather.json"},
 {"kolibrios/utils/man2html", VAR_PROGS .."/other/man2html/man2html"},
})
end -- tup.getconfig('NO_TCC') ~= 'full'

-- Programs that require GCC to compile.
if tup.getconfig('NO_GCC') ~= 'full' then
tup.append_table(img_files, {
 {"GAMES/REVERSI", VAR_PROGS .. "/games/reversi/trunk/reversi"},
 {"LIB/BASE64.OBJ", VAR_PROGS .. "/develop/libraries/base64/base64.obj"},
 {"LIB/LIBC.OBJ", VAR_PROGS .. "/develop/ktcc/trunk/libc.obj/source/libc.obj"},
 {"LIB/ICONV.OBJ", VAR_PROGS .. "/develop/libraries/iconv/iconv.obj"},
 -- {"LIB/MTAR.OBJ", VAR_PROGS .. "/develop/libraries/microtar/mtar.obj"},
})
tup.append_table(extra_files, {
-- {"kolibrios/3D/cubeline", VAR_PROGS .. "/demos/cubeline/trunk/cubeline"},
 {"kolibrios/3D/gears", VAR_PROGS .. "/demos/gears/gears"},
 {"kolibrios/emul/e80/e80", VAR_PROGS .. "/emulator/e80/trunk/e80"},
 {"kolibrios/emul/uarm/", VAR_CONTRIB .. "/other/uarm/uARM"},
 {"kolibrios/games/2048", VAR_PROGS .. "/games/2048/2048"},
 {"kolibrios/games/checkers", VAR_PROGS .. "/games/checkers/trunk/checkers"},
 {"kolibrios/games/donkey", VAR_PROGS .. "/games/donkey/donkey"},
 {"kolibrios/games/heliothryx", VAR_PROGS .. "/games/heliothryx/heliothryx"},
 {"kolibrios/games/marblematch3", VAR_PROGS .. "/games/marblematch3/marblematch3"},
 {"kolibrios/games/nsider", VAR_PROGS .. "/games/nsider/nsider"},
 {"kolibrios/games/fridge/", VAR_PROGS .. "/games/fridge/fridge"},
 {"kolibrios/develop/lua/lua", VAR_CONTRIB .. "/other/lua-5.2.0/lua"},
 {"kolibrios/develop/lua/calc.lua", "../contrib/other/lua-5.2.0/calc.lua"},
 {"kolibrios/develop/lua/console.lua", "../contrib/other/lua-5.2.0/console.lua"},
 {"kolibrios/lib/libc.dll", VAR_PROGS .. "/../contrib/sdk/bin/libc.dll", group = "../contrib/sdk/lib/<libc.dll.a>"},
 {"kolibrios/lib/dr_flac.obj", VAR_CONTRIB .. "/media/dr_flac/dr_flac.obj"},
 {"kolibrios/lib/minimp3.obj", VAR_CONTRIB .. "/media/minimp3/minimp3.obj"},
 {"kolibrios/lib/sqlite3.dll", VAR_CONTRIB .. "/sdk/bin/sqlite3.dll", group = "../contrib/sdk/lib/<libsqlite3.dll.a>"},
 {"kolibrios/lib/stb_vorbis.obj", VAR_CONTRIB .. "/media/stb_vorbis/stb_vorbis.obj"},
 {"kolibrios/utils/minizip/minizip", VAR_PROGS .. "/fs/minizip/minizip"},
 {"kolibrios/utils/minizip/miniunz", VAR_PROGS .. "/fs/minizip/miniunz"},
 {"kolibrios/develop/c--/c--", VAR_PROGS .. "/develop/cmm/cmm"},
 {"kolibrios/develop/tcc/tcc", VAR_PROGS .. "/develop/ktcc/trunk/source/tcc"},
 {"kolibrios/develop/sqlite3/sqlite3", VAR_CONTRIB .. "/sdk/sources/sqlite3/shell/sqlite3"},
 {"kolibrios/develop/utils/objconv", VAR_PROGS .. "/develop/objconv/objconv"},
 {"kolibrios/drivers/sensors/k10temp.sys", VAR_DRVS .. "/sensors/k10temp/k10temp.sys"},
 {"kolibrios/drivers/acpi/acpi.sys", VAR_DRVS .. "/devman/acpi.sys"},
 {"kolibrios/drivers/acpi/acpi", VAR_DRVS .. "/devman/acpi"},
 {"kolibrios/drivers/geode/geode.sys", "common/drivers/geode/geode.sys"}, -- there is also an autobuid version that is not working
 {"kolibrios/drivers/geode/geode", VAR_DRVS .. "/audio/a5536/geode"},
})
if tup.getconfig('NO_NASM') ~= 'full' then
  tup.append_table(extra_files, {
   {"kolibrios/games/tyrian/", VAR_CONTRIB .. "/games/opentyrian/opentyrian"},
   {"kolibrios/games/tyrian/data/", "common/games/tyrian/data/*"},
   {"kolibrios/games/quake/", "common/games/quake/*"}, -- not really gcc, but no sense without sdlquake
   {"kolibrios/games/quake/", VAR_CONTRIB .. "/other/sdlquake-1.0.9/sdlquake"},
   {"kolibrios/games/wolf3d/", VAR_CONTRIB .. "/games/wolf3d/wolf3d"},
   {"kolibrios/games/wolf3d/", "common/games/wolf3d/*"},
   {"kolibrios/emul/dgen/dgen", VAR_PROGS .. "/emulator/dgen-sdl-1.33/dgen"},
   {"kolibrios/emul/dgen/dgen.html", SRC_PROGS .. "/emulator/dgen-sdl-1.33/dgen.html"},
   {"kolibrios/emul/dgen/dgenrc.html", SRC_PROGS .. "/emulator/dgen-sdl-1.33/dgenrc.html"},
  })
end
-- For russian build, add russian-only programs.
if build_type == "ru_RU" then tup.append_table(extra_files, {
 {"kolibrios/games/21days", VAR_PROGS .. "/games/21days/21days"},
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
  table.insert(extra_files, {"kolibrios/res/skins/", VAR_SKINS .. "/" .. v})
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
make_img_command1 = '^ MKIMG kolibri.img^ ' -- for tup: don't write full command to logs
make_img_command1 += "dd status=none if=/dev/zero of=%o count=2880 bs=512"
-- format it as a standard 1.44M floppy
make_img_command1 += " && mformat -f 1440 -i %o ::"
-- copy bootloader
if tup.getconfig("NO_FASM") ~= "full" then
bootloader = VAR_KERNEL .. "/bootloader/boot_fat12.bin"
make_img_command1 += " && dd status=none if=" .. bootloader .. " of=%o count=1 bs=512 conv=notrunc"
table.insert(input_deps, bootloader)
end
-- make folders
table.sort(img_dirs)
for i,v in ipairs(img_dirs) do
  if v ~= img_dirs[i-1] then
    make_img_command1 += ' && mmd -i %o "::' .. v .. '"'
  end
end
-- copy files
for i,v in ipairs(img_files) do
  local_file = v[2]
  if v[1] == "KERNEL.MNT"
  then
    -- for kernel.mnt, insert autobuild commit id to .kernel.mnt
    -- note that .kernel.mnt must begin with a dot to prevent
    -- tup from tracking it
    cmd = "cp %f %o"
    if tup.getconfig("INSERT_COMMIT_ID") ~= ""
    then
      if build_type == "ru_RU"
      then str='$(LANG=ru_RU.utf8 date -u +"[автосборка %d %b %Y %R, r$(get-current-cmtid)]"|iconv -f utf8 -t cp866)'
      else str='$(date -u +"[auto-build %d %b %Y %R, r$(get-current-cmtid)]")'
      end
      str = string.gsub(str, "%$", "\\$") -- escape $ as \$
      str = string.gsub(str, "%%", "%%%%") -- escape % as %%
      cmd += " && str=" .. str
      cmd += ' && echo -n $str | dd status=none of=%o bs=1 seek=`expr 274 - length "$str"` conv=notrunc'
    end
    local_file = VAR_KERNEL .. "/.kernel.mnt"
    tup.definerule{inputs = {v[2]}, command = cmd, outputs = {local_file}}
  end
  table.insert(input_deps, local_file)
  make_img_command1 += ' && mcopy -moi %o "' .. local_file .. '" "::' .. v[1] .. '"'
end

-- generate tup rule for kolibri.img
tup.definerule{inputs = input_deps, command = make_img_command1, outputs = {"kolibri.img"}}

-- generate command and dependencies for mkisofs
input_deps = {VAR_DATA .. "/kolibri.img"}
iso_files_list = ""
for i,v in ipairs(iso_extra_files) do
  iso_files_list = iso_files_list .. ' "' .. v[1] .. '=' .. v[2] .. '"'
  table.insert(input_deps, v.group or v[2])
end

-- generate tup rule for kolibri.iso
if tup.getconfig("INSERT_COMMIT_ID") ~= ""
then volume_id = "KolibriOS r`get-current-cmtid`"
else volume_id = "KolibriOS"
end
tup.definerule{inputs = input_deps, command =
  '^ MKISOFS kolibri.iso^ ' .. -- for tup: don't write full command to logs
  'mkisofs -U -J -pad -b kolibri.img -c boot.catalog -hide-joliet boot.catalog -graft-points ' ..
  '-A "KolibriOS AutoBuilder" -p "CleverMouse" -publisher "KolibriOS Team" -V "' .. volume_id .. '" -sysid "KOLIBRI" ' ..
  '-iso-level 3 -o %o ' .. VAR_DATA .. '/kolibri.img' .. iso_files_list .. ' 2>&1',
  outputs = {"kolibri.iso"}}

-- generate command and dependencies for distribution kit
cp = 'cp "%f" "%o"'
tup.definerule{inputs = {VAR_DATA .. "/kolibri.img"}, command = cp, outputs = {"distribution_kit/kolibri.img"}}
for i,v in ipairs(distr_extra_files) do
  cmd = cp:gsub("%%f", string.gsub(v[2], "%%", "%%%%")) -- input can be a group, we can't rely on tup's expansion of %f in this case
  if string.sub(v[1], -1) == "/"
  then tup.definerule{inputs = {v.group or v[2]}, command = cmd, outputs = {"distribution_kit/" .. v[1] .. tup.file(v[2])}}
  else tup.definerule{inputs = {v.group or v[2]}, command = cmd, outputs = {"distribution_kit/" .. v[1]}}
  end
end

-- build kolibri.raw
VAR_KOLIBRI_RAW = VAR_DATA .. "/kolibri.raw"
raw_mbr = SRC_PROGS .. "/hd_load/usb_boot/mbr"
raw_bootsector = VAR_KERNEL .. "/bootloader/extended_primary_loader/fat32/bootsect.bin"
raw_files = {
 {"KOLIBRI.IMG", VAR_DATA .. "/kolibri.img"},
 {"KORDLDR.F32", VAR_KERNEL .. "/bootloader/extended_primary_loader/fat32/kordldr.f32"},
 {"KERNEL.MNT", VAR_KERNEL .. "/kernel.mnt.ext_loader"},
 {"CONFIG.INI", SRC_KERNEL .. "/bootloader/extended_primary_loader/config.ini"},
 {"EFI/BOOT/BOOTX64.EFI", VAR_KERNEL .. "/bootloader/uefi4kos/bootx64.efi"},
 {"EFI/BOOT/BOOTIA32.EFI", VAR_KERNEL .. "/bootloader/uefi4kos/bootia32.efi"},
 {"EFI/KOLIBRIOS/KOLIBRI.IMG", VAR_DATA .. "/kolibri.img"},
 {"EFI/KOLIBRIOS/KOLIBRI.INI", SRC_KERNEL .. "/bootloader/uefi4kos/kolibri.ini"},
 {"EFI/KOLIBRIOS/KOLIBRI.KRN", VAR_KERNEL .. "/kernel.mnt.ext_loader"}
}

for i,v in ipairs(img_files) do
  raw_file = "KOLIBRIOS/" .. string.upper(v[1])
  local_file = v[2]
  tup.append_table(raw_files, {{raw_file, local_file}})
end

tup.append_table(raw_files, extra_files)

make_raw_command1 = '^ MKRAW kolibri.raw^ ' -- for tup: don't write full command to logs
make_raw_command1 += "dd status=none if=/dev/zero of=" .. VAR_KOLIBRI_RAW .. " bs=1MiB count=128 2>&1"
make_raw_command1 += " && parted --script " .. VAR_KOLIBRI_RAW .. " mktable gpt"
make_raw_command1 += " && parted --script " .. VAR_KOLIBRI_RAW .. " unit MiB mkpart primary fat32 1 127"
make_raw_command1 += " && parted --script " .. VAR_KOLIBRI_RAW .. " set 1 esp on"
make_raw_command1 += " && sgdisk " .. VAR_KOLIBRI_RAW .. " --hybrid 1:EE"
make_raw_command1 += " && dd status=none if=" .. raw_mbr .. " of=" .. VAR_KOLIBRI_RAW .. " bs=1 count=\\$((0x1b8)) conv=notrunc"
make_raw_command1 += " && dd status=none if=" .. raw_mbr .. " of=" .. VAR_KOLIBRI_RAW .. " bs=1 count=1 skip=\\$((0x5a)) seek=\\$((0x1be)) conv=notrunc"
make_raw_command1 += " && mformat -i " .. VAR_KOLIBRI_RAW .. "@@1M -v KOLIBRIOS -T \\$(((128-1-1)*1024*1024/512)) -h 16 -s 32 -H 2048 -c 1 -F -B " .. raw_bootsector .. " ::"

-- generate list of directories to be created inside kolibri.raw
raw_dirs = {}
input_deps = {raw_mbr, raw_bootsector}
for i,v in ipairs(raw_files) do
  raw_file = v[1]
  local_file = v[2]

  cur_dir = ""
  for dir in string.gmatch(raw_file, "([^/]+)/") do
    cur_dir = cur_dir .. "/" .. string.upper(dir)
    raw_dirs[cur_dir] = true
  end

  -- tup does not want to see hidden files as dependencies
  if not string.match(local_file, "/%.") then
    table.insert(input_deps, v.group or local_file)
  end
end

-- Sorting is needed to mkdir /one before /one/two
raw_dirs_sorted = {}
for k in pairs(raw_dirs) do table.insert(raw_dirs_sorted, k) end
table.sort(raw_dirs_sorted)
raw_dirs = raw_dirs_sorted

-- make folders
table.sort(raw_dirs)
for _, dir in pairs(raw_dirs) do
  make_raw_command1 += ' && mmd -i ' .. VAR_KOLIBRI_RAW .. '@@1M "::' .. dir .. '"'
end

-- Put copying of raw_files into separate scripts to avoid 'execl: Argument list too long'
make_raw_command2 = "true"
make_raw_command3 = "true"
-- copy files
for i,v in ipairs(raw_files) do
  local_file = v[2]
  cmd = ' && mcopy -moi ' .. VAR_KOLIBRI_RAW .. '@@1M "' .. local_file .. '" "::' .. v[1] .. '"'
  if i < 900 then -- 900 commands require ~100kiB which is below 128kiB with some margin
    make_raw_command2 += cmd
  else
    make_raw_command3 += cmd
  end
  table.insert(input_deps, v.group or local_file)
end

tup.definerule{inputs = {}, command = "echo '" .. make_raw_command1 .. "' > %o", outputs = {'make_raw_command1_file'}}
tup.definerule{inputs = {}, command = "echo '" .. make_raw_command2 .. "' > %o", outputs = {'make_raw_command2_file'}}
tup.definerule{inputs = {}, command = "echo '" .. make_raw_command3 .. "' > %o", outputs = {'make_raw_command3_file'}}

table.insert(input_deps, 'make_raw_command1_file')
table.insert(input_deps, 'make_raw_command2_file')
table.insert(input_deps, 'make_raw_command3_file')
-- generate tup rule for kolibri.raw
tup.definerule{inputs = input_deps, command = make_raw_command1 .. " && bash " .. VAR_DATA .. "/make_raw_command2_file && bash " .. VAR_DATA .. "/make_raw_command3_file", outputs = {"kolibri.raw"}}

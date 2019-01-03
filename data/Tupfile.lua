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
 {"FB2READ", "common/fb2read"},
 {"ALLGAMES", "common/allgames"},
 {"ICONS32.PNG", "common/icons32.png"},
 {"ICONS16.PNG", "common/icons16.png"},
 {"INDEX.HTM", "common/index_htm"},
 {"KUZKINA.MID", "common/kuzkina.mid"},
 {"LANG.INC", build_type .. "/lang.inc"},
 {"NOTIFY3.PNG", "common/notify3.png"},
 {"TOOLBAR.PNG", PROGS .. "/cmm/txtread/toolbar.png"},
 {"VMODE", "common/vmode"},
 {"3D/HOUSE.3DS", "common/3d/house.3ds"},
 {"DEVELOP/T_EDIT.INI", PROGS .. "/other/t_edit/t_edit.ini"},
 {"File Managers/ICONS.INI", "common/File Managers/icons.ini"},
 {"File Managers/KFM.INI", "common/File Managers/kfm.ini"},
 {"File Managers/BUTTONS.BMP", PROGS .. "/fs/kfm/trunk/buttons.bmp"},
 {"File Managers/ICONS.BMP", PROGS .. "/fs/kfm/trunk/icons.bmp"},
 {"File Managers/FNAV/ABOUT.TXT", "common/File Managers/fNav/About.txt"},
 {"File Managers/FNAV/FNAV", "common/File Managers/fNav/fNav.kex"},
 {"File Managers/FNAV/FNAV.EXT", "common/File Managers/fNav/fnav.ext"},
 {"File Managers/FNAV/FNAV.SET", "common/File Managers/fNav/fnav.set"},
 {"File Managers/FNAV/FNAV_CUR.PNG", "common/File Managers/fNav/fnav_cur.png"},
 {"File Managers/FNAV/FNAV_FNT.PNG", "common/File Managers/fNav/fnav_fnt.png"},
 {"File Managers/FNAV/FNAV_ICN.PNG", "common/File Managers/fNav/fnav_icn.png"},
 {"FONTS/TAHOMA.KF", "common/fonts/tahoma.kf"},
 {"LIB/ICONV.OBJ", "common/lib/iconv.obj"},
 {"LIB/KMENU.OBJ", "common/lib/kmenu.obj"},
 {"LIB/NETCODE.OBJ", "common/lib/netcode.obj"},
 {"LIB/PIXLIB.OBJ", "common/lib/pixlib.obj"},
 {"MEDIA/IMGF/IMGF", "common/media/ImgF/ImgF"},
 {"MEDIA/IMGF/CEDG.OBJ", "common/media/ImgF/cEdg.obj"},
 {"MEDIA/IMGF/DITHER.OBJ", "common/media/ImgF/dither.obj"},
 {"MEDIA/IMGF/INVSOL.OBJ", "common/media/ImgF/invSol.obj"},
 {"MEDIA/PIXIE/SKIN.PNG", PROGS .. "/cmm/pixie2/skin.png"},
 {"NETWORK/FTPC.INI", PROGS .. "/network/ftpc/ftpc.ini"},
 {"NETWORK/FTPD.INI", "common/network/ftpd.ini"},
 {"NETWORK/USERS.INI", "common/network/users.ini"},
 {"NETWORK/TL_SYS_16.PNG", PROGS .. "/network/ftpc/tl_sys_16.png"},
 {"NETWORK/TL_NOD_16.PNG", PROGS .. "/network/ftpc/tl_nod_16.png"},
 {"SETTINGS/APP.INI", "common/settings/app.ini"},
 {"SETTINGS/APP_PLUS.INI", "common/settings/app_plus.ini"},
 {"SETTINGS/ASSOC.INI", "common/settings/assoc.ini"},
 {"SETTINGS/AUTORUN.DAT", "common/settings/AUTORUN.DAT"},
 {"SETTINGS/DOCKY.INI", "common/settings/docky.ini"},
 {"SETTINGS/ESKIN.INI", "common/settings/eskin.ini"},
 {"SETTINGS/FB2READ.INI", "common/settings/fb2read.ini"},
 {"SETTINGS/HOTANGLES.CFG", PROGS .. "/other/ha/SETTINGS/HOTANGLES.CFG"},
 {"SETTINGS/ICON.INI", build_type .. "/settings/icon.ini"},
 {"SETTINGS/KEYMAP.KEY", PROGS .. "/system/taskbar/trunk/KEYMAP.KEY"},
 {"SETTINGS/KOLIBRI.LBL", build_type .. "/settings/kolibri.lbl"},
 {"SETTINGS/LANG.INI", build_type .. "/settings/lang.ini"},
 {"SETTINGS/MENU.DAT", build_type .. "/settings/menu.dat"},
 {"SETTINGS/NETWORK.INI", "common/settings/network.ini"},
 {"SETTINGS/SYSPANEL.INI", "common/settings/syspanel.ini"},
 {"SETTINGS/SYSTEM.INI", "common/settings/system.ini"},
 {"SETTINGS/TASKBAR.INI", "common/settings/taskbar.ini"},
}

-- For russian build, add russian-only files.
if build_type == "rus" then tup.append_table(img_files, {
 {"File Managers/KFM_KEYS.TXT", PROGS .. "/fs/kfm/trunk/docs/russian/dos_kolibri/kfm_keys.txt"},
 {"SETTINGS/.shell", PROGS .. "/system/shell/bin/rus/.shell"},
 {"SETTINGS/GAMES.INI", "rus/settings/games.ini"},
 {"SETTINGS/MYKEY.INI", PROGS .. "/system/MyKey/trunk/mykey.ini"},
 {"EXAMPLE.ASM", PROGS .. "/develop/examples/example/trunk/rus/example.asm"},
 {"PIPETKA", build_type .. "/pipetka"},
 {"File Managers/KFAR.INI", build_type .. "/File Managers/kfar.ini"},
 {"GAMES/ATAKA", build_type .. "/games/ataka"},
 {"GAMES/BASEKURS.KLA", build_type .. "/games/basekurs.kla"},
 {"GAMES/PADENIE", build_type .. "/games/padenie"}, 
}) elseif build_type == "eng" then tup.append_table(img_files, {
 {"File Managers/KFM_KEYS.TXT", PROGS .. "/fs/kfm/trunk/docs/english/kfm_keys.txt"},
 {"SETTINGS/GAMES.INI", "common/settings/games.ini"},
 {"SETTINGS/.shell", PROGS .. "/system/shell/bin/eng/.shell"},
 {"EXAMPLE.ASM", PROGS .. "/develop/examples/example/trunk/example.asm"},
 {"SETTINGS/MYKEY.INI", PROGS .. "/system/MyKey/trunk/mykey.ini"},
 {"File Managers/KFAR.INI", "common/File Managers/kfar.ini"}, 
}) elseif build_type == "sp" then tup.append_table(img_files, {
 {"File Managers/KFM_KEYS.TXT", PROGS .. "/fs/kfm/trunk/docs/english/kfm_keys.txt"},
 {"SETTINGS/GAMES.INI", "common/settings/games.ini"},
 {"SETTINGS/.shell", PROGS .. "/system/shell/bin/eng/.shell"},
 {"EXAMPLE.ASM", PROGS .. "/develop/examples/example/trunk/example.asm"},
 {"SETTINGS/MYKEY.INI", PROGS .. "/system/MyKey/trunk/mykey.ini"},
 {"File Managers/KFAR.INI", "common/File Managers/kfar.ini"}, 
}) elseif build_type == "it" then tup.append_table(img_files, {
 {"File Managers/KFM_KEYS.TXT", PROGS .. "/fs/kfm/trunk/docs/english/kfm_keys.txt"},
 {"SETTINGS/GAMES.INI", "common/settings/games.ini"},
 {"SETTINGS/.shell", PROGS .. "/system/shell/bin/eng/.shell"},
 {"EXAMPLE.ASM", PROGS .. "/develop/examples/example/trunk/example.asm"},
 {"SETTINGS/MYKEY.INI", PROGS .. "/system/MyKey/trunk/mykey_it.ini"},
 {"File Managers/KFAR.INI", "common/File Managers/kfar.ini"},
}) else tup.append_table(img_files, {
 {"File Managers/KFM_KEYS.TXT", PROGS .. "/fs/kfm/trunk/docs/english/kfm_keys.txt"},
 {"SETTINGS/GAMES.INI", "common/settings/games.ini"},
 {"SETTINGS/.shell", PROGS .. "/system/shell/bin/eng/.shell"},
 {"EXAMPLE.ASM", PROGS .. "/develop/examples/example/trunk/example.asm"},
 {"SETTINGS/MYKEY.INI", PROGS .. "/system/MyKey/trunk/mykey.ini"},
 {"File Managers/KFAR.INI", "common/File Managers/kfar.ini"},
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
 {"/", build_type .. "/settings/kolibri.lbl"},
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
 {"kolibrios/3D/info3ds/INFO3DS.INI", PROGS .. "/develop/info3ds/info3ds.ini"},
 {"kolibrios/3D/info3ds/OBJECTS.PNG", PROGS .. "/develop/info3ds/objects.png"},
 {"kolibrios/3D/info3ds/TL_SYS_16.PNG", PROGS .. "/develop/info3ds/tl_sys_16.png"},
 {"kolibrios/3D/info3ds/TOOLBAR.PNG", PROGS .. "/develop/info3ds/toolbar.png"},
 {"kolibrios/3D/info3ds/FONT8X9.BMP", PROGS .. "/fs/kfar/trunk/font8x9.bmp"},
 {"kolibrios/3D/md2view/", "common/3d/md2view/*"},
 {"kolibrios/3D/md2view/md2_model/", "common/3d/md2view/md2_model/*"},
 {"kolibrios/3D/voxel_editor/VOX_EDITOR.INI", PROGS .. "/media/voxel_editor/trunk/vox_editor.ini"},
 {"kolibrios/3D/voxel_editor/HOUSE1.VOX", PROGS .. "/media/voxel_editor/trunk/house1.vox"},
 {"kolibrios/3D/voxel_editor/HOUSE2.VOX", PROGS .. "/media/voxel_editor/trunk/house2.vox"},
 {"kolibrios/3D/voxel_editor/SQUIRREL.VOX", PROGS .. "/media/voxel_editor/trunk/squirrel.vox"},
 {"kolibrios/3D/voxel_utilites/VOX_MOVER.INI" , PROGS .. "/media/voxel_editor/utilites/vox_mover.ini"},
 {"kolibrios/3D/FONT8X9.BMP", PROGS .. "/fs/kfar/trunk/font8x9.bmp"},
 {"kolibrios/3D/TOOLB_1.PNG", PROGS .. "/develop/libraries/TinyGL/asm_fork/examples/toolb_1.png"},
 {"kolibrios/3D/TEST_GLU1", PROGS .. "/develop/libraries/TinyGL/asm_fork/examples/test_glu1"},
 {"kolibrios/3D/TEST_GLU2", PROGS .. "/develop/libraries/TinyGL/asm_fork/examples/test_glu2"},
 {"kolibrios/3D/TEXT_2.PNG", PROGS .. "/develop/libraries/TinyGL/asm_fork/examples/text_2.png"},
 {"kolibrios/develop/oberon07/", PROGS .. "/develop/oberon07/*"},
 {"kolibrios/develop/oberon07/Docs/", PROGS .. "/develop/oberon07/Docs/*"},
 {"kolibrios/develop/oberon07/Lib/KolibriOS/", PROGS .. "/develop/oberon07/Lib/KolibriOS/*"},
 {"kolibrios/develop/oberon07/Samples/", PROGS .. "/develop/oberon07/Samples/*"},
 {"kolibrios/develop/c--/", PROGS .. "/cmm/c--/*"},
 {"kolibrios/develop/tcc/", "common/develop/tcc/*"},
 {"kolibrios/develop/TinyBasic/", "common/develop/TinyBasic/*"},
 {"kolibrios/utils/cnc_editor/cnc_editor", PROGS .. "/other/cnc_editor/cnc_editor"},
 {"kolibrios/utils/cnc_editor/kolibri.NC", PROGS .. "/other/cnc_editor/kolibri.NC"},
 {"kolibrios/emul/dosbox/", "common/emul/DosBox/*"},
 {"kolibrios/emul/e80/readme.txt", PROGS .. "/emulator/e80/trunk/readme.txt"},
 {"kolibrios/emul/fceu/fceu", PROGS .. "/emulator/fceu/fceu"},
 {"kolibrios/emul/fceu/FCEU ReadMe.txt", PROGS .. "/emulator/fceu/FCEU ReadMe.txt"},
 {"kolibrios/emul/", "common/emul/gameboy"},
 {"kolibrios/emul/", "common/emul/pokemini"},
 {"kolibrios/emul/", "common/emul/scummvm"},
 {"kolibrios/emul/", "common/emul/zsnes"},
 {"kolibrios/demos/ak47.lif", "common/demos/ak47.lif"},
 {"kolibrios/demos/life2", "common/demos/life2"},
 {"kolibrios/demos/life3tb.png", PROGS .. "/games/life3/trunk/life3tb.png"},
 {"kolibrios/demos/relay.lif", "common/demos/relay.lif"},
 {"kolibrios/demos/rpento.lif", "common/demos/rpento.lif"},
 {"kolibrios/games/BabyPainter", "common/games/BabyPainter"},
 {"kolibrios/games/bomber/ackack.bmp", PROGS .. "/games/bomber/ackack.bmp"},
 {"kolibrios/games/bomber/bomb.bmp", PROGS .. "/games/bomber/bomb.bmp"},
 {"kolibrios/games/bomber/plane.bmp", PROGS .. "/games/bomber/plane.bmp"},
 {"kolibrios/games/bomber/tile.bmp", PROGS .. "/games/bomber/tile.bmp"},
 {"kolibrios/games/doom1/", "common/games/doom/*"},
 {"kolibrios/games/fara/fara.gfx", "common/games/fara.gfx"},
 {"kolibrios/games/jumpbump/", "common/games/jumpbump/*"},
 {"kolibrios/games/knight", "common/games/knight"},
 {"kolibrios/games/LaserTank/", "common/games/LaserTank/*"},
 {"kolibrios/games/lrl/", "common/games/lrl/*"},
 {"kolibrios/games/mun/data/", "common/games/mun/data/*"},
 {"kolibrios/games/mun/libc.dll", "common/games/mun/libc.dll"},
 {"kolibrios/games/mun/mun", "common/games/mun/mun"}, 
 {"kolibrios/games/pig/", "common/games/pig/*"},
 {"kolibrios/games/soko/", "common/games/soko/*"},
 {"kolibrios/games/sstartrek/", "common/games/sstartrek/*"},
 {"kolibrios/games/the_bus/menu.png", PROGS .. "/cmm/the_bus/menu.png"},
 {"kolibrios/games/the_bus/objects.png", PROGS .. "/cmm/the_bus/objects.png"},
 {"kolibrios/games/the_bus/road.png", PROGS .. "/cmm/the_bus/road.png"},
 {"kolibrios/grafx2/fonts/", "common/media/grafx2/fonts/*"},
 {"kolibrios/grafx2/scripts/", "common/media/grafx2/scripts/libs/*"},
 {"kolibrios/grafx2/scripts/libs/", "common/media/grafx2/scripts/*"},
 {"kolibrios/grafx2/skins/", "common/media/grafx2/skins/*"},
 {"kolibrios/grafx2/", "common/media/grafx2/*"},
 {"kolibrios/drivers/drvinf.ini", "common/drivers/drvinf.ini"},
 {"kolibrios/drivers/ahci/", "common/drivers/ahci/*"},
 {"kolibrios/drivers/atikms/", "common/drivers/atikms/*"},
 {"kolibrios/drivers/i915/", "common/drivers/i915/*"},
 {"kolibrios/drivers/vmware/", "common/drivers/vmware/*"},
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
 {"kolibrios/lib/libc.dll", "../contrib/sdk/bin/libc.dll", group = "../contrib/sdk/lib/<libc.dll.a>"},
 {"kolibrios/lib/osmesa.dll", "common/lib/osmesa.dll"},
 {"kolibrios/lib/pixlib-gl.dll", "common/lib/pixlib-gl.dll"},
 {"kolibrios/lib/pixman-1.dll", "common/lib/pixman-1.dll"},
 {"kolibrios/lib/swresample-1.dll", "common/lib/swresample-1.dll"},
 {"kolibrios/lib/i915_dri.drv", "common/lib/i915_dri.drv"},
 {"kolibrios/media/fplay", "common/media/fplay"},
 {"kolibrios/media/fplay_run", "common/media/fplay_run"},
 {"kolibrios/media/updf", "common/media/updf"},
 {"kolibrios/media/vttf", "common/media/vttf"},
 {"kolibrios/media/zsea/zsea.ini", PROGS .. "/media/zsea/zSea.ini"},
 {"kolibrios/media/zsea/buttons/buttons.png", PROGS .. "/media/zsea/buttons.png"},
 {"kolibrios/res/skins/", "../skins/authors.txt"},
 {"kolibrios/res/templates/", "common/templates/*"},
 {"kolibrios/res/wallpapers/", "common/wallpapers/*"},
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
 {"kolibrios/games/Dungeons/Resources/Textures/Environment/", PROGS .. "/games/Dungeons/Resources/Textures/Environment/*"},
 {"kolibrios/games/Dungeons/Resources/Textures/Objects/", PROGS .. "/games/Dungeons/Resources/Textures/Objects/*"},
 {"kolibrios/games/Dungeons/Resources/Textures/HUD/", PROGS .. "/games/Dungeons/Resources/Textures/HUD/*"},
 {"kolibrios/games/Dungeons/Resources/Textures/", PROGS .. "/games/Dungeons/Resources/Textures/Licenses.txt"},
 {"kolibrios/games/Dungeons/", PROGS .. "/games/Dungeons/readme_ru.txt"},
 {"kolibrios/games/WHOWTBAM/", build_type .. "/games/whowtbam"},
 {"kolibrios/games/WHOWTBAM/", build_type .. "/games/appdata.dat"},
 {"kolibrios/media/zsea/zsea_keys.txt", PROGS .. "/media/zsea/Docs/zSea_keys_rus.txt"},
 {"kolibrios/res/guide/", build_type .. "/docs/guide/*"}, 
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
 {"@DOCKY", PROGS .. "/system/docky/trunk/docky"},
 {"@HOTANGLES", PROGS .. "/other/ha/HOTANGLES"},
 {"@ICON", PROGS .. "/system/icon_new/icon"},
 {"@MENU", PROGS .. "/system/menu/trunk/menu"},
 {"@NOTIFY", PROGS .. "/system/notify3/notify"},
 {"@OPEN", PROGS .. "/system/open/open"},
 {"@TASKBAR", PROGS .. "/system/taskbar/trunk/TASKBAR"},
 {"@SS", PROGS .. "/system/ss/trunk/@ss"},
 {"@VOLUME", PROGS .. "/media/volume/volume"},
 {"HACONFIG", PROGS .. "/other/ha/HACONFIG"},
 {"ASCIIVJU", PROGS .. "/develop/asciivju/trunk/asciivju"},
 {"CALC", PROGS .. "/other/calc/trunk/calc"},
 {"CALENDAR", PROGS .. "/system/calendar/trunk/calendar"},
 {"COLRDIAL", PROGS .. "/system/colrdial/color_dialog"},
 {"CROPFLAT", PROGS .. "/system/cropflat/cropflat"},
 {"CPU", PROGS .. "/system/cpu/trunk/cpu"},
 {"CPUID", PROGS .. "/system/cpuid/trunk/CPUID"},
 {"DOCPACK", PROGS .. "/system/docpack/trunk/docpack"},
 {"DEFAULT.SKN", "../skins/Leency/Shkvorka/Shkvorka.skn"},
 {"DISPTEST", PROGS .. "/system/disptest/trunk/disptest"},
 {"END", PROGS .. "/system/end/light/end"},
 {"ESKIN", PROGS .. "/system/eskin/trunk/eskin"},
 {"FSPEED", PROGS .. "/fs/fspeed/fspeed"},
 {"GMON", PROGS .. "/system/gmon/gmon"},
 {"HDD_INFO", PROGS .. "/system/hdd_info/trunk/hdd_info"},
 {"KBD", PROGS .. "/system/kbd/trunk/kbd"},
 {"KPACK", PROGS .. "/other/kpack/trunk/kpack"},
 {"KERPACK", PROGS .. "/other/outdated/kerpack/trunk/kerpack"},
 {"LAUNCHER", PROGS .. "/system/launcher/trunk/launcher"},
 {"LOADDRV", PROGS .. "/system/loaddrv/loaddrv"},
 {"MAGNIFY", PROGS .. "/demos/magnify/trunk/magnify"},
 {"MGB", PROGS .. "/system/mgb/trunk/mgb"},
 {"MOUSEMUL", PROGS .. "/system/mousemul/trunk/mousemul"},
 {"MADMOUSE", PROGS .. "/other/madmouse/madmouse"},
 {"MYKEY", PROGS .. "/system/MyKey/trunk/MyKey"},
 {"PCIDEV", PROGS .. "/system/pcidev/trunk/PCIDEV"},
 {"RDSAVE", PROGS .. "/system/rdsave/trunk/rdsave"},
 {"RTFREAD", PROGS .. "/other/rtfread/trunk/rtfread"},
 {"SEARCHAP", PROGS .. "/system/searchap/searchap"},
 {"SCRSHOOT", PROGS .. "/media/scrshoot/scrshoot"},
 {"SETUP", PROGS .. "/system/setup/trunk/setup"},
 {"SKINCFG", PROGS .. "/system/skincfg/trunk/skincfg"},
 {"TERMINAL", PROGS .. "/system/terminal/terminal"},
 {"TEST", PROGS .. "/system/test/trunk/test"},
 {"TINYPAD", PROGS .. "/develop/tinypad/trunk/tinypad"},
 {"UNZ", PROGS .. "/fs/unz/unz"},
 {"ZKEY", PROGS .. "/system/zkey/trunk/ZKEY"},
 {"3D/3DWAV", PROGS .. "/demos/3dwav/trunk/3dwav"},
 {"3D/CROWNSCR", PROGS .. "/demos/crownscr/trunk/crownscr"},
 {"3D/3DCUBE2", PROGS .. "/demos/3dcube2/trunk/3DCUBE2"},
 {"3D/FREE3D04", PROGS .. "/demos/free3d04/trunk/free3d04"},
 {"3D/GEARS", PROGS .. "/develop/libraries/TinyGL/asm_fork/examples/gears"},
 {"3D/RAY", PROGS .. "/demos/ray/ray"},
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
 {"DEVELOP/test_gets", PROGS .. "/develop/libraries/console_coff/examples/test_gets"},
 {"DEVELOP/THREAD", PROGS .. "/develop/examples/thread/trunk/thread"},
 {"DEVELOP/INFO/ASM.SYN", PROGS .. "/other/t_edit/info/asm.syn"},
 {"DEVELOP/INFO/CPP_KOL_CLA.SYN", PROGS .. "/other/t_edit/info/cpp_kol_cla.syn"},
 {"DEVELOP/INFO/CPP_KOL_DAR.SYN", PROGS .. "/other/t_edit/info/cpp_kol_dar.syn"},
 {"DEVELOP/INFO/CPP_KOL_DEF.SYN", PROGS .. "/other/t_edit/info/cpp_kol_def.syn"},
 {"DEVELOP/INFO/DEFAULT.SYN", PROGS .. "/other/t_edit/info/default.syn"},
 {"DEVELOP/INFO/HTML.SYN", PROGS .. "/other/t_edit/info/html.syn"},
 {"DEVELOP/INFO/INI_FILES.SYN", PROGS .. "/other/t_edit/info/ini_files.syn"},
 {"File Managers/KFAR", PROGS .. "/fs/kfar/trunk/kfar"},
 {"File Managers/KFM", PROGS .. "/fs/kfm/trunk/kfm"},
 {"File Managers/OPENDIAL", PROGS .. "/fs/opendial/opendial"},
 {"GAMES/15", PROGS .. "/games/15/trunk/15"},
 {"GAMES/FREECELL", PROGS .. "/games/freecell/freecell"},
 {"GAMES/GOMOKU", PROGS .. "/games/gomoku/trunk/gomoku"},
 {"GAMES/KLAVISHA", PROGS .. "/games/klavisha/trunk/klavisha"},
 {"GAMES/LIGHTS", PROGS .. "/games/sq_game/trunk/SQ_GAME"},
 {"GAMES/LINES", PROGS .. "/games/lines/lines"},
 {"GAMES/MSQUARE", PROGS .. "/games/MSquare/trunk/MSquare"},
 {"GAMES/PIPES", PROGS .. "/games/pipes/pipes"},
 {"GAMES/PONG", PROGS .. "/games/pong/trunk/pong"},
 {"GAMES/PONG3", PROGS .. "/games/pong3/trunk/pong3"},
 {"GAMES/RSQUARE", PROGS .. "/games/rsquare/trunk/rsquare"},
 {"GAMES/SNAKE", PROGS .. "/games/snake/trunk/snake"},
 {"GAMES/SUDOKU", PROGS .. "/games/sudoku/trunk/sudoku"},
 {"GAMES/SW", PROGS .. "/games/sw/trunk/sw"},
 {"GAMES/TANKS", PROGS .. "/games/tanks/trunk/tanks"},
 {"GAMES/TETRIS", PROGS .. "/games/tetris/trunk/tetris"},
 {"LIB/ARCHIVER.OBJ", PROGS .. "/fs/kfar/trunk/kfar_arc/kfar_arc.obj"},
 {"LIB/BOX_LIB.OBJ", PROGS .. "/develop/libraries/box_lib/trunk/box_lib.obj"},
 {"LIB/BUF2D.OBJ", PROGS .. "/develop/libraries/buf2d/trunk/buf2d.obj"},
 {"LIB/CONSOLE.OBJ", PROGS .. "/develop/libraries/console_coff/console.obj"},
 {"LIB/CNV_PNG.OBJ", PROGS .. "/media/zsea/plugins/png/cnv_png.obj"},
 {"LIB/HTTP.OBJ", PROGS .. "/develop/libraries/http/http.obj"},
 {"LIB/LIBGFX.OBJ", PROGS .. "/develop/libraries/libs-dev/libgfx/libgfx.obj"},
 {"LIB/LIBIMG.OBJ", PROGS .. "/develop/libraries/libs-dev/libimg/libimg.obj"},
 {"LIB/LIBINI.OBJ", PROGS .. "/develop/libraries/libs-dev/libini/libini.obj"},
 {"LIB/LIBIO.OBJ", PROGS .. "/develop/libraries/libs-dev/libio/libio.obj"},
 {"LIB/MSGBOX.OBJ", PROGS .. "/develop/libraries/msgbox/msgbox.obj"},
 {"LIB/NETWORK.OBJ", PROGS .. "/develop/libraries/network/network.obj"},
 {"LIB/PROC_LIB.OBJ", PROGS .. "/develop/libraries/proc_lib/trunk/proc_lib.obj"},
 {"LIB/RASTERWORKS.OBJ", PROGS .. "/develop/libraries/fontRasterWorks(unicode)/RasterWorks.obj"},
 {"LIB/SORT.OBJ", PROGS .. "/develop/libraries/sorter/sort.obj"},
 {"LIB/TINYGL.OBJ", PROGS .. "/develop/libraries/TinyGL/asm_fork/tinygl.obj"},
 {"MEDIA/ANIMAGE", PROGS .. "/media/animage/trunk/animage"},
 {"MEDIA/KIV", PROGS .. "/media/kiv/trunk/kiv"},
 {"MEDIA/LISTPLAY", PROGS .. "/media/listplay/trunk/listplay"},
 {"MEDIA/MIDAMP", PROGS .. "/media/midamp/trunk/midamp"},
 {"MEDIA/MP3INFO", PROGS .. "/media/mp3info/mp3info"},
 {"MEDIA/PALITRA", PROGS .. "/media/palitra/trunk/palitra"},
 {"MEDIA/STARTMUS", PROGS .. "/media/startmus/trunk/STARTMUS"},
 {"NETWORK/PING", PROGS .. "/network/ping/ping"},
 {"NETWORK/NETCFG", PROGS .. "/network/netcfg/netcfg"},
 {"NETWORK/NETSTAT", PROGS .. "/network/netstat/netstat"},
 {"NETWORK/NSINSTALL", PROGS .. "/network/netsurf/nsinstall"},
 {"NETWORK/NSLOOKUP", PROGS .. "/network/nslookup/nslookup"},
 {"NETWORK/PASTA", PROGS .. "/network/pasta/pasta"},
 {"NETWORK/SYNERGYC", PROGS .. "/network/synergyc/synergyc"},
 {"NETWORK/TELNET", PROGS .. "/network/telnet/telnet"},
 {"NETWORK/@ZEROCONF", PROGS .. "/network/zeroconf/zeroconf"},
 {"NETWORK/FTPC", PROGS .. "/network/ftpc/ftpc"},
 {"NETWORK/FTPD", PROGS .. "/network/ftpd/ftpd"},
 {"NETWORK/TFTPC", PROGS .. "/network/tftpc/tftpc"},
 {"NETWORK/IRCC", PROGS .. "/network/ircc/ircc"},
 {"NETWORK/DOWNLOADER", PROGS .. "/network/downloader/downloader"},
 {"NETWORK/VNCC", PROGS .. "/network/vncc/vncc"},
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
 {"kolibrios/3D/3dsheart", PROGS .. "/demos/3dsheart/trunk/3dsheart"},
 {"kolibrios/3D/info3ds/INFO3DS", PROGS .. "/develop/info3ds/info3ds"},
 {"kolibrios/3D/info3ds/INFO3DS_U", PROGS .. "/develop/info3ds/info3ds_u"},
 {"kolibrios/3D/mos3de", PROGS .. "/demos/mos3de/mos3de"},
 {"kolibrios/3D/voxel_editor/VOXEL_EDITOR", PROGS .. "/media/voxel_editor/trunk/voxel_editor"},
 {"kolibrios/3D/voxel_utilites/VOX_CREATOR" , PROGS .. "/media/voxel_editor/utilites/vox_creator"},
 {"kolibrios/3D/voxel_utilites/VOX_MOVER" , PROGS .. "/media/voxel_editor/utilites/vox_mover"},
 {"kolibrios/3D/voxel_utilites/VOX_TGL" , PROGS .. "/media/voxel_editor/utilites/vox_tgl"},
 {"kolibrios/3D/textures1", PROGS .. "/develop/libraries/TinyGL/asm_fork/examples/textures1"},
 {"kolibrios/demos/buddhabrot", PROGS .. "/demos/buddhabrot/trunk/buddhabrot"},
 {"kolibrios/demos/life3", PROGS .. "/games/life3/trunk/life3"},
 {"kolibrios/demos/qjulia", PROGS .. "/demos/qjulia/trunk/qjulia"},
 {"kolibrios/develop/utils/GenFiles", PROGS .. "/testing/genfiles/GenFiles"},
 {"kolibrios/games/Almaz", PROGS .. "/games/almaz/almaz"},
 {"kolibrios/games/arcanii", PROGS .. "/games/arcanii/trunk/arcanii"},
 {"kolibrios/games/bomber/bomber", PROGS .. "/games/bomber/bomber"},
 {"kolibrios/games/bomber/bomberdata.bin", PROGS .. "/games/bomber/sounds/bomberdata.bin"},
 {"kolibrios/games/codemaster/binary_master", PROGS .. "/games/codemaster/binary_master"},
 {"kolibrios/games/codemaster/hang_programmer", PROGS .. "/games/codemaster/hang_programmer"},
 {"kolibrios/games/codemaster/kolibri_puzzle", PROGS .. "/games/codemaster/kolibri_puzzle"},
 {"kolibrios/games/megamaze", PROGS .. "/games/megamaze/trunk/megamaze"},
 {"kolibrios/games/invaders", PROGS .. "/games/invaders/invaders"},
 {"kolibrios/games/phenix", PROGS .. "/games/phenix/trunk/phenix"},
 {"kolibrios/games/soko/soko", PROGS .. "/games/soko/trunk/SOKO"},
 {"kolibrios/media/zsea/zsea", PROGS .. "/media/zsea/zSea"},
 {"kolibrios/media/zsea/plugins/cnv_bmp.obj", PROGS .. "/media/zsea/plugins/bmp/cnv_bmp.obj"},
 {"kolibrios/media/zsea/plugins/cnv_gif.obj", PROGS .. "/media/zsea/plugins/gif/cnv_gif.obj"},
 {"kolibrios/media/zsea/plugins/cnv_jpeg.obj", PROGS .. "/media/zsea/plugins/jpeg/cnv_jpeg.obj"},
 {"kolibrios/media/zsea/plugins/convert.obj", PROGS .. "/media/zsea/plugins/convert/convert.obj"},
 {"kolibrios/media/zsea/plugins/rotate.obj", PROGS .. "/media/zsea/plugins/rotate/rotate.obj"},
 {"kolibrios/media/zsea/plugins/scaling.obj", PROGS .. "/media/zsea/plugins/scaling/scaling.obj"},
 {"kolibrios/utils/calcplus", PROGS .. "/other/calcplus/calcplus"},
})
-- For russian build, add russian-only programs.
if build_type == "rus" then tup.append_table(img_files, {
 {"PERIOD", PROGS .. "/other/period/trunk/period"},
 {"DEVELOP/TESTCON2", PROGS .. "/develop/libraries/console_coff/examples/testcon2_rus"},
}) else tup.append_table(img_files, {
 {"DEVELOP/TESTCON2", PROGS .. "/develop/libraries/console_coff/examples/testcon2_eng"},
}) end

if build_type == "rus" then tup.append_table(extra_files, {
 {"kolibrios/games/Dungeons/Dungeons", PROGS .. "/games/Dungeons/Dungeons"},
}) end

end -- tup.getconfig('NO_FASM') ~= 'full'

-- Programs that require NASM to compile.
if tup.getconfig('NO_NASM') ~= 'full' then
tup.append_table(img_files, {
 {"LOD", PROGS .. "/fs/lod/lod"},
 {"TINFO", PROGS .. "/system/tinfo/tinfo"},
 {"DEMOS/ACLOCK", PROGS .. "/demos/aclock/trunk/aclock"},
 {"DEMOS/TIMER", PROGS .. "/other/Timer/timer"},
 {"DEVELOP/MSTATE", PROGS .. "/develop/mstate/mstate"},
 {"GAMES/C4", PROGS .. "/games/c4/trunk/c4"},
 {"MEDIA/FILLSCR", PROGS .. "/media/FillScr/fillscr"},
})
end -- tup.getconfig('NO_NASM') ~= 'full'

-- Programs that require JWASM to compile.
if tup.getconfig('NO_JWASM') ~= 'full' then
tup.append_table(img_files, {
 {"RUN", PROGS .. "/system/RunOD/1/RUN"},
 {"LIB/INPUTBOX.OBJ", PROGS .. "/develop/libraries/InputBox/INPUTBOX.OBJ"},
})
end -- tup.getconfig('NO_JWASM') ~= 'full'

-- Programs that require C-- to compile.
if tup.getconfig('NO_CMM') ~= 'full' then
tup.append_table(img_files, {
 {"APP_PLUS", PROGS .. "/cmm/app_plus/app_plus.com"},
 {"EASYSHOT", PROGS .. "/cmm/easyshot/easyshot.com"},
 {"MOUSECFG", PROGS .. "/cmm/mousecfg/mousecfg.com"},
 {"NOTES", PROGS .. "/cmm/notes/notes.com"},
 {"PANELS_CFG", PROGS .. "/cmm/panels_cfg/panels_cfg.com"},
 {"SYSPANEL", PROGS .. "/cmm/software_widget/software_widget.com"},
 {"SYSMON", PROGS .. "/cmm/sysmon/sysmon.com"},
 {"TXTREAD", PROGS .. "/cmm/txtread/txtread.com"},
 {"TMPDISK", PROGS .. "/cmm/tmpdisk/tmpdisk.com"},
 {"DEVELOP/CLIPVIEW", PROGS .. "/cmm/clipview/clipview.com"},
 {"File Managers/EOLITE", PROGS .. "/cmm/eolite/Eolite.com"},
 {"KF_VIEW", PROGS .. "/cmm/kf_font_viewer/font_viewer.com"},
 {"GAMES/CLICKS", PROGS .. "/games/clicks/trunk/clicks.com"},
 {"GAMES/MBLOCKS", PROGS .. "/cmm/mblocks/mblocks.com"},
 {"DEVELOP/DIFF", PROGS .. "/cmm/diff/diff.com"},
 {"GAMES/FindNumbers", PROGS .. "/games/FindNumbers/trunk/FindNumbers"},
 {"GAMES/FLOOD-IT", PROGS .. "/games/flood-it/trunk/flood-it.com"},
 {"GAMES/MINE", PROGS .. "/games/mine/trunk/mine"},
 {"MEDIA/PIXIE/PIXIE", PROGS .. "/cmm/pixie2/pixie.com"},
 {"MEDIA/ICONEDIT", PROGS .. "/cmm/iconedit/iconedit.com"},
 {"NETWORK/WEBVIEW", PROGS .. "/cmm/browser/WebView.com"},
})
tup.append_table(extra_files, {
 {"kolibrios/drivers/drvinst.kex", PROGS .. "/cmm/drvinst/drvinst.com"},
 {"kolibrios/games/pig/pigex", PROGS .. "/cmm/examples/pigex.com"},
 {"kolibrios/games/the_bus/the_bus", PROGS .. "/cmm/the_bus/the_bus.com"},
 {"kolibrios/utils/dicty.kex", PROGS .. "/cmm/dicty/dicty.com"},
 {"kolibrios/utils/appearance", PROGS .. "/cmm/appearance/appearance.com"},
 {"kolibrios/utils/netcheck", PROGS .. "/cmm/examples/netcheck.com"},
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
 {"GAMES/CHECKERS", PROGS .. "/games/checkers/trunk/checkers"},
 {"GAMES/REVERSI", PROGS .. "/games/reversi/trunk/reversi"},
 {"SHELL", PROGS .. "/system/shell/shell"},
})
tup.append_table(extra_files, {
 {"kolibrios/3D/cubeline", PROGS .. "/demos/cubeline/trunk/cubeline"},
 {"kolibrios/emul/e80/e80", PROGS .. "/emulator/e80/trunk/e80"},
 {"kolibrios/games/2048", PROGS .. "/games/2048/2048"},
 {"kolibrios/games/donkey", PROGS .. "/games/donkey/donkey"},
 {"kolibrios/games/heliothryx", PROGS .. "/games/heliothryx/heliothryx"},
 {"kolibrios/games/marblematch3", PROGS .. "/games/marblematch3/marblematch3"},
 {"kolibrios/games/nsider", PROGS .. "/games/nsider/nsider"},
 {"kolibrios/games/quake/", "common/games/quake/*"}, -- not really gcc, but no sense without sdlquake
 {"kolibrios/games/quake/", "../contrib/other/sdlquake-1.0.9/sdlquake"},
})
-- For russian build, add russian-only programs.
if build_type == "rus" then tup.append_table(extra_files, {
 {"kolibrios/games/21days", PROGS .. "/games/21days/21days"},
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
  table.insert(extra_files, {"kolibrios/res/skins/", "../skins/" .. v})
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
tup.definerule{inputs = {"../kernel/trunk/boot/uefi4kos.asm", "kolibri.img", "../kernel/trunk/kernel.bin"}, command = "fasm ../kernel/trunk/boot/uefi4kos.asm %o", outputs = {"kolibri.efi"}}

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

tup.append_table(tmp_img_files, {
  {"ALLGAMES", tup.getcwd() .. "/allgames"},
  {"ICONS32.PNG", tup.getcwd() .. "/icons32.png"},
  {"ICONS18.PNG", tup.getcwd() .. "/icons18.png"},
  {"INDEX.HTM", tup.getcwd() .. "/index_htm"},
  {"KUZKINA.MID", tup.getcwd() .. "/kuzkina.mid"},
  {"SINE.MP3", tup.getcwd() .. "/sine.mp3"},
  {"NOTIFY3.PNG", tup.getcwd() .. "/notify3.png"},
})

tup.append_table(tmp_extra_files, {
  {"kolibrios/res/system/", tup.getcwd() .. "/signs.png"},
  {"kolibrios/utils/vmode", tup.getcwd() .. "/vmode"},
})

tup.include("./3d/package.lua")
tup.include("./demos/package.lua")
tup.include("./develop/package.lua")
tup.include("./distr_data/package.lua")
tup.include("./drivers/package.lua")
tup.include("./emul/package.lua")
tup.include("./File Managers/package.lua")
tup.include("./fonts/package.lua")
tup.include("./games/package.lua")
tup.include("./HD_load/package.lua")
tup.include("./KolibriNext/package.lua")
tup.include("./lib/package.lua")
tup.include("./media/package.lua")
tup.include("./network/package.lua")
tup.include("./settings/package.lua")
tup.include("./templates/package.lua")
tup.include("./utils/package.lua")
tup.include("./wallpapers/package.lua")

if not lang("ru_RU") then return end

tup.include("./settings/package.lua")
tup.include("./docs/package.lua")
tup.include("./games/package.lua")

tup.append_table(tmp_img_files, {
  {"File Managers/KFAR.INI", "ru_RU/File Managers/kfar.ini"},
})

tup.include("./common/package.lua")
tup.include("./ru_RU/package.lua")

tup.append_table(tmp_img_files, {
  {"LANG.INC",          build_type .. "/lang.inc"},
  {"SETTINGS/ICON.INI", build_type .. "/settings/icon.ini"},
  {"SETTINGS/LANG.INI", build_type .. "/settings/lang.ini"},
  {"SETTINGS/MENU.DAT", build_type .. "/settings/menu.dat"},
})

if nlang("ru_RU") then
  tup.append_table(tmp_extra_files, {
    {"Docs/Config.txt",   build_type .. "/docs/Config.txt"},
    {"Docs/Copying.txt",  build_type .. "/docs/Copying.txt"},
    {"Docs/Hot_Keys.txt", build_type .. "/docs/Hot_Keys.txt"},
    {"Docs/Install.txt",  build_type .. "/docs/Install.txt"},
    {"Docs/Credits.txt",  build_type .. "/docs/Credits.txt"},
  })
end

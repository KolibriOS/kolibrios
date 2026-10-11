tup.include("./common/package.lua")
tup.include("./ru_RU/package.lua")

local var = tup.getvariantdir() .. "/"

tup.append_table(tmp_img_files, {
  {"LANG.INC",          build_type .. "/lang.inc"},
  {"SETTINGS/ICON.INI", build_type .. "/settings/icon.ini"},
  {"SETTINGS/LANG.INI", build_type .. "/settings/lang.ini"},
  {"SETTINGS/MENU.DAT", build_type .. "/settings/menu.dat"},
})

tup.append_table(tmp_extra_files, {
  {"kolibrios/res/system/", build_type .. "/settings/kolibri.lbl"},
})

tup.append_table(tmp_iso_extra_files, {
  {"kolibri.img", var .. "kolibri.img"},
})

tup.append_table(tmp_distr_extra_files, {
  {"distribution_kit/kolibri.img", var .. "kolibri.img"},
})

if not lang("ru_RU") then
  tup.append_table(tmp_extra_files, {
    {"Docs/Config.txt",   build_type .. "/docs/Config.txt"},
    {"Docs/Copying.txt",  build_type .. "/docs/Copying.txt"},
    {"Docs/Hot_Keys.txt", build_type .. "/docs/Hot_Keys.txt"},
    {"Docs/Install.txt",  build_type .. "/docs/Install.txt"},
    {"Docs/Credits.txt",  build_type .. "/docs/Credits.txt"},
  })

  tup.append_table(tmp_iso_extra_files, {
    {"/readme.txt", build_type .. "/distr_data/readme.txt"},
  })

  tup.append_table(tmp_distr_extra_files, {
    {"/readme.txt", build_type .. "/distr_data/readme_distr.txt"},
  })
end

if lang({"ru_RU", "en_US", "es_ES"}) then
  local var = var .. build_type

  tup.append_table(tmp_img_files, {
    {"WELCOME.HTM", var .. "/welcome.htm.kpack"},
  })
end

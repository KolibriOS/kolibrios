tup.append_table(tmp_img_files, {
  {"SETTINGS/APP.INI",      tup.getcwd() .. "/app.ini"},
  {"SETTINGS/APP_PLUS.INI", tup.getcwd() .. "/app_plus.ini"},
  {"SETTINGS/ASSOC.INI",    tup.getcwd() .. "/assoc.ini"},
  {"SETTINGS/AUTORUN.DAT",  tup.getcwd() .. "/AUTORUN.DAT"},
  {"SETTINGS/DOCKY.INI",    tup.getcwd() .. "/docky.ini"},
  {"SETTINGS/FB2READ.INI",  tup.getcwd() .. "/fb2read.ini"},
  {"SETTINGS/NETWORK.INI",  tup.getcwd() .. "/network.ini"},
  {"SETTINGS/SYSTEM.INI",   tup.getcwd() .. "/system.ini"},
  {"SETTINGS/TASKBAR.INI",  tup.getcwd() .. "/taskbar.ini"},
  {"SETTINGS/SYSTEM.ENV",   tup.getcwd() .. "/system.env"},
})

if build_type ~= "ru_RU" then
  tup.append_table(tmp_img_files, {
    {"SETTINGS/GAMES.INI", tup.getcwd() .. "/games.ini"},
  })
end

if build_type ~= "ru_RU" and
   build_type ~= "es_ES" then
  tup.append_table(tmp_img_files, {
    {"SETTINGS/SYSPANEL.INI", tup.getcwd() .. "/syspanel.ini"}
  })
end

tup.append_table(tmp_img_files, {
  {"File Managers/ICON2EXT.INI", tup.getcwd() .. "/icon2ext.ini"},
})

if nlang("ru_RU") then
  tup.append_table(tmp_img_files, {
    {"File Managers/KFAR.INI", tup.getcwd() .. "/kfar.ini"},
  })
end

tup.append_table(tmp_extra_files, {
  {"kolibrios/utils/kfm/kfm.ini", tup.getcwd() .. "/kfm.ini"},
  {"kolibrios/utils/fNav/", tup.getcwd() .. "/fNav/*"},
  {"kolibrios/utils/NDN/", tup.getcwd() .. "/ndn/*"},
  {"kolibrios/utils/NDN/COLORS/", tup.getcwd() .. "/ndn/COLORS/*"},
  {"kolibrios/utils/NDN/PLUGINS/", tup.getcwd() .. "/ndn/PLUGINS/*"},
  {"kolibrios/utils/NDN/SSAVERS/", tup.getcwd() .. "/ndn/SSAVERS/*"},
  {"kolibrios/utils/NDN/XLT/", tup.getcwd() .. "/ndn/XLT/*"},
})

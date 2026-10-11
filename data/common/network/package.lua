tup.append_table(tmp_img_files, {
  {"NETWORK/FTPD.INI", tup.getcwd() .. "/ftpd.ini"},
  {"NETWORK/KNMAP", tup.getcwd() .. "/knmap"},
  {"NETWORK/USERS.INI", tup.getcwd() .. "/users.ini"},
})

tup.append_table(tmp_extra_files, {
  {"kolibrios/netsurf/netsurf", tup.getcwd() .. "/netsurf/netsurf"},
  {"kolibrios/netsurf/res/", tup.getcwd() .. "/netsurf/res/*"},
})

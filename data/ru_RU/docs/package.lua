local src = tup.getcwd()
local var = tup.getvariantdir()

tup.append_table(tmp_extra_files, {
  {"kolibrios/res/guide/",    src .. "/guide/*"},

  {"Docs/cp866/Config.txt",   src .. "/Config.txt"},
  {"Docs/cp866/Copying.txt",  src .. "/Copying.txt"},
  {"Docs/cp866/History.txt",  src .. "/History.txt"},
  {"Docs/cp866/Hot_Keys.txt", src .. "/Hot_Keys.txt"},
  {"Docs/cp866/Install.txt",  src .. "/Install.txt"},
  {"Docs/cp866/Credits.txt",  src .. "/Credits.txt"},

  {"Docs/cp1251/Config.txt",   var .. "/CONFIG.WIN.TXT",   cp1251_from = src .. "/Config.txt"},
  {"Docs/cp1251/Copying.txt",  var .. "/COPYING.WIN.TXT",  cp1251_from = src .. "/Copying.txt"},
  {"Docs/cp1251/History.txt",  var .. "/HISTORY.WIN.TXT",  cp1251_from = src .. "/History.txt"},
  {"Docs/cp1251/Hot_Keys.txt", var .. "/HOT_KEYS.WIN.TXT", cp1251_from = src .. "/Hot_Keys.txt"},
  {"Docs/cp1251/Install.txt",  var .. "/INSTALL.WIN.TXT",  cp1251_from = src .. "/Install.txt"},
  {"Docs/cp1251/credits.txt",  var .. "/CREDITS.WIN.TXT",  cp1251_from = src .. "/Credits.txt"},
})

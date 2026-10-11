tup.append_table(tmp_extra_files, {
  {"kolibrios/res/guide/",    tup.getcwd() .. "/guide/*"},

  {"Docs/cp866/Config.txt",   tup.getcwd() .. "/Config.txt"},
  {"Docs/cp866/Copying.txt",  tup.getcwd() .. "/Copying.txt"},
  {"Docs/cp866/History.txt",  tup.getcwd() .. "/History.txt"},
  {"Docs/cp866/Hot_Keys.txt", tup.getcwd() .. "/Hot_Keys.txt"},
  {"Docs/cp866/Install.txt",  tup.getcwd() .. "/Install.txt"},
  {"Docs/cp866/Credits.txt",  tup.getcwd() .. "/Credits.txt"},
})

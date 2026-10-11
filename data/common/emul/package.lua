tup.append_table(tmp_extra_files, {
  {"kolibrios/emul/", tup.getcwd() .. "/*"},
  {"kolibrios/emul/dosbox/", tup.getcwd() .. "/DosBox/*"},
  {"kolibrios/emul/uarm/", tup.getcwd() .. "/uarm/*"},
  {"kolibrios/emul/zsnes/", tup.getcwd() .. "/zsnes/*"},
})

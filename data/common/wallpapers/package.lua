tup.append_table(tmp_img_files, {
  {"HOME.PNG", tup.getcwd() .. "/T_Home.png"},
})

tup.append_table(tmp_extra_files, {
  {"kolibrios/res/wallpapers/", tup.getcwd() .. "/*"},
})

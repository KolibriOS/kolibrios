tup.append_table(tmp_img_files, {
  {"3D/HOUSE.3DS", tup.getcwd() .. "/house.3ds"},
})

tup.append_table(tmp_extra_files, {
  {"kolibrios/3D/md2view/", tup.getcwd() .. "/md2view/*"},
  {"kolibrios/3D/md2view/md2_model/", tup.getcwd() .. "/md2view/md2_model/*"},
})

local src = tup.getcwd()
local dst = tup.getvariantdir()

tup.append_table(tmp_iso_extra_files, {
  {"/readme_dos.txt", src .. "/readme_dos.txt"},
  {"/readme.txt",     dst .. "/readme.txt",
    cp1251_from =     src .. "/readme_dos.txt"},
})

tup.append_table(tmp_distr_extra_files, {
  {"/readme_dos.txt", src .. "/readme_dos_distr.txt"},
  {"/readme.txt",     src .. "/readme_distr.txt",
    cp1251_from =     src .. "/readme_dos_distr.txt"},
})

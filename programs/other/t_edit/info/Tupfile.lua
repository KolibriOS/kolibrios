if tup.getconfig("NO_FASM") ~= "" then return end
for i,v in ipairs{"asm", "cpp_kol_cla", "cpp_kol_dar", "cpp_kol_def", "default", "html", "ini_files", "voc_eng_rus", "win_const"} do
  tup.rule(v .. "_syn.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), v .. ".syn")
end

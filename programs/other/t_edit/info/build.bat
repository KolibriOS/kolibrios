@fasm.exe -m 16384 %1asm_syn.asm %2asm.syn
@kpack %2asm.syn
@fasm.exe -m 16384 %1cpp_kol_cla_syn.asm %2cpp_cla.syn
@kpack %2cpp_kol_cla.syn
@fasm.exe -m 16384 %1cpp_kol_dar_syn.asm %2cpp_dar.syn
@kpack %2cpp_kol_dar.syn
@fasm.exe -m 16384 %1cpp_kol_def_syn.asm %2cpp_def.syn
@kpack %2cpp_kol_def.syn
@fasm.exe -m 16384 %1default_syn.asm %2default.syn
@kpack %2default.syn
@fasm.exe -m 16384 %1html_syn.asm %2html.syn
@kpack %2html.syn
@fasm.exe -m 16384 %1ini_files_syn.asm %2ini.syn
@fasm.exe -m 16384 %1voc_eng_rus_syn.asm %2voc_eng_rus.syn
@kpack %2voc_eng_rus.syn
@fasm.exe -m 16384 %1win_const_syn.asm %2win_const.syn
@kpack %2win_const.syn
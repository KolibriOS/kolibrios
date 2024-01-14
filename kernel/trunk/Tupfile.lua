if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("echo lang fix " .. ((tup.getconfig("LANG") == "") and "en" or tup.getconfig("LANG")) .. " > %o", {"lang.inc"})
tup.rule({"kernel.asm", extra_inputs = {"lang.inc"}}, "fasm -m 131072 %f %o " .. tup.getconfig("KERPACK_CMD"), "kernel.mnt")
tup.rule({"kernel.asm", extra_inputs = {"lang.inc"}}, "fasm -m 131072 %f %o -dextended_primary_loader=1" .. tup.getconfig("KERPACK_CMD"), "kernel.mnt.ext_loader")
tup.rule({"kernel.asm", extra_inputs = {"lang.inc"}}, "fasm -m 131072 %f %o -dpretest_build=1 -ddebug_com_base=0xe9", "kernel.mnt.pretest")

if tup.getconfig("NO_FASM") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_fasm.lua")
add_include(tup.getvariantdir())

tup.rule("echo lang fix " .. ((tup.getconfig("LANG") == "") and "en" or tup.getconfig("LANG")) .. " > %o", {"lang.inc"})
tup.rule({"kol_f_edit.asm", extra_inputs = {"lang.inc"}}, FASM .. " %f %o " .. tup.getconfig("KPACK_CMD"), "bin/kol_f_edit")
tup.rule({"ob_o.asm", extra_inputs = {"lang.inc"}}, FASM .. " %f %o", "bin/ob_o.opt")

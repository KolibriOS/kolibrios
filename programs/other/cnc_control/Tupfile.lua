if tup.getconfig("NO_FASM") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_fasm.lua")
add_include(tup.getvariantdir())

tup.rule("echo lang fix " .. ((tup.getconfig("LANG") == "") and "ru" or tup.getconfig("LANG")) .. " > %o", {"lang.inc"})
tup.rule({"cnc_control.asm", extra_inputs = {"lang.inc"}}, FASM .. " %f %o " .. tup.getconfig("KPACK_CMD"), "cnc_control")


if tup.getconfig("NO_FASM") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_fasm.lua")
add_include(tup.getvariantdir())

tup.rule("mtldr_code/mtldr.asm", "fasm %f %o", "mtldr_for_installer")
tup.rule({"mtldr_install.asm", extra_inputs = {"mtldr_for_installer"}}, FASM .. " %f %o", "mtldr_install.exe")

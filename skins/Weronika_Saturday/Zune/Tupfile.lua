if tup.getconfig("NO_FASM") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../../programs" or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_fasm.lua")
add_include(tup.getvariantdir())

tup.rule("Zune.dtp.asm", 'fasm "%f" "%o"', "Zune.dtp")
tup.rule({"Zune.asm", extra_inputs = {"Zune.dtp"}}, FASM .. ' "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "Zune.skn")

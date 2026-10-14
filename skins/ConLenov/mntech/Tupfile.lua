if tup.getconfig("NO_FASM") ~= "" then return end

tup.include(HELPERDIR .. "/use_fasm.lua")
add_include(tup.getvariantdir())

tup.rule("default.dtp.asm", 'fasm "%f" "%o"', "default.dtp")
tup.rule({"mntech.asm", extra_inputs = {"default.dtp"}}, FASM .. ' "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "mntech.skn")

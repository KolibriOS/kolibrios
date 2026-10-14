if tup.getconfig("NO_FASM") ~= "" then return end

tup.include(HELPERDIR .. "/use_fasm.lua")
add_include(tup.getvariantdir())

tup.rule("col.dtp.asm", 'fasm "%f" "%o"', "col.dtp")
tup.rule({"florence.asm", extra_inputs = {"col.dtp"}}, FASM .. ' "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "Florence.skn")

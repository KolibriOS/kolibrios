if tup.getconfig("NO_FASM") ~= "" then return end

tup.include(HELPERDIR .. "/use_fasm.lua")
add_include(tup.getvariantdir())

tup.rule("colour.asm", 'fasm "%f" "%o"', "colour.dtp")
tup.rule({"default.asm", extra_inputs = {"colour.dtp"}}, FASM .. ' "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "mage-scroll.skn")

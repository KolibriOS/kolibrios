if tup.getconfig("NO_FASM") ~= "" then return end
tup.include(HELPERDIR .. "/use_fasm.lua")
add_include(tup.getvariantdir())

tup.rule("solitare.asm", FASM .. " %f %o " .. tup.getconfig("KPACK_CMD"), "solitare")

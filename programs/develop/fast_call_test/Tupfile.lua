if tup.getconfig("NO_FASM") ~= "" then return end
tup.include(HELPERDIR .. "/use_fasm.lua")
tup.rule("test.ASM", FASM .. " %f %o " .. tup.getconfig("KPACK_CMD"), "test")

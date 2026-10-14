if tup.getconfig("NO_FASM") ~= "" then return end
tup.include(HELPERDIR .. "/use_fasm.lua")

tup.rule("libcrash.asm", FASM .. " %f %o " .. tup.getconfig("KPACK_CMD"), "libcrash.obj")
tup.rule("crashtest.asm", FASM .. " %f %o " .. tup.getconfig("KPACK_CMD"), "crashtest")

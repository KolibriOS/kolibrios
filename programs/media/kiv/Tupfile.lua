if tup.getconfig("NO_FASM") ~= "" then return end
tup.include(HELPERDIR .. "/use_fasm.lua")

add_include(HELPERDIR .. "/develop/libraries/libs-dev/libimg")
tup.rule("kiv.asm", FASM .. " %f %o " .. tup.getconfig("KPACK_CMD"), "kiv")

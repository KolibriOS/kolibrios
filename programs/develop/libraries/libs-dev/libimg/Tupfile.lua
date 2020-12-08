if tup.getconfig("NO_FASM") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_fasm.lua")

add_include(HELPERDIR .. "/develop/libraries/libs-dev/libio")
tup.rule("libimg.asm", FASM .. " -m 32768 %f %o " .. tup.getconfig("KPACK_CMD"), "%B.obj")

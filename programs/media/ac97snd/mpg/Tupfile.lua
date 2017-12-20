if tup.getconfig("NO_FASM") ~= "" or tup.getconfig("NO_MSVC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_fasm.lua")
tup.include(HELPERDIR .. "/use_msvc.lua")
compile_msvc{"*.c"}
tup.append_table(OBJS, tup.rule("pow.asm", FASM .. " %f %o", "pow.obj"))
tup.rule(OBJS, "link.exe /lib /out:%o %f", "mpg.lib")

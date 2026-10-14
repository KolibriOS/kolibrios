if tup.getconfig("NO_GCC") ~= "" then return end
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")

CFLAGS = CFLAGS .. " -Dasm=__asm__ -std=c99 -DGCC_BUILD"
LDFLAGS = LDFLAGS .. " --subsystem native"

compile_gcc{"tte.c", "notify.c", "getline.c"}
link_gcc("tte")

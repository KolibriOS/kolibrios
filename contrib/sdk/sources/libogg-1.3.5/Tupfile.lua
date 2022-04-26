if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../../../programs" or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")

CFLAGS = CFLAGS_OPTIMIZE_SPEED
INCLUDES = INCLUDES .. " -Iinclude "

compile_gcc{"src/bitwise.c", "src/framing.c"}
tup.rule(OBJS, "kos32-ar rcs %o %f", {"../../lib/libogg.a", "../../lib/<libogg>"})

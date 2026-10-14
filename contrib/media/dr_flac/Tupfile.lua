if tup.getconfig("NO_GCC") ~= "" then return end

tup.include(HELPERDIR .. "/use_gcc.lua")
NEWLIB_INCLUDE = "../../sdk/sources/newlib/libc/include"
INCLUDES = INCLUDES .. " -I" .. NEWLIB_INCLUDE
CFLAGS = CFLAGS_OPTIMIZE_SPEED .. " -nostdlib -fwhole-program"
compile_gcc("dr_flac.c", "dr_flac.obj")

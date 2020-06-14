if tup.getconfig("NO_GCC") ~= "" then return end
if tup.getconfig("HELPERDIR") == ""
then
  HELPERDIR = "../../../programs"
end
tup.include(HELPERDIR .. "/use_gcc.lua")
NEWLIB_INCLUDE = "../../sdk/sources/newlib/libc/include"
INCLUDES = INCLUDES .. " -I" .. NEWLIB_INCLUDE
CFLAGS = CFLAGS_OPTIMIZE_SPEED .. " -nostdlib -fwhole-program"
compile_gcc("libvorbis.c", "stb_vorbis.obj")

if tup.getconfig("NO_GCC") ~= "" then return end
if tup.getconfig("HELPERDIR") == ""
then
  if tup.getconfig("NO_NASM") ~= "" then return end -- required for SDL compilation
  HELPERDIR = "../../../programs"
end
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_menuetlibc.lua")
tup.include(HELPERDIR .. "/use_sdl.lua")
CFLAGS = CFLAGS_OPTIMIZE_SPEED .. " -std=c99 -U_WIN32 -fwhole-program"
compile_gcc{"8086tiny.c"}
link_gcc("8086tiny")

if tup.getconfig("NO_GCC") ~= "" then return end
if tup.getconfig("HELPERDIR") == ""
then
  if tup.getconfig("NO_NASM") ~= "" then return end -- required for SDL compilation
  HELPERDIR = "../../../../../programs"
end
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_menuetlibc.lua")
tup.include(HELPERDIR .. "/use_sdl.lua")
compile_gcc{"fire.c"}
link_gcc("fire")

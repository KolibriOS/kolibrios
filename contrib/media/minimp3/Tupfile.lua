if tup.getconfig("NO_GCC") ~= "" then return end
if tup.getconfig("HELPERDIR") == ""
then
  HELPERDIR = "../../../programs"
end
tup.include(HELPERDIR .. "/use_gcc.lua")
CFLAGS = CFLAGS_OPTIMIZE_SPEED .. " -o minimp3.obj  -U_WIN32 -nostdlib -fwhole-program"
compile_gcc{"minimp3.c"}

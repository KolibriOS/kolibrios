if tup.getconfig("NO_GCC") ~= "" or tup.getconfig("NO_FASM") ~= "" then return end
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")
compile_gcc{"main.c"}
link_gcc("binclock")

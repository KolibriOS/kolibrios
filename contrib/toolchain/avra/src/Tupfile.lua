if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../../../programs" or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")

compile_gcc{"args.c", "avra.c", "coff.c", "device.c", "directiv.c", "expr.c", "file.c", "macro.c", "map.c", "mnemonic.c", "parser.c", "stdextra.c"}
link_gcc("avra")

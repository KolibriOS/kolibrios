if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../.." or tup.getconfig("HELPERDIR")

tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")

LIBS = " -lstdc++ -lsupc++ -lgcc -lc.dll "

compile_gcc{
    "elf2cof.cpp",
    "macho.cpp",
    "cmdline.cpp",
    "elf.cpp",
    "stdafx.cpp",
    "error.cpp",
    "omfhash.cpp",
    "elf2asm.cpp",
    "main.cpp",
    "cof2omf.cpp",
    "omf2asm.cpp",
    "cof2asm.cpp",
    "elf2elf.cpp",
    "containers.cpp",
    "mac2asm.cpp",
    "mac2elf.cpp",
    "opcodes.cpp",
    "cof2elf.cpp",
    "omf2cof.cpp",
    "library.cpp",
    "elf2mac.cpp",
    "mac2mac.cpp",
    "coff.cpp",
    "cof2cof.cpp",
    "omf.cpp",
    "disasm2.cpp",
    "disasm1.cpp",
}

link_gcc("objconv")
 

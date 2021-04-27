if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../../../" or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")

CFLAGS = " -c -w -nostdinc -DGNUC -D_KOLIBRI_LIBC_OBJ -Os -fno-common -fno-builtin -fno-leading-underscore -fno-pie" 
INCLUDES = " -I../include"

tup.rule("../linuxtools/src/ExportGen.c", "gcc %f -o %o" , "../linuxtools/ExportGen")
tup.rule("../linuxtools/src/LoaderGen.c", "gcc %f -o %o" , "../linuxtools/LoaderGen")

tup.rule({"symbols.txt",extra_inputs = {"../linuxtools/ExportGen"}}, "../linuxtools/ExportGen %f %o" , "exports/exports.c")

tup.rule({"libc.c",extra_inputs = {"exports/exports.c"}} , "kos32-gcc" .. CFLAGS .. INCLUDES .. " -o %o %f " .. tup.getconfig("KPACK_CMD"), "../lib/libc.obj")


 

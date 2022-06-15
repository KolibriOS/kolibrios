if tup.getconfig("NO_GCC") ~= "" then return end
if tup.getconfig("HELPERDIR") == ""
then
  if tup.getconfig("NO_NASM") ~= "" then return end -- required for SDL compilation
  HELPERDIR = "../../"
end
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")
tup.include(HELPERDIR .. "/use_sdl_newlib.lua")

CFLAGS = CFLAGS_OPTIMIZE_SPEED ..[[ -DPACKAGE_NAME=\"DGen/SDL\" -DPACKAGE_TARNAME=\"dgen-sdl\" -DPACKAGE_VERSION=\"1.33\" -DPACKAGE_STRING=\"DGen/SDL\ 1.33\" -DPACKAGE_BUGREPORT=\"zamaz@users.sourceforge.net\" -DPACKAGE_URL=\"http://sourceforge.net/projects/dgen\" -DPACKAGE=\"dgen-sdl\" -DVERSION=\"1.33\" -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_FTELLO=1 -DHAVE_GLOB_H=1 -DWITH_JOYSTICK=1 -DWITH_MUSA=1 -DWITH_STAR=1 -DWITH_MZ80=1 -DWITH_CZ80=1 -DWITH_X86_ASM=1 -DHAVE_MEMCPY_H=1 -DWITH_CTV=1 -DWITH_SCALE2X=1 -DWITH_X86_MZ80=1 -DWITH_X86_MMX=1 -DWITH_X86_CTV=1 -DWITH_X86_TILES=1 ]]

LDFLAGS = LDFLAGS .. " --subsystem native"
INCLUDES = INCLUDES .. " -Isdl -I. -Iscale2x"

compile_gcc {
  "romload.c",
  "ckvp.c",
  "system.c",
  "fm.c",
  "decode.c",
  "main.cpp",
  "joystick.cpp",
  "mdfr.cpp",
  "rc.cpp",
  "myfm.cpp",
  "graph.cpp",
  "md.cpp",
  "mem.cpp",
  "ras.cpp",
  "vdp.cpp",
  "save.cpp",
  "getopt.c",
  "sn76496.c",
  "cpp_dep.cpp"
}

compile_gcc {
  "sdl/font.cpp",
  "sdl/dgenfont_8x13.cpp",
  "sdl/sdl.cpp",
  "sdl/dgenfont_16x26.cpp",
  "sdl/dgenfont_7x5.cpp",
  "sdl/prompt.c"
}

compile_gcc {
  "scale2x/scalebit.c",
  "scale2x/scale2x.c",
  "scale2x/scale3x.c"
}

compile_gcc {
  "cz80/cz80.c"
}

compile_gcc {
  "musa/m68kcpu.c",
  "musa/m68kops.c"
}

ASM_SRC = {"x86_memcpy.asm", "x86_ctv.asm", "x86_mmx_memcpy.asm", "x86_tiles.asm", "mz80/x86-mz80.asm", "star/starcpu.asm"}

tup.append_table(OBJS,
  tup.foreach_rule(ASM_SRC, "nasm -f coff --prefix _ %f -o %o", "%B.o")
)

link_gcc("dgen")

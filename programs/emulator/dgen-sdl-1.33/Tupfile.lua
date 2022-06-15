if tup.getconfig("NO_GCC") ~= "" then return end
if tup.getconfig("HELPERDIR") == ""
then
  if tup.getconfig("NO_NASM") ~= "" then return end
  HELPERDIR = "../../"
end
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")
tup.include(HELPERDIR .. "/use_sdl_newlib.lua")

CFLAGS = CFLAGS_OPTIMIZE_SPEED
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

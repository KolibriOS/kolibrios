if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../../programs" or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")
CFLAGS = "-fno-ident -O2 -fomit-frame-pointer -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32  -DLUA_COMPAT_ALL -DLUA_ANSI"
LDFLAGS = LDFLAGS .. " --disable-runtime-pseudo-reloc --subsystem native"
compile_gcc{
  "lapi.c",
  "lauxlib.c",
  "lbaselib.c",
  "lbitlib.c",
  "lcode.c",
  "lcorolib.c",
  "lctype.c",
  "ldblib.c",
  "ldebug.c",
  "ldo.c",
  "ldump.c",
  "lfunc.c",
  "lgc.c",
  "linit.c",
  "liolib.c",
  "llex.c",
  "lmathlib.c",
  "lmem.c",
  "loadlib.c",
  "lobject.c",
  "lopcodes.c",
  "loslib.c",
  "lparser.c",
  "lstate.c",
  "lstring.c",
  "lstrlib.c",
  "ltable.c",
  "ltablib.c",
  "ltm.c",
  "lua.c",
  "lundump.c",
  "lvm.c",
  "lzio.c",
  "kolibri.c"
}
link_gcc("lua")

NEWLIB_BASE = tup.getcwd() .. "/../contrib/sdk/sources/newlib"
NEWLIB_LIB = tup.getcwd() .. "/../contrib/sdk/lib"

TOOLCHAIN_LIBPATH = tup.getconfig("TOOLCHAIN_LIBPATH")
-- if not given explicitly in config, try to guess
if TOOLCHAIN_LIBPATH == "" then
  if tup.getconfig("TUP_PLATFORM") == "win32"
  then TOOLCHAIN_LIBPATH="C:\\MinGW\\msys\\1.0\\home\\autobuild\\tools\\win32\\mingw32\\lib"
  else TOOLCHAIN_LIBPATH="/home/autobuild/tools/win32/mingw32/lib"
  end
end

INCLUDES = INCLUDES .. " -I" .. NEWLIB_BASE .. "/libc/include"
LDFLAGS = LDFLAGS .. " -T$(NEWLIB_BASE)/app-dynamic.lds -L$(NEWLIB_LIB) --image-base 0"
tup.append_table(LIBDEPS, {NEWLIB_LIB .. "/<libc.dll.a>"})
LIBS = LIBS .. "-lgcc -lc.dll"

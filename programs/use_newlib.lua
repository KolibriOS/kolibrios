NEWLIB_BASE = tup.getcwd() .. "/../contrib/sdk/sources/newlib"
SDK_CWD = tup.getcwd() .. "/../contrib/sdk/lib"
SDK_VAR = tup.getvariantdir() .. "/../contrib/sdk/lib"

TOOLCHAIN_LIBPATH = tup.getconfig("TOOLCHAIN_LIBPATH")
-- if not given explicitly in config, try to guess
if TOOLCHAIN_LIBPATH == "" then
  if tup.getconfig("TUP_PLATFORM") == "win32"
  then TOOLCHAIN_LIBPATH="C:\\MinGW\\msys\\1.0\\home\\autobuild\\tools\\win32\\mingw32\\lib"
  else TOOLCHAIN_LIBPATH="/home/autobuild/tools/win32/mingw32/lib"
  end
end

INCLUDES += " -I$(NEWLIB_BASE)/libc/include"
LDFLAGS += " -T$(NEWLIB_BASE)/app-dynamic.lds -L$(SDK_CWD) -L$(SDK_VAR) --image-base 0"
tup.append_table(LIBDEPS, {SDK_CWD .. "/<libc.dll.a>"})
LIBS += "-lgcc -lc.dll"

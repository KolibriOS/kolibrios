MELIBC_CWD = tup.getcwd() .. "/develop/libraries/menuetlibc"
MELIBC_VAR = tup.getvariantdir() .. "/develop/libraries/menuetlibc"

INCLUDES = INCLUDES .. " -I" .. MELIBC_CWD .. "/include"
STARTUP = MELIBC_VAR .. "/stub/crt0.o"
CFLAGS_c = " -fgnu89-inline"
LDFLAGS = LDFLAGS .. " -T" .. MELIBC_CWD .. "/include/scripts/menuetos_app_v01.ld -L" .. MELIBC_VAR .. "/lib"
tup.append_table(LIBDEPS, {MELIBC_VAR .. "/<libc>", MELIBC_VAR .. "/<libm>", MELIBC_VAR .. "/<libcpp>"})
LIBS = LIBS .. " -L" .. tup.getvariantdir() .. " -lcpp -lm -lc"

function use_dynamic_stack()
  STARTUP = MELIBC_VAR .. "/stub/crt0_dynstack.o"
end

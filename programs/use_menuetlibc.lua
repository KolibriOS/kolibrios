MELIBC = tup.getcwd() .. "/develop/libraries/menuetlibc"

INCLUDES = INCLUDES .. " -I" .. MELIBC .. "/include"
STARTUP = MELIBC .. "/stub/crt0.o"
CFLAGS_c = " -fgnu89-inline"
LDFLAGS = LDFLAGS .. string.gsub(" -T$/include/scripts/menuetos_app_v01.ld -L$/lib", "%$", MELIBC)
tup.append_table(LIBDEPS, {MELIBC .. "/<libc>", MELIBC .. "/<libm>", MELIBC .. "/<libcpp>"})
LIBS = LIBS .. " -lcpp -lm -lc"

function use_dynamic_stack()
  STARTUP = MELIBC .. "/stub/crt0_dynstack.o"
end

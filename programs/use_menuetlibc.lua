MELIBC = tup.getcwd() .. "/develop/libraries/menuetlibc"

INCLUDES = INCLUDES .. " -I" .. MELIBC .. "/include"
LDFLAGS = LDFLAGS .. string.gsub(" -T$/include/scripts/menuetos_app_v01.ld -L$/lib $/stub/crt0.o", "%$", MELIBC)
tup.append_table(LIBDEPS, {MELIBC .. "/stub/crt0.o", MELIBC .. "/<libc>", MELIBC .. "/<libm>", MELIBC .. "/<libcpp>"})
LIBS = LIBS .. " -lcpp -lm -lc"

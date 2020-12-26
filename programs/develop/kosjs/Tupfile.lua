if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")

CFLAGS = CFLAGS .. " -std=c99 -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32"
INCLUDES = INCLUDES .. " -I ."

compile_gcc{"libmujs/utftype.c", "libmujs/jsproperty.c", "libmujs/jsobject.c", "libmujs/jsdtoa.c", "libmujs/jsbuiltin.c", "libmujs/jsvalue.c", "libmujs/jsnumber.c", "libmujs/jsparse.c", "libmujs/jsstate.c", "libmujs/jsgc.c", "libmujs/jsrepr.c", "libmujs/pp.c", "libmujs/utf.c", "libmujs/jsfunction.c", "libmujs/jsdump.c", "libmujs/regexp.c", "libmujs/jsstring.c", "libmujs/jsarray.c", "libmujs/jsrun.c", "libmujs/jsdate.c", "libmujs/jscompile.c", "libmujs/jslex.c", "libmujs/jsboolean.c", "libmujs/jserror.c", "libmujs/jsintern.c", "libmujs/jsmath.c", "libmujs/jsregexp.c", "libmujs/json.c"}
tup.rule(OBJS, "kos32-ar rcs %o %f", {"libmujs.a", "<libmujs>"})

OBJS = ""

LDFLAGS = LDFLAGS .. " -L. --subsystem native"
LIBS = LIBS .. " -lmujs"
table.insert(LIBDEPS, "<libmujs>")
INCLUDES = INCLUDES .. " -Ilibmujs"

compile_gcc{"kosjs.c", "import.c"}
link_gcc("kosjs")

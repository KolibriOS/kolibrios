if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")
 
CFLAGS = CFLAGS .. " -std=c99 -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32"
INCLUDES = INCLUDES .. " -I ."
 
compile_gcc{"utftype.c", "jsproperty.c", "jsobject.c", "jsdtoa.c", "jsbuiltin.c", "jsvalue.c", "jsnumber.c", "jsparse.c", "jsstate.c", "jsgc.c", "jsrepr.c", "pp.c", "utf.c", "jsfunction.c", "jsdump.c", "regexp.c", "jsstring.c", "jsarray.c", "jsrun.c", "jsdate.c", "jscompile.c", "jslex.c", "jsboolean.c", "jserror.c", "jsintern.c", "jsmath.c", "jsregexp.c", "json.c"}
tup.rule(OBJS, "kos32-ar rcs %o %f", {"libmujs.a", "<libmujs>"})

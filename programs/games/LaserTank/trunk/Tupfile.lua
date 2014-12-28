if tup.getconfig("NO_FASM") ~= "" or tup.getconfig("NO_MSVC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_msvc.lua")
if tup.getconfig("LANG") == "ru"
then CFLAGS = CFLAGS .. " /DLANG=RUS"
else CFLAGS = CFLAGS .. " /DLANG=ENG"
end
tup.append_table(OBJS,
  tup.foreach_rule("memcmp.asm", "fasm %f %o", "%B.obj")
)
table.insert(OBJS, "smalllibc/init.obj") -- ??? it doesn't work after fasm recompilation
compile_msvc{"smalllibc/func.cpp", "smalllibc/func.h", "smalllibc/kosFile.h", "smalllibc/kosFile.cpp", "smalllibc/kosSyst.cpp", "smalllibc/mymath.h", "smalllibc/kosSyst.h", "smalllibc/math2.cpp", "smalllibc/mcsmemm.h", "smalllibc/mcsmemm.cpp", "smalllibc/purecall.cpp", "smalllibc/sprintf.cpp", "image.h", "image.cpp", "render.h", "render.cpp", "LaserTank.cpp" }
link_msvc("LaserTank")

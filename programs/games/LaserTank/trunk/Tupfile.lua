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
compile_msvc{"smalllibc/func.cpp", "smalllibc/kosFile.cpp", "smalllibc/kosSyst.cpp", "smalllibc/math2.cpp", "smalllibc/mcsmemm.cpp", "smalllibc/sprintf.cpp", "image.cpp", "render.cpp", "LaserTank.cpp" }
link_msvc("LaserTank")

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
table.insert(OBJS, "lzma_unpack.obj") -- ??? it doesn't work after fasm recompilation
compile_msvc{"*.cpp"}
link_msvc("fara")

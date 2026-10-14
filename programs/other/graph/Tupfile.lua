if tup.getconfig("NO_FASM") ~= "" or tup.getconfig("NO_MSVC") ~= "" then return end
tup.include(HELPERDIR .. "/use_msvc.lua")
tup.append_table(OBJS,
  tup.foreach_rule("*.asm", "fasm %f %o", "%B.obj")
)
compile_msvc{"*.cpp"}
link_msvc("graph")

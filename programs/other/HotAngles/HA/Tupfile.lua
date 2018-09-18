if tup.getconfig("NO_FASM") ~= "" then return end
--[[tup.foreach_rule(
  {"@HOTANGLES.asm"},
  "fasm %f %o " .. tup.getconfig("KPACK_CMD"),
  "%B"
)
]]--
tup.rule("echo lang fix " .. ((tup.getconfig("LANG") == "") and "ru" or tup.getconfig("LANG")) .. " > lang.inc", {"lang.inc"})
tup.rule({"@HOTANGLES.asm", extra_inputs = {"lang.inc"}}, "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "@HOTANGLES")

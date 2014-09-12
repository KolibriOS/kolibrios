if tup.getconfig("NO_FASM") ~= "" then return end
tup.foreach_rule(
  {"clip_get.asm", "clip_put.asm"},
  "fasm %f %o " .. tup.getconfig("KPACK_CMD"),
  "%B"
)

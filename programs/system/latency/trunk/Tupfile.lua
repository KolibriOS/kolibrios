if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("latency.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "latency")

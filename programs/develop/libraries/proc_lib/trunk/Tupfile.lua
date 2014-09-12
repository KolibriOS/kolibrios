if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("proc_lib.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "proc_lib.obj")

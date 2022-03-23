if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("SERVICES.ASM", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "drv_view")

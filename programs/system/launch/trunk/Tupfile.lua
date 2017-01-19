if tup.getconfig("NO_FASM") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../.." or tup.getconfig("HELPERDIR")
tup.rule("launch.asm", "INCLUDE=" .. HELPERDIR .. " fasm %f %o -dlang=" .. tup.getconfig("LANG") .. tup.getconfig("KPACK_CMD"), "launch")

if tup.getconfig("NO_FASM") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_fasm.lua")
add_include(tup.getvariantdir())

lang = (tup.getconfig("LANG") == "") and "en_US" or tup.getconfig("LANG")
deps = (lang == "ru_RU") and tup.rule("../../../kernel/trunk/docs/sysfuncr.txt", "iconv -f utf-8 -t cp866 %f > %o", "SysFuncr.txt") or {}
tup.rule({"docpack.asm", extra_inputs = deps}, FASM .. " -dlang=" .. lang .. " %f %o " .. tup.getconfig("KPACK_CMD"), "docpack")

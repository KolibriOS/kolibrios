if tup.getconfig("NO_FASM") ~= "" or tup.getconfig("BUILD_TYPE") == "" then return end
deps = tup.rule("echo lang fix " .. ((tup.getconfig("LANG") == "") and "en" or tup.getconfig("LANG")) .. " > lang.inc", {"lang.inc"})
DOCDIR = "../../../../data/" .. tup.getconfig("BUILD_TYPE") .. "/docs/"
if tup.getconfig("TUP_PLATFORM") == "win32"
then env_prefix = "set DOCDIR=$(DOCDIR)&&"; cp_cmd = "copy %f %o"
else env_prefix = "DOCDIR=$(DOCDIR) "; cp_cmd = "cp %f %o"
end
if tup.getconfig("LANG") == "ru"
then tup.append_table(deps,
  tup.rule("../../../../kernel/trunk/docs/sysfuncr.txt", "iconv -f utf-8 -t cp866 %f > %o", "SYSFUNCR.TXT"))
else tup.append_table(deps,
  tup.rule("../../../../kernel/trunk/docs/sysfuncs.txt", cp_cmd, "SYSFUNCS.TXT"))
end
tup.append_table(deps,
  tup.rule("../../../develop/fasm/trunk/fasm.txt", cp_cmd, "FASM.TXT")
)
tup.rule({"docpack.asm", extra_inputs = deps}, env_prefix .. "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "docpack")

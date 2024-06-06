if tup.getconfig("NO_FASM") ~= "" or tup.getconfig("BUILD_TYPE") == "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_fasm.lua")
add_include(tup.getvariantdir())

deps = tup.rule("echo lang fix " .. ((tup.getconfig("LANG") == "") and "en_US" or tup.getconfig("LANG")) .. " > %o", {"lang.inc"})
DOCDIR = "../../../../data/" .. tup.getconfig("BUILD_TYPE") .. "/docs/"
if tup.getconfig("TUP_PLATFORM") == "win32"
then env_prefix = "set DOCDIR=$(DOCDIR)&&"; cp_cmd = "copy %f %o"
else env_prefix = "DOCDIR=$(DOCDIR) "; cp_cmd = "cp %f %o"
end
if tup.getconfig("LANG") == "ru_RU"
then tup.append_table(deps,
  tup.rule("../../../../kernel/trunk/docs/sysfuncr.txt", "iconv -f utf-8 -t cp866 %f > %o", "SYSFUNCR.TXT"))
else tup.append_table(deps,
  tup.rule("../../../../kernel/trunk/docs/sysfuncs.txt", cp_cmd, "SYSFUNCS.TXT"))
end
tup.append_table(deps,
  tup.rule("../../../develop/fasm/1.73/fasm.txt", cp_cmd, "FASM.TXT")
)
tup.append_table(deps,
  tup.rule("../../../../kernel/trunk/docs/stack.txt", cp_cmd, "STACK.TXT")
)
tup.rule({"docpack.asm", extra_inputs = deps}, env_prefix .. FASM .. " %f %o " .. tup.getconfig("KPACK_CMD"), "docpack")

if tup.getconfig('NO_JWASM') ~= "" or tup.getconfig("NO_GCC") ~= "" then return end

if     tup.getconfig("LANG") == "it_IT" then
  tup.definerule{command = "echo LANG_IT = 1 > %o", outputs = {"lang.inc"}}
elseif tup.getconfig("LANG") == "es_ES" then
  tup.definerule{command = "echo LANG_SP = 1 > %o", outputs = {"lang.inc"}}
elseif tup.getconfig("LANG") == "ru_RU" then
  tup.definerule{command = "echo LANG_RU = 1 > %o", outputs = {"lang.inc"}}
elseif tup.getconfig("LANG") == "en_US" then
  tup.definerule{command = "echo LANG_EN = 1 > %o", outputs = {"lang.inc"}}
else
  tup.definerule{command = "echo LANG_EN = 1 > %o", outputs = {"lang.inc"}}
end
 
tup.rule({"RUN.asm", extra_inputs = {"lang.inc"}}, "jwasm " .. " -I" .. tup.getcwd() .. " -I" .. tup.getvariantdir() .. " -zt0 -coff -Fi lang.inc -Fo %o %f", "RUN.o")
tup.rule("RUN.o", "kos32-ld -T LScript.x -o %o %f -L ../../../../contrib/sdk/lib -l KolibriOS && kos32-objcopy %o -O binary -j .all" .. tup.getconfig("KPACK_CMD"),"RUN")

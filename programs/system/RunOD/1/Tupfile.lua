if     tup.getconfig("LANG") == "it" then
  tup.definerule{command = "echo LANG_IT = 1 > lang.inc", outputs = {"lang.inc"}}
elseif tup.getconfig("LANG") == "sp" then
  tup.definerule{command = "echo LANG_SP = 1 > lang.inc", outputs = {"lang.inc"}}
elseif tup.getconfig("LANG") == "ru" then
  tup.definerule{command = "echo LANG_RU = 1 > lang.inc", outputs = {"lang.inc"}}
elseif tup.getconfig("LANG") == "en" then
  tup.definerule{command = "echo LANG_EN = 1 > lang.inc", outputs = {"lang.inc"}}
else
  tup.definerule{command = "echo LANG_EN = 1 > lang.inc", outputs = {"lang.inc"}}
end

: RUN.asm |> jwasm -zt0 -coff -Fi lang.inc RUN.asm |> RUN.obj
: RUN.obj |> ld -T LScript.x RUN.obj -o RUN -L ../../../../contrib/sdk/lib -l KolibriOS |> RUN
: RUN     |> objcopy RUN -O binary -j .all |> RUN
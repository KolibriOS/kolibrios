CFLAGS = "/O2 /Os /Oy /GF /GS- /GR- /EHs-c- /fp:fast /GL /QIfist /Gr /arch:IA32 /DAUTOBUILD"
LDFLAGS = "/section:.bss,E /fixed:no /subsystem:native /merge:.data=.text /merge:.rdata=.text /merge:.1seg=.text /entry:crtStartUp /ltcg /nodefaultlib"

THISDIR = tup.getcwd() -- here it points to use_msvc.lua, inside rules it would be just "."
OBJS = {}

function compile_msvc(input, output)
  if not output then output = '%B.obj' end
  tup.append_table(OBJS,
    tup.foreach_rule(input, "cl /c " .. CFLAGS .. " /Fo%o %f >&2", output)
  )
end

if tup.getconfig("TUP_PLATFORM") == "win32"
then env_prefix = "set EXENAME=%f&&" -- note that there should be no space between value and "&&"
else env_prefix = "EXENAME=%f "
end

function link_msvc(input, output)
  if not output then input,output = OBJS,input end

  if type(output) ~= "string" then error("output for link_msvc should be simple string") end
  local exename = output .. ".exe"
  local mapname = output .. ".map"

  tup.rule(input, "link.exe " .. LDFLAGS .. " /out:%o /Map:" .. mapname .. " %f", {exename, extra_outputs = {mapname}})
  tup.rule(exename, env_prefix .. "fasm " .. THISDIR .. "/../data/common/doexe2.asm %o " .. tup.getconfig("KPACK_CMD"), output)
end

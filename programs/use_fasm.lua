INCLUDES = ""
FASM_DEFAULT = "fasm"

function add_include(dir)
  if INCLUDES == ""
    then INCLUDES = dir
    else INCLUDES = INCLUDES .. ";" .. dir
  end
  if tup.getconfig("TUP_PLATFORM") == "win32"
    then env_prefix = "set INCLUDE='$(INCLUDES)'&&"
    else env_prefix = "INCLUDE='$(INCLUDES)' "
  end
  FASM = env_prefix .. FASM_DEFAULT
end

add_include(tup.getcwd())

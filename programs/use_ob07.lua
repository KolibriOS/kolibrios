OB07 = tup.getcwd() .. "/develop/oberon07/compiler"
OB07_FLAGS = "-stk 1 -nochk a"

function build_ob07(input, output)
  tup.rule(input, OB07 .. " %f kosexe -out %o " .. OB07_FLAGS .. " " .. tup.getconfig("KPACK_CMD"),  output)
end

TCC = "kos32-tcc -B" .. tup.getcwd().. "/develop/ktcc/bin"

CFLAGS = "-I" .. tup.getcwd().. "/develop/ktcc/libc.obj/include "
LFLAGS = ""
LIBS = ""

OBJS = {}

function compile_tcc(input, output)
  if not output then output = '%B.o' end
  tup.append_table(OBJS,
        tup.foreach_rule(input, TCC .. " -c " .. CFLAGS .. " %f -o %o", output)
  )
end

function link_tcc(input, output)
  if not output then input,output = OBJS,input end
  tup.rule(input, TCC .. " " .. CFLAGS .. " " .. LFLAGS .. " %f -o %o " .. LIBS .. " " .. tup.getconfig("KPACK_CMD"), output)
end

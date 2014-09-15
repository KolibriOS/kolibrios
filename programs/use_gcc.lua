CFLAGS = "-Os -fno-ident -fomit-frame-pointer -fno-stack-check -fno-stack-protector -mno-stack-arg-probe -mpreferred-stack-boundary=2 -fno-exceptions -fno-asynchronous-unwind-tables -ffast-math -mno-ms-bitfields -march=pentium-mmx"
CFLAGS_c = "" -- extra flags for *.c
CFLAGS_cpp = " -fno-rtti" -- extra flags for *.cpp
LDFLAGS = "-nostdlib -n --file-alignment=16 --section-alignment=16"
INCLUDES = ""
LIBS = ""
LIBDEPS = {}
OBJS = {}

function compile_gcc(input, output)
  if not output then output = '%B.o' end
  tup.append_table(OBJS,
    tup.foreach_rule(input, "kos32-gcc -c " .. CFLAGS .. "$(CFLAGS_%e) " .. INCLUDES .. " -o %o %f", output)
  )
end

function link_gcc(input, output)
  if not output then input,output = OBJS,input end
  if tup.getconfig("HELPERDIR") == "" and #LIBDEPS then
    if type(input) == "string" then input = {input} end
    if not input.extra_inputs then input.extra_inputs = {} end
    tup.append_table(input.extra_inputs, LIBDEPS)
  end

  if type(output) == "string" then output = {output} end
  if not output.extra_outputs then output.extra_outputs = {} end
  table.insert(output.extra_outputs, output[1] .. ".map")

  tup.rule(input,
    "kos32-ld " .. LDFLAGS .. " -o %o %f -Map %o.map " .. LIBS .. " && kos32-objcopy %o -O binary " .. tup.getconfig("KPACK_CMD"),
    output)
end

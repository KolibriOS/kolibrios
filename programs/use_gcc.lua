CFLAGS_GENERIC = "-fno-ident -fomit-frame-pointer -fno-stack-check -fno-stack-protector -mno-stack-arg-probe -fno-exceptions -fno-asynchronous-unwind-tables -ffast-math -mno-ms-bitfields -march=pentium-mmx"
CFLAGS_OPTIMIZE_SIZE = "-Os -mpreferred-stack-boundary=2 " .. CFLAGS_GENERIC
CFLAGS_OPTIMIZE_SPEED = "-O2 -mpush-args -mno-accumulate-outgoing-args " .. CFLAGS_GENERIC
-- The following could in specific cases be marginally faster CFLAGS_OPTIMIZE_SPEED,
-- and in all cases gives a notable overhead in size.
CFLAGS_OPTIMIZE_SPEED_INSANE = "-O2 " .. CFLAGS_GENERIC
-- Default is optimizing by size. Override on per-project or per-file basis.
CFLAGS = CFLAGS_OPTIMIZE_SIZE

CFLAGS_c = "" -- extra flags for *.c
CFLAGS_cpp = " -fno-rtti" -- extra flags for *.cpp
LDFLAGS = "-static -nostdlib -n --file-alignment=16 --section-alignment=16"
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
  if STARTUP then
    table.insert(LIBDEPS, STARTUP)
    LDFLAGS = LDFLAGS .. " " .. STARTUP
  end
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

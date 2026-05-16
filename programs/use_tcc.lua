KTCC_TRUNK = tup.getcwd() .. "/develop/ktcc/"

TCC = "kos32-tcc -B" .. tup.getvariantdir() .. "/develop/ktcc/bin"

CFLAGS = "-I" .. KTCC_TRUNK .. "libc.obj/include -L" .. KTCC_TRUNK .. "bin/lib"
LFLAGS = ""
LIBS = ""

OBJS = {}

-- Mandatory libs: every TCC-linked program transitively depends on these
-- (kos32-tcc looks them up automatically from -B<bin>/lib/).
LIST_OF_LIBS = {}

-- Optional libs: requested by individual programs via the third argument
-- of link_tcc, e.g. link_tcc(src, out, {"cryptal"}). Only entries that the
-- current toolchain configuration can actually produce are exposed here.
OPTIONAL_LIBS = {}

if tup.getconfig("NO_FASM") == "" then
  tup.append_table(LIST_OF_LIBS, { "crt0.o", "tiny.o" })
  OPTIONAL_LIBS.sound = "libsound.a"
end

if tup.getconfig("NO_TCC") == "" then
  OPTIONAL_LIBS.cryptal = "libcryptal.a"
  OPTIONAL_LIBS.shell   = "libshell.a"
end

if tup.getconfig("NO_FASM") == "" and tup.getconfig("NO_TCC") == "" then
  tup.append_table(LIST_OF_LIBS, { "libtcc1.a" })
end

function compile_tcc(input, output)
  if not output then output = '%B.o' end
  tup.append_table(OBJS,
    tup.foreach_rule(input, TCC .. " -c " .. CFLAGS .. " %f -o %o", output)
  )
end

function link_tcc(input, output, libs)
  if not output then input, output = OBJS, input end

  if type(input) ~= "table" then
    input = { input }
  end

  if not input.extra_inputs then
    input.extra_inputs = {}
  end

  for _, lib in pairs(LIST_OF_LIBS) do
    table.insert(input.extra_inputs, KTCC_TRUNK .. "<" .. lib .. ">")
  end

  if libs then
    for _, name in pairs(libs) do
      local libfile = OPTIONAL_LIBS[name]
      if libfile then
        table.insert(input.extra_inputs, KTCC_TRUNK .. "<" .. libfile .. ">")
      end
    end
  end

  table.insert(input.extra_inputs, KTCC_TRUNK .. "bin/lib/*.def")

  tup.rule(input,
    TCC .. " " .. CFLAGS .. " " .. LFLAGS .. " " .. "%f -o %o " .. LIBS .. " " .. tup.getconfig("KPACK_CMD"),
    output)
end

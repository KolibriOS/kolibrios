if tup.getconfig("NO_TCC") ~= "" then return end
if tup.getconfig("HELPERDIR") == ""
then
  HELPERDIR = "../../../programs"
end
tup.include(HELPERDIR .. "/use_tcc.lua")

CFLAGS = CFLAGS .. " -Iinc "

SRCS = {
    "src/common.c",
    "src/errors.c",
    "src/expression.c",
    "src/formatter.c",
    "src/generatec.c",
    "src/interpret.c",
    "src/options.c",
    "src/parser.c",
    "src/statement.c",
    "src/tinybasic.c",
    "src/token.c",
    "src/tokeniser.c"
}

link_tcc(SRCS, "tinybasic");

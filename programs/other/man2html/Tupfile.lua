if tup.getconfig("NO_TCC") ~= "" then return end
if tup.getconfig("HELPERDIR") == ""
then
  HELPERDIR = "../../../programs"
end
tup.include(HELPERDIR .. "/use_tcc.lua")

SRCS = {
    "man2html.c",
    "cgibase.c",
    "abbrev.c",
    "strdefs.c"
}

link_tcc(SRCS, "man2html");

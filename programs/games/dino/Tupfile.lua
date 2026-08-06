if tup.getconfig("NO_TCC") ~= "" or tup.getconfig("NO_FASM") ~= "" then return end
if tup.getconfig("HELPERDIR") == ""
then
  HELPERDIR = "../../../programs"
end
tup.include(HELPERDIR .. "/use_tcc.lua")

if tup.getconfig("LANG") == "ru_RU"
then C_LANG = "LANG_RUS"
elseif tup.getconfig("LANG") == "es_ES"
then C_LANG = "LANG_SPA"
else C_LANG = "LANG_ENG" -- this includes default case without config
end
CFLAGS = CFLAGS .. " -D" .. C_LANG

SRCS = {
    "cloud.c",
    "game_over_panel.c",
    "horizon.c",
    "main.c",
    "obstacle.c",
    "trex.c",
    "distance_meter.c",
    "graphics.c",
    "horizon_line.c",
    "misc.c",
    "runner.c"
}

link_tcc(SRCS, "dino");

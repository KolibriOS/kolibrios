if tup.getconfig("NO_TCC") ~= "" then return end
if tup.getconfig("HELPERDIR") == ""
then
  HELPERDIR = "../../../programs"
end
tup.include(HELPERDIR .. "/use_tcc.lua")

SRCS = {
    "cloud.c"
    "game_over_panel.c"
    "horizon.c"
    "main.c"
    "obstacle.c"
    "trex.c
    "distance_meter.c"
    "graphics.c"
    "horizon_line.c"
    "misc.c"
    "runner.c"
    "ulist.c
}

LIBS = " -limg"

link_tcc(SRCS, "dino");

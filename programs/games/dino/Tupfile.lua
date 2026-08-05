if tup.getconfig("NO_TCC") ~= "" or tup.getconfig("NO_FASM") ~= "" then return end
if tup.getconfig("HELPERDIR") == ""
then
  HELPERDIR = "../../../programs"
end
tup.include(HELPERDIR .. "/use_tcc.lua")

-- Link order tuned for kpack: this permutation compresses ~27 bytes
-- better than the old one (measured over the concatenated sections)
SRCS = {
    "cloud.c",
    "horizon_line.c",
    "trex.c",
    "obstacle.c",
    "runner.c",
    "graphics.c",
    "misc.c",
    "distance_meter.c",
    "main.c",
    "game_over_panel.c",
    "horizon.c"
}

link_tcc(SRCS, "dino");

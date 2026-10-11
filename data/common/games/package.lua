tup.append_table(tmp_extra_files, {
  {"kolibrios/games/BabyPainter", tup.getcwd() .. "/BabyPainter"},
  {"kolibrios/games/fara/fara.gfx", tup.getcwd() .. "/fara.gfx"},
  {"kolibrios/games/jumpbump/", tup.getcwd() .. "/jumpbump/*"},
  {"kolibrios/games/knight", tup.getcwd() .. "/knight"},
  {"kolibrios/games/KosChess/", tup.getcwd() .. "/KosChess/*"},
  {"kolibrios/games/KosChess/images/", tup.getcwd() .. "/KosChess/images/*"},
  {"kolibrios/games/LaserTank/", tup.getcwd() .. "/LaserTank/*"},
  {"kolibrios/games/lrl/", tup.getcwd() .. "/lrl/*"},
  {"kolibrios/games/mun/data/", tup.getcwd() .. "/mun/data/*"},
  {"kolibrios/games/mun/libc.dll", tup.getcwd() .. "/mun/libc.dll"},
  {"kolibrios/games/mun/mun", tup.getcwd() .. "/mun/mun"},
  {"kolibrios/games/pig/", tup.getcwd() .. "/pig/*"},
  {"kolibrios/games/soko/", tup.getcwd() .. "/soko/*"},
  {"kolibrios/games/fridge/", tup.getcwd() .. "/fridge/*"},
})

if lang("ru_RU") then
  tup.append_table(tmp_extra_files, {
    {"kolibrios/games/ataka", tup.getcwd() .. "/ataka/ataka_ru"},
    {"kolibrios/games/sstartrek/SStarTrek", tup.getcwd() .. "/sstartrek/SStarTrek_ru"},
  })
else
  tup.append_table(tmp_img_files, {
    {"GAMES/DESCENT", tup.getcwd() .. "/descent"},
  })

  tup.append_table(tmp_extra_files, {
    {"kolibrios/games/ataka", tup.getcwd() .. "/ataka/ataka_en"},
    {"kolibrios/games/sstartrek/SStarTrek", tup.getcwd() .. "/sstartrek/SStarTrek_en"},
  })
end

if to_image({"GCC", "NASM"}) then
  tup.append_table(tmp_extra_files, {
    {"kolibrios/games/tyrian/data/", tup.getcwd() .. "/tyrian/data/*"},
    {"kolibrios/games/quake/", tup.getcwd() .. "/quake/*"}, -- not really gcc, but no sense without sdlquake
    {"kolibrios/games/wolf3d/", tup.getcwd() .. "/wolf3d/*"},
    {"kolibrios/games/doom1/", tup.getcwd() .. "/doom/*"},
  })
end

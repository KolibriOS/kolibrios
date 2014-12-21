if not exist bin mkdir rstearth_eng_bin
@erase lang.inc
@echo lang fix en >lang.inc
@fasm -m 16384 rstearth.asm rstearth_eng_bin\rstearth.kex
@erase lang.inc
@kpack rstearth_eng_bin\rstearth.kex
@copy resources\base_8bpp.png rstearth_eng_bin\base_8bpp.png
@copy resources\red_brick_8bpp.png rstearth_eng_bin\red_brick_8bpp.png
@copy resources\white_brick_8bpp.png rstearth_eng_bin\white_brick_8bpp.png
@copy resources\miku_8bpp.png rstearth_eng_bin\miku_8bpp.png
@copy resources\death_8bpp.png rstearth_eng_bin\death_8bpp.png
@copy resources\skeleton_8bpp.png rstearth_eng_bin\skeleton_8bpp.png
@copy resources\ifrit_8bpp.png rstearth_eng_bin\ifrit_8bpp.png
@copy resources\barret_8bpp.png rstearth_eng_bin\barret_8bpp.png
@copy resources\walking_with_poseidon.wav rstearth_eng_bin\walking_with_poseidon.wav
@copy resources\flaunch.wav rstearth_eng_bin\flaunch.wav
@pause
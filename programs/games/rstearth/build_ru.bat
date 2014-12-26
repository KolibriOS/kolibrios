if not exist bin mkdir rstearth_rus_bin
@erase lang.inc
@echo lang fix ru >lang.inc
@fasm -m 16384 rstearth.asm rstearth_rus_bin\rstearth.kex
@erase lang.inc
@kpack rstearth_rus_bin\rstearth.kex
@copy resources\font_russo_1bpp.png rstearth_rus_bin\font_russo_1bpp.png
@copy resources\base_8bpp.png rstearth_rus_bin\base_8bpp.png
@copy resources\red_brick_8bpp.png rstearth_rus_bin\red_brick_8bpp.png
@copy resources\white_brick_8bpp.png rstearth_rus_bin\white_brick_8bpp.png
@copy resources\miku_8bpp.png rstearth_rus_bin\miku_8bpp.png
@copy resources\death_8bpp.png rstearth_rus_bin\death_8bpp.png
@copy resources\skeleton_8bpp.png rstearth_rus_bin\skeleton_8bpp.png
@copy resources\ifrit_8bpp.png rstearth_rus_bin\ifrit_8bpp.png
@copy resources\barret_8bpp.png rstearth_rus_bin\barret_8bpp.png
@copy resources\walking_with_poseidon.wav rstearth_rus_bin\walking_with_poseidon.wav
@copy resources\flaunch.wav rstearth_rus_bin\flaunch.wav
@pause
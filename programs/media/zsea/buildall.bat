REM compiling examples
if not exist bin mkdir zSea_bin
if not exist bin mkdir zSea_bin\buttons
if not exist bin mkdir zSea_bin\plugins

@echo lang fix en >lang.inc
@fasm -m 16384 zSea.asm zSea_bin\zSea
@kpack zSea_bin\zSea

@fasm -m 16384 plugins\bmp\cnv_bmp.asm zSea_bin\plugins\cnv_bmp.obj
@kpack zSea_bin\plugins\cnv_bmp.obj

@fasm -m 16384 plugins\convert\convert.asm zSea_bin\plugins\convert.obj
@kpack zSea_bin\plugins\convert.obj

@fasm -m 16384 plugins\gif\cnv_gif.asm zSea_bin\plugins\cnv_gif.obj
@kpack zSea_bin\plugins\cnv_gif.obj

@fasm -m 16384 plugins\jpeg\cnv_jpeg.asm zSea_bin\plugins\cnv_jpeg.obj
@kpack zSea_bin\plugins\cnv_jpeg.obj

@fasm -m 16384 plugins\png\cnv_png.asm zSea_bin\plugins\cnv_png.obj
@kpack zSea_bin\plugins\cnv_png.obj

@fasm -m 16384 plugins\rotate\rotate.asm zSea_bin\plugins\rotate.obj
@kpack zSea_bin\plugins\rotate.obj

@fasm -m 16384 plugins\scaling\scaling.asm zSea_bin\plugins\scaling.obj
@kpack zSea_bin\plugins\scaling.obj

@copy zSea.ini zSea_bin\zSea.ini
@copy Docs\zSea_keys.txt zSea_bin\zSea_keys.txt
@copy buttons.png zSea_bin\buttons\buttons.png


@erase lang.inc
@pause
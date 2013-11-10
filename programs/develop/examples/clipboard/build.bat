@echo lang fix en >lang.inc
@fasm.exe -m 16384 clip_put.asm clip_put
@fasm.exe -m 16384 clip_get.asm clip_get
@erase lang.inc
@kpack clip_put
@kpack clip_get
@pause
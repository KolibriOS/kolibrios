@echo lang fix ru_RU >lang.inc
@fasm @clip.asm @clip
@fasm cliptest.asm cliptest
@fasm test2.asm test2
@erase lang.inc
@kpack @clip
@rem pause

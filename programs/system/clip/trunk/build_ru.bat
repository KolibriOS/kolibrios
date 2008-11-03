@echo lang fix ru >lang.inc
@fasm @clip.asm @clip
@fasm cliptest.asm cliptest
@fasm test2.asm test2
@erase lang.inc
@kpack @clip
@rem pause
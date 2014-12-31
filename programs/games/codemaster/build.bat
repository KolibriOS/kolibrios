@echo lang fix en >lang.inc
@fasm -m 65536 binary_master.asm binary_master
@fasm -m 65536 hang_programmer.asm hang_programmer
@fasm -m 65536 kolibri_puzzle.asm kolibri_puzzle
@erase lang.inc
@kpack binary_master
@kpack hang_programmer
@kpack kolibri_puzzle
@pause
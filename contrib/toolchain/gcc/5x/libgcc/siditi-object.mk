# This file is included several times in a row, once for each element of
# $(iter-items).  On each inclusion, we advance $o to the next element.
# $(iter-labels) and $(iter-sizes) are also advanced.

o := $(firstword $(iter-items))
iter-items := $(filter-out $o,$(iter-items))

$o-label := $(firstword $(iter-labels))
iter-labels := $(wordlist 2,$(words $(iter-labels)),$(iter-labels))

$o-size := $(firstword $(iter-sizes))
iter-sizes := $(wordlist 2,$(words $(iter-sizes)),$(iter-sizes))

$o$(objext): %$(objext): libgcc2.c
	$(CC) $(INCLUDES) $(CFLAGS) -DLIBGCC2_UNITS_PER_WORD=$($*-size) -DL$($*-label) -c $< -o $@

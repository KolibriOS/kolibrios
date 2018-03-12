# CFLAGS=     -O -DSCORE -DCAPTURE -DCLOAKING -Wno-unused-result 
CFLAGS=     -O -Wno-unused-result -DWINDOWS
CC=gcc

.c.o:
	$(CC) $(CFLAGS) -c $<

OFILES=     sst.o finish.o reports.o setup.o osx.o moving.o battle.o events.o ai.o planets.o

HFILES=     sst.h

sst:  $(OFILES)
	gcc  -o sst $(OFILES) -lm

clean:
	rm $(OFILES)
        
$(OFILES):  $(HFILES)


rem To build under Windows, execute

SET TCC=D:\_bin\ktcc_26_161007

%TCC%\kos32-tcc -I%TCC%\INCLUDE -L%TCC%\LIB -DWINDOWS -DKOS32 sst.c ai.c battle.c events.c finish.c moving.c osx.c planets.c reports.c setup.c -lck -o SStarTrek -g

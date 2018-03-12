rem To build under Windows, execute

SET TCC=D:\_bin\tcc_26_180312

%TCC%\kos32-tcc -I%TCC%\INCLUDE -L%TCC%\LIB -DKOS32 sst.c ai.c battle.c events.c finish.c moving.c osx.c planets.c reports.c setup.c -lck -o SStarTrek -g

#include "sst.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

void attakreport(void) {
	if (future[FCDBAS] < 1e30) {
		proutn("Станция в");/*Starbase in*/
		cramlc(1, batx, baty);
/* 		prout(" is currently under attack.");
		proutn("It can hold out until Stardate ");
 */		prout(" находится под атакой.");
		proutn("Она продержится максимум до ");
		cramf(future[FCDBAS], 0,1);
		prout(".");
	}
	if (isatb == 1) {
		proutn("Станция в");/*Starbase in*/
		cramlc(1, d.isx, d.isy);
/* 		prout(" is under Super-commander attack.");
		proutn("It can hold out until Stardate ");
 */		prout(" находится под атакой СуперКоммандера.");
		proutn("Она продержится максимум до ");
		cramf(future[FSCDBAS], 0, 1);
		prout(".");
	}
}
	

void report(int f) {
	char *s1,*s2,*s3;

	chew();
	s1 = (thawed?"сохраненную ":"");/*thawed*/
	switch (length) {
		case 1: s2="быструю/short"; break;
		case 2: s2="среднюю/medium"; break;
		case 4: s2="длинную/long"; break;
		default: s2="неизвестной длительности/unknown length"; break;
	}
	switch (skill) {
		case SNOVICE: s3="новичок"; break;/*novice*/
		case SFAIR: s3="салага"; break;/*fair*/
		case SGOOD: s3="отличник"; break;/*good*/
		case SEXPERT: s3="эксперт"; break;/*expert*/
		case SEMERITUS: s3="эмеритус"; break;/*emeritus*/
		default: s3="опытный"; break;/*skilled*/
	}
/* 	printf("\nYou %s playing a %s%s %s game.\n",
		   alldone? "were": "are now", s1, s2, s3);
 */	printf("\nВы %s a %s%s игру на сложности %s.\n",
		   alldone? "сыграли": "сейчас играете", s1, s2, s3);
	if (skill>SGOOD && thawed && !alldone) prout("Знак отличия не выдается.");/*No plaque is allowed*/
	/* if (tourn) printf("This is tournament game %d.\n", tourn); */
	if (tourn) printf("Это турнирная игра %d.\n", tourn);
	/* if (f) printf("Your secret password is \"%s\"\n",passwd); */
	if (f) printf("Ваш секретный пароль \"%s\"\n",passwd);
	/* printf("%d of %d Klingon ships have been destroyed", */
	printf("%d из %d клингонских кораблей уничтожено",
		   d.killk+d.killc+d.nsckill, inkling);
	/* if (d.killc) printf(", including %d Commander%s.\n", d.killc, d.killc==1?"":"s"); */
	if (d.killc) printf(", включая %d Коммандер%s.\n", d.killc, d.killc==1?"а":"ов");
	else if (d.killk+d.nsckill > 0) prout(", но ни одного Коммандера.");/*but no Commanders*/
	else prout(".");
/* 	if (skill > SFAIR) printf("The Super Commander has %sbeen destroyed.\n",
						  d.nscrem?"not ":"");
 */	if (skill > SFAIR) printf("СуперКоммандер %sбыл уничтожен.\n",
						  d.nscrem?"не ":"");
	if (d.rembase != inbase) {
/* 		proutn("There ");
		if (inbase-d.rembase==1) proutn("has been 1 base");
		else {
			proutn("have been ");
			crami(inbase-d.rembase, 1);
			proutn(" bases");
		}
		proutn(" destroyed, ");
		crami(d.rembase, 1);
		prout(" remaining.");
 */		
		if (inbase-d.rembase==1) proutn("Единственная станция была уничтожена,");
		else {
			proutn("Было ");
			crami(inbase-d.rembase, 1);
			proutn(" станций уничтожено, ");
		}
		crami(d.rembase, 1);
		prout(" осталось.");
	}
	else printf("Всего %d станций.\n", inbase);/*There are %d bases*/
	if (REPORTS || iseenit) {
		/* Don't report this if not seen and
			either the radio is dead or not at base! */
		attakreport();
		iseenit = 1;
	}
/* 	if (casual) printf("%d casualt%s suffered so far.\n",
					   casual, casual==1? "y" : "ies");
 */	if (casual) printf("%d смерт%s среди экипажа.\n",
					   casual, casual==1? "ь" : "ей");
#ifdef CAPTURE
/*     if (brigcapacity != brigfree) printf("%d Klingon%s in brig.\n",
    							brigcapacity-brigfree, brigcapacity-brigfree>1 ? "s" : "");
 */    if (brigcapacity != brigfree) printf("%d клингон%s на бриге.\n",
    							brigcapacity-brigfree, brigcapacity-brigfree>1 ? "ов" : "");
/*     if (kcaptured > 0) printf("%d captured Klingon%s turned in to Star Fleet.\n", 
                               kcaptured, kcaptured>1 ? "s" : "");
 */    if (kcaptured > 0) printf("%d пленных клингон%s присоединились к Звездному Флоту.\n", 
                               kcaptured, kcaptured>1 ? "ов" : "");
#endif
/* 	if (nhelp) printf("There were %d call%s for help.\n",
					  nhelp, nhelp==1 ? "" : "s");
 */	if (nhelp) printf("Зафиксировано %d вызов%s о помощи.\n",
					  nhelp, nhelp==1 ? "" : "ов");
	if (ship == IHE) {
		proutn("У вас осталось ");/**/
		if (nprobes) crami(nprobes,1);
		else proutn("0");/*no*/
		proutn(" разведзонд");/*deep space probe*/
		if (nprobes!=1) proutn("ов");/*s*/
		prout(".");
	}
	if (REPORTS && future[FDSPROB] != 1e30) {
		if (isarmed) 
			/* proutn("An armed deep space probe is in"); */
			proutn("Разведывательный зонд с боеголовкой в точке");
		else
			proutn("Разведывательный зонд в точке");/*A deep space probe is in*/
		cramlc(1, probecx, probecy);
		prout(".");
	}
	if (icrystl) {
		if (cryprob <= .05)
			/* prout("Dilithium crystals aboard ship...not yet used."); */
			prout("Дилитиевые кристаллы на борту...не были использованы.");
		else {
			int i=0;
			double ai = 0.05;
			while (cryprob > ai) {
				ai *= 2.0;
				i++;
			}
/* 			printf("Dilithium crystals have been used %d time%s.\n",
				   i, i==1? "" : "s");
 */			printf("Дилитиевые кристалл использовались %d раз%s.\n",
				   i, (i>1 && i<5)? "а" : "");
		}
	}
	skip(1);
}
	
void lrscan(void) {
	int x, y;
	chew();
	if (damage[DLRSENS] != 0.0) {
		/* Now allow base's sensors if docked */
		if (condit != IHDOCKED) {
			/* prout("LONG-RANGE SENSORS DAMAGED."); */
			prout("СЕНСОРЫ ДАЛЬНЕГО ОБЗОРА ПОВРЕЖДЕНЫ.");
			return;
		}
		skip(1);
		/* proutn("Starbase's long-range scan for"); */
		proutn("Используем сенсоры станции для");
	}
	else {
		skip(1);
		/* proutn("Long-range scan for"); */
		proutn("Производим дальнее сканирование для");
	}
	cramlc(1, quadx, quady);
	skip(1);
	if (coordfixed)
	for (y = quady+1; y >= quady-1; y--) {
		for (x = quadx-1; x <= quadx+1; x++) {
			if (x == 0 || x > 8 || y == 0 || y > 8)
				printf("   -1");
			else {
				printf("%5d", d.galaxy[x][y]);
				// If radio works, mark star chart so
				// it will show current information.
				// Otherwise mark with current
				// value which is fixed. 
				starch[x][y] = damage[DRADIO] > 0 ? d.galaxy[x][y]+1000 :1;
			}
		}
		putchar('\n');
	}
	else
	for (x = quadx-1; x <= quadx+1; x++) {
		for (y = quady-1; y <= quady+1; y++) {
			if (x == 0 || x > 8 || y == 0 || y > 8)
				printf("   -1");
			else {
				printf("%5d", d.galaxy[x][y]);
				// If radio works, mark star chart so
				// it will show current information.
				// Otherwise mark with current
				// value which is fixed. 
				starch[x][y] = damage[DRADIO] > 0 ? d.galaxy[x][y]+1000 :1;
			}
		}
		putchar('\n');
	}

}

void dreprt(void) {
	int jdam = FALSE, i;
	chew();

	for (i = 1; i <= ndevice; i++) {
		if (damage[i] > 0.0) {
			if (!jdam) {
				skip(1);
/* 				prout("DEVICE            -REPAIR TIMES-");
				prout("                IN FLIGHT   DOCKED");
 */				prout("ПОДСИСТЕМА        -ВРЕМЯ НА РЕМОНТ-");
				prout("                В КОСМОСЕ   НА СТАНЦИИ");
				jdam = TRUE;
			}
			printf("  %16s ", device[i]);
			cramf(damage[i]+0.05, 8, 2);
			proutn("  ");
			cramf(docfac*damage[i]+0.005, 8, 2);
			skip(1);
		}
	}
	/* if (!jdam) prout("All devices functional."); */
	if (!jdam) prout("Все подсистемы исправны.");
}

void chart(int nn) {
	int i,j;

	chew();
	skip(1);
	if (stdamtim != 1e30 && stdamtim != d.date && condit == IHDOCKED) {
/* 		prout("Spock-  \"I revised the Star Chart from the");
		prout("  starbase's records.\"");
 */		prout("Спок-  \"Я обновил звездную карту");
		prout("  по данным станции.\"");
		skip(1);
	}
	/* if (nn == 0) prout("STAR CHART FOR THE KNOWN GALAXY"); */
	if (nn == 0) prout("ЗВЕЗДНАЯ КАРТА ИЗВЕСТНОЙ ЧАСТИ ГАЛАКТИКИ");
	if (stdamtim != 1e30) {
		if (condit == IHDOCKED) {
			/* We are docked, so restore chart from base information -- these values won't update! */
			stdamtim = d.date;
			for (i=1; i <= 8 ; i++)
				for (j=1; j <= 8; j++)
					if (starch[i][j] == 1) starch[i][j] = d.galaxy[i][j]+1000;
		}
		else {
			proutn("(Последнее обновление данных ");/*Last surveillance update*/
			cramf(d.date-stdamtim, 0, 1);
			prout(" звездных лет назад.)");/*stardates ago*/
		}
	}
	if (nn ==0) skip(1);

	prout("      1    2    3    4    5    6    7    8");
	prout("    ----------------------------------------");
	if (nn==0) prout("  -");
	if (coordfixed)
	for (j = 8; j >= 1; j--) {
		printf("%d -", j);
		for (i = 1; i <= 8; i++) {
			if (starch[i][j] < 0) // We know only about the bases
				printf("  .1.");
			else if (starch[i][j] == 0) // Unknown
				printf("  ...");
			else if (starch[i][j] > 999) // Memorized value
				printf("%5d", starch[i][j]-1000);
			else
				printf("%5d", d.galaxy[i][j]); // What is actually there (happens when value is 1)
		}
		prout("  -");
	}
	else
	for (i = 1; i <= 8; i++) {
		printf("%d -", i);
		for (j = 1; j <= 8; j++) {
			if (starch[i][j] < 0) // We know only about the bases
				printf("  .1.");
			else if (starch[i][j] == 0) // Unknown
				printf("  ...");
			else if (starch[i][j] > 999) // Memorized value
				printf("%5d", starch[i][j]-1000);
			else
				printf("%5d", d.galaxy[i][j]); // What is actually there (happens when value is 1)
		}
		prout("  -");
	}
	if (nn == 0) {
		skip(1);
		crmshp();
		proutn(" находится в точке");/*is currently in*/
		cramlc(1, quadx, quady);
		skip(1);
	}
}
		
		
void srscan(int l) {
	static char requests[][3] =
		{"","da","co","po","ls","wa","en","to","sh","kl","ti"};
	char *cp;
	int leftside=TRUE, rightside=TRUE, i, j, jj, k=0, nn=FALSE;
	int goodScan=TRUE;
	switch (l) {
		case 1: // SRSCAN
			if (damage[DSRSENS] != 0) {
				/* Allow base's sensors if docked */
				if (condit != IHDOCKED) {
					/* prout("SHORT-RANGE SENSORS DAMAGED"); */
					prout("БЛИЖНИЕ СЕНСОРЫ ПОВРЕЖДЕНЫ");
					goodScan=FALSE;
				}
				else
					/* prout("[Using starbase's sensors]"); */
					prout("[Используем сенсоры станции]");
			}
			if (goodScan)
				starch[quadx][quady] = damage[DRADIO]>0.0 ?
									   d.galaxy[quadx][quady]+1000:1;
			scan();
			if (isit("chart")) nn = TRUE;
			if (isit("no")) rightside = FALSE;
			chew();
			prout("\n    1 2 3 4 5 6 7 8 9 10");
			break;
		case 2: // REQUEST
			while (scan() == IHEOL)
				/* printf("Information desired? "); */
				printf("Желаемая информация? ");
			chew();
			for (k = 1; k <= 10; k++)
				if (strncmp(citem,requests[k],min(2,strlen(citem)))==0)
					break;
			if (k > 10) {
/* 				prout("UNRECOGNIZED REQUEST. Legal requests are:\n"
					 "  date, condition, position, lsupport, warpfactor,\n"
					 "  energy, torpedoes, shields, klingons, time.");
 */				prout("ЗАПРОС НЕРАСПОЗНАН. Корректные запросы (две буквы):\n"
					 "  date(дата), condition(состояние), position(положение),\n"
					 "  lsupport (жизнеобеспечение), warpfactor (варп-фактор),\n"
					 "  energy(энергия), torpedoes(торпеды), shields (силовые щиты),\n"
					 "  klingons(клингоны), time(время).");
				return;
			}
			// no "break"
		case 3: // STATUS
			chew();
			leftside = FALSE;
			skip(1);
	}
	for (i = 1; i <= 10; i++) {
		int jj = (k!=0 ? k : i);
		if (leftside) {
			if (coordfixed) {
				printf("%2d  ", 11-i);
				for (j = 1; j <= 10; j++) {
					if (goodScan || (abs((11-i)-secty)<= 1 && abs(j-sectx) <= 1))
						printf("%c ",quad[j][11-i]);
					else
						printf("- ");
				}
			} else {
				printf("%2d  ", i);
				for (j = 1; j <= 10; j++) {
					if (goodScan || (abs(i-sectx)<= 1 && abs(j-secty) <= 1))
						printf("%c ",quad[i][j]);
					else
						printf("- ");
				}
			}
		}
		if (rightside) {
			switch (jj) {
				case 1:
					printf(" Звездная дата      %.1f", d.date);/*Stardate*/
					break;
				case 2:
					if (condit != IHDOCKED) newcnd();
					switch (condit) {
						case IHRED: cp = "КРАСНЫЙ"; break;/*RED*/
						case IHGREEN: cp = "ЗЕЛЕНЫЙ"; break;/*GREEN*/
						case IHYELLOW: cp = "ЖЕЛТЫЙ"; break;/*YELLOW*/
						case IHDOCKED: cp = "ПРИСТЫКОВАН"; break;/*DOCKED*/
					}
					/* printf(" Condition     %s", cp); */
					printf(" Состояние/Статус %s", cp);
#ifdef CLOAKING
				    if (iscloaked) printf(", МАКИРОВАН");/*CLOAKED*/
#endif
					break;
				case 3:
					printf(" Положение     ");/*Position*/
					cramlc(0, quadx, quady);
					putchar(',');
					cramlc(0, sectx, secty);
					break;
				case 4:
					printf(" Жизнеобеспечение  ");/*Life Support*/
					if (damage[DLIFSUP] != 0.0) {
						if (condit == IHDOCKED)
/* 							printf("DAMAGED, supported by starbase");
						else
							printf("DAMAGED, reserves=%4.2f", lsupres);
 */							printf("ПОВРЕЖДЕНО, питаемся от станции");
						else
							printf("ПОВРЕЖДЕНО, резерв=%4.2f", lsupres);
					}
					else
						printf("АКТИВНО");/*ACTIVE*/
					break;
				case 5:
					printf(" Варп-фактор   %.1f", warpfac);/*Warp Factor*/
					break;
				case 6:
					printf(" Энергия       %.2f", energy);/*Energy */
					break;
				case 7:
					printf(" Торпед        %d", torps);/*Torpedoes*/
					break;
				case 8:
					printf(" Силовые щиты       ");/*Shields*/
					if (damage[DSHIELD] != 0)
						printf("ПОВРЕЖДЕНЫ,");/*DAMAGED*/
					else if (shldup)
						printf("АКТИВНЫ,");/*UP*/
					else
						printf("ОТКЛЮЧЕНЫ,");/*DOWN*/
					printf(" %d%% %.1f единиц",/*units*/
						   (int)((100.0*shield)/inshld + 0.5), shield);
					break;
				case 9:
					printf(" Клингонов осталось %d", d.remkl);/*Klingons Left*/
					break;
				case 10:
					printf(" Времени осталось     %.2f", d.remtime);/*Time Left*/
					break;
			}
					
		}
		skip(1);
		if (k!=0) return;
	}
	if (nn) chart(1);
}
			
			
void eta(void) {
	int key, ix1, ix2, iy1, iy2, prompt=FALSE;
	int wfl;
	double ttime, twarp, tpower;
	if (damage[DCOMPTR] != 0.0) {
		/* prout("COMPUTER DAMAGED, USE A POCKET CALCULATOR."); */
		prout("КОМПЬЮТЕР ПОВРЕЖДЕН, ИСПОЛЬЗУЮ НАСТОЛЬНЫЙ КАЛЬКУЛЯТОР.");
		skip(1);
		return;
	}
	if (scan() != IHREAL) {
		prompt = TRUE;
		chew();
		/* proutn("Destination quadrant and/or sector? "); */
		proutn("Квадрант назначения и/или сектор? ");
		if (scan()!=IHREAL) {
			huh();
			return;
		}
	}
	iy1 = aaitem +0.5;
	if (scan() != IHREAL) {
		huh();
		return;
	}
	ix1 = aaitem + 0.5;
	if (scan() == IHREAL) {
		iy2 = aaitem + 0.5;
		if (scan() != IHREAL) {
			huh();
			return;
		}
		ix2 = aaitem + 0.5;
	}
	else {	// same quadrant
		ix2 = ix1;
		iy2 = iy1;
		ix1 = quady;	// ya got me why x and y are reversed!
		iy1 = quadx;
	}

	if (ix1 > 8 || ix1 < 1 || iy1 > 8 || iy1 < 1 ||
		ix2 > 10 || ix2 < 1 || iy2 > 10 || iy2 < 1) {
		huh();
		return;
	}
	dist = sqrt(square(iy1-quadx+0.1*(iy2-sectx))+
				square(ix1-quady+0.1*(ix2-secty)));
	wfl = FALSE;

	/* if (prompt) prout("Answer \"no\" if you don't know the value:"); */
	if (prompt) prout("Ответьте \"no\" если вы не знаете значение:");
	while (TRUE) {
		chew();
		/* proutn("Time or arrival date? "); */
		proutn("Время или дата прибытия? ");
		if (scan()==IHREAL) {
			ttime = aaitem;
			if (ttime > d.date) ttime -= d.date; // Actually a star date
			if (ttime <= 1e-10 ||
				(twarp=(floor(sqrt((10.0*dist)/ttime)*10.0)+1.0)/10.0) > 10) {
				/* prout("We'll never make it, sir."); */
				prout("Мы никогда не сделаем это, Сэр.");
				chew();
				return;
			}
			if (twarp < 1.0) twarp = 1.0;
			break;
		}
		chew();
		proutn("Варп-фактор? ");/*Warp factor*/
		if (scan()== IHREAL) {
			wfl = TRUE;
			twarp = aaitem;
			if (twarp<1.0 || twarp > 10.0) {
				huh();
				return;
			}
			break;
		}
		/* prout("Captain, certainly you can give me one of these."); */
		prout("Капитан, мне определенно нужно знать один из вариантов.");
	}
	while (TRUE) {
		chew();
		ttime = (10.0*dist)/square(twarp);
		tpower = dist*twarp*twarp*twarp*(shldup+1);
		if (tpower >= energy) { // Suggestion from Ethan Staffin -- give amount needed
			/* prout("Insufficient energy, sir: we would need "); */
			prout("Недостаточно энергии, Сэр: нам нужно ");
			cramf(tpower, 1, 1);
			proutn (" единиц.");/*units*/
			if (shldup==0 || tpower > energy*2.0) {
				if (!wfl) return;
				/* proutn("New warp factor to try? "); */
				proutn("Пробуем другой варп-фактор? ");
				if (scan() == IHREAL) {
					wfl = TRUE;
					twarp = aaitem;
					if (twarp<1.0 || twarp > 10.0) {
						huh();
						return;
					}
					continue;
				}
				else {
					chew();
					skip(1);
					return;
				}
			}
/* 			prout("But if you lower your shields,");
			proutn("remaining");
 */			prout("Но если опустить силовые щиты,");
			proutn("останется");
			tpower /= 2;
		}
		else
			proutn("Останется"); /*Remaining*/
		proutn(" энергии ");/*energy will be*/
		cramf(energy-tpower, 1, 1);
		prout(".");
		if (wfl) {
			/* proutn("And we will arrive at stardate "); */
			proutn("И мы прилетим в нужную точку к дате ");
			cramf(d.date+ttime, 1, 1);
			prout(".");
		}
		else if (twarp==1.0)
			/* prout("Any warp speed is adequate."); */
			prout("Нас устроит любая скорость варпа.");
		else {
			/* proutn("Minimum warp needed is "); */
			proutn("Минимально требуемая варп-скорость ");
			cramf(twarp, 1, 2);
			skip(1);
			/* proutn("and we will arrive at stardate "); */
			proutn("и мы прилетим в нужную точку к дате ");
			cramf(d.date+ttime, 1, 2);
			prout(".");
		}
		if (d.remtime < ttime)
			/* prout("Unfortunately, the Federation will be destroyed by then."); */
			prout("Печально, но к тому времени Федерация будет уничтожена.");
		if (twarp > 6.0)
			/* prout("You'll be taking risks at that speed, Captain"); */
			prout("Вы должны понимать риски такой скорости, Капитан");
		if ((isatb==1 && d.isy == ix1 && d.isx == iy1 &&
			 future[FSCDBAS]< ttime+d.date)||
			(future[FCDBAS]<ttime+d.date && baty==ix1 && batx == iy1))
			/* prout("The starbase there will be destroyed by then."); */
			prout("Станция будет уже уничтожена к этому времени.");
		/* proutn("New warp factor to try? "); */
		proutn("Пробуем другой варп-фактор? ");
		if (scan() == IHREAL) {
			wfl = TRUE;
			twarp = aaitem;
			if (twarp<1.0 || twarp > 10.0) {
				huh();
				return;
			}
		}
		else {
			chew();
			skip(1);
			return;
		}
	}
			
}

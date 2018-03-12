#include "sst.h"
#include <string.h>
#ifndef KOS32
#include <time.h>
#else
#include <kolibrisys.h>
#endif

void dstrct() {
	/* Finish with a BANG! */
	chew();
	if (damage[DCOMPTR] != 0.0) {
		/* prout("Computer damaged; cannot execute destruct sequence."); */
		prout("Компьютер поврежден; Невозможно запустить самоуничтожение.");
		return;
	}
	skip(1);
/* 	prouts("---WORKING---"); skip(1);
	prout("SELF-DESTRUCT-SEQUENCE-ACTIVATED");
 */	prouts("---ОЖИДАЙТЕ---"); skip(1);
	prout("ПРОЦЕДУРА-САМОУНИЧТОЖЕНИЯ-АКТИВИРОВАНА");
	prouts("   10"); skip(1);
	prouts("       9"); skip(1);
	prouts("          8"); skip(1);
	prouts("             7"); skip(1);
	prouts("                6"); skip(1);
/* 	prout("ENTER-CORRECT-PASSWORD-TO-CONTINUE-");
	prout("SELF-DESTRUCT-SEQUENCE-OTHERWISE-");
	prout("SELF-DESTRUCT-SEQUENCE-WILL-BE-ABORTED");
 */	prout("ВВЕДИТЕ-ПРАВИЛЬНО-ПАРОЛЬ-ДЛЯ-ПРОДОЛЖЕНИЯ-");
	prout("ЗАПУСКА-ПРОЦЕДУРЫ-САМОУНИЧТОЖЕНИЯ-ИНАЧЕ-");
	prout("ПРОЦЕДУРА-САМОУНИЧТОЖЕНИЯ-БУДЕТ-АБОРТИРОВАНА");
	scan();
	chew();
	if (strcmp(passwd, citem) != 0) {
/* 		prouts("PASSWORD-REJECTED;"); skip(1);
		prout("CONTINUITY-EFFECTED");
 */		prouts("ПАРОЛЬ-НЕВЕРЕН;"); skip(1);
		prout("НЕПРЕРЫВНОСТЬ-ЭФФЕКТИВНА");
		skip(1);
		return;
	}
	prouts("ПАРОЛЬ-ПРИНЯТ"); skip(1); /*PASSWORD-ACCEPTED*/
	prouts("                   5"); skip(1);
	prouts("                      4"); skip(1);
	prouts("                         3"); skip(1);
	prouts("                            2"); skip(1);
	prouts("                              1"); skip(1);
	if (Rand() < 0.15) {
		prouts("ПРОЩАЙ-ЖЕСТОКИЙ-МИР"); /*GOODBYE-CRUEL-WORLD*/
		skip(1);
	}
	skip(2);
	kaboom();
}

void kaboom(void) {
	stars();
	if (ship==IHE) prouts("***");
	prouts("********* Энтропия ");/*Entropy of*/
	crmshp();
	prouts(" максимизирована *********");/*maximized*/
	skip(1);
	stars();
	skip(1);
	if (nenhere != 0) {
		double whammo = 25.0 * energy;
		int l=1;
		while (l <= nenhere) {
			if (kpower[l]*kdist[l] <= whammo) 
				deadkl(kx[l],ky[l], quad[kx[l]][ky[l]], kx[l], ky[l]);
			l++;
		}
	}
	finish(FDILITHIUM);
}
				

void finish(FINTYPE ifin) {
	int igotit = 0;
	alldone = 1;
	skip(3);
	/* printf("It is stardate %.1f .\n\n", d.date); */
	printf("Звездное время %.1f .\n\n", d.date);
	switch (ifin) {
		case FWON: // Game has been won
			if (d.nromrem != 0)
				/* printf("The remaining %d Romulans surrender to Starfleet Command.\n", */
				printf("Последние %d ромуланцев сдались на милость Звездного Командования.\n",
					   d.nromrem);


/*             prout("You have smashed the Klingon invasion fleet and saved");
            prout("the Federation.");
 */            prout("Вы сломали хребет вторжению клингонов и спасли Федерацию.");

#ifdef CAPTURE
            if (alive && brigcapacity-brigfree > 0) { // captured Klingon crew will get transfered to starbase
                kcaptured += brigcapacity-brigfree;
                /* printf("The %d captured Klingons are transferred to Star Fleet Command.\n", */
                printf("Захваченные в плен %d клингонов переданы Звездному Командованию.\n",
                       brigcapacity-brigfree);
            }
#endif
			gamewon=1;
			if (alive) {
                double badpt;

				badpt = 5.*d.starkl + casual + 10.*d.nplankl +
						45.*nhelp+100.*d.basekl;
				if (ship == IHF) badpt += 100.0;
				else if (ship == 0) badpt += 200.0;
				if (badpt < 100.0) badpt = 0.0;	// Close enough!
				if (d.date-indate < 5.0 ||
					// killsPerDate >= RateMax
					(d.killk+d.killc+d.nsckill)/(d.date-indate) >=
					0.1*skill*(skill+1.0) + 0.1 + 0.008*badpt) {
					skip(1);
					/* prout("In fact, you have done so well that Starfleet Command"); */
					prout("Вы неплохо справились с заданием, и Звездное Командование");
					switch (skill) {
						case SNOVICE:
							/* prout("promotes you one step in rank from \"Novice\" to \"Fair\"."); */
							prout("повышает Вас в звании с \"Новичка\" до \"Салаги\".");
							break;
						case SFAIR:
							/* prout("promotes you one step in rank from \"Fair\" to \"Good\"."); */
							prout("повышает Вас в звании с \"Обнадеживающего\" до \"Отличника\".");
							break;
						case SGOOD:
							/* prout("promotes you one step in rank from \"Good\" to \"Expert\"."); */
							prout("повышает Вас в звании с \"Отличника\" до \"Эксперта\".");
							break;
						case SEXPERT:
							/* prout("promotes you to Commodore Emeritus."); */
							prout("повышает Вас в звании до Коммандера Эмеритус.");
							skip(1);
/* 							prout("Now that you think you're really good, try playing");
							prout("the \"Emeritus\" game. It will splatter your ego.");
 */							prout("Теперь, если Вы считаете себе реально крутым, попробуйте");
							prout("уровень сложности \"Эмеритус\". Это утихомирит ваше эго.");
							break;
						case SEMERITUS:
							skip(1);
/* 							prout("Computer-  ERROR-ERROR-ERROR-ERROR");
							skip(1);
							prout("  YOUR-SKILL-HAS-EXCEEDED-THE-CAPACITY-OF-THIS-PROGRAM");
							prout("  THIS-PROGRAM-MUST-SURVIVE");
							prout("  THIS-PROGRAM-MUST-SURVIVE");
							prout("  THIS-PROGRAM-MUST-SURVIVE");
							prout("  THIS-PROGRAM-MUST?- MUST ? - SUR? ? -?  VI");
							skip(1);
							prout("Now you can retire and write your own Star Trek game!");
 */							prout("Компьютер-  ОШИБКА-ОШИБКА-ОШИБКА-ОШИБКА");
							skip(1);
							prout("  ВАШ-ОПЫТ-ПРЕВЫШАЕТ-ВОЗМОЖНОСТИ-ЭТОЙ-ПРОГРАММЫ");
							prout("  ПРОГРАММА-ДОЛЖНА-ВЫЖИТЬ");
							prout("  ПРОГРАММА-ДОЛЖНА-ВЫЖИТЬ");
							prout("  ПРОГРАММА-ДОЛЖНА-ВЫЖИТЬ");
							prout("  ПРОГРАММА-ДОЛЖНА-?- ДОЛЖНА ? - ВЫЖ? ? -?  ИТЬ");
							skip(1);
							prout("Теперь можете со спокойной душой уйти в отсавку и написать собственную игру Стар Трек!");
							skip(1);
							break;
					}
					if (skill > SGOOD) {
						if (thawed
#ifdef DEBUG
							&& !idebug
#endif
							)
							/* prout("You cannot get a citation, so..."); */
							prout("Вас не запишут в анналы, так что...");
						else {
/* 							prout("Do you want your Commodore Emeritus Citation printed?");
							proutn("(You need a 132 column printer.)");
 */							prout("Вы хотите напечатать список своих высказываний, Командер Эмеритус?");
							proutn("(Вам потребуется 132-символьный принтер.)");
							chew();
							if (ja()) {
								igotit = 1;
							}
						}
					}
				}
				// Only grant long life if alive (original didn't!)
				skip(1);
				/* prout("LIVE LONG AND PROSPER."); */
				prout("ДОЛГОЙ ВАМ ЖИЗНИ И ПРОЦВЕТАНИЯ.");
			}
			score(0);
			if (igotit != 0) plaque();
			return;
		case FDEPLETE: // Federation Resources Depleted
/* 			prout("Your time has run out and the Federation has been");
			prout("conquered.  Your starship is now Klingon property,");
			prout("and you are put on trial as a war criminal.  On the");
			proutn("basis of your record, you are ");
 */			prout("Ваше время вышло и Федерация была захвачена.");
			prout("Ваш корабль теперь трофей клингонов,");
			prout("и Вас будут судить как военного преступника.");
			proutn("Рассмотрев ваши действия, Вы ");
			if (d.remkl*3.0 > inkling) {
				prout("оправданы.");/*aquitted*/
				skip(1);
				prout("ДОЛГОЙ ВАМ ЖИЗНИ И ПРОЦВЕТАНИЯ.");/*LIVE LONG AND PROSPER*/
			}
			else {
/* 				prout("found guilty and");
				prout("sentenced to death by slow torture.");
 */				prout("признаны виновным");
				prout("и приговорены к смерти под пытками.");
				alive = 0;
			}
			score(0);
			return;
		case FLIFESUP:
/* 			prout("Your life support reserves have run out, and");
			prout("you die of thirst, starvation, and asphyxiation.");
			prout("Your starship is a derelict in space.");
 */			prout("У вас кончились запасы воздуха и пищи и");
			prout("вы с командой умерли от голода и жажды, задыхаясь.");
			prout("Ваш мертвый корабль дрейфует в космосе.");
			break;
		case FNRG:
/* 			prout("Your energy supply is exhausted.");
			skip(1);
			prout("Your starship is a derelict in space.");
 */			prout("У корабля кончились запасы энергии.");
			skip(1);
			prout("Ваш мертвый корабль дрейфует в космосе.");
			break;
		case FBATTLE:
/* 			proutn("The ");
			crmshp();
			prout("has been destroyed in battle.");
			skip(1);
			prout("Dulce et decorum est pro patria mori.");
 */			proutn("Ваш корабль ");
			crmshp();
			prout("был уничтожен в бою.");
			skip(1);
			prout("Dulce et decorum est pro patria mori.");
			prout("/Сладка и прекрасна за родину смерть./");
			break;
		case FNEG3:
/* 			prout("You have made three attempts to cross the negative energy");
			prout("barrier which surrounds the galaxy.");
			skip(1);
			prout("Your navigation is abominable.");
 */			prout("Вы сделали три попытки пересечь барьер отрицательной энергии");
			prout(" окружающий галактику.");
			skip(1);
			prout("Вы ужасный навигатор.");
			score(0);
			return;
		case FNOVA:
/* 			prout("Your starship has been destroyed by a nova.");
			prout("That was a great shot.");
 */			prout("Ваш корабль был уничтожен новой звездой.");
			prout("Хорошая попытка.");
			skip(1);
			break;
		case FSNOVAED:
/* 			proutn("The ");
			crmshp();
			prout(" has been fried by a supernova.");
			prout("...Not even cinders remain...");
 */			proutn("Ваш корабль ");
			crmshp();
			prout(" был уничтожен сверхновой.");
			prout("...И не осталось даже пепла...");
			break;
		case FABANDN:
/* 			prout("You have been captured by the Klingons. If you still");
			prout("had a starbase to be returned to, you would have been");
			prout("repatriated and given another chance. Since you have");
			prout("no starbases, you will be mercilessly tortured to death.");
 */			prout("Вы были захвачены клингонами. Если бы у вас ");
			prout("осталась хоть одна станция для возвращения, вы могли бы");
			prout("получить второй шанс. Но поскольку станций больше нет,");
			prout("вас беспощадно запытают до смерти.");
			break;
		case FDILITHIUM:
			/* prout("Your starship is now an expanding cloud of subatomic particles"); */
			prout("Ваш корабль превратился в облако субатомных частиц");
			break;
		case FMATERIALIZE:
/* 			prout("Starbase was unable to re-materialize your starship.");
			prout("Sic transit gloria muntdi");
 */			prout("Станция не смогла обратно материализовать ваш корабль.");
			prout("Sic transit gloria muntdi");
			prout("/Так проходит мирская слава/");
			break;
		case FPHASER:
/* 			proutn("The ");
			crmshp();
			prout(" has been cremated by its own phasers.");
 */			proutn("Ваш корабль ");
			crmshp();
			prout(" самоуничтожился огнем собственных фазеров.");
			break;
		case FLOST:
/* 			prout("You and your landing party have been");
			prout("converted to energy, dissipating through space.");
 */			prout("Вы и группа высадки ");
			prout("превратились в чистую энергию, распыленную в космосе.");
			break;
		case FMINING:
/* 			prout("You are left with your landing party on");
			prout("a wild jungle planet inhabited by primitive cannibals.");
			skip(1);
			prout("They are very fond of \"Captain Kirk\" soup.");
			skip(1);
			proutn("Without your leadership, the ");
			crmshp();
			prout(" is destroyed.");
 */			prout("Вы остались вместе с группой высадки");
			prout("на дикой планете, населенной доисторическими каннибалами.");
			skip(1);
			prout("Они очень любят суп \"Капитан Кирк\".");
			skip(1);
			proutn("Без Вашего командования, корабль ");
			crmshp();
			prout(" был уничтожен.");
			break;
		case FDPLANET:
/* 			prout("You and your mining party perish.");
			skip(1);
			prout("That was a great shot.");
 */			prout("Вы бесследно пропали вместе с шахтерами.");
			skip(1);
			prout("Это была хорошая попытка.");
			skip(1);
			break;
		case FSSC:
			/* prout("The Galileo is instantly annihilated by the supernova."); */
			prout("Галилео был мгновенно аннигилирован суперновой.");
			// no break;
		case FPNOVA:
/* 			prout("You and your mining party are atomized.");
			skip(1);
			proutn("Mr. Spock takes command of the ");
			crmshp();
			prout(" and");
			prout("joins the Romulans, reigning terror on the Federation.");
 */			prout("Вы с группой шахтеров распались на атомы.");
			skip(1);
			proutn("Мистер Спок принял командование ");
			crmshp();
			prout(" и");
			prout("присоединился к ромуланцам, сея разрушение и террор по всей Федерации.");
			break;
		case FSTRACTOR:
/* 			prout("The shuttle craft Galileo is also caught,");
			prout("and breaks up under the strain.");
			skip(1);
			prout("Your debris is scattered for millions of miles.");
			proutn("Without your leadership, the ");
			crmshp();
			prout(" is destroyed.");
 */			prout("Шаттл Галилео попал в поле действия силового луча,");
			prout("и был разорван силами тяготения.");
			skip(1);
			prout("Ваши обломки рассеяны на миллионы миль.");
			proutn("Без Вашего командования, корабль ");
			crmshp();
			prout(" был уничтожен.");
			break;
		case FDRAY:
/* 			prout("The mutants attack and kill Spock.");
			prout("Your ship is captured by Klingons, and");
			prout("your crew is put on display in a Klingon zoo.");
 */			prout("Мутанты атаковали и убили Спока.");
			prout("Ваш корабль был захвачен клингонами и теперь");
			prout("ваша команда представлена в клингонском зоопарке.");
			break;
		case FTRIBBLE:
/* 			prout("Tribbles consume all remaining water,");
			prout("food, and oxygen on your ship.");
			skip(1);
			prout("You die of thirst, starvation, and asphyxiation.");
			prout("Your starship is a derelict in space.");
 */			prout("Трибблы сожрали всю оставшуюся воду,");
			prout("еду и кислород на вашем корабле.");
			skip(1);
			prout("Вы с командой умерли от голода и жажды, задыхаясь.");
			prout("Ваш мертвый корабль дрейфует в космосе.");
			break;
		case FHOLE:
/* 			prout("Your ship is drawn to the center of the black hole.");
			prout("You are crushed into extremely dense matter.");
 */			prout("Вас затянуло в центр черной дыры.");
			prout("Вы теперь исключительно маленькая и сверхтвердая пылинка.");
			break;
#ifdef CLOAKING
		case FCLOAK:
			ncviol++;
/* 			prout("You have violated the Treaty of Algeron.");
			prout("The Romulan Empire can never trust you again.");
 */			prout("Вы нарушили Алгеронское Соглашение.");
			prout("Ромуланская Империя никогда больше не поверит вам.");
			break;
#endif
	}
#ifdef CLOAKING
	if (ifin!=FWON && ifin!=FCLOAK && iscloaked!=0) {
/* 		prout("Your ship was cloaked so your subspace radio did not receive anything.");
		prout("You may have missed some warning messages.");
 */		prout("Ваш корабль был невидим, так что подпространсвенное радио не работало.");
		prout("Вы могли пропустить некоторые предупредительные сообщения.");
		skip(1);
	}
#endif

	if (ship==IHF) ship= 0;
	else if (ship == IHE) ship = IHF;
	alive = 0;
	if (d.remkl != 0) {
		double goodies = d.remres/inresor;
		double baddies = (d.remkl + 2.0*d.remcom)/(inkling+2.0*incom);
		if (goodies/baddies >= 1.0+0.5*Rand()) {
/* 			prout("As a result of your actions, a treaty with the Klingon");
			prout("Empire has been signed. The terms of the treaty are");
 */			prout("В результате ваших дествий, было подписано мирное");
			prout("соглашение с клингонами. Условия соглашения были");
			if (goodies/baddies >= 3.0+Rand()) {
/* 				prout("favorable to the Federation.");
				skip(1);
				prout("Congratulations!");
 */				prout("выгодными для Федерации.");
				skip(1);
				prout("Поздравляем!");
			}
			else
				/* prout("highly unfavorable to the Federation."); */
				prout("исключительно невыгодными для Федерации.");
		}
		else
			/* prout("The Federation will be destroyed."); */
			prout("Федерация будет уничтожена.");
	}
	else {
/* 		prout("Since you took the last Klingon with you, you are a");
		prout("martyr and a hero. Someday maybe they'll erect a");
		prout("statue in your memory. Rest in peace, and try not");
		prout("to think about pigeons.");
 */		prout("Вы забрали последнего клингона с собой на тот свет.");
		prout("Ваша героическая жертва не будет забыта и, возможно, потомки даже");
		prout("возведут статю в Вашу честь. Покойтесь с миром и старайтесь");
		prout("не думать о голубях.");
		gamewon = 1;
	}
	score(0);
}

void score(int inGame) {
	double timused = d.date - indate;
    int ithperd, iwon, klship;
    int dnromrem = d.nromrem; // Leave global value alone

    if (!inGame) pause(0);

	iskill = skill;
	if ((timused == 0 || d.remkl != 0) && timused < 5.0) timused = 5.0;
	perdate = (d.killc + d.killk + d.nsckill)/timused;
	ithperd = 500*perdate + 0.5;
	iwon = 0;
	if (gamewon) iwon = 100*skill;
	if (ship == IHE) klship = 0;
	else if (ship == IHF) klship = 1;
	else klship = 2;
	if (gamewon == 0 || inGame) dnromrem = 0; // None captured if no win or if still in the game
	iscore = 10*d.killk + 50*d.killc + ithperd + iwon
			 - 100*d.basekl - 100*klship - 45*nhelp -5*d.starkl - casual
		 + 20*d.nromkl + 200*d.nsckill - 10*d.nplankl + dnromrem;
#ifdef CLOAKING
	iscore -= 100*ncviol;
#endif
#ifdef CAPTURE
	iscore += 3*kcaptured;
#endif
	if (alive == 0) iscore -= 200;
	skip(2);
/*     if (inGame) prout("Your score so far --");
    else prout("Your score --");
 */    if (inGame) prout("На данный момент ваши баллы --");
    else prout("Ваши баллы --");
	if (d.nromkl)
	 /* printf(d.nromkl> 1 ? "%6d Romulan ships destroyed            %5d\n" : "%6d Romulan ship destroyed             %5d\n", */
		printf(d.nromkl> 1 ? "%6d ромуланских кораблей уничтожено        %5d\n" : "%6d ромуланских кораблей уничтожено             %5d\n",
			   d.nromkl, 20*d.nromkl);
	if (dnromrem)
	 /* printf(dnromrem > 1 ? "%6d Romulan ships captured             %5d\n" : "%6d Romulan ship captured              %5d\n", */
		printf(dnromrem > 1 ? "%6d ромуланских кораблей захвачено         %5d\n" : "%6d ромуланских кораблей захвачено              %5d\n",
			   dnromrem, dnromrem);
	if (d.killk)
	 /* printf(d.killk > 1 ? "%6d ordinary Klingon ships destroyed   %5d\n" : "%6d ordinary Klingon ship destroyed    %5d\n", */
		printf(d.killk > 1 ? "%6d боевых кораблей клингонов уничтожено    %5d\n" : "%6d боевых кораблей клингонов уничтожено    %5d\n",
			   d.killk,  10*d.killk);
	if (d.killc)
		/* printf(d.killc > 1 ? "%6d Klingon Commander ships destroyed  %5d\n" : "%6d Klingon Commander ship destroyed   %5d\n", */
		printf(d.killc > 1 ? "%6d клингонских Командеров уничтожено       %5d\n" : "%6d клингонских Командеров уничтожено   %5d\n",
			   d.killc, 50*d.killc);
	if (d.nsckill)
     /* printf("%6d Super-Commander ship destroyed     %5d\n", */
		printf("%6d СуперКоммандеров уничтожено        %5d\n",
			   d.nsckill, 200*d.nsckill);
	if (ithperd)
     /* printf("%6.2f Klingons per stardate              %5d\n", */
		printf("%6.2f клингонов в среднем за дату       %5d\n",
			   perdate, ithperd);
#ifdef CAPTURE
	if (kcaptured)
		/* printf(kcaptured > 1 ? "%6d Klingons captured                  %5d\n" : "%6d Klingon captured                   %5d\n", */
		printf(kcaptured > 1 ? "%6d клингонов захвачено                  %5d\n" : "%6d клингонов захвачено                   %5d\n",
		        kcaptured, 3*kcaptured);
#endif
	if (d.starkl)
		/* printf(d.starkl > 1 ? "%6d stars destroyed by your action     %5d\n" : "%6d star destroyed by your action      %5d\n", */
		printf(d.starkl > 1 ? "%6d звезд уничтожено вашими действиями     %5d\n" : "%6d звезд уничтожено вашими действиями      %5d\n",
			   d.starkl, -5*d.starkl);
	if (d.nplankl)
		/* printf(d.nplankl > 1 ? "%6d planets destroyed by your action   %5d\n" : "%6d planet destroyed by your action    %5d\n", */
		printf(d.nplankl > 1 ? "%6d планет уничтожено вашими действиями    %5d\n" : "%6d планет уничтожено вашими действиями    %5d\n",
			   d.nplankl, -10*d.nplankl);
	if (d.basekl)
		printf(d.basekl > 1 ? "%6d станций уничтожено вашими действиями     %5d\n" : "%6d станций уничтожено вашими действиями      %5d\n",
			   d.basekl, -100*d.basekl);
	if (nhelp)
		printf(nhelp > 1 ? "%6d вызовов о помощи со станций       %5d\n" : "%6d вызовов о помощи со станций        %5d\n",
			   nhelp, -45*nhelp);
	if (casual)
		printf(casual > 1 ? "%6d потерь среди экипажа                %5d\n" : "%6d потерь среди экипажа                  %5d\n",
			   casual, -casual);
	if (klship)
		printf(klship > 1 ? "%6d кораблей потеряно или уничтожено            %5d\n" : "%6d кораблей потеряно или уничтожено             %5d\n",
			   klship, -100*klship);
#ifdef CLOAKING
	if (ncviol>0)
		printf(ncviol > 1 ? "%6d нарушений Алгеронского соглашения       %5d\n" : "%6d нарушений Алгеронского соглашения        %5d\n",
		       ncviol, -100*ncviol);
#endif
	if (alive==0)
		/* prout("Penalty for getting yourself killed        -200"); */
		prout("Штраф за то, что Вы дали себя убить        -200");
	if (gamewon) {
		skip(1);
		/* proutn("Bonus for winning "); */
		proutn("Бонус за победу ");
		switch (skill) {
/* 			case SNOVICE: proutn("Novice game  "); break;
			case SFAIR: proutn("Fair game    "); break;
			case SGOOD: proutn("Good game    "); break;
			case SEXPERT: proutn("Expert game  "); break;
			case SEMERITUS: proutn("Emeritus game"); break;
 */			case SNOVICE: proutn("Игра Новичка  "); break;
			case SFAIR: proutn("Игра Салаги    "); break;
			case SGOOD: proutn("Игра Отличника    "); break;
			case SEXPERT: proutn("Игра Эксперта  "); break;
			case SEMERITUS: proutn("Игра Эмеритуса"); break;
		}
		printf("           %5d\n", iwon);
	}
	skip(2);
    printf("Итоговый балл                               %5d\n", iscore);/*TOTAL SCORE*/
    /* if (inGame && skill < SGOOD) printf("REMEMBER--The score doesn't really matter until the mission is accomplished!\n"); */
    if (inGame && skill < SGOOD) printf("ПОМНИТЕ--Баллы ничего не значат, пока миссия не окончена!\n");
}

void plaque(void) {
	FILE *fp=NULL;
#ifndef KOS32	
	time_t t;
#else
	int kos_date, kos_time;
#endif
	char *timestring;
	int nskip;
	char winner[128];
	skip(2);
	
	while (fp == NULL) {
		/* printf("File or device name for your plaque:"); */
		printf("Введите имя файла для ваших достижений:");
#ifndef KOS32		
		fgets(winner, 128, stdin);
#else
		gets(winner);
#endif		
		winner[strlen(winner)-1] = '\0';
		fp = fopen(winner, "w");
		if (fp==NULL) {
			printf("Неверное имя.\n");/*Invalid name*/
		}
	}

	/* printf("Enter name to go on plaque (up to 30 characters):"); */
	printf("Введите имя для памятного значка (максимум 30 символов):");
#ifndef KOS32		
		fgets(winner, 128, stdin);
#else
		gets(winner);
#endif		
	winner[strlen(winner)-1] = '\0';
	winner[30] = '\0';
	nskip = 64 - strlen(winner)/2;

	fprintf(fp,"\n\n\n\n");
	/* --------DRAW ENTERPRISE PICTURE. */
	fprintf(fp, "                                                                EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n" );
	fprintf(fp, "                                      EEE                      E  : :                                         :  E\n" );
	fprintf(fp, "                                    EE   EEE                   E  : :                   NCC-1701              :  E\n");
	fprintf(fp, "                    EEEEEEEEEEEEEEEE        EEEEEEEEEEEEEEE    E  : :                                         : E\n");
	fprintf(fp, "                     E                                     E    EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n");
	fprintf(fp, "                      EEEEEEEEE               EEEEEEEEEEEEE                 E  E\n");
	fprintf(fp, "                               EEEEEEE   EEEEE    E          E              E  E\n");
	fprintf(fp, "                                      EEE           E          E            E  E\n");
	fprintf(fp, "                                                       E         E          E  E\n");
	fprintf(fp, "                                                         EEEEEEEEEEEEE      E  E\n");
	fprintf(fp, "                                                      EEE :           EEEEEEE  EEEEEEEE\n");
	fprintf(fp, "                                                    :E    :                 EEEE       E\n");
	fprintf(fp, "                                                   .-E   -:-----                       E\n");
	fprintf(fp, "                                                    :E    :                            E\n");
	fprintf(fp, "                                                      EE  :                    EEEEEEEE\n");
	fprintf(fp, "                                                       EEEEEEEEEEEEEEEEEEEEEEE\n");
	fprintf(fp, "\n\n\n");
	fprintf(fp, "                                                       U. S. S. ENTERPRISE\n");
	fprintf(fp, "\n\n\n\n");
	/* fprintf(fp, "                                  For demonstrating outstanding ability as a starship captain\n"); */
	fprintf(fp, "                                  За проявленные выдающиеся достижения в должности капитана космического корабля\n");
	fprintf(fp, "\n");
	/* fprintf(fp, "                                                Starfleet Command bestows to you\n"); */
	fprintf(fp, "                                                Звездное Командование представляет Вас\n");
	fprintf(fp, "\n");
	fprintf(fp,"%*s%s\n\n", nskip, "", winner);
/* 	fprintf(fp, "                                                           the rank of\n\n");
	fprintf(fp, "                                                       \"Commodore Emeritus\"\n\n");
 */	fprintf(fp, "                                                           к званию\n\n");
	fprintf(fp, "                                                       \"Командор Эмеритус\"\n\n");
	fprintf(fp, "                                                          ");
	switch (iskill) {
/* 		case SEXPERT: fprintf(fp," Expert level\n\n"); break;
		case SEMERITUS: fprintf(fp,"Emeritus level\n\n"); break;
		default: fprintf(fp," Cheat level\n\n"); break;
 */		case SEXPERT: fprintf(fp," уровень Эксперта\n\n"); break;
		case SEMERITUS: fprintf(fp,"уровень Эмеритуса\n\n"); break;
		default: fprintf(fp," обычный читерский уровень\n\n"); break;
	}
#ifndef KOS32	
	t = time(NULL);
	timestring = ctime(&t);
	/* fprintf(fp, "                                                 This day of %.6s %.4s, %.8s\n\n", */
	fprintf(fp, "                                                 Этот день %.6s %.4s, %.8s\n\n",
			timestring+4, timestring+20, timestring+11);
#else
	kos_date = _ksys_get_date();
	kos_time = _ksys_get_system_clock();
	/* fprintf(fp, "                                                 This day of %02i/%02i/%02i %02i:%02i:%02i\n\n", */
	fprintf(fp, "                                                 Этот день %02i/%02i/%02i %02i:%02i:%02i\n\n",
			kos_date >> 16, (kos_date & 0xFF00) >> 8, (kos_date & 0xFF) + 2000,
			kos_time & 0xFF, (kos_time & 0xFF00) >> 8, kos_time >> 16 );
#endif
/* 	fprintf(fp,"                                                        Your score:  %d\n\n", iscore);
	fprintf(fp,"                                                    Klingons per stardate:  %.2f\n", perdate);
 */	fprintf(fp,"                                                        Ваш балл:  %d\n\n", iscore);
	fprintf(fp,"                                                    Клингонов на дату:  %.2f\n", perdate);
	fclose(fp);
}

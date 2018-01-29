#ifndef KOS32
#include <time.h>
#endif
#include "sst.h"

void prelim(void) {
	skip(2);
	prout("-SUPER- STAR TREK");
	skip(1);
	prout("Latest update-21 Sept 78");
	skip(1);
}

void freeze(int boss) {
	char *x, *y;
	FILE *fp;
	int key;
	if (boss) {
		strcpy(citem, "emsave.trk");
	}
	else {
		if ((key = scan()) == IHEOL) {
			proutn("File name(9 characters maximum): ");
			key = scan();
		}
		if (key != IHALPHA) {
			huh();
			return;
		}
		chew();
		if (strchr(citem, '.') == NULL) {
			strcat(citem, ".trk");
		}
	}
	if ((fp = fopen(citem, "wb")) == NULL) {
		proutn("Can't freeze game as file ");
		proutn(citem);
		skip(1);
		return;
	}
	fwrite(&d, sizeof(d), 1, fp);
	fwrite(&snapsht, sizeof(snapsht), 1, fp);
	fwrite(quad, sizeof(quad), 1, fp);
	fwrite(kx, sizeof(kx), 1, fp);
	fwrite(ky, sizeof(ky), 1, fp);
	fwrite(starch, sizeof(starch), 1, fp);
	fwrite(kpower, sizeof(kpower), 1, fp);
	fwrite(kdist, sizeof(kdist), 1, fp);
	fwrite(kavgd, sizeof(kavgd), 1, fp);
	fwrite(damage, sizeof(damage), 1, fp);
	fwrite(future, sizeof(future), 1, fp);
	fwrite(&a, sizeof(a), 1, fp);
	fwrite(passwd, sizeof(passwd), 1, fp);

	fclose(fp);

	/* I hope that's enough! */
}


void thaw(void) {
	char *x, *y;
	FILE *fp;
	int key;

	passwd[0] = '\0';
	if ((key = scan()) == IHEOL) {
		proutn("File name: ");
		key = scan();
	}
	if (key != IHALPHA) {
		huh();
		return;
	}
	chew();
	if (strchr(citem, '.') == NULL) {
		strcat(citem, ".trk");
	}
	if ((fp = fopen(citem, "rb")) == NULL) {
		proutn("Can't find game file ");
		proutn(citem);
		skip(1);
		return;
	}
	fread(&d, sizeof(d), 1, fp);
	fread(&snapsht, sizeof(snapsht), 1, fp);
	fread(quad, sizeof(quad), 1, fp);
	fread(kx, sizeof(kx), 1, fp);
	fread(ky, sizeof(ky), 1, fp);
	fread(starch, sizeof(starch), 1, fp);
	fread(kpower, sizeof(kpower), 1, fp);
	fread(kdist, sizeof(kdist), 1, fp);
	fread(kavgd, sizeof(kavgd), 1, fp);
	fread(damage, sizeof(damage), 1, fp);
	fread(future, sizeof(future), 1, fp);
	fread(&a, sizeof(a), 1, fp);
	fread(passwd, sizeof(passwd), 1, fp);

	fclose(fp);

	/* I hope that's enough! */
}

void abandn(void) {
	int nb, l;

	chew();
	if (condit==IHDOCKED) {
		if (ship!=IHE) {
			prout("You cannot abandon Ye Faerie Queene.");
			return;
		}
	}
	else {
		/* Must take shuttle craft to exit */
		if (damage[DSHUTTL]==-1) {
			prout("Ye Faerie Queene has no shuttle craft.");
			return;
		}
		if (damage[DSHUTTL]<0) {
			prout("Shuttle craft now serving Big Mac's.");
			return;
		}
		if (damage[DSHUTTL]>0) {
			prout("Shuttle craft damaged.");
			return;
		}
		if (landed==1) {
			prout("You must be aboard the Enterprise.");
			return;
		}
		if (iscraft!=1) {
			prout("Shuttle craft not currently available.");
			return;
		}
		/* Print abandon ship messages */
		skip(1);
		prouts("***ABANDON SHIP!  ABANDON SHIP!");
		skip(1);
		prouts("***ALL HANDS ABANDON SHIP!");
		skip(2);
		prout("Captain and crew escape in shuttle craft.");
		prout("Remainder of ship's complement beam down");
		prout("to nearest habitable planet.");
		if (d.rembase==0) {
			/* Ops! no place to go... */
			finish(FABANDN);
			return;
		}
		/* If at least one base left, give 'em the Faerie Queene */
		skip(1);
		icrystl = 0; /* crystals are lost */
		nprobes = 0; /* No probes */
		prout("You are captured by Klingons and released to");
		prout("the Federation in a prisoner-of-war exchange.");
		nb = Rand()*d.rembase+1;
		/* Set up quadrant and position FQ adjacient to base */
		if (quadx!=d.baseqx[nb] || quady!=d.baseqy[nb]) {
			quadx = d.baseqx[nb];
			quady = d.baseqy[nb];
			sectx = secty = 5;
			newqad(1);
		}
		for (;;) {
			/* position next to base by trial and error */
			quad[sectx][secty] = IHDOT;
			for (l = 1; l <= 10; l++) {
				sectx = 3.0*Rand() - 1.0 + basex;
				secty = 3.0*Rand() - 1.0 + basey;
				if (sectx >= 1 && sectx <= 10 &&
					secty >= 1 && secty <= 10 &&
					quad[sectx][secty] == IHDOT) break;
			}
			if (l < 11) break; /* found a spot */
			sectx=5;
			secty=5;
			newqad(1);
		}
	}
	/* Get new commission */
	quad[sectx][secty] = ship = IHF;
	prout("Starfleet puts you in command of another ship,");
	prout("the Faerie Queene, which is antiquated but,");
	prout("still useable.");
	if (icrystl!=0) prout("The dilithium crystals have been moved.");
	imine=0;
	iscraft=0; /* Gallileo disappears */
#ifdef CAPTURE
	brigcapacity = 300; // Less capacity now
	brigfree = brigcapacity;
#endif
#ifdef CLOAKING
    iscloaked = iscloaking = FALSE;
#endif
	/* Resupply ship */
	condit=IHDOCKED;
	for (l = 1; l <= ndevice; l++) damage[l] = 0.0;
	damage[DSHUTTL] = -1;
	energy = inenrg = 3000.0;
	shield = inshld = 1250.0;
	torps = intorps = 6;
	lsupres=inlsr=3.0;
	shldup=0;
	warpfac=5.0;
	wfacsq=25.0;
	return;
}
	
void setup(void) {
	int i,j, krem, klumper;
	int ix, iy;
	alldone = gamewon = 0;
#ifdef DEBUG
	idebug = 0;
#endif
	//  Decide how many of everything
	if (choose()) return; // frozen game
	// Prepare the Enterprise
	ship = IHE;
	energy = inenrg = 5000.0;
	shield = inshld = 2500.0;
	shldchg = shldup = 0;
	inlsr = 4.0;
	lsupres = 4.0;
	iran8(&quadx, &quady);
	iran10(&sectx, &secty);
	torps = intorps = 10;
	nprobes = (int)(3.0*Rand() + 2.0);	/* Give them 2-4 of these wonders */
	warpfac = 5.0;
	wfacsq = warpfac * warpfac;
	for (i=0; i <= ndevice; i++) damage[i] = 0.0;
	// Set up assorted game parameters
	batx = baty = 0;
	d.date = indate = 100.0*(int)(31.0*Rand()+20.0);
	d.killk = d.killc = nkinks = nhelp = resting = casual = d.nromkl = 0;
// Added d.starkl=0 6/2015
    isatb = iscate = imine = icrystl = icraft = d.nsckill = d.nplankl = d.starkl = 0;
	iscraft = 1;
	landed = -1;
	alive = 1;
	docfac = 0.25;
	for (i = 1; i <= 8; i++)
		for (j = 1; j <= 8; j++) d.newstuf[i][j] = starch[i][j] = 0;
	// Initialize times for extraneous events
	future[FSNOVA] = d.date + expran(0.5 * intime);
	future[FTBEAM] = d.date + expran(1.5 * (intime / d.remcom));
	future[FSNAP] = d.date + 1.0 + Rand(); // Force an early snapshot
	future[FBATTAK] = d.date + expran(0.3*intime);
	future[FCDBAS] = 1e30;
	future[FSCMOVE] = d.nscrem ? d.date+0.2777 : 1e30;
	future[FSCDBAS] = 1e30;
	future[FDSPROB] = 1e30;
	// Starchart is functional
	stdamtim = 1e30;
	// Put stars in the galaxy
	instar = 0;
	for (i=1; i<=8; i++)
		for (j=1; j<=8; j++) {
			int k = Rand()*9.0 + 1.0;
			instar += k;
			d.galaxy[i][j] = k;
		}
	// Locate star bases in galaxy
	for (i = 1; i <= inbase; i++) {
		int contflag;
		do {
			do iran8(&ix, &iy);
			while (d.galaxy[ix][iy] >= 10);
			contflag = FALSE;
			for (j = i-1; j > 0; j--) {
				/* Improved placement algorithm to spread out bases */
				double distq = square(ix-d.baseqx[j]) + square(iy-d.baseqy[j]);
				if (distq < 6.0*(6-inbase) && Rand() < 0.75) {
					contflag = TRUE;
#ifdef DEBUG
					printf("DEBUG: Abandoning base #%d at %d-%d\n", i, ix, iy);
#endif
					break;
				}
#ifdef DEBUG
				else if (distq < 6.0 * (6-inbase)) {
					printf("DEBUG: saving base #%d, close to #%d\n", i, j);
				}
#endif
			}
		} while (contflag);
			
		d.baseqx[i] = ix;
		d.baseqy[i] = iy;
		starch[ix][iy] = -1;
		d.galaxy[ix][iy] += 10;
	}
	// Position ordinary Klingon Battle Cruisers
	krem = inkling - incom - d.nscrem;
	klumper = 0.25*skill*(9.0-length)+1.0;
	if (klumper > 9) klumper = 9; // Can't have more than 9 in quadrant
	do {
		double r = Rand();
		int klump = (1.0 - r*r)*klumper;
		if (klump > krem) klump = krem;
		krem -= klump;
		klump *= 100;
		do iran8(&ix, &iy);
		while (d.galaxy[ix][iy] + klump >= 1000);
		d.galaxy[ix][iy] += klump;
	} while (krem > 0);
	// Position Klingon Commander Ships
#ifdef DEBUG
	klumper = 1;
#endif
	for (i = 1; i <= incom; i++) {
		do {
			do { /* IF debugging, put commanders by bases, always! */
#ifdef DEBUG
				if (idebug && klumper <= inbase) {
					ix = d.baseqx[klumper];
					iy = d.baseqy[klumper];
					klumper++;
				}
				else
#endif
					iran8(&ix, &iy);
			}
			while ((d.galaxy[ix][iy] < 99 && Rand() < 0.75)||
				   d.galaxy[ix][iy]>899);
			// check for duplicate
			for (j = 1; j < i; j++)
				if (d.cx[j]==ix && d.cy[j]==iy) break;
		} while (j < i);
		d.galaxy[ix][iy] += 100;
		d.cx[i] = ix;
		d.cy[i] = iy;
	}
	// Locate planets in galaxy
	for (i = 1; i <= inplan; i++) {
		do iran8(&ix, &iy);
		while (d.newstuf[ix][iy] > 0);
		d.newstuf[ix][iy] = 1;
		d.plnets[i].x = ix;
		d.plnets[i].y = iy;
		d.plnets[i].pclass = Rand()*3.0 + 1.0; // Planet class M N or O
		d.plnets[i].crystals = 1.5*Rand();		// 1 in 3 chance of crystals
		d.plnets[i].known = 0;
	}
	// Locate Romulans
	for (i = 1; i <= d.nromrem; i++) {
		iran8(&ix, &iy);
		d.newstuf[ix][iy] += 10;
	}
	// Locate the Super Commander
	if (d.nscrem > 0) {
		do iran8(&ix, &iy);
		while (d.galaxy[ix][iy] >= 900);
		d.isx = ix;
		d.isy = iy;
		d.galaxy[ix][iy] += 100;
	}
	// Place thing (in tournament game, thingx == -1, don't want one!)
	if (Rand() < 0.1 && thingx != -1) {
		iran8(&thingx, &thingy);
	}
	else {
		thingx = thingy = 0;
	}

//	idate = date;
	skip(3);
	d.snap = 0;
		
	if (skill == SNOVICE) {
		printf("It is stardate %d. The Federation is being attacked by\n",
			   (int)d.date);
		printf("a deadly Klingon invasion force. As captain of the United\n"
			   "Starship U.S.S. Enterprise, it is your mission to seek out\n"
			   "and destroy this invasion force of %d battle cruisers.\n",
			   inkling);
		printf("You have an initial allotment of %d stardates to complete\n"
			   "your mission.  As you proceed you may be given more time.\n\n"
			   "You will have %d supporting starbases.\n"
			   "Starbase locations-  ",
			   (int)intime, inbase);
	}
	else {
		printf("Stardate %d.\n\n"
			   "%d Klingons,\nan unknown number of Romulans\n",
			   (int)d.date, inkling);
		if (d.nscrem) printf("and one (GULP) Super-Commander.\n");
		printf("%d stardates\n%d starbases in  ",(int)intime, inbase);
	}
	for (i = 1; i <= inbase; i++) {
		cramlc(0, d.baseqx[i], d.baseqy[i]);
		if (i < inbase) proutn("  ");
	}
	skip(2);
	proutn("The Enterprise is currently in");
	cramlc(1, quadx, quady);
	proutn(" ");
	cramlc(2, sectx, secty);
	skip(2);
	prout("Good Luck!");
	if (d.nscrem) proutn("  YOU'LL NEED IT.");
	skip(1);
	newqad(0);
	if (nenhere) shldup=1.0;
	if (neutz) attack(0);	// bad luck to start in a Romulan Neutral Zone
}

int choose(void) {
	tourn = 0;
	thawed = 0;
	skill = 0;
	length = 0;
	while (TRUE) {
		if (fromcommandline) /* Can start with command line options */
			fromcommandline = 0;
		else
			proutn("Would you like a regular, tournament, or frozen game?");
		scan();
		if (strlen(citem)==0) continue; // Try again
		if (isit("tournament")) {
			while (scan() == IHEOL) {
				proutn("Type in tournament number-");
			}
			if (aaitem == 0) {
				chew();
				continue; // We don't want a blank entry
			}
			tourn = (int)aaitem;
			thingx = -1;
			srand((unsigned int)(int)aaitem);
			break;
		}
		if (isit("frozen")) {
			thaw();
			chew();
			if (*passwd==0) continue;
			randomize();
			Rand(); Rand(); Rand(); Rand();
			if (!alldone) thawed = 1; // No plaque if not finished
			report(1);
			return TRUE;
		}
		if (isit("regular")) {
			skip(2);
			randomize();
			Rand(); Rand(); Rand(); Rand();
			break;
		}
		proutn("What is \"");
		proutn(citem);
		prout("\"?");
		chew();
	}
	while (length==0 || skill==0) {
		if (scan() == IHALPHA) {
			if (isit("short")) length = 1;
			else if (isit("medium")) length = 2;
			else if (isit("long")) length = 4;
			else if (isit("novice")) skill = SNOVICE;
			else if (isit("fair")) skill = SFAIR;
			else if (isit("good")) skill = SGOOD;
			else if (isit("expert")) skill = SEXPERT;
			else if (isit("emeritus")) skill = SEMERITUS;
			else {
				proutn("What is \"");
				proutn(citem);
				prout("\"?");
			}
		}
		else {
			chew();
			if (length==0) proutn("Would you like a Short, Medium, or Long game? ");
			else if (skill == 0) proutn("Are you a Novice, Fair, Good, Expert, or Emeritus player?");
		}
	}
	while (TRUE) {
		scan();
		strcpy(passwd, citem);
		chew();
		if (*passwd != 0) break;
		proutn("Please type in a secret password (9 characters maximum)-");
	}
#ifdef DEBUG
	if (strcmp(passwd, "debug")==0) idebug = 1;
#endif

	// Use parameters to generate initial values of things
	damfac = 0.5 * skill;
	d.rembase = 3.0*Rand()+2.0;
	inbase = d.rembase;
	inplan = (PLNETMAX/2) + (PLNETMAX/2+1)*Rand();
	d.nromrem = (2.0+Rand())*skill;
	d.nscrem = (skill > SFAIR? 1 : 0);
	d.remtime = 7.0 * length;
	intime = d.remtime;
	d.remkl = 2.0*intime*((skill+1 - 2*Rand())*skill*0.1+.15); // d.remkl and inkling includes commanders and SC
	inkling = d.remkl;
	incom = skill + 0.0625*inkling*Rand();
	d.remcom= min(10, incom);
	incom = d.remcom;
	d.remres = (inkling+4*incom)*intime;
	inresor = d.remres;
	if (inkling > 50) {
		inbase = (d.rembase += 1);
    }
#ifdef CAPTURE
	brigcapacity = 400;
    brigfree = brigcapacity;
    kcaptured = 0; // TAA fix 6/2015
#endif
#ifdef CLOAKING
    ncviol = 0; // TAA fix 6/2015
    iscloaked = FALSE;
    iscloaking = FALSE;
#endif
	return FALSE;
}

void dropin(int iquad, int *ix, int *iy) {
	do iran10(ix, iy);
	while (quad[*ix][*iy] != IHDOT);
	quad[*ix][*iy] = iquad;
}

void newcnd(void) {
	condit = IHGREEN;
	if (energy < 1000.0) condit = IHYELLOW;
	if (d.galaxy[quadx][quady] > 99 || d.newstuf[quadx][quady] > 9)
		condit = IHRED;
}


void newqad(int shutup) {
	int quadnum = d.galaxy[quadx][quady];
	int newnum = d.newstuf[quadx][quady];
	int i, j, ix, iy, nplan;

	iattak = 1;
	justin = 1;
	basex = basey = 0;
	klhere = 0;
	comhere = 0;
	plnetx = plnety = 0;
	ishere = 0;
	irhere = 0;
	iplnet = 0;
	nenhere = 0;
	neutz = 0;
	inorbit = 0;
	landed = -1;
	ientesc = 0;
	ithere = 0;
    iseenit = 0;

#ifdef CLOAKING
    isviolreported = FALSE;
#endif
	if (iscate) {
		// Attempt to escape Super-commander, so tbeam back!
		iscate = 0;
		ientesc = 1;
	}
	// Clear quadrant
	for (i=1; i <= 10; i++)
		for (j=1; j <= 10; j++) quad[i][j] = IHDOT;
	// cope with supernova
	if (quadnum > 999) {
		return;
	}
	klhere = quadnum/100;
	irhere = newnum/10;
	nplan = newnum%10;
	nenhere = klhere + irhere;

	// Position Starship
	quad[sectx][secty] = ship;

	// Decide if quadrant needs a Tholian
	if ((skill <  SGOOD && Rand() <= 0.02) ||   /* Lighten up if skill is low */
		(skill == SGOOD && Rand() <= 0.05) ||
		(skill > SGOOD && Rand() <= 0.08)
#ifdef DEBUG
		|| strcmp(passwd, "tholianx")==0
#endif
		) {
		do {
			ithx = Rand() > 0.5 ? 10 : 1;
			ithy = Rand() > 0.5 ? 10 : 1;
		} while (quad[ithx][ithy] != IHDOT);
		quad[ithx][ithy] = IHT;
		ithere = 1;
		/* Reserve unocupied corners */
		if (quad[1][1]==IHDOT) quad[1][1] = 'X';
		if (quad[1][10]==IHDOT) quad[1][10] = 'X';
		if (quad[10][1]==IHDOT) quad[10][1] = 'X';
		if (quad[10][10]==IHDOT) quad[10][10] = 'X';
	}

	if (quadnum >= 100) {
		// Position ordinary Klingons
		quadnum -= 100*klhere;
		for (i = 1; i <= klhere; i++) {
			dropin(IHK, &ix, &iy);
			kx[i] = ix;
			ky[i] = iy;
			kdist[i] = kavgd[i] = sqrt(square(sectx-ix) + square(secty-iy));
			kpower[i] = Rand()*150.0 +300.0 +25.0*skill;
		}
		// If we need a commander, promote a Klingon
		for (i = 1; i <= d.remcom ; i++) 
			if (d.cx[i]==quadx && d.cy[i]==quady) break;
			
		if (i <= d.remcom) {
			quad[ix][iy] = IHC;
			kpower[klhere] = 950.0+400.0*Rand()+50.0*skill;
			comhere = 1;
		}

		// If we need a super-commander, promote a Klingon
		if (quadx == d.isx && quady == d.isy) {
			quad[kx[1]][ky[1]] = IHS;
			kpower[1] = 1175.0 + 400.0*Rand() + 125.0*skill;
			iscate = 1;
			ishere = 1;
		}
	}
	// Put in Romulans if needed
	for (i = klhere+1; i <= nenhere; i++) {
		dropin(IHR, &ix, &iy);
		kx[i] = ix;
		ky[i] = iy;
		kdist[i] = kavgd[i] = sqrt(square(sectx-ix) + square(secty-iy));
		kpower[i] = Rand()*400.0 + 450.0 + 50.0*skill;
	}
	sortkl();
	// If quadrant needs a starbase, put it in
	if (quadnum >= 10) {
		quadnum -= 10;
		dropin(IHB, &basex, &basey);
	}
	
	if (nplan) {
		// If quadrant needs a planet, put it in
		for (i=1; i <= inplan; i++)
			if (d.plnets[i].x == quadx && d.plnets[i].y == quady) break;
		if (i <= inplan) {
			iplnet = i;
			dropin(IHP, &plnetx, &plnety);
		}
	}
	// Check for condition
	newcnd();
	// And finally the stars
	for (i = 1; i <= quadnum; i++) dropin(IHSTAR, &ix, &iy);

	// Check for RNZ
	if (irhere > 0 && klhere == 0 && basex == 0) {
		neutz = 1;
		if (REPORTS) { 
			skip(1);
			prout("LT. UHURA- \"Captain, an urgent message.");
			prout("  I'll put it on audio.\"  CLICK");
			skip(1);
			prout("INTRUDER! YOU HAVE VIOLATED THE ROMULAN NEUTRAL ZONE.");
			prout("LEAVE AT ONCE, OR YOU WILL BE DESTROYED!");
		}
	}

	if (shutup==0) {
		// Put in THING if needed
		if (thingx == quadx && thingy == quady) {
			dropin(IHQUEST, &ix, &iy);
			thingx = thingy = 0; // Transient
			if (damage[DSRSENS] == 0.0) {
				skip(1);
				prout("MR. SPOCK- \"Captain, this is most unusual.");
				prout("    Please examine your short-range scan.\"");
			}
		}
	}

	// Put in a few black holes
	for (i = 1; i <= 3; i++)
		if (Rand() > 0.5) dropin(IHBLANK, &ix, &iy);

	// Take out X's in corners if Tholian present
	if (ithere) {
		if (quad[1][1]=='X') quad[1][1] = IHDOT;
		if (quad[1][10]=='X') quad[1][10] = IHDOT;
		if (quad[10][1]=='X') quad[10][1] = IHDOT;
		if (quad[10][10]=='X') quad[10][10] = IHDOT;
	}		
}

void sortkl(void) {
	double t;
	int sw, j, k;

	// The author liked bubble sort. So we will use it. :-(

	if (nenhere < 2) return;

	do {
		sw = FALSE;
		for (j = 1; j < nenhere; j++)
			if (kdist[j] > kdist[j+1]) {
				sw = TRUE;
				t = kdist[j];
				kdist[j] = kdist[j+1];
				kdist[j+1] = t;
				t = kavgd[j];
				kavgd[j] = kavgd[j+1];
				kavgd[j+1] = t;
				k = kx[j];
				kx[j] = kx[j+1];
				kx[j+1] = k;
				k = ky[j];
				ky[j] = ky[j+1];
				ky[j+1] = k;
				t = kpower[j];
				kpower[j] = kpower[j+1];
				kpower[j+1] = t;
			}
	} while (sw);
}

#include "sst.h"

static int tryexit(int lookx, int looky, int ienm, int loccom, int irun) {
	int iqx, iqy, l;

	iqx = quadx+(lookx+9)/10 - 1;
	iqy = quady+(looky+9)/10 - 1;
	if (iqx < 1 || iqx > 8 || iqy < 1 || iqy > 8 ||
		d.galaxy[iqx][iqy] > 899)
		return 0; /* no can do -- neg energy, supernovae, or >8 Klingons */
	if (ienm == IHR) return 0; /* Romulans cannot escape! */
	if (irun == 0) {
		/* avoid intruding on another commander's territory */
		if (ienm == IHC) {
			for (l = 1; l <= d.remcom; l++)
				if (d.cx[l]==iqx && d.cy[l]==iqy) return 0;
			/* refuse to leave if currently attacking starbase */
			if (batx==quadx && baty==quady) return 0;
		}
		/* don't leave if over 1000 units of energy */
		if (kpower[loccom] > 1000.) return 0;
	}
	/* print escape message and move out of quadrant.
	   We know this if either short or long range sensors are working */
	if (damage[DSRSENS] == 0.0 || damage[DLRSENS] == 0.0 ||
		condit == IHDOCKED) {
		proutn("***");
		cramen(ienm);
		proutn(" отступил в"); /* escapes to */
		cramlc(1, iqx, iqy);
		prout(" (и перезарядился)."); /*(and regains strength).*/
	}
	/* handle local matters related to escape */
	kx[loccom] = kx[nenhere];
	ky[loccom] = ky[nenhere];
	kavgd[loccom] = kavgd[nenhere];
	kpower[loccom] = kpower[nenhere];
	kdist[loccom] = kdist[nenhere];
	klhere--;
	nenhere--;
	if (condit != IHDOCKED) newcnd();
	/* Handle global matters related to escape */
	d.galaxy[quadx][quady] -= 100;
	d.galaxy[iqx][iqy] += 100;
	if (ienm==IHS) {
		ishere=0;
		iscate=0;
		ientesc=0;
		isatb=0;
		future[FSCMOVE]=0.2777+d.date;
		future[FSCDBAS]=1e30;
		d.isx=iqx;
		d.isy=iqy;
	}
	else {
		for (l=1; l<=d.remcom; l++) {
			if (d.cx[l]==quadx && d.cy[l]==quady) {
				d.cx[l]=iqx;
				d.cy[l]=iqy;
				break;
			}
		}
		comhere = 0;
	}
	return 1; /* success */
}


static void movebaddy(int comx, int comy, int loccom, int ienm) {
	int motion, mdist, nsteps, mx, my, nextx, nexty, lookx, looky, ll;
	int irun = 0;
	int krawlx, krawly;
	int success;
	int attempts;
	/* This should probably be just comhere + ishere */
	int nbaddys = skill > SGOOD ?
				  (int)((comhere*2 + ishere*2+klhere*1.23+irhere*1.5)/2.0):
				  (comhere + ishere);
	double dist1, forces;

	dist1 = kdist[loccom];
	mdist = dist1 + 0.5; /* Nearest integer distance */

	/* If SC, check with spy to see if should hi-tail it */
	if (ienm==IHS &&
		(kpower[loccom] <= 500.0 || (condit==IHDOCKED && damage[DPHOTON]==0))) {
		irun = 1;
		motion = -10;
	}
	else {
		/* decide whether to advance, retreat, or hold position */
/* Algorithm:
   * Enterprise has "force" based on condition of phaser and photon torpedoes.
     If both are operating full strength, force is 1000. If both are damaged,
	 force is -1000. Having shields down subtracts an additional 1000.

   * Enemy has forces equal to the energy of the attacker plus
     100*(K+R) + 500*(C+S) - 400 for novice through good levels OR
	 346*K + 400*R + 500*(C+S) - 400 for expert and emeritus.

	 Attacker Initial energy levels (nominal):
	          Klingon   Romulan   Commander   Super-Commander
	 Novice    400        700        1200        
	 Fair      425        750        1250
	 Good      450        800        1300        1750
	 Expert    475        850        1350        1875
	 Emeritus  500        900        1400        2000
     VARIANCE   75        200         200         200

	 Enemy vessels only move prior to their attack. In Novice - Good games
	 only commanders move. In Expert games, all enemy vessels move if there
	 is a commander present. In Emeritus games all enemy vessels move.

  *  If Enterprise is not docked, an agressive action is taken if enemy
     forces are 1000 greater than Enterprise.

	 Agressive action on average cuts the distance between the ship and
	 the enemy to 1/4 the original.

  *  At lower energy advantage, movement units are proportional to the
     advantage with a 650 advantage being to hold ground, 800 to move forward
	 1, 950 for two, 150 for back 4, etc. Variance of 100.

	 If docked, is reduced by roughly 1.75*skill, generally forcing a
	 retreat, especially at high skill levels.

  *  Motion is limited to skill level, except for SC hi-tailing it out.
  */

		forces = kpower[loccom]+100.0*nenhere+400*(nbaddys-1);
		if (shldup==0) forces += 1000; /* Good for enemy if shield is down! */
		if (damage[DPHASER] == 0.0 || damage[DPHOTON] == 0.0) {
			if (damage[DPHASER] != 0) /* phasers damaged */
				forces += 300.0;
			else
				forces -= 0.2*(energy - 2500.0);
			if (damage[DPHOTON] != 0) /* photon torpedoes damaged */
				forces += 300.0;
			else
				forces -= 50.0*torps;
		}
		else {
			/* phasers and photon tubes both out! */
			forces += 1000.0;
		}
		motion = 0;
		if (forces <= 1000.0 && condit != IHDOCKED) /* Typical situation */
			motion = ((forces+200.0*Rand())/150.0) - 5.0;
		else {
			if (forces > 1000.0) /* Very strong -- move in for kill */
				motion = (1.0-square(Rand()))*dist1 + 1.0;
			if (condit==IHDOCKED) /* protected by base -- back off ! */
				motion -= skill*(2.0-square(Rand()));
		}
#ifdef DEBUG
		if (idebug) {
			proutn("MOTION = ");
			cramf(motion, 1, 2);
            proutn("  FORCES = ");
			cramf(forces, 1, 2);
			skip(1);
		}
#endif
		/* don't move if no motion */
		if (motion==0) return;
		/* Limit motion according to skill */
		if (abs(motion) > skill) motion = (motion < 0) ? -skill : skill;
	}
	/* calcuate preferred number of steps */
	nsteps = motion < 0 ? -motion : motion;
	if (motion > 0 && nsteps > mdist) nsteps = mdist; /* don't overshoot */
	if (nsteps > 10) nsteps = 10; /* This shouldn't be necessary */
	if (nsteps < 1) nsteps = 1; /* This shouldn't be necessary */
#ifdef DEBUG
	if (idebug) {
		proutn("NSTEPS = ");
		crami(nsteps, 1);
		skip(1);
	}
#endif
	/* Compute preferred values of delta X and Y */
	mx = sectx - comx;
	my = secty - comy;
	if (2.0 * abs(mx) < abs(my)) mx = 0;
	if (2.0 * abs(my) < abs(sectx-comx)) my = 0;
	if (mx != 0) mx = mx*motion < 0 ? -1 : 1;
	if (my != 0) my = my*motion < 0 ? -1 : 1;
	nextx = comx;
	nexty = comy;
	quad[comx][comy] = IHDOT;
	/* main move loop */
	for (ll = 1; ll <= nsteps; ll++) {
#ifdef DEBUG
		if (idebug) {
			crami(ll,2);
			skip(1);
		}
#endif
		/* Check if preferred position available */
		lookx = nextx + mx;
		looky = nexty + my;
		krawlx = mx < 0 ? 1 : -1;
		krawly = my < 0 ? 1 : -1;
		success = 0;
		attempts = 0; /* Settle mysterious hang problem */
		while (attempts++ < 20 && !success) {
			if (lookx < 1 || lookx > 10) {
				if (motion < 0 && tryexit(lookx, looky, ienm, loccom, irun))
					return;
				if (krawlx == mx || my == 0) break;
				lookx = nextx + krawlx;
				krawlx = -krawlx;
			}
			else if (looky < 1 || looky > 10) {
				if (motion < 0 && tryexit(lookx, looky, ienm, loccom, irun))
					return;
				if (krawly == my || mx == 0) break;
				looky = nexty + krawly;
				krawly = -krawly;
			}
			else if (quad[lookx][looky] != IHDOT) {
				/* See if we should ram ship */
				if (quad[lookx][looky] == ship &&
					(ienm == IHC || ienm == IHS)) {
					ram(1, ienm, comx, comy);
					return;
				}
				if (krawlx != mx && my != 0) {
					lookx = nextx + krawlx;
					krawlx = -krawlx;
				}
				else if (krawly != my && mx != 0) {
					looky = nexty + krawly;
					krawly = -krawly;
				}
				else break; /* we have failed */
			}
			else success = 1;
		}
		if (success) {
			nextx = lookx;
			nexty = looky;
#ifdef DEBUG
			if (idebug) {
				cramlc(0, nextx, nexty);
				skip(1);
			}
#endif
		}
		else break; /* done early */
	}
	/* Put commander in place within same quadrant */
	quad[nextx][nexty] = ienm;
	if (nextx != comx || nexty != comy) {
		/* it moved */
		kx[loccom] = nextx;
		ky[loccom] = nexty;
		kdist[loccom] = kavgd[loccom] =
					sqrt(square(sectx-nextx)+square(secty-nexty));
		if (damage[DSRSENS] == 0 || condit == IHDOCKED) {
			proutn("***");
			cramen(ienm);
			if (kdist[loccom] < dist1) proutn(" наступает в"); /*advances to*/
			else proutn(" отступил в"); /*retreats to*/
			cramlc(2, nextx, nexty);
			skip(1);
		}
	}
}

void movcom(void) {
	int ix, iy, i;

#ifdef DEBUG
	if (idebug) prout("MOVCOM");
#endif

	/* Figure out which Klingon is the commander (or Supercommander)
	   and do move */
	if (comhere) for (i = 1; i <= nenhere; i++) {
		ix = kx[i];
		iy = ky[i];
		if (quad[ix][iy] == IHC) {
			movebaddy(ix, iy, i, IHC);
			break;
		}
	}
	if (ishere) for (i = 1; i <= nenhere; i++) {
		ix = kx[i];
		iy = ky[i];
		if (quad[ix][iy] == IHS) {
			movebaddy(ix, iy, i, IHS);
			break;
		}
	}
	/* if skill level is high, move other Klingons and Romulans too!
	   Move these last so they can base their actions on what the
       commander(s) do. */
	if (skill > SGOOD) for (i = 1; i <= nenhere; i++) {
		ix = kx[i];
		iy = ky[i];
		if (quad[ix][iy] == IHK || quad[ix][iy] == IHR)
			movebaddy(ix, iy, i, quad[ix][iy]);
	}

	sortkl();
}

static int checkdest(int iqx, int iqy, int flag, int *ipage) {
	int i, j;

	if ((iqx==quadx && iqy==quady) ||
		iqx < 1 || iqx > 8 || iqy < 1 || iqy > 8 ||
		d.galaxy[iqx][iqy] > 899) return 1;
	if (flag) {
		/* Avoid quadrants with bases if we want to avoid Enterprise */
		for (i = 1; i <= d.rembase; i++)
			if (d.baseqx[i]==iqx && d.baseqy[i]==iqy) return 1;
	}

	/* do the move */
	d.galaxy[d.isx][d.isy] -= 100;
	d.isx = iqx;
	d.isy = iqy;
	d.galaxy[d.isx][d.isy] += 100;
	if (iscate) {
		/* SC has scooted, Remove him from current quadrant */
		iscate=0;
		isatb=0;
		ishere=0;
		ientesc=0;
		future[FSCDBAS]=1e30;
		for (i = 1; i <= nenhere; i++) 
			if (quad[kx[i]][ky[i]] == IHS) break;
		quad[kx[i]][ky[i]] = IHDOT;
		kx[i] = kx[nenhere];
		ky[i] = ky[nenhere];
		kdist[i] = kdist[nenhere];
		kavgd[i] = kavgd[nenhere];
		kpower[i] = kpower[nenhere];
		klhere--;
		nenhere--;
		if (condit!=IHDOCKED) newcnd();
		sortkl();
	}
	/* check for a helpful planet */
	for (i = 1; i <= inplan; i++) {
		if (d.plnets[i].x==d.isx && d.plnets[i].y==d.isy &&
			d.plnets[i].crystals == 1) {
			/* destroy the planet */
			d.plnets[i] = nulplanet;
			d.newstuf[d.isx][d.isy] -= 1;
			if (REPORTS) {
				if (*ipage==0) pause(1);
				*ipage = 1;
/*				prout("Lt. Uhura-  \"Captain, Starfleet Intelligence reports");*/
				prout("Лейтенант Ухура-  \"Капитан, Разведка Звездного Флота докладывает");
				proutn("   планета в"); /*a planet in*/
				cramlc(1, d.isx, d.isy);
				prout(" была уничтожена"); /*has been destroyed*/
				prout("   Супер-Коммандером.\""); /*by the Super-commander.*/
			}
			break;
		}
	}
	return 0; /* looks good! */
}
			
		
	


void scom(int *ipage) {
	int i, i2, j, ideltax, ideltay, ibqx, ibqy, sx, sy, ifindit, iwhichb;
	int iqx, iqy;
	int basetbl[6];
	double bdist[6];
	int flag;
#ifdef DEBUG
	if (idebug) prout("SCOM");
#endif

	/* Decide on being active or passive */
	flag = ((d.killc+d.killk)/(d.date+0.01-indate) < 0.1*skill*(skill+1.0) ||
			(d.date-indate) < 3.0);
	if (iscate==0 && flag) {
		/* compute move away from Enterprise */
		ideltax = d.isx-quadx;
		ideltay = d.isy-quady;
		if (sqrt(ideltax*(double)ideltax+ideltay*(double)ideltay) > 2.0) {
			/* circulate in space */
			ideltax = d.isy-quady;
			ideltay = quadx-d.isx;
		}
	}
	else {
		/* compute distances to starbases */
		if (d.rembase <= 0) {
			/* nothing left to do */
			future[FSCMOVE] = 1e30;
			return;
		}
		sx = d.isx;
		sy = d.isy;
		for (i = 1; i <= d.rembase; i++) {
			basetbl[i] = i;
			ibqx = d.baseqx[i];
			ibqy = d.baseqy[i];
			bdist[i] = sqrt(square(ibqx-sx) + square(ibqy-sy));
		}
		if (d.rembase > 1) {
			/* sort into nearest first order */
			int iswitch;
			do {
				iswitch = 0;
				for (i=1; i < d.rembase-1; i++) {
					if (bdist[i] > bdist[i+1]) {
						int ti = basetbl[i];
						double t = bdist[i];
						bdist[i] = bdist[i+1];
						bdist[i+1] = t;
						basetbl[i] = basetbl[i+1];
						basetbl[i+1] =ti;
						iswitch = 1;
					}
				}
			} while (iswitch);
		}
		/* look for nearest base without a commander, no Enterprise, and
		   without too many Klingons, and not already under attack. */
		ifindit = iwhichb = 0;

		for (i2 = 1; i2 <= d.rembase; i2++) {
			i = basetbl[i2];	/* bug in original had it not finding nearest*/
			ibqx = d.baseqx[i];
			ibqy = d.baseqy[i];
			if ((ibqx == quadx && ibqy == quady) ||
				(ibqx == batx && ibqy == baty) ||
				d.galaxy[ibqx][ibqy] > 899) continue;
			/* if there is a commander, an no other base is appropriate,
			   we will take the one with the commander */
			for (j = 1; j <= d.remcom; j++) {
				if (ibqx==d.cx[j] && ibqy==d.cy[j] && ifindit!= 2) {
						ifindit = 2;
						iwhichb = i;
						break;
				}
			}
			if (j > d.remcom) { /* no commander -- use this one */
				ifindit = 1;
				iwhichb = i;
				break;
			}
		}
		if (ifindit==0) return; /* Nothing suitable -- wait until next time*/
		ibqx = d.baseqx[iwhichb];
		ibqy = d.baseqy[iwhichb];
		/* decide how to move toward base */
		ideltax = ibqx - d.isx;
		ideltay = ibqy - d.isy;
	}
	/* Maximum movement is 1 quadrant in either or both axis */
	if (ideltax > 1) ideltax = 1;
	if (ideltax < -1) ideltax = -1;
	if (ideltay > 1) ideltay = 1;
	if (ideltay < -1) ideltay = -1;

	/* try moving in both x and y directions */
	iqx = d.isx + ideltax;
	iqy = d.isy + ideltax;
	if (checkdest(iqx, iqy, flag, ipage)) {
		/* failed -- try some other maneuvers */
		if (ideltax==0 || ideltay==0) {
			/* attempt angle move */
			if (ideltax != 0) {
				iqy = d.isy + 1;
				if (checkdest(iqx, iqy, flag, ipage)) {
					iqy = d.isy - 1;
					checkdest(iqx, iqy, flag, ipage);
				}
			}
			else {
				iqx = d.isx + 1;
				if (checkdest(iqx, iqy, flag, ipage)) {
					iqx = d.isx - 1;
					checkdest(iqx, iqy, flag, ipage);
				}
			}
		}
		else {
			/* try moving just in x or y */
			iqy = d.isy;
			if (checkdest(iqx, iqy, flag, ipage)) {
				iqy = d.isy + ideltay;
				iqx = d.isx;
				checkdest(iqx, iqy, flag, ipage);
			}
		}
	}
	/* check for a base */
	if (d.rembase == 0) {
		future[FSCMOVE] = 1e30;
	}
	else for (i=1; i<=d.rembase; i++) {
		ibqx = d.baseqx[i];
		ibqy = d.baseqy[i];
		if (ibqx==d.isx && ibqy == d.isy && d.isx != batx && d.isy != baty) {
			/* attack the base */
			if (flag) return; /* no, don't attack base! */
			iseenit = 0;
			isatb=1;
			future[FSCDBAS] = d.date + 1.0 +2.0*Rand();
			if (batx != 0) future[FSCDBAS] += future[FCDBAS]-d.date;
			if (!REPORTS)
				return; /* no warning */
			iseenit = 1;
			if (*ipage == 0)  pause(1);
			*ipage=1;
/*			proutn("Lt. Uhura-  \"Captain, the starbase in");*/
			proutn("Лейт.Ухура-  \"Капитан, станция в");
			cramlc(1, d.isx, d.isy);
			skip(1);
/*			prout("   reports that it is under attack from the Klingon Super-commander.");*/
			proutn("   It can survive until stardate ");
			prout("   докладывает, что находится под атакой Супер-Коммандером Клингонов.");
			proutn("   Она продержится максимум до ");
			cramf(future[FSCDBAS], 0, 1);
			prout(" .\"");
			if (resting==0) return;
/*			prout("Mr. Spock-  \"Captain, shall we cancel the rest period?\"");*/
			prout("Мистер Спок-  \"Капитан, мы должны прекратить отдых?\"");
			if (ja()==0) return;
			resting = 0;
			Time = 0.0; /* actually finished */
			return;
		}
	}
	/* Check for intelligence report */
	if (
#ifdef DEBUG
		idebug==0 &&
#endif
		(Rand() > 0.2 ||
		 (!REPORTS) ||
		 starch[d.isx][d.isy] > 0))
		return;
	if (*ipage==0) pause(1);
	*ipage = 1;
/*	prout("Lt. Uhura-  \"Captain, Starfleet Intelligence reports");*/
	prout("Лейт.Ухура-  \"Капитан, Разведка Звездного Флота докладывает");
/*	proutn("   the Super-commander is in");*/
	proutn("   Супер-Коммандер находится в");
	cramlc(1, d.isx, d.isy);
	prout(".\"");
	return;
}

void movetho(void) {
	int idx, idy, im, i, dum, my;
	/* Move the Tholean */
	if (ithere==0 || justin == 1) return;

	if (ithx == 1 && ithy == 1) {
		idx = 1; idy = 10;
	}
	else if (ithx == 1 && ithy == 10) {
		idx = 10; idy = 10;
	}
	else if (ithx == 10 && ithy == 10) {
		idx = 10; idy = 1;
	}
	else if (ithx == 10 && ithy == 1) {
		idx = 1; idy = 1;
	}
	else {
		/* something is wrong! */
		ithere = 0;
		return;
	}

	/* Do nothing if we are blocked */
	if (quad[idx][idy]!= IHDOT && quad[idx][idy]!= IHWEB) return;
	quad[ithx][ithy] = IHWEB;

	if (ithx != idx) {
		/* move in x axis */
		im = fabs((double)idx - ithx)/((double)idx - ithx);
		while (ithx != idx) {
			ithx += im;
			if (quad[ithx][ithy]==IHDOT) quad[ithx][ithy] = IHWEB;
		}
	}
	else if (ithy != idy) {
		/* move in y axis */
		im = fabs((double)idy - ithy)/((double)idy - ithy);
		while (ithy != idy) {
			ithy += im;
			if (quad[ithx][ithy]==IHDOT) quad[ithx][ithy] = IHWEB;
		}
	}
	quad[ithx][ithy] = IHT;

	/* check to see if all holes plugged */
	for (i = 1; i < 11; i++) {
		if (quad[1][i]!=IHWEB && quad[1][i]!=IHT) return;
		if (quad[10][i]!=IHWEB && quad[10][i]!=IHT) return;
		if (quad[i][1]!=IHWEB && quad[i][1]!=IHT) return;
		if (quad[i][10]!=IHWEB && quad[i][10]!=IHT) return;
	}
	/* All plugged up -- Tholian splits */
	quad[ithx][ithy]=IHWEB;
	dropin(IHBLANK, &dum, &my);
	crmena(1,IHT, 2, ithx, ithy);
	prout(" сеть создана."); /*completes web*/
	ithere = ithx = ithy = 0;
	return;
}

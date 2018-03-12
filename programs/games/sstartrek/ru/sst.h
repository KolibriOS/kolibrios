#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#ifndef INCLUDED
#define EXTERN extern
#else
#define EXTERN
#endif

#ifdef WINDOWS
#define DEBUG
#define SCORE
#define CLOAKING
#define CAPTURE
#endif

#ifdef CLOAKING
#define ndevice (16)
#else
#define ndevice (15)	// Number of devices
#endif
#define phasefac (2.0)
#define PLNETMAX (10)
#define NEVENTS (8)

typedef struct {
	int x;	/* Quadrant location of planet */
	int y;
	int pclass; /* class M, N, or O (1, 2, or 3) */
	int crystals; /* has crystals */
	int known;   /* =1 contents known, =2 shuttle on this planet */
} PLANETS;

EXTERN struct foo {
		int snap,		// snapshot taken
		remkl,			// remaining klingons
	        remcom,			// remaining commanders
		rembase,		// remaining bases
		starkl,			// destroyed stars
		basekl,			// destroyed bases
		killk,			// Klingons killed
		killc,			// commanders killed
		galaxy[9][9], 	// The Galaxy (subscript 0 not used)
		cx[11],cy[11],	// Commander quadrant coordinates
		baseqx[6],		// Base quadrant X
		baseqy[6],		// Base quadrant Y
		newstuf[9][9],	// Extended galaxy goodies
		isx, isy,		// Coordinate of Super Commander
		nscrem,			// remaining super commanders
		nromkl,			// Romulans killed
		nromrem,		// Romulans remaining
		nsckill,		// super commanders killed
		nplankl;		// destroyed planets
	PLANETS plnets[PLNETMAX+1];  // Planet information
#ifdef CAPTURE
    int kcaptured, brigfree;
#endif
	double date,		// stardate
		remres,			// remaining resources
	    remtime;		// remaining time
} d, snapsht;			// Data that is snapshot

EXTERN char
		quad[11][11];	// contents of our quadrant

// Scalar variables that are needed for freezing the game
// are placed in a structure. #defines are used to access by their
// original names. Gee, I could have done this with the d structure,
// but I just didn't think of it back when I started.

EXTERN struct foo2 {
	int inkling,
	inbase,
	incom,
	instar,
	intorps,
	condit,
	torps,
	ship,
	quadx,
	quady,
	sectx,
	secty,
	length,
	skill,
	basex,
	basey,
	klhere,
	comhere,
	casual,
	nhelp,
	nkinks,
	ididit,
	gamewon,
	alive,
	justin,
	alldone,
	shldchg,
	thingx,
	thingy,
	plnetx,
	plnety,
	inorbit,
	landed,
	iplnet,
	imine,
	inplan,
	nenhere,
	ishere,
	neutz,
	irhere,
	icraft,
	ientesc,
	iscraft,
	isatb,
	iscate,
#ifdef DEBUG
	idebug,
#endif
#ifdef CLOAKING
    iscloaked,
    iscloaking,
    ncviol,
    isviolreported,
#endif
#ifdef CAPTURE
    brigcapacity,
#endif
	iattak,
	icrystl,
	tourn,
	thawed,
	batx,
	baty,
	ithere,
	ithx,
	ithy,
	iseenit,
	probecx,
	probecy,
	proben,
	isarmed,
	nprobes;

	double inresor,
	intime,
	inenrg,
	inshld,
	inlsr,
	indate,
	energy,
	shield,
	shldup,
	warpfac,
	wfacsq,
	lsupres,
	dist,
	direc,
	Time,
	docfac,
	resting,
	damfac,
	stdamtim,
	cryprob,
	probex,
	probey,
	probeinx,
	probeiny;
} a;

#define inkling a.inkling		// Initial number of klingons
#define inbase a.inbase			// Initial number of bases
#define incom a.incom			// Initian number of commanders
#define instar a.instar			// Initial stars
#define intorps a.intorps		// Initial/Max torpedoes
#define condit a.condit			// Condition (red, yellow, green docked)
#define torps a.torps			// number of torpedoes
#define ship a.ship				// Ship type -- 'E' is Enterprise
#define quadx a.quadx			// where we are
#define quady a.quady			//
#define sectx a.sectx			// where we are
#define secty a.secty			//
#define length a.length			// length of game
#define skill a.skill			// skill level
#define basex a.basex			// position of base in current quad
#define basey a.basey			//
#define klhere a.klhere			// klingons here
#define comhere a.comhere		// commanders here
#define casual a.casual			// causalties
#define nhelp a.nhelp			// calls for help
#define nkinks a.nkinks			//
#define ididit a.ididit			// Action taken -- allows enemy to attack
#define gamewon a.gamewon		// Finished!
#define alive a.alive			// We are alive (not killed)
#define justin a.justin			// just entered quadrant
#define alldone a.alldone		// game is now finished
#define shldchg a.shldchg		// shield is changing (affects efficiency)
#define thingx a.thingx			// location of strange object in galaxy
#define thingy a.thingy			//
#define plnetx a.plnetx			// location of planet in quadrant
#define plnety a.plnety			//
#define inorbit a.inorbit		// orbiting
#define landed a.landed			// party on planet (1), on ship (-1)
#define iplnet a.iplnet			// planet # in quadrant
#define imine a.imine			// mining
#define inplan a.inplan			// initial planets
#define nenhere a.nenhere		// Number of enemies in quadrant
#define ishere a.ishere			// Super-commander in quandrant
#define neutz a.neutz			// Romulan Neutral Zone
#define irhere a.irhere			// Romulans in quadrant
#define icraft a.icraft			// Kirk in Galileo
#define ientesc a.ientesc		// Attempted escape from supercommander
#define iscraft a.iscraft		// =1 if craft on ship, -1 if removed from game
#define isatb a.isatb			// =1 if SuperCommander is attacking base
#define iscate a.iscate			// Super Commander is here
#ifdef DEBUG
#define idebug a.idebug			// Debug mode
#endif
#ifdef CLOAKING
#define iscloaked a.iscloaked  // Cloaking is enabled
#define iscloaking a.iscloaking // However if iscloaking is TRUE then in process of cloaking and can be attacked
#define ncviol a.ncviol		// Treaty violations
#define isviolreported a.isviolreported // Violation reported by Romulan in quadrant
#endif
#ifdef CAPTURE
#define kcaptured d.kcaptured   // number of captured Klingons                  
#define brigfree d.brigfree     // room in the brig
#define brigcapacity a.brigcapacity        // How many Klingons the brig will hold
#endif
#define iattak a.iattak			// attack recursion elimination (was cracks[4])
#define icrystl a.icrystl		// dilithium crystals aboard
#define tourn a.tourn			// Tournament number
#define thawed a.thawed			// Thawed game
#define batx a.batx				// Base coordinates being attacked
#define baty a.baty				//
#define ithere a.ithere			// Tholean is here 
#define ithx a.ithx				// coordinates of tholean
#define ithy a.ithy
#define iseenit a.iseenit		// Seen base attack report
#define inresor a.inresor		// initial resources
#define intime a.intime			// initial time
#define inenrg a.inenrg			// Initial/Max Energy
#define inshld a.inshld			// Initial/Max Shield
#define inlsr a.inlsr			// initial life support resources
#define indate a.indate			// Initial date
#define energy a.energy			// Energy level
#define shield a.shield			// Shield level
#define shldup a.shldup			// Shields are up
#define warpfac a.warpfac		// Warp speed
#define wfacsq a.wfacsq			// squared warp factor
#define lsupres a.lsupres		// life support reserves
#define dist a.dist				// movement distance
#define direc a.direc			// movement direction
#define Time a.Time				// time taken by current operation
#define docfac a.docfac			// repair factor when docking (constant?)
#define resting a.resting		// rest time
#define damfac a.damfac			// damage factor
#define stdamtim a.stdamtim		// time that star chart was damaged
#define cryprob a.cryprob		// probability that crystal will work
#define probex a.probex			// location of probe
#define probey a.probey
#define probecx a.probecx		// current probe quadrant
#define probecy a.probecy	
#define probeinx a.probeinx		// Probe x,y increment
#define probeiny a.probeiny		
#define proben a.proben			// number of moves for probe
#define isarmed a.isarmed		// Probe is armed
#define nprobes a.nprobes		// number of probes available

EXTERN int
		kx[21],			// enemy sector locations
		ky[21],
		starch[9][9];	// star chart

EXTERN int fromcommandline; // Game start from command line options
EXTERN int coordfixed; // Fix those dumb coordinates. 

EXTERN char	passwd[10],		// Self Destruct password
		*device[ndevice+1];

EXTERN PLANETS nulplanet;	// zeroed planet structure

EXTERN double
		kpower[21],		// enemy energy levels
		kdist[21],		// enemy distances
		kavgd[21],		// average distances
		damage[ndevice+1],		// damage encountered
		future[NEVENTS+1];		// future events

EXTERN int iscore, iskill; // Common PLAQ
EXTERN double perdate;

typedef enum {FWON, FDEPLETE, FLIFESUP, FNRG, FBATTLE,
              FNEG3, FNOVA, FSNOVAED, FABANDN, FDILITHIUM,
			  FMATERIALIZE, FPHASER, FLOST, FMINING, FDPLANET,
			  FPNOVA, FSSC, FSTRACTOR, FDRAY, FTRIBBLE,
			  FHOLE
#ifdef CLOAKING
   , FCLOAK
#endif
} FINTYPE ;

/* Skill levels */
typedef enum {SNOVICE=1, SFAIR, SGOOD, SEXPERT, SEMERITUS} SKILLTYPE;

EXTERN double aaitem;
EXTERN char citem[24];


/* Define devices */
#define DSRSENS 1
#define DLRSENS 2
#define DPHASER 3
#define DPHOTON 4
#define DLIFSUP 5
#define DWARPEN 6
#define DIMPULS 7
#define DSHIELD 8
#define DRADIO  9
#define DSHUTTL 10
#define DCOMPTR 11
#define DTRANSP 12
#define DSHCTRL 13
#define DDRAY   14  // Added deathray
#define DDSP    15  // Added deep space probe
#define DCLOAK  16  // Added cloaking device

/* Define future events */
#define FSPY	0	// Spy event happens always (no future[] entry)
					// can cause SC to tractor beam Enterprise
#define FSNOVA  1   // Supernova
#define FTBEAM  2   // Commander tractor beams Enterprise
#define FSNAP   3   // Snapshot for time warp
#define FBATTAK 4   // Commander attacks base
#define FCDBAS  5   // Commander destroys base
#define FSCMOVE 6   // Supercommander moves (might attack base)
#define FSCDBAS 7   // Supercommander destroys base
#define FDSPROB 8   // Move deep space probe

#ifdef INCLUDED
PLANETS nulplanet = {0};
char *device[ndevice+1] = {
	"",
/* 	"S. R. Sensors",
	"L. R. Sensors",
	"Phasers",
	"Photon Tubes",
	"Life Support",
	"Warp Engines",
	"Impulse Engines",
	"Shields",
	"Subspace Radio",
	"Shuttle Craft",
	"Computer",
	"Transporter",
	"Shield Control",
	"Death Ray",
	"D. S. Probe" */	
	"Ближние сенсоры",
	"Дальние сенсоры",
	"Фазеры",
	"Фотонные торпеды",
	"Жизнеобеспечение",
	"Варп-двигатели",
	"Импульсные двиг.",
	"Силовые щиты",
	"Радиостанция",
	"Шаттл",
	"Компьютер",
	"Транспортер",
	"Регулятор щитов",
	"Луч Смерти",
	"Зонды глуб.скан"
#ifdef CLOAKING
	/* ,"Cloaking Device"  */
	,"Генератор невид." 
#endif
};									
#endif

#define ALGERON (2311) /* Date of the Treaty of Algeron */

#ifndef TRUE
#define TRUE (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif

#define IHR 'R'
#define IHK 'K'
#define IHC 'C'
#define IHS 'S'
#define IHSTAR '*'
#define IHP 'P'
#define IHB 'B'
#define IHBLANK ' '
#define IHDOT '.'
#define IHQUEST '?'
#define IHE 'E'
#define IHF 'F'
#define IHT 'T'
#define IHWEB '#'
#define IHGREEN 'G'
#define IHYELLOW 'Y'
#define IHRED 'R'
#define IHDOCKED 'D'


/* Function prototypes */
void prelim(void);
void attack(int);
int choose(void);
void setup(void);
void score(int);
void atover(int);
void srscan(int);
void lrscan(void);
void phasers(void);
void photon(void);
void warp(int);
void sheild(int);
void dock(void);
void dreprt(void);
void chart(int);
void impuls(void);
void waiting(void);
void setwrp(void);
void events(void);
void report(int);
void eta(void);
void help(void);
void abandn(void);
void finish(FINTYPE);
void dstrct(void);
void kaboom(void);
void freeze(int);
void thaw(void);
void plaque(void);
int scan(void);
#define IHEOL (0)
#define IHALPHA (1)
#define IHREAL (2)
void chew(void);
void chew2(void);
void skip(int);
void prout(char *s);
void proutn(char *s);
void stars(void);
void newqad(int);
int ja(void);
void cramen(int);
void crmshp(void);
void cramlc(int, int, int);
double expran(double);
double Rand(void);
void iran8(int *, int *);
void iran10(int *, int *);
double square(double);
void dropin(int, int*, int*);
void newcnd(void);
void sortkl(void);
void lmove(void);
void ram(int, int, int, int);
void crmena(int, int, int, int, int);
void deadkl(int, int, int, int, int);
void timwrp(void);
void movcom(void);
void torpedo(double, double, int, int, double *);
void cramf(double, int, int);
void crami(int, int);
void huh(void);
void pause(int);
void nova(int, int);
void snova(int, int);
void scom(int *);
void hittem(double *);
void prouts(char *);
int isit(char *);
void preport(void);
void orbit(void);
void sensor(void);
void beam(void);
void mine(void);
void usecrystals(void);
void shuttle(void);
void deathray(void);
void debugme(void);
void attakreport(void);
void movetho(void);
void probe(void);

#ifndef WINDOWS
#ifndef KOS32
int min(int, int);
int max(int, int);
#endif
#endif
void randomize(void);
///int getch(void);

#ifdef CLOAKING
void cloak(void);
#endif
#ifdef CAPTURE
void capture(void);
#endif

#ifdef CLOAKING
#define REPORTS ((condit==IHDOCKED || damage[DRADIO]<=0.0) && !iscloaked)
#else
#define REPORTS (condit==IHDOCKED || damage[DRADIO]<=0.0)
#endif

#include "defs.h"

#ifndef NULL
#define NULL	((void *) 0)
#endif

int nroff = 1;

#define NROFF (-666)
#define TROFF (-667)

STRDEF *chardef, *strdef, *defdef;
INTDEF *intdef;

static INTDEF standardint[] = {
    { V('n',' '), NROFF, 0, NULL },
    { V('t',' '), TROFF, 0, NULL },
    { V('o',' '), 1,     0, NULL },
    { V('e',' '), 0,     0, NULL },
    { V('.','l'), 70,    0, NULL },
    { V('.','$'), 0,     0, NULL },
    { V('.','A'), NROFF, 0, NULL },
    { V('.','T'), TROFF, 0, NULL },
    { V('.','V'), 1,     0, NULL }, /* the me package tests for this */
    { 0, 0, 0, NULL } };

static STRDEF standardstring[] = {
    { V('R',' '), 1, "&#174;", NULL },
    { V('l','q'), 2, "``", NULL },
    { V('r','q'), 2, "''", NULL },
    { 0, 0, NULL, NULL}
};


static STRDEF standardchar[] = {
    { V('*','*'), 1, "*", NULL  },	/* math star */
    { V('*','A'), 1, "A", NULL  },
    { V('*','B'), 1, "B", NULL  },
    { V('*','C'), 2, "Xi", NULL  },
    { V('*','D'), 5, "Delta", NULL  },
    { V('*','E'), 1, "E", NULL  },
    { V('*','F'), 3, "Phi", NULL  },
    { V('*','G'), 5, "Gamma", NULL  },
    { V('*','H'), 5, "Theta", NULL  },
    { V('*','I'), 1, "I", NULL  },
    { V('*','K'), 1, "K", NULL  },
    { V('*','L'), 6, "Lambda", NULL  },
    { V('*','M'), 1, "M", NULL  },
    { V('*','N'), 1, "N", NULL  },
    { V('*','O'), 1, "O", NULL  },
    { V('*','P'), 2, "Pi", NULL  },
    { V('*','Q'), 3, "Psi", NULL  },
    { V('*','R'), 1, "P", NULL  },
    { V('*','S'), 5, "Sigma", NULL  },
    { V('*','T'), 1, "T", NULL  },
    { V('*','U'), 1, "Y", NULL  },
    { V('*','W'), 5, "Omega", NULL  },
    { V('*','X'), 1, "X", NULL  },
    { V('*','Y'), 1, "H", NULL  },
    { V('*','Z'), 1, "Z", NULL  },
    { V('*','a'), 5, "alpha", NULL },
    { V('*','b'), 4, "beta", NULL },
    { V('*','c'), 2, "xi", NULL },
    { V('*','d'), 5, "delta", NULL },
    { V('*','e'), 7, "epsilon", NULL },
    { V('*','f'), 3, "phi", NULL },
    { V('*','g'), 5, "gamma", NULL },
    { V('*','h'), 5, "theta", NULL },
    { V('*','i'), 4, "iota", NULL },
    { V('*','k'), 5, "kappa", NULL },
    { V('*','l'), 6, "lambda", NULL },
    { V('*','m'), 1, "&#181;", NULL  },
    { V('*','n'), 2, "nu", NULL },
    { V('*','o'), 1, "o", NULL },
    { V('*','p'), 2, "pi", NULL },
    { V('*','q'), 3, "psi", NULL },
    { V('*','r'), 3, "rho", NULL },
    { V('*','s'), 5, "sigma", NULL },
    { V('*','t'), 3, "tau", NULL },
    { V('*','u'), 7, "upsilon", NULL },
    { V('*','w'), 5, "omega", NULL },
    { V('*','x'), 3, "chi", NULL },
    { V('*','y'), 3, "eta", NULL },
    { V('*','z'), 4, "zeta", NULL },
    { V('+','-'), 1, "&#177;", NULL  },
    { V('1','2'), 1, "&#189;", NULL  },
    { V('1','4'), 1, "&#188;", NULL  },
    { V('3','4'), 1, "&#190;", NULL  },
    { V('F','i'), 3, "ffi", NULL  },
    { V('F','l'), 3, "ffl", NULL  },
    { V('a','a'), 1, "&#180;", NULL  },
    { V('a','p'), 1, "~", NULL  },
    { V('b','r'), 1, "|", NULL  },
    { V('b','u'), 1, "*", NULL  }, 	/* bullet */
    { V('b','v'), 1, "|", NULL  },
    { V('c','i'), 1, "o", NULL  }, 	/* circle */
    { V('c','o'), 1, "&#169;", NULL  },
    { V('c','t'), 1, "&#162;", NULL  },
    { V('d','e'), 1, "&#176;", NULL  },
    { V('d','g'), 1, "+", NULL  }, 	/* dagger */
    { V('d','i'), 1, "&#247;", NULL  },
    { V('e','m'), 3, "---", NULL  }, 	/* em dash */
    { V('e','n'), 1, "-", NULL }, 	/* en dash */
    { V('e','q'), 1, "=", NULL  },
    { V('e','s'), 1, "&#216;", NULL  },
    { V('f','f'), 2, "ff", NULL  },
    { V('f','i'), 2, "fi", NULL  },
    { V('f','l'), 2, "fl", NULL  },
    { V('f','m'), 1, "&#180;", NULL  },
    { V('g','a'), 1, "`", NULL  },
    { V('h','y'), 1, "-", NULL  },
    { V('l','c'), 2, "|&#175;", NULL  },
    { V('i','f'), 8, "Infinity", NULL }, /* infinity sign */
    { V('i','s'), 8, "Integral", NULL }, /* integral sign */
    { V('l','f'), 2, "|_", NULL  },
    { V('l','k'), 1, "<FONT SIZE=\"+2\">{</FONT>", NULL  },
    { V('m','i'), 1, "-", NULL  },
    { V('m','u'), 1, "&#215;", NULL  },
    { V('n','o'), 1, "&#172;", NULL  },
    { V('o','r'), 1, "|", NULL  },
    { V('p','d'), 1, "d", NULL }, 	/* partial derivative */
    { V('p','l'), 1, "+", NULL  },
    { V('r','c'), 2, "&#175;|", NULL  },
    { V('r','f'), 2, "_|", NULL  },
    { V('r','g'), 1, "&#174;", NULL  },
    { V('r','k'), 1, "<FONT SIZE=\"+2\">}</FONT>", NULL  },
    { V('r','n'), 1, "&#175;", NULL  },
    { V('r','u'), 1, "_", NULL  },
    { V('s','c'), 1, "&#167;", NULL  },
    { V('s','l'), 1, "/", NULL  },
    { V('s','q'), 2, "[]", NULL  },
    { V('t','s'), 1, "s", NULL }, 	/* should be terminal sigma */
    { V('u','l'), 1, "_", NULL  },
    { V('>','='), 1, "&gt;", NULL },
    { V('<','='), 1, "&lt;", NULL },
    { 0, 0, NULL, NULL  }
};

void stdinit(void) {
    STRDEF *stdf;
    int i;

    stdf = &standardchar[0];
    i = 0;
    while (stdf->nr) {
	if (stdf->st) stdf->st = xstrdup(stdf->st);
	stdf->next = &standardchar[i];
	stdf = stdf->next;
	i++;
    }
    chardef=&standardchar[0];

    stdf=&standardstring[0];
    i=0;
    while (stdf->nr) {
	 /* waste a little memory, and make a copy, to avoid
	    the segfault when we free non-malloced memory */
	if (stdf->st) stdf->st = xstrdup(stdf->st);
	stdf->next = &standardstring[i];
	stdf = stdf->next;
	i++;
    }
    strdef=&standardstring[0];

    intdef=&standardint[0];
    i=0;
    while (intdef->nr) {
	if (intdef->nr == NROFF) intdef->nr = nroff; else
	if (intdef->nr == TROFF) intdef->nr = !nroff;
	intdef->next = &standardint[i];
	intdef = intdef->next;
	i++;
    }
    intdef = &standardint[0];
    defdef = NULL;
}

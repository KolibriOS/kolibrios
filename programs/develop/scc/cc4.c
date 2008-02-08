/*
** Small-C Compiler -- Part 4 -- Back End.
** Copyright 1982, 1983, 1985, 1988 J. E. Hendrix
** Copyright 1998 H T Walheim
** All rights reserved.
*/
#include <stdio.h>
#include "cc.h"

/* #define DISOPT */       /* display optimizations values */

/*************************** externals ****************************/

extern char
  *cptr, *macn, *litq, *symtab, optimize, ssname[NAMESIZE];

extern int
  *stage, litlab, litptr, csp, output, oldseg, usexpr,
  *snext, *stail, *slast;


/***************** optimizer command definitions ******************/

             /*     --      p-codes must not overlap these */
#define any     0x00FF   /* matches any p-code */
#define _pop    0x00FE   /* matches if corresponding POP2 exists */
#define pfree   0x00FD   /* matches if pri register free */
#define sfree   0x00FC   /* matches if sec register free */
#define comm    0x00FB   /* matches if registers are commutative */

             /*     --      these digits are reserved for n */
#define go      0x0100   /* go n entries */
#define gc      0x0200   /* get code from n entries away */
#define gv      0x0300   /* get value from n entries away */
#define sum     0x0400   /* add value from nth entry away */
#define neg     0x0500   /* negate the value */
#define ife     0x0600   /* if value == n do commands to next 0 */
#define ifl     0x0700   /* if value <  n do commands to next 0 */
#define swv     0x0800   /* swap value with value n entries away */
#define topop   0x0900   /* moves |code and current value to POP2 */

#define p1      0x0001   /* plus 1 */
#define p2      0x0002   /* plus 2 */
#define p3      0x0003   /* plus 3 */
#define p4      0x0004   /* plus 4 */
#define m1      0x00FF   /* minus 1 */
#define m2      0x00FE   /* minus 2 */
#define m3      0x00FD   /* minus 3 */
#define m4      0x00FC   /* minus 4 */

#define PRI      0030    /* primary register bits */
#define SEC      0003    /* secondary register bits */
#define USES     0011    /* use register contents */
#define ZAPS     0022    /* zap register contents */
#define PUSHES   0100    /* pushes onto the stack */
#define COMMUTES 0200    /* commutative p-code */

/******************** optimizer command lists *********************/

int
  seq00[] = {0,ADD12,MOVE21,0,                       /* ADD21 */
             go|p1,ADD21,0},

  seq01[] = {0,ADD1n,0,                              /* rINC1 or rDEC1 ? */ 
             ifl|m2,0,ifl|0,rDEC1,neg,0,ifl|p3,rINC1,0,0},

  seq02[] = {0,ADD2n,0,                              /* rINC2 or rDEC2 ? */ 
             ifl|m2,0,ifl|0,rDEC2,neg,0,ifl|p3,rINC2,0,0},

  seq03[] = {0,rDEC1,PUTbp1,rINC1,0,                 /* SUBbpn or DECbp */
             go|p2,ife|p1,DECbp,0,SUBbpn,0},

  seq04[] = {0,rDEC1,PUTwp1,rINC1,0,                 /* SUBwpn or DECwp */
             go|p2,ife|p1,DECwp,0,SUBwpn,0},

  seq05[] = {0,rDEC1,PUTbm1,rINC1,0,                 /* SUB_m_ COMMAn */
             go|p1,SUB_m_,go|p1,COMMAn,go|m1,0},

  seq06[] = {0,rDEC1,PUTwm1,rINC1,0,                 /* SUB_m_ COMMAn */
             go|p1,SUB_m_,go|p1,COMMAn,go|m1,0},

  seq07[] = {0,GETw1m,GETw2n,ADD12,MOVE21,GETb1p,0,  /* GETw2m GETb1p */
             go|p4,gv|m3,go|m1,GETw2m,gv|m3,0},

  seq08[] = {0,GETw1m,GETw2n,ADD12,MOVE21,GETb1pu,0, /* GETw2m GETb1pu */
             go|p4,gv|m3,go|m1,GETw2m,gv|m3,0},

  seq09[] = {0,GETw1m,GETw2n,ADD12,MOVE21,GETw1p,0,  /* GETw2m GETw1p */
             go|p4,gv|m3,go|m1,GETw2m,gv|m3,0},

  seq10[] = {0,GETw1m,GETw2m,SWAP12,0,               /* GETw2m GETw1m */
             go|p2,GETw1m,gv|m1,go|m1,gv|m1,0},

  seq11[] = {0,GETw1m,MOVE21,0,                      /* GETw2m */
             go|p1,GETw2m,gv|m1,0},

  seq12[] = {0,GETw1m,PUSH1,pfree,0,                 /* PUSHm */
             go|p1,PUSHm,gv|m1,0},

  seq13[] = {0,GETw1n,PUTbm1,pfree,0,                /* PUT_m_ COMMAn */
             PUT_m_,go|p1,COMMAn,go|m1,swv|p1,0},

  seq14[] = {0,GETw1n,PUTwm1,pfree,0,                /* PUT_m_ COMMAn */ 
             PUT_m_,go|p1,COMMAn,go|m1,swv|p1,0}, 

  seq15[] = {0,GETw1p,PUSH1,pfree,0,                 /* PUSHp */ 
             go|p1,PUSHp,gv|m1,0},

  seq16[] = {0,GETw1s,GETw2n,ADD12,MOVE21,0,         /* GETw2s ADD2n */
             go|p3,ADD2n,gv|m2,go|m1,GETw2s,gv|m2,0},

  seq17[] = {0,GETw1s,GETw2s,SWAP12,0,               /* GETw2s GETw1s */
             go|p2,GETw1s,gv|m1,go|m1,GETw2s,gv|m1,0},

  seq18[] = {0,GETw1s,MOVE21,0,                      /* GETw2s */
             go|p1,GETw2s,gv|m1,0},

  seq19[] = {0,GETw2m,GETw1n,SWAP12,SUB12,0,         /* GETw1m SUB1n */
             go|p3,SUB1n,gv|m2,go|m1,GETw1m,gv|m2,0},

  seq20[] = {0,GETw2n,ADD12,0,                       /* ADD1n */ 
             go|p1,ADD1n,gv|m1,0},

  seq21[] = {0,GETw2s,GETw1n,SWAP12,SUB12,0,         /* GETw1s SUB1n */ 
             go|p3,SUB1n,gv|m2,go|m1,GETw1s,gv|m2,0},

  seq22[] = {0,rINC1,PUTbm1,rDEC1,0,                 /* ADDm_ COMMAn */ 
             go|p1,ADDm_,go|p1,COMMAn,go|m1,0}, 

  seq23[] = {0,rINC1,PUTwm1,rDEC1,0,                 /* ADDm_ COMMAn */ 
             go|p1,ADDm_,go|p1,COMMAn,go|m1,0}, 

  seq24[] = {0,rINC1,PUTbp1,rDEC1,0,                 /* ADDbpn or INCbp */ 
             go|p2,ife|p1,INCbp,0,ADDbpn,0}, 

  seq25[] = {0,rINC1,PUTwp1,rDEC1,0,                 /* ADDwpn or INCwp */ 
             go|p2,ife|p1,INCwp,0,ADDwpn,0}, 

  seq26[] = {0,MOVE21,GETw1n,SWAP12,SUB12,0,         /* SUB1n */
             go|p3,SUB1n,gv|m2,0},

  seq27[] = {0,MOVE21,GETw1n,comm,0,                 /* GETw2n comm */
             go|p1,GETw2n,0},

  seq28[] = {0,POINT1m,GETw2n,ADD12,MOVE21,0,        /* POINT2m_ PLUSn */
             go|p3,PLUSn,gv|m2,go|m1,POINT2m_,gv|m2,0},

  seq29[] = {0,POINT1m,MOVE21,pfree,0,               /* POINT2m */
             go|p1,POINT2m,gv|m1,0},

  seq30[] = {0,POINT1m,PUSH1,pfree,_pop,0,           /* ... POINT2m */
             topop|POINT2m,go|p2,0},

  seq31[] = {0,POINT1s,GETw2n,ADD12,MOVE21,0,        /* POINT2s */
             sum|p1,go|p3,POINT2s,gv|m3,0},

  seq32[] = {0,POINT1s,PUSH1,MOVE21,0,               /* POINT2s PUSH2 */
             go|p1,POINT2s,gv|m1,go|p1,PUSH2,go|m1,0},

  seq33[] = {0,POINT1s,PUSH1,pfree,_pop,0,           /* ... POINT2s */
             topop|POINT2s,go|p2,0},

  seq34[] = {0,POINT1s,MOVE21,0,                     /* POINT2s */
             go|p1,POINT2s,gv|m1,0},

  seq35[] = {0,POINT2m,GETb1p,sfree,0,               /* GETb1m */
             go|p1,GETb1m,gv|m1,0},

  seq36[] = {0,POINT2m,GETb1pu,sfree,0,              /* GETb1mu */
             go|p1,GETb1mu,gv|m1,0},

  seq37[] = {0,POINT2m,GETw1p,sfree,0,               /* GETw1m */
             go|p1,GETw1m,gv|m1,0},

  seq38[] = {0,POINT2m_,PLUSn,GETw1p,sfree,0,        /* GETw1m_ PLUSn */
             go|p2,gc|m1,gv|m1,go|m1,GETw1m_,gv|m1,0},

  seq39[] = {0,POINT2s,GETb1p,sfree,0,               /* GETb1s */
             sum|p1,go|p1,GETb1s,gv|m1,0},

  seq40[] = {0,POINT2s,GETb1pu,sfree,0,              /* GETb1su */
             sum|p1,go|p1,GETb1su,gv|m1,0},

  seq41[] = {0,POINT2s,GETw1p,PUSH1,pfree,0,         /* PUSHs */
             sum|p1,go|p2,PUSHs,gv|m2,0},

  seq42[] = {0,POINT2s,GETw1p,sfree,0,               /* GETw1s */
             sum|p1,go|p1,GETw1s,gv|m1,0},

  seq43[] = {0,PUSH1,any,POP2,0,                     /* MOVE21 any */
             go|p2,gc|m1,gv|m1,go|m1,MOVE21,0},

  seq44[] = {0,PUSHm,_pop,0,                         /* ... GETw2m */
             topop|GETw2m,go|p1,0},

  seq45[] = {0,PUSHp,any,POP2,0,                     /* GETw2p ... */
             go|p2,gc|m1,gv|m1,go|m1,GETw2p,gv|m1,0},

  seq46[] = {0,PUSHs,_pop,0,                         /* ... GETw2s */
             topop|GETw2s,go|p1,0},

  seq47[] = {0,SUB1n,0,                              /* rDEC1 or rINC1 ? */
             ifl|m2,0,ifl|0,rINC1,neg,0,ifl|p3,rDEC1,0,0};

#define HIGH_SEQ  47
int seq[HIGH_SEQ + 1];  
setseq() {
  seq[ 0] = seq00;  seq[ 1] = seq01;  seq[ 2] = seq02;  seq[ 3] = seq03;
  seq[ 4] = seq04;  seq[ 5] = seq05;  seq[ 6] = seq06;  seq[ 7] = seq07;
  seq[ 8] = seq08;  seq[ 9] = seq09;  seq[10] = seq10;  seq[11] = seq11;
  seq[12] = seq12;  seq[13] = seq13;  seq[14] = seq14;  seq[15] = seq15;
  seq[16] = seq16;  seq[17] = seq17;  seq[18] = seq18;  seq[19] = seq19;
  seq[20] = seq20;  seq[21] = seq21;  seq[22] = seq22;  seq[23] = seq23;
  seq[24] = seq24;  seq[25] = seq25;  seq[26] = seq26;  seq[27] = seq27;
  seq[28] = seq28;  seq[29] = seq29;  seq[30] = seq30;  seq[31] = seq31;
  seq[32] = seq32;  seq[33] = seq33;  seq[34] = seq34;  seq[35] = seq35;
  seq[36] = seq36;  seq[37] = seq37;  seq[38] = seq38;  seq[39] = seq39;
  seq[40] = seq40;  seq[41] = seq41;  seq[42] = seq42;  seq[43] = seq43;
  seq[44] = seq44;  seq[45] = seq45;  seq[46] = seq46;  seq[47] = seq47;
  } 

/***************** assembly-code strings ******************/

int code[PCODES];

/*
** First byte contains flag bits indicating:
**    the value in ax is needed (010) or zapped (020)
**    the value in bx is needed (001) or zapped (002)
*/
setcodes() {
  setseq();
  code[ADD12]   = "\211ADD EAX,EBX\15\n";
  code[ADD1n]   = "\010?ADD EAX,<n>\15\n??";
  code[ADD21]   = "\211ADD EBX,EAX\15\n";
  code[ADD2n]   = "\010?ADD EBX,<n>\15\n??";
  code[ADDbpn]  = "\001ADD BYTE [EBX],<n>\15\n";
  code[ADDwpn]  = "\001ADD WORD [EBX],<n>\15\n";
  code[ADDm_]   = "\000ADD <m>";
  code[ADDSP]   = "\000?ADD ESP,<n>\15\n??";
  code[AND12]   = "\211AND EAX,EBX\15\n";
  code[ANEG1]   = "\010NEG EAX\15\n";
  code[ARGCNTn] = "\000?MOV CL,<n>?XOR CL,CL?\15\n";
  code[ASL12]   = "\011MOV ECX,EAX\15\nMOV EAX,EBX\15\nSAL EAX,CL\15\n";
  code[ASR12]   = "\011MOV ECX,EAX\15\nMOV EAX,EBX\15\nSAR EAX,CL\15\n";
  code[CALL1]   = "\010CALL EAX\15\n";
  code[CALLm]   = "\020CALL <m>\15\n";
  code[BYTE_]   = "\000 DB ";
  code[BYTEn]   = "\000 RESB <n>\15\n";
  code[BYTEr0]  = "\000 TIMES <n> DB 0\15\n";
  code[COM1]    = "\010NOT EAX\15\n";
  code[COMMAn]  = "\000,<n>\15\n";
  code[DBL1]    = "\010SHL EAX,1\15\n";
  code[DBL2]    = "\001SHL EBX,1\15\n";
  code[DECbp]   = "\001DEC BYTE [EBX]\15\n";
  code[DECwp]   = "\001DEC WORD [EBX]\15\n";
  code[DIV12]   = "\011CDQ\15\nIDIV EBX\15\n";                 /* see gen() */
  code[DIV12u]  = "\011XOR EDX,EDX\15\nDIV EBX\15\n";            /* see gen() */
  
  code[DWORD_]   = "\000 DD ";
  code[DWORDn]   = "\000 DD <n>\15\n";
  code[DWORDr0]  = "\000 TIMES <n> DD 0\15\n";
  
  code[ENTER]   = "\100PUSH EBP\15\nMOV EBP,ESP\15\n";
  code[EQ10f]   = "\010<g>OR EAX,EAX\15\nJE _<d>\15\nJMP _<n>\15\n_<d>:\15\n";
  code[EQ12]    = "\211CALL __eq\15\n";
  code[GE10f]   = "\010<g>OR EAX,EAX\15\nJGE _<d>\15\nJMP _<n>\15\n_<d>:\15\n";
  code[GE12]    = "\011CALL __ge\15\n";
  code[GE12u]   = "\011CALL __uge\15\n";
  code[GETb1m]  = "\020MOVSX EAX,BYTE [<m>]\15\n";
  code[GETb1mu] = "\020MOVZX EAX,BYTE [<m>]\15\n";
  code[GETb1p]  = "\021MOVSX EAX,BYTE [EBX?<o>??]\15\n";       /* see gen() */
  code[GETb1pu] = "\021MOVZX EAX,BYTE [EBX?<o>??]\15\n"; /* see gen() */
  code[GETb1s]  = "\020MOVSX EAX,BYTE [EBP<o>]\15\n";
  code[GETb1su] = "\020MOVZX EAX,BYTE [EBP<o>]\15\n";

  code[GETd1m]  = "\020MOV EAX,[<m>]\15\n";
  code[GETd1n]  = "\020?MOV EAX,<n>?XOR EAX,EAX?\15\n";
  code[GETd1p]  = "\021MOV EAX, [EBX?<o>??]\15\n";            /* see gen() */
  code[GETd2n]  = "\002?MOV EBX,<n>?XOR EBX,EBX?\15\n";
  
  code[GETw1m]  = "\020MOVSX EAX,WORD [<m>]\15\n";
  code[GETw1m_] = "\020MOVSX EAX,WORD [<m>]";
  code[GETw1n]  = "\020?MOV  EAX,<n>?XOR EAX,EAX?\15\n";
  code[GETw1p]  = "\021MOVSX EAX, WORD [EBX?<o>??]\15\n";            /* see gen() */
  code[GETw1s]  = "\020MOVSX EAX, WORD [EBP<o>]\15\n";
  code[GETw2m]  = "\002MOVSX EBX,WORD <m>\15\n";
  code[GETw2n]  = "\002?MOV EBX,<n>?XOR EBX,EBX?\15\n";
  code[GETw2p]  = "\021MOVSX EBX,WORD [EBX?<o>??]\15\n";
  code[GETw2s]  = "\002MOVSX EBX,WORD [EBP<o>]\15\n";
  code[GT10f]   = "\010<g>OR EAX,EAX\15\nJG _<d>\15\nJMP _<n>\15\n_<d>:\15\n";
  code[GT12]    = "\010CALL __gt\15\n";
  code[GT12u]   = "\011CALL __ugt\15\n";
  code[INCbp]   = "\001INC BYTE [EBX]\15\n";
  code[INCwp]   = "\001INC WORD [EBX]\15\n";
  code[WORD_]   = "\000 DW ";
  code[WORDn]   = "\000 RESW <n>\15\n";
  code[WORDr0]  = "\000 TIMES <n> DW 0\15\n";
  code[JMPm]    = "\000JMP _<n>\15\n";
  code[LABm]    = "\000_<n>:\15\n";
  code[LE10f]   = "\010<g>OR EAX,EAX\15\nJLE _<d>\15\nJMP _<n>\15\n_<d>:\15\n";
  code[LE12]    = "\011CALL __le\15\n";
  code[LE12u]   = "\011CALL __ule\15\n";
  code[LNEG1]   = "\010CALL __lneg\15\n";
  code[LT10f]   = "\010<g>OR EAX,EAX\15\nJL _<d>\15\nJMP _<n>\15\n_<d>:\15\n";
  code[LT12]    = "\011CALL __lt\15\n";
  code[LT12u]   = "\011CALL __ult\15\n";
  code[MOD12]   = "\011CDQ\15\nIDIV EBX\15\nMOV EAX,EDX\15\n";      /* see gen() */
  code[MOD12u]  = "\011XOR EDX,EDX\15\nDIV EBX\15\nMOV EAX,EDX\15\n"; /* see gen() */
  code[MOVE21]  = "\012MOV EBX,EAX\15\n";
  code[MUL12]   = "\211IMUL EBX\15\n";
  code[MUL12u]  = "\211MUL EBX\15\n";
  code[NE10f]   = "\010<g>OR EAX,EAX\15\nJNE _<d>\15\nJMP _<n>\15\n_<d>:\15\n";
  code[NE12]    = "\211CALL __ne\15\n";
  code[NEARm]   = "\000 DD _<n>\15\n";
  code[OR12]    = "\211OR EAX,EBX\15\n";
  code[PLUSn]   = "\000?+<n>??\15\n";
  code[POINT1l] = "\020MOV EAX,_<l>+<n>\15\n";
  code[POINT1m] = "\020MOV EAX,<m>\15\n";
  code[POINT1s] = "\020LEA EAX,[EBP<o>]\15\n";
  code[POINT2m] = "\002MOV EBX,<m>\15\n";
  code[POINT2m_]= "\002MOV EBX,<m>";
  code[POINT2s] = "\002LEA EBX,[EBP<o>]\15\n";
  code[POP2]    = "\002POP EBX\15\n";
  code[PUSH1]   = "\110PUSH EAX\15\n";
  code[PUSH2]   = "\101PUSH EBX\15\n";
  code[PUSHm]   = "\100PUSH <m>\15\n";
  code[PUSHp]   = "\100PUSH [EBX?<o>??]\15\n";
  code[PUSHs]   = "\100PUSH [EBP?<o>??]\15\n";
  code[PUT_m_]  = "\000MOV <m>";
  code[PUTbm1]  = "\010MOV BYTE [<m>],AL\15\n";
  code[PUTbp1]  = "\011MOV [EBX],AL\15\n";
  code[PUTdm1]  = "\010MOV DWORD [<m>],EAX\15\n";
  code[PUTdp1]  = "\011MOV [EBX],EAX\15\n";
  code[PUTwm1]  = "\010MOV WORD [<m>],AX\15\n";
  code[PUTwp1]  = "\011MOV [EBX],AX\15\n";
  code[rDEC1]   = "\010#DEC EAX\15\n#";
  code[rDEC2]   = "\010#DEC EBX\15\n#";
  code[REFm]    = "\000_<n>";
  code[RETURN]  = "\000?MOV ESP,EBP\15\n??POP EBP\15\nRET\15\n";
  code[rINC1]   = "\010#INC EAX\15\n#";
  code[rINC2]   = "\010#INC EBX\15\n#";
  code[SUB_m_]  = "\000SUB <m>";
  code[SUB12]   = "\011SUB EAX,EBX\15\n";                    /* see gen() */
  code[SUB1n]   = "\010?SUB EAX,<n>\15\n??";
  code[SUBbpn]  = "\001SUB [EBX],<n>\15\n";
  code[SUBwpn]  = "\001SUB [EBX],<n>\15\n";
  code[SWAP12]  = "\011XCHG EAX,EBX\15\n";
  code[SWAP1s]  = "\012POP EBX\15\nXCHG EAX,EBX\15\nPUSH EBX\15\n";
  code[SWITCH]  = "\012CALL __switch\15\n";
  code[XOR12]   = "\211XOR EAX,EBX\15\n";
  }

/***************** code generation functions *****************/

/*
** print all assembler info before any code is generated
** and ensure that the segments appear in the correct order.
*/
header()  {
/*  outline("  .386");
  outline("  .MODEL FLAT");
*/  toseg(CODESEG);

/* - FASM  
  outline("EXTERN __eq");
  outline("EXTERN __ne");
  outline("EXTERN __le");
  outline("EXTERN __lt");
  outline("EXTERN __ge");
  outline("EXTERN __gt");
  outline("EXTERN __ule");
  outline("EXTERN __ult");
  outline("EXTERN __uge");
  outline("EXTERN __ugt");
  outline("EXTERN __lneg");
  outline("EXTERN __switch");
*/

  /* outline("dw 0"); *//* force non-zero code pointers, word alignment */
  toseg(DATASEG);
  /* outline("dw 0"); *//* force non-zero data pointers, word alignment */
  }

/*
** print any assembler stuff needed at the end
*/
trailer()  {  
  char *cp;
  cptr = STARTGLB;
  while(cptr < ENDGLB) {
    if(cptr[IDENT] == FUNCTION && cptr[CLASS] == AUTOEXT)
      external(cptr + NAME, 0, FUNCTION);
    cptr += SYMMAX;
    }
/*
  if((cp = findglb("main")) && cp[CLASS]==STATIC)
    external("_main", 0, FUNCTION);
*/
  toseg(NULL);
/* outline("END"); */
#ifdef DISOPT
    {
    int i, *count;
    printf(";opt   count\n");
    for(i = -1; ++i <= HIGH_SEQ; ) {
      count = seq[i];
      printf("; %2u   %5u\n", i, *count);
      }
    }  
#endif 
  }

/*
** remember where we are in the queue in case we have to back up.
*/
setstage(before, start) int *before, *start; {
  if((*before = snext) == 0)
    snext = stage;
  *start = snext;
  }

/*
** generate code in staging buffer.
*/
gen(pcode, value)
int pcode, value;
  {
    int newcsp;
    switch(pcode)
      {
	case GETb1pu:
	case GETb1p:
	case GETw1p:
	case GETd1p:
	  gen(MOVE21, 0);
	  break;
	case SUB12:
	case MOD12:
	case MOD12u:
	case DIV12:
	case DIV12u:
	  gen(SWAP12, 0);
	  break;
	case PUSH1:
#ifdef INT32
	  csp -= BPD;
#else
	  csp -= BPW;
#endif
	  break;
	case POP2:
#ifdef INT32
	  csp += BPD;
#else
	  csp += BPW;
#endif
	  break;
	case ADDSP:
	case RETURN:
	  newcsp = value;
	  value -= csp;
	  csp = newcsp;
      }
    if(snext == 0)
      {
	outcode(pcode, value);
	return;
      }
    if(snext >= slast)
      {
	error("staging buffer overflow");
	return;
      }
    snext[0] = pcode;
    snext[1] = value;
    snext += 2;
  }

/*
** dump the contents of the queue.
** If start = 0, throw away contents.
** If before != 0, don't dump queue yet.
*/
clearstage(before, start)
int *before, *start;
  {
    if(before)
      {
	snext = before;
	return;
      }
    if(start)
      dumpstage();
    snext = 0;
  }

/*
** dump the staging buffer
*/
dumpstage()
  {
    int i;
    stail = snext;
    snext = stage;
    while(snext < stail)
      {
	if(optimize)
	  {
restart:
	    i = -1; 
	    while(++i <= HIGH_SEQ)
	      if(peep(seq[i]))
		{
#ifdef DISOPT
		  if(isatty(output))
		    fprintf(stderr, "                   optimized %2u\n", i);
#endif
		  goto restart;
		}
	  }
	outcode(snext[0], snext[1]);
	snext += 2;
      }
  }

/*
** change to a new segment
** may be called with NULL, CODESEG, or DATASEG
** With NASM the section names are case-sensitive
*/
toseg(newseg)
int newseg;
  {
    if(oldseg == newseg)
      return;
/*    if(oldseg == CODESEG)
      outline("_TEXT ENDS");
    else if(oldseg == DATASEG)
      outline("_DATA ENDS");  <-- */

/* - FASM
    if(newseg == CODESEG)
      {
	outline("SECTION .text");
      }
    else if(newseg == DATASEG)
      outline("SECTION .data");
*/
    oldseg = newseg;
  }

/*
** declare entry point
*/
public(ident) int ident;{
  if(ident == FUNCTION)
       toseg(CODESEG);
  else toseg(DATASEG);
/* - FASM
  outstr("GLOBAL ");
  outname(ssname);
*/
  newline();
  outname(ssname);
  if(ident == FUNCTION) {
    colon();
    newline();
    }
  }

/*
** declare external reference
*/
external(name, size, ident) char *name; int size, ident; {
  if(ident == FUNCTION)
       toseg(CODESEG);
  else toseg(DATASEG);
/* - FASM
  outstr("EXTERN ");
  outname(name);
/#  colon();
  outsize(size, ident);	<-- #/
  newline();
*/
  }

/*
** output the size of the object pointed to.
*/
outsize(size, ident) int size, ident;
  {
    /* why not size on FUNCTION and POINTER ? */
    if (ident == FUNCTION)
      outstr("NEAR");
    else if (ident == POINTER)
      outstr("DWORD");
    else if(size == 1)
      outstr("BYTE");
    else if(size == 2)
      outstr("WORD");
    else
      outstr("DWORD");
  }

/*
** point to following object(s)
*/
point() {
  outline(" DW $+2");
  }

/*
** dump the literal pool
*/
dumplits(size) int size;
{
  int j, k;
  k = 0;
  while (k < litptr)
    {
      if(size == 1)
	{
	  gen(BYTE_, NULL);
	}
      else if (size == 2)
	{
	  gen(WORD_, NULL);
	}
      else
	{
	  gen(DWORD_,NULL);
	}
      j = 10;
      while(j--)
	{
	  outdec(getint(litq + k, size));
	  k += size;
	  if(j == 0 || k >= litptr)
	    {
	      newline();
	      break;
	    }
	  fputc(',', output);
	}
    }
}

/*
** dump zeroes for default initial values
*/
dumpzero(size, count)
int size, count;
{
  if(count > 0)
    {
      if(size == 1)
	gen(BYTEr0, count);
      else if (size == 2)
	gen(WORDr0, count);
      else
	gen(DWORDr0, count);
    }
  }

/******************** optimizer functions ***********************/

/*
** Try to optimize sequence at snext in the staging buffer.
*/
peep(seq) int *seq; {
  int *next, *count, *pop, n, skip, tmp, reply;
  char c;
  next = snext;
  count = seq++;
  while(*seq) {
    switch(*seq) {
      case any:   if(next < stail)       break;      return (NO);
      case pfree: if(isfree(PRI, next))  break;      return (NO);
      case sfree: if(isfree(SEC, next))  break;      return (NO);
      case comm:  if(*next & COMMUTES)   break;      return (NO);
      case _pop:  if(pop = getpop(next)) break;      return (NO);
      default:    if(next >= stail || *next != *seq) return (NO);
      }
    next += 2; ++seq;
    }

  /****** have a match, now optimize it ******/

  *count += 1;
  reply = skip = NO;
  while(*(++seq) || skip) {
    if(skip) {
      if(*seq == 0) skip = NO;
      continue;
      }
    if(*seq >= PCODES) {
      c = *seq & 0xFF;            /* get low byte of command */
      n = c;                      /* and sign extend into n */
      switch(*seq & 0xFF00) {
        case ife:   if(snext[1] != n) skip = YES;  break;
        case ifl:   if(snext[1] >= n) skip = YES;  break;
        case go:    snext += (n<<1);               break;
        case gc:    snext[0] =  snext[(n<<1)];     goto done;
        case gv:    snext[1] =  snext[(n<<1)+1];   goto done;
        case sum:   snext[1] += snext[(n<<1)+1];   goto done;
        case neg:   snext[1] = -snext[1];          goto done;
        case topop: pop[0] = n; pop[1] = snext[1]; goto done;
        case swv:   tmp = snext[1];
                    snext[1] = snext[(n<<1)+1];
                    snext[(n<<1)+1] = tmp;
        done:       reply = YES;
                    break;
        }
      }
    else snext[0] = *seq;         /* set p-code */
    }
  return (reply);
  }

/*
** Is the primary or secondary register free?
** Is it zapped or unused by the p-code at pp
** or a successor?  If the primary register is
** unused by it still may not be free if the
** context uses the value of the expression.
*/
isfree(reg, pp) int reg, *pp; {
  char *cp;
  while(pp < stail) {
    cp = code[*pp];
    if(*cp & USES & reg) return (NO);
    if(*cp & ZAPS & reg) return (YES);
    pp += 2;
    }
  if(usexpr) return (reg & 001);   /* PRI => NO, SEC => YES at end */
  else       return (YES);
  }

/*
** Get place where the currently pushed value is popped?
** NOTE: Function arguments are not popped, they are
** wasted with an ADDSP.
*/
getpop(next) int *next; {
  char *cp;
  int level;  level = 0;
  while(YES) {
    if(next >= stail)                     /* compiler error */
      return 0;
    if(*next == POP2)
      if(level) --level;
      else return next;                   /* have a matching POP2 */
    else if(*next == ADDSP) {             /* after func call */
      if((level -= (next[1]>>LBPW)) < 0)
        return 0;
      }
    else {
      cp = code[*next];                   /* code string ptr */
      if(*cp & PUSHES) ++level;           /* must be a push */
      } 
    next += 2;
    }
  }

/******************* output functions *********************/

colon() {
  fputc(':', output);
  }

newline() {
  fputc('\15', output);
  fputc(NEWLINE, output);
  }

/*
** output assembly code.
** 
*/
outcode(pcode, value)
int pcode, value;
  {
    int part, skip, count;
    int byte_opt;
    char *cp, *back;
    int loc_label;
    part = back = 0;
    skip = NO;
    byte_opt = 0;
    
    cp = code[pcode] + 1;          /* skip 1st byte of code string */

#ifdef _MSC_VER

    switch (pcode)
      {
	case BYTE_:
	case BYTEn:
	case BYTEr0:
	case WORD_:
	case WORDn:
	case WORDr0:
	case DWORD_:
	case DWORDn:
	case DWORDr0:
	case REFm:
	case COMMAn:
	case PLUSn:
	  break;

	default:

	  dump_debug (pcode, value);
	  outtab ();
    }
#endif

    if (pcode == ADD1n)
      {
	if (value < 0)
	  {
	    pcode = SUB1n;
	    value = -value;
	  }
	if (value < 128)
	  {
	    byte_opt = 1;
	  }
      }
    
    while(*cp)
      {
	if(*cp == '<')
	  {
	    ++cp;                      /* skip to action code */
	    if(skip == NO)
	      switch(*cp)
		{
		  case 'm':
		    outname(value+NAME);
		    break; /* mem ref by label */
		  case 'n':
		    if (byte_opt)
		      {
			outstr ("BYTE ");
		      }
		    outdec(value);
		    break; /* numeric constant */
		  case 'o':
		    offset(value);
		    break; /* numeric constant */
		  case 'l':
		    outdec(litlab);
		    break; /* current literal label */
		  case 'g':	/* generate local label */
		    loc_label = getlabel ();
		    break;
		  case 'd':	/* dump local label */
		    outdec(loc_label);
		    break;
		}
	      cp += 2;                   /* skip past > */
	  }
	else if(*cp == '?')        /* ?..if value...?...if not value...? */
	  {
	    switch(++part)
	      {
		case 1:
		  if(value == 0)
		    skip = YES;
		  break;
		case 2:
		  skip = !skip;
		  break;
		case 3:
		  part = 0;
		  skip = NO;
		  break;
	      }
	    ++cp;                      /* skip past ? */
	  }
	else if(*cp == '#')
	  {        /* repeat #...# value times */
	    ++cp;
	    if(back == 0)
	      {
		if((count = value) < 1)
		  {
		    while(*cp && *cp++ != '#')
		      ;
		    continue;
		  }
		back = cp;
		continue;
	      }
	    if(--count > 0)
	      cp = back;
	    else
	      back = 0;
	  }
	else if(skip == NO)
	  fputc(*cp++, output);
	else
	  ++cp;
      }
  }

outdec(number)  int number; {
  int k, zs;
  char c, *q, *r;
  zs = 0;
  k = 1000000000;

#ifdef _MSC_VER
//  fprintf(output, "/* %d */", number);
#endif
  
  if(number < 0) {
    number = -number;
    fputc('-', output);
    }
  
  while (k >= 1) {
    q = 0;
    r = number;
    while(r >= k) {++q; r = r - k;}
    c = q + '0';
    if(c != '0' || k == 1 || zs) {
      zs = 1;
      fputc(c, output);
      }
    number = r;
    k /= 10;
    }
  }

offset(number)
int number;
{
  int k, zs;
  char c, *q, *r;
  zs = 0;
  k = 1000000000;
  if(number < 0) {
    number = -number;
    fputc('-', output);
    }
  else
    {
      fputc('+',output);
    }
  
  while (k >= 1) {
    q = 0;
    r = number;
    while(r >= k) {++q; r = r - k;}
    c = q + '0';
    if(c != '0' || k == 1 || zs) {
      zs = 1;
      fputc(c, output);
      }
    number = r;
    k /= 10;
    }
  }

outline(ptr)  char ptr[];  {
  outstr(ptr);
  newline();
  }

outname(ptr) char ptr[]; {
  outstr("_");
  while(*ptr >= ' ') fputc(*ptr++, output);
  }

outstr(ptr) char ptr[]; {
  while(*ptr == '\t' || *ptr >= ' ') fputc(*ptr++, output);
  }

outtab ()
  {
    fputc ('\t', output);
  }
/*
	dZ80 Disassembler Private Header
*/

#ifndef _MIDISSZ80P_
#define _MIDISSZ80P_

#include "types.h"
#include "dissz80.h"

#define MARKBLANKLINE			d->lineCmd |= LC_BLANKLINE
#define	ISCODEBYTE(d, pc)   	(d->opMap[pc >> 3] & (1 << (pc & 7)))

#define Z80IX					0x01		/* We're on a IX prefix */
#define Z80IY					0x02		/* We're on a IY prefix */
#define Z80GOTIXIYDISP			0x04		/* Got IX/IY displacement  */
#define Z80CB					0x08		/* Processing a CB op-code */
#define Z80ED					0x10		/* Processing a ED op-code */
#define Z80COMMENT				0x20		/* Have written a comment  */

#define Z80IXIY 				(Z80IX | Z80IY)

/* The options are used to control what happens after each instruction disassembly */
#define LC_BLANKLINE			0x0001

#define TABSIZE 				8
#define COMMENTCOLUMN			40

/* Opcode trap map bits */
#define	D_SCRIPT_PRE			0x01
#define D_SCRIPT_POST			0x02

#define PROGRESSUPDATEFREQ		100 	/* Num. instructions to disassemble before updating progress */

#define REG_B		0
#define REG_C		1
#define REG_D		2
#define REG_E		3
#define REG_H		4
#define REG_L		5
#define REG_HL		6
#define REG_A		7

void	PrepareDisInstruction(DISZ80 *d);
void	DisassembleInstruction(DISZ80 *d);
int 	LookOpcode(DISZ80 *d, int offset);
int 	GetNextOpCode(DISZ80 *d);

void	DisCB(DISZ80 *d);
void	DisDDCB(DISZ80 *d);
void	DisED(DISZ80 *d);
void	Dis40to7F(DISZ80 *d);
void	Dis80toBF(DISZ80 *d);
void	DisC0toFF(DISZ80 *d);
void	Dis00to3F(DISZ80 *d);
int 	DisED00to3F(DISZ80 *d);
int 	DisED40to7F(DISZ80 *d);
int 	DisED80toBF(DISZ80 *d);

void	AddToDis(DISZ80 *d, char *str);
void	AddToDisTab(DISZ80 *d, char *str);
void	AddToDisTabDB(DISZ80 *d);
void	AddToDisTabLD(DISZ80 *d, char *str);
void	AddToDisReg8(DISZ80 *d, int op, int op2);
void	AddToDisReg16(DISZ80 *d, int op);
void	AddToDisReg16NoAnd(DISZ80 *d, int op);
void	AddToDisHLIXIY(DISZ80 *d);
void	AddToDisUnknown(DISZ80 *d, char *Comment);
char	GetIXIYDisplacement(DISZ80 *d);

WORD	AddToDis16BitAbs(DISZ80 *d, int CommaFlag);
void	AddToDis8BitAbs(DISZ80 *d, int CommaFlag);
WORD	AddToDisRel8(DISZ80 *d, int CommaFlag);
void	AddToDisComment(DISZ80 *d, char *str);
void	AddToDisCommentZ180(DISZ80 *d);
void	FlagFn(DISZ80 *d, unsigned int Addr); 
int 	IsFnUsed(DISZ80 *d, unsigned int Addr);
void	AddToDisUndoc(DISZ80 *d);
void	AddToDisUndocNop(DISZ80 *d);
WORD	Get16BitParam(DISZ80 *d);
void	DisZ80CleanUp(DISZ80 *d);
int 	CreateOutputASMFile(DISZ80 *d);
int 	PrepareOpMap(DISZ80 *d);
void	AddRefEntry(DISZ80 *d, int Addr, int PC, int RefType);
void	AddReferenceAddr(DISZ80 *d, DISREF *p, int PC);
void	WriteReferenceFile(DISZ80 *d);
int 	PrepareReferenceListing(DISZ80 *d);
void	DoProgress(DISZ80 *d, int forceUpdate);
void	Add16BitAddress(DISZ80 *d, WORD Addr);
void	UnderlineText(FILE *stream, char *text);

/* Added in 1.50 */
void	Add8BitNum(DISZ80 *d, int Num);
void	Make8BitNum(DISZ80 *d, char *Dst, int Num);
void	Add16BitNum(DISZ80 *d, int Num);
void	Make16BitNum(DISZ80 *d, char *Dst, int Num);
void	MakeLJustified16BitNum(DISZ80 *d, char *Dst, int Num);
BYTE*	AllocateMap(DISZ80 *d, char *ErrorStr, unsigned int BytesWanted);
int 	WriteDisLine(DISZ80 *d, unsigned int Addr);
void	StartPass(DISZ80 *d);
int		WithinDisRange(DISZ80 *d);

/* Added in 2.0: Script functions */
int 	InitOpcodeTraps(DISZ80 *d);
int 	ShutdownScripting(DISZ80 *d);
int 	InitScripting(DISZ80 *d);
int 	ExecPreTrap(DISZ80 *d);
void	ExecPostTrap(DISZ80 *d);

#ifndef _DZ80_EXCLUDE_SCRIPT
/* Lua support extensions */
DISZ80 *GetD(lua_State *ls);
int d_FromHex(lua_State *ls);
int LuaErrorHandler(lua_State *ls);
#endif

/* Misc tables */
extern char *VersionString;
extern char *IMModes[4];
extern char *Conditions[8];
extern char *AccRotType[8];
extern char *CBRotType[8];
extern char *Reg8Idx[8];
extern char *Reg8AFIdx[8];
extern char *Reg16Idx[5];
extern char *BasicOps[8];
extern char *RepeatOps[16];
extern char *Z180RepeatOps[4];
extern char *dZ80ErrorMsgs[DERR_TOTAL];



#endif		/* _MIDISSZ80P_ */


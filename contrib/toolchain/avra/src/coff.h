//
// coff.h - Common Object File Format (COFF) support
//
//	This file was developed for the avra assembler in order to produce COFF output files 
//	for use with the Atmel AVR Studio.  The Lean C Compiler (LCC) debugging stabs 
//	output was used as input to the assembler.
//
//	This software has absolutely no warrantee!  The money you paid for this will be 
//	promptly refunded if not fully satisfied.
//
//	Beta release 1/20/2000 by Bob Harris
//
//	This software has not been fully tested and probably has a few software deficiencies.
//	Some software support may be possible by sending a problem description report to 
//	rth@mclean.sparta.com

#define MAGIC_NUMBER_AVR   0xa12

#define	N_GSYM	0x20		/* global symbol: name,,0,type,0 */
#define	N_FNAME	0x22		/* procedure name (f77 kludge): name,,0 */
#define	N_FUN	0x24		/* procedure: name,,0,linenumber,address */
#define	N_STSYM	0x26		/* static symbol: name,,0,type,address */
#define	N_LCSYM	0x28		/* .lcomm symbol: name,,0,type,address */
#define N_MAIN  0x2a            /* name of main routine : name,,0,0,0 */
#define N_ROSYM 0x2c		/* ro_data objects */
#define N_OBJ	0x38		/* object file path or name */
#define N_OPT	0x3c		/* compiler options */
#define	N_RSYM	0x40		/* register sym: name,,0,type,register */
#define	N_SLINE	0x44		/* src line: 0,,0,linenumber,address */
#define	N_FLINE	0x4c		/* function start.end */
#define	N_SSYM	0x60		/* structure elt: name,,0,type,struct_offset */
#define N_ENDM	0x62		/* last stab emitted for module */
#define	N_SO	0x64		/* source file name: name,,0,0,address */
#define	N_LSYM	0x80		/* local sym: name,,0,type,offset */
#define	N_BINCL 0x82		/* header file: name,,0,0,0 */
#define	N_SOL	0x84		/* #included file name: name,,0,0,address */
#define	N_PSYM	0xa0		/* parameter: name,,0,type,offset */
#define N_EINCL 0xa2		/* end of include file */
#define	N_ENTRY	0xa4		/* alternate entry: name,linenumber,address */
#define	N_LBRAC	0xc0		/* left bracket: 0,,0,nesting level,address */
#define	N_EXCL	0xc2		/* excluded include file */
#define	N_RBRAC	0xe0		/* right bracket: 0,,0,nesting level,address */
#define	N_BCOMM	0xe2		/* begin common: name,, */
#define	N_ECOMM	0xe4		/* end common: name,, */
#define	N_ECOML	0xe8		/* end common (local name): ,,address */
#define	N_LENG	0xfe		/* second stab entry with length information */


/*
 * Type of a symbol, in low N bits of the word
 */
#define T_NULL		0
#define T_VOID		1	/* function argument (only used by compiler) */
#define T_CHAR		2	/* character		*/
#define T_SHORT		3	/* short integer	*/
#define T_INT		4	/* integer		*/
#define T_LONG		5	/* long integer		*/
#define T_FLOAT		6	/* floating point	*/
#define T_DOUBLE	7	/* double word		*/
#define T_STRUCT	8	/* structure 		*/
#define T_UNION		9	/* union 		*/
#define T_ENUM		10	/* enumeration 		*/
#define T_MOE		11	/* member of enumeration*/
#define T_UCHAR		12	/* unsigned character	*/
#define T_USHORT	13	/* unsigned short	*/
#define T_UINT		14	/* unsigned integer	*/
#define T_ULONG		15	/* unsigned long	*/
#define T_LNGDBL	16	/* long double		*/

/*
 * derived types, in n_type
*/
#define DT_NON		(0)	/* no derived type */
#define DT_PTR		(1)	/* pointer */
#define DT_FCN		(2)	/* function */
#define DT_ARY		(3)	/* array */

struct external_filehdr {
	unsigned short f_magic;		/* magic number			*/
	unsigned short f_nscns;		/* number of sections		*/
	unsigned long f_timdat;	/* time & date stamp		*/
	unsigned long f_symptr;	/* file pointer to symtab	*/
	unsigned long f_nsyms;		/* number of symtab entries	*/
	unsigned short f_opthdr;	/* sizeof(optional hdr)		*/
	unsigned short f_flags;		/* flags			*/
};

/* Bits for f_flags:
 *	F_RELFLG	relocation info stripped from file
 *	F_EXEC		file is executable (no unresolved external references)
 *	F_LNNO		line numbers stripped from file
 *	F_LSYMS		local symbols stripped from file
 *	F_AR32WR	file has byte ordering of an AR32WR machine (e.g. vax)
 */

#define F_RELFLG	(0x0001)
#define F_EXEC		(0x0002)
#define F_LNNO		(0x0004)
#define F_LSYMS		(0x0008)

/*********************************************************************/
struct external_scnhdr {
	char		s_name[8];	/* section name			*/
	unsigned long		s_paddr;	/* physical address, aliased s_nlib */
	unsigned long		s_vaddr;	/* virtual address		*/
	unsigned long		s_size;		/* section size			*/
	unsigned long		s_scnptr;	/* file ptr to raw data for section */
	unsigned long		s_relptr;	/* file ptr to relocation	*/
	unsigned long		s_lnnoptr;	/* file ptr to line numbers	*/
	unsigned short		s_nreloc;	/* number of relocation entries	*/
	unsigned short		s_nlnno;	/* number of line number entries*/
	unsigned long		s_flags;	/* flags			*/
};

#define	SCNHDR	struct external_scnhdr
#define	SCNHSZ	sizeof(SCNHDR)

/*
 * names of "special" sections
 */
#define _TEXT	".text"
#define _DATA	".data"
#define _BSS	".bss"
#define _COMMENT ".comment"
#define _LIB ".lib"

/*
 * s_flags "type"
 */
#define STYP_TEXT	 (0x0020)	/* section contains text only */
#define STYP_DATA	 (0x0040)	/* section contains data only */
#define STYP_BSS	 (0x0080)	/* section contains bss only */


/*********************************************************************/

struct lineno
{
   union
   {
             long  l_symndx;  /* symtbl index of func name */
             long  l_paddr;   /* paddr of line number */
   } l_addr;
   unsigned short  l_lnno;    /* line number */
};

#define LINENO      struct lineno
#define LINESZ      6



#define N_UNDEF	((short)0)	/* undefined symbol */
#define N_ABS	((short)-1)	/* value of symbol is absolute */
#define N_DEBUG	((short)-2)	/* debugging symbol -- value is meaningless */

/********************** STORAGE CLASSES **********************/

/* This used to be defined as -1, but now n_sclass is unsigned.  */
#define C_EFCN		0xff	/* physical end of function	*/
#define C_NULL		0
#define C_AUTO		1	/* automatic variable		*/
#define C_EXT		2	/* external symbol		*/
#define C_STAT		3	/* static			*/
#define C_REG		4	/* register variable		*/
#define C_EXTDEF	5	/* external definition		*/
#define C_LABEL		6	/* label			*/
#define C_ULABEL	7	/* undefined label		*/
#define C_MOS		8	/* member of structure		*/
#define C_ARG		9	/* function argument		*/
#define C_STRTAG	10	/* structure tag		*/
#define C_MOU		11	/* member of union		*/
#define C_UNTAG		12	/* union tag			*/
#define C_TPDEF		13	/* type definition		*/
#define C_USTATIC	14	/* undefined static		*/
#define C_ENTAG		15	/* enumeration tag		*/
#define C_MOE		16	/* member of enumeration	*/
#define C_REGPARM	17	/* register parameter		*/
#define C_FIELD		18	/* bit field			*/
#define C_AUTOARG	19	/* auto argument		*/
#define C_LASTENT	20	/* dummy entry (end of block)	*/
#define C_BLOCK		100	/* ".bb" or ".eb"		*/
#define C_FCN		101	/* ".bf" or ".ef"		*/
#define C_EOS		102	/* end of structure		*/
#define C_FILE		103	/* file name			*/
#define C_LINE		104	/* line # reformatted as symbol table entry */
#define C_ALIAS	 	105	/* duplicate tag		*/
#define C_HIDDEN	106	/* ext symbol in dmert public lib */

#define E_SYMNMLEN	8	/* # characters in a symbol name	*/
#define E_FILNMLEN	14	/* # characters in a file name		*/
#define E_DIMNUM	4	/* # array dimensions in auxiliary entry */

struct syment
{
  union
  {
      char          _n_name[E_SYMNMLEN];  /* symbol name*/
      struct
      {
            long    _n_zeroes;          /* symbol name */

            long    _n_offset;          /* location in string table */
      } _n_n;
      char          *_n_nptr[2];        /* allows overlaying */
  } _n;
  unsigned long     n_value;            /* value of symbol */

  short             n_scnum;            /* section number */

  unsigned short    n_type;             /* type and derived */

  char              n_sclass;           /* storage class */

  char              n_numaux;           /* number of aux entries */
};

#define  n_name          _n._n_name
#define  n_zeroes        _n._n_n._n_zeroes
#define  n_offset        _n._n_n._n_offset
#define  n_nptr          _n._n_nptr[1]

#define  SYMNMLEN  8
#define  SYMESZ    18                    /* size of a symbol table entry */

union auxent
{
     struct
     {
           long   x_tagndx;
           union
           {
               struct
               {
                     unsigned short   x_lnno;
                     unsigned short   x_size;
               } x_lnsz;
               long    x_fsize;
           } x_misc;
           union
           {
               struct
               {
                     long    x_lnnoptr;
                     long    x_endndx;
               } x_fcn;
               struct
               {
                     unsigned short   x_dimen[E_DIMNUM];
               } x_ary;
           } x_fcnary;
           unsigned short   x_tvndx;
       } x_sym;
       union
       {
           char   x_fname[E_FILNMLEN];
			struct {
				unsigned long x_zeroes;
				unsigned long x_offset;
			} x_n;
       } x_file;
       struct
       {
           long   x_scnlen;
           unsigned short   x_nreloc;
           unsigned short   x_nlinno;
       } x_scn;
       struct
       {
           long   x_tvfill;
           unsigned short   x_tvlen;
           unsigned short   x_tvran[2];
       } x_tv;
};

#define FILNMLEN  14
#define DIMNUM    4
#define AUXENT    union auxent
#define AUXESZ    18


/* Coff additions */
typedef struct ListNodeTag{
	struct ListNodeTag *Next;	/* Double Linked List */
	struct ListNodeTag *Last;	/* Double Linked List */
	void *pObject;	/* points to list object */	
	unsigned long Size;
	int FileNumber;		/* corresponds to individual file(s) */
} LISTNODE;

//#define LISTNODE struct ListNodeTag;

typedef struct ListNodeHeadTag {
	LISTNODE Node;
//	struct ListNodeTag *Next;	/* Double Linked List */
//	struct ListNodeTag *Last;	/* Double Linked List */
	int TotalBytes;	/* size of allocated object(s) */
	int TotalItems; /* number of allocated objects */
	LISTNODE *current;	/* pointer for FindFirst/FindNext */
} LISTNODEHEAD ;


typedef struct  {
	unsigned short StabType;
	unsigned short CoffType;
	unsigned short ByteSize;
	unsigned short Line;	/* used by arrays */
	unsigned short Dimensions[6]; /* used by arrays */
} STABCOFFMAP;

struct coff_info {

	int CurrentFileNumber;
	int FunctionStartLine;	/* used in Line number table */
	int CurrentSourceLine;

	/* Internal */
	unsigned char *pRomMemory;	/* 16 bit wide words/addresses */
	unsigned char *pEEPRomMemory;	/* 8 bit wide words/addresses */
   int MaxRomAddress;
   int MaxEepromAddress;
   int NeedLineNumberFixup;
   int GlobalStartAddress;
   int GlobalEndAddress;
   LISTNODEHEAD ListOfSplitLines;

	/* External */
	struct external_filehdr FileHeader;		/* Only one of these per output file */
	LISTNODEHEAD ListOfSectionHeaders;	/* .text, .bss */
	LISTNODEHEAD ListOfRawData;			/* Program, EEPROM */
	LISTNODEHEAD ListOfRelocations;		/* Not used now */
	LISTNODEHEAD ListOfLineNumbers;
	LISTNODEHEAD ListOfSymbols;
	LISTNODEHEAD ListOfGlobals;
	LISTNODEHEAD ListOfSpecials;
	LISTNODEHEAD ListOfUndefined;
	LISTNODEHEAD ListOfStrings;
	LISTNODEHEAD ListOfTypes;
};

#if 0 /* defined in avra.h */

FILE *open_coff_file(struct prog_info *pi, char *filename);
void write_coff_file(struct prog_info *pi);
void write_coff_eeprom( struct prog_info *pi, int address, unsigned char data);
void write_coff_program( struct prog_info *pi, int address, unsigned char data);
void close_coff_file(struct prog_info *pi, FILE *fp);
int parse_stabs( struct prog_info *pi, char *p );
int parse_stabn( struct prog_info *pi, char *p );

#endif

/**************************************************************/
/*********** Internal Routines ********************************/
/**************************************************************/
int stab_add_lineno(  struct prog_info *pi, int LineNumber, char *pLabel, char *pFunction );
int stab_add_lbracket( struct prog_info *pi, int Level, char *pLabel, char *pFunction );
int stab_add_rbracket( struct prog_info *pi, int Level, char *pLabel, char *pFunction );
int stab_add_filename( char *pName, char *pLabel );
int stab_add_function( struct prog_info *pi, char *pName, char *pLabel );
int stab_add_global( struct prog_info *pi, char *pName, char *pType );
int stab_add_local(  struct prog_info *pi, char *pName, char *pType, char *pOffset  );
int stab_add_parameter_symbol( struct prog_info *pi, char *pName, char *pType, char *pOffset  );
int stab_add_static_symbol(  struct prog_info *pi, char *pName, char *pType, char *pLabel  );
int stab_add_local_register(  struct prog_info *pi, char *pName, char *pType, char *pRegister  );
int stab_add_local_type( char *pString, char *pType );
int stab_add_tag_type( char *pName, char *pDesciptor );

int GetStabType( char *p, unsigned short *pType, char **pEnd );
int AddNameToEntry( char *pName, struct syment *pEntry );
int GetArrayType( char *p, char **pEnd, STABCOFFMAP *pMap, unsigned short *DerivedBits, int ExtraLevels );
int GetEnumTagItem( char *p, char **pEnd, char **pEnumName, int *pEnumValue );
int GetStructUnionTagItem( char *p, char **pEnd, char **pName, unsigned short *pType, unsigned short *pBitOffset, unsigned short *pBitSize);
int GetStringDelimiters( char *pString, char **pTokens, int MaxTokens );
int SetupDefinedType( unsigned short Type, STABCOFFMAP *pMap, unsigned short *DerivedBits, int ExtraLevels );
int GetArrayDefinitions( STABCOFFMAP *pMap , char *pMinIndex, char *pMaxIndex, char *pType, unsigned short *DerivedBits, int ExtraLevels );
int GetInternalType( char *pName, STABCOFFMAP *pMap );
unsigned short GetCoffType( unsigned short StabType );
unsigned short GetCoffTypeSize( unsigned short StabType );
int CopyStabCoffMap( unsigned short StabType, STABCOFFMAP *pMap );
int IsTypeArray( unsigned short CoffType );
void AddArrayAuxInfo( union auxent *pAux, unsigned short SymbolIndex, STABCOFFMAP *pMap );
int GetSubRangeType( unsigned short Type, STABCOFFMAP *pMap , char *pLow, char *pHigh );
char *SkipPastDigits( char *p );
int GetDigitLength( char *p );

/****************************************************************************************/
/* List management routines */
/****************************************************************************************/

void InitializeList( LISTNODEHEAD *pNode );
void *AllocateTwoListObjects( LISTNODEHEAD *pHead, int size );
void *AllocateListObject( LISTNODEHEAD *pHead, int size );
LISTNODE *AllocateListNode( void *pObject, int size );
void AddNodeToList( LISTNODEHEAD *pHead, LISTNODE *pNode );
void *FindFirstListObject( LISTNODEHEAD *pHead );
void *FindNextListObject( LISTNODEHEAD *pHead );
LISTNODE *GetCurrentNode( LISTNODEHEAD *pHead );
void *GetCurrentListObject( LISTNODEHEAD *pHead );
void *FindLastListObject( LISTNODEHEAD *pHead );
void *FindNextLastListObject( LISTNODEHEAD *pHead );
void FreeList( LISTNODEHEAD *pHead );
LISTNODE  *AddListObject(LISTNODEHEAD *pHead, void *pObject, int size );


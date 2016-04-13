/***********************************************************************
 *
 *  avra - Assembler for the Atmel AVR microcontroller series
 *
 *  Copyright (C) 1998-2004 Jon Anders Haugum, Tobias Weber
 *
 * coff.c - Common Object File Format (COFF) support
 *
 *	This file was developed for the avra assembler in order to produce COFF output files 
 *  for use with the Atmel AVR Studio.  The Lean C Compiler (LCC) debugging stabs 
 *	output was used as input to the assembler.  The information used to develop this file
 *  was obtained from various sources on the Internet, most notably, the Free Software Foundation,
 *  The "stabs" debug format, ??? Chapter 7: Common Object File Format (COFF), 
 *
 *	This software has absolutely no warrantee!  The money you paid for this will be 
 *  promptly refunded if not fully satisfied.
 * 
 *	Beta release 1/20/2000 by Bob Harris
 *
 *	This software has not been fully tested and probably has a few software deficiencies.
 *  Some software support may be possible by sending a problem description report to 
 *  rth@mclean.sparta.com
 *
 *  Made the recommended change in write_coff_program().
 *  Fixed an obvious typo in SkipPastDigits().  The if() statement was terminated 
 *  with a semicolon, which terminated the if(); early.  JEG 4-01-03
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "misc.h"
#include "avra.h"
#include "args.h"

#include "coff.h"
#include "device.h"	/* device flash and eeprom size */


struct FundamentalType {

    char const *pString;
    int Type;
    int Size;
};

struct FundamentalType FundamentalTypes[] = {
    {"null", T_NULL, 0},
    {"void", T_VOID, 0},
    {"char", T_CHAR, 1},
    {"short", T_SHORT, 1},
    {"int", T_INT, 1},
    {"long", T_LONG, 2}, 
    {"float", T_FLOAT, 4},
    {"double", T_DOUBLE, 4},
    {"struct", T_STRUCT, 0},
    {"union", T_UNION, 0},
    {"enum",  T_ENUM, 0},
    {"moe", T_MOE, 0},
    {"unsigned char", T_UCHAR, 1},
    {"unsigned short", T_USHORT, 1},
    {"unsigned int", T_UINT, 1},
    {"unsigned long", T_ULONG, 2},
    {"long double", T_LNGDBL, 2},
    {"long long int", T_LONG, 2},
    {"long int", T_LONG, 2},
    {"unsigned long long", T_ULONG, 2}, 
    {"signed char", T_CHAR, 1},
    {0, 0}
};


struct coff_info *ci;

/****************************************************************************************/

FILE *open_coff_file(struct prog_info *pi, char *filename){

    int ok /*, i*/;
    FILE *fp;
    //unsigned long *pu4;
    char*p;


    ci = calloc( 1, sizeof(struct coff_info) );
    if ( !ci )
        return( 0 );

    ok = True;
    /* default values */
    ci->CurrentFileNumber = 0;
    ci->pRomMemory = 0; 
    ci->pEEPRomMemory = 0;
    ci->MaxRomAddress = 0;
    ci->MaxEepromAddress = 0;
    ci->NeedLineNumberFixup = 0;
    ci->GlobalStartAddress = -1;
    ci->GlobalEndAddress = 0;

    /* Linked lists start out at zero */
    InitializeList( &ci->ListOfSectionHeaders );
    InitializeList( &ci->ListOfRawData );
    InitializeList( &ci->ListOfRelocations );
    InitializeList( &ci->ListOfLineNumbers );
    InitializeList( &ci->ListOfSymbols );
    InitializeList( &ci->ListOfGlobals );
    InitializeList( &ci->ListOfSpecials );
    InitializeList( &ci->ListOfUndefined );
    InitializeList( &ci->ListOfStrings );
    InitializeList( &ci->ListOfTypes );
    InitializeList( &ci->ListOfSplitLines );

    /* add two default sections to SectionHeaders */
    if (  !AllocateListObject( &ci->ListOfSectionHeaders, sizeof(struct external_scnhdr) ) ||
          !AllocateListObject( &ci->ListOfSectionHeaders, sizeof(struct external_scnhdr) ) ) {

        fprintf(stderr, "\nOut of memory allocating section headers!");
        return( 0 );
    }

    /* add to string table */
    p = (char *)AllocateListObject( &ci->ListOfStrings,  4 );
    if ( !p ) {
        fprintf(stderr, "\nOut of memory allocating string table space!");
        return( 0 );
    }

    /* Allocate space for binary output into ROM, and EEPROM memory buffers for COFF output */
    /* ASSUMES ci->device is accurate */
    if ( (ci->pRomMemory = AllocateListObject( &ci->ListOfRawData, pi->device->flash_size * 2 ) ) != 0) {
        if ( (ci->pEEPRomMemory = AllocateListObject( &ci->ListOfRawData, pi->device->eeprom_size )) != 0) {
            ok = True; /* only true if both buffers are properly allocated */
            /* now fill them with 0xff's to simulate flash erasure */
            memset( (void *)ci->pRomMemory, 0xff, pi->device->flash_size * 2 );
            memset(    (   void    *)ci->pEEPRomMemory,    0xff,    pi->device->eeprom_size    );
        }
    }
    if ( ok != True )
        return( 0 );

    fp = fopen(filename,"wb");
    if ( fp == NULL ) {
        fprintf(stderr,"Error: cannot write coff file\n");
        return( fp );
    }
    /* simulate void type .stabs void:t15=r1;*/
    stab_add_local_type( "void", "15=r1;0;0;" );

    return( fp );   
}

/****************************************************************************************/
void write_coff_file(struct prog_info *pi){

    //FILE *fp;
    //struct label *label;
    char /* File[256],*/*p;
    struct external_scnhdr *pSectionHdr;
    struct syment *pEntry;
    union auxent *pAux;
    unsigned long *plong;
    int NumberOfSymbols, SymbolIndex, LastFileIndex, LastFunctionIndex, LastFunctionAddress;
    LISTNODE *pNode;
    int LinesOffset, SymbolsOffset, StringsOffset, RawOffset;
    struct lineno *pLine; 

    /* add two special sections */
    /* one for .text */
    if (  ( pEntry = (struct syment*)AllocateTwoListObjects( &ci->ListOfSpecials, sizeof(struct syment) * 2 ) ) == 0 ) {
        fprintf(stderr, "\nOut of memory allocating special headers for .text!");
        return;
    }
    memset( pEntry->n_name, 0, 8 );
    strcpy( pEntry->n_name, ".text" );
    pEntry->n_value = 0;
    pEntry->n_scnum = 1;
    pEntry->n_type = 0;
    pEntry->n_sclass = C_STAT;
    pEntry->n_numaux = 1;
    pEntry++;
    pAux = (union auxent *)pEntry;
    pAux->x_scn.x_scnlen = ci->MaxRomAddress + 2;
    pAux->x_scn.x_nreloc = 0;
    pAux->x_scn.x_nlinno = ci->ListOfLineNumbers.TotalItems;
    /* one for .bss */
    if (  ( pEntry = (struct syment*)AllocateTwoListObjects( &ci->ListOfSpecials, sizeof(struct syment) * 2 ) ) == 0 ) {
        fprintf(stderr, "\nOut of memory allocating special header for .bss!");
        return;
    }
    memset( pEntry->n_name, 0, 8 );
    strcpy( pEntry->n_name, ".bss" );
    if ( ci->GlobalStartAddress == -1 ) {
        ci->GlobalEndAddress = ci->GlobalStartAddress = 0x60;
    }
    pEntry->n_value = ci->GlobalStartAddress;
    pEntry->n_scnum = 2;
    pEntry->n_type = 0;
    pEntry->n_sclass = C_STAT;
    pEntry->n_numaux = 1;
    pEntry++;
    pAux = (union auxent *)pEntry;
    pAux->x_scn.x_scnlen = 0; /* we don't store any data here */
    pAux->x_scn.x_nreloc = 0;
    pAux->x_scn.x_nlinno = 0;

    /* one more for .data - eeprom ??? */

    /* Calculate common offsets into the file */
    RawOffset = sizeof(struct external_filehdr) + ci->ListOfSectionHeaders.TotalBytes;
    LinesOffset = RawOffset + ci->MaxRomAddress + 2; /* ignore eeprom for now */
    SymbolsOffset = LinesOffset + ci->ListOfLineNumbers.TotalBytes;
    StringsOffset = SymbolsOffset + ci->ListOfSymbols.TotalBytes + ci->ListOfSpecials.TotalBytes + ci->ListOfGlobals.TotalBytes;

    /* Clean up loose ends in string table */
    if ( !(plong  = (unsigned long *)FindFirstListObject(&ci->ListOfStrings))  ) {
        fprintf(stderr,"\nInternal error in string table!");
        return;
    }
    *plong = ci->ListOfStrings.TotalBytes; /* Size of string table */

    /* Clean up loose ends in symbol table */

    /*	symbol table - Filename value - index to next .file or global symbol */
    /* The value of that symbol equals the symbol table entry index of the next .file symbol or .global */
    LastFunctionAddress = ci->MaxRomAddress;
    NumberOfSymbols = ci->ListOfSymbols.TotalItems + ci->ListOfSpecials.TotalItems + ci->ListOfGlobals.TotalItems;
    SymbolIndex = LastFileIndex = NumberOfSymbols;
    LastFunctionIndex = 0; /* set to zero on last function */
    for ( pEntry = (struct syment *)FindLastListObject(&ci->ListOfSymbols);
        pEntry != 0;
        pEntry = (struct syment *)FindNextLastListObject(&ci->ListOfSymbols) ) {

        /* Search for .file entries designated by C_FILE */
        if ( pEntry->n_sclass == C_FILE ) {
            pEntry->n_value = LastFileIndex;
            LastFileIndex = SymbolIndex; /* save current index */
        }
        /* Search for Function entries C_EXT */
        else if (  pEntry->n_sclass == C_EXT ) {
            pEntry++;
            pAux = (union auxent *)pEntry;
            pAux->x_sym.x_misc.x_fsize = LastFunctionAddress - pEntry->n_value; /* function updated size */
            pAux->x_sym.x_fcnary.x_fcn.x_lnnoptr += LinesOffset;
            LastFunctionAddress = pEntry->n_value;
            pAux->x_sym.x_fcnary.x_fcn.x_endndx = LastFunctionIndex; /* point to next function index */
            pAux->x_sym.x_tvndx = 0; /* ??? */
            LastFunctionIndex = SymbolIndex;
        } else if (  (pEntry->n_sclass == C_FCN ) || ( pEntry->n_sclass == C_BLOCK) ) {
            if ( pEntry->n_name[1] == 'b' ) {
                /* .bf and .bb */
                pEntry++;
                pAux = (union auxent *)pEntry;
                pAux->x_sym.x_fcnary.x_fcn.x_endndx = LastFunctionIndex;
            }
        }
        /* else do nothing */

        /* update current symbol index */
        pNode = GetCurrentNode( &ci->ListOfSymbols );
        SymbolIndex -= ( pNode->Size / sizeof(struct syment) );
    }

    // File Header
    ci->FileHeader.f_magic = MAGIC_NUMBER_AVR;
    ci->FileHeader.f_nscns = 2;
//    ci->FileHeader.f_timdat = time( (time_t *)&ci->FileHeader.f_timdat);
    ci->FileHeader.f_timdat = pi->time;
    ci->FileHeader.f_symptr = SymbolsOffset;
    ci->FileHeader.f_nsyms = NumberOfSymbols;
    ci->FileHeader.f_opthdr = 0;
    ci->FileHeader.f_flags = 0xff; /*F_RELFLG;*/ /* No relocation information available */

    /* write it out */
    if ( fwrite(&ci->FileHeader, 1, sizeof(struct external_filehdr), pi->coff_file ) !=  sizeof(struct external_filehdr) ) {
        fprintf(stderr,"\nFile error writing header ...(disk full?)");
        return;
    }

    // Optional Information

    // Section 1 Header
    pSectionHdr = (struct external_scnhdr *)FindFirstListObject(&ci->ListOfSectionHeaders);
    if ( !pSectionHdr ) {
        fprintf(stderr, "\nInternal Coff error - cannot find section header .text!");
        return;
    }
    memset( &pSectionHdr->s_name[0], 0, sizeof(struct external_scnhdr) );
    strcpy( &pSectionHdr->s_name[0], ".text");
    pSectionHdr->s_paddr = 0;
    pSectionHdr->s_vaddr = 0;
    pSectionHdr->s_size = ci->MaxRomAddress + 2; /* remember the last instruction */
    pSectionHdr->s_scnptr = RawOffset;
    pSectionHdr->s_relptr = 0;
    pSectionHdr->s_lnnoptr = LinesOffset;
    pSectionHdr->s_nreloc = 0;
    pSectionHdr->s_nlnno = ci->ListOfLineNumbers.TotalBytes/sizeof(struct lineno);
    pSectionHdr->s_flags = STYP_TEXT;

    /* write it out */
    if ( fwrite(&pSectionHdr->s_name[0], 1, sizeof(struct external_scnhdr), pi->coff_file ) !=  sizeof(struct external_scnhdr) ) {
        fprintf(stderr,"\nFile error writing section header ...(disk full?)");
        return;
    }

    // Section 2 Header
    pSectionHdr = (struct external_scnhdr *)FindNextListObject(&ci->ListOfSectionHeaders);
    if ( !pSectionHdr ) {
        fprintf(stderr, "\nInternal Coff error - cannot find section header .bss!");
        return;
    }
    memset( &pSectionHdr->s_name[0], 0, sizeof(struct external_scnhdr) );
    strcpy( &pSectionHdr->s_name[0], ".bss");
    /* later expansion */
    pSectionHdr->s_paddr = ci->GlobalStartAddress;
    pSectionHdr->s_vaddr = ci->GlobalStartAddress;
    pSectionHdr->s_flags = STYP_DATA; /* seems it should be STYP_BSS */

    /* write it out */
    if ( fwrite(&pSectionHdr->s_name[0], 1, sizeof(struct external_scnhdr), pi->coff_file ) !=  sizeof(struct external_scnhdr) ) {
        fprintf(stderr,"\nFile error writing section header ...(disk full?)");
        return;
    }

    /* Section N Header - .data or eeprom */

    // Raw Data for Section 1
    if ( (p = FindFirstListObject(&ci->ListOfRawData) ) == 0 ) {
        fprintf(stderr,"\nInternal error - unable to find binary data!");
        return;
    }

    /* write it out */
    if ( fwrite( p, 1, ci->MaxRomAddress + 2, pi->coff_file ) != (size_t)(ci->MaxRomAddress + 2) ) {
        fprintf(stderr,"\nFile error writing raw .text data ...(disk full?)");
        return;
    }
    // Raw data for section n

    // Relocation Info for section 1

    // Relocation info for section n

    // Line numbers for section 1
    for ( pLine = (struct lineno *)FindFirstListObject( &ci->ListOfLineNumbers );
        pLine != 0;
        pLine = (struct lineno *)FindNextListObject( &ci->ListOfLineNumbers ) ) {

        pNode = GetCurrentNode( &ci->ListOfLineNumbers );

        /* write it out */
        if ( fwrite( pLine, 1, pNode->Size, pi->coff_file ) != pNode->Size ) {
            fprintf(stderr,"\nFile error writing line numbers ...(disk full?)");
            return;
        }
    }


    // Line numbers for section n

    // Symbol table
    for ( pEntry = (struct syment *)FindFirstListObject( &ci->ListOfSymbols );
        pEntry != 0;
        pEntry = (struct syment *)FindNextListObject( &ci->ListOfSymbols ) ) {

        pNode = GetCurrentNode( &ci->ListOfSymbols );

        /* write it out */
        if ( fwrite( pEntry, 1, pNode->Size, pi->coff_file ) != pNode->Size ) {
            fprintf(stderr,"\nFile error writing symbol table ...(disk full?)");
            return;
        }
    }

    // Symbol table of Globals
    for ( pEntry = (struct syment *)FindFirstListObject( &ci->ListOfGlobals );
        pEntry != 0;
        pEntry = (struct syment *)FindNextListObject( &ci->ListOfGlobals ) ) {

        pNode = GetCurrentNode( &ci->ListOfGlobals );

        /* write it out */
        if ( fwrite( pEntry, 1, pNode->Size, pi->coff_file ) != pNode->Size ) {
            fprintf(stderr,"\nFile error writing global symbols ...(disk full?)");
            return;
        }
    }

    /* Specials .text, .bss, .data */

    for ( pEntry = (struct syment *)FindFirstListObject( &ci->ListOfSpecials );
        pEntry != 0;
        pEntry = (struct syment *)FindNextListObject( &ci->ListOfSpecials ) ) {

        pNode = GetCurrentNode( &ci->ListOfSpecials );

        /* write it out */
        if ( fwrite( pEntry, 1, pNode->Size, pi->coff_file ) != pNode->Size ) {
            fprintf(stderr,"\nFile error writing special symbols ...(disk full?)");
            return;
        }
    }

    // String Table
    for ( p = (char *)FindFirstListObject( &ci->ListOfStrings );
        p != 0;
        p = (char *)FindNextListObject( &ci->ListOfStrings ) ) {

        pNode = GetCurrentNode( &ci->ListOfStrings );

        /* write it out */
        if ( fwrite( p, 1, pNode->Size, pi->coff_file ) != pNode->Size ) {
            fprintf(stderr,"\nFile error writing strings data ...(disk full?)");
            return;
        }
    }

    return;
}

/****************************************************************************************/

void write_coff_eeprom( struct prog_info *pi, int address, unsigned char data){

    if ( !GET_ARG(pi->args, ARG_COFF) )
        return;

    /* Coff output keeps track of binary data in memory buffers */
    if ( ci->pEEPRomMemory ) {
        if ( address <= pi->device->eeprom_size ) {
            *(ci->pEEPRomMemory + address) = data;
            if ( address >= ci->MaxEepromAddress )
                ci->MaxEepromAddress = address;   /* keep high water mark */
        } else {
            pi->error_count++;
            fprintf(stderr, "Error: EEPROM address %d exceeds max range %d", address, pi->device->eeprom_size );
        }
    }
}
/****************************************************************************************/

void write_coff_program( struct prog_info *pi, int address, unsigned int data){

    unsigned char *pByte;

    if ( !GET_ARG(pi->args, ARG_COFF) )
        return;

    /* Coff output keeps track of binary data in memory buffers, address is in bytes not words */
    if ( ci->pRomMemory ) {
/* JEG	if ( address <= pi->device->flash_size ) {  */  /* JEG 4-23-03 */
		if ( address <= pi->device->flash_size*2 ) {
            pByte = (unsigned char *)(ci->pRomMemory + address); /* point to low byte in memory */
            *pByte++ = (data & 0xff);   /* low byte */
            *pByte = ((data >> 8) & 0xff); /* high byte */

            if ( address >= ci->MaxRomAddress )
                ci->MaxRomAddress = address;   /* keep high water mark */
        } else {
            pi->error_count++;
/* JEG		fprintf(stderr, "Error: FLASH address %d exceeds max range %d", address, pi->device->flash_size ); */
			fprintf(stderr, "Error: FLASH address %d exceeds max range %d", address, pi->device->flash_size*2 );
        }
    }
}

/****************************************************************************************/

void close_coff_file(struct prog_info *pi, FILE *fp){

    /* close the output file */
    fclose( fp );
    pi->coff_file = 0;

    /* free all the internal memory buffers used by ci */

    FreeList( &ci->ListOfSectionHeaders );
    FreeList( &ci->ListOfRawData );
    FreeList( &ci->ListOfRelocations );
    FreeList( &ci->ListOfLineNumbers );
    FreeList(    &ci->ListOfSymbols    );
    FreeList( &ci->ListOfGlobals );
    FreeList( &ci->ListOfUndefined );
    FreeList( &ci->ListOfStrings );
    FreeList( &ci->ListOfTypes );
    FreeList( &ci->ListOfSplitLines );

    /* now free ci */
    free( ci );
    ci = 0;
}

/****************************************************************************************/

int parse_stabs( struct prog_info *pi, char *p ){

    int ok = True;
    int TypeCode, n;
    char *pString, *p2, *p3, *p4, *p5, *pType, *pEnd, *pp, *pJoined;


    if ( !GET_ARG(pi->args, ARG_COFF) || ( pi->pass == PASS_1 ) )
        return(True);

    /* stabs debugging information is in the form:
    .stabs "symbolic info string", HexorDecimalTypecode, parm3, parm4, parm5
    parm1, parm2, parm3 depend on typecode

    N_LSYM	0x80		local sym: name,,0,type,offset
    N_OPT	0x3c		compiler options
    N_SO	0x64		source file name: name,,0,0,address
    N_SOL	0x84		#included file name: name,,0,0,address
    N_FUN	0x24		procedure: name,,0,linenumber,address
    N_GSYM	0x20		global symbol: name,,0,type,0
    N_LCSYM	0x28		.lcomm symbol: name,,0,type,address
    N_STSYM	0x26		static symbol: name,,0,type,address
    N_RSYM	0x40		register sym: name,,0,type,register
    N_PSYM	0xa0		parameter: name,,0,type,offset

    */

    /* Look for multiple commands per line */

    /* .stabs "linktag:T19=s46next:20=*19,0,16;last:20,16,16;a:21=ar1;0;2;22=ar1;0;3;1,32,96;\\",128,0,0,0 */
    /* .stabs "b:23=ar1;0;4;24=ar1;0;5;2,128,240;;",128,0,0,0 */


    /* Look for continuation lines per line */

    /* Get String information as a token */
    /* Parse the tokens in the stabn line buffer */
    pString = get_next_token(p, TERM_DOUBLEQUOTE );     /* zap first doublequote */
    p2 = get_next_token(pString, TERM_DOUBLEQUOTE );    /* zap last doublequote */
    p2 = get_next_token(p2, TERM_COMMA );               /* zap comma */
    p3 = get_next_token(p2, TERM_COMMA );
    p4 = get_next_token(p3, TERM_COMMA );
    p5 = get_next_token(p4, TERM_COMMA );
    pEnd = get_next_token(p5, TERM_END ); /* zap CR LF, make ASCIIZ */

    if ( !pString || !p2 || !p3 || !p4 || !p5 )
        return( False );

    /* Check for split lines */
    n = strlen( pString );
    if ( ( pString[n - 1] == '\\' ) && (pString[n - 2] == '\\') ) {
        /* We have a continuation string here */
        pString[n - 2] = 0;
        n -= 2;
        if ( !(pp = (char *)AllocateListObject( &ci->ListOfSplitLines, n + 1 ))  ) {
            fprintf(stderr, "\nOut of memory allocating continuation line!");
            return( False );
        }
        strcpy( pp, pString ); /* loose the continuation characters */
        return(True);
    }
    if ( ci->ListOfSplitLines.TotalItems > 0 ) {
        /* Join lines together and process */
        if ( !(pJoined = calloc( 1, n + ci->ListOfSplitLines.TotalBytes ) ) ) {
            fprintf(stderr, "\nOut of memory joining continuation lines!");
            return( False );
        }
        for ( pp = (char *)FindFirstListObject( &ci->ListOfSplitLines );
            pp != 0;
            pp = (char *)FindNextListObject( &ci->ListOfSplitLines ) ) {

            strcat( pJoined, pp ); /* connect the lines */
        }
        strcat( pJoined, pString );
        FreeList( &ci->ListOfSplitLines );
        if ( !AddListObject( &ci->ListOfSplitLines, pJoined, n + ci->ListOfSplitLines.TotalBytes )  ) {
            fprintf(stderr, "\nUnable to add joined continuation line");
            return( False );
        }
        pString = pJoined;
    }


    if ( *p2 == '0' )
        TypeCode = atox(p2);    /* presume to be hex 0x */
    else
        TypeCode = atoi(p2);

    switch ( TypeCode ) {

    case N_OPT:     /* compiler options */
        break;      /* nothing used here */

    case N_SO:      /* source file name: name,,0,0,address */
        ok = stab_add_filename( pString, p5 );
        break;

    case N_GSYM:    /* global symbol: name,,0,type,0 */
        pType = get_next_token(pString, TERM_COLON );   /* separate at colon */
        ok = stab_add_global( pi, pString, pType );
        break;

    case N_FUN:     /* procedure: name,,0,linenumber,address */
        ok = stab_add_function( pi, pString, p5 );
        break;

    case N_LSYM:    /* local sym: name,,0,type,offset */
        /* pString, p2 = TypeCode, p3 = 0, p4 = 0, p5 = offset */
        pType = get_next_token(pString, TERM_COLON );   /* pType = symbol descriptor (character after the colon) */
        if ( *pType == 't')
            ok = stab_add_local_type( pString, ++pType );
        else if (*pType == 'T')
            ok = stab_add_tag_type( pString, ++pType );
        else
            ok = stab_add_local( pi, pString, pType, p5 );
        break;

    case N_RSYM:    /* Symbol:[P|r]type,0,size,register */
        pType = get_next_token(pString, TERM_COLON );   /* separate at colon */
        ok = stab_add_local_register( pi, pString, pType, p5 );
        break;

    case N_LCSYM:   /* .lcomm symbol: name,,0,type,address */
        ok = True;
        break;  /* ignore constants */

    case N_STSYM:   /* static symbol: name,,0,type,address */
        pType = get_next_token(pString, TERM_COLON );   /* separate at colon */
        ok = stab_add_static_symbol( pi, pString, pType, p5 );
        break;

    case N_PSYM:    /* parameter: name,,0,type,offset */
        pType = get_next_token(pString, TERM_COLON );   /* separate at colon */
        ok = stab_add_parameter_symbol( pi, pString, pType, p5 );
        break;

    case N_SOL:     /* #included file name: name,,0,0,address */
        ok = True;
        break;  /* ignore include files */

    default:
        ok = False;
    }

    if ( ci->ListOfSplitLines.TotalItems > 0 )
        FreeList( &ci->ListOfSplitLines );

    return( ok );
}
/****************************************************************************************/

int parse_stabn( struct prog_info *pi, char *p ){

    int ok = True;
    int TypeCode /* , LineNumber */, Level;
    char *p1, *p2, *p3, *p4, *pLabel, *pFunction, *pEnd;

    /* stabn debugging information is in the form:
    .stabn TypeCode, 0, parm1, parm2
    parm1 is level
    parm2 is Label-Function

    compiler currently produces the following TypeCodes:
    N_LBRAC	0xc0		left bracket: 0,,0,nesting level,address
    N_RBRAC	0xe0		right bracket: 0,,0,nesting level,address
    N_SLINE	0x44		src line: 0,,0,linenumber,address
    */

    if ( !GET_ARG(pi->args, ARG_COFF) || ( pi->pass == PASS_1 ) )
        return(True);

    /* Parse the tokens in the stabn line buffer */
    p1 = get_next_token(p, TERM_SPACE );
    p2 = get_next_token(p1, TERM_COMMA );
    p3 = get_next_token(p2, TERM_COMMA );
    p4 = get_next_token(p3, TERM_COMMA );
    pEnd = get_next_token(p4, TERM_END ); /* zap CR LF, make ASCIIZ */

    if ( !p1 || !p2 || !p3 || !p4 )
        return( False );

    /* first convert TypeCode to binary */
    if ( *p1 == '0' )
        TypeCode = atox(p1);    /* presume to be hex 0x */
    else
        TypeCode = atoi(p1);

    Level = atoi(p3);   /* line number or level */
    pLabel = p4;                /* Assembly label */
    pFunction = get_next_token( pLabel, TERM_DASH ); /* Function */

    switch ( TypeCode ) {
    case N_SLINE:           /* src line: 0,,0,linenumber,address */
        ok = stab_add_lineno( pi, Level, pLabel, pFunction );
        break;

    case N_LBRAC:           /* left bracket: 0,,0,nesting level,address */
        ok = stab_add_lbracket( pi, Level, pLabel, pFunction );
        break;

    case N_RBRAC:           /* right bracket: 0,,0,nesting level,address */
        ok = stab_add_rbracket( pi, Level, pLabel, pFunction );
        break;

    default:
        fprintf(stderr, "\nUnknown .stabn TypeCode = 0x%x", TypeCode );
        ok = False;
    }
    return( ok );
}

/****************************************************************************************/
int stab_add_lineno(  struct prog_info *pi, int LineNumber, char *pLabel, char *pFunction ){

    int Address;
    struct lineno *pln;
    struct syment *pEntry;
    union auxent *pAux;

    /* Allocate LineNumber Table entry and fill it in */
    pln = (struct lineno *)AllocateListObject(&ci->ListOfLineNumbers, sizeof(struct lineno)  );
    if ( !pln ) {
        fprintf(stderr, "\nOut of memory allocating lineno table for function %s", pFunction );
        return( False );
    }
    /* set value field to be address of label in bytes */
    if ( !get_symbol(pi, pLabel, &Address) ) {
        fprintf(stderr, "\nUnable to locate label %s", pLabel );
        return( False );
    }
    pln->l_addr.l_paddr = Address * 2; /* need byte quanities */

    /* Line number is relative to beginning of function, starts at 1 */
    if ( ci->FunctionStartLine == 0 ) {
        /* This line number is that same as the function start */
        ci->FunctionStartLine = LineNumber;
    }
    pln->l_lnno = LineNumber - ci->FunctionStartLine + 1;
    ci->CurrentSourceLine = LineNumber; /* keep track of source line for .eb .ef arrays */
    if ( ci->NeedLineNumberFixup ) {
        /* need to go into symbol table and fix last NeedLineNumberFixup entries */
        for ( pEntry = (struct syment *)FindLastListObject(&ci->ListOfSymbols);
            (pEntry != 0) && ( ci->NeedLineNumberFixup != 0);
            pEntry = (struct syment *)FindNextLastListObject(&ci->ListOfSymbols) ) {

            /* Fix up line number entries */
            if ( (pEntry->n_sclass == C_FCN ) || ( pEntry->n_sclass == C_BLOCK ) || ( pEntry->n_sclass == C_EXT) ) {
                pEntry++;
                pAux = (union auxent *)pEntry;
                pAux->x_sym.x_misc.x_lnsz.x_lnno = LineNumber;
                ci->NeedLineNumberFixup--; 
            }
        }
    }

    return(True);
}
/****************************************************************************************/

int stab_add_lbracket( struct prog_info *pi, int Level, char *pLabel, char *pFunction ){

    int Address;
    struct syment *pEntry;
    union auxent *pAux;
    //char *p;
    //struct lineno *pln;

    if ( !get_symbol(pi, pLabel, &Address) ) {
        fprintf(stderr, "\nUnable to locate label %s", pLabel );
        return( False );
    }

    /* Now create a .bb symbol table entry and aux entry too */
    pEntry = (struct syment *)AllocateTwoListObjects( &ci->ListOfSymbols,  sizeof(struct syment) * 2 );
    if ( !pEntry ) {
        fprintf(stderr, "\nOut of memory allocating symbol table entry for .bb %s", pLabel );
        return( False );
    }
    /* n_name */
    memset( pEntry->n_name, 0, 8 );
    strcpy( pEntry->n_name, ".bb" );

    pEntry->n_value = Address * 2;  /* bytes not words */
    pEntry->n_scnum = 1;    /* .text */
    pEntry->n_type = 0;
    pEntry->n_sclass = C_BLOCK;
    pEntry->n_numaux = 1;
    pEntry++;   /* point to aux entry */
    pAux = (union auxent *)pEntry; 
    pAux->x_sym.x_misc.x_lnsz.x_lnno = 0;   /* UNKNOWN - post process */
    pAux->x_sym.x_misc.x_lnsz.x_size = 0;   /* UNKNOWN - post process */
    ci->NeedLineNumberFixup++; /* once for .bb block */
    return(True);
}

/****************************************************************************************/
int stab_add_rbracket( struct prog_info *pi, int Level, char *pLabel, char *pFunction ){

    int Address;
    struct syment *pEntry;
    union auxent *pAux;
    //char *p;
    //struct lineno *pln;

    if ( !get_symbol(pi, pLabel, &Address) ) {
        fprintf(stderr, "\nUnable to locate label %s", pLabel );
        return( False );
    }

    /* Now create a .eb symbol table entry */
    pEntry = (struct syment *)AllocateTwoListObjects( &ci->ListOfSymbols,  sizeof(struct syment) * 2 );
    if ( !pEntry ) {
        fprintf(stderr, "\nOut of memory allocating symbol table entry for .eb %s", pLabel );
        return( False );
    }
    /* n_name */
    memset( pEntry->n_name, 0, 8 );
    strcpy( pEntry->n_name, ".eb" );
    pEntry->n_sclass = C_BLOCK;
    pEntry->n_value = Address * 2;  /* bytes not words */
    pEntry->n_scnum = 1;    /* .text */
    pEntry->n_type = 0;
    pEntry->n_numaux = 1;
    pEntry++;   /* point to aux entry */
    pAux = (union auxent *)pEntry; 
    pAux->x_sym.x_misc.x_lnsz.x_lnno = ci->CurrentSourceLine;

    /* create an .ef if at level 0 */
    if ( Level == 0 ) {

        /* Now create a .ef symbol table entry */
        pEntry = (struct syment *)AllocateTwoListObjects( &ci->ListOfSymbols,  sizeof(struct syment) * 2 );
        if ( !pEntry ) {
            fprintf(stderr, "\nOut of memory allocating symbol table entry for .ef %s", pLabel );
            return( False );
        }
        /* n_name */
        memset( pEntry->n_name, 0, 8 );
        strcpy( pEntry->n_name, ".ef" );
        pEntry->n_sclass = C_FCN;
        pEntry->n_value = Address * 2;  /* bytes not words */
        pEntry->n_scnum = 1;    /* .text */
        pEntry->n_type = 0;
        pEntry->n_numaux = 1;
        pEntry++;   /* point to aux entry */
        pAux = (union auxent *)pEntry; 
        pAux->x_sym.x_misc.x_lnsz.x_lnno = ci->CurrentSourceLine;
    }
    return(True);
}

/****************************************************************************************/
int stab_add_filename( char *pName, char *pLabel ){

    int ok, n;
    struct syment *pEntry;
    union auxent *pAux;
    char *p;

    /* if( pLabel == "Ltext0" ) then beginning of .text, pName = cwd, next pName = file */

    /* if( pLabel == "Letext" ) then end of .text , pName == NULL */

    /* we only need the one not ending in Slash */
    ok = True;
    n = strlen(pName);
    if ( n > 0 ) {
        if ( ( pName[ n - 1] == '\\') || (pName[ n - 1] == '/') )
            return(True); /* ignore */
    } else
        return(True);


    /* allocate entry in symbol table list */
    pEntry = (struct syment *)AllocateTwoListObjects(
                                                    &ci->ListOfSymbols,  sizeof(struct syment) * 2 ); /* aux entry too */
    if ( !pEntry ) {
        fprintf(stderr, "\nOut of memory allocating symbol table entry for global %s", pName );
        return( False );
    }
    /* n_name */
    memset( pEntry->n_name, 0, 8 );
    strcpy( pEntry->n_name, ".file" );
    /* n_value is determined after processing done UNKNOWN - post process */
    /* The value of that symbol equals the symbol table entry index of the next .file symbol or .global */
    /* post process */
    pEntry->n_scnum = N_DEBUG;
    pEntry->n_sclass = C_FILE;
    pEntry->n_numaux = 1;
    pEntry++;   /* point to aux entry */
    pAux = (union auxent *)pEntry; 

    /* Add Label name to symbol table */
    if ( n <= FILNMLEN ) {
        /* inline filename */
        memset( pAux->x_file.x_fname, 0, FILNMLEN );
        strncpy( pAux->x_file.x_fname, pName, n ); /* might not be zero terminated */
    } else {
        pAux->x_file.x_n.x_zeroes = 0;  /* symbol name is in string table */
        pAux->x_file.x_n.x_offset = ci->ListOfStrings.TotalBytes;

        /* add to string table */
        p = (char *)AllocateListObject( &ci->ListOfStrings,  n + 1 );
        if ( !p ) {
            fprintf(stderr, "\nOut of memory allocating string table space!");
            return( False );
        }
        strcpy( p, pName );
    }
    return( ok );
}

/****************************************************************************************/
int stab_add_function( struct prog_info *pi, char *pName, char *pLabel ){

    int n, Address;
    unsigned short CoffType, Type;
    struct syment *pEntry;
    char *pType;
    struct lineno *pln;
    union auxent *pAux;
    int SymbolIndex;

    pType = get_next_token(pName, TERM_COLON ); /* pType = symbol descriptor (character after the colon) */
    Type = atoi(pType + 1);     /* skip past F, predefined variable type */
    if ( (CoffType = GetCoffType( Type )) == 0 ) {
        fprintf(stderr, "\nUnrecognized return type found for function %s = %d", pName, Type );
        return(False);
    }
    /* Get Current Symbol Index, Allocate Symbol Table entry and fill it in */
    SymbolIndex = ci->ListOfSymbols.TotalItems;
    pEntry = (struct syment *)AllocateTwoListObjects( &ci->ListOfSymbols, sizeof(struct syment) * 2 );
    if ( !pEntry ) {
        fprintf(stderr, "\nOut of memory allocating symbol table entry for function %s", pName );
        return( False );
    }
    if ( (n = AddNameToEntry( pName, pEntry )) == 0 ) {
        fprintf(stderr,"\nOut of memory adding local %s to string table", pName );
    }
    if ( !get_symbol(pi, pLabel, &Address) ) {
        fprintf(stderr, "\nUnable to locate function %s", pName );
        return( False );
    }
    pEntry->n_value = Address * 2;  /* convert words to bytes */
    pEntry->n_scnum = 2;    /* .bss */
    if ( (CoffType = GetCoffType( Type )) == 0 ) {
        fprintf(stderr, "\nUnrecognized type found for function %s = %d", pName, Type );
        return(False);
    }
    pEntry->n_type = (unsigned short)(CoffType | (DT_FCN << 4));
    pEntry->n_sclass = C_EXT;
    pEntry->n_numaux = 1;
    pEntry++;   /* point to aux entry */
    pAux = (union auxent *)pEntry; 
    pAux->x_sym.x_tagndx = SymbolIndex + 1; /* point to the .bf entry index */
    // wrong!
    //	pAux->x_sym.x_misc.x_lnsz.x_lnno = ci->ListOfLineNumbers.TotalBytes; /* Relative Fixup point to where line numbers start */
    //	pAux->x_sym.x_misc.x_lnsz.x_size = 0; /* UNKNOWN till next function called */
    pAux->x_sym.x_misc.x_fsize = 0; /* unknown till end */
    pAux->x_sym.x_fcnary.x_fcn.x_lnnoptr = ci->ListOfLineNumbers.TotalBytes; /* relative offset to line number entry */
    pAux->x_sym.x_fcnary.x_fcn.x_endndx = 0; /* index to next entry */

    /* Now add function entry into the line number table */
    /* Allocate Symbol Table entry and fill it in */
    pln = (struct lineno *)AllocateListObject(&ci->ListOfLineNumbers, sizeof(struct lineno)  );
    if ( !pln ) {
        fprintf(stderr, "\nOut of memory allocating lineno table for function %s", pName );
        return( False );
    }
    pln->l_lnno = 0;
    pln->l_addr.l_symndx = SymbolIndex;

    /* Initialize the FunctionStartLine from the beginning of the function */
    ci->FunctionStartLine = 0;

    /* Allocate Symbol Table entry and fill it in */
    pEntry = (struct syment *)AllocateTwoListObjects( &ci->ListOfSymbols, sizeof(struct syment) * 2 );
    if ( !pEntry ) {
        fprintf(stderr, "\nOut of memory allocating symbol table entry .bf for function %s", pName );
        return( False );
    }
    memset( pEntry->n_name, 0, 8 );
    strcpy( pEntry->n_name, ".bf" );
    pEntry->n_value = Address * 2;  /* bytes not words */
    pEntry->n_scnum = 1;    /* .text */
    pEntry->n_type = 0;
    pEntry->n_sclass = C_FCN;
    pEntry->n_numaux = 1;
    pEntry++;   /* point to aux entry */
    pAux = (union auxent *)pEntry; 
    pAux->x_sym.x_misc.x_lnsz.x_lnno = 0;   /* UNKNOWN - post process */
    pAux->x_sym.x_misc.x_lnsz.x_size = 0;   /* UNKNOWN - post process */

    ci->NeedLineNumberFixup++; /* once for function C_EXT symbol */
    ci->NeedLineNumberFixup++; /* once for .bf block */
    return( True );
}
/****************************************************************************************/

int stab_add_global( struct prog_info *pi, char *pName, char *pType ){

    int n, Address, IsArray, SymbolIndex;
    unsigned short CoffType, Type;
    struct syment *pEntry;
    char *p;
    STABCOFFMAP *pMap;


    n = strlen( pName );    /* see if it's 8 bytes or less */
    Type = atoi(pType + 1);     /* skip past G, predefined variable type */
    if ( (CoffType = GetCoffType( Type )) == 0 ) {
        fprintf(stderr, "\nUnrecognized type found for global %s = %d", pName, Type );
        return(False);
    }
    pMap = (STABCOFFMAP *)GetCurrentListObject( &ci->ListOfTypes );

    SymbolIndex = ci->ListOfSymbols.TotalItems;
    /* Allocate Symbol Table entry and fill it in, Auxiliary table if its an array */
    if ( IsTypeArray( CoffType ) == True ) {
        IsArray = True;
        pEntry = (struct syment *)AllocateTwoListObjects( &ci->ListOfGlobals, sizeof(struct syment) * 2 );
    } else {
        IsArray = False;
        pEntry = (struct syment *)AllocateListObject( &ci->ListOfGlobals, sizeof(struct syment) );
    }
    if ( (n = AddNameToEntry( pName, pEntry )) == 0 ) {
        fprintf(stderr,"\nOut of memory adding local %s to string table", pName );
    }
    /* set value field to be address of label in bytes */
    /* add underscore to lookup label */
    if ( (p = calloc( 1, n + 2)) == 0 ) {
        fprintf(stderr,"\nOut of memory adding global %s", pName );
        return(False);
    }
    *p = '_';
    strcpy( p + 1, pName ); 
    if ( !get_symbol(pi, p, &Address) ) {
        fprintf(stderr, "\nUnable to locate global %s", p );
        free( p );
        return( False );
    }
    free( p );
    pEntry->n_value = Address;  /* already in bytes */
    if ( ci->GlobalStartAddress == -1 ) {
        ci->GlobalStartAddress = Address;
    }
    if ( Address < ci->GlobalStartAddress )
        ci->GlobalStartAddress = Address;
    if ( Address > ci->GlobalEndAddress )
        ci->GlobalEndAddress = Address;

    pEntry->n_scnum = 2;    /* .bss */
    pEntry->n_type = CoffType;
    pEntry->n_sclass = C_STAT;
    if ( IsArray == False )
        pEntry->n_numaux = 0;
    else {
        pEntry->n_numaux = 1;
        pEntry++;
        AddArrayAuxInfo( (union auxent *)pEntry, (unsigned short)SymbolIndex, pMap );
    }
    return( True );
}

/****************************************************************************************/
int stab_add_local(  struct prog_info *pi, char *pName, char *pType, char *pOffset  ){

    int n, Offset, IsArray;
    unsigned short CoffType, Type, SymbolIndex;
    struct syment *pEntry;
    STABCOFFMAP *pMap;

    n = strlen( pName );    /* see if it's 8 bytes or less */
    Type = atoi(pType);     /* predefined variable type */
    Offset = atoi(pOffset); /* offset in stack frame */
    if ( (CoffType = GetCoffType( Type )) == 0 ) {
        fprintf(stderr, "\nUnrecognized type found for local %s = %d", pName, Type );
        return(False);
    }
    pMap = (STABCOFFMAP *)GetCurrentListObject( &ci->ListOfTypes );
    SymbolIndex = ci->ListOfSymbols.TotalItems;
    /* Allocate Symbol Table entry and fill it in, Auxiliary table if its an array */
    if ( IsTypeArray( CoffType ) == True ) {
        IsArray = True;
        pEntry = (struct syment *)AllocateTwoListObjects( &ci->ListOfGlobals, sizeof(struct syment) * 2 );
    } else {
        IsArray = False;
        pEntry = (struct syment *)AllocateListObject( &ci->ListOfSymbols, sizeof(struct syment) );
    }
    if ( (n = AddNameToEntry( pName, pEntry )) == 0 ) {
        fprintf(stderr,"\nOut of memory adding local %s to string table", pName );
    }
    pEntry->n_type = CoffType;
    pEntry->n_sclass = C_AUTO;
    pEntry->n_scnum = N_ABS;
    pEntry->n_value = Offset + 1; /* Silly avr studio is set in its ways */
    if ( IsArray == False )
        pEntry->n_numaux = 0;
    else {
        pEntry->n_numaux = 1;
        pEntry++;
        AddArrayAuxInfo( (union auxent *)pEntry, SymbolIndex, pMap );
    }
    return( True );
}

/****************************************************************************************/
int stab_add_parameter_symbol( struct prog_info *pi, char *pName, char *pType, char *pOffset  ){

    int n, Offset;
    unsigned short CoffType, Type;
    struct syment *pEntry;

    n = strlen( pName );    /* see if it's 8 bytes or less */
    Type = atoi(pType);     /* predefined variable type */
    Offset = atoi(pOffset); /* offset in stack frame */
    if ( (CoffType = GetCoffType( Type )) == 0 ) {
        fprintf(stderr, "\nUnrecognized type found for %s = %d", pName, Type );
        return(False);
    }
    /* Allocate Symbol Table entry and fill it in */
    pEntry = (struct syment *)AllocateListObject( &ci->ListOfSymbols, sizeof(struct syment) );
    if ( (n = AddNameToEntry( pName, pEntry )) == 0 ) {
        fprintf(stderr,"\nOut of memory adding local %s to string table", pName );
    }
    pEntry->n_type = CoffType;
    pEntry->n_sclass = C_ARG;
    pEntry->n_scnum = N_ABS;
    pEntry->n_value = Offset;
    pEntry->n_numaux = 0;
    return( True );
}
/****************************************************************************************/
int stab_add_static_symbol(  struct prog_info *pi, char *pName, char *pType, char *pLabel  ){

    int n, Address;
    unsigned short CoffType, Type;
    struct syment *pEntry;

    n = strlen( pName );    /* see if it's 8 bytes or less */
    Type = atoi(pType + 1);     /* skip past S, predefined variable type */
    if ( (CoffType = GetCoffType( Type )) == 0 ) {
        fprintf(stderr, "\nUnrecognized type found for %s = %d", pName, Type );
        return(False);
    }
    /* Allocate Symbol Table entry and fill it in */
    pEntry = (struct syment *)AllocateListObject( &ci->ListOfSymbols, sizeof(struct syment) );
    if ( (n = AddNameToEntry( pName, pEntry )) == 0 ) {
        fprintf(stderr,"\nOut of memory adding local %s to string table", pName );
    }
    pEntry->n_type = CoffType;
    pEntry->n_sclass = C_STAT;
    pEntry->n_scnum = N_ABS;
    if ( !get_symbol(pi, pLabel, &Address) ) {
        fprintf(stderr, "\nUnable to locate label %s", pLabel );
        return( False );
    }
    pEntry->n_value = Address * 2;  /* find address of variable in bytes */
    pEntry->n_numaux = 0;
    return( True );
}
/****************************************************************************************/

int stab_add_local_register(  struct prog_info *pi, char *pName, char *pType, char *pRegister  ){

    int n, Register, Size;
    unsigned short CoffType, Type;
    struct syment *pEntry;

    n = strlen( pName );    /* see if it's 8 bytes or less */
    Type = (unsigned short)atoi(pType + 1);     /* skip past P, predefined variable type */
    Register = atoi(pRegister); /* offset in stack frame */
    if ( (CoffType = GetCoffType( Type )) == 0 ) {
        fprintf(stderr, "\nUnrecognized type found for %s = %d", pName, Type );
        return(False);
    }
    Size = GetCoffTypeSize( Type ); /* Silly requirement for avr studio */
    /* Allocate Symbol Table entry and fill it in */
    pEntry = (struct syment *)AllocateListObject( &ci->ListOfSymbols, sizeof(struct syment) );
    if ( (n = AddNameToEntry( pName, pEntry )) == 0 ) {
        fprintf(stderr,"\nOut of memory adding local %s to string table", pName );
        return(False);
    }
    pEntry->n_type = CoffType;
    //	if( (*pType == 'r') || (*pType == 'R') )
    //		pEntry->n_sclass = C_REG;
    //	else if( (*pType == 'p') || (*pType == 'P') )
    pEntry->n_sclass = C_REGPARM;   /* Silly avr studio only accepts this for registers */
    //	else{
    //	    fprintf(stderr,"\nUnknown register type -> %s", pType );
    //		return(False);
    //	}
    pEntry->n_scnum = N_ABS;
    pEntry->n_numaux = 0;
    if ( Size == 1 )
        pEntry->n_value = 0xffffff00 | Register; /* Silly requirement for avr studio */
    else if ( Size == 2 )
        pEntry->n_value = 0xffff0000 | ((Register + 1) << 8) | Register; /* Silly requirement for avr studio */
    else if ( Size == 4 )
        pEntry->n_value = ((Register + 3) << 24) | ((Register + 3) << 16) | ((Register + 1) << 8) | Register; /* Silly requirement for avr studio */
    else {
        fprintf(stderr,"\nUnknown register size (%d) and coff type (%d)", Size, CoffType );
        return(False);
    }
    return( True );
}

/****************************************************************************************/

int stab_add_local_type( char *pName, char *pType ){

    char *p;
    unsigned short StabType;

    /* .stabs "int:t1=r1;-128;127;",128,0,0,0 */
    /* .stabs ":t20=ar1;0;1;21=ar1;0;1;2",128,0,0,0 */
    /* pType-----^                                   */
    /* Stab Type - convert to Coff type at end (after inline assignments */
    if ( GetStabType( pType, &StabType, &p ) != True ) {
        fprintf(stderr,"\nInvalid tag type found in structure item -> %s", p);
        return(False);      
    }

    return(True);
}

/****************************************************************************************/

int GetStructUnionTagItem( char *p, char **pEnd, char **pName, unsigned short *pType, unsigned short *pBitOffset, unsigned short *pBitSize) {

    unsigned short StabType;
    /* Structure or Union Tag Item consists of -> name:type,bitoffset,bitsize; */

    /* name */
    *pName = p;
    while ( *p && (*p != ':') ) p++; // locate colon
    if ( *p != ':' ) {
      fprintf(stderr,"\nNo colon found in structure item -> %s", *pName);
      return(False);      
    }
    *p++ = 0; // Asciiz
 
    /* Stab Type - convert to Coff type at end (after inline assignments */
    if ( GetStabType( p, &StabType, &p ) != True ) {
        fprintf(stderr,"\nInvalid tag type found in structure item -> %s", p);
        return(False);      
    }

    /* BitSize */
    if ( *p != ',' ) {
        fprintf(stderr,"\nNo Bit size found in structure item -> %s", p );
        return(False);      
    }
    *pBitOffset = (unsigned short)atoi( ++p );
    while ( *p && (*p >= '0') && (*p <= '9') ) p++; // locate end of digits

    /* BitOffset */
    if ( *p != ',' ) {
        fprintf(stderr,"\nNo Bit offset found in structure item -> %s", p );
        return(False);      
    }
    *pBitSize =  (unsigned short)atoi( ++p );
    while ( *p && (*p >= '0') && (*p <= '9') ) p++; // locate end of digits

    /* Now convert stab type to COFF */
    if ( (*pType = GetCoffType( (unsigned short)StabType)) == 0 ) {
        fprintf(stderr,"\nNo COFF type found for stab type %d", StabType );
        return(   False);          
    }
    if ( *++p == ';' ) /* Now eat last semicolon(s) */
        p++;
    *pEnd = p;

    return( True );
}
/****************************************************************************************/

int GetEnumTagItem( char *p, char **pEnd, char **pEnumName, int *pEnumValue ) {

    /* Enum Tag Item consists of -> member1:value,member2:value2,; */
    *pEnumName = p;
    while ( *p && (*p != ':') ) p++; // locate colon
    if ( *p != ':' ) {
      fprintf(stderr,"\nNo colon found in enum item -> %s", *pEnumName);
      return(False);      
    }
    *p++ = 0; // Asciiz
    *pEnumValue = atoi(p);

    while ( *p && (*p >= '0') && (*p <= '9') ) p++; // locate end of digits
    if ( *p != ',' ) {
        fprintf(stderr,"\nNo comma found after enum value -> %s", p );
        return(False);      
    }
    if ( *++p ==';' )
        p++; /* eat last semicolon */
    *pEnd = p;
    return( True );
}

/****************************************************************************************/
int GetArrayType( char *p, char **pEnd, STABCOFFMAP *pMap, unsigned short *DerivedBits, int ExtraLevels ){

    int MinIndex, MaxIndex, Result, Size, i;
    char *pMinIndex, *pMaxIndex, *pType;
    unsigned short Type;

    Result = True;

    pMinIndex = pMaxIndex = pType = 0;    
    while ( *p && (*p != ';') ) p++;   /* find min index */
    pMinIndex = ++p;
    while ( *p && (*p != ';') ) p++;   /* find max index */
    pMaxIndex = ++p;
    while ( *p && (*p != ';') ) p++;   /* find type index */
    pType = ++p;

    /* bump the pointers to the digits */
    if ( !isdigit(*pMinIndex) )
        Result = False;
    if ( !isdigit(*pMaxIndex) )
        Result = False;
    if ( !isdigit(*pType) )
        Result = False;
    /* Is syntax ok ? */
    if ( Result != True ) {
        fprintf(stderr,"\nSyntax error on array parameters %s%s%s", pMinIndex, pMaxIndex, pType );
        return(False);
    }
    MinIndex = atoi(pMinIndex);
    MaxIndex = atoi(pMaxIndex);

    if ( GetStabType( p, &Type, &p ) != True )
        return(False);

    if ( !SetupDefinedType( Type, pMap, DerivedBits, ExtraLevels ) )
        return( False );

    /* Now update the size based on the indicies */
    Size = (MaxIndex - MinIndex) + 1;
    pMap->ByteSize *= Size;
    pMap->Line = ci->CurrentSourceLine;
    /* add the dimension information */
    for ( i = 5; i >= 0; i-- ) {
        if ( pMap->Dimensions[i] != 0 ) {
            i++;
            pMap->Dimensions[i] = Size;
            break;
        }
    }

    *pEnd = p;
    return(True);
}

/****************************************************************************************/
int GetStabType( char *p, unsigned short *pType, char **pEnd ) {

    STABCOFFMAP *pMap;
    int extra, ok;
    unsigned short derivedbits[6];
    unsigned short LStabType, RStabType;
    char *pHigh, *pLow;


    LStabType = atoi( p );
    while ( *p && (*p >= '0') && (*p <= '9') ) p++; // locate end of digits

    *pType = LStabType;

    if ( GetCoffType( LStabType ) != 0 ) {
        *pEnd = p;
        return(True);
    }
    if ( *p != '=' ) {
        fprintf(stderr, "\nSyntax error in type assignment -> %s", p );
        return(False);
    }
    p++;

    /* Allocate space for new internal type */
    if ( !(pMap = (STABCOFFMAP *)AllocateListObject(&ci->ListOfTypes, sizeof(STABCOFFMAP)) ) ) {
        fprintf(stderr, "\nOut of memory allocating type info!");
        return(False);
    }
    pMap->StabType = LStabType;

    /* process items to right of equals */
    for ( extra = 0; extra < 6; extra++ ) {

        if ( isdigit( *p ) ) {
            /* Finally found base type, try to terminate loop */
            GetStabType( p, &RStabType, &p );
            //			RStabType = atoi( p );
            while ( *p && (*p >= '0') && (*p <= '9') ) p++; // locate end of digits
            if ( SetupDefinedType( RStabType, pMap, &derivedbits[0], extra ) != True )
                return( False );
            break;
        } else if ( *p == 'a' ) {
            derivedbits[extra] = DT_ARY;
            p++;
            /* Calculate size */
            /* Since type assignment will be made we need to set extra bits here */
            extra++;
            /* =ar1;MinIndex;MaxIndex;BaseType */
            if ( GetArrayType( p, &p, pMap, &derivedbits[0], extra  ) != True )
                return(False);
            break;

        } else if ( *p == 'f' ) {
            derivedbits[extra] = DT_FCN;
            p++;
        } else if ( *p == '*' ) {
            derivedbits[extra] = DT_PTR;
            p++;
        } else if ( *p == 'r' ) {
            //			if( LStabType < 15 )
            //				ok = GetInternalType( pString, pMap ); /* internal types not yet installed */
            //			else
            while ( *p && (*p != ';' ) ) p++;
            pLow = p++;
            while ( *p && (*p != ';' ) ) p++;
            pHigh = p++;
            ok = GetSubRangeType( LStabType, pMap, pLow, pHigh );
            if ( ok != True )
                return(False);
            while ( *p && (*p != ';' ) ) p++; /* find end of range */
            p++;
            break;
        } else {
            fprintf(stderr, "\nUnrecognized Type modifier %c!", *p );
            return(False);
        }
    }
    *pEnd = p; /* Update return pointer */

    return(True);
}


/****************************************************************************************/
int stab_add_tag_type( char *pName, char *pString ){

    int SymbolIndex, StabType, TotalSize, n, EnumValue;
    unsigned short TagType, ItemType, BitOffset, BitSize;
    char *p;
    struct syment* pEntry;
    union auxent *pAux;
    STABCOFFMAP *pMap;

    /* We arrived here due to :T defining either a structure, union or enumeration */
    /* store the basic type as for internals and emit coff structures for debugging */
    /* .stabs "stag:T17=s2i:1,0,8;c:2,8,8;;",128,0,0,0 */
    /* .stabs "2:T18=u2a:2,0,8;b:1,0,8;c:6,0,16;;",128,0,0,0 */
    /* .stabs "1:T19=eenum1:1,enum2:2,enum3:3,;",128,0,0,0 */
    /* we don't care about the name */


    /* check for bogus errors */
    if ( !pName || !pString ) {
        fprintf(stderr,"\nInvalid .stabs type format - no information!");
        return(False);
    }

    p = pString;
    /* Stab Type - convert to Coff type at end (after inline assignments */
    if ( (StabType = (unsigned short)atoi(p)) == 0 ) {
        fprintf(stderr,"\nInvalid .stabs type format - no information! - > %s", p );
        return(False);
    }
    while ( *p && (*p >= '0') && (*p <= '9') ) p++; // locate end of digits
    if ( *p != '=' ) {
        fprintf(stderr,"\nInvalid .stabs type format - no equals - > %s", p );
        return(False);
    }
    SymbolIndex = ci->ListOfSymbols.TotalItems;
    if (  ( pEntry = (struct syment*)AllocateTwoListObjects( &ci->ListOfGlobals, sizeof(struct syment) * 2 ) ) == 0 ) {
        fprintf(stderr, "\nOut of memory allocating symbol tag entries");
        return(False);
    }
    /* Prepare Tag Header */
    if ( (n = AddNameToEntry( pName, pEntry )) == 0 ) {
        fprintf(stderr,"\nOut of memory adding local %s to string table", pString );
        return(False);
    }
    if ( !(pMap = (STABCOFFMAP *)AllocateListObject(&ci->ListOfTypes, sizeof(STABCOFFMAP)) ) ) {
        fprintf(stderr, "\nOut of memory allocating type info!");
        return(False);
    }
    pMap->StabType = StabType;
    pEntry->n_value = 0;
    pEntry->n_scnum = N_DEBUG;
    pEntry->n_numaux = 1;
    if ( *++p == 's' ) {
        TagType = pEntry->n_type = pMap->CoffType = T_STRUCT;
        pEntry->n_sclass = C_STRTAG;
        TotalSize = (unsigned short)atoi(++p);
    } else if ( *p == 'u' ) {
        TagType = pEntry->n_type = pMap->CoffType = T_UNION;
        pEntry->n_sclass = C_UNTAG;
        TotalSize = (unsigned short)atoi(++p);
    } else if ( *p == 'e' ) {
        TagType = pEntry->n_type = pMap->CoffType = T_ENUM;
        pEntry->n_sclass = C_ENTAG;
        TotalSize = FundamentalTypes[T_INT].Size; /* use size of int for enums */
    } else {
        fprintf(stderr,"\nUnknown tag type -> %s", p );
        return(False);
    }
    while ( *p && (*p >= '0') && (*p <= '9') ) p++; // locate end of digits
    pEntry++;   /* point to aux entry */
    pAux = (union auxent *)pEntry; 
    pAux->x_sym.x_tagndx = SymbolIndex; 
    pAux->x_sym.x_misc.x_lnsz.x_size = TotalSize;

    /* update our local knowledge of tag type */
    pMap->CoffType = TagType;
    pMap->ByteSize = TotalSize;
    pMap->Line = ci->CurrentSourceLine;

    /* Process the items until the end of the line */
    while ( *pName ) {

        if (  ( pEntry = (struct syment*)AllocateTwoListObjects( &ci->ListOfGlobals, sizeof(struct syment) * 2 ) ) == 0 ) {
            fprintf(stderr, "\nOut of memory allocating symbol tag member entries");
            return(False);
        }

        if ( TagType == T_STRUCT ) {
            if ( GetStructUnionTagItem( p, &p, &pName, &ItemType, &BitOffset, &BitSize) != True ) {
                return(False);
            }
            pEntry->n_value = BitOffset/8;
            pEntry->n_type = ItemType;
            pEntry->n_sclass = C_MOS;
        } else if ( TagType == T_UNION ) {
            if ( GetStructUnionTagItem(  p, &p, &pName, &ItemType, &BitOffset, &BitSize) != True ) {
                return(False);
            }
            pEntry->n_value = BitOffset/8;
            pEntry->n_type = ItemType;
            pEntry->n_sclass = C_MOU;
        } else { /* T_ENUM */
            if ( GetEnumTagItem( p, &p, &pName, &EnumValue ) != True ) {
                return(False);
            }
            pEntry->n_value = EnumValue;
            pEntry->n_type = TotalSize;
            pEntry->n_sclass = C_MOE;
        }

        /* Prepare Common Tag Header items */
        if ( (n = AddNameToEntry( pName, pEntry )) == 0 ) {
            fprintf(stderr,"\nOut of memory adding local %s to string table", pString );
            return(False);
        }
        pEntry->n_scnum = N_ABS;
        pEntry->n_numaux = 1;
        pEntry++;   /* point to aux entry */
        pAux = (union auxent *)pEntry; 
        pAux->x_sym.x_tagndx = SymbolIndex; 
        pAux->x_sym.x_misc.x_lnsz.x_size = TotalSize;
        pName = p;
    }

    /* End of Structures/Unions/Enumberations */
    if (  ( pEntry = (struct syment*)AllocateTwoListObjects( &ci->ListOfGlobals, sizeof(struct syment) * 2 ) ) == 0 ) {
        fprintf(stderr, "\nOut of memory allocating special headers for structure!");
        return(False);
    }
    strcpy( pEntry->n_name, ".eos" );
    pEntry->n_value = TotalSize;
    pEntry->n_scnum = N_ABS;
    pEntry->n_type = 0;
    pEntry->n_sclass = C_EOS;
    pEntry->n_numaux = 1;
    pEntry++;   /* point to aux entry */
    pAux = (union auxent *)pEntry; 
    pAux->x_sym.x_tagndx = SymbolIndex; /* point to the .bf entry index */
    pAux->x_sym.x_misc.x_lnsz.x_size = TotalSize;

    return(True);
}

/****************************************************************************************/
int SetupDefinedType( unsigned short Type, STABCOFFMAP *pMap, unsigned short *DerivedBits, int ExtraLevels ){

    int i, Dlimit, Dstart;
    unsigned short StabType;

    StabType = pMap->StabType; /* save the new type we found earlier */
    if ( CopyStabCoffMap( Type, pMap ) != True ) {
        fprintf(stderr, "\nCould not find defined type %d", Type );
        return(False);
    }
    pMap->StabType = StabType; /* save the new type we found earlier */

    /* Determine existing derived types for base class */
    for ( i = 0; i < 6; i++ ) {
        if ( (pMap->CoffType & ( 3 << (4 + i + i))) == 0 )
            break;
    }
    Dstart = i;
    Dlimit = i + ExtraLevels;
    if ( (Dlimit) >= 6 ) {
        fprintf(stderr, "\nStab Type %d has too many derived (%d) types!", pMap->StabType, Dlimit );
        return(False);
    }
    /* Add the new derived levels */
    for ( ; i < Dlimit; i++ ) {
        pMap->CoffType |= ( ( DerivedBits[i - Dstart] & 3) << (4 + i + i) ); /* add in the derived bits */
    }
    return(True);
}

/****************************************************************************************/
int GetArrayDefinitions( STABCOFFMAP *pMap , char *pMinIndex, char *pMaxIndex, char *pType, unsigned short *DerivedBits, int ExtraLevels ){

    int MinIndex, MaxIndex, Result, Size, i;
    unsigned short Type;

    Result = True;
    if ( (*pMinIndex != ';') || (*pMaxIndex != ';') || (*pType != ';') )
        Result = False;
    /* bump the pointers to the digits */
    pMinIndex++;
    if ( !isdigit(*pMinIndex) )
        Result = False;
    pMaxIndex++;
    if ( !isdigit(*pMaxIndex) )
        Result = False;
    pType++;
    if ( !isdigit(*pType) )
        Result = False;
    /* Is syntax ok ? */
    if ( Result != True ) {
        fprintf(stderr,"\nSyntax error on array parameters %s%s%s", pMinIndex, pMaxIndex, pType );
        return(False);
    }
    MinIndex = atoi(pMinIndex);
    MaxIndex = atoi(pMaxIndex);
    Type = (unsigned short)atoi(pType);
    if (    SetupDefinedType(    Type,    pMap,    DerivedBits,    ExtraLevels    )    !=    True    )
        return( False );
    /* Now update the size based on the indicies */
    Size = (MaxIndex - MinIndex) + 1;
    pMap->ByteSize *= Size;
    pMap->Line = ci->CurrentSourceLine;
    /* add the dimension information */
    for ( i = 5; i >= 0; i-- ) {
        if ( pMap->Dimensions[i] != 0 ) {
            i++;
            pMap->Dimensions[i] = Size;
            break;
        }
    }
    return(True);
}

/****************************************************************************************/

int GetInternalType( char *pName, STABCOFFMAP *pMap ){

    int n, found, i;

    if ( !pName ) {
        return(False);
    }

    found = False;
    n = strlen(pName);
    /* Find out if it is a local type */
    for (i = 0; FundamentalTypes[i].pString != 0; i++) {
        if ( !strncmp(pName, FundamentalTypes[i].pString, n) ) {
            /* found an internal type */
            pMap->CoffType = FundamentalTypes[i].Type;
            pMap->ByteSize = FundamentalTypes[i].Size;
            found = True;
        }
    }
    return(found);
}

/****************************************************************************************/
int GetSubRangeType( unsigned short Type, STABCOFFMAP *pMap , char *pLow, char *pHigh ){

    int Result, i;
    long High, Low;
    unsigned long Test;

    Result = True;
    if ( (*pLow != ';') || (*pHigh != ';') || (Type <= 0) )
        Result = False;

    /* Is syntax ok ? */
    if ( Result != True ) {
        fprintf(stderr,"\nSyntax error on sub range parameters!" );
        return(False);
    }
    Low = atol(++pLow);
    High = atol(++pHigh);

    /* Special handling of type void */
    if ( (Low == 0) && (High == 0) ) {
        /* Declare type void */
        pMap->ByteSize =0;
        pMap->CoffType = T_VOID;
        pMap->Line = ci->CurrentSourceLine;
        return(True);
    }

    if ( (pMap->CoffType = GetCoffType( Type )) != 0 ) {
        pMap->ByteSize = GetCoffTypeSize( Type );
    } else {
        /* Try to base everything off integer */
        pMap->ByteSize = FundamentalTypes[T_INT].Size;
    }

    /* Now calculate the byte size */
    if ( High == 0 ) {
        pMap->ByteSize = (unsigned short)Low; /* floating point */
    } else {
        if ( Low == 0 ) {
            /* Unsigned */
            Test = (unsigned long)High;
        } else if ( Low < 0 ) {
            /* signed */
            Test = (unsigned long)High << 1;
        } else {
            if ( Low <= High )
                Test = (unsigned long)High;
            else
                Test = (unsigned long)Low;
        }
        if ( pMap->ByteSize == 0 ) {
            fprintf(stderr,"\nType Range Error 1, need previous type %d size!", pMap->CoffType );
            return(False);
        }
        for ( i = 0; i < sizeof(unsigned long); i++ ) {
            if ( !(Test & (0xff << (i * 8))) )
                break;
        }
        pMap->ByteSize = i; 
    }
    /* Now determine the best fit based on byte size, compare against IAR Compiler */
    if ( pMap->ByteSize == 1 ) {
        if ( Low < 0 )
            pMap->CoffType = T_CHAR;
        else
            pMap->CoffType = T_UCHAR;
    } else if ( pMap->ByteSize == 2 ) {
        if ( Low < 0 )
            pMap->CoffType = T_INT;
        else
            pMap->CoffType = T_UINT;
    } else if ( pMap->ByteSize == 4 ) {
        if ( Low == 0 )
            pMap->CoffType = T_FLOAT;
        if ( Low < 0 )
            pMap->CoffType = T_LONG;
        else
            pMap->CoffType = T_ULONG;
    } else {
        fprintf(stderr,"\nGetSubRangeType failure - byte size %d", pMap->ByteSize );
        return(False);
    }
    return(True);
}

/****************************************************************************************/
int CopyStabCoffMap( unsigned short StabType, STABCOFFMAP *pMap ){

    STABCOFFMAP *p;

    for ( p = FindFirstListObject( &ci->ListOfTypes ); p != 0; p = FindNextListObject( &ci->ListOfTypes) ) {
        if ( p->StabType == StabType ) {
            memcpy( pMap, p, sizeof(STABCOFFMAP) );
            return(True);
        }
    }
    return( False ); /* Nothing found */
}

/****************************************************************************************/
unsigned short GetCoffType( unsigned short StabType ){

    STABCOFFMAP *p;

    for ( p = FindFirstListObject( &ci->ListOfTypes ); p != 0; p = FindNextListObject( &ci->ListOfTypes) ) {
        if ( p->StabType == StabType )
            return( p->CoffType );
    }
    return( 0 ); /* Nothing found */
}

/****************************************************************************************/
unsigned short GetCoffTypeSize( unsigned short StabType ){

    STABCOFFMAP *p;

    for ( p = FindFirstListObject( &ci->ListOfTypes ); p != 0; p = FindNextListObject( &ci->ListOfTypes) ) {
        if ( p->StabType == StabType )
            return( p->ByteSize );
    }
    return( 0 ); /* Nothing found */
}


/****************************************************************************************/
int GetDigitLength( char *p ){

    int i;

    if ( p == 0 )
        return(0);

    for ( i = 0; (*p != 0) && ( *p >= '0' ) && ( *p <= '9' ); i++ );

    return( i );

}

/****************************************************************************************/
int GetStringDelimiters( char *pString, char **pTokens, int MaxTokens ){

    int i;
    char *p;

    p = pString;

    if ( !p )
        return( 0 );

    for ( i = 0; i < MaxTokens; i++ ) {
        while ( True ) {
            if ( (*p == ':') || (*p == ';') || (*p == '=') || (*p == ',') || (*p == '"') || (*p == 0 ) ) {
                *(pTokens + i) = p; /* Remember this location */
                p++;
                if ( *p == 0 )
                    return( i );
                break;
            }
            p++;
        }
    }
    return( i );
}

/****************************************************************************************/
int IsTypeArray( unsigned short CoffType ){

    int Result;

    Result = False;

    if ( (CoffType & (DT_ARY << 4 )) == (DT_ARY << 4 ) )
        Result = True;
    if ( (CoffType & (DT_ARY << 6 )) == (DT_ARY << 6 ) )
        Result = True;
    if ( (CoffType & (DT_ARY << 8 )) == (DT_ARY << 8 ) )
        Result = True;
    if ( (CoffType & (DT_ARY << 10 )) == (DT_ARY << 10 ) )
        Result = True;
    if ( (CoffType & (DT_ARY << 12 )) == (DT_ARY << 12 ) )
        Result = True;
    if ( (CoffType & (DT_ARY << 14 )) == (DT_ARY << 14 ) )
        Result = True;

    return(Result);
}

/****************************************************************************************/
void AddArrayAuxInfo( union auxent *pAux, unsigned short SymbolIndex, STABCOFFMAP *pMap ){

    int i;

    pAux->x_sym.x_tagndx = SymbolIndex; /* point to the .bf entry index */
    pAux->x_sym.x_misc.x_lnsz.x_lnno =  pMap->Line;
    pAux->x_sym.x_misc.x_lnsz.x_size = pMap->ByteSize;
    for ( i = 0; i < 4; i++ )
        pAux->x_sym.x_fcnary.x_ary.x_dimen[i] = pMap->Dimensions[i];
}

/****************************************************************************************/
int AddNameToEntry( char *pName, struct syment *pEntry ) {

    int n;
    char *p;

    n = strlen( pName );    /* see if it's 8 bytes or less */
    if ( n <= 8 ) {
        strncpy( pEntry->n_name, pName, 8 );
    } else {
        /* point to current offset in string table */
        pEntry->n_offset = ci->ListOfStrings.TotalBytes;        
        /* Allocate string table entry */
        if ( (p = (char *)AllocateListObject( &ci->ListOfStrings, n + 1 )) == 0 ) {
            return(0);
        }
        strcpy( p, pName );
    }
    return(n); /* return size of string */
}

/****************************************************************************************/

char *SkipPastDigits( char *p ){

    if ( !p )
        return(p);
/*	if ( *p == 0 );  */  /* JEG 5-01-03 */
	if ( *p == 0 )
    return(p);   /* This line s/b indented JEG */
    for ( p--; (*p >= '0') && (*p <= '9') && (*p != 0); p-- );
    return(p);
}

/****************************************************************************************/

/****************************************************************************************/
/****************************************************************************************/
/* List management routines */
/****************************************************************************************/
/****************************************************************************************/

/****************************************************************************************/

/****************************************************************************************/
void InitializeList( LISTNODEHEAD *pHead ){

    pHead->Node.Next = &pHead->Node;
    pHead->Node.Last = &pHead->Node;
    pHead->TotalBytes = 0;
    pHead->TotalItems = 0;
    pHead->current = &pHead->Node;
    return;
}

/****************************************************************************************/

void *AllocateTwoListObjects( LISTNODEHEAD *pHead, int size ){

    void *p;

    if ( (p = AllocateListObject( pHead, size ) ) )
        pHead->TotalItems++;   /* already incremented once in addtolist */
    return( p );
}

/****************************************************************************************/
void *AllocateListObject( LISTNODEHEAD *pHead, int size ){

    void *pObject;

    LISTNODE *pNode;

    if ( (pObject = calloc( 1, size )) != 0 ) {
        if ( !(pNode = AddListObject( pHead, pObject, size )) ) {
            free( pObject );
            pObject = 0;
        }
    }
    return( pObject );
}

/****************************************************************************************/
LISTNODE  *AddListObject(LISTNODEHEAD *pHead, void *pObject, int size ){

    LISTNODE *pNode;

    if ( (pNode = calloc( 1, sizeof(LISTNODE) )) != 0 ) {
        pNode->pObject = pObject;
        pNode->Size = size;
        pNode->FileNumber = ci->CurrentFileNumber;
        AddNodeToList( pHead, pNode );
    }
    return( pNode );
}

/****************************************************************************************/
LISTNODE *AllocateListNode( void *pObject, int size ){

    LISTNODE *pNew;

    if ( (pNew = calloc( 1, sizeof( LISTNODE ) ) ) != 0 ) {
        /* Then we initialize the node */
        pNew->pObject = pObject;
        pNew->Size = size;
        pNew->FileNumber = ci->CurrentFileNumber;
    }
    return(pNew);
}

/****************************************************************************************/
void AddNodeToList( LISTNODEHEAD *pHead, LISTNODE *pNode ){

    LISTNODE *p;

    p = &pHead->Node;

    pNode->Next = p->Last->Next;
    p->Last->Next = pNode;
    pNode->Last = p->Last;
    p->Last = pNode;

    /* and update current size of data contained in the list */
    pHead->TotalBytes += pNode->Size;
    pHead->TotalItems++;
}

/****************************************************************************************/
void RemoveNodeFromList( LISTNODEHEAD *pHead, LISTNODE *pNode ){

    pNode->Last->Next = pNode->Next;
    pNode->Next->Last = pNode->Last;

    pHead->TotalBytes -= pNode->Size;
    pHead->TotalItems--;
}


/****************************************************************************************/
void *FindFirstListObject( LISTNODEHEAD *pHead ){

    if ( pHead->Node.Next == &pHead->Node )
        return(0);  /* Nothing in list */

    pHead->current = pHead->Node.Next;
    return( pHead->current->pObject );
}
/****************************************************************************************/
void *FindNextListObject( LISTNODEHEAD *pHead ){

    if ( pHead->current->Next == &pHead->Node )
        return( 0 );

    pHead->current = pHead->current->Next;

    return( pHead->current->pObject );
}
/****************************************************************************************/

LISTNODE *GetCurrentNode( LISTNODEHEAD *pHead ){

    return( pHead->current );
}

/****************************************************************************************/
void *GetCurrentListObject( LISTNODEHEAD *pHead ){

    return( pHead->current->pObject );
}


/****************************************************************************************/
void *FindLastListObject( LISTNODEHEAD *pHead ){

    if ( pHead->Node.Last == &pHead->Node )
        return(0);  /* Nothing in list */

    pHead->current = pHead->Node.Last;
    return( pHead->current->pObject );
}
/****************************************************************************************/
void *FindNextLastListObject( LISTNODEHEAD *pHead ){

    if ( pHead->current->Last == &pHead->Node )
        return( 0 );

    pHead->current = pHead->current->Last;

    return( pHead->current->pObject );
}

/****************************************************************************************/

void FreeList( LISTNODEHEAD *pHead ){

    LISTNODE *pNode;

    for ( pNode = pHead->Node.Last; pNode->Next != &pHead->Node; pNode = pHead->Node.Last ) {

        RemoveNodeFromList( pHead, pNode );
        free( pNode->pObject );
        free( pNode );
    } 
    pHead->TotalBytes = 0;
    pHead->TotalItems = 0;
    pHead->current = &pHead->Node;
}
/****************************************************************************************/



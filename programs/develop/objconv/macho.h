/****************************    macho.h    ****************************************
* Author:        Agner Fog
* Date created:  2007-01-06
* Last modified: 2008-05-23
* Project:       objconv
* Module:        macho.h
* Description:
* Header file for definition of data structures in 32 bit Mach-O object file.
* Also defines class MacSymbolTableBuilder
* Also defines structures for MacIntosh universal binaries
*
* Copyright 2006-2008 GNU General Public License http://www.gnu.org/licenses
* Parts (c) 2003 Apple public source license http://www.opensource.apple.com/apsl/
***********************************************************************************/
#ifndef MACHO_H
#define MACHO_H

/********************** FILE HEADER **********************/

struct MAC_header_32 {
	uint32	magic;		// mach magic number identifier
	uint32	cputype;	   // cpu specifier
	uint32	cpusubtype;	// machine specifier
	uint32	filetype;	// type of file
	uint32	ncmds;		// number of load commands
	uint32	sizeofcmds;	// the size of all the load commands
	uint32   flags;		// flags
};

struct MAC_header_64 {
	uint32	magic;		// mach magic number identifier
	uint32	cputype;	   // cpu specifier
	uint32	cpusubtype;	// machine specifier
	uint32	filetype;	// type of file
	uint32	ncmds;		// number of load commands
	uint32	sizeofcmds;	// the size of all the load commands
	uint32   flags;		// flags
   uint32   reserved;   // reserved for future use
};


// Constant for the magic field of the MAC_header (32-bit architectures)
#define	MAC_MAGIC_32   0xFEEDFACE  // 32 bit little endian
#define  MAC_MAGIC_64   0xFEEDFACF  // 64 bit little endian
#define	MAC_CIGAM_32   0xCEFAEDFE  // 32 bit big endian
#define  MAC_CIGAM_64   0xCFFAEDFE  // 64 bit big endian
#define	MAC_CIGAM_UNIV 0xBEBAFECA  // MacIntosh universal binary

// Constants for cputype
#define MAC_CPU_TYPE_I386      7
#define MAC_CPU_TYPE_X86_64    0x1000007
#define MAC_CPU_TYPE_ARM       12
#define MAC_CPU_TYPE_SPARC     14
#define MAC_CPU_TYPE_POWERPC   18
#define MAC_CPU_TYPE_POWERPC64 0x1000012

// Constants for cpusubtype
#define MAC_CPU_SUBTYPE_I386_ALL     3
#define MAC_CPU_SUBTYPE_X86_64_ALL   3
#define MAC_CPU_SUBTYPE_ARM_ALL      0
#define MAC_CPU_SUBTYPE_SPARC_ALL    0
#define MAC_CPU_SUBTYPE_POWERPC_ALL  0

// Constants for the filetype field of the MAC_header
#define	MAC_OBJECT   0x1		/* relocatable object file */
#define	MAC_EXECUTE	 0x2		/* demand paged executable file */
#define	MAC_FVMLIB	 0x3		/* fixed VM shared library file */
#define	MAC_CORE		 0x4		/* core file */
#define	MAC_PRELOAD	 0x5		/* preloaded executable file */
#define	MAC_DYLIB	 0x6		/* dynamicly bound shared library file*/
#define	MAC_DYLINKER 0x7	   /* dynamic link editor */
#define	MAC_BUNDLE	 0x8		/* dynamicly bound bundle file */

// Constants for the flags field of the MAC_header
#define MAC_NOUNDEFS                   0x1 // the object file has no undefined references, can be executed
#define MAC_INCRLINK                   0x2 // the object file is the output of an incremental link against a base file and can't be link edited again
#define MAC_DYLDLINK	                  0x4 // the object file is input for the dynamic linker and can't be staticly link edited again
#define MAC_BINDATLOAD                 0x8 // the object file's undefined references are bound by the dynamic linker when loaded.
#define MAC_PREBOUND	                 0x10 // the file has it's dynamic undefined references prebound.
#define MAC_SPLIT_SEGS                0x20 // the file has its read-only and read-write segments split
#define MAC_LAZY_INIT                 0x40 // the shared library init routine is to be run lazily via catching memory faults to its writeable segments (obsolete)
#define MAC_TWOLEVEL                  0x80 // the image is using two-level name space bindings
#define MAC_FORCE_FLAT               0x100 // the executable is forcing all images to use flat name space bindings
#define MAC_NOMULTIDEFS              0x200 // this umbrella guarantees no multiple defintions of symbols in its sub-images so the two-level namespace hints can always be used
#define MAC_NOFIXPREBINDING          0x400 // do not have dyld notify the prebinding agent about this executable
#define MAC_PREBINDABLE              0x800 // the binary is not prebound but can have its prebinding redone. only used when MH_PREBOUND is not set
#define MAC_ALLMODSBOUND            0x1000 // indicates that this binary binds to all two-level namespace modules of its dependent libraries. only used when MH_PREBINDABLE and MH_TWOLEVEL are both set
#define MAC_SUBSECTIONS_VIA_SYMBOLS 0x2000 // safe to divide up the sections into sub-sections via symbols for dead code stripping
#define MAC_CANONICAL               0x4000 // the binary has been canonicalized via the unprebind operation

//??
#define MAC_VM_PROT_NONE           0x00
#define MAC_VM_PROT_READ           0x01
#define MAC_VM_PROT_WRITE          0x02
#define MAC_VM_PROT_EXECUTE        0x04
#define MAC_VM_PROT_ALL            0x07

// Load commands
struct MAC_load_command {
	uint32 cmd;		         // type of load command
	uint32 cmdsize;	      // total size of command in bytes
};

// Constants for the cmd field of all load commands, the type
#define MAC_LC_REQ_DYLD  0x80000000 // This bit is added if unknown command cannot be ignored
#define MAC_LC_SEGMENT          0x1 /* segment of this file to be mapped */
#define MAC_LC_SYMTAB	        0x2	/* link-edit stab symbol table info */
#define MAC_LC_SYMSEG	        0x3	/* link-edit gdb symbol table info (obsolete) */
#define MAC_LC_THREAD	        0x4	/* thread */
#define MAC_LC_UNIXTHREAD	     0x5	/* unix thread (includes a stack) */
#define MAC_LC_LOADFVMLIB	     0x6	/* load a specified fixed VM shared library */
#define MAC_LC_IDFVMLIB	        0x7	/* fixed VM shared library identification */
#define MAC_LC_IDENT	           0x8	/* object identification info (obsolete) */
#define MAC_LC_FVMFILE	        0x9	/* fixed VM file inclusion (internal use) */
#define MAC_LC_PREPAGE          0xa /* prepage command (internal use) */
#define MAC_LC_DYSYMTAB	        0xb	/* dynamic link-edit symbol table info */
#define MAC_LC_LOAD_DYLIB	     0xc	/* load a dynamicly linked shared library */
#define MAC_LC_ID_DYLIB	        0xd	/* dynamicly linked shared lib identification */
#define MAC_LC_LOAD_DYLINKER    0xe	/* load a dynamic linker */
#define MAC_LC_ID_DYLINKER	     0xf	/* dynamic linker identification */
#define MAC_LC_PREBOUND_DYLIB  0x10	/* modules prebound for a dynamicly linked shared library */
#define MAC_LC_ROUTINES	       0x11	/* image routines */
#define MAC_LC_SUB_FRAMEWORK   0x12 /* sub framework */
#define MAC_LC_SUB_UMBRELLA    0x13 /* sub umbrella */
#define MAC_LC_SUB_CLIENT      0x14 /* sub client */
#define MAC_LC_SUB_LIBRARY     0x15 /* sub library */
#define MAC_LC_TWOLEVEL_HINTS  0x16 /* two-level namespace lookup hints */
#define MAC_LC_PREBIND_CKSUM   0x17 /* prebind checksum */
#define MAC_LC_LOAD_WEAK_DYLIB (0x18 | MAC_LC_REQ_DYLD)
#define MAC_LC_SEGMENT_64      0x19 /* 64-bit segment of this file to be mapped */
#define MAC_LC_ROUTINES_64     0x1a /* 64-bit image routines */
#define MAC_LC_UUID            0x1b /* the uuid */

/*
 * The segment load command indicates that a part of this file is to be
 * mapped into the task's address space.  The size of this segment in memory,
 * vmsize, maybe equal to or larger than the amount to map from this file,
 * filesize.  The file is mapped starting at fileoff to the beginning of
 * the segment in memory, vmaddr.  The rest of the memory of the segment,
 * if any, is allocated zero fill on demand.  The segment's maximum virtual
 * memory protection and initial virtual memory protection are specified
 * by the maxprot and initprot fields.  If the segment has sections then the
 * section structures directly follow the segment command and their size is
 * reflected in cmdsize.
 */
struct MAC_segment_command_32 {	/* for 32-bit architectures */
	uint32	cmd;		      /* LC_SEGMENT */
	uint32	cmdsize;	      /* includes sizeof section structs */
	char		segname[16];	/* segment name */
	uint32	vmaddr;		   /* memory address of this segment */
	uint32	vmsize;		   /* memory size of this segment */
	uint32	fileoff;	      /* file offset of this segment */
	uint32	filesize;	   /* amount to map from the file */
	uint32	maxprot;    	/* maximum VM protection */
	uint32	initprot;	   /* initial VM protection */
	uint32	nsects;		   /* number of sections in segment */
	uint32	flags;		   /* flags */
};

/*
 * The 64-bit segment load command indicates that a part of this file is to be
 * mapped into a 64-bit task's address space.  If the 64-bit segment has
 * sections then section_64 structures directly follow the 64-bit segment
 * command and their size is reflected in cmdsize.
 */
struct MAC_segment_command_64 {	/* for 64-bit architectures */
	uint32	cmd;		    /* LC_SEGMENT_64 */
	uint32	cmdsize;	    /* includes sizeof section_64 structs */
	char		segname[16]; /* segment name */
	uint64	vmaddr;		 /* memory address of this segment */
	uint64	vmsize;		 /* memory size of this segment */
	uint64	fileoff;	    /* file offset of this segment */
	uint64	filesize;	 /* amount to map from the file */
	uint32	maxprot;	    /* maximum VM protection */
	uint32	initprot;	 /* initial VM protection */
	uint32	nsects;		 /* number of sections in segment */
	uint32	flags;		 /* flags */
};


/* Constants for the flags field of the segment_command */
#define	MAC_SG_HIGHVM	0x1	// the file contents for this segment is for the high part of the 
                              // VM space, the low part is zero filled (for stacks in core files) 
#define	MAC_SG_FVMLIB	0x2	// this segment is the VM that is allocated by a fixed VM library, 
                              // for overlap checking in the link editor 
#define	MAC_SG_NORELOC	0x4	// this segment has nothing that was relocated in it and nothing 
                              // relocated to it, that is it maybe safely replaced without relocation

/*
 * A segment is made up of zero or more sections.  Non-MH_OBJECT files have
 * all of their segments with the proper sections in each, and padded to the
 * specified segment alignment when produced by the link editor.  The first
 * segment of a MH_EXECUTE and MH_FVMLIB format file contains the mach_header
 * and load commands of the object file before it's first section.  The zero
 * fill sections are always last in their segment (in all formats).  This
 * allows the zeroed segment padding to be mapped into memory where zero fill
 * sections might be. The gigabyte zero fill sections, those with the section
 * type S_GB_ZEROFILL, can only be in a segment with sections of this type.
 * These segments are then placed after all other segments.
 *
 * The MH_OBJECT format has all of it's sections in one segment for
 * compactness.  There is no padding to a specified segment boundary and the
 * mach_header and load commands are not part of the segment.
 *
 * Sections with the same section name, sectname, going into the same segment,
 * segname, are combined by the link editor.  The resulting section is aligned
 * to the maximum alignment of the combined sections and is the new section's
 * alignment.  The combined sections are aligned to their original alignment in
 * the combined section.  Any padded bytes to get the specified alignment are
 * zeroed.
 *
 * The format of the relocation entries referenced by the reloff and nreloc
 * fields of the section structure for mach object files is described in the
 * header file <reloc.h>.
 */
struct MAC_section_32 {	      /* for 32-bit architectures */
	char		sectname[16];	/* name of this section */
	char		segname[16];	/* segment this section goes in */
	uint32	addr;		      /* memory address of this section */
	uint32	size;		      /* size in bytes of this section */
	uint32	offset;	   	/* file offset of this section */
	uint32	align;		   /* section alignment (power of 2) */
	uint32	reloff;		   /* file offset of relocation entries */
	uint32	nreloc;		   /* number of relocation entries */
	uint32	flags;		   /* flags (section type and attributes)*/
	uint32	reserved1;	   /* reserved */
	uint32	reserved2;	   /* reserved */
};

struct MAC_section_64 {    /* for 64-bit architectures */
	char		sectname[16];	/* name of this section */
	char		segname[16];	/* segment this section goes in */
	uint64	addr;		      /* memory address of this section */
	uint64	size;		      /* size in bytes of this section */
	uint32	offset;		   /* file offset of this section */
	uint32	align;		   /* section alignment (power of 2) */
	uint32	reloff;		   /* file offset of relocation entries */
	uint32	nreloc;		   /* number of relocation entries */
	uint32	flags;		   /* flags (section type and attributes)*/
	uint32	reserved1;	   /* reserved (for offset or index) */
	uint32	reserved2;	   /* reserved (for count or sizeof) */
	uint32	reserved3;	   // reserved (Note: specified in loader.h, but not in MachORuntime.pdf)
};


/* The flags field of a section structure is separated into two parts a section
 * type and section attributes.  The section types are mutually exclusive (it
 * can only have one type) but the section attributes are not (it may have more
 * than one attribute).  */

#define MAC_SECTION_TYPE		    0x000000ff	/* 256 section types */
#define MAC_SECTION_ATTRIBUTES	 0xffffff00	/*  24 section attributes */

/* Constants for the type of a section */
#define	MAC_S_REGULAR		      0x0	 /* regular section */
#define	MAC_S_ZEROFILL		      0x1	 /* zero fill on demand section */
#define	MAC_S_CSTRING_LITERALS  0x2	 /* section with only literal C strings*/
#define	MAC_S_4BYTE_LITERALS	   0x3    /* section with only 4 byte literals */
#define	MAC_S_8BYTE_LITERALS	   0x4	 /* section with only 8 byte literals */
#define	MAC_S_LITERAL_POINTERS  0x5	 /* section with only pointers to literals */

/* For the two types of symbol pointers sections and the symbol stubs section
 * they have indirect symbol table entries.  For each of the entries in the
 * section the indirect symbol table entries, in corresponding order in the
 * indirect symbol table, start at the index stored in the reserved1 field
 * of the section structure.  Since the indirect symbol table entries
 * correspond to the entries in the section the number of indirect symbol table
 * entries is inferred from the size of the section divided by the size of the
 * entries in the section.  For symbol pointers sections the size of the entries
 * in the section is 4 bytes and for symbol stubs sections the byte size of the
 * stubs is stored in the reserved2 field of the section structure. */

#define  MAC_S_NON_LAZY_SYMBOL_POINTERS	0x6  // section with only non-lazy symbol pointers
#define  MAC_S_LAZY_SYMBOL_POINTERS		   0x7  // section with only lazy symbol pointers
#define  MAC_S_SYMBOL_STUBS	            0x8  // section with only symbol stubs, byte size of stub in the reserved2 field
#define  MAC_S_MOD_INIT_FUNC_POINTERS	   0x9  // section with only function pointers for initialization
#define  MAC_S_MOD_TERM_FUNC_POINTERS	   0xa  // section with only function pointers for termination
#define  MAC_S_COALESCED                  0xb  // section contains symbols that are to be coalesced
#define  MAC_S_GB_ZEROFILL                0xc  // zero fill on demand section that can be larger than 4 gigabytes
#define  MAC_S_INTERPOSING                0xd  // section with only pairs of function pointers for interposing
#define  MAC_S_16BYTE_LITERALS            0xe  // section with only 16 byte literals


// Constants for the section attributes part of the flags field of a section structure.

#define MAC_SECTION_ATTRIBUTES_USR	  0xff000000	/* User setable attributes */
#define MAC_S_ATTR_PURE_INSTRUCTIONS  0x80000000	/* section contains only true machine instructions */
#define MAC_S_ATTR_NO_TOC             0x40000000	/* section contains coalesced symbols that are not to be in a ranlib table of contents */
#define MAC_S_ATTR_STRIP_STATIC_SYMS  0x20000000	/* ok to strip static symbols in this section in files with the MH_DYLDLINK flag */
#define MAC_S_ATTR_NO_DEAD_STRIP      0x10000000	/* no dead stripping */
#define MAC_S_ATTR_LIVE_SUPPORT       0x08000000	/* blocks are live if they reference live blocks */
#define MAC_S_ATTR_SELF_MODIFYING_CODE 0x04000000	/* Used with i386 code stubs written on by dyld */
#define MAC_S_ATTR_DEBUG              0x02000000	/* a debug section */
#define MAC_SECTION_ATTRIBUTES_SYS	  0x00ffff00	/* system setable attributes */
#define MAC_S_ATTR_SOME_INSTRUCTIONS  0x00000400	/* section contains some machine instructions */
#define MAC_S_ATTR_EXT_RELOC	        0x00000200	/* section has external relocation entries */
#define MAC_S_ATTR_LOC_RELOC	        0x00000100	/* section has local relocation entries */


/* The names of segments and sections in them are mostly meaningless to the
 * link-editor.  But there are few things to support traditional UNIX
 * executables that require the link-editor and assembler to use some names
 * agreed upon by convention.
 *
 * The initial protection of the "__TEXT" segment has write protection turned
 * off (not writeable).
 *
 * The link-editor will allocate common symbols at the end of the "__common"
 * section in the "__DATA" segment.  It will create the section and segment
 * if needed. */

/* The currently known segment names and the section names in those segments */

#define	MAC_SEG_PAGEZERO	    "__PAGEZERO"      // the pagezero segment which has no protections and catches NULL references for MH_EXECUTE files
#define	MAC_SEG_TEXT	       "__TEXT"          // the tradition UNIX text segment
#define	MAC_SECT_TEXT	       "__text"          // the real text part of the text section no headers, and no padding 
#define  MAC_SECT_FVMLIB_INIT0 "__fvmlib_init0"  // the fvmlib initialization section 
#define  MAC_SECT_FVMLIB_INIT1 "__fvmlib_init1"  // the section following the fvmlib initialization section 
#define	MAC_SEG_DATA	       "__DATA"	       // the tradition UNIX data segment 
#define	MAC_SECT_DATA	       "__data"          // the real initialized data section no padding, no bss overlap 
#define	MAC_SECT_BSS	       "__bss"		       // the real uninitialized data section no padding 
#define  MAC_SECT_COMMON	    "__common"	       // the section common symbols are allocated in by the link editor
#define	MAC_SEG_OBJC	       "__OBJC"	       // objective-C runtime segment 
#define  MAC_SECT_OBJC_SYMBOLS "__symbol_table"	 // symbol table 
#define  MAC_SECT_OBJC_MODULES "__module_info"	 // module information 
#define  MAC_SECT_OBJC_STRINGS "__selector_strs" // string table 
#define  MAC_SECT_OBJC_REFS    "__selector_refs" // string table 
#define	MAC_SEG_ICON	       "__ICON"          // the NeXT icon segment 
#define	MAC_SECT_ICON_HEADER  "__header"        // the icon headers 
#define	MAC_SECT_ICON_TIFF    "__tiff"          // the icons in tiff format 
#define	MAC_SEG_LINKEDIT	    "__LINKEDIT"      // the segment containing all structs created and maintained by the link editor.  Created with -seglinkedit option to ld(1) for MH_EXECUTE and FVMLIB file types only
#define  MAC_SEG_UNIXSTACK	    "__UNIXSTACK"	    // the unix stack segment 
#define  MAC_SEG_IMPORT        "__IMPORT"        // the segment for the self (dyld) modifing code stubs that has read, write and execute permissions


/* The symtab_command contains the offsets and sizes of the link-edit 4.3BSD
 * "stab" style symbol table information as described in the header files
 * <nlist.h> and <stab.h>. */

struct MAC_symtab_command {
	uint32	cmd;		   /* LC_SYMTAB */
	uint32	cmdsize;	   /* sizeof(MAC_symtab_command) */
	uint32	symoff;		/* symbol table offset */
	uint32	nsyms;		/* number of symbol table entries */
	uint32	stroff;		/* string table offset */
	uint32	strsize;	   /* string table size in bytes */
};

/* This is the second set of the symbolic information which is used to support
 * the data structures for the dynamicly link editor.
 *
 * The original set of symbolic information in the symtab_command which contains
 * the symbol and string tables must also be present when this load command is
 * present.  When this load command is present the symbol table is organized
 * into three groups of symbols:
 *	local symbols (static and debugging symbols) - grouped by module
 *	defined external symbols - grouped by module (sorted by name if not lib)
 *	undefined external symbols (sorted by name)
 * In this load command there are offsets and counts to each of the three groups
 * of symbols.
 *
 * This load command contains a the offsets and sizes of the following new
 * symbolic information tables:
 *	table of contents
 *	module table
 *	reference symbol table
 *	indirect symbol table
 * The first three tables above (the table of contents, module table and
 * reference symbol table) are only present if the file is a dynamicly linked
 * shared library.  For executable and object modules, which are files
 * containing only one module, the information that would be in these three
 * tables is determined as follows:
 * 	table of contents - the defined external symbols are sorted by name
 *	module table - the file contains only one module so everything in the
 *		       file is part of the module.
 *	reference symbol table - is the defined and undefined external symbols
 *
 * For dynamicly linked shared library files this load command also contains
 * offsets and sizes to the pool of relocation entries for all sections
 * separated into two groups:
 *	external relocation entries
 *	local relocation entries
 * For executable and object modules the relocation entries continue to hang
 * off the section structures.  */

struct MAC_dysymtab_command {
    uint32 cmd;		/* LC_DYSYMTAB */
    uint32 cmdsize;	/* sizeof(struct dysymtab_command) */

    /* The symbols indicated by symoff and nsyms of the LC_SYMTAB load command
     * are grouped into the following three groups:
     *    local symbols (further grouped by the module they are from)
     *    defined external symbols (further grouped by the module they are from)
     *    undefined symbols
     *
     * The local symbols are used only for debugging.  The dynamic binding
     * process may have to use them to indicate to the debugger the local
     * symbols for a module that is being bound.
     *
     * The last two groups are used by the dynamic binding process to do the
     * binding (indirectly through the module table and the reference symbol
     * table when this is a dynamicly linked shared library file).    */

    uint32 ilocalsym;	// index to local symbols
    uint32 nlocalsym;	// number of local symbols 

    uint32 iextdefsym;	// index to externally defined symbols
    uint32 nextdefsym;	// number of externally defined symbols 

    uint32 iundefsym;	// index to undefined symbols
    uint32 nundefsym;	// number of undefined symbols

    /* For the dynamic binding process to find which module a symbol
     * is defined in the table of contents is used (analogous to the ranlib
     * structure in an archive) which maps defined external symbols to modules
     * they are defined in.  This exists only in a dynamicly linked shared
     * library file.  For executable and object modules the defined external
     * symbols are sorted by name and is use as the table of contents.     */

    uint32 tocoff;	/* file offset to table of contents */
    uint32 ntoc;		/* number of entries in table of contents */

    /* To support dynamic binding of "modules" (whole object files) the symbol
     * table must reflect the modules that the file was created from.  This is
     * done by having a module table that has indexes and counts into the merged
     * tables for each module.  The module structure that these two entries
     * refer to is described below.  This exists only in a dynamicly linked
     * shared library file.  For executable and object modules the file only
     * contains one module so everything in the file belongs to the module.     */

    uint32 modtaboff;	/* file offset to module table */
    uint32 nmodtab;	   /* number of module table entries */

    /* To support dynamic module binding the module structure for each module
     * indicates the external references (defined and undefined) each module
     * makes.  For each module there is an offset and a count into the
     * reference symbol table for the symbols that the module references.
     * This exists only in a dynamicly linked shared library file.  For
     * executable and object modules the defined external symbols and the
     * undefined external symbols indicates the external references.     */

    uint32 extrefsymoff;  /* offset to referenced symbol table */
    uint32 nextrefsyms;	  /* number of referenced symbol table entries */

    /* The sections that contain "symbol pointers" and "routine stubs" have
     * indexes and (implied counts based on the size of the section and fixed
     * size of the entry) into the "indirect symbol" table for each pointer
     * and stub.  For every section of these two types the index into the
     * indirect symbol table is stored in the section header in the field
     * reserved1.  An indirect symbol table entry is simply a 32bit index into
     * the symbol table to the symbol that the pointer or stub is referring to.
     * The indirect symbol table is ordered to match the entries in the section. */

    uint32 indirectsymoff; // file offset to the indirect symbol table
    uint32 nindirectsyms;  // number of indirect symbol table entries

    /* To support relocating an individual module in a library file quickly the
     * external relocation entries for each module in the library need to be
     * accessed efficiently.  Since the relocation entries can't be accessed
     * through the section headers for a library file they are separated into
     * groups of local and external entries further grouped by module.  In this
     * case the presents of this load command who's extreloff, nextrel,
     * locreloff and nlocrel fields are non-zero indicates that the relocation
     * entries of non-merged sections are not referenced through the section
     * structures (and the reloff and nreloc fields in the section headers are
     * set to zero).
     *
     * Since the relocation entries are not accessed through the section headers
     * this requires the r_address field to be something other than a section
     * offset to identify the item to be relocated.  In this case r_address is
     * set to the offset from the vmaddr of the first LC_SEGMENT command.
     *
     * The relocation entries are grouped by module and the module table
     * entries have indexes and counts into them for the group of external
     * relocation entries for that the module.
     *
     * For sections that are merged across modules there must not be any
     * remaining external relocation entries for them (for merged sections
     * remaining relocation entries must be local).     */

    uint32 extreloff;	/* offset to external relocation entries */
    uint32 nextrel;	   /* number of external relocation entries */

    /* All the local relocation entries are grouped together (they are not
     * grouped by their module since they are only used if the object is moved
     * from it staticly link edited address).     */

    uint32 locreloff;	/* offset to local relocation entries */
    uint32 nlocrel;	/* number of local relocation entries */

};	

/* An indirect symbol table entry is simply a 32bit index into the symbol table 
 * to the symbol that the pointer or stub is refering to.  Unless it is for a
 * non-lazy symbol pointer section for a defined symbol which strip(1) as 
 * removed.  In which case it has the value INDIRECT_SYMBOL_LOCAL.  If the
 * symbol was also absolute INDIRECT_SYMBOL_ABS is or'ed with that. */

#define MAC_INDIRECT_SYMBOL_LOCAL  0x80000000
#define MAC_INDIRECT_SYMBOL_ABS    0x40000000

// Relocation entries
/* Format of a relocation entry of a Mach-O file.  Modified from the 4.3BSD
 * format.  The modifications from the original format were changing the value
 * of the r_symbolnum field for "local" (r_extern == 0) relocation entries.
 * This modification is required to support symbols in an arbitrary number of
 * sections not just the three sections (text, data and bss) in a 4.3BSD file.
 * Also the last 4 bits have had the r_type tag added to them. */

#define R_SCATTERED 0x80000000	// mask to be applied to the r_address field of a relocation_info structure to tell that
                                 // is is really a scattered_relocation_info stucture

struct MAC_relocation_info {
   uint32  r_address;      // offset in the section to what is being relocated (source)
   uint32  r_symbolnum:24, // symbol table index (0-based) if r_extern == 1 or section number (1-based) if r_extern == 0
           r_pcrel:1,      // pc relative. The target address (inline) is already pc relative
           r_length:2,     // 0=byte, 1=word, 2=dword
           r_extern:1,     // r_extern = 1 for symbols in symbol table
           r_type:4;       // if not 0, machine specific relocation type
};                         // The inline value of the source is the target address (pc-relative
                           // or absolute) if r_extern = 0, or an addend if r_extern = 1.

struct MAC_scattered_relocation_info {
   uint32  r_address:24,   // offset in the section to what is being relocated (source)
           r_type:4,       // if not 0, machine specific relocation type
           r_length:2,     // 0=byte, 1=word, 2=dword, 3=qword
           r_pcrel:1,      // pc relative. The target address is already pc relative
           r_scattered:1;  // 1=scattered, 0=non-scattered (see above)
   int32   r_value;        // target address (without any offset added. The offset is stored inline in the source)
};

// 32-bit relocation types:
/* Relocation types used in a generic implementation.  Relocation entries for
 * normal things use the generic relocation as discribed above and their r_type
 * is GENERIC_RELOC_VANILLA (a value of zero).
 *
 * Another type of generic relocation, GENERIC_RELOC_SECTDIFF, is to support
 * the difference of two symbols defined in different sections.  That is the
 * expression "symbol1 - symbol2 + constant" is a relocatable expression when
 * both symbols are defined in some section.  For this type of relocation 
 * both relocations entries are scattered relocation entries.  The value of
 * symbol1 is stored in the first relocation entry's r_value field and the
 * value of symbol2 is stored in the pair's r_value field.
 *
 * A special case for a prebound lazy pointer is needed to be able to set the
 * value of the lazy pointer back to its non-prebound state.  This is done
 * using the GENERIC_RELOC_PB_LA_PTR r_type.  This is a scattered relocation
 * entry where the r_value field is the value of the lazy pointer not prebound. */

/* My interpretation (A Fog):
   32-bit: Objects are not addressed by their offset into the section but by 
   their "absolute" address. This "absolute" address has no reality. 
   It is the address that the object would have if the section was placed 
   at the address specified in the addr field of the section header. 
   Scattered:
   The first record, of type MAC32_RELOC_SECTDIFF or MAC32_RELOC_LOCAL_SECTDIFF
   contains the "absolute" address of a first reference point, let's call it ref1,
   in the r_value field. The second record, of type MAC32_RELOC_PAIR contains the 
   "absolute" address of a second reference point, ref2, in the r_value field.
   The inline value is the "absolute" address of the relocation target minus ref2.
   ref1 is often = target, but may be any label preceding the target. The linker
   has to add (ref1 - ref2) in image minus (ref1 - ref2) in object file to the
   inline value. The relocation source (the position of the inline field) is
   given in r_address in the first record, relative the the section.
   Non-scattered, absolute, r_extern = 1:
   r_symbolnum = symbol index (0-based)
   Non-scattered, absolute, r_extern = 0:
   r_symbolnum = section index, inline = absolute address of target?
   Non-scattered, r_pcrel = 1, r_extern = 1:
   r_symbolnum = symbol index (0-based)
   Inline = source absolute address - 4
   Non-scattered, r_pcrel = 1, r_extern = 0:
   r_symbolnum = section index, 
   inline = absolute address of target - absolute address of source - 4
*/

#define MAC32_RELOC_VANILLA        0   // A generic relocation entry for both addresses contained in data
                                       // and addresses contained in CPU instructions.
#define MAC32_RELOC_PAIR           1   // The second relocation entry of a pair. Only follows a GENERIC_RELOC_SECTDIFF
#define MAC32_RELOC_SECTDIFF       2   // A relocation entry for an item that contains the difference of
                                       // two section addresses. This is generally used for position-independent code generation.
#define MAC32_RELOC_PB_LA_PTR      3   // —Arelocation entry for a prebound lazy pointer. This is always
                                       // a scattered relocation entry. The r_value field contains the non-prebound value of the lazy pointer.
#define MAC32_RELOC_LOCAL_SECTDIFF 4   // SECTDIFF—Similar to GENERIC_RELOC_SECTDIFF except that this entry refers specifically to the address in this item. 
                                       // If the address is that of a globally visible coalesced symbol, this relocation entry does not change if the symbol is overridden. 
                                       // This is used to associate stack unwinding information with the object code this relocation entry describes.

// 64-bit relocation types:
// Scattered relocations are not used in 64-bit Mach-O.
// reloc.h says that references to local symbols are made by the nearest
// preceding public symbol + displacement, but my experiments show that
// local symbol records are used, which of course is easier.
// r_extern = 1 is used even for non-external symbols!
// The target address is not stored inline. The -4 offset for self-relative
// addresses is implicit, unlike in 32-bit Mach-O. If the difference 
// between source address and instruction pointer is e.g. -5, then the
// -4 is implicit, and the -1 is explicit!

#define MAC64_RELOC_UNSIGNED       0   // absolute address, 32 or 64 bits
#define MAC64_RELOC_SIGNED         1   // signed 32-bit displacement with implicit -4 addend
#define MAC64_RELOC_BRANCH         2   // same, used for CALL and JMP instructions
#define MAC64_RELOC_GOT_LOAD       3   // self-relative load of a GOT entry
#define MAC64_RELOC_GOT            4   // other GOT references
#define MAC64_RELOC_SUBTRACTOR     5   // must be followed by a X86_64_RELOC_UNSIGNED
#define MAC64_RELOC_SIGNED_1       6   // signed 32-bit displacement with implicit -4 addend and explicit -1 addend
#define MAC64_RELOC_SIGNED_2       7   // signed 32-bit displacement with implicit -4 addend and explicit -2 addend
#define MAC64_RELOC_SIGNED_4       8   // signed 32-bit displacement with implicit -4 addend and explicit -4 addend


// Symbol table entries
/* Format of a symbol table entry of a Mach-O file.  Modified from the BSD
 * format.  The modifications from the original format were changing n_other
 * (an unused field) to n_sect and the addition of the N_SECT type.  These
 * modifications are required to support symbols in an arbitrary number of
 * sections not just the three sections (text, data and bss) in a BSD file. */

struct MAC_nlist_32 {
   uint32  n_strx;   // index into the string table 
   uint8   n_type;   // type flag, see below 
   uint8   n_sect;   // section number or NO_SECT 
   int16   n_desc;   // see <mach-o/stab.h> 
   uint32  n_value;  // value of this symbol (or stab offset) 
};

struct MAC_nlist_64 {
   uint32  n_strx;   // index into the string table 
   uint8   n_type;   // type flag, see below 
   uint8   n_sect;   // section number or NO_SECT 
   int16   n_desc;   // see <mach-o/stab.h> 
   uint64  n_value;  // value of this symbol (or stab offset) 
};

/* Symbols with a index into the string table of zero are
 * defined to have a null, "", name.  */

/* The n_type field really contains three fields:
*      unsigned char N_STAB:3,
*                    N_PEXT:1,
*                    N_TYPE:3,
*                    N_EXT:1;
* which are used via the following masks. */

#define MAC_N_STAB  0xe0  /* if any of these bits set, a symbolic debugging entry */
#define MAC_N_PEXT  0x10  /* private external symbol bit */
#define MAC_N_TYPE  0x0e  /* mask for the type bits */
#define MAC_N_EXT   0x01  /* external symbol bit, set for external symbols */

/* Only symbolic debugging entries have some of the N_STAB bits set and if any
 * of these bits are set then it is a symbolic debugging entry (a stab).  In
 * which case then the values of the n_type field (the entire field) are given
 * in <mach-o/stab.h> */

// Values for N_TYPE bits of the n_type field.
#define MAC_N_UNDF  0x0   // undefined, n_sect == NO_SECT 
#define MAC_N_ABS   0x2   // absolute, n_sect == NO_SECT 
#define MAC_N_SECT  0xe   // defined in section number n_sect 
#define MAC_N_PBUD  0xc   // prebound undefined (defined in a dylib) 
#define MAC_N_INDR  0xa   // indirect 

/* If the type is MAC_N_INDR then the symbol is defined to be the same as another
 * symbol.  In this case the n_value field is an index into the string table
 * of the other symbol's name.  When the other symbol is defined then they both
 * take on the defined type and value. */

/* If the type is MAC_N_SECT then the n_sect field contains an ordinal of the
 * section the symbol is defined in.  The sections are numbered from 1 and 
 * refer to sections in order they appear in the load commands for the file
 * they are in.  This means the same ordinal may very well refer to different
 * sections in different files.
 *
 * The n_value field for all symbol table entries (including N_STAB's) gets
 * updated by the link editor based on the value of it's n_sect field and where
 * the section n_sect references gets relocated.  If the value of the n_sect 
 * field is NO_SECT then it's n_value field is not changed by the link editor. */

#define MAC_NO_SECT         0       // symbol is not in any section
#define MAC_MAX_SECT        255     // 1 thru 255 inclusive

/* Common symbols are represented by undefined (N_UNDF) external (N_EXT) types
 * who's values (n_value) are non-zero.  In which case the value of the n_value
 * field is the size (in bytes) of the common symbol.  The n_sect field is set
 * to NO_SECT. */

/* To support the lazy binding of undefined symbols in the dynamic link-editor,
 * the undefined symbols in the symbol table (the nlist structures) are marked
 * with the indication if the undefined reference is a lazy reference or
 * non-lazy reference.  If both a non-lazy reference and a lazy reference is
 * made to the same symbol the non-lazy reference takes precedence.  A reference
 * is lazy only when all references to that symbol are made through a symbol
 * pointer in a lazy symbol pointer section.
 *
 * The implementation of marking nlist structures in the symbol table for
 * undefined symbols will be to use some of the bits of the n_desc field as a
 * reference type.  The mask REFERENCE_TYPE will be applied to the n_desc field
 * of an nlist structure for an undefined symbol to determine the type of
 * undefined reference (lazy or non-lazy).
 *
 * The constants for the REFERENCE FLAGS are propagated to the reference table
 * in a shared library file.  In that case the constant for a defined symbol,
 * REFERENCE_FLAG_DEFINED, is also used. */

/* Reference type bits of the n_desc field of undefined symbols */
#define MAC_REF_TYPE                                  0xf
/* types of references */
#define MAC_REF_FLAG_UNDEFINED_NON_LAZY               0
#define MAC_REF_FLAG_UNDEFINED_LAZY                   1
#define MAC_REF_FLAG_DEFINED                          2
#define MAC_REF_FLAG_PRIVATE_DEFINED                  3
#define MAC_REF_FLAG_PRIVATE_UNDEFINED_NON_LAZY       4
#define MAC_REF_FLAG_PRIVATE_UNDEFINED_LAZY           5

/* To simplify stripping of objects that use are used with the dynamic link
 * editor, the static link editor marks the symbols defined an object that are
 * referenced by a dynamicly bound object (dynamic shared libraries, bundles).
 * With this marking strip knows not to strip these symbols. */

/* The non-reference type bits of the n_desc field for global symbols are
 * reserved for the dynamic link editor.  All of these bits must start out
 * zero in the object file. */


// Additional n_desc flags
#define MAC_REFERENCED_DYNAMICALLY 0x10  // Must be set for any defined symbol that is referenced by dynamic-loader APIs (such as dlsym and NSLookupSymbolInImage) and not ordinary
                                         // undefined symbol references. The strip tool uses this bit to avoid removing symbols that must exist: If the symbol has this bit set, strip does not strip it.

#define MAC_N_DESC_DISCARDED       0x20  // Sometimes used by the dynamic linker at runtime in a fully linked image. Do not set this bit in a fully linked image.
//#define MAC_N_DESC_DISCARDED 0x8000

#define MAC_N_NO_DEAD_STRIP        0x20  // When set in a relocatable object file (file type MH_OBJECT) on a defined symbol, 
                                         // indicates to the static linker to never dead-strip the symbol. (Note that the same bit (0x20) is used for two nonoverlapping purposes.)

#define MAC_N_WEAK_REF             0x40  // Indicates that this undefined symbol is aweak reference. If the dynamic linker cannot find a definition 
                                         // for this symbol, it sets the address of this symbol to 0. The static linker sets this symbol given the appropriate weak-linking flags.

#define MAC_N_WEAK_DEF             0x80  // Indicates that this symbol is a weak definition. If the static linker or the dynamic linker finds another 
                                         // (non-weak) definition for this symbol, theweak definition is ignored. Only symbols in a coalesced section (page 21) can be marked as a weak definition.

// Data structure used when sorting symbol table for Mach-O file in MacSymbolTableBuilder
template <class TMAC_nlist>
struct MacSymbolRecord : public TMAC_nlist {
   uint32 Name;                        // Index into MacSymbolTableBuilder::StringBuffer
   int OldIndex;                       // Old symbol index
};

// Class for building and storing symbol table, sorted or unsorted
template <class TMAC_nlist, class MInt>
class MacSymbolTableBuilder : public CMemoryBuffer {
   int sorted;                                   // Remember if list is sorted
   CMemoryBuffer StringBuffer;                   // Temporary storage of symbol names
public:
   MacSymbolTableBuilder();                      // Constructor
   void AddSymbol(int OldIndex, const char * name, int type, int Desc, int section, MInt value); // Add symbol to list
   void SortList();                              // Sort the list
   int TranslateIndex(int OldIndex);             // Translate old index to new index, after sorting
   void StoreList(CMemoryBuffer * SymbolTable, CMemoryBuffer * StringTable); // Store sorted list in buffers
   int Search(const char * name);                // Search for name. -1 if not found
   MacSymbolRecord<TMAC_nlist> & operator[] (uint32 i);      // Access member
};

// structures for MacIntosh universal binaries
struct MAC_UNIV_FAT_HEADER {           // File header for universal binary
   uint32 magic;                       // Magic number 0xCAFEBABE, big endian
   uint32 num_arch;                    // Number of members, big endian
};

struct MAC_UNIV_FAT_ARCH {             // Member pointer
   uint32 cputype;                     // cpu type
   uint32 cpusubtype;                  // cpu subtype
   uint32 offset;                      // file offset of member
   uint32 size;                        // size of member
   uint32 align;                       // alignment in file = 2^align
};

// Structure used for list of sections that have relocations during disassembly
struct MAC_SECT_WITH_RELOC {
   int32  Section;                     // Section index
   uint32 SectOffset;                  // File offset of section binary data
   uint32 NumReloc;                    // Number of relocations records for this section
   uint32 ReltabOffset;                // File offset of relocation table for this section
};

/********************** Strings **********************/
#define MAC_CONSTRUCTOR_NAME    "__mod_init_func"  // Name of constructors section


// Macros listing all word-size dependent structures, used as template parameter list
#define MACSTRUCTURES    TMAC_header,   TMAC_segment_command,   TMAC_section,   TMAC_nlist, MInt
#define MAC32STRUCTURES  MAC_header_32, MAC_segment_command_32, MAC_section_32, MAC_nlist_32, int32
#define MAC64STRUCTURES  MAC_header_64, MAC_segment_command_64, MAC_section_64, MAC_nlist_64, int64

#endif // #ifndef MACHO_H

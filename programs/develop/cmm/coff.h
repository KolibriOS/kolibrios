//
// Section characteristics.
//
//      IMAGE_SCN_TYPE_REG                   0x00000000  // Reserved.
//      IMAGE_SCN_TYPE_DSECT                 0x00000001  // Reserved.
//      IMAGE_SCN_TYPE_NOLOAD                0x00000002  // Reserved.
//      IMAGE_SCN_TYPE_GROUP                 0x00000004  // Reserved.
#define IMAGE_SCN_TYPE_NO_PAD                0x00000008  // Reserved.
//      IMAGE_SCN_TYPE_COPY                  0x00000010  // Reserved.

#define IMAGE_SCN_CNT_CODE                   0x00000020  // Section contains code.
#define IMAGE_SCN_CNT_INITIALIZED_DATA       0x00000040  // Section contains initialized data.
#define IMAGE_SCN_CNT_UNINITIALIZED_DATA     0x00000080  // Section contains uninitialized data.

#define IMAGE_SCN_LNK_OTHER                  0x00000100  // Reserved.
#define IMAGE_SCN_LNK_INFO                   0x00000200  // Section contains comments or some other type of information.
//      IMAGE_SCN_TYPE_OVER                  0x00000400  // Reserved.
#define IMAGE_SCN_LNK_REMOVE                 0x00000800  // Section contents will not become part of image.
#define IMAGE_SCN_LNK_COMDAT                 0x00001000  // Section contents comdat.
//                                           0x00002000  // Reserved.
//      IMAGE_SCN_MEM_PROTECTED - Obsolete   0x00004000
#define IMAGE_SCN_NO_DEFER_SPEC_EXC          0x00004000  // Reset speculative exceptions handling bits in the TLB entries for this section.
#define IMAGE_SCN_GPREL                      0x00008000  // Section content can be accessed relative to GP
#define IMAGE_SCN_MEM_FARDATA                0x00008000
//      IMAGE_SCN_MEM_SYSHEAP  - Obsolete    0x00010000
#define IMAGE_SCN_MEM_PURGEABLE              0x00020000
#define IMAGE_SCN_MEM_16BIT                  0x00020000
#define IMAGE_SCN_MEM_LOCKED                 0x00040000
#define IMAGE_SCN_MEM_PRELOAD                0x00080000

#define IMAGE_SCN_ALIGN_1BYTES               0x00100000  //
#define IMAGE_SCN_ALIGN_2BYTES               0x00200000  //
#define IMAGE_SCN_ALIGN_4BYTES               0x00300000  //
#define IMAGE_SCN_ALIGN_8BYTES               0x00400000  //
#define IMAGE_SCN_ALIGN_16BYTES              0x00500000  // Default alignment if no others are specified.
#define IMAGE_SCN_ALIGN_32BYTES              0x00600000  //
#define IMAGE_SCN_ALIGN_64BYTES              0x00700000  //
#define IMAGE_SCN_ALIGN_128BYTES             0x00800000  //
#define IMAGE_SCN_ALIGN_256BYTES             0x00900000  //
#define IMAGE_SCN_ALIGN_512BYTES             0x00A00000  //
#define IMAGE_SCN_ALIGN_1024BYTES            0x00B00000  //
#define IMAGE_SCN_ALIGN_2048BYTES            0x00C00000  //
#define IMAGE_SCN_ALIGN_4096BYTES            0x00D00000  //
#define IMAGE_SCN_ALIGN_8192BYTES            0x00E00000  //
// Unused                                    0x00F00000
#define IMAGE_SCN_ALIGN_MASK                 0x00F00000

#define IMAGE_SCN_LNK_NRELOC_OVFL            0x01000000  // Section contains extended relocations.
#define IMAGE_SCN_MEM_DISCARDABLE            0x02000000  // Section can be discarded.
#define IMAGE_SCN_MEM_NOT_CACHED             0x04000000  // Section is not cachable.
#define IMAGE_SCN_MEM_NOT_PAGED              0x08000000  // Section is not pageable.
#define IMAGE_SCN_MEM_SHARED                 0x10000000  // Section is shareable.
#define IMAGE_SCN_MEM_EXECUTE                0x20000000  // Section is executable.
#define IMAGE_SCN_MEM_READ                   0x40000000  // Section is readable.
#define IMAGE_SCN_MEM_WRITE                  0x80000000  // Section is writeable.

//
// I386 relocation types.
//
#define IMAGE_REL_I386_ABSOLUTE 0x0000  //Reference is absolute, no relocation is necessary
#define IMAGE_REL_I386_DIR16    0x0001  //Direct 16-bit reference to the symbols virtual address
#define IMAGE_REL_I386_REL16    0x0002  //PC-relative 16-bit reference to the symbols virtual address
#define IMAGE_REL_I386_DIR32    0x0006  //Direct 32-bit reference to the symbols virtual address
#define IMAGE_REL_I386_DIR32NB  0x0007  //Direct 32-bit reference to the symbols virtual address, base not included
#define IMAGE_REL_I386_SEG12    0x0009  //Direct 16-bit reference to the segment-selector bits of a 32-bit virtual address
#define IMAGE_REL_I386_SECTION  0x000A
#define IMAGE_REL_I386_SECREL   0x000B
#define IMAGE_REL_I386_REL32    0x0014  //PC-relative 32-bit reference to the symbols virtual address

typedef struct _COFF_HEADER_
{
	short cpu;    //мин тип CPU - всегда 0x14C
	short numobj;	//число входов в таблицу объектов
	long date_time;	//дата модификации линкером
	long pCOFF;
	long COFFsize;
  short SizeOfOptionalHeader;
  short Characteristics;
}COFF_HEADER;

typedef struct _IMAGE_SYMBOL
{
	union{
		char sname[8];
		struct{
			unsigned long Short;
			unsigned long Long;
		}Name;
		char *LongName[2];
	}N;
	unsigned long Value;
	short SectionNumber;
	unsigned short Type;
	unsigned char StorageClass;
	unsigned char NumberOfAuxSymbols;
}IMAGE_SYMBOL;

typedef struct _IMAGE_RELOCATION {
	union {
		unsigned long VirtualAddress;
		unsigned long RelocCount; // Set to the real count when IMAGE_SCN_LNK_NRELOC_OVFL is set
	};
	unsigned long SymbolTableIndex;
	unsigned short Type;
}IMAGE_RELOCATION;


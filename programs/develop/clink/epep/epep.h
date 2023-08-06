// Dependencies:
// <assert.h> or any another source of assert()
// <stdint.h> or any another source of uint64_t, uint32_t, uint16_t, uint8_t, size_t

#ifndef EPEP_ASSERT
#include <assert.h>
#define EPEP_ASSERT(x) assert(x)
#endif

#ifndef EPEP_READER
#include <stdio.h>
#define EPEP_READER FILE *
#define EPEP_READER_GET(preader) getc(*preader)
#define EPEP_READER_SEEK(preader, offset) fseek(*preader, offset, SEEK_SET)
#define EPEP_READER_SEEK_END(preader, offset) fseek(*preader, offset, SEEK_END)
#define EPEP_READER_TELL(preader) ftell(*preader)
#define EPEP_READER_GET_BLOCK(preader, size, buf) fread(buf, 1, size, *preader);
#endif

//
// Constants
//

typedef enum {
	EPEP_INVALID,
	EPEP_IMAGE,
	EPEP_OBJECT,
} EpepKind;

typedef enum {
	EPEP_ERR_SUCCESS,
	EPEP_ERR_DATA_DIRECTORY_INDEX_IS_INVALID,
	EPEP_ERR_SECTION_HEADER_INDEX_IS_INVALID,
	EPEP_ERR_SYMBOL_INDEX_IS_INVALID,
	EPEP_ERR_NOT_AN_OBJECT,
	EPEP_ERR_ADDRESS_IS_OUT_OF_SECTION_RAW_DATA,
	EPEP_ERR_OUTPUT_CAPACITY_IS_ZERO,
	EPEP_ERR_OUTPUT_IS_NULL,
	EPEP_ERR_ADDRESS_IS_OUT_OF_ANY_SECTION,
	EPEP_ERR_EXPORT_ADDRESS_TABLE_ENTRY_NAME_NOT_FOUND,
	EPEP_ERR_NO_BASE_RELOCATION_TABLE,
	EPEP_ERR_BASE_RELOCATION_IS_ALREADY_END,
	EPEP_ERR_INVALID_DATA_DIRECTORY_OFFSET,
	EPEP_ERR_INVALID_SECTION_HEADER_OFFSET,
	EPEP_ERR_INVALID_SECTION_DATA_OFFSET,
	EPEP_ERR_INVALID_STRING_TABLE_SIZE_OFFSET,
	EPEP_ERR_INVALID_SYMBOL_OFFSET,
	EPEP_ERR_INVALID_IMPORT_DIRECTORY_OFFSET,
	EPEP_ERR_INVALID_IMPORT_DIRECTORY_NAME_OFFSET,
	EPEP_ERR_INVALID_LOOKUP_OFFSET,
	EPEP_ERR_INVALID_LOOKUP_NAME_OFFSET,
	EPEP_ERR_INVALID_EXPORT_TABLE_OFFSET,
	EPEP_ERR_INVALID_DLL_NAME_OFFSET,
	EPEP_ERR_INVALID_EXPORT_NAME_POINTER_OFFSET,
	EPEP_ERR_INVALID_ORDINAL_TABLE_OFFSET,
	EPEP_ERR_INVALID_EXPORT_NAME_OFFSET,
	EPEP_ERR_INVALID_EXPORT_ADDRESS_OFFSET,
	EPEP_ERR_INVALID_FORWARDER_OFFSET,
	EPEP_ERR_INVALID_BASE_RELOCATION_BLOCK_OFFSET,
	EPEP_ERR_INVALID_NEXT_BASE_RELOCATION_BLOCK_OFFSET,
	EPEP_ERR_INVALID_BASE_RELOCATION_BLOCK_BASE_RELOCATION_OFFSET,
	EPEP_ERR_INVALID_SECTION_RELOCATION_OFFSET,
	EPEP_ERR_INVALID_LINENUMBER_OFFSET,
	EPEP_ERR_INVALID_NUMBER_OF_RELOCATIONS_FOR_EXTENDED,
	EPEP_ERR_END
} EpepError;

//
// Generic
//

typedef struct {
	EPEP_READER reader;
	EpepKind kind;
	EpepError error_code;
	size_t file_size;
	size_t signature_offset_offset;
	size_t signature_offset;
	size_t first_data_directory_offset;
	size_t first_section_header_offset;
	size_t export_table_offset;
	size_t import_table_offset;
	size_t base_relocation_table_offset;
	size_t base_relocation_table_end_offset;
	struct {
		uint16_t Machine;
		uint16_t NumberOfSections;
		uint32_t TimeDateStamp;
		uint32_t PointerToSymbolTable;
		uint32_t NumberOfSymbols;
		uint16_t SizeOfOptionalHeader;
		uint16_t Characteristics;
	} coffFileHeader;
	struct {
		// Standard fields
		uint16_t Magic;
		uint8_t MajorLinkerVersion;
		uint8_t MinorLinkerVersion;
		uint32_t SizeOfCode;
		uint32_t SizeOfInitializedData;
		uint32_t SizeOfUninitializedData;
		uint32_t AddressOfEntryPoint;
		uint32_t BaseOfCode;
		uint32_t BaseOfData; // PE32-only
		// Windows-specific fields
		uint64_t ImageBase;
		uint32_t SectionAlignment;
		uint32_t FileAlignment;
		uint16_t MajorOperatingSystemVersion;
		uint16_t MinorOperatingSystemVersion;
		uint16_t MajorImageVersion;
		uint16_t MinorImageVersion;
		uint16_t MajorSubsystemVersion;
		uint16_t MinorSubsystemVersion;
		uint32_t Win32VersionValue;
		uint32_t SizeOfImage;
		uint32_t SizeOfHeaders;
		uint32_t CheckSum;
		uint16_t Subsystem;
		uint16_t DllCharacteristics;
		uint64_t SizeOfStackReserve;
		uint64_t SizeOfStackCommit;
		uint64_t SizeOfHeapReserve;
		uint64_t SizeOfHeapCommit;
		uint32_t LoaderFlags;
		uint32_t NumberOfRvaAndSizes;
	} optionalHeader;
	struct {
		uint32_t ExportFlags;
		uint32_t TimeDateStamp;
		uint16_t MajorVersion;
		uint16_t MinorVersion;
		uint32_t NameRva;
		uint32_t OrdinalBase;
		uint32_t AddressTableEntries;
		uint32_t NumberOfNamePointers;
		uint32_t ExportAddressTableRva;
		uint32_t NamePointerRva;
		uint32_t OrdinalTableRva;
	} export_directory;
} Epep;

/// Constructor of the general information container
int epep_init(Epep *epep, EPEP_READER reader);

/// Gives file offset corresponding to RVA is any, returns 0 othervice
int epep_get_file_offset_by_rva(Epep *epep, size_t *offset, size_t addr);

//
// Data Directories
//

typedef struct {
	uint32_t VirtualAddress;
	uint32_t Size;
} EpepImageDataDirectory;

/// Gives Data Directiry by its index
int epep_get_data_directory_by_index(Epep *epep, EpepImageDataDirectory *idd, size_t index);

//
// Sections
//

typedef struct {
	char Name[8];
	uint32_t VirtualSize;
	uint32_t VirtualAddress;
	uint32_t SizeOfRawData;
	uint32_t PointerToRawData;
	uint32_t PointerToRelocations;
	uint32_t PointerToLinenumbers;
	uint16_t NumberOfRelocations;
	uint16_t NumberOfLinenumbers;
	uint32_t Characteristics;
} EpepSectionHeader;

/// Gives Section Header by its index
int epep_get_section_header_by_index(Epep *epep, EpepSectionHeader *sh, size_t index);

/// Gives section header by RVA
int epep_get_section_header_by_rva(Epep *epep, EpepSectionHeader *sh, size_t addr);

/// Gives section contents by Section Header
int epep_get_section_contents(Epep *epep, EpepSectionHeader *sh, void *buf);

//
// COFF Symbols (object file symbols)
//

typedef union {
	struct {
		union {
			char ShortName[8];
			struct {
				uint32_t Zeroes;
				uint32_t Offset;
			};
		};
		uint32_t Value;
		uint16_t SectionNumber;
		uint16_t Type;
		uint8_t StorageClass;
		uint8_t NumberOfAuxSymbols;
	} symbol;
	struct {
		uint32_t TagIndex;
		uint32_t TotalSize;
		uint32_t PointerToLinenumber;
		uint32_t PointerToNextFunction;
		uint16_t Unused;
	} auxFunctionDefinition;
	struct {
		uint8_t Unused0[4];
		uint16_t Linenumber;
		uint8_t Unused1[6];
		uint32_t PointerToNextFunction;
		uint8_t Unused2[2];
	} auxBfOrEfSymbol;
	struct {
		uint32_t TagIndex;
		uint32_t Characteristics;
		uint8_t Unused[10];
	} auxWeakExternal;
	struct {
		char FileName[18];
	} auxFile;
	struct {
		uint32_t Length;
		uint16_t NumberOfRelocations;
		uint16_t NumberOfLinenumbers;
		uint32_t CheckSum;
		uint16_t Number;
		uint8_t  Selection;
		uint8_t Unused[3];
	} auxSectionDefinition;
} EpepCoffSymbol;

/// Gives COFF string table size
int epep_get_string_table_size(Epep *epep, size_t *size);

/// Gives COFF string table
int epep_get_string_table(Epep *epep, char *string_table);

/// Gives COFF Symbol by its index
int epep_get_symbol_by_index(Epep *epep, EpepCoffSymbol *sym, size_t index);

//
// Imports
//

typedef struct {
	uint32_t ImportLookupTableRva;
	uint32_t TimeDateStamp;
	uint32_t ForwarderChain;
	uint32_t NameRva;
	uint32_t ImportAddressTableRva;
} EpepImportDirectory;

/// Returns non-zero if import table exists in the file
int epep_has_import_table(Epep *epep);

/// Places offset of import table into epep structure
int epep_read_import_table_offset(Epep *epep);

/// Gives Import Directory by index
int epep_get_import_directory_by_index(Epep *epep, EpepImportDirectory *import_directory, size_t index);

/// Gives name of Import Directory (library)
int epep_get_import_directory_name_s(Epep *epep, EpepImportDirectory *import_directory, char *name, size_t name_max);

/// Gives Import Lookup (imported symbol) by import directory and index
int epep_get_import_directory_lookup_by_index(Epep *epep, EpepImportDirectory *import_directory, size_t *lookup, size_t index);

/// Gives name of Import Directory Lookup (imported symbol) or nothing if imported by ordinal
int epep_get_lookup_name_s(Epep *epep, size_t lookup, char *name, size_t name_max);

//
// Exports
//

typedef union {
	uint32_t ExportRva;
	uint32_t ForwarderRva;
} EpepExportAddress;

/// Returns non-zero if export table exists in the file
int epep_has_export_table(Epep *epep);

/// Palces offset of export table into epep structure
int epep_read_export_table_offset(Epep *epep);

/// Palces export table into epep structure
//! Needs to be called before next export functions
int epep_read_export_directory(Epep *epep);

/// Gives name of the DLL
//! epep_read_export_directory needs to be called before
int epep_get_dll_name_s(Epep *epep, char *name, size_t name_max);

/// Gives entry from Export Name Pointer Table by its index
//! epep_read_export_directory needs to be called before
int epep_get_export_name_pointer_by_index(Epep *epep, size_t *name_rva, size_t index);

/// Gives export name by its index in Export Address Table (receives name buffer length)
//! epep_read_export_directory needs to be called before
int epep_get_export_name_s_by_index(Epep *epep, char *name, size_t name_max, size_t index);

/// Gives export address by its index in Export Address Table
//! epep_read_export_directory needs to be called before
int epep_get_export_address_by_index(Epep *epep, EpepExportAddress *export_address, size_t index);

/// Gives forwarder string of Export Address
//! epep_read_export_directory needs to be called before
int epep_get_export_address_forwarder_s(Epep *epep, EpepExportAddress *export_address, char *forwarder, size_t forwarder_max);

/// Returns non-zero if the export address specifies forwarder string
//! epep_read_export_directory needs to be called before
int epep_export_address_is_forwarder(Epep *epep, EpepExportAddress *export_address);

//
// DLL Base Relocations
//

typedef struct {
	size_t offset;
	uint32_t PageRva;
	uint32_t BlockSize;
	uint16_t BaseRelocation[0];
} EpepBaseRelocationBlock;

typedef union {
	struct {
		uint16_t Offset: 12,
			Type: 4;
	};
	uint16_t u16;
} EpepBaseRelocation;

/// Returns non-zero if the file contains Base Relocations
int epep_has_base_relocation_table(Epep *epep);

/// Places offset to Base Relocation Table into epep structure
int epep_read_base_relocation_table_offset(Epep *epep);

/// Gives first Base Relocation Block
int epep_get_first_base_relocation_block(Epep *epep, EpepBaseRelocationBlock *brb);

/// Gives next Base Relocation Block (replaces contents of the given block)
int epep_get_next_base_relocation_block(Epep *epep, EpepBaseRelocationBlock *it);

/// Gives Base Relocation by its index in Base Relocation Block
int epep_get_base_relocation_block_base_relocation_by_index(Epep *epep, EpepBaseRelocationBlock *brb, EpepBaseRelocation *br, size_t index);

//
// COFF Relocations
//

typedef struct {
	uint32_t VirtualAddress;
	uint32_t SymbolTableIndex;
	uint16_t Type;
} EpepCoffRelocation;

/// Gives a COFF Relocation by its index
int epep_get_section_relocation_by_index(Epep *epep, EpepSectionHeader *sh, EpepCoffRelocation *rel, size_t index);

/// Checks if the section contains more than 2^16 - 1 relocations
int epep_section_contains_extended_relocations(Epep *epep, EpepSectionHeader *sh, int *result);

/// Gives the section relocation count if the section contains more than 2^16 - 1 relocations
int epep_get_section_extended_number_of_relocations(Epep *epep, EpepSectionHeader *sh, size_t *result);

/// Gives the meaningful COFF Relocation count
//! Returns the value to pass to the following _x functions in the last parameter
int epep_get_section_number_of_relocations_x(Epep *epep, EpepSectionHeader *sh, size_t *result, int *extended);

/// Gives a meaningful COFF Relocation by its index
//! Requires the value returned by epep_get_section_number_of_relocations_x as the last argument
int epep_get_section_relocation_by_index_x(Epep *epep, EpepSectionHeader *sh, EpepCoffRelocation *rel, size_t index, int extended);

//
// COFF Line Numbers
//

typedef struct {
	union {
		uint32_t SymbolTableIndex;
		uint32_t VirtualAddress;
	} Type;
	uint16_t Linenumber;
} EpepCoffLinenumber;

int epep_get_section_line_number_by_index(Epep *epep, EpepSectionHeader *sh, EpepCoffLinenumber *ln, size_t index);

#ifdef EPEP_INST

//
// Private functions
//

static int epep_seek(Epep *epep, size_t offset) {
	EPEP_READER_SEEK(&epep->reader, offset);
	return 1;
}

static int epep_seek_end(Epep *epep, size_t offset) {
	EPEP_READER_SEEK_END(&epep->reader, offset);
	return 1;
}

static int epep_read_block(Epep *epep, size_t size, void *block) {
	EPEP_READER_GET_BLOCK(&epep->reader, size, block);
	return 1;
}

static int is_pe32(Epep *epep) {
	return epep->optionalHeader.Magic == 0x10b;
}

static int is_pe32p(Epep *epep) {
	return epep->optionalHeader.Magic == 0x20b;
}

static uint8_t epep_read_u8(Epep *epep) {
	return EPEP_READER_GET(&epep->reader);
}

static uint16_t epep_read_u16(Epep *epep) {
	unsigned l = epep_read_u8(epep);
	unsigned h = epep_read_u8(epep);
	return l | (h << 8);
}

static uint32_t epep_read_u32(Epep *epep) {
	unsigned b0 = epep_read_u8(epep);
	unsigned b1 = epep_read_u8(epep);
	unsigned b2 = epep_read_u8(epep);
	unsigned b3 = epep_read_u8(epep);
	return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
}

static uint64_t epep_read_u64(Epep *epep) {
	uint64_t res = 0;
	for (unsigned i = 0; i < 64; i += 8) {
		res |= epep_read_u8(epep) << i;
	}
	return res;
}

static uint64_t epep_read_ptr(Epep *epep) {
	return is_pe32(epep) ? epep_read_u32(epep) : epep_read_u64(epep);
}

static int epep_valid_offset(Epep *epep, size_t offset, size_t size) {
	return (offset + size) <= epep->file_size;
}

//
// Generic
//

int epep_init(Epep *epep, EPEP_READER reader) {
	*epep = (Epep){ 0 };
	epep->kind = EPEP_IMAGE;
	epep->reader = reader;
	epep_seek_end(epep, 0);
	epep->file_size = EPEP_READER_TELL(&epep->reader);
	epep_seek(epep, 0);
	epep->error_code = EPEP_ERR_SUCCESS;
	epep->signature_offset_offset = 0x3c;
	epep_seek(epep, epep->signature_offset_offset);
	epep->signature_offset = epep_read_u32(epep);
	epep_seek(epep, epep->signature_offset);
	char signature_buf[4];
	signature_buf[0] = epep_read_u8(epep);
	signature_buf[1] = epep_read_u8(epep);
	signature_buf[2] = epep_read_u8(epep);
	signature_buf[3] = epep_read_u8(epep);
	if (signature_buf[0] != 'P' || signature_buf[1] != 'E' ||
		signature_buf[2] != '\0' || signature_buf[3] != '\0') {
		epep->kind = EPEP_OBJECT;
		epep_seek(epep, 0);
	}
	epep->coffFileHeader.Machine = epep_read_u16(epep);
	epep->coffFileHeader.NumberOfSections = epep_read_u16(epep);
	epep->coffFileHeader.TimeDateStamp = epep_read_u32(epep);
	epep->coffFileHeader.PointerToSymbolTable = epep_read_u32(epep);
	epep->coffFileHeader.NumberOfSymbols = epep_read_u32(epep);
	epep->coffFileHeader.SizeOfOptionalHeader = epep_read_u16(epep);
	epep->coffFileHeader.Characteristics = epep_read_u16(epep);
	if (epep->coffFileHeader.SizeOfOptionalHeader != 0) {
		// Standard fields
		epep->optionalHeader.Magic = epep_read_u16(epep);
		epep->optionalHeader.MajorLinkerVersion = epep_read_u8(epep);
		epep->optionalHeader.MinorLinkerVersion = epep_read_u8(epep);
		epep->optionalHeader.SizeOfCode = epep_read_u32(epep);
		epep->optionalHeader.SizeOfInitializedData = epep_read_u32(epep);
		epep->optionalHeader.SizeOfUninitializedData = epep_read_u32(epep);
		epep->optionalHeader.AddressOfEntryPoint = epep_read_u32(epep);
		epep->optionalHeader.BaseOfCode = epep_read_u32(epep);
		if (is_pe32(epep)) {
			epep->optionalHeader.BaseOfData = epep_read_u32(epep);
		}
		// Windows-specific fields
		epep->optionalHeader.ImageBase = epep_read_ptr(epep);
		epep->optionalHeader.SectionAlignment = epep_read_u32(epep);
		epep->optionalHeader.FileAlignment = epep_read_u32(epep);
		epep->optionalHeader.MajorOperatingSystemVersion = epep_read_u16(epep);
		epep->optionalHeader.MinorOperatingSystemVersion = epep_read_u16(epep);
		epep->optionalHeader.MajorImageVersion = epep_read_u16(epep);
		epep->optionalHeader.MinorImageVersion = epep_read_u16(epep);
		epep->optionalHeader.MajorSubsystemVersion = epep_read_u16(epep);
		epep->optionalHeader.Win32VersionValue = epep_read_u32(epep);
		epep->optionalHeader.MinorSubsystemVersion = epep_read_u16(epep);
		epep->optionalHeader.SizeOfImage = epep_read_u32(epep);
		epep->optionalHeader.SizeOfHeaders = epep_read_u32(epep);
		epep->optionalHeader.CheckSum = epep_read_u32(epep);
		epep->optionalHeader.Subsystem = epep_read_u16(epep);
		epep->optionalHeader.DllCharacteristics = epep_read_u16(epep);
		epep->optionalHeader.SizeOfStackReserve = epep_read_ptr(epep);
		epep->optionalHeader.SizeOfStackCommit = epep_read_ptr(epep);
		epep->optionalHeader.SizeOfHeapReserve = epep_read_ptr(epep);
		epep->optionalHeader.SizeOfHeapCommit = epep_read_ptr(epep);
		epep->optionalHeader.LoaderFlags = epep_read_u32(epep);
		epep->optionalHeader.NumberOfRvaAndSizes = epep_read_u32(epep);
		epep->first_data_directory_offset = EPEP_READER_TELL(&epep->reader);
	}
	epep->first_section_header_offset = EPEP_READER_TELL(&epep->reader);
	if (epep->coffFileHeader.SizeOfOptionalHeader != 0) {
		epep->first_section_header_offset += epep->optionalHeader.NumberOfRvaAndSizes * sizeof(EpepImageDataDirectory);
	}
	return 1;
}

int epep_get_file_offset_by_rva(Epep *epep, size_t *offset, size_t addr) {
	EpepSectionHeader sh = { 0 };
	if (!epep_get_section_header_by_rva(epep, &sh, addr)) {
		return 0;
	}
	size_t diff = addr - sh.VirtualAddress;
	if (diff >= sh.SizeOfRawData) {
		epep->error_code = EPEP_ERR_ADDRESS_IS_OUT_OF_SECTION_RAW_DATA;
		return 0;
	}
	*offset = sh.PointerToRawData + diff;
	return 1;
}

//
// Data Directories
//

int epep_get_data_directory_by_index(Epep *epep, EpepImageDataDirectory *idd, size_t index) {
	if (index >= epep->optionalHeader.NumberOfRvaAndSizes) {
		epep->error_code = EPEP_ERR_DATA_DIRECTORY_INDEX_IS_INVALID;
		return 0;
	}
	size_t offset = epep->first_data_directory_offset + sizeof(EpepImageDataDirectory) * index;
	if (!epep_valid_offset(epep, offset, sizeof(EpepImageDataDirectory))) {
		epep->error_code = EPEP_ERR_INVALID_DATA_DIRECTORY_OFFSET;
		return 0;
	}
	epep_seek(epep, offset);
	idd->VirtualAddress = epep_read_u32(epep);
	idd->Size = epep_read_u32(epep);
	return 1;
}

//
// Sections
//

int epep_get_section_header_by_index(Epep *epep, EpepSectionHeader *sh, size_t index) {
	if (index >= epep->coffFileHeader.NumberOfSections) {
		epep->error_code = EPEP_ERR_SECTION_HEADER_INDEX_IS_INVALID;
		return 0;
	}
	size_t offset = epep->first_section_header_offset + sizeof(EpepSectionHeader) * index;
	if (!epep_valid_offset(epep, offset, sizeof(EpepSectionHeader))) {
		epep->error_code = EPEP_ERR_INVALID_SECTION_HEADER_OFFSET;
		return 0;
	}
	epep_seek(epep, offset);
	for (int i = 0; i < 8; i++) {
		sh->Name[i] = epep_read_u8(epep);
	}
	sh->VirtualSize = epep_read_u32(epep);
	sh->VirtualAddress = epep_read_u32(epep);
	sh->SizeOfRawData = epep_read_u32(epep);
	sh->PointerToRawData = epep_read_u32(epep);
	sh->PointerToRelocations = epep_read_u32(epep);
	sh->PointerToLinenumbers = epep_read_u32(epep);
	sh->NumberOfRelocations = epep_read_u16(epep);
	sh->NumberOfLinenumbers = epep_read_u16(epep);
	sh->Characteristics = epep_read_u32(epep);
	return 1;
}

int epep_get_section_header_by_rva(Epep *epep, EpepSectionHeader *sh, size_t addr) {
	EpepSectionHeader sh0 = { 0 };
	for (size_t i = 0; i < epep->coffFileHeader.NumberOfSections; i++) {
		epep_get_section_header_by_index(epep, &sh0, i);
		if (addr >= sh0.VirtualAddress && addr < (sh0.VirtualAddress + sh0.VirtualSize)) {
			*sh = sh0;
			return 1;
		}
	}
	epep->error_code = EPEP_ERR_ADDRESS_IS_OUT_OF_ANY_SECTION;
	return 0;
}

int epep_get_section_contents(Epep *epep, EpepSectionHeader *sh, void *buf) {
	size_t offset = sh->PointerToRawData;
	size_t size_of_raw_data = sh->SizeOfRawData;
	if (!epep_valid_offset(epep, offset, size_of_raw_data)) {
		epep->error_code = EPEP_ERR_INVALID_SECTION_DATA_OFFSET;
		return 0;
	}
	epep_seek(epep, offset);
	epep_read_block(epep, size_of_raw_data, buf);
	return 1;
}

//
// COFF Symbols
//

int epep_get_string_table_size(Epep *epep, size_t *size) {
	size_t offset = epep->coffFileHeader.PointerToSymbolTable + 18 * epep->coffFileHeader.NumberOfSymbols;
	if (!epep_valid_offset(epep, offset, sizeof(uint32_t))) {
		epep->error_code = EPEP_ERR_INVALID_STRING_TABLE_SIZE_OFFSET;
		return 0;
	}
	epep_seek(epep, offset);
	*size = epep_read_u32(epep);
	return 1;
}

int epep_get_string_table(Epep *epep, char *string_table) {
	size_t size = 0;
	if (!epep_get_string_table_size(epep, &size)) {
		return 0;
	}
	// A COFF strings table starts with its size
	*string_table++ = (size & 0x000000ff) >> 0;
	*string_table++ = (size & 0x0000ff00) >> 8;
	*string_table++ = (size & 0x00ff0000) >> 16;
	*string_table++ = (size & 0xff000000) >> 24;
	epep_read_block(epep, size - 4, string_table);
	return 1;
}

int epep_get_symbol_by_index(Epep *epep, EpepCoffSymbol *sym, size_t index) {
	if (epep->kind != EPEP_OBJECT) {
		epep->error_code = EPEP_ERR_NOT_AN_OBJECT;
		return 0;
	}
	if (index >= epep->coffFileHeader.NumberOfSymbols) {
		epep->error_code = EPEP_ERR_SYMBOL_INDEX_IS_INVALID;
		return 0;
	}
	size_t offset = epep->coffFileHeader.PointerToSymbolTable + 18 * index;
	if (!epep_valid_offset(epep, offset, 18)) {
		epep->error_code = EPEP_ERR_INVALID_SYMBOL_OFFSET;
		return 0;
	}
	epep_seek(epep, offset);
	for (size_t i = 0; i < 18; i++) {
		sym->auxFile.FileName[i] = epep_read_u8(epep);
	}
	return 1;
}

//
// Imports
//

int epep_has_import_table(Epep *epep) {
	if (epep->kind != EPEP_IMAGE) {
		return 0;
	}
	EpepImageDataDirectory idd = { 0 };
	if (!epep_get_data_directory_by_index(epep, &idd, 1)) {
		return 0;
	}
	return idd.VirtualAddress;
}

int epep_read_import_table_offset(Epep *epep) {
	EpepImageDataDirectory import_table_dd = { 0 };
	if (!epep_get_data_directory_by_index(epep, &import_table_dd, 1)) {
		return 0;
	}
	if (!epep_get_file_offset_by_rva(epep, &epep->import_table_offset, import_table_dd.VirtualAddress)) {
		return 0;
	}
	return 1;
}

int epep_get_import_directory_by_index(Epep *epep, EpepImportDirectory *import_directory, size_t index) {
	if (epep->import_table_offset == 0) {
		if (!epep_read_import_table_offset(epep)) {
			return 0;
		}
	}
	size_t offset = epep->import_table_offset + index * sizeof(*import_directory);
	if (!epep_valid_offset(epep, offset, sizeof(*import_directory))) {
		epep->error_code = EPEP_ERR_INVALID_IMPORT_DIRECTORY_OFFSET;
		return 0;
	}
	epep_seek(epep, offset);
	epep_read_block(epep, sizeof(*import_directory), import_directory);
	return 1;
}

int epep_get_import_directory_name_s(Epep *epep, EpepImportDirectory *import_directory, char *name, size_t name_max) {
	size_t name_rva = import_directory->NameRva;
	size_t name_offset = 0;
	if (!epep_get_file_offset_by_rva(epep, &name_offset, name_rva)) {
		return 0;
	}
	if (!epep_valid_offset(epep, name_offset, 0)) {
		epep->error_code = EPEP_ERR_INVALID_IMPORT_DIRECTORY_NAME_OFFSET;
		return 0;
	}
	epep_seek(epep, name_offset);
	epep_read_block(epep, name_max, name);
	return 1;
}

int epep_get_import_directory_lookup_by_index(Epep *epep, EpepImportDirectory *import_directory, size_t *lookup, size_t index) {
	size_t first_lookup_offset = 0;
	if (!epep_get_file_offset_by_rva(epep, &first_lookup_offset, import_directory->ImportLookupTableRva)) {
		return 0;
	}
	size_t size_of_lookup = is_pe32(epep) ? 4 : 8;
	size_t lookup_offset = first_lookup_offset + size_of_lookup * index;
	if (!epep_valid_offset(epep, lookup_offset, size_of_lookup)) {
		epep->error_code = EPEP_ERR_INVALID_LOOKUP_OFFSET;
		return 0;
	}
	epep_seek(epep, lookup_offset);
	epep_read_block(epep, size_of_lookup, lookup);
	return 1;
}

int epep_get_lookup_name_s(Epep *epep, size_t lookup, char *name, size_t name_max) {
	if (name_max == 0) {
		epep->error_code = EPEP_ERR_OUTPUT_CAPACITY_IS_ZERO;
		return 0;
	}
	if (name == NULL) {
		epep->error_code = EPEP_ERR_OUTPUT_IS_NULL;
		return 0;
	}
	uint64_t mask = is_pe32(epep) ? 0x80000000 : 0x8000000000000000;
	if (lookup & mask) {
		name[0] = '\0';
		return 1;
	}
	size_t name_rva = lookup;
	size_t name_offset = 0;
	if (!epep_get_file_offset_by_rva(epep, &name_offset, name_rva)) {
		return 0;
	}
	// skip 2 bytes (Name Table :: Hint)
	name_offset += 2;
	if (!epep_valid_offset(epep, name_offset, 0)) {
		epep->error_code = EPEP_ERR_INVALID_LOOKUP_NAME_OFFSET;
		return 0;
	}
	epep_seek(epep, name_offset);
	epep_read_block(epep, name_max, name);
	return 1;
}

//
// Exports
//

int epep_has_export_table(Epep *epep) {
	if (epep->kind != EPEP_IMAGE) {
		return 0;
	}
	EpepImageDataDirectory idd = { 0 };
	if (!epep_get_data_directory_by_index(epep, &idd, 0)) {
		return 0;
	}
	return idd.VirtualAddress;
}

int epep_read_export_table_offset(Epep *epep) {
	EpepImageDataDirectory export_table_dd = { 0 };
	if (!epep_get_data_directory_by_index(epep, &export_table_dd, 0)) {
		return 0;
	}
	if (!epep_get_file_offset_by_rva(epep, &epep->export_table_offset, export_table_dd.VirtualAddress)) {
		return 0;
	}
	return 1;
}

int epep_read_export_directory(Epep *epep) {
	if (epep->export_table_offset == 0) {
		if (!epep_read_export_table_offset(epep)) {
			return 0;
		}
	}
	if (!epep_valid_offset(epep, epep->export_table_offset, sizeof(epep->export_directory))) {
		epep->error_code = EPEP_ERR_INVALID_EXPORT_TABLE_OFFSET;
		return 0;
	}
	epep_seek(epep, epep->export_table_offset);
	epep_read_block(epep, sizeof(epep->export_directory), &epep->export_directory);
	return 1;
}

int epep_get_dll_name_s(Epep *epep, char *name, size_t name_max) {
	size_t offset = 0;
	if (!epep_get_file_offset_by_rva(epep, &offset, epep->export_directory.NameRva)) {
		return 0;
	}
	if (!epep_valid_offset(epep, offset, 0)) {
		epep->error_code = EPEP_ERR_INVALID_DLL_NAME_OFFSET;
		return 0;
	}
	epep_seek(epep, offset);
	epep_read_block(epep, name_max, name);
	return 1;
}

int epep_get_export_name_pointer_by_index(Epep *epep, size_t *name_rva, size_t index) {
	size_t name_pointer_table_rva = epep->export_directory.NamePointerRva;
	size_t name_pointer_table_offset = 0;
	if (!epep_get_file_offset_by_rva(epep, &name_pointer_table_offset, name_pointer_table_rva)) {
		return 0;
	}
	size_t offset = name_pointer_table_offset + sizeof(uint32_t) * index;
	if (!epep_valid_offset(epep, offset, sizeof(uint32_t))) {
		epep->error_code = EPEP_ERR_INVALID_EXPORT_NAME_POINTER_OFFSET;
		return 0;
	}
	epep_seek(epep, offset);
	*name_rva = epep_read_u32(epep);	
	return 1;
}

int epep_get_export_name_s_by_index(Epep *epep, char *name, size_t name_max, size_t index) {
	size_t ordinal_table_offset = 0;
	if (!epep_get_file_offset_by_rva(epep, &ordinal_table_offset, epep->export_directory.OrdinalTableRva)) {
		return 0;
	}
	if (!epep_valid_offset(epep, ordinal_table_offset, 0)) {
		epep->error_code = EPEP_ERR_INVALID_ORDINAL_TABLE_OFFSET;
		return 0;
	}
	epep_seek(epep, ordinal_table_offset);
	for (size_t i = 0; i < epep->export_directory.NumberOfNamePointers; i++) {
		uint16_t ordinal = epep_read_u16(epep);
		if (ordinal == index) { // SPEC_VIOL: Why should not epep->export_directory.OrdinalBase be substracted?
			size_t name_rva = 0;
			if (!epep_get_export_name_pointer_by_index(epep, &name_rva, i)) {
				return 0;
			}
			size_t name_offset = 0;
			if (!epep_get_file_offset_by_rva(epep, &name_offset, name_rva)) {
				return 0;
			}
			if (!epep_valid_offset(epep, name_offset, 0)) {
				epep->error_code = EPEP_ERR_INVALID_EXPORT_NAME_OFFSET;
				return 0;
			}
			epep_seek(epep, name_offset);
			epep_read_block(epep, name_max, name);
			return 1;
		}
	}
	epep->error_code = EPEP_ERR_EXPORT_ADDRESS_TABLE_ENTRY_NAME_NOT_FOUND;
	return 0;
}

int epep_get_export_address_by_index(Epep *epep, EpepExportAddress *export_address, size_t index) {
	size_t export_address_table_offset = 0;
	if (!epep_get_file_offset_by_rva(epep, &export_address_table_offset, epep->export_directory.ExportAddressTableRva)) {
		return 0;
	}
	EPEP_ASSERT(sizeof(EpepExportAddress) == sizeof(uint32_t));
	size_t offset = export_address_table_offset + sizeof(EpepExportAddress) * index;
	if (!epep_valid_offset(epep, offset, sizeof(*export_address))) {
		epep->error_code = EPEP_ERR_INVALID_EXPORT_ADDRESS_OFFSET;
		return 0;
	}
	epep_seek(epep, offset);
	epep_read_block(epep, sizeof(*export_address), export_address);
	return 1;
}

int epep_get_export_address_forwarder_s(Epep *epep, EpepExportAddress *export_address, char *forwarder, size_t forwarder_max) {
	size_t forwarder_offset = 0;
	if (!epep_get_file_offset_by_rva(epep, &forwarder_offset, export_address->ForwarderRva)) {
		return 0;
	}
	if (!epep_valid_offset(epep, forwarder_offset, 0)) {
		epep->error_code = EPEP_ERR_INVALID_FORWARDER_OFFSET;
		return 0;
	}
	epep_seek(epep, forwarder_offset);
	epep_read_block(epep, forwarder_max, forwarder);
	return 1;
}

int epep_export_address_is_forwarder(Epep *epep, EpepExportAddress *export_address) {
	EpepImageDataDirectory edd = { 0 };
	if (!epep_get_data_directory_by_index(epep, &edd, 0)) {
		return 0;
	}
	if (export_address->ForwarderRva >= edd.VirtualAddress && export_address->ForwarderRva < edd.VirtualAddress + edd.Size) {
		return 1;
	}
	return 0;
}

//
// DLL Base Relocaions
//

int epep_has_base_relocation_table(Epep *epep) {
	EpepImageDataDirectory brtdd = { 0 };
	if (!epep_get_data_directory_by_index(epep, &brtdd, 5)) {
		return 0;
	}
	if (brtdd.VirtualAddress == 0) {
		return 0;
	}
	return 1;
}

int epep_read_base_relocation_table_offset(Epep *epep) {
	EpepImageDataDirectory brtdd = { 0 };
	if (!epep_get_data_directory_by_index(epep, &brtdd, 5)) {
		return 0;
	}
	if (!epep_get_file_offset_by_rva(epep, &epep->base_relocation_table_offset, brtdd.VirtualAddress)) {
		return 0;
	}
	epep->base_relocation_table_end_offset = epep->base_relocation_table_offset + brtdd.Size;
	return 1;
}

int epep_get_first_base_relocation_block(Epep *epep, EpepBaseRelocationBlock *brb) {
	if (epep->base_relocation_table_offset == 0) {
		if (!epep_read_base_relocation_table_offset(epep)) {
			return 0;
		}
	}
	if (epep->base_relocation_table_offset == 0) {
		epep->error_code = EPEP_ERR_NO_BASE_RELOCATION_TABLE;
		return 0;
	}
	if (!epep_valid_offset(epep, epep->base_relocation_table_offset, 0)) {
		epep->error_code = EPEP_ERR_INVALID_BASE_RELOCATION_BLOCK_OFFSET;
		return 0;
	}
	if (!epep_seek(epep, epep->base_relocation_table_offset)) {
		return 0;
	}
	brb->offset = epep->base_relocation_table_offset;
	brb->PageRva = epep_read_u32(epep);
	brb->BlockSize = epep_read_u32(epep);
	return 1;
}

int epep_get_next_base_relocation_block(Epep *epep, EpepBaseRelocationBlock *it) {
	if (it->offset == 0) {
		epep->error_code = EPEP_ERR_BASE_RELOCATION_IS_ALREADY_END;
		return 0;
	}
	it->offset = it->offset + it->BlockSize;
	if (it->offset >= epep->base_relocation_table_end_offset) {
		*it = (EpepBaseRelocationBlock){ 0 };
		return 1;
	}
	if (!epep_valid_offset(epep, it->offset, 0)) {
		epep->error_code = EPEP_ERR_INVALID_NEXT_BASE_RELOCATION_BLOCK_OFFSET;
		return 0;
	}
	if (!epep_seek(epep, it->offset)) {
		return 0;
	}
	it->PageRva = epep_read_u32(epep);
	it->BlockSize = epep_read_u32(epep);
	return 1;
}

int epep_get_base_relocation_block_base_relocation_by_index(Epep *epep, EpepBaseRelocationBlock *brb, EpepBaseRelocation *br, size_t index) {
	size_t offset = brb->offset + 8 + sizeof(EpepBaseRelocation) * index;
	if (!epep_valid_offset(epep, offset, sizeof(EpepBaseRelocation))) {
		epep->error_code = EPEP_ERR_INVALID_BASE_RELOCATION_BLOCK_BASE_RELOCATION_OFFSET;
		return 0;
	}
	if (!epep_seek(epep, offset)) {
		return 0;
	}
	br->u16 = epep_read_u16(epep);
	return 1;
}

//
// COFF Relocations
//

int epep_get_section_relocation_by_index(Epep *epep, EpepSectionHeader *sh, EpepCoffRelocation *rel, size_t index) {
	size_t offset = sh->PointerToRelocations + 10 * index;
	if (!epep_valid_offset(epep, offset, 10)) {
		epep->error_code = EPEP_ERR_INVALID_SECTION_RELOCATION_OFFSET;
		return 0;
	}
	epep_seek(epep, offset);
	epep_read_block(epep, 10, rel);
	return 1;
}

int epep_section_contains_extended_relocations(Epep *epep, EpepSectionHeader *sh, int *result) {
	const uint32_t flag_IMAGE_SCN_LNK_NRELOC_OVFL = 0x01000000;
	if (sh->Characteristics & flag_IMAGE_SCN_LNK_NRELOC_OVFL) {
                if (sh->NumberOfRelocations != 0xffff) {
                        epep->error_code = EPEP_ERR_INVALID_NUMBER_OF_RELOCATIONS_FOR_EXTENDED;
                        return 0;
                }
		*result = 1;
	} else {
		*result = 0;
	}
	return 1;
}

int epep_get_section_extended_number_of_relocations(Epep *epep, EpepSectionHeader *sh, size_t *result) {
	EpepCoffRelocation first_relocation;
	if (!epep_get_section_relocation_by_index(epep, sh, &first_relocation, 0)) {
		return 0;
	}
	*result = first_relocation.VirtualAddress;
	return 1;
}

int epep_get_section_number_of_relocations_x(Epep *epep, EpepSectionHeader *sh, size_t *result, int *extended) {
	if (!epep_section_contains_extended_relocations(epep, sh, extended)) {
		return 0;
	}
	if (*extended) {
		size_t real_number_of_relocations;
		if (!epep_get_section_extended_number_of_relocations(epep, sh, &real_number_of_relocations)) {
			return 0;
		}
		*result = real_number_of_relocations - 1;
	} else {
		*result = sh->NumberOfRelocations;
	}
	return 1;
}

int epep_get_section_relocation_by_index_x(Epep *epep, EpepSectionHeader *sh, EpepCoffRelocation *rel, size_t index, int extended) {
	return epep_get_section_relocation_by_index(epep, sh, rel, index + extended);
}

//
// COFF Line Numbers
//

int epep_get_section_line_number_by_index(Epep *epep, EpepSectionHeader *sh, EpepCoffLinenumber *ln, size_t index) {
	size_t offset = sh->PointerToLinenumbers + 6 * index;
	if (!epep_valid_offset(epep, offset, 6)) {
		epep->error_code = EPEP_ERR_INVALID_LINENUMBER_OFFSET;
		return 0;
	}
	epep_seek(epep, offset);
	epep_read_block(epep, 6, ln);
	return 1;
}

#endif // EPEP_INST

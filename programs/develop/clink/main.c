#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#define EPEP_INST
#include "epep/epep.h"

const char *epep_errors[] = {
	"EPEP_ERR_SUCCESS",
	"EPEP_ERR_DATA_DIRECTORY_INDEX_IS_INVALID",
	"EPEP_ERR_SECTION_HEADER_INDEX_IS_INVALID",
	"EPEP_ERR_SYMBOL_INDEX_IS_INVALID",
	"EPEP_ERR_NOT_AN_OBJECT",
	"EPEP_ERR_ADDRESS_IS_OUT_OF_SECTION_RAW_DATA",
	"EPEP_ERR_OUTPUT_CAPACITY_IS_ZERO",
	"EPEP_ERR_OUTPUT_IS_NULL",
	"EPEP_ERR_ADDRESS_IS_OUT_OF_ANY_SECTION",
	"EPEP_ERR_EXPORT_ADDRESS_TABLE_ENTRY_NAME_NOT_FOUND",
	"EPEP_ERR_NO_BASE_RELOCATION_TABLE",
	"EPEP_ERR_BASE_RELOCATION_IS_ALREADY_END",
	"EPEP_ERR_INVALID_DATA_DIRECTORY_OFFSET",
	"EPEP_ERR_INVALID_SECTION_HEADER_OFFSET",
	"EPEP_ERR_INVALID_SECTION_DATA_OFFSET",
	"EPEP_ERR_INVALID_STRING_TABLE_SIZE_OFFSET",
	"EPEP_ERR_INVALID_SYMBOL_OFFSET",
	"EPEP_ERR_INVALID_IMPORT_DIRECTORY_OFFSET",
	"EPEP_ERR_INVALID_IMPORT_DIRECTORY_NAME_OFFSET",
	"EPEP_ERR_INVALID_LOOKUP_OFFSET",
	"EPEP_ERR_INVALID_LOOKUP_NAME_OFFSET",
	"EPEP_ERR_INVALID_EXPORT_TABLE_OFFSET",
	"EPEP_ERR_INVALID_DLL_NAME_OFFSET",
	"EPEP_ERR_INVALID_EXPORT_NAME_POINTER_OFFSET",
	"EPEP_ERR_INVALID_ORDINAL_TABLE_OFFSET",
	"EPEP_ERR_INVALID_EXPORT_NAME_OFFSET",
	"EPEP_ERR_INVALID_EXPORT_ADDRESS_OFFSET",
	"EPEP_ERR_INVALID_FORWARDER_OFFSET",
	"EPEP_ERR_INVALID_BASE_RELOCATION_BLOCK_OFFSET",
	"EPEP_ERR_INVALID_NEXT_BASE_RELOCATION_BLOCK_OFFSET",
	"EPEP_ERR_INVALID_BASE_RELOCATION_BLOCK_BASE_RELOCATION_OFFSET",
	"EPEP_ERR_INVALID_SECTION_RELOCATION_OFFSET",
	"EPEP_ERR_INVALID_LINENUMBER_OFFSET",
	"EPEP_ERR_INVALID_NUMBER_OF_RELOCATIONS_FOR_EXTENDED",
};

static_assert(sizeof(epep_errors) / sizeof(epep_errors[0]) == EPEP_ERR_END,
              "Each EPEP error should be stringified.");

typedef char *pchar;

typedef struct {
	Epep epep;
	char *name;
	size_t *section_offsets;
} CoffObject;

typedef struct {
	size_t obj_id;
	size_t sec_id;
} ObjIdSecId;

typedef struct {
	ObjIdSecId *source;
	uint32_t characteristics;
	size_t size;
	size_t number_of_relocations;
	// Number of relocations is greater than 2^16 - 1
	int number_of_relocations_is_extended;
} SectionInfo;

typedef struct {
	EpepCoffSymbol sym;
	EpepCoffSymbol *auxes;
	char *name;
	size_t object_index;
	size_t index;
} Symbol;

#define CDICT_VAL_T SectionInfo
#define CDICT_INST
#include "cdict/cdict.h"

#define CDICT_VAL_T Symbol
#define CDICT_INST
#include "cdict/cdict.h"

typedef struct {
	CoffObject *objects;
	char **section_names_set;
	CDict_CStr_SectionInfo info_per_section;
	CDict_CStr_Symbol symtab;
	char **sym_name_set;
	size_t number_of_symbols;
} ObjectIr;

#define CVEC_INST
#define CVEC_TYPE CoffObject
#include "cvec/cvec.h"

#define CVEC_INST
#define CVEC_TYPE size_t
#include "cvec/cvec.h"

#define CVEC_INST
#define CVEC_TYPE pchar
#include "cvec/cvec.h"

#define CVEC_INST
#define CVEC_TYPE char
#include "cvec/cvec.h"

#define CVEC_INST
#define CVEC_TYPE ObjIdSecId
#include "cvec/cvec.h"

#define CVEC_INST
#define CVEC_TYPE EpepCoffSymbol
#include "cvec/cvec.h"

#define ERROR_EPEP(epep) printf("Error: epep returned %u (%s) at "__FILE__":%u", \
                                (epep)->error_code, epep_errors[(epep)->error_code], __LINE__); exit(-1)

#define ERROR_CDICT(cdict) printf("Error: cdict returned %u at "__FILE__":%u", \
                                  (cdict)->error_code, __LINE__); exit(-1);

static int emit_logs;

static void log_info(const char *fmt, ...) {
	if (emit_logs) {
		va_list ap;
		va_start(ap, fmt);
		vprintf(fmt, ap);
		va_end(ap);
	}
}

static void fwrite8(FILE *f, uint8_t b) {
	fputc(b, f);
}

static void fwrite16(FILE *f, uint16_t w) {
	fputc((w & 0x00ff) >> 0, f);
	fputc((w & 0xff00) >> 8, f);
}

static void fwrite32(FILE *f, uint32_t d) {
	fputc((d & 0x000000ff) >> 0, f);
	fputc((d & 0x0000ff00) >> 8, f);
	fputc((d & 0x00ff0000) >> 16, f);
	fputc((d & 0xff000000) >> 24, f);
}

static size_t strtab_add(char **strtab, char *str) {
	size_t res = cvec_char_size(strtab);

	for (char *p = str; *p; p++) {
		cvec_char_push_back(strtab, *p);
	}
	cvec_char_push_back(strtab, '\0');
	return res + 4;
}

static size_t get_section_number(char ***section_names_set, char *sec_name) {
	for (size_t i = 0; i < cvec_pchar_size(section_names_set); i++) {
		char *it = cvec_pchar_at(section_names_set, i);
		if (!strcmp(it, sec_name)) {
			return i + 1;
		}
	}
	return 0;
}

static void add_name_to_set(char *sym_name, char ***set) {
	for (size_t i = 0; i < cvec_pchar_size(set); i++) {
		char *it = cvec_pchar_at(set, i);
		if (!strcmp(it, sym_name)) {
			return;
		}
	}
	cvec_pchar_push_back(set, sym_name);
}

static void build(ObjectIr *ir, const char *outname) {
	FILE *out = fopen(outname, "wb");
	char *strtab = cvec_char_new(1024);
	size_t size_of_sections = 0;
	size_t number_of_relocations = 0;

	log_info("Calculating all sections size and relocations count... ");
	for (size_t sec_i = 0; sec_i < cvec_pchar_size(&ir->section_names_set); sec_i++) {
		char *name = ir->section_names_set[sec_i];

		SectionInfo si = cdict_CStr_SectionInfo_get_v(&ir->info_per_section, name);
		size_of_sections += si.size;
		number_of_relocations += si.number_of_relocations;
	}
	log_info("Done: %u & %u\n", size_of_sections, number_of_relocations);

	size_t fisrt_section_offset = 20 + 40 * cvec_pchar_size(&ir->section_names_set);
	size_t offset_to_first_relocation = fisrt_section_offset + size_of_sections;
	size_t offset_to_next_relocation = offset_to_first_relocation;
	size_t next_section_offset = fisrt_section_offset;

	size_t PointerToSymbolTable = fisrt_section_offset + size_of_sections + number_of_relocations * 10;

	// COFF Header
	log_info("Writing COFF header... ");
	fwrite16(out, 0x14c);                                   // Machine
	fwrite16(out, cvec_pchar_size(&ir->section_names_set)); // NumberOfSections
	fwrite32(out, 0);                                       // TimeDataStamp
	fwrite32(out, PointerToSymbolTable);                    // PointerToSymbolTable
	fwrite32(out, ir->number_of_symbols);                   // NumberOfSymbols
	fwrite16(out, 0);                                       // SizeOfOptionalHeader
	fwrite16(out, 0);                                       // Characteristics
	log_info("Done.\n");

	// Section Headers
	log_info("Writing section headers {\n");
	for (size_t sec_i = 0; sec_i < cvec_pchar_size(&ir->section_names_set); sec_i++) {
		char *name = ir->section_names_set[sec_i];
		SectionInfo si = cdict_CStr_SectionInfo_get_v(&ir->info_per_section, name);

		// Name
		log_info(" Writing %s Section Header... ", name);
		if (strlen(name) <= 8) {
			for (size_t i = 0; i < 8; i++) {
				size_t sl = strlen(name);
				fwrite8(out, i < sl ? name[i] : '\0');
			}
		} else {
			fwrite8(out, '/');

			size_t strtab_index = strtab_add(&strtab, name);
			char numstr[8] = { 0 };
			sprintf(numstr, "%lu", strtab_index);
			fwrite(numstr, 1, 7, out);
		}
		fwrite32(out, 0);                         // VirtualSize
		fwrite32(out, 0);                         // VirtualAddress
		fwrite32(out, si.size);                   // SizeOfRawData
		fwrite32(out, next_section_offset);       // PointerToRawData
		next_section_offset += si.size;
		fwrite32(out, offset_to_next_relocation); // PointerToRelocations
		offset_to_next_relocation += si.number_of_relocations * 10;
		fwrite32(out, 0);                         // PointerToLinenumbers
		// NumberOfRelocations
		if (si.number_of_relocations_is_extended) {
			fwrite16(out, 0xffff);
		} else {
			fwrite16(out, si.number_of_relocations);
		}
		fwrite16(out, 0);                         // NumberOfLinenumbers
		fwrite32(out, si.characteristics);        // Characteristics
		log_info("Done.\n");
	}
	log_info("}\n");

	// Section data
	log_info("Writing sections {\n");
	for (size_t sec_i = 0; sec_i < cvec_pchar_size(&ir->section_names_set); sec_i++) {
		char *name = ir->section_names_set[sec_i];
		SectionInfo si = cdict_CStr_SectionInfo_get_v(&ir->info_per_section, name);

		log_info(" Writing %s... ", name);
		for (size_t i = 0; i < cvec_ObjIdSecId_size(&si.source); i++) {
			ObjIdSecId id = cvec_ObjIdSecId_at(&si.source, i);
			CoffObject *object = &ir->objects[id.obj_id];
			Epep *epep = &object->epep;

			EpepSectionHeader sh = { 0 };
			if (!epep_get_section_header_by_index(epep, &sh, id.sec_id)) {
				ERROR_EPEP(epep);
			}

			// If the section contains uninitialized data (BSS)
			// it should be filled by zeroes
			// Yes, current implementation emits BSS sections too
			// cause KOS has no idea they should be allocated automatically
			// cause FASM has no idea they should be generated without contents
			// cause Tomasz Grysztar didn't care
			char *buf = calloc(sh.SizeOfRawData, 1);

			// Othervice it should be filled by its contents from source object
			if (!(sh.Characteristics & 0x00000080)) {
				if (!epep_get_section_contents(epep, &sh, buf)) {
					ERROR_EPEP(epep);
				}
			}

			fwrite(buf, 1, sh.SizeOfRawData, out);
		}
		log_info("Done.\n");
	}
	log_info("}\n");

	// COFF Relocations
	char **undefined_symbols = cvec_pchar_new(8);

	log_info("Writing COFF Relocations {\n");
	for (size_t sec_i = 0; sec_i < cvec_pchar_size(&ir->section_names_set); sec_i++) {
		char *name = ir->section_names_set[sec_i];
		SectionInfo si = cdict_CStr_SectionInfo_get_v(&ir->info_per_section, name);

		log_info(" Writing relocations of %s {\n", name);
		if (si.number_of_relocations_is_extended) {
			EpepCoffRelocation rel = { 0 };
			rel.VirtualAddress = si.number_of_relocations;
			fwrite(&rel, 1, 10, out);
		}
		for (size_t i = 0; i < cvec_ObjIdSecId_size(&si.source); i++) {
			ObjIdSecId id = cvec_ObjIdSecId_at(&si.source, i);
			CoffObject *object = &ir->objects[id.obj_id];
			Epep *epep = &object->epep;

			size_t strtab_size = 0;
			if (!epep_get_string_table_size(epep, &strtab_size)) {
				ERROR_EPEP(epep);
			}

			char *obj_strtab = malloc(strtab_size);
			if (!epep_get_string_table(epep, obj_strtab)) {
				ERROR_EPEP(epep);
			}

			EpepSectionHeader sh = { 0 };
			if (!epep_get_section_header_by_index(epep, &sh, id.sec_id)) {
				ERROR_EPEP(epep);
			}
			size_t number_of_relocations = 0;
			int extended = 0;
			if (!epep_get_section_number_of_relocations_x(epep, &sh, &number_of_relocations, &extended)) {
				ERROR_EPEP(epep);
			}
			for (size_t rel_i = 0; rel_i < number_of_relocations; rel_i++) {
				EpepCoffRelocation rel = { 0 };

				if (!epep_get_section_relocation_by_index_x(epep, &sh, &rel, rel_i, extended)) {
					ERROR_EPEP(epep);
				}
				log_info("  { %02x, %02x, %02x }", rel.VirtualAddress, rel.SymbolTableIndex, rel.Type);
				rel.VirtualAddress += object->section_offsets[sec_i];
				{
					size_t index = rel.SymbolTableIndex;
					EpepCoffSymbol sym = { 0 };

					if (!epep_get_symbol_by_index(epep, &sym, index)) {
						ERROR_EPEP(epep);
					}

					size_t name_max = 1024;
					char name[name_max];

					if (sym.symbol.Zeroes == 0) {
						strcpy(name, &obj_strtab[sym.symbol.Offset]);
					} else {
						memcpy(name, sym.symbol.ShortName, 8);
						name[8] = '\0';
					}

					if (!strcmp(name, "_EXPORTS")) {
						strcpy(name, "EXPORTS");
					}

					if (sym.symbol.StorageClass != 2) {
						sprintf(name, "%s@%s", name, object->name);
					}

					Symbol old_sym = cdict_CStr_Symbol_get_v(&ir->symtab, name);

					if (old_sym.name == NULL) {
						add_name_to_set(strdup(name), &undefined_symbols);
					}

					rel.SymbolTableIndex = old_sym.index;
					log_info(" -> { %02x, %02x, %02x }: ", rel.VirtualAddress, rel.SymbolTableIndex, rel.Type);
					log_info("New relocation of %s in %s\n", name, sh.Name);
				}
				fwrite(&rel, 1, 10, out);
			}
		}
		log_info(" }\n");
	}
	log_info("}\n");

	if (cvec_pchar_size(&undefined_symbols) > 0) {
		printf("Undefined symbols found, aborting\nUndefined:\n");
		for (int i = 0; i < cvec_pchar_size(&undefined_symbols); i++) {
			printf("%s\n", undefined_symbols[i]);
		}
		exit(-1);
	}

	// Symbols Table
	log_info("Writing symbols {\n");
	for (size_t sym_i = 0; sym_i < cvec_pchar_size(&ir->sym_name_set); sym_i++) {
		char *name = ir->sym_name_set[sym_i];

		Symbol sym = cdict_CStr_Symbol_get_v(&ir->symtab, name);

		if (sym.sym.symbol.SectionNumber == 0xffff ||
			sym.sym.symbol.SectionNumber == 0xfffe ||
			(sym.sym.symbol.StorageClass != 2 &&  // Not an external symbol
			 sym.sym.symbol.StorageClass != 3 &&  // Not a static symbol
			 sym.sym.symbol.StorageClass != 6)) { // Not a label
			fwrite(&sym.sym.symbol, 1, 18, out);
		} else {
			size_t sec_name_max = 1024;
			char sec_name[sec_name_max];

			size_t object_index = sym.object_index;
			CoffObject *object = &ir->objects[object_index];
			Epep *epep = &object->epep;
			size_t section_offset = object->section_offsets[sym.sym.symbol.SectionNumber - 1];

			size_t strtab_size = 0;
			if (!epep_get_string_table_size(epep, &strtab_size)) {
				ERROR_EPEP(epep);
			}

			char *obj_strtab = malloc(strtab_size);
			if (!epep_get_string_table(epep, obj_strtab)) {
				ERROR_EPEP(epep);
			}

			EpepSectionHeader sh = { 0 };
			if (!epep_get_section_header_by_index(epep, &sh, sym.sym.symbol.SectionNumber - 1)) {
				ERROR_EPEP(epep);
			}

			if (sh.Name[0] == '/') {
				strcpy(sec_name, &obj_strtab[atoi(&sh.Name[1])]);
			} else {
				memcpy(sec_name, sh.Name, 8);
				sec_name[8] = '\0';
			}

			log_info("%s:\n", sym.name);
			log_info(" Section:      %s\n", sec_name);
			log_info(" StorageClass: %u\n", sym.sym.symbol.StorageClass);

			sym.sym.symbol.SectionNumber = get_section_number(&ir->section_names_set, sec_name);

			if (sym.sym.symbol.SectionNumber == 0) {
				printf("Internal error: %s section is not found in output file", sec_name);
				exit(-1);
			}

			sym.sym.symbol.Value += section_offset;

			if (strlen(sym.name) <= 8) {
				memcpy(sym.sym.symbol.ShortName, sym.name, 8);
			} else {
				sym.sym.symbol.Zeroes = 0;
				sym.sym.symbol.Offset = strtab_add(&strtab, name);
			}

			fwrite(&sym.sym.symbol, 1, 18, out);
		}
		for (size_t aux_i = 0; aux_i < sym.sym.symbol.NumberOfAuxSymbols; aux_i++) {
			fwrite(&sym.auxes[aux_i].symbol, 1, 18, out);
		}
	}
	log_info("}\n");

	// COFF String Table
	log_info("Writing COFF String Table... ");
	fwrite32(out, cvec_char_size(&strtab) + 4);
	fwrite(strtab, 1, cvec_char_size(&strtab), out);
	log_info("Done.\n");
}

static ObjectIr parse_objects(int argc, char **argv) {
	CoffObject *objects = cvec_CoffObject_new(128);
	char **section_names_set = cvec_pchar_new(4);
	char **sym_name_set = cvec_pchar_new(128);
	size_t number_of_symbols = 0;

	for (int i = 1; i < argc; i++) {
		// If one arg is NULL, that means it was a parameter and was cleared
		// It's not a input file name
		if (argv[i] == NULL) {
			continue;
		}

		log_info("Primary parsing of %s... ", argv[i]);

		CoffObject object = { 0 };
		object.name = argv[i];
		object.section_offsets = cvec_size_t_new(128);
		
		{
			FILE *fp = fopen(object.name, "rb");
			if (!fp) {
				printf("Error: Can't open \"%s\"", object.name);
				exit(-1);
			}

			if (!epep_init(&object.epep, fp)) {
				ERROR_EPEP(&object.epep);
			}
		}

		cvec_CoffObject_push_back(&objects, object);

		log_info("Done.\n");
	}

	CDict_CStr_Symbol symtab;

	if (!cdict_CStr_Symbol_init(&symtab)) {
		ERROR_CDICT(&symtab);
	}

	CDict_CStr_SectionInfo info_per_section;

	if (!cdict_CStr_SectionInfo_init(&info_per_section)) {
		ERROR_CDICT(&info_per_section);
	}

	for (size_t i = 0; i < cvec_CoffObject_size(&objects); i++) {
		log_info("Secondary parsing of %s {\n", objects[i].name);

		Epep *epep = &(objects[i].epep);

		size_t strtab_size = 0;
		if (!epep_get_string_table_size(epep, &strtab_size)) {
			ERROR_EPEP(epep);
		}

		char *strtab = malloc(strtab_size);
		if (!epep_get_string_table(epep, strtab)) {
			ERROR_EPEP(epep);
		}

		// Fill symbols table
		log_info(" Symbols {\n");
		for (size_t sym_i = 0; sym_i < epep->coffFileHeader.NumberOfSymbols; sym_i++) {
			EpepCoffSymbol sym = { 0 };

			if (!epep_get_symbol_by_index(epep, &sym, sym_i)) {
				ERROR_EPEP(epep);
			}

			size_t name_max = 1024;
			char name[name_max];

			if (sym.symbol.Zeroes == 0) {
				strcpy(name, &strtab[sym.symbol.Offset]);
			} else {
				memcpy(name, sym.symbol.ShortName, 8);
				name[8] = '\0';
			}

			if (!strcmp(name, "_EXPORTS")) {
				strcpy(name, "EXPORTS");
			}

			if (sym.symbol.StorageClass != 2) {
				sprintf(name, "%s@%s", name, objects[i].name);
			}

			if (sym.symbol.StorageClass != 2 || sym.symbol.SectionNumber) {
				if (memcmp(cdict_CStr_Symbol_get_v(&symtab, name).sym.symbol.ShortName, "\0\0\0\0\0\0\0\0", 8)) {
					printf("Error: Redefinition of \"%s\"", name);
					exit(-1);
				}

				EpepCoffSymbol *auxes = cvec_EpepCoffSymbol_new(1);
				size_t index = number_of_symbols;

				for (size_t aux_i = 0; aux_i < sym.symbol.NumberOfAuxSymbols; aux_i++) {
					EpepCoffSymbol aux = { 0 };

					if (!epep_get_symbol_by_index(epep, &aux, sym_i + aux_i)) {
						ERROR_EPEP(epep);
					}
					cvec_EpepCoffSymbol_push_back(&auxes, aux);
					number_of_symbols++;
				}

				Symbol new_sym = { sym, auxes, strdup(name), i, index };
				if (!cdict_CStr_Symbol_add_vv(&symtab, strdup(name), new_sym, CDICT_NO_CHECK)) {
					ERROR_CDICT(&symtab);
				}
				number_of_symbols++;

				log_info("  Symbol #%u: %s (%u auxes, #%u)\n", sym_i, name, cvec_EpepCoffSymbol_size(&auxes), number_of_symbols - 1);

				add_name_to_set(strdup(name), &sym_name_set);
			}

			sym_i += sym.symbol.NumberOfAuxSymbols;
		}
		log_info(" }\n");

		// Set section offsets and fill unique section name set
		log_info(" Sections {\n");
		for (size_t sec_i = 0; sec_i < epep->coffFileHeader.NumberOfSections; sec_i++) {
			EpepSectionHeader sh = { 0 };

			if (!epep_get_section_header_by_index(epep, &sh, sec_i)) {
				ERROR_EPEP(epep);
			}

			size_t name_max = 1024;
			char name[name_max];

			if (sh.Name[0] == '/') {
				strcpy(name, &strtab[atoi(&sh.Name[1])]);
			} else {
				memcpy(name, sh.Name, 8);
				name[8] = '\0';
			}

			add_name_to_set(strdup(name), &section_names_set);

			SectionInfo si = cdict_CStr_SectionInfo_get_v(&info_per_section, name);
			if (si.source == NULL) {
				si.source = cvec_ObjIdSecId_new(32);
			}

			size_t sec_offset = si.size;
			cvec_size_t_push_back(&objects[i].section_offsets, sec_offset);

			size_t number_of_relocations = 0;
			int unused = 0;
			if (!epep_get_section_number_of_relocations_x(epep, &sh, &number_of_relocations, &unused)) {
				ERROR_EPEP(epep);
			}

			si.size += sh.SizeOfRawData;
			si.characteristics |= sh.Characteristics;
			si.number_of_relocations += number_of_relocations;
			if (si.number_of_relocations > 0xffff && !si.number_of_relocations_is_extended) {
				// One more relocation to store the actual relocation number
				si.number_of_relocations++;
				si.number_of_relocations_is_extended = 1;
				const uint32_t flag_IMAGE_SCN_LNK_NRELOC_OVFL = 0x01000000;
				si.characteristics |= flag_IMAGE_SCN_LNK_NRELOC_OVFL;
			}
			cvec_ObjIdSecId_push_back(&si.source, (ObjIdSecId){ i, sec_i });
			cdict_CStr_SectionInfo_add_vv(&info_per_section, strdup(name), si, CDICT_REPLACE_EXIST);

			log_info("  Section #%llu {\n", sec_i);
			log_info("   Name:                      %s\n", name);
			log_info("   Virtual Address:           %u\n", sh.VirtualAddress);
			log_info("   Characteristics:           %08x\n", sh.Characteristics);
			log_info("   Offset in the big section: %u\n", objects[i].section_offsets[sec_i]);
			log_info("  }\n");

			if (sh.VirtualAddress != 0) {
				printf("Warning: Handling of section with Virtual Address another that 0 is not implemented");
			}
		}
		log_info(" }\n");
		log_info("}\n");
	}

	ObjectIr ir;
	ir.objects = objects;
	ir.section_names_set = section_names_set;
	ir.info_per_section = info_per_section;
	ir.symtab = symtab;
	ir.sym_name_set = sym_name_set;
	ir.number_of_symbols = number_of_symbols;
	return ir;
}

int arg_got_flag(int argc, char **argv, ...) {
	char *arg_names[8];
	int arg_name_c = 0;

	va_list ap;
	va_start(ap, argv);
	for (char *arg_name = va_arg(ap, char *); arg_name; arg_name = va_arg(ap, char *)) {
		if (arg_name_c >= 8) {
			printf("Internal error: Too many parameter aliases passed to %s", __func__);
			exit(-1);
		}
		arg_names[arg_name_c++] = arg_name;
	}
	va_end(ap);

	for (int i = 1; i < argc; i++) {
		// If an argumetns was handled already then it's NULL here
		if (argv[i] == NULL) {
			continue;
		}
		for (int arg_name_i = 0; arg_name_i < arg_name_c; arg_name_i++) {
			char *arg_name = arg_names[arg_name_i];
			if (!strcmp(argv[i], arg_name)) {
				argv[i] = NULL; // Do not handle this argument as a input name
				return i;
			}
		}
	}
	return 0;
}

char *arg_got_param(int argc, char **argv, char *arg) {
	int i = arg_got_flag(argc, argv, arg, 0);
	if (i == 0) {
		return NULL;
	}

	if (i + 1 >= argc) {
		printf("Warning: %s parameter expects a value (like %s <value>)", arg, arg);
		return NULL;
	}
	char *result = argv[i + 1];
	argv[i + 1] = NULL;
	return result;
}

int usage(char *name, char *remark) {
	if (remark) {
		printf("Error: %s\n\n", remark);
	}
	printf("Usage: %s [option]... [object file name]...\n", name);
	printf("  Link multiple COFF files into one\n");
	printf("\n");
	printf("Options:\n");
	printf("  -o <outname>  Output into <outname>\n");
	printf("  -v, --verbose Emit information logs\n");
	printf("  -h, --help    Output this help and exit\n");
	return 0;
}

int main(int argc, char **argv) {
	if (arg_got_flag(argc, argv, "-h", "-help", "--help", 0)) {
		return usage(argv[0], NULL);
	}

	const char *outname = arg_got_param(argc, argv, "-o");
	if (outname == NULL) {
		outname = "a.out.obj";
	}

	emit_logs = arg_got_flag(argc, argv, "-v", "-verbose", "--verbose", 0);

	// After handling arguments there only leaven unhandled ones
	// They should be names if inputs. But if there's no input - exit
	int input_file_count = 0;
	for (int i = 1; i < argc; i++) {
		if (argv[i] != NULL) {
			input_file_count++;
		}
	}
	if (input_file_count == 0) {
		return usage(argv[0], "No input file names supplied");
	}

	ObjectIr ir = parse_objects(argc, argv);
	build(&ir, outname);
	return 0;
}

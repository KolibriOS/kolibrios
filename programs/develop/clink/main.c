// TODO: Substract section's RVA from external and static defined symbol's value
// TODO: Substract section's RVA from relocations

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#define EPEP_INST
#include "epep/epep.h"

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

#define ERROR_EPEP(epep) printf("Error: epep returned %u at "__FILE__":%u", \
                                (epep)->error_code, __LINE__); exit(-1)

#define ERROR_CDICT(cdict) printf("Error: cdict returned %u at "__FILE__":%u", \
                                  (cdict)->error_code, __LINE__); exit(-1);

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

static void build(ObjectIr *ir) {
	FILE *out = fopen("a.out.obj", "wb");
	char *strtab = cvec_char_new(1024);
	size_t size_of_sections = 0;
	size_t number_of_relocations = 0;

	printf("Calculating all sections size and relocations count... ");
	for (size_t sec_i = 0; sec_i < cvec_pchar_size(&ir->section_names_set); sec_i++) {
		char *name = ir->section_names_set[sec_i];

		SectionInfo si = cdict_CStr_SectionInfo_get_v(&ir->info_per_section, name);
		size_of_sections += si.size;
		number_of_relocations += si.number_of_relocations;
	}
	printf("Done: %u & %u\n", size_of_sections, number_of_relocations);

	size_t fisrt_section_offset = 20 + 40 * cvec_pchar_size(&ir->section_names_set);
	size_t offset_to_first_relocation = fisrt_section_offset + size_of_sections;
	size_t offset_to_next_relocation = offset_to_first_relocation;
	size_t next_section_offset = fisrt_section_offset;

	size_t PointerToSymbolTable = fisrt_section_offset + size_of_sections + number_of_relocations * 10;

	// COFF Header
	printf("Writing COFF header... ");
	fwrite16(out, 0x14c);                                   // Machine
	fwrite16(out, cvec_pchar_size(&ir->section_names_set)); // NumberOfSections
	fwrite32(out, 0);                                       // TimeDataStamp
	fwrite32(out, PointerToSymbolTable);                    // PointerToSymbolTable
	fwrite32(out, ir->number_of_symbols);                   // NumberOfSymbols
	fwrite16(out, 0);                                       // SizeOfOptionalHeader
	fwrite16(out, 0);                                       // Characteristics
	printf("Done.\n");

	// Section Headers
	printf("Writing section headers {\n");
	for (size_t sec_i = 0; sec_i < cvec_pchar_size(&ir->section_names_set); sec_i++) {
		char *name = ir->section_names_set[sec_i];
		SectionInfo si = cdict_CStr_SectionInfo_get_v(&ir->info_per_section, name);

		// Name
		printf(" Writing %s Section Header... ", name);
		if (strlen(name) <= 8) {
			for (size_t i = 0; i < 8; i++) {
				size_t sl = strlen(name);
				fwrite8(out, i < sl ? name[i] : '\0');
			}
		} else {
			fwrite8(out, '/');

			size_t strtab_index = strtab_add(&strtab, name);
			char numstr[8] = { 0 };
			sprintf(numstr, "%u", strtab_index);
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
		fwrite16(out, si.number_of_relocations);  // NumberOfRelocations
		fwrite16(out, 0);                         // NumberOfLinenumbers
		fwrite32(out, si.characteristics);        // Characteristics
		printf("Done.\n");
	}
	printf("}\n");

	// Section data
	printf("Writing sections {\n");
	for (size_t sec_i = 0; sec_i < cvec_pchar_size(&ir->section_names_set); sec_i++) {
		char *name = ir->section_names_set[sec_i];
		SectionInfo si = cdict_CStr_SectionInfo_get_v(&ir->info_per_section, name);

		printf(" Writing %s... ", name);
		for (size_t i = 0; i < cvec_ObjIdSecId_size(&si.source); i++) {
			ObjIdSecId id = cvec_ObjIdSecId_at(&si.source, i);
			CoffObject *object = &ir->objects[id.obj_id];
			Epep *epep = &object->epep;

			EpepSectionHeader sh = { 0 };
			if (!epep_get_section_header_by_index(epep, &sh, id.sec_id)) {
				ERROR_EPEP(epep);
			}
			char *buf = malloc(sh.SizeOfRawData);
			if (!epep_get_section_contents(epep, &sh, buf)) {
				ERROR_EPEP(epep);
			}
			fwrite(buf, 1, sh.SizeOfRawData, out);
		}
		printf("Done.\n");
	}
	printf("}\n");

	// COFF Relocations
	printf("Writing COFF Relocations {\n");
	for (size_t sec_i = 0; sec_i < cvec_pchar_size(&ir->section_names_set); sec_i++) {
		char *name = ir->section_names_set[sec_i];
		SectionInfo si = cdict_CStr_SectionInfo_get_v(&ir->info_per_section, name);

		printf(" Writing relocations of %s {\n", name);
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
			for (size_t rel_i = 0; rel_i < sh.NumberOfRelocations; rel_i++) {
				EpepCoffRelocation rel = { 0 };

				if (!epep_get_section_relocation_by_index(epep, &sh, &rel, rel_i)) {
					ERROR_EPEP(epep);
				}
				printf("  { %02x, %02x, %02x }", rel.VirtualAddress, rel.SymbolTableIndex, rel.Type);
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
						printf("Internal error: Symbol of %s relocation not found", name);
						exit(-1);
					}

					rel.SymbolTableIndex = old_sym.index;
					printf(" -> { %02x, %02x, %02x }: ", rel.VirtualAddress, rel.SymbolTableIndex, rel.Type);
					printf("New relocation of %s in %s\n", name, sh.Name);
				}
				fwrite(&rel, 1, 10, out);
			}
		}
		printf(" }\n");
	}
	printf("}\n");

	// Symbols Table
	printf("Writing symbols {\n");
	for (size_t sym_i = 0; sym_i < cvec_pchar_size(&ir->sym_name_set); sym_i++) {
		char *name = ir->sym_name_set[sym_i];

		Symbol sym = cdict_CStr_Symbol_get_v(&ir->symtab, name);

		if (sym.sym.symbol.SectionNumber == 0xffff ||
			sym.sym.symbol.SectionNumber == 0xfffe ||
			(sym.sym.symbol.StorageClass != 2 && sym.sym.symbol.StorageClass != 3)) {
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

			printf("%s:\n", sym.name);
			printf(" Section:      %s\n", sec_name);
			printf(" StorageClass: %u\n", sym.sym.symbol.StorageClass);

			sym.sym.symbol.SectionNumber = get_section_number(&ir->section_names_set, sec_name);

			if (sym.sym.symbol.SectionNumber == 0) {
				printf("Internal error: %s section is not found in output file");
				exit(-1);
			}

			sym.sym.symbol.Value += section_offset;

			if (strlen(sym.name) <= 8) {
				strcpy(sym.sym.symbol.ShortName, sym.name);
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
	printf("}\n");

	// COFF String Table
	printf("Writing COFF String Table... ");
	fwrite32(out, cvec_pchar_size(&strtab) + 4);
	fwrite(strtab, 1, cvec_pchar_size(&strtab), out);
	printf("Done.\n");
}

static ObjectIr parse_objects(int argc, char **argv) {
	CoffObject *objects = cvec_CoffObject_new(128);
	char **section_names_set = cvec_pchar_new(4);
	char **sym_name_set = cvec_pchar_new(128);
	size_t number_of_symbols = 0;

	for (int i = 1; i < argc; i++) {
		printf("Primary parsing of %s... ", argv[i]);

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

		printf("Done.\n");
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
		printf("Secondary parsing of %s {\n", objects[i].name);

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
		printf(" Symbols {\n");
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

				printf("  Symbol #%u: %s (%u auxes, #%u)\n", sym_i, name, cvec_EpepCoffSymbol_size(&auxes), number_of_symbols - 1);

				add_name_to_set(strdup(name), &sym_name_set);
			}

			sym_i += sym.symbol.NumberOfAuxSymbols;
		}
		printf(" }\n");

		// Set section offsets and fill unique section name set
		printf(" Sections {\n");
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

			si.size += sh.SizeOfRawData;
			si.characteristics |= sh.Characteristics;
			si.number_of_relocations += sh.NumberOfRelocations;
			cvec_ObjIdSecId_push_back(&si.source, (ObjIdSecId){ i, sec_i });
			cdict_CStr_SectionInfo_add_vv(&info_per_section, strdup(name), si, CDICT_REPLACE_EXIST);

			printf("  Section #%llu {\n", sec_i);
			printf("   Name:                      %s\n", name);
			printf("   Virtual Address:           %u\n", sh.VirtualAddress);
			printf("   Characteristics:           %08x\n", sh.Characteristics);
			printf("   Offset in the big section: %u\n", objects[i].section_offsets[sec_i]);
			printf("  }\n");
		}
		printf(" }\n");
		printf("}\n");
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

int main(int argc, char **argv) {
	ObjectIr ir = parse_objects(argc, argv);
	build(&ir);
}

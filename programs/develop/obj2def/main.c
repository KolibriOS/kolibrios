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
};

#define ERROR_EPEP(epep) printf("Error: epep returned %u (%s) at "__FILE__":%u", \
                                (epep)->error_code, epep_errors[(epep)->error_code], __LINE__); exit(-1)

static int emit_logs;
static void log_info(const char *fmt, ...) {
	if (emit_logs) {
		va_list ap;
		va_start(ap, fmt);
		vprintf(fmt, ap);
		va_end(ap);
	}
}

uint32_t get32(const char *buf, size_t offset) {
	return ((uint32_t)(uint8_t)buf[offset + 0] << 0)
		 | ((uint32_t)(uint8_t)buf[offset + 1] << 8)
		 | ((uint32_t)(uint8_t)buf[offset + 2] << 16)
		 | ((uint32_t)(uint8_t)buf[offset + 3] << 24);
}

EpepCoffRelocation get_relocation_for_section_and_offset(Epep *epep, EpepSectionHeader *sh, size_t offset) {
	EpepCoffRelocation rel = { 0 };
	for (size_t i = 0; i < sh->NumberOfRelocations; i++) {
		if (!epep_get_section_relocation_by_index(epep, sh, &rel, i)) {
			ERROR_EPEP(epep);
		}
		if (rel.VirtualAddress == offset) {
			return rel;
		}
	}
	printf("Error: Can't find relocation of pointer to name of symbol");
	exit(-1);
	return rel;
}

char *read_string(Epep *epep, size_t section_index, size_t offset) {
	EpepSectionHeader sh = { 0 };
	if (!epep_get_section_header_by_index(epep, &sh, section_index)) {
		ERROR_EPEP(epep);
	}
	char *section = calloc(1, sh.SizeOfRawData);
	if (!epep_get_section_contents(epep, &sh, section)) {
		ERROR_EPEP(epep);
	}
	char *result = strdup(&section[offset]);
	free(section);
	return result;
}

int gendef(const char *obj_path, const char *outname) {
	FILE *out = fopen(outname, "wb");
	Epep epep;
	{
		FILE *fp = fopen(obj_path, "rb");
		if (!fp) {
			printf("Error: Can't open \"%s\"", obj_path);
			exit(-1);
		}

		if (!epep_init(&epep, fp)) {
			ERROR_EPEP(&epep);
		}
	}

	size_t strtab_size = 0;
	if (!epep_get_string_table_size(&epep, &strtab_size)) {
		ERROR_EPEP(&epep);
	}

	char *strtab = malloc(strtab_size);
	if (!epep_get_string_table(&epep, strtab)) {
		ERROR_EPEP(&epep);
	}

	for (size_t sym_i = 0; sym_i < epep.coffFileHeader.NumberOfSymbols; sym_i++) {
		EpepCoffSymbol sym = { 0 };

		if (!epep_get_symbol_by_index(&epep, &sym, sym_i)) {
			ERROR_EPEP(&epep);
		}

		size_t name_max = 1024;
		char name[name_max];

		if (sym.symbol.Zeroes == 0) {
			strcpy(name, &strtab[sym.symbol.Offset]);
		} else {
			memcpy(name, sym.symbol.ShortName, 8);
			name[8] = '\0';
		}

		if (!strcmp(name, "_EXPORTS") || !strcmp(name, "EXPORTS")) {
			fprintf(out, "LIBRARY %s\n\nEXPORTS\n", obj_path);

			size_t export_table_offset_in_section = sym.symbol.Value;
			size_t section_index = sym.symbol.SectionNumber - 1;
			EpepSectionHeader sh = { 0 };
			if (!epep_get_section_header_by_index(&epep, &sh, section_index)) {
				ERROR_EPEP(&epep);
			}
			size_t section_offset = sh.PointerToRawData;
			size_t export_table_offset = section_offset + export_table_offset_in_section;

			char *section = calloc(1, sh.SizeOfRawData);
			if (!epep_get_section_contents(&epep, &sh, section)) {
				ERROR_EPEP(&epep);
			}

			for (size_t offset = export_table_offset_in_section;; offset += 8) {
				size_t name_offset = get32(section, offset);
				size_t data_offset = get32(section, offset + 4);

				if (name_offset == 0 || data_offset == 0) {
					break;
				}

				EpepCoffRelocation rel = get_relocation_for_section_and_offset(&epep, &sh, offset);
				EpepCoffSymbol name_sym = { 0 };
				if (!epep_get_symbol_by_index(&epep, &name_sym, rel.SymbolTableIndex)) {
					ERROR_EPEP(&epep);
				}
				size_t name_offset_in_section = name_sym.symbol.Value + name_offset;
				char *name = read_string(&epep, name_sym.symbol.SectionNumber - 1, name_offset_in_section);
				fprintf(out, "%s\n", name);
			}
			break;
		}
		sym_i += sym.symbol.NumberOfAuxSymbols;
	}
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
	printf("  Generate DEF files from COFF objects\n");
	printf("\n");
	printf("Options:\n");
	printf("  -v, --verbose Emit information logs\n");
	printf("  -h, --help    Output this help and exit\n");
	return 0;
}

int main(int argc, char **argv) {
	if (arg_got_flag(argc, argv, "-h", "-help", "--help", 0)) {
		return usage(argv[0], NULL);
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

	for (size_t i = 1; i < argc; i++) {
		char outname[256] = { 0 };
		sprintf(outname, "%.*s.def", strlen(argv[i]) - strlen(".obj"), argv[i]);
		if (argv[i] != NULL) {
			gendef(argv[i], outname);
		}
	}
	return 0;
}

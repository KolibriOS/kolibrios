#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define EPEP_INST
#include "epep.h"

#define ERROR(epep) (printf("Error #%u from EPEP at " __FILE__ ": %u", epep.error_code, __LINE__), 1)

int main(int argc, char **argv) {
	if (argc < 2) {
		printf("Usage:\n%s <filename>\n", argv[0]);
		return 0;
	}
	Epep epep = { 0 };
	FILE *fp = fopen(argv[1], "rb");
	if (!fp) {
		printf("File not found: %s\n", argv[1]);
		return 1;
	}
	if (!epep_init(&epep, fp)) {
		printf("Not PE");
		return 1;
	}
	printf("COFF File Header:\n");
	printf("  Machine:              %04x\n", epep.coffFileHeader.Machine);
	printf("  NumberOfSections:     %04x\n", epep.coffFileHeader.NumberOfSections);
	printf("  TimeDateStamp:        %08x\n", epep.coffFileHeader.TimeDateStamp);
	printf("  PointerToSymbolTable: %08x\n", epep.coffFileHeader.PointerToSymbolTable);
	printf("  NumberOfSymbols:      %08x\n", epep.coffFileHeader.NumberOfSymbols);
	printf("  SizeOfOptionalHeader: %04x\n", epep.coffFileHeader.SizeOfOptionalHeader);
	printf("  Characteristics:      %04x\n", epep.coffFileHeader.Characteristics);
	printf("\n");
	if (epep.coffFileHeader.SizeOfOptionalHeader != 0) {
		printf("Optional Header:\n");
		printf("  Magic:                       %04x\n", epep.optionalHeader.Magic);
		printf("  MajorLinkerVersion:          %02x\n", epep.optionalHeader.MajorLinkerVersion);
		printf("  MinorLinkerVersion:          %02x\n", epep.optionalHeader.MinorLinkerVersion);
		printf("  SizeOfCode:                  %08x\n", epep.optionalHeader.SizeOfCode);
		printf("  SizeOfInitializedData:       %08x\n", epep.optionalHeader.SizeOfInitializedData);
		printf("  SizeOfUninitializedData:     %08x\n", epep.optionalHeader.SizeOfUninitializedData);
		printf("  AddressOfEntryPoint:         %08x\n", epep.optionalHeader.AddressOfEntryPoint);
		printf("  BaseOfCode:                  %08x\n", epep.optionalHeader.BaseOfCode);
		printf("  BaseOfData:                  %08x\n", epep.optionalHeader.BaseOfData);
		printf("  ImageBase:                   %016x\n", epep.optionalHeader.ImageBase);
		printf("  SectionAlignment:            %08x\n", epep.optionalHeader.SectionAlignment);
		printf("  FileAlignment:               %08x\n", epep.optionalHeader.FileAlignment);
		printf("  MajorOperatingSystemVersion: %04x\n", epep.optionalHeader.MajorOperatingSystemVersion);
		printf("  MinorOperatingSystemVersion: %04x\n", epep.optionalHeader.MinorOperatingSystemVersion);
		printf("  MajorImageVersion:           %04x\n", epep.optionalHeader.MajorImageVersion);
		printf("  MinorImageVersion:           %04x\n", epep.optionalHeader.MinorImageVersion);
		printf("  MajorSubsystemVersion:       %04x\n", epep.optionalHeader.MajorSubsystemVersion);
		printf("  MinorSubsystemVersion:       %04x\n", epep.optionalHeader.MinorSubsystemVersion);
		printf("  Win32VersionValue:           %08x\n", epep.optionalHeader.Win32VersionValue);
		printf("  SizeOfImage:                 %08x\n", epep.optionalHeader.SizeOfImage);
		printf("  SizeOfHeaders:               %08x\n", epep.optionalHeader.SizeOfHeaders);
		printf("  CheckSum:                    %08x\n", epep.optionalHeader.CheckSum);
		printf("  Subsystem:                   %04x\n", epep.optionalHeader.Subsystem);
		printf("  DllCharacteristics:          %04x\n", epep.optionalHeader.DllCharacteristics);
		printf("  SizeOfStackReserve:          %016x\n", epep.optionalHeader.SizeOfStackReserve);
		printf("  SizeOfStackCommit:           %016x\n", epep.optionalHeader.SizeOfStackCommit);
		printf("  SizeOfHeapReserve:           %016x\n", epep.optionalHeader.SizeOfHeapReserve);
		printf("  SizeOfHeapCommit:            %016x\n", epep.optionalHeader.SizeOfHeapCommit);
		printf("  LoaderFlags:                 %08x\n", epep.optionalHeader.LoaderFlags);
		printf("  NumberOfRvaAndSizes:         %08x\n", epep.optionalHeader.NumberOfRvaAndSizes);
		printf("\n");
		printf("Data directories:\n");
		for (size_t i = 0; i < epep.optionalHeader.NumberOfRvaAndSizes; i++) {
			char *dds[] = {
				"Export Table",
				"Import Table",
				"Resource Table",
				"Exception Table",
				"Certificate Table",
				"Base Relocation Table",
				"Debug",
				"Architecture",
				"Global Ptr",
				"TLS Table",
				"Load Config Table",
				"Bound Import",
				"Import Address Table",
				"Delay Import Descriptor",
				"CLR Runtime Header",
				"Reserved, must be zero"
			};
			EpepImageDataDirectory idd = { 0 };
			if (!epep_get_data_directory_by_index(&epep, &idd, i)) {
				return ERROR(epep);
			}
			printf("  Data directory #%u:\n", i);
			printf("    Type:           %s\n", dds[i % 16]);
			printf("    VirtualAddress: %016x\n", idd.VirtualAddress);
			printf("    Size:           %016x\n", idd.Size);
			// Certificate table (4'th) data directory's VirtualAddress isn't a real RVA, it's a file offset
			// so it's actually outside of any section, so let's skip section name printing for it
			if (idd.VirtualAddress && i != 4) {
				EpepSectionHeader sh = { 0 };
				if (!epep_get_section_header_by_rva(&epep, &sh, idd.VirtualAddress)) {
					return ERROR(epep);
				}
				printf("    Section:        %s\n", sh.Name);
			}
		}
		printf("\n");
	}

	// Get string table useful to show long names of sections
	size_t string_table_size = 1;
	if (epep.kind == EPEP_OBJECT && !epep_get_string_table_size(&epep, &string_table_size)) {
		return ERROR(epep);
	}
	char *string_table = malloc(string_table_size);
	if (epep.kind == EPEP_OBJECT && !epep_get_string_table(&epep, string_table)) {
		return ERROR(epep);
	}

	printf("Section Table:\n");
	for (size_t i = 0; i < epep.coffFileHeader.NumberOfSections; i++) {
		EpepSectionHeader sh = { 0 };
		if (!epep_get_section_header_by_index(&epep, &sh, i)) {
			return ERROR(epep);
		}
		printf("  Section #%u\n", i);
		if (epep.kind == EPEP_OBJECT && sh.Name[0] == '/') {
			printf("    Name:                 %s\n", &string_table[atoi(sh.Name + 1)]);
		} else {
			printf("    Name:                 %.*s\n", 8, sh.Name);
		}
		printf("    VirtualSize:          %08x\n", sh.VirtualSize);
		printf("    VirtualAddress:       %08x\n", sh.VirtualAddress);
		printf("    SizeOfRawData:        %08x\n", sh.SizeOfRawData);
		printf("    PointerToRawData:     %08x\n", sh.PointerToRawData);
		printf("    PointerToRelocations: %08x\n", sh.PointerToRelocations);
		printf("    PointerToLinenumbers: %08x\n", sh.PointerToLinenumbers);
		printf("    NumberOfRelocations:  %08x\n", sh.NumberOfRelocations);
		printf("    NumberOfLinenumbers:  %08x\n", sh.NumberOfLinenumbers);
		printf("    Characteristics:      %08x\n", sh.Characteristics);
	}
	printf("\n");
	if (epep.kind == EPEP_OBJECT && epep.coffFileHeader.NumberOfSymbols != 0) {
		printf("Symbols:\n");
		for (size_t i = 0; i < epep.coffFileHeader.NumberOfSymbols; i++) {
			EpepCoffSymbol sym = { 0 };
			if (!epep_get_symbol_by_index(&epep, &sym, i)) {
				return ERROR(epep);
			}
			printf("  Symbol #%u\n", i);
			if (sym.symbol.Zeroes == 0) {
				printf("    Name:               %s\n", &string_table[sym.symbol.Offset]);
			} else {
				printf("    Name:               %.*s\n", 8, sym.symbol.ShortName);
			}
			printf("    Value:              %08x\n", sym.symbol.Value);
			printf("    SectionNumber:      %04x\n", sym.symbol.SectionNumber);
			printf("    Type:               %04x\n", sym.symbol.Type);
			printf("    StorageClass:       %02x\n", sym.symbol.StorageClass);
			printf("    NumberOfAuxSymbols: %02x\n", sym.symbol.NumberOfAuxSymbols);
			for (size_t j = 0; j < sym.symbol.NumberOfAuxSymbols; j++) {
				i++;
			}
		}
		printf("\n");
	}
	if (epep_has_import_table(&epep)) {
		printf("Import Directory Table:\n");
		for (size_t i = 0; i < 1024; i++) {
			EpepImportDirectory import_directory = { 0 };
			if (!epep_get_import_directory_by_index(&epep, &import_directory, i)) {
				return ERROR(epep);
			}
			if (import_directory.NameRva == 0) {
				break;
			}
			size_t name_max = 1024;
			char name[name_max];
			if (!epep_get_import_directory_name_s(&epep, &import_directory, name, name_max)) {
				return ERROR(epep);
			}
			printf("  Import Directory #%lu:\n", i);
			printf("    Name:                  %s\n", name);
			printf("    ImportLookupTableRva:  %08x\n", import_directory.ImportLookupTableRva);
			printf("    TimeDateStamp:         %08x\n", import_directory.TimeDateStamp);
			printf("    ForwarderChain:        %08x\n", import_directory.ForwarderChain);
			printf("    ImportAddressTableRva: %08x\n", import_directory.ImportAddressTableRva);
			for (size_t j = 0; j < 1024 * 1024; j++) {
				size_t lookup = 0;
				if (!epep_get_import_directory_lookup_by_index(&epep, &import_directory, &lookup, j)) {
					return ERROR(epep);
				}
				if (lookup == 0) {
					break;
				}
				size_t name_max = 1024;
				char name[name_max];
				if (!epep_get_lookup_name_s(&epep, lookup, name, name_max)) {
					return ERROR(epep);
				}
				printf("      Lookup:              %016x (%s)\n", lookup, name);
			}
		}
		printf("\n");
	}
	if (epep_has_export_table(&epep)) {
		if (!epep_read_export_directory(&epep)) {
			return ERROR(epep);
		}
		size_t name_max = 256;
		char name[name_max];
		strcpy(name, "undefined");
		if (!epep_get_dll_name_s(&epep, name, name_max)) {
			return ERROR(epep);
		}
		printf("Export Directory:\n");
		printf("  ExportFlags:           %08x\n", epep.export_directory.ExportFlags);
		printf("  TimeDateStamp:         %08x\n", epep.export_directory.TimeDateStamp);
		printf("  MajorVersion:          %04x\n", epep.export_directory.MajorVersion);
		printf("  MinorVersion:          %04x\n", epep.export_directory.MinorVersion);
		printf("  NameRva:               %08x (%s)\n", epep.export_directory.NameRva, name);
		printf("  OrdinalBase:           %08x\n", epep.export_directory.OrdinalBase);
		printf("  AddressTableEntries:   %08x\n", epep.export_directory.AddressTableEntries);
		printf("  NumberOfNamePointers:  %08x\n", epep.export_directory.NumberOfNamePointers);
		printf("  ExportAddressTableRva: %08x\n", epep.export_directory.ExportAddressTableRva);
		printf("  NamePointerRva:        %08x\n", epep.export_directory.NamePointerRva);
		printf("  OrdinalTableRva:       %08x\n", epep.export_directory.OrdinalTableRva);
		printf("  Exports:\n");
		for (size_t i = 0; i < epep.export_directory.AddressTableEntries; i++) {
			printf("    Export #%u:\n", i);
			size_t name_max = 1024;
			char name[name_max];
			printf("      Ordinal:      %u\n", epep.export_directory.OrdinalBase + i);
			if (epep_get_export_name_s_by_index(&epep, name, name_max, i)) {
				printf("      Name:         %s\n", name);
			}
			EpepExportAddress ea = { 0 };
			if (!epep_get_export_address_by_index(&epep, &ea, i)) {
				return ERROR(epep);
			}
			if (epep_export_address_is_forwarder(&epep, &ea)) {
				size_t forwarder_max = 1024;
				char forwarder[forwarder_max];
				if (!epep_get_export_address_forwarder_s(&epep, &ea, forwarder, forwarder_max)) {
					return ERROR(epep);
				}
				printf("      ForwarderRva: %08x (%s)\n", ea.ForwarderRva, forwarder);
			} else {
				printf("      ExportRva:    %08x\n", ea.ExportRva);
			}
		}
		printf("\n");
	}
	if (epep_has_base_relocation_table(&epep)) {
		printf("Base Relocations:\n");
		EpepBaseRelocationBlock brb = { 0 };
		if (!epep_get_first_base_relocation_block(&epep, &brb)) {
			return ERROR(epep);
		}
		for (size_t i = 0; brb.offset; i++) {
			printf("  Base Relocation Block #%u:\n", i);
			printf("    PageRva:   %08x\n", brb.PageRva);
			printf("    BlockSize: %08x\n", brb.BlockSize);
			printf("    Relocations:\n");
			for (size_t j = 0; j < ((brb.BlockSize - 8) / 2); j++) {
				char *strs[] = {
					"IMAGE_REL_BASED_ABSOLUTE",
					"IMAGE_REL_BASED_HIGH",
					"IMAGE_REL_BASED_LOW",
					"IMAGE_REL_BASED_HIGHLOW",
					"IMAGE_REL_BASED_HIGHADJ",
					"IMAGE_REL_BASED_MIPS_JMPADDR | IMAGE_REL_BASED_ARM_MOV32 | IMAGE_REL_BASED_RISCV_HIGH20",
					"reserved, must be zero",
					"IMAGE_REL_BASED_THUMB_MOV32 | IMAGE_REL_BASED_RISCV_LOW12I",
					"IMAGE_REL_BASED_RISCV_LOW12S",
					"IMAGE_REL_BASED_MIPS_JMPADDR16",
					"IMAGE_REL_BASED_DIR64",
				};
				printf("      Relocation #%u:\n", j);
				EpepBaseRelocation br = { 0 };
				if (!epep_get_base_relocation_block_base_relocation_by_index(&epep, &brb, &br, j)) {
					return ERROR(epep);
				}
				printf("        Type:   %01x (%s)\n", br.Type, strs[br.Type % (sizeof(strs) / sizeof(*strs))]);
				printf("        Offset: %03x (%u)\n", br.Offset, br.Offset);
			}
			if (!epep_get_next_base_relocation_block(&epep, &brb)) {
				return ERROR(epep);
			}
		}
		printf("\n");
	}
	if (epep.kind == EPEP_OBJECT) {
		for (size_t i = 0; i < epep.coffFileHeader.NumberOfSections; i++) {
			EpepSectionHeader sh = { 0 };
			if (!epep_get_section_header_by_index(&epep, &sh, i)) {
				return ERROR(epep);
			}
			printf("  Relocations for section #%u", i);
			if (epep.kind == EPEP_OBJECT && sh.Name[0] == '/') {
				printf(" (%s)\n", &string_table[atoi(sh.Name + 1)]);
			} else {
				printf(" (%.*s)\n", 8, sh.Name);
			}
			for (size_t i = 0; i < sh.NumberOfRelocations; i++) {
				EpepCoffRelocation rel = { 0 };
				if (!epep_get_section_relocation_by_index(&epep, &sh, &rel, i)) {
					return ERROR(epep);
				}
				printf("    COFF Relocation #%u\n", i);
				printf("      VirtualAddress: %08x\n", rel.VirtualAddress);
				printf("      SymbolTableIndex: %08x\n", rel.SymbolTableIndex);
				printf("      Type: %04x\n", rel.Type);
			}
		}
	}
	if (epep.kind == EPEP_OBJECT) {
		for (size_t i = 0; i < epep.coffFileHeader.NumberOfSections; i++) {
			EpepSectionHeader sh = { 0 };
			if (!epep_get_section_header_by_index(&epep, &sh, i)) {
				return ERROR(epep);
			}
			printf("  Linenumbers for section #%u", i);
			if (epep.kind == EPEP_OBJECT && sh.Name[0] == '/') {
				printf(" (%s)\n", &string_table[atoi(sh.Name + 1)]);
			} else {
				printf(" (%.*s)\n", 8, sh.Name);
			}
			for (size_t i = 0; i < sh.NumberOfLinenumbers; i++) {
				EpepCoffLinenumber ln = { 0 };
				if (!epep_get_section_line_number_by_index(&epep, &sh, &ln, i)) {
					return ERROR(epep);
				}
				printf("    COFF Line Number #%u\n", i);
				printf("      Type: %04x\n", ln.Type);
				printf("      Linenumber: %08x\n", ln.Linenumber);
			}
		}
	}
	return 0;
}

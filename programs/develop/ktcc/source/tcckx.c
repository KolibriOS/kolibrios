/*
 *  TCCKX.C - KolibriOS/KX file output for the TinyC Compiler
 *
 *  Copyright (c) 2021-2022 Coldy
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
 
#define TCC_KX_VERSION_INFO		"0.4.6"

typedef struct {
	char magic[4];
	long flags;
	long i_ptr;
} kx_header;

static kx_header __kx_header = { 'K','X',0, 0, 0x40, 0 };

typedef struct {
	 uint32_t ImportEntry;
	 uint32_t LibraryName;
 } LibraryEntry;

 /*union ImportEntry {
 uint32_t ImportStr;
 uint32_t ImportPrt;
};*/

 //static char __kx_import_table_sym[] = "__i_ptr__";

 void kx_build_imports(me_info* me) {

	 ElfW(Sym) *sym;
	 int sym_index, sym_end;
	 sym_end = symtab_section->data_offset / sizeof(ElfW(Sym));
	 DLLReference **dllref = me->s1->loaded_dlls;
	 CString *str_arr, *len_arr, *sym_arr;
	 char dll_len;
	 int dll_loaded = me->s1->nb_loaded_dlls;
	 int nlib = 0;
	 int i;

	 if (me->header.version != 2)
		 return;

	 str_arr = tcc_malloc(sizeof(CString) * dll_loaded);

	 len_arr = tcc_malloc(sizeof(CString)* dll_loaded);
	 
	 sym_arr = tcc_malloc(sizeof(CString)* dll_loaded);

	 for (sym_index = 1; sym_index < sym_end; ++sym_index) {
		 sym = (ElfW(Sym) *)symtab_section->data + sym_index;
		 if (sym->st_shndx == SHN_UNDEF) {
			 const char *name = symtab_section->link->data + sym->st_name;
			 int dynsym_index = find_elf_sym(me->s1->dynsymtab_section, name);

			 if (dynsym_index == 0) {
				 //if (strcmp(name, __kx_import_table_sym) != 0) {
					 tcc_error/*_noabort*/("(kx) undefined symbol '%s'", name);
			 }

			 // KOS support 32 bit only
			 Elf32_Sym* dyn_sym = &((ElfW(Sym) *)me->s1->dynsymtab_section->data)[dynsym_index];
			 char* dll_name;
			 // TCC store dll index in dyn_sym->st_size field
			 i = dyn_sym->st_size - 1;
			 if (dllref[i]->level != -1) {
				 dll_name = dllref[i]->name;
				 dll_len = strlen(dll_name) + 1;

				 nlib++;

				 cstr_new(&str_arr[i]);
				 cstr_new(&len_arr[i]);
				 cstr_new(&sym_arr[i]);

				 cstr_ccat(&len_arr[i], dll_len);
				 cstr_cat(&str_arr[i], dll_name, dll_len);
				 //Mark dll as already used
				 dllref[i]->level = -1;
			 }
			 
			 cstr_wccat(&sym_arr[i], (int)name);

			 // Export defined with prefix?
			 if (dyn_sym->st_value == -1){
				 name += (dll_len - 4); // skip prefix_
			 }

			 char name_len = strlen(name) + 1;
			 cstr_ccat(&len_arr[i], name_len);
			 cstr_cat(&str_arr[i], name, name_len);

		 }
	 }

	 /*if (len_arr[0].size == 0)
	 {

		 //tcc_error("");
		 return;
	 }*/
   
	 // Fixed BUG#15 (possible access to uninitialized due unused library)
	 // Exclude unused librarys
	 if (nlib < dll_loaded) {
		 i = 0; int j, n = 0;
		 do {

			 // Find unused library
			 if (dllref[i]->level == 0) {
				 j = i + 1;
				 
				 while (j < dll_loaded) {
					 // Find first used library
					 if (dllref[j]->level == -1) {
						 // Found, copy i from j
						 str_arr[i] = str_arr[j];
						 len_arr[i] = len_arr[j];
						 sym_arr[i] = sym_arr[j];
						 // Mark j as unused
						 dllref[j]->level = 0;

						 if (++n == nlib)
							 goto __done;

						 break;
					 }

					 j++;

				 }

			 }

			 i++;

		 } while (i < dll_loaded);

	 }

 __done:

	 // Zero terminate of ptr (was BUG#3)
	 i = 0;
	 do {

		 cstr_ccat(&len_arr[i], 0);

		 i++;

	 } while (i < nlib);

	 kx_import_table* imp_sect;

	 imp_sect = tcc_mallocz(sizeof(kx_import_table));
	 imp_sect->data = tcc_mallocz(4096); // FIXME!!! I increased it to 4Kb, but steel need dynamicaly size
	 imp_sect->data_size = 0;
	 //imp_sect->sh_addr = me->header.image_size;// +1;
	 
	 long imp_data = (long)imp_sect->data; //FIXED changed to long for gcc compatible

	 // Strings
	 i = 0;
	 do {
		 memcpy((void*)imp_data, str_arr[i].data, str_arr[i].size);
		 imp_data += str_arr[i].size;
		 imp_sect->data_size += str_arr[i].size;

		 i++;

	 } while (i < nlib);

	 // Align pad (check algorithm!)
	 int align = 4 - (me->header.image_size + imp_sect->data_size) % 4;
	 align = align < 4 ? align : 0;
	 imp_data += align;
	 imp_sect->data_size += align;

	 /*add_elf_sym(
		 me->s1->dynsymtab_section,
		 me->header.image_size + imp_sect->data_size,
		 0, ELFW(ST_INFO)(STB_GLOBAL, STT_NOTYPE),
		 0, SHN_ABS, __kx_import_table_sym);*/
	 __kx_header.i_ptr = me->header.image_size + imp_sect->data_size;

	 LibraryEntry lib;
	 lib.ImportEntry = me->header.image_size + imp_sect->data_size + (nlib * 8) + 4;
	 lib.LibraryName = me->header.image_size + 0;

	 // LibraryEntry 
	 memcpy((void*)imp_data, &lib, sizeof(LibraryEntry));

	 if (nlib > 1) {
		 int prev_sum = 0;
		 int prev = 0;
		 i = 1;
		 do {
			 lib.ImportEntry += (len_arr[prev].size - 2) * 4 + 4; //TODO: check that +4 is correct
			 prev_sum += str_arr[prev].size;
			 lib.LibraryName = me->header.image_size + prev_sum; // FIXED (was BUG#10)
			 imp_data += sizeof(LibraryEntry);
			 imp_sect->data_size += sizeof(LibraryEntry);
			 memcpy((void*)imp_data, &lib, sizeof(LibraryEntry));

			 prev++;
			 i++;

		 } while (i < nlib);
	 }

	 // End of LibraryEntry
	 imp_data += sizeof(LibraryEntry) + 4;
	 imp_sect->data_size += sizeof(LibraryEntry) + 4;

	 const char *sym_name;
	 char name_len;
	 long len_sum;

	 len_sum = me->header.image_size;
	 i = 0;
	 do {
		 char* len_data = len_arr[i].data;
		 long* sym_data = sym_arr[i].data;

		 name_len = *len_data++; // Skip library name

		 do {
			 			 
			 memcpy(&sym_name, sym_data++, 4);
			 
			 add_elf_sym(
				 me->s1->dynsymtab_section,
				 me->header.image_size + imp_sect->data_size,
				 0, ELFW(ST_INFO)(STB_GLOBAL, STT_NOTYPE),
				 0, SHN_ABS, sym_name);

			 len_sum += name_len;
			 memcpy((void*)imp_data, &len_sum, 4);

			 imp_data += 4;
			 imp_sect->data_size += 4;
			 name_len = */*++*/len_data/*++*/;		//(was BUG#3)

		 } while (/*name_len*/*(++len_data) > 0);

		 imp_data += 4;
		 imp_sect->data_size += 4;

		 len_sum += name_len;
		 i++;

	 } while (i < nlib);

	 me->header.image_size += imp_sect->data_size;
	 me->imp_table = imp_sect;

	 tcc_free(str_arr);
	 tcc_free(len_arr);
	 tcc_free(sym_arr);

 }

 

 void kx_init(me_info* me) {
	 ElfW(Sym) *sym;
	 int sym_index = 1, sym_end = symtab_section->data_offset / sizeof(ElfW(Sym));
	 // Check that we have at last one import...
	 for (; sym_index < sym_end; ++sym_index) {
		 sym = (ElfW(Sym) *)symtab_section->data + sym_index;
		 if (sym->st_shndx == SHN_UNDEF) 
			 break;
	 }
	 if ((sym_index < sym_end) &&
		 // ... and user attached at least one *.def
		 (me->s1->nb_loaded_dlls))
			me->header.version = 2;
 }

 long kx_get_header_length(me_info* me) {
	 if (me->header.version == 2)
		 return sizeof(kx_header);

	 return 0;
 }

 void kx_write_header(me_info* me, FILE* f) {
	 if (me->header.version == 2)
		 fwrite(&__kx_header, 1, sizeof(kx_header), f);
 }

 void kx_write_imports(me_info* me, FILE* f) {
	 if (me->imp_table)
		 fwrite(me->imp_table->data, 1, me->imp_table->data_size, f);
		 
 }

 void kx_free(me_info* me) {
	 kx_import_table* imp = me->imp_table;
	 if (imp){
		 tcc_free(imp->data);
		 tcc_free(imp);
	}
 }

 // This routine is called from build_reloc to check the code for incorrect import calls
void kx_check_import_error(int reloc_type, const char* code_base, Elf32_Addr offset, const char* symbol_name) {
	 
	unsigned char fError = 0;
	char* p = (char*)(offset + code_base);

	// Hook for "[extern] rtype foo([?])" declaration
	if (((unsigned char)*((char*)(p-1))) == 0xe8){
		// call		m32
		tcc_error_noabort("import symbol '%s' has direct call (not supported),\n\t    use __attribute__((dllimport)) prefix", symbol_name);
		fError = 1;
	}

	//	Hook for MSVC "__declspec(dllimport) rtype(*foo)([?])"
	/*if ((((unsigned char)*(p + 4)) == 0xff) &&
		(((unsigned char)*(p + 5)) == 0x10)) {*/
	if (*((uint16_t*)(p + 4)) == 0x10ff) {
		/*
		   (mov		eax [m32])
			call	dword[eax]
		*/
		tcc_error_noabort("import symbol '%s' has unsupported call type", symbol_name);
		fError = 1;
	}
	if (reloc_type == R_386_PC32) {
		tcc_error_noabort("incorrect relocation type for import symbol '%s'", symbol_name);
		fError = 1;
	}
	if (fError)
		tcc_abort();
}
  
#if /*!*/defined(_DEBUG)// && !defined(_WIN32)
 #define	kx_debug_output		printf
#else
 #define kx_debug_output(s,p)	((void)0)
#endif

/*
	Calling once from tcc_set_lib_path_xxx
	This function correct tcc_root if tcc_root/kx is a run directory,
	otherwise do trim filename
*/
 void kx_fix_root_directory(char *buf, size_t size) {
	 
	 int defult = 1;
	 char* tcc_conf = tcc_malloc(strlen(buf)+5);
	 strcpy(tcc_conf, buf);
	 char* base = tcc_basename(tcc_conf);
	 *base = 0;
	 base = tcc_basename(buf);
	 strcat(tcc_conf, "tcc.conf");
	 FILE* f = fopen(tcc_conf,"r");
	 if (f) {
		 char line[100];
		 while (fgets(line, sizeof line, f)){
			 switch (*line)
				case '#':
				case '\n':
					continue;
					if ((strspn(line, "tcc_root") == 8) && line[8] == ' ') {

						if (strcmp(line + 9, "kx") == 0) {
							strcpy(base, line + 9);
							defult = 0;
						}
						else
						{
							// Disallow change tcc_root with arbitrary path
							continue;
						}

					}
		 }

		 fclose(f);
	 }
	 if (defult) {

		 *--base = 0;
	 }

	 tcc_free(tcc_conf);
	 //kx_debug_output("tcc root = %s\n", buf);
 }
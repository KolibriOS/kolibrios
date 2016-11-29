; Module management, non-PE-specific code.
; Works in conjuction with peloader.inc for PE-specific code.

;  void* dlopen(const char* filename, int mode)
;  Opens the module named filename and maps it in; returns a handle that can be
;  passed to dlsym to get symbol values from it.
;
;  If filename starts with '/', it is treated as an absolute file name.
;  Otherwise, dlopen searches for filename in predefined locations:
;  /rd/1/lib, /kolibrios/lib, directory of the executable file.
;  The current directory is *not* searched.
;
;  If the same module is loaded again with dlopen(), the same
;  handle is returned.  The loader maintains reference
;  counts for loaded modules, so a dynamically loaded module is
;  not deallocated until dlclose() has been called on it as many times
;  as dlopen() has succeeded on it.  Any initialization functions
;  are called just once.
;
;  If dlopen() fails for any reason, it returns NULL.
;
;  mode is reserved and should be zero.
proc dlopen stdcall uses esi edi, file, mode
; find_module_by_name and load_module do all the work.
; We just need to acquire/release the mutex and adjust input/output.
        cmp     [mode], 0
        jnz     .invalid_mode
        mutex_lock modules_mutex
        mov     edi, [file]
        call    find_module_by_name
        test    esi, esi
        jnz     .inc_refcount
        call    load_module
        xor     edi, edi
        test    eax, eax
        jz      .unlock_return
; The handle returned on success is module base address.
; Unlike pointer to MODULE struct, it can be actually useful
; for the caller as is.
        mov     edi, [eax+MODULE.base]
        jmp     .unlock_return
.inc_refcount:
        inc     [esi+MODULE.refcount]
        mov     edi, [esi+MODULE.base]
.unlock_return:
        mutex_unlock modules_mutex
        mov     eax, edi
        ret
.invalid_mode:
        xor     eax, eax
        ret
endp

;  int dlclose(void* handle)
;  Decrements the reference count on the dynamically loaded module
;  referred to by handle. If the reference count drops to zero,
;  then the module is unloaded. All modules that were automatically loaded
;  when dlopen() was invoked on the module referred to by handle are
;  recursively closed in the same manner.
;
;  A successful return from dlclose() does not guarantee that the
;  module has been actually removed from the caller's address space.
;  In addition to references resulting from explicit dlopen() calls,
;  a module may have been implicitly loaded (and reference counted)
;  because of dependencies in other shared objects.
;  Only when all references have been released can the module be removed
;  from the address space.
;  On success, dlclose() returns 0; on error, it returns a nonzero value.
proc dlclose stdcall uses esi, handle
; This function uses two worker functions:
; find_module_by_addr to map handle -> MODULE,
; dereference_module for the main work.
; Aside of calling these, we should only acquire/release the mutex.
        mutex_lock modules_mutex
        mov     ecx, [handle]
        call    find_module_by_addr
        test    esi, esi
        jz      .invalid_handle
        call    dereference_module
        mutex_unlock modules_mutex
        xor     eax, eax
        ret
.invalid_handle:
        mutex_unlock modules_mutex
        xor     eax, eax
        inc     eax
        ret
endp

;  void* dlsym(void* handle, const char* symbol)
;  Obtains address of a symbol in a module.
;  On failure, returns NULL.
;
;  symbol can also be a number between 0 and 0xFFFF;
;  it is interpreted as an ordinal of a symbol.
;  Low 64K of address space are blocked for the allocation,
;  so a valid pointer cannot be less than 0x10000.
;
;  handle is not validated. Passing an invalid handle can result in a crash.
proc dlsym stdcall, handle, symbol
locals
export_base             dd      ?
export_ptr              dd      ?
export_size             dd      ?
import_module           dd      0
endl
; Again, helper functions do all the work.
; We don't need to browse list of MODULEs,
; so we don't need to acquire/release the mutex.
; Unless the function is forwarded or module name is required for error message,
; but this should be processed by get_exported_function_*.
        mov     eax, [handle]
        call    prepare_import_from_module
        mov     ecx, [symbol]
        cmp     ecx, 0x10000
        jb      .ordinal
        mov     edx, -1 ; no hint for lookup in name table
        call    get_exported_function_by_name
        ret
.ordinal:
        call    get_exported_function_by_ordinal
        ret
endp

; Errors happen.
; Some errors should be reported to the user. Some errors are normal.
; After the process has been initialized, we don't know what an error
; should mean - is the failed DLL absolutely required or unimportant enhancement?
; So we report an error to the caller and let it decide how to handle it.
; However, when the process is initializing, there is no one to report to,
; so we must inform the user ourselves.
; In any case, write to the debug board - it is *debug* board, after all.
;
; This function is called whenever an error occurs in the loader.
; Except errors in malloc/realloc - they shouldn't happen anyway,
; and if they happened after all, we are screwed and likely will fail anyway,
; so don't bother.
; Variable number of arguments: strings to be concatenated, end with NULL.
proc loader_say_error c uses ebx esi, first_msg, ...
; 1. Concatenate all given strings to the final error message.
; 1a. Calculate the total length.
        xor     ebx, ebx
        lea     edx, [first_msg]
.get_length:
        mov     ecx, [edx]
        test    ecx, ecx
        jz      .length_done
@@:
        inc     ebx
        inc     ecx
        cmp     byte [ecx-1], 0
        jnz     @b
        dec     ebx
        add     edx, 4
        jmp     .get_length
.length_done:
        inc     ebx ; terminating zero
; 1b. Allocate memory. Exit if failed.
        stdcall malloc, ebx
        test    eax, eax
        jz      .nothing
        mov     esi, eax
; 1c. Copy data.
        lea     edx, [first_msg]
.copy_data:
        mov     ecx, [edx]
        test    ecx, ecx
        jz      .data_done
@@:
        mov     bl, [ecx]
        test    bl, bl
        jz      @f
        mov     [eax], bl
        inc     ecx
        inc     eax
        jmp     @b
@@:
        add     edx, 4
        jmp     .copy_data
.data_done:
        mov     byte [eax], 0 ; terminating zero
; 2. Print to the debug board.
        mov     ecx, loader_debugboard_prefix
        call    sys_msg_board_str
        mov     ecx, esi
        call    sys_msg_board_str
        mov     ecx, msg_newline
        call    sys_msg_board_str
; 3. If the initialization is in process, report to the user.
        xor     eax, eax
        cmp     [process_initialized], al
        jnz     .no_report
; Use @notify. Create structure for function 70.7 on the stack.
        push    eax ; to be rewritten with part of path
        push    eax ; to be rewritten with part of path
        push    eax ; reserved
        push    eax ; reserved
        push    esi ; command line
        push    eax ; flags: none
        push    7
        mov     eax, 70
        mov     ebx, esp
        mov     dword [ebx+21], notify_program
        call    FS_SYSCALL_PTR
        add     esp, 28
; Ignore any errors. We can't do anything with them anyway.
.no_report:
        stdcall free, esi
.nothing:
        ret
endp

; When the loader is initializing the process, errors can happen.
; They should be reported to the user.
; The main executable cannot do this, it is not initialized yet.
; So we should do it ourselves.
; However, after the process has been initialized, the main 
;
; Helper function that is called whenever an error is occured.

; For now, we don't expect many modules in one process.
; So, all modules are linked into a single list,
; and lookup functions simply walk the entire list.
; This should be revisited if dozens of modules would be typical.

; This structure describes one loaded PE module.
; malloc'd from the default heap,
; includes variable-sized module path in the end.
struct MODULE
; All modules are linked in the global list with head at modules_list.
next            dd      ?
prev            dd      ?
base            dd      ?       ; base address
size            dd      ?       ; size in memory
refcount        dd      ?       ; reference counter
timestamp       dd      ?       ; for bound imports
basedelta       dd      ?       ; base address - preferred address, for bound imports
num_imports     dd      ?       ; size of imports array
imports         dd      ?
; Pointer to array of pointers to MODULEs containing imported functions.
; Used to unload all dependencies when the module is unloaded.
; Contains all modules referenced by import table;
; if the module forwards some export to another module,
; then forward target is added to this array when forward source is requested.
filename        dd      ?       ; pointer inside path array after dirname
filenamelen     dd      ?       ; strlen(filename) + 1
path            rb      0
ends

; Fills some fields in a new MODULE struct based on given PE image.
; Assumes that MODULE.path has been filled during the allocation,
; does not insert the structure in the common list, fills everything else.
; in: eax -> MODULE
; in: esi = module base
proc init_module_struct
; Straightforward initialization of all non-PE-specific fields.
        lea     edx, [eax+MODULE.path]
        mov     [eax+MODULE.filename], edx
@@:
        inc     edx
        cmp     byte [edx-1], 0
        jz      @f
        cmp     byte [edx-1], '/'
        jnz     @b
        mov     [eax+MODULE.filename], edx
        jmp     @b
@@:
        sub     edx, [eax+MODULE.filename]
        mov     [eax+MODULE.filenamelen], edx
        xor     edx, edx
        mov     [eax+MODULE.base], esi
        mov     [eax+MODULE.refcount], 1
        mov     [eax+MODULE.num_imports], edx
        mov     [eax+MODULE.imports], edx
; Let the PE-specific part do its job.
        init_module_struct_pe_specific
endp

; Helper function for dlclose and resolving forwarded exports from dlsym.
; in: ecx = module base address
; out: esi -> MODULE or esi = NULL
; modules_mutex should be locked
proc find_module_by_addr
; Simple linear lookup in the list.
        mov     esi, [modules_list + MODULE.next]
.scan:
        cmp     esi, modules_list
        jz      .notfound
        cmp     ecx, [esi+MODULE.base]
        jz      .found
        mov     esi, [esi+MODULE.next]
        jmp     .scan
.notfound:
        xor     esi, esi
.found:
        ret
endp

; Helper function for whenever we have a module name
; and want to check whether it is already loaded.
; in: edi -> name with or without a path
; out: esi -> MODULE or esi = NULL
; modules_mutex should be locked
proc find_module_by_name uses edi
; 1. Skip the path, if it is present.
; eax = current pointer,
; edi is updated whenever the previous character is '/'
        mov     eax, edi
.find_basename:
        cmp     byte [eax], 0
        jz      .found_basename
        inc     eax
        cmp     byte [eax-1], '/'
        jnz     .find_basename
        mov     edi, eax
        jmp     .find_basename
.found_basename:
; 2. Simple linear lookup in the list.
        mov     eax, [modules_list + MODULE.next]
.scan:
        cmp     eax, modules_list
        jz      .notfound
; For every module, compare base names ignoring paths.
        push    edi
        mov     esi, [eax+MODULE.filename]
        mov     ecx, [eax+MODULE.filenamelen]
        repz cmpsb
        pop     edi
        jz      .found
        mov     eax, [eax+MODULE.next]
        jmp     .scan
.found:
        mov     esi, eax
        ret
.notfound:
        xor     esi, esi
        ret
endp

; Called when some module is implicitly loaded by another module,
; either due to a record in import table,
; or because some exported function forwards to another module.
; Checks whether the target module has already been referenced
; by the source module. The first reference is passed down
; to load_module increasing refcount of the target and possibly
; loading it if not yet, subsequent references just return
; without modifying refcount.
; We don't actually need to deduplicate DLLs from import table
; as long as we decrement refcount on unload the same number of times
; that we have incremented it on load.
; However, we need to keep track of references to forward targets,
; and we don't want to scan the entire export table and load all forward
; targets just in case some of those would be useful,
; so load them on-demand first time and ignore subsequential references.
; To be consistent, do the same for import table too.
;
; in: esi -> source MODULE struct
; in: edi -> target module name
; out: eax -> imported MODULE, 0 on error
; modules_mutex should be locked
proc load_imported_module uses edi
; 1. Find the target module in the loaded modules list.
; If not found, go to 5.
        push    esi
        call    find_module_by_name
        test    esi, esi
        mov     eax, esi
        pop     esi
        jz      .load
; 2. The module has been already loaded.
; Now check whether it is already stored in imports array.
; If yes, just return without doing anything.
        mov     edi, [esi+MODULE.imports]
        mov     ecx, [esi+MODULE.num_imports]
        test    ecx, ecx
        jz      .newref
        repnz scasd
        jz      .nothing
.newref:
; The module is loaded, but not by us.
; 3. Increment the reference counter of the target.
        inc     [eax+MODULE.refcount]
.add_to_imports:
; 4. Add the new pointer to the imports array.
; 4a. Check whether there is place in the array.
; If so, go to 4c.
; We don't want to reallocate too often, since reallocation
; may involve copying our data to a new place.
; We always reserve space that is a power of two; in this way,
; the wasted space is never greater than the used space,
; and total time of copying the data is O(number of modules).
; The last fact is not really important right now,
; since the current implementation of step 2 makes everything
; quadratic and the number of modules is very small anyway,
; but since this enhancement costs only a few instructions, why not?
        mov     edi, eax
; X is a power of two or zero if and only if (X and (X - 1)) is zero
        mov     ecx, [esi+MODULE.num_imports]
        lea     edx, [ecx-1]
        test    ecx, edx
        jnz     .has_space
; 4b. Reallocate the imports array:
; if the current size is zero, allocate 1 item,
; otherwise double number of items.
; Item size is 4 bytes.
        lea     ecx, [ecx*8]
        test    ecx, ecx
        jnz     @f
        mov     ecx, 4
@@:
        stdcall realloc, [esi+MODULE.imports], ecx
        test    eax, eax
        jz      .realloc_failed
        mov     [esi+MODULE.imports], eax
        mov     ecx, [esi+MODULE.num_imports]
.has_space:
; 4c. Append pointer to the target MODULE to imports array.
        mov     eax, [esi+MODULE.imports]
        mov     [eax+ecx*4], edi
        inc     [esi+MODULE.num_imports]
        mov     eax, edi
.nothing:
        ret
.load:
; 5. This is a totally new module. Load it.
        call    load_module
; On error, return it to the caller. On success, go to 4.
        test    eax, eax
        jz      .nothing
        jmp     .add_to_imports
.realloc_failed:
; Out of memory for a couple of dwords? Should not happen.
; Dereference the target referenced by step 3 or 5
; and return error to the caller.
        push    esi
        mov     esi, edi
        call    dereference_module
        pop     esi
        xor     eax, eax
        ret
endp

; Helper procedure for load_module.
; Allocates MODULE structure for (given path) + (module name),
; calls the kernel to map it,
; on success, fills the MODULE structure.
; in: edi -> module name
; in: ebx = strlen(filename) + 1
proc try_map_module uses ebx esi, path_ptr, path_len
; 1. Allocate MODULE structure.
        mov     eax, [path_len]
        lea     eax, [eax+ebx+MODULE.path]
        stdcall malloc, eax
        test    eax, eax
        jz      .nothing
; 2. Create the full name of module in MODULE structure:
; concatenate module path, if given, and module name.
        mov     ecx, [path_len]
        mov     esi, [path_ptr]
        push    edi
        lea     edi, [eax+MODULE.path]
        rep movsb
        mov     ecx, ebx
        mov     esi, [esp]
        rep movsb
        pop     edi
        mov     esi, eax
; 3. Call the kernel to map the module.
        lea     ecx, [eax+MODULE.path]
        mov     eax, 68
        mov     ebx, 28
        call    FS_SYSCALL_PTR
        cmp     eax, -0x1000
        ja      .failed
; 4. On success, fill the rest of MODULE structure and return it.
        xchg    eax, esi
        call    init_module_struct
        ret
.failed:
; On failure, undo allocation at step 1 and return zero.
        stdcall free, esi
        xor     eax, eax
.nothing:
        ret
endp

; Worker procedure for loading a new module.
; Does not check whether the module has been already loaded;
; find_module_by_name should be called beforehand.
; in: edi -> filename
; out: eax -> MODULE or 0
; modules_mutex should be locked
proc load_module uses ebx esi ebp
; 1. Map the module.
; 1a. Prepare for try_map_module: calculate length of the name.
        mov     ebx, edi
@@:
        inc     ebx
        cmp     byte [ebx-1], 0
        jnz     @b
        sub     ebx, edi
; 1b. Check whether the given path is absolute.
; If so, proceed to 1c. If not, go to 1d.
        cmp     byte [edi], '/'
        jnz     .relative
; 1c. The given path is absolute. Use it as is. Don't try any other paths.
        stdcall try_map_module, 0, 0
        test    eax, eax
        jnz     .loaded_ok
        ccall   loader_say_error, msg_cannot_open, edi, 0
        jmp     .load_failed
.relative:
; 1d. The given path is relative.
; Try /rd/1/lib/, /kolibrios/lib/ and path to executable
; in this order.
        stdcall try_map_module, module_path1, module_path1.size
        test    eax, eax
        jnz     .loaded_ok
        stdcall try_map_module, module_path2, module_path2.size
        test    eax, eax
        jnz     .loaded_ok
; Note: we assume that the executable is always the first module in the list.
        mov     eax, [modules_list + MODULE.next]
        mov     ecx, [eax+MODULE.filename]
        add     eax, MODULE.path
        mov     esi, eax
        sub     ecx, eax
        stdcall try_map_module, eax, ecx
        test    eax, eax
        jnz     .loaded_ok
        mov     ebx, dword [esi+MODULE.filename-MODULE.path]
        movzx   eax, byte [ebx]
        mov     byte [ebx], 0
        push    eax
        ccall   loader_say_error, msg_cannot_open, edi, msg_paths_begin, esi, 0
        pop     eax
        mov     byte [ebx], al
.load_failed:
        xor     eax, eax
        ret
.loaded_ok:
; Module has been mapped.
; MODULE structure has been initialized, but not yet inserted in the common list.
; 2. Insert the MODULE structure in the end of the common list.
        mov     esi, eax
        mov     eax, [modules_list+MODULE.prev]
        mov     [eax+MODULE.next], esi
        mov     [esi+MODULE.prev], eax
        mov     [modules_list+MODULE.prev], esi
        mov     [esi+MODULE.next], modules_list
; 3. Call PE-specific code to initialize the mapped module.
        push    esi
        push    edi ; for messages in fixup_pe_relocations
        mov     esi, [esi+MODULE.base]
        call    fixup_pe_relocations
        pop     ecx
        pop     esi
        jc      .fail_unload
        call    resolve_pe_imports
        test    eax, eax
        jnz     .fail_unload
        mov     eax, esi
        ret
.fail_unload:
        call    dereference_module
        xor     eax, eax
        ret
endp

; Worker procedure for unloading a module.
; Drops one reference to the module; if it was the last one,
; unloads the module and all referenced modules recursively.
; in: esi -> MODULE struct
; modules_mutex should be locked
proc dereference_module
; 1. Decrement reference counter.
; If the decremented value is nonzero, exit.
        dec     [esi+MODULE.refcount]
        jnz     .nothing
; 2. Remove the module from the common list.
        mov     eax, [esi+MODULE.prev]
        mov     edx, [esi+MODULE.next]
        mov     [eax+MODULE.next], edx
        mov     [edx+MODULE.prev], eax
; 3. Recursively unload dependencies.
        cmp     [esi+MODULE.num_imports], 0
        jz      .import_deref_done
.import_deref_loop:
        mov     eax, [esi+MODULE.num_imports]
        push    esi
        mov     esi, [esi+MODULE.imports]
        mov     esi, [esi+(eax-1)*4]
        call    dereference_module
        pop     esi
        dec     [esi+MODULE.num_imports]
        jnz     .import_deref_loop
.import_deref_done:
        stdcall free, [esi+MODULE.imports] ; free(NULL) is ok
; 4. Unmap the module.
        push    ebx
        mov     eax, 68
        mov     ebx, 29
        mov     ecx, [esi+MODULE.base]
        call    FS_SYSCALL_PTR
        pop     ebx
; 5. Free the MODULE struct.
        stdcall free, esi
.nothing:
        ret
endp

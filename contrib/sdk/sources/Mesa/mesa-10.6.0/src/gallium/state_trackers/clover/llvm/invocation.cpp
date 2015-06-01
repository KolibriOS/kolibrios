//
// Copyright 2012 Francisco Jerez
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//

#include "core/compiler.hpp"

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/TextDiagnosticBuffer.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/CodeGen/CodeGenAction.h>
#include <clang/Basic/TargetInfo.h>
#include <llvm/Bitcode/BitstreamWriter.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Linker/Linker.h>
#include <llvm/IR/DiagnosticInfo.h>
#include <llvm/IR/DiagnosticPrinter.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/IRReader/IRReader.h>
#if HAVE_LLVM >= 0x0307
#include <llvm/IR/LegacyPassManager.h>
#else
#include <llvm/PassManager.h>
#endif
#include <llvm/Support/CodeGen.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/FormattedStream.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <llvm/Transforms/Utils/Cloning.h>


#include <llvm/IR/DataLayout.h>
#if HAVE_LLVM >= 0x0307
#include <llvm/Analysis/TargetLibraryInfo.h>
#else
#include <llvm/Target/TargetLibraryInfo.h>
#endif
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>

#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>
#include <llvm-c/Core.h>

#include "pipe/p_state.h"
#include "util/u_memory.h"
#include "util/u_math.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdio>
#include <sstream>
#include <libelf.h>
#include <gelf.h>

using namespace clover;

namespace {
#if 0
   void
   build_binary(const std::string &source, const std::string &target,
                const std::string &name) {
      clang::CompilerInstance c;
      clang::EmitObjAction act(&llvm::getGlobalContext());
      std::string log;
      llvm::raw_string_ostream s_log(log);

      LLVMInitializeTGSITarget();
      LLVMInitializeTGSITargetInfo();
      LLVMInitializeTGSITargetMC();
      LLVMInitializeTGSIAsmPrinter();

      c.getFrontendOpts().Inputs.push_back(
         std::make_pair(clang::IK_OpenCL, name));
      c.getHeaderSearchOpts().UseBuiltinIncludes = false;
      c.getHeaderSearchOpts().UseStandardIncludes = false;
      c.getLangOpts().NoBuiltin = true;
      c.getTargetOpts().Triple = target;
      c.getInvocation().setLangDefaults(clang::IK_OpenCL);
      c.createDiagnostics(0, NULL, new clang::TextDiagnosticPrinter(
                             s_log, c.getDiagnosticOpts()));

      c.getPreprocessorOpts().addRemappedFile(
         name, llvm::MemoryBuffer::getMemBuffer(source));

      if (!c.ExecuteAction(act))
         throw build_error(log);
   }

   module
   load_binary(const char *name) {
      std::ifstream fs((name));
      std::vector<unsigned char> str((std::istreambuf_iterator<char>(fs)),
                                     (std::istreambuf_iterator<char>()));
      compat::istream cs(str);
      return module::deserialize(cs);
   }
#endif
   void debug_log(const std::string &msg, const std::string &suffix) {
      const char *dbg_file = debug_get_option("CLOVER_DEBUG_FILE", "stderr");
      if (!strcmp("stderr", dbg_file)) {
         std::cerr << msg;
       } else {
        std::ofstream file(dbg_file + suffix, std::ios::app);
        file << msg;
       }
   }

   llvm::Module *
   compile_llvm(llvm::LLVMContext &llvm_ctx, const std::string &source,
                const header_map &headers,
                const std::string &name, const std::string &triple,
                const std::string &processor, const std::string &opts,
                clang::LangAS::Map& address_spaces, unsigned &optimization_level,
                std::string &r_log) {

      clang::CompilerInstance c;
      clang::EmitLLVMOnlyAction act(&llvm_ctx);
      std::string log;
      llvm::raw_string_ostream s_log(log);
      std::string libclc_path = LIBCLC_LIBEXECDIR + processor + "-"
                                                  + triple + ".bc";

      // Parse the compiler options:
      std::vector<std::string> opts_array;
      std::istringstream ss(opts);

      while (!ss.eof()) {
         std::string opt;
         getline(ss, opt, ' ');
         opts_array.push_back(opt);
      }

      opts_array.push_back(name);

      std::vector<const char *> opts_carray;
      for (unsigned i = 0; i < opts_array.size(); i++) {
         opts_carray.push_back(opts_array.at(i).c_str());
      }

      llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> DiagID;
      llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> DiagOpts;
      clang::TextDiagnosticBuffer *DiagsBuffer;

      DiagID = new clang::DiagnosticIDs();
      DiagOpts = new clang::DiagnosticOptions();
      DiagsBuffer = new clang::TextDiagnosticBuffer();

      clang::DiagnosticsEngine Diags(DiagID, &*DiagOpts, DiagsBuffer);
      bool Success;

      Success = clang::CompilerInvocation::CreateFromArgs(c.getInvocation(),
                                        opts_carray.data(),
                                        opts_carray.data() + opts_carray.size(),
                                        Diags);
      if (!Success) {
         throw error(CL_INVALID_COMPILER_OPTIONS);
      }
      c.getFrontendOpts().ProgramAction = clang::frontend::EmitLLVMOnly;
      c.getHeaderSearchOpts().UseBuiltinIncludes = true;
      c.getHeaderSearchOpts().UseStandardSystemIncludes = true;
      c.getHeaderSearchOpts().ResourceDir = CLANG_RESOURCE_DIR;

      // Add libclc generic search path
      c.getHeaderSearchOpts().AddPath(LIBCLC_INCLUDEDIR,
                                      clang::frontend::Angled,
                                      false, false
                                      );

      // Add libclc include
      c.getPreprocessorOpts().Includes.push_back("clc/clc.h");

      // clc.h requires that this macro be defined:
      c.getPreprocessorOpts().addMacroDef("cl_clang_storage_class_specifiers");

      c.getLangOpts().NoBuiltin = true;
      c.getTargetOpts().Triple = triple;
      c.getTargetOpts().CPU = processor;

      // This is a workaround for a Clang bug which causes the number
      // of warnings and errors to be printed to stderr.
      // http://www.llvm.org/bugs/show_bug.cgi?id=19735
      c.getDiagnosticOpts().ShowCarets = false;
      c.getInvocation().setLangDefaults(c.getLangOpts(), clang::IK_OpenCL,
                                        clang::LangStandard::lang_opencl11);
      c.createDiagnostics(
                          new clang::TextDiagnosticPrinter(
                                 s_log,
                                 &c.getDiagnosticOpts()));

#if HAVE_LLVM >= 0x0306
      c.getPreprocessorOpts().addRemappedFile(name,
                                              llvm::MemoryBuffer::getMemBuffer(source).release());
#else
      c.getPreprocessorOpts().addRemappedFile(name,
                                      llvm::MemoryBuffer::getMemBuffer(source));
#endif

      if (headers.size()) {
         const std::string tmp_header_path = "/tmp/clover/";

         c.getHeaderSearchOpts().AddPath(tmp_header_path,
                                         clang::frontend::Angled,
                                         false, false
                                         );

         for (header_map::const_iterator it = headers.begin();
              it != headers.end(); ++it) {
            const std::string path = tmp_header_path + std::string(it->first);
            c.getPreprocessorOpts().addRemappedFile(path,
#if HAVE_LLVM >= 0x0306
                    llvm::MemoryBuffer::getMemBuffer(it->second.c_str()).release());
#else
                    llvm::MemoryBuffer::getMemBuffer(it->second.c_str()));
#endif
         }
      }

      // Setting this attribute tells clang to link this file before
      // performing any optimizations.  This is required so that
      // we can replace calls to the OpenCL C barrier() builtin
      // with calls to target intrinsics that have the noduplicate
      // attribute.  This attribute will prevent Clang from creating
      // illegal uses of barrier() (e.g. Moving barrier() inside a conditional
      // that is no executed by all threads) during its optimizaton passes.
      c.getCodeGenOpts().LinkBitcodeFile = libclc_path;

      optimization_level = c.getCodeGenOpts().OptimizationLevel;

      // Compile the code
      bool ExecSuccess = c.ExecuteAction(act);
      r_log = log;

      if (!ExecSuccess)
         throw build_error();

      // Get address spaces map to be able to find kernel argument address space
      memcpy(address_spaces, c.getTarget().getAddressSpaceMap(),
                                                        sizeof(address_spaces));

#if HAVE_LLVM >= 0x0306
      return act.takeModule().release();
#else
      return act.takeModule();
#endif
   }

   void
   find_kernels(llvm::Module *mod, std::vector<llvm::Function *> &kernels) {
      const llvm::NamedMDNode *kernel_node =
                                 mod->getNamedMetadata("opencl.kernels");
      // This means there are no kernels in the program.  The spec does not
      // require that we return an error here, but there will be an error if
      // the user tries to pass this program to a clCreateKernel() call.
      if (!kernel_node) {
         return;
      }

      for (unsigned i = 0; i < kernel_node->getNumOperands(); ++i) {
#if HAVE_LLVM >= 0x0306
         kernels.push_back(llvm::mdconst::dyn_extract<llvm::Function>(
#else
         kernels.push_back(llvm::dyn_cast<llvm::Function>(
#endif
                                    kernel_node->getOperand(i)->getOperand(0)));
      }
   }

   void
   optimize(llvm::Module *mod, unsigned optimization_level,
            const std::vector<llvm::Function *> &kernels) {

#if HAVE_LLVM >= 0x0307
      llvm::legacy::PassManager PM;
#else
      llvm::PassManager PM;
#endif

      // Add a function internalizer pass.
      //
      // By default, the function internalizer pass will look for a function
      // called "main" and then mark all other functions as internal.  Marking
      // functions as internal enables the optimizer to perform optimizations
      // like function inlining and global dead-code elimination.
      //
      // When there is no "main" function in a module, the internalize pass will
      // treat the module like a library, and it won't internalize any functions.
      // Since there is no "main" function in our kernels, we need to tell
      // the internalizer pass that this module is not a library by passing a
      // list of kernel functions to the internalizer.  The internalizer will
      // treat the functions in the list as "main" functions and internalize
      // all of the other functions.
      std::vector<const char*> export_list;
      for (std::vector<llvm::Function *>::const_iterator I = kernels.begin(),
                                                         E = kernels.end();
                                                         I != E; ++I) {
         llvm::Function *kernel = *I;
         export_list.push_back(kernel->getName().data());
      }
#if HAVE_LLVM < 0x0306
      PM.add(new llvm::DataLayoutPass(mod));
#elif HAVE_LLVM < 0x0307
      PM.add(new llvm::DataLayoutPass());
#endif
      PM.add(llvm::createInternalizePass(export_list));

      llvm::PassManagerBuilder PMB;
      PMB.OptLevel = optimization_level;
#if HAVE_LLVM < 0x0307
      PMB.LibraryInfo = new llvm::TargetLibraryInfo(
#else
      PMB.LibraryInfo = new llvm::TargetLibraryInfoImpl(
#endif
            llvm::Triple(mod->getTargetTriple()));
      PMB.populateModulePassManager(PM);
      PM.run(*mod);
   }

   std::vector<module::argument>
   get_kernel_args(const llvm::Module *mod, const std::string &kernel_name,
                   const clang::LangAS::Map &address_spaces) {

      std::vector<module::argument> args;
      llvm::Function *kernel_func = mod->getFunction(kernel_name);

      llvm::DataLayout TD(mod);

      for (llvm::Function::const_arg_iterator I = kernel_func->arg_begin(),
                                      E = kernel_func->arg_end(); I != E; ++I) {
         const llvm::Argument &arg = *I;

         llvm::Type *arg_type = arg.getType();
         const unsigned arg_store_size = TD.getTypeStoreSize(arg_type);

         // OpenCL 1.2 specification, Ch. 6.1.5: "A built-in data
         // type that is not a power of two bytes in size must be
         // aligned to the next larger power of two".  We need this
         // alignment for three element vectors, which have
         // non-power-of-2 store size.
         const unsigned arg_api_size = util_next_power_of_two(arg_store_size);

         llvm::Type *target_type = arg_type->isIntegerTy() ?
               TD.getSmallestLegalIntType(mod->getContext(), arg_store_size * 8)
               : arg_type;
         unsigned target_size = TD.getTypeStoreSize(target_type);
         unsigned target_align = TD.getABITypeAlignment(target_type);

         if (llvm::isa<llvm::PointerType>(arg_type) && arg.hasByValAttr()) {
            arg_type =
                  llvm::dyn_cast<llvm::PointerType>(arg_type)->getElementType();
         }

         if (arg_type->isPointerTy()) {
            unsigned address_space = llvm::cast<llvm::PointerType>(arg_type)->getAddressSpace();
            if (address_space == address_spaces[clang::LangAS::opencl_local
                                                     - clang::LangAS::Offset]) {
               args.push_back(module::argument(module::argument::local,
                                               arg_api_size, target_size,
                                               target_align,
                                               module::argument::zero_ext));
            } else {
               // XXX: Correctly handle constant address space.  There is no
               // way for r600g to pass a handle for constant buffers back
               // to clover like it can for global buffers, so
               // creating constant arguments will break r600g.  For now,
               // continue treating constant buffers as global buffers
               // until we can come up with a way to create handles for
               // constant buffers.
               args.push_back(module::argument(module::argument::global,
                                               arg_api_size, target_size,
                                               target_align,
                                               module::argument::zero_ext));
           }

         } else {
            llvm::AttributeSet attrs = kernel_func->getAttributes();
            enum module::argument::ext_type ext_type =
                  (attrs.hasAttribute(arg.getArgNo() + 1,
                                     llvm::Attribute::SExt) ?
                   module::argument::sign_ext :
                   module::argument::zero_ext);

            args.push_back(
               module::argument(module::argument::scalar, arg_api_size,
                                target_size, target_align, ext_type));
         }
      }

      // Append implicit arguments.  XXX - The types, ordering and
      // vector size of the implicit arguments should depend on the
      // target according to the selected calling convention.
      llvm::Type *size_type =
         TD.getSmallestLegalIntType(mod->getContext(), sizeof(cl_uint) * 8);

      args.push_back(
         module::argument(module::argument::scalar, sizeof(cl_uint),
                          TD.getTypeStoreSize(size_type),
                          TD.getABITypeAlignment(size_type),
                          module::argument::zero_ext,
                          module::argument::grid_dimension));

      args.push_back(
         module::argument(module::argument::scalar, sizeof(cl_uint),
                          TD.getTypeStoreSize(size_type),
                          TD.getABITypeAlignment(size_type),
                          module::argument::zero_ext,
                          module::argument::grid_offset));

      return args;
   }

   module
   build_module_llvm(llvm::Module *mod,
                     const std::vector<llvm::Function *> &kernels,
                     clang::LangAS::Map& address_spaces) {

      module m;
      struct pipe_llvm_program_header header;

      llvm::SmallVector<char, 1024> llvm_bitcode;
      llvm::raw_svector_ostream bitcode_ostream(llvm_bitcode);
      llvm::BitstreamWriter writer(llvm_bitcode);
      llvm::WriteBitcodeToFile(mod, bitcode_ostream);
      bitcode_ostream.flush();

      for (unsigned i = 0; i < kernels.size(); ++i) {
         std::string kernel_name = kernels[i]->getName();
         std::vector<module::argument> args =
               get_kernel_args(mod, kernel_name, address_spaces);

         m.syms.push_back(module::symbol(kernel_name, 0, i, args ));
      }

      header.num_bytes = llvm_bitcode.size();
      std::vector<char> data;
      data.insert(data.end(), (char*)(&header),
                              (char*)(&header) + sizeof(header));
      data.insert(data.end(), llvm_bitcode.begin(),
                                  llvm_bitcode.end());
      m.secs.push_back(module::section(0, module::section::text,
                                       header.num_bytes, data));

      return m;
   }

   void
   emit_code(LLVMTargetMachineRef tm, LLVMModuleRef mod,
             LLVMCodeGenFileType file_type,
             LLVMMemoryBufferRef *out_buffer,
             std::string &r_log) {
      LLVMBool err;
      char *err_message = NULL;

      err = LLVMTargetMachineEmitToMemoryBuffer(tm, mod, file_type,
                                                &err_message, out_buffer);

      if (err) {
         r_log = std::string(err_message);
      }

      LLVMDisposeMessage(err_message);

      if (err) {
         throw build_error();
      }
   }

   std::vector<char>
   compile_native(const llvm::Module *mod, const std::string &triple,
                  const std::string &processor, unsigned dump_asm,
                  std::string &r_log) {

      std::string log;
      LLVMTargetRef target;
      char *error_message;
      LLVMMemoryBufferRef out_buffer;
      unsigned buffer_size;
      const char *buffer_data;
      LLVMModuleRef mod_ref = wrap(mod);

      if (LLVMGetTargetFromTriple(triple.c_str(), &target, &error_message)) {
         r_log = std::string(error_message);
         LLVMDisposeMessage(error_message);
         throw build_error();
      }

      LLVMTargetMachineRef tm = LLVMCreateTargetMachine(
            target, triple.c_str(), processor.c_str(), "",
            LLVMCodeGenLevelDefault, LLVMRelocDefault, LLVMCodeModelDefault);

      if (!tm) {
         r_log = "Could not create TargetMachine: " + triple;
         throw build_error();
      }

      if (dump_asm) {
         LLVMSetTargetMachineAsmVerbosity(tm, true);
         LLVMModuleRef debug_mod = wrap(llvm::CloneModule(mod));
         emit_code(tm, debug_mod, LLVMAssemblyFile, &out_buffer, r_log);
         buffer_size = LLVMGetBufferSize(out_buffer);
         buffer_data = LLVMGetBufferStart(out_buffer);
         debug_log(std::string(buffer_data, buffer_size), ".asm");

         LLVMSetTargetMachineAsmVerbosity(tm, false);
         LLVMDisposeMemoryBuffer(out_buffer);
         LLVMDisposeModule(debug_mod);
      }

      emit_code(tm, mod_ref, LLVMObjectFile, &out_buffer, r_log);

      buffer_size = LLVMGetBufferSize(out_buffer);
      buffer_data = LLVMGetBufferStart(out_buffer);

      std::vector<char> code(buffer_data, buffer_data + buffer_size);

      LLVMDisposeMemoryBuffer(out_buffer);
      LLVMDisposeTargetMachine(tm);

      return code;
   }

   std::map<std::string, unsigned>
   get_kernel_offsets(std::vector<char> &code,
                      const std::vector<llvm::Function *> &kernels,
                      std::string &r_log) {

      // One of the libelf implementations
      // (http://www.mr511.de/software/english.htm) requires calling
      // elf_version() before elf_memory().
      //
      elf_version(EV_CURRENT);

      Elf *elf = elf_memory(&code[0], code.size());
      size_t section_str_index;
      elf_getshdrstrndx(elf, &section_str_index);
      Elf_Scn *section = NULL;
      Elf_Scn *symtab = NULL;
      GElf_Shdr symtab_header;

      // Find the symbol table
      try {
         while ((section = elf_nextscn(elf, section))) {
            const char *name;
            if (gelf_getshdr(section, &symtab_header) != &symtab_header) {
               r_log = "Failed to read ELF section header.";
               throw build_error();
            }
            name = elf_strptr(elf, section_str_index, symtab_header.sh_name);
           if (!strcmp(name, ".symtab")) {
               symtab = section;
               break;
           }
         }
         if (!symtab) {
            r_log = "Unable to find symbol table.";
            throw build_error();
         }
      } catch (build_error &e) {
         elf_end(elf);
         throw e;
      }


      // Extract symbol information from the table
      Elf_Data *symtab_data = NULL;
      GElf_Sym *symbol;
      GElf_Sym s;

      std::map<std::string, unsigned> kernel_offsets;
      symtab_data = elf_getdata(symtab, symtab_data);

      // Determine the offsets for each kernel
      for (int i = 0; (symbol = gelf_getsym(symtab_data, i, &s)); i++) {
         char *name = elf_strptr(elf, symtab_header.sh_link, symbol->st_name);
         for (std::vector<llvm::Function*>::const_iterator it = kernels.begin(),
              e = kernels.end(); it != e; ++it) {
            llvm::Function *f = *it;
            if (f->getName() == std::string(name))
               kernel_offsets[f->getName()] = symbol->st_value;
         }
      }
      elf_end(elf);
      return kernel_offsets;
   }

   module
   build_module_native(std::vector<char> &code,
                       const llvm::Module *mod,
                       const std::vector<llvm::Function *> &kernels,
                       const clang::LangAS::Map &address_spaces,
                       std::string &r_log) {

      std::map<std::string, unsigned> kernel_offsets =
            get_kernel_offsets(code, kernels, r_log);

      // Begin building the clover module
      module m;
      struct pipe_llvm_program_header header;

      // Store the generated ELF binary in the module's text section.
      header.num_bytes = code.size();
      std::vector<char> data;
      data.insert(data.end(), (char*)(&header),
                              (char*)(&header) + sizeof(header));
      data.insert(data.end(), code.begin(), code.end());
      m.secs.push_back(module::section(0, module::section::text,
                                       header.num_bytes, data));

      for (std::map<std::string, unsigned>::iterator i = kernel_offsets.begin(),
           e = kernel_offsets.end(); i != e; ++i) {
         std::vector<module::argument> args =
               get_kernel_args(mod, i->first, address_spaces);
         m.syms.push_back(module::symbol(i->first, 0, i->second, args ));
      }

      return m;
   }

   void
   diagnostic_handler(const llvm::DiagnosticInfo &di, void *data) {
      if (di.getSeverity() == llvm::DS_Error) {
         std::string message = *(std::string*)data;
         llvm::raw_string_ostream stream(message);
         llvm::DiagnosticPrinterRawOStream dp(stream);
         di.print(dp);
         stream.flush();
         *(std::string*)data = message;

         throw build_error();
      }
   }

   void
   init_targets() {
      static bool targets_initialized = false;
      if (!targets_initialized) {
         LLVMInitializeAllTargets();
         LLVMInitializeAllTargetInfos();
         LLVMInitializeAllTargetMCs();
         LLVMInitializeAllAsmPrinters();
         targets_initialized = true;
      }
   }

#define DBG_CLC  (1 << 0)
#define DBG_LLVM (1 << 1)
#define DBG_ASM  (1 << 2)

   unsigned
   get_debug_flags() {
      static const struct debug_named_value debug_options[] = {
         {"clc", DBG_CLC, "Dump the OpenCL C code for all kernels."},
         {"llvm", DBG_LLVM, "Dump the generated LLVM IR for all kernels."},
         {"asm", DBG_ASM, "Dump kernel assembly code for targets specifying "
          "PIPE_SHADER_IR_NATIVE"},
         DEBUG_NAMED_VALUE_END // must be last
      };
      static const unsigned debug_flags =
         debug_get_flags_option("CLOVER_DEBUG", debug_options, 0);

      return debug_flags;
   }

} // End anonymous namespace

module
clover::compile_program_llvm(const std::string &source,
                             const header_map &headers,
                             enum pipe_shader_ir ir,
                             const std::string &target,
                             const std::string &opts,
                             std::string &r_log) {

   init_targets();

   std::vector<llvm::Function *> kernels;
   size_t processor_str_len = std::string(target).find_first_of("-");
   std::string processor(target, 0, processor_str_len);
   std::string triple(target, processor_str_len + 1,
                      target.size() - processor_str_len - 1);
   clang::LangAS::Map address_spaces;
   llvm::LLVMContext llvm_ctx;
   unsigned optimization_level;

   llvm_ctx.setDiagnosticHandler(diagnostic_handler, &r_log);

   if (get_debug_flags() & DBG_CLC)
      debug_log(source, ".cl");

   // The input file name must have the .cl extension in order for the
   // CompilerInvocation class to recognize it as an OpenCL source file.
   llvm::Module *mod = compile_llvm(llvm_ctx, source, headers, "input.cl",
                                    triple, processor, opts, address_spaces,
                                    optimization_level, r_log);

   find_kernels(mod, kernels);

   optimize(mod, optimization_level, kernels);

   if (get_debug_flags() & DBG_LLVM) {
      std::string log;
      llvm::raw_string_ostream s_log(log);
      mod->print(s_log, NULL);
      s_log.flush();
      debug_log(log, ".ll");
    }

   module m;
   // Build the clover::module
   switch (ir) {
      case PIPE_SHADER_IR_TGSI:
         //XXX: Handle TGSI
         assert(0);
         m = module();
         break;
      case PIPE_SHADER_IR_LLVM:
         m = build_module_llvm(mod, kernels, address_spaces);
         break;
      case PIPE_SHADER_IR_NATIVE: {
         std::vector<char> code = compile_native(mod, triple, processor,
                                                 get_debug_flags() & DBG_ASM,
                                                 r_log);
         m = build_module_native(code, mod, kernels, address_spaces, r_log);
         break;
      }
   }
#if HAVE_LLVM >= 0x0306
   // LLVM 3.6 and newer, the user takes ownership of the module.
   delete mod;
#endif

   return m;
}

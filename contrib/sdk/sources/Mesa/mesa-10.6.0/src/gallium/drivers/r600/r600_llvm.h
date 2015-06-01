
#ifndef R600_LLVM_H
#define R600_LLVM_H

#if defined R600_USE_LLVM || defined HAVE_OPENCL

#include "radeon/radeon_llvm.h"
#include <llvm-c/Core.h>

struct r600_bytecode;
struct r600_shader_ctx;
struct radeon_llvm_context;
struct radeon_shader_binary;
enum radeon_family;

LLVMModuleRef r600_tgsi_llvm(
	struct radeon_llvm_context * ctx,
	const struct tgsi_token * tokens);

unsigned r600_llvm_compile(
	LLVMModuleRef mod,
	enum radeon_family family,
	struct r600_bytecode *bc,
	boolean *use_kill,
	unsigned dump);

unsigned r600_create_shader(struct r600_bytecode *bc,
		const struct radeon_shader_binary *binary,
		boolean *use_kill);

void r600_shader_binary_read_config(const struct radeon_shader_binary *binary,
		struct r600_bytecode *bc,
		uint64_t symbol_offset,
		boolean *use_kill);

#endif /* defined R600_USE_LLVM || defined HAVE_OPENCL */

#endif /* R600_LLVM_H */

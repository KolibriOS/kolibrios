/*
 * Copyright © 2014 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "brw_nir.h"
#include "glsl/glsl_parser_extras.h"
#include "glsl/nir/glsl_to_nir.h"
#include "program/prog_to_nir.h"

static void
nir_optimize(nir_shader *nir)
{
   bool progress;
   do {
      progress = false;
      nir_lower_vars_to_ssa(nir);
      nir_validate_shader(nir);
      nir_lower_alu_to_scalar(nir);
      nir_validate_shader(nir);
      progress |= nir_copy_prop(nir);
      nir_validate_shader(nir);
      nir_lower_phis_to_scalar(nir);
      nir_validate_shader(nir);
      progress |= nir_copy_prop(nir);
      nir_validate_shader(nir);
      progress |= nir_opt_dce(nir);
      nir_validate_shader(nir);
      progress |= nir_opt_cse(nir);
      nir_validate_shader(nir);
      progress |= nir_opt_peephole_select(nir);
      nir_validate_shader(nir);
      progress |= nir_opt_algebraic(nir);
      nir_validate_shader(nir);
      progress |= nir_opt_constant_folding(nir);
      nir_validate_shader(nir);
      progress |= nir_opt_remove_phis(nir);
      nir_validate_shader(nir);
   } while (progress);
}

static bool
count_nir_instrs_in_block(nir_block *block, void *state)
{
   int *count = (int *) state;
   nir_foreach_instr(block, instr) {
      *count = *count + 1;
   }
   return true;
}

static int
count_nir_instrs(nir_shader *nir)
{
   int count = 0;
   nir_foreach_overload(nir, overload) {
      if (!overload->impl)
         continue;
      nir_foreach_block(overload->impl, count_nir_instrs_in_block, &count);
   }
   return count;
}

nir_shader *
brw_create_nir(struct brw_context *brw,
               const struct gl_shader_program *shader_prog,
               const struct gl_program *prog,
               gl_shader_stage stage)
{
   struct gl_context *ctx = &brw->ctx;
   const nir_shader_compiler_options *options =
      ctx->Const.ShaderCompilerOptions[stage].NirOptions;
   struct gl_shader *shader = shader_prog ? shader_prog->_LinkedShaders[stage] : NULL;
   bool debug_enabled = INTEL_DEBUG & intel_debug_flag_for_shader_stage(stage);
   nir_shader *nir;

   /* First, lower the GLSL IR or Mesa IR to NIR */
   if (shader_prog) {
      nir = glsl_to_nir(shader, options);
   } else {
      nir = prog_to_nir(prog, options);
      nir_convert_to_ssa(nir); /* turn registers into SSA */
   }
   nir_validate_shader(nir);

   nir_lower_global_vars_to_local(nir);
   nir_validate_shader(nir);

   nir_lower_tex_projector(nir);
   nir_validate_shader(nir);

   nir_normalize_cubemap_coords(nir);
   nir_validate_shader(nir);

   nir_split_var_copies(nir);
   nir_validate_shader(nir);

   nir_optimize(nir);

   /* Lower a bunch of stuff */
   nir_lower_var_copies(nir);
   nir_validate_shader(nir);

   /* Get rid of split copies */
   nir_optimize(nir);

   if (shader_prog) {
      nir_assign_var_locations_scalar_direct_first(nir, &nir->uniforms,
                                                   &nir->num_direct_uniforms,
                                                   &nir->num_uniforms);
   } else {
      /* ARB programs generally create a giant array of "uniform" data, and allow
       * indirect addressing without any boundaries.  In the absence of bounds
       * analysis, it's all or nothing.  num_direct_uniforms is only useful when
       * we have some direct and some indirect access; it doesn't matter here.
       */
      nir->num_direct_uniforms = 0;
   }
   nir_assign_var_locations_scalar(&nir->inputs, &nir->num_inputs);
   nir_assign_var_locations_scalar(&nir->outputs, &nir->num_outputs);

   nir_lower_io(nir);
   nir_validate_shader(nir);

   nir_remove_dead_variables(nir);
   nir_validate_shader(nir);

   if (shader_prog) {
      nir_lower_samplers(nir, shader_prog, stage);
      nir_validate_shader(nir);
   }

   nir_lower_system_values(nir);
   nir_validate_shader(nir);

   nir_lower_atomics(nir);
   nir_validate_shader(nir);

   nir_optimize(nir);

   if (brw->gen >= 6) {
      /* Try and fuse multiply-adds */
      nir_opt_peephole_ffma(nir);
      nir_validate_shader(nir);
   }

   nir_opt_algebraic_late(nir);
   nir_validate_shader(nir);

   nir_lower_locals_to_regs(nir);
   nir_validate_shader(nir);

   nir_lower_to_source_mods(nir);
   nir_validate_shader(nir);
   nir_copy_prop(nir);
   nir_validate_shader(nir);
   nir_opt_dce(nir);
   nir_validate_shader(nir);

   if (unlikely(debug_enabled)) {
      fprintf(stderr, "NIR (SSA form) for %s shader:\n",
              _mesa_shader_stage_to_string(stage));
      nir_print_shader(nir, stderr);
   }

   static GLuint msg_id = 0;
   _mesa_gl_debug(&brw->ctx, &msg_id,
                  MESA_DEBUG_SOURCE_SHADER_COMPILER,
                  MESA_DEBUG_TYPE_OTHER,
                  MESA_DEBUG_SEVERITY_NOTIFICATION,
                  "%s NIR shader: %d inst\n",
                  _mesa_shader_stage_to_abbrev(stage),
                  count_nir_instrs(nir));

   nir_convert_from_ssa(nir);
   nir_validate_shader(nir);

   /* This is the last pass we run before we start emitting stuff.  It
    * determines when we need to insert boolean resolves on Gen <= 5.  We
    * run it last because it stashes data in instr->pass_flags and we don't
    * want that to be squashed by other NIR passes.
    */
   if (brw->gen <= 5)
      brw_nir_analyze_boolean_resolves(nir);

   nir_sweep(nir);

   if (unlikely(debug_enabled)) {
      fprintf(stderr, "NIR (final form) for %s shader:\n",
              _mesa_shader_stage_to_string(stage));
      nir_print_shader(nir, stderr);
   }

   return nir;
}

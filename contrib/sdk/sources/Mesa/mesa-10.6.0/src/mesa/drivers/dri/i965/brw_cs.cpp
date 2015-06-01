/*
 * Copyright (c) 2014 - 2015 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */


#include "util/ralloc.h"
#include "brw_context.h"
#include "brw_cs.h"
#include "brw_fs.h"
#include "brw_eu.h"
#include "brw_wm.h"
#include "intel_mipmap_tree.h"
#include "brw_state.h"
#include "intel_batchbuffer.h"

extern "C"
bool
brw_cs_prog_data_compare(const void *in_a, const void *in_b)
{
   const struct brw_cs_prog_data *a =
      (const struct brw_cs_prog_data *)in_a;
   const struct brw_cs_prog_data *b =
      (const struct brw_cs_prog_data *)in_b;

   /* Compare the base structure. */
   if (!brw_stage_prog_data_compare(&a->base, &b->base))
      return false;

   /* Compare the rest of the structure. */
   const unsigned offset = sizeof(struct brw_stage_prog_data);
   if (memcmp(((char *) a) + offset, ((char *) b) + offset,
              sizeof(struct brw_cs_prog_data) - offset))
      return false;

   return true;
}


static const unsigned *
brw_cs_emit(struct brw_context *brw,
            void *mem_ctx,
            const struct brw_cs_prog_key *key,
            struct brw_cs_prog_data *prog_data,
            struct gl_compute_program *cp,
            struct gl_shader_program *prog,
            unsigned *final_assembly_size)
{
   bool start_busy = false;
   double start_time = 0;

   if (unlikely(brw->perf_debug)) {
      start_busy = (brw->batch.last_bo &&
                    drm_intel_bo_busy(brw->batch.last_bo));
      start_time = get_time();
   }

   struct brw_shader *shader =
      (struct brw_shader *) prog->_LinkedShaders[MESA_SHADER_COMPUTE];

   if (unlikely(INTEL_DEBUG & DEBUG_CS))
      brw_dump_ir("compute", prog, &shader->base, &cp->Base);

   prog_data->local_size[0] = cp->LocalSize[0];
   prog_data->local_size[1] = cp->LocalSize[1];
   prog_data->local_size[2] = cp->LocalSize[2];
   int local_workgroup_size =
      cp->LocalSize[0] * cp->LocalSize[1] * cp->LocalSize[2];

   cfg_t *cfg = NULL;
   const char *fail_msg = NULL;

   /* Now the main event: Visit the shader IR and generate our CS IR for it.
    */
   fs_visitor v8(brw, mem_ctx, MESA_SHADER_COMPUTE, key, &prog_data->base, prog,
                 &cp->Base, 8);
   if (!v8.run_cs()) {
      fail_msg = v8.fail_msg;
   } else if (local_workgroup_size <= 8 * brw->max_cs_threads) {
      cfg = v8.cfg;
      prog_data->simd_size = 8;
   }

   fs_visitor v16(brw, mem_ctx, MESA_SHADER_COMPUTE, key, &prog_data->base, prog,
                  &cp->Base, 16);
   if (likely(!(INTEL_DEBUG & DEBUG_NO16)) &&
       !fail_msg && !v8.simd16_unsupported &&
       local_workgroup_size <= 16 * brw->max_cs_threads) {
      /* Try a SIMD16 compile */
      v16.import_uniforms(&v8);
      if (!v16.run_cs()) {
         perf_debug("SIMD16 shader failed to compile: %s", v16.fail_msg);
         if (!cfg) {
            fail_msg =
               "Couldn't generate SIMD16 program and not "
               "enough threads for SIMD8";
         }
      } else {
         cfg = v16.cfg;
         prog_data->simd_size = 16;
      }
   }

   if (unlikely(cfg == NULL)) {
      assert(fail_msg);
      prog->LinkStatus = false;
      ralloc_strcat(&prog->InfoLog, fail_msg);
      _mesa_problem(NULL, "Failed to compile compute shader: %s\n",
                    fail_msg);
      return NULL;
   }

   fs_generator g(brw, mem_ctx, (void*) key, &prog_data->base, &cp->Base,
                  v8.promoted_constants, v8.runtime_check_aads_emit, "CS");
   if (INTEL_DEBUG & DEBUG_CS) {
      char *name = ralloc_asprintf(mem_ctx, "%s compute shader %d",
                                   prog->Label ? prog->Label : "unnamed",
                                   prog->Name);
      g.enable_debug(name);
   }

   g.generate_code(cfg, prog_data->simd_size);

   if (unlikely(brw->perf_debug) && shader) {
      if (shader->compiled_once) {
         _mesa_problem(&brw->ctx, "CS programs shouldn't need recompiles");
      }
      shader->compiled_once = true;

      if (start_busy && !drm_intel_bo_busy(brw->batch.last_bo)) {
         perf_debug("CS compile took %.03f ms and stalled the GPU\n",
                    (get_time() - start_time) * 1000);
      }
   }

   return g.get_assembly(final_assembly_size);
}

static bool
brw_codegen_cs_prog(struct brw_context *brw,
                    struct gl_shader_program *prog,
                    struct brw_compute_program *cp,
                    struct brw_cs_prog_key *key)
{
   struct gl_context *ctx = &brw->ctx;
   const GLuint *program;
   void *mem_ctx = ralloc_context(NULL);
   GLuint program_size;
   struct brw_cs_prog_data prog_data;

   struct gl_shader *cs = prog->_LinkedShaders[MESA_SHADER_COMPUTE];
   assert (cs);

   memset(&prog_data, 0, sizeof(prog_data));

   /* Allocate the references to the uniforms that will end up in the
    * prog_data associated with the compiled program, and which will be freed
    * by the state cache.
    */
   int param_count = cs->num_uniform_components;

   /* The backend also sometimes adds params for texture size. */
   param_count += 2 * ctx->Const.Program[MESA_SHADER_COMPUTE].MaxTextureImageUnits;
   prog_data.base.param =
      rzalloc_array(NULL, const gl_constant_value *, param_count);
   prog_data.base.pull_param =
      rzalloc_array(NULL, const gl_constant_value *, param_count);
   prog_data.base.nr_params = param_count;

   program = brw_cs_emit(brw, mem_ctx, key, &prog_data,
                         &cp->program, prog, &program_size);
   if (program == NULL) {
      ralloc_free(mem_ctx);
      return false;
   }

   if (prog_data.base.total_scratch) {
      brw_get_scratch_bo(brw, &brw->cs.base.scratch_bo,
                         prog_data.base.total_scratch * brw->max_cs_threads);
   }

   if (unlikely(INTEL_DEBUG & DEBUG_CS))
      fprintf(stderr, "\n");

   brw_upload_cache(&brw->cache, BRW_CACHE_CS_PROG,
                    key, sizeof(*key),
                    program, program_size,
                    &prog_data, sizeof(prog_data),
                    &brw->cs.base.prog_offset, &brw->cs.prog_data);
   ralloc_free(mem_ctx);

   return true;
}


static void
brw_cs_populate_key(struct brw_context *brw, struct brw_cs_prog_key *key)
{
   /* BRW_NEW_COMPUTE_PROGRAM */
   const struct brw_compute_program *cp =
      (struct brw_compute_program *) brw->compute_program;

   memset(key, 0, sizeof(*key));

   /* The unique compute program ID */
   key->program_string_id = cp->id;
}


extern "C"
void
brw_upload_cs_prog(struct brw_context *brw)
{
   struct gl_context *ctx = &brw->ctx;
   struct brw_cs_prog_key key;
   struct brw_compute_program *cp = (struct brw_compute_program *)
      brw->compute_program;

   if (!cp)
      return;

   if (!brw_state_dirty(brw, 0, BRW_NEW_COMPUTE_PROGRAM))
      return;

   brw_cs_populate_key(brw, &key);

   if (!brw_search_cache(&brw->cache, BRW_CACHE_CS_PROG,
                         &key, sizeof(key),
                         &brw->cs.base.prog_offset, &brw->cs.prog_data)) {
      bool success =
         brw_codegen_cs_prog(brw,
                             ctx->Shader.CurrentProgram[MESA_SHADER_COMPUTE],
                             cp, &key);
      (void) success;
      assert(success);
   }
   brw->cs.base.prog_data = &brw->cs.prog_data->base;
}


extern "C" bool
brw_cs_precompile(struct gl_context *ctx,
                  struct gl_shader_program *shader_prog,
                  struct gl_program *prog)
{
   struct brw_context *brw = brw_context(ctx);
   struct brw_cs_prog_key key;

   struct gl_compute_program *cp = (struct gl_compute_program *) prog;
   struct brw_compute_program *bcp = brw_compute_program(cp);

   memset(&key, 0, sizeof(key));
   key.program_string_id = bcp->id;

   brw_setup_tex_for_precompile(brw, &key.tex, prog);

   uint32_t old_prog_offset = brw->cs.base.prog_offset;
   struct brw_cs_prog_data *old_prog_data = brw->cs.prog_data;

   bool success = brw_codegen_cs_prog(brw, shader_prog, bcp, &key);

   brw->cs.base.prog_offset = old_prog_offset;
   brw->cs.prog_data = old_prog_data;

   return success;
}


static void
brw_upload_cs_state(struct brw_context *brw)
{
   if (!brw->cs.prog_data)
      return;

   uint32_t offset;
   uint32_t *desc = (uint32_t*) brw_state_batch(brw, AUB_TRACE_SURFACE_STATE,
                                                8 * 4, 64, &offset);
   struct brw_stage_state *stage_state = &brw->cs.base;
   struct brw_cs_prog_data *cs_prog_data = brw->cs.prog_data;
   struct brw_stage_prog_data *prog_data = &cs_prog_data->base;

   if (INTEL_DEBUG & DEBUG_SHADER_TIME) {
      brw->vtbl.emit_buffer_surface_state(
         brw, &stage_state->surf_offset[
                 prog_data->binding_table.shader_time_start],
         brw->shader_time.bo, 0, BRW_SURFACEFORMAT_RAW,
         brw->shader_time.bo->size, 1, true);
   }

   uint32_t *bind = (uint32_t*) brw_state_batch(brw, AUB_TRACE_BINDING_TABLE,
                                            prog_data->binding_table.size_bytes,
                                            32, &stage_state->bind_bo_offset);

   uint32_t dwords = brw->gen < 8 ? 8 : 9;
   BEGIN_BATCH(dwords);
   OUT_BATCH(MEDIA_VFE_STATE << 16 | (dwords - 2));

   if (prog_data->total_scratch) {
      if (brw->gen >= 8)
         OUT_RELOC64(stage_state->scratch_bo,
                     I915_GEM_DOMAIN_RENDER, I915_GEM_DOMAIN_RENDER,
                     ffs(prog_data->total_scratch) - 11);
      else
         OUT_RELOC(stage_state->scratch_bo,
                   I915_GEM_DOMAIN_RENDER, I915_GEM_DOMAIN_RENDER,
                   ffs(prog_data->total_scratch) - 11);
   } else {
      OUT_BATCH(0);
      if (brw->gen >= 8)
         OUT_BATCH(0);
   }

   const uint32_t vfe_num_urb_entries = brw->gen >= 8 ? 2 : 0;
   const uint32_t vfe_gpgpu_mode =
      brw->gen == 7 ? SET_FIELD(1, GEN7_MEDIA_VFE_STATE_GPGPU_MODE) : 0;
   OUT_BATCH(SET_FIELD(brw->max_cs_threads - 1, MEDIA_VFE_STATE_MAX_THREADS) |
             SET_FIELD(vfe_num_urb_entries, MEDIA_VFE_STATE_URB_ENTRIES) |
             SET_FIELD(1, MEDIA_VFE_STATE_RESET_GTW_TIMER) |
             SET_FIELD(1, MEDIA_VFE_STATE_BYPASS_GTW) |
             vfe_gpgpu_mode);

   OUT_BATCH(0);
   const uint32_t vfe_urb_allocation = brw->gen >= 8 ? 2 : 0;
   OUT_BATCH(SET_FIELD(vfe_urb_allocation, MEDIA_VFE_STATE_URB_ALLOC));
   OUT_BATCH(0);
   OUT_BATCH(0);
   OUT_BATCH(0);
   ADVANCE_BATCH();

   /* BRW_NEW_SURFACES and BRW_NEW_*_CONSTBUF */
   memcpy(bind, stage_state->surf_offset,
          prog_data->binding_table.size_bytes);

   memset(desc, 0, 8 * 4);

   int dw = 0;
   desc[dw++] = brw->cs.base.prog_offset;
   if (brw->gen >= 8)
      desc[dw++] = 0; /* Kernel Start Pointer High */
   desc[dw++] = 0;
   desc[dw++] = 0;
   desc[dw++] = stage_state->bind_bo_offset;

   BEGIN_BATCH(4);
   OUT_BATCH(MEDIA_INTERFACE_DESCRIPTOR_LOAD << 16 | (4 - 2));
   OUT_BATCH(0);
   OUT_BATCH(8 * 4);
   OUT_BATCH(offset);
   ADVANCE_BATCH();
}


extern "C"
const struct brw_tracked_state brw_cs_state = {
   /* explicit initialisers aren't valid C++, comment
    * them for documentation purposes */
   /* .dirty = */{
      /* .mesa = */ 0,
      /* .brw = */  BRW_NEW_CS_PROG_DATA,
   },
   /* .emit = */ brw_upload_cs_state
};

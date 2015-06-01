/*
 Copyright (C) Intel Corp.  2006.  All Rights Reserved.
 Intel funded Tungsten Graphics to
 develop this 3D driver.

 Permission is hereby granted, free of charge, to any person obtaining
 a copy of this software and associated documentation files (the
 "Software"), to deal in the Software without restriction, including
 without limitation the rights to use, copy, modify, merge, publish,
 distribute, sublicense, and/or sell copies of the Software, and to
 permit persons to whom the Software is furnished to do so, subject to
 the following conditions:

 The above copyright notice and this permission notice (including the
 next paragraph) shall be included in all copies or substantial
 portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE COPYRIGHT OWNER(S) AND/OR ITS SUPPLIERS BE
 LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 **********************************************************************/
 /*
  * Authors:
  *   Keith Whitwell <keithw@vmware.com>
  */

#include <pthread.h>
#include "main/imports.h"
#include "main/enums.h"
#include "main/shaderobj.h"
#include "program/prog_parameter.h"
#include "program/prog_print.h"
#include "program/program.h"
#include "program/programopt.h"
#include "tnl/tnl.h"
#include "util/ralloc.h"
#include "glsl/ir.h"

#include "brw_context.h"
#include "brw_shader.h"
#include "brw_nir.h"
#include "brw_wm.h"
#include "intel_batchbuffer.h"

static unsigned
get_new_program_id(struct intel_screen *screen)
{
   static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
   pthread_mutex_lock(&m);
   unsigned id = screen->program_id++;
   pthread_mutex_unlock(&m);
   return id;
}

static struct gl_program *brwNewProgram( struct gl_context *ctx,
				      GLenum target,
				      GLuint id )
{
   struct brw_context *brw = brw_context(ctx);

   switch (target) {
   case GL_VERTEX_PROGRAM_ARB: {
      struct brw_vertex_program *prog = CALLOC_STRUCT(brw_vertex_program);
      if (prog) {
	 prog->id = get_new_program_id(brw->intelScreen);

	 return _mesa_init_vertex_program( ctx, &prog->program,
					     target, id );
      }
      else
	 return NULL;
   }

   case GL_FRAGMENT_PROGRAM_ARB: {
      struct brw_fragment_program *prog = CALLOC_STRUCT(brw_fragment_program);
      if (prog) {
	 prog->id = get_new_program_id(brw->intelScreen);

	 return _mesa_init_fragment_program( ctx, &prog->program,
					     target, id );
      }
      else
	 return NULL;
   }

   case MESA_GEOMETRY_PROGRAM: {
      struct brw_geometry_program *prog = CALLOC_STRUCT(brw_geometry_program);
      if (prog) {
         prog->id = get_new_program_id(brw->intelScreen);

         return _mesa_init_geometry_program(ctx, &prog->program, target, id);
      } else {
         return NULL;
      }
   }

   case GL_COMPUTE_PROGRAM_NV: {
      struct brw_compute_program *prog = CALLOC_STRUCT(brw_compute_program);
      if (prog) {
         prog->id = get_new_program_id(brw->intelScreen);

         return _mesa_init_compute_program(ctx, &prog->program, target, id);
      } else {
         return NULL;
      }
   }

   default:
      unreachable("Unsupported target in brwNewProgram()");
   }
}

static void brwDeleteProgram( struct gl_context *ctx,
			      struct gl_program *prog )
{
   _mesa_delete_program( ctx, prog );
}


static GLboolean
brwProgramStringNotify(struct gl_context *ctx,
		       GLenum target,
		       struct gl_program *prog)
{
   struct brw_context *brw = brw_context(ctx);

   switch (target) {
   case GL_FRAGMENT_PROGRAM_ARB: {
      struct gl_fragment_program *fprog = (struct gl_fragment_program *) prog;
      struct brw_fragment_program *newFP = brw_fragment_program(fprog);
      const struct brw_fragment_program *curFP =
         brw_fragment_program_const(brw->fragment_program);

      if (newFP == curFP)
	 brw->ctx.NewDriverState |= BRW_NEW_FRAGMENT_PROGRAM;
      newFP->id = get_new_program_id(brw->intelScreen);

      brw_add_texrect_params(prog);

      if (ctx->Const.ShaderCompilerOptions[MESA_SHADER_FRAGMENT].NirOptions) {
         prog->nir = brw_create_nir(brw, NULL, prog, MESA_SHADER_FRAGMENT);
      }

      brw_fs_precompile(ctx, NULL, prog);
      break;
   }
   case GL_VERTEX_PROGRAM_ARB: {
      struct gl_vertex_program *vprog = (struct gl_vertex_program *) prog;
      struct brw_vertex_program *newVP = brw_vertex_program(vprog);
      const struct brw_vertex_program *curVP =
         brw_vertex_program_const(brw->vertex_program);

      if (newVP == curVP)
	 brw->ctx.NewDriverState |= BRW_NEW_VERTEX_PROGRAM;
      if (newVP->program.IsPositionInvariant) {
	 _mesa_insert_mvp_code(ctx, &newVP->program);
      }
      newVP->id = get_new_program_id(brw->intelScreen);

      /* Also tell tnl about it:
       */
      _tnl_program_string(ctx, target, prog);

      brw_add_texrect_params(prog);

      if (ctx->Const.ShaderCompilerOptions[MESA_SHADER_VERTEX].NirOptions) {
         prog->nir = brw_create_nir(brw, NULL, prog, MESA_SHADER_VERTEX);
      }

      brw_vs_precompile(ctx, NULL, prog);
      break;
   }
   default:
      /*
       * driver->ProgramStringNotify is only called for ARB programs, fixed
       * function vertex programs, and ir_to_mesa (which isn't used by the
       * i965 back-end).  Therefore, even after geometry shaders are added,
       * this function should only ever be called with a target of
       * GL_VERTEX_PROGRAM_ARB or GL_FRAGMENT_PROGRAM_ARB.
       */
      unreachable("Unexpected target in brwProgramStringNotify");
   }

   return true;
}

static void
brw_memory_barrier(struct gl_context *ctx, GLbitfield barriers)
{
   struct brw_context *brw = brw_context(ctx);
   unsigned bits = (PIPE_CONTROL_DATA_CACHE_INVALIDATE |
                    PIPE_CONTROL_NO_WRITE |
                    PIPE_CONTROL_CS_STALL);
   assert(brw->gen >= 7 && brw->gen <= 8);

   if (barriers & (GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT |
                   GL_ELEMENT_ARRAY_BARRIER_BIT |
                   GL_COMMAND_BARRIER_BIT))
      bits |= PIPE_CONTROL_VF_CACHE_INVALIDATE;

   if (barriers & GL_UNIFORM_BARRIER_BIT)
      bits |= (PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE |
               PIPE_CONTROL_CONST_CACHE_INVALIDATE);

   if (barriers & GL_TEXTURE_FETCH_BARRIER_BIT)
      bits |= PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE;

   if (barriers & GL_TEXTURE_UPDATE_BARRIER_BIT)
      bits |= PIPE_CONTROL_RENDER_TARGET_FLUSH;

   if (barriers & GL_FRAMEBUFFER_BARRIER_BIT)
      bits |= (PIPE_CONTROL_DEPTH_CACHE_FLUSH |
               PIPE_CONTROL_RENDER_TARGET_FLUSH);

   /* Typed surface messages are handled by the render cache on IVB, so we
    * need to flush it too.
    */
   if (brw->gen == 7 && !brw->is_haswell)
      bits |= PIPE_CONTROL_RENDER_TARGET_FLUSH;

   brw_emit_pipe_control_flush(brw, bits);
}

void
brw_add_texrect_params(struct gl_program *prog)
{
   for (int texunit = 0; texunit < BRW_MAX_TEX_UNIT; texunit++) {
      if (!(prog->TexturesUsed[texunit] & (1 << TEXTURE_RECT_INDEX)))
         continue;

      int tokens[STATE_LENGTH] = {
         STATE_INTERNAL,
         STATE_TEXRECT_SCALE,
         texunit,
         0,
         0
      };

      _mesa_add_state_reference(prog->Parameters, (gl_state_index *)tokens);
   }
}

/* Per-thread scratch space is a power-of-two multiple of 1KB. */
int
brw_get_scratch_size(int size)
{
   int i;

   for (i = 1024; i < size; i *= 2)
      ;

   return i;
}

void
brw_get_scratch_bo(struct brw_context *brw,
		   drm_intel_bo **scratch_bo, int size)
{
   drm_intel_bo *old_bo = *scratch_bo;

   if (old_bo && old_bo->size < size) {
      drm_intel_bo_unreference(old_bo);
      old_bo = NULL;
   }

   if (!old_bo) {
      *scratch_bo = drm_intel_bo_alloc(brw->bufmgr, "scratch bo", size, 4096);
   }
}

void brwInitFragProgFuncs( struct dd_function_table *functions )
{
   assert(functions->ProgramStringNotify == _tnl_program_string);

   functions->NewProgram = brwNewProgram;
   functions->DeleteProgram = brwDeleteProgram;
   functions->ProgramStringNotify = brwProgramStringNotify;

   functions->NewShader = brw_new_shader;
   functions->LinkShader = brw_link_shader;

   functions->MemoryBarrier = brw_memory_barrier;
}

void
brw_init_shader_time(struct brw_context *brw)
{
   const int max_entries = 4096;
   brw->shader_time.bo = drm_intel_bo_alloc(brw->bufmgr, "shader time",
                                            max_entries * SHADER_TIME_STRIDE,
                                            4096);
   brw->shader_time.names = rzalloc_array(brw, const char *, max_entries);
   brw->shader_time.ids = rzalloc_array(brw, int, max_entries);
   brw->shader_time.types = rzalloc_array(brw, enum shader_time_shader_type,
                                          max_entries);
   brw->shader_time.cumulative = rzalloc_array(brw, uint64_t,
                                               max_entries);
   brw->shader_time.max_entries = max_entries;
}

static int
compare_time(const void *a, const void *b)
{
   uint64_t * const *a_val = a;
   uint64_t * const *b_val = b;

   /* We don't just subtract because we're turning the value to an int. */
   if (**a_val < **b_val)
      return -1;
   else if (**a_val == **b_val)
      return 0;
   else
      return 1;
}

static void
get_written_and_reset(struct brw_context *brw, int i,
                      uint64_t *written, uint64_t *reset)
{
   enum shader_time_shader_type type = brw->shader_time.types[i];
   assert(type == ST_VS || type == ST_GS || type == ST_FS8 ||
          type == ST_FS16 || type == ST_CS);

   /* Find where we recorded written and reset. */
   int wi, ri;

   for (wi = i; brw->shader_time.types[wi] != type + 1; wi++)
      ;

   for (ri = i; brw->shader_time.types[ri] != type + 2; ri++)
      ;

   *written = brw->shader_time.cumulative[wi];
   *reset = brw->shader_time.cumulative[ri];
}

static void
print_shader_time_line(const char *stage, const char *name,
                       int shader_num, uint64_t time, uint64_t total)
{
   fprintf(stderr, "%-6s%-18s", stage, name);

   if (shader_num != 0)
      fprintf(stderr, "%4d: ", shader_num);
   else
      fprintf(stderr, "    : ");

   fprintf(stderr, "%16lld (%7.2f Gcycles)      %4.1f%%\n",
           (long long)time,
           (double)time / 1000000000.0,
           (double)time / total * 100.0);
}

static void
brw_report_shader_time(struct brw_context *brw)
{
   if (!brw->shader_time.bo || !brw->shader_time.num_entries)
      return;

   uint64_t scaled[brw->shader_time.num_entries];
   uint64_t *sorted[brw->shader_time.num_entries];
   uint64_t total_by_type[ST_CS + 1];
   memset(total_by_type, 0, sizeof(total_by_type));
   double total = 0;
   for (int i = 0; i < brw->shader_time.num_entries; i++) {
      uint64_t written = 0, reset = 0;
      enum shader_time_shader_type type = brw->shader_time.types[i];

      sorted[i] = &scaled[i];

      switch (type) {
      case ST_VS_WRITTEN:
      case ST_VS_RESET:
      case ST_GS_WRITTEN:
      case ST_GS_RESET:
      case ST_FS8_WRITTEN:
      case ST_FS8_RESET:
      case ST_FS16_WRITTEN:
      case ST_FS16_RESET:
      case ST_CS_WRITTEN:
      case ST_CS_RESET:
         /* We'll handle these when along with the time. */
         scaled[i] = 0;
         continue;

      case ST_VS:
      case ST_GS:
      case ST_FS8:
      case ST_FS16:
      case ST_CS:
         get_written_and_reset(brw, i, &written, &reset);
         break;

      default:
         /* I sometimes want to print things that aren't the 3 shader times.
          * Just print the sum in that case.
          */
         written = 1;
         reset = 0;
         break;
      }

      uint64_t time = brw->shader_time.cumulative[i];
      if (written) {
         scaled[i] = time / written * (written + reset);
      } else {
         scaled[i] = time;
      }

      switch (type) {
      case ST_VS:
      case ST_GS:
      case ST_FS8:
      case ST_FS16:
      case ST_CS:
         total_by_type[type] += scaled[i];
         break;
      default:
         break;
      }

      total += scaled[i];
   }

   if (total == 0) {
      fprintf(stderr, "No shader time collected yet\n");
      return;
   }

   qsort(sorted, brw->shader_time.num_entries, sizeof(sorted[0]), compare_time);

   fprintf(stderr, "\n");
   fprintf(stderr, "type          ID                  cycles spent                   %% of total\n");
   for (int s = 0; s < brw->shader_time.num_entries; s++) {
      const char *stage;
      /* Work back from the sorted pointers times to a time to print. */
      int i = sorted[s] - scaled;

      if (scaled[i] == 0)
         continue;

      int shader_num = brw->shader_time.ids[i];
      const char *shader_name = brw->shader_time.names[i];

      switch (brw->shader_time.types[i]) {
      case ST_VS:
         stage = "vs";
         break;
      case ST_GS:
         stage = "gs";
         break;
      case ST_FS8:
         stage = "fs8";
         break;
      case ST_FS16:
         stage = "fs16";
         break;
      case ST_CS:
         stage = "cs";
         break;
      default:
         stage = "other";
         break;
      }

      print_shader_time_line(stage, shader_name, shader_num,
                             scaled[i], total);
   }

   fprintf(stderr, "\n");
   print_shader_time_line("total", "vs", 0, total_by_type[ST_VS], total);
   print_shader_time_line("total", "gs", 0, total_by_type[ST_GS], total);
   print_shader_time_line("total", "fs8", 0, total_by_type[ST_FS8], total);
   print_shader_time_line("total", "fs16", 0, total_by_type[ST_FS16], total);
   print_shader_time_line("total", "cs", 0, total_by_type[ST_CS], total);
}

static void
brw_collect_shader_time(struct brw_context *brw)
{
   if (!brw->shader_time.bo)
      return;

   /* This probably stalls on the last rendering.  We could fix that by
    * delaying reading the reports, but it doesn't look like it's a big
    * overhead compared to the cost of tracking the time in the first place.
    */
   drm_intel_bo_map(brw->shader_time.bo, true);

   uint32_t *times = brw->shader_time.bo->virtual;

   for (int i = 0; i < brw->shader_time.num_entries; i++) {
      brw->shader_time.cumulative[i] += times[i * SHADER_TIME_STRIDE / 4];
   }

   /* Zero the BO out to clear it out for our next collection.
    */
   memset(times, 0, brw->shader_time.bo->size);
   drm_intel_bo_unmap(brw->shader_time.bo);
}

void
brw_collect_and_report_shader_time(struct brw_context *brw)
{
   brw_collect_shader_time(brw);

   if (brw->shader_time.report_time == 0 ||
       get_time() - brw->shader_time.report_time >= 1.0) {
      brw_report_shader_time(brw);
      brw->shader_time.report_time = get_time();
   }
}

/**
 * Chooses an index in the shader_time buffer and sets up tracking information
 * for our printouts.
 *
 * Note that this holds on to references to the underlying programs, which may
 * change their lifetimes compared to normal operation.
 */
int
brw_get_shader_time_index(struct brw_context *brw,
                          struct gl_shader_program *shader_prog,
                          struct gl_program *prog,
                          enum shader_time_shader_type type)
{
   int shader_time_index = brw->shader_time.num_entries++;
   assert(shader_time_index < brw->shader_time.max_entries);
   brw->shader_time.types[shader_time_index] = type;

   int id = shader_prog ? shader_prog->Name : prog->Id;
   const char *name;
   if (id == 0) {
      name = "ff";
   } else if (!shader_prog) {
      name = "prog";
   } else if (shader_prog->Label) {
      name = ralloc_strdup(brw->shader_time.names, shader_prog->Label);
   } else {
      name = "glsl";
   }

   brw->shader_time.names[shader_time_index] = name;
   brw->shader_time.ids[shader_time_index] = id;

   return shader_time_index;
}

void
brw_destroy_shader_time(struct brw_context *brw)
{
   drm_intel_bo_unreference(brw->shader_time.bo);
   brw->shader_time.bo = NULL;
}

void
brw_mark_surface_used(struct brw_stage_prog_data *prog_data,
                      unsigned surf_index)
{
   assert(surf_index < BRW_MAX_SURFACES);

   prog_data->binding_table.size_bytes =
      MAX2(prog_data->binding_table.size_bytes, (surf_index + 1) * 4);
}

bool
brw_stage_prog_data_compare(const struct brw_stage_prog_data *a,
                            const struct brw_stage_prog_data *b)
{
   /* Compare all the struct up to the pointers. */
   if (memcmp(a, b, offsetof(struct brw_stage_prog_data, param)))
      return false;

   if (memcmp(a->param, b->param, a->nr_params * sizeof(void *)))
      return false;

   if (memcmp(a->pull_param, b->pull_param, a->nr_pull_params * sizeof(void *)))
      return false;

   return true;
}

void
brw_stage_prog_data_free(const void *p)
{
   struct brw_stage_prog_data *prog_data = (struct brw_stage_prog_data *)p;

   ralloc_free(prog_data->param);
   ralloc_free(prog_data->pull_param);
}

void
brw_dump_ir(const char *stage, struct gl_shader_program *shader_prog,
            struct gl_shader *shader, struct gl_program *prog)
{
   if (shader_prog) {
      fprintf(stderr,
              "GLSL IR for native %s shader %d:\n", stage, shader_prog->Name);
      _mesa_print_ir(stderr, shader->ir, NULL);
      fprintf(stderr, "\n\n");
   } else {
      fprintf(stderr, "ARB_%s_program %d ir for native %s shader\n",
              stage, prog->Id, stage);
      _mesa_print_program(prog);
   }
}

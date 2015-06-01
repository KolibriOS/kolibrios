/*
 * Copyright © 2015 Intel Corporation
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

#pragma once

#include "brw_context.h"
#include "glsl/nir/nir.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Flags set in the instr->pass_flags field by i965 analysis passes */
enum {
   BRW_NIR_NON_BOOLEAN           = 0x0,

   /* Indicates that the given instruction's destination is a boolean
    * value but that it needs to be resolved before it can be used.
    * On Gen <= 5, CMP instructions return a 32-bit value where the bottom
    * bit represents the actual true/false value of the compare and the top
    * 31 bits are undefined.  In order to use this value, we have to do a
    * "resolve" operation by replacing the value of the CMP with -(x & 1)
    * to sign-extend the bottom bit to 0/~0.
    */
   BRW_NIR_BOOLEAN_NEEDS_RESOLVE = 0x1,

   /* Indicates that the given instruction's destination is a boolean
    * value that has intentionally been left unresolved.  Not all boolean
    * values need to be resolved immediately.  For instance, if we have
    *
    *    CMP r1 r2 r3
    *    CMP r4 r5 r6
    *    AND r7 r1 r4
    *
    * We don't have to resolve the result of the two CMP instructions
    * immediately because the AND still does an AND of the bottom bits.
    * Instead, we can save ourselves instructions by delaying the resolve
    * until after the AND.  The result of the two CMP instructions is left
    * as BRW_NIR_BOOLEAN_UNRESOLVED.
    */
   BRW_NIR_BOOLEAN_UNRESOLVED    = 0x2,

   /* Indicates a that the given instruction's destination is a boolean
    * value that does not need a resolve.  For instance, if you AND two
    * values that are BRW_NIR_BOOLEAN_NEEDS_RESOLVE then we know that both
    * values will be 0/~0 before we get them and the result of the AND is
    * also guaranteed to be 0/~0 and does not need a resolve.
    */
   BRW_NIR_BOOLEAN_NO_RESOLVE    = 0x3,

   /* A mask to mask the boolean status values off of instr->pass_flags */
   BRW_NIR_BOOLEAN_MASK          = 0x3,
};

void brw_nir_analyze_boolean_resolves(nir_shader *nir);

nir_shader *brw_create_nir(struct brw_context *brw,
                           const struct gl_shader_program *shader_prog,
                           const struct gl_program *prog,
                           gl_shader_stage stage);

#ifdef __cplusplus
}
#endif

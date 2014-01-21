#ifndef GEN4_VERTEX_H
#define GEN4_VERTEX_H

#include "compiler.h"

#include "sna.h"
#include "sna_render.h"

void gen4_vertex_align(struct sna *sna, const struct sna_composite_op *op);
void gen4_vertex_flush(struct sna *sna);
int gen4_vertex_finish(struct sna *sna);
void gen4_vertex_close(struct sna *sna);

unsigned gen4_choose_composite_emitter(struct sna *sna, struct sna_composite_op *tmp);

#endif /* GEN4_VERTEX_H */

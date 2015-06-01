
#ifndef _NINE_FF_H_
#define _NINE_FF_H_

#include "device9.h"

boolean nine_ff_init(struct NineDevice9 *);
void    nine_ff_fini(struct NineDevice9 *);

void nine_ff_update(struct NineDevice9 *);

void
nine_d3d_matrix_matrix_mul(D3DMATRIX *, const D3DMATRIX *, const D3DMATRIX *);

void
nine_d3d_vector4_matrix_mul(D3DVECTOR *, const D3DVECTOR *, const D3DMATRIX *);
void
nine_d3d_vector3_matrix_mul(D3DVECTOR *, const D3DVECTOR *, const D3DMATRIX *);

float
nine_d3d_matrix_det(const D3DMATRIX *);

void
nine_d3d_matrix_inverse(D3DMATRIX *, const D3DMATRIX *);

void
nine_d3d_matrix_inverse_3x3(D3DMATRIX *, const D3DMATRIX *);

void
nine_d3d_matrix_transpose(D3DMATRIX *, const D3DMATRIX *);

#endif /* _NINE_FF_H_ */

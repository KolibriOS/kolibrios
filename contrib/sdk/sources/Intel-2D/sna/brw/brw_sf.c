#include "brw.h"

bool brw_sf_kernel__nomask(struct brw_compile *p)
{
	struct brw_reg inv, v0, v1, v2, delta;

	v0 = brw_vec4_grf(3, 0);
	v1 = brw_vec4_grf(4, 0);
	v2 = brw_vec4_grf(5, 0);
	delta = brw_vec8_grf(7, 0);

	inv = brw_vec4_grf(6, 0);
	brw_math_invert(p, inv, brw_vec4_grf(1, 11));

	brw_MOV(p, brw_message_reg(3), v0);

	brw_ADD(p, delta, v1, brw_negate(v2));
	brw_MUL(p, brw_message_reg(1), delta, brw_vec1_grf(6,0));

	brw_ADD(p, delta, v2, brw_negate(v0));
	brw_MUL(p, brw_message_reg(2), delta, brw_vec1_grf(6,2));

	brw_urb_WRITE(p, brw_null_reg(), 0, brw_vec8_grf(0 ,0),
		      false, true, 4, 0, true, true, 0,
		      BRW_URB_SWIZZLE_TRANSPOSE);

	return true;
}

bool brw_sf_kernel__mask(struct brw_compile *p)
{
	struct brw_reg inv, v0, v1, v2;

	v0 = brw_vec8_grf(3, 0);
	v1 = brw_vec8_grf(4, 0);
	v2 = brw_vec8_grf(5, 0);

	inv = brw_vec4_grf(6, 0);
	brw_math_invert(p, inv, brw_vec4_grf(1, 11));

	brw_MOV(p, brw_message_reg(3), v0);

	brw_ADD(p, brw_vec8_grf(7, 0), v1, brw_negate(v2));
	brw_MUL(p, brw_message_reg(1), brw_vec8_grf(7, 0), brw_vec1_grf(6,0));

	brw_ADD(p, brw_vec8_grf(7, 0), v2, brw_negate(v0));
	brw_MUL(p, brw_message_reg(2), brw_vec8_grf(7, 0), brw_vec1_grf(6,2));

	brw_urb_WRITE(p, brw_null_reg(), 0, brw_vec8_grf(0 ,0),
		      false, true, 4, 0, true, true, 0,
		      BRW_URB_SWIZZLE_TRANSPOSE);

	return true;
}

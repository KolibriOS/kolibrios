#include <stdlib.h>
#include "pxdraw.h"

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

rgn_t* create_round_rect_rgn(int left, int top, int right, int bottom,
	int ellipse_width, int ellipse_height)
{
	rgn_t  *obj;
	rect_t *rects;
	int a, b, i, x, y;
	int64_t asq, bsq, dx, dy, err;

	right--;
	bottom--;

    ellipse_width  = min(right - left, abs(ellipse_width));
	ellipse_height = min(bottom - top, abs(ellipse_height));

	obj = malloc(sizeof(rgn_t));
	if (obj == NULL)
		return NULL;

	obj->num_rects = ellipse_height;
	obj->extents.l = left;
	obj->extents.t = top;
	obj->extents.r = right;
	obj->extents.b = bottom;

	obj->rects = rects = malloc(obj->num_rects * sizeof(rect_t));
	if (rects == NULL)
	{
		free(obj);
		return NULL;
	};

	/* based on an algorithm by Alois Zingl */

	a = ellipse_width - 1;
	b = ellipse_height - 1;
	asq = (int64_t)8 * a * a;
	bsq = (int64_t)8 * b * b;
	dx = (int64_t)4 * b * b * (1 - a);
	dy = (int64_t)4 * a * a * (1 + (b % 2));
	err = dx + dy + a * a * (b % 2);

	x = 0;
	y = ellipse_height / 2;

	rects[y].l = left;
	rects[y].r = right;

	while (x <= ellipse_width / 2)
	{
		int64_t e2 = 2 * err;
		if (e2 >= dx)
		{
			x++;
			err += dx += bsq;
		}
		if (e2 <= dy)
		{
			y++;
			err += dy += asq;
			rects[y].l = left + x;
			rects[y].r = right - x;
		}
	}

	for (i = 0; i < ellipse_height / 2; i++)
	{
		rects[i].l = rects[b - i].l;
		rects[i].r = rects[b - i].r;
		rects[i].t = top + i;
		rects[i].b = rects[i].t + 1;
	}
	for (; i < ellipse_height; i++)
	{
		rects[i].t = bottom - ellipse_height + i;
		rects[i].b = rects[i].t + 1;
	}
	rects[ellipse_height / 2].t = top + ellipse_height / 2;  /* extend to top of rectangle */

	return obj;
};

void destroy_region(rgn_t *rgn)
{
	free(rgn->rects);
	free(rgn);
};


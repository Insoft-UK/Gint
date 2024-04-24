#include <gint/gray.h>
#include "../render-fx/render-fx.h"
#include "../render-fx/bopti-asm.h"

#pragma GCC optimize("O3")

/* gsubimage(): Render a section of an image */
void gsubimage(bopti_image_t const *img, struct rbox *r, GUNUSED int flags)
{
	uint32_t *light, *dark;
	dgray_getvram(&light, &dark);

	/* Intersect the bounding box with both the source image and the VRAM,
	   except if DIMAGE_NOCLIP is provided */
	if(!(flags & DIMAGE_NOCLIP))
	{
		/* Early finish for empty intersections */
		if(bopti_clip(img, r)) return;
	}

	int left = r->left;
	int width = r->width;
	int visual_x = r->visual_x;

	r->left = left >> 5;
	r->columns = ((left + width - 1) >> 5) - r->left + 1;

	if(r->columns == 1 && (visual_x & 31) + width <= 32)
	{
		r->x = (left & 31) - (visual_x & 31);
		bopti_render_scsp(img, r, light, dark);
	}
	else
	{
		/* x-coordinate of the first pixel of the first column */
		r->x = visual_x - (left & 31);
		bopti_render(img, r, light, dark);
	}
}

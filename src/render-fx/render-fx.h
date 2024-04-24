//---
//	render-fx - Internal definitions for the display module on fx9860g
//---

#ifndef RENDER_FX
#define RENDER_FX

#include <gint/defs/types.h>
#include <gint/display.h>
#include "bopti-asm.h"

/* masks(): Compute the vram masks for a given rectangle

   Since the VRAM is line-based with four uin32_t elements per row, we can
   execute any operation on a rectangle by running it on each set of four
   uint32_t elements.

   This function calculates four uint32_t values and stores them in @mask. Each
   of the 128 bits in this array represents a column of the screen, and the bit
   of column c is 1 iff x1 <= c <= x2.

   These masks can then be and-ed/or-ed/anything on the VRAM to draw.

   @x1 @x2  Targeted screen range, horizontally (both included)
   @masks   Stores the result of the function (four uint32_t values) */
void masks(int x1, int x2, uint32_t *masks);

/* bopti_clip(): Clip a bounding box to image and VRAM
   @img   Image encoded by [fxconv]
   @rbox  Rendering box */
int bopti_clip(bopti_image_t const *img, struct rbox *rbox);

/* bopti_render(): Render a bopti image
   Copies an image into the VRAM. This function does not perform clipping;
   use bopti_clip() on the rbox before calling it if needed.

   @img     Image encoded by [fxconv]
   @rbox    Rendering box (may or may not be clipped)
   @v1 @v2  VRAMs (gray rendering is used if v2 != NULL) */
void bopti_render(bopti_image_t const *img, struct rbox *rbox, uint32_t *v1,
    uint32_t *v2);

/* bopti_render_scsp(): Single-column single-position image renderer
   This function is a specialized version of bopti_render() that can be used
   when only a single column of the source image is used (all pixels to be
   rendered are in a single 32-aligned 32-wide pixel column of the source) and
   a single position of the VRAM is used (all pixels to be rendered end up in a
   single 32-aligned 32-wide pixel column of the VRAM). */
void bopti_render_scsp(bopti_image_t const *img, struct rbox *rbox,
    uint32_t *v1, uint32_t *v2);

//---
//	Alternate rendering modes
//---

/* The gray engine overrides the rendering functions by specifying a set of
   alternate primitives that are suited to work with two VRAMs. To avoid
   linking with them when the gray engine is not used, the display module
   exposes a global state in the form of a struct rendering_mode and the gray
   engine modifies that state when it runs. */
struct rendering_mode
{
	/* Because the gray engine still has business to do after the call to
	   dgray(DGRAY_OFF), the original dupdate() is made to execute after
	   the replacement one if the replacement one returns 1. */
	int (*dupdate)(void);
	/* Area rendering */
	void (*dclear)(color_t color);
	void (*drect)(int x1, int y1, int x2, int y2, color_t color);
	/* Point rendering */
	void (*dpixel)(int x, int y, color_t color);
	int (*dgetpixel)(int x, int y);
	void (*gint_dhline)(int x1, int x2, int y, int color);
	void (*gint_dvline)(int y1, int y2, int x, int color);
	/* Text and image rendering */
	void (*dtext_opt)
		(int x, int y, int fg, int bg, int halign, int valign,
		 char const *str, int size);
	void (*dsubimage)
		(bopti_image_t const *image, struct rbox *r, int flags);
};

/* The alternate rendering mode pointer (initially NULL)*/
extern struct rendering_mode const *dmode;

/* These are the corresponding gray rendering functions */
int gupdate(void);
void gclear(color_t color);
void grect(int x1, int y1, int x2, int y2, color_t color);
void gpixel(int x, int y, color_t color);
int ggetpixel(int x, int y);
void gint_ghline(int x1, int x2, int y, int color);
void gint_gvline(int y1, int y2, int x, int color);
void gtext_opt(int x, int y, int fg, int bg, int halign, int valign,
	 char const *str, int size);
void gsubimage(bopti_image_t const *image, struct rbox *r, int flags);

/* Short macro to call the alternate rendering function when available */
#define DMODE_OVERRIDE(func, ...)			\
	if(dmode && dmode->func) {			\
		return dmode->func(__VA_ARGS__);	\
	}

#endif /* RENDER_FX */

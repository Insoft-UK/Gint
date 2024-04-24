//---
//	gint:render-fx:bopti-asm - Assembler drawing routines for bopti
//---

#ifndef GINT_RENDERFX_BOPTIASM
#define GINT_RENDERFX_BOPTIASM

#include <gint/defs/types.h>
#include <gint/defs/attributes.h>

/* pair_t: A pair of consecutive VRAM longwords */
typedef struct {
	uint32_t l;
	uint32_t r;
} pair_t;

/* quadr_t: Two pairs for light and gray VRAMs */
typedef struct {
	uint32_t l1;
	uint32_t r1;
	uint32_t l2;
	uint32_t r2;
} quadr_t;

/* Signature of mono rendering functions */
typedef pair_t asm_mono_t(pair_t p,  void **layer, uint32_t *masks, int x);
/* Signature of gray rendering functions */
typedef void asm_gray_t(quadr_t q, void **layer, uint32_t *masks, int x,
	quadr_t *ret);
/* Signature of mono single-column single-position rendering functions */
typedef void asm_mono_scsp_t(uint32_t *vram, uint32_t const *layer,
	uint32_t mask, int x);
/* Signature of gray single-column single-position rendering functions */
typedef void asm_gray_scsp_t(uint32_t *v1, uint32_t const *layer,
	uint32_t mask, uint32_t *v2, int x);

/* Type of any rendering function */
typedef union {
	void *gen;
	asm_mono_t *asm_mono;
	asm_gray_t *asm_gray;
	asm_mono_scsp_t *asm_mono_scsp;
	asm_gray_scsp_t *asm_gray_scsp;
} bopti_asm_t;

/* Each of the following rendering functions:
   1. Takes VRAM data for two longword positions of the screen.
   2. Reads data for one longword position of the image from *layer. This
      consists in n longwords where n is the number of layers in the image.
   3. Increments *layer by 4*n.
   4. Shifts the image data and apply it to the VRAM positions in accordance
      with the two masks given in the masks argument. */

/* bopti_asm_mono(): Rendering function for the "mono" profile */
extern asm_mono_t bopti_asm_mono;
/* bopti_asm_mono_alpha(): Rendering function for the "mono alpha" profile */
extern asm_mono_t bopti_asm_mono_alpha;

/* bopti_gasm_mono(): "mono" profile on gray VRAMs */
extern asm_gray_t bopti_gasm_mono;
/* bopti_gasm_mono_alpha(): "mono_alpha" profile on gray VRAMs */
extern asm_gray_t bopti_gasm_mono_alpha;
/* bopti_asm_gray(): Rendering function for the "gray" profile */
extern asm_gray_t bopti_gasm_gray;
/* bpoti_asm_gray_alpha(): Rendering function for the "gray_alpha" profile */
extern asm_gray_t bopti_gasm_gray_alpha;

/* Each of the following rendering functions:
   1. Takes VRAM data from one longword position of the screen.
   2. Reads data from one longword position of the image from layer.
   3. Shifts the image data and applies it to the VRAM position.
   None update the layer pointer. */

/* bopti_asm_mono_scsp(): SCSP "mono" profile */
extern asm_mono_scsp_t bopti_asm_mono_scsp;
/* bopti_asm_mono_alpha_scsp(): SCSP "mono_alpha" profile */
extern asm_mono_scsp_t bopti_asm_mono_alpha_scsp;

/* bopti_gasm_mono_scsp(): SCSP "mono" profile on gray VRAMs */
extern asm_gray_scsp_t bopti_gasm_mono_scsp;
/* bopti_gasm_mono_scsp_alpha(): SCSP "mono_alpha" profile on gray VRAMs */
extern asm_gray_scsp_t bopti_gasm_mono_alpha_scsp;
/* bopti_asm_gray_scsp(): SCSP "gray" profile */
extern asm_gray_scsp_t bopti_gasm_gray_scsp;
/* bpoti_asm_gray_alpha_scsp(): SCSP "gray_alpha" profile */
extern asm_gray_scsp_t bopti_gasm_gray_alpha_scsp;

//---
//	Renderer's data structures
//---

/* struct rbox: A rendering box (target coordinates and source rectangle)
   Meaning of fields vary during the rendering process! */
struct rbox
{
	/* General renderer:
	     On-screen location of the leftmost pixel of the leftmost rendered
	     column (this particular pixel might not be drawn but is of
	     importance in the positioning process)
	   SCSP renderer:
	     Shift value used to align columns with positions */
	int x;
	/* On-screen location of top-left corner; the (x,y) of dsubimage() */
	int visual_x, y;
	/* Width of rendered sub-image */
	int width;
	/* Before bopti_render{_scsp}():
	     Left-coordinate of the source box (included, in pixels)
	   In bopti_render{_scsp}():
	     Left-coordinate of the source box (included, in columns) */
	int left;
	/* Number of columns used in the source box */
	int columns;
	/* Vertical bounds of the box in the image (inc-excluded, in pixels) */
	int top, height;
};

#endif /* GINT_RENDERFX_BOPTIASM */

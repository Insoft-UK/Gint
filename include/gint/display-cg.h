//---
// gint:display-cg - fx-CG 50 rendering functions
//
// This module covers rendering functions specific to the fx-CG 50. In addition
// to triple-buffering management, this mainly includes image manipulation
// tools as well as the very versatile dimage_effect() and dsubimage_effect()
// functions that support high-performance image rendering with a number of
// geometric and color effects.
//
// The fx-CG OS restricts the display to a 384x216 rectangle rougly around the
// center, leaving margins on three sides. However, gint configures the display
// to use the full 396x224 surface!
//---

#ifndef GINT_DISPLAY_CG
#define GINT_DISPLAY_CG

#ifdef FXCG50

#ifdef __cplusplus
extern "C" {
#endif

#include <gint/defs/types.h>
#include <gint/image.h>

/* Dimensions of the VRAM */
#define DWIDTH 396
#define DHEIGHT 224

/* gint VRAM address. This value must always point to a 32-aligned buffer of
   size 177408. Any function can use it freely to perform rendering or store
   data when not drawing. Triple buffering is already implemented in gint, see
   the dvram() function below.

   In this module, colors are in the 16-bit big-endian R5G6B5 format, as it is
   the format used by the display controller. */
extern uint16_t *gint_vram;

/* Provide a platform-agnostic definition of color_t.
   Some functions also support transparency, in which case they take an [int]
   as argument and recognize negative values as transparent. */
typedef uint16_t color_t;

enum {
	/* Compatibility with fx9860g color names */
	C_WHITE = 0xffff,
	C_LIGHT = 0xad55,
	C_DARK  = 0x528a,
	C_BLACK = 0x0000,

	/* Other colors */
	C_RED   = 0xf800,
	C_GREEN = 0x07e0,
	C_BLUE  = 0x001f,

	C_NONE = -1,
	C_INVERT = -2,
};

/* RGB color maker. Takes three channels of value 0..31 each (the extra bit of
   green is not used). */
#define C_RGB(r,g,b) (((r) << 11) | ((g) << 6) | (b))

/* See <gint/image.h> for the details on image manipulation. */
typedef image_t bopti_image_t;


//---
//	Video RAM management
//---

/* dsetvram(): Control video RAM address and triple buffering

   Normal rendering under gint uses double-buffering: there is one image
   displayed on the screen and one in memory, in a region called the video RAM
   (VRAM). The application draws frames in the VRAM then sends them to the
   screen only when they are finished, using dupdate().

   On fx-CG, sending frames with dupdate() is a common bottleneck because it
   takes about 11 ms. Fortunately, while the DMA is sending the frame to the
   display, the CPU is available to do work in parallel. This function sets up
   triple buffering (ie. a second VRAM) so that the CPU can start working on
   the next frame while the DMA is sending the current one.

   However, experience shows minimal performance improvements, because writing
   to main RAM does not parallelize with DMA transfers. Since gint 2.8, this
   is no longer the default, and the memory for the extra VRAM is instead
   available via malloc().

   VRAMs must be contiguous, 32-aligned, (2*396*224)-byte buffers with 32 bytes
   of extra data on each side (ie. 32 bytes into a 32-aligned buffer of size
   177472).

   @main       Main VRAM area, used alone if [secondary] is NULL
   @secondary  Additional VRAM area, enables triple buffering if non-NULL */
void dsetvram(uint16_t *main, uint16_t *secondary);

/* dgetvram() - Get VRAM addresses
   Returns the VRAM buffer addresses used to render on fx-CG 50. */
void dgetvram(uint16_t **main, uint16_t **secondary);

#ifdef __cplusplus
}
#endif

#endif /* FXCG50 */

#endif /* GINT_DISPLAY_CG */

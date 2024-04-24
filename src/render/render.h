//---
//	render - Internal definitions for common display functions
//---

#ifndef RENDER_COMMON
#define RENDER_COMMON

#include <gint/display.h>

/* gint_dhline(): Optimized horizontal line
   @x1 @x2 @y  Coordinates of endpoints of line (both included)
   @color      Any color suitable for dline() */
void gint_dhline(int x1, int x2, int y, color_t color);

/* gint_dvline(): Optimized vertical line
   @y1 @y2 @x  Coordinates of endpoints of line (both included)
   @color      Any color suitable for dline() */
void gint_dvline(int y1, int y2, int x, color_t color);

//---
//	Font rendering (topti)
//---

/* Current font */
extern font_t const *topti_font;
/* Default font */
extern font_t const *gint_default_font;

/* topti_utf8_next(): Read the next UTF-8 code point of a string
   Returns the next code point and advances the string. Returns 0 (NUL) at the
   end of the string. */
uint32_t topti_utf8_next(uint8_t const **str_pointer);

/* topti_glyph_index(): Obtain the glyph index of a Unicode code point
   Returns the position of code_point in the character table of the given font,
   or -1 if code_point is not part of that set.
   @f           Font object
   @code_point  Unicode code point to locate the glyph for */
int topti_glyph_index(font_t const *f, uint32_t code_point);

/* topti_offset(): Use a font index to find the location of a glyph
   @f      Font object
   @glyph  Glyph number obtained by charset_decode(), must be nonnegative.
   Returns the offset the this glyph's data in the font's data array. When
   using a proportional font, the size array is not heeded for. */
int topti_offset(font_t const *f, uint glyph);

#endif /* RENDER_COMMON */

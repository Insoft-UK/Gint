//---
//	gint:render-fx:topti-asm - Assembler drawing routines for topti
//---

#ifndef GINT_RENDERFX_TOPTIASM
#define GINT_RENDERFX_TOPTIASM

/* Signature of text rendering functions (which do not render text but really
   just blend a column of operators onto the VRAM */
typedef void asm_text_t(uint32_t *v1, uint32_t *v2, uint32_t *op, int height);

/* One rendering function per color */
extern asm_text_t *topti_asm_text[8];

/* topti_render(): Render a string on the VRAM
   Combines glyph data onto VRAM operands and blits them efficiently onto the
   VRAM. To write a single character, use a 2-byte string with a NUL.

   @x @y    Target position on VRAM
   @str     Text source
   @f       Font
   @asm_fg  Assembler function for text rendering
   @asm_bg  Assembler function for background rendering
   @v1      Monochrome VRAM or light gray VRAM
   @v2      Monochrome or dark gray VRAM
   @size    Maximum number of characters to render */
void topti_render(int x, int y, char const *str, font_t const *f,
	asm_text_t *asm_fg, asm_text_t *asm_bg, uint32_t *v1, uint32_t *v2,
	int size);

#endif /* GINT_RENDERFX_TOPTIASM */

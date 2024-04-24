#include <gint/display.h>
#include <gint/dma.h>

void dclear(uint16_t color)
{
	bool full_width = (dwindow.left == 0 && dwindow.right == DWIDTH);
	bool dma_aligned = !(dwindow.top & 3) && !(dwindow.bottom & 3);

	if(full_width && dma_aligned) {
		uint16_t *vram = gint_vram + DWIDTH * dwindow.top;
		int size_bytes = DWIDTH * (dwindow.bottom - dwindow.top) * 2;
		dma_memset(vram, (color << 16) | color, size_bytes);
	}
	else {
		drect(dwindow.left, dwindow.top, dwindow.right - 1,
			dwindow.bottom - 1, color);
	}
}

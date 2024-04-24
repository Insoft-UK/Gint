#include <gint/display.h>

void dpixel(int x, int y, int color)
{
	if(x < dwindow.left || x >= dwindow.right) return;
	if(y < dwindow.top || y >= dwindow.bottom) return;
	if(color == C_NONE) return;

	int index = DWIDTH * y + x;

	if(color == C_INVERT) gint_vram[index] ^= 0xffff;
	else gint_vram[index] = color;
}

#include <gint/display.h>
#include <gint/drivers/t6k11.h>
#include "render-fx.h"

/* Standard video RAM for fx9860g is 1 bit per pixel */
GSECTION(".bss") GALIGNED(32) static uint32_t fx_vram[256];

/* Here is the definition of the VRAM pointer, exposed in <gint/display.h> */
uint32_t *gint_vram = fx_vram;

/* The current rendering mode */
struct rendering_mode const *dmode = NULL;

/* dupdate(): Push the video RAM to the display driver */
void dupdate(void)
{
	bool run_default = true;

	if(dmode && dmode->dupdate)
	{
		/* Call the overridden dupdate(), but continue if it returns
		   non-zero (this is used when stopping the gray engine) */
		int rc = dmode->dupdate();
		run_default = (rc != 0);
	}
	if(run_default)
	{
		t6k11_display(gint_vram, 0, 64, 16);
	}

	gint_call(dupdate_get_hook());
}
__attribute__((alias("dupdate")))
void _WEAK_dupdate(void);

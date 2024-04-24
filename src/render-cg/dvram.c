#include <gint/display.h>
#include <gint/kmalloc.h>
#include <gint/config.h>

/* Up to two VRAM pointers can be set, for triple buffering. */
static uint16_t *vram_1 = NULL, *vram_2 = NULL;
/* Current VRAM pointer, always equal to either vram_1 or vram_2. */
uint16_t *gint_vram = NULL;

bool dvram_init(void)
{
	int const MARGIN = 32;

	/* Leave MARGIN bytes on each side of the region; this enables some
	   important optimizations in the image renderer. We also add another
	   32 bytes so we can manually 32-align the region */
	uint32_t region = (uint32_t)kmalloc(DWIDTH*DHEIGHT*2 + MARGIN*2 + 32,
#if !defined(GINT_NO_OS_STACK)
		"_ostk"
#else
		NULL
#endif
	);
	if(region == 0)
		return false;

	/* 32-align the region */
	region = (region + 31) & -32;
	/* Skip a MARGIN */
	region += MARGIN;
	/* Use an uncached address */
	region = (region & 0x1fffffff) | 0xa0000000;

	/* Don't enable triple buffering by default */
	vram_1 = (void *)region;
	vram_2 = vram_1;
	gint_vram = vram_1;
	return true;
}

/* dsetvram(): Control video RAM address and triple buffering */
void dsetvram(uint16_t *new_vram_1, uint16_t *new_vram_2)
{
	if(!new_vram_1 && !new_vram_2) return;
	if(!new_vram_1) new_vram_1 = new_vram_2;
	if(!new_vram_2) new_vram_2 = new_vram_1;

	if(gint_vram == vram_1)
		gint_vram = new_vram_1;
	else if(gint_vram == vram_2)
		gint_vram = new_vram_2;

	vram_1 = new_vram_1;
	vram_2 = new_vram_2;
}

/* dgetvram(): Get VRAM addresses */
void dgetvram(uint16_t **ptr_vram_1, uint16_t **ptr_vram_2)
{
	if(ptr_vram_1) *ptr_vram_1 = vram_1;
	if(ptr_vram_2) *ptr_vram_2 = vram_2;
}

/* dvram_switch(): Triple buffering switch
   This function is not part of the API; it is used only by dupdate(). */
void dvram_switch(void)
{
	gint_vram = (gint_vram == vram_1) ? vram_2 : vram_1;
}

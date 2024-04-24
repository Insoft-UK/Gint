//---
//	gint:r61524 - Renesas R61524 driver
//---

#include <gint/defs/types.h>
#include <gint/defs/util.h>
#include <gint/hardware.h>
#include <gint/drivers.h>
#include <gint/drivers/states.h>
#include <gint/dma.h>
#include <gint/drivers/r61524.h>

#define DMA SH7305_DMA
#define POWER SH7305_POWER

//---
//	Device specification sheet
//---

/* Registers and operations */
enum {
	device_code_read		= 0x000,
	driver_output_control		= 0x001,
	entry_mode			= 0x003,
	display_control_2		= 0x008,
	low_power_control		= 0x00b,

	ram_address_horizontal		= 0x200,
	ram_address_vertical		= 0x201,
	write_data			= 0x202,

	horizontal_ram_start		= 0x210,
	horizontal_ram_end		= 0x211,
	vertical_ram_start		= 0x212,
	vertical_ram_end		= 0x213,
};

typedef word_union(entry_mode_t,
	uint TRI	:1;
	uint DFM	:1;
	uint		:1;
	uint BGR	:1;
	uint		:2;
	uint HWM	:1;
	uint		:1;
	uint ORG	:1;
	uint		:1;
	uint ID		:2;
	uint AM		:1;
	uint		:1;
	uint EPF	:2;
);

//---
//	Device communication primitives
//---

/* Interface with the controller */
static volatile uint16_t *intf = (void *)0xb4000000;
/* Bit 4 of Port R controls the RS bit of the display driver */
static volatile uint8_t *PRDR = (void *)0xa405013c;

GINLINE static void select(uint16_t reg)
{
	/* Clear RS and write the register number */
	*PRDR &= ~0x10;
	synco();
	*intf = reg;
	synco();

	/* Set RS back. We don't do this in read()/write() because the display
	   driver is optimized for consecutive GRAM access. LCD-transfers will
	   be faster when executing select() followed by several calls to
	   write(). (Although most applications should use the DMA instead.) */
	*PRDR |= 0x10;
	synco();
}

GINLINE static uint16_t read(void)
{
	return *intf;
}

GINLINE static void write(uint16_t data)
{
	*intf = data;
}

uint16_t r61524_get(int ID)
{
	select(ID);
	return read();
}

void r61524_set(int ID, uint16_t value)
{
	select(ID);
	write(value);
}

//---
//	Window management
//---

void r61524_win_get(uint16_t *HSA, uint16_t *HEA, uint16_t *VSA, uint16_t *VEA)
{
	select(horizontal_ram_start);
	*HSA = read();
	select(horizontal_ram_end);
	*HEA = read();

	select(vertical_ram_start);
	*VSA = read();
	select(vertical_ram_end);
	*VEA = read();
}

void r61524_win_set(uint16_t HSA, uint16_t HEA, uint16_t VSA, uint16_t VEA)
{
	select(horizontal_ram_start);
	write(HSA);
	select(horizontal_ram_end);
	write(HEA);

	select(vertical_ram_start);
	write(VSA);
	select(vertical_ram_end);
	write(VEA);
}

//---
//	Driver functions
//---

/* TODO: r61524: update, backlight, brightness, gamma */

void r61524_start_frame(int xmin, int xmax, int ymin, int ymax)
{
	/* Move the window to the desired region, then select address 0 */
	r61524_win_set(395-xmax, 395-xmin, ymin, ymax);
	select(ram_address_horizontal);
	write(0);
	select(ram_address_vertical);
	write(0);

	/* Bind address 0xb4000000 to the data write command */
	select(write_data);
}

void r61524_display(uint16_t *vram, int start, int height, int method)
{
	/* Wait for any transfer to finish before using the screen, otherwise
	   the DMA might write data *while* we're sending commands! */
	dma_transfer_wait(0);

	r61524_start_frame(0, 395, start, start + height - 1);

	if(method == R61524_CPU)
	{
		for(int i = 0; i < 396 * height; i++)
			write(vram[i + 396 * start]);
		return;
	}

	void *src = (void *)vram + start * 396*2;
	void *dst = (void *)0xb4000000;

	/* The amount of data sent per row, 396*2, is not a multiple of 32. For
	   now I assume [height] is a multiple of 4, which makes the factor 32
	   appear. */
	int blocks = 99 * (height >> 2);

	if(method == R61524_DMA) {
		dma_transfer_async(0, DMA_32B, blocks, src, DMA_INC, dst,
			DMA_FIXED, GINT_CALL_NULL);
	}
	else {
		/* Transfer atomically */
		dma_transfer_atomic(0, DMA_32B, blocks, src, DMA_INC, dst,
			DMA_FIXED);
	}
}

void r61524_display_rect(uint16_t *vram, int xmin, int xmax, int ymin,
	int ymax)
{
	dma_transfer_wait(0);
	r61524_start_frame(xmin, xmax, ymin, ymax);

	vram += 396 * ymin + xmin;

	for(int y = 0; y < ymax - ymin + 1; y++) {
		for(int x = 0; x < xmax - xmin + 1; x++)
			write(vram[x]);
		vram += 396;
	}
}

//---
// State and driver metadata
//---

static void hsave(r61524_state_t *s)
{
	r61524_win_get(&s->HSA, &s->HEA, &s->VSA, &s->VEA);
}

static void hrestore(r61524_state_t const *s)
{
	r61524_win_set(s->HSA, s->HEA, s->VSA, s->VEA);
}

gint_driver_t drv_r61524 = {
	.name         = "R61524",
	.hsave        = (void *)hsave,
	.hrestore     = (void *)hrestore,
	.state_size   = sizeof(r61524_state_t),
};
GINT_DECLARE_DRIVER(26, drv_r61524);

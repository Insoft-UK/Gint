#include <gint/gint.h>
#include <gint/display.h>
#include <gint/hardware.h>

#include <string.h>

int   __Timer_Install(int id, void (*handler)(void), int delay);
int   __Timer_Start(int id);
int   __Timer_Stop(int id);
int   __Timer_Deinstall(int id);
int   __PutKeyCode(int row, int column, int keycode);
int   __GetKeyWait(int *col,int *row,int type,int time,int menu,uint16_t *key);
void  __ClearKeyBuffer(void); /* ? */
void *__GetVRAMAddress(void);
void  __ConfigureStatusArea(int mode);
void __SetQuitHandler(void (*callback)(void));

static int __osmenu_id;

static void __osmenu_handler(void)
{
	if(isSlim())
		__PutKeyCode(0x07, 0x0A, 0);
	else
		__PutKeyCode(0x04, 0x09, 0);

	__Timer_Stop(__osmenu_id);
	__Timer_Deinstall(__osmenu_id);
}

void gint_osmenu_native(void)
{
	__ClearKeyBuffer();

	#ifdef FX9860G
	memcpy(__GetVRAMAddress(), gint_vram, 1024);
	#endif

	#ifdef FXCG50
	/* Unfortunately ineffective (main menu probably reenables it)
	__ConfigureStatusArea(3); */

	/* TODO: Improve copied VRAM behavior in gint_osmenu() on fxcg50 */
	uint16_t *vram1, *vram2;
	dgetvram(&vram1, &vram2);

	uint16_t *dst = __GetVRAMAddress();
	uint16_t *src = (gint_vram == vram1) ? vram2 + 6 : vram1 + 6;

	for(int y = 0; y < 216; y++, dst+=384, src+=396)
	for(int x = 0; x < 384; x++)
	{
		dst[x] = src[x];
	}
	#endif

	/* Mysteriously crashes when coming back; might be useful another time
	   instead of GetKeyWait()
	int C=0x04, R=0x09;
	__SpecialMatrixCodeProcessing(&C, &R); */

	__osmenu_id = __Timer_Install(0, __osmenu_handler, 0 /* ms */);
	if(__osmenu_id <= 0) return;
	__Timer_Start(__osmenu_id);

	int column, row;
	unsigned short keycode;
	__GetKeyWait(&column, &row,
		0 /* KEYWAIT_HALTON_TIMEROFF */,
		1 /* Delay in seconds */,
		0 /* Enable return to main menu */,
		&keycode);
}

/* gint_osmenu() - switch out of gint and call the calculator's main menu */
void gint_osmenu(void)
{
	gint_world_switch(GINT_CALL(gint_osmenu_native));
}

static gint_call_t __gcall;
static bool __do_world_switch;

static void __handler()
{
	if(__do_world_switch){
		gint_call(__gcall);
	}else{
		/* TODO: quit the world switch */
		gint_call(__gcall);
	}
}

static void __sethandler()
{
	__SetQuitHandler((void *)__handler);
}

void gint_set_quit_handler(gint_call_t gcall, bool do_world_switch)
{
	__gcall = gcall;
	__do_world_switch = do_world_switch;
	gint_world_switch(GINT_CALL(__sethandler));
}

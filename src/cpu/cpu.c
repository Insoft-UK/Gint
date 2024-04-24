//---
// gint:cpu - Driver for CPU built-in features
//---

#include <gint/cpu.h>
#include <gint/drivers.h>
#include <gint/drivers/states.h>
#include <gint/hardware.h>

/* VBR address to be used in the next world's configure() */
static uint32_t configure_VBR = 0;

void cpu_configure_VBR(uint32_t VBR)
{
	configure_VBR = VBR;
}

static void configure(void)
{
	cpu_setVBR(configure_VBR);
	configure_VBR = 0;

	if(isSH4()) {
		/* Set CPUOPM.INTMU. On the fx-CG 50 emulator it is available but
		   ignored by the emulator, so additional checks still need to be done
		   in interrupt handlers. */
		cpu_setCPUOPM(cpu_getCPUOPM() | 0x00000008);

		/* Enable DSP instructions */
		cpu_sr_t SR = cpu_getSR();
		SR.DSP = 1;
		cpu_setSR(SR);
	}
}

//---
// Device state and driver metadata
//---

static void hsave(cpu_state_t *s)
{
	s->VBR = cpu_getVBR();

	if(isSH4()) {
		s->CPUOPM = cpu_getCPUOPM();
		s->SR = cpu_getSR().lword;
	}
}

static void hrestore(cpu_state_t const *s)
{
	cpu_setVBR(s->VBR);

	if(isSH4()) {
		cpu_setCPUOPM(s->CPUOPM);
		cpu_setSR((cpu_sr_t)s->SR);
	}
}

gint_driver_t drv_cpu = {
	.name        = "CPU",
	.configure   = configure,
	.hsave       = (void *)hsave,
	.hrestore    = (void *)hrestore,
	.state_size  = sizeof(cpu_state_t),
};
GINT_DECLARE_DRIVER(00, drv_cpu);

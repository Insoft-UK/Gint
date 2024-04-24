//---
//	core:vbr - VBR-related functions and linker script symbols
//---

#ifndef GINT_CORE_VBR
#define GINT_CORE_VBR

/* The kernel's interrupt and exception handlers' entry points */
void gint_exch(void);
void gint_tlbh(void);
void gint_inth_7705(void);
void gint_inth_7305(void);

/* Size of exception and TLB handlers */
extern char gint_exch_size;
extern char gint_tlbh_size;

#endif /* GINT_CORE_VBR */
